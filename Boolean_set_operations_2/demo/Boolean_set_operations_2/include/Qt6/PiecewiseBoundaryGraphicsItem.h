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

#ifndef CGAL_QT6_PIECEWISE_BOUNDARY_GRAPHICS_ITEM_BEZIER_H
#define CGAL_QT6_PIECEWISE_BOUNDARY_GRAPHICS_ITEM_BEZIER_H

#include <CGAL/iterator.h>

#include "Typedefs.h"
#include "Qt6/PiecewiseGraphicsItemBase.h"

namespace CGAL {
namespace Qt {

/*! A graphics item that draws the boundary of a general polygon.
 */
template <class Piecewise_boundary_, class Draw_piece_, class Piece_bbox_>
class Piecewise_boundary_graphics_item_bezier : public Piecewise_graphics_item_base_bezier
{
  using Piecewise_boundary = Piecewise_boundary_;
  using Draw_piece = Draw_piece_;
  using Piece_bbox = Piece_bbox_;

public:
  /*! constructs from the boundary, a functor that draws a curve, and a functor that bounds a curve.
   */
  Piecewise_boundary_graphics_item_bezier(Piecewise_boundary* aBoundary, const Draw_piece& aPieceDrawer = Draw_piece(),
                                          const Piece_bbox& aPieceBBox = Piece_bbox()) :
    mBoundary(aBoundary),
    mPieceDrawer(aPieceDrawer),
    mPieceBBox(aPieceBBox)
  {}

  /*! determines whether there is no boundary to draw.
   */
  virtual bool isModelEmpty() const { return ! mBoundary || mBoundary->is_empty(); }

protected:
  /*! constructs from a functor that draws a curve and a functor that bounds a curve.
   */
  Piecewise_boundary_graphics_item_bezier(const Draw_piece& aPieceDrawer = Draw_piece(),
                                          const Piece_bbox& aPieceBBox = Piece_bbox()) :
    mBoundary(0),
    mPieceDrawer(aPieceDrawer),
    mPieceBBox(aPieceBBox)
  {}

  /*! extends the bounding box with the box of the boundary.
   */
  virtual void update_bbox(Bbox_builder& aBboxBuilder)
  { if (mBoundary) update_boundary_bbox(*mBoundary, aBboxBuilder); }

  /*! adds the boundary to the path.
   */
  virtual void draw_model(QPainterPath& aPath) { if (mBoundary) draw_boundary(*mBoundary, aPath); }

  /*! extends the bounding box with the boxes of the curves of a boundary.
   */
  void update_boundary_bbox(const Piecewise_boundary& aBoundary, Bbox_builder& aBboxBuilder) {
    for (auto pit = aBoundary.curves_begin(); pit != aBoundary.curves_end(); ++pit)
      aBboxBuilder.add(mPieceBBox(*pit));
  }

  /*! adds the curves of a boundary to the path.
   */
  void draw_boundary(const Piecewise_boundary& aBoundary, QPainterPath& aPath);

protected:
  Piecewise_boundary* mBoundary;
  Draw_piece mPieceDrawer;
  Piece_bbox mPieceBBox;
};

template <class B, class D, class P>
void Piecewise_boundary_graphics_item_bezier<B, D, P>::draw_boundary(const Piecewise_boundary& aBoundary,
                                                                     QPainterPath& aPath) {
  int c = 0;
  for (auto pit = aBoundary.curves_begin(); pit != aBoundary.curves_end(); ++pit, ++c)
    mPieceDrawer(*pit, aPath, c);
}

} // namespace Qt
} // namespace CGAL

#endif
