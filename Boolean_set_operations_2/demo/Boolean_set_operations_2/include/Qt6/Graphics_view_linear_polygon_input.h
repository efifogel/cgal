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

#ifndef CGAL_QT6_GRAPHICS_VIEW_LINEAR_POLYGON_INPUT_H
#define CGAL_QT6_GRAPHICS_VIEW_LINEAR_POLYGON_INPUT_H

#include <limits>
#include <typeinfo>
#include <list>

#include <QtCore>
#include <QPolygonF>
#include <QPointF>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/auto_link/Qt.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>

#include "Typedefs.h"
#include "Qt6/Linear_polygons.h"

namespace CGAL {
namespace Qt {

/*! Constructs a linear polygon from mouse clicks.
 */
template <typename Kernel_>
class Graphics_view_linear_polygon_input : public GraphicsViewInput {
public:
  using Kernel1 = CGAL::Exact_predicates_exact_constructions_kernel;
  using Point_2 = Kernel1::Point_2;
  using Polygon_2 = CGAL::Polygon_2<Kernel1>;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel1>;
  using Pgn_with_holes_2_container = std::list<Polygon_with_holes_2>;

  using Kernel = Kernel_;

  using Gps_traits = Linear_traits;
  using Linear_curve = typename Gps_traits::Curve_2;
  using Linear_X_monotone_curve = typename Gps_traits::X_monotone_curve_2;
  using Linear_polygon = typename Gps_traits::Polygon_2;
  using Linear_point = typename Gps_traits::Point_2;
  using FT = typename Kernel::FT;
  using Vector = typename Kernel::Vector_2;
  using Point = typename Kernel::Point_2;
  using Linear_curve_vector = std::vector<Linear_curve>;

  using GI = Linear_boundary_pieces_graphics_item<Linear_curve_vector>;

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_linear_polygon_input(QObject* a_parent, QGraphicsScene* a_scene) :
    GraphicsViewInput(a_parent),
    m_scene(a_scene),
    m_linear_GI(new GI(&m_linear_polygon_pieces)),
    m_ongoing_piece_GI(new GI(&m_ongoing_piece_ctr)),
    m_handle_GI(new QGraphicsLineItem()),
    m_linear_polygon_pen(QColor(0, 255, 0)),
    m_ongoing_curve_pen(QColor(255, 215, 0)),
    m_handle_pen(QColor(255, 165, 0)),
    m_state(State::ST_START),
    m_bound_rect(true),
    m_is_mink(false),
    m_hole(false) {
    m_ongoing_piece_GI->set_pen(m_ongoing_curve_pen);
    m_handle_GI->setPen(m_handle_pen);

    m_handle_GI->setLine(0, 0, 1, 1);
    m_handle_GI->hide();

    m_linear_GI->set_pen(m_linear_polygon_pen);

    m_scene->addItem(m_ongoing_piece_GI);
    m_scene->addItem(m_handle_GI);
    m_scene->addItem(m_linear_GI);
  }

  /*! destructs.
   */
  ~Graphics_view_linear_polygon_input() {}

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
  enum class State { ST_START, PIECE_STARTED, PIECE_ONGOING, HANDLE_ONGOING, PIECE_ENDED, CURVE_ENDED };

  /*! converts a scene point to a point.
   */
  Point cvt(const QPointF& a_p) const { return Point(a_p.x(), a_p.y()); }

  /*! handles a mouse press.
   * \return true if the event is handled.
   */
  bool mouse_press(QGraphicsSceneMouseEvent* a_event) {
    m_bound_rect = false;
    bool r_handled = false;
    Point l_p = cvt(a_event->scenePos());
    if (a_event->button() == ::Qt::LeftButton) {
      switch (m_state) {
       case State::ST_START:
        m_p0 = l_p;
        m_state = State::PIECE_STARTED;
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
    return r_handled;
  }

  /*! handles a mouse move.
   * \return true if the event is handled.
   */
  bool mouse_move(QGraphicsSceneMouseEvent* a_event) {
    bool r_handled = false;
    Point l_p = cvt(a_event->scenePos());
    switch (m_state) {
     case State::PIECE_ONGOING:
      m_p1 = l_p;
      update_ongoing_piece();
      r_handled = true;
      break;

     case State::HANDLE_ONGOING:
      update_handle(l_p);
      update_ongoing_piece();
      r_handled = true;
      break;

     case State::PIECE_ENDED:
      m_state = State::PIECE_ONGOING;
      r_handled = true;
      break;

     default: break; //! \todo handle default case
    }
    return r_handled;
  }

  /*! handles a mouse release. A left release completes the edge under construction; a right release closes the
   * polygon.
   * \return true if the event is handled.
   */
  bool mouse_release(QGraphicsSceneMouseEvent* a_event) {
    bool r_handled = false;
    Point l_p = cvt(a_event->scenePos());
    if (a_event->button() == ::Qt::LeftButton) {
      switch (m_state) {
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
       case State::PIECE_ONGOING:
        m_bound_rect = false;
        commit_curr_linear_polygon();
        restart();
        r_handled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return r_handled;
  }

  /*! handles a key press. Escape, Delete, and Backspace undo one step: they discard the edge under construction (while
   * the mouse button is pressed) if there is one, and otherwise remove the last edge of the boundary, or its first
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
      hide_handle();
      m_h = std::optional<Point>();
      m_state = State::PIECE_ONGOING;
      update_ongoing_piece();
      return true;

     case State::PIECE_STARTED: reset(); return true;

     case State::PIECE_ONGOING:
     case State::PIECE_ENDED:
      if (m_linear_polygon_pieces.empty()) reset();
      else {
        remove_last_piece();
        m_state = State::PIECE_ONGOING;
      }
      return true;

     default: return false;
    }
  }

  /*! obtains the edge under construction, or null if there is none.
   */
  const Linear_curve* ongoing_piece() const
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
    m_linear_polygon_pieces.clear();
    m_ongoing_piece_ctr.clear();
    m_linear_GI->modelChanged();
    m_ongoing_piece_GI->modelChanged();
    restart();
  }

  /*! hides the handle.
   */
  void hide_handle() { m_handle_GI->hide(); }

  /*! creates the edge from the last vertex to the current point.
   */
  Linear_curve create_piece() { return Linear_curve(m_p0, m_p1); }

  /*! removes the last edge.
   */
  void remove_last_piece() {
    if (m_linear_polygon_pieces.empty()) return;
    m_p0 = cvt(m_linear_polygon_pieces.back().source());
    m_linear_polygon_pieces.pop_back();
    m_linear_GI->modelChanged();
    m_h = std::optional<Point>();
    update_ongoing_piece();
  }

  /*! updates the edge under construction.
   */
  void update_ongoing_piece() {
    if (m_ongoing_piece_ctr.size() > 0) m_ongoing_piece_ctr.clear();
    m_ongoing_piece_ctr.push_back(create_piece());
    m_ongoing_piece_GI->modelChanged();
  }

  /*! adds the edge under construction to the polygon (unless it is degenerate), and starts the next edge.
   */
  void commit_ongoing_piece(const Point& a_p) {
    if (! ongoing_piece()) return;
    if (ongoing_piece()->source() != ongoing_piece()->target()) {
      m_linear_polygon_pieces.push_back(*ongoing_piece());
      m_linear_GI->modelChanged();
    }
    m_ongoing_piece_ctr.clear();
    m_ongoing_piece_GI->modelChanged();
    m_p0 = m_p1;
    m_p1 = a_p;
    m_h = std::optional<Point>();
  }

  /*! updates the handle; it is shown only when it is at least 2 units long.
   */
  void update_handle(const Point& a_p) {
    if (squared_distance(m_p1, a_p) >= 4) {
      m_h = a_p;
      m_handle_GI->setLine(to_double(m_p1.x()), to_double(m_p1.y()), to_double(m_h->x()), to_double(m_h->y()));
      m_handle_GI->show();
    }
    else {
      hide_handle();
      m_h = std::optional<Point>();
    }
  }

  /*! rounds a point to double precision.
   */
  Point cvt(const Point& a_p) { return Point(to_double(a_p.x()), to_double(a_p.y())); }

  /*! completes the polygon under construction.
   */
  void commit_curr_linear_polygon() {
    generate_linear_polygon();
    m_ongoing_piece_ctr.clear();
    m_ongoing_piece_GI->modelChanged();
    m_linear_polygon_pieces.clear();
    m_linear_GI->modelChanged();
    m_h = std::optional<Point>();
    hide_handle();
  }

  /*! closes the polygon with an edge from its last vertex to its first (unless they coincide), and emits it.
   */
  void generate_linear_polygon() {
    if (m_linear_polygon_pieces.size() <= 1) return;
    Gps_traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    using Pnt = typename Gps_traits::Point_2;
    using Xcv = typename Gps_traits::X_monotone_curve_2;
    using Make_x_monotone_result = std::variant<Pnt, Xcv>;

    std::vector<Linear_X_monotone_curve> xcvs;
    for (auto it = m_linear_polygon_pieces.begin(); it != m_linear_polygon_pieces.end(); ++it) {
      std::vector<Make_x_monotone_result> x_objs;
      make_x_monotone(*it, std::back_inserter(x_objs));
      for (std::size_t i = 0; i < x_objs.size(); ++i) {
        auto* xcv = std::get_if<Linear_X_monotone_curve>(&x_objs[i]);
        CGAL_assertion(xcv != nullptr);
        xcvs.push_back(*xcv);
      }
    }

    if (xcvs.empty()) return;
    const Linear_point first_point = xcvs.front().source();
    const Linear_point last_point = xcvs.back().target();
    if (last_point != first_point) xcvs.push_back(Linear_X_monotone_curve(last_point, first_point));
    if (xcvs.size() < 3) return;
    Linear_polygon lp(xcvs.begin(), xcvs.end());
    emit(generate(std::variant<Linear_polygon>(lp)));
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

    commit_curr_linear_polygon();
    restart();
  }

  /*! constructs polygons from the holes of a given polygon with holes (e.g., the result of a Minkowski sum).
   */
  void get_minkowski_holes(Polygon_with_holes_2 polygon, Polygon_with_holes_2 p0) {
    m_bound_rect = false;
    Point_2 pt;
    Polygon_2 hole;

    for (auto hi = polygon.holes_begin(); hi != polygon.holes_end(); ++hi) {
      hole = *hi;
      int i = 0;
      for (auto vit = hole.begin(); vit != hole.end(); ++vit) {
        pt = *vit;
        Point p(CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
        if (i == 0) {
          m_p0 = p;
          m_state = State::PIECE_STARTED;
        }
        else {
          m_state = State::PIECE_ONGOING;
          m_p1 = p;
          update_ongoing_piece();
          m_p1 = p;
          m_state = State::HANDLE_ONGOING;
          hide_handle();
          commit_ongoing_piece(p);
          m_state = State::PIECE_ENDED;
        }
        i++;
      }

      m_state = State::PIECE_ONGOING;
      m_p1 = Point(-90, -90);
      update_ongoing_piece();

      commit_curr_linear_polygon();
      restart();
    }
  }

  /*! constructs a polygon from the outer boundary of a given polygon with holes (e.g., the result of a Minkowski
   * sum).
   */
  void get_minkowski_result(Polygon_with_holes_2 polygon, Polygon_with_holes_2 p0) {
    m_bound_rect = false;
    Point_2 pt;

    int i = 0;
    for (auto vit = polygon.outer_boundary().vertices_begin(); vit != polygon.outer_boundary().vertices_end(); ++vit) {
      pt = *vit;
      Point p(CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
      if (i == 0) {
        m_p0 = p;
        m_state = State::PIECE_STARTED;
      }
      else {
        m_state = State::PIECE_ONGOING;
        m_p1 = p;
        update_ongoing_piece();
        m_p1 = p;
        m_state = State::HANDLE_ONGOING;
        hide_handle();
        commit_ongoing_piece(p);
        m_state = State::PIECE_ENDED;
      }
      i++;
    }

    m_state = State::PIECE_ONGOING;
    m_p1 = Point(-90, -90);
    update_ongoing_piece();

    commit_curr_linear_polygon();
    restart();
  }

  /*! determines whether the last polygon is the bounding polygon.
   */
  bool is_bounding_rect() { return m_bound_rect; }

  /*! determines whether the last polygon is a hole.
   */
  bool ishole() { return m_hole; }

  /*! determines whether the last polygon is a Minkowski sum.
   */
  bool is_mink() { return m_is_mink; }

public:
  QGraphicsScene* m_scene;
  GI* m_linear_GI;
  GI* m_ongoing_piece_GI;
  QGraphicsLineItem* m_handle_GI;

  QPen m_linear_polygon_pen;
  QPen m_ongoing_curve_pen;
  QPen m_handle_pen;

  Linear_curve_vector m_linear_polygon_pieces;
  Linear_curve_vector m_linear_polygon_pieces_bound_r;
  Linear_curve_vector m_ongoing_piece_ctr;

  State m_state;

  bool m_bound_rect;
  bool m_is_mink;
  bool m_hole;

  Point m_p0;
  Point m_p1;
  std::optional<Point> m_h;
};

} // namespace Qt
} // namespace CGAL

#endif
