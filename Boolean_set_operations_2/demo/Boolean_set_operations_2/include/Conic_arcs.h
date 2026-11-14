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

#ifndef CGAL_CONIC_ARCS_H
#define CGAL_CONIC_ARCS_H

/*! \file Conic_arcs.h
 * The exact description of the boundaries of conic polygons, used to write them (to files and to programs) and to
 * read them back. The endpoints of the arcs are algebraic points, which cannot be written exactly; instead, an endpoint
 * is written approximately together with another conic (an auxiliary conic) that passes through it. The endpoint is
 * the intersection of the supporting conic of the arc and the auxiliary conic nearest to the approximation (see
 * Arr_conic_traits_2::Construct_curve_2).
 */

#include <array>
#include <vector>

#include "Typedefs.h"

/*! A conic arc of a boundary: its supporting conic r*x^2 + s*y^2 + t*xy + u*x + v*y + w = 0, its orientation
 * (COLLINEAR for a line), and its endpoints, each one given approximately with an auxiliary conic. A full conic (an
 * ellipse) has no endpoints.
 */
struct Conic_arc_record {
  using Coefficients = std::array<Conic_integer, 6>;

  Coefficients m_conic;
  CGAL::Orientation m_orientation = CGAL::COLLINEAR;
  bool m_full = false;
  std::array<double, 2> m_source{0, 0};
  Coefficients m_source_conic;
  std::array<double, 2> m_target{0, 0};
  Coefficients m_target_conic;
};

/*! obtains the exact description of a boundary. Consecutive x-monotone arcs of the same conic and the same orientation
 * are merged into one arc, so that every endpoint is shared with an arc of another conic, which is its auxiliary
 * conic. A segment must be given by the equation of its line.
 * \throw std::invalid_argument if a segment is given by its endpoints only, or if two consecutive arcs have the same
 *        conic (and opposite orientations).
 */
std::vector<Conic_arc_record> conic_arc_records(const Conic_polygon& a_boundary);

/*! constructs the arc of a description.
 */
Conic_curve conic_curve(const Conic_arc_record& a_record);

/*! constructs the boundary of a description: the x-monotone arcs, in order.
 */
std::vector<Conic_X_monotone_curve> conic_boundary(const std::vector<Conic_arc_record>& a_records);

#endif
