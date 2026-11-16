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

#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

#include <QFileInfo>

#include "Main_window.h"

#include "Polygon_io.h"

void Main_window::open_family(Curve_type a_type) {
  const auto& selected = family(a_type);
  QString label = selected.m_label;
  open(QFileDialog::getOpenFileName(this, "Open " + label, data_directory(a_type),
                                    QString("%1 (*%2)").arg(label, selected.m_extension)));
}

template <typename Set, typename Read_boundary, typename To_python>
bool Main_window::read_polygons(const QString& a_file_name, Read_boundary a_read_boundary, To_python a_to_python) {
  QString name = QFileInfo(a_file_name).fileName();
  std::ifstream in_file(qPrintable(a_file_name));
  if (! in_file) {
    show_error("Open", QString("Cannot open %1.").arg(name));
    return false;
  }
  bool r_ok = false;
  try {
    // Skip the format line of old files (DOUBLE or RATIONAL); the form of each number determines its kind.
    in_file >> std::ws;
    if (std::isalpha(in_file.peek())) {
      std::string format;
      std::getline(in_file, format);
    }

    // Read and check all the boundaries first, so that a bad file adds nothing.
    struct Boundary {
      typename Set::Polygon_2 m_polygon;
      QString m_statement;                  // logged
    };
    using Traits = typename Set::Traits_2;
    Traits traits;
    QString bucket = bucket_name(m_color_active);
    unsigned int n_regions;
    if (! (in_file >> n_regions)) throw std::runtime_error("bad file");
    std::vector<std::vector<Boundary>> regions(n_regions);
    for (unsigned int r = 0; r < n_regions; ++r) {
      unsigned int n_boundaries;
      in_file >> n_boundaries;
      for (unsigned int b = 0; b < n_boundaries; ++b) {
        auto xcvs = a_read_boundary(in_file);
        if (! in_file) throw std::runtime_error("bad file");
        if (xcvs.empty()) continue;
        typename Set::Polygon_2 pgn(xcvs.begin(), xcvs.end());
        QString which = (b == 0) ? QString("the outer boundary of polygon %1").arg(r + 1) :
          QString("hole %1 of polygon %2").arg(b).arg(r + 1);
        if (! CGAL::is_closed_polygon(pgn, traits)) throw std::runtime_error((which + " is not closed").toStdString());
        if (! CGAL::is_simple_polygon(pgn, traits))
          throw std::runtime_error((which + " intersects itself").toStdString());
        bool reversed = (pgn.orientation() == CGAL::CLOCKWISE);
        if (reversed) pgn.reverse_orientation();
        // The first boundary is the outer one; the others are holes.
        QString operation = (b == 0) ? "join" : "difference";
        QString statement;
        try {
          QString python = a_to_python(pgn, reversed);
          if (python.isEmpty()) statement = QString("# %1.%2(<%3 of %4>)").arg(bucket, operation, which, name);
          else statement = QString("%1.%2(%3)").arg(bucket, operation, python);
        }
        catch (const std::invalid_argument& e) {
          statement = QString("# The %1 polygon cannot be written exactly: %2").arg(bucket, e.what());
        }
        regions[r].push_back({pgn, statement});
      }
    }

    for (const auto& region : regions) {
      if (region.empty()) continue;
      get_new_state(State_op::SO_DRAW);
      auto& set = states_stack.back().set(m_color_active).get<Set>();
      for (std::size_t b = 0; b < region.size(); ++b) {
        typename Set::Polygon_with_holes_2 pwh(region[b].m_polygon);
        if (b == 0) set.join(pwh);
        else set.difference(Set(pwh));
        log_statements({region[b].m_statement});
      }
      r_ok = true;
    }
  }
  catch (const std::exception& e) {
    show_error("Open", QString("Cannot read %1: %2.").arg(name, e.what()));
  }
  catch (...) {
    show_error("Open", QString("Cannot read %1.").arg(name));
  }
  return r_ok;
}

// The polygons of the polycurve and Bezier families are not written to programs yet.
template <typename Polygon>
static QString no_python(const Polygon&, bool) { return QString(); }

bool Main_window::read_linear(QString a_file_name) {
  auto python = [](const Linear_polygon& a_pgn, bool) { return to_python(a_pgn); };
  return read_polygons<Linear_polygon_set>(a_file_name, read_linear_boundary, python);
}

bool Main_window::read_circular(QString a_file_name) {
  // The program constructs the boundary from the pieces read, as from those the user constructs.
  Circular_boundary_source source;
  auto read = [&source](std::istream& a_is) { return read_circular_boundary(a_is, &source); };
  auto python = [&source](const Circular_polygon&, bool a_reversed)
  { return to_python(a_reversed ? reversed(source) : source); };
  return read_polygons<Circular_polygon_set>(a_file_name, read, python);
}

bool Main_window::read_polyline(QString a_file_name) {
  return read_polygons<Polyline_polygon_set>(a_file_name, read_polyline_boundary, no_python<Polyline_polygon>);
}

bool Main_window::read_circular_polycurve(QString a_file_name) {
  return read_polygons<Circular_polycurve_polygon_set>(a_file_name, read_circular_polycurve_boundary,
                                                       no_python<Circular_polycurve_polygon>);
}

bool Main_window::read_conic_polycurve(QString a_file_name) {
  return read_polygons<Conic_polycurve_polygon_set>(a_file_name, read_conic_polycurve_boundary,
                                                    no_python<Conic_polycurve_polygon>);
}

bool Main_window::read_conic(QString a_file_name) {
  auto python = [](const Conic_polygon& a_pgn, bool) { return to_python(a_pgn); };
  return read_polygons<Conic_polygon_set>(a_file_name, read_conic_boundary, python);
}

bool Main_window::read_bezier(QString a_file_name) {
  return read_polygons<Bezier_polygon_set>(a_file_name, read_bezier_boundary, no_python<Bezier_polygon>);
}

void Main_window::save_draw_bucket() {
  const auto& selected = family(m_curve_type);
  if (! selected.m_extension) {
    show_warning("Save", QString("The polygons of %1 cannot be saved yet.").arg(selected.m_label));
    return;
  }
  QString label = selected.m_label;
  QString file_name = QFileDialog::getSaveFileName(this, "Save " + label,
                                                   data_directory(m_curve_type) + "/index" + selected.m_extension,
                                                   QString("%1 (*%2)").arg(label, selected.m_extension));
  if (file_name.isEmpty()) return;
  auto& bucket = states_stack.back().set(m_color_active);
  bool saved = false;
  int precision = m_save_precision;
  try {
    switch (m_curve_type) {
     case Curve_type::CT_LINEAR:
      saved = save_polygons(file_name, bucket.linear(), write_linear_boundary, precision);
      break;
     case Curve_type::CT_CIRCULAR:
      saved = save_polygons(file_name, bucket.circular(), write_circular_boundary, precision);
      break;
     case Curve_type::CT_BEZIER:
      saved = save_polygons(file_name, bucket.bezier(), write_bezier_boundary, precision);
      break;
     case Curve_type::CT_POLYLINE:
      saved = save_polygons(file_name, bucket.polyline(), write_polyline_boundary, precision);
      break;
     case Curve_type::CT_CONIC:
      saved = save_polygons(file_name, bucket.conic(), write_conic_boundary, precision);
      break;
     case Curve_type::CT_CIRCULAR_POLYCURVE:
      saved = save_polygons(file_name, bucket.circular_polycurve(), write_circular_polycurve_boundary, precision);
      break;
     case Curve_type::CT_CONIC_POLYCURVE:
      saved = save_polygons(file_name, bucket.conic_polycurve(), write_conic_polycurve_boundary, precision);
      break;
     default: break;
    }
  }
  catch (const std::exception& e) {
    show_error("Save", QString("Cannot save %1: %2.").arg(QFileInfo(file_name).fileName(), e.what()));
    return;
  }
  if (! saved) show_error("Save", QString("Cannot save %1.").arg(QFileInfo(file_name).fileName()));
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
  show_warning("Open", QString("The type of %1 is unknown.").arg(QFileInfo(a_file_name).fileName()));
}
