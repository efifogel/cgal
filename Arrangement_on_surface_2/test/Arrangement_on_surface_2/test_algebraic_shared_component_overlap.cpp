// Copyright (c) 2026 Tel-Aviv University (Israel).
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Tests the intersection of arcs of the algebraic segment traits whose supporting curves share a component, e.g.,
// an arc of the line y - x and an arc of (y - x)(x^2 + y^2 - 4) on that line. Such arcs overlap. When one supporting
// curve is entirely the common component, Intersect_2 used to miss the overlap and then failed the precondition that
// the arcs do not overlap (unless Do_overlap_2 had been applied to the same arcs before, which simplifies their
// supporting curves). Every pair of arcs is intersected in both orders, each time with arcs that are constructed anew;
// the result must be an overlap exactly when Do_overlap_2 reports one, and the intersection points must lie on both
// arcs. Finally, the operations must not modify the arcs: the supporting curve of every arc must remain the curve that
// the arc was constructed from (the simplification of supporting curves that share a component is kept apart).

#include <CGAL/config.h>
#include <iostream>

#if ! defined(CGAL_USE_CORE)

int main() {
  std::cout << "This test requires CORE; it is skipped." << std::endl;
  return 0;
}

#else

#include <iterator>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/CORE_BigInt.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>

using Traits = CGAL::Arr_algebraic_segment_traits_2<CORE::BigInt>;
using Polynomial_2 = Traits::Polynomial_2;
using X_monotone_curve_2 = Traits::X_monotone_curve_2;
using Point_2 = Traits::Point_2;
using Intersection_point = std::pair<Point_2, Traits::Multiplicity>;
using Intersection_result = std::variant<Intersection_point, X_monotone_curve_2>;

int main() {
  Traits traits;
  auto construct_curve = traits.construct_curve_2_object();
  auto make_x_monotone = traits.make_x_monotone_2_object();
  auto intersect = traits.intersect_2_object();
  auto do_overlap = traits.curved_kernel()->do_overlap_2_object();
  auto is_on = traits.is_on_2_object();

  // obtains new x-monotone arcs of the curve of a polynomial
  auto arcs = [&](const Polynomial_2& p) {
    std::vector<std::variant<Point_2, X_monotone_curve_2>> objs;
    make_x_monotone(construct_curve(p), std::back_inserter(objs));
    std::vector<X_monotone_curve_2> res;
    for (const auto& obj : objs)
      if (const auto* xcv = std::get_if<X_monotone_curve_2>(&obj)) res.push_back(*xcv);
    return res;
  };

  Polynomial_2 x = CGAL::shift(Polynomial_2(1), 1, 0);
  Polynomial_2 y = CGAL::shift(Polynomial_2(1), 1, 1);
  struct Case { std::string name; Polynomial_2 g, f; int overlaps; };
  const std::vector<Case> cases = {
    // the first curve is entirely the common component
    {"y-x and (y-x)(x^2+y^2-4)", y - x, (y - x) * (x * x + y * y - 4), 5},
    {"y and y(x^2+y^2-1)", y, y * (x * x + y * y - 1), 3},
    // both curves have components of their own
    // (the parabola is one arc of the first curve, and three arcs of the second, split where y = 1 meets it)
    {"(y-x^2)(y+1) and (y-x^2)(y-1)", (y - x * x) * (y + 1), (y - x * x) * (y - 1), 3},
    {"(x^2+y^2-1)(y-2) and (x^2+y^2-1)(y+2)", (x * x + y * y - 1) * (y - 2), (x * x + y * y - 1) * (y + 2), 2}};

  bool ok = true;
  for (const auto& tc : cases) {
    const std::size_t ng = arcs(tc.g).size();
    const std::size_t nf = arcs(tc.f).size();
    for (int order = 0; order < 2; ++order) {
      int overlaps = 0;
      for (std::size_t i = 0; i < ng; ++i) {
        for (std::size_t j = 0; j < nf; ++j) {
          const auto ag = arcs(tc.g);
          const auto af = arcs(tc.f);
          const X_monotone_curve_2& cv1 = (order == 0) ? ag[i] : af[j];
          const X_monotone_curve_2& cv2 = (order == 0) ? af[j] : ag[i];
          std::vector<Intersection_result> res;
          try {
            intersect(cv1, cv2, std::back_inserter(res));
          }
          catch (const CGAL::Failure_exception& e) {
            std::cerr << "Error: " << tc.name << ", arcs " << i << " and " << j << " (order " << order
                      << "): " << e.what() << std::endl;
            ok = false;
            continue;
          }

          // the expected result, on new arcs
          const auto bg = arcs(tc.g);
          const auto bf = arcs(tc.f);
          const bool expected_overlap = do_overlap(bg[i], bf[j]);
          int found = 0;
          for (const auto& item : res) {
            if (std::holds_alternative<X_monotone_curve_2>(item)) {
              ++found;
              continue;
            }
            const Point_2& p = std::get<Intersection_point>(item).first;
            const auto cg = arcs(tc.g);
            const auto cf = arcs(tc.f);
            if (is_on(p, cg[i]) && is_on(p, cf[j])) continue;
            std::cerr << "Error: " << tc.name << ", arcs " << i << " and " << j
                      << ": an intersection point does not lie on both arcs" << std::endl;
            ok = false;
          }
          if (found != (expected_overlap ? 1 : 0)) {
            std::cerr << "Error: " << tc.name << ", arcs " << i << " and " << j << " (order " << order << "): "
                      << found << " overlaps, expected " << (expected_overlap ? 1 : 0) << std::endl;
            ok = false;
          }
          overlaps += found;
        }
      }
      if (overlaps != tc.overlaps) {
        std::cerr << "Error: " << tc.name << " (order " << order << "): " << overlaps << " overlaps, expected "
                  << tc.overlaps << std::endl;
        ok = false;
      }
    }
  }

  // the operations must not modify the arcs (or their copies)
  for (const auto& tc : cases) {
    const Traits::Curve_2 cg = construct_curve(tc.g);
    const Traits::Curve_2 cf = construct_curve(tc.f);
    const auto ag = arcs(tc.g);
    const auto af = arcs(tc.f);
    const std::vector<X_monotone_curve_2> copies_g(ag);
    const std::vector<X_monotone_curve_2> copies_f(af);
    auto equal = traits.equal_2_object();
    for (const auto& xg : ag) {
      for (const auto& xf : af) {
        std::vector<Intersection_result> res;
        intersect(xg, xf, std::back_inserter(res));
        do_overlap(xf, xg);
        equal(xg, xf);
      }
    }
    for (std::size_t i = 0; i < ag.size(); ++i) {
      if (ag[i].curve().is_identical(cg) && copies_g[i].curve().is_identical(cg)) continue;
      std::cerr << "Error: " << tc.name << ": an arc of the first curve changed its supporting curve" << std::endl;
      ok = false;
    }
    for (std::size_t j = 0; j < af.size(); ++j) {
      if (af[j].curve().is_identical(cf) && copies_f[j].curve().is_identical(cf)) continue;
      std::cerr << "Error: " << tc.name << ": an arc of the second curve changed its supporting curve" << std::endl;
      ok = false;
    }
  }

  std::cout << (ok ? "passed" : "failed") << std::endl;
  return ok ? 0 : 1;
}

#endif
