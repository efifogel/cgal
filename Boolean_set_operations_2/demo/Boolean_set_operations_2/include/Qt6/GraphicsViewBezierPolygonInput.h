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

#include "Qt6/BezierCurves.h"

namespace CGAL {
namespace Qt {

/*! Constructs a Bezier polygon from mouse clicks and drags. Dragging at a vertex sets the handles (the inner control
 * points) of the curves that meet at the vertex.
 */
template <class Traits_>
class GraphicsViewBezierPolygonInput : public GraphicsViewInput {
public:
  using Traits = Traits_;
  using Bezier_gps_traits = CGAL::Gps_traits_2<Traits>;
  using Bezier_curve = typename Traits::Curve_2;
  using Bezier_X_monotone_curve = typename Traits::X_monotone_curve_2;
  using Bezier_polygon = typename Bezier_gps_traits::General_polygon_2;
  using Vector = typename Traits::Rat_kernel::Vector_2;
  using Point = typename Traits::Rat_kernel::Point_2;

  using Bezier_curve_vector = std::vector<Bezier_curve>;
  using const_bezier_curve_iterator = typename Bezier_curve_vector::const_iterator;

  using GI = Bezier_boundary_pieces_graphics_item<Bezier_curve_vector>;

  /*! constructs from the parent object and the scene.
   */
  GraphicsViewBezierPolygonInput(QObject* aParent, QGraphicsScene* aScene) :
    GraphicsViewInput(aParent),
    mScene(aScene),
    mBezierGI(0),
    mBezierPolygonPen(QColor(0, 255, 0)),
    mOngoingCurvePen(QColor(255, 0, 0)),
    mHandlePen(QColor(0, 0, 255)),
    m_bound_rect(true),
    m_last_bezier(false),
    m_last(false),
    mState(Start) {
    mOngoingPieceGI = new GI(&mOngoingPieceCtr);
    mHandle0GI = new QGraphicsLineItem();
    mHandle1GI = new QGraphicsLineItem();

    mOngoingPieceGI->set_pen(mOngoingCurvePen);
    mHandle0GI->setPen(mHandlePen);
    mHandle1GI->setPen(mHandlePen);

    mHandle0GI->setLine(0, 0, 1, 1);
    mHandle1GI->setLine(0, 0, 1, 1);
    mHandle0GI->hide();
    mHandle1GI->hide();

    mBezierGI = new GI(&mBezierPolygonPieces);
    mBezierGI->set_pen(mBezierPolygonPen);

    mScene->addItem(mOngoingPieceGI);
    mScene->addItem(mHandle0GI);
    mScene->addItem(mHandle1GI);
    mScene->addItem(mBezierGI);
  }

  /*! destructs.
   */
  ~GraphicsViewBezierPolygonInput() {}

  /*! handles the mouse and key events of the scene.
   * \return true if the event is handled.
   */
  bool eventFilter(QObject* obj, QEvent* aEvent) {
    bool rHandled = false;
    if (aEvent->type() == QEvent::GraphicsSceneMousePress)
      rHandled = mouse_press(static_cast<QGraphicsSceneMouseEvent*>(aEvent));
    else if (aEvent->type() == QEvent::GraphicsSceneMouseRelease)
      rHandled = mouse_release(static_cast<QGraphicsSceneMouseEvent*>(aEvent));
    else if (aEvent->type() == QEvent::GraphicsSceneMouseMove)
      rHandled = mouse_move(static_cast<QGraphicsSceneMouseEvent*>(aEvent));
    else if (aEvent->type() == QEvent::KeyPress)
      rHandled = key_press(static_cast<QKeyEvent*>(aEvent));
    if (! rHandled) rHandled = QObject::eventFilter(obj, aEvent);
    return rHandled;
  }

public:
  enum State {
    Start, PieceOrFirstHandleStarted, PieceOngoing, FirstHandleOngoing, HandleOngoing, PieceEnded, CurveEnded
  };

  /*! converts a scene point to a point.
   */
  Point cvt(const QPointF& aP) const { return Point(aP.x(), aP.y()); }

  /*! handles a mouse press. A right press while a curve is under construction lets the user set the handles of the
   * closing curve too.
   * \return true if the event is handled.
   */
  bool mouse_press(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    m_bound_rect = false;
    Point lP = cvt(aEvent->scenePos());
    if (aEvent->button() == ::Qt::LeftButton) {
      switch (mState) {
       case Start:
        mP0 = lP;
        mState = PieceOrFirstHandleStarted;
        rHandled = true;
        break;

       case PieceOngoing:
        mP1 = lP;
        mState = HandleOngoing;
        rHandled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    else if (aEvent->button() == ::Qt::RightButton) {
      switch (mState) {
       case PieceOngoing:
        m_last = true;
        mState = HandleOngoing;
        rHandled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return rHandled;
  }

  /*! handles a mouse move.
   * \return true if the event is handled.
   */
  bool mouse_move(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    Point lP = cvt(aEvent->scenePos());
    switch (mState) {
     case PieceOrFirstHandleStarted:
      mState = FirstHandleOngoing;
      rHandled = true;
      break;

     case PieceOngoing:
      mP1 = lP;
      update_ongoing_piece();
      rHandled = true;
      break;

     case FirstHandleOngoing:
      update_very_first_handle(lP);
      rHandled = true;
      break;

     case HandleOngoing:
      if (m_last) {
        mP1 = mBezierPolygonPieces.front().control_point(0);
        m_last_bezier = true;
      }
      update_handles(lP);
      update_ongoing_piece();
      rHandled = true;
      break;

     case PieceEnded:
      mState = PieceOngoing;
      rHandled = true;
      break;
    }
    return rHandled;
  }

  /*! handles a mouse release. A right release closes the polygon.
   * \return true if the event is handled.
   */
  bool mouse_release(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    Point lP = cvt(aEvent->scenePos());
    if (aEvent->button() == ::Qt::LeftButton) {
      switch (mState) {
       case PieceOrFirstHandleStarted:
        mState = PieceOngoing;
        rHandled = true;
        break;

       case FirstHandleOngoing:
        update_very_first_handle(lP);
        mPrevH0 = mH1;
        mH1 = std::optional<Point>();
        mState = PieceOngoing;
        rHandled = true;
        break;

       case HandleOngoing:
        update_handles(lP);
        commit_ongoing_piece(lP);
        mState = PieceEnded;
        rHandled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    else if (aEvent->button() == ::Qt::RightButton) {
      switch (mState) {
       case HandleOngoing:
        m_bound_rect = false;
        if (m_last_bezier) {
          hide_handles();
          commit_ongoing_piece(lP);
        }
        close_curr_boundary();
        commit_curr_bezier_polygon();
        restart();
        rHandled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return rHandled;
  }

  /*! handles a key press. Delete and Backspace remove the last curve; Escape discards the polygon.
   * \return true if the event is handled.
   */
  bool key_press(QKeyEvent* aEvent) {
    bool rHandled = false;
    if (aEvent->key() == ::Qt::Key_Delete || aEvent->key() == ::Qt::Key_Backspace) {
      remove_last_piece();
      mState = mBezierPolygonPieces.size() > 0 ? PieceEnded : Start;
      rHandled = true;
    }
    else if (aEvent->key() == ::Qt::Key_Escape) {
      reset();
      mState = Start;
      rHandled = true;
    }
    return rHandled;
  }

public:
  /*! obtains the curve under construction, or null if there is none.
   */
  const Bezier_curve* ongoing_piece() const { return mOngoingPieceCtr.size() == 1 ? &mOngoingPieceCtr[0] : NULL; }

  /*! starts a new polygon.
   */
  void restart() {
    mPrevH0 = mH0 = mH1 = std::optional<Point>();
    mState = Start;
  }

  /*! discards the polygon under construction.
   */
  void reset() {
    mBezierPolygonPieces.clear();
    mOngoingPieceCtr.clear();
    mBezierGI->modelChanged();
    mOngoingPieceGI->modelChanged();
    restart();
  }

  /*! removes the last curve.
   */
  void remove_last_piece() {
    mBezierPolygonPieces.pop_back();
    mOngoingPieceCtr.clear();
    mBezierGI->modelChanged();
    mOngoingPieceGI->modelChanged();
    if (mBezierPolygonPieces.size() > 0) {
      mP0 = mBezierPolygonPieces.back().control_point(mBezierPolygonPieces.back().number_of_control_points() - 1);
      update_ongoing_piece();
    }
    mPrevH0 = mH0 = mH1 = std::optional<Point>();
  }

  /*! hides the handles.
   */
  void hide_handles() {
    mHandle0GI->hide();
    mHandle1GI->hide();
  }

  /*! creates the curve from the last vertex to the current point, using the handles that are set.
   */
  Bezier_curve create_piece() {
    if (mPrevH0 && mH1 && *mPrevH0 != *mH1 && *mPrevH0 != mP0 && *mH1 != mP1) {
      Point lControlPoints[4] = {mP0, *mPrevH0, *mH1, mP1};
      return Bezier_curve(lControlPoints, lControlPoints + 4);
    }
    if (mPrevH0 && ! mH1 && *mPrevH0 != mP0 && *mPrevH0 != mP1) {
      Point lControlPoints[3] = {mP0, *mPrevH0, mP1};
      return Bezier_curve(lControlPoints, lControlPoints + 3);
    }
    if (! mPrevH0 && mH1 && *mH1 != mP0 && *mH1 != mP1) {
      Point lControlPoints[3] = {mP0, *mH1, mP1};
      return Bezier_curve(lControlPoints, lControlPoints + 3);
    }
    Point lControlPoints[2] = {mP0, mP1};
    return Bezier_curve(lControlPoints, lControlPoints + 2);
  }

  /*! updates the curve under construction.
   */
  void update_ongoing_piece() {
    if (mOngoingPieceCtr.size() > 0) mOngoingPieceCtr.clear();
    mOngoingPieceCtr.push_back(create_piece());
    mOngoingPieceGI->modelChanged();
  }

  /*! determines whether a curve is degenerate, that is, its endpoints coincide.
   */
  static bool is_degenerate(const Bezier_curve& aCurve) {
    return aCurve.control_point(0) == aCurve.control_point(aCurve.number_of_control_points() - 1);
  }

  /*! adds the curve under construction to the polygon (unless it is degenerate), and starts the next curve.
   */
  void commit_ongoing_piece(const Point& aP) {
    if (! ongoing_piece()) return;
    if (! is_degenerate(*ongoing_piece())) {
      mBezierPolygonPieces.push_back(*ongoing_piece());
      mBezierGI->modelChanged();
    }
    mOngoingPieceCtr.clear();
    mOngoingPieceGI->modelChanged();
    mP0 = mP1;
    mP1 = aP;
    mPrevH0 = mH0;
    mH0 = mH1 = std::optional<Point>();
  }

  /*! updates the handle of the first vertex; it is shown only when it is at least 3 units long.
   */
  void update_very_first_handle(const Point& aP) {
    if (squared_distance(mP0, aP) >= 9) {
      mH1 = aP;
      mHandle1GI->setLine(to_double(mP0.x()), to_double(mP0.y()), to_double(mH1->x()), to_double(mH1->y()));
      mHandle1GI->show();
      mH0 = std::optional<Point>();
      mHandle0GI->hide();
    }
    else {
      hide_handles();
      mH0 = mH1 = std::optional<Point>();
    }
  }

  /*! updates the two (symmetric) handles of the current vertex; they are shown only when they are at least 3 units
   * long.
   */
  void update_handles(const Point& aP) {
    if (squared_distance(mP1, aP) >= 9) {
      mH0 = aP;
      mH1 = mP1 - (aP - mP1);
      mHandle0GI->setLine(to_double(mP1.x()), to_double(mP1.y()), to_double(mH0->x()), to_double(mH0->y()));
      mHandle1GI->setLine(to_double(mP1.x()), to_double(mP1.y()), to_double(mH1->x()), to_double(mH1->y()));
      mHandle0GI->show();
      mHandle1GI->show();
    }
    else {
      hide_handles();
      mH0 = mH1 = std::optional<Point>();
    }
  }

  /*! closes the boundary with a curve that ends at the first vertex (unless the last vertex is the first).
   */
  void close_curr_boundary() {
    if ((mBezierPolygonPieces.size() == 0) || (ongoing_piece() == NULL) || m_last_bezier) return;
    std::vector<Point> lControlPoints(ongoing_piece()->control_points_begin(), ongoing_piece()->control_points_end());
    lControlPoints.back() = mBezierPolygonPieces.front().control_point(0);
    if (lControlPoints.front() == lControlPoints.back()) return;
    mBezierPolygonPieces.push_back(Bezier_curve(lControlPoints.begin(), lControlPoints.end()));
    mBezierGI->modelChanged();
  }

  /*! completes the polygon under construction.
   */
  void commit_curr_bezier_polygon() {
    generate_bezier_polygon();

    mOngoingPieceCtr.clear();
    mOngoingPieceGI->modelChanged();

    mBezierPolygonPieces.clear();
    mBezierGI->modelChanged();

    mPrevH0 = mH0 = mH1 = std::optional<Point>();

    hide_handles();
  }

  /*! emits the polygon, together with its curves. A polygon bounded by fewer than three segments, or by a single curve,
   * is dropped.
   */
  void generate_bezier_polygon() {
    m_last = false;
    m_last_bezier = false;
    auto n = mBezierPolygonPieces.size();
    bool linear = std::all_of(mBezierPolygonPieces.begin(), mBezierPolygonPieces.end(),
                              [](const Bezier_curve& cv) { return cv.number_of_control_points() == 2; });
    if ((n < 2) || (linear && (n < 3))) return;
    Traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    using Make_x_monotone_result = std::variant<typename Traits::Point_2, Bezier_X_monotone_curve>;

    std::vector<Bezier_X_monotone_curve> xcvs;
    for (auto it = mBezierPolygonPieces.begin(); it != mBezierPolygonPieces.end(); ++it) {
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
    emit(generate(std::variant<std::pair<Bezier_polygon, Bezier_curve_vector>>(std::make_pair(bp,
                                                                                              mBezierPolygonPieces))));
  }

  /*! constructs a huge (bounding) polygon.
   */
  void get_bounding_rect() {
    m_bound_rect = true;

    mP0 = Point(-15500000, -10000000);
    mState = PieceOrFirstHandleStarted;

    mState = PieceOngoing;
    mP1 = Point(-15500000, 10000000);
    update_ongoing_piece();

    mP1 = Point(-15500000, 10000000);
    mState = HandleOngoing;
    update_handles(Point(-15500000, 10000000));
    commit_ongoing_piece(Point(-15500000, 10000000));
    mState = PieceEnded;

    mState = PieceOngoing;
    mP1 = Point(15500000, 10000000);
    update_ongoing_piece();

    mP1 = Point(15500000, 10000000);
    mState = HandleOngoing;
    update_handles(Point(15500000, 10000000));
    commit_ongoing_piece(Point(15500000, 10000000));
    mState = PieceEnded;

    mState = PieceOngoing;
    mP1 = Point(15500000, -10000000);
    update_ongoing_piece();

    mP1 = Point(15500000, -10000000);
    mState = HandleOngoing;
    update_handles(Point(15500000, -10000000));
    commit_ongoing_piece(Point(15500000, -10000000));
    mState = PieceEnded;

    mState = PieceOngoing;
    mP1 = Point(-9000000, -9000000);
    update_ongoing_piece();

    close_curr_boundary();
    commit_curr_bezier_polygon();
    restart();
  }

  /*! determines whether the last polygon is the bounding polygon.
   */
  bool is_bounding_rect() { return m_bound_rect; }

public:
  QGraphicsScene* mScene;
  GI* mBezierGI;
  GI* mOngoingPieceGI;
  QGraphicsLineItem* mHandle0GI;
  QGraphicsLineItem* mHandle1GI;

  QPen mBezierPolygonPen;
  QPen mOngoingCurvePen;
  QPen mHandlePen;

  bool m_bound_rect;
  bool m_last_bezier;
  bool m_last;

  Bezier_curve_vector mBezierPolygonPieces;
  Bezier_curve_vector mOngoingPieceCtr;

  int mState;

  Point mP0;
  Point mP1;

  std::optional<Point> mPrevH0;
  std::optional<Point> mH0;
  std::optional<Point> mH1;
};

} // namespace Qt
} // namespace CGAL

#endif
