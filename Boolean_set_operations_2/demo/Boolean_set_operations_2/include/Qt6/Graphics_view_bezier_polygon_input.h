// Copyright (c) 2009  GeometryFactory Sarl (France).
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
//             Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>

#ifndef CGAL_QT6_GRAPHICS_VIEW_BEZIER_POLYGON_INPUT_H
#define CGAL_QT6_GRAPHICS_VIEW_BEZIER_POLYGON_INPUT_H

#include <CGAL/auto_link/Qt.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Arr_Bezier_curve_traits_2.h>

#include <algorithm>

#include <QPolygonF>
#include <QPointF>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "Qt6/Bezier_curves.h"

namespace CGAL {
namespace Qt {

/*! Constructs a Bezier polygon from mouse clicks and drags. Dragging at a vertex sets the handles (the inner control
 * points) of the curves that meet at the vertex.
 */
template <class Traits_>
class Graphics_view_bezier_polygon_input : public GraphicsViewInput {
public:
  using Traits = Traits_;
  using Bezier_gps_traits = CGAL::Gps_traits_2<Traits>;
  using Bezier_curve = typename Traits::Curve_2;
  using Bezier_X_monotone_curve = typename Traits::X_monotone_curve_2;
  using Bezier_polygon = typename Bezier_gps_traits::General_polygon_2;
  using Vector = typename Traits::Rat_kernel::Vector_2;
  using Point = typename Traits::Rat_kernel::Point_2;

  using Bezier_curve_vector = std::vector<Bezier_curve>;

  using GI = Bezier_boundary_pieces_graphics_item<Bezier_curve_vector>;

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_bezier_polygon_input(QObject* a_parent, QGraphicsScene* a_scene) :
    GraphicsViewInput(a_parent),
    m_scene(a_scene),
    m_bezier_GI(0),
    m_bezier_polygon_pen(QColor(0, 255, 0)),
    m_ongoing_curve_pen(QColor(255, 0, 0)),
    m_handle_pen(QColor(0, 0, 255)),
    m_bound_rect(true),
    m_last_bezier(false),
    m_last(false),
    m_state(State::ST_START) {
    m_ongoing_piece_GI = new GI(&m_ongoing_piece_ctr);
    m_handle0_GI = new QGraphicsLineItem();
    m_handle1_GI = new QGraphicsLineItem();

    m_ongoing_piece_GI->set_pen(m_ongoing_curve_pen);
    m_handle0_GI->setPen(m_handle_pen);
    m_handle1_GI->setPen(m_handle_pen);

    m_handle0_GI->setLine(0, 0, 1, 1);
    m_handle1_GI->setLine(0, 0, 1, 1);
    m_handle0_GI->hide();
    m_handle1_GI->hide();

    m_bezier_GI = new GI(&m_bezier_polygon_pieces);
    m_bezier_GI->set_pen(m_bezier_polygon_pen);

    m_scene->addItem(m_ongoing_piece_GI);
    m_scene->addItem(m_handle0_GI);
    m_scene->addItem(m_handle1_GI);
    m_scene->addItem(m_bezier_GI);
  }

  /*! destructs.
   */
  ~Graphics_view_bezier_polygon_input() {}

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
  enum class State {
    ST_START, PIECE_OR_FIRST_HANDLE_STARTED, PIECE_ONGOING, FIRST_HANDLE_ONGOING, HANDLE_ONGOING, PIECE_ENDED,
    CURVE_ENDED
  };

  /*! converts a scene point to a point.
   */
  Point cvt(const QPointF& a_p) const { return Point(a_p.x(), a_p.y()); }

  /*! handles a mouse press. A right press while a curve is under construction lets the user set the handles of the
   * closing curve too.
   * \return true if the event is handled.
   */
  bool mouse_press(QGraphicsSceneMouseEvent* a_event) {
    bool r_handled = false;
    m_bound_rect = false;
    Point l_p = cvt(a_event->scenePos());
    if (a_event->button() == ::Qt::LeftButton) {
      switch (m_state) {
       case State::ST_START:
        m_p0 = l_p;
        m_state = State::PIECE_OR_FIRST_HANDLE_STARTED;
        r_handled = true;
        break;

       case State::PIECE_ONGOING:
        m_p1 = l_p;
        m_state = State::HANDLE_ONGOING;
        r_handled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    else if (a_event->button() == ::Qt::RightButton) {
      switch (m_state) {
       case State::PIECE_ONGOING:
        m_last = true;
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
     case State::PIECE_OR_FIRST_HANDLE_STARTED:
      m_state = State::FIRST_HANDLE_ONGOING;
      r_handled = true;
      break;

     case State::PIECE_ONGOING:
      m_p1 = l_p;
      update_ongoing_piece();
      r_handled = true;
      break;

     case State::FIRST_HANDLE_ONGOING:
      update_very_first_handle(l_p);
      r_handled = true;
      break;

     case State::HANDLE_ONGOING:
      if (m_last) {
        m_p1 = m_bezier_polygon_pieces.front().control_point(0);
        m_last_bezier = true;
      }
      update_handles(l_p);
      update_ongoing_piece();
      r_handled = true;
      break;

     case State::PIECE_ENDED:
      m_state = State::PIECE_ONGOING;
      r_handled = true;
      break;
    }
    return r_handled;
  }

  /*! handles a mouse release. A right release closes the polygon.
   * \return true if the event is handled.
   */
  bool mouse_release(QGraphicsSceneMouseEvent* a_event) {
    bool r_handled = false;
    Point l_p = cvt(a_event->scenePos());
    if (a_event->button() == ::Qt::LeftButton) {
      switch (m_state) {
       case State::PIECE_OR_FIRST_HANDLE_STARTED:
        m_state = State::PIECE_ONGOING;
        r_handled = true;
        break;

       case State::FIRST_HANDLE_ONGOING:
        update_very_first_handle(l_p);
        m_prev_h0 = m_h1;
        m_h1 = std::optional<Point>();
        m_state = State::PIECE_ONGOING;
        r_handled = true;
        break;

       case State::HANDLE_ONGOING:
        update_handles(l_p);
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
        m_bound_rect = false;
        if (m_last_bezier) {
          hide_handles();
          commit_ongoing_piece(l_p);
        }
        close_curr_boundary();
        commit_curr_bezier_polygon();
        restart();
        r_handled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return r_handled;
  }

  /*! handles a key press. Escape, Delete, and Backspace undo one step: they discard the curve under construction (while
   * the mouse button is pressed) if there is one, and otherwise remove the last curve of the boundary, or its first
   * vertex if there is none.
   * \return true if the event is handled.
   */
  bool key_press(QKeyEvent* a_event) {
    if ((a_event->key() != ::Qt::Key_Escape) && (a_event->key() != ::Qt::Key_Delete) &&
        (a_event->key() != ::Qt::Key_Backspace))
      return false;
    return step_back();
  }

  /*! undoes one step of the construction (see key_press()).
   * \return false if there is nothing to undo.
   */
  bool step_back() {
    switch (m_state) {
     case State::HANDLE_ONGOING:
      hide_handles();
      m_h0 = m_h1 = std::optional<Point>();
      m_last = m_last_bezier = false;
      m_state = State::PIECE_ONGOING;
      update_ongoing_piece();
      return true;

     case State::PIECE_OR_FIRST_HANDLE_STARTED:
     case State::FIRST_HANDLE_ONGOING: reset(); return true;

     case State::PIECE_ONGOING:
     case State::PIECE_ENDED:
      if (m_bezier_polygon_pieces.empty()) reset();
      else {
        remove_last_piece();
        m_state = State::PIECE_ONGOING;
      }
      return true;

     default: return false;
    }
  }

public:
  /*! obtains the curve under construction, or null if there is none.
   */
  const Bezier_curve* ongoing_piece() const { return m_ongoing_piece_ctr.size() == 1 ? &m_ongoing_piece_ctr[0] : NULL; }

  /*! starts a new polygon.
   */
  void restart() {
    m_prev_h0 = m_h0 = m_h1 = std::optional<Point>();
    m_state = State::ST_START;
  }

  /*! discards the polygon under construction.
   */
  void reset() {
    m_bezier_polygon_pieces.clear();
    m_piece_prev_h0s.clear();
    m_ongoing_piece_ctr.clear();
    hide_handles();
    m_bezier_GI->modelChanged();
    m_ongoing_piece_GI->modelChanged();
    restart();
  }

  /*! removes the last curve.
   */
  void remove_last_piece() {
    if (m_bezier_polygon_pieces.empty()) return;
    m_p0 = m_bezier_polygon_pieces.back().control_point(0);
    m_bezier_polygon_pieces.pop_back();
    m_prev_h0 = m_piece_prev_h0s.back();
    m_piece_prev_h0s.pop_back();
    m_bezier_GI->modelChanged();
    m_h0 = m_h1 = std::optional<Point>();
    hide_handles();
    update_ongoing_piece();
  }

  /*! hides the handles.
   */
  void hide_handles() {
    m_handle0_GI->hide();
    m_handle1_GI->hide();
  }

  /*! creates the curve from the last vertex to the current point, using the handles that are set.
   */
  Bezier_curve create_piece() {
    if (m_prev_h0 && m_h1 && *m_prev_h0 != *m_h1 && *m_prev_h0 != m_p0 && *m_h1 != m_p1) {
      Point l_control_points[4] = {m_p0, *m_prev_h0, *m_h1, m_p1};
      return Bezier_curve(l_control_points, l_control_points + 4);
    }
    if (m_prev_h0 && ! m_h1 && *m_prev_h0 != m_p0 && *m_prev_h0 != m_p1) {
      Point l_control_points[3] = {m_p0, *m_prev_h0, m_p1};
      return Bezier_curve(l_control_points, l_control_points + 3);
    }
    if (! m_prev_h0 && m_h1 && *m_h1 != m_p0 && *m_h1 != m_p1) {
      Point l_control_points[3] = {m_p0, *m_h1, m_p1};
      return Bezier_curve(l_control_points, l_control_points + 3);
    }
    Point l_control_points[2] = {m_p0, m_p1};
    return Bezier_curve(l_control_points, l_control_points + 2);
  }

  /*! updates the curve under construction.
   */
  void update_ongoing_piece() {
    if (m_ongoing_piece_ctr.size() > 0) m_ongoing_piece_ctr.clear();
    m_ongoing_piece_ctr.push_back(create_piece());
    m_ongoing_piece_GI->modelChanged();
  }

  /*! determines whether a curve is degenerate, that is, its endpoints coincide.
   */
  static bool is_degenerate(const Bezier_curve& a_curve) {
    return a_curve.control_point(0) == a_curve.control_point(a_curve.number_of_control_points() - 1);
  }

  /*! adds the curve under construction to the polygon (unless it is degenerate), and starts the next curve.
   */
  void commit_ongoing_piece(const Point& a_p) {
    if (! ongoing_piece()) return;
    if (! is_degenerate(*ongoing_piece())) {
      m_bezier_polygon_pieces.push_back(*ongoing_piece());
      m_piece_prev_h0s.push_back(m_prev_h0);
      m_bezier_GI->modelChanged();
    }
    m_ongoing_piece_ctr.clear();
    m_ongoing_piece_GI->modelChanged();
    m_p0 = m_p1;
    m_p1 = a_p;
    m_prev_h0 = m_h0;
    m_h0 = m_h1 = std::optional<Point>();
  }

  /*! updates the handle of the first vertex; it is shown only when it is at least 3 units long.
   */
  void update_very_first_handle(const Point& a_p) {
    if (squared_distance(m_p0, a_p) >= 9) {
      m_h1 = a_p;
      m_handle1_GI->setLine(to_double(m_p0.x()), to_double(m_p0.y()), to_double(m_h1->x()), to_double(m_h1->y()));
      m_handle1_GI->show();
      m_h0 = std::optional<Point>();
      m_handle0_GI->hide();
    }
    else {
      hide_handles();
      m_h0 = m_h1 = std::optional<Point>();
    }
  }

  /*! updates the two (symmetric) handles of the current vertex; they are shown only when they are at least 3 units
   * long.
   */
  void update_handles(const Point& a_p) {
    if (squared_distance(m_p1, a_p) >= 9) {
      m_h0 = a_p;
      m_h1 = m_p1 - (a_p - m_p1);
      m_handle0_GI->setLine(to_double(m_p1.x()), to_double(m_p1.y()), to_double(m_h0->x()), to_double(m_h0->y()));
      m_handle1_GI->setLine(to_double(m_p1.x()), to_double(m_p1.y()), to_double(m_h1->x()), to_double(m_h1->y()));
      m_handle0_GI->show();
      m_handle1_GI->show();
    }
    else {
      hide_handles();
      m_h0 = m_h1 = std::optional<Point>();
    }
  }

  /*! closes the boundary with a curve that ends at the first vertex (unless the last vertex is the first).
   */
  void close_curr_boundary() {
    if ((m_bezier_polygon_pieces.size() == 0) || (ongoing_piece() == NULL) || m_last_bezier) return;
    std::vector<Point> l_control_points(ongoing_piece()->control_points_begin(), ongoing_piece()->control_points_end());
    l_control_points.back() = m_bezier_polygon_pieces.front().control_point(0);
    if (l_control_points.front() == l_control_points.back()) return;
    m_bezier_polygon_pieces.push_back(Bezier_curve(l_control_points.begin(), l_control_points.end()));
    m_bezier_GI->modelChanged();
  }

  /*! completes the polygon under construction.
   */
  void commit_curr_bezier_polygon() {
    generate_bezier_polygon();

    m_ongoing_piece_ctr.clear();
    m_ongoing_piece_GI->modelChanged();

    m_bezier_polygon_pieces.clear();
    m_piece_prev_h0s.clear();
    m_bezier_GI->modelChanged();

    m_prev_h0 = m_h0 = m_h1 = std::optional<Point>();

    hide_handles();
  }

  /*! emits the polygon, together with its curves. A polygon bounded by fewer than three segments, or by a single curve,
   * is dropped.
   */
  void generate_bezier_polygon() {
    m_last = false;
    m_last_bezier = false;
    auto n = m_bezier_polygon_pieces.size();
    bool linear = std::all_of(m_bezier_polygon_pieces.begin(), m_bezier_polygon_pieces.end(),
                              [](const Bezier_curve& cv) { return cv.number_of_control_points() == 2; });
    if ((n < 2) || (linear && (n < 3))) return;
    Traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    using Make_x_monotone_result = std::variant<typename Traits::Point_2, Bezier_X_monotone_curve>;

    std::vector<Bezier_X_monotone_curve> xcvs;
    for (auto it = m_bezier_polygon_pieces.begin(); it != m_bezier_polygon_pieces.end(); ++it) {
      std::vector<Make_x_monotone_result> x_objs;
      make_x_monotone(*it, std::back_inserter(x_objs));
      for (std::size_t i = 0; i < x_objs.size(); ++i) {
        auto* xcv = std::get_if<Bezier_X_monotone_curve>(&x_objs[i]);
        CGAL_assertion(xcv != nullptr);
        xcvs.push_back(*xcv);
      }
    }

    if (xcvs.size() == 0) return;
    Bezier_polygon bp(xcvs.begin(), xcvs.end());
    using Generated = std::variant<std::pair<Bezier_polygon, Bezier_curve_vector>>;
    emit(generate(Generated(std::make_pair(bp, m_bezier_polygon_pieces))));
  }

  /*! constructs a huge (bounding) polygon.
   */
  void get_bounding_rect() {
    m_bound_rect = true;

    m_p0 = Point(-15500000, -10000000);
    m_state = State::PIECE_OR_FIRST_HANDLE_STARTED;

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(-15500000, 10000000);
    update_ongoing_piece();

    m_p1 = Point(-15500000, 10000000);
    m_state = State::HANDLE_ONGOING;
    update_handles(Point(-15500000, 10000000));
    commit_ongoing_piece(Point(-15500000, 10000000));
    m_state = State::PIECE_ENDED;

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(15500000, 10000000);
    update_ongoing_piece();

    m_p1 = Point(15500000, 10000000);
    m_state = State::HANDLE_ONGOING;
    update_handles(Point(15500000, 10000000));
    commit_ongoing_piece(Point(15500000, 10000000));
    m_state = State::PIECE_ENDED;

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(15500000, -10000000);
    update_ongoing_piece();

    m_p1 = Point(15500000, -10000000);
    m_state = State::HANDLE_ONGOING;
    update_handles(Point(15500000, -10000000));
    commit_ongoing_piece(Point(15500000, -10000000));
    m_state = State::PIECE_ENDED;

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(-9000000, -9000000);
    update_ongoing_piece();

    close_curr_boundary();
    commit_curr_bezier_polygon();
    restart();
  }

  /*! determines whether the last polygon is the bounding polygon.
   */
  bool is_bounding_rect() { return m_bound_rect; }

public:
  QGraphicsScene* m_scene;
  GI* m_bezier_GI;
  GI* m_ongoing_piece_GI;
  QGraphicsLineItem* m_handle0_GI;
  QGraphicsLineItem* m_handle1_GI;

  QPen m_bezier_polygon_pen;
  QPen m_ongoing_curve_pen;
  QPen m_handle_pen;

  bool m_bound_rect;
  bool m_last_bezier;
  bool m_last;

  Bezier_curve_vector m_bezier_polygon_pieces;
  std::vector<std::optional<Point>> m_piece_prev_h0s;  // the handle that leaves the first vertex of each curve
  Bezier_curve_vector m_ongoing_piece_ctr;

  State m_state;

  Point m_p0;
  Point m_p1;

  std::optional<Point> m_prev_h0;
  std::optional<Point> m_h0;
  std::optional<Point> m_h1;
};

} // namespace Qt
} // namespace CGAL

#endif
