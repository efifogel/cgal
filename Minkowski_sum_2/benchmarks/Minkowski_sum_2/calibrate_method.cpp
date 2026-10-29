// Calibrates the automatic choice of the method that computes the Minkowski
// sum of two polygons (see CGAL::Minkowski_sum_2::Method and
// internal::choose_method() in <CGAL/Minkowski_sum_2/minkowski_sum_2.h>).
//
// For every pair of polygons (with holes), the program removes the holes
// that cannot contribute to the boundary of the sum, as the automatic choice
// does, and computes the features that choose_method() uses, namely, the
// numbers m and n of vertices of the polygons, the numbers l1 and l2 of their
// vertices that do not turn left, and the resulting estimate
// E = m + n + l1 n + l2 m of the number of edges of the full convolution.
// Then, it times the decomposition, the reduced convolution, and the full
// convolution methods.  Pairs that are convex once their holes are removed are
// skipped, as their sum is computed directly.  Every pair is written as a row
// of a CSV file, which can be analyzed further.  Then, the program summarizes
// how the rule of the library fares compared to choosing the fastest method for
// every pair; an alternative rule, given by its thresholds, is summarized as
// well, so that thresholds can be tuned without timing the pairs again (use
// --input to read the rows written by a former run).
//
// The rule of the library: the decomposition method is chosen if E > t, both
// polygons have no holes, and l1 > r m and l2 > r n; otherwise, the full
// convolution method is chosen if E <= s(m + n), where s = s1 if
// max(l1, l2) <= k and s = s2 otherwise; otherwise, the reduced convolution
// method is chosen, where (t, r, k, s1, s2) = (40000, 0.3, 30, 8, 2).
//
// Usage: calibrate_method [options]
//   --kinds, --sizes, --size2, --ratios, --pairs, --seed, --polygons
//                generate or read the pairs (see make_pairs() in
//                calibration_utils.h); default kinds: cc,xc,hc,hx,hh
//   --csv FILE   writes the rows to FILE
//   --input FILE reads the rows from FILE (written by --csv) instead of
//                generating and timing pairs
//   --rule t,r,k,s1,s2
//                the thresholds of an alternative rule to summarize
//
// Build in Release mode; the timings are meaningless otherwise.
//
// Example: calibrate_method --kinds cc,hc --sizes 8,16,50,150,300 --size2 0.25,1 --csv methods.csv

#include <memory>

#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Minkowski_sum_2/Hole_filter_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_2.h>

#include "calibration_utils.h"

namespace Ms = CGAL::Minkowski_sum_2;
namespace params = CGAL::parameters;
using namespace calibration;

// The methods, in the order of the alternatives of the summary.
const std::vector<Ms::Method> methods{Ms::Method::DECOMPOSITION, Ms::Method::REDUCED_CONVOLUTION,
                                      Ms::Method::FULL_CONVOLUTION};
const std::vector<std::string> method_names{"decomposition", "reduced", "full"};

// A row: the features of a pair and the times of the methods.
struct Row {
  std::string kind, ratio;
  std::size_t m = 0, n = 0, l1 = 0, l2 = 0, holes1 = 0, holes2 = 0;
  std::vector<double> ms;               // the times of the methods
};

// The thresholds of a rule (see the rule of the library above); the result
// is an index into `methods`.
struct Rule {
  double t = 40000, r = 0.3, k = 30, s1 = 8, s2 = 2;

  std::size_t choose(const Row& row) const {
    const double m = double(row.m), n = double(row.n), l1 = double(row.l1), l2 = double(row.l2);
    const double E = m + n + l1 * n + l2 * m;
    if ((E > t) && (row.holes1 == 0) && (row.holes2 == 0) && (l1 > r * m) && (l2 > r * n)) return 0;
    const double s = (std::max(l1, l2) <= k) ? s1 : s2;
    return (E <= s * (m + n)) ? 2 : 1;
  }
};

const char* const header = "kind,ratio,m,n,l1,l2,holes1,holes2,E,decomposition_ms,reduced_ms,full_ms,library_choice";

int main(int argc, char* argv[]) {
  auto options = parse_options(argc, argv);
  if (! options.count("kinds")) options["kinds"] = "cc,xc,hc,hx,hh";
  std::vector<Row> rows;
  std::vector<std::size_t> library_choice;

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
      if (f.size() < 13) continue;
      Row r;
      r.kind = f[0]; r.ratio = f[1];
      r.m = std::stoul(f[2]); r.n = std::stoul(f[3]); r.l1 = std::stoul(f[4]); r.l2 = std::stoul(f[5]);
      r.holes1 = std::stoul(f[6]); r.holes2 = std::stoul(f[7]);
      r.ms = {std::stod(f[9]), std::stod(f[10]), std::stod(f[11])};
      rows.push_back(r);
      library_choice.push_back(std::size_t(std::find(method_names.begin(), method_names.end(), f[12]) -
                                           method_names.begin()));
    }
  }
  else {
    std::ofstream csv;
    if (options.count("csv")) {
      csv.open(options.at("csv"));
      csv << header << "\n";
    }
    std::cout << header << std::endl;
    const CGAL::Gps_segment_traits_2<Kernel> traits;
    for (const auto& pair : make_pairs(options)) {
      // The features, computed as the automatic choice computes them.
      CGAL::Hole_filter_2<Kernel, Polygon_2::Container> hole_filter;
      Polygon_with_holes_2 filtered1, filtered2;
      hole_filter(pair.pgn1, pair.pgn2, filtered1);
      hole_filter(pair.pgn2, pair.pgn1, filtered2);
      if (Ms::internal::is_filtered_convex(pair.pgn1, filtered1, std::nullopt) &&
          Ms::internal::is_filtered_convex(pair.pgn2, filtered2, std::nullopt))
        continue;
      const auto [m, l1] = Ms::internal::count_vertices(filtered1, traits);
      const auto [n, l2] = Ms::internal::count_vertices(filtered2, traits);
      Row r{pair.kind, pair.ratio, m, n, l1, l2, filtered1.number_of_holes(), filtered2.number_of_holes(), {}};
      const auto choice = std::size_t(std::find(methods.begin(), methods.end(),
                                                Ms::internal::choose_method(filtered1, filtered2, traits)) -
                                      methods.begin());

      for (auto method : methods) {
        r.ms.push_back(time_ms([&] {
          Ms::minkowski_sum_2(pair.pgn1, pair.pgn2, params::default_values(), params::default_values(),
                              params::method(method));
        }));
      }

      std::ostringstream line;
      line << r.kind << "," << r.ratio << "," << r.m << "," << r.n << "," << r.l1 << "," << r.l2 << ","
           << r.holes1 << "," << r.holes2 << "," << (r.m + r.n + r.l1 * r.n + r.l2 * r.m) << ","
           << r.ms[0] << "," << r.ms[1] << "," << r.ms[2] << "," << method_names[choice];
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

  // Summary.
  std::vector<std::vector<double>> times;
  for (const auto& r : rows) times.push_back(r.ms);
  summary_header(rows.size());
  for (std::size_t a = 0; a < methods.size(); ++a)
    summarize("always " + method_names[a], times, std::vector<std::size_t>(rows.size(), a));
  summarize("library rule", times, library_choice);
  if (options.count("rule")) {
    const auto t = split(options.at("rule"));
    if (t.size() != 5) {
      std::cerr << "--rule expects five comma-separated thresholds t,r,k,s1,s2" << std::endl;
      return EXIT_FAILURE;
    }
    const Rule rule{std::stod(t[0]), std::stod(t[1]), std::stod(t[2]), std::stod(t[3]), std::stod(t[4])};
    std::vector<std::size_t> choice;
    for (const auto& r : rows) choice.push_back(rule.choose(r));
    summarize("rule " + options.at("rule"), times, choice);
  }
  return EXIT_SUCCESS;
}
