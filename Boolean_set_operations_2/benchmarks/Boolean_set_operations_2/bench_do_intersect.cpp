#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <string>
#include <sstream>

// Exact Predicates Exact Constructions (EPEC)
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Boolean_set_operations_2.h>

// Circular / General Polygon Support
#include <CGAL/Exact_circular_kernel_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/General_polygon_2.h>
#include <CGAL/General_polygon_with_holes_2.h>

#include <CGAL/draw_polygon_2.h>
#include <CGAL/draw_polygon_with_holes_2.h>

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------

using Exact_kernel   = CGAL::Exact_predicates_exact_constructions_kernel;
using Inexact_Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Kernel         = Exact_kernel;
using Point_2        = Kernel::Point_2;
using Polygon_2      = CGAL::Polygon_2<Kernel>;
using Polygon_wh_2   = CGAL::Polygon_with_holes_2<Kernel>;

using Circ_Kernel          = CGAL::Exact_circular_kernel_2;
using Gps_Traits           = CGAL::Gps_circle_segment_traits_2<Circ_Kernel>;
using General_Polygon_2    = Gps_Traits::Polygon_2;
using General_Polygon_wh_2 = Gps_Traits::Polygon_with_holes_2;
using X_monotone_curve_2   = Gps_Traits::X_monotone_curve_2;
using Traits_Point_2       = Gps_Traits::Point_2;
using CoordNT              = Traits_Point_2::CoordNT;
using Kernel_Point_2       = Circ_Kernel::Point_2;
using Circle_2             = Circ_Kernel::Circle_2;

// -----------------------------------------------------------------------------
// Benchmark Statistics and Chrono Harness
// -----------------------------------------------------------------------------

struct BenchmarkStats {
  double mean_ms;
  double min_ms;
  double max_ms;
  double stddev_ms;
};

template <typename Func>
BenchmarkStats measure_execution(Func&& func, int repetitions) {
  std::vector<double> durations;
  durations.reserve(repetitions);

  for (int r = 0; r < repetitions; ++r) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    durations.push_back(elapsed.count());
  }

  double sum = std::accumulate(durations.begin(), durations.end(), 0.0);
  double mean = sum / repetitions;

  double min_v = durations[0];
  double max_v = durations[0];
  double var_accum = 0.0;

  for (double d : durations) {
    if (d < min_v) min_v = d;
    if (d > max_v) max_v = d;
    var_accum += (d - mean) * (d - mean);
  }
  double stddev = std::sqrt(var_accum / repetitions);

  return {mean, min_v, max_v, stddev};
}

void print_result(const std::string& label, const BenchmarkStats& stats) {
  std::cout << std::left << std::setw(60) << label
            << " | Mean: " << std::setw(8) << std::fixed << std::setprecision(3) << stats.mean_ms << " ms"
            << " | Min: "  << std::setw(8) << stats.min_ms << " ms"
            << " | Max: "  << std::setw(8) << stats.max_ms << " ms"
            << " | (std: " << std::setprecision(4) << stats.stddev_ms << ")\n";
}

// -----------------------------------------------------------------------------
// Geometric Generators
// -----------------------------------------------------------------------------

class TopologicalPairFactory {
private:
  std::mt19937 rng_;
  std::uniform_real_distribution<double> jitter_dist_;

public:
  explicit TopologicalPairFactory(unsigned int seed)
      : rng_(seed), jitter_dist_(-1.5, 1.5) {}

  void set_seed(unsigned int seed) {
    rng_.seed(seed);
  }

  // --- Linear Helpers ---
  Polygon_2 make_regular_polygon(double cx, double cy, double r, int n = 16) {
    Polygon_2 pgn;
    for (int i = 0; i < n; ++i) {
      double a = 2.0 * M_PI * i / n;
      pgn.push_back(Point_2(cx + r * std::cos(a), cy + r * std::sin(a)));
    }
    if (!pgn.is_counterclockwise_oriented()) {
      pgn.reverse_orientation();
    }
    return pgn;
  }

  Polygon_wh_2 make_polygon_with_hole(double cx, double cy, double r_out, double r_in, int n = 16) {
    Polygon_2 outer = make_regular_polygon(cx, cy, r_out, n);
    Polygon_2 hole  = make_regular_polygon(cx, cy, r_in, n);
    if (!hole.is_clockwise_oriented()) {
      hole.reverse_orientation();
    }
    std::vector<Polygon_2> holes = { hole };
    return Polygon_wh_2(outer, holes.begin(), holes.end());
  }

  // --- Circular Helpers ---
  General_Polygon_2 make_circle_polygon(double cx, double cy, double r, CGAL::Orientation orient) {
    Kernel_Point_2 center(cx, cy);
    Circle_2 circle(center, r * r, orient);

    Traits_Point_2 left(CoordNT(cx - r), CoordNT(cy));
    Traits_Point_2 right(CoordNT(cx + r), CoordNT(cy));

    std::vector<X_monotone_curve_2> arcs;
    if (orient == CGAL::COUNTERCLOCKWISE) {
      arcs.emplace_back(circle, left, right, CGAL::COUNTERCLOCKWISE);
      arcs.emplace_back(circle, right, left, CGAL::COUNTERCLOCKWISE);
    } else {
      arcs.emplace_back(circle, left, right, CGAL::CLOCKWISE);
      arcs.emplace_back(circle, right, left, CGAL::CLOCKWISE);
    }
    return General_Polygon_2(arcs.begin(), arcs.end());
  }

  General_Polygon_wh_2 make_general_polygon_with_hole(double cx, double cy, double r_out, double r_in) {
    General_Polygon_2 outer = make_circle_polygon(cx, cy, r_out, CGAL::COUNTERCLOCKWISE);
    General_Polygon_2 hole  = make_circle_polygon(cx, cy, r_in,  CGAL::CLOCKWISE);
    General_Polygon_wh_2 gpwh(outer);
    gpwh.add_hole(hole);
    return gpwh;
  }

  // ---------------------------------------------------------------------------
  // Pair 1: Polygon - Polygon (5 distinct topological states)
  // ---------------------------------------------------------------------------
  std::vector<std::pair<Polygon_2, Polygon_2>> generate_all_p_p_states(double base_cx, double base_cy) {
    std::vector<std::pair<Polygon_2, Polygon_2>> pairs;
    double r1 = 30.0 + jitter_dist_(rng_);
    double r2 = 10.0 + jitter_dist_(rng_);

    Polygon_2 p1 = make_regular_polygon(base_cx, base_cy, r1);

    // 1. Disjoint
    pairs.push_back({ p1, make_regular_polygon(base_cx + r1 + r2 + 15.0, base_cy, r2) });
    // 2. Boundary Crossing
    pairs.push_back({ p1, make_regular_polygon(base_cx + r1 * 0.7, base_cy, r2) });
    // 3. Strict Containment (p2 inside p1)
    pairs.push_back({ p1, make_regular_polygon(base_cx, base_cy, r1 * 0.4) });
    // 4. Touching from Outside (Outer Tangency at (base_cx + r1, base_cy))
    pairs.push_back({ p1, make_regular_polygon(base_cx + r1 + r2, base_cy, r2) });
    // 5. Touching from Inside (Inner Tangency sharing point at (base_cx + r1, base_cy))
    pairs.push_back({ p1, make_regular_polygon(base_cx + r1 - r2, base_cy, r2) });

    return pairs;
  }

  // ---------------------------------------------------------------------------
  // Pair 2: Polygon - Polygon with Holes (6 distinct topological states)
  // ---------------------------------------------------------------------------
  std::vector<std::pair<Polygon_2, Polygon_wh_2>> generate_all_p_pwh_states(double base_cx, double base_cy) {
    std::vector<std::pair<Polygon_2, Polygon_wh_2>> pairs;
    double r_out = 50.0 + jitter_dist_(rng_);
    double r_in  = 25.0 + jitter_dist_(rng_);
    double r_sub = 8.0;

    Polygon_wh_2 pwh = make_polygon_with_hole(base_cx, base_cy, r_out, r_in);

    // 1. Disjoint
    pairs.push_back({ make_regular_polygon(base_cx + r_out + 20.0, base_cy, r_sub), pwh });
    // 2. Boundary Crossing (Crossing outer boundary)
    pairs.push_back({ make_regular_polygon(base_cx + r_out, base_cy, r_sub), pwh });
    // 3. Strictly Inside Solid Interior Region (Between hole and outer boundary)
    double solid_offset = (r_out + r_in) / 2.0;
    pairs.push_back({ make_regular_polygon(base_cx + solid_offset, base_cy, (r_out - r_in) * 0.35), pwh });
    // 4. Strictly Inside the Hole (Does NOT intersect)
    pairs.push_back({ make_regular_polygon(base_cx, base_cy, r_in * 0.5), pwh });
    // 5. Touching Outer Boundary from Outside
    pairs.push_back({ make_regular_polygon(base_cx + r_out + r_sub, base_cy, r_sub), pwh });
    // 6. Touching Inner Hole Boundary from Inside Hole (Does NOT intersect)
    pairs.push_back({ make_regular_polygon(base_cx + r_in - r_sub, base_cy, r_sub), pwh });

    return pairs;
  }

  // ---------------------------------------------------------------------------
  // Pair 3: Polygon with Holes - Polygon with Holes (6 distinct topological states)
  // ---------------------------------------------------------------------------
  std::vector<std::pair<Polygon_wh_2, Polygon_wh_2>> generate_all_pwh_pwh_states(double base_cx, double base_cy) {
    std::vector<std::pair<Polygon_wh_2, Polygon_wh_2>> pairs;
    double r_out = 60.0 + jitter_dist_(rng_);
    double r_in  = 30.0 + jitter_dist_(rng_);

    Polygon_wh_2 pwh1 = make_polygon_with_hole(base_cx, base_cy, r_out, r_in);

    // 1. Disjoint
    pairs.push_back({ pwh1, make_polygon_with_hole(base_cx + r_out + 30.0, base_cy, 12.0, 5.0) });
    // 2. Boundary Crossing
    pairs.push_back({ pwh1, make_polygon_with_hole(base_cx + r_out * 0.8, base_cy, 15.0, 6.0) });
    // 3. Strictly Inside Solid Interior Region
    double solid_offset = (r_out + r_in) / 2.0;
    double sub_out = (r_out - r_in) * 0.35;
    pairs.push_back({ pwh1, make_polygon_with_hole(base_cx + solid_offset, base_cy, sub_out, sub_out * 0.4) });
    // 4. Strictly Inside Hole (Does NOT intersect)
    pairs.push_back({ pwh1, make_polygon_with_hole(base_cx, base_cy, r_in * 0.6, r_in * 0.25) });
    // 5. Touching Outer Boundary from Outside
    pairs.push_back({ pwh1, make_polygon_with_hole(base_cx + r_out + 10.0, base_cy, 10.0, 4.0) });
    // 6. Touching Hole Boundary from Inside Hole
    pairs.push_back({ pwh1, make_polygon_with_hole(base_cx + r_in - 8.0, base_cy, 8.0, 3.0) });

    return pairs;
  }

  // ---------------------------------------------------------------------------
  // Pair 7: General Polygon - General Polygon (5 distinct topological states)
  // ---------------------------------------------------------------------------
  std::vector<std::pair<General_Polygon_2, General_Polygon_2>> generate_all_gp_gp_states(double base_cx, double base_cy) {
    std::vector<std::pair<General_Polygon_2, General_Polygon_2>> pairs;
    double r1 = 30.0 + jitter_dist_(rng_);
    double r2 = 10.0 + jitter_dist_(rng_);

    General_Polygon_2 gp1 = make_circle_polygon(base_cx, base_cy, r1, CGAL::COUNTERCLOCKWISE);

    // 1. Disjoint
    pairs.push_back({ gp1, make_circle_polygon(base_cx + r1 + r2 + 15.0, base_cy, r2, CGAL::COUNTERCLOCKWISE) });
    // 2. Boundary Crossing
    pairs.push_back({ gp1, make_circle_polygon(base_cx + r1 * 0.7, base_cy, r2, CGAL::COUNTERCLOCKWISE) });
    // 3. Strict Containment
    pairs.push_back({ gp1, make_circle_polygon(base_cx, base_cy, r1 * 0.4, CGAL::COUNTERCLOCKWISE) });
    // 4. Touching from Outside
    pairs.push_back({ gp1, make_circle_polygon(base_cx + r1 + r2, base_cy, r2, CGAL::COUNTERCLOCKWISE) });
    // 5. Touching from Inside
    pairs.push_back({ gp1, make_circle_polygon(base_cx + r1 - r2, base_cy, r2, CGAL::COUNTERCLOCKWISE) });

    return pairs;
  }

  // ---------------------------------------------------------------------------
  // Pair 8: General Polygon - GP with Holes (6 distinct topological states)
  // ---------------------------------------------------------------------------
  std::vector<std::pair<General_Polygon_2, General_Polygon_wh_2>> generate_all_gp_gpwh_states(double base_cx, double base_cy) {
    std::vector<std::pair<General_Polygon_2, General_Polygon_wh_2>> pairs;
    double r_out = 50.0 + jitter_dist_(rng_);
    double r_in  = 25.0 + jitter_dist_(rng_);
    double r_sub = 8.0;

    General_Polygon_wh_2 gpwh = make_general_polygon_with_hole(base_cx, base_cy, r_out, r_in);

    // 1. Disjoint
    pairs.push_back({ make_circle_polygon(base_cx + r_out + 20.0, base_cy, r_sub, CGAL::COUNTERCLOCKWISE), gpwh });
    // 2. Boundary Crossing
    pairs.push_back({ make_circle_polygon(base_cx + r_out, base_cy, r_sub, CGAL::COUNTERCLOCKWISE), gpwh });
    // 3. Strictly Inside Solid Interior Region
    double solid_offset = (r_out + r_in) / 2.0;
    pairs.push_back({ make_circle_polygon(base_cx + solid_offset, base_cy, (r_out - r_in) * 0.35, CGAL::COUNTERCLOCKWISE), gpwh });
    // 4. Strictly Inside the Hole (Does NOT intersect)
    pairs.push_back({ make_circle_polygon(base_cx, base_cy, r_in * 0.5, CGAL::COUNTERCLOCKWISE), gpwh });
    // 5. Touching Outer Boundary from Outside
    pairs.push_back({ make_circle_polygon(base_cx + r_out + r_sub, base_cy, r_sub, CGAL::COUNTERCLOCKWISE), gpwh });
    // 6. Touching Inner Hole Boundary from Inside Hole
    pairs.push_back({ make_circle_polygon(base_cx + r_in - r_sub, base_cy, r_sub, CGAL::COUNTERCLOCKWISE), gpwh });

    return pairs;
  }

  // ---------------------------------------------------------------------------
  // Pair 9: GP with Holes - GP with Holes (6 distinct topological states)
  // ---------------------------------------------------------------------------
  std::vector<std::pair<General_Polygon_wh_2, General_Polygon_wh_2>> generate_all_gpwh_gpwh_states(double base_cx, double base_cy) {
    std::vector<std::pair<General_Polygon_wh_2, General_Polygon_wh_2>> pairs;
    double r_out = 60.0 + jitter_dist_(rng_);
    double r_in  = 30.0 + jitter_dist_(rng_);

    General_Polygon_wh_2 gpwh1 = make_general_polygon_with_hole(base_cx, base_cy, r_out, r_in);

    // 1. Disjoint
    pairs.push_back({ gpwh1, make_general_polygon_with_hole(base_cx + r_out + 30.0, base_cy, 12.0, 5.0) });
    // 2. Boundary Crossing
    pairs.push_back({ gpwh1, make_general_polygon_with_hole(base_cx + r_out * 0.8, base_cy, 15.0, 6.0) });
    // 3. Strictly Inside Solid Interior Region
    double solid_offset = (r_out + r_in) / 2.0;
    double sub_out = (r_out - r_in) * 0.35;
    pairs.push_back({ gpwh1, make_general_polygon_with_hole(base_cx + solid_offset, base_cy, sub_out, sub_out * 0.4) });
    // 4. Strictly Inside Hole (Does NOT intersect)
    pairs.push_back({ gpwh1, make_general_polygon_with_hole(base_cx, base_cy, r_in * 0.6, r_in * 0.25) });
    // 5. Touching Outer Boundary from Outside
    pairs.push_back({ gpwh1, make_general_polygon_with_hole(base_cx + r_out + 10.0, base_cy, 10.0, 4.0) });
    // 6. Touching Hole Boundary from Inside Hole
    pairs.push_back({ gpwh1, make_general_polygon_with_hole(base_cx + r_in - 8.0, base_cy, 8.0, 3.0) });

    return pairs;
  }
};

// -----------------------------------------------------------------------------
// Main Runner
// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  unsigned int seed = 42;
  int repetitions   = 50;
  size_t set_size   = 30;

  // CLI parser: ./benchmark [seed] [repetitions] [set_size]
  if (argc > 1) seed = static_cast<unsigned int>(std::stoul(argv[1]));
  if (argc > 2) repetitions = std::stoi(argv[2]);
  if (argc > 3) set_size = static_cast<size_t>(std::stoul(argv[3]));

  std::cout << "================================================================================================\n";
  std::cout << " CGAL Polygon Intersection Benchmark Suite (Full Topological Coverage)\n";
  std::cout << " Seed: " << seed << " | Repetitions: " << repetitions << " | Set Size: " << set_size << "\n";
  std::cout << "================================================================================================\n\n";

  TopologicalPairFactory factory(seed);
  Gps_Traits traits;

  const size_t NUM_CLUSTERS = 10;

  // -------------------------------------------------------------------------
  // Pre-generate Datasets Covering All Topological Relationships
  // -------------------------------------------------------------------------

  // 1. Polygon - Polygon Pairs
  std::vector<std::pair<Polygon_2, Polygon_2>> p_p_pairs;
  for (size_t i = 0; i < NUM_CLUSTERS; ++i) {
    auto batch = factory.generate_all_p_p_states(i * 120.0, 0.0);
    p_p_pairs.insert(p_p_pairs.end(), batch.begin(), batch.end());
  }

  // 2. Polygon - Polygon with Holes Pairs
  std::vector<std::pair<Polygon_2, Polygon_wh_2>> p_pwh_pairs;
  for (size_t i = 0; i < NUM_CLUSTERS; ++i) {
    auto batch = factory.generate_all_p_pwh_states(i * 180.0, 0.0);
    p_pwh_pairs.insert(p_pwh_pairs.end(), batch.begin(), batch.end());
  }

  // 3. Polygon with Holes - Polygon with Holes Pairs
  std::vector<std::pair<Polygon_wh_2, Polygon_wh_2>> pwh_pwh_pairs;
  for (size_t i = 0; i < NUM_CLUSTERS; ++i) {
    auto batch = factory.generate_all_pwh_pwh_states(i * 220.0, 0.0);
    pwh_pwh_pairs.insert(pwh_pwh_pairs.end(), batch.begin(), batch.end());
  }

  std::size_t i = 0;
  for (const auto& pair : pwh_pwh_pairs) {
    CGAL::Graphics_scene scene_pwh;
    CGAL::add_to_graphics_scene(pair.first, scene_pwh);
    CGAL::add_to_graphics_scene(pair.second, scene_pwh);
    CGAL::draw_graphics_scene(scene_pwh, std::to_string(i++).c_str());
  }

  // 4-6. Linear Sets (Mix of concentric, overlapping, and disjoint polygons)
  std::vector<Polygon_2> poly_set(set_size);
  std::vector<Polygon_wh_2> pwh_set(set_size);
  for (size_t i = 0; i < set_size; ++i) {
    poly_set[i] = factory.make_regular_polygon(
      (i % 5) * 15.0 - 30.0, ((i / 5) % 5) * 15.0 - 30.0, (i % 3 == 0) ? 22.0 : 8.0, 16);
    pwh_set[i]  = factory.make_polygon_with_hole(
      (i % 5) * 20.0 - 40.0, ((i / 5) % 5) * 20.0 - 40.0, 24.0, 10.0, 16);
  }

  // 7. General Polygon - General Polygon Pairs
  std::vector<std::pair<General_Polygon_2, General_Polygon_2>> gp_gp_pairs;
  for (size_t i = 0; i < NUM_CLUSTERS; ++i) {
    auto batch = factory.generate_all_gp_gp_states(i * 120.0, 0.0);
    gp_gp_pairs.insert(gp_gp_pairs.end(), batch.begin(), batch.end());
  }

  // 8. General Polygon - GP with Holes Pairs
  std::vector<std::pair<General_Polygon_2, General_Polygon_wh_2>> gp_gpwh_pairs;
  for (size_t i = 0; i < NUM_CLUSTERS; ++i) {
    auto batch = factory.generate_all_gp_gpwh_states(i * 180.0, 0.0);
    gp_gpwh_pairs.insert(gp_gpwh_pairs.end(), batch.begin(), batch.end());
  }

  // 9. GP with Holes - GP with Holes Pairs
  std::vector<std::pair<General_Polygon_wh_2, General_Polygon_wh_2>> gpwh_gpwh_pairs;
  for (size_t i = 0; i < NUM_CLUSTERS; ++i) {
    auto batch = factory.generate_all_gpwh_gpwh_states(i * 220.0, 0.0);
    gpwh_gpwh_pairs.insert(gpwh_gpwh_pairs.end(), batch.begin(), batch.end());
  }

  // 10-12. General Polygon Sets
  std::vector<General_Polygon_2> gp_set(set_size);
  std::vector<General_Polygon_wh_2> gpwh_set(set_size);
  for (size_t i = 0; i < set_size; ++i) {
    gp_set[i]   = factory.make_circle_polygon(
      (i % 5) * 15.0 - 30.0, ((i / 5) % 5) * 15.0 - 30.0, (i % 3 == 0) ? 22.0 : 8.0, CGAL::COUNTERCLOCKWISE);
    gpwh_set[i] = factory.make_general_polygon_with_hole(
      (i % 5) * 20.0 - 40.0, ((i / 5) % 5) * 20.0 - 40.0, 24.0, 10.0);
  }

  // -------------------------------------------------------------------------
  // Execution & Measurement (All 12 Benchmark Cases)
  // -------------------------------------------------------------------------

  // [01] Detection of intersections of two polygons (without holes)
  print_result("[01] Two Polygons (No Holes) [All Topological States]",
    measure_execution([&]() {
      bool found = false;
      for (const auto& pair : p_p_pairs) {
        if (CGAL::do_intersect(pair.first, pair.second)) found = true;
      }
      (void)found;
    }, repetitions));

  // [02] Detection of intersections of one polygon and one polygon with holes
  print_result("[02] One Polygon vs One Polygon with Holes [All States]",
    measure_execution([&]() {
      bool found = false;
      for (const auto& pair : p_pwh_pairs) {
        if (CGAL::do_intersect(pair.first, pair.second)) found = true;
      }
      (void)found;
    }, repetitions));

  // [03] Detection of intersections of two polygons with holes
  print_result("[03] Two Polygons with Holes [All States]",
    measure_execution([&]() {
      bool found = false;
      for (const auto& pair : pwh_pwh_pairs) {
        if (CGAL::do_intersect(pair.first, pair.second)) found = true;
      }
      (void)found;
    }, repetitions));

  // [04] Detection of intersections of a set of polygons (without holes)
  print_result("[04] Set of Polygons (No Holes) [All-Pairs]",
    measure_execution([&]() {
      bool found = false;
      for (size_t i = 0; i < poly_set.size(); ++i) {
        for (size_t j = i + 1; j < poly_set.size(); ++j) {
          if (CGAL::do_intersect(poly_set[i], poly_set[j])) found = true;
        }
      }
      (void)found;
    }, repetitions));

  // [05] Detection of intersections of a set of polygons and a set of polygons with holes
  print_result("[05] Set of Polygons vs Set of Polygons with Holes",
    measure_execution([&]() {
      bool found = false;
      for (const auto& poly : poly_set) {
        for (const auto& pwh : pwh_set) {
          if (CGAL::do_intersect(poly, pwh)) found = true;
        }
      }
      (void)found;
    }, repetitions));

  // [06] Detection of intersections of a set of polygons with holes
  print_result("[06] Set of Polygons with Holes [All-Pairs]",
    measure_execution([&]() {
      bool found = false;
      for (size_t i = 0; i < pwh_set.size(); ++i) {
        for (size_t j = i + 1; j < pwh_set.size(); ++j) {
          if (CGAL::do_intersect(pwh_set[i], pwh_set[j])) found = true;
        }
      }
      (void)found;
    }, repetitions));

  // [07] Detection of intersections of two general polygons (without holes)
  print_result("[07] Two General Polygons (No Holes) [All States]",
    measure_execution([&]() {
      bool found = false;
      for (const auto& pair : gp_gp_pairs) {
        if (CGAL::do_intersect(pair.first, pair.second, traits)) found = true;
      }
      (void)found;
    }, repetitions));

  // [08] Detection of intersections of one general polygon and one general polygon with holes
  print_result("[08] One GP vs One GP with Holes [All States]",
    measure_execution([&]() {
      bool found = false;
      for (const auto& pair : gp_gpwh_pairs) {
        if (CGAL::do_intersect(pair.first, pair.second, traits)) found = true;
      }
      (void)found;
    }, repetitions));

  // [09] Detection of intersections of two general polygons with holes
  print_result("[09] Two General Polygons with Holes [All States]",
    measure_execution([&]() {
      bool found = false;
      for (const auto& pair : gpwh_gpwh_pairs) {
        if (CGAL::do_intersect(pair.first, pair.second, traits)) found = true;
      }
      (void)found;
    }, repetitions));

  // [10] Detection of intersections of a set of general polygons (without holes)
  print_result("[10] Set of General Polygons (No Holes) [All-Pairs]",
    measure_execution([&]() {
      bool found = false;
      for (size_t i = 0; i < gp_set.size(); ++i) {
        for (size_t j = i + 1; j < gp_set.size(); ++j) {
          if (CGAL::do_intersect(gp_set[i], gp_set[j], traits)) found = true;
        }
      }
      (void)found;
    }, repetitions));

  // [11] Detection of intersections of a set of general polygons and a set of GP with holes
  print_result("[11] Set of General Polygons vs Set of GP with Holes",
    measure_execution([&]() {
      bool found = false;
      for (const auto& gp : gp_set) {
        for (const auto& gpwh : gpwh_set) {
          if (CGAL::do_intersect(gp, gpwh, traits)) found = true;
        }
      }
      (void)found;
    }, repetitions));

  // [12] Detection of intersections of a set of general polygons with holes
  print_result("[12] Set of General Polygons with Holes [All-Pairs]",
    measure_execution([&]() {
      bool found = false;
      for (size_t i = 0; i < gpwh_set.size(); ++i) {
        for (size_t j = i + 1; j < gpwh_set.size(); ++j) {
          if (CGAL::do_intersect(gpwh_set[i], gpwh_set[j], traits)) found = true;
        }
      }
      (void)found;
    }, repetitions));

  std::cout << "\nBenchmark complete.\n";
  return 0;
}
