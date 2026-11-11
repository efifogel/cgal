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
  using const_linear_curve_iterator = typename Linear_curve_vector::const_iterator;

  using GI = Linear_boundary_pieces_graphics_item<Linear_curve_vector>;

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_linear_polygon_input(QObject* aParent, QGraphicsScene* aScene) :
    GraphicsViewInput(aParent),
    mScene(aScene),
    mLinearGI(new GI(&mLinearPolygonPieces)),
    mOngoingPieceGI(new GI(&mOngoingPieceCtr)),
    mHandleGI(new QGraphicsLineItem()),
    mLinearPolygonPen(QColor(0, 255, 0)),
    mOngoingCurvePen(QColor(255, 215, 0)),
    mHandlePen(QColor(255, 165, 0)),
    mState(Start),
    m_bound_rect(true),
    m_is_mink(false),
    m_hole(false) {
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
  ~Graphics_view_linear_polygon_input() {}

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

public:
  enum State { Start, PieceStarted, PieceOngoing, HandleOngoing, PieceEnded, CurveEnded };

  /*! converts a scene point to a point.
   */
  Point cvt(const QPointF& aP) const { return Point(aP.x(), aP.y()); }

  /*! handles a mouse press.
   * \return true if the event is handled.
   */
  bool mousePressEvent(QGraphicsSceneMouseEvent* aEvent) {
    m_bound_rect = false;
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

  /*! handles a mouse release. A left release completes the edge under construction; a right release closes the
   * polygon.
   * \return true if the event is handled.
   */
  bool mouseReleaseEvent(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    Point lP = cvt(aEvent->scenePos());
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
    }
    else if (aEvent->button() == ::Qt::RightButton) {
      switch (mState) {
       case PieceOngoing:
        m_bound_rect = false;
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
      mState = (mLinearPolygonPieces.size() > 0) ? PieceEnded : Start;
      rHandled = true;
    }
    else if (aEvent->key() == ::Qt::Key_Escape) {
      Reset();
      mState = Start;
      rHandled = true;
    }
    return rHandled;
  }

  /*! obtains the edge under construction, or null if there is none.
   */
  const Linear_curve* ongoing_piece() const { return (mOngoingPieceCtr.size() == 1) ? &mOngoingPieceCtr[0] : NULL; }

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

  /*! adds the edge under construction to the polygon (unless it is degenerate), and starts the next edge.
   */
  void CommitOngoingPiece(const Point& aP) {
    if (! ongoing_piece()) return;
    if (ongoing_piece()->source() != ongoing_piece()->target()) {
      mLinearPolygonPieces.push_back(*ongoing_piece());
      mLinearGI->modelChanged();
    }
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

  /*! closes the polygon with an edge from its last vertex to its first (unless they coincide), and emits it.
   */
  void GenerateLinearPolygon() {
    if (mLinearPolygonPieces.size() <= 1) return;
    Gps_traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    using Pnt = typename Gps_traits::Point_2;
    using Xcv = typename Gps_traits::X_monotone_curve_2;
    using Make_x_monotone_result = std::variant<Pnt, Xcv>;

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
  void get_BoundingRect() {
    m_bound_rect = true;

    mP0 = Point(-15500000, -10000000);
    mState = PieceStarted;
    mState = PieceOngoing;

    mP1 = Point(-15500000, 10000000);
    UpdateOngoingPiece();

    mP1 = Point(-15500000, 10000000);
    mState = HandleOngoing;
    HideHandle();
    CommitOngoingPiece(Point(-15500000, 10000000));
    mState = PieceEnded;

    mState = PieceOngoing;

    mP1 = Point(15500000, 10000000);
    UpdateOngoingPiece();

    mP1 = Point(15500000, 10000000);
    mState = HandleOngoing;
    HideHandle();
    CommitOngoingPiece(Point(15500000, 10000000));
    mState = PieceEnded;

    mState = PieceOngoing;

    mP1 = Point(15500000, -10000000);
    UpdateOngoingPiece();

    mP1 = Point(15500000, -10000000);
    mState = HandleOngoing;
    HideHandle();
    CommitOngoingPiece(Point(15500000, -10000000));
    mState = PieceEnded;

    mState = PieceOngoing;

    mP1 = Point(-9000000, -9000000);
    UpdateOngoingPiece();

    CommitCurrLinearPolygon();
    ReStart();
  }

  /*! constructs polygons from the holes of a given polygon with holes (e.g., the result of a Minkowski sum).
   */
  void get_Minkowski_holes(Polygon_with_holes_2 polygon, Polygon_with_holes_2 p0) {
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
          mP0 = p;
          mState = PieceStarted;
        }
        else {
          mState = PieceOngoing;
          mP1 = p;
          UpdateOngoingPiece();
          mP1 = p;
          mState = HandleOngoing;
          HideHandle();
          CommitOngoingPiece(p);
          mState = PieceEnded;
        }
        i++;
      }

      mState = PieceOngoing;
      mP1 = Point(-90, -90);
      UpdateOngoingPiece();

      CommitCurrLinearPolygon();
      ReStart();
    }
  }

  /*! constructs a polygon from the outer boundary of a given polygon with holes (e.g., the result of a Minkowski
   * sum).
   */
  void get_Minkowski_result(Polygon_with_holes_2 polygon, Polygon_with_holes_2 p0) {
    m_bound_rect = false;
    Point_2 pt;

    int i = 0;
    for (auto vit = polygon.outer_boundary().vertices_begin(); vit != polygon.outer_boundary().vertices_end(); ++vit) {
      pt = *vit;
      Point p(CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
      if (i == 0) {
        mP0 = p;
        mState = PieceStarted;
      }
      else {
        mState = PieceOngoing;
        mP1 = p;
        UpdateOngoingPiece();
        mP1 = p;
        mState = HandleOngoing;
        HideHandle();
        CommitOngoingPiece(p);
        mState = PieceEnded;
      }
      i++;
    }

    mState = PieceOngoing;
    mP1 = Point(-90, -90);
    UpdateOngoingPiece();

    CommitCurrLinearPolygon();
    ReStart();
  }

  /*! determines whether the last polygon is the bounding polygon.
   */
  bool isboundingRect() { return m_bound_rect; }

  /*! determines whether the last polygon is a hole.
   */
  bool ishole() { return m_hole; }

  /*! determines whether the last polygon is a Minkowski sum.
   */
  bool is_mink() { return m_is_mink; }

public:
  QGraphicsScene* mScene;
  GI* mLinearGI;
  GI* mOngoingPieceGI;
  QGraphicsLineItem* mHandleGI;

  QPen mLinearPolygonPen;
  QPen mOngoingCurvePen;
  QPen mHandlePen;

  Linear_curve_vector mLinearPolygonPieces;
  Linear_curve_vector mLinearPolygonPieces_boundR;
  Linear_curve_vector mOngoingPieceCtr;

  int mState;

  bool m_bound_rect;
  bool m_is_mink;
  bool m_hole;

  Point mP0;
  Point mP1;
  std::optional<Point> mH;
};

} // namespace Qt
} // namespace CGAL

#endif
