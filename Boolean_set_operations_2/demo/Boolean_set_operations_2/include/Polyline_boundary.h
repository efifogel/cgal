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

#ifndef CGAL_POLYLINE_BOUNDARY_H
#define CGAL_POLYLINE_BOUNDARY_H

/*! \file Polyline_boundary.h
 * The boundaries of polygons bounded by polylines. A boundary given by its vertices is subdivided into maximal
 * x-monotone polylines: the endpoints of the polylines are the vertices of the polygon, and the other vertices are
 * vertices of the polylines. A boundary is subdivided where it turns from leftward to rightward or vice versa, and
 * where it starts or stops being vertical (a vertical segment is a polyline of its own).
 */

#include <algorithm>
#include <vector>

#include "Typedefs.h"

/*! The direction of a segment, which determines the x-monotone polylines that the segment can be a part of.
 */
enum class Segment_direction { LEFTWARD, RIGHTWARD, DOWNWARD, UPWARD };

/*! obtains the direction of the segment from a_p to a_q (distinct points).
 */
template <typename Point>
Segment_direction segment_direction(const Point& a_p, const Point& a_q) {
  if (a_p.x() < a_q.x()) return Segment_direction::RIGHTWARD;
  if (a_q.x() < a_p.x()) return Segment_direction::LEFTWARD;
  return (a_p.y() < a_q.y()) ? Segment_direction::UPWARD : Segment_direction::DOWNWARD;
}

/*! determines which vertices of an open chain end the maximal x-monotone polylines that the chain consists of. The
 * first and the last vertices end polylines. Consecutive vertices must be distinct.
 * \return a flag per vertex, true for an endpoint of a polyline.
 */
template <typename Point>
std::vector<bool> polyline_endpoints(const std::vector<Point>& a_chain) {
  std::vector<bool> r_flags(a_chain.size(), true);
  for (std::size_t i = 1; i + 1 < a_chain.size(); ++i)
    r_flags[i] = segment_direction(a_chain[i - 1], a_chain[i]) != segment_direction(a_chain[i], a_chain[i + 1]);
  return r_flags;
}

/*! subdivides a closed boundary, given by its vertices in order, into maximal x-monotone polylines. Consecutive equal
 * vertices (including the last and the first) count once.
 * \return the polylines in order along the boundary, or nothing if the boundary has fewer than three vertices.
 */
inline std::vector<Polyline_X_monotone_curve> polyline_boundary(const std::vector<Kernel::Point_2>& a_vertices) {
  std::vector<Kernel::Point_2> vertices;
  for (const auto& p : a_vertices) {
    if (vertices.empty() || (p != vertices.back())) vertices.push_back(p);
  }
  while ((vertices.size() > 1) && (vertices.back() == vertices.front())) vertices.pop_back();
  if (vertices.size() < 3) return {};

  // The lexicographically smallest vertex ends two polylines (the boundary turns there), so the chain starts there.
  std::rotate(vertices.begin(), std::min_element(vertices.begin(), vertices.end()), vertices.end());
  vertices.push_back(vertices.front());
  auto ends = polyline_endpoints(vertices);

  Polyline_traits traits;
  auto ctr_xcv = traits.construct_x_monotone_curve_2_object();
  std::vector<Polyline_X_monotone_curve> r_xcvs;
  std::size_t first = 0;
  for (std::size_t i = 1; i < vertices.size(); ++i) {
    if (! ends[i]) continue;
    r_xcvs.push_back(ctr_xcv(vertices.begin() + first, vertices.begin() + i + 1));
    first = i;
  }
  return r_xcvs;
}

#endif
