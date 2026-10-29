// Utilities shared by the calibration programs calibrate_convolution_curves
// and calibrate_method: the generation and the reading of polygon pairs, the
// parsing of the command line, the timing of computations, and the summary of
// a selection rule against an a-posteriori (oracle) selection.

#ifndef CGAL_MINKOWSKI_SUM_2_CALIBRATION_UTILS_H
#define CGAL_MINKOWSKI_SUM_2_CALIBRATION_UTILS_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/random_convex_set_2.h>
#include <CGAL/random_polygon_2.h>
#include <CGAL/Random.h>

namespace calibration {

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;

// The polygons are generated with an inexact-constructions kernel, which is
// much faster, and then converted (exactly) to the exact kernel.
using Gen_point_2 = CGAL::Exact_predicates_inexact_constructions_kernel::Point_2;

// ----------------------------------------------------------------------------
// Polygon generation
// ----------------------------------------------------------------------------

inline std::vector<Point_2> to_exact(const std::vector<Gen_point_2>& points) {
  std::vector<Point_2> res;
  res.reserve(points.size());
  for (const auto& p : points) res.emplace_back(p.x(), p.y());
  return res;
}

// generates a random convex polygon with (about) the given number of vertices.
// The inexact constructions may leave a few vertices that are not strictly
// convex; the convex hull drops them.
inline Polygon_2 convex_polygon(std::size_t num_vertices, double size, CGAL::Random& rnd) {
  std::vector<Gen_point_2> points;
  CGAL::Random_points_in_square_2<Gen_point_2> g(size, rnd);
  CGAL::random_convex_set_2(num_vertices, std::back_inserter(points), g);
  auto exact = to_exact(points);
  std::vector<Point_2> hull;
  CGAL::convex_hull_2(exact.begin(), exact.end(), std::back_inserter(hull));
  return Polygon_2(hull.begin(), hull.end());
}

// generates a random simple polygon with the given number of vertices,
// oriented counterclockwise, by untangling a random closed chain through
// random points (see CGAL::random_polygon_2()).  If no reflex ratio is given,
// the points are drawn uniformly from a square, which yields a highly concave
// polygon.  Otherwise, most points are drawn from a circle and the rest from
// the interior of the disc; the points on the circle become convex vertices of
// every simple polygon through the points, so only interior points can become
// reflex vertices, and the given ratio bounds the ratio between the numbers of
// reflex and convex vertices from above.
inline Polygon_2 concave_polygon(std::size_t num_vertices, double size, std::optional<double> reflex_ratio,
                                 CGAL::Random& rnd, double cx = 0, double cy = 0) {
  std::vector<Gen_point_2> points;
  if (! reflex_ratio) {
    CGAL::Random_points_in_square_2<Gen_point_2> g(size, rnd);
    CGAL::random_polygon_2(num_vertices, std::back_inserter(points), g);
  }
  else {
    auto num_on_circle = static_cast<std::size_t>(std::lround(double(num_vertices) / (1.0 + *reflex_ratio)));
    num_on_circle = std::clamp<std::size_t>(num_on_circle, 3, num_vertices);
    std::vector<Gen_point_2> candidates;
    CGAL::Random_points_on_circle_2<Gen_point_2> on_circle(size, rnd);
    std::copy_n(on_circle, num_on_circle, std::back_inserter(candidates));
    CGAL::Random_points_in_disc_2<Gen_point_2> in_disc(0.95 * size, rnd);
    std::copy_n(in_disc, num_vertices - num_on_circle, std::back_inserter(candidates));
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
    CGAL::random_polygon_2(candidates.size(), std::back_inserter(points), candidates.begin());
  }
  Polygon_2 pgn;
  for (const auto& p : points) pgn.push_back(Point_2(p.x() + cx, p.y() + cy));
  if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
  return pgn;
}

// generates a polygon with holes: a square whose side is `size`, with about
// `num_holes` random small simple holes in a grid of cells (oriented
// clockwise), each with 3 to `max_hole_vertices` vertices.
inline Polygon_with_holes_2 polygon_with_holes(double size, std::size_t num_holes, std::size_t max_hole_vertices,
                                               CGAL::Random& rnd) {
  const auto cells = static_cast<std::size_t>(std::ceil(std::sqrt(double(std::max<std::size_t>(num_holes, 1)))));
  const double cell = size / double(cells);
  std::vector<Polygon_2> holes;
  for (std::size_t x = 0; (x < cells) && (holes.size() < num_holes); ++x) {
    for (std::size_t y = 0; (y < cells) && (holes.size() < num_holes); ++y) {
      const std::size_t n = 3 + std::size_t(rnd.get_int(0, int(std::max<std::size_t>(max_hole_vertices, 3) - 2)));
      holes.push_back(concave_polygon(n, 0.35 * cell, std::nullopt, rnd, cell * (double(x) + 0.5),
                                      cell * (double(y) + 0.5)));
      holes.back().reverse_orientation();
    }
  }
  Polygon_2 outer;
  for (auto p : {Point_2(0, 0), Point_2(size, 0), Point_2(size, size), Point_2(0, size)}) outer.push_back(p);
  return Polygon_with_holes_2(outer, holes.begin(), holes.end());
}

// counts the vertices of a polygon at which it does not turn left (the reflex
// vertices of a counterclockwise polygon, and the collinear vertices).
inline std::size_t number_of_non_left_vertices(const Polygon_2& pgn) {
  if (pgn.size() < 3) return 0;
  std::size_t count = 0;
  auto prev = pgn.vertices_circulator();
  auto curr = prev;
  auto next = curr;
  ++next;
  --prev;
  do {
    if (CGAL::orientation(*prev, *curr, *next) != CGAL::LEFT_TURN) ++count;
    prev = curr;
    curr = next;
    ++next;
  } while (curr != pgn.vertices_circulator());
  return count;
}

// reads a polygon with holes from a file in the format of the input operator
// of CGAL::Polygon_with_holes_2 (the outer boundary, the number of holes, and
// the holes); a file with a single polygon is read as a polygon without holes.
inline Polygon_with_holes_2 read_polygon(const std::string& filename) {
  std::ifstream in(filename);
  if (! in) {
    std::cerr << "Cannot open " << filename << std::endl;
    std::exit(EXIT_FAILURE);
  }
  Polygon_2 outer;
  in >> outer;
  std::size_t num_holes = 0;
  std::vector<Polygon_2> holes;
  if (in >> num_holes) {
    holes.resize(num_holes);
    for (auto& hole : holes) in >> hole;
  }
  if (outer.orientation() == CGAL::CLOCKWISE) outer.reverse_orientation();
  for (auto& hole : holes) if (hole.orientation() == CGAL::COUNTERCLOCKWISE) hole.reverse_orientation();
  return Polygon_with_holes_2(outer, holes.begin(), holes.end());
}

// ----------------------------------------------------------------------------
// Command line
// ----------------------------------------------------------------------------

// parses options of the form `--name value` into a map; an option without a
// value (followed by another option, or last) gets the value "1".
inline std::map<std::string, std::string> parse_options(int argc, char* argv[]) {
  std::map<std::string, std::string> options;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg.rfind("--", 0) != 0) {
      std::cerr << "Unexpected argument " << arg << std::endl;
      std::exit(EXIT_FAILURE);
    }
    arg = arg.substr(2);
    if ((i + 1 < argc) && (std::string(argv[i + 1]).rfind("--", 0) != 0)) options[arg] = argv[++i];
    else options[arg] = "1";
  }
  return options;
}

// splits a comma-separated list.
inline std::vector<std::string> split(const std::string& s) {
  std::vector<std::string> res;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, ',')) if (! item.empty()) res.push_back(item);
  return res;
}

inline std::string option(const std::map<std::string, std::string>& options, const std::string& name,
                          const std::string& default_value) {
  auto it = options.find(name);
  return (it == options.end()) ? default_value : it->second;
}

// ----------------------------------------------------------------------------
// Polygon pairs
// ----------------------------------------------------------------------------

// A pair of polygons and a description of how it was generated.
struct Pair {
  std::string kind;                     // e.g., "cc"
  std::string ratio;                    // the reflex ratio, or "square"
  std::size_t size1 = 0, size2 = 0;     // the requested sizes
  Polygon_with_holes_2 pgn1, pgn2;
};

// generates the pairs given by the options:
//   --kinds  a list of kinds: cc (concave and concave), xc (convex and
//            concave), xx (convex and convex), hc (a polygon with holes and a
//            concave polygon), hx (a polygon with holes and a convex polygon),
//            hh (two polygons with holes); default: cc,xc
//   --sizes  a list of numbers of vertices of the first polygon (for a polygon
//            with holes, its number of holes); default: 15,30,60,120,240
//   --size2  a list of factors of the size of the second polygon relative to
//            the first; default: 1
//   --ratios a list of reflex ratios of concave polygons (see
//            concave_polygon()), or "square"; default: square,0.1,0.3,1
//   --pairs  the number of pairs per combination; default: 2
//   --seed   the seed of the random generator; default: 1
// or reads a single pair, if the option --polygons file1,file2 is given.
inline std::vector<Pair> make_pairs(const std::map<std::string, std::string>& options) {
  std::vector<Pair> pairs;
  if (options.count("polygons")) {
    const auto files = split(options.at("polygons"));
    if (files.size() != 2) {
      std::cerr << "--polygons expects two comma-separated files" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    Pair pair{"file", "-", 0, 0, read_polygon(files[0]), read_polygon(files[1])};
    pair.size1 = pair.pgn1.outer_boundary().size();
    pair.size2 = pair.pgn2.outer_boundary().size();
    pairs.push_back(pair);
    return pairs;
  }
  CGAL::Random rnd(std::stoi(option(options, "seed", "1")));
  const int num_pairs = std::stoi(option(options, "pairs", "2"));
  for (const auto& kind : split(option(options, "kinds", "cc,xc"))) {
    for (const auto& size_str : split(option(options, "sizes", "15,30,60,120,240"))) {
      const std::size_t size = std::stoul(size_str);
      for (const auto& factor_str : split(option(options, "size2", "1"))) {
        const auto size2 = std::max<std::size_t>(3, std::size_t(std::lround(double(size) * std::stod(factor_str))));
        for (const auto& ratio_str : split(option(options, "ratios", "square,0.1,0.3,1"))) {
          const std::optional<double> ratio =
            (ratio_str == "square") ? std::nullopt : std::optional<double>(std::stod(ratio_str));
          for (int k = 0; k < num_pairs; ++k) {
            auto concave = [&](std::size_t n) { return Polygon_with_holes_2(concave_polygon(n, 1, ratio, rnd)); };
            auto convex = [&](std::size_t n) { return Polygon_with_holes_2(convex_polygon(n, 1, rnd)); };
            auto holes = [&](std::size_t n) { return polygon_with_holes(10, n, 8, rnd); };
            Pair pair{kind, ratio_str, size, size2, {}, {}};
            if (kind == "cc") { pair.pgn1 = concave(size); pair.pgn2 = concave(size2); }
            else if (kind == "xc") { pair.pgn1 = convex(size); pair.pgn2 = concave(size2); }
            else if (kind == "xx") { pair.pgn1 = convex(size); pair.pgn2 = convex(size2); }
            else if (kind == "hc") { pair.pgn1 = holes(size); pair.pgn2 = concave(size2); }
            else if (kind == "hx") { pair.pgn1 = holes(size); pair.pgn2 = convex(size2); }
            else if (kind == "hh") { pair.pgn1 = holes(size); pair.pgn2 = holes(size2); }
            else {
              std::cerr << "Unknown kind " << kind << std::endl;
              std::exit(EXIT_FAILURE);
            }
            pairs.push_back(std::move(pair));
          }
          // The reflex ratio does not affect convex polygons and polygons
          // with holes; one ratio suffices for these kinds.
          if ((kind == "xx") || (kind == "hx") || (kind == "hh")) break;
        }
      }
    }
  }
  return pairs;
}

// ----------------------------------------------------------------------------
// Timing
// ----------------------------------------------------------------------------

// times a computation in milliseconds, by the median of several runs: fast
// computations are repeated more (the first run is also a warm-up).
inline double time_ms(const std::function<void()>& f) {
  auto run = [&] {
    const auto start = std::chrono::steady_clock::now();
    f();
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
  };
  const double first = run();
  const int reps = (first < 20) ? std::min(30, int(300 / std::max(first, 0.01)) + 1) : ((first < 300) ? 3 : 1);
  std::vector<double> times;
  for (int i = 0; i < reps; ++i) times.push_back(run());
  std::sort(times.begin(), times.end());
  return times[times.size() / 2];
}

// ----------------------------------------------------------------------------
// Summary
// ----------------------------------------------------------------------------

// prints how a selection fares against the oracle, which selects the fastest
// alternative of every pair: the ratio between the total times, the geometric
// mean of the per-pair ratios, and the worst per-pair ratio (all 1 for the
// oracle).  `times[i][a]` is the time of alternative `a` on pair `i`, and
// `choice[i]` is the alternative selected for pair `i`.
inline void summarize(const std::string& name, const std::vector<std::vector<double>>& times,
                      const std::vector<std::size_t>& choice) {
  double total = 0, total_oracle = 0, log_sum = 0, worst = 1;
  for (std::size_t i = 0; i < times.size(); ++i) {
    const double best = *std::min_element(times[i].begin(), times[i].end());
    const double chosen = times[i][choice[i]];
    total += chosen;
    total_oracle += best;
    log_sum += std::log(chosen / best);
    worst = std::max(worst, chosen / best);
  }
  const double n = double(std::max<std::size_t>(times.size(), 1));
  std::cout << std::left << std::setw(28) << name << std::right << std::fixed << std::setprecision(3)
            << std::setw(14) << (total / total_oracle) << std::setw(14) << std::exp(log_sum / n)
            << std::setw(14) << worst << std::endl;
}

inline void summary_header(std::size_t num_pairs) {
  std::cout << "\n" << num_pairs << " pairs; times relative to the fastest alternative of every pair\n"
            << std::left << std::setw(28) << "selection" << std::right << std::setw(14) << "total" << std::setw(14)
            << "geomean" << std::setw(14) << "worst" << std::endl;
}

} // namespace calibration

#endif
