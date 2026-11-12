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

/*! \file Main_window_files.cpp
 * Opening and saving polygon files in the main window.
 */

#include "Main_window.h"

#include "Polygon_io.h"

void Main_window::open_linear() {
  open(QFileDialog::getOpenFileName(this, tr("Open Linear Polygon"), data_directory(Curve_type::CT_LINEAR),
                                    tr("Linear Curve files (*.lps)")));
}

void Main_window::open_circular() {
  open(QFileDialog::getOpenFileName(this, tr("Open Line segments and circular arcs"),
                                    data_directory(Curve_type::CT_CIRCULAR), tr("Circular curve files (*.cps)")));
}

void Main_window::open_bezier() {
  open(QFileDialog::getOpenFileName(this, tr("Open Bezier Polygon"), data_directory(Curve_type::CT_BEZIER),
                                    tr("Bezier Curve files (*.bps)")));
}

void Main_window::open_polyline() {
  open(QFileDialog::getOpenFileName(this, tr("Open Polyline Polygon"), data_directory(Curve_type::CT_POLYLINE),
                                    tr("Polyline Curve files (*.plps)")));
}

template <typename Set, typename Read_boundary>
bool Main_window::read_polygons(const QString& a_file_name, const char* a_kind, Read_boundary a_read_boundary) {
  std::ifstream in_file(qPrintable(a_file_name));
  if (! in_file) return false;
  bool r_ok = false;
  try {
    std::string format;
    std::getline(in_file, format);
    bool double_format = (format.substr(0, 6) == "DOUBLE");

    unsigned int n_regions;
    in_file >> n_regions;
    for (unsigned int r = 0; r < n_regions; ++r) {
      unsigned int n_boundaries;
      in_file >> n_boundaries;
      for (unsigned int b = 0; b < n_boundaries; ++b) {
        auto xcvs = a_read_boundary(in_file, double_format);
        if (xcvs.empty()) continue;
        typename Set::Polygon_2 pgn(xcvs.begin(), xcvs.end());
        if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
        typename Set::Polygon_with_holes_2 pwh(pgn);
        // The first boundary is the outer one; the others are holes.
        if (b == 0) {
          get_new_state(State_op::SO_DRAW);
          states_stack.back().set(m_color_active).get<Set>().join(pwh);
        }
        else states_stack.back().set(m_color_active).get<Set>().difference(Set(pwh));
      }
      r_ok = true;
    }
  }
  catch (...) {
    show_warning(std::string("Exception occurred during reading of ") + a_kind + " polygon set.");
  }
  return r_ok;
}

bool Main_window::read_linear(QString a_file_name) {
  return read_polygons<Linear_polygon_set>(a_file_name, "linear", read_linear_boundary);
}

bool Main_window::read_circular(QString a_file_name) {
  return read_polygons<Circular_polygon_set>(a_file_name, "circular", read_circular_boundary);
}

bool Main_window::read_polyline(QString a_file_name) {
  return read_polygons<Polyline_polygon_set>(a_file_name, "polyline", read_polyline_boundary);
}

bool Main_window::read_bezier(QString a_file_name) {
  return read_polygons<Bezier_polygon_set>(a_file_name, "Bezier", read_bezier_boundary);
}

void Main_window::save_draw_bucket() {
  if (m_curve_type == Curve_type::CT_CONIC) {
    show_error("Saving conic polygon sets is not supported yet.");
    return;
  }
  const auto& selected = family(m_curve_type);
  QString name = capitalized(selected.m_name);
  QString file_name = QFileDialog::getSaveFileName(this, "Save " + name + " Polygon Set",
                                                   data_directory(m_curve_type) + "/index" + selected.m_extension,
                                                   QString("%1 Curve files (*%2)").arg(name, selected.m_extension));
  if (file_name.isEmpty()) return;
  auto& bucket = states_stack.back().set(m_color_active);
  bool saved = false;
  switch (m_curve_type) {
   case Curve_type::CT_LINEAR: saved = save_polygons(file_name, bucket.linear(), write_linear_boundary); break;
   case Curve_type::CT_CIRCULAR: saved = save_polygons(file_name, bucket.circular(), write_circular_boundary); break;
   case Curve_type::CT_BEZIER: saved = save_polygons(file_name, bucket.bezier(), write_bezier_boundary); break;
   case Curve_type::CT_POLYLINE: saved = save_polygons(file_name, bucket.polyline(), write_polyline_boundary); break;
   default: break;
  }
  if (! saved) show_error("Cannot save " + std::string(selected.m_name) + " polygon set.");
}

void Main_window::open(QString a_file_name) {
  if (a_file_name.isEmpty()) return;
  for (const auto& family : m_families) {
    if (! family.m_extension || ! a_file_name.endsWith(family.m_extension)) continue;
    if (ensure_curve_type(family.m_type) && (this->*family.m_read)(a_file_name)) {
      model_changed();
      this->addToRecentFiles(a_file_name);
    }
    return;
  }
}
