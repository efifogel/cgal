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

/*! \file Main_window.cpp
 * The main window: its construction, the buckets and the panels, the undo stack, the curve families and their inputs,
 * the view, the messages, and the help dialogs.
 */

#include "Main_window.h"
#include "Polynomial_dialog.h"
#include "Polynomial_parser.h"
#include "Algebraic_polygons.h"

Main_window::Main_window() :
  DemosMainWindow(),
  m_color_active(0),
  minkowski_sum_operated(false),
  empty_warn(true),
  operand_error_warn(true),
  m_disjoint(false),
  m_pan(false),
  m_grid(false),
  m_reset(false),
  m_state_num(0) {
  CGAL::set_error_handler(error_handler);
  CGAL::set_warning_handler(error_handler);
  CGAL::set_error_behaviour(CGAL::CONTINUE);

  setupUi(this);

  // Make the check boxes and radio buttons of the Operations panel visible on dark themes.
  auto* indicator_style = new Visible_indicator_style;
  indicator_style->setParent(this);
  for (auto* box : scene_dock_widget->findChildren<QCheckBox*>()) box->setStyle(indicator_style);
  for (auto* button : scene_dock_widget->findChildren<QRadioButton*>()) button->setStyle(indicator_style);

  // The Log panel: the program (the executed statements) and a button that saves it as a Python script.
  {
    auto* panel = new QWidget;
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(4, 4, 4, 4);
    m_log = new QPlainTextEdit;
    m_log->setReadOnly(true);
    m_log->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_log->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(m_log);
    auto* buttons = new QHBoxLayout;
    buttons->addStretch();
    auto* save_button = new QPushButton("Save Program...");
    buttons->addWidget(save_button);
    layout->addLayout(buttons);
    if (QWidget* old_panel = info_dock_widget->widget()) old_panel->deleteLater();
    info_dock_widget->setWidget(panel);
    QObject::connect(save_button, &QPushButton::clicked, this, [this]() { save_program(); });
  }

  // The initial buckets.
  log_statements({"red = Polygon_set_2()", "black = Polygon_set_2()", "blue = Polygon_set_2()"}, true);

  // The Console panel: the output, and an input line with a prompt.
  {
    auto* panel = new QWidget;
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(4, 4, 4, 4);
    QFont fixed = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_console_output = new QPlainTextEdit;
    m_console_output->setReadOnly(true);
    m_console_output->setFont(fixed);
    m_console_output->appendPlainText("Type Python-style statements on the polygon sets, e.g.,\n"
      "  red.join(Polygon_2([Point_2(0, 0), Point_2(40, 0), Point_2(0, 30)]))\n" "  blue.intersection(red, black)\n"
      "  result = Polygon_set_2(red)");
    layout->addWidget(m_console_output);
    auto* input_row = new QHBoxLayout;
    auto* prompt = new QLabel(">>>");
    prompt->setFont(fixed);
    input_row->addWidget(prompt);
    m_console_input = new QLineEdit;
    m_console_input->setFont(fixed);
    input_row->addWidget(m_console_input);
    layout->addLayout(input_row);
    if (QWidget* old_panel = console_dock_widget->widget()) old_panel->deleteLater();
    console_dock_widget->setWidget(panel);
    QObject::connect(m_console_input, &QLineEdit::returnPressed, this, [this]() {
      QString line = m_console_input->text();
      m_console_input->clear();
      run_console_line(line);
    });
    m_console_input->installEventFilter(this);
  }

  // Grid: show/hide, spacing, and snapping of clicked points to the grid.
  QObject::connect(action_grid, &QAction::toggled, this, [this](bool a_check) { m_scene.set_grid_visible(a_check); });
  auto* grid_snapper = new Grid_snapper(&m_scene, this);
  qApp->installEventFilter(grid_snapper);
  QObject::connect(action_snap_to_grid, &QAction::toggled, this, [grid_snapper](bool a_check) {
    grid_snapper->set_enabled(a_check);
  });
  // Grid density: step through the spacings below. A denser grid has a smaller spacing. The menu items show the spacing
  // they lead to, and are disabled at the ends of the range.
  static const std::array<double, 7> spacings = {1, 2, 5, 10, 20, 50, 100};
  auto update_density_actions = [this]() {
    auto it = std::find(spacings.begin(), spacings.end(), m_scene.grid_spacing());
    bool denser = (it != spacings.begin());
    bool sparser = (it + 1 != spacings.end());
    action_grid_denser->setEnabled(denser);
    action_grid_sparser->setEnabled(sparser);
    action_grid_denser->setText(denser ? QString("&Increase Grid Density (Spacing %1)").arg(*(it - 1)) :
      QString("&Increase Grid Density"));
    action_grid_sparser->setText(sparser ? QString("&Decrease Grid Density (Spacing %1)").arg(*(it + 1)) :
      QString("&Decrease Grid Density"));
  };
  auto change_density = [this, update_density_actions](int a_direction) {
    auto it = std::find(spacings.begin(), spacings.end(), m_scene.grid_spacing());
    it += a_direction;
    if ((it < spacings.begin()) || (it >= spacings.end())) return;
    m_scene.set_grid_spacing(*it);
    update_density_actions();
    statusBar()->showMessage(QString("Grid spacing: %1").arg(*it), 3000);
  };
  QObject::connect(action_grid_denser, &QAction::triggered, this, [change_density]() { change_density(-1); });
  QObject::connect(action_grid_sparser, &QAction::triggered, this, [change_density]() { change_density(+1); });
  update_density_actions();

  // The cells of the bucket rows, by column, and the other widgets of the rows.
  m_show_column = {show_red, show_black, show_blue, show_brown, show_yellow, show_magenta, show_aqua};
  m_draw_column = {draw_red, draw_black, draw_blue, draw_brown, draw_yellow, draw_magenta, draw_aqua};
  m_complement_column = {show_red_comp, show_black_comp, show_blue_comp, show_brown_comp, show_yellow_comp,
                         show_magenta_comp, show_aqua_comp};
  m_intersection_column = {show_red_int, show_black_int, show_blue_int, show_brown_int, show_yellow_int,
                           show_magenta_int, show_aqua_int};
  m_union_column = {show_red_union, show_black_union, show_blue_union, show_brown_union, show_yellow_union,
                    show_magenta_union, show_aqua_union};
  m_difference_column = {show_red_diff, show_black_diff, show_blue_diff, show_brown_diff, show_yellow_diff,
                         show_magenta_diff, show_aqua_diff};
  m_symmetric_difference_column = {show_red_sym_diff, show_black_sym_diff, show_blue_sym_diff, show_brown_sym_diff,
                                   show_yellow_sym_diff, show_magenta_sym_diff, show_aqua_sym_diff};
  m_minkowski_sum_column = {show_red_mink_sum, show_black_mink_sum, show_blue_mink_sum, show_brown_mink_sum,
                            show_yellow_mink_sum, show_magenta_mink_sum, show_aqua_mink_sum};
  m_copy_column = {copy_red, copy_black, copy_blue, copy_brown, copy_yellow, copy_magenta, copy_aqua};
  m_move_column = {move_red, move_black, move_blue, move_brown, move_yellow, move_magenta, move_aqua};
  m_clear_column = {clear_red, clear_black, clear_blue, clear_brown, clear_yellow, clear_magenta, clear_aqua};
  m_bucket_labels = {show_red_label, show_black_label, show_blue_label, show_brown_label, show_yellow_label,
                     show_magenta_label, show_aqua_label};
  m_bucket_separators = {nullptr, nullptr, nullptr, line10, line9, line8, line7};

  // Visible shows or hides a bucket; Draw/Insert selects one bucket (the radio buttons are exclusive).
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    QObject::connect(m_show_column[i], &QAbstractButton::toggled, this,
                     [this, i](bool checked) { show_bucket(i, checked); });
    QObject::connect(m_draw_column[i], &QAbstractButton::toggled, this, [this, i](bool checked) {
      if (checked) set_draw_bucket(i);
    });
  }

  // Exactly one bucket is the operand of the complement operation: make the Complement column exclusive, so clicking
  // the marked cell is a no-op.
  auto* complement_group = new QButtonGroup(this);
  complement_group->setExclusive(true);
  for (auto* box : m_complement_column) complement_group->addButton(box);

  // At most one bucket is copied or moved: checking a cell of the Copy or Move column unchecks the others.
  for (const Column* column : {&m_copy_column, &m_move_column}) {
    for (auto* box : *column) {
      QObject::connect(box, &QAbstractButton::toggled, this, [column, box](bool checked) {
        if (! checked) return;
        for (auto* other : *column) {
          if (other != box) other->setChecked(false);
        }
      });
    }
  }

  // In the Intersection and Union columns at least one bucket stays unchecked, to hold the result, and the operations
  // are enabled only with at least one operand.
  const std::pair<const Column*, QAbstractButton*>
    multi_operand_columns[] = {{&m_intersection_column, action_intersection_h}, {&m_union_column, action_union_h}};
  for (const auto& [column, operation] : multi_operand_columns) {
    for (auto* box : *column)
      QObject::connect(box, &QAbstractButton::toggled, this,
                       [this, box, column, operation](bool checked)
                       { limit_to_all_but_one(*column, box, checked, operation); });
    // Set the initial state of the operation.
    limit_to_all_but_one(*column, (*column)[0], false, operation);
  }

  // The Difference, Symmetric Difference, and Minkowski Sum columns have at most two marked cells, and their operations
  // are enabled only with exactly two.
  const std::pair<const Column*, QAbstractButton*> two_operand_columns[] = {
    {&m_difference_column, action_difference_h}, {&m_symmetric_difference_column, action_symmetric_difference_h},
    {&m_minkowski_sum_column, action_minkowski_sum_h}};
  for (const auto& [column, operation] : two_operand_columns) {
    for (auto* box : *column)
      QObject::connect(box, &QAbstractButton::toggled, this,
                       [this, box, column, operation](bool checked)
                       { limit_to_two_operands(*column, box, checked, operation); });
    // Set the initial state of the operation.
    limit_to_two_operands(*column, (*column)[0], false, operation);
  }

  // Fit the Visible and Draw/Insert header images into the same icon area as the operation buttons next to them
  // (27x25), keeping their aspect ratio.
  for (auto* label : {visible_icon, draw_header}) {
    QPixmap pixmap = label->pixmap();
    if (! pixmap.isNull()) label->setPixmap(pixmap.scaled(27, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }

  // The scene and the view.
  m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
  m_scene.setSceneRect(-320, -210, 640, 420);
  this->graphics_view->setScene(&m_scene);
  // Use a white canvas regardless of the desktop color scheme, so that the
  // drawing area stands out from the surrounding widgets. (The scene, rather
  // than the view, paints it, so that the scene can draw the grid over it.)
  m_scene.setBackgroundBrush(Qt::white);
  this->graphics_view->setMouseTracking(true);
  this->graphics_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->graphics_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Turn the vertical axis upside down
  this->graphics_view->scale(2.5, -2.5);

  // The navigation adds zooming and translation functionality to the view.
  this->addNavigation(this->graphics_view);
  // Every change of the view (zoom, pan, resize) ends in a repaint, where the area that unbounded sets are drawn over
  // is updated.
  this->graphics_view->viewport()->installEventFilter(this);

  // The menus.
  this->setupStatusBar();
  this->setupOptionsMenu();

  // The recently opened files.
  this->addRecentFiles(this->menu_file, this->action_close);

  // The first state of the undo stack.
  get_new_state(State_op::SO_START);
  action_undo->setEnabled(false);

  this->setWindowState(Qt::WindowMaximized);

  // The inputs, which construct polygons with the mouse.
  m_bezier_input = new CGAL::Qt::Graphics_view_bezier_polygon_input<Bezier_traits>(this, &m_scene);
  m_linear_input = new CGAL::Qt::Graphics_view_linear_polygon_input<Kernel>(this, &m_scene);
  m_circular_input = new CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>(this, &m_scene);
  m_polyline_input = new CGAL::Qt::Graphics_view_polyline_input<Kernel>(this, &m_scene);
  m_conic_input = new CGAL::Qt::Graphics_view_conic_input(this, &m_scene);
  m_algebraic_input = new CGAL::Qt::GraphicsViewInput(this);

  // The curve families. Selecting one in the tool bar starts constructing its curves.
  m_families = {{
    {Curve_type::CT_LINEAR, "linear", "Segments", action_insert_linear, m_linear_input,
     [this] { m_linear_input->reset(); }, action_open_linear, ".lps", &Main_window::read_linear},
    {Curve_type::CT_CIRCULAR, "circular", "Line Segments and Circular Arcs", action_insert_circular, m_circular_input,
     [this] { m_circular_input->reset(); }, action_open_circular, ".cps", &Main_window::read_circular},
    {Curve_type::CT_BEZIER, "Bezier", "Bezier Curves", action_insert_bezier, m_bezier_input,
     [this] { m_bezier_input->reset(); }, action_open_bezier, ".bps", &Main_window::read_bezier},
    {Curve_type::CT_POLYLINE, "polyline", "Polylines", action_insert_polyline, m_polyline_input,
     [this] { m_polyline_input->reset(); }, action_open_polyline, ".plps", &Main_window::read_polyline},
    {Curve_type::CT_CONIC, "conic", "Conics", action_insert_conic, m_conic_input, [this] { m_conic_input->reset(); },
     action_open_conic, ".qps", &Main_window::read_conic},
    {Curve_type::CT_ALGEBRAIC, "algebraic", "Algebraic Curves", action_insert_algebraic, m_algebraic_input, [] {},
     nullptr, nullptr, nullptr}}};
  // The Polyline/Polycurve button selects the polycurve family, whose subcurve family is selected by the buttons
  // underneath (only Segments, for now). Unchecking a family button by clicking it again keeps it checked.
  for (const auto& family : m_families) {
    Curve_type type = family.m_type;
    if (type == Curve_type::CT_POLYLINE) continue;
    QObject::connect(family.m_insert_action, &QAction::toggled, this, [this, type](bool checked) {
      if (m_updating_family_actions) return;
      if (! checked) {
        update_family_actions(! m_pan);
        return;
      }
      bool polycurve = (m_curve_type == Curve_type::CT_POLYLINE);
      start_inserting((polycurve && (type == Curve_type::CT_LINEAR)) ? Curve_type::CT_POLYLINE : type);
    });
  }
  QObject::connect(action_insert_polyline, &QAction::toggled, this, [this](bool checked) {
    if (! m_updating_family_actions) start_inserting(checked ? Curve_type::CT_POLYLINE : Curve_type::CT_LINEAR);
  });

  // Algebraic curves are entered as equations, which requires a parser.
  action_insert_algebraic->setToolTip(polynomial_parser() ?
                                      "Algebraic Curves: enter the equation of a curve, e.g., x^2 + y^2 == 25" :
                                      "Algebraic Curves (not available: the demo was built without Lexy)");

  // The curve tool bar, underneath the curve families: the ways to construct curves of the selected family.
  {
    using Circular_input = CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>;
    auto* bar = new QToolBar;
    bar->setIconSize(QSize(24, 24));
    auto* group = new QActionGroup(this);
    auto* arcs = bar->addAction(curve_tool_icon("arcs"), "Line Segments and Circular Arcs");
    arcs->setToolTip("Line Segments and Circular Arcs: click at each vertex; "
                     "press and drag to bend a piece into an arc through the "
                     "cursor; right-click to close (drag to bend the last " "piece)");
    auto* circle = bar->addAction(curve_tool_icon("full circle"), "Circle");
    circle->setToolTip("Circle: press at the center and drag to the radius");
    for (auto* action : {arcs, circle}) {
      action->setCheckable(true);
      group->addAction(action);
    }
    arcs->setChecked(true);
    QObject::connect(circle, &QAction::toggled, this, [this](bool a_checked) {
      m_circular_input->set_mode(a_checked ? Circular_input::Mode::CIRCLE_MODE : Circular_input::Mode::MO_ARCS);
    });

    // The conic tools: conics that bound a polygon, full curves, and a conic given by its coefficients.
    using Conic_input = CGAL::Qt::Graphics_view_conic_input;
    auto* conic_group = new QActionGroup(this);
    const std::vector<std::tuple<QString, QString, Conic_input::Mode, QString>>
      conic_tools = {
      {"line", "Line", Conic_input::Mode::MO_LINE,
       "Line: press at one of its points and drag to another"},
      {"circle", "Circle", Conic_input::Mode::MO_CIRCLE,
       "Circle: press at the center and drag to the radius"},
      {"ellipse", "Ellipse", Conic_input::Mode::MO_ELLIPSE,
       "Ellipse: click at the center, press at the end of an axis, and "
       "drag to the width of the other axis"},
      {"full circle", "Full Circle", Conic_input::Mode::FULL_CIRCLE,
       "Full Circle: press at the center and drag to the radius"},
      {"full ellipse", "Full Ellipse", Conic_input::Mode::FULL_ELLIPSE,
       "Full Ellipse: click at the center, press at the end of an axis, and "
       "drag to the width of the other axis"}};
    const QString boundary_help = ". Every conic you add meets the previous one at the next vertex (click "
      "at it when there are several); on a circle or an ellipse, click near "
      "the arc to keep; right-click to close the boundary. Escape or Backspace "
      "discards the conic under construction, or else removes the last "
      "conic.";
    std::vector<QAction*> conic_actions;
    for (const auto& [icon, text, mode, tip] : conic_tools) {
      if (mode == Conic_input::Mode::FULL_CIRCLE)
        conic_actions.push_back(bar->addSeparator());
      auto* action = bar->addAction(curve_tool_icon(icon), text);
      action->setToolTip((mode < Conic_input::Mode::FULL_CIRCLE) ? tip + boundary_help : tip);
      action->setCheckable(true);
      conic_group->addAction(action);
      Conic_input::Mode m = mode;
      QObject::connect(action, &QAction::triggered, this, [this, m]() { m_conic_input->set_mode(m); });
      conic_actions.push_back(action);
    }
    conic_actions[0]->setChecked(true);
    conic_actions.push_back(bar->addSeparator());
    std::vector<QAction*> algebraic_actions;
    auto* equation = bar->addAction(curve_tool_icon("equation"), "Equation...");
    equation->setToolTip("Equation: enter the equation of an algebraic curve, e.g., x^2 + y^2 == 25, and add the "
                         "region it encloses");
    QObject::connect(equation, &QAction::triggered, this, &Main_window::enter_equation);
    algebraic_actions.push_back(equation);

    auto* coefficients = bar->addAction(curve_tool_icon("coefficients"), "Coefficients...");
    coefficients->setToolTip("Coefficients: enter a conic " "r x\u00b2 + s y\u00b2 + t xy + u x + v y + w = 0, "
                             "and add it to the boundary or construct the full " "ellipse");
    QObject::connect(coefficients, &QAction::triggered, this, [this]() { m_conic_input->enter_coefficients(this); });
    conic_actions.push_back(coefficients);

    // Keep the height of the bar when it shows no buttons.
    bar->setMinimumHeight(bar->sizeHint().height());
    auto show_tools = [this, arcs, circle, conic_actions, algebraic_actions]() {
      bool circular = action_insert_circular->isChecked();
      arcs->setVisible(circular);
      circle->setVisible(circular);
      for (auto* action : conic_actions)
        action->setVisible(action_insert_conic->isChecked());
      for (auto* action : algebraic_actions) action->setVisible(action_insert_algebraic->isChecked());
    };
    show_tools();
    QObject::connect(action_insert_circular, &QAction::toggled, this, show_tools);
    QObject::connect(action_insert_conic, &QAction::toggled, this, show_tools);
    QObject::connect(action_insert_algebraic, &QAction::toggled, this, show_tools);
    vertical_layout->insertWidget(1, bar);
  }

  // The polycurve tool bar, above the curve families.
  {
    auto* bar = new QToolBar;
    bar->setIconSize(traits_tool_bar->iconSize());
    bar->addAction(action_insert_polyline);
    vertical_layout->insertWidget(0, bar);
  }
  update_family_actions();

  // The polygons constructed by the inputs are added to the Draw/Insert bucket.
  for (const auto& family : m_families) {
    QObject::connect(family.m_input, &CGAL::Qt::GraphicsViewInput::generate, this, &Main_window::process_input);
  }

  m_scene.installEventFilter(m_linear_input);

  // Under Wayland, let the compositor move floating dock widgets.
  if (QGuiApplication::platformName().startsWith("wayland")) {
    auto* dock_mover = new Floating_dock_mover(this);
    for (auto* dock : findChildren<QDockWidget*>()) dock->installEventFilter(dock_mover);
  }

  QObject::connect(this->action_quit, SIGNAL(triggered()), this, SLOT(close()));

  // The actions and the widgets of the panels.
  QObject::connect(action_new, &QAction::triggered, this, &Main_window::new_program);
  QObject::connect(action_undo, &QAction::triggered, this, &Main_window::undo);
  QObject::connect(action_axis, &QAction::triggered, this, &Main_window::toggle_axes);
  QObject::connect(action_pan, &QAction::triggered, this, &Main_window::toggle_pan);
  QObject::connect(action_recenter, &QAction::triggered, this, &Main_window::zoom_to_fit);
  QObject::connect(action_open_linear, &QAction::triggered, this, &Main_window::open_linear);
  QObject::connect(action_open_circular, &QAction::triggered, this, &Main_window::open_circular);
  QObject::connect(action_open_bezier, &QAction::triggered, this, &Main_window::open_bezier);
  QObject::connect(action_open_polyline, &QAction::triggered, this, &Main_window::open_polyline);
  QObject::connect(action_open_conic, &QAction::triggered, this, &Main_window::open_conic);
  QObject::connect(action_save_current_bucket, &QAction::triggered, this, &Main_window::save_draw_bucket);
  QObject::connect(action_add_color, &QAction::triggered, this, &Main_window::add_bucket);
  QObject::connect(action_minus_color, &QAction::triggered, this, &Main_window::remove_bucket);
  QObject::connect(action_delete_all, &QAction::triggered, this, &Main_window::delete_all);
  QObject::connect(show_color_bucket, &QAction::toggled, this, &Main_window::show_operations_panel);
  QObject::connect(show_console, &QAction::toggled, this, &Main_window::show_console_panel);
  QObject::connect(show_info, &QAction::toggled, this, &Main_window::show_log_panel);
  QObject::connect(action_about_bolop, &QAction::triggered, this, &Main_window::about_boolean_set_operations);
  QObject::connect(action_about_cgal, &QAction::triggered, this, &Main_window::about_cgal);
  QObject::connect(action_about_minkop, &QAction::triggered, this, &Main_window::about_minkowski_sums);
  QObject::connect(action_about_demo, &QAction::triggered, this, &Main_window::about_demo);
  QObject::connect(action_complement_h, &QAbstractButton::toggled, this, &Main_window::apply_complement);
  QObject::connect(action_intersection_h, &QAbstractButton::toggled, this, &Main_window::apply_intersection);
  QObject::connect(action_union_h, &QAbstractButton::toggled, this, &Main_window::apply_union);
  QObject::connect(action_difference_h, &QAbstractButton::toggled, this, &Main_window::apply_difference);
  QObject::connect(action_symmetric_difference_h, &QAbstractButton::toggled, this,
                   &Main_window::apply_symmetric_difference);
  QObject::connect(action_minkowski_sum_h, &QAbstractButton::toggled, this, &Main_window::apply_minkowski_sum);
  QObject::connect(action_copy_h, &QAbstractButton::toggled, this, &Main_window::apply_copy);
  QObject::connect(action_move_h, &QAbstractButton::toggled, this, &Main_window::apply_move);
  QObject::connect(action_clear_h, &QAbstractButton::toggled, this, &Main_window::apply_clear);
  QObject::connect(show_result, &QAbstractButton::toggled, this, &Main_window::show_result_set);
  QObject::connect(visible_header, &QAbstractButton::toggled, this, &Main_window::show_all_buckets);
  QObject::connect(show_clear, &QAbstractButton::toggled, this, &Main_window::mark_all_for_clearing);
  QObject::connect(scene_dock_widget, &QDockWidget::visibilityChanged, this,
                   &Main_window::operations_panel_visibility_changed);
  QObject::connect(info_dock_widget, &QDockWidget::visibilityChanged, this, &Main_window::log_panel_visibility_changed);
  QObject::connect(console_dock_widget, &QDockWidget::visibilityChanged, this,
                   &Main_window::console_panel_visibility_changed);

  // Opening a recently opened file.
  QObject::connect(this, SIGNAL(openRecentFile(QString)), this, SLOT(open(QString)));

  QObject::connect(action_error_operand, &QAction::toggled, this,
                   [this](bool a_check) { operand_error_warn = a_check; });
  QObject::connect(action_warn_not_empty, &QAction::toggled, this, [this](bool a_check) { empty_warn = a_check; });

  this->graphics_view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

  // The axes.
  QPen axis_pen(QBrush(Qt::black), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
  m_x_axis->setLine(-155000000000000, 0, 155000000000000, 0);
  m_x_axis->setPen(axis_pen);
  m_y_axis->setLine(0, -100000000000000, 0, 100000000000000);
  m_y_axis->setPen(axis_pen);

  m_linear_input->m_ongoing_piece_GI->set_pen(s_pens[0]);
  m_linear_input->m_linear_GI->set_pen(s_pens[0]);
  m_linear_input->m_handle_GI->setPen(s_pens[0]);
  action_open_linear->setEnabled(true);
  action_open_circular->setEnabled(false);
  action_open_bezier->setEnabled(false);
  action_open_polyline->setEnabled(false);
  action_open_conic->setEnabled(false);
}

void Main_window::show_result_set(bool a_check) { states_stack.back().toggle_view(result_index, a_check); }

void Main_window::show_bucket(std::size_t a_index, bool a_checked) {
  states_stack.back().toggle_view(a_index, a_checked);
  m_draw_column[a_index]->setEnabled(a_checked);
  if (a_index >= m_number_of_buckets) return;
  auto visible = number_of_visible_buckets();
  if (a_checked) {
    if (visible == 1) m_scene.installEventFilter(current_input());
  }
  else if (visible == 0) remove_inputs();
  else if (a_index == m_color_active) {
    for (std::size_t i = 0; i < m_number_of_buckets; ++i) {
      if (m_show_column[i]->isChecked()) {
        m_draw_column[i]->setChecked(true);
        break;
      }
    }
  }
  // The header is checked when all buckets are visible.
  QSignalBlocker blocker(visible_header);
  visible_header->setChecked(visible == m_number_of_buckets);
}

void Main_window::set_draw_bucket(std::size_t a_index) {
  m_color_active = a_index;
  set_input_pen(s_pens[a_index]);
}

std::size_t Main_window::number_of_visible_buckets() const {
  return std::count_if(m_show_column.begin(), m_show_column.begin() + m_number_of_buckets,
                       [](QAbstractButton* box) { return box->isChecked(); });
}

void Main_window::set_number_of_buckets(std::size_t a_number) {
  m_number_of_buckets = a_number;
  const Column* columns[] = {&m_show_column, &m_draw_column, &m_complement_column, &m_intersection_column,
                             &m_union_column, &m_difference_column, &m_symmetric_difference_column,
                             &m_minkowski_sum_column, &m_copy_column, &m_move_column, &m_clear_column};
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    bool exists = (i < a_number);
    for (const Column* column : columns) (*column)[i]->setVisible(exists);
    m_bucket_labels[i]->setVisible(exists);
    if (m_bucket_separators[i]) m_bucket_separators[i]->setVisible(exists);
  }

  // The vertical lines between the columns span the rows.
  int height = 155 + 30 * int(a_number - min_number_of_buckets);
  for (auto* line : {line0006, line1, line2, line3, line4, line5, line6, line0007, line06, line061, line062, line063})
    line->setGeometry(QRect(line->x(), 0, 7, height));

  bool can_add = (a_number < number_of_buckets);
  action_add_color->setEnabled(can_add);
  action_add_color->setText(can_add ? "Add a Bucket" : "Maximum Number of Buckets Reached");
  bool can_remove = (a_number > min_number_of_buckets);
  action_minus_color->setEnabled(can_remove);
  if (can_remove) {
    action_minus_color->setText("Remove " + capitalized(bucket_name(a_number - 1)));
  }
  else action_minus_color->setText("Minimum Number of Buckets Reached");
}

void Main_window::show_all_buckets(bool a_check) {
  for (auto* box : m_show_column) box->setChecked(a_check);
  if (a_check) m_draw_column[0]->setChecked(true);
}

void Main_window::mark_all_for_clearing(bool a_check) {
  for (auto* box : m_clear_column) box->setChecked(a_check);
}

void Main_window::operations_panel_visibility_changed(bool a_check) {
  if (a_check) show_color_bucket->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) show_color_bucket->setChecked(false);
}

void Main_window::log_panel_visibility_changed(bool a_check) {
  if (a_check) show_info->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) show_info->setChecked(false);
}

void Main_window::console_panel_visibility_changed(bool a_check) {
  if (a_check) show_console->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) show_console->setChecked(false);
}

void Main_window::show_operations_panel(bool a_check) {
  if (a_check) scene_dock_widget->setVisible(true);
  else scene_dock_widget->setVisible(false);
}

void Main_window::show_console_panel(bool a_check) {
  if (a_check) console_dock_widget->setVisible(true);
  else console_dock_widget->setVisible(false);
}

void Main_window::show_log_panel(bool a_check) {
  if (a_check) info_dock_widget->setVisible(true);
  else info_dock_widget->setVisible(false);
}

void Main_window::add_bucket() {
  if (m_number_of_buckets == number_of_buckets) return;
  log_statements({QString("%1 = Polygon_set_2()").arg(bucket_name(m_number_of_buckets))}, true);
  set_number_of_buckets(m_number_of_buckets + 1);
  model_changed();
}

void Main_window::remove_bucket() {
  if (m_number_of_buckets == min_number_of_buckets) return;
  std::size_t i = m_number_of_buckets - 1;
  log_statements({QString("del %1").arg(bucket_name(i))}, true);
  states_stack.back().clear(i);

  // If the removed bucket is marked for Draw/Insert or Complement, mark the first bucket instead.
  if (m_color_active == i) m_draw_column[0]->setChecked(true);
  if (m_complement_column[i]->isChecked()) m_complement_column[0]->setChecked(true);
  // Unmark the removed bucket as an operand (this disables an operation that no longer has two operands), and as the
  // bucket to copy, move, or clear.
  for (const Column* column : {&m_intersection_column, &m_union_column, &m_difference_column,
                               &m_symmetric_difference_column, &m_minkowski_sum_column, &m_copy_column, &m_move_column,
                               &m_clear_column})
    (*column)[i]->setChecked(false);
  set_number_of_buckets(i);
  // Keep at least one remaining bucket unmarked in the Intersection and Union columns, for the result.
  keep_one_unchecked(m_intersection_column);
  keep_one_unchecked(m_union_column);
  model_changed();
}

void Main_window::undo() {
  if (m_state_num > 1) {
    // The sets of the previous state are shown again by model_changed().
    states_stack.pop_back();
    if (states_stack.size() == 1) action_undo->setEnabled(false);
    m_state_num = states_stack.size();
    trim_program_to_states();
    model_changed();
  }
  else {
    show_error("Nothing to Undo!!!");
  }
}

void Main_window::reset_marks() {
  // The initial marks: all buckets are visible, the first one is the Draw/Insert bucket and the complement operand,
  // and the first two are the operands of the other operations. (Unmark the other operands first, so that marking the
  // first two is not undone.)
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    m_show_column[i]->setChecked(true);
    for (const Column* column : {&m_copy_column, &m_move_column, &m_clear_column})
      (*column)[i]->setChecked(false);
  }
  const Column* operand_columns[] = {&m_intersection_column, &m_union_column, &m_difference_column,
                                     &m_symmetric_difference_column, &m_minkowski_sum_column};
  for (const Column* column : operand_columns) {
    for (std::size_t i = 2; i < number_of_buckets; ++i) (*column)[i]->setChecked(false);
  }
  for (const Column* column : operand_columns) {
    for (std::size_t i = 0; i < 2; ++i) (*column)[i]->setChecked(true);
  }
  m_draw_column[0]->setChecked(true);
  m_complement_column[0]->setChecked(true);
  show_result->setChecked(false);
}

void Main_window::new_program() {
  if (m_state_num > 1) get_new_state(State_op::SO_RESET);

  // Reset starts a new program, which constructs the initial buckets.
  m_program.clear();
  if (m_log) m_log->clear();
  QStringList program;
  for (std::size_t i = 0; i < min_number_of_buckets; ++i)
    program << QString("%1 = Polygon_set_2()").arg(bucket_name(i));
  log_statements(program, true);

  for (std::size_t i = 0; i <= number_of_buckets; ++i) states_stack.back().clear(i);

  m_scene.setSceneRect(-320, -210, 640, 420);
  this->graphics_view->setScene(&m_scene);
  this->graphics_view->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  this->graphics_view->scale(2.5, 2.5);

  m_disjoint = false;

  m_linear_input->reset();
  m_circular_input->reset();
  m_bezier_input->reset();
  m_polyline_input->reset();

  m_pan = false;

  m_color_active = 0;

  m_linear_input->m_ongoing_piece_GI->set_pen(s_pens[0]);
  m_linear_input->m_linear_GI->set_pen(s_pens[0]);
  m_linear_input->m_handle_GI->setPen(s_pens[0]);

  scene_dock_widget->setVisible(true);
  console_dock_widget->setVisible(true);
  info_dock_widget->setVisible(true);

  show_color_bucket->setChecked(true);
  show_console->setChecked(true);
  show_info->setChecked(true);

  action_copy_h->setChecked(false);
  action_move_h->setChecked(false);

  action_complement_h->setChecked(false);
  action_union_h->setChecked(false);
  action_intersection_h->setChecked(false);
  action_difference_h->setChecked(false);
  action_symmetric_difference_h->setChecked(false);
  action_minkowski_sum_h->setChecked(false);

  m_reset = true;
  start_inserting(m_curve_type);
  m_reset = false;

  action_difference_h->setEnabled(true);

  reset_marks();

  set_number_of_buckets(min_number_of_buckets);
  visible_header->setChecked(true);
  show_clear->setChecked(false);
  line11->setVisible(true);

  zoom_to_fit();
  model_changed();
}

void Main_window::switch_set_type(Curve_set& a_set, Curve_type a_type) {
  a_set.reset_type(a_type);
  model_changed();
}

void Main_window::switch_sets_type(Curve_type a_type) {
  for (std::size_t i = 0; i <= number_of_buckets; ++i) switch_set_type(states_stack.back().set(i), a_type);
}

void Main_window::toggle_axes() {
  if (! m_grid) {
    m_scene.addItem(m_x_axis.get());
    m_scene.addItem(m_y_axis.get());
    m_grid = true;
  }
  else {
    m_scene.removeItem(m_x_axis.get());
    m_scene.removeItem(m_y_axis.get());
    m_grid = false;
  }
  // Keep the button checked (highlighted) while the axes are shown.
  action_axis->setChecked(m_grid);
  m_scene.update();
  model_changed();
}

void Main_window::toggle_pan() {
  if (! m_pan) {
    remove_inputs();
    update_family_actions(false);
    this->graphics_view->setDragMode(QGraphicsView::ScrollHandDrag);

    this->graphics_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_scene.update();
    m_pan = true;
    model_changed();
  }
  else {
    m_pan = false;
    update_family_actions();
    model_changed();
  }
}

void Main_window::wheelEvent(QWheelEvent* event) {

  this->graphics_view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
  static const double scale_factor = 1.15;
  static double current_scale = 1.0;  // stores the current scale value.
  static const double scale_min = .1; // defines the min scale limit.

  if (m_scene.width() >= 1500000 && event->angleDelta().y() > 0) {
    this->graphics_view->setDragMode(QGraphicsView::NoDrag);
    this->graphics_view->setSceneRect(-1500000, -1000000, 3100000, 2000000);
    this->graphics_view->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  }
  if (m_scene.width() <= 1500000) {
    if (event->angleDelta().y() > 0) {
      m_scene.setSceneRect(-m_scene.sceneRect().width() * scale_factor / 2,
                           -m_scene.sceneRect().height() * scale_factor / 2, m_scene.sceneRect().width() * scale_factor,
                           m_scene.sceneRect().height() * scale_factor);
      this->graphics_view->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
      this->graphics_view->scale(2.5, 2.5);
    }
    else if (current_scale > scale_min) {
      m_scene.setSceneRect(-m_scene.sceneRect().width() / (scale_factor * 2),
                           -m_scene.sceneRect().height() / (scale_factor * 2),
                           m_scene.sceneRect().width() / scale_factor, m_scene.sceneRect().height() / scale_factor);
      this->graphics_view->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
      this->graphics_view->scale(2.5, 2.5);
    }
  }
  model_changed();
}

QIcon Main_window::curve_tool_icon(const QString& a_kind) {
  const qreal ratio = 2;
  QPixmap pixmap(QSize(24, 24) * ratio);
  pixmap.setDevicePixelRatio(ratio);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  const QColor green(6, 131, 2);
  painter.setPen(QPen(green, 2));
  painter.setBrush(Qt::NoBrush);
  // Full curves are filled; the curves that bound a polygon show two vertices.
  if (a_kind.startsWith("full")) painter.setBrush(QColor(0, 255, 1, 140));
  QList<QPointF> vertices;
  if (a_kind == "arcs") {
    QPainterPath path(QPointF(3, 20));
    path.lineTo(7, 9);
    path.arcTo(QRectF(7, 2, 14, 14), 180, -180);
    path.lineTo(17, 20);
    painter.drawPath(path);
    vertices << QPointF(3, 20) << QPointF(7, 9) << QPointF(21, 9)
             << QPointF(17, 20);
  }
  else if (a_kind == "line") {
    painter.drawLine(QPointF(2, 20), QPointF(22, 4));
    vertices << QPointF(7, 16) << QPointF(17, 8);
  }
  else if ((a_kind == "circle") || (a_kind == "full circle")) {
    painter.drawEllipse(QPointF(12, 12), 9, 9);
    if (a_kind == "circle") vertices << QPointF(3, 12) << QPointF(18.4, 5.6);
    else {
      painter.setPen(QPen(QColor(64, 64, 64), 1.5, Qt::DashLine));
      painter.drawLine(QPointF(12, 12), QPointF(18.4, 5.6));
      vertices << QPointF(12, 12);
    }
  }
  else if ((a_kind == "ellipse") || (a_kind == "full ellipse")) {
    painter.translate(12, 12);
    painter.rotate(-30);
    painter.drawEllipse(QPointF(0, 0), 10.5, 6);
    painter.resetTransform();
    if (a_kind == "ellipse") vertices << QPointF(3, 17) << QPointF(21, 7);
  }
  else if (a_kind == "coefficients") {
    QFont font = painter.font();
    font.setPixelSize(11);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(64, 64, 64));
    painter.drawText(QRectF(0, 0, 24, 24), Qt::AlignCenter, "ax²");
  }
  else if (a_kind == "equation") {
    QFont font = painter.font();
    font.setPixelSize(10);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(64, 64, 64));
    painter.drawText(QRectF(0, 0, 24, 24), Qt::AlignCenter, "f=0");
  }
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(255, 7, 0));
  for (const auto& p : vertices) painter.drawEllipse(p, 2, 2);
  return QIcon(pixmap);
}

QObject* Main_window::current_input() const { return family(m_curve_type).m_input; }

void Main_window::remove_inputs() {
  for (const auto& family : m_families) {
    m_scene.removeEventFilter(family.m_input);
    family.m_reset_input();
  }
}

void Main_window::install_input(QObject* a_input) {
  for (const auto& family : m_families) m_scene.removeEventFilter(family.m_input);
  m_scene.installEventFilter(a_input);
}

void Main_window::set_input_pen(const QPen& a_pen) {
  m_linear_input->m_ongoing_piece_GI->set_pen(a_pen);
  m_linear_input->m_linear_GI->set_pen(a_pen);
  m_linear_input->m_handle_GI->setPen(a_pen);
  m_circular_input->m_ongoing_piece_GI->set_pen(a_pen);
  m_circular_input->m_circular_GI->set_pen(a_pen);
  m_circular_input->m_handle_GI->setPen(a_pen);
  m_bezier_input->m_ongoing_piece_GI->set_pen(a_pen);
  m_bezier_input->m_bezier_GI->set_pen(a_pen);
  m_bezier_input->m_handle0_GI->setPen(a_pen);
  m_bezier_input->m_handle1_GI->setPen(a_pen);
  m_polyline_input->m_ongoing_piece_GI->set_pen(a_pen);
  m_polyline_input->m_polyline_GI->set_pen(a_pen);
  m_polyline_input->m_handle_GI->setPen(a_pen);
  m_conic_input->set_pen(a_pen);
}

bool Main_window::ensure_curve_type(Curve_type a_type) {
  if (m_curve_type == a_type) return true;
  QString name = family(a_type).m_name;
  if (! are_all_sets_empty() &&
      ! ask_user_yesno(capitalized(name) + " mode switch",
                       QString("You are about to construct or load %1 polygons, but there are polygons of another "
                               "family already loaded.\n"
                               "Both families are not interoperable. In order to proceed, the polygons must be removed "
                               "first.\n"
                               "Once deleted you cannot undo the action.\n"
                               "Yes to remove and proceed?\n").arg(name)))
    return false;
  switch_sets_type(a_type);
  for (const auto& other : m_families) {
    if (other.m_type != a_type) other.m_reset_input();
  }
  m_curve_type = a_type;
  update_family_actions(! m_pan);
  return true;
}

void Main_window::update_family_actions(bool a_active) {
  m_updating_family_actions = true;
  bool polycurve = (m_curve_type == Curve_type::CT_POLYLINE);
  Curve_type selected = polycurve ? Curve_type::CT_LINEAR : m_curve_type;
  for (const auto& family : m_families) {
    if (family.m_type == Curve_type::CT_POLYLINE) {
      family.m_insert_action->setChecked(a_active && polycurve);
      continue;
    }
    family.m_insert_action->setChecked(a_active && (family.m_type == selected));
    bool available = (family.m_type != Curve_type::CT_ALGEBRAIC) || polynomial_parser();
    if (family.m_type != Curve_type::CT_LINEAR) family.m_insert_action->setEnabled(! polycurve && available);
  }
  m_updating_family_actions = false;
}

void Main_window::start_inserting(Curve_type a_type) {
  const auto& selected = family(a_type);
  this->graphics_view->setDragMode(QGraphicsView::NoDrag);
  if (! ensure_curve_type(a_type)) {
    update_family_actions(! m_pan);
    return;
  }
  if (! m_pan && ! m_reset) {
    // Switching the family discards the history; the first state gets empty sets of the family.
    while (m_state_num > 1) undo();
    if (states_stack.back().set(0).type() != a_type) switch_sets_type(a_type);
  }
  action_pan->setChecked(false);
  m_pan = false;
  for (const auto& other : m_families) {
    if (other.m_open_action) other.m_open_action->setEnabled(other.m_type == a_type);
  }
  update_family_actions();
  install_input(selected.m_input);
  delete_result();
  action_minkowski_sum_h->setEnabled(a_type == Curve_type::CT_LINEAR);
  set_input_pen(s_pens[m_color_active]);
}

void Main_window::zoom_to_fit() {
  std::optional<QRectF> l_total_rect;
  for (auto si = states_stack.back().m_curve_sets.begin(); si != states_stack.back().m_curve_sets.end(); ++si) {
    if (! si->is_empty()) {
      QRectF l_rect = si->bounding_rect();
      if (l_total_rect) l_total_rect = *l_total_rect | l_rect;
      else l_total_rect = l_rect;
    }
  }
  if (l_total_rect) {
    this->graphics_view->setSceneRect(*l_total_rect);
    this->graphics_view->fitInView(*l_total_rect, Qt::KeepAspectRatio);
  }
  model_changed();
}

QMessageBox::StandardButton
Main_window::show_message(QMessageBox::Icon a_icon, const QString& a_title, const QString& a_text, QAction* a_toggle,
                         QMessageBox::StandardButtons a_buttons) {
  // Declared before the dialog, so that it outlives the check box using it.
  Visible_indicator_style indicator_style;
  QDialog dialog(this);
  dialog.setWindowTitle(a_title);

  // The same icons QMessageBox uses.
  QStyle::StandardPixmap pixmap = QStyle::SP_MessageBoxInformation;
  switch (a_icon) {
   case QMessageBox::Warning: pixmap = QStyle::SP_MessageBoxWarning; break;
   case QMessageBox::Critical: pixmap = QStyle::SP_MessageBoxCritical; break;
   case QMessageBox::Question: pixmap = QStyle::SP_MessageBoxQuestion; break;
   default: break;
  }
  int icon_size = style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
  auto* icon_label = new QLabel;
  icon_label->setPixmap(style()->standardIcon(pixmap).pixmap(icon_size));
  icon_label->setAlignment(Qt::AlignTop);
  auto* message_row = new QHBoxLayout;
  message_row->addWidget(icon_label);
  message_row->addSpacing(12);
  message_row->addWidget(new QLabel(a_text), 1);

  auto* check_box = new QCheckBox("Don't show this message again");
  // Make the check box visible on dark themes, like those of the Operations panel.
  check_box->setStyle(&indicator_style);

  auto* ok_button = new QPushButton("OK");
  QPushButton* cancel_button = a_buttons.testFlag(QMessageBox::Cancel) ? new QPushButton("Cancel") : nullptr;
  auto* button_row = new QHBoxLayout;
  if (cancel_button) {
    button_row->addWidget(ok_button);
    button_row->addStretch();
    button_row->addWidget(cancel_button);
  }
  else {
    button_row->addStretch();
    button_row->addWidget(ok_button);
  }
  for (auto* button : {ok_button, cancel_button}) {
    if (! button) continue;
    button->setAutoDefault(false);
    button->setDefault(false);
  }
  QObject::connect(ok_button, &QPushButton::clicked, &dialog, &QDialog::accept);
  if (cancel_button)
    QObject::connect(cancel_button, &QPushButton::clicked, &dialog, &QDialog::reject);

  auto* layout = new QVBoxLayout(&dialog);
  layout->addLayout(message_row);
  layout->addSpacing(6);
  layout->addWidget(check_box);
  layout->addSpacing(6);
  layout->addLayout(button_row);
  // Fit the dialog to its content (no clipped text, not resizable).
  layout->setSizeConstraint(QLayout::SetFixedSize);

  // Center the dialog over the canvas. (Under native Wayland the compositor
  // places dialogs and ignores the requested position.)
  dialog.adjustSize();
  QWidget* canvas = graphics_view->viewport();
  QPoint center = canvas->mapToGlobal(canvas->rect().center());
  dialog.move(center - dialog.rect().center());

  bool accepted = (dialog.exec() == QDialog::Accepted);
  if (check_box->isChecked()) a_toggle->setChecked(false);
  return (cancel_button && ! accepted) ? QMessageBox::Cancel : QMessageBox::Ok;
}

bool Main_window::eventFilter(QObject* a_object, QEvent* a_event) {
  if ((a_object == graphics_view->viewport()) && (a_event->type() == QEvent::Paint))
    update_unbounded_rect();
  if ((a_object == m_console_input) && m_console_input) {
    if (a_event->type() == QEvent::ShortcutOverride) {
      auto* ke = static_cast<QKeyEvent*>(a_event);
      if (ke->modifiers().testFlag(Qt::ControlModifier) && (ke->key() != Qt::Key_Q)) {
        a_event->accept();
        return true;
      }
    }
    if (a_event->type() == QEvent::KeyPress) {
      auto* ke = static_cast<QKeyEvent*>(a_event);
      if ((ke->key() == Qt::Key_Up) || (ke->key() == Qt::Key_Down)) {
        int n = m_console_history.size();
        int pos = m_console_history_pos + ((ke->key() == Qt::Key_Up) ? -1 : 1);
        if ((pos >= 0) && (pos <= n)) {
          m_console_history_pos = pos;
          m_console_input->setText((pos < n) ? m_console_history[pos] : QString());
        }
        return true;
      }
    }
  }
  return CGAL::Qt::DemosMainWindow::eventFilter(a_object, a_event);
}

void Main_window::update_unbounded_rect() {
  QRectF rect = graphics_view->mapToScene(graphics_view->viewport()->rect()).
    boundingRect();
  double margin = 0.05 * std::max(rect.width(), rect.height());
  rect.adjust(-margin, -margin, margin, margin);
  QRectF& unbounded_rect = CGAL::Qt::Piecewise_graphics_item_base::unbounded_rect();
  if ((rect == unbounded_rect) || states_stack.empty()) return;
  unbounded_rect = rect;
  for (auto& curve_set : states_stack.back().m_curve_sets)
    curve_set.unbounded_rect_changed();
}

void Main_window::resizeEvent(QResizeEvent* e) {
  this->graphics_view->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  this->graphics_view->scale(2.5, 2.5);
  model_changed();
}

void Main_window::about_cgal() {
  QString title = "About CGAL";
  QString html_resource_name = "help/about_CGAL.html";

  QFile about_CGAL(html_resource_name);
  about_CGAL.open(QIODevice::ReadOnly);
  QString about_CGAL_txt = QTextStream(&about_CGAL).readAll();
#ifdef CGAL_VERSION_STR
  QString cgal_version(CGAL_VERSION_STR);
#  ifdef CGAL_FAKE_PUBLIC_RELEASE
  cgal_version.replace(QRegExp("-Ic?.*"), "");
#  endif
  about_CGAL_txt.replace("<!--CGAL_VERSION-->", QString(" (version %1)").arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 550px; min-height: 250px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel* mb_label = mb.findChild<QLabel*>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() | ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void Main_window::about_minkowski_sums() {
  QString title = "About 2D Minkowski sums";
  QString html_resource_name = "help/About2DMinkowskisums.html";
  QFile about_CGAL(html_resource_name);
  about_CGAL.open(QIODevice::ReadOnly);
  QString about_CGAL_txt = QTextStream(&about_CGAL).readAll();
#ifdef CGAL_VERSION_STR
  QString cgal_version(CGAL_VERSION_STR);
#  ifdef CGAL_FAKE_PUBLIC_RELEASE
  cgal_version.replace(QRegExp("-Ic?.*"), "");
#  endif
  about_CGAL_txt.replace("<!--CGAL_VERSION-->", QString(" (version %1)").arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel* mb_label = mb.findChild<QLabel*>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() | ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void Main_window::about_boolean_set_operations() {

  QString title = "About 2D Minkowski sums";
  QString html_resource_name = "help/About2DRegularizedBooleanOperations.html";
  QFile about_CGAL(html_resource_name);
  about_CGAL.open(QIODevice::ReadOnly);
  QString about_CGAL_txt = QTextStream(&about_CGAL).readAll();
#ifdef CGAL_VERSION_STR
  QString cgal_version(CGAL_VERSION_STR);
#  ifdef CGAL_FAKE_PUBLIC_RELEASE
  cgal_version.replace(QRegExp("-Ic?.*"), "");
#  endif
  about_CGAL_txt.replace("<!--CGAL_VERSION-->", QString(" (version %1)").arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel* mb_label = mb.findChild<QLabel*>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() | ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void Main_window::about_demo() {
  QString title = "About Demo";
  QString html_resource_name = "help/aboutThisDemo.html";
  QFile about_CGAL(html_resource_name);
  about_CGAL.open(QIODevice::ReadOnly);
  QString about_CGAL_txt = QTextStream(&about_CGAL).readAll();
#ifdef CGAL_VERSION_STR
  QString cgal_version(CGAL_VERSION_STR);
#  ifdef CGAL_FAKE_PUBLIC_RELEASE
  cgal_version.replace(QRegExp("-Ic?.*"), "");
#  endif
  about_CGAL_txt.replace("<!--CGAL_VERSION-->", QString(" (version %1)").arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel* mb_label = mb.findChild<QLabel*>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() | ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

template <typename Set>
bool Main_window::add_constructed_polygon(Curve_type a_type, typename Set::Polygon_2 a_polygon, bool a_to_result) {
  if (! ensure_curve_type(a_type)) return false;
  if (a_polygon.orientation() == CGAL::CLOCKWISE) a_polygon.reverse_orientation();
  typename Set::Polygon_with_holes_2 pwh(a_polygon);
  if (a_to_result) {
    states_stack.back().result_set().get<Set>().join(pwh);
    return false;
  }
  get_new_state(State_op::SO_DRAW);
  states_stack.back().set(m_color_active).get<Set>().join(pwh);
  return true;
}

void Main_window::log_join(const QString& a_polygon) {
  log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), a_polygon)});
}

void Main_window::enter_equation() {
  Polynomial_dialog dialog(polynomial_parser(), this);
  dialog.set_text(m_equation);
  if (dialog.exec() != QDialog::Accepted) return;
  m_equation = dialog.text();
  QString polynomial = QString::fromStdString(to_string(*dialog.polynomial()));
  std::vector<Algebraic_polygon> polygons;
  try {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    polygons = algebraic_polygons(*dialog.polynomial());
    QApplication::restoreOverrideCursor();
  }
  catch (const std::exception& e) {
    QApplication::restoreOverrideCursor();
    show_error("Cannot analyze the curve " + polynomial.toStdString() + " == 0: " + e.what());
    return;
  }
  if (polygons.empty()) {
    show_warning("The curve " + polynomial.toStdString() + " == 0 encloses no bounded region.");
    return;
  }
  if (! ensure_curve_type(Curve_type::CT_ALGEBRAIC)) return;
  try {
    get_new_state(State_op::SO_DRAW);
    auto& set = states_stack.back().set(m_color_active).algebraic();
    for (const auto& polygon : polygons) set.join(Algebraic_polygon_with_holes(polygon));
    // The polygons of algebraic curves are not written to programs yet.
    log_statements({QString("# %1.join(<the region enclosed by %2 == 0>)")
                      .arg(bucket_name(m_color_active), polynomial)});
  }
  catch (const std::exception& e) {
    show_error("Cannot add the region enclosed by " + polynomial.toStdString() + " == 0: " + e.what());
    undo();
  }
  model_changed();
}

void Main_window::process_input(CGAL::Object o) {
  try {
    if (auto* pgn = CGAL::object_cast<Conic_polygon>(&o)) {
      Conic_polygon ccw = *pgn;
      if (ccw.orientation() == CGAL::CLOCKWISE) ccw.reverse_orientation();
      if (add_constructed_polygon<Conic_polygon_set>(Curve_type::CT_CONIC, ccw, false)) {
        try {
          log_join(to_python(ccw));
        }
        catch (const std::invalid_argument& e) {
          log_statements({QString("# The %1 polygon cannot be written exactly: %2")
                            .arg(bucket_name(m_color_active), e.what())});
        }
      }
    }
    else if (auto* bezier = CGAL::object_cast<std::pair<Bezier_polygon, Bezier_boundary_source>>(&o))
      add_constructed_polygon<Bezier_polygon_set>(Curve_type::CT_BEZIER, bezier->first,
                                                  m_bezier_input->is_bounding_rect());
    else if (auto* circular = CGAL::object_cast<std::pair<Circular_polygon, Circular_boundary_source>>(&o)) {
      // The program constructs the polygon from its source, in counterclockwise order.
      bool clockwise = circular->first.orientation() == CGAL::CLOCKWISE;
      if (add_constructed_polygon<Circular_polygon_set>(Curve_type::CT_CIRCULAR, circular->first,
                                                        m_circular_input->is_bounding_rect()))
        log_join(to_python(clockwise ? reversed(circular->second) : circular->second));
    }
    else if (auto* pgn = CGAL::object_cast<Polyline_polygon>(&o))
      add_constructed_polygon<Polyline_polygon_set>(Curve_type::CT_POLYLINE, *pgn,
                                                    m_polyline_input->is_bounding_rect());
    else if (auto* pgn = CGAL::object_cast<Linear_polygon>(&o)) {
      // A linear polygon may also be a hole to subtract, or a part of a Minkowski sum (not an action on its own).
      if (! ensure_curve_type(Curve_type::CT_LINEAR)) return;
      Linear_polygon l_li = *pgn;
      if (l_li.orientation() == CGAL::CLOCKWISE) l_li.reverse_orientation();
      Linear_polygon_with_holes l_cpwh(l_li);
      if (! m_linear_input->is_bounding_rect() && ! m_linear_input->ishole()) {
        if (! m_linear_input->is_mink()) get_new_state(State_op::SO_DRAW);
        states_stack.back().set(m_color_active).linear().join(l_cpwh);
        if (! m_linear_input->is_mink())
          log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), to_python(l_li))});
      }
      else {
        states_stack.back().result_set().linear().join(l_cpwh);
        if (m_linear_input->ishole()) {
          log_statements({QString("%1.difference(%2)").arg(bucket_name(m_color_active), to_python(l_li))});
          states_stack.back().set(m_color_active).difference(states_stack.back().result_set());
          states_stack.back().result_set().clear();
        }
      }
    }
  }
  catch (const std::exception& e) {
    undo();
  }
  model_changed();
}
