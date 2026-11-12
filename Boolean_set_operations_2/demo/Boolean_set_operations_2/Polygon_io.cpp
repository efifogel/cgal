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

/*! \file Polygon_io.cpp
 * Reading and writing the polygon files of the curve families (.lps, .cps, .plps, and .bps). A file holds polygons with
 * holes, each one given by its boundaries; a function of each family reads or writes a boundary.
 */

#include "Polygon_io.h"

#include <cmath>

#include "Qt6/Bezier_curves.h"

std::vector<Kernel::Point_2> read_vertices(std::istream& a_is) {
  unsigned int n_points;
  a_is >> n_points;
  std::vector<Kernel::Point_2> points;
  for (unsigned int i = 0; i < n_points; ++i) {
    double x, y;
    a_is >> x >> y;
    points.emplace_back(x, y);
  }
  return points;
}

std::vector<Linear_X_monotone_curve> read_linear_boundary(std::istream& a_is, bool /* a_double_format */) {
  auto points = read_vertices(a_is);
  std::vector<Linear_curve> curves;
  for (std::size_t i = 0; i < points.size(); ++i) {
    const auto& p = points[i];
    const auto& q = points[(i + 1) % points.size()];
    if (p != q) curves.emplace_back(p, q);
  }
  return x_monotone_curves<Linear_traits>(curves.begin(), curves.end());
}

std::vector<Polyline_X_monotone_curve> read_polyline_boundary(std::istream& a_is, bool /* a_double_format */) {
  auto points = read_vertices(a_is);
  Polyline_traits traits;
  auto construct_segment = traits.subcurve_traits_2()->construct_x_monotone_curve_2_object();
  std::vector<Polyline_curve> curves;
  for (std::size_t i = 0; i < points.size(); ++i) {
    const auto& p = points[i];
    const auto& q = points[(i + 1) % points.size()];
    if (p != q) curves.emplace_back(construct_segment(p, q));
  }
  return x_monotone_curves<Polyline_traits>(curves.begin(), curves.end());
}

std::vector<Circular_X_monotone_curve> read_circular_boundary(std::istream& a_is, bool /* a_double_format */) {
  using Point = Kernel::Point_2;
  unsigned int n_curves;
  a_is >> n_curves;
  std::vector<Circular_curve> curves;
  for (unsigned int i = 0; i < n_curves; ++i) {
    unsigned int is_arc;
    double sx, sy, mx, my, tx, ty;
    a_is >> is_arc >> sx >> sy;
    if (is_arc) a_is >> mx >> my;
    a_is >> tx >> ty;
    if (is_arc) curves.emplace_back(Point(sx, sy), Point(mx, my), Point(tx, ty));
    else curves.emplace_back(Point(sx, sy), Point(tx, ty));
  }
  return x_monotone_curves<Circular_traits>(curves.begin(), curves.end());
}

Bezier_curve read_bezier_curve(std::istream& is, bool a_double_format) {
  // Read the number of control points.
  unsigned int n;
  is >> n;

  // Read the control points.
  std::vector<Bezier_rat_point> ctrl_pts;

  for (unsigned int k = 0; k < n; k++) {
    Bezier_rat_point p;
    if (a_double_format) {
      double x, y;
      is >> x >> y;
      Bezier_rational rx(static_cast<int> (1000 * x + 0.5), 1000);
      Bezier_rational ry(static_cast<int> (1000 * y + 0.5), 1000);
      p = Bezier_rat_point(rx, ry);
    }
    else {
      is >> p;
    }

    if (k == 0 || ctrl_pts[k - 1] != p) {
      ctrl_pts.push_back(p);
    }
  }
  std::vector<Bezier_rat_point> ctrl_pts2;
  auto beg = ctrl_pts.begin();
  auto end = ctrl_pts.end();
  auto last = end - 1;
  ctrl_pts2.push_back(*beg);
  if (ctrl_pts.size() > 2) {
    auto curr = beg;
    auto next1 = curr + 1;
    auto next2 = next1 + 1;
    do {
      CGAL::Orientation l_orient = orientation(*curr, *next1, *next2);
      if (l_orient != CGAL::COLLINEAR)
        ctrl_pts2.push_back(*next1);
      ++curr;
      ++next1;
      ++next2;
    } while (next2 != end);
  }
  ctrl_pts2.push_back(*last);
  return Bezier_curve(ctrl_pts2.begin(), ctrl_pts2.end());
}

std::vector<Bezier_X_monotone_curve> read_bezier_boundary(std::istream& a_is, bool a_double_format) {
  unsigned int n_curves;
  a_is >> n_curves;
  std::vector<Bezier_curve> curves;
  for (unsigned int i = 0; i < n_curves; ++i) {
    Bezier_curve curve = read_bezier_curve(a_is, a_double_format);
    if (curve.number_of_control_points() >= 2) curves.push_back(curve);
  }
  return x_monotone_curves<Bezier_traits>(curves.begin(), curves.end());
}

void write_linear_boundary(std::ostream& a_os, const Linear_polygon& a_boundary) {
  a_os << "  " << a_boundary.size() << std::endl;
  for (auto cit = a_boundary.curves_begin(); cit != a_boundary.curves_end(); ++cit)
    a_os << "   " << CGAL::to_double(cit->source().x()) << " " << CGAL::to_double(cit->source().y()) << std::endl;
}

void write_polyline_boundary(std::ostream& a_os, const Polyline_polygon& a_boundary) {
  a_os << "  " << a_boundary.size() << std::endl;
  for (auto cit = a_boundary.curves_begin(); cit != a_boundary.curves_end(); ++cit) {
    auto it = cit->points_begin();
    a_os << " " << CGAL::to_double(it->x()) << " " << CGAL::to_double(it->y()) << std::endl;
  }
}

std::pair<double, double> arc_midpoint(const Circular_X_monotone_curve& a_arc) {
  double r = std::sqrt(CGAL::to_double(a_arc.supporting_circle().squared_radius()));
  double sx = CGAL::to_double(a_arc.source().x());
  double sy = CGAL::to_double(a_arc.source().y());
  double tx = CGAL::to_double(a_arc.target().x());
  double ty = CGAL::to_double(a_arc.target().y());
  double cx = CGAL::to_double(a_arc.supporting_circle().center().x());
  double cy = CGAL::to_double(a_arc.supporting_circle().center().y());
  double st = std::sqrt((sx - tx) * (sx - tx) + (sy - ty) * (sy - ty));    // the length of the chord
  double tnx = tx - cx;                                                     // the target relative to the center
  double tny = ty - cy;
  double frmst = std::sqrt((4 * r * r) - (st * st));
  if (a_arc.supporting_circle().orientation() == CGAL::CLOCKWISE)
    return {(((frmst * tnx) - (st * tny)) / (2 * r)) + cx, (((st * tnx) + (frmst * tny)) / (2 * r)) + cy};
  return {(((tnx * frmst) + (tny * st)) / (2 * r)) + cx, (((tny * frmst) - (st * tnx)) / (2 * r)) + cy};
}

void write_circular_boundary(std::ostream& a_os, const Circular_polygon& a_boundary) {
  a_os << "  " << a_boundary.size() << std::endl;
  for (auto cit = a_boundary.curves_begin(); cit != a_boundary.curves_end(); ++cit) {
    a_os << "   " << (cit->is_circular() ? 1 : 0) << std::endl;
    a_os << "    " << CGAL::to_double(cit->source().x()) << " " << CGAL::to_double(cit->source().y()) << std::endl;
    if (cit->is_circular()) {
      auto [mx, my] = arc_midpoint(*cit);
      a_os << "    " << mx << " " << my << std::endl;
    }
    a_os << "    " << CGAL::to_double(cit->target().x()) << " " << CGAL::to_double(cit->target().y()) << std::endl;
  }
}

void write_bezier_boundary(std::ostream& out_file, const Bezier_polygon& a_bp) {
  using Bezier_rat_point_vector = std::vector<Bezier_rat_point>;

  int cc = a_bp.size();
  int lc = cc - 1;
  out_file << "  " << cc << std::endl;
  Bezier_rat_point l_first_p, l_prev_p;
  int i = 0;

  for (auto cit = a_bp.curves_begin(); cit != a_bp.curves_end(); ++cit, ++i) {
    Bezier_rat_point_vector l_q;
    CGAL::Qt::Bezier_helper::clip(*cit, l_q);
    out_file << "   " << l_q.size() << std::endl;

    if (i == 0)
      l_first_p = l_q.front();

    if (i == lc)
      l_q.back() = l_first_p;

    for (auto pit = l_q.begin(); pit != l_q.end(); ++pit) {
      Bezier_rat_point l_p = pit == l_q.begin() && i > 0 ? l_prev_p : *pit;
      out_file << "    " << CGAL::to_double(l_p.x()) << " " << CGAL::to_double(l_p.y()) << std::endl;
      l_prev_p = l_p;
    }
  }
}
