// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// The timing harness shared by the benchmarks of the 2D Arrangements package
// (arr_traits_benchmark, arr_algebraic_benchmark): a stopwatch, the run loop,
// statistics, checksum helpers, and the reporting of the results as a table
// and, optionally, as CSV that compare_benchmarks.py reads.
//
// Every benchmark returns a checksum derived from the result of the timed
// work. Two builds that perform identical work produce identical checksums,
// which guards against comparing timings of different computations.

#ifndef CGAL_ARR_BENCH_HARNESS_HPP
#define CGAL_ARR_BENCH_HARNESS_HPP

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// Timing infrastructure
//-----------------------------------------------------------------------------
// A benchmark brackets the work it wants measured with start()/stop(), so
// that per-repetition setup (e.g., copying an input arrangement) is excluded.
class Stopwatch {
  using Clock = std::chrono::steady_clock;
  Clock::time_point m_start;
  double m_elapsed = 0.0;   // milliseconds
public:
  void start() { m_start = Clock::now(); }
  void stop() {
    m_elapsed +=
      std::chrono::duration<double, std::milli>(Clock::now() - m_start).count();
  }
  double elapsed() const { return m_elapsed; }
};

using Bench_fn = std::function<std::uint64_t(Stopwatch&)>;

struct Benchmark {
  std::string name;
  std::string description;
  Bench_fn    fn;
};

struct Result {
  std::string         name;
  std::vector<double> times;   // milliseconds
  std::uint64_t       checksum = 0;
  bool                consistent = true;  // same checksum in every run
};

inline double median(std::vector<double> v) {
  std::sort(v.begin(), v.end());
  const std::size_t n = v.size();
  return (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

inline double mean(const std::vector<double>& v)
{ return std::accumulate(v.begin(), v.end(), 0.0) / v.size(); }

inline double stddev(const std::vector<double>& v) {
  if (v.size() < 2) return 0.0;
  const double m = mean(v);
  double s = 0.0;
  for (double x : v) s += (x - m) * (x - m);
  return std::sqrt(s / (v.size() - 1));
}

inline Result run(const Benchmark& b, std::size_t warmup, std::size_t repetitions) {
  Result r;
  r.name = b.name;
  for (std::size_t i = 0; i < warmup + repetitions; ++i) {
    Stopwatch sw;
    const std::uint64_t cs = b.fn(sw);
    if (i == 0) r.checksum = cs;
    else if (cs != r.checksum) r.consistent = false;
    if (i >= warmup) r.times.push_back(sw.elapsed());
  }
  return r;
}

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------
// Combine counts into a checksum (order-dependent, FNV-1a style).
inline std::uint64_t mix(std::uint64_t h, std::uint64_t v) {
  h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
  return h;
}

// Prevent the optimizer from discarding results.
inline volatile std::uint64_t g_sink = 0;

template <typename A>
std::uint64_t size_checksum(const A& arr) {
  std::uint64_t h = 0;
  h = mix(h, arr.number_of_vertices());
  h = mix(h, arr.number_of_edges());
  h = mix(h, arr.number_of_faces());
  return h;
}

//-----------------------------------------------------------------------------
// Reporting
//-----------------------------------------------------------------------------
inline void list_benchmarks(const std::vector<Benchmark>& benches) {
  for (const auto& bm : benches)
    std::cout << std::left << std::setw(30) << bm.name << bm.description << "\n";
}

/*! runs the benchmarks whose names contain `filter`, prints a table of the
 * results, and, if `csv_path` is not empty, writes them as CSV preceded by
 * the comment line `csv_comment` (which should start with '#').
 * Returns EXIT_SUCCESS if every benchmark produced the same checksum in all
 * its runs and EXIT_FAILURE otherwise.
 */
inline int run_benchmarks(const std::vector<Benchmark>& benches, const std::string& filter,
                          std::size_t warmup, std::size_t repetitions,
                          const std::string& csv_path, const std::string& csv_comment) {
  std::cout << std::left << std::setw(30) << "benchmark" << std::right
            << std::setw(12) << "min ms" << std::setw(12) << "median ms"
            << std::setw(12) << "mean ms" << std::setw(10) << "cv %"
            << "  checksum\n";

  std::ofstream csv;
  if (! csv_path.empty()) {
    csv.open(csv_path);
    if (! csv) {
      std::cerr << "Cannot open " << csv_path << "\n";
      return EXIT_FAILURE;
    }
    csv << csv_comment << "\n"
        << "name,repetitions,min_ms,median_ms,mean_ms,stddev_ms,checksum,"
           "times_ms\n";
  }

  bool all_consistent = true;
  for (const auto& bm : benches) {
    if (! filter.empty() && bm.name.find(filter) == std::string::npos) continue;
    const Result r = run(bm, warmup, repetitions);
    all_consistent = all_consistent && r.consistent;
    const double mn = *std::min_element(r.times.begin(), r.times.end());
    const double md = median(r.times);
    const double me = mean(r.times);
    const double sd = stddev(r.times);
    std::cout << std::left << std::setw(30) << r.name << std::right
              << std::fixed << std::setprecision(3)
              << std::setw(12) << mn << std::setw(12) << md
              << std::setw(12) << me << std::setprecision(1)
              << std::setw(10) << (me > 0 ? 100.0 * sd / me : 0.0)
              << "  " << std::hex << r.checksum << std::dec
              << (r.consistent ? "" : "  (INCONSISTENT)") << std::endl;
    if (csv) {
      csv << r.name << ',' << r.times.size() << ',' << std::setprecision(6)
          << mn << ',' << md << ',' << me << ',' << sd << ','
          << std::hex << r.checksum << std::dec << ',';
      for (std::size_t k = 0; k < r.times.size(); ++k)
        csv << (k ? ";" : "") << r.times[k];
      csv << "\n";
    }
  }

  if (! all_consistent) {
    std::cerr << "\nWarning: some benchmarks produced different checksums "
                 "across repetitions.\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

#endif // CGAL_ARR_BENCH_HARNESS_HPP
