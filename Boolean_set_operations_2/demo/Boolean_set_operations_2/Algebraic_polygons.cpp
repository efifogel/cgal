// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s) : Efi Fogel <efifogel@gmail.com>

/*! \file Algebraic_polygons.cpp
 * The polygons bounded by algebraic curves, and the approximation of their arcs for drawing.
 */

#include "Algebraic_polygons.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <utility>

#include <CGAL/Arrangement_2.h>

std::vector<Algebraic_polygon> algebraic_polygons(const Bivariate_polynomial& a_polynomial) {
  // A constant polynomial has no curve.
  bool constant = true;
  for (const auto& [monomial, c] : a_polynomial) {
    if ((monomial.first > 0) || (monomial.second > 0)) constant = false;
  }
  if (constant) return {};

  // The arrangement of the curve, which may be unbounded.
  using Arrangement = CGAL::Arrangement_2<Algebraic_curve_traits>;
  Algebraic_curve_traits curve_traits;
  Arrangement arr(&curve_traits);
  CGAL::insert(arr, algebraic_curve(a_polynomial, curve_traits));

  Algebraic_traits traits;
  auto opposite = traits.construct_opposite_2_object();
  std::vector<Algebraic_polygon> r_polygons;
  for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
    if (fit->is_fictitious() || fit->is_unbounded()) continue;
    // The outer boundary of a bounded face is counterclockwise; arcs that bound the face on both sides are skipped.
    Algebraic_polygon polygon;
    auto first = fit->outer_ccb();
    auto he = first;
    do {
      if (he->twin()->face() == fit) continue;
      if (he->direction() == CGAL::ARR_LEFT_TO_RIGHT) polygon.push_back(he->curve());
      else polygon.push_back(opposite(he->curve()));
    } while (++he != first);
    if (! polygon.is_empty()) r_polygons.push_back(std::move(polygon));
  }
  return r_polygons;
}

std::vector<QPointF> algebraic_arc_points(const Algebraic_X_monotone_curve& a_xcv) {
  // The points of an arc are computed once, and cached by the representation of the arc, which the cache keeps alive
  // (so that its address is not reused by another arc).
  static std::unordered_map<std::size_t, std::pair<Algebraic_X_monotone_curve, std::vector<QPointF>>> s_cache;
  auto it = s_cache.find(a_xcv.id());
  if (it != s_cache.end()) return it->second.second;
  if (s_cache.size() > 10000) s_cache.clear();

  auto to_point = [](const std::pair<double, double>& a_p) { return QPointF(a_p.first, a_p.second); };
  QPointF min = to_point(a_xcv.curve_end(CGAL::ARR_MIN_END).to_double());
  QPointF max = to_point(a_xcv.curve_end(CGAL::ARR_MAX_END).to_double());
  std::vector<QPointF> points;
  points.push_back(min);
  if (! a_xcv.is_vertical()) {
    // The x-coordinates are denser near the endpoints, where the arc may be steep (e.g., uniform in the angle along a
    // half circle).
    using Bound = Algebraic_traits::Bound;
    using Coordinate_1 = Algebraic_traits::Algebraic_real_1;
    auto construct_point = Algebraic_traits::CKvA_2::instance().construct_point_2_object();
    const int n = 64;
    double mid = (min.x() + max.x()) / 2, half = (max.x() - min.x()) / 2;
    for (int i = 1; i < n; ++i) {
      double x = mid - half * std::cos(M_PI * i / n);
      if ((x <= min.x()) || (x >= max.x())) continue;
      points.push_back(to_point(construct_point(Coordinate_1{Bound(x)}, a_xcv.curve(), a_xcv.arcno()).to_double()));
    }
  }
  points.push_back(max);
  if (Algebraic_traits().compare_endpoints_xy_2_object()(a_xcv) == CGAL::LARGER)
    std::reverse(points.begin(), points.end());
  s_cache.emplace(a_xcv.id(), std::make_pair(a_xcv, points));
  return points;
}

CGAL::Bbox_2 algebraic_arc_bbox(const Algebraic_X_monotone_curve& a_xcv) { return a_xcv.bbox(); }
