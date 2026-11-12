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

MainWindow::MainWindow() :
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
  for (auto* box : sceneDockWidget->findChildren<QCheckBox*>()) box->setStyle(indicator_style);
  for (auto* button : sceneDockWidget->findChildren<QRadioButton*>()) button->setStyle(indicator_style);

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
    if (QWidget* old_panel = infoDockWidget->widget()) old_panel->deleteLater();
    infoDockWidget->setWidget(panel);
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
    if (QWidget* old_panel = consoleDockWidget->widget()) old_panel->deleteLater();
    consoleDockWidget->setWidget(panel);
    QObject::connect(m_console_input, &QLineEdit::returnPressed, this, [this]() {
      QString line = m_console_input->text();
      m_console_input->clear();
      run_console_line(line);
    });
    m_console_input->installEventFilter(this);
  }

  // Grid: show/hide, spacing, and snapping of clicked points to the grid.
  QObject::connect(actionGrid, &QAction::toggled, this, [this](bool a_check) { m_scene.set_grid_visible(a_check); });
  auto* grid_snapper = new Grid_snapper(&m_scene, this);
  qApp->installEventFilter(grid_snapper);
  QObject::connect(actionSnapToGrid, &QAction::toggled, this, [grid_snapper](bool a_check) {
    grid_snapper->set_enabled(a_check);
  });
  // Grid density: step through the spacings below. A denser grid has a smaller spacing. The menu items show the spacing
  // they lead to, and are disabled at the ends of the range.
  static const std::array<double, 7> spacings = {1, 2, 5, 10, 20, 50, 100};
  auto update_density_actions = [this]() {
    auto it = std::find(spacings.begin(), spacings.end(), m_scene.grid_spacing());
    bool denser = (it != spacings.begin());
    bool sparser = (it + 1 != spacings.end());
    actionGridDenser->setEnabled(denser);
    actionGridSparser->setEnabled(sparser);
    actionGridDenser->setText(denser ? QString("&Increase Grid Density (Spacing %1)").arg(*(it - 1)) :
      QString("&Increase Grid Density"));
    actionGridSparser->setText(sparser ? QString("&Decrease Grid Density (Spacing %1)").arg(*(it + 1)) :
      QString("&Decrease Grid Density"));
  };
  auto change_density = [this, update_density_actions](int aDirection) {
    auto it = std::find(spacings.begin(), spacings.end(), m_scene.grid_spacing());
    it += aDirection;
    if ((it < spacings.begin()) || (it >= spacings.end())) return;
    m_scene.set_grid_spacing(*it);
    update_density_actions();
    statusBar()->showMessage(QString("Grid spacing: %1").arg(*it), 3000);
  };
  QObject::connect(actionGridDenser, &QAction::triggered, this, [change_density]() { change_density(-1); });
  QObject::connect(actionGridSparser, &QAction::triggered, this, [change_density]() { change_density(+1); });
  update_density_actions();

  // The cells of the bucket rows, by column, and the other widgets of the rows.
  m_show_column = {showRed, showBlack, showBlue, showBrown, showYellow, showMagenta, showAqua};
  m_draw_column = {drawRed, drawBlack, drawBlue, drawBrown, drawYellow, drawMagenta, drawAqua};
  m_complement_column = {showRedComp, showBlackComp, showBlueComp, showBrownComp, showYellowComp, showMagentaComp,
                         showAquaComp};
  m_intersection_column = {showRedInt, showBlackInt, showBlueInt, showBrownInt, showYellowInt, showMagentaInt,
                           showAquaInt};
  m_union_column = {showRedUnion, showBlackUnion, showBlueUnion, showBrownUnion, showYellowUnion, showMagentaUnion,
                    showAquaUnion};
  m_difference_column = {showRedDiff, showBlackDiff, showBlueDiff, showBrownDiff, showYellowDiff, showMagentaDiff,
                         showAquaDiff};
  m_symmetric_difference_column = {showRedSym_Diff, showBlackSym_Diff, showBlueSym_Diff, showBrownSym_Diff,
                                   showYellowSym_Diff, showMagentaSym_Diff, showAquaSym_Diff};
  m_minkowski_sum_column = {showRedMink_Sum, showBlackMink_Sum, showBlueMink_Sum, showBrownMink_Sum, showYellowMink_Sum,
                            showMagentaMink_Sum, showAquaMink_Sum};
  m_copy_column = {copyRed, copyBlack, copyBlue, copyBrown, copyYellow, copyMagenta, copyAqua};
  m_move_column = {moveRed, moveBlack, moveBlue, moveBrown, moveYellow, moveMagenta, moveAqua};
  m_clear_column = {clearRed, clearBlack, clearBlue, clearBrown, clearYellow, clearMagenta, clearAqua};
  m_bucket_labels = {showRedLabel, showBlackLabel, showBlueLabel, showBrownLabel, showYellowLabel, showMagentaLabel,
                     showAquaLabel};
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
    multi_operand_columns[] = {{&m_intersection_column, actionIntersectionH}, {&m_union_column, actionUnionH}};
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
    {&m_difference_column, actionDifferenceH}, {&m_symmetric_difference_column, actionSymmetric_DifferenceH},
    {&m_minkowski_sum_column, actionMinkowski_SumH}};
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
  for (auto* label : {VisibleIcon, DrawHeader}) {
    QPixmap pixmap = label->pixmap();
    if (! pixmap.isNull()) label->setPixmap(pixmap.scaled(27, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }

  // The scene and the view.
  m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
  m_scene.setSceneRect(-320, -210, 640, 420);
  this->graphicsView->setScene(&m_scene);
  // Use a white canvas regardless of the desktop color scheme, so that the
  // drawing area stands out from the surrounding widgets. (The scene, rather
  // than the view, paints it, so that the scene can draw the grid over it.)
  m_scene.setBackgroundBrush(Qt::white);
  this->graphicsView->setMouseTracking(true);
  this->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Turn the vertical axis upside down
  this->graphicsView->scale(2.5, -2.5);

  // The navigation adds zooming and translation functionality to the view.
  this->addNavigation(this->graphicsView);
  // Every change of the view (zoom, pan, resize) ends in a repaint, where the area that unbounded sets are drawn over
  // is updated.
  this->graphicsView->viewport()->installEventFilter(this);

  // The menus.
  this->setupStatusBar();
  this->setupOptionsMenu();

  // The recently opened files.
  this->addRecentFiles(this->menuFile, this->actionClose);

  // The first state of the undo stack.
  get_new_state(State_op::Start);
  actionUndo->setEnabled(false);

  this->setWindowState(Qt::WindowMaximized);

  // The inputs, which construct polygons with the mouse.
  m_bezier_input = new CGAL::Qt::Graphics_view_bezier_polygon_input<Bezier_traits>(this, &m_scene);
  m_linear_input = new CGAL::Qt::Graphics_view_linear_polygon_input<Kernel>(this, &m_scene);
  m_circular_input = new CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>(this, &m_scene);
  m_polyline_input = new CGAL::Qt::Graphics_view_polyline_input<Kernel>(this, &m_scene);
  m_conic_input = new CGAL::Qt::Graphics_view_conic_input(this, &m_scene);

  // The curve families. Selecting one in the tool bar starts constructing its curves.
  m_families = {{
    {Curve_type::Linear, "linear", actionInsertLinear, m_linear_input, [this] { m_linear_input->reset(); },
     actionOpenLinear, ".lps", &MainWindow::read_linear},
    {Curve_type::Circular, "circular", actionInsertCircular, m_circular_input, [this] { m_circular_input->reset(); },
     actionOpenDXF, ".cps", &MainWindow::read_circular},
    {Curve_type::Bezier, "Bezier", actionInsertBezier, m_bezier_input, [this] { m_bezier_input->reset(); },
     actionOpenBezier, ".bps", &MainWindow::read_bezier},
    {Curve_type::Polyline, "polyline", actionInsertPolyline, m_polyline_input, [this] { m_polyline_input->reset(); },
     actionOpenPolyline, ".mps", &MainWindow::read_polyline},
    {Curve_type::Conic, "conic", actionInsertConic, m_conic_input, [this] { m_conic_input->reset(); }, nullptr,
     nullptr, nullptr}}};
  for (const auto& family : m_families) {
    Curve_type type = family.m_type;
    QObject::connect(family.m_insert_action, &QAction::toggled, this, [this, type](bool checked) {
      if (checked) start_inserting(type);
    });
  }

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
    QObject::connect(circle, &QAction::toggled, this, [this](bool aChecked) {
      m_circular_input->set_mode(aChecked ? Circular_input::Circle_mode : Circular_input::Arcs);
    });

    // The conic tools: conics that bound a polygon, full curves, and a conic given by its coefficients.
    using Conic_input = CGAL::Qt::Graphics_view_conic_input;
    auto* conic_group = new QActionGroup(this);
    const std::vector<std::tuple<QString, QString, Conic_input::Mode, QString>>
      conic_tools = {
      {"line", "Line", Conic_input::LINE,
       "Line: press at one of its points and drag to another"},
      {"circle", "Circle", Conic_input::CIRCLE,
       "Circle: press at the center and drag to the radius"},
      {"ellipse", "Ellipse", Conic_input::ELLIPSE,
       "Ellipse: click at the center, press at the end of an axis, and "
       "drag to the width of the other axis"},
      {"full circle", "Full Circle", Conic_input::FULL_CIRCLE,
       "Full Circle: press at the center and drag to the radius"},
      {"full ellipse", "Full Ellipse", Conic_input::FULL_ELLIPSE,
       "Full Ellipse: click at the center, press at the end of an axis, and "
       "drag to the width of the other axis"}};
    const QString boundary_help = ". Every conic you add meets the previous one at the next vertex (click "
      "at it when there are several); on a circle or an ellipse, click near "
      "the arc to keep; right-click to close the boundary. Escape or Backspace "
      "discards the conic under construction, or else removes the last "
      "conic.";
    std::vector<QAction*> conic_actions;
    for (const auto& [icon, text, mode, tip] : conic_tools) {
      if (mode == Conic_input::FULL_CIRCLE)
        conic_actions.push_back(bar->addSeparator());
      auto* action = bar->addAction(curve_tool_icon(icon), text);
      action->setToolTip((mode < Conic_input::FULL_CIRCLE) ? tip + boundary_help : tip);
      action->setCheckable(true);
      conic_group->addAction(action);
      Conic_input::Mode m = mode;
      QObject::connect(action, &QAction::triggered, this, [this, m]() { m_conic_input->set_mode(m); });
      conic_actions.push_back(action);
    }
    conic_actions[0]->setChecked(true);
    conic_actions.push_back(bar->addSeparator());
    auto* coefficients = bar->addAction(curve_tool_icon("coefficients"), "Coefficients...");
    coefficients->setToolTip("Coefficients: enter a conic " "r x\u00b2 + s y\u00b2 + t xy + u x + v y + w = 0, "
                             "and add it to the boundary or construct the full " "ellipse");
    QObject::connect(coefficients, &QAction::triggered, this, [this]() { m_conic_input->enter_coefficients(this); });
    conic_actions.push_back(coefficients);

    // Keep the height of the bar when it shows no buttons.
    bar->setMinimumHeight(bar->sizeHint().height());
    auto show_tools = [this, arcs, circle, conic_actions]() {
      bool circular = actionInsertCircular->isChecked();
      arcs->setVisible(circular);
      circle->setVisible(circular);
      for (auto* action : conic_actions)
        action->setVisible(actionInsertConic->isChecked());
    };
    show_tools();
    QObject::connect(actionInsertCircular, &QAction::toggled, this, show_tools);
    QObject::connect(actionInsertConic, &QAction::toggled, this, show_tools);
    verticalLayout->insertWidget(1, bar);
  }

  // The polygons constructed by the inputs are added to the Draw/Insert bucket.
  for (const auto& family : m_families) {
    QObject::connect(family.m_input, &CGAL::Qt::GraphicsViewInput::generate, this, &MainWindow::process_input);
  }

  m_scene.installEventFilter(m_linear_input);

  // Under Wayland, let the compositor move floating dock widgets.
  if (QGuiApplication::platformName().startsWith("wayland")) {
    auto* dock_mover = new Floating_dock_mover(this);
    for (auto* dock : findChildren<QDockWidget*>()) dock->installEventFilter(dock_mover);
  }

  QObject::connect(this->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

  // The actions and the widgets of the panels.
  QObject::connect(actionNew, &QAction::triggered, this, &MainWindow::new_program);
  QObject::connect(actionUndo, &QAction::triggered, this, &MainWindow::undo);
  QObject::connect(actionAxis, &QAction::triggered, this, &MainWindow::toggle_axes);
  QObject::connect(actionPAN, &QAction::triggered, this, &MainWindow::toggle_pan);
  QObject::connect(actionRecenter, &QAction::triggered, this, &MainWindow::zoom_to_fit);
  QObject::connect(actionOpenLinear, &QAction::triggered, this, &MainWindow::open_linear);
  QObject::connect(actionOpenDXF, &QAction::triggered, this, &MainWindow::open_circular);
  QObject::connect(actionOpenBezier, &QAction::triggered, this, &MainWindow::open_bezier);
  QObject::connect(actionOpenPolyline, &QAction::triggered, this, &MainWindow::open_polyline);
  QObject::connect(actionSaveCurrentBucket, &QAction::triggered, this, &MainWindow::save_draw_bucket);
  QObject::connect(actionAddColor, &QAction::triggered, this, &MainWindow::add_bucket);
  QObject::connect(actionMinusColor, &QAction::triggered, this, &MainWindow::remove_bucket);
  QObject::connect(actionDeleteAll, &QAction::triggered, this, &MainWindow::delete_all);
  QObject::connect(showColorBucket, &QAction::toggled, this, &MainWindow::show_operations_panel);
  QObject::connect(showConsole, &QAction::toggled, this, &MainWindow::show_console_panel);
  QObject::connect(showInfo, &QAction::toggled, this, &MainWindow::show_log_panel);
  QObject::connect(aboutBolop, &QAction::triggered, this, &MainWindow::about_boolean_set_operations);
  QObject::connect(aboutCGAL, &QAction::triggered, this, &MainWindow::about_cgal);
  QObject::connect(aboutMinkop, &QAction::triggered, this, &MainWindow::about_minkowski_sums);
  QObject::connect(aboutDemo, &QAction::triggered, this, &MainWindow::about_demo);
  QObject::connect(actionComplementH, &QAbstractButton::toggled, this, &MainWindow::apply_complement);
  QObject::connect(actionIntersectionH, &QAbstractButton::toggled, this, &MainWindow::apply_intersection);
  QObject::connect(actionUnionH, &QAbstractButton::toggled, this, &MainWindow::apply_union);
  QObject::connect(actionDifferenceH, &QAbstractButton::toggled, this, &MainWindow::apply_difference);
  QObject::connect(actionSymmetric_DifferenceH, &QAbstractButton::toggled, this,
                   &MainWindow::apply_symmetric_difference);
  QObject::connect(actionMinkowski_SumH, &QAbstractButton::toggled, this, &MainWindow::apply_minkowski_sum);
  QObject::connect(actionCopyH, &QAbstractButton::toggled, this, &MainWindow::apply_copy);
  QObject::connect(actionMoveH, &QAbstractButton::toggled, this, &MainWindow::apply_move);
  QObject::connect(actionClearH, &QAbstractButton::toggled, this, &MainWindow::apply_clear);
  QObject::connect(showResult, &QAbstractButton::toggled, this, &MainWindow::show_result);
  QObject::connect(VisibleHeader, &QAbstractButton::toggled, this, &MainWindow::show_all_buckets);
  QObject::connect(showClear, &QAbstractButton::toggled, this, &MainWindow::mark_all_for_clearing);
  QObject::connect(sceneDockWidget, &QDockWidget::visibilityChanged, this,
                   &MainWindow::operations_panel_visibility_changed);
  QObject::connect(infoDockWidget, &QDockWidget::visibilityChanged, this, &MainWindow::log_panel_visibility_changed);
  QObject::connect(consoleDockWidget, &QDockWidget::visibilityChanged, this,
                   &MainWindow::console_panel_visibility_changed);

  // Opening a recently opened file.
  QObject::connect(this, SIGNAL(openRecentFile(QString)), this, SLOT(open(QString)));

  QObject::connect(actionErrorOperand, &QAction::toggled, this, [this](bool a_check) { operand_error_warn = a_check; });
  QObject::connect(actionWarnNotEmpty, &QAction::toggled, this, [this](bool a_check) { empty_warn = a_check; });

  this->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

  // The axes.
  QPen axis_pen(QBrush(Qt::black), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
  xAxis->setLine(-155000000000000, 0, 155000000000000, 0);
  xAxis->setPen(axis_pen);
  yAxis->setLine(0, -100000000000000, 0, 100000000000000);
  yAxis->setPen(axis_pen);

  m_linear_input->mOngoingPieceGI->set_pen(sPens[0]);
  m_linear_input->mLinearGI->set_pen(sPens[0]);
  m_linear_input->mHandleGI->setPen(sPens[0]);
  actionOpenLinear->setEnabled(true);
  actionOpenDXF->setEnabled(false);
  actionOpenBezier->setEnabled(false);
  actionOpenPolyline->setEnabled(false);
}

void MainWindow::show_result(bool a_check) { states_stack.back().toggle_view(result_index, a_check); }

void MainWindow::show_bucket(std::size_t aIndex, bool aChecked) {
  states_stack.back().toggle_view(aIndex, aChecked);
  m_draw_column[aIndex]->setEnabled(aChecked);
  if (aIndex >= m_number_of_buckets) return;
  auto visible = number_of_visible_buckets();
  if (aChecked) {
    if (visible == 1) m_scene.installEventFilter(current_input());
  }
  else if (visible == 0) remove_inputs();
  else if (aIndex == m_color_active) {
    for (std::size_t i = 0; i < m_number_of_buckets; ++i) {
      if (m_show_column[i]->isChecked()) {
        m_draw_column[i]->setChecked(true);
        break;
      }
    }
  }
  // The header is checked when all buckets are visible.
  QSignalBlocker blocker(VisibleHeader);
  VisibleHeader->setChecked(visible == m_number_of_buckets);
}

void MainWindow::set_draw_bucket(std::size_t aIndex) {
  m_color_active = aIndex;
  set_input_pen(sPens[aIndex]);
}

std::size_t MainWindow::number_of_visible_buckets() const {
  return std::count_if(m_show_column.begin(), m_show_column.begin() + m_number_of_buckets,
                       [](QAbstractButton* box) { return box->isChecked(); });
}

void MainWindow::set_number_of_buckets(std::size_t aNumber) {
  m_number_of_buckets = aNumber;
  const Column* columns[] = {&m_show_column, &m_draw_column, &m_complement_column, &m_intersection_column,
                             &m_union_column, &m_difference_column, &m_symmetric_difference_column,
                             &m_minkowski_sum_column, &m_copy_column, &m_move_column, &m_clear_column};
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    bool exists = (i < aNumber);
    for (const Column* column : columns) (*column)[i]->setVisible(exists);
    m_bucket_labels[i]->setVisible(exists);
    if (m_bucket_separators[i]) m_bucket_separators[i]->setVisible(exists);
  }

  // The vertical lines between the columns span the rows.
  int height = 155 + 30 * int(aNumber - min_number_of_buckets);
  for (auto* line : {line0006, line1, line2, line3, line4, line5, line6, line0007, line06, line061, line062, line063})
    line->setGeometry(QRect(line->x(), 0, 7, height));

  bool can_add = (aNumber < number_of_buckets);
  actionAddColor->setEnabled(can_add);
  actionAddColor->setText(can_add ? "Add a Bucket" : "Maximum Number of Buckets Reached");
  bool can_remove = (aNumber > min_number_of_buckets);
  actionMinusColor->setEnabled(can_remove);
  if (can_remove) {
    actionMinusColor->setText("Remove " + capitalized(bucket_name(aNumber - 1)));
  }
  else actionMinusColor->setText("Minimum Number of Buckets Reached");
}

void MainWindow::show_all_buckets(bool a_check) {
  for (auto* box : m_show_column) box->setChecked(a_check);
  if (a_check) m_draw_column[0]->setChecked(true);
}

void MainWindow::mark_all_for_clearing(bool a_check) {
  for (auto* box : m_clear_column) box->setChecked(a_check);
}

void MainWindow::operations_panel_visibility_changed(bool a_check) {
  if (a_check) showColorBucket->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) showColorBucket->setChecked(false);
}

void MainWindow::log_panel_visibility_changed(bool a_check) {
  if (a_check) showInfo->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) showInfo->setChecked(false);
}

void MainWindow::console_panel_visibility_changed(bool a_check) {
  if (a_check) showConsole->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) showConsole->setChecked(false);
}

void MainWindow::show_operations_panel(bool a_check) {
  if (a_check) sceneDockWidget->setVisible(true);
  else sceneDockWidget->setVisible(false);
}

void MainWindow::show_console_panel(bool a_check) {
  if (a_check) consoleDockWidget->setVisible(true);
  else consoleDockWidget->setVisible(false);
}

void MainWindow::show_log_panel(bool a_check) {
  if (a_check) infoDockWidget->setVisible(true);
  else infoDockWidget->setVisible(false);
}

void MainWindow::add_bucket() {
  if (m_number_of_buckets == number_of_buckets) return;
  log_statements({QString("%1 = Polygon_set_2()").arg(bucket_name(m_number_of_buckets))}, true);
  set_number_of_buckets(m_number_of_buckets + 1);
  model_changed();
}

void MainWindow::remove_bucket() {
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

void MainWindow::undo() {
  if (m_state_num > 1) {
    // The sets of the previous state are shown again by model_changed().
    states_stack.pop_back();
    if (states_stack.size() == 1) actionUndo->setEnabled(false);
    m_state_num = states_stack.size();
    trim_program_to_states();
    model_changed();
  }
  else {
    show_error("Nothing to Undo!!!");
  }
}

void MainWindow::reset_marks() {
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
  showResult->setChecked(false);
}

void MainWindow::new_program() {
  if (m_state_num > 1) get_new_state(State_op::Reset);

  // Reset starts a new program, which constructs the initial buckets.
  m_program.clear();
  if (m_log) m_log->clear();
  QStringList program;
  for (std::size_t i = 0; i < min_number_of_buckets; ++i)
    program << QString("%1 = Polygon_set_2()").arg(bucket_name(i));
  log_statements(program, true);

  for (std::size_t i = 0; i <= number_of_buckets; ++i) states_stack.back().clear(i);

  m_scene.setSceneRect(-320, -210, 640, 420);
  this->graphicsView->setScene(&m_scene);
  this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  this->graphicsView->scale(2.5, 2.5);

  m_disjoint = false;

  m_linear_input->reset();
  m_circular_input->reset();
  m_bezier_input->reset();
  m_polyline_input->reset();

  m_pan = false;

  m_color_active = 0;

  m_linear_input->mOngoingPieceGI->set_pen(sPens[0]);
  m_linear_input->mLinearGI->set_pen(sPens[0]);
  m_linear_input->mHandleGI->setPen(sPens[0]);

  sceneDockWidget->setVisible(true);
  consoleDockWidget->setVisible(true);
  infoDockWidget->setVisible(true);

  showColorBucket->setChecked(true);
  showConsole->setChecked(true);
  showInfo->setChecked(true);

  actionCopyH->setChecked(false);
  actionMoveH->setChecked(false);

  actionComplementH->setChecked(false);
  actionUnionH->setChecked(false);
  actionIntersectionH->setChecked(false);
  actionDifferenceH->setChecked(false);
  actionSymmetric_DifferenceH->setChecked(false);
  actionMinkowski_SumH->setChecked(false);

  m_reset = true;
  for (const auto& family : m_families) family.m_insert_action->setChecked(family.m_type == m_curve_type);

  m_reset = false;

  actionDifferenceH->setEnabled(true);

  reset_marks();

  set_number_of_buckets(min_number_of_buckets);
  VisibleHeader->setChecked(true);
  showClear->setChecked(false);
  line11->setVisible(true);

  zoom_to_fit();
  model_changed();
}

void MainWindow::switch_set_type(Curve_set& aSet, Curve_type aType) {
  aSet.reset_type(aType);
  model_changed();
}

void MainWindow::switch_sets_type(Curve_type aType) {
  for (std::size_t i = 0; i <= number_of_buckets; ++i) switch_set_type(states_stack.back().set(i), aType);
}

void MainWindow::toggle_axes() {
  if (! m_grid) {
    m_scene.addItem(xAxis.get());
    m_scene.addItem(yAxis.get());
    m_grid = true;
  }
  else {
    m_scene.removeItem(xAxis.get());
    m_scene.removeItem(yAxis.get());
    m_grid = false;
  }
  // Keep the button checked (highlighted) while the axes are shown.
  actionAxis->setChecked(m_grid);
  m_scene.update();
  model_changed();
}

void MainWindow::toggle_pan() {
  if (! m_pan) {
    remove_inputs();
    family(m_curve_type).m_insert_action->setChecked(false);
    this->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    this->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_scene.update();
    m_pan = true;
    model_changed();
  }
  else {
    family(m_curve_type).m_insert_action->setChecked(true);
    m_pan = false;
    model_changed();
  }
}

void MainWindow::wheelEvent(QWheelEvent* event) {

  this->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
  static const double scaleFactor = 1.15;
  static double currentScale = 1.0;  // stores the current scale value.
  static const double scaleMin = .1; // defines the min scale limit.

  if (m_scene.width() >= 1500000 && event->angleDelta().y() > 0) {
    this->graphicsView->setDragMode(QGraphicsView::NoDrag);
    this->graphicsView->setSceneRect(-1500000, -1000000, 3100000, 2000000);
    this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  }
  if (m_scene.width() <= 1500000) {
    if (event->angleDelta().y() > 0) {
      m_scene.setSceneRect(-m_scene.sceneRect().width() * scaleFactor / 2,
                           -m_scene.sceneRect().height() * scaleFactor / 2, m_scene.sceneRect().width() * scaleFactor,
                           m_scene.sceneRect().height() * scaleFactor);
      this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
      this->graphicsView->scale(2.5, 2.5);
    }
    else if (currentScale > scaleMin) {
      m_scene.setSceneRect(-m_scene.sceneRect().width() / (scaleFactor * 2),
                           -m_scene.sceneRect().height() / (scaleFactor * 2), m_scene.sceneRect().width() / scaleFactor,
                           m_scene.sceneRect().height() / scaleFactor);
      this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
      this->graphicsView->scale(2.5, 2.5);
    }
  }
  model_changed();
}

QIcon MainWindow::curve_tool_icon(const QString& aKind) {
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
  if (aKind.startsWith("full")) painter.setBrush(QColor(0, 255, 1, 140));
  QList<QPointF> vertices;
  if (aKind == "arcs") {
    QPainterPath path(QPointF(3, 20));
    path.lineTo(7, 9);
    path.arcTo(QRectF(7, 2, 14, 14), 180, -180);
    path.lineTo(17, 20);
    painter.drawPath(path);
    vertices << QPointF(3, 20) << QPointF(7, 9) << QPointF(21, 9)
             << QPointF(17, 20);
  }
  else if (aKind == "line") {
    painter.drawLine(QPointF(2, 20), QPointF(22, 4));
    vertices << QPointF(7, 16) << QPointF(17, 8);
  }
  else if ((aKind == "circle") || (aKind == "full circle")) {
    painter.drawEllipse(QPointF(12, 12), 9, 9);
    if (aKind == "circle") vertices << QPointF(3, 12) << QPointF(18.4, 5.6);
    else {
      painter.setPen(QPen(QColor(64, 64, 64), 1.5, Qt::DashLine));
      painter.drawLine(QPointF(12, 12), QPointF(18.4, 5.6));
      vertices << QPointF(12, 12);
    }
  }
  else if ((aKind == "ellipse") || (aKind == "full ellipse")) {
    painter.translate(12, 12);
    painter.rotate(-30);
    painter.drawEllipse(QPointF(0, 0), 10.5, 6);
    painter.resetTransform();
    if (aKind == "ellipse") vertices << QPointF(3, 17) << QPointF(21, 7);
  }
  else if (aKind == "coefficients") {
    QFont font = painter.font();
    font.setPixelSize(11);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(64, 64, 64));
    painter.drawText(QRectF(0, 0, 24, 24), Qt::AlignCenter, "ax²");
  }
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(255, 7, 0));
  for (const auto& p : vertices) painter.drawEllipse(p, 2, 2);
  return QIcon(pixmap);
}

QObject* MainWindow::current_input() const { return family(m_curve_type).m_input; }

void MainWindow::remove_inputs() {
  for (const auto& family : m_families) {
    m_scene.removeEventFilter(family.m_input);
    family.m_reset_input();
  }
}

void MainWindow::install_input(QObject* aInput) {
  for (const auto& family : m_families) m_scene.removeEventFilter(family.m_input);
  m_scene.installEventFilter(aInput);
}

void MainWindow::set_input_pen(const QPen& aPen) {
  m_linear_input->mOngoingPieceGI->set_pen(aPen);
  m_linear_input->mLinearGI->set_pen(aPen);
  m_linear_input->mHandleGI->setPen(aPen);
  m_circular_input->mOngoingPieceGI->set_pen(aPen);
  m_circular_input->mCircularGI->set_pen(aPen);
  m_circular_input->mHandleGI->setPen(aPen);
  m_bezier_input->m_ongoing_piece_GI->set_pen(aPen);
  m_bezier_input->m_bezier_GI->set_pen(aPen);
  m_bezier_input->m_handle0_GI->setPen(aPen);
  m_bezier_input->m_handle1_GI->setPen(aPen);
  m_polyline_input->mOngoingPieceGI->set_pen(aPen);
  m_polyline_input->mPolylineGI->set_pen(aPen);
  m_polyline_input->mHandleGI->setPen(aPen);
  m_conic_input->set_pen(aPen);
}

bool MainWindow::ensure_curve_type(Curve_type aType) {
  if (m_curve_type == aType) return true;
  QString name = family(aType).m_name;
  if (! are_all_sets_empty() &&
      ! ask_user_yesno(capitalized(name) + " mode switch",
                       QString("You are about to construct or load %1 polygons, but there are polygons of another "
                               "family already loaded.\n"
                               "Both families are not interoperable. In order to proceed, the polygons must be removed "
                               "first.\n"
                               "Once deleted you cannot undo the action.\n"
                               "Yes to remove and proceed?\n").arg(name)))
    return false;
  switch_sets_type(aType);
  for (const auto& other : m_families) {
    if (other.m_type != aType) other.m_reset_input();
  }
  m_curve_type = aType;
  return true;
}

void MainWindow::start_inserting(Curve_type aType) {
  const auto& selected = family(aType);
  this->graphicsView->setDragMode(QGraphicsView::NoDrag);
  if (! ensure_curve_type(aType)) {
    selected.m_insert_action->setChecked(false);
    return;
  }
  if (! m_pan && ! m_reset) {
    // Switching the family discards the history; the first state gets empty sets of the family.
    while (m_state_num > 1) undo();
    if (states_stack.back().set(0).type() != aType) switch_sets_type(aType);
  }
  actionPAN->setChecked(false);
  m_pan = false;
  for (const auto& other : m_families) {
    if (other.m_open_action) other.m_open_action->setEnabled(other.m_type == aType);
    if (other.m_type != aType) other.m_insert_action->setChecked(false);
  }
  install_input(selected.m_input);
  delete_result();
  actionMinkowski_SumH->setEnabled(aType == Curve_type::Linear);
  set_input_pen(sPens[m_color_active]);
}

void MainWindow::zoom_to_fit() {
  std::optional<QRectF> lTotalRect;
  for (auto si = states_stack.back().m_curve_sets.begin(); si != states_stack.back().m_curve_sets.end(); ++si) {
    if (! si->is_empty()) {
      QRectF lRect = si->bounding_rect();
      if (lTotalRect) lTotalRect = *lTotalRect | lRect;
      else lTotalRect = lRect;
    }
  }
  if (lTotalRect) {
    this->graphicsView->setSceneRect(*lTotalRect);
    this->graphicsView->fitInView(*lTotalRect, Qt::KeepAspectRatio);
  }
  model_changed();
}

QMessageBox::StandardButton
MainWindow::show_message(QMessageBox::Icon aIcon, const QString& aTitle, const QString& aText, QAction* aToggle,
                         QMessageBox::StandardButtons aButtons) {
  // Declared before the dialog, so that it outlives the check box using it.
  Visible_indicator_style indicator_style;
  QDialog dialog(this);
  dialog.setWindowTitle(aTitle);

  // The same icons QMessageBox uses.
  QStyle::StandardPixmap pixmap = QStyle::SP_MessageBoxInformation;
  switch (aIcon) {
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
  message_row->addWidget(new QLabel(aText), 1);

  auto* check_box = new QCheckBox("Don't show this message again");
  // Make the check box visible on dark themes, like those of the Operations panel.
  check_box->setStyle(&indicator_style);

  auto* ok_button = new QPushButton("OK");
  QPushButton* cancel_button = aButtons.testFlag(QMessageBox::Cancel) ? new QPushButton("Cancel") : nullptr;
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
  QWidget* canvas = graphicsView->viewport();
  QPoint center = canvas->mapToGlobal(canvas->rect().center());
  dialog.move(center - dialog.rect().center());

  bool accepted = (dialog.exec() == QDialog::Accepted);
  if (check_box->isChecked()) aToggle->setChecked(false);
  return (cancel_button && ! accepted) ? QMessageBox::Cancel : QMessageBox::Ok;
}

bool MainWindow::eventFilter(QObject* aObject, QEvent* aEvent) {
  if ((aObject == graphicsView->viewport()) && (aEvent->type() == QEvent::Paint))
    update_unbounded_rect();
  if ((aObject == m_console_input) && m_console_input) {
    if (aEvent->type() == QEvent::ShortcutOverride) {
      auto* ke = static_cast<QKeyEvent*>(aEvent);
      if (ke->modifiers().testFlag(Qt::ControlModifier) && (ke->key() != Qt::Key_Q)) {
        aEvent->accept();
        return true;
      }
    }
    if (aEvent->type() == QEvent::KeyPress) {
      auto* ke = static_cast<QKeyEvent*>(aEvent);
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
  return CGAL::Qt::DemosMainWindow::eventFilter(aObject, aEvent);
}

void MainWindow::update_unbounded_rect() {
  QRectF rect = graphicsView->mapToScene(graphicsView->viewport()->rect()).
    boundingRect();
  double margin = 0.05 * std::max(rect.width(), rect.height());
  rect.adjust(-margin, -margin, margin, margin);
  QRectF& unbounded_rect = CGAL::Qt::Piecewise_graphics_item_base::unbounded_rect();
  if ((rect == unbounded_rect) || states_stack.empty()) return;
  unbounded_rect = rect;
  for (auto& curve_set : states_stack.back().m_curve_sets)
    curve_set.unbounded_rect_changed();
}

void MainWindow::resizeEvent(QResizeEvent* e) {
  this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  this->graphicsView->scale(2.5, 2.5);
  model_changed();
}

void MainWindow::about_cgal() {
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

void MainWindow::about_minkowski_sums() {
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

void MainWindow::about_boolean_set_operations() {

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

void MainWindow::about_demo() {
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
void MainWindow::add_constructed_polygon(Curve_type aType, typename Set::Polygon_2 aPolygon, bool aToResult) {
  if (! ensure_curve_type(aType)) return;
  if (aPolygon.orientation() == CGAL::CLOCKWISE) aPolygon.reverse_orientation();
  typename Set::Polygon_with_holes_2 pwh(aPolygon);
  if (aToResult) {
    states_stack.back().result_set().get<Set>().join(pwh);
    return;
  }
  get_new_state(State_op::Draw);
  states_stack.back().set(m_color_active).get<Set>().join(pwh);
}

void MainWindow::process_input(CGAL::Object o) {
  try {
    if (auto* pgn = CGAL::object_cast<Conic_polygon>(&o))
      add_constructed_polygon<Conic_polygon_set>(Curve_type::Conic, *pgn, false);
    else if (auto* bezier = CGAL::object_cast<std::pair<Bezier_polygon, Bezier_boundary_source>>(&o))
      add_constructed_polygon<Bezier_polygon_set>(Curve_type::Bezier, bezier->first,
                                                  m_bezier_input->is_bounding_rect());
    else if (auto* pgn = CGAL::object_cast<Circular_polygon>(&o))
      add_constructed_polygon<Circular_polygon_set>(Curve_type::Circular, *pgn, m_circular_input->is_bounding_rect());
    else if (auto* pgn = CGAL::object_cast<Polyline_polygon>(&o))
      add_constructed_polygon<Polyline_polygon_set>(Curve_type::Polyline, *pgn, m_polyline_input->is_bounding_rect());
    else if (auto* pgn = CGAL::object_cast<Linear_polygon>(&o)) {
      // A linear polygon may also be a hole to subtract, or a part of a Minkowski sum (not an action on its own).
      if (! ensure_curve_type(Curve_type::Linear)) return;
      Linear_polygon lLI = *pgn;
      if (lLI.orientation() == CGAL::CLOCKWISE) lLI.reverse_orientation();
      Linear_polygon_with_holes lCPWH(lLI);
      if (! m_linear_input->is_bounding_rect() && ! m_linear_input->ishole()) {
        if (! m_linear_input->is_mink()) get_new_state(State_op::Draw);
        states_stack.back().set(m_color_active).linear().join(lCPWH);
        if (! m_linear_input->is_mink())
          log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), to_python(lLI))});
      }
      else {
        states_stack.back().result_set().linear().join(lCPWH);
        if (m_linear_input->ishole()) {
          log_statements({QString("%1.difference(%2)").arg(bucket_name(m_color_active), to_python(lLI))});
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
