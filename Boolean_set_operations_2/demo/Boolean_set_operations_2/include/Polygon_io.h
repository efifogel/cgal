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
 * Reading and writing the polygon files of the curve families (.lps, .cps, .plps, .bps, and .qps). A file holds
 * polygons with holes, each one given by its boundaries; a function of each family reads or writes a boundary.
 *
 * The suffix of a file is the code of its curves followed by "ps" (polygon set). The code of a basic family is a
 * letter: l (segments), c (circle-segments), q (conics), r (rational function arcs), b (Bezier curves), and a
 * (algebraic curves). The code of a polycurve is p followed by the code of its subcurves; e.g., polylines are .plps,
 * polycurves of Bezier curves are .pbps, and polycurves of polylines are .pplps.
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
template <typename Traits, typename Input_iterator>
std::vector<typename Traits::X_monotone_curve_2> x_monotone_curves(Input_iterator a_begin, Input_iterator a_end) {
  using Point = typename Traits::Point_2;
  using X_monotone_curve = typename Traits::X_monotone_curve_2;
  Traits traits;
  auto make_x_monotone = traits.make_x_monotone_2_object();
  std::vector<std::variant<Point, X_monotone_curve>> objects;
  for (auto it = a_begin; it != a_end; ++it) make_x_monotone(*it, std::back_inserter(objects));
  std::vector<X_monotone_curve> xcvs;
  for (const auto& object : objects) {
    if (const auto* xcv = std::get_if<X_monotone_curve>(&object)) xcvs.push_back(*xcv);
  }
  return xcvs;
}

/*! reads the vertices of a boundary: their number followed by their coordinates.
 */
std::vector<Kernel::Point_2> read_vertices(std::istream& a_is);

/*! reads the boundary of a linear polygon: its vertices. \return the x-monotone curves of the boundary.
 */
std::vector<Linear_X_monotone_curve> read_linear_boundary(std::istream& a_is, bool /* a_double_format */);

/*! reads the boundary of a polygon bounded by polylines: its vertices. The boundary is subdivided into maximal
 * x-monotone polylines (see polyline_boundary()). \return the polylines.
 */
std::vector<Polyline_X_monotone_curve> read_polyline_boundary(std::istream& a_is, bool /* a_double_format */);

/*! reads the boundary of a polygon bounded by segments and circular arcs: the number of its curves followed by the
 * curves, each given by a flag (1 for an arc) followed by its source, a point in its interior (for an arc), and its
 * target. \return the x-monotone curves of the boundary.
 */
std::vector<Circular_X_monotone_curve> read_circular_boundary(std::istream& a_is, bool /* a_double_format */);

/*! reads a Bezier curve: the number of its control points followed by their coordinates, as doubles (rounded to
 * 1/1000) or as rationals. Repeated control points and control points collinear with their neighbors are dropped.
 */
Bezier_curve read_bezier_curve(std::istream& is, bool a_double_format);

/*! reads the boundary of a Bezier polygon: the number of its curves followed by the curves. \return the x-monotone
 * curves of the boundary.
 */
std::vector<Bezier_X_monotone_curve> read_bezier_boundary(std::istream& a_is, bool a_double_format);

/*! saves a polygon set to a file: a format line, the number of polygons with holes, and for each one the number of its
 * boundaries followed by the boundaries (the outer one first), each written by a given function. The coordinates are
 * written with a given number of significant digits; 17 digits write every double exactly. \return false if the file
 * cannot be written.
 */
template <typename Set, typename Write_boundary>
bool save_polygons(const QString& a_file_name, const Set& a_set, Write_boundary a_write_boundary,
                   int a_precision = 17) {
  std::ofstream out_file(qPrintable(a_file_name));
  if (! out_file) return false;
  out_file.precision(a_precision);
  std::vector<typename Set::Polygon_with_holes_2> pwhs;
  a_set.polygons_with_holes(std::back_inserter(pwhs));
  out_file << "DOUBLE" << std::endl << pwhs.size() << std::endl;
  for (const auto& pwh : pwhs) {
    out_file << " " << (1 + pwh.number_of_holes()) << std::endl;
    a_write_boundary(out_file, pwh.outer_boundary());
    for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) a_write_boundary(out_file, *hit);
  }
  return bool(out_file);
}

/*! writes the boundary of a linear polygon: the number of its vertices followed by their coordinates.
 */
void write_linear_boundary(std::ostream& a_os, const Linear_polygon& a_boundary);

/*! writes the boundary of a polygon bounded by polylines: the number of its vertices followed by their coordinates
 * (all the vertices of the polylines, in order).
 */
void write_polyline_boundary(std::ostream& a_os, const Polyline_polygon& a_boundary);

/*! obtains the midpoint of a circular arc.
 */
std::pair<double, double> arc_midpoint(const Circular_X_monotone_curve& a_arc);

/*! writes the boundary of a polygon bounded by segments and circular arcs: the number of its curves followed by the
 * curves, each given by a flag (1 for an arc) followed by its source, its midpoint (for an arc), and its target.
 */
void write_circular_boundary(std::ostream& a_os, const Circular_polygon& a_boundary);

/*! writes the boundary of a Bezier polygon: the number of its curves followed by their control points.
 */
void write_bezier_boundary(std::ostream& out_file, const Bezier_polygon& a_bp);

/*! reads the boundary of a conic polygon (see write_conic_boundary()). \return the x-monotone arcs of the boundary.
 */
std::vector<Conic_X_monotone_curve> read_conic_boundary(std::istream& a_is, bool /* a_double_format */);

/*! writes the boundary of a conic polygon: the number of its arcs followed by the arcs (see Conic_arc_record), one per
 * line, either
 *   full r s t u v w
 * for a full conic r*x^2 + s*y^2 + t*xy + u*x + v*y + w = 0 (an ellipse), or
 *   arc r s t u v w o sx sy r1 s1 t1 u1 v1 w1 tx ty r2 s2 t2 u2 v2 w2
 * for an arc of the conic of orientation o (-1 for clockwise, 1 for counterclockwise, and 0 for a segment), whose
 * source is the intersection of the conic and the auxiliary conic (r1, ..., w1) nearest to (sx, sy), and whose target
 * is the intersection of the conic and the auxiliary conic (r2, ..., w2) nearest to (tx, ty). The coefficients are
 * integers.
 * \throw std::invalid_argument if the boundary cannot be written exactly (see conic_arc_records()).
 */
void write_conic_boundary(std::ostream& a_os, const Conic_polygon& a_boundary);

#endif
