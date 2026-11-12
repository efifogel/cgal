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

#ifndef CGAL_QT6_BOUNDARY_PIECES_GRAPHICS_ITEM_H
#define CGAL_QT6_BOUNDARY_PIECES_GRAPHICS_ITEM_H

#include <iostream>

#include "Qt6/Piecewise_graphics_item_base.h"

using namespace std;

namespace CGAL {
namespace Qt {

/*! A graphics item that draws a sequence of curves (the pieces of a boundary under construction).
 */
template <typename Boundary_pieces_, typename Draw_piece_, typename Piece_bbox_>
class Boundary_pieces_graphics_item : public Piecewise_graphics_item_base {
  // A vector of Gps_traits::Curves_2.
  using Boundary_pieces = Boundary_pieces_;

  using Draw_piece = Draw_piece_;
  using Piece_bbox = Piece_bbox_;

public:
  /*! constructs from the pieces, a functor that draws a piece, and a functor that bounds a piece.
   */
  Boundary_pieces_graphics_item(Boundary_pieces* aBoundary, const Draw_piece& aPieceDrawer = Draw_piece(),
                                const Piece_bbox& aPieceBBox = Piece_bbox()) :
    m_boundary(aBoundary),
    m_piece_drawer(aPieceDrawer),
    m_piece_BBox(aPieceBBox)
  {}

  /*! determines whether there are no pieces.
   */
  virtual bool is_model_empty() const { return ! m_boundary || (m_boundary->size() == 0); }

protected:
  /*! extends the bounding box with the boxes of the pieces.
   */
  virtual void update_bbox(Bbox_builder& aBboxBuilder) {
    if (! m_boundary) return;
    for (auto pit = m_boundary->begin(); pit != m_boundary->end(); ++pit) aBboxBuilder.add(m_piece_BBox(*pit));
  }

  /*! adds the pieces to the path.
   */
  virtual void draw_model(QPainterPath& aPath) {
    if (! m_boundary) return;
    int c = 0;
    for (auto pit = m_boundary->begin(); pit != m_boundary->end(); ++pit, ++c) m_piece_drawer(*pit, aPath, c);
  }

protected:
  Boundary_pieces* m_boundary;          // a vector of Gps_traits::Curves_2
  Draw_piece m_piece_drawer;
  Piece_bbox m_piece_BBox;
};

} // namespace Qt
} // namespace CGAL

#endif
