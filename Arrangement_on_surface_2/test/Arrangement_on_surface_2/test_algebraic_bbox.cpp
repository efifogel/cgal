// Copyright (c) 2026 Tel-Aviv University (Israel).
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Tests that the bounding box of an arc of the algebraic segment traits contains the arc: its finite ends and
// points in the interior of its x-range, all compared exactly. The curves have irrational coordinates of various
// magnitudes, and some have horizontal-line components (whose polynomials share a factor with their x-derivatives).

#include <CGAL/config.h>
#include <iostream>

#if ! defined(CGAL_USE_CORE)

int main() {
  std::cout << "This test requires CORE; it is skipped." << std::endl;
  return 0;
}

#else

#include <cmath>
#include <iterator>
#include <limits>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/CORE_BigInt.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>

using Traits = CGAL::Arr_algebraic_segment_traits_2<CORE::BigInt>;
using Polynomial_2 = Traits::Polynomial_2;
using X_monotone_curve_2 = Traits::X_monotone_curve_2;
using Point_2 = Traits::Point_2;
using Bound = Traits::Bound;
using Algebraic_real_1 = Traits::Algebraic_real_1;

/*! checks whether an interval lies in [lo, hi], where lo and hi may be infinite.
 */
bool is_in(const std::pair<Bound, Bound>& iv, double lo, double hi) {
  constexpr double inf = std::numeric_limits<double>::infinity();
  return ((lo == -inf) || (Bound(lo) <= iv.first)) && ((hi == inf) || (iv.second <= Bound(hi)));
}

int main() {
  Traits traits;
  auto& kernel = traits.curved_kernel()->kernel();
  auto approximate_x = kernel.approximate_absolute_1_object();
  auto approximate_y = kernel.approximate_absolute_y_2_object();
  auto construct_bbox = traits.construct_bbox_2_object();
  auto construct_point = traits.construct_point_2_object();
  auto make_x_monotone = traits.make_x_monotone_2_object();

  Polynomial_2 x = CGAL::shift(Polynomial_2(1), 1, 0);
  Polynomial_2 y = CGAL::shift(Polynomial_2(1), 1, 1);
  const std::vector<Polynomial_2> polynomials = {
    x * x + y * y - 2, 3 * y - x * x * x + 7 * x, x * y - 3, y * (x * x + 1) - 1, y * y - x * x * x + x,
    CGAL::ipower(x, 4) + CGAL::ipower(y, 3) - 5, 1000 * x * x + y * y - 3000000, x * x + 1000000 * y * y - 3,
    7 * y * y - x * x * x - 2,
    // horizontal-line components
    x * y, y * (x * x + y * y - 2), (y - 1) * (y - x * x), y * y - 2};

  const long precision = 300;   // to compare the coordinates with the bounds
  const int samples = 20;       // interior points per arc
  int arcs = 0;
  int checks = 0;
  int failures = 0;
  for (const auto& pol : polynomials) {
    std::vector<std::variant<Point_2, X_monotone_curve_2>> objs;
    make_x_monotone(traits.construct_curve_2_object()(pol), std::back_inserter(objs));
    for (const auto& obj : objs) {
      const auto* xcv = std::get_if<X_monotone_curve_2>(&obj);
      if (xcv == nullptr) continue;
      ++arcs;
      const CGAL::Bbox_2 bbox = construct_bbox(*xcv);

      std::vector<Point_2> points;
      for (auto ce : {CGAL::ARR_MIN_END, CGAL::ARR_MAX_END})
        if (xcv->location(ce) == CGAL::ARR_INTERIOR) points.push_back(xcv->curve_end(ce));
      // the ends of an arc whose bounding box is bounded in x have x-coordinates
      if (! xcv->is_vertical() && std::isfinite(bbox.xmin()) && std::isfinite(bbox.xmax())) {
        const Algebraic_real_1 xmin = xcv->curve_end_x(CGAL::ARR_MIN_END);
        const Algebraic_real_1 xmax = xcv->curve_end_x(CGAL::ARR_MAX_END);
        for (int i = 1; i < samples; ++i) {
          const Bound xs = Bound(bbox.xmin()) + (Bound(bbox.xmax()) - Bound(bbox.xmin())) * Bound(i) / Bound(samples);
          const Algebraic_real_1 xr(xs);
          if ((xmin < xr) && (xr < xmax)) points.push_back(construct_point(xr, *xcv));
        }
      }

      for (const auto& p : points) {
        ++checks;
        if (is_in(approximate_x(p.x(), precision), bbox.xmin(), bbox.xmax()) &&
            is_in(approximate_y(p.xy(), precision), bbox.ymin(), bbox.ymax()))
          continue;
        ++failures;
        const auto pd = p.to_double();
        std::cerr << "Error: the point (" << pd.first << ", " << pd.second << ") of an arc of " << pol
                  << " lies outside its bounding box " << bbox << std::endl;
      }
    }
  }

  std::cout << arcs << " arcs, " << checks << " points checked, " << failures << " failures" << std::endl;
  return (failures == 0) ? 0 : 1;
}

#endif
