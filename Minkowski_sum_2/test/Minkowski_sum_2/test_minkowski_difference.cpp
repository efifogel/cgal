// Tests the Minkowski difference of two polygons, the complement of the
// Minkowski sum of the first and the complement of the second, by comparing
// the result with the complement of a Minkowski sum computed with the reduced
// convolution method (within a large box).

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
#include <CGAL/Minkowski_sum_2/minkowski_difference_2.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using FT = Kernel::FT;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;
using Polygon_set_2 = CGAL::Polygon_set_2<Kernel>;
namespace Ms = CGAL::Minkowski_sum_2;
namespace params = CGAL::parameters;

Polygon_2 make(std::initializer_list<std::pair<double, double>> pts) {
  Polygon_2 pgn;
  for (auto [x, y] : pts) pgn.push_back(Point_2(x, y));
  return pgn;
}

Polygon_2 box(FT x0, FT y0, FT x1, FT y1) {
  Polygon_2 pgn;
  pgn.push_back(Point_2(x0, y0));
  pgn.push_back(Point_2(x1, y0));
  pgn.push_back(Point_2(x1, y1));
  pgn.push_back(Point_2(x0, y1));
  return pgn;
}

// generates a random simple polygon, oriented counterclockwise.
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

// computes the complement of P + complement(Q) within a window around Q, where
// the complement of Q is taken within a large box.
Polygon_set_2 expected_difference(const Polygon_with_holes_2& P, const Polygon_with_holes_2& Q) {
  auto bb = Q.outer_boundary().bbox() + P.outer_boundary().bbox();
  const double w = (bb.xmax() - bb.xmin()) + (bb.ymax() - bb.ymin()) + 10;
  Polygon_2 outer = Q.outer_boundary();
  if (outer.orientation() == CGAL::COUNTERCLOCKWISE) outer.reverse_orientation();
  std::vector<Polygon_2> holes{outer};
  Polygon_with_holes_2 outside(box(bb.xmin() - 4 * w, bb.ymin() - 4 * w, bb.xmax() + 4 * w, bb.ymax() + 4 * w),
                               holes.begin(), holes.end());
  Polygon_set_2 sum(Ms::minkowski_sum_by_reduced_convolution_2(P, outside));
  for (auto hit = Q.holes_begin(); hit != Q.holes_end(); ++hit) {
    Polygon_2 hole = *hit;
    if (hole.orientation() == CGAL::CLOCKWISE) hole.reverse_orientation();
    sum.join(Ms::minkowski_sum_by_reduced_convolution_2(P, hole));
  }
  Polygon_set_2 window(box(bb.xmin() - w, bb.ymin() - w, bb.xmax() + w, bb.ymax() + w));
  window.difference(sum);
  return window;
}

// compares the difference computed with every representation of the
// convolution cycles with the expected one.
template <typename PolygonType>
bool test(const std::string& name, const PolygonType& P, const Polygon_with_holes_2& Q) {
  const auto expected = expected_difference(Polygon_with_holes_2(P), Q);
  std::vector<std::vector<Polygon_with_holes_2>> results(4);
  Ms::minkowski_difference_2(P, Q, std::back_inserter(results[0]));
  Ms::minkowski_difference_by_full_convolution_2(P, Q, std::back_inserter(results[1]));
  Ms::minkowski_difference_by_full_convolution_2(P, Q, std::back_inserter(results[2]), params::default_values(),
                                                 params::default_values(),
                                                 params::convolution_curves(Ms::Convolution_curves::POLYLINES));
  Ms::minkowski_difference_by_full_convolution_2(P, Q, std::back_inserter(results[3]), params::default_values(),
                                                 params::default_values(),
                                                 params::convolution_curves(Ms::Convolution_curves::SEGMENTS));
  bool ok = true;
  for (const auto& result : results) {
    Polygon_set_2 computed;
    for (const auto& pwh : result) {
      if (! pwh.outer_boundary().is_simple() || (pwh.outer_boundary().orientation() != CGAL::COUNTERCLOCKWISE))
        ok = false;
      for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit)
        if (hit->orientation() != CGAL::CLOCKWISE) ok = false;
      computed.join(pwh);
    }
    computed.symmetric_difference(expected);
    if (! computed.is_empty()) ok = false;
  }
  std::cout << (ok ? "Passed: " : "Failed: ") << name << " (" << results[0].size() << " component(s))" << std::endl;
  return ok;
}

// tests the difference of a polygon P and an unbounded polygon with holes Q:
// the expected difference, in a window, is the window minus the union of the
// sums of P and the regions bounded by the holes of Q.
bool test_unbounded(const std::string& name, const Polygon_2& P, const Polygon_with_holes_2& Q,
                    const Polygon_2& window, std::size_t expected_components) {
  Polygon_set_2 expected(window);
  for (auto hit = Q.holes_begin(); hit != Q.holes_end(); ++hit) {
    Polygon_2 hole = *hit;
    hole.reverse_orientation();
    expected.difference(Ms::minkowski_sum_by_reduced_convolution_2(P, hole));
  }
  bool ok = true;
  for (auto curves : {Ms::Convolution_curves::AUTOMATIC, Ms::Convolution_curves::POLYLINES,
                      Ms::Convolution_curves::SEGMENTS}) {
    std::vector<Polygon_with_holes_2> result;
    Ms::minkowski_difference_by_full_convolution_2(P, Q, std::back_inserter(result), params::default_values(),
                                                   params::default_values(), params::convolution_curves(curves));
    // The unbounded component is reported first.
    if ((result.size() != expected_components) || ! result.front().is_unbounded()) {
      ok = false;
      continue;
    }
    Polygon_set_2 computed(window);
    for (auto hit = result.front().holes_begin(); hit != result.front().holes_end(); ++hit) {
      Polygon_2 hole = *hit;
      hole.reverse_orientation();
      computed.difference(hole);
    }
    for (std::size_t i = 1; i < result.size(); ++i) {
      if (result[i].is_unbounded()) ok = false;
      else computed.join(result[i]);
    }
    computed.symmetric_difference(expected);
    if (! computed.is_empty()) ok = false;
  }
  std::cout << (ok ? "Passed: " : "Failed: ") << name << " (unbounded, " << expected_components
            << " component(s))" << std::endl;
  return ok;
}

int main() {
  std::size_t failures = 0;
  auto run = [&](const std::string& name, const auto& P, const Polygon_with_holes_2& Q)
  { if (! test(name, P, Q)) ++failures; };

  const Polygon_2 L = make({{0, 0}, {6, 0}, {6, 2}, {2, 2}, {2, 6}, {0, 6}});
  run("square in a square", box(-1, -1, 1, 1), Polygon_with_holes_2(box(-2, -2, 2, 2)));
  run("triangle in a square", make({{0, 0}, {1, 0}, {0, 1}}), Polygon_with_holes_2(box(0, 0, 4, 4)));
  run("square in an L", box(0, 0, 1, 1), Polygon_with_holes_2(L));
  run("square in a U", box(0, 0, 1, 1),
      Polygon_with_holes_2(make({{0, 0}, {7, 0}, {7, 5}, {5, 5}, {5, 2}, {2, 2}, {2, 5}, {0, 5}})));
  run("a polygon that does not fit (empty)", box(0, 0, 5, 5), Polygon_with_holes_2(box(0, 0, 4, 4)));
  run("dumbbell (disconnected)", box(0, 0, 2, 2),
      Polygon_with_holes_2(make({{0, 0}, {4, 0}, {4, 1.5}, {6, 1.5}, {6, 0}, {10, 0}, {10, 4}, {6, 4}, {6, 2.5},
                                 {4, 2.5}, {4, 4}, {0, 4}})));
  run("polygon with a straight vertex", make({{0, 0}, {0.5, 0}, {1, 0}, {1, 1}, {0, 1}}), Polygon_with_holes_2(L));
  {
    std::vector<Polygon_2> holes{box(4, 4, 5, 5), box(10, 3, 12, 4), make({{3, 12}, {5, 12}, {4, 14}})};
    for (auto& hole : holes) hole.reverse_orientation();
    const Polygon_with_holes_2 Q(box(0, 0, 16, 16), holes.begin(), holes.end());
    run("square in a square with holes", box(0, 0, 1, 1), Q);
    run("triangle in a square with holes", make({{0, 0}, {2, 0}, {1, 1.5}}), Q);
  }

  // Polygons with holes as the first operand: a ring may surround an
  // obstacle (a hole of the second operand), in which case the difference has
  // a component around the obstacle; without obstacles, the hole of the ring
  // does not matter.
  {
    std::vector<Polygon_2> ring_hole{box(-1, -1, 1, 1)};
    ring_hole.front().reverse_orientation();
    const Polygon_with_holes_2 ring(box(-2, -2, 2, 2), ring_hole.begin(), ring_hole.end());
    std::vector<Polygon_2> obstacles{box(9.5, 9.5, 10.5, 10.5), make({{4, 14}, {5, 14}, {4.5, 15}})};
    for (auto& obstacle : obstacles) obstacle.reverse_orientation();
    run("ring in a square with obstacles", ring, Polygon_with_holes_2(box(0, 0, 20, 20), obstacles.begin(),
                                                                       obstacles.end()));
    run("ring in an L", ring, Polygon_with_holes_2(make({{0, 0}, {12, 0}, {12, 5}, {5, 5}, {5, 12}, {0, 12}})));
    run("ring and a square", ring, Polygon_with_holes_2(box(0, 0, 3, 3)));
  }

  // Unbounded polygons with holes (without outer boundaries).
  {
    auto unbounded = [](std::vector<Polygon_2> holes) {
      for (auto& hole : holes) hole.reverse_orientation();
      return Polygon_with_holes_2(Polygon_2(), holes.begin(), holes.end());
    };
    const Polygon_2 window = box(-10, -10, 25, 25);
    if (! test_unbounded("square and the plane minus a square", box(0, 0, 2, 2), unbounded({box(0, 0, 10, 10)}),
                         window, 1))
      ++failures;
    // Four bars around a square, with gaps that the sums close, so that the
    // difference has a bounded component inside the bars.
    if (! test_unbounded("square and the plane minus four bars", box(0, 0, 2, 2),
                         unbounded({box(0, 0, 9, 1), box(10, 0, 11, 9), box(2, 10, 11, 11), box(0, 2, 1, 11)}),
                         window, 2))
      ++failures;
    // An unbounded P fits in an unbounded Q iff the holes of Q fit in the
    // holes of P, and it does not fit in a bounded Q.
    {
      const auto P = unbounded({box(0, 0, 4, 4)});
      std::vector<Polygon_with_holes_2> result, none;
      Ms::minkowski_difference_2(P, unbounded({box(1, 1, 2, 2)}), std::back_inserter(result));
      Polygon_set_2 computed;
      for (const auto& pwh : result) computed.join(pwh);
      computed.symmetric_difference(Polygon_set_2(box(2, 2, 5, 5)));
      Ms::minkowski_difference_2(P, box(-10, -10, 10, 10), std::back_inserter(none));
      const bool ok = (result.size() == 1) && ! result.front().is_unbounded() && computed.is_empty() && none.empty();
      std::cout << (ok ? "Passed: " : "Failed: ") << "unbounded polygons" << std::endl;
      if (! ok) ++failures;
    }
    std::vector<Polygon_with_holes_2> plane;
    Ms::minkowski_difference_2(box(0, 0, 1, 1), Polygon_with_holes_2(), std::back_inserter(plane));
    const bool ok = (plane.size() == 1) && plane.front().is_unbounded() && (plane.front().number_of_holes() == 0);
    std::cout << (ok ? "Passed: " : "Failed: ") << "the plane" << std::endl;
    if (! ok) ++failures;
  }

  // Random nonconvex polygons.
  CGAL::Random rnd(17);
  for (int i = 0; i < 20; ++i) {
    const Polygon_2 Q = random_polygon(10 + rnd.get_int(0, 40), 10, 0, 0, rnd);
    const Polygon_2 P = random_polygon(3 + rnd.get_int(0, 6), 0.5 + rnd.get_double(0, 1.5), 0, 0, rnd);
    run("random " + std::to_string(i), P, Polygon_with_holes_2(Q));
  }

  // Random nonconvex polygons in a square with random holes.
  for (int i = 0; i < 8; ++i) {
    std::vector<Polygon_2> holes;
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) {
      if (rnd.get_double() < 0.5) continue;
      holes.push_back(random_polygon(3 + rnd.get_int(0, 6), 0.8, 2 + 4 * x, 2 + 4 * y, rnd));
      holes.back().reverse_orientation();
    }
    const Polygon_2 P = random_polygon(3 + rnd.get_int(0, 5), 0.4 + rnd.get_double(0, 0.8), 0, 0, rnd);
    run("random with holes " + std::to_string(i), P, Polygon_with_holes_2(box(0, 0, 16, 16), holes.begin(),
                                                                          holes.end()));
  }

  // Random rings in a square with random obstacles.
  for (int i = 0; i < 8; ++i) {
    std::vector<Polygon_2> ring_hole{random_polygon(3 + rnd.get_int(0, 6), 0.9, 0, 0, rnd)};
    ring_hole.front().reverse_orientation();
    const Polygon_with_holes_2 ring(box(-1.5, -1.5, 1.5, 1.5), ring_hole.begin(), ring_hole.end());
    std::vector<Polygon_2> obstacles;
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) {
      if (rnd.get_double() < 0.5) continue;
      obstacles.push_back(random_polygon(3 + rnd.get_int(0, 4), 0.1 + rnd.get_double(0, 0.4), 2 + 4 * x, 2 + 4 * y,
                                         rnd));
      obstacles.back().reverse_orientation();
    }
    run("random ring with obstacles " + std::to_string(i), ring,
        Polygon_with_holes_2(box(0, 0, 16, 16), obstacles.begin(), obstacles.end()));
  }

  std::cout << failures << " failure(s)" << std::endl;
  return (failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
