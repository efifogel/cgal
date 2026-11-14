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
 * Reading and writing the polygon files of the curve families (.lps, .cps, .plps, .bps, and .qps). A file holds
 * polygons with holes, each one given by its boundaries; a function of each family reads or writes a boundary.
 */

#include "Polygon_io.h"

#include <cmath>
#include <iomanip>
#include <stdexcept>
#include <string>

#include "Qt6/Bezier_curves.h"
#include "Polyline_boundary.h"
#include "Conic_arcs.h"

namespace {

/*! reads a number of a given type, exactly (see read_number()).
 */
template <typename NT>
NT read_exact(std::istream& a_is) {
  NT x;
  CGAL::internal::read_float_or_quotient(a_is, x);
  if (! a_is) throw std::runtime_error("bad number");
  return x;
}

} // namespace

Kernel::FT read_number(std::istream& a_is) { return read_exact<Kernel::FT>(a_is); }

Kernel::Point_2 read_point(std::istream& a_is) {
  auto x = read_number(a_is);
  return Kernel::Point_2(x, read_number(a_is));
}

std::vector<Kernel::Point_2> read_vertices(std::istream& a_is) {
  unsigned int n_points;
  a_is >> n_points;
  std::vector<Kernel::Point_2> points;
  for (unsigned int i = 0; i < n_points; ++i) points.push_back(read_point(a_is));
  return points;
}

std::vector<Linear_X_monotone_curve> read_linear_boundary(std::istream& a_is) {
  auto points = read_vertices(a_is);
  std::vector<Linear_curve> curves;
  for (std::size_t i = 0; i < points.size(); ++i) {
    const auto& p = points[i];
    const auto& q = points[(i + 1) % points.size()];
    if (p != q) curves.emplace_back(p, q);
  }
  return x_monotone_curves<Linear_traits>(curves.begin(), curves.end());
}

std::vector<Polyline_X_monotone_curve> read_polyline_boundary(std::istream& a_is) {
  return polyline_boundary(read_vertices(a_is));
}

std::vector<Circular_X_monotone_curve> read_circular_boundary(std::istream& a_is) {
  unsigned int n_curves;
  a_is >> n_curves;
  std::vector<Circular_curve> curves;
  for (unsigned int i = 0; i < n_curves; ++i) {
    unsigned int is_arc;
    if (! (a_is >> is_arc)) throw std::runtime_error("bad circular boundary");
    auto s = read_point(a_is);
    if (is_arc) {
      auto m = read_point(a_is);
      curves.emplace_back(s, m, read_point(a_is));
    }
    else curves.emplace_back(s, read_point(a_is));
  }
  return x_monotone_curves<Circular_traits>(curves.begin(), curves.end());
}

Bezier_curve read_bezier_curve(std::istream& is) {
  // Read the number of control points.
  unsigned int n;
  is >> n;

  // Read the control points.
  std::vector<Bezier_rat_point> ctrl_pts;

  for (unsigned int k = 0; k < n; k++) {
    auto x = read_exact<Bezier_rational>(is);
    Bezier_rat_point p(x, read_exact<Bezier_rational>(is));

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

std::vector<Bezier_X_monotone_curve> read_bezier_boundary(std::istream& a_is) {
  unsigned int n_curves;
  a_is >> n_curves;
  std::vector<Bezier_curve> curves;
  for (unsigned int i = 0; i < n_curves; ++i) {
    Bezier_curve curve = read_bezier_curve(a_is);
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
  std::size_t n = 0;
  for (auto cit = a_boundary.curves_begin(); cit != a_boundary.curves_end(); ++cit) n += cit->number_of_subcurves();
  a_os << "  " << n << std::endl;
  // The target of each polyline is the source of the next one.
  for (auto cit = a_boundary.curves_begin(); cit != a_boundary.curves_end(); ++cit) {
    for (auto it = cit->points_begin(); std::next(it) != cit->points_end(); ++it)
      a_os << "   " << CGAL::to_double(it->x()) << " " << CGAL::to_double(it->y()) << std::endl;
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

namespace {

/*! reads the six coefficients of a conic.
 */
void read_coefficients(std::istream& a_is, Conic_arc_record::Coefficients& r_c) {
  for (auto& c : r_c) {
    std::string token;
    a_is >> token;
    c = Conic_integer(token);
  }
}

/*! writes the six coefficients of a conic.
 */
void write_coefficients(std::ostream& a_os, const Conic_arc_record::Coefficients& a_c) {
  for (const auto& c : a_c) a_os << " " << c;
}

} // namespace

std::vector<Conic_X_monotone_curve> read_conic_boundary(std::istream& a_is) {
  unsigned int n_arcs;
  a_is >> n_arcs;
  std::vector<Conic_arc_record> records(n_arcs);
  for (auto& record : records) {
    std::string kind;
    a_is >> kind;
    read_coefficients(a_is, record.m_conic);
    if (kind == "full") {
      record.m_full = true;
      continue;
    }
    if (kind != "arc") throw std::runtime_error("unknown conic arc kind: " + kind);
    int orientation;
    a_is >> orientation;
    record.m_orientation = CGAL::Orientation(orientation);
    for (auto& c : record.m_source) c = CGAL::to_double(read_number(a_is));
    read_coefficients(a_is, record.m_source_conic);
    for (auto& c : record.m_target) c = CGAL::to_double(read_number(a_is));
    read_coefficients(a_is, record.m_target_conic);
  }
  if (! a_is) throw std::runtime_error("bad conic boundary");
  return conic_boundary(records);
}

void write_conic_boundary(std::ostream& a_os, const Conic_polygon& a_boundary) {
  auto records = conic_arc_records(a_boundary);
  // Always all the digits: an approximate endpoint selects one of the intersections of two conics.
  auto precision = a_os.precision(17);
  a_os << "  " << records.size() << std::endl;
  for (const auto& record : records) {
    a_os << "   " << (record.m_full ? "full" : "arc");
    write_coefficients(a_os, record.m_conic);
    if (! record.m_full) {
      a_os << " " << int(record.m_orientation) << "  " << record.m_source[0] << " " << record.m_source[1];
      write_coefficients(a_os, record.m_source_conic);
      a_os << "  " << record.m_target[0] << " " << record.m_target[1];
      write_coefficients(a_os, record.m_target_conic);
    }
    a_os << std::endl;
  }
  a_os.precision(precision);
}
