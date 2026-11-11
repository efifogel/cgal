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

#ifndef CGAL_QT6_BOUNDARY_PIECES_GRAPHICS_ITEM_BEZIER_H
#define CGAL_QT6_BOUNDARY_PIECES_GRAPHICS_ITEM_BEZIER_H

#include "Qt6/PiecewiseGraphicsItemBase.h"

namespace CGAL {
namespace Qt {

/*! A graphics item that draws a sequence of curves (the pieces of a boundary under construction).
 */
template <class Boundary_pieces_, class Draw_piece_, class Piece_bbox_>
class Boundary_pieces_graphics_item_bezier : public Piecewise_graphics_item_base_bezier
{
  using Boundary_pieces = Boundary_pieces_;
  using Draw_piece = Draw_piece_;
  using Piece_bbox = Piece_bbox_;

public:
  /*! constructs from the pieces, a functor that draws a piece, and a functor that bounds a piece.
   */
  Boundary_pieces_graphics_item_bezier(Boundary_pieces* aBoundary, const Draw_piece& aPieceDrawer = Draw_piece(),
                                       const Piece_bbox& aPieceBBox = Piece_bbox()) :
    mBoundary(aBoundary),
    mPieceDrawer(aPieceDrawer),
    mPieceBBox(aPieceBBox)
  {}

  /*! determines whether there are no pieces.
   */
  virtual bool isModelEmpty() const { return ! mBoundary || (mBoundary->size() == 0); }

protected:
  /*! extends the bounding box with the boxes of the pieces.
   */
  virtual void update_bbox(Bbox_builder& aBboxBuilder) {
    if (! mBoundary) return;
    for (auto pit = mBoundary->begin(); pit != mBoundary->end(); ++pit)
      aBboxBuilder.add(mPieceBBox(*pit));
  }

  /*! adds the pieces to the path.
   */
  virtual void draw_model(QPainterPath& aPath) {
    if (! mBoundary) return;
    int c = 0;
    for (auto pit = mBoundary->begin(); pit != mBoundary->end(); ++pit, ++c)
      mPieceDrawer(*pit, aPath, c);
  }

protected:
  Boundary_pieces* mBoundary;
  Draw_piece mPieceDrawer;
  Piece_bbox mPieceBBox;
};

} // namespace Qt
} // namespace CGAL

#endif
