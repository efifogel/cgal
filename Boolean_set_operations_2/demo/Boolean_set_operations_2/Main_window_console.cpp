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

void MainWindow::log_statements(const QStringList& aStatements, bool aSticky) {
  if (! m_logging) return;
  for (const auto& statement : aStatements) {
    m_program.push_back({aSticky ? 0 : states_stack.size(), statement});
    if (m_log) m_log->appendPlainText(statement);
  }
}

void MainWindow::trim_program_to_states() {
  auto it = std::remove_if(m_program.begin(), m_program.end(), [this](const Program_line& aLine) {
    return aLine.m_state_count > states_stack.size();
  });
  bool changed = (it != m_program.end());
  m_program.erase(it, m_program.end());
  if (! changed || ! m_log) return;
  QStringList lines;
  for (const auto& line : m_program) lines << line.m_text;
  m_log->setPlainText(lines.join("\n"));
}

bool MainWindow::bucket_exists(std::size_t aIndex) const { return aIndex < m_number_of_buckets; }

std::size_t MainWindow::bucket_index(const QString& aName) const {
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    if (aName == bucket_name(i)) return i;
  }
  return number_of_buckets;
}

Curve_set* MainWindow::polygon_set(const QString& aName) {
  std::size_t i = bucket_index(aName);
  if (i < number_of_buckets) return bucket_exists(i) ? &states_stack.back().set(i) : nullptr;
  auto it = m_variables.find(aName);
  return (it == m_variables.end()) ? nullptr : &it->second;
}

Linear_polygon_with_holes MainWindow::polygon(const Py_expr& aExpr) const {
  const auto& pts = aExpr.m_points;
  if (pts.size() < 3)
    throw std::runtime_error("a polygon needs at least 3 vertices");
  CGAL::Polygon_2<Kernel> check;
  for (const auto& [x, y] : pts) check.push_back(Kernel::Point_2(x, y));
  if (! check.is_simple())
    throw std::runtime_error("the polygon is not simple");
  if (check.orientation() != CGAL::COUNTERCLOCKWISE)
    throw std::runtime_error("the polygon is not counterclockwise oriented");
  std::vector<Linear_curve> curves;
  for (std::size_t i = 0; i < pts.size(); ++i) {
    const auto& p = pts[i];
    const auto& q = pts[(i + 1) % pts.size()];
    curves.emplace_back(Kernel::Point_2(p.first, p.second), Kernel::Point_2(q.first, q.second));
  }
  return Linear_polygon_with_holes(Linear_polygon(curves.begin(), curves.end()));
}

QString MainWindow::execute(const Py_statement& aStatement) {
  switch (aStatement.m_kind) {
   case Py_statement::Kind::Delete: execute_delete(aStatement); return QString();
   case Py_statement::Kind::Assign: execute_assign(aStatement); return QString();
   case Py_statement::Kind::Call: break;
  }
  return execute_call(aStatement);
}

Curve_set& MainWindow::existing_set(const QString& aName) {
  Curve_set* set = polygon_set(aName);
  if (! set) throw std::runtime_error(("name '" + aName + "' is not defined").toStdString());
  return *set;
}

Curve_set& MainWindow::named_set(const Py_expr& aExpr) {
  if (aExpr.m_kind != Py_expr::Kind::Name)
    throw std::runtime_error(("expected a polygon set name instead of " + to_python(aExpr)).toStdString());
  return existing_set(aExpr.m_name);
}

void MainWindow::execute_delete(const Py_statement& aStatement) {
  const QString& target = aStatement.m_target;
  std::size_t index = bucket_index(target);
  if (index == number_of_buckets) {
    if (m_variables.erase(target) == 0)
      throw std::runtime_error(("name '" + target + "' is not defined").toStdString());
    log_statements({to_python(aStatement)}, true);
    return;
  }
  // Only the last added bucket can be removed.
  std::size_t last = m_number_of_buckets - 1;
  if ((index < min_number_of_buckets) || (index != last))
    throw std::runtime_error(index < min_number_of_buckets ? "the red, black, and blue buckets cannot be deleted" :
      ("only the last bucket (" + QString(bucket_name(last)) + ") can be deleted").toStdString());
  remove_bucket();      // logs "del <name>"
}

void MainWindow::execute_assign(const Py_statement& aStatement) {
  const QString& target = aStatement.m_target;
  std::size_t index = bucket_index(target);
  const Py_expr& value = aStatement.m_args[0];
  if (value.m_kind != Py_expr::Kind::Polygon_set)
    throw std::runtime_error("only polygon sets can be assigned, e.g., " + (target + " = Polygon_set_2(" +
       (value.m_kind == Py_expr::Kind::Name ? value.m_name : QString()) + ")").toStdString() + " (copies the set)");
  const Py_expr* arg = value.m_arg.get();
  if ((index < number_of_buckets) && ! bucket_exists(index)) {
    // Buckets are added in order.
    std::size_t next = m_number_of_buckets;
    if (index != next)
      throw std::runtime_error(("buckets are added in order; the next one is " + QString(bucket_name(next)))
                               .toStdString());
    m_logging = false;
    add_bucket();
    m_logging = true;
    if (! arg) {       // a new, empty bucket
      log_statements({to_python(aStatement)}, true);
      return;
    }
    log_statements({target + " = Polygon_set_2()"}, true);
  }
  get_new_state(State_op::Console);
  Curve_set* set = polygon_set(target);
  if (! set) set = &m_variables.emplace(target, Curve_set(Curve_type::Linear, QPen(), QBrush())).first->second;
  if (! arg) set->clear();
  else if (arg->m_kind == Py_expr::Kind::Polygon) {
    Linear_polygon_with_holes pgn = polygon(*arg);
    set->clear();
    set->linear().join(pgn);
  }
  else {
    Curve_set& other = named_set(*arg);
    if (&other != set) set->assign(other);
  }
  log_statements({to_python(aStatement)});
}

QString MainWindow::execute_call(const Py_statement& aStatement) {
  const QString& target = aStatement.m_target;
  const QString& method = aStatement.m_method;
  const auto& args = aStatement.m_args;
  Curve_set* set = &existing_set(target);
  auto arity = [&](std::size_t aMin, std::size_t aMax) {
    if ((args.size() < aMin) || (args.size() > aMax))
      throw std::runtime_error((method + "() takes " + ((aMin == aMax) ? QString::number(aMin) :
         QString("%1 or %2").arg(aMin).arg(aMax)) + " argument(s)").toStdString());
  };

  // Queries.
  if (method == "is_empty") {
    arity(0, 0);
    return set->is_empty() ? "True" : "False";
  }
  if (method == "is_plane") {
    arity(0, 0);
    return set->linear().is_plane() ? "True" : "False";
  }
  if (method == "number_of_polygons_with_holes") {
    arity(0, 0);
    return QString::number(set->linear().number_of_polygons_with_holes());
  }

  // Actions; each one is undoable, so the sets are obtained again after the new state is pushed.
  if (method == "clear") {
    arity(0, 0);
    get_new_state(State_op::Console);
    existing_set(target).clear();
  }
  else if (method == "complement") {
    arity(0, 1);
    if (! args.empty() && (&named_set(args[0]) == set))
      throw std::runtime_error("use complement() to complement in place");
    get_new_state(State_op::Console);
    if (args.empty()) existing_set(target).complement();
    else existing_set(target).complement(named_set(args[0]));
  }
  else {
    std::optional<Bucket_op> op;
    for (auto candidate : {Bucket_op::Intersection, Bucket_op::Join, Bucket_op::Difference,
                           Bucket_op::Symmetric_difference}) {
      if (method == method_name(candidate)) op = candidate;
    }
    if (! op) throw std::runtime_error(("unknown method '" + method + "'").toStdString());
    arity(1, 2);
    if (args.size() == 2) {
      // The bivariate form overwrites the target, which must not be an operand.
      if ((&named_set(args[0]) == set) || (&named_set(args[1]) == set))
        throw std::runtime_error(("the target of " + method + "(a, b) must differ from a and b").toStdString());
      get_new_state(State_op::Console);
      apply(*op, existing_set(target), named_set(args[0]), named_set(args[1]));
    }
    else if (args[0].m_kind == Py_expr::Kind::Polygon) {
      Linear_polygon_with_holes pgn = polygon(args[0]);
      get_new_state(State_op::Console);
      apply(*op, existing_set(target).linear(), pgn);
    }
    else {
      named_set(args[0]);      // check the name
      get_new_state(State_op::Console);
      apply(*op, existing_set(target), named_set(args[0]));
    }
  }
  log_statements({to_python(aStatement)});
  return QString();
}

void MainWindow::console_print(const QString& aText) { if (m_console_output) m_console_output->appendPlainText(aText); }

void MainWindow::run_console_line(const QString& aLine) {
  console_print(">>> " + aLine);
  if (! aLine.trimmed().isEmpty()) {
    m_console_history << aLine;
  }
  m_console_history_pos = m_console_history.size();
  if (m_curve_type != Curve_type::Linear) {
    console_print("Error: the console supports linear polygons only");
    return;
  }
  try {
    auto statements = Py_parser(aLine).parse();
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

void MainWindow::save_program() {
  QString file_name = QFileDialog::getSaveFileName(this, "Save Program", "bso_program.py", "Python files (*.py)");
  if (file_name.isEmpty()) return;
  QSaveFile file(file_name);
  if (! file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    show_error("Cannot open " + file_name.toStdString() + " for writing");
    return;
  }
  QTextStream out(&file);
  out << "# Generated by the CGAL 2D Boolean Set Operations demo.\n"
      << "from CGALPY_kerEpec_aos2Seg_bso2_pol2 import Ker, Pol2, Bso2\n\n"
      << "Point_2 = Ker.Point_2\n"
      << "Polygon_2 = Pol2.Polygon_2\n"
      << "Polygon_set_2 = Bso2.Polygon_set_2\n\n";
  for (const auto& line : m_program) out << line.m_text << "\n";
  out.flush();
  if (! file.commit())
    show_error("Cannot write " + file_name.toStdString());
}
