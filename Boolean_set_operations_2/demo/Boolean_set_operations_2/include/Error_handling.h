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

#ifndef CGAL_ERROR_HANDLING_H
#define CGAL_ERROR_HANDLING_H

/*! \file Error_handling.h
 * Reporting errors: message boxes for warnings and errors, and the handler of failed CGAL assertions and preconditions.
 */

#include <optional>

#include <QAction>
#include <QMessageBox>
#include <QString>

/*! shows a message over the active window (or the main window), centered over its view if it has one. The text is
 * plain and wrapped to a bounded width. The severity is conveyed by the icon: Critical for a failed operation, Warning
 * for an operation that is not carried out because of the input, and Question for a question. The buttons may be
 * Ok, Cancel, Yes, and No; the affirmative one is on the left, the negative one on the right, and neither is the
 * default unless it is the only one. If a_toggle is given, a "Don't show this message again" check box is added;
 * checking it unchecks a_toggle. \return the button the user clicked (Escape and the close box count as the
 * negative button when one is offered, and as Ok otherwise).
 */
QMessageBox::StandardButton show_message(QMessageBox::Icon a_icon, const QString& a_title, const QString& a_text,
                                         QMessageBox::StandardButtons a_buttons = QMessageBox::Ok,
                                         QAction* a_toggle = nullptr);

/*! asks for an integer in [a_min, a_max], starting at a_value, in a message box like those of show_message() with
 * a spin box below the text. \return the integer, or nothing if the user cancels.
 */
std::optional<int> ask_integer(const QString& a_title, const QString& a_text, int a_value, int a_min, int a_max);

/*! shows a warning: the operation is not carried out because of the input.
 */
void show_warning(const QString& a_title, const QString& a_text);

/*! shows an error: the operation failed.
 */
void show_error(const QString& a_title, const QString& a_text);

/*! handles a failed CGAL assertion or precondition by showing an error and throwing a std::runtime_error.
 */
void error_handler(const char* what, const char* expr, const char* file, int line, const char* msg);

#endif
