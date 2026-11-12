// Copyright (c) 2009  GeometryFactory Sarl (France).
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
//             Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>
//             Kabir Kedia <kabirkedia0111@gmail.com>

#ifndef CGAL_QT6_BEZIER_CURVES_H
#define CGAL_QT6_BEZIER_CURVES_H

#include "Typedefs.h"

#include <CGAL/value_type_traits.h>
#include <CGAL/Arr_Bezier_curve_traits_2.h>
#include <CGAL/Qt/Converter.h>

#include "Qt6/Boundary_pieces_graphics_item.h"
#include "Qt6/Piecewise_set_graphics_item.h"

#define USE_CLIPPING

namespace CGAL {
namespace Qt {

/*! Utilities for approximating Bezier curves.
 */
struct Bezier_helper {
  /*! converts a point to a point of another type with double coordinates.
   */
  template <class Out_point, class In_point>
  static Out_point cvtp2(const In_point& p) { return Out_point(to_double(p.x()), to_double(p.y())); }

  /*! refines the bounding box of a Bezier point until it is smaller than a given error, and obtains its center.
   */
  template <class Bezier_point>
  static Point_2<Simple_cartesian<double>> refine_bezier_point(const Bezier_point& aP, double aError = 1e-2) {
    using Bounding_traits = typename Bezier_point::Bounding_traits;
    using NT = typename Bounding_traits::NT;

    NT min_x, min_y, max_x, max_y;
    const NT error(aError);
    bool l_can_refine = true;
    do {
      aP.get_bbox(min_x, min_y, max_x, max_y);
      l_can_refine = ! aP.is_exact() && (CGAL_NTS abs(max_x - min_x) > error || CGAL_NTS abs(max_y - min_y) > error);
      if (l_can_refine) l_can_refine = aP.refine();
    } while (l_can_refine);

    const NT two(2.0);
    double x = to_double((min_x + max_x) / two);
    double y = to_double((min_y + max_y) / two);
    return Point_2<Simple_cartesian<double>>(x, y);
  }

  /*! obtains (an approximation of) the parameter of an endpoint of an x-monotone subcurve of a Bezier curve.
   */
  template <class Bezier_point, class Bezier_curve>
  static typename Bezier_point::Bez_point_bound::NT get_endpoint_parameter(const Bezier_point& aP,
                                                                           const Bezier_curve& aCurve,
                                                                           unsigned int aXID) {
    using NT = typename Bezier_point::Bez_point_bound::NT;

    auto l_org = aP.get_originator(aCurve, aXID);
    refine_bezier_point(aP);
    return (l_org->point_bound().t_min + l_org->point_bound().t_max) / NT(2.0);
  }

  /*! replaces the control points of a Bezier curve with the control points of its part in the parameter range
   * [aS, aT] (de Casteljau subdivision).
   */
  template <class Ctrl_points, class FT>
  static void clip_impl(Ctrl_points& rQ, FT aS, FT aT) {
    const FT zero(0.0);
    const FT one(1.0);
    if (aS < zero && aT > one) return;
    int nc = rQ.size();
    int ncc = nc - 1;
    FT l_alfa = aS;
    FT l_beta = (aT - aS) / (one - aS);
    for (int i = 1; i <= ncc; ++i) {
      for (int j = 0; j < ncc; ++j) rQ[j] = rQ[j] + l_alfa * (rQ[j+1] - rQ[j]);
      for (int j = nc - i; j <= ncc; ++j) rQ[j] = rQ[j-1] + l_beta * (rQ[j] - rQ[j-1]);
    }
  }

  /*! obtains the control points of an x-monotone Bezier curve, directed from its source to its target.
   */
  template <class Bezier_X_monotone_curve, class Ctrl_points>
  static void clip(const Bezier_X_monotone_curve& aXMCurve, Ctrl_points& rQ) {
    using Bezier_curve = typename Bezier_X_monotone_curve::Curve_2;
    using Bound_nt = typename Bezier_X_monotone_curve::Bounding_traits::NT;

    const Bezier_curve& l_bc = aXMCurve.supporting_curve();

    Bound_nt l_s = get_endpoint_parameter(aXMCurve.source(), l_bc, aXMCurve.xid());
    Bound_nt l_t = get_endpoint_parameter(aXMCurve.target(), l_bc, aXMCurve.xid());

    bool l_fwd = l_s <= l_t;

    Bound_nt l_min = l_fwd ? l_s : Bound_nt(1.0) - l_s;
    Bound_nt l_max = l_fwd ? l_t : Bound_nt(1.0) - l_t;

    std::copy(l_bc.control_points_begin(), l_bc.control_points_end(), std::back_inserter(rQ));
    if (! l_fwd) std::reverse(rQ.begin(), rQ.end());

    clip_impl(rQ, l_min, l_max);
  }

  /*! samples the Bezier curve given by its control points in the parameter range [aMin, aMax] recursively, until the
   * pieces are flat enough; outputs the samples after aMin.
   */
  template <class Control_point_in_iterator, class NT, class Control_point_out_iterator>
  static void recursive_subdivision(Control_point_in_iterator aBeginCtrlPts, Control_point_in_iterator aEndCtrlPts,
                                    NT aMin, NT aMid, NT aMax, Control_point_out_iterator aOut, NT aFlatness) {
    using Control_point = typename value_type_traits<Control_point_in_iterator>::type;
    using Sample_point = typename value_type_traits<Control_point_out_iterator>::type;

    Control_point l_min_p = point_on_Bezier_curve_2(aBeginCtrlPts, aEndCtrlPts, aMin);
    Control_point l_mid_p = point_on_Bezier_curve_2(aBeginCtrlPts, aEndCtrlPts, aMid);
    Control_point l_max_p = point_on_Bezier_curve_2(aBeginCtrlPts, aEndCtrlPts, aMax);

    NT l_b = squared_distance(l_min_p, l_max_p);
    NT l_h = squared_distance(l_mid_p, typename Kernel_traits<Control_point>::Kernel::Line_2(l_min_p, l_max_p));

    if (l_h > l_b * aFlatness) {
      recursive_subdivision(aBeginCtrlPts, aEndCtrlPts, aMin, (aMin + aMid) / NT(2.0), aMid, aOut, aFlatness);
      *aOut++ = cvtp2<Sample_point>(l_mid_p);
      recursive_subdivision(aBeginCtrlPts, aEndCtrlPts, aMid, (aMid + aMax) / NT(2.0), aMax, aOut, aFlatness);
    }
    else *aOut++ = cvtp2<Sample_point>(l_max_p);
  }

  /*! samples a Bezier curve in the parameter range [aSourceT, aTargetT].
   */
  template <class Bezier_curve, class NT, class Output_iterator>
  static void sample_curve(const Bezier_curve& aCurve, NT aSourceT, NT aTargetT, Output_iterator aOut, NT aFlatness) {
    bool l_fwd = aSourceT < aTargetT;
    NT l_min_t = l_fwd ? aSourceT : NT(1.0) - aSourceT;
    NT l_max_t = l_fwd ? aTargetT : NT(1.0) - aTargetT;
    recursive_subdivision(aCurve.control_points_begin(), aCurve.control_points_end(), l_min_t,
                          (l_min_t + l_max_t) / NT(2.0), l_max_t, aOut, aFlatness);
  }

  /*! samples a Bezier curve.
   */
  template <class Bezier_curve, class Output_iterator, class NT>
  static void sample_curve(const Bezier_curve& aCurve, Output_iterator aOut, NT aFlatness)
  { sample_curve(aCurve, NT(0.0), NT(1.0), aOut, aFlatness); }

  /*! samples an x-monotone Bezier curve, including its endpoints.
   */
  template <class Bezier_X_monotone_curve, class Output_iterator, class NT>
  static void sample_x_monotone_curve(const Bezier_X_monotone_curve& aXMCurve, bool aFwd, Output_iterator aOut,
                                      NT aFlatness) {
    // For a vertical subcurve, the points corresponding to the parameter extremes might not be at the correct
    // vertical positions, so the endpoints are added explicitly.
    *aOut++ = refine_bezier_point(aXMCurve.source());

    NT l_from_t = get_endpoint_parameter(aFwd ? aXMCurve.source() : aXMCurve.target(), aXMCurve.supporting_curve(),
                                       aXMCurve.xid());
    NT l_to_t = get_endpoint_parameter(aFwd ? aXMCurve.target() : aXMCurve.source(), aXMCurve.supporting_curve(),
                                     aXMCurve.xid());

    sample_curve(aXMCurve.supporting_curve(), l_from_t, l_to_t, aOut, aFlatness);

    *aOut++ = refine_bezier_point(aXMCurve.target());
  }
};

/*! Bounds a Bezier curve.
 */
struct Bezier_bbox
{
  /*! obtains the bounding box of the control points of a Bezier curve.
   */
  template <class Bezier_curve>
  Bbox_2 operator()(const Bezier_curve& aBC) const
  { return CGAL::bbox_2(aBC.control_points_begin(), aBC.control_points_end()); }
};

/*! Bounds an x-monotone Bezier curve.
 */
struct Bezier_X_monotone_bbox
{
  /*! obtains the bounding box of the control points of an x-monotone Bezier curve.
   */
  template <class Bezier_X_monotone_curve>
  CGAL::Bbox_2 operator()(const Bezier_X_monotone_curve& aBXMC) const {
    using Rat_point_2 = typename Bezier_X_monotone_curve::Rat_kernel::Point_2;
    std::vector<Rat_point_2> l_q;
    Bezier_helper::clip(aBXMC, l_q);
    return CGAL::bbox_2(l_q.begin(), l_q.end());
  }
};

/*! Draws a Bezier curve.
 */
struct Draw_bezier_curve
{
  /*! adds a Bezier curve to the path. The first curve (of index 0) starts a new subpath.
   */
  template <class Bezier_curve, class Path>
  void operator()(const Bezier_curve& aBC, Path& aPath, int aIdx) const {
    using Bound_nt = typename Bezier_curve::Bounding_traits::NT;
    using Linear_kernel = Simple_cartesian<double>;
    using Linear_point = Point_2<Linear_kernel>;
    using Linear_point_vector = std::vector<Linear_point>;

    auto cp = [&](int i) { return Bezier_helper::cvtp2<QPointF>(aBC.control_point(i)); };
    auto n = aBC.number_of_control_points();
    if ((n >= 2) && (n <= 4)) {
      if (aIdx == 0) aPath.moveTo(cp(0));
      else aPath.lineTo(cp(0));
      if (n == 2) aPath.lineTo(cp(1));
      else if (n == 3) aPath.quadTo(cp(1), cp(2));
      else aPath.cubicTo(cp(1), cp(2), cp(3));
      return;
    }

    Linear_point_vector l_sample;
    Bezier_helper::sample_curve(aBC, std::back_inserter(l_sample), Bound_nt(1e-4));
    for (auto it = l_sample.begin(); it != l_sample.end(); ++it) {
      QPointF l_p = Bezier_helper::cvtp2<QPointF>(*it);
      if (aIdx == 0 && it == l_sample.begin()) aPath.moveTo(l_p);
      else aPath.lineTo(l_p);
    }
  }
};

#ifdef USE_CLIPPING

/*! Draws an x-monotone Bezier curve.
 */
struct Draw_bezier_X_monotone_curve
{
  /*! adds an x-monotone Bezier curve to the path. The first curve (of index 0) starts a new subpath.
   */
  template <class Bezier_X_monotone_curve, class Path>
  void operator()(const Bezier_X_monotone_curve& aBXMC, Path& aPath, int aIdx) const {
    using Bound_nt = typename Bezier_X_monotone_curve::Bounding_traits::NT;
    using Rat_point_2 = typename Bezier_X_monotone_curve::Rat_kernel::Point_2;
    using Rat_point_vector = std::vector<Rat_point_2>;
    using Linear_kernel = Simple_cartesian<double>;
    using Linear_point = Point_2<Linear_kernel>;
    using Linear_point_vector = std::vector<Linear_point>;

    Rat_point_vector l_q;
    Bezier_helper::clip(aBXMC, l_q);

    auto cp = [&](int i) { return Bezier_helper::cvtp2<QPointF>(l_q[i]); };
    auto n = l_q.size();
    if ((n >= 2) && (n <= 4)) {
      if (aIdx == 0) aPath.moveTo(cp(0));
      else aPath.lineTo(cp(0));
      if (n == 2) aPath.lineTo(cp(1));
      else if (n == 3) aPath.quadTo(cp(1), cp(2));
      else aPath.cubicTo(cp(1), cp(2), cp(3));
      return;
    }

    Linear_point_vector l_sample;
    Bezier_helper::sample_x_monotone_curve(aBXMC, true, std::back_inserter(l_sample), Bound_nt(1e-4));
    for (auto it = l_sample.begin(); it != l_sample.end(); ++it) {
      QPointF l_p = Bezier_helper::cvtp2<QPointF>(*it);
      if (aIdx == 0 && it == l_sample.begin()) aPath.moveTo(l_p);
      else aPath.lineTo(l_p);
    }
  }
};

#else

/*! Draws an x-monotone Bezier curve.
 */
struct Draw_bezier_X_monotone_curve
{
  /*! adds the polyline that approximates an x-monotone Bezier curve to the path. The first curve (of index 0) starts
   * a new subpath.
   */
  template <class Bezier_X_monotone_curve, class Path>
  void operator()(const Bezier_X_monotone_curve& aBXMC, Path& aPath, int aIdx) const {
    using Linear_kernel = Simple_cartesian<double>;
    using Linear_point = Point_2<Linear_kernel>;
    using Linear_point_vector = std::vector<Linear_point>;

    Linear_point_vector l_sample;
    Bezier_helper::sample_x_monotone_curve(aBXMC, true, std::back_inserter(l_sample));
    for (auto it = l_sample.begin(); it != l_sample.end(); ++it) {
      QPointF l_p = Bezier_helper::cvtp2<QPointF>(*it);
      if (aIdx == 0 && it == l_sample.begin()) aPath.moveTo(l_p);
      else aPath.lineTo(l_p);
    }
  }
};

#endif

/*! A graphics item that draws the Bezier curves of a boundary under construction.
 */
template <class Bezier_boundary_pieces>
class Bezier_boundary_pieces_graphics_item :
    public Boundary_pieces_graphics_item<Bezier_boundary_pieces, Draw_bezier_curve, Bezier_bbox>
{
  using Base = Boundary_pieces_graphics_item<Bezier_boundary_pieces, Draw_bezier_curve, Bezier_bbox>;

public:
  /*! constructs from the Bezier curves.
   */
  Bezier_boundary_pieces_graphics_item(Bezier_boundary_pieces* aPieces) : Base(aPieces) {}
};

/*! A graphics item that draws the boundary of a Bezier polygon.
 */
template <class Bezier_boundary>
class Bezier_boundary_graphics_item :
    public Piecewise_boundary_graphics_item<Bezier_boundary, Draw_bezier_X_monotone_curve,
                                                   Bezier_X_monotone_bbox>
{
  using Base =
    Piecewise_boundary_graphics_item<Bezier_boundary, Draw_bezier_X_monotone_curve, Bezier_X_monotone_bbox>;

public:
  /*! constructs from the boundary.
   */
  Bezier_boundary_graphics_item(Bezier_boundary* aBoundary) : Base(aBoundary) {}
};

/*! A graphics item that draws a Bezier polygon with holes.
 */
template <class Bezier_region>
class Bezier_region_graphics_item :
    public Piecewise_region_graphics_item<Bezier_region, Draw_bezier_X_monotone_curve, Bezier_X_monotone_bbox>
{
  using Base =
    Piecewise_region_graphics_item<Bezier_region, Draw_bezier_X_monotone_curve, Bezier_X_monotone_bbox>;

public:
  /*! constructs from the polygon with holes.
   */
  Bezier_region_graphics_item(Bezier_region* aRegion) : Base(aRegion) {}
};

/*! A graphics item that draws a set of Bezier polygons.
 */
template <class Bezier_set>
class Bezier_set_graphics_item :
    public Piecewise_set_graphics_item<Bezier_set, Draw_bezier_X_monotone_curve, Bezier_X_monotone_bbox>
{
  using Base = Piecewise_set_graphics_item<Bezier_set, Draw_bezier_X_monotone_curve, Bezier_X_monotone_bbox>;

public:
  /*! constructs from the set.
   */
  Bezier_set_graphics_item(Bezier_set* aSet) : Base(aSet) {}
};

} // namespace Qt
} // namespace CGAL

#endif
