#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <random>
#include <iomanip>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/draw_polygon_2.h>
#include <CGAL/draw_polygon_with_holes_2.h>
#include <CGAL/generators.h>
#include <CGAL/minkowski_sum_2.h>
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

// Kernel and Polygon definitions
using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;

// --- Helper: Generate a Convex Polygon using CGAL Generators & Convex Hull ---
Polygon_2 create_convex_polygon(int num_vertices, unsigned int seed) {
  std::vector<Point_2> points;
  points.reserve(num_vertices * 2);

  CGAL::Random cgal_rnd(seed);
  CGAL::Random_points_in_square_2<Point_2> g(10.0, cgal_rnd);
  CGAL::copy_n(g, num_vertices * 2, std::back_inserter(points));

  std::vector<Point_2> hull;
  CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(hull));

  Polygon_2 pgn;
  for (const auto& pt : hull) pgn.push_back(pt);
  if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
  return pgn;
}

// --- Helper: Generate a Highly Concave Simple Polygon using CGAL Generators ---
Polygon_2 create_concave_polygon(int num_vertices, unsigned int seed) {
  Polygon_2 pgn;
  CGAL::Random cgal_rnd(seed);
  CGAL::Random_points_in_square_2<Point_2> g(10.0, cgal_rnd);

  CGAL::random_polygon_2(num_vertices, std::back_inserter(pgn), g);
  if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
  return pgn;
}

// Struct to store benchmark metrics for tabular display
struct BenchmarkResult {
  std::string method_name;
  size_t outer_vertices;
  size_t holes;
  double avg_time_ms;
};

// --- Benchmark Runner for a Single Pair ---
template <typename MethodFunc>
BenchmarkResult measure_method(const std::string& name, const Polygon_2& pgn1, const Polygon_2& pgn2, int iterations, MethodFunc method) {
  // Warm-up run
  Polygon_with_holes_2 result = method(pgn1, pgn2);

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; ++i) {
    Polygon_with_holes_2 res = method(pgn1, pgn2);
    (void)res;
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;

  return {name, result.outer_boundary().size(), result.number_of_holes(), elapsed.count() / iterations};
}

// --- Run all methods for a given test case and print as a matrix table ---
void run_test_case(const std::string& case_name, const Polygon_2& pgn1, const Polygon_2& pgn2, int iterations) {
  std::cout << "========================================================================================\n";
  std::cout << "Test Case: " << case_name << " (Pgn1 Vertices: " << pgn1.size() << ", Pgn2 Vertices: " << pgn2.size() << ")\n";
  std::cout << "========================================================================================\n";
  std::cout << std::left << std::setw(38) << "Method"
            << std::right << std::setw(12) << "Outer Verts"
            << std::setw(10) << "Holes"
            << std::setw(16) << "Avg Time (ms)" << "\n";
  std::cout << "----------------------------------------------------------------------------------------\n";

  std::vector<BenchmarkResult> results;

  // 1. Full Convolution
  results.push_back(measure_method("Full Convolution", pgn1, pgn2, iterations, [](const Polygon_2& p1, const Polygon_2& p2) {
    return CGAL::minkowski_sum_by_full_convolution_2(p1, p2);
  }));

  // 2. Reduced Convolution
  results.push_back(measure_method("Reduced Convolution", pgn1, pgn2, iterations, [](const Polygon_2& p1, const Polygon_2& p2) {
    return CGAL::minkowski_sum_by_reduced_convolution_2(p1, p2);
  }));

  // 3. Small Side Angle Bisector Decomposition
  CGAL::Small_side_angle_bisector_decomposition_2<Kernel> ssab_decomp;
  results.push_back(measure_method("Decomp: Small Side Angle Bisector", pgn1, pgn2, iterations, [&ssab_decomp](const Polygon_2& p1, const Polygon_2& p2) {
    return CGAL::minkowski_sum_2(p1, p2, ssab_decomp);
  }));

  // 4. Optimal Convex Decomposition
  CGAL::Optimal_convex_decomposition_2<Kernel> opt_decomp;
  results.push_back(measure_method("Decomp: Optimal Convex", pgn1, pgn2, iterations, [&opt_decomp](const Polygon_2& p1, const Polygon_2& p2) {
    return CGAL::minkowski_sum_2(p1, p2, opt_decomp);
  }));

  // 5. Hertel-Mehlhorn Decomposition
  CGAL::Hertel_Mehlhorn_convex_decomposition_2<Kernel> hm_decomp;
  results.push_back(measure_method("Decomp: Hertel-Mehlhorn", pgn1, pgn2, iterations, [&hm_decomp](const Polygon_2& p1, const Polygon_2& p2) {
    return CGAL::minkowski_sum_2(p1, p2, hm_decomp);
  }));

  // 6. Greene Decomposition
  CGAL::Greene_convex_decomposition_2<Kernel> greene_decomp;
  results.push_back(measure_method("Decomp: Greene Convex", pgn1, pgn2, iterations, [&greene_decomp](const Polygon_2& p1, const Polygon_2& p2) {
    return CGAL::minkowski_sum_2(p1, p2, greene_decomp);
  }));

  // Print matrix rows
  for (const auto& r : results) {
    std::cout << std::left << std::setw(38) << r.method_name
              << std::right << std::setw(12) << r.outer_vertices
              << std::setw(10) << r.holes
              << std::setw(16) << std::fixed << std::setprecision(4) << r.avg_time_ms << "\n";
  }
  std::cout << "\n\n";
}

int main(int argc, char* argv[]) {
  unsigned int seed = 1337;
  int iterations = 10;

  if (argc > 1) seed = static_cast<unsigned int>(std::stoul(argv[1]));
  if (argc > 2) iterations = std::stoi(argv[2]);

  std::cout << "Starting CGAL Minkowski Sum Matrix Benchmark...\n"
            << "  Random Seed: " << seed << "\n"
            << "  Iterations : " << iterations << "\n\n";

  CGAL::Random rnd(seed);

  // --- Test Case 1: Convex vs Convex ---
  int v_conv1 = rnd.get_int(10, 30);
  int v_conv2 = rnd.get_int(10, 30);
  Polygon_2 conv1 = create_convex_polygon(v_conv1, seed);
  Polygon_2 conv2 = create_convex_polygon(v_conv2, seed + 42);
  run_test_case("Convex vs Convex", conv1, conv2, iterations);

  // --- Test Case 2: Convex vs Concave ---
  int v_conc1 = rnd.get_int(15, 30);
  Polygon_2 concave1 = create_concave_polygon(v_conc1, seed + 100);
  run_test_case("Convex vs Concave", conv1, concave1, iterations);

  // --- Test Case 3: Concave vs Concave ---
  int v_conc2 = rnd.get_int(15, 30);
  Polygon_2 concave2 = create_concave_polygon(v_conc2, seed + 200);
  run_test_case("Concave vs Concave", concave1, concave2, iterations);

  std::cout << "All benchmarks completed successfully.\n";
  return 0;
}
