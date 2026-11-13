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

/*! \file Main_window_help.cpp
 * The help of the buttons, shown in the What's This mode.
 */

#include <QWhatsThis>

#include "Main_window.h"

void Main_window::setup_help() {
  auto* help = QWhatsThis::createAction(this);
  help->setIcon(curve_tool_icon("help"));
  help->setToolTip("What's This? (Shift+F1): click a button to see its help");
  tools_tool_bar->addSeparator();
  tools_tool_bar->addAction(help);
  menu_help->insertAction(menu_help->actions().value(0), help);

  const QString step_back = "<p>Escape (or Backspace) undoes one step: it discards the curve under construction, if "
    "there is one, and otherwise removes the last curve.</p>";
  const QString family_switch = "<p>Switching to another family discards the polygons (after a confirmation) and the "
    "undo history.</p>";

  // The curve families.
  action_insert_linear->setWhatsThis("<b>Segments</b><p>Polygons bounded by line segments. Click at each vertex; "
                                     "right-click to close the polygon.</p>" + step_back +
                                     family_switch);
  action_insert_circular->setWhatsThis("<b>Line Segments and Circular Arcs</b><p>Polygons bounded by segments and "
                                       "circular arcs, or circles. Choose how to construct them in the tool bar "
                                       "underneath.</p>" + step_back + family_switch);
  action_insert_conic->setWhatsThis("<b>Conic Arcs</b><p>Polygons bounded by arcs of lines, circles, and ellipses, "
                                    "or full circles and ellipses. Choose how to construct them in the tool bar "
                                    "underneath.</p>" + step_back + family_switch);
  action_insert_bezier->setWhatsThis("<b>Bezier Curves</b><p>Polygons bounded by Bezier curves.</p><ul>"
                                     "<li>Click to add a corner.</li>"
                                     "<li>Press and drag to add a smooth vertex; the drag sets its two (symmetric) "
                                     "handles, which bend the curves on both sides.</li>"
                                     "<li>Right-click to close the polygon; right-press and drag to smooth the last "
                                     "vertex.</li></ul>"
                                     "<p>A polygon of clicks only is bounded by straight curves.</p>" + step_back +
                                     family_switch);
  action_insert_algebraic->setWhatsThis("<b>Algebraic Curves</b><p>Regions bounded by algebraic curves, entered as "
                                        "equations with the Equation button underneath. Available only if the demo "
                                        "is built with Lexy.</p>" + family_switch);
  action_insert_polyline->setWhatsThis("<b>Polyline/Polycurve</b><p>Polygons bounded by polycurves, that is, "
                                       "curves that consist of curves of the family selected underneath (Segments "
                                       "only, for now). Click at each vertex; right-click to close the polygon.</p>" +
                                       step_back + family_switch);

  // The tools.
  action_add_color->setWhatsThis("<b>Add a Bucket</b><p>Adds a bucket (a row of the Operations panel), which holds a "
                                 "set of polygons.</p>");
  action_minus_color->setWhatsThis("<b>Remove a Bucket</b><p>Removes the last bucket and its polygons.</p>");
  action_recenter->setWhatsThis("<b>Recenter</b><p>Zooms to fit all the polygons in the view.</p>");
  action_pan->setWhatsThis("<b>Pan</b><p>Drag the view to move it; select a curve family to construct polygons "
                           "again. The mouse wheel zooms in either mode.</p>");
  action_delete_all->setWhatsThis("<b>Clear All Buckets</b><p>Removes the polygons of all the buckets. Undo "
                                  "restores them.</p>");
  action_axis->setWhatsThis("<b>Axes</b><p>Shows or hides the coordinate axes.</p>");
  action_grid->setWhatsThis("<b>Grid</b><p>Shows or hides the grid. Its density is set in the View menu.</p>");
  action_snap_to_grid->setWhatsThis("<b>Snap to Grid</b><p>Moves the clicked points to the nearest grid point.</p>");
  action_undo->setWhatsThis("<b>Undo</b><p>Undoes the last change of the buckets: a polygon, an operation, or a "
                            "clearing.</p>");
  action_new->setWhatsThis("<b>Reset</b><p>Starts over: clears all the buckets and the program, and restores the "
                           "initial buckets and marks of the Operations panel.</p>");

  // The Operations panel.
  const QString target = "<p>The result is added to the Draw/Insert bucket. If that bucket is not empty, you are "
    "asked first; it cannot be an operand unless it is empty.</p>";
  visible_icon->setWhatsThis("<b>Visible</b><p>Shows or hides the polygons of each bucket. The box in the Buckets "
                             "row shows or hides all of them.</p>");
  visible_header->setWhatsThis(visible_icon->whatsThis());
  draw_header->setWhatsThis("<b>Draw/Insert</b><p>The bucket that receives the polygons you construct or open, and "
                            "the results of the operations.</p>");
  action_complement_h->setWhatsThis("<b>Complement</b><p>The complement of the selected bucket.</p>" + target);
  action_intersection_h->setWhatsThis("<b>Intersection</b><p>The intersection of the checked buckets.</p>" + target);
  action_union_h->setWhatsThis("<b>Union</b><p>The union of the checked buckets.</p>" + target);
  action_difference_h->setWhatsThis("<b>Difference</b><p>The difference of the two checked buckets: the upper one "
                                    "minus the lower one.</p>" + target);
  action_symmetric_difference_h->setWhatsThis("<b>Symmetric Difference</b><p>The symmetric difference of the two "
                                              "checked buckets.</p>" + target);
  action_minkowski_sum_h->setWhatsThis("<b>Minkowski Sum</b><p>The Minkowski sum of the two checked buckets, each "
                                       "holding a single polygon (possibly with holes). Segments only.</p>" + target);
  action_copy_h->setWhatsThis("<b>Copy</b><p>Adds the polygons of the checked bucket to the Draw/Insert "
                              "bucket.</p>");
  action_move_h->setWhatsThis("<b>Move</b><p>Moves the polygons of the checked bucket to the Draw/Insert "
                              "bucket.</p>");
  action_clear_h->setWhatsThis("<b>Clear</b><p>Removes the polygons of the checked buckets. The box in the Buckets "
                               "row checks or unchecks all of them.</p>");
  show_clear->setWhatsThis(action_clear_h->whatsThis());

  // The view, the console, and the log.
  graphics_view->setWhatsThis("<b>View</b><p>Construct polygons of the selected curve family here. The mouse wheel "
                              "zooms; the Pan button lets you drag the view.</p>");
  m_console_input->setWhatsThis("<b>Console</b><p>Type Python-style statements on the buckets (e.g., "
                                "<tt>blue.intersection(red, black)</tt>) and press Enter. Only Segments are "
                                "supported.</p>");
  m_log->setWhatsThis("<b>Log</b><p>The program: the statements that reproduce the buckets, whether executed by the "
                      "buttons or in the console. Save Program writes it as a Python script.</p>");
}
