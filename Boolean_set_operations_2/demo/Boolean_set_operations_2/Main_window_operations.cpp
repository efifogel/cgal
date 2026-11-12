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

std::optional<std::size_t> Main_window::checked_bucket(const Column& a_column) const {
  auto buckets = checked_buckets(a_column);
  if (buckets.empty()) return std::nullopt;
  return buckets.front();
}

void Main_window::delete_all() {
  action_complement_h->setChecked(false);
  action_intersection_h->setChecked(false);
  action_union_h->setChecked(false);
  action_difference_h->setChecked(false);
  action_symmetric_difference_h->setChecked(false);
  action_minkowski_sum_h->setChecked(false);

  get_new_state(State_op::DELETE_ALL);
  QStringList program;
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    if (! states_stack.back().set(i).is_empty()) program << QString("%1.clear()").arg(bucket_name(i));
  }
  log_statements(program);

  for (std::size_t i = 0; i < number_of_buckets; ++i) states_stack.back().clear(i);
  delete_result();
  model_changed();
}

void Main_window::delete_result() {
  states_stack.back().clear(result_index);
  model_changed();
}

void Main_window::apply_clear(bool a_checked) {
  if (! a_checked) return;
  action_complement_h->setChecked(false);
  action_intersection_h->setChecked(false);
  action_union_h->setChecked(false);
  action_difference_h->setChecked(false);
  action_symmetric_difference_h->setChecked(false);
  action_minkowski_sum_h->setChecked(false);

  auto buckets = checked_buckets(m_clear_column);
  get_new_state(State_op::SO_CLEAR);
  QStringList program;
  for (auto i : buckets) program << QString("%1.clear()").arg(bucket_name(i));
  log_statements(program);

  for (auto i : buckets) states_stack.back().clear(i);
  if (buckets.empty()) show_error("Please select color bucket to be cleaned!!!");

  action_clear_h->setChecked(false);
  model_changed();
}

void Main_window::apply_complement(bool /* a_checked */) {
  if (! action_complement_h->isChecked()) return;
  auto operands = checked_buckets(m_complement_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(action_complement_h, is_operand)) return;
  uncheck_operations_except(action_complement_h);
  run_operation(Bucket_op::BO_COMPLEMENT, operands, action_complement_h);
}

void Main_window::apply_intersection(bool /* a_checked */) {
  if (! action_intersection_h->isChecked()) return;
  auto operands = checked_buckets(m_intersection_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(action_intersection_h, is_operand)) return;
  uncheck_operations_except(action_intersection_h);
  run_operation(Bucket_op::BO_INTERSECTION, operands, action_intersection_h);
}

void Main_window::apply_difference(bool /* a_checked */) {
  if (! action_difference_h->isChecked()) return;
  auto operands = checked_buckets(m_difference_column);
  if (operands.size() != 2) {
    action_difference_h->setChecked(false);
    ask_user_ok("Operation Error", "The difference takes exactly two buckets.\n");
    return;
  }
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(action_difference_h, is_operand)) return;
  uncheck_operations_except(action_difference_h);
  run_operation(Bucket_op::BO_DIFFERENCE, operands, action_difference_h);
}

void Main_window::apply_symmetric_difference(bool /* a_checked */) {
  if (! action_symmetric_difference_h->isChecked()) return;
  auto operands = checked_buckets(m_symmetric_difference_column);
  if (operands.size() != 2) {
    action_symmetric_difference_h->setChecked(false);
    ask_user_ok("Operation Error", "The symmetric difference takes exactly two buckets.\n");
    return;
  }
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(action_symmetric_difference_h, is_operand)) return;
  uncheck_operations_except(action_symmetric_difference_h);
  run_operation(Bucket_op::SYMMETRIC_DIFFERENCE, operands, action_symmetric_difference_h);
}

void Main_window::apply_union(bool /* a_checked */) {
  if (! action_union_h->isChecked()) return;
  auto operands = checked_buckets(m_union_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(action_union_h, is_operand)) return;
  uncheck_operations_except(action_union_h);
  run_operation(Bucket_op::BO_JOIN, operands, action_union_h);
}

void Main_window::apply_copy(bool a_checked) {
  if (action_copy_h->isChecked()) {
    auto source = checked_bucket(m_copy_column);
    if (abort_for_draw_bucket(action_copy_h, source == m_color_active)) return;
    bool l_done = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    action_union_h->setChecked(false);
    action_intersection_h->setChecked(false);
    action_difference_h->setChecked(false);
    action_symmetric_difference_h->setChecked(false);
    action_minkowski_sum_h->setChecked(false);
    action_move_h->setChecked(false);

    get_new_state(State_op::SO_COPY);
    if (source)
      log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), bucket_name(*source))});

    states_stack.back().clear(result_index);

    if (! source) show_warning("Please select a Bucket!!!");
    else if (! states_stack.back().set(*source).is_empty())
      states_stack.back().result_set().assign(states_stack.back().set(*source));

    action_copy_h->setChecked(false);
    l_done = true;
    if (! states_stack.back().result_set().is_empty())
      states_stack.back().set(m_color_active).join(states_stack.back().result_set());
    states_stack.back().clear(result_index);
    this->setCursor(old);
    if (l_done) model_changed();
  }
}

void Main_window::apply_move(bool a_checked) {
  if (action_move_h->isChecked()) {
    auto source = checked_bucket(m_move_column);
    if (abort_for_draw_bucket(action_move_h, source == m_color_active)) return;
    bool l_done = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    action_union_h->setChecked(false);
    action_intersection_h->setChecked(false);
    action_difference_h->setChecked(false);
    action_symmetric_difference_h->setChecked(false);
    action_minkowski_sum_h->setChecked(false);
    action_copy_h->setChecked(false);

    get_new_state(State_op::SO_MOVE);
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

    action_move_h->setChecked(false);
    states_stack.back().clear(result_index);

    l_done = true;
    this->setCursor(old);
    if (l_done) model_changed();
  }
}

Polygon_with_holes_2 Main_window::get_mink_input_polygon(std::size_t a_index) {
  const auto& lps = states_stack.back().set(a_index).linear();
  if (lps.number_of_polygons_with_holes() > 1) m_disjoint = true;
  if (m_disjoint) return Polygon_with_holes_2();

  // Converts a boundary to a polygon of a given orientation.
  auto to_polygon = [](const Linear_polygon& a_boundary, CGAL::Orientation a_orientation) {
    Polygon_2 pgn;
    for (auto cit = a_boundary.curves_begin(); cit != a_boundary.curves_end(); ++cit)
      pgn.push_back(Kernel::Point_2(CGAL::to_double(cit->source().x()), CGAL::to_double(cit->source().y())));
    if (pgn.orientation() == CGAL::opposite(a_orientation)) pgn.reverse_orientation();
    return pgn;
  };
  std::vector<Linear_polygon_with_holes> pwhs;
  lps.polygons_with_holes(std::back_inserter(pwhs));
  const auto& pwh = pwhs.front();
  std::vector<Polygon_2> holes;
  for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) holes.push_back(to_polygon(*hit, CGAL::CLOCKWISE));
  return Polygon_with_holes_2(to_polygon(pwh.outer_boundary(), CGAL::COUNTERCLOCKWISE), holes.begin(), holes.end());
}

void Main_window::apply_minkowski_sum(bool a_checked) {
  if (action_minkowski_sum_h->isChecked()) {
    if (abort_for_draw_bucket(action_minkowski_sum_h,
                              draw_row_checked(m_minkowski_sum_column)))
      return;
    bool l_done = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    if (m_curve_type == Curve_type::CT_LINEAR) {
      action_complement_h->setChecked(false);
      action_union_h->setChecked(false);
      action_intersection_h->setChecked(false);
      action_difference_h->setChecked(false);
      action_symmetric_difference_h->setChecked(false);

      get_new_state(State_op::MINKOWSKI_SUM);
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
          l_done = true;
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

    action_minkowski_sum_h->setChecked(false);
    this->setCursor(old);
    if (l_done) {
      model_changed();
    }
  }
}

bool Main_window::show_not_empty_warning() {
  return show_message(QMessageBox::Information, "Warning: Bucket Not Empty", "The Draw/Insert bucket is not empty. "
                      "Add the result to it?", action_warn_not_empty, QMessageBox::Ok | QMessageBox::Cancel) ==
    QMessageBox::Ok;
}

void Main_window::show_operand_error() {
  show_message(QMessageBox::Critical, "Error: Operation Aborted", "The result cannot be stored in a non-empty operand.",
               action_error_operand);
}

void Main_window::
limit_to_all_but_one(const Column& a_column, QAbstractButton* a_box, bool a_checked,
                     QAbstractButton* a_operation) {
  if (a_checked && ! a_box->isHidden()) {
    bool all_checked = std::all_of(a_column.begin(), a_column.end(), [](QAbstractButton* box) {
      return box->isHidden() || box->isChecked();
    });
    if (all_checked) {
      a_box->setChecked(false);  // toggles again, and updates the operation
      return;
    }
  }
  bool any_checked = std::any_of(a_column.begin(), a_column.end(), [](QAbstractButton* box) {
    return ! box->isHidden() && box->isChecked();
  });
  a_operation->setEnabled(any_checked);
}

void Main_window::
keep_one_unchecked(const Column& a_column) {
  QAbstractButton* last = nullptr;
  for (auto* box : a_column) {
    if (box->isHidden()) continue;
    if (! box->isChecked()) return;
    last = box;
  }
  if (last) last->setChecked(false);
}

void Main_window::
limit_to_two_operands(const Column& a_column, QAbstractButton* a_box, bool a_checked,
                      QAbstractButton* a_operation) {
  auto count = std::count_if(a_column.begin(), a_column.end(), [](QAbstractButton* box) { return box->isChecked(); });
  if (a_checked && (count > 2)) {
    a_box->setChecked(false);    // toggles again, and updates the operation
    return;
  }
  a_operation->setEnabled(count == 2);
}

std::vector<std::size_t> Main_window::
checked_buckets(const Column& a_column) const {
  std::vector<std::size_t> buckets;
  for (std::size_t i = 0; i < a_column.size(); ++i) {
    if (! a_column[i]->isHidden() && a_column[i]->isChecked())
      buckets.push_back(i);
  }
  return buckets;
}

void Main_window::uncheck_operations_except(QAbstractButton* a_operation) {
  for (QAbstractButton* button : {static_cast<QAbstractButton*>(action_complement_h),
          static_cast<QAbstractButton*>(action_intersection_h), static_cast<QAbstractButton*>(action_union_h),
          static_cast<QAbstractButton*>(action_difference_h),
          static_cast<QAbstractButton*>(action_symmetric_difference_h),
          static_cast<QAbstractButton*>(action_minkowski_sum_h), static_cast<QAbstractButton*>(action_copy_h),
          static_cast<QAbstractButton*>(action_move_h)}) {
    if (button != a_operation) button->setChecked(false);
  }
}

void Main_window::clear_result() {
  auto& state = states_stack.back();
  state.clear(result_index);
}

void Main_window::run_operation(Bucket_op a_op, const std::vector<std::size_t>& a_operands,
                                QAbstractButton* a_operation) {
  if (a_operands.empty() || ((a_op == Bucket_op::BO_COMPLEMENT) && (a_operands.size() != 1))) {
    a_operation->setChecked(false);
    return;
  }

  State_op code = State_op::SO_COMPLEMENT;
  switch (a_op) {
   case Bucket_op::BO_COMPLEMENT: code = State_op::SO_COMPLEMENT; break;
   case Bucket_op::BO_INTERSECTION: code = State_op::SO_INTERSECTION; break;
   case Bucket_op::BO_JOIN: code = State_op::SO_UNION; break;
   case Bucket_op::BO_DIFFERENCE: code = State_op::SO_DIFFERENCE; break;
   case Bucket_op::SYMMETRIC_DIFFERENCE: code = State_op::SYMMETRIC_DIFFERENCE; break;
  }
  const QString name = method_name(a_op);

  QCursor old = this->cursor();
  this->setCursor(Qt::WaitCursor);
  get_new_state(code);
  clear_result();
  auto& state = states_stack.back();
  auto bucket = [&state](std::size_t a_index) -> Curve_set& { return state.set(a_index); };
  Curve_set& draw_bucket = state.set(m_color_active);
  bool direct = draw_bucket.is_empty() && (std::find(a_operands.begin(), a_operands.end(), m_color_active) ==
     a_operands.end());
  Curve_set& result = direct ? draw_bucket : state.result_set();
  const QString target = direct ? bucket_name(m_color_active) : "result";
  QStringList program;

  if (a_op == Bucket_op::BO_COMPLEMENT) {
    result.complement(bucket(a_operands[0]));
    if (! direct) program << "result = Polygon_set_2()";
    program << QString("%1.complement(%2)")
      .arg(target, bucket_name(a_operands[0]));
  }
  else if (a_operands.size() == 1) {
    if (direct) {
      result.join(bucket(a_operands[0]));
      program << QString("%1.join(%2)").arg(target, bucket_name(a_operands[0]));
    }
    else {
      result.assign(bucket(a_operands[0]));
      program << QString("result = Polygon_set_2(%1)")
        .arg(bucket_name(a_operands[0]));
    }
  }
  else {
    apply(a_op, result, bucket(a_operands[0]), bucket(a_operands[1]));
    if (! direct) program << "result = Polygon_set_2()";
    program << QString("%1.%2(%3, %4)").arg(target, name, bucket_name(a_operands[0]), bucket_name(a_operands[1]));
    for (std::size_t i = 2; i < a_operands.size(); ++i) {
      apply(a_op, result, bucket(a_operands[i]));
      program << QString("%1.%2(%3)").arg(target, name, bucket_name(a_operands[i]));
    }
  }

  if (! direct) {
    draw_bucket.join(result);
    program << QString("%1.join(result)").arg(bucket_name(m_color_active));
  }
  clear_result();

  a_operation->setChecked(false);
  this->setCursor(old);
  log_statements(program);
  model_changed();
}

bool Main_window::
draw_row_checked(const Column& a_column) const
{ return (m_color_active < a_column.size()) && a_column[m_color_active]->isChecked(); }

bool Main_window::abort_for_draw_bucket(QAbstractButton* a_operation, bool a_is_operand) {
  if (states_stack.back().set(m_color_active).is_empty()) return false;
  if (a_is_operand) {
    a_operation->setChecked(false);
    if (operand_error_warn) show_operand_error();
    return true;
  }
  if (empty_warn && ! show_not_empty_warning()) {
    a_operation->setChecked(false);
    return true;
  }
  return false;
}
