
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
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>

#ifndef CGAL_QT6_GRAPHICS_VIEW_MINKOWSKI_INPUT_H
#define CGAL_QT6_GRAPHICS_VIEW_MINKOWSKI_INPUT_H

#include <list>

#include <QPolygonF>
#include <QPointF>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <CGAL/auto_link/Qt.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>

#include "Typedefs.h"
#include "Qt6/Linear_polygons.h"

namespace CGAL {
namespace Qt {

/*! Constructs a linear polygon, an operand of a Minkowski sum, from mouse clicks.
 */
template <typename Kernel_>
class Graphics_view_minkowski_input : public GraphicsViewInput {
public:
  using Kernel1 = CGAL::Exact_predicates_exact_constructions_kernel;
  using Point_2 = Kernel1::Point_2;
  using Polygon_2 = CGAL::Polygon_2<Kernel1>;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel1>;
  using Pgn_with_holes_2_container = std::list<Polygon_with_holes_2>;

  using Kernel2 = Kernel_;

  using Gps_traits = Linear_traits;
  using Linear_curve = typename Gps_traits::Curve_2;
  using Linear_X_monotone_curve = typename Gps_traits::X_monotone_curve_2;
  using Linear_polygon = typename Gps_traits::Polygon_2;
  using Linear_point = typename Gps_traits::Point_2;
  using FT = typename Kernel2::FT;
  using Vector = typename Kernel2::Vector_2;
  using Point = typename Kernel2::Point_2;
  using Linear_curve_vector = std::vector<Linear_curve>;
  using const_linear_curve_iterator = typename Linear_curve_vector::const_iterator;

  using GI = Linear_boundary_pieces_graphics_item<Linear_curve_vector>;

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_minkowski_input(QObject* aParent, QGraphicsScene* aScene) :
    GraphicsViewInput(aParent),
    mScene(aScene),
    mLinearGI(new GI(&mLinearPolygonPieces)),
    mOngoingPieceGI(new GI(&mOngoingPieceCtr)),
    mHandleGI(new QGraphicsLineItem()),
    mLinearPolygonPen(QColor(0, 255, 0)),
    mOngoingCurvePen(QColor(255, 215, 0)),
    mHandlePen(QColor(255, 165, 0)),
    mState(Start) {
    mOngoingPieceGI->setPen(mOngoingCurvePen);
    mHandleGI->setPen(mHandlePen);

    mHandleGI->setLine(0, 0, 1, 1);
    mHandleGI->hide();

    mLinearGI->setPen(mLinearPolygonPen);

    mScene->addItem(mOngoingPieceGI);
    mScene->addItem(mHandleGI);
    mScene->addItem(mLinearGI);
  }

  /*! destructs.
   */
  ~Graphics_view_minkowski_input() {}

  /*! handles the mouse and key events of the scene.
   * \return true if the event is handled.
   */
  bool eventFilter(QObject* obj, QEvent* aEvent) {
    bool rHandled = false;
    if (aEvent->type() == QEvent::GraphicsSceneMousePress)
      rHandled = mousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(aEvent));
    else if (aEvent->type() == QEvent::GraphicsSceneMouseRelease)
      rHandled = mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(aEvent));
    else if (aEvent->type() == QEvent::GraphicsSceneMouseMove)
      rHandled = mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(aEvent));
    else if (aEvent->type() == QEvent::KeyPress)
      rHandled = keyPressEvent(static_cast<QKeyEvent*>(aEvent));
    if (! rHandled) rHandled = QObject::eventFilter(obj, aEvent);
    return rHandled;
  }

  /*! starts a new polygon.
   */
  void ReStart() {
    mH = std::optional<Point>();
    mState = Start;
  }

  /*! discards the polygon under construction.
   */
  void Reset() {
    mLinearPolygonPieces.clear();
    mOngoingPieceCtr.clear();
    mLinearGI->modelChanged();
    mOngoingPieceGI->modelChanged();
    ReStart();
  }

protected:
  enum State { Start, PieceStarted, PieceOngoing, HandleOngoing, PieceEnded, CurveEnded };

  /*! converts a scene point to a point.
   */
  Point cvt(const QPointF& aP) const { return Point(aP.x(), aP.y()); }

  /*! converts a scene point to a kernel point.
   */
  Point_2 cvt1(const QPointF& aP) const { return Point_2(aP.x(), aP.y()); }

  /*! obtains the polygon of the clicked vertices.
   */
  Polygon_2 getMinkPolygon() { return mink_polygon; }

  /*! handles a mouse press.
   * \return true if the event is handled.
   */
  bool mousePressEvent(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    Point lP = cvt(aEvent->scenePos());
    if (aEvent->button() == ::Qt::LeftButton) {
      switch (mState) {
       case Start:
        mP0 = lP;
        mState = PieceStarted;
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
    return rHandled;
  }

  /*! handles a mouse move.
   * \return true if the event is handled.
   */
  bool mouseMoveEvent(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    Point lP = cvt(aEvent->scenePos());
    switch (mState) {
     case PieceOngoing:
      mP1 = lP;
      UpdateOngoingPiece();
      rHandled = true;
      break;

     case HandleOngoing:
      UpdateHandle(lP);
      UpdateOngoingPiece();
      rHandled = true;
      break;

     case PieceEnded:
      mState = PieceOngoing;
      rHandled = true;
      break;

     default: break; //! \todo handle default case
    }
    return rHandled;
  }

  /*! handles a mouse release. A right release closes the polygon.
   * \return true if the event is handled.
   */
  bool mouseReleaseEvent(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    Point lP = cvt(aEvent->scenePos());
    const QPointF& aP = aEvent->scenePos();
    if (aEvent->button() == ::Qt::LeftButton) {
      switch (mState) {
       case PieceStarted:
        mState = PieceOngoing;
        rHandled = true;
        break;

       case HandleOngoing:
        HideHandle();
        CommitOngoingPiece(lP);
        mState = PieceEnded;
        rHandled = true;
        break;

       default: break; //! \todo handle default case
      }
      mink_polygon.push_back(Point_2(aP.x(), aP.y()));
      vertex_count++;
    }
    else if (aEvent->button() == ::Qt::RightButton) {
      switch (mState) {
       case PieceOngoing:
        CommitCurrLinearPolygon();
        ReStart();
        rHandled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return rHandled;
  }

  /*! handles a key press. Delete and Backspace remove the last edge; Escape discards the polygon.
   * \return true if the event is handled.
   */
  bool keyPressEvent(QKeyEvent* aEvent) {
    bool rHandled = false;
    if ((aEvent->key() == ::Qt::Key_Delete) || (aEvent->key() == ::Qt::Key_Backspace)) {
      RemoveLastPiece();
      mink_polygon.erase(mink_polygon.vertices_end());
      mState = (mLinearPolygonPieces.size() > 0) ? PieceEnded : Start;
      rHandled = true;
    }
    else if (aEvent->key() == ::Qt::Key_Escape) {
      Reset();
      mink_polygon.clear();
      mState = Start;
      rHandled = true;
    }
    return rHandled;
  }

private:
  /*! obtains the edge under construction, or null if there is none.
   */
  const Linear_curve* ongoing_piece() const { return (mOngoingPieceCtr.size() == 1) ? &mOngoingPieceCtr[0] : NULL; }

  /*! hides the handle.
   */
  void HideHandle() { mHandleGI->hide(); }

  /*! creates the edge from the last vertex to the current point.
   */
  Linear_curve CreatePiece() { return Linear_curve(mP0, mP1); }

  /*! removes the last edge.
   */
  void RemoveLastPiece() {
    mLinearPolygonPieces.pop_back();
    mOngoingPieceCtr.clear();
    mLinearGI->modelChanged();
    mOngoingPieceGI->modelChanged();
    if (mLinearPolygonPieces.size() > 0) {
      mP0 = cvt(mLinearPolygonPieces.back().target());
      UpdateOngoingPiece();
    }
    mH = std::optional<Point>();
  }

  /*! updates the edge under construction.
   */
  void UpdateOngoingPiece() {
    if (mOngoingPieceCtr.size() > 0) mOngoingPieceCtr.clear();
    mOngoingPieceCtr.push_back(CreatePiece());
    mOngoingPieceGI->modelChanged();
  }

  /*! adds the edge under construction to the polygon, and starts the next edge.
   */
  void CommitOngoingPiece(const Point& aP) {
    if (! ongoing_piece()) return;
    mLinearPolygonPieces.push_back(*ongoing_piece());
    mLinearGI->modelChanged();
    mOngoingPieceCtr.clear();
    mOngoingPieceGI->modelChanged();
    mP0 = mP1;
    mP1 = aP;
    mH = std::optional<Point>();
  }

  /*! updates the handle; it is shown only when it is at least 2 units long.
   */
  void UpdateHandle(const Point& aP) {
    if (squared_distance(mP1, aP) >= 4) {
      mH = aP;
      mHandleGI->setLine(to_double(mP1.x()), to_double(mP1.y()), to_double(mH->x()), to_double(mH->y()));
      mHandleGI->show();
    }
    else {
      HideHandle();
      mH = std::optional<Point>();
    }
  }

  /*! rounds a point to double precision.
   */
  Point cvt(const Point& aP) { return Point(to_double(aP.x()), to_double(aP.y())); }

  /*! completes the polygon under construction.
   */
  void CommitCurrLinearPolygon() {
    GenerateLinearPolygon();

    mOngoingPieceCtr.clear();
    mOngoingPieceGI->modelChanged();

    mLinearPolygonPieces.clear();
    mLinearGI->modelChanged();

    mH = std::optional<Point>();

    HideHandle();
  }

  /*! closes the polygon with an edge from its last vertex to its first, and emits it.
   */
  void GenerateLinearPolygon() {
    if (mLinearPolygonPieces.size() <= 1) return;
    Gps_traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    using Make_x_monotone_result = std::variant<Linear_X_monotone_curve, Linear_point>;

    std::vector<Linear_X_monotone_curve> xcvs;
    for (auto it = mLinearPolygonPieces.begin(); it != mLinearPolygonPieces.end(); ++it) {
      std::vector<Make_x_monotone_result> x_objs;
      make_x_monotone(*it, std::back_inserter(x_objs));
      for (std::size_t i = 0; i < x_objs.size(); ++i) {
        auto* xcv = std::get_if<Linear_X_monotone_curve>(&x_objs[i]);
        CGAL_assertion(xcv != nullptr);
        xcvs.push_back(*xcv);
      }
    }

    if (xcvs.size() == 0) return;
    const Linear_point& first_point = xcvs.front().source();
    const Linear_point& last_point = xcvs.back().target();
    FT fxs = first_point.x();
    FT fys = first_point.y();
    FT lxs = last_point.x();
    FT lys = last_point.y();
    xcvs.push_back(Linear_X_monotone_curve(Point(lxs, lys), Point(fxs, fys)));
    Linear_polygon lp(xcvs.begin(), xcvs.end());
    emit(generate(std::variant<Linear_polygon>(lp)));
  }

public:
  QGraphicsScene* mScene;
  GI* mLinearGI;
  GI* mOngoingPieceGI;
  QGraphicsLineItem* mHandleGI;

  QPen mLinearPolygonPen;
  QPen mOngoingCurvePen;
  QPen mHandlePen;

  Linear_curve_vector mLinearPolygonPieces;
  Linear_curve_vector mOngoingPieceCtr;

  int mState;

  int vertex_count = 0;
  Polygon_2 mink_polygon;

  Point mP0;
  Point mP1;
  std::optional<Point> mH;
};

} // namespace Qt
} // namespace CGAL

#endif
