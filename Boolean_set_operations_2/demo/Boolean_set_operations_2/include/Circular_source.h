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

#ifndef CGAL_CIRCULAR_SOURCE_H
#define CGAL_CIRCULAR_SOURCE_H

/*! \file Circular_source.h
 * The data that the boundary of a polygon bounded by segments and circular arcs is constructed from. The points are
 * the points the user clicked at, so the boundary can be written exactly (e.g., as a program), unlike the arcs
 * themselves, whose endpoints may be irrational.
 */

#include <array>
#include <vector>

#include "Typedefs.h"

/*! A piece of a boundary: a segment (its source and target), an arc through three points (its source, a point in its
 * interior, and its target), or a full circle (its center and a point on it).
 */
struct Circular_piece_source {
  enum class Kind { SEGMENT, ARC, CIRCLE };

  Kind m_kind = Kind::SEGMENT;
  std::array<Kernel::Point_2, 3> m_points;
};

/*! The pieces of a boundary, in order.
 */
using Circular_boundary_source = std::vector<Circular_piece_source>;

/*! reverses a boundary: the pieces are given in the opposite order, each one in the opposite direction.
 */
inline Circular_boundary_source reversed(const Circular_boundary_source& a_source) {
  Circular_boundary_source r_source(a_source.rbegin(), a_source.rend());
  for (auto& piece : r_source) {
    if (piece.m_kind == Circular_piece_source::Kind::SEGMENT) std::swap(piece.m_points[0], piece.m_points[1]);
    else if (piece.m_kind == Circular_piece_source::Kind::ARC) std::swap(piece.m_points[0], piece.m_points[2]);
  }
  return r_source;
}

#endif
