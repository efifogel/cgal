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
//             Apurva Bhatt <response2apurva@gmail.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>
//

#ifndef CGAL_QT6_GRAPHICS_VIEW_CIRCULAR_POLYGON_INPUT_H
#define CGAL_QT6_GRAPHICS_VIEW_CIRCULAR_POLYGON_INPUT_H

#include <CGAL/auto_link/Qt.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Arr_circle_segment_traits_2.h>

#include <algorithm>

#include <QPolygonF>
#include <QPointF>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "Qt6/Circular_polygons.h"

namespace CGAL {
namespace Qt {

/*! Constructs a polygon bounded by line segments and circular arcs, or a full circle, from mouse clicks and drags.
 */
template <typename Kernel_>
class Graphics_view_circular_polygon_input : public GraphicsViewInput {
public:
  using Kernel = Kernel_;

  using Gps_traits = CGAL::Gps_circle_segment_traits_2<Kernel>;
  using Circular_curve = typename Gps_traits::Curve_2;
  using Circular_X_monotone_curve = typename Gps_traits::X_monotone_curve_2;
  using Circular_polygon = typename Gps_traits::Polygon_2;
  using Arc_point = typename Circular_polygon::Point_2;
  using FT = typename Kernel::FT;
  using Vector = typename Kernel::Vector_2;
  using Point = typename Kernel::Point_2;
  using Circle = typename Kernel::Circle_2;

  using Circular_curve_vector = std::vector<Circular_curve>;

  using GI = Circular_boundary_pieces_graphics_item<Circular_curve_vector>;

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_circular_polygon_input(QObject* a_parent, QGraphicsScene* a_scene) :
    GraphicsViewInput(a_parent),
    m_scene(a_scene),
    m_ongoing_piece_GI(new GI(&m_ongoing_piece_ctr)),
    m_handle_GI(new QGraphicsLineItem()),
    m_circle_GI(new QGraphicsEllipseItem()),
    m_circular_polygon_pen(QColor(0, 255, 0)),
    m_ongoing_curve_pen(QColor(255, 215, 0)),
    m_handle_pen(QColor(255, 165, 0)),
    m_bound_rect(true),
    m_last_circular(false),
    m_last(false),
    m_state(State::ST_START) {
    m_ongoing_piece_GI->set_pen(m_ongoing_curve_pen);
    m_handle_GI->setPen(m_handle_pen);

    m_handle_GI->setLine(0, 0, 1, 1);
    m_handle_GI->hide();
    m_circle_GI->hide();

    m_circular_GI = new GI(&m_circular_polygon_pieces);
    m_circular_GI->set_pen(m_circular_polygon_pen);

    m_scene->addItem(m_ongoing_piece_GI);
    m_scene->addItem(m_handle_GI);
    m_scene->addItem(m_circle_GI);
    m_scene->addItem(m_circular_GI);
  }

  /*! destructs.
   */
  ~Graphics_view_circular_polygon_input() {}

  /*! handles the mouse and key events of the scene.
   * \return true if the event is handled.
   */
  bool eventFilter(QObject* obj, QEvent* a_event) {
    bool r_handled = false;
    if (a_event->type() == QEvent::GraphicsSceneMousePress)
      r_handled = mouse_press(static_cast<QGraphicsSceneMouseEvent*>(a_event));
    else if (a_event->type() == QEvent::GraphicsSceneMouseRelease)
      r_handled = mouse_release(static_cast<QGraphicsSceneMouseEvent*>(a_event));
    else if (a_event->type() == QEvent::GraphicsSceneMouseMove)
      r_handled = mouse_move(static_cast<QGraphicsSceneMouseEvent*>(a_event));
    else if (a_event->type() == QEvent::KeyPress)
      r_handled = key_press(static_cast<QKeyEvent*>(a_event));
    if (! r_handled) r_handled = QObject::eventFilter(obj, a_event);
    return r_handled;
  }

public:
  // The states of the construction.
  enum class State { ST_START, PIECE_STARTED, PIECE_ONGOING, HANDLE_ONGOING, PIECE_ENDED, CURVE_ENDED, CIRCLE_ONGOING };

  // What the user constructs: a boundary of line segments and circular arcs, or a full circle.
  enum class Mode { MO_ARCS, CIRCLE_MODE };

  /*! sets what the user constructs, and discards the polygon under construction.
   */
  void set_mode(Mode a_mode) {
    reset();
    m_mode = a_mode;
  }

  /*! converts a scene point to a point.
   */
  Point cvt(const QPointF& a_p) const { return Point(a_p.x(), a_p.y()); }

  /*! handles a mouse press. In the circle mode, a press sets the center. Otherwise, a left press ends the segment
   * under construction (a drag then bends it into an arc), and a right press starts closing the polygon.
   * \return true if the event is handled.
   */
  bool mouse_press(QGraphicsSceneMouseEvent* a_event) {
    bool r_handled = false;
    m_bound_rect = false;
    Point l_p = cvt(a_event->scenePos());
    if (m_mode == Mode::CIRCLE_MODE) {
      // Press at the center, drag to the radius, and release.
      if ((a_event->button() == ::Qt::LeftButton) && (m_state == State::ST_START)) {
        m_center = l_p;
        m_state = State::CIRCLE_ONGOING;
        update_circle(l_p);
        r_handled = true;
      }
      return r_handled;
    }

    if (a_event->button() == ::Qt::LeftButton) {
      switch (m_state) {
       case State::ST_START:
        m_p0 = l_p;
        m_state = State::PIECE_STARTED;
        r_handled = true;
        break;

       case State::PIECE_ONGOING:
        m_p1 = l_p;
        m_press = l_p;
        m_state = State::HANDLE_ONGOING;
        r_handled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    else if (a_event->button() == ::Qt::RightButton) {
      switch (m_state) {
       case State::PIECE_ONGOING:
        // Closing the polygon needs at least one piece.
        if (m_circular_polygon_pieces.empty()) break;
        // Allow the user to bend the closing piece as well.
        m_last = true;
        m_press = l_p;
        m_state = State::HANDLE_ONGOING;
        r_handled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return r_handled;
  }

  /*! handles a mouse move.
   * \return true if the event is handled.
   */
  bool mouse_move(QGraphicsSceneMouseEvent* a_event) {
    bool r_handled = false;
    Point l_p = cvt(a_event->scenePos());
    switch (m_state) {
     case State::CIRCLE_ONGOING:
      update_circle(l_p);
      r_handled = true;
      break;

     case State::PIECE_ONGOING:
      m_p1 = l_p;
      update_ongoing_piece();
      r_handled = true;
      break;

     case State::HANDLE_ONGOING:
      if (m_last) {
        m_p1 = cvt(m_circular_polygon_pieces.front().source());
        m_last_circular = true;
      }
      update_handle(l_p);
      update_ongoing_piece();
      r_handled = true;
      break;

     case State::PIECE_ENDED:
      m_state = State::PIECE_ONGOING;
      m_p1 = l_p;
      update_ongoing_piece();
      r_handled = true;
      break;

     default: break; //! \todo handle default case
    }
    return r_handled;
  }

  /*! handles a mouse release. In the circle mode, a release sets the radius. Otherwise, a left release completes the
   * piece under construction, and a right release closes the polygon.
   * \return true if the event is handled.
   */
  bool mouse_release(QGraphicsSceneMouseEvent* a_event) {
    bool r_handled = false;
    Point l_p = cvt(a_event->scenePos());
    if (a_event->button() == ::Qt::LeftButton) {
      switch (m_state) {
       case State::CIRCLE_ONGOING:
        m_circle_GI->hide();
        if (squared_distance(m_center, l_p) >= 4) generate_circle(Circle(m_center, squared_distance(m_center, l_p)));
        m_state = State::ST_START;
        r_handled = true;
        break;

       case State::PIECE_STARTED:
        m_state = State::PIECE_ONGOING;
        r_handled = true;
        break;

       case State::HANDLE_ONGOING:
        hide_handle();
        commit_ongoing_piece(l_p);
        m_state = State::PIECE_ENDED;
        r_handled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    else if (a_event->button() == ::Qt::RightButton) {
      switch (m_state) {
       case State::HANDLE_ONGOING:
        if (m_last_circular) {
          hide_handle();
          commit_ongoing_piece(l_p);
        }
        m_bound_rect = false;
        commit_curr_circular_polygon();
        restart();
        r_handled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return r_handled;
  }

  /*! handles a key press. Delete and Backspace remove the last piece; Escape discards the polygon.
   * \return true if the event is handled.
   */
  bool key_press(QKeyEvent* a_event) {
    bool r_handled = false;
    if ((a_event->key() == ::Qt::Key_Delete) || (a_event->key() == ::Qt::Key_Backspace)) {
      remove_last_piece();
      m_state = (m_circular_polygon_pieces.size() > 0) ? State::PIECE_ENDED : State::ST_START;
      r_handled = true;
    }
    else if (a_event->key() == ::Qt::Key_Escape) {
      reset();
      m_state = State::ST_START;
      r_handled = true;
    }
    return r_handled;
  }

public:
  /*! obtains the piece under construction, or null if there is none.
   */
  const Circular_curve* ongoing_piece() const
  { return (m_ongoing_piece_ctr.size() == 1) ? &m_ongoing_piece_ctr[0] : NULL; }

  /*! starts a new polygon.
   */
  void restart() {
    m_h = std::optional<Point>();
    m_state = State::ST_START;
  }

  /*! discards the polygon under construction.
   */
  void reset() {
    m_circular_polygon_pieces.clear();
    m_ongoing_piece_ctr.clear();
    m_circular_GI->modelChanged();
    m_ongoing_piece_GI->modelChanged();
    m_circle_GI->hide();
    hide_handle();
    restart();
  }

  /*! hides the handle.
   */
  void hide_handle() { m_handle_GI->hide(); }

  /*! creates the piece from the last vertex to the current point: an arc through the cursor if it is set, and a
   * segment otherwise.
   */
  Circular_curve create_piece() {
    if (m_h && ! collinear(m_p0, *m_h, m_p1)) return Circular_curve(m_p0, *m_h, m_p1);
    return Circular_curve(m_p0, m_p1);
  }

  /*! removes the last piece.
   */
  void remove_last_piece() {
    m_circular_polygon_pieces.pop_back();
    m_ongoing_piece_ctr.clear();
    m_circular_GI->modelChanged();
    m_ongoing_piece_GI->modelChanged();
    if (m_circular_polygon_pieces.size() > 0) {
      m_p0 = cvt(m_circular_polygon_pieces.back().target());
      update_ongoing_piece();
    }
    m_h = std::optional<Point>();
  }

  /*! updates the piece under construction.
   */
  void update_ongoing_piece() {
    if (m_ongoing_piece_ctr.size() > 0) m_ongoing_piece_ctr.clear();
    m_ongoing_piece_ctr.push_back(create_piece());
    m_ongoing_piece_GI->modelChanged();
  }

  /*! adds the piece under construction to the polygon (unless its endpoints coincide), and starts the next piece.
   */
  void commit_ongoing_piece(const Point& a_p) {
    if (! ongoing_piece()) return;
    if (! ongoing_piece()->source().equals(ongoing_piece()->target())) {
      m_circular_polygon_pieces.push_back(*ongoing_piece());
      m_circular_GI->modelChanged();
    }
    m_ongoing_piece_ctr.clear();
    m_ongoing_piece_GI->modelChanged();
    m_p0 = m_p1;
    m_p1 = a_p;
    m_h = std::optional<Point>();
  }

  /*! updates the point the arc passes through. Once the cursor is dragged away from where the button was pressed, the
   * piece becomes an arc through the cursor; otherwise it stays a segment.
   */
  void update_handle(const Point& a_p) {
    if (squared_distance(m_press, a_p) >= 4) m_h = a_p;
    else m_h = std::optional<Point>();
  }

  /*! updates the circle under construction, whose boundary passes through a given point.
   */
  void update_circle(const Point& a_p) {
    double cx = to_double(m_center.x());
    double cy = to_double(m_center.y());
    double r = std::sqrt(to_double(squared_distance(m_center, a_p)));
    // Drawn like the ongoing piece (the pen of the Draw/Insert bucket).
    m_circle_GI->setPen(m_ongoing_piece_GI->pen());
    m_circle_GI->setRect(cx - r, cy - r, 2 * r, 2 * r);
    m_circle_GI->show();
  }

  /*! emits the polygon bounded by a given circle (counterclockwise).
   */
  void generate_circle(const Circle& a_circle) {
    Gps_traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    std::vector<std::variant<Circular_X_monotone_curve, Arc_point>> x_objs;
    make_x_monotone(Circular_curve(a_circle), std::back_inserter(x_objs));
    std::vector<Circular_X_monotone_curve> xcvs;
    for (const auto& obj : x_objs) {
      if (auto* xcv = std::get_if<Circular_X_monotone_curve>(&obj)) xcvs.push_back(*xcv);
    }
    Circular_polygon cp(xcvs.begin(), xcvs.end());
    emit(generate(std::variant<Circular_polygon>(cp)));
  }

  /*! rounds a point of a curve to double precision.
   */
  Point cvt(const typename Circular_curve::Point_2& a_p) { return Point(to_double(a_p.x()), to_double(a_p.y())); }

  /*! completes the polygon under construction.
   */
  void commit_curr_circular_polygon() {
    generate_circular_polygon();
    m_ongoing_piece_ctr.clear();
    m_ongoing_piece_GI->modelChanged();
    m_circular_polygon_pieces.clear();
    m_circular_GI->modelChanged();
    m_h = std::optional<Point>();
    hide_handle();
  }

  /*! closes the polygon with a segment from its last vertex to its first (unless the closing piece is an arc or the
   * vertices coincide), and emits it. A polygon bounded by fewer than three segments, or by a single curve, is dropped.
   */
  void generate_circular_polygon() {
    bool closed = m_last_circular;
    m_last = false;
    m_last_circular = false;
    if (m_circular_polygon_pieces.size() == 0) return;
    Gps_traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    using Make_x_monotone_result = std::variant<Circular_X_monotone_curve, Arc_point>;

    std::vector<Circular_X_monotone_curve> xcvs;
    for (auto it = m_circular_polygon_pieces.begin(); it != m_circular_polygon_pieces.end(); ++it) {
      std::vector<Make_x_monotone_result> x_objs;
      make_x_monotone(*it, std::back_inserter(x_objs));
      for (std::size_t i = 0; i < x_objs.size(); ++i) {
        auto* xcv = std::get_if<Circular_X_monotone_curve>(&x_objs[i]);
        CGAL_assertion(xcv != nullptr);
        xcvs.push_back(*xcv);
      }
    }
    if (xcvs.size() == 0) return;

    const Arc_point first_point = xcvs.front().source();
    const Arc_point last_point = xcvs.back().target();
    if (! closed && ! last_point.equals(first_point)) {
      CGAL_assertion(! first_point.x().is_extended() && ! first_point.y().is_extended());
      CGAL_assertion(! last_point.x().is_extended() && ! last_point.y().is_extended());
      FT fxs = first_point.x().alpha();
      FT fys = first_point.y().alpha();
      FT lxs = last_point.x().alpha();
      FT lys = last_point.y().alpha();
      xcvs.push_back(Circular_X_monotone_curve(Point(lxs, lys), Point(fxs, fys)));
    }

    bool linear = std::all_of(xcvs.begin(), xcvs.end(), [](const auto& xcv) { return xcv.is_linear(); });
    if ((xcvs.size() < 2) || (linear && (xcvs.size() < 3))) return;
    Circular_polygon cp(xcvs.begin(), xcvs.end());
    emit(generate(std::variant<Circular_polygon>(cp)));
  }

  /*! constructs a huge (bounding) polygon.
   */
  void get_bounding_rect() {
    m_bound_rect = true;

    m_p0 = Point(-15500000, -10000000);
    m_state = State::PIECE_STARTED;

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(-15500000, 10000000);
    update_ongoing_piece();

    m_p1 = Point(-15500000, 10000000);
    m_state = State::HANDLE_ONGOING;
    hide_handle();
    commit_ongoing_piece(Point(-15500000, 10000000));
    m_state = State::PIECE_ENDED;

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(15500000, 10000000);
    update_ongoing_piece();

    m_p1 = Point(15500000, 10000000);
    m_state = State::HANDLE_ONGOING;
    hide_handle();
    commit_ongoing_piece(Point(15500000, 10000000));
    m_state = State::PIECE_ENDED;

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(15500000, -10000000);
    update_ongoing_piece();

    m_p1 = Point(15500000, -10000000);
    m_state = State::HANDLE_ONGOING;
    hide_handle();
    commit_ongoing_piece(Point(15500000, -10000000));
    m_state = State::PIECE_ENDED;

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(-9000000, -9000000);
    update_ongoing_piece();

    commit_curr_circular_polygon();
    restart();
  }

  /*! determines whether the last polygon is the bounding polygon.
   */
  bool is_bounding_rect() { return m_bound_rect; }

public:
  QGraphicsScene* m_scene;
  GI* m_circular_GI;
  GI* m_ongoing_piece_GI;
  QGraphicsLineItem* m_handle_GI;
  QGraphicsEllipseItem* m_circle_GI;

  QPen m_circular_polygon_pen;
  QPen m_ongoing_curve_pen;
  QPen m_handle_pen;

  bool m_bound_rect;
  bool m_last_circular;
  bool m_last;

  Circular_curve_vector m_circular_polygon_pieces;
  Circular_curve_vector m_ongoing_piece_ctr;

  State m_state;
  Mode m_mode = Mode::MO_ARCS;

  Point m_p0;
  Point m_p1;
  Point m_press;   // where the button was pressed to bend the piece
  Point m_center;  // the center of the circle under construction

  std::optional<Point> m_h;
};

} // namespace Qt
} // namespace CGAL

#endif
