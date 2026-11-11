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

#ifndef CGAL_QT6_PIECEWISE_REGION_GRAPHICS_ITEM_H
#define CGAL_QT6_PIECEWISE_REGION_GRAPHICS_ITEM_H

#include "Typedefs.h"
#include "Qt6/Piecewise_boundary_graphics_item.h"

namespace CGAL {
namespace Qt {

/*! A graphics item that draws a general polygon with holes.
 */
template <typename Gps_traits, typename Draw_piece_, typename Piece_bbox_>
class Piecewise_region_graphics_item : public Piecewise_boundary_graphics_item<Gps_traits, Draw_piece_, Piece_bbox_>
{
  using Piecewise_region = typename Gps_traits::Polygon_with_holes_2;
  using Base = Piecewise_boundary_graphics_item<Gps_traits, Draw_piece_, Piece_bbox_>;

  using Draw_piece = Draw_piece_;
  using Piece_bbox = Piece_bbox_;

public:
  /*! determines whether there is no polygon with holes to draw.
   */
  virtual bool isModelEmpty() const { return ! m_region || m_region->outer_boundary().size(); }

protected:
  /*! constructs from a functor that draws a curve and a functor that bounds a curve.
   */
  Piecewise_region_graphics_item(const Draw_piece& aPieceDrawer = Draw_piece(),
                                 const Piece_bbox& aPieceBBox = Piece_bbox()) :
    Base(aPieceDrawer, aPieceBBox)
  {}

  /*! extends the bounding box with the box of the polygon with holes.
   */
  virtual void update_bbox(Piecewise_graphics_item_base::Bbox_builder& aBboxBuilder)
  { if (m_region) update_region_bbox(*m_region, aBboxBuilder); }

  /*! adds the polygon with holes to the path.
   */
  virtual void draw_model(QPainterPath& aPath) { if (m_region) draw_region(*m_region, aPath); }

  /*! extends the bounding box with the boxes of the boundaries of a polygon with holes. An unbounded polygon marks
   * the box as unbounded.
   */
  void update_region_bbox(const Piecewise_region& aRegion, Piecewise_graphics_item_base::Bbox_builder& aBboxBuilder);

  /*! adds the boundaries of a polygon with holes to the path.
   */
  void draw_region(const Piecewise_region& aRegion, QPainterPath& aPath);

protected:
  Piecewise_region* m_region;
  Gps_traits* m_traits;
};

template <typename R, typename D, typename P>
void Piecewise_region_graphics_item<R, D, P>::
update_region_bbox(const Piecewise_region& aRegion, Piecewise_graphics_item_base::Bbox_builder& aBboxBuilder) {
  if (aRegion.is_unbounded()) aBboxBuilder.unbounded = true;
  this->update_boundary_bbox(aRegion.outer_boundary(), aBboxBuilder);
  for (auto hit = aRegion.holes_begin(); hit != aRegion.holes_end(); ++hit)
    this->update_boundary_bbox(*hit, aBboxBuilder);
}

template <typename R, typename D, typename P>
void Piecewise_region_graphics_item<R, D, P>::draw_region(const Piecewise_region& aRegion, QPainterPath& aPath) {
  // An unbounded region (e.g., a complement) has no outer boundary; use a rectangle that covers the view instead, so
  // that the area outside its holes is filled (the path uses the odd-even fill rule).
  if (aRegion.is_unbounded()) aPath.addRect(this->unbounded_rect());
  this->draw_boundary(aRegion.outer_boundary(), aPath);
  for (auto hit = aRegion.holes_begin(); hit != aRegion.holes_end(); ++hit) this->draw_boundary(*hit, aPath);
}

} // namespace Qt
} // namespace CGAL

#endif
