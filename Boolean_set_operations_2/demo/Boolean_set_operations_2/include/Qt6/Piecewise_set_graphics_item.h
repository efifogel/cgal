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

#ifndef CGAL_QT6_PIECEWISE_SET_GRAPHICS_ITEM_H
#define CGAL_QT6_PIECEWISE_SET_GRAPHICS_ITEM_H

#include "Qt6/Piecewise_region_graphics_item.h"

namespace CGAL {
namespace Qt {

/*! A graphics item that draws a general polygon set.
 */
template <typename Piecewise_set_, typename Draw_piece_, typename Piece_bbox_>
class Piecewise_set_graphics_item :
    public Piecewise_region_graphics_item<typename Piecewise_set_::Polygon_with_holes_2, Draw_piece_, Piece_bbox_> {
  using Piecewise_set = Piecewise_set_;
  using Draw_piece = Draw_piece_;
  using Piece_bbox = Piece_bbox_;

  using Region = typename Piecewise_set::Polygon_with_holes_2;
  using Base = Piecewise_region_graphics_item<Region, Draw_piece, Piece_bbox>;
  using Region_vector = std::vector<Region>;

public:
  /*! constructs from the set, a functor that draws a curve, and a functor that bounds a curve.
   */
  Piecewise_set_graphics_item(Piecewise_set* a_set, const Draw_piece& a_piece_drawer = Draw_piece(),
                              const Piece_bbox& a_piece_bbox = Piece_bbox()) :
    Base(a_piece_drawer, a_piece_bbox),
    m_set(a_set)
  {}

  /*! determines whether the set is empty.
   */
  virtual bool is_model_empty() const { return ! m_set || m_set->is_empty(); }

protected:
  /*! extends the bounding box with the box of the set.
   */
  virtual void update_bbox(Piecewise_graphics_item_base::Bbox_builder& a_bbox_builder)
  { if (m_set) update_set_bbox(*m_set, a_bbox_builder); }

  /*! adds the set to the path.
   */
  virtual void draw_model(QPainterPath& a_path) { if (m_set) draw_set(*m_set, a_path); }

  /*! extends the bounding box with the boxes of the polygons with holes of a set.
   */
  void update_set_bbox(const Piecewise_set& a_set, Piecewise_graphics_item_base::Bbox_builder& a_bbox_builder);

  /*! adds the polygons with holes of a set to the path.
   */
  void draw_set(const Piecewise_set& a_set, QPainterPath& a_path);

protected:
  Piecewise_set* m_set;
};

template <typename S, typename D, typename P>
void Piecewise_set_graphics_item<S, D, P>::
update_set_bbox(const Piecewise_set& a_set, Piecewise_graphics_item_base::Bbox_builder& a_bbox_builder) {
  Region_vector vec;
  a_set.polygons_with_holes(std::back_inserter(vec));
  for (auto rit = vec.begin(); rit != vec.end(); ++rit) this->update_region_bbox(*rit, a_bbox_builder);
}

template <typename S, typename D, typename P>
void Piecewise_set_graphics_item<S, D, P>::draw_set(const Piecewise_set& a_set, QPainterPath& a_path) {
  Region_vector vec;
  a_set.polygons_with_holes(std::back_inserter(vec));
  for (auto rit = vec.begin(); rit != vec.end(); ++rit) this->draw_region(*rit, a_path);
}

} // namespace Qt
} // namespace CGAL

#endif
