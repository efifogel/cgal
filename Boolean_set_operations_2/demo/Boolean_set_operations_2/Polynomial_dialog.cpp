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

/*! \file Polynomial_dialog.cpp
 * The dialog of the equation of an algebraic curve.
 */

#include "Polynomial_dialog.h"

#include <string>
#include <utility>

#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

Polynomial_dialog::Polynomial_dialog(Polynomial_parser a_parser, QWidget* a_parent) :
  QDialog(a_parent),
  m_parser(std::move(a_parser)),
  m_edit(new QLineEdit),
  m_status(new QLabel),
  m_buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel))
{
  setWindowTitle(tr("Algebraic Curve"));
  auto* prompt = new QLabel(tr("The equation of the curve in x and y, e.g., x^2 + 3*y^2 == 10 (an expression p stands "
                               "for p == 0). Numbers are integers or rationals, written as 1/2 or RN(1, 2)."));
  prompt->setWordWrap(true);
  m_edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  m_edit->setPlaceholderText("x^2 + 3*y^2 == 10");
  m_status->setWordWrap(true);
  m_status->setTextInteractionFlags(Qt::TextSelectableByMouse);

  auto* layout = new QVBoxLayout(this);
  layout->addWidget(prompt);
  layout->addWidget(m_edit);
  layout->addWidget(m_status);
  layout->addWidget(m_buttons);
  resize(560, sizeHint().height());

  QObject::connect(m_edit, &QLineEdit::textChanged, this, [this]() { parse(); });
  QObject::connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  QObject::connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  parse();
}

void Polynomial_dialog::set_text(const QString& a_text) { m_edit->setText(a_text); }

QString Polynomial_dialog::text() const { return m_edit->text(); }

void Polynomial_dialog::parse() {
  m_polynomial.reset();
  QString status;
  bool error = true;
  if (! m_parser) status = tr("No parser of equations is available yet.");
  else if (m_edit->text().trimmed().isEmpty()) {
    status = tr("Type an equation.");
    error = false;
  }
  else {
    std::string message;
    m_polynomial = m_parser(m_edit->text().toStdString(), message);
    if (! m_polynomial) status = QString::fromStdString(message);
    else if (m_polynomial->empty()) {
      m_polynomial.reset();
      status = tr("The equation holds everywhere (its polynomial is 0).");
    }
    else {
      status = QString::fromStdString(to_string(*m_polynomial)) + " == 0";
      error = false;
    }
  }
  m_status->setStyleSheet(error ? "color: #b00020;" : "");
  m_status->setText(status);
  m_buttons->button(QDialogButtonBox::Ok)->setEnabled(m_polynomial.has_value());
}
