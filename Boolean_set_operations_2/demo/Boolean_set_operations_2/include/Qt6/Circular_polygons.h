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

#ifndef CGAL_QT6_CIRCULAR_POLYGONS_H
#define CGAL_QT6_CIRCULAR_POLYGONS_H

#include <iostream>

#include <CGAL/Arr_circle_segment_traits_2.h>

#include "Typedefs.h"
#include "Qt6/Piecewise_set_graphics_item.h"
#include "Qt6/Boundary_pieces_graphics_item.h"

using namespace std;

namespace CGAL {
namespace Qt {

/*! Bounds an x-monotone circle segment (a circular arc or a line segment).
 */
struct Circular_X_monotone_bbox {
  /*! obtains the bounding box of an x-monotone circle segment.
   */
  template <class X_monotone_circle_segment_2>
  CGAL::Bbox_2 operator()(const X_monotone_circle_segment_2& aC) const { return aC.bbox(); }
};

/*! Bounds a circle segment (a circular arc or a line segment).
 */
struct Circular_bbox
{
  /*! obtains a bounding box of a circle segment. The box of an arc is the box of its supporting circle.
   */
  template <class Circle_segment_2>
  CGAL::Bbox_2 operator()(const Circle_segment_2& aC) const {
    if (aC.is_circular()) return aC.supporting_circle().bbox();
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

/*! Draws an x-monotone circle segment.
 */
struct Draw_circular_X_monotone_curve {
  /*! adds an x-monotone circle segment to the path. The first curve (of index 0) starts a new subpath.
   */
  template <typename X_monotone_circle_segment_2, typename Path>
  void operator()(const X_monotone_circle_segment_2& curve, Path& aPath, int aIdx) const {
    using Converter = Qt::Converter<Kernel>;
    Converter convert;

    if (! curve.is_circular()) {
      Circular_Linear_point lS(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
      Circular_Linear_point lT(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
      if (aIdx == 0) aPath.moveTo(convert(lS));
      else aPath.lineTo(convert(lS));
      aPath.lineTo(convert(lT));
      return;
    }

    const auto& circ = curve.supporting_circle();
    const auto& center = circ.center();
    const auto& source = curve.source();
    const auto& target = curve.target();

    double sx = to_double(source.x());
    double sy = to_double(source.y());
    double tx = to_double(target.x());
    double ty = to_double(target.y());
    double cx = to_double(center.x());
    double cy = to_double(center.y());

    bool degenerate = (sx == tx) && (sy == ty);
    if (degenerate) {
      if (aIdx == 0) aPath.moveTo(sx, sy);
      aPath.lineTo(sx, sy);
      return;
    }

    double sdy = sy - cy;
    double sdx = sx - cx;
    double tdy = ty - cy;
    double tdx = tx - cx;

    double asource = std::atan2(sdy, sdx);
    double atarget = std::atan2(tdy, tdx);

    if (asource < 0.0) asource += 2 * CGAL_PI;
    if (atarget <= 0.0) atarget += 2 * CGAL_PI;
    if (atarget < asource) atarget += 2 * CGAL_PI;

    double aspan = atarget - asource;
    const double to_deg = 180.0 / CGAL_PI;
    Orientation lO = curve.orientation();

    if (lO == CLOCKWISE) aspan = 2.0 * CGAL_PI - aspan;

    if (aIdx == 0) aPath.moveTo(sx, sy);
    else aPath.lineTo(sx, sy);

    QRectF bbox = convert(circ.bbox());
    double dasource = std::atan2(-sdy, sdx) * to_deg;
    double daspan = aspan * to_deg * (lO == COUNTERCLOCKWISE ? -1.0 : +1.0);

    // Prevent the approximations from turning a tiny arc into a full circle by inverting the relative ordering of the
    // source and target angles. (An x-monotone arc never spans an angle greater than pi.)
    if (daspan < 270) aPath.arcTo(bbox, dasource, daspan);
    else {
      if (aIdx == 0) aPath.moveTo(sx, sy);
      aPath.lineTo(sx, sy);
    }
  }
};

/*! Draws a circle segment.
 */
struct Draw_circular_curve {
  /*! adds a circle segment to the path. The first curve (of index 0) starts a new subpath.
   */
  template <typename Circle_segment_2, typename Path>
  void operator()(const Circle_segment_2& curve, Path& aPath, int aIdx) const {
    using Converter = Qt::Converter<Kernel>;
    Converter convert;

    if (! curve.is_circular()) {
      Circular_Linear_point lS(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
      Circular_Linear_point lT(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
      if (aIdx == 0) aPath.moveTo(convert(lS));
      else aPath.lineTo(convert(lS));
      aPath.lineTo(convert(lT));
      return;
    }

    const auto& circ = curve.supporting_circle();
    const auto& center = circ.center();
    const auto& source = curve.source();
    const auto& target = curve.target();

    double sx = to_double(source.x());
    double sy = to_double(source.y());
    double tx = to_double(target.x());
    double ty = to_double(target.y());
    double cx = to_double(center.x());
    double cy = to_double(center.y());

    bool degenerate = (sx == tx) && (sy == ty);
    if (degenerate) return;

    double sdy = sy - cy;
    double sdx = sx - cx;
    double tdy = ty - cy;
    double tdx = tx - cx;

    double asource = std::atan2(sdy, sdx);
    double atarget = std::atan2(tdy, tdx);

    if (asource < 0.0) asource += 2 * CGAL_PI;
    if (atarget <= 0.0) atarget += 2 * CGAL_PI;
    if (atarget < asource) atarget += 2 * CGAL_PI;

    double aspan = atarget - asource;
    const double to_deg = 180.0 / CGAL_PI;
    Orientation lO = curve.orientation();

    if (lO == CLOCKWISE) aspan = 2.0 * CGAL_PI - aspan;

    if (aIdx == 0) aPath.moveTo(sx, sy);
    else aPath.lineTo(sx, sy);

    QRectF bbox = convert(circ.bbox());
    double dasource = std::atan2(-sdy, sdx) * to_deg;
    double daspan = aspan * to_deg * (lO == COUNTERCLOCKWISE ? -1.0 : +1.0);

    aPath.arcTo(bbox, dasource, daspan);
  }
};

/*! A graphics item that draws the circle segments of a boundary under construction.
 */
template <typename Circular_boundary_pieces>
class Circular_boundary_pieces_graphics_item :
    public Boundary_pieces_graphics_item<Circular_boundary_pieces, Draw_circular_curve, Circular_bbox>
{
  using Base = Boundary_pieces_graphics_item<Circular_boundary_pieces, Draw_circular_curve, Circular_bbox>;

public:
  /*! constructs from the circle segments.
   */
  Circular_boundary_pieces_graphics_item(Circular_boundary_pieces* aPieces) : Base(aPieces) {}
};

/*! A graphics item that draws a set of circular polygons.
 */
template <typename Circular_set>
class Circular_set_graphics_item :
    public Piecewise_set_graphics_item<Circular_set, Draw_circular_X_monotone_curve, Circular_X_monotone_bbox> {
  using Base = Piecewise_set_graphics_item<Circular_set, Draw_circular_X_monotone_curve, Circular_X_monotone_bbox>;

public:
  /*! constructs from the set.
   */
  Circular_set_graphics_item(Circular_set* aSet) : Base(aSet) {}
};

} // namespace Qt
} // namespace CGAL

#endif
