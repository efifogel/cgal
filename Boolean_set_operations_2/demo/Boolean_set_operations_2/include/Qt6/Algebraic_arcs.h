// Copyright (c) 2026  Tel-Aviv University (Israel).
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

#ifndef CGAL_QT6_ALGEBRAIC_ARCS_H
#define CGAL_QT6_ALGEBRAIC_ARCS_H

#include "Typedefs.h"
#include "Algebraic_polygons.h"
#include "Qt6/Piecewise_set_graphics_item.h"

namespace CGAL {
namespace Qt {

/*! Bounds an x-monotone algebraic arc.
 */
struct Algebraic_X_monotone_bbox {
  /*! obtains the bounding box of an x-monotone algebraic arc.
   */
  Bbox_2 operator()(const Algebraic_X_monotone_curve& a_xcv) const { return algebraic_arc_bbox(a_xcv); }
};

/*! Draws an x-monotone algebraic arc as a polyline from its source to its target.
 */
struct Draw_algebraic_X_monotone_curve {
  /*! adds the polyline that approximates an x-monotone algebraic arc to the path. The first curve (of index 0) starts
   * a new subpath.
   */
  template <typename Path>
  void operator()(const Algebraic_X_monotone_curve& a_xcv, Path& a_path, int a_idx) const {
    auto points = algebraic_arc_points(a_xcv);
    for (std::size_t i = 0; i < points.size(); ++i) {
      if ((a_idx == 0) && (i == 0)) a_path.moveTo(points[i]);
      else a_path.lineTo(points[i]);
    }
  }
};

using Algebraic_GI =
  Piecewise_set_graphics_item<Algebraic_polygon_set, Draw_algebraic_X_monotone_curve, Algebraic_X_monotone_bbox>;

} // namespace Qt
} // namespace CGAL

#endif
