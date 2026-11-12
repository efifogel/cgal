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

#ifndef CGAL_POLYNOMIAL_DIALOG_H
#define CGAL_POLYNOMIAL_DIALOG_H

/*! \file Polynomial_dialog.h
 * A dialog in which the user types the equation of an algebraic curve. The equation is parsed as it is typed; the
 * dialog shows the polynomial it stands for, or why it is not an equation, and accepts only an equation.
 */

#include <optional>

#include <QDialog>
#include <QString>

#include "Algebraic_curves.h"

class QDialogButtonBox;
class QLabel;
class QLineEdit;

/*! The dialog of the equation of an algebraic curve.
 */
class Polynomial_dialog : public QDialog {
public:
  /*! constructs from the parser of the equations (empty if there is none yet) and the parent widget.
   */
  Polynomial_dialog(Polynomial_parser a_parser, QWidget* a_parent = nullptr);

  /*! sets the text of the equation (e.g., the previous one, to edit).
   */
  void set_text(const QString& a_text);

  /*! obtains the text of the equation.
   */
  QString text() const;

  /*! obtains the polynomial of the equation, or nothing if the text is not an equation.
   */
  const std::optional<Bivariate_polynomial>& polynomial() const { return m_polynomial; }

private:
  /*! parses the text, and shows the polynomial or the error.
   */
  void parse();

  Polynomial_parser m_parser;
  std::optional<Bivariate_polynomial> m_polynomial;
  QLineEdit* m_edit;
  QLabel* m_status;
  QDialogButtonBox* m_buttons;
};

#endif
