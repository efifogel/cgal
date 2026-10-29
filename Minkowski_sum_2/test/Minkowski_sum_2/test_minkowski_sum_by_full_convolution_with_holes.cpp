// Tests the Minkowski sum of polygons with holes computed with the full
// convolution method, by comparing it with the sum computed with the reduced
// convolution method.

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_reduced_convolution_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/random_polygon_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>

namespace Ms = CGAL::Minkowski_sum_2;
namespace params = CGAL::parameters;

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using FT = Kernel::FT;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;
using Polygon_set_2 = CGAL::Polygon_set_2<Kernel>;

Polygon_2 box(FT x0, FT y0, FT x1, FT y1) {
  Polygon_2 pgn;
  pgn.push_back(Point_2(x0, y0));
  pgn.push_back(Point_2(x1, y0));
  pgn.push_back(Point_2(x1, y1));
  pgn.push_back(Point_2(x0, y1));
  return pgn;
}

// generates a random simple polygon, oriented counterclockwise, whose
// vertices lie in the square of the given half-size about (cx, cy).
Polygon_2 random_polygon(int n, double size, double cx, double cy, CGAL::Random& rnd) {
  using Gen_point_2 = CGAL::Exact_predicates_inexact_constructions_kernel::Point_2;
  std::vector<Gen_point_2> pts;
  CGAL::Random_points_in_square_2<Gen_point_2> gen(size, rnd);
  CGAL::random_polygon_2(n, std::back_inserter(pts), gen);
  Polygon_2 pgn;
  for (const auto& p : pts) pgn.push_back(Point_2(p.x() + cx, p.y() + cy));
  if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
  return pgn;
}

// generates a random polygon with holes, the square [0, size]^2 with random
// holes, oriented clockwise, in some of the cells of a grid of cells of a
// given size; every hole lies in the square of the given half-size about the
// center of its cell.
Polygon_with_holes_2 random_polygon_with_holes(double size, double cell, double hole_size, CGAL::Random& rnd) {
  std::vector<Polygon_2> holes;
  const int n = static_cast<int>(size / cell);
  for (int x = 0; x < n; ++x) for (int y = 0; y < n; ++y) {
    if (rnd.get_double() < 0.5) continue;
    holes.push_back(random_polygon(3 + rnd.get_int(0, 6), hole_size, cell * (x + 0.5), cell * (y + 0.5), rnd));
    holes.back().reverse_orientation();
  }
  return Polygon_with_holes_2(box(0, 0, size, size), holes.begin(), holes.end());
}

// compares the sums computed with every representation of the convolution
// cycles, and in both orders of the summands, with the expected one.
bool test(const std::string& name, const Polygon_with_holes_2& P, const Polygon_with_holes_2& Q) {
  const auto expected = Ms::minkowski_sum_by_reduced_convolution_2(P, Q);
  bool ok = true;
  for (auto curves : {Ms::Convolution_curves::AUTOMATIC, Ms::Convolution_curves::POLYLINES,
                      Ms::Convolution_curves::SEGMENTS}) {
    const auto np = params::convolution_curves(curves);
    for (const auto& sum : {Ms::minkowski_sum_by_full_convolution_2(P, Q, params::default_values(),
                                                                    params::default_values(), np),
                            Ms::minkowski_sum_by_full_convolution_2(Q, P, params::default_values(),
                                                                    params::default_values(), np)}) {
      Polygon_set_2 computed(sum);
      computed.symmetric_difference(expected);
      if (! computed.is_empty()) ok = false;
    }
  }
  std::cout << (ok ? "Passed: " : "Failed: ") << name << " (" << expected.number_of_holes() << " hole(s))"
            << std::endl;
  return ok;
}

// intersects an unbounded polygon with holes with a window: the window minus
// the holes.
Polygon_set_2 clip(const Polygon_with_holes_2& pwh, const Polygon_2& window) {
  Polygon_set_2 result(window);
  for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) {
    Polygon_2 hole = *hit;
    hole.reverse_orientation();
    result.difference(hole);
  }
  return result;
}

// tests the sum of an unbounded polygon with holes U and a bounded polygon
// with holes B: the expected sum, in a window around the holes of U, is
// computed with the reduced convolution method, where a large box that
// contains the window replaces the missing outer boundary of U.
bool test_unbounded(const std::string& name, const Polygon_with_holes_2& U, const Polygon_with_holes_2& B,
                    const Polygon_2& window, const Polygon_2& large_box) {
  const Polygon_with_holes_2 bounded_U(large_box, U.holes_begin(), U.holes_end());
  Polygon_set_2 expected(Ms::minkowski_sum_by_reduced_convolution_2(bounded_U, B));
  expected.intersection(window);
  bool ok = true;
  std::size_t num_holes = 0;
  for (auto curves : {Ms::Convolution_curves::AUTOMATIC, Ms::Convolution_curves::POLYLINES,
                      Ms::Convolution_curves::SEGMENTS}) {
    const auto np = params::convolution_curves(curves);
    for (const auto& sum : {Ms::minkowski_sum_by_full_convolution_2(U, B, params::default_values(),
                                                                    params::default_values(), np),
                            Ms::minkowski_sum_by_full_convolution_2(B, U, params::default_values(),
                                                                    params::default_values(), np)}) {
      if (! sum.is_unbounded()) ok = false;
      num_holes = sum.number_of_holes();
      auto computed = clip(sum, window);
      computed.symmetric_difference(expected);
      if (! computed.is_empty()) ok = false;
    }
  }
  std::cout << (ok ? "Passed: " : "Failed: ") << name << " (unbounded, " << num_holes << " hole(s))" << std::endl;
  return ok;
}

int main() {
  std::size_t failures = 0;
  auto run = [&](const std::string& name, const Polygon_with_holes_2& P, const Polygon_with_holes_2& Q)
  { if (! test(name, P, Q)) ++failures; };

  // A ring and squares; the hole of the ring is filled by the large square
  // and remains a (smaller) hole with the small square.
  std::vector<Polygon_2> ring_hole{box(2, 2, 8, 8)};
  ring_hole.front().reverse_orientation();
  const Polygon_with_holes_2 ring(box(0, 0, 10, 10), ring_hole.begin(), ring_hole.end());
  run("ring and large square", ring, Polygon_with_holes_2(box(-20, -20, 20, 20)));
  run("ring and small square", ring, Polygon_with_holes_2(box(0, 0, 1, 1)));
  run("ring and ring", ring, ring);

  // The mixed overloads.
  {
    const auto expected = Ms::minkowski_sum_by_reduced_convolution_2(ring, box(0, 0, 1, 1));
    Polygon_set_2 computed1(Ms::minkowski_sum_by_full_convolution_2(ring, box(0, 0, 1, 1)));
    Polygon_set_2 computed2(Ms::minkowski_sum_by_full_convolution_2(box(0, 0, 1, 1), ring));
    computed1.symmetric_difference(expected);
    computed2.symmetric_difference(expected);
    const bool ok = computed1.is_empty() && computed2.is_empty();
    std::cout << (ok ? "Passed: " : "Failed: ") << "polygon with holes and polygon" << std::endl;
    if (! ok) ++failures;
  }

  // Random polygons with holes, and random polygons, which may fit in the
  // holes.
  CGAL::Random rnd(5);
  for (int i = 0; i < 20; ++i) {
    const auto P = random_polygon_with_holes(20 + rnd.get_double(0, 10), 5, 1.2 + rnd.get_double(0, 1.2), rnd);
    const auto Q = (i % 2 == 0) ?
      Polygon_with_holes_2(random_polygon(3 + rnd.get_int(0, 8), 0.5 + rnd.get_double(0, 1.5), 0, 0, rnd)) :
      random_polygon_with_holes(4 + rnd.get_double(0, 2), 2, 0.4 + rnd.get_double(0, 0.5), rnd);
    run("random " + std::to_string(i), P, Q);
  }

  // Random pairs of polygons with holes, where the translations of one cover
  // holes of the other (so that the sum has no holes).
  for (int i = 0; i < 10; ++i) {
    const auto P = random_polygon_with_holes(30, 6, 0.8 + rnd.get_double(0, 1.6), rnd);
    const auto Q = random_polygon_with_holes(8.5, 4, 1.7, rnd);
    run("random holed pair " + std::to_string(i), P, Q);
  }

  // Unbounded polygons with holes (without outer boundaries).
  {
    std::vector<Polygon_2> holes{box(0, 0, 10, 10)};
    holes.front().reverse_orientation();
    const Polygon_with_holes_2 U(Polygon_2(), holes.begin(), holes.end());
    const Polygon_2 window = box(-20, -20, 30, 30), large_box = box(-100, -100, 110, 110);
    // The sum is the plane minus the square [2, 10] x [2, 10] ...
    if (! test_unbounded("unbounded and square", U, Polygon_with_holes_2(box(0, 0, 2, 2)), window, large_box))
      ++failures;
    // ... unaffected by the hole of the ring, ...
    std::vector<Polygon_2> small_hole{box(0.5, 0.5, 1.5, 1.5)};
    small_hole.front().reverse_orientation();
    const Polygon_with_holes_2 small_ring(box(0, 0, 2, 2), small_hole.begin(), small_hole.end());
    if (! test_unbounded("unbounded and ring", U, small_ring, window, large_box)) ++failures;
    // ... and the entire plane if the summand does not fit in the hole.
    if (! test_unbounded("unbounded and large square", U, Polygon_with_holes_2(box(0, 0, 11, 1)), window,
                         large_box))
      ++failures;
    // The mixed overloads.
    const auto sum1 = Ms::minkowski_sum_by_full_convolution_2(U, box(0, 0, 2, 2));
    const auto sum2 = Ms::minkowski_sum_by_full_convolution_2(box(0, 0, 2, 2), U);
    Polygon_set_2 expected(window);
    expected.difference(box(2, 2, 10, 10));
    auto diff1 = clip(sum1, window), diff2 = clip(sum2, window);
    diff1.symmetric_difference(expected);
    diff2.symmetric_difference(expected);
    const bool ok = sum1.is_unbounded() && (sum1.number_of_holes() == 1) && sum2.is_unbounded() &&
      (sum2.number_of_holes() == 1) && diff1.is_empty() && diff2.is_empty();
    std::cout << (ok ? "Passed: " : "Failed: ") << "unbounded polygon with holes and polygon" << std::endl;
    if (! ok) ++failures;
    // The sum of two unbounded polygons is the entire plane.
    const auto plane = Ms::minkowski_sum_by_full_convolution_2(U, U);
    const bool plane_ok = plane.is_unbounded() && (plane.number_of_holes() == 0);
    std::cout << (plane_ok ? "Passed: " : "Failed: ") << "two unbounded polygons" << std::endl;
    if (! plane_ok) ++failures;
  }
  for (int i = 0; i < 10; ++i) {
    std::vector<Polygon_2> holes;
    for (int x = 0; x < 3; ++x) for (int y = 0; y < 3; ++y) {
      if (rnd.get_double() < 0.4) continue;
      holes.push_back(random_polygon(3 + rnd.get_int(0, 8), 1 + rnd.get_double(0, 1.8), 6 * x + 3, 6 * y + 3, rnd));
      holes.back().reverse_orientation();
    }
    const Polygon_with_holes_2 U(Polygon_2(), holes.begin(), holes.end());
    const Polygon_with_holes_2 B(random_polygon(4 + rnd.get_int(0, 12), 0.3 + rnd.get_double(0, 1.5), 0, 0, rnd));
    if (! test_unbounded("random unbounded " + std::to_string(i), U, B, box(-10, -10, 28, 28),
                         box(-100, -100, 118, 118)))
      ++failures;
  }

  std::cout << failures << " failure(s)" << std::endl;
  return (failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
