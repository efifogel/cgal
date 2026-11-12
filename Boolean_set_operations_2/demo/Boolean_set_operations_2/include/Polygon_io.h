// Copyright (c) 2012  Tel-Aviv University (Israel).
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
//             Kabir Kedia <kabirkedia0111@gmail.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>
//             Apurva Bhatt <response2apurva@gmail.com>

#ifndef CGAL_POLYGON_IO_H
#define CGAL_POLYGON_IO_H

/*! \file Polygon_io.h
 * Reading and writing the polygon files of the curve families (.lps, .cps, .mps, and .bps). A file holds polygons with
 * holes, each one given by its boundaries; a function of each family reads or writes a boundary.
 */

#include <fstream>
#include <istream>
#include <ostream>
#include <utility>
#include <variant>
#include <vector>

#include <QString>

#include "Typedefs.h"

/*! obtains the x-monotone curves of given curves.
 */
template <typename Traits, typename InputIterator>
std::vector<typename Traits::X_monotone_curve_2> x_monotone_curves(InputIterator aBegin, InputIterator aEnd) {
  using Point = typename Traits::Point_2;
  using X_monotone_curve = typename Traits::X_monotone_curve_2;
  Traits traits;
  auto make_x_monotone = traits.make_x_monotone_2_object();
  std::vector<std::variant<Point, X_monotone_curve>> objects;
  for (auto it = aBegin; it != aEnd; ++it) make_x_monotone(*it, std::back_inserter(objects));
  std::vector<X_monotone_curve> xcvs;
  for (const auto& object : objects) {
    if (const auto* xcv = std::get_if<X_monotone_curve>(&object)) xcvs.push_back(*xcv);
  }
  return xcvs;
}

/*! reads the vertices of a boundary: their number followed by their coordinates.
 */
std::vector<Kernel::Point_2> read_vertices(std::istream& aIs);

/*! reads the boundary of a linear polygon: its vertices. \return the x-monotone curves of the boundary.
 */
std::vector<Linear_X_monotone_curve> read_linear_boundary(std::istream& aIs, bool /* aDoubleFormat */);

/*! reads the boundary of a polygon bounded by polylines: its vertices. \return the x-monotone curves of the boundary.
 */
std::vector<Polyline_X_monotone_curve> read_polyline_boundary(std::istream& aIs, bool /* aDoubleFormat */);

/*! reads the boundary of a polygon bounded by segments and circular arcs: the number of its curves followed by the
 * curves, each given by a flag (1 for an arc) followed by its source, a point in its interior (for an arc), and its
 * target. \return the x-monotone curves of the boundary.
 */
std::vector<Circular_X_monotone_curve> read_circular_boundary(std::istream& aIs, bool /* aDoubleFormat */);

/*! reads a Bezier curve: the number of its control points followed by their coordinates, as doubles (rounded to
 * 1/1000) or as rationals. Repeated control points and control points collinear with their neighbors are dropped.
 */
Bezier_curve read_bezier_curve(std::istream& is, bool aDoubleFormat);

/*! reads the boundary of a Bezier polygon: the number of its curves followed by the curves. \return the x-monotone
 * curves of the boundary.
 */
std::vector<Bezier_X_monotone_curve> read_bezier_boundary(std::istream& aIs, bool aDoubleFormat);

/*! saves a polygon set to a file: a format line, the number of polygons with holes, and for each one the number of its
 * boundaries followed by the boundaries (the outer one first), each written by a given function. \return false if the
 * file cannot be written.
 */
template <typename Set, typename Write_boundary>
bool save_polygons(const QString& aFileName, const Set& aSet, Write_boundary aWriteBoundary) {
  std::ofstream out_file(qPrintable(aFileName));
  if (! out_file) return false;
  std::vector<typename Set::Polygon_with_holes_2> pwhs;
  aSet.polygons_with_holes(std::back_inserter(pwhs));
  out_file << "DOUBLE" << std::endl << pwhs.size() << std::endl;
  for (const auto& pwh : pwhs) {
    out_file << " " << (1 + pwh.number_of_holes()) << std::endl;
    aWriteBoundary(out_file, pwh.outer_boundary());
    for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) aWriteBoundary(out_file, *hit);
  }
  return bool(out_file);
}

/*! writes the boundary of a linear polygon: the number of its vertices followed by their coordinates.
 */
void write_linear_boundary(std::ostream& aOs, const Linear_polygon& aBoundary);

/*! writes the boundary of a polygon bounded by polylines: the number of its curves followed by the coordinates of their
 * first points.
 */
void write_polyline_boundary(std::ostream& aOs, const Polyline_polygon& aBoundary);

/*! obtains the midpoint of a circular arc.
 */
std::pair<double, double> arc_midpoint(const Circular_X_monotone_curve& aArc);

/*! writes the boundary of a polygon bounded by segments and circular arcs: the number of its curves followed by the
 * curves, each given by a flag (1 for an arc) followed by its source, its midpoint (for an arc), and its target.
 */
void write_circular_boundary(std::ostream& aOs, const Circular_polygon& aBoundary);

/*! writes the boundary of a Bezier polygon: the number of its curves followed by their control points.
 */
void write_bezier_boundary(std::ostream& out_file, const Bezier_polygon& aBP);

#endif
