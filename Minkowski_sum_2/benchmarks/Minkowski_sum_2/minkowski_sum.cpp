#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/draw_polygon_with_holes_2.h>
#include <CGAL/minkowski_sum_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/random_convex_set_2.h>
#include <CGAL/random_polygon_2.h>
#include <CGAL/Random.h>

// Convex decomposition strategy headers
#include <CGAL/Small_side_angle_bisector_decomposition_2.h>
#include <CGAL/Polygon_convex_decomposition_2.h>

namespace po = boost::program_options;

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;

// The polygons are generated with an inexact-constructions kernel, which is
// much faster, and then converted (exactly) to the exact kernel.
using Gen_kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Gen_point_2 = Gen_kernel::Point_2;

template <typename InputIterator>
std::vector<Point_2> to_exact(InputIterator begin, InputIterator end) {
  std::vector<Point_2> points;
  for (auto it = begin; it != end; ++it) points.emplace_back(it->x(), it->y());
  return points;
}

// --- Helper: Generate a random convex polygon with (about) the given number of vertices ---
Polygon_2 create_convex_polygon(std::size_t num_vertices, double size, CGAL::Random& rnd) {
  std::vector<Gen_point_2> points;
  CGAL::Random_points_in_square_2<Gen_point_2> g(size, rnd);
  CGAL::random_convex_set_2(num_vertices, std::back_inserter(points), g);

  // The inexact constructions may leave a few vertices that are not strictly
  // convex; the convex hull drops them.
  auto exact_points = to_exact(points.begin(), points.end());
  std::vector<Point_2> hull;
  CGAL::convex_hull_2(exact_points.begin(), exact_points.end(), std::back_inserter(hull));
  return Polygon_2(hull.begin(), hull.end());
}

// --- Helper: Generate a random simple (typically highly concave) polygon with the given number of vertices ---
Polygon_2 create_concave_polygon(std::size_t num_vertices, double size, CGAL::Random& rnd) {
  std::vector<Gen_point_2> points;
  CGAL::Random_points_in_square_2<Gen_point_2> g(size, rnd);
  CGAL::random_polygon_2(num_vertices, std::back_inserter(points), g);

  auto exact_points = to_exact(points.begin(), points.end());
  Polygon_2 pgn(exact_points.begin(), exact_points.end());
  if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
  return pgn;
}

// Struct to store benchmark metrics for tabular display
struct Benchmark_result {
  std::string m_method_name;
  double m_avg_time_ms;
  Polygon_with_holes_2 m_pwh;
};

// --- Benchmark Runner for a Single Pair ---
template <typename MethodFunc>
Benchmark_result measure_method(const std::string& name, const Polygon_2& pgn1, const Polygon_2& pgn2,
                                int iterations, MethodFunc method) {
  // Warm-up run
  Polygon_with_holes_2 pwh = method(pgn1, pgn2);

  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iterations; ++i) {
    Polygon_with_holes_2 res = method(pgn1, pgn2);
    (void)res;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;
  return {name, elapsed.count() / iterations, std::move(pwh)};
}

bool are_polygons_equivalent(const Polygon_2& p1, const Polygon_2& p2, const Kernel& kernel) {
  const auto equal = kernel.equal_2_object();
  const auto compare_xy = kernel.compare_xy_2_object();

  std::size_t n = p1.size();
  if (n != p2.size()) return false;
  if (n == 0) return true;

  // Find lexicographically smallest vertex in p1
  std::size_t min_idx1 = 0;
  for (std::size_t i = 1; i < n; ++i) {
    if (compare_xy(p1[i], p1[min_idx1]) == CGAL::SMALLER) min_idx1 = i;
  }

  // Find lexicographically smallest vertex in p2
  std::size_t min_idx2 = 0;
  for (std::size_t i = 1; i < n; ++i) {
    if (compare_xy(p2[i], p2[min_idx2]) == CGAL::SMALLER) min_idx2 = i;
  }

  // Compare starting from min vertices with same winding order
  for (std::size_t i = 0; i < n; ++i) {
    if (! equal(p1[(min_idx1 + i) % n], p2[(min_idx2 + i) % n])) return false;
  }

  return true;
}

bool are_polygons_with_holes_equivalent(const Polygon_with_holes_2& poly1, const Polygon_with_holes_2& poly2,
                                        const Kernel& kernel) {
  // Compare outer boundaries
  if (! are_polygons_equivalent(poly1.outer_boundary(), poly2.outer_boundary(), kernel)) return false;

  // Compare number of holes
  if (poly1.number_of_holes() != poly2.number_of_holes()) return false;

  // Match each hole from poly1 with a hole in poly2
  std::vector<bool> matched(poly2.number_of_holes(), false);

  auto it1 = poly1.holes_begin();
  for (std::size_t i = 0; i < poly1.number_of_holes(); ++i, ++it1) {
    bool found = false;
    auto it2 = poly2.holes_begin();
    for (std::size_t j = 0; j < poly2.number_of_holes(); ++j, ++it2) {
      if (! matched[j] && are_polygons_equivalent(*it1, *it2, kernel)) {
        matched[j] = true;
        found = true;
        break;
      }
    }
    if (! found) return false;
  }

  return true;
}

// A Minkowski-sum method to benchmark
struct Method {
  std::string m_key;            // the name used on the command line
  std::string m_name;           // the name displayed
  std::function<Polygon_with_holes_2(const Polygon_2&, const Polygon_2&)> m_func;
};

std::vector<Method> all_methods() {
  return {
    {"full", "Full Convolution",
     [](const Polygon_2& p1, const Polygon_2& p2) { return CGAL::minkowski_sum_by_full_convolution_2(p1, p2); }},
    {"new-full", "New Full Convolution",
     [](const Polygon_2& p1, const Polygon_2& p2)
     { return CGAL::Minkowski_sum_2::new_minkowski_sum_by_full_convolution_2(p1, p2); }},
    {"reduced", "Reduced Convolution",
     [](const Polygon_2& p1, const Polygon_2& p2) { return CGAL::minkowski_sum_by_reduced_convolution_2(p1, p2); }},
    {"ssab", "Decomp: Small Side Angle Bisector",
     [](const Polygon_2& p1, const Polygon_2& p2)
     { return CGAL::minkowski_sum_2(p1, p2, CGAL::Small_side_angle_bisector_decomposition_2<Kernel>()); }},
    {"optimal", "Decomp: Optimal Convex",
     [](const Polygon_2& p1, const Polygon_2& p2)
     { return CGAL::minkowski_sum_2(p1, p2, CGAL::Optimal_convex_decomposition_2<Kernel>()); }},
    {"hertel-mehlhorn", "Decomp: Hertel-Mehlhorn",
     [](const Polygon_2& p1, const Polygon_2& p2)
     { return CGAL::minkowski_sum_2(p1, p2, CGAL::Hertel_Mehlhorn_convex_decomposition_2<Kernel>()); }},
    {"greene", "Decomp: Greene Convex",
     [](const Polygon_2& p1, const Polygon_2& p2)
     { return CGAL::minkowski_sum_2(p1, p2, CGAL::Greene_convex_decomposition_2<Kernel>()); }}
  };
}

// --- Run the given methods for a given test case and print as a matrix table ---
// The result of the first method is the reference; returns false if any result differs from it.
bool run_test_case(const std::string& case_name, const Polygon_2& pgn1, const Polygon_2& pgn2,
                   const std::vector<Method>& methods, int iterations, bool draw, const Kernel& kernel) {
  std::cout << "================================================================================\n";
  std::cout << "Test Case: " << case_name << " (Pgn1 Vertices: " << pgn1.size() << ", Pgn2 Vertices: " << pgn2.size() << ")\n";
  std::cout << "================================================================================\n";
  std::cout << std::left << std::setw(38) << "Method"
            << std::right << std::setw(12) << "Outer Verts"
            << std::setw(10) << "Holes"
            << std::setw(16) << "Avg Time (ms)" << "\n";
  std::cout << "--------------------------------------------------------------------------------\n";

  bool ok = true;
  std::vector<Benchmark_result> results;
  for (const auto& method : methods) {
    results.push_back(measure_method(method.m_name, pgn1, pgn2, iterations, method.m_func));
    const auto& r = results.back();
    std::cout << std::left << std::setw(38) << r.m_method_name
              << std::right << std::setw(12) << r.m_pwh.outer_boundary().size()
              << std::setw(10) << r.m_pwh.number_of_holes()
              << std::setw(16) << std::fixed << std::setprecision(4) << r.m_avg_time_ms << std::endl;
    const auto& ref_pwh = results.front().m_pwh;
    if (! are_polygons_with_holes_equivalent(r.m_pwh, ref_pwh, kernel)) {
      std::cerr << "Error: The resulting polygon with holes of method " << r.m_method_name
                << " differs from that of " << results.front().m_method_name << "!\n";
      if (draw) {
        CGAL::draw(r.m_pwh);
        CGAL::draw(ref_pwh);
      }
      ok = false;
    }
  }
  std::cout << "\n\n";
  return ok;
}

// Parses a vertex-count range given as "N" or "MIN MAX".
std::pair<std::size_t, std::size_t> vertex_range(const po::variables_map& vm, const std::string& name) {
  const auto& v = vm[name].as<std::vector<std::size_t>>();
  if (v.empty() || (v.size() > 2)) throw po::error("--" + name + " expects one or two values");
  std::size_t lo = v.front();
  std::size_t hi = v.back();
  if (lo < 3) throw po::error("--" + name + ": a polygon has at least 3 vertices");
  if (lo > hi) throw po::error("--" + name + ": MIN is larger than MAX");
  return {lo, hi};
}

int main(int argc, char* argv[]) {
  Kernel kernel;

  std::string method_keys;
  for (const auto& m : all_methods()) method_keys += (method_keys.empty() ? "" : ", ") + m.m_key;

  po::options_description desc("Usage: minkowski_sum [options] [seed [iterations]]\n"
                               "Computes the Minkowski sum of every pair of distinct generated polygons "
                               "with every selected method.\nOptions");
  desc.add_options()
    ("help,h", "print this help message")
    ("seed,s", po::value<unsigned int>()->default_value(1337), "the random seed")
    ("iterations,i", po::value<int>()->default_value(10), "the number of timed runs per method (after a warm-up run)")
    ("convex", po::value<std::size_t>()->default_value(2), "the number of convex polygons")
    ("concave", po::value<std::size_t>()->default_value(2), "the number of concave polygons")
    ("convex-vertices", po::value<std::vector<std::size_t>>()->multitoken()->
     default_value(std::vector<std::size_t>{10, 60}, "10 60"),
     "the number of vertices of a convex polygon, N or a range MIN MAX to draw from uniformly; "
     "the polygon may end up with a few vertices less")
    ("concave-vertices", po::value<std::vector<std::size_t>>()->multitoken()->
     default_value(std::vector<std::size_t>{15, 30}, "15 30"),
     "the number of vertices of a concave polygon, N or a range MIN MAX to draw from uniformly")
    ("size", po::value<double>()->default_value(10.0),
     "the half-side of the axis-parallel square centered at the origin that contains the polygons")
    ("methods,m", po::value<std::vector<std::string>>()->multitoken(),
     ("the methods to run, the first being the reference; any of " + method_keys + " (default: all)").c_str())
    ("pairs,p", po::value<std::vector<std::string>>()->multitoken(),
     "the kinds of pairs to run; any of convex-convex, convex-concave, concave-concave (default: all); "
     "the polygons generated do not depend on this option")
    ("draw", "draw the results that differ from the reference result");

  po::positional_options_description pos;
  pos.add("seed", 1).add("iterations", 1);

  po::variables_map vm;
  std::pair<std::size_t, std::size_t> convex_range, concave_range;
  std::vector<Method> methods;
  bool run_convex_convex = true, run_convex_concave = true, run_concave_concave = true;
  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
    po::notify(vm);
    if (vm.count("help")) {
      std::cout << desc << "\n";
      return EXIT_SUCCESS;
    }
    if (vm["iterations"].as<int>() < 1) throw po::error("--iterations must be positive");
    if (vm["size"].as<double>() <= 0) throw po::error("--size must be positive");
    convex_range = vertex_range(vm, "convex-vertices");
    concave_range = vertex_range(vm, "concave-vertices");

    auto available = all_methods();
    if (vm.count("methods")) {
      for (const auto& key : vm["methods"].as<std::vector<std::string>>()) {
        auto it = std::find_if(available.begin(), available.end(), [&](const Method& m) { return m.m_key == key; });
        if (it == available.end()) throw po::error("unknown method '" + key + "'; use one of " + method_keys);
        methods.push_back(*it);
      }
    }
    else methods = available;

    if (vm.count("pairs")) {
      run_convex_convex = run_convex_concave = run_concave_concave = false;
      for (const auto& kind : vm["pairs"].as<std::vector<std::string>>()) {
        if (kind == "convex-convex") run_convex_convex = true;
        else if (kind == "convex-concave") run_convex_concave = true;
        else if (kind == "concave-concave") run_concave_concave = true;
        else throw po::error("unknown pair kind '" + kind + "'; use one of convex-convex, convex-concave, concave-concave");
      }
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\nRun with --help for the list of options.\n";
    return EXIT_FAILURE;
  }

  const unsigned int seed = vm["seed"].as<unsigned int>();
  const int iterations = vm["iterations"].as<int>();
  const double size = vm["size"].as<double>();
  const bool draw = vm.count("draw") > 0;

  std::cout << "Starting CGAL Minkowski Sum Matrix Benchmark...\n"
            << "  Random Seed: " << seed << "\n"
            << "  Iterations : " << iterations << "\n"
            << "  Convex     : " << vm["convex"].as<std::size_t>() << " polygons with "
            << convex_range.first << "-" << convex_range.second << " vertices\n"
            << "  Concave    : " << vm["concave"].as<std::size_t>() << " polygons with "
            << concave_range.first << "-" << concave_range.second << " vertices\n\n";

  // Generate the polygons.
  CGAL::Random rnd(seed);
  auto draw_count = [&](const std::pair<std::size_t, std::size_t>& range)
  { return static_cast<std::size_t>(rnd.get_int(int(range.first), int(range.second) + 1)); };

  std::vector<std::pair<std::string, Polygon_2>> pgns;
  for (std::size_t i = 0; i < vm["convex"].as<std::size_t>(); ++i)
    pgns.emplace_back("Convex " + std::to_string(i + 1), create_convex_polygon(draw_count(convex_range), size, rnd));
  for (std::size_t i = 0; i < vm["concave"].as<std::size_t>(); ++i)
    pgns.emplace_back("Concave " + std::to_string(i + 1), create_concave_polygon(draw_count(concave_range), size, rnd));

  // Run every pair of distinct polygons of the selected kinds; the convex
  // polygons precede the concave ones.
  const std::size_t num_convex = vm["convex"].as<std::size_t>();
  std::size_t num_failures = 0;
  for (std::size_t i = 0; i < pgns.size(); ++i) {
    for (std::size_t j = i + 1; j < pgns.size(); ++j) {
      const bool convex1 = i < num_convex;
      const bool convex2 = j < num_convex;
      if (convex1 && convex2 && ! run_convex_convex) continue;
      if (convex1 && ! convex2 && ! run_convex_concave) continue;
      if (! convex1 && ! convex2 && ! run_concave_concave) continue;
      if (! run_test_case(pgns[i].first + " vs " + pgns[j].first, pgns[i].second, pgns[j].second,
                          methods, iterations, draw, kernel))
        ++num_failures;
    }
  }

  if (num_failures > 0) {
    std::cerr << num_failures << " test case(s) produced inconsistent results.\n";
    return EXIT_FAILURE;
  }
  std::cout << "All benchmarks completed successfully.\n";
  return EXIT_SUCCESS;
}
