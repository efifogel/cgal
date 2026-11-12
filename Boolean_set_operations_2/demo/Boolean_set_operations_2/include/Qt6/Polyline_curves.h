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

#ifndef CGAL_QT6_POLYLINE_CURVES_H
#define CGAL_QT6_POLYLINE_CURVES_H

#include <CGAL/Qt/Converter.h>

#include "Typedefs.h"
#include "Qt6/Piecewise_set_graphics_item.h"
#include "Qt6/Boundary_pieces_graphics_item.h"

namespace CGAL {
namespace Qt {

/*! Bounds an x-monotone polyline.
 */
struct Polyline_X_monotone_bbox {
  /*! obtains the bounding box of an x-monotone polyline.
   */
  template <typename X_monotone_polyline_segment_2>
  CGAL::Bbox_2 operator()(const X_monotone_polyline_segment_2& aC) const { return aC.bbox(); }
};

/*! Bounds a polyline.
 */
struct Polyline_bbox {
  /*! obtains the bounding box of a polyline.
   */
  template <typename Polyline_segment_2>
  CGAL::Bbox_2 operator()(const Polyline_segment_2& aC) const {
    auto n = aC.number_of_subcurves();
    Bbox_2 bbox;
    for (std::size_t i = 0; i < n; ++i) bbox = (i > 0) ? (bbox + aC[i].bbox()) : aC[i].bbox();
    return bbox;
  }
};

/*! Draws an x-monotone polyline as the segment between its endpoints.
 */
struct Draw_polyline_X_monotone_curve {
  /*! adds the segment between the endpoints of an x-monotone polyline to the path. The first curve (of index 0)
   * starts a new subpath.
   */
  template <typename X_monotone_polyline_segment_2, typename Path>
  void operator()(const X_monotone_polyline_segment_2& curve, Path& aPath, int aIdx) const {
    using Point = Point_2<Kernel>;
    using Converter = CGAL::Qt::Converter<Kernel>;
    Converter convert;

    Point ps(CGAL::to_double(curve[0].source().x()), CGAL::to_double(curve[0].source().y()));
    Point pt(CGAL::to_double(curve[curve.number_of_subcurves() - 1].target().x()),
             CGAL::to_double(curve[curve.number_of_subcurves() - 1].target().y()));

    if (aIdx == 0) aPath.moveTo(convert(ps));
    else aPath.lineTo(convert(ps));
    aPath.lineTo(convert(pt));
  }
};

/*! Draws a polyline as the segment between its endpoints.
 */
struct Draw_polyline_curve {
  /*! adds the segment between the endpoints of a polyline to the path. The first curve (of index 0) starts a new
   * subpath.
   */
  template <typename Polyline_segment_2, class Path>
  void operator()(const Polyline_segment_2& curve, Path& aPath, int aIdx) const {
    using Point = Point_2<Kernel>;
    using Converter = Qt::Converter<Kernel>;
    Converter convert;

    Point ps(CGAL::to_double(curve[0].source().x()), CGAL::to_double(curve[0].source().y()));
    Point pt(CGAL::to_double(curve[curve.number_of_subcurves() - 1].target().x()),
             CGAL::to_double(curve[curve.number_of_subcurves() - 1].target().y()));

    if (aIdx == 0) aPath.moveTo(convert(ps));
    else aPath.lineTo(convert(ps));
    aPath.lineTo(convert(pt));
  }
};

/*! A graphics item that draws the polylines of a boundary under construction.
 */
template <typename Polyline_boundary_pieces>
class Polyline_boundary_pieces_graphics_item :
    public Boundary_pieces_graphics_item<Polyline_boundary_pieces, Draw_polyline_curve, Polyline_bbox> {
  using Base = Boundary_pieces_graphics_item<Polyline_boundary_pieces, Draw_polyline_curve, Polyline_bbox>;

public:
  /*! constructs from the polylines.
   */
  Polyline_boundary_pieces_graphics_item(Polyline_boundary_pieces* aPieces) : Base(aPieces) {}
};

/*! A graphics item that draws a set of polygons bounded by polylines.
 */
template <typename Polyline_set>
class Polyline_set_graphics_item :
    public Piecewise_set_graphics_item<Polyline_set, Draw_polyline_X_monotone_curve, Polyline_X_monotone_bbox> {
  using Base = Piecewise_set_graphics_item<Polyline_set, Draw_polyline_X_monotone_curve, Polyline_X_monotone_bbox>;

public:
  /*! constructs from the set.
   */
  Polyline_set_graphics_item(Polyline_set* aSet) : Base(aSet) {}
};

} // namespace Qt
} // namespace CGAL

#endif
