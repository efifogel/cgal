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

/*! \file Main_window_console.cpp
 * The console and the program of the main window: executing the console statements, and logging the executed statements
 * and saving them as a Python program.
 */

#include "Main_window.h"

void Main_window::log_statements(const QStringList& a_statements, bool a_sticky) {
  if (! m_logging) return;
  for (const auto& statement : a_statements) {
    m_program.push_back({a_sticky ? 0 : states_stack.size(), statement});
    if (m_log) m_log->appendPlainText(statement);
  }
}

void Main_window::trim_program_to_states() {
  auto it = std::remove_if(m_program.begin(), m_program.end(), [this](const Program_line& a_line) {
    return a_line.m_state_count > states_stack.size();
  });
  bool changed = (it != m_program.end());
  m_program.erase(it, m_program.end());
  if (! changed || ! m_log) return;
  QStringList lines;
  for (const auto& line : m_program) lines << line.m_text;
  m_log->setPlainText(lines.join("\n"));
}

bool Main_window::bucket_exists(std::size_t a_index) const { return a_index < m_number_of_buckets; }

std::size_t Main_window::bucket_index(const QString& a_name) const {
  for (std::size_t i = 0; i < max_number_of_buckets; ++i) {
    if (a_name == bucket_name(i)) return i;
  }
  return max_number_of_buckets;
}

Curve_set* Main_window::polygon_set(const QString& a_name) {
  std::size_t i = bucket_index(a_name);
  if (i < max_number_of_buckets) return bucket_exists(i) ? &states_stack.back().set(i) : nullptr;
  auto it = m_variables.find(a_name);
  return (it == m_variables.end()) ? nullptr : &it->second;
}

namespace {

/*! checks that a polygon is closed, simple, and counterclockwise oriented. Throws a std::runtime_error otherwise (with
 * a given hint if the orientation is wrong).
 */
template <typename Traits>
void check_polygon(const typename Traits::Polygon_2& a_polygon, const std::string& a_orientation_hint = "") {
  Traits traits;
  if (a_polygon.is_empty()) throw std::runtime_error("the polygon has no curves");
  if (! CGAL::is_closed_polygon(a_polygon, traits))
    throw std::runtime_error("the boundary of the polygon is not closed");
  if (! CGAL::is_simple_polygon(a_polygon, traits)) throw std::runtime_error("the polygon is not simple");
  if (! CGAL::has_valid_orientation_polygon(a_polygon, traits))
    throw std::runtime_error("the polygon is not counterclockwise oriented" + a_orientation_hint);
}

/*! constructs a curve of segments and circular arcs. Throws a std::runtime_error if it is degenerate.
 */
Circular_curve circular_curve(const Py_curve& a_curve) {
  const auto& p = a_curve.m_points;
  switch (a_curve.m_kind) {
   case Py_curve::Kind::KI_SEGMENT:
    if (p[0] == p[1]) throw std::runtime_error("the two points of a segment coincide");
    return Circular_curve(p[0], p[1]);
   case Py_curve::Kind::KI_ARC:
    if (CGAL::collinear(p[0], p[1], p[2])) throw std::runtime_error("the three points of an arc are collinear");
    return Circular_curve(p[0], p[1], p[2]);
   case Py_curve::Kind::KI_CIRCLE:
    if (CGAL::sign(a_curve.m_squared_radius) != CGAL::POSITIVE)
      throw std::runtime_error("the squared radius of a circle must be positive");
    return Circular_curve(Kernel::Circle_2(p[0], a_curve.m_squared_radius));
   default: break;
  }
  throw std::runtime_error("a conic arc is not a curve of Segments and Circular Arcs");
}

/*! determines whether a conic is an ellipse with real points (the only conics that are full curves).
 */
bool is_real_ellipse(const Conic_arc_record::Coefficients& a_c) {
  const Conic_integer& r = a_c[0], &s = a_c[1], &t = a_c[2], &u = a_c[3], &v = a_c[4], &w = a_c[5];
  if (CGAL::sign(4*r*s - t*t) != CGAL::POSITIVE) return false;
  // 4 times the determinant of [[r, t/2, u/2], [t/2, s, v/2], [u/2, v/2, w]].
  Conic_integer det4 = 4*r*s*w - r*v*v - t*t*w + t*u*v - s*u*u;
  return CGAL::sign(r + s) * CGAL::sign(det4) == CGAL::NEGATIVE;
}

} // namespace

Curve_set Main_window::polygon(const Py_expr& a_expr) const {
  if ((a_expr.m_kind != Py_expr::Kind::KI_POLYGON) && (a_expr.m_kind != Py_expr::Kind::KI_CURVES))
    throw std::runtime_error(("expected a polygon set name or a polygon instead of " + to_python(a_expr))
                             .toStdString());
  bool circular = (a_expr.m_kind == Py_expr::Kind::KI_CURVES) && a_expr.m_curves.front().is_circular();
  Curve_type type = (a_expr.m_kind == Py_expr::Kind::KI_POLYGON) ? Curve_type::CT_LINEAR :
    circular ? Curve_type::CT_CIRCULAR : Curve_type::CT_CONIC;
  if (type != m_curve_type)
    throw std::runtime_error(QString("%1 is a polygon of %2, but the polygon sets are of %3")
                             .arg((type == Curve_type::CT_LINEAR) ? "Polygon_2([...])" : "polygon([...])",
                                  family(type).m_label, family(m_curve_type).m_label).toStdString());
  Curve_set r_set(type, QPen(), QBrush());
  switch (type) {
   case Curve_type::CT_LINEAR: {
     const auto& pts = a_expr.m_points;
     if (pts.size() < 3) throw std::runtime_error("a polygon needs at least 3 vertices");
     CGAL::Polygon_2<Kernel> check;
     for (const auto& [x, y] : pts) check.push_back(Kernel::Point_2(x, y));
     if (! check.is_simple()) throw std::runtime_error("the polygon is not simple");
     if (check.orientation() != CGAL::COUNTERCLOCKWISE)
       throw std::runtime_error("the polygon is not counterclockwise oriented");
     std::vector<Linear_curve> curves;
     for (std::size_t i = 0; i < pts.size(); ++i) {
       const auto& p = pts[i];
       const auto& q = pts[(i + 1) % pts.size()];
       curves.emplace_back(Kernel::Point_2(p.first, p.second), Kernel::Point_2(q.first, q.second));
     }
     r_set.linear().join(Linear_polygon_with_holes(Linear_polygon(curves.begin(), curves.end())));
     break;
   }
   case Curve_type::CT_CIRCULAR: {
     std::vector<Circular_curve> curves;
     for (const auto& curve : a_expr.m_curves) curves.push_back(circular_curve(curve));
     auto xcvs = x_monotone_curves<Circular_traits>(curves.begin(), curves.end());
     Circular_polygon pgn(xcvs.begin(), xcvs.end());
     check_polygon<Circular_traits>(pgn);
     r_set.circular().join(Circular_polygon_with_holes(pgn));
     break;
   }
   default: {
     std::vector<Conic_arc_record> records;
     for (const auto& curve : a_expr.m_curves) {
       if (curve.m_conic.m_full && ! is_real_ellipse(curve.m_conic.m_conic))
         throw std::runtime_error("a full conic must be an ellipse with real points");
       records.push_back(curve.m_conic);
     }
     auto xcvs = conic_boundary(records);
     if (xcvs.empty()) throw std::runtime_error("a conic arc is invalid (e.g., an endpoint is not on its conic)");
     Conic_polygon pgn(xcvs.begin(), xcvs.end());
     // The coefficients of a full conic determine its orientation.
     bool full = (records.size() == 1) && records.front().m_full;
     check_polygon<Conic_gps_traits>(pgn, full ? "; negate the coefficients of the full conic to reverse it" : "");
     r_set.conic().join(Conic_polygon_with_holes(pgn));
     break;
   }
  }
  return r_set;
}

QString Main_window::execute(const Py_statement& a_statement) {
  switch (a_statement.m_kind) {
   case Py_statement::Kind::KI_DELETE: execute_delete(a_statement); return QString();
   case Py_statement::Kind::KI_ASSIGN: execute_assign(a_statement); return QString();
   case Py_statement::Kind::KI_CALL: break;
  }
  return execute_call(a_statement);
}

Curve_set& Main_window::existing_set(const QString& a_name) {
  Curve_set* set = polygon_set(a_name);
  if (! set) throw std::runtime_error(("name '" + a_name + "' is not defined").toStdString());
  return *set;
}

Curve_set& Main_window::named_set(const Py_expr& a_expr) {
  if (a_expr.m_kind != Py_expr::Kind::KI_NAME)
    throw std::runtime_error(("expected a polygon set name instead of " + to_python(a_expr)).toStdString());
  return existing_set(a_expr.m_name);
}

void Main_window::execute_delete(const Py_statement& a_statement) {
  const QString& target = a_statement.m_target;
  std::size_t index = bucket_index(target);
  if (index == max_number_of_buckets) {
    if (m_variables.erase(target) == 0)
      throw std::runtime_error(("name '" + target + "' is not defined").toStdString());
    log_statements({to_python(a_statement)}, true);
    return;
  }
  // Only the last added bucket can be removed.
  std::size_t last = m_number_of_buckets - 1;
  if ((index < min_number_of_buckets) || (index != last))
    throw std::runtime_error(index < min_number_of_buckets ? "the red, black, and blue buckets cannot be deleted" :
      ("only the last bucket (" + QString(bucket_name(last)) + ") can be deleted").toStdString());
  remove_bucket();      // logs "del <name>"
}

void Main_window::execute_assign(const Py_statement& a_statement) {
  const QString& target = a_statement.m_target;
  std::size_t index = bucket_index(target);
  const Py_expr& value = a_statement.m_args[0];
  if (value.m_kind != Py_expr::Kind::POLYGON_SET)
    throw std::runtime_error("only polygon sets can be assigned, e.g., " + (target + " = Polygon_set_2(" +
       (value.m_kind == Py_expr::Kind::KI_NAME ? value.m_name : QString()) + ")").toStdString() + " (copies the set)");
  const Py_expr* arg = value.m_arg.get();
  if ((index < max_number_of_buckets) && ! bucket_exists(index)) {
    // Buckets are added in order.
    std::size_t next = m_number_of_buckets;
    if (index != next)
      throw std::runtime_error(("buckets are added in order; the next one is " + QString(bucket_name(next)))
                               .toStdString());
    if (next >= m_max_number_of_buckets)
      throw std::runtime_error(QString("the maximum number of buckets is %1 (see the Options menu)")
                               .arg(m_max_number_of_buckets).toStdString());
    m_logging = false;
    add_bucket();
    m_logging = true;
    if (! arg) {       // a new, empty bucket
      log_statements({to_python(a_statement)}, true);
      return;
    }
    log_statements({target + " = Polygon_set_2()"}, true);
  }
  // The polygon is checked before the new state is pushed.
  std::optional<Curve_set> pgn;
  if (arg && (arg->m_kind != Py_expr::Kind::KI_NAME)) pgn = polygon(*arg);
  get_new_state(State_op::SO_CONSOLE);
  Curve_set* set = polygon_set(target);
  if (! set) set = &m_variables.emplace(target, Curve_set(m_curve_type, QPen(), QBrush())).first->second;
  if (! arg) set->clear();
  else if (pgn) set->assign(*pgn);
  else {
    Curve_set& other = named_set(*arg);
    if (&other != set) set->assign(other);
  }
  log_statements({to_python(a_statement)});
}

QString Main_window::execute_call(const Py_statement& a_statement) {
  const QString& target = a_statement.m_target;
  const QString& method = a_statement.m_method;
  const auto& args = a_statement.m_args;
  Curve_set* set = &existing_set(target);
  auto arity = [&](std::size_t a_min, std::size_t a_max) {
    if ((args.size() < a_min) || (args.size() > a_max))
      throw std::runtime_error((method + "() takes " + ((a_min == a_max) ? QString::number(a_min) :
         QString("%1 or %2").arg(a_min).arg(a_max)) + " argument(s)").toStdString());
  };

  // Queries.
  if (method == "is_empty") {
    arity(0, 0);
    return set->is_empty() ? "True" : "False";
  }
  if (method == "is_plane") {
    arity(0, 0);
    return set->is_plane() ? "True" : "False";
  }
  if (method == "number_of_polygons_with_holes") {
    arity(0, 0);
    return QString::number(set->number_of_polygons_with_holes());
  }

  // Actions; each one is undoable, so the sets are obtained again after the new state is pushed.
  if (method == "clear") {
    arity(0, 0);
    get_new_state(State_op::SO_CONSOLE);
    existing_set(target).clear();
  }
  else if (method == "complement") {
    arity(0, 1);
    if (! args.empty() && (&named_set(args[0]) == set))
      throw std::runtime_error("use complement() to complement in place");
    get_new_state(State_op::SO_CONSOLE);
    if (args.empty()) existing_set(target).complement();
    else existing_set(target).complement(named_set(args[0]));
  }
  else {
    std::optional<Bucket_op> op;
    for (auto candidate : {Bucket_op::BO_INTERSECTION, Bucket_op::BO_JOIN, Bucket_op::BO_DIFFERENCE,
                           Bucket_op::SYMMETRIC_DIFFERENCE}) {
      if (method == method_name(candidate)) op = candidate;
    }
    if (! op) throw std::runtime_error(("unknown method '" + method + "'").toStdString());
    arity(1, 2);
    if (args.size() == 2) {
      // The bivariate form overwrites the target, which must not be an operand.
      if ((&named_set(args[0]) == set) || (&named_set(args[1]) == set))
        throw std::runtime_error(("the target of " + method + "(a, b) must differ from a and b").toStdString());
      get_new_state(State_op::SO_CONSOLE);
      apply(*op, existing_set(target), named_set(args[0]), named_set(args[1]));
    }
    else if (args[0].m_kind != Py_expr::Kind::KI_NAME) {
      Curve_set pgn = polygon(args[0]);
      get_new_state(State_op::SO_CONSOLE);
      apply(*op, existing_set(target), pgn);
    }
    else {
      named_set(args[0]);      // check the name
      get_new_state(State_op::SO_CONSOLE);
      apply(*op, existing_set(target), named_set(args[0]));
    }
  }
  log_statements({to_python(a_statement)});
  return QString();
}

void Main_window::console_print(const QString& a_text)
{ if (m_console_output) m_console_output->appendPlainText(a_text); }

void Main_window::run_console_line(const QString& a_line) {
  console_print(">>> " + a_line);
  if (! a_line.trimmed().isEmpty()) {
    m_console_history << a_line;
  }
  m_console_history_pos = m_console_history.size();
  if ((m_curve_type != Curve_type::CT_LINEAR) && (m_curve_type != Curve_type::CT_CIRCULAR) &&
      (m_curve_type != Curve_type::CT_CONIC)) {
    console_print(QString("Error: the console does not support %1 yet").arg(family(m_curve_type).m_label));
    return;
  }
  try {
    auto statements = Py_parser(a_line).parse();
    for (const auto& statement : statements) {
      QString output = execute(statement);
      if (! output.isEmpty()) console_print(output);
    }
  }
  catch (std::exception& e) {
    console_print(QString("Error: ") + e.what());
  }
  model_changed();
}

void Main_window::save_program() {
  QString file_name = QFileDialog::getSaveFileName(this, "Save Program", "bso_program.py", "Python files (*.py)");
  if (file_name.isEmpty()) return;
  QSaveFile file(file_name);
  if (! file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    show_error("Save Program", "Cannot open " + file_name + " for writing.");
    return;
  }
  QTextStream out(&file);
  out << program_header(m_curve_type);
  for (const auto& line : m_program) out << line.m_text << "\n";
  out.flush();
  if (! file.commit())
    show_error("Save Program", "Cannot write " + file_name + ".");
}
