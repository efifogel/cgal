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

#ifndef CGAL_QT_HELPERS_H
#define CGAL_QT_HELPERS_H

/*! \file Qt_helpers.h
 * Qt helpers of the main window:
 * - Floating_dock_mover: moves the floating dock widgets under Wayland.
 * - Visible_indicator_style: draws the check boxes and radio buttons visibly on dark themes.
 * - Grid_scene: a scene that draws a grid in its background.
 * - Grid_snapper: snaps the clicked points to the grid, or rounds them to about half a pixel.
 */

#include <algorithm>
#include <cmath>

#include <QDockWidget>
#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QProxyStyle>
#include <QStyleOption>
#include <QWidget>
#include <QWindow>

// Lets the user move a floating dock widget by dragging its title bar under Wayland. There, QDockWidget's own drag
// fails, because it grabs the mouse, which Wayland allows only for popup windows ("This plugin supports grabbing the
// mouse only for popup windows"). Instead, ask the compositor to move the window.
class Floating_dock_mover : public QObject {
public:
  /*! constructs from the parent object.
   */
  explicit Floating_dock_mover(QObject* parent) : QObject(parent) {}

protected:
  /*! moves a floating dock widget when its title bar is pressed.
   */
  bool eventFilter(QObject* obj, QEvent* event) override {
    if (event->type() != QEvent::MouseButtonPress) return false;
    auto* dock = qobject_cast<QDockWidget*>(obj);
    if (! dock || ! dock->isFloating()) return false;
    auto* me = static_cast<QMouseEvent*>(event);
    if (me->button() != Qt::LeftButton) return false;
    // Presses on the title-bar buttons go to the buttons themselves; a press outside the content widget is on the title
    // bar.
    QWidget* content = dock->widget();
    if (content && content->geometry().contains(me->position().toPoint()))
      return false;
    QWindow* window = dock->windowHandle();
    return window && window->startSystemMove();
  }
};

// On dark desktop themes the outlines of check box squares and radio button circles are hardly visible. Draw the
// indicator as usual (keeping the check mark or dot), then draw its outline on top: a gray square with slightly rounded
// corners for a check box, and a gray ring for a radio button. The outline turns white while the cursor hovers over the
// button.
class Visible_indicator_style : public QProxyStyle {
public:
  /*! enables hovering over check boxes and radio buttons.
   */
  void polish(QWidget* widget) override {
    QProxyStyle::polish(widget);
    // Track hovering, so that State_MouseOver is set.
    if (qobject_cast<QCheckBox*>(widget) || qobject_cast<QRadioButton*>(widget))
      widget->setAttribute(Qt::WA_Hover);
  }

  /*! draws a primitive element; the indicators of check boxes and radio buttons get a visible outline.
   */
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                     const QWidget* widget = nullptr) const override {
    QProxyStyle::drawPrimitive(element, option, painter, widget);
    if ((element != PE_IndicatorCheckBox) && (element != PE_IndicatorRadioButton))
      return;
    bool hover = option->state.testFlag(State_MouseOver) && option->state.testFlag(State_Enabled);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(hover ? QColor(Qt::white) : QColor(0x80, 0x80, 0x80), 1));
    QRectF rect = QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5);
    if (element == PE_IndicatorCheckBox)
      painter->drawRoundedRect(rect, 1.5, 1.5);
    else painter->drawEllipse(rect);
    painter->restore();
  }
};

// A scene that can draw a grid in its background, behind all items. Grid lines that would be denser than a few pixels
// on screen are thinned out (by factors of 10), so that zooming out does not fill the view with lines.
class Grid_scene : public QGraphicsScene {
public:
  /*! determines whether the grid is shown.
   */
  bool grid_visible() const { return m_grid_visible; }

  /*! shows or hides the grid.
   */
  void set_grid_visible(bool aVisible) { m_grid_visible = aVisible; update(); }

  /*! obtains the grid spacing.
   */
  double grid_spacing() const { return m_grid_spacing; }

  /*! sets the grid spacing.
   */
  void set_grid_spacing(double aSpacing) {
    m_grid_spacing = aSpacing;
    update();
  }

  /*! obtains the grid point nearest to a point.
   */
  QPointF snap(const QPointF& aP) const {
    return QPointF(std::round(aP.x() / m_grid_spacing) * m_grid_spacing,
                   std::round(aP.y() / m_grid_spacing) * m_grid_spacing);
  }

protected:
  /*! draws the background and, if it is shown, the grid.
   */
  void drawBackground(QPainter* painter, const QRectF& rect) override {
    QGraphicsScene::drawBackground(painter, rect);
    if (! m_grid_visible || (m_grid_spacing <= 0)) return;
    double scale = std::abs(painter->worldTransform().m11());
    if (scale <= 0) return;
    const double min_pixels = 6;
    double step = m_grid_spacing;
    while (step * scale < min_pixels) step *= 10;
    QVector<QLineF> lines;
    for (double x = std::ceil(rect.left() / step) * step; x <= rect.right(); x += step)
      lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (double y = std::ceil(rect.top() / step) * step; y <= rect.bottom(); y += step)
      lines.append(QLineF(rect.left(), y, rect.right(), y));
    QPen pen(QColor(0xdc, 0xdc, 0xdc), 1);
    pen.setCosmetic(true);
    painter->save();
    painter->setPen(pen);
    painter->drawLines(lines);
    painter->restore();
  }

private:
  bool m_grid_visible = false;
  double m_grid_spacing = 10;
};

// Adjusts mouse positions in a Grid_scene. With snapping enabled, a position
// is moved to the nearest grid point. Otherwise, it is rounded to a multiple
// of the largest power of 2 that does not exceed half a pixel in scene
// coordinates. A double carries 53 bits, while such a multiple has a small
// exact (dyadic) value, so the exact curves constructed from clicked points
// have much smaller coefficients, which makes the exact computations (e.g.,
// the Boolean operations on conics) much faster. The filter is installed on
// the application, so that it sees the scene's mouse events before the
// scene's own event filters (the polygon input tools, which read scenePos()).
class Grid_snapper : public QObject {
public:
  /*! constructs from the scene and the parent object.
   */
  Grid_snapper(Grid_scene* aScene, QObject* aParent) : QObject(aParent), m_scene(aScene) {}

  /*! determines whether snapping to the grid is enabled.
   */
  bool enabled() const { return m_enabled; }

  /*! enables or disables snapping to the grid.
   */
  void set_enabled(bool aEnabled) { m_enabled = aEnabled; }

protected:
  /*! snaps or rounds the position of a mouse event of the scene.
   */
  bool eventFilter(QObject* obj, QEvent* event) override {
    if (obj != m_scene) return false;
    switch (event->type()) {
     case QEvent::GraphicsSceneMousePress:
     case QEvent::GraphicsSceneMouseMove:
     case QEvent::GraphicsSceneMouseRelease:
     case QEvent::GraphicsSceneMouseDoubleClick: {
       auto* me = static_cast<QGraphicsSceneMouseEvent*>(event);
       me->setScenePos(m_enabled ? m_scene->snap(me->scenePos()) : round_to_pixel(me->scenePos(), me->widget()));
       break;
     }
     default: break;
    }
    return false;
  }

private:
  /*! rounds a point to a multiple of the largest power of 2 that does not exceed half a pixel of the view whose
   * viewport is given. Dividing and multiplying by a power of 2 is exact.
   */
  static QPointF round_to_pixel(const QPointF& aP, QWidget* aViewport) {
    auto* view = aViewport ? qobject_cast<QGraphicsView*>(aViewport->parentWidget()) : nullptr;
    if (! view) return aP;
    double scale = std::abs(view->transform().m11());
    if (! (scale > 0) || ! std::isfinite(scale)) return aP;
    double step = std::ldexp(1.0, int(std::floor(std::log2(0.5 / scale))));
    return QPointF(std::round(aP.x() / step) * step, std::round(aP.y() / step) * step);
  }

  Grid_scene* m_scene;
  bool m_enabled = false;
};

#endif
