// Calibrates the automatic choice of the representation of the convolution
// cycles of the full convolution method (see
// CGAL::Minkowski_sum_2::Convolution_curves and prefer_segments() in
// <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>).
//
// For every pair of simple polygons, the program computes the features that
// prefer_segments() uses, namely, the total number of vertices m + n, the
// number E of convolution edges, and the number C of x-monotone chains of the
// convolution cycles, and it times the full convolution method with the
// cycles represented by segments and by polylines.  Every pair is written as a
// row of a CSV file, which can be analyzed further (e.g., with a script that
// searches for better thresholds).  Then, the program summarizes how the rule
// of the library fares compared to choosing the faster representation for
// every pair; an alternative rule, given by its thresholds, is summarized as
// well, so that thresholds can be tuned without timing the pairs again (use
// --input to read the rows written by a former run).
//
// The rule of the library: segments are preferred if E > a(m + n) and, in
// addition, either E > b(m + n), or E < cC and (E > e or E < dC), where
// (a, b, c, d, e) = (4, 20, 40, 15, 500).
//
// Usage: calibrate_convolution_curves [options]
//   --kinds, --sizes, --size2, --ratios, --pairs, --seed, --polygons
//                generate or read the pairs (see make_pairs() in
//                calibration_utils.h); polygons with holes are skipped,
//                as are pairs of convex polygons, whose sum is computed
//                directly
//   --csv FILE   writes the rows to FILE
//   --input FILE reads the rows from FILE (written by --csv) instead of
//                generating and timing pairs
//   --rule a,b,c,d,e
//                the thresholds of an alternative rule to summarize
//
// Build in Release mode; the timings are meaningless otherwise.
//
// Example: calibrate_convolution_curves --kinds cc,xc --sizes 50,100,200 --size2 0.5,1 --csv curves.csv

#include <memory>

#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>

#include "calibration_utils.h"

namespace Ms = CGAL::Minkowski_sum_2;
namespace params = CGAL::parameters;
using namespace calibration;

// A row: the features of a pair and the times of both representations.
struct Row {
  std::string kind, ratio;
  std::size_t n1 = 0, n2 = 0, reflex1 = 0, reflex2 = 0, num_edges = 0, num_chains = 0;
  double seg_ms = 0, poly_ms = 0;
};

// The thresholds of a rule (see the rule of the library above).
struct Rule {
  double a = 4, b = 20, c = 40, d = 15, e = 500;

  bool prefer_segments(const Row& r) const {
    const double mn = double(r.n1 + r.n2), E = double(r.num_edges), C = double(r.num_chains);
    if (E <= a * mn) return false;
    if (E > b * mn) return true;
    return (E < c * C) && ((E > e) || (E < d * C));
  }
};

const char* const header = "kind,ratio,n1,n2,reflex1,reflex2,E,chains,seg_ms,poly_ms,library_choice";

int main(int argc, char* argv[]) {
  const auto options = parse_options(argc, argv);
  std::vector<Row> rows;
  std::vector<bool> library_choice;

  if (options.count("input")) {
    // Read the rows written by a former run.
    std::ifstream in(options.at("input"));
    std::string line;
    std::getline(in, line);
    while (std::getline(in, line)) {
      std::stringstream ss(line);
      std::string field;
      std::vector<std::string> f;
      while (std::getline(ss, field, ',')) f.push_back(field);
      if (f.size() < 11) continue;
      Row r;
      r.kind = f[0]; r.ratio = f[1];
      r.n1 = std::stoul(f[2]); r.n2 = std::stoul(f[3]); r.reflex1 = std::stoul(f[4]); r.reflex2 = std::stoul(f[5]);
      r.num_edges = std::stoul(f[6]); r.num_chains = std::stoul(f[7]);
      r.seg_ms = std::stod(f[8]); r.poly_ms = std::stod(f[9]);
      rows.push_back(r);
      library_choice.push_back(f[10] == "segments");
    }
  }
  else {
    std::ofstream csv;
    if (options.count("csv")) {
      csv.open(options.at("csv"));
      csv << header << "\n";
    }
    std::cout << header << std::endl;
    const auto traits_ptr = std::make_shared<const CGAL::Gps_segment_traits_2<Kernel>>();
    for (const auto& pair : make_pairs(options)) {
      if (pair.pgn1.has_holes() || pair.pgn2.has_holes()) continue;
      const auto& P = pair.pgn1.outer_boundary();
      const auto& Q = pair.pgn2.outer_boundary();
      if (P.is_convex() && Q.is_convex()) continue;

      // The features, computed as the full convolution method computes them
      // for two simple polygons.
      std::vector<std::vector<std::size_t>> ids;
      const auto paths = Ms::internal::generate_convolution_paths(P, Q, traits_ptr, &ids, 0, true);
      Row r{pair.kind, pair.ratio, P.size(), Q.size(), number_of_non_left_vertices(P), number_of_non_left_vertices(Q),
            0, Ms::internal::number_of_x_monotone_chains(paths), 0, 0};
      for (const auto& path : paths) if (path.size() > 1) r.num_edges += path.size() - 1;
      const bool choice = Ms::internal::prefer_segments(paths, P.size() + Q.size());

      auto run = [&](Ms::Convolution_curves curves) {
        return time_ms([&] {
          Ms::minkowski_sum_by_full_convolution_2(P, Q, params::default_values(), params::default_values(),
                                                  params::convolution_curves(curves));
        });
      };
      r.seg_ms = run(Ms::Convolution_curves::SEGMENTS);
      r.poly_ms = run(Ms::Convolution_curves::POLYLINES);

      std::ostringstream line;
      line << r.kind << "," << r.ratio << "," << r.n1 << "," << r.n2 << "," << r.reflex1 << "," << r.reflex2 << ","
           << r.num_edges << "," << r.num_chains << "," << r.seg_ms << "," << r.poly_ms << ","
           << (choice ? "segments" : "polylines");
      std::cout << line.str() << std::endl;
      if (csv) csv << line.str() << "\n";
      rows.push_back(r);
      library_choice.push_back(choice);
    }
  }
  if (rows.empty()) {
    std::cout << "No pairs." << std::endl;
    return EXIT_SUCCESS;
  }

  // Summary; alternative 0 is segments, and alternative 1 is polylines.
  std::vector<std::vector<double>> times;
  for (const auto& r : rows) times.push_back({r.seg_ms, r.poly_ms});
  auto choices = [&](auto choose_segments) {
    std::vector<std::size_t> res;
    for (std::size_t i = 0; i < rows.size(); ++i) res.push_back(choose_segments(i) ? 0 : 1);
    return res;
  };
  summary_header(rows.size());
  summarize("always segments", times, choices([](std::size_t) { return true; }));
  summarize("always polylines", times, choices([](std::size_t) { return false; }));
  summarize("library rule", times, choices([&](std::size_t i) { return bool(library_choice[i]); }));
  if (options.count("rule")) {
    const auto t = split(options.at("rule"));
    if (t.size() != 5) {
      std::cerr << "--rule expects five comma-separated thresholds a,b,c,d,e" << std::endl;
      return EXIT_FAILURE;
    }
    const Rule rule{std::stod(t[0]), std::stod(t[1]), std::stod(t[2]), std::stod(t[3]), std::stod(t[4])};
    summarize("rule " + options.at("rule"), times, choices([&](std::size_t i) { return rule.prefer_segments(rows[i]); }));
  }
  return EXIT_SUCCESS;
}
