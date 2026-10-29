// Tests the named parameters of the Minkowski sum and difference functions:
// the method of Minkowski_sum_2::minkowski_sum_2(), the decomposition strategies and convexity
// hints of the polygons, the traits, and the representation of the
// convolution cycles. The sums are compared with the sums computed with the
// reduced convolution method.

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_difference_2.h>
#include <CGAL/Minkowski_sum_2/approximated_offset_2.h>
#include <CGAL/Minkowski_sum_2/approximated_inset_2.h>
#include <CGAL/Exact_rational.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Polygon_triangulation_decomposition_2.h>
#include <CGAL/Polygon_convex_decomposition_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/random_polygon_2.h>
#include <CGAL/random_convex_set_2.h>
#include <CGAL/convex_hull_2.h>

namespace Ms = CGAL::Minkowski_sum_2;
namespace params = CGAL::parameters;

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using FT = Kernel::FT;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;
using Polygon_set_2 = CGAL::Polygon_set_2<Kernel>;
using Gen_point_2 = CGAL::Exact_predicates_inexact_constructions_kernel::Point_2;

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
  std::vector<Gen_point_2> pts;
  CGAL::Random_points_in_square_2<Gen_point_2> gen(size, rnd);
  CGAL::random_polygon_2(n, std::back_inserter(pts), gen);
  Polygon_2 pgn;
  for (const auto& p : pts) pgn.push_back(Point_2(p.x() + cx, p.y() + cy));
  if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
  return pgn;
}

// generates a random convex polygon, oriented counterclockwise.
Polygon_2 random_convex_polygon(int n, double size, CGAL::Random& rnd) {
  std::vector<Gen_point_2> pts;
  CGAL::Random_points_in_square_2<Gen_point_2> gen(size, rnd);
  CGAL::random_convex_set_2(n, std::back_inserter(pts), gen);
  std::vector<Point_2> exact, hull;
  for (const auto& p : pts) exact.push_back(Point_2(p.x(), p.y()));
  CGAL::convex_hull_2(exact.begin(), exact.end(), std::back_inserter(hull));
  return Polygon_2(hull.begin(), hull.end());
}

// generates a square with random holes in the cells of a grid.
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

bool same(const Polygon_with_holes_2& a, const Polygon_with_holes_2& b) {
  Polygon_set_2 diff(a);
  diff.symmetric_difference(Polygon_set_2(b));
  return diff.is_empty();
}

std::size_t failures = 0;

void report(const std::string& name, bool ok) {
  std::cout << (ok ? "Passed: " : "Failed: ") << name << std::endl;
  if (! ok) ++failures;
}

// compares the sums computed by minkowski_sum_2() with every method, by
// minkowski_sum_by_decomposition_2(), and by the full convolution method with
// both representations of the convolution cycles, with the expected sum.
template <typename PolygonType1, typename PolygonType2, typename NP1, typename NP2>
void test_methods(const std::string& name, const PolygonType1& P, const PolygonType2& Q,
                  const NP1& np1, const NP2& np2) {
  const auto expected = Ms::minkowski_sum_by_reduced_convolution_2(P, Q);
  bool ok = same(Ms::minkowski_sum_2(P, Q), expected) && same(Ms::minkowski_sum_2(P, Q, np1, np2), expected);
  for (auto method : {Ms::Method::AUTOMATIC, Ms::Method::DECOMPOSITION, Ms::Method::REDUCED_CONVOLUTION,
                      Ms::Method::FULL_CONVOLUTION})
    if (! same(Ms::minkowski_sum_2(P, Q, np1, np2, params::method(method)), expected)) ok = false;
  if (! same(Ms::minkowski_sum_by_decomposition_2(P, Q, np1, np2), expected)) ok = false;
  for (auto curves : {Ms::Convolution_curves::POLYLINES, Ms::Convolution_curves::SEGMENTS})
    if (! same(Ms::minkowski_sum_by_full_convolution_2(P, Q, np1, np2, params::convolution_curves(curves)), expected))
      ok = false;
  report(name, ok);
}

// A decomposition strategy that counts its copies (the copies of a
// default-constructed instance are not counted).
template <typename Strategy>
struct Counting_strategy : Strategy {
  std::size_t* copies = nullptr;

  Counting_strategy() = default;

  explicit Counting_strategy(std::size_t* c) : copies(c) {}

  Counting_strategy(const Counting_strategy& other) : Strategy(other), copies(other.copies) { if (copies) ++*copies; }
};

int main() {
  CGAL::Random rnd(11);
  const Polygon_2 triangle = [] {
    Polygon_2 pgn;
    pgn.push_back(Point_2(0, 0));
    pgn.push_back(Point_2(4, 0));
    pgn.push_back(Point_2(2, 3));
    return pgn;
  }();
  const Polygon_2 L = [] {
    Polygon_2 pgn;
    for (auto p : {Point_2(0, 0), Point_2(6, 0), Point_2(6, 2), Point_2(2, 2), Point_2(2, 6), Point_2(0, 6)})
      pgn.push_back(p);
    return pgn;
  }();
  std::vector<Polygon_2> ring_holes{box(2, 2, 8, 8)};
  ring_holes.front().reverse_orientation();
  const Polygon_with_holes_2 ring(box(0, 0, 10, 10), ring_holes.begin(), ring_holes.end());

  // All the methods, for every combination of the operand types.
  const auto none = params::default_values();
  test_methods("triangle and L", triangle, L, none, none);
  test_methods("ring and triangle", ring, triangle, none, none);
  test_methods("triangle and ring", triangle, ring, none, none);
  test_methods("ring and ring", ring, ring, none, none);
  // Unbounded polygons, which the decomposition method does not support.
  {
    const Polygon_with_holes_2 unbounded(Polygon_2(), ring_holes.begin(), ring_holes.end());
    bool ok = true;
    for (const auto& Q : {Polygon_with_holes_2(triangle), ring, unbounded}) {
      const auto expected = Ms::minkowski_sum_by_full_convolution_2(unbounded, Q);
      for (auto method : {Ms::Method::AUTOMATIC, Ms::Method::REDUCED_CONVOLUTION, Ms::Method::FULL_CONVOLUTION})
        if (! same(Ms::minkowski_sum_2(unbounded, Q, none, none, params::method(method)), expected) ||
            ! same(Ms::minkowski_sum_2(Q, unbounded, none, none, params::method(method)), expected))
          ok = false;
    }
    report("unbounded polygons", ok);
  }

  // Convexity hints; a hint that a convex polygon is not convex only disables
  // the shortcuts.
  test_methods("convex hints", triangle, box(0, 0, 1, 1), params::is_convex(true), params::is_convex(true));
  test_methods("negative hints of convex polygons", triangle, box(0, 0, 1, 1), params::is_convex(false),
               params::is_convex(false));
  test_methods("convex and non-convex hints", triangle, L, params::is_convex(true), params::is_convex(false));
  test_methods("non-convex hint of a polygon with holes", ring, triangle, params::is_convex(false),
               params::is_convex(true));

  // Convex polygons with collinear vertices and parallel edges, whose sum is
  // computed directly.
  {
    Polygon_2 P;
    for (auto p : {Point_2(0, 0), Point_2(1, 0), Point_2(2, 0), Point_2(2, 1), Point_2(2, 2), Point_2(0, 2)})
      P.push_back(p);
    Polygon_2 Q;
    for (auto p : {Point_2(0, 0), Point_2(3, 0), Point_2(3, 1), Point_2(1, 2), Point_2(0, 1)}) Q.push_back(p);
    test_methods("convex polygons with collinear vertices", P, Q, none, none);
    test_methods("convex polygons with collinear vertices, hinted", P, Q, params::is_convex(true),
                 params::is_convex(true));
  }
  for (int i = 0; i < 10; ++i) {
    const auto P = random_convex_polygon(3 + rnd.get_int(0, 30), 5, rnd);
    const auto Q = random_convex_polygon(3 + rnd.get_int(0, 30), 2, rnd);
    test_methods("random convex polygons " + std::to_string(i), P, Q, none, none);
  }

  // Decomposition strategies, different for the two polygons, and for
  // polygons with and without holes.
  {
    CGAL::Polygon_triangulation_decomposition_2<Kernel> triangulation;
    CGAL::Optimal_convex_decomposition_2<Kernel> optimal;
    CGAL::Hertel_Mehlhorn_convex_decomposition_2<Kernel> hertel_mehlhorn;
    for (int i = 0; i < 6; ++i) {
      const auto P = random_polygon(5 + rnd.get_int(0, 15), 3, 0, 0, rnd);
      const auto Q = random_polygon_with_holes(16, 4, 0.5 + rnd.get_double(0, 1), rnd);
      const auto expected = Ms::minkowski_sum_by_reduced_convolution_2(Polygon_with_holes_2(P), Q);
      const bool ok =
        same(Ms::minkowski_sum_by_decomposition_2(P, Q,
                                                  params::decomposition_strategy(optimal),
                                                  params::decomposition_strategy(hertel_mehlhorn)
                                                    .decomposition_strategy_with_holes(triangulation)),
             expected) &&
        same(Ms::minkowski_sum_2(Q, P,
                                 params::decomposition_strategy_with_holes(triangulation),
                                 params::decomposition_strategy(hertel_mehlhorn),
                                 params::method(Ms::Method::DECOMPOSITION)),
             expected) &&
        same(Ms::minkowski_sum_2(P, Q), expected);
      report("decomposition strategies " + std::to_string(i), ok);
    }
  }

  // The automatic choice of the method (see Ms::internal::choose_method()):
  // the full convolution method for a sparse convolution, the reduced
  // convolution method for a dense one, and the decomposition method for a
  // very large convolution of two highly concave polygons.  A comb with k
  // teeth has 2k + 3 vertices, k of which are reflex.
  {
    auto comb = [](int k) {
      Polygon_2 pgn;
      pgn.push_back(Point_2(0, 0));
      pgn.push_back(Point_2(2 * k, 0));
      for (int i = k; i > 0; --i) {
        pgn.push_back(Point_2(2 * i, 3));
        pgn.push_back(Point_2(2 * i - 1, 1));
      }
      pgn.push_back(Point_2(0, 3));
      return pgn;
    };
    auto choice = [](const Polygon_2& P, const Polygon_2& Q)
    { return Ms::internal::choose_method(Polygon_with_holes_2(P), Polygon_with_holes_2(Q), Kernel()); };
    report("automatic choice",
           (choice(triangle, comb(10)) == Ms::Method::FULL_CONVOLUTION) &&
           (choice(comb(10), comb(10)) == Ms::Method::REDUCED_CONVOLUTION) &&
           (choice(comb(150), comb(150)) == Ms::Method::DECOMPOSITION));
    test_methods("triangle and comb", triangle, comb(10), none, none);
    test_methods("comb and comb", comb(6), comb(4), none, none);
    // The hole of the ring is removed, as the box does not fit in it; then,
    // both polygons are convex, and their sum is computed directly.
    test_methods("ring and a large box", ring, box(0, 0, 7, 7), none, none);
  }

  // Traits.
  {
    CGAL::Gps_segment_traits_2<Kernel> traits;
    const auto expected = Ms::minkowski_sum_by_reduced_convolution_2(ring, L);
    bool ok = true;
    for (auto method : {Ms::Method::AUTOMATIC, Ms::Method::DECOMPOSITION, Ms::Method::REDUCED_CONVOLUTION,
                        Ms::Method::FULL_CONVOLUTION})
      if (! same(Ms::minkowski_sum_2(ring, L, none, none, params::geom_traits(traits).method(method)), expected))
        ok = false;
    std::vector<Polygon_with_holes_2> diff1, diff2;
    Ms::minkowski_difference_2(box(0, 0, 1, 1), ring, std::back_inserter(diff1));
    Ms::minkowski_difference_2(box(0, 0, 1, 1), ring, std::back_inserter(diff2), params::is_convex(true), none,
                               params::geom_traits(traits).convolution_curves(Ms::Convolution_curves::SEGMENTS));
    Polygon_set_2 set1, set2;
    for (const auto& pwh : diff1) set1.join(pwh);
    for (const auto& pwh : diff2) set2.join(pwh);
    set1.symmetric_difference(set2);
    report("traits", ok && ! diff1.empty() && set1.is_empty());
  }

  // Traits given by a shared pointer or by a reference (with std::cref()) are
  // shared or referred to by the named parameter, and not copied; traits given
  // by value are copied.  The computation itself may copy the traits (see the
  // documentation of `geom_traits`), except for the full convolution with
  // polylines, which does not.
  {
    // The traits must be default constructible (see `AosBasicTraits_2`); the
    // copies of a default-constructed instance are not counted.
    struct Counting_traits : CGAL::Gps_segment_traits_2<Kernel> {
      std::size_t* copies = nullptr;

      Counting_traits() = default;

      explicit Counting_traits(std::size_t* c) : copies(c) {}

      Counting_traits(const Counting_traits& other) :
        CGAL::Gps_segment_traits_2<Kernel>(other), copies(other.copies)
      { if (copies) ++*copies; }
    };

    std::size_t copies = 0;
    const auto expected = Ms::minkowski_sum_by_reduced_convolution_2(ring, L);
    auto shared = std::make_shared<Counting_traits>(&copies);
    std::shared_ptr<const Counting_traits> shared_const = shared;
    const Counting_traits& traits = *shared;
    bool ok = true;
    for (auto method : {Ms::Method::AUTOMATIC, Ms::Method::DECOMPOSITION, Ms::Method::FULL_CONVOLUTION}) {
      ok = ok && same(Ms::minkowski_sum_2(ring, L, none, none, params::geom_traits(shared).method(method)), expected);
      ok = ok && same(Ms::minkowski_sum_2(ring, L, none, none, params::geom_traits(shared_const).method(method)),
                      expected);
      ok = ok && same(Ms::minkowski_sum_2(ring, L, none, none, params::geom_traits(std::cref(traits)).method(method)),
                      expected);
    }
    std::vector<Polygon_with_holes_2> diff;
    Ms::minkowski_difference_2(box(0, 0, 1, 1), ring, std::back_inserter(diff), none, none,
                               params::geom_traits(shared));
    ok = ok && same(Ms::minkowski_sum_2(ring, L, none, none, params::geom_traits(traits)), expected);
    ok = ok && ! diff.empty() && (shared.use_count() == 2);

    // The named parameter shares, refers to, or copies the traits.
    using Ms::internal::shared_traits;
    ok = ok && (shared_traits<Kernel, Polygon_2::Container>(params::geom_traits(shared)).get() == shared.get()) &&
      (shared_traits<Kernel, Polygon_2::Container>(params::geom_traits(std::cref(traits))).get() == &traits) &&
      (shared_traits<Kernel, Polygon_2::Container>(params::geom_traits(traits)).get() != &traits);

    // The full convolution with polylines does not copy the traits.
    copies = 0;
    Ms::minkowski_sum_by_full_convolution_2(ring, L, none, none,
      params::geom_traits(shared).convolution_curves(Ms::Convolution_curves::POLYLINES));
    report("shared traits", ok && (copies == 0));
  }

  // The offset and the inset, with the convolution method and with the
  // decomposition method, using the default strategy, a given strategy, and
  // the nop strategy for a polygon hinted to be convex.
  {
    using Rat_kernel = CGAL::Cartesian<CGAL::Exact_rational>;
    using Rat_polygon_2 = CGAL::Polygon_2<Rat_kernel>;
    using Rat_polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Rat_kernel>;
    using Circle_traits_2 = CGAL::Gps_circle_segment_traits_2<Rat_kernel>;
    using Offset_polygon_with_holes_2 = Circle_traits_2::Polygon_with_holes_2;
    auto to_rat = [](const Polygon_2& pgn) {
      Rat_polygon_2 res;
      for (const auto& p : pgn)
        res.push_back(Rat_kernel::Point_2(CGAL::to_double(p.x()), CGAL::to_double(p.y())));
      return res;
    };
    auto same_offset = [](const Offset_polygon_with_holes_2& a, const Offset_polygon_with_holes_2& b) {
      std::vector<Offset_polygon_with_holes_2> diff;
      CGAL::symmetric_difference(a, b, std::back_inserter(diff));
      return diff.empty();
    };
    const auto decomposition = params::method(Ms::Method::DECOMPOSITION);
    const Rat_polygon_2 rat_L = to_rat(L), rat_triangle = to_rat(triangle);
    const Rat_kernel::FT r(1, 2);
    const double eps = 0.001;
    CGAL::Optimal_convex_decomposition_2<Rat_kernel> optimal;
    const auto conv = Ms::approximated_offset_2(rat_L, r, eps);
    bool ok = same_offset(conv, Ms::approximated_offset_2(rat_L, r, eps, none, decomposition)) &&
      same_offset(conv, Ms::approximated_offset_2(rat_L, r, eps, params::decomposition_strategy(optimal),
                                                  decomposition)) &&
      same_offset(Ms::approximated_offset_2(rat_triangle, r, eps),
                  Ms::approximated_offset_2(rat_triangle, r, eps, params::is_convex(true), decomposition));
    std::vector<Rat_polygon_2> holes{to_rat(ring_holes.front())};
    const Rat_polygon_with_holes_2 rat_ring(to_rat(ring.outer_boundary()), holes.begin(), holes.end());
    const auto ring_offset = Ms::approximated_offset_2(rat_ring, r, eps);
    if (ring_offset.number_of_holes() != 1) ok = false;
    std::vector<Circle_traits_2::Polygon_2> inset;
    Ms::approximated_inset_2(rat_triangle, Rat_kernel::FT(1, 4), eps, std::back_inserter(inset),
                             params::is_convex(true));
    report("offset and inset", ok && (inset.size() == 1));

    // A strategy given by a reference (with std::cref()) is not copied; a
    // strategy given by value is.
    std::size_t copies = 0;
    const Counting_strategy<CGAL::Optimal_convex_decomposition_2<Rat_kernel>> counting_optimal(&copies);
    bool ok_cref =
      same_offset(conv, Ms::approximated_offset_2(rat_L, r, eps,
                                                  params::decomposition_strategy(std::cref(counting_optimal)),
                                                  decomposition)) && (copies == 0);
    ok_cref = ok_cref && same_offset(conv, Ms::approximated_offset_2(rat_L, r, eps,
                                                                     params::decomposition_strategy(counting_optimal),
                                                                     decomposition)) && (copies > 0);
    report("offset strategy by reference", ok_cref);
  }

  // A strategy given by a reference (with std::cref()) or by a shared pointer
  // is not copied; a strategy given by value is.
  {
    std::size_t copies = 0;
    const Counting_strategy<CGAL::Greene_convex_decomposition_2<Kernel>> greene(&copies);
    const Counting_strategy<CGAL::Polygon_triangulation_decomposition_2<Kernel>> triangulation(&copies);
    const auto expected_L = Ms::minkowski_sum_by_reduced_convolution_2(L, L);
    const auto expected_ring = Ms::minkowski_sum_by_reduced_convolution_2(ring, L);
    const auto by_ref = params::decomposition_strategy(std::cref(greene))
      .decomposition_strategy_with_holes(std::cref(triangulation));
    bool ok = same(Ms::minkowski_sum_by_decomposition_2(L, L, by_ref, by_ref), expected_L) &&
      same(Ms::minkowski_sum_by_decomposition_2(ring, L, by_ref, by_ref), expected_ring) &&
      same(Ms::minkowski_sum_2(ring, L, by_ref, by_ref, params::method(Ms::Method::DECOMPOSITION)), expected_ring) &&
      (copies == 0);
    auto shared_greene =
      std::make_shared<const Counting_strategy<CGAL::Greene_convex_decomposition_2<Kernel>>>(&copies);
    auto shared_triangulation =
      std::make_shared<const Counting_strategy<CGAL::Polygon_triangulation_decomposition_2<Kernel>>>(&copies);
    const auto by_ptr = params::decomposition_strategy(shared_greene)
      .decomposition_strategy_with_holes(shared_triangulation);
    ok = ok && same(Ms::minkowski_sum_by_decomposition_2(L, L, by_ptr, by_ptr), expected_L) &&
      same(Ms::minkowski_sum_by_decomposition_2(ring, L, by_ptr, by_ptr), expected_ring) && (copies == 0) &&
      // Only the named parameters (by_ptr) share the strategies besides us.
      (shared_greene.use_count() == 2) && (shared_triangulation.use_count() == 2);
    const auto by_value = params::decomposition_strategy(greene).decomposition_strategy_with_holes(triangulation);
    ok = ok && same(Ms::minkowski_sum_by_decomposition_2(ring, L, by_value, by_value), expected_ring) && (copies > 0);
    report("strategies by reference or by a shared pointer", ok);
  }

  std::cout << failures << " failure(s)" << std::endl;
  return (failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
