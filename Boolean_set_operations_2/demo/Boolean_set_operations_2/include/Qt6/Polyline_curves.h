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

#include <utility>
#include <vector>

#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QTransform>

#include <CGAL/Qt/Converter.h>

#include "Typedefs.h"
#include "Qt6/Piecewise_set_graphics_item.h"
#include "Qt6/Boundary_pieces_graphics_item.h"
#include "Polyline_boundary.h"

namespace CGAL {
namespace Qt {

/*! Bounds an x-monotone polyline.
 */
struct Polyline_X_monotone_bbox {
  /*! obtains the bounding box of an x-monotone polyline.
   */
  template <typename X_monotone_polyline_segment_2>
  CGAL::Bbox_2 operator()(const X_monotone_polyline_segment_2& a_c) const { return a_c.bbox(); }
};

/*! Bounds a polyline.
 */
struct Polyline_bbox {
  /*! obtains the bounding box of a polyline.
   */
  template <typename Polyline_segment_2>
  CGAL::Bbox_2 operator()(const Polyline_segment_2& a_c) const {
    auto n = a_c.number_of_subcurves();
    Bbox_2 bbox;
    for (std::size_t i = 0; i < n; ++i) bbox = (i > 0) ? (bbox + a_c[i].bbox()) : a_c[i].bbox();
    return bbox;
  }
};

/*! Draws a polyline (x-monotone or not) through its points.
 */
struct Draw_polyline_curve {
  /*! adds a polyline to the path. The first curve (of index 0) starts a new subpath.
   */
  template <typename Polyline, typename Path>
  void operator()(const Polyline& a_curve, Path& a_path, int a_idx) const {
    bool first = true;
    for (auto it = a_curve.points_begin(); it != a_curve.points_end(); ++it) {
      QPointF p(CGAL::to_double(it->x()), CGAL::to_double(it->y()));
      if (first && (a_idx == 0)) a_path.moveTo(p);
      else a_path.lineTo(p);
      first = false;
    }
  }
};

/*! Draws an x-monotone polyline through its points.
 */
using Draw_polyline_X_monotone_curve = Draw_polyline_curve;

/*! paints the marks of the vertices of a polygon bounded by polylines, in a fixed size on the screen: a filled disc at
 * a vertex of the polygon, and a hollow disc at a vertex of a polyline (that is not a vertex of the polygon).
 * \param a_vertices the vertices, each with a flag that tells whether it is a vertex of the polygon.
 */
template <typename Point>
void paint_polyline_vertices(QPainter* a_painter, const std::vector<std::pair<Point, bool>>& a_vertices,
                             const QColor& a_color) {
  if (a_vertices.empty()) return;
  QTransform transform = a_painter->worldTransform();
  a_painter->save();
  a_painter->resetTransform();
  a_painter->setRenderHint(QPainter::Antialiasing);
  QPen pen(a_color, 1.5);
  pen.setCosmetic(true);
  a_painter->setPen(pen);
  for (const auto& [p, polygon_vertex] : a_vertices) {
    QPointF q = transform.map(QPointF(CGAL::to_double(p.x()), CGAL::to_double(p.y())));
    a_painter->setBrush(polygon_vertex ? QBrush(a_color) : QBrush(::Qt::white));
    double r = polygon_vertex ? 3.5 : 2.5;
    a_painter->drawEllipse(q, r, r);
  }
  a_painter->restore();
}

/*! A graphics item that draws the polylines of a boundary under construction.
 */
template <typename Polyline_boundary_pieces>
class Polyline_boundary_pieces_graphics_item :
    public Boundary_pieces_graphics_item<Polyline_boundary_pieces, Draw_polyline_curve, Polyline_bbox> {
  using Base = Boundary_pieces_graphics_item<Polyline_boundary_pieces, Draw_polyline_curve, Polyline_bbox>;

public:
  /*! constructs from the polylines.
   */
  Polyline_boundary_pieces_graphics_item(Polyline_boundary_pieces* a_pieces) : Base(a_pieces) {}

protected:
  /*! marks the vertices of the chain under construction. A vertex where the chain stops being x-monotone will be a
   * vertex of the polygon (see polyline_boundary()); so are, for now, the first and the last vertices.
   */
  void paint_marks(QPainter* a_painter) override {
    if (! this->m_boundary) return;
    using Point = ::Kernel::Point_2;
    std::vector<Point> chain;
    for (const auto& piece : *this->m_boundary) {
      for (auto it = piece.points_begin(); it != piece.points_end(); ++it) {
        if (chain.empty() || (*it != chain.back())) chain.push_back(*it);
      }
    }
    auto ends = polyline_endpoints(chain);
    std::vector<std::pair<Point, bool>> vertices;
    for (std::size_t i = 0; i < chain.size(); ++i) vertices.emplace_back(chain[i], ends[i]);
    paint_polyline_vertices(a_painter, vertices, this->m_pen.color());
  }
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
  Polyline_set_graphics_item(Polyline_set* a_set) : Base(a_set) {}

protected:
  /*! marks the vertices of the polygons: the endpoints of the x-monotone polylines are vertices of the polygons, and
   * the other points of the polylines are vertices of the polylines.
   */
  void paint_marks(QPainter* a_painter) override {
    if (! this->m_set) return;
    using Polygon = typename Polyline_set::Polygon_2;
    using Point = typename Polyline_set::Traits_2::Point_2;
    std::vector<typename Polyline_set::Polygon_with_holes_2> pwhs;
    this->m_set->polygons_with_holes(std::back_inserter(pwhs));
    std::vector<std::pair<Point, bool>> vertices;
    auto add = [&vertices](const Polygon& a_boundary) {
      for (auto cit = a_boundary.curves_begin(); cit != a_boundary.curves_end(); ++cit) {
        bool first = true;
        for (auto it = cit->points_begin(); std::next(it) != cit->points_end(); ++it, first = false)
          vertices.emplace_back(*it, first);
      }
    };
    for (const auto& pwh : pwhs) {
      if (! pwh.is_unbounded()) add(pwh.outer_boundary());
      for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) add(*hit);
    }
    paint_polyline_vertices(a_painter, vertices, this->m_pen.color());
  }
};

} // namespace Qt
} // namespace CGAL

#endif
