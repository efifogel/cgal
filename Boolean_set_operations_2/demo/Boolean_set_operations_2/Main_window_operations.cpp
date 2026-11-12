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

/*! \file Main_window_operations.cpp
 * The operations of the main window on the buckets: the Boolean operations, the Minkowski sum, copy, move, and clear,
 * and the marks of the Operations panel that select their operands.
 */

#include "Main_window.h"

std::optional<std::size_t> MainWindow::checked_bucket(const Column& aColumn) const {
  auto buckets = checked_buckets(aColumn);
  if (buckets.empty()) return std::nullopt;
  return buckets.front();
}

void MainWindow::delete_all() {
  actionComplementH->setChecked(false);
  actionIntersectionH->setChecked(false);
  actionUnionH->setChecked(false);
  actionDifferenceH->setChecked(false);
  actionSymmetric_DifferenceH->setChecked(false);
  actionMinkowski_SumH->setChecked(false);

  get_new_state(State_op::Delete_all);
  QStringList program;
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    if (! states_stack.back().set(i).is_empty()) program << QString("%1.clear()").arg(bucket_name(i));
  }
  log_statements(program);

  for (std::size_t i = 0; i < number_of_buckets; ++i) states_stack.back().clear(i);
  delete_result();
  model_changed();
}

void MainWindow::delete_result() {
  states_stack.back().clear(result_index);
  model_changed();
}

void MainWindow::apply_clear(bool aChecked) {
  if (! aChecked) return;
  actionComplementH->setChecked(false);
  actionIntersectionH->setChecked(false);
  actionUnionH->setChecked(false);
  actionDifferenceH->setChecked(false);
  actionSymmetric_DifferenceH->setChecked(false);
  actionMinkowski_SumH->setChecked(false);

  auto buckets = checked_buckets(m_clear_column);
  get_new_state(State_op::Clear);
  QStringList program;
  for (auto i : buckets) program << QString("%1.clear()").arg(bucket_name(i));
  log_statements(program);

  for (auto i : buckets) states_stack.back().clear(i);
  if (buckets.empty()) show_error("Please select color bucket to be cleaned!!!");

  actionClearH->setChecked(false);
  model_changed();
}

void MainWindow::apply_complement(bool /* aChecked */) {
  if (! actionComplementH->isChecked()) return;
  auto operands = checked_buckets(m_complement_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionComplementH, is_operand)) return;
  uncheck_operations_except(actionComplementH);
  run_operation(Bucket_op::Complement, operands, actionComplementH);
}

void MainWindow::apply_intersection(bool /* aChecked */) {
  if (! actionIntersectionH->isChecked()) return;
  auto operands = checked_buckets(m_intersection_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionIntersectionH, is_operand)) return;
  uncheck_operations_except(actionIntersectionH);
  run_operation(Bucket_op::Intersection, operands, actionIntersectionH);
}

void MainWindow::apply_difference(bool /* aChecked */) {
  if (! actionDifferenceH->isChecked()) return;
  auto operands = checked_buckets(m_difference_column);
  if (operands.size() != 2) {
    actionDifferenceH->setChecked(false);
    ask_user_ok("Operation Error", "The difference takes exactly two buckets.\n");
    return;
  }
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionDifferenceH, is_operand)) return;
  uncheck_operations_except(actionDifferenceH);
  run_operation(Bucket_op::Difference, operands, actionDifferenceH);
}

void MainWindow::apply_symmetric_difference(bool /* aChecked */) {
  if (! actionSymmetric_DifferenceH->isChecked()) return;
  auto operands = checked_buckets(m_symmetric_difference_column);
  if (operands.size() != 2) {
    actionSymmetric_DifferenceH->setChecked(false);
    ask_user_ok("Operation Error", "The symmetric difference takes exactly two buckets.\n");
    return;
  }
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionSymmetric_DifferenceH, is_operand)) return;
  uncheck_operations_except(actionSymmetric_DifferenceH);
  run_operation(Bucket_op::Symmetric_difference, operands, actionSymmetric_DifferenceH);
}

void MainWindow::apply_union(bool /* aChecked */) {
  if (! actionUnionH->isChecked()) return;
  auto operands = checked_buckets(m_union_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionUnionH, is_operand)) return;
  uncheck_operations_except(actionUnionH);
  run_operation(Bucket_op::Join, operands, actionUnionH);
}

void MainWindow::apply_copy(bool aChecked) {
  if (actionCopyH->isChecked()) {
    auto source = checked_bucket(m_copy_column);
    if (abort_for_draw_bucket(actionCopyH, source == m_color_active)) return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    actionUnionH->setChecked(false);
    actionIntersectionH->setChecked(false);
    actionDifferenceH->setChecked(false);
    actionSymmetric_DifferenceH->setChecked(false);
    actionMinkowski_SumH->setChecked(false);
    actionMoveH->setChecked(false);

    get_new_state(State_op::Copy);
    if (source)
      log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), bucket_name(*source))});

    states_stack.back().clear(result_index);

    if (! source) show_warning("Please select a Bucket!!!");
    else if (! states_stack.back().set(*source).is_empty())
      states_stack.back().result_set().assign(states_stack.back().set(*source));

    actionCopyH->setChecked(false);
    lDone = true;
    if (! states_stack.back().result_set().is_empty())
      states_stack.back().set(m_color_active).join(states_stack.back().result_set());
    states_stack.back().clear(result_index);
    this->setCursor(old);
    if (lDone) model_changed();
  }
}

void MainWindow::apply_move(bool aChecked) {
  if (actionMoveH->isChecked()) {
    auto source = checked_bucket(m_move_column);
    if (abort_for_draw_bucket(actionMoveH, source == m_color_active)) return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    actionUnionH->setChecked(false);
    actionIntersectionH->setChecked(false);
    actionDifferenceH->setChecked(false);
    actionSymmetric_DifferenceH->setChecked(false);
    actionMinkowski_SumH->setChecked(false);
    actionCopyH->setChecked(false);

    get_new_state(State_op::Move);
    if (source)
      log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), bucket_name(*source)),
                      QString("%1.clear()").arg(bucket_name(*source))});

    states_stack.back().clear(result_index);

    if (! source) show_warning("Please select a Bucket!!!");
    else if (! states_stack.back().set(*source).is_empty()) {
      auto& source_set = states_stack.back().set(*source);
      states_stack.back().result_set().assign(source_set);
      states_stack.back().set(m_color_active).join(states_stack.back().result_set());
      source_set.clear();
    }

    actionMoveH->setChecked(false);
    states_stack.back().clear(result_index);

    lDone = true;
    this->setCursor(old);
    if (lDone) model_changed();
  }
}

Polygon_with_holes_2 MainWindow::get_mink_input_polygon(std::size_t aIndex) {
  const auto& lps = states_stack.back().set(aIndex).linear();
  if (lps.number_of_polygons_with_holes() > 1) m_disjoint = true;
  if (m_disjoint) return Polygon_with_holes_2();

  // Converts a boundary to a polygon of a given orientation.
  auto to_polygon = [](const Linear_polygon& aBoundary, CGAL::Orientation aOrientation) {
    Polygon_2 pgn;
    for (auto cit = aBoundary.curves_begin(); cit != aBoundary.curves_end(); ++cit)
      pgn.push_back(Kernel::Point_2(CGAL::to_double(cit->source().x()), CGAL::to_double(cit->source().y())));
    if (pgn.orientation() == CGAL::opposite(aOrientation)) pgn.reverse_orientation();
    return pgn;
  };
  std::vector<Linear_polygon_with_holes> pwhs;
  lps.polygons_with_holes(std::back_inserter(pwhs));
  const auto& pwh = pwhs.front();
  std::vector<Polygon_2> holes;
  for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) holes.push_back(to_polygon(*hit, CGAL::CLOCKWISE));
  return Polygon_with_holes_2(to_polygon(pwh.outer_boundary(), CGAL::COUNTERCLOCKWISE), holes.begin(), holes.end());
}

void MainWindow::apply_minkowski_sum(bool aChecked) {
  if (actionMinkowski_SumH->isChecked()) {
    if (abort_for_draw_bucket(actionMinkowski_SumH,
                              draw_row_checked(m_minkowski_sum_column)))
      return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    if (m_curve_type == Curve_type::Linear) {
      actionComplementH->setChecked(false);
      actionUnionH->setChecked(false);
      actionIntersectionH->setChecked(false);
      actionDifferenceH->setChecked(false);
      actionSymmetric_DifferenceH->setChecked(false);

      get_new_state(State_op::Minkowski_sum);
      log_statements({"# Minkowski sum (not supported in the program yet)"});

      // The two checked buckets, in order.
      auto operands = checked_buckets(m_minkowski_sum_column);

      if (operands.size() == 2) {
        Polygon_with_holes_2 p0, p1;
        if (! states_stack.back().set(operands[0]).is_empty()) {
          p0 = get_mink_input_polygon(operands[0]);
          if (! states_stack.back().set(operands[1]).is_empty()) p1 = get_mink_input_polygon(operands[1]);
        }

        if (! m_disjoint) {
          Polygon_with_holes_2 mink_sum_res = CGAL::minkowski_sum_2(p0, p1);
          if (! mink_sum_res.is_unbounded()) {
            m_linear_input->m_is_mink = true;
            m_linear_input->get_minkowski_result(mink_sum_res, p0);
            m_linear_input->m_hole = true;
            m_linear_input->get_minkowski_holes(mink_sum_res, p0);
            states_stack.back().result_set().clear();
            m_linear_input->m_hole = false;
            m_linear_input->m_is_mink = false;
          }
          else ask_user_ok("Minkowski Sum Operation Error", "resultant polygon is unbounded\n");
          lDone = true;
          minkowski_sum_operated = true;
        }
        else {
          undo();
          show_error("DISJOINT POLYGON SET ERROR\n\nCannot perform Minkowski Sum operation since Input Bucket contains "
                     "more than disjoint polygons!!!");
          m_disjoint = false;
        }
      }
      else {
        ask_user_ok("Minkowski Sum Operation Error", "Function supports 2 polygon as input\n");
      }
    }

    actionMinkowski_SumH->setChecked(false);
    this->setCursor(old);
    if (lDone) {
      model_changed();
    }
  }
}

bool MainWindow::show_not_empty_warning() {
  return show_message(QMessageBox::Information, "Warning: Bucket Not Empty", "The Draw/Insert bucket is not empty. "
                      "Add the result to it?", actionWarnNotEmpty, QMessageBox::Ok | QMessageBox::Cancel) ==
    QMessageBox::Ok;
}

void MainWindow::show_operand_error() {
  show_message(QMessageBox::Critical, "Error: Operation Aborted", "The result cannot be stored in a non-empty operand.",
               actionErrorOperand);
}

void MainWindow::
limit_to_all_but_one(const Column& aColumn, QAbstractButton* aBox, bool aChecked,
                     QAbstractButton* aOperation) {
  if (aChecked && ! aBox->isHidden()) {
    bool all_checked = std::all_of(aColumn.begin(), aColumn.end(), [](QAbstractButton* box) {
      return box->isHidden() || box->isChecked();
    });
    if (all_checked) {
      aBox->setChecked(false);  // toggles again, and updates the operation
      return;
    }
  }
  bool any_checked = std::any_of(aColumn.begin(), aColumn.end(), [](QAbstractButton* box) {
    return ! box->isHidden() && box->isChecked();
  });
  aOperation->setEnabled(any_checked);
}

void MainWindow::
keep_one_unchecked(const Column& aColumn) {
  QAbstractButton* last = nullptr;
  for (auto* box : aColumn) {
    if (box->isHidden()) continue;
    if (! box->isChecked()) return;
    last = box;
  }
  if (last) last->setChecked(false);
}

void MainWindow::
limit_to_two_operands(const Column& aColumn, QAbstractButton* aBox, bool aChecked,
                      QAbstractButton* aOperation) {
  auto count = std::count_if(aColumn.begin(), aColumn.end(), [](QAbstractButton* box) { return box->isChecked(); });
  if (aChecked && (count > 2)) {
    aBox->setChecked(false);    // toggles again, and updates the operation
    return;
  }
  aOperation->setEnabled(count == 2);
}

std::vector<std::size_t> MainWindow::
checked_buckets(const Column& aColumn) const {
  std::vector<std::size_t> buckets;
  for (std::size_t i = 0; i < aColumn.size(); ++i) {
    if (! aColumn[i]->isHidden() && aColumn[i]->isChecked())
      buckets.push_back(i);
  }
  return buckets;
}

void MainWindow::uncheck_operations_except(QAbstractButton* aOperation) {
  for (QAbstractButton* button : {static_cast<QAbstractButton*>(actionComplementH),
          static_cast<QAbstractButton*>(actionIntersectionH), static_cast<QAbstractButton*>(actionUnionH),
          static_cast<QAbstractButton*>(actionDifferenceH), static_cast<QAbstractButton*>(actionSymmetric_DifferenceH),
          static_cast<QAbstractButton*>(actionMinkowski_SumH), static_cast<QAbstractButton*>(actionCopyH),
          static_cast<QAbstractButton*>(actionMoveH)}) {
    if (button != aOperation) button->setChecked(false);
  }
}

void MainWindow::clear_result() {
  auto& state = states_stack.back();
  state.clear(result_index);
}

void MainWindow::run_operation(Bucket_op aOp, const std::vector<std::size_t>& aOperands, QAbstractButton* aOperation) {
  if (aOperands.empty() || ((aOp == Bucket_op::Complement) && (aOperands.size() != 1))) {
    aOperation->setChecked(false);
    return;
  }

  State_op code = State_op::Complement;
  switch (aOp) {
   case Bucket_op::Complement: code = State_op::Complement; break;
   case Bucket_op::Intersection: code = State_op::Intersection; break;
   case Bucket_op::Join: code = State_op::Union; break;
   case Bucket_op::Difference: code = State_op::Difference; break;
   case Bucket_op::Symmetric_difference: code = State_op::Symmetric_difference; break;
  }
  const QString name = method_name(aOp);

  QCursor old = this->cursor();
  this->setCursor(Qt::WaitCursor);
  get_new_state(code);
  clear_result();
  auto& state = states_stack.back();
  auto bucket = [&state](std::size_t aIndex) -> Curve_set& { return state.set(aIndex); };
  Curve_set& draw_bucket = state.set(m_color_active);
  bool direct = draw_bucket.is_empty() && (std::find(aOperands.begin(), aOperands.end(), m_color_active) ==
     aOperands.end());
  Curve_set& result = direct ? draw_bucket : state.result_set();
  const QString target = direct ? bucket_name(m_color_active) : "result";
  QStringList program;

  if (aOp == Bucket_op::Complement) {
    result.complement(bucket(aOperands[0]));
    if (! direct) program << "result = Polygon_set_2()";
    program << QString("%1.complement(%2)")
      .arg(target, bucket_name(aOperands[0]));
  }
  else if (aOperands.size() == 1) {
    if (direct) {
      result.join(bucket(aOperands[0]));
      program << QString("%1.join(%2)").arg(target, bucket_name(aOperands[0]));
    }
    else {
      result.assign(bucket(aOperands[0]));
      program << QString("result = Polygon_set_2(%1)")
        .arg(bucket_name(aOperands[0]));
    }
  }
  else {
    apply(aOp, result, bucket(aOperands[0]), bucket(aOperands[1]));
    if (! direct) program << "result = Polygon_set_2()";
    program << QString("%1.%2(%3, %4)").arg(target, name, bucket_name(aOperands[0]), bucket_name(aOperands[1]));
    for (std::size_t i = 2; i < aOperands.size(); ++i) {
      apply(aOp, result, bucket(aOperands[i]));
      program << QString("%1.%2(%3)").arg(target, name, bucket_name(aOperands[i]));
    }
  }

  if (! direct) {
    draw_bucket.join(result);
    program << QString("%1.join(result)").arg(bucket_name(m_color_active));
  }
  clear_result();

  aOperation->setChecked(false);
  this->setCursor(old);
  log_statements(program);
  model_changed();
}

bool MainWindow::
draw_row_checked(const Column& aColumn) const
{ return (m_color_active < aColumn.size()) && aColumn[m_color_active]->isChecked(); }

bool MainWindow::abort_for_draw_bucket(QAbstractButton* aOperation, bool aIsOperand) {
  if (states_stack.back().set(m_color_active).is_empty()) return false;
  if (aIsOperand) {
    aOperation->setChecked(false);
    if (operand_error_warn) show_operand_error();
    return true;
  }
  if (empty_warn && ! show_not_empty_warning()) {
    aOperation->setChecked(false);
    return true;
  }
  return false;
}
