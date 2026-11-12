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

#ifndef CGAL_QT6_GRAPHICS_VIEW_POLYLINE_INPUT_H
#define CGAL_QT6_GRAPHICS_VIEW_POLYLINE_INPUT_H

#include <iostream>

#include <QPolygonF>
#include <QPointF>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <CGAL/auto_link/Qt.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Arr_polyline_traits_2.h>

#include "Qt6/Polyline_curves.h"
#include "Polyline_boundary.h"

namespace CGAL {
namespace Qt {

/*! Constructs a polygon bounded by polylines from mouse clicks.
 */
template <typename Kernel_>
class Graphics_view_polyline_input : public GraphicsViewInput {
public:
  using Kernel1 = CGAL::Exact_predicates_exact_constructions_kernel;
  using Point_2 = Kernel1::Point_2;
  using Polygon_2 = CGAL::Polygon_2<Kernel1>;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel1>;
  using Pgn_with_holes_2_container = std::list<Polygon_with_holes_2>;

  using Kernel = Kernel_;
  using Gps_traits = Polyline_traits;
  using Polyline_curve = typename Gps_traits::Curve_2;
  using Polyline_X_monotone_curve = typename Gps_traits::X_monotone_curve_2;
  using Polyline_polygon = typename Gps_traits::Polygon_2;
  using Polyline_point = typename Gps_traits::Point_2;
  using Polyline_polygon_with_holes = typename Gps_traits::General_polygon_with_holes_2;
  using FT = typename Kernel::FT;
  using Vector = typename Kernel::Vector_2;
  using Point = typename Kernel::Point_2;
  using Polyline_curve_vector = std::vector<Polyline_curve>;

  using GI = Polyline_boundary_pieces_graphics_item<Polyline_curve_vector>;

  using Segment_traits_2 = CGAL::Arr_segment_traits_2<Kernel>;
  using Base = CGAL::Gps_traits_2<CGAL::Arr_polycurve_traits_2<Segment_traits_2>>;
  using X_monotone_subcurve_2 = typename Base::X_monotone_subcurve_2;

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_polyline_input(QObject* a_parent, QGraphicsScene* a_scene) :
    GraphicsViewInput(a_parent),
    m_scene(a_scene),
    m_ongoing_piece_GI(new GI(&m_ongoing_piece_ctr)),
    m_handle_GI(new QGraphicsLineItem()),
    m_polyline_polygon_pen(QColor(0, 255, 0)),
    m_ongoing_curve_pen(QColor(255, 0, 0)),
    m_handle_pen(QColor(0, 0, 255)),
    m_bound_rect(true),
    m_last_polyline(false),
    m_last(false),
    m_state(State::ST_START) {
    m_ongoing_piece_GI->set_pen(m_ongoing_curve_pen);
    m_handle_GI->setPen(m_handle_pen);

    m_handle_GI->setLine(0, 0, 1, 1);
    m_handle_GI->hide();

    m_polyline_GI = new GI(&m_polyline_polygon_pieces);
    m_polyline_GI->set_pen(m_polyline_polygon_pen);

    m_scene->addItem(m_ongoing_piece_GI);
    m_scene->addItem(m_handle_GI);
    m_scene->addItem(m_polyline_GI);
  }

  /*! destructs.
   */
  ~Graphics_view_polyline_input() {}

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

  /*! handles a mouse press. A left press ends the edge under construction; a right press starts closing the polygon.
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
     case State::PIECE_ONGOING:
      m_p1 = l_p;
      update_ongoing_piece();
      r_handled = true;
      break;

     case State::HANDLE_ONGOING:
      if (m_last) {
        Point ps(m_polyline_polygon_pieces.front()[0].source().x(), m_polyline_polygon_pieces.front()[0].source().y());
        m_p1 = ps;
        m_last_polyline = true;
      }
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
       case State::HANDLE_ONGOING:
        if (m_last_polyline) {
          hide_handle();
          commit_ongoing_piece(l_p);
        }
        m_bound_rect = false;
        commit_curr_polyline_polygon();
        restart();
        r_handled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return r_handled;
  }

  /*! handles a key press. Delete and Backspace remove the last edge; Escape discards the polygon.
   * \return true if the event is handled.
   */
  bool key_press(QKeyEvent* a_event) {
    bool r_handled = false;
    if ((a_event->key() == ::Qt::Key_Delete) || (a_event->key() == ::Qt::Key_Backspace)) {
      remove_last_piece();
      m_state = (m_polyline_polygon_pieces.size() > 0) ? State::PIECE_ENDED : State::ST_START;
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
  /*! obtains the edge under construction, or null if there is none.
   */
  const Polyline_curve* ongoing_piece() const
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
    m_polyline_polygon_pieces.clear();
    m_ongoing_piece_ctr.clear();
    m_polyline_GI->modelChanged();
    m_ongoing_piece_GI->modelChanged();
    restart();
  }

  /*! hides the handle.
   */
  void hide_handle() { m_handle_GI->hide(); }

  /*! creates the edge (a one-segment polyline) from the last vertex to the current point.
   */
  Polyline_curve create_piece() {
    Gps_traits poly_tr;
    auto construct_poly = poly_tr.construct_curve_2_object();
    return Polyline_curve(construct_poly(m_p0, m_p1));
  }

  /*! removes the last edge.
   */
  void remove_last_piece() {
    m_polyline_polygon_pieces.pop_back();
    m_ongoing_piece_ctr.clear();
    m_polyline_GI->modelChanged();
    m_ongoing_piece_GI->modelChanged();
    if (m_polyline_polygon_pieces.size() > 0) {
      auto xx = m_polyline_polygon_pieces.back().number_of_subcurves();
      const auto& last = m_polyline_polygon_pieces.back()[xx - 1];
      Point pt(last.target().x(), last.target().y());
      m_p0 = pt;
      update_ongoing_piece();
    }
    m_h = std::optional<Point>();
  }

  /*! updates the edge under construction.
   */
  void update_ongoing_piece() {
    if (m_ongoing_piece_ctr.size() > 0) m_ongoing_piece_ctr.clear();
    m_ongoing_piece_ctr.push_back(create_piece());
    m_ongoing_piece_GI->modelChanged();
  }

  /*! determines whether a piece is degenerate, that is, its endpoints coincide.
   */
  static bool is_degenerate(const Polyline_curve& a_piece) {
    auto n = a_piece.number_of_subcurves();
    return (n == 0) || (a_piece[0].source() == a_piece[n - 1].target());
  }

  /*! adds the edge under construction to the polygon (unless it is degenerate), and starts the next edge.
   */
  void commit_ongoing_piece(const Point& a_p) {
    if (! ongoing_piece()) return;
    if (! is_degenerate(*ongoing_piece())) {
      m_polyline_polygon_pieces.push_back(*ongoing_piece());
      m_polyline_GI->modelChanged();
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

  /*! obtains the number of segments of given x-monotone polylines.
   */
  static std::size_t number_of_edges(const std::vector<Polyline_X_monotone_curve>& a_xcvs) {
    std::size_t n = 0;
    for (const auto& xcv : a_xcvs) n += xcv.number_of_subcurves();
    return n;
  }

  /*! completes the polygon under construction.
   */
  void commit_curr_polyline_polygon() {
    generate_polyline_polygon();
    m_ongoing_piece_ctr.clear();
    m_ongoing_piece_GI->modelChanged();
    m_polyline_polygon_pieces.clear();
    m_polyline_GI->modelChanged();
    m_h = std::optional<Point>();
    hide_handle();
  }

  /*! obtains the vertices of the edges constructed so far, in order.
   */
  std::vector<Point> vertices() const { return chain_vertices(m_polyline_polygon_pieces); }

  /*! obtains the vertices of a chain of polylines, in order.
   */
  static std::vector<Point> chain_vertices(const Polyline_curve_vector& a_pieces) {
    std::vector<Point> r_points;
    for (const auto& piece : a_pieces) {
      for (auto it = piece.points_begin(); it != piece.points_end(); ++it) {
        if (r_points.empty() || (*it != r_points.back())) r_points.push_back(*it);
      }
    }
    return r_points;
  }

  /*! closes the polygon (with an edge from its last vertex to its first, unless they coincide), subdivides its
   * boundary into maximal x-monotone polylines (see polyline_boundary()), and emits it.
   */
  void generate_polyline_polygon() {
    m_last = false;
    m_last_polyline = false;
    auto xcvs = polyline_boundary(vertices());
    if (xcvs.empty()) return;
    Polyline_polygon pp(xcvs.begin(), xcvs.end());
    emit(generate(std::variant<Polyline_polygon>(pp)));
  }

  /*! completes the polygon under construction, used for the complement.
   */
  void commit_complement_curr_polyline_polygon() {
    generate_complement_polyline_polygon();
    m_ongoing_piece_ctr.clear();
    m_ongoing_piece_GI->modelChanged();
    m_polyline_polygon_pieces.clear();
    m_polyline_GI->modelChanged();
    m_h = std::optional<Point>();
    hide_handle();
  }

  /*! closes the polygon, used for the complement, and emits it. The x-coordinates of the closing edge are negated.
   */
  void generate_complement_polyline_polygon() {
    bool closed = m_last_polyline;
    m_last = false;
    m_last_polyline = false;
    if (m_polyline_polygon_pieces.size() == 0) return;
    Gps_traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    using Pnt = typename Gps_traits::Point_2;
    using Xcv = typename Gps_traits::X_monotone_curve_2;
    using Make_x_monotone_result = std::variant<Pnt, Xcv>;

    std::vector<Polyline_X_monotone_curve> xcvs;
    for (auto it = m_polyline_polygon_pieces.begin(); it != m_polyline_polygon_pieces.end(); ++it) {
      std::vector<Make_x_monotone_result> x_objs;
      make_x_monotone(*it, std::back_inserter(x_objs));
      for (std::size_t i = 0; i < x_objs.size(); ++i) {
        auto* xcv = std::get_if<Polyline_X_monotone_curve>(&x_objs[i]);
        CGAL_assertion(xcv != nullptr);
        xcvs.push_back(*xcv);
      }
    }
    if (xcvs.size() == 0) return;

    const Polyline_point first_point = xcvs.front()[0].source();
    const Polyline_point last_point = xcvs.back()[xcvs.back().number_of_subcurves() - 1].target();
    if (! closed && (last_point != first_point)) {
      FT fxs = first_point.x();
      FT fys = first_point.y();
      FT lxs = last_point.x();
      FT lys = last_point.y();
      fxs = fxs * -1;
      lxs = lxs * -1;
      Gps_traits x;
      X_monotone_subcurve_2 seg =
        x.subcurve_traits_2()->construct_x_monotone_curve_2_object()(Point(fxs, fys), Point(lxs, lys));
      xcvs.push_back(Polyline_X_monotone_curve(seg));
    }
    if (number_of_edges(xcvs) < 3) return;
    Polyline_polygon pp(xcvs.begin(), xcvs.end());
    emit(generate(std::variant<Polyline_polygon>(pp)));
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
    commit_complement_curr_polyline_polygon();
    restart();
  }

  /*! determines whether the last polygon is the bounding polygon.
   */
  bool is_bounding_rect() { return m_bound_rect; }

public:
  QGraphicsScene* m_scene;
  GI* m_polyline_GI;
  GI* m_ongoing_piece_GI;
  QGraphicsLineItem* m_handle_GI;

  QPen m_polyline_polygon_pen;
  QPen m_ongoing_curve_pen;
  QPen m_handle_pen;

  bool m_bound_rect;
  bool m_last_polyline;
  bool m_last;

  Polyline_curve_vector m_polyline_polygon_pieces;
  Polyline_curve_vector m_ongoing_piece_ctr;

  State m_state;

  Point m_p0;
  Point m_p1;

  std::optional<Point> m_h;
};

} // namespace Qt
} // namespace CGAL

#endif
