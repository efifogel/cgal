#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/draw_polygon_2.h>
#include <CGAL/draw_polygon_with_holes_2.h>
#include <CGAL/generators.h>
#include <CGAL/minkowski_sum_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/random_polygon_2.h>
#include <CGAL/Random.h>

// Convex decomposition strategy headers
#include <CGAL/Small_side_angle_bisector_decomposition_2.h>
#include <CGAL/Polygon_convex_decomposition_2.h>
// #include <CGAL/Optimal_convex_decomposition_2.h>
// #include <CGAL/Hertel_Mehlhorn_convex_decomposition_2.h>
// #include <CGAL/Greene_convex_decomposition_2.h>

// ============================================================================
// FACE DATA FOR WINDING NUMBERS
// ============================================================================

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;

// --- Helper: Generate a Convex Polygon using CGAL Generators & Convex Hull ---
Polygon_2 create_convex_polygon(std::size_t num_vertices, unsigned int seed) {
  std::vector<Point_2> points;
  points.reserve(num_vertices);

  CGAL::Random cgal_rnd(seed);
  CGAL::Random_points_in_square_2<Point_2> g(10.0, cgal_rnd);
  CGAL::copy_n(g, num_vertices, std::back_inserter(points));

  std::vector<Point_2> hull;
  CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(hull));

  Polygon_2 pgn;
  for (const auto& pt : hull) pgn.push_back(pt);
  if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
  return pgn;
}

// --- Helper: Generate a Highly Concave Simple Polygon using CGAL Generators ---
Polygon_2 create_concave_polygon(std::size_t num_vertices, unsigned int seed) {
  Polygon_2 pgn;
  CGAL::Random cgal_rnd(seed);
  CGAL::Random_points_in_square_2<Point_2> g(10.0, cgal_rnd);

  CGAL::random_polygon_2(num_vertices, std::back_inserter(pgn), g);
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

// --- Run all methods for a given test case and print as a matrix table ---
void run_test_case(const std::string& case_name, const Polygon_2& pgn1, const Polygon_2& pgn2, int iterations,
                   const Kernel& kernel) {
  std::cout << "================================================================================\n";
  std::cout << "Test Case: " << case_name << " (Pgn1 Vertices: " << pgn1.size() << ", Pgn2 Vertices: " << pgn2.size() << ")\n";
  std::cout << "================================================================================\n";
  std::cout << std::left << std::setw(38) << "Method"
            << std::right << std::setw(12) << "Outer Verts"
            << std::setw(10) << "Holes"
            << std::setw(16) << "Avg Time (ms)" << "\n";
  std::cout << "--------------------------------------------------------------------------------\n";

  std::vector<Benchmark_result> results;

  // 1. Full Convolution
  results.push_back(measure_method("Full Convolution", pgn1, pgn2, iterations,
                                   [](const Polygon_2& p1, const Polygon_2& p2)
                                   { return CGAL::minkowski_sum_by_full_convolution_2(p1, p2); }));

  // 1. Full Convolution
  results.push_back(measure_method("New Full Convolution", pgn1, pgn2, iterations,
                                   [](const Polygon_2& p1, const Polygon_2& p2)
                                   { return CGAL::Minkowski_sum_2::new_minkowski_sum_by_full_convolution_2(p1, p2); }));

  // 2. Reduced Convolution
  results.push_back(measure_method("Reduced Convolution", pgn1, pgn2, iterations,
                                   [](const Polygon_2& p1, const Polygon_2& p2)
                                   { return CGAL::minkowski_sum_by_reduced_convolution_2(p1, p2); }));

  // 3. Small Side Angle Bisector Decomposition
  CGAL::Small_side_angle_bisector_decomposition_2<Kernel> ssab_decomp;
  results.push_back(measure_method("Decomp: Small Side Angle Bisector", pgn1, pgn2, iterations,
                                   [&ssab_decomp](const Polygon_2& p1, const Polygon_2& p2)
                                   { return CGAL::minkowski_sum_2(p1, p2, ssab_decomp); }));

  // 4. Optimal Convex Decomposition
  CGAL::Optimal_convex_decomposition_2<Kernel> opt_decomp;
  results.push_back(measure_method("Decomp: Optimal Convex", pgn1, pgn2, iterations,
                                   [&opt_decomp](const Polygon_2& p1, const Polygon_2& p2)
                                   { return CGAL::minkowski_sum_2(p1, p2, opt_decomp); }));

  // 5. Hertel-Mehlhorn Decomposition
  CGAL::Hertel_Mehlhorn_convex_decomposition_2<Kernel> hm_decomp;
  results.push_back(measure_method("Decomp: Hertel-Mehlhorn", pgn1, pgn2, iterations,
                                   [&hm_decomp](const Polygon_2& p1, const Polygon_2& p2)
                                   { return CGAL::minkowski_sum_2(p1, p2, hm_decomp); }));

  // 6. Greene Decomposition
  CGAL::Greene_convex_decomposition_2<Kernel> greene_decomp;
  results.push_back(measure_method("Decomp: Greene Convex", pgn1, pgn2, iterations,
                                   [&greene_decomp](const Polygon_2& p1, const Polygon_2& p2)
                                   { return CGAL::minkowski_sum_2(p1, p2, greene_decomp); }));

  // Print matrix rows
  const auto& ref_pwh = results[0].m_pwh;
  for (const auto& r : results) {
    std::cout << std::left << std::setw(38) << r.m_method_name
              << std::right << std::setw(12) << r.m_pwh.outer_boundary().size()
              << std::setw(10) << r.m_pwh.number_of_holes()
              << std::setw(16) << std::fixed << std::setprecision(4) << r.m_avg_time_ms << "\n";
    if (! are_polygons_with_holes_equivalent(r.m_pwh, ref_pwh, kernel)) {
      CGAL::draw(r.m_pwh);
      CGAL::draw(ref_pwh);
      std::cerr << "Error: The resulting polygon with holes of method " << r.m_method_name << " differs!\n";
      break;
    }
  }
  std::cout << "\n\n";
}

int main(int argc, char* argv[]) {
  Kernel kernel;;

  unsigned int seed = 1337;
  int iterations = 10;

  if (argc > 1) seed = static_cast<unsigned int>(std::stoul(argv[1]));
  if (argc > 2) iterations = std::stoi(argv[2]);

  std::cout << "Starting CGAL Minkowski Sum Matrix Benchmark...\n"
            << "  Random Seed: " << seed << "\n"
            << "  Iterations : " << iterations << "\n\n";

  CGAL::Random rnd(seed);

  // --- Test Case 1: Convex vs Convex ---
  std::size_t v_conv1 = rnd.get_int(10, 60);
  Polygon_2 conv1 = create_convex_polygon(v_conv1, seed);
  // CGAL::draw(conv1);
  std::size_t v_conv2 = rnd.get_int(10, 60);
  Polygon_2 conv2 = create_convex_polygon(v_conv2, seed + 42);
  // CGAL::draw(conv2);
  run_test_case("Convex vs Convex", conv1, conv2, iterations, kernel);

  // --- Test Case 2: Convex vs Concave ---
  std::size_t v_conc1 = rnd.get_int(15, 30);
  Polygon_2 conc1 = create_concave_polygon(v_conc1, seed + 100);
  // CGAL::draw(conc1);
  run_test_case("Convex vs Concave", conv1, conc1, iterations, kernel);

  // --- Test Case 3: Concave vs Concave ---
  int v_conc2 = rnd.get_int(15, 30);
  Polygon_2 conc2 = create_concave_polygon(v_conc2, seed + 200);
  run_test_case("Concave vs Concave", conc1, conc2, iterations, kernel);

  std::cout << "All benchmarks completed successfully.\n";
  return 0;
}
