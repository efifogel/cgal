// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Efi Fogel <efifogel@gmail.com>

#ifndef CGAL_BOUNDARY_VALIDITY_H
#define CGAL_BOUNDARY_VALIDITY_H

/*! \file Boundary_validity.h
 * Checks that keep the boundary under construction simple: a new curve may meet the boundary only at the vertex where
 * it starts (the end of the last curve), and, if it closes the boundary, at the first vertex.
 */

#include <cstddef>
#include <iterator>
#include <variant>
#include <vector>

#include <CGAL/intersections.h>

/*! determines whether the segment from a_source to a_target can extend a chain of segments, given by its vertices,
 * without making it self-intersecting: it may meet the last segment only at a_source (the last vertex), and, if it
 * closes the chain, the first segment only at a_target (the first vertex); it must not meet the other segments.
 */
template <typename Kernel>
bool is_valid_segment(const std::vector<typename Kernel::Point_2>& a_vertices, const typename Kernel::Point_2& a_source,
                      const typename Kernel::Point_2& a_target, bool a_closing) {
  using Point = typename Kernel::Point_2;
  using Segment = typename Kernel::Segment_2;
  if (a_source == a_target) return true;
  Segment edge(a_source, a_target);
  std::size_t n = (a_vertices.size() < 2) ? 0 : a_vertices.size() - 1;
  for (std::size_t i = 0; i < n; ++i) {
    if (a_vertices[i] == a_vertices[i + 1]) continue;
    auto res = CGAL::intersection(edge, Segment(a_vertices[i], a_vertices[i + 1]));
    if (! res) continue;
    const Point* p = std::get_if<Point>(&*res);
    // An overlap is never allowed.
    if (! p) return false;
    bool shared = ((i + 1 == n) && (*p == a_source)) || (a_closing && (i == 0) && (*p == a_target));
    if (! shared) return false;
  }
  return true;
}

/*! obtains the x-monotone curves of curves, in order.
 */
template <typename Traits, typename Input_iterator>
std::vector<typename Traits::X_monotone_curve_2> x_monotone_pieces(const Traits& a_traits, Input_iterator a_begin,
                                                                   Input_iterator a_end) {
  using Point = typename Traits::Point_2;
  using X_monotone_curve = typename Traits::X_monotone_curve_2;
  auto make_x_monotone = a_traits.make_x_monotone_2_object();
  std::vector<std::variant<Point, X_monotone_curve>> objects;
  for (auto it = a_begin; it != a_end; ++it) make_x_monotone(*it, std::back_inserter(objects));
  std::vector<X_monotone_curve> r_xcvs;
  for (const auto& object : objects) {
    if (const auto* xcv = std::get_if<X_monotone_curve>(&object)) r_xcvs.push_back(*xcv);
  }
  return r_xcvs;
}

/*! determines whether a curve, given by its x-monotone pieces, can extend a boundary, given by its x-monotone pieces,
 * without making it self-intersecting. The curve may meet the boundary only at a_start (its first point, the end of
 * the boundary), and, if a_closing_point is given, at a_closing_point (the first point of the boundary); at such a
 * point both pieces must end. The pieces of the curve may meet one another only where consecutive pieces meet. All
 * the curves must be constructed by a_traits (e.g., the x-monotone Bezier curves refer to the cache of the traits).
 */
template <typename Traits>
bool is_valid_extension(const Traits& a_traits, const std::vector<typename Traits::X_monotone_curve_2>& a_boundary,
                        const std::vector<typename Traits::X_monotone_curve_2>& a_curve,
                        const typename Traits::Point_2& a_start,
                        const typename Traits::Point_2* a_closing_point = nullptr) {
  using Point = typename Traits::Point_2;
  using X_monotone_curve = typename Traits::X_monotone_curve_2;
  using Multiplicity = typename Traits::Multiplicity;
  using Intersection = std::variant<std::pair<Point, Multiplicity>, X_monotone_curve>;
  auto intersect = a_traits.intersect_2_object();
  auto equal = a_traits.equal_2_object();
  auto min_vertex = a_traits.construct_min_vertex_2_object();
  auto max_vertex = a_traits.construct_max_vertex_2_object();
  auto is_endpoint = [&](const X_monotone_curve& a_xcv, const Point& a_p)
  { return equal(min_vertex(a_xcv), a_p) || equal(max_vertex(a_xcv), a_p); };
  // The points where two curves meet, or false if they overlap.
  auto meet = [&](const X_monotone_curve& a_xcv1, const X_monotone_curve& a_xcv2, std::vector<Point>& r_points) {
    std::vector<Intersection> objects;
    intersect(a_xcv1, a_xcv2, std::back_inserter(objects));
    r_points.clear();
    for (const auto& object : objects) {
      const auto* p = std::get_if<std::pair<Point, Multiplicity>>(&object);
      if (! p) return false;
      r_points.push_back(p->first);
    }
    return true;
  };

  std::vector<Point> points;
  for (const auto& c : a_curve) {
    for (const auto& b : a_boundary) {
      if (! meet(c, b, points)) return false;
      for (const auto& p : points) {
        bool shared = (equal(p, a_start) || (a_closing_point && equal(p, *a_closing_point))) &&
          is_endpoint(c, p) && is_endpoint(b, p);
        if (! shared) return false;
      }
    }
  }
  for (std::size_t i = 0; i < a_curve.size(); ++i) {
    for (std::size_t j = i + 1; j < a_curve.size(); ++j) {
      if (! meet(a_curve[i], a_curve[j], points)) return false;
      for (const auto& p : points) {
        if ((j != i + 1) || ! is_endpoint(a_curve[i], p) || ! is_endpoint(a_curve[j], p)) return false;
      }
    }
  }
  return true;
}

#endif
