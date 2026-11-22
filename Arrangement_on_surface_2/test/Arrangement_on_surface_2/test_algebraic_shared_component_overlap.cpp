// Copyright (c) 2026 Tel-Aviv University (Israel).
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Tests that the operations on arcs of the algebraic segment traits whose supporting curves share a component, e.g.,
// an arc of the line y - x and an arc of (y - x)(x^2 + y^2 - 4) on that line, do not modify the arcs: the supporting
// curve of every arc (and of its copies) must remain the curve that the arc was constructed from. (The operations
// simplify the supporting curves of such arcs; the simplified arcs are kept apart.) The results of the intersections
// of such arcs are tested by the generic traits test (data/algebraic/intersect).

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
  auto equal = traits.equal_2_object();

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
  struct Case { std::string name; Polynomial_2 g, f; };
  const std::vector<Case> cases = {
    // the first curve is entirely the common component
    {"y-x and (y-x)(x^2+y^2-4)", y - x, (y - x) * (x * x + y * y - 4)},
    {"y and y(x^2+y^2-1)", y, y * (x * x + y * y - 1)},
    // both curves have components of their own
    {"(y-x^2)(y+1) and (y-x^2)(y-1)", (y - x * x) * (y + 1), (y - x * x) * (y - 1)},
    {"(x^2+y^2-1)(y-2) and (x^2+y^2-1)(y+2)", (x * x + y * y - 1) * (y - 2), (x * x + y * y - 1) * (y + 2)}};

  // the operations must not modify the arcs (or their copies)
  bool ok = true;
  for (const auto& tc : cases) {
    const Traits::Curve_2 cg = construct_curve(tc.g);
    const Traits::Curve_2 cf = construct_curve(tc.f);
    const auto ag = arcs(tc.g);
    const auto af = arcs(tc.f);
    const std::vector<X_monotone_curve_2> copies_g(ag);
    const std::vector<X_monotone_curve_2> copies_f(af);
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
