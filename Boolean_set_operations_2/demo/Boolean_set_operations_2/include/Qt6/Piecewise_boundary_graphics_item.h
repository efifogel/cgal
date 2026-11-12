// Copyright (c) 2012,2018  Tel-Aviv University (Israel).
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

#ifndef CGAL_QT6_PIECEWISE_BOUNDARY_GRAPHICS_ITEM_H
#define CGAL_QT6_PIECEWISE_BOUNDARY_GRAPHICS_ITEM_H

#include <CGAL/iterator.h>

#include "Qt6/Piecewise_graphics_item_base.h"

namespace CGAL {
namespace Qt {

/*! A graphics item that draws the boundary of a general polygon.
 */
template <typename Piecewise_boundary_, typename Draw_piece_, typename Piece_bbox_>
class Piecewise_boundary_graphics_item : public Piecewise_graphics_item_base {
  using Piecewise_boundary = Piecewise_boundary_;

  using Draw_piece = Draw_piece_;
  using Piece_bbox = Piece_bbox_;

public:
  /*! constructs from the boundary, a functor that draws a curve, and a functor that bounds a curve.
   */
  Piecewise_boundary_graphics_item(Piecewise_boundary* aBoundary, const Draw_piece& aPieceDrawer = Draw_piece(),
                                   const Piece_bbox& aPieceBBox = Piece_bbox()) :
    m_boundary(aBoundary),
    m_piece_drawer(aPieceDrawer),
    m_piece_BBox(aPieceBBox)
  {}

  /*! determines whether there is no boundary to draw.
   */
  virtual bool is_model_empty() const { return ! m_boundary || m_boundary->is_empty(); }

protected:
  /*! constructs from a functor that draws a curve and a functor that bounds a curve.
   */
  Piecewise_boundary_graphics_item(const Draw_piece& aPieceDrawer = Draw_piece(),
                                   const Piece_bbox& aPieceBBox = Piece_bbox()) :
    m_boundary(0),
    m_piece_drawer(aPieceDrawer),
    m_piece_BBox(aPieceBBox)
  {}

  /*! extends the bounding box with the box of the boundary.
   */
  virtual void update_bbox(Bbox_builder& aBboxBuilder)
  { if (m_boundary) update_boundary_bbox(*m_boundary, aBboxBuilder); }

  /*! adds the boundary to the path.
   */
  virtual void draw_model(QPainterPath& aPath) { if (m_boundary) draw_boundary(*m_boundary, aPath); }

  /*! extends the bounding box with the boxes of the curves of a boundary.
   */
  void update_boundary_bbox(const Piecewise_boundary& aBoundary, Bbox_builder& aBboxBuilder) {
    for (auto pit = aBoundary.curves_begin(); pit != aBoundary.curves_end(); ++pit)
      aBboxBuilder.add(m_piece_BBox(*pit));
  }

  /*! adds the curves of a boundary to the path.
   */
  void draw_boundary(const Piecewise_boundary& aBoundary, QPainterPath& aPath);

protected:
  Piecewise_boundary* m_boundary;
  Draw_piece m_piece_drawer;
  Piece_bbox m_piece_BBox;
};

template <class B, class D, class P>
void Piecewise_boundary_graphics_item<B, D, P>::draw_boundary(const Piecewise_boundary& aBoundary,
                                                              QPainterPath& aPath) {
  int c = 0;
  for (auto pit = aBoundary.curves_begin(); pit != aBoundary.curves_end(); ++pit, ++c)
    m_piece_drawer(*pit, aPath, c);
}

} // namespace Qt
} // namespace CGAL

#endif
