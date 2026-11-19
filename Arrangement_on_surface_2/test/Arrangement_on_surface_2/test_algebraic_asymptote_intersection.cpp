// Copyright (c) 2026 Tel-Aviv University (Israel).
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Tests the intersections of arcs of the algebraic segment traits at the ends of their joint x-range, where an arc
// approaches a vertical asymptote. The curves have a vertical asymptote and also a finite point on the asymptote line
// (e.g., x*y^2 - y - x has the asymptote x = 0 and passes through (0,0)), so the curve pair has an intersection event
// there, which must not be attributed to the asymptotic arcs. For every pair of curves, the intersection points of all
// pairs of their x-monotone arcs must lie on both arcs and must be exactly the intersection points of the curves, as
// obtained from the curve kernel.

#include <CGAL/config.h>
#include <iostream>

#if ! defined(CGAL_USE_CORE)

int main() {
  std::cout << "This test requires CORE; it is skipped." << std::endl;
  return 0;
}

#else

#include <iterator>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/CORE_BigInt.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>

using Traits = CGAL::Arr_algebraic_segment_traits_2<CORE::BigInt>;
using Polynomial_2 = Traits::Polynomial_2;
using Curve_2 = Traits::Curve_2;
using X_monotone_curve_2 = Traits::X_monotone_curve_2;
using Point_2 = Traits::Point_2;
using Curve_kernel_2 = Traits::CKvA_2::Curve_kernel_2;
using Coordinate_2 = Curve_kernel_2::Coordinate_2;
using Multiplicity = Curve_kernel_2::Multiplicity_type;

int main() {
  Traits traits;
  const auto& kernel = traits.curved_kernel()->kernel();
  auto construct_curve = traits.construct_curve_2_object();
  auto make_x_monotone = traits.make_x_monotone_2_object();
  auto intersect = traits.intersect_2_object();
  auto is_on = traits.is_on_2_object();
  auto solve = kernel.solve_2_object();
  auto compare_xy = kernel.compare_xy_2_object();

  Polynomial_2 x = CGAL::shift(Polynomial_2(1), 1, 0);
  Polynomial_2 y = CGAL::shift(Polynomial_2(1), 1, 1);
  // curves with a vertical asymptote and a finite point on the asymptote line
  const std::vector<Polynomial_2> fs = {x * y * y - y - x,                      // x = 0, through (0,0)
                                        (x - 1) * y * y - y - (x - 1)};         // x = 1, through (1,0)
  // curves through those finite points or not, and a curve with the same asymptote
  const std::vector<Polynomial_2> gs = {y - x, y + x, y, y - x + 1, y - 2 * x + 2, x * y - 1, (x - 1) * y - 1};

  auto arcs = [&](const Polynomial_2& p) {
    std::vector<std::variant<Point_2, X_monotone_curve_2>> objs;
    make_x_monotone(construct_curve(p), std::back_inserter(objs));
    std::vector<X_monotone_curve_2> res;
    for (const auto& obj : objs)
      if (const auto* xcv = std::get_if<X_monotone_curve_2>(&obj)) res.push_back(*xcv);
    return res;
  };

  int failures = 0;
  int points = 0;
  for (const auto& f : fs) {
    const auto arcs_f = arcs(f);
    for (const auto& g : gs) {
      const auto arcs_g = arcs(g);

      // the intersection points of the curves
      std::vector<std::pair<Coordinate_2, Multiplicity>> expected;
      solve(construct_curve(f), construct_curve(g), std::back_inserter(expected));
      std::vector<bool> found(expected.size(), false);

      for (const auto& xcv_f : arcs_f) {
        for (const auto& xcv_g : arcs_g) {
          using Intersection_point = std::pair<Point_2, Traits::Multiplicity>;
          std::vector<std::variant<Intersection_point, X_monotone_curve_2>> res;
          intersect(xcv_f, xcv_g, std::back_inserter(res));
          for (const auto& item : res) {
            const auto* ip = std::get_if<Intersection_point>(&item);
            if (ip == nullptr) {
              std::cerr << "Error: arcs of " << f << " and " << g << " overlap" << std::endl;
              ++failures;
              continue;
            }
            ++points;
            const Point_2& p = ip->first;
            if (! is_on(p, xcv_f) || ! is_on(p, xcv_g)) {
              std::cerr << "Error: an intersection point of arcs of " << f << " and " << g
                        << " does not lie on both arcs" << std::endl;
              ++failures;
            }
            bool known = false;
            for (std::size_t i = 0; i < expected.size(); ++i) {
              if (compare_xy(p.xy(), expected[i].first) != CGAL::EQUAL) continue;
              found[i] = known = true;
              break;
            }
            if (! known) {
              std::cerr << "Error: an intersection point of arcs of " << f << " and " << g
                        << " is not an intersection point of the curves" << std::endl;
              ++failures;
            }
          }
        }
      }
      for (std::size_t i = 0; i < expected.size(); ++i) {
        if (found[i]) continue;
        std::cerr << "Error: an intersection point of " << f << " and " << g << " is not found" << std::endl;
        ++failures;
      }
    }
  }

  std::cout << points << " intersection points of arcs checked, " << failures << " failures" << std::endl;
  return (failures == 0) ? 0 : 1;
}

#endif
