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

#ifndef CGAL_QT6_LINEAR_POLYGONS_H
#define CGAL_QT6_LINEAR_POLYGONS_H

#include <CGAL/Qt/Converter.h>

#include "Typedefs.h"
#include "Qt6/Piecewise_set_graphics_item.h"
#include "Qt6/Boundary_pieces_graphics_item.h"

namespace CGAL {
namespace Qt {

/*! Bounds an x-monotone segment.
 */
struct Linear_X_monotone_bbox {
  /*! obtains the bounding box of an x-monotone segment.
   */
  template <typename X_monotone_linear_segment_2>
  CGAL::Bbox_2 operator()(const X_monotone_linear_segment_2& aC) const { return aC.bbox(); }
};

/*! Bounds a segment.
 */
struct Linear_bbox {
  /*! obtains the bounding box of a segment.
   */
  template <class Linear_segment_2>
  CGAL::Bbox_2 operator()(const Linear_segment_2& aC) const {
    double x_min = to_double(aC.source().x());
    double x_max = to_double(aC.target().x());
    double y_min = to_double(aC.source().y());
    double y_max = to_double(aC.target().y());
    if (x_min > x_max) {
      std::swap(x_min, x_max);
      std::swap(y_min, y_max);
    }
    if (y_min > y_max) std::swap(y_min, y_max);
    return Bbox_2(x_min, y_min, x_max, y_max);
  }
};

/*! Draws an x-monotone segment.
 */
struct Draw_linear_X_monotone_curve {
  /*! adds an x-monotone segment to the path. The first curve (of index 0) starts a new subpath.
   */
  template <typename X_monotone_linear_segment_2, typename Path>
  void operator()(const X_monotone_linear_segment_2& curve, Path& aPath, int aIdx) const {
    using Linear_point = Point_2<Kernel>;
    using Converter = CGAL::Qt::Converter<Kernel>;
    Converter convert;

    Linear_point ps(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
    Linear_point pt(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));

    if (aIdx == 0) aPath.moveTo(convert(ps));
    else aPath.lineTo(convert(ps));
    aPath.lineTo(convert(pt));
  }
};

/*! Draws a segment.
 */
struct Draw_linear_curve {
  /*! adds a segment to the path. The first curve (of index 0) starts a new subpath.
   */
  template <typename Linear_segment_2, class Path>
  void operator()(const Linear_segment_2& curve, Path& aPath, int aIdx) const {
    using Linear_point = Point_2<Kernel>;
    using Converter = Qt::Converter<Kernel>;
    Converter convert;

    Linear_point ps(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
    Linear_point pt(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));

    if (aIdx == 0) aPath.moveTo(convert(ps));
    else aPath.lineTo(convert(ps));
    aPath.lineTo(convert(pt));
  }
};

/*! A graphics item that draws the segments of a boundary under construction.
 */
template <typename Linear_boundary_pieces>
class Linear_boundary_pieces_graphics_item :
    public Boundary_pieces_graphics_item<Linear_boundary_pieces, Draw_linear_curve, Linear_bbox> {
  using Base = Boundary_pieces_graphics_item<Linear_boundary_pieces, Draw_linear_curve, Linear_bbox>;

public:
  /*! constructs from the segments.
   */
  Linear_boundary_pieces_graphics_item(Linear_boundary_pieces* aPieces) : Base(aPieces) {}
};

/*! A graphics item that draws a set of linear polygons.
 */
template <class Linear_set>
class Linear_set_graphics_item :
    public Piecewise_set_graphics_item<Linear_set, Draw_linear_X_monotone_curve, Linear_X_monotone_bbox> {
  using Base = Piecewise_set_graphics_item<Linear_set, Draw_linear_X_monotone_curve, Linear_X_monotone_bbox>;

public:
  /*! constructs from the set.
   */
  Linear_set_graphics_item(Linear_set* aSet) : Base(aSet) {}
};

} // namespace Qt
} // namespace CGAL

#endif
