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

#ifndef CGAL_POLYCURVE_BOUNDARY_H
#define CGAL_POLYCURVE_BOUNDARY_H

/*! \file Polycurve_boundary.h
 * The boundaries of polygons bounded by polycurves (of circle segments or of conic arcs). A boundary given by its
 * x-monotone subcurves is subdivided into maximal x-monotone polycurves, as a boundary of segments is subdivided into
 * maximal x-monotone polylines (see Polyline_boundary.h): where it turns from leftward to rightward or vice versa, and
 * where it starts or stops being vertical.
 */

#include <cstddef>
#include <vector>

#include "Polyline_boundary.h"

/*! obtains the direction of an x-monotone curve (a circle segment or a conic arc).
 */
template <typename X_monotone_curve>
Segment_direction curve_direction(const X_monotone_curve& a_xcv) {
  if (! a_xcv.is_vertical())
    return a_xcv.is_directed_right() ? Segment_direction::RIGHTWARD : Segment_direction::LEFTWARD;
  return (CGAL::compare(a_xcv.source().y(), a_xcv.target().y()) == CGAL::SMALLER) ? Segment_direction::UPWARD :
    Segment_direction::DOWNWARD;
}

/*! determines which x-monotone curves of an open chain start maximal x-monotone polycurves: the first one, and every
 * one whose direction differs from the direction of the previous one.
 * \return a flag per curve, true if its source is an endpoint of a polycurve.
 */
template <typename X_monotone_curve>
std::vector<bool> polycurve_starts(const std::vector<X_monotone_curve>& a_xcvs) {
  std::vector<bool> r_flags(a_xcvs.size(), true);
  for (std::size_t i = 1; i < a_xcvs.size(); ++i)
    r_flags[i] = curve_direction(a_xcvs[i - 1]) != curve_direction(a_xcvs[i]);
  return r_flags;
}

/*! subdivides a closed boundary, given by its x-monotone subcurves in order, into maximal x-monotone polycurves.
 * \return the polycurves in order along the boundary, or nothing if the boundary is empty.
 */
template <typename Polycurve_traits, typename X_monotone_curve>
std::vector<typename Polycurve_traits::X_monotone_curve_2>
polycurve_boundary(const std::vector<X_monotone_curve>& a_xcvs) {
  std::size_t n = a_xcvs.size();
  if (n == 0) return {};
  // Start where the direction changes; a closed boundary turns somewhere.
  std::size_t start = 0;
  for (std::size_t i = 0; i < n; ++i) {
    if (curve_direction(a_xcvs[(i + n - 1) % n]) != curve_direction(a_xcvs[i])) {
      start = i;
      break;
    }
  }
  Polycurve_traits traits;
  auto ctr_xcv = traits.construct_x_monotone_curve_2_object();
  std::vector<typename Polycurve_traits::X_monotone_curve_2> r_xcvs;
  std::vector<X_monotone_curve> run;
  for (std::size_t k = 0; k < n; ++k) {
    const auto& xcv = a_xcvs[(start + k) % n];
    if (! run.empty() && (curve_direction(run.back()) != curve_direction(xcv))) {
      r_xcvs.push_back(ctr_xcv(run.begin(), run.end()));
      run.clear();
    }
    run.push_back(xcv);
  }
  r_xcvs.push_back(ctr_xcv(run.begin(), run.end()));
  return r_xcvs;
}

/*! obtains the boundary of a polygon bounded by polycurves as a boundary of their subcurves, in order.
 */
template <typename Polygon, typename Polycurve_polygon>
Polygon flattened(const Polycurve_polygon& a_polygon) {
  std::vector<typename Polygon::X_monotone_curve_2> xcvs;
  for (auto cit = a_polygon.curves_begin(); cit != a_polygon.curves_end(); ++cit) {
    for (std::size_t i = 0; i < cit->number_of_subcurves(); ++i) xcvs.push_back((*cit)[i]);
  }
  return Polygon(xcvs.begin(), xcvs.end());
}

#endif
