//! \file test_algebraic_approximate.cpp
// Tests that Approximate_2 of the algebraic segment traits approximates every bounded arc, also an arc whose
// x-range is too narrow to sample in double precision, and every arc that meets a bounding box, also where a side of
// the box passes through an end of the arc. The polyline must span the y-range of the arc, clamped to the box if the
// box contains the x-range of the arc. Within a box, a point below or above the box must be adjacent only to points
// that lie below or above the box or on its bottom or top side.

#include <CGAL/config.h>
#include <iostream>

#if ! defined(CGAL_USE_CORE)

int main() {
  std::cout << "This test requires CORE; it is skipped." << std::endl;
  return 0;
}

#else

#include <algorithm>
#include <cmath>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include <CGAL/CORE_BigInt.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>

using Traits = CGAL::Arr_algebraic_segment_traits_2<CORE::BigInt>;
using Polynomial_2 = Traits::Polynomial_2;
using X_monotone_curve_2 = Traits::X_monotone_curve_2;
using Approximate_point_2 = Traits::Approximate_2::Approximate_point_2;
using Arrangement = CGAL::Arrangement_2<Traits>;

int failures = 0;

/*! checks that the polyline of an arc spans the y-range of the arc clamped to the box (if any), up to the error,
 * and that a point below or above the box (if any) is adjacent only to points below or above the box or on its bottom
 * or top side.
 */
void check(const Traits& traits, const X_monotone_curve_2& xcv, double error, const std::optional<CGAL::Bbox_2>& box,
           const std::string& name) {
  std::vector<Approximate_point_2> points;
  auto approximate = traits.approximate_2_object();
  if (box) approximate(xcv, error, std::back_inserter(points), *box);
  else approximate(xcv, error, std::back_inserter(points));

  const CGAL::Bbox_2 bbox = traits.construct_bbox_2_object()(xcv);
  double ymin = bbox.ymin();
  double ymax = bbox.ymax();
  if (box) {
    ymin = (std::min)((std::max)(ymin, box->ymin()), box->ymax());
    ymax = (std::min)((std::max)(ymax, box->ymin()), box->ymax());
  }
  // the y-range within a box that cuts the arc is not known; then a polyline is only required
  const bool cut = box && ((bbox.xmin() < box->xmin()) || (bbox.xmax() > box->xmax()));
  bool ok = (points.size() >= 2);
  if (ok && box && ! cut) {
    // an arc that leaves the box through its bottom (resp. top) side must be represented by points below (resp.
    // above) the box
    auto by_y = [](const Approximate_point_2& p, const Approximate_point_2& q) { return p.y() < q.y(); };
    const auto [lo, hi] = std::minmax_element(points.begin(), points.end(), by_y);
    if (((bbox.ymin() < box->ymin() - error) && ! (lo->y() < box->ymin())) ||
        ((bbox.ymax() > box->ymax() + error) && ! (hi->y() > box->ymax()))) {
      ++failures;
      std::cerr << "Error: " << name << ": no point represents the part of an arc below or above the box" << std::endl;
      return;
    }
  }
  if (ok && box) {
    auto outside = [&](const Approximate_point_2& p) { return (p.y() < box->ymin()) || (p.y() > box->ymax()); };
    auto on_side = [&](const Approximate_point_2& p) { return (p.y() == box->ymin()) || (p.y() == box->ymax()); };
    for (std::size_t i = 1; i < points.size(); ++i) {
      const auto& p = points[i - 1];
      const auto& q = points[i];
      if ((outside(p) && ! outside(q) && ! on_side(q)) || (outside(q) && ! outside(p) && ! on_side(p))) {
        ++failures;
        std::cerr << "Error: " << name << ": a point outside the box is adjacent to (" << (outside(p) ? q : p)
                  << "), which lies inside the box but not on its bottom or top side" << std::endl;
        return;
      }
    }
    // clamping the points to the box yields the approximation of the arc clamped to the box
    for (auto& p : points) p = Approximate_point_2(p.x(), (std::min)((std::max)(p.y(), box->ymin()), box->ymax()));
  }
  if (ok && ! cut) {
    auto by_y = [](const Approximate_point_2& p, const Approximate_point_2& q) { return p.y() < q.y(); };
    const auto [lo, hi] = std::minmax_element(points.begin(), points.end(), by_y);
    ok = (lo->y() <= ymin + error) && (hi->y() >= ymax - error);
  }
  if (ok) return;
  ++failures;
  std::cerr << "Error: " << name << ": " << points.size() << " points do not span the y-range [" << ymin << ", "
            << ymax << "] of an arc" << std::endl;
}

/*! checks the arcs of the arrangement of the curves of given polynomials, which have bounded arcs or meet the box.
 */
void check_curves(const std::vector<Polynomial_2>& polynomials, double error, const std::optional<CGAL::Bbox_2>& box,
                  const std::string& name) {
  Traits traits;
  Arrangement arr(&traits);
  for (const auto& pol : polynomials) CGAL::insert(arr, traits.construct_curve_2_object()(pol));
  for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
    const auto& xcv = eit->curve();
    if (xcv.is_vertical()) continue;
    const CGAL::Bbox_2 bbox = traits.construct_bbox_2_object()(xcv);
    if (box) {
      // arcs that do not reach into the box, or touch it only at infinity, are not checked
      if ((bbox.xmax() <= box->xmin()) || (bbox.xmin() >= box->xmax())) continue;
      if ((bbox.ymax() <= box->ymin()) || (bbox.ymin() >= box->ymax())) continue;
    }
    else if (! xcv.is_finite(CGAL::ARR_MIN_END) || ! xcv.is_finite(CGAL::ARR_MAX_END)) continue;
    check(traits, xcv, error, box, name);
  }
}

int main() {
  const Polynomial_2 x = CGAL::shift(Polynomial_2(1), 1, 0);
  const Polynomial_2 y = CGAL::shift(Polynomial_2(1), 1, 1);
  const CORE::BigInt e15 = CGAL::ipower(CORE::BigInt(10), 15);
  const CORE::BigInt e19 = CGAL::ipower(CORE::BigInt(10), 19);
  const CORE::BigInt e20 = CGAL::ipower(CORE::BigInt(10), 20);
  const CORE::BigInt e42 = CGAL::ipower(CORE::BigInt(10), 42);
  auto circle = [&](const CORE::BigInt& cx, const CORE::BigInt& r2) { return (x - cx) * (x - cx) + y * y - r2; };
  auto circle_over = [&](const CORE::BigInt& cx, const CORE::BigInt& r2, const CORE::BigInt& d) {
    return d * ((x - cx) * (x - cx) + y * y) - r2;
  };

  // arcs that are too narrow in x to sample
  check_curves({circle(0, 1)}, 1e-3, std::nullopt, "a unit circle");
  check_curves({e42 * (x - 1) * (x - 1) + e42 * y * y - 1}, 1e-25, std::nullopt, "a circle of radius 1e-21");
  check_curves({y - e20 * (x - 1), y - 50, y + 50}, 1e-3, std::nullopt, "a segment of slope 1e20");
  check_curves({circle_over(e15, 1, 400)}, 1e-3, std::nullopt, "a circle of radius 0.05 at 1e15");
  check_curves({circle(e19, 10000)}, 1e-3, std::nullopt, "a circle of radius 100 at 1e19");

  // boxes whose sides pass through ends of arcs
  check_curves({circle(0, 1)}, 1e-3, CGAL::Bbox_2(-2, -2, 2, 2), "a unit circle in a larger box");
  check_curves({circle(0, 1)}, 1e-3, CGAL::Bbox_2(-1, -2, 2, 2), "a unit circle, box side at x = -1");
  check_curves({circle(0, 1)}, 1e-3, CGAL::Bbox_2(-2, -2, 1, 2), "a unit circle, box side at x = 1");
  check_curves({circle(0, 1)}, 1e-3, CGAL::Bbox_2(-1, -1, 1, 1), "a unit circle in its bounding box");
  check_curves({x * y - 1}, 1e-3, CGAL::Bbox_2(0, -5, 5, 5), "x*y = 1, box side at its asymptote x = 0");
  check_curves({circle(0, 4)}, 1e-3, CGAL::Bbox_2(-3, -1, 3, 1),
               "a circle of radius 2 that crosses the bottom and top");
  for (int r2 = 2; r2 <= 30; ++r2) {
    Traits traits;
    std::vector<std::variant<Traits::Point_2, X_monotone_curve_2>> objs;
    traits.make_x_monotone_2_object()(traits.construct_curve_2_object()(circle(3, r2)), std::back_inserter(objs));
    for (const auto& obj : objs) {
      const auto* xcv = std::get_if<X_monotone_curve_2>(&obj);
      if (xcv == nullptr) continue;
      const double xmin = CGAL::to_double(xcv->curve_end_x(CGAL::ARR_MIN_END));
      const double xmax = CGAL::to_double(xcv->curve_end_x(CGAL::ARR_MAX_END));
      const std::string name = "a circle of squared radius " + std::to_string(r2) + ", box side at a rounded end";
      check(traits, *xcv, 1e-3, CGAL::Bbox_2(xmin, -100, 100, 100), name);
      check(traits, *xcv, 1e-3, CGAL::Bbox_2(-100, -100, xmax, 100), name);
    }
  }

  std::cout << failures << " failures" << std::endl;
  return (failures == 0) ? 0 : 1;
}

#endif
