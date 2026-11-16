// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Efi Fogel <efifogel@gmail.com>

#ifndef CGAL_QT6_POLYCURVES_H
#define CGAL_QT6_POLYCURVES_H

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include "Typedefs.h"
#include "Qt6/Polyline_curves.h"
#include "Qt6/Piecewise_set_graphics_item.h"

namespace CGAL {
namespace Qt {

/*! Bounds an x-monotone polycurve, given a functor that bounds its subcurves.
 */
template <typename Subcurve_bbox>
struct Polycurve_X_monotone_bbox {
  /*! obtains the bounding box of an x-monotone polycurve: the union of the boxes of its subcurves.
   */
  template <typename X_monotone_polycurve>
  CGAL::Bbox_2 operator()(const X_monotone_polycurve& a_xcv) const {
    Subcurve_bbox bbox;
    CGAL::Bbox_2 r_bbox;
    for (std::size_t i = 0; i < a_xcv.number_of_subcurves(); ++i)
      r_bbox = (i > 0) ? (r_bbox + bbox(a_xcv[i])) : bbox(a_xcv[i]);
    return r_bbox;
  }
};

/*! Draws an x-monotone polycurve, given a functor that draws its subcurves: its subcurves, in order.
 */
template <typename Draw_subcurve>
struct Draw_polycurve_X_monotone_curve {
  /*! adds an x-monotone polycurve to the path. The first curve (of index 0) starts a new subpath.
   */
  template <typename X_monotone_polycurve, typename Path>
  void operator()(const X_monotone_polycurve& a_xcv, Path& a_path, int a_idx) const {
    Draw_subcurve draw;
    for (std::size_t i = 0; i < a_xcv.number_of_subcurves(); ++i)
      draw(a_xcv[i], a_path, ((a_idx == 0) && (i == 0)) ? 0 : 1);
  }
};

/*! A graphics item that draws a set of polygons bounded by polycurves, given functors that draw and bound their
 * subcurves.
 */
template <typename Polycurve_set, typename Draw_subcurve, typename Subcurve_bbox>
class Polycurve_set_graphics_item :
    public Piecewise_set_graphics_item<Polycurve_set, Draw_polycurve_X_monotone_curve<Draw_subcurve>,
                                       Polycurve_X_monotone_bbox<Subcurve_bbox>> {
  using Base = Piecewise_set_graphics_item<Polycurve_set, Draw_polycurve_X_monotone_curve<Draw_subcurve>,
                                           Polycurve_X_monotone_bbox<Subcurve_bbox>>;

public:
  /*! constructs from the set.
   */
  Polycurve_set_graphics_item(Polycurve_set* a_set) : Base(a_set) {}

protected:
  /*! marks the vertices of the polygons, as those of polylines (see Polyline_set_graphics_item): the endpoints of the
   * x-monotone polycurves are vertices of the polygons, and the other endpoints of their subcurves are vertices of the
   * polycurves.
   */
  void paint_marks(QPainter* a_painter) override {
    if (! this->m_set) return;
    using Polygon = typename Polycurve_set::Polygon_2;
    using X_monotone_polycurve = typename Polygon::X_monotone_curve_2;
    using Point = std::decay_t<decltype(std::declval<X_monotone_polycurve>()[0].source())>;
    std::vector<typename Polycurve_set::Polygon_with_holes_2> pwhs;
    this->m_set->polygons_with_holes(std::back_inserter(pwhs));
    std::vector<std::pair<Point, bool>> vertices;
    auto add = [&vertices](const Polygon& a_boundary) {
      for (auto cit = a_boundary.curves_begin(); cit != a_boundary.curves_end(); ++cit) {
        for (std::size_t i = 0; i < cit->number_of_subcurves(); ++i) vertices.emplace_back((*cit)[i].source(), i == 0);
      }
    };
    for (const auto& pwh : pwhs) {
      if (! pwh.is_unbounded()) add(pwh.outer_boundary());
      for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) add(*hit);
    }
    paint_polyline_vertices(a_painter, vertices, this->m_pen.color());
  }

  double marks_extent() const override { return polyline_marks_extent; }
};

} // namespace Qt
} // namespace CGAL

#endif
