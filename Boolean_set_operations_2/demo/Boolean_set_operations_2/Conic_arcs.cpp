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

/*! \file Conic_arcs.cpp
 * The exact description of the boundaries of conic polygons.
 */

#include "Conic_arcs.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <variant>

namespace {

using Coefficients = Conic_arc_record::Coefficients;

/*! obtains the coefficients of the supporting conic of an arc.
 */
Coefficients coefficients(const Conic_X_monotone_curve& a_xcv)
{ return {a_xcv.r(), a_xcv.s(), a_xcv.t(), a_xcv.u(), a_xcv.v(), a_xcv.w()}; }

/*! determines whether two conics are the same, that is, their coefficients are proportional.
 */
bool same_conic(const Coefficients& a_c1, const Coefficients& a_c2) {
  for (std::size_t i = 0; i < 6; ++i) {
    for (std::size_t j = 0; j < 6; ++j) {
      if (a_c1[i] * a_c2[j] != a_c2[i] * a_c1[j]) return false;
    }
  }
  return true;
}

/*! determines whether two consecutive arcs are parts of one arc: they have the same conic and the same orientation.
 */
bool same_arc(const Conic_X_monotone_curve& a_xcv1, const Conic_X_monotone_curve& a_xcv2)
{ return (a_xcv1.orientation() == a_xcv2.orientation()) && same_conic(coefficients(a_xcv1), coefficients(a_xcv2)); }

/*! obtains the auxiliary conic of an endpoint of an arc: the conic of the neighboring arc.
 * \throw std::invalid_argument if the neighboring arc has the same conic (the boundary turns back along it).
 */
const Coefficients& auxiliary_conic(const Coefficients& a_conic, const Coefficients& a_neighbor) {
  if (same_conic(a_conic, a_neighbor))
    throw std::invalid_argument("an endpoint is shared by two arcs of the same conic");
  return a_neighbor;
}

/*! obtains an approximation of a point.
 */
std::array<double, 2> approximate(const Conic_point& a_p)
{ return {CGAL::to_double(a_p.x()), CGAL::to_double(a_p.y())}; }

} // namespace

std::vector<Conic_arc_record> conic_arc_records(const Conic_polygon& a_boundary) {
  std::vector<Conic_X_monotone_curve> xcvs(a_boundary.curves_begin(), a_boundary.curves_end());
  std::vector<Conic_arc_record> r_records;
  if (xcvs.empty()) return r_records;
  // A segment constructed from its endpoints has no equation (its coefficients are 0).
  for (const auto& xcv : xcvs) {
    const auto c = coefficients(xcv);
    if (std::all_of(c.begin(), c.end(), [](const Conic_integer& a_c) { return CGAL::is_zero(a_c); }))
      throw std::invalid_argument("a segment of the boundary is not given by the equation of its line");
  }

  // Start at an arc that does not continue its predecessor; if there is none, the boundary is a full conic.
  std::size_t n = xcvs.size(), start = 0;
  while ((start < n) && same_arc(xcvs[(start + n - 1) % n], xcvs[start])) ++start;
  if (start == n) {
    Conic_arc_record record;
    record.m_conic = coefficients(xcvs.front());
    record.m_orientation = xcvs.front().orientation();
    record.m_full = true;
    // The orientation of a full conic follows from the signs of its coefficients.
    if (conic_curve(record).orientation() != record.m_orientation) {
      for (auto& c : record.m_conic) c = -c;
    }
    r_records.push_back(record);
    return r_records;
  }

  // Merge the consecutive arcs of the same conic and orientation.
  for (std::size_t k = 0; k < n; ++k) {
    const auto& xcv = xcvs[(start + k) % n];
    if ((k > 0) && same_arc(xcvs[(start + k + n - 1) % n], xcv)) {
      r_records.back().m_target = approximate(xcv.target());
      continue;
    }
    Conic_arc_record record;
    record.m_conic = coefficients(xcv);
    record.m_orientation = xcv.orientation();
    record.m_source = approximate(xcv.source());
    record.m_target = approximate(xcv.target());
    r_records.push_back(record);
  }

  // The auxiliary conics are the conics of the neighbors.
  std::size_t m = r_records.size();
  for (std::size_t i = 0; i < m; ++i) {
    auto& record = r_records[i];
    record.m_source_conic = auxiliary_conic(record.m_conic, r_records[(i + m - 1) % m].m_conic);
    record.m_target_conic = auxiliary_conic(record.m_conic, r_records[(i + 1) % m].m_conic);
  }
  return r_records;
}

Conic_curve conic_curve(const Conic_arc_record& a_record) {
  Conic_traits traits;
  auto ctr = traits.construct_curve_2_object();
  const auto& c = a_record.m_conic;
  auto q = [](const Conic_integer& a_i) { return Conic_rational(a_i); };
  if (a_record.m_full) return ctr(q(c[0]), q(c[1]), q(c[2]), q(c[3]), q(c[4]), q(c[5]));
  const auto& c1 = a_record.m_source_conic;
  const auto& c2 = a_record.m_target_conic;
  Conic_point source(a_record.m_source[0], a_record.m_source[1]);
  Conic_point target(a_record.m_target[0], a_record.m_target[1]);
  return ctr(q(c[0]), q(c[1]), q(c[2]), q(c[3]), q(c[4]), q(c[5]), a_record.m_orientation,
             source, q(c1[0]), q(c1[1]), q(c1[2]), q(c1[3]), q(c1[4]), q(c1[5]),
             target, q(c2[0]), q(c2[1]), q(c2[2]), q(c2[3]), q(c2[4]), q(c2[5]));
}

std::vector<Conic_X_monotone_curve> conic_boundary(const std::vector<Conic_arc_record>& a_records) {
  Conic_traits traits;
  auto make_x_monotone = traits.make_x_monotone_2_object();
  std::vector<Conic_X_monotone_curve> r_xcvs;
  for (const auto& record : a_records) {
    Conic_curve cv = conic_curve(record);
    if (! cv.is_valid()) return {};
    std::vector<std::variant<Conic_point, Conic_X_monotone_curve>> objs;
    make_x_monotone(cv, std::back_inserter(objs));
    for (const auto& obj : objs) {
      if (const auto* xcv = std::get_if<Conic_X_monotone_curve>(&obj)) r_xcvs.push_back(*xcv);
    }
  }
  return r_xcvs;
}
