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

/*! \file Error_handling.cpp
 * Reporting errors: message boxes for warnings and errors, and the handler of failed CGAL assertions and preconditions.
 */

#include "Error_handling.h"

#include <algorithm>
#include <stdexcept>

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

#include "Qt_helpers.h"

namespace {

// The window that shows the message: the active one, or else the main window.
QWidget* message_parent() {
  if (auto* active = QApplication::activeWindow()) return active;
  for (auto* widget : QApplication::topLevelWidgets())
    if (qobject_cast<QMainWindow*>(widget) && widget->isVisible()) return widget;
  return nullptr;
}

} // namespace

QMessageBox::StandardButton show_message(QMessageBox::Icon a_icon, const QString& a_title, const QString& a_text,
                                         QMessageBox::StandardButtons a_buttons, QAction* a_toggle) {
  QWidget* parent = message_parent();
  // Declared before the dialog, so that it outlives the check box using it.
  Visible_indicator_style indicator_style;
  QDialog dialog(parent);
  dialog.setWindowTitle(a_title);

  // The same icons QMessageBox uses.
  QStyle::StandardPixmap pixmap = QStyle::SP_MessageBoxInformation;
  switch (a_icon) {
   case QMessageBox::Warning: pixmap = QStyle::SP_MessageBoxWarning; break;
   case QMessageBox::Critical: pixmap = QStyle::SP_MessageBoxCritical; break;
   case QMessageBox::Question: pixmap = QStyle::SP_MessageBoxQuestion; break;
   default: break;
  }
  QStyle* style = dialog.style();
  int icon_size = style->pixelMetric(QStyle::PM_MessageBoxIconSize);
  auto* icon_label = new QLabel;
  icon_label->setPixmap(style->standardIcon(pixmap).pixmap(icon_size));
  icon_label->setAlignment(Qt::AlignTop);

  // Plain text, wrapped to at most 50 average characters. A word wider than that (e.g., a path) may break after a
  // slash.
  const QFontMetrics metrics(dialog.font());
  int max_width = 50 * metrics.averageCharWidth();
  QString text;
  for (const auto& word : a_text.split(' ')) {
    QString piece = word;
    if (metrics.horizontalAdvance(word) > max_width) piece.replace('/', QString("/") + QChar(0x200B));
    text += (text.isEmpty() ? "" : " ") + piece;
  }
  auto* text_label = new QLabel(text);
  text_label->setTextFormat(Qt::PlainText);
  text_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  text_label->setWordWrap(true);
  int width = 0;
  for (const auto& line : a_text.split('\n')) width = std::max(width, metrics.horizontalAdvance(line));
  width = std::min(width + 2, max_width);
  text_label->setFixedWidth(width);
  text_label->setMinimumHeight(text_label->heightForWidth(width));

  auto* message_row = new QHBoxLayout;
  message_row->addWidget(icon_label);
  message_row->addSpacing(12);
  message_row->addWidget(text_label, 1);

  // The affirmative button on the left, the negative one on the right.
  QPushButton* yes_button = nullptr;
  QPushButton* no_button = nullptr;
  QMessageBox::StandardButton yes = QMessageBox::Ok;
  QMessageBox::StandardButton no = QMessageBox::NoButton;
  if (a_buttons.testFlag(QMessageBox::Yes)) {
    yes_button = new QPushButton("Yes");
    yes = QMessageBox::Yes;
  }
  else yes_button = new QPushButton("OK");
  if (a_buttons.testFlag(QMessageBox::No)) {
    no_button = new QPushButton("No");
    no = QMessageBox::No;
  }
  else if (a_buttons.testFlag(QMessageBox::Cancel)) {
    no_button = new QPushButton("Cancel");
    no = QMessageBox::Cancel;
  }
  auto* button_row = new QHBoxLayout;
  if (no_button) {
    button_row->addWidget(yes_button);
    button_row->addStretch();
    button_row->addWidget(no_button);
  }
  else {
    button_row->addStretch();
    button_row->addWidget(yes_button);
  }
  for (auto* button : {yes_button, no_button}) {
    if (! button) continue;
    button->setAutoDefault(false);
    button->setDefault(false);
  }
  if (! no_button) yes_button->setDefault(true);
  QObject::connect(yes_button, &QPushButton::clicked, &dialog, &QDialog::accept);
  if (no_button) QObject::connect(no_button, &QPushButton::clicked, &dialog, &QDialog::reject);

  auto* layout = new QVBoxLayout(&dialog);
  layout->addLayout(message_row);
  QCheckBox* check_box = nullptr;
  if (a_toggle) {
    check_box = new QCheckBox("Don't show this message again");
    // Make the check box visible on dark themes, like those of the Operations panel.
    check_box->setStyle(&indicator_style);
    layout->addSpacing(6);
    layout->addWidget(check_box);
  }
  layout->addSpacing(6);
  layout->addLayout(button_row);
  // Fit the dialog to its content (no clipped text, not resizable).
  layout->setSizeConstraint(QLayout::SetFixedSize);

  // Center the dialog over the view of the window, if it has one. (Under native Wayland the compositor places
  // dialogs and ignores the requested position.)
  dialog.adjustSize();
  if (parent) {
    QWidget* over = parent;
    if (auto* view = parent->findChild<QGraphicsView*>()) over = view->viewport();
    dialog.move(over->mapToGlobal(over->rect().center()) - dialog.rect().center());
  }

  bool accepted = (dialog.exec() == QDialog::Accepted);
  if (check_box && check_box->isChecked()) a_toggle->setChecked(false);
  return (no_button && ! accepted) ? no : yes;
}

void show_warning(const QString& a_title, const QString& a_text)
{ show_message(QMessageBox::Warning, a_title, a_text); }

void show_error(const QString& a_title, const QString& a_text)
{ show_message(QMessageBox::Critical, a_title, a_text); }

void error_handler(const char* a_what, const char* a_expr, const char* a_file, int a_line, const char* a_msg) {
  QString text = QString("%1 violation: %2\n%3:%4").arg(a_what, a_expr, a_file).arg(a_line);
  if (a_msg && *a_msg) text += QString("\n") + a_msg;
  show_error("CGAL Error", text);
  throw std::runtime_error(text.toStdString());
}
