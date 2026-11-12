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

#ifndef CGAL_MAIN_WINDOW_H
#define CGAL_MAIN_WINDOW_H

/*! \file Main_window.h
 * The main window of the demo (MainWindow): the Operations panel with the buckets, the curve families and their inputs,
 * the undo stack, the console, and the program log.
 */

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <limits>
#include <optional>
#include <functional>
#include <type_traits>
#include <vector>

#include <QApplication>
#include <qmessagebox.h>

#include <QMainWindow>
#include <QPointer>
#include <QGraphicsScene>
#include <QToolButton>
#include <QActionGroup>
#include <QPainterPath>
#include <QToolBar>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <cmath>
#include <QSize>
#include <QPen>
#include <QtGui>
#include <QString>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QFontDatabase>
#include <QSaveFile>
#include <charconv>
#include <QLineEdit>
#include <QRegularExpression>
#include <algorithm>
#include <map>
#include <memory>
#include <QInputDialog>
#include <QWheelEvent>
#include <QPainter>
#include <QSlider>
#include <QGraphicsLineItem>
#include <QProgressBar>
#include <QMessageBox>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QTimer>
#include <QDockWidget>
#include <array>
#include <QButtonGroup>
#include <QPushButton>
#include <QDialog>
#include <QBoxLayout>
#include <QStyle>
#include <QProxyStyle>
#include <QStyleOption>
#include <QMouseEvent>
#include <QWindow>
#include <QGuiApplication>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/auto_link/Qt.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Point_2.h>
#include <CGAL/number_utils.h>
#include <CGAL/Cartesian_converter.h>
#include <CGAL/Timer.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/iterator.h>
#include <CGAL/assertions_behaviour.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/General_polygon_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Arr_Bezier_curve_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_circle_segment_traits_2.h>
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/Iso_rectangle_2.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/minkowski_sum_2.h>
#include <CGAL/Small_side_angle_bisector_decomposition_2.h>
#include <CGAL/approximated_offset_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/squared_distance_2.h>

#ifdef CGAL_USE_GMP
#include <CGAL/Gmpq.h>
#else
#include <CGAL/MP_Float.h>
#include <CGAL/Quotient.h>
#endif

#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Qt/DemosMainWindow.h>
#include <CGAL/Qt/utility.h>
#include <CGAL/IO/Dxf_bsop_reader.h>
#include <CGAL/Qt/GraphicsViewNavigation.h>

#include "Qt6/Circular_polygons.h"
#include "Qt6/Linear_polygons.h"
#include "Qt6/Bezier_curves.h"
#include "Qt6/Polyline_curves.h"
#include "Qt6/Graphics_view_circular_polygon_input.h"
#include "Qt6/Graphics_view_linear_polygon_input.h"
#include "Qt6/Graphics_view_polyline_input.h"
#include "Qt6/Graphics_view_bezier_polygon_input.h"
#include "Qt6/Conic_curves.h"
#include "Qt6/Graphics_view_conic_input.h"

#include "ui_Boolean_set_operations_2.h"

#include "Typedefs.h"

#include "Error_handling.h"
#include "Buckets.h"
#include "Curve_set.h"
#include "Bucket_operations.h"
#include "State.h"
#include "Qt_helpers.h"
#include "Python.h"

// The main window.
class MainWindow : public CGAL::Qt::DemosMainWindow, public Ui::Boolean_set_operations_2 {
  Q_OBJECT

private:
  Grid_scene m_scene;
  // The cells of the bucket rows of the Operations panel, by column, and the other widgets of the rows.
  using Column = std::array<QAbstractButton*, number_of_buckets>;
  Column m_show_column;                         // Visible
  Column m_draw_column;                         // Draw/Insert
  Column m_complement_column;
  Column m_intersection_column;
  Column m_union_column;
  Column m_difference_column;
  Column m_symmetric_difference_column;
  Column m_minkowski_sum_column;
  Column m_copy_column;
  Column m_move_column;
  Column m_clear_column;
  std::array<QLabel*, number_of_buckets> m_bucket_labels;
  std::array<QFrame*, number_of_buckets> m_bucket_separators;   // the lines below the rows (null for the initial rows)
  std::size_t m_number_of_buckets = min_number_of_buckets;     // the existing buckets are the first ones

  // The program: the statements executed so far, each tagged with the size of the undo stack after its action, so that
  // undo can drop them.
  struct Program_line {
    std::size_t m_state_count;
    QString m_text;
  };
  std::vector<Program_line> m_program;
  QPlainTextEdit* m_log = nullptr;
  bool m_logging = true;      // whether actions are logged

  // The console: its output, its input line and history, and the polygon
  // sets (variables) other than the buckets, e.g., result.
  QPlainTextEdit* m_console_output = nullptr;
  QLineEdit* m_console_input = nullptr;
  QStringList m_console_history;
  int m_console_history_pos = 0;
  std::map<QString, Curve_set> m_variables;
  Curve_type m_curve_type = Curve_type::Linear;     // the curve family of the sets

  // A curve family: its tool-bar action, the input that constructs its curves, and its name; and if its polygons can
  // be loaded, the action that does it, the extension of their files, and the reader.
  struct Curve_family {
    Curve_type m_type;
    const char* m_name;
    QAction* m_insert_action;
    CGAL::Qt::GraphicsViewInput* m_input;
    std::function<void()> m_reset_input;
    QAction* m_open_action;
    const char* m_extension;
    bool (MainWindow::*m_read)(QString);
  };
  std::array<Curve_family, number_of_curve_types> m_families;

  size_t m_color_active;           // the Draw/Insert bucket
  bool empty_warn;
  bool operand_error_warn;
  bool m_grid;
  bool m_pan;
  bool m_reset;
  bool minkowski_sum_operated;
  bool m_disjoint;

  size_t m_state_num;

  // The axes. (Declared after the scene, so that they are deleted first, removing themselves from the scene if shown.)
  std::unique_ptr<QGraphicsLineItem> xAxis = std::make_unique<QGraphicsLineItem>();
  std::unique_ptr<QGraphicsLineItem> yAxis = std::make_unique<QGraphicsLineItem>();

  state_container states_stack;
  // The graphics items in the scene (null once deleted along with their sets).
  std::array<QPointer<CGAL::Qt::GraphicsItem>, number_of_buckets + 1> m_linked_items;

  CGAL::Qt::Graphics_view_linear_polygon_input<Kernel>* m_linear_input;
  CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>* m_circular_input;
  CGAL::Qt::Graphics_view_bezier_polygon_input<Bezier_traits>* m_bezier_input;
  CGAL::Qt::Graphics_view_polyline_input<Kernel>* m_polyline_input;
  CGAL::Qt::Graphics_view_conic_input* m_conic_input;

public:
  /*! constructs the main window: the scene, the panels, the tool bars, and the inputs.
   */
  MainWindow();

private:
  /*! fits the view to the curves of all buckets.
   */
  void zoom_to_fit();

protected slots:
  /*! loads polygons from a file into the Draw/Insert bucket; the file type is determined by its extension.
   */
  void open(QString filename);

public slots:
  /*! adds a polygon constructed by one of the inputs to the Draw/Insert bucket.
   */
  void process_input(CGAL::Object o);

  /*! undoes the last action.
   */
  void undo();

  /*! clears all buckets and starts a new program.
   */
  void new_program();

  /*! shows or hides the axes.
   */
  void toggle_axes();

  /*! adds the complement of the marked bucket to the Draw/Insert bucket.
   */
  void apply_complement(bool aChecked);

  /*! adds the union of the marked buckets to the Draw/Insert bucket.
   */
  void apply_union(bool aChecked);

  /*! adds the intersection of the marked buckets to the Draw/Insert bucket.
   */
  void apply_intersection(bool aChecked);

  /*! adds the difference of the two marked buckets to the Draw/Insert bucket.
   */
  void apply_difference(bool aChecked);

  /*! adds the symmetric difference of the two marked buckets to the Draw/Insert bucket.
   */
  void apply_symmetric_difference(bool aChecked);

  /*! adds the Minkowski sum of the two marked (linear) buckets to the Draw/Insert bucket.
   */
  void apply_minkowski_sum(bool aChecked);

  /*! shows or hides the Operations panel.
   */
  void show_operations_panel(bool aChecked);

  /*! keeps the Operations menu item in sync with the visibility of its panel.
   */
  void operations_panel_visibility_changed(bool visible);

  /*! keeps the Log menu item in sync with the visibility of its panel.
   */
  void log_panel_visibility_changed(bool visible);

  /*! keeps the Console menu item in sync with the visibility of its panel.
   */
  void console_panel_visibility_changed(bool visible);

  /*! shows or hides the Console panel.
   */
  void show_console_panel(bool aChecked);

  /*! shows or hides the Log panel.
   */
  void show_log_panel(bool aChecked);

  /*! loads linear polygons from a file.
   */
  void open_linear();

  /*! loads polygons bounded by segments and circular arcs from a file.
   */
  void open_circular();

  /*! loads Bezier polygons from a file.
   */
  void open_bezier();

  /*! loads polygons bounded by polylines from a file.
   */
  void open_polyline();

  /*! zooms the view in or out.
   */
  void wheelEvent(QWheelEvent* event);

  /*! saves the Draw/Insert bucket to a file.
   */
  void save_draw_bucket();

  /*! adds a bucket (up to seven).
   */
  void add_bucket();

  /*! removes the last bucket (down to three).
   */
  void remove_bucket();

  /*! copies the marked bucket into the Draw/Insert bucket.
   */
  void apply_copy(bool aChecked);

  /*! moves the marked bucket into the Draw/Insert bucket.
   */
  void apply_move(bool aChecked);

  /*! shows a message with a "Don't show this message again" check box. aToggle is the Edit-menu action that controls
   * whether the message is shown; checking the box unchecks it. Unlike a QMessageBox, OK is always on the left and
   * Cancel (when offered) on the right, and neither is the default button. \return the button the user clicked (Escape
   * and the close box count as Cancel when it is offered, and as OK otherwise).
   */
  QMessageBox::StandardButton
  show_message(QMessageBox::Icon aIcon, const QString& aTitle, const QString& aText, QAction* aToggle,
               QMessageBox::StandardButtons aButtons = QMessageBox::Ok);

  /*! warns that the Draw/Insert bucket, where the result is added, is not empty. \return true if the user chooses to
   * continue.
   */
  bool show_not_empty_warning();

  /*! shows an error: the Draw/Insert bucket is a non-empty operand of the operation.
   */
  void show_operand_error();

  /*! determines whether the Draw/Insert bucket is marked in a given operation column.
   */
  bool draw_row_checked(const Column& aColumn) const;

  /*! obtains the buckets (indices) marked in a given operation column.
   */
  std::vector<std::size_t>
  checked_buckets(const Column& aColumn) const;

  /*! unchecks all operation buttons but a given one.
   */
  void uncheck_operations_except(QAbstractButton* aOperation);

  /*! clears the scratch result set.
   */
  void clear_result();

  /*! applies a Boolean operation to the operand buckets, and adds the result to the Draw/Insert bucket. The first two
   * operands are combined with the bivariate operation, and the remaining ones are folded in one at a time. If the
   * Draw/Insert bucket is empty and not an operand, the result is computed directly into it; otherwise into a
   * temporary, which is then added to it. The executed statements are logged.
   */
  void run_operation(Bucket_op aOp, const std::vector<std::size_t>& aOperands, QAbstractButton* aOperation);

  /*! appends statements to the program, and shows them in the Log. Sticky statements (the creation and deletion of
   * buckets, which are not undoable actions) are never dropped by undo.
   */
  void log_statements(const QStringList& aStatements, bool aSticky = false);

  /*! handles the keys of the console input line: Up and Down browse the history, and Ctrl+<key> combinations go to the
   * input line rather than to the operation shortcuts, except Ctrl+Q.
   */
  bool eventFilter(QObject* aObject, QEvent* aEvent) override;

  /*! sets the area that unbounded sets (e.g., complements) are drawn over to the visible area of the view, enlarged a
   * bit so that the edges of the fill never show.
   */
  void update_unbounded_rect();

  /*! obtains the input that constructs curves of the active family.
   */
  QObject* current_input() const;

  /*! stops constructing curves, and discards those under construction.
   */
  void remove_inputs();

  /*! constructs curves with a given input only.
   */
  void install_input(QObject* aInput);

  /*! sets the pen of the curves under construction.
   */
  void set_input_pen(const QPen& aPen);

  /*! obtains a curve family.
   */
  const Curve_family& family(Curve_type aType) const { return m_families[std::size_t(aType)]; }

  /*! switches to a curve family; if there are polygons of another family, asks the user first. \return true if the
   * family is active.
   */
  bool ensure_curve_type(Curve_type aType);

  /*! starts constructing curves of a given family.
   */
  void start_inserting(Curve_type aType);

  /*! obtains an icon of the curve tool bar: a boundary of a segment and an arc, or a circle with its center and radius.
   */
  QIcon curve_tool_icon(const QString& aKind);

  /*! determines whether the bucket with a given index currently exists (is shown).
   */
  bool bucket_exists(std::size_t aIndex) const;

  /*! obtains the index of the bucket with a given name, or number_of_buckets if there is none.
   */
  std::size_t bucket_index(const QString& aName) const;

  /*! obtains the polygon set (an existing bucket or variable) with a given name, or null.
   */
  Curve_set* polygon_set(const QString& aName);

  /*! obtains the polygon of a Polygon_2([...]) expression. It must be simple and counterclockwise oriented (as the
   * bindings require).
   */
  Linear_polygon_with_holes polygon(const Py_expr& aExpr) const;

  /*! executes a console statement; statements that change a polygon set are undoable actions, and are logged. \return
   * the text to print (for queries), or an empty string. Throws a std::runtime_error with a message on an error.
   */
  QString execute(const Py_statement& aStatement);

  /*! obtains the polygon set (an existing bucket or variable) with a given name. Throws a std::runtime_error if there
   * is none.
   */
  Curve_set& existing_set(const QString& aName);

  /*! obtains the polygon set named by an expression. Throws a std::runtime_error if the expression is not a name of a
   * polygon set.
   */
  Curve_set& named_set(const Py_expr& aExpr);

  /*! executes a console statement that deletes a polygon set: del name.
   */
  void execute_delete(const Py_statement& aStatement);

  /*! executes a console statement that assigns a polygon set: name = Polygon_set_2(...).
   */
  void execute_assign(const Py_statement& aStatement);

  /*! executes a console statement that calls a method of a polygon set: name.method(args). \return the text to print
   * (for queries), or an empty string.
   */
  QString execute_call(const Py_statement& aStatement);

  /*! parses and executes a console line.
   */
  void run_console_line(const QString& aLine);

  /*! prints to the console output.
   */
  void console_print(const QString& aText);

  /*! drops the statements of undone actions (those logged at a deeper undo stack than the current one), and refreshes
   * the Log.
   */
  void trim_program_to_states();

  /*! saves the program as a Python script for the CGALPY bindings.
   */
  void save_program();

  /*! keeps at least one bucket unmarked in the intersection or union column (for the result) when a cell is toggled:
   * checking the last unchecked cell is undone. The operation button is enabled only when at least one cell is checked.
   */
  void limit_to_all_but_one(const Column& aColumn, QAbstractButton* aBox, bool aChecked,
                            QAbstractButton* aOperation);

  /*! restores the invariant of limit_to_all_but_one() after a bucket is removed: if all (remaining) buckets are
   * checked, unchecks the last one.
   */
  void keep_one_unchecked(const Column& aColumn);

  /*! keeps at most two buckets marked in the difference, symmetric difference, or Minkowski-sum column when a cell is
   * toggled: checking a third cell is undone. The operation button is enabled only when exactly two cells are checked.
   */
  void limit_to_two_operands(const Column& aColumn, QAbstractButton* aBox, bool aChecked,
                             QAbstractButton* aOperation);

  /*! checks the Draw/Insert bucket, where every operation stores its result. If it is not empty and an operand, shows
   * an error; if it is not empty otherwise, warns and lets the user cancel. When aborting, unchecks the operation
   * button. \return true if the operation is aborted.
   */
  bool abort_for_draw_bucket(QAbstractButton* aOperation, bool aIsOperand);

  /*! shows or hides the scratch result set.
   */
  void show_result(bool a_check);

  /*! shows or hides all buckets.
   */
  void show_all_buckets(bool a_check);

  /*! marks or unmarks all buckets for clearing.
   */
  void mark_all_for_clearing(bool a_check);

  /*! shows or hides a bucket. When the Draw/Insert bucket is hidden, the first visible bucket becomes the Draw/Insert
   * bucket; when no bucket is visible, curves are not constructed.
   */
  void show_bucket(std::size_t aIndex, bool aChecked);

  /*! makes a bucket the Draw/Insert bucket.
   */
  void set_draw_bucket(std::size_t aIndex);

  /*! obtains the number of existing buckets that are visible.
   */
  std::size_t number_of_visible_buckets() const;

  /*! sets the number of existing buckets (the first ones), and shows their rows of the Operations panel.
   */
  void set_number_of_buckets(std::size_t aNumber);

  /*! restores the initial marks of the Operations panel.
   */
  void reset_marks();

  /*! obtains the first bucket marked in a given column, if any.
   */
  std::optional<std::size_t> checked_bucket(const Column& aColumn) const;

  /*! shows information about 2D regularized Boolean set-operations.
   */
  void about_boolean_set_operations();

  /*! shows information about CGAL.
   */
  void about_cgal();

  /*! shows information about 2D Minkowski sums.
   */
  void about_minkowski_sums();

  /*! shows information about the demo.
   */
  void about_demo();

  /*! fits the view to the scene after the window is resized.
   */
  void resizeEvent(QResizeEvent* event);

  /*! clears the scratch result set.
   */
  void delete_result();

  /*! clears all buckets.
   */
  void delete_all();

  /*! clears the marked buckets.
   */
  void apply_clear(bool aChecked);

  /*! starts or stops panning the view by dragging.
   */
  void toggle_pan();

  /*! obtains the (linear) polygon with holes of a given bucket, an operand of a Minkowski sum.
   */
  Polygon_with_holes_2 get_mink_input_polygon(std::size_t aIndex);

  /*! reads linear polygons from a file. \return false if the file cannot be read.
   */
  bool read_linear(QString aFileName);

  /*! reads polygons bounded by segments and circular arcs from a file. \return false if the file cannot be read.
   */
  bool read_circular(QString aFileName);

  /*! reads polygons bounded by polylines from a file. \return false if the file cannot be read.
   */
  bool read_polyline(QString aFileName);

  /*! reads Bezier polygons from a file into the Draw/Insert bucket. \return false if the file cannot be read.
   */
  bool read_bezier(QString aFileName);

signals:
  /*! signals that the model changed.
   */
  void changed();

private:
  /*! adds a polygon of a given curve family, constructed by its input, to the Draw/Insert bucket (as an undoable
   * action), or to the scratch result set while a bounding rectangle is constructed.
   */
  template <typename Set>
  void add_constructed_polygon(Curve_type aType, typename Set::Polygon_2 aPolygon, bool aToResult);

  /*! reads polygons with holes from a file into the Draw/Insert bucket. The file has a format line (DOUBLE for
   * coordinates given as doubles), the number of polygons with holes, and for each one the number of its boundaries
   * followed by the boundaries (the outer one first), each read by a given function. \return false if the file cannot
   * be read.
   */
  template <typename Set, typename Read_boundary>
  bool read_polygons(const QString& aFileName, const char* aKind, Read_boundary aReadBoundary);

  /*! pushes a new state onto the undo stack before an operation is applied. The new state shares the sets of the
   * current one; a set is copied when it first changes (see Curve_set::detach()). (Starting pushes empty sets.)
   */
  void get_new_state(State_op aOperation) {
    if (aOperation == State_op::Start) states_stack.push_back(State_current(aOperation, m_curve_type));
    else {
      State_current state = states_stack.back();
      state.m_operation = aOperation;
      state.m_type = m_curve_type;
      // The sets of another curve family (of a state before the family was switched) are replaced with empty sets.
      for (auto& set : state.m_curve_sets) {
        if (set.type() != m_curve_type) set.reset_type(m_curve_type);
      }
      states_stack.push_back(state);
    }
    m_state_num = states_stack.size();
    actionUndo->setEnabled(true);
    model_changed();
  }

  /*! shows the graphics items of the sets of the current state in the scene, instead of those of the sets they replaced
   * (of other states, or of other curve families).
   */
  void sync_graphics_items() {
    if (states_stack.empty()) return;
    auto& sets = states_stack.back().m_curve_sets;
    for (std::size_t i = 0; i < sets.size(); ++i) {
      CGAL::Qt::GraphicsItem* gi = sets[i].gi();
      if (gi == m_linked_items[i]) continue;
      if (m_linked_items[i]) unlink_gi(m_linked_items[i].data());
      gi->setZValue(double(i));       // the sets are drawn in the order of their indices
      link_gi(gi);
      m_linked_items[i] = gi;
    }
  }

  /*! shows the graphics items of the current state, and emits the changed() signal.
   */
  void model_changed() {
    sync_graphics_items();
    emit(changed());
  }

  /*! asks the user a yes/no question. \return true if the answer is yes.
   */
  bool ask_user_yesno(const QString& aTitle, const QString& aQuestion) {
    return QMessageBox::warning(this, aTitle, aQuestion, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ==
      QMessageBox::Yes;
  }

  /*! shows a warning.
   */
  void ask_user_ok(std::string aTitle, std::string aQuestion)
  { QMessageBox::warning(this, QString(aTitle.c_str()), QString(aQuestion.c_str())); }

  /*! adds a graphics item to the scene and connects it to the changed() signal.
   */
  void link_gi(CGAL::Qt::GraphicsItem* aGI) {
    QObject::connect(this, SIGNAL(changed()), aGI, SLOT(modelChanged()));
    m_scene.addItem(aGI);
  }

  /*! removes a graphics item from the scene and disconnects it from the changed() signal.
   */
  void unlink_gi(CGAL::Qt::GraphicsItem* aGI) {
    m_scene.removeItem(aGI);
    QObject::disconnect(this, SIGNAL(changed()), aGI, SLOT(modelChanged()));
  }

  /*! replaces a set with an empty set of a given curve family.
   */
  void switch_set_type(Curve_set& aSet, Curve_type aType);

  /*! replaces all sets with empty sets of a given curve family.
   */
  void switch_sets_type(Curve_type aType);

  /*! determines whether all sets (the buckets and the result) are empty.
   */
  bool are_all_sets_empty() {
    for (std::size_t i = 0; i <= number_of_buckets; ++i) {
      if (! states_stack.back().set(i).is_empty()) return false;
    }
    return true;
  }

};

#endif
