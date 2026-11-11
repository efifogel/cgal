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
  using const_circular_curve_iterator = typename Circular_curve_vector::const_iterator;

  using GI = Circular_boundary_pieces_graphics_item<Circular_curve_vector>;

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_circular_polygon_input(QObject* aParent, QGraphicsScene* aScene) :
    GraphicsViewInput(aParent),
    mScene(aScene),
    mOngoingPieceGI(new GI(&mOngoingPieceCtr)),
    mHandleGI(new QGraphicsLineItem()),
    mCircleGI(new QGraphicsEllipseItem()),
    mCircularPolygonPen(QColor(0, 255, 0)),
    mOngoingCurvePen(QColor(255, 215, 0)),
    mHandlePen(QColor(255, 165, 0)),
    m_bound_rect(true),
    m_last_circular(false),
    m_last(false),
    mState(Start) {
    mOngoingPieceGI->setPen(mOngoingCurvePen);
    mHandleGI->setPen(mHandlePen);

    mHandleGI->setLine(0, 0, 1, 1);
    mHandleGI->hide();
    mCircleGI->hide();

    mCircularGI = new GI(&mCircularPolygonPieces);
    mCircularGI->setPen(mCircularPolygonPen);

    mScene->addItem(mOngoingPieceGI);
    mScene->addItem(mHandleGI);
    mScene->addItem(mCircleGI);
    mScene->addItem(mCircularGI);
  }

  /*! destructs.
   */
  ~Graphics_view_circular_polygon_input() {}

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
  // The states of the construction.
  enum State { Start, PieceStarted, PieceOngoing, HandleOngoing, PieceEnded, CurveEnded, CircleOngoing };

  // What the user constructs: a boundary of line segments and circular arcs, or a full circle.
  enum Mode { Arcs, Circle_mode };

  /*! sets what the user constructs, and discards the polygon under construction.
   */
  void set_mode(Mode aMode) {
    Reset();
    mMode = aMode;
  }

  /*! converts a scene point to a point.
   */
  Point cvt(const QPointF& aP) const { return Point(aP.x(), aP.y()); }

  /*! handles a mouse press. In the circle mode, a press sets the center. Otherwise, a left press ends the segment
   * under construction (a drag then bends it into an arc), and a right press starts closing the polygon.
   * \return true if the event is handled.
   */
  bool mousePressEvent(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    m_bound_rect = false;
    Point lP = cvt(aEvent->scenePos());
    if (mMode == Circle_mode) {
      // Press at the center, drag to the radius, and release.
      if ((aEvent->button() == ::Qt::LeftButton) && (mState == Start)) {
        mCenter = lP;
        mState = CircleOngoing;
        UpdateCircle(lP);
        rHandled = true;
      }
      return rHandled;
    }

    if (aEvent->button() == ::Qt::LeftButton) {
      switch (mState) {
       case Start:
        mP0 = lP;
        mState = PieceStarted;
        rHandled = true;
        break;

       case PieceOngoing:
        mP1 = lP;
        mPress = lP;
        mState = HandleOngoing;
        rHandled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    else if (aEvent->button() == ::Qt::RightButton) {
      switch (mState) {
       case PieceOngoing:
        // Closing the polygon needs at least one piece.
        if (mCircularPolygonPieces.empty()) break;
        // Allow the user to bend the closing piece as well.
        m_last = true;
        mPress = lP;
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
     case CircleOngoing:
      UpdateCircle(lP);
      rHandled = true;
      break;

     case PieceOngoing:
      mP1 = lP;
      UpdateOngoingPiece();
      rHandled = true;
      break;

     case HandleOngoing:
      if (m_last) {
        mP1 = cvt(mCircularPolygonPieces.front().source());
        m_last_circular = true;
      }
      UpdateHandle(lP);
      UpdateOngoingPiece();
      rHandled = true;
      break;

     case PieceEnded:
      mState = PieceOngoing;
      mP1 = lP;
      UpdateOngoingPiece();
      rHandled = true;
      break;

     default: break; //! \todo handle default case
    }
    return rHandled;
  }

  /*! handles a mouse release. In the circle mode, a release sets the radius. Otherwise, a left release completes the
   * piece under construction, and a right release closes the polygon.
   * \return true if the event is handled.
   */
  bool mouseReleaseEvent(QGraphicsSceneMouseEvent* aEvent) {
    bool rHandled = false;
    Point lP = cvt(aEvent->scenePos());
    if (aEvent->button() == ::Qt::LeftButton) {
      switch (mState) {
       case CircleOngoing:
        mCircleGI->hide();
        if (squared_distance(mCenter, lP) >= 4) GenerateCircle(Circle(mCenter, squared_distance(mCenter, lP)));
        mState = Start;
        rHandled = true;
        break;

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
       case HandleOngoing:
        if (m_last_circular) {
          HideHandle();
          CommitOngoingPiece(lP);
        }
        m_bound_rect = false;
        CommitCurrCircularPolygon();
        ReStart();
        rHandled = true;
        break;

       default: break; //! \todo handle default case
      }
    }
    return rHandled;
  }

  /*! handles a key press. Delete and Backspace remove the last piece; Escape discards the polygon.
   * \return true if the event is handled.
   */
  bool keyPressEvent(QKeyEvent* aEvent) {
    bool rHandled = false;
    if ((aEvent->key() == ::Qt::Key_Delete) || (aEvent->key() == ::Qt::Key_Backspace)) {
      RemoveLastPiece();
      mState = (mCircularPolygonPieces.size() > 0) ? PieceEnded : Start;
      rHandled = true;
    }
    else if (aEvent->key() == ::Qt::Key_Escape) {
      Reset();
      mState = Start;
      rHandled = true;
    }
    return rHandled;
  }

public:
  /*! obtains the piece under construction, or null if there is none.
   */
  const Circular_curve* ongoing_piece() const { return (mOngoingPieceCtr.size() == 1) ? &mOngoingPieceCtr[0] : NULL; }

  /*! starts a new polygon.
   */
  void ReStart() {
    mH = std::optional<Point>();
    mState = Start;
  }

  /*! discards the polygon under construction.
   */
  void Reset() {
    mCircularPolygonPieces.clear();
    mOngoingPieceCtr.clear();
    mCircularGI->modelChanged();
    mOngoingPieceGI->modelChanged();
    mCircleGI->hide();
    HideHandle();
    ReStart();
  }

  /*! hides the handle.
   */
  void HideHandle() { mHandleGI->hide(); }

  /*! creates the piece from the last vertex to the current point: an arc through the cursor if it is set, and a
   * segment otherwise.
   */
  Circular_curve CreatePiece() {
    if (mH && ! collinear(mP0, *mH, mP1)) return Circular_curve(mP0, *mH, mP1);
    return Circular_curve(mP0, mP1);
  }

  /*! removes the last piece.
   */
  void RemoveLastPiece() {
    mCircularPolygonPieces.pop_back();
    mOngoingPieceCtr.clear();
    mCircularGI->modelChanged();
    mOngoingPieceGI->modelChanged();
    if (mCircularPolygonPieces.size() > 0) {
      mP0 = cvt(mCircularPolygonPieces.back().target());
      UpdateOngoingPiece();
    }
    mH = std::optional<Point>();
  }

  /*! updates the piece under construction.
   */
  void UpdateOngoingPiece() {
    if (mOngoingPieceCtr.size() > 0) mOngoingPieceCtr.clear();
    mOngoingPieceCtr.push_back(CreatePiece());
    mOngoingPieceGI->modelChanged();
  }

  /*! adds the piece under construction to the polygon, and starts the next piece.
   */
  void CommitOngoingPiece(const Point& aP) {
    if (! ongoing_piece()) return;
    mCircularPolygonPieces.push_back(*ongoing_piece());
    mCircularGI->modelChanged();
    mOngoingPieceCtr.clear();
    mOngoingPieceGI->modelChanged();
    mP0 = mP1;
    mP1 = aP;
    mH = std::optional<Point>();
  }

  /*! updates the point the arc passes through. Once the cursor is dragged away from where the button was pressed, the
   * piece becomes an arc through the cursor; otherwise it stays a segment.
   */
  void UpdateHandle(const Point& aP) {
    if (squared_distance(mPress, aP) >= 4) mH = aP;
    else mH = std::optional<Point>();
  }

  /*! updates the circle under construction, whose boundary passes through a given point.
   */
  void UpdateCircle(const Point& aP) {
    double cx = to_double(mCenter.x());
    double cy = to_double(mCenter.y());
    double r = std::sqrt(to_double(squared_distance(mCenter, aP)));
    // Drawn like the ongoing piece (the pen of the Draw/Insert bucket).
    mCircleGI->setPen(mOngoingPieceGI->pen());
    mCircleGI->setRect(cx - r, cy - r, 2 * r, 2 * r);
    mCircleGI->show();
  }

  /*! emits the polygon bounded by a given circle (counterclockwise).
   */
  void GenerateCircle(const Circle& aCircle) {
    Gps_traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    std::vector<std::variant<Circular_X_monotone_curve, Arc_point>> x_objs;
    make_x_monotone(Circular_curve(aCircle), std::back_inserter(x_objs));
    std::vector<Circular_X_monotone_curve> xcvs;
    for (const auto& obj : x_objs) {
      if (auto* xcv = std::get_if<Circular_X_monotone_curve>(&obj)) xcvs.push_back(*xcv);
    }
    Circular_polygon cp(xcvs.begin(), xcvs.end());
    emit(generate(std::variant<Circular_polygon>(cp)));
  }

  /*! rounds a point of a curve to double precision.
   */
  Point cvt(const typename Circular_curve::Point_2& aP) { return Point(to_double(aP.x()), to_double(aP.y())); }

  /*! completes the polygon under construction.
   */
  void CommitCurrCircularPolygon() {
    GenerateCircularPolygon();
    mOngoingPieceCtr.clear();
    mOngoingPieceGI->modelChanged();
    mCircularPolygonPieces.clear();
    mCircularGI->modelChanged();
    mH = std::optional<Point>();
    HideHandle();
  }

  /*! closes the polygon with a segment from its last vertex to its first (unless the closing piece is an arc), and
   * emits it.
   */
  void GenerateCircularPolygon() {
    if (mCircularPolygonPieces.size() == 0) return;
    Gps_traits traits;
    auto make_x_monotone = traits.make_x_monotone_2_object();
    using Make_x_monotone_result = std::variant<Circular_X_monotone_curve, Arc_point>;

    std::vector<Circular_X_monotone_curve> xcvs;
    for (auto it = mCircularPolygonPieces.begin(); it != mCircularPolygonPieces.end(); ++it) {
      std::vector<Make_x_monotone_result> x_objs;
      make_x_monotone(*it, std::back_inserter(x_objs));

      auto* xcv = std::get_if<Circular_X_monotone_curve>(&x_objs[0]);
      if ((*xcv).is_linear() && mCircularPolygonPieces.size() == 1) return;
      for (std::size_t i = 0; i < x_objs.size(); ++i) {
        auto* xcv = std::get_if<Circular_X_monotone_curve>(&x_objs[i]);
        CGAL_assertion(xcv != nullptr);
        xcvs.push_back(*xcv);
      }
    }
    if (xcvs.size() == 0) return;

    if (! m_last_circular) {
      const Arc_point& first_point = xcvs.front().source();
      const Arc_point& last_point = xcvs.back().target();
      CGAL_assertion(! first_point.x().is_extended() && ! first_point.y().is_extended());
      CGAL_assertion(! last_point.x().is_extended() && ! last_point.y().is_extended());
      FT fxs = first_point.x().alpha();
      FT fys = first_point.y().alpha();
      FT lxs = last_point.x().alpha();
      FT lys = last_point.y().alpha();
      xcvs.push_back(Circular_X_monotone_curve(Point(lxs, lys), Point(fxs, fys)));
    }

    m_last = false;
    m_last_circular = false;

    Circular_polygon cp(xcvs.begin(), xcvs.end());
    emit(generate(std::variant<Circular_polygon>(cp)));
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

    CommitCurrCircularPolygon();
    ReStart();
  }

  /*! determines whether the last polygon is the bounding polygon.
   */
  bool isboundingRect() { return m_bound_rect; }

public:
  QGraphicsScene* mScene;
  GI* mCircularGI;
  GI* mOngoingPieceGI;
  QGraphicsLineItem* mHandleGI;
  QGraphicsEllipseItem* mCircleGI;

  QPen mCircularPolygonPen;
  QPen mOngoingCurvePen;
  QPen mHandlePen;

  bool m_bound_rect;
  bool m_last_circular;
  bool m_last;

  Circular_curve_vector mCircularPolygonPieces;
  Circular_curve_vector mOngoingPieceCtr;

  int mState;
  Mode mMode = Arcs;

  Point mP0;
  Point mP1;
  Point mPress;   // where the button was pressed to bend the piece
  Point mCenter;  // the center of the circle under construction

  std::optional<Point> mH;
};

} // namespace Qt
} // namespace CGAL

#endif
