// Copyright (c) 2007,2008,2009,2010,2011 Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s) : Eric Berberich <eric@mpi-inf.mpg.de>
//             Pavel Emeliyanenko <asm@mpi-sb.mpg.de>

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_FUNCTORS_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_FUNCTORS_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*! \file include/CGAL/Curved_kernel_via_analysis_2/Curved_kernel_via_analysis_2_functors.h
 * defines Curved_kernel_via_analysis_2 function objects + class
 */

#include <cstddef>
#include <iostream>
#include <iterator>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/Arr_enums.h>
#include <CGAL/assertions.h>
#include <CGAL/enum.h>
#include <CGAL/Fraction_traits.h>
#include <CGAL/Object.h>
#include <CGAL/Polynomial.h>
#include <CGAL/Polynomial_traits_d.h>
#include <CGAL/Curved_kernel_via_analysis_2/Make_x_monotone_2.h>

namespace CGAL {

namespace internal {

#ifndef CERR
//#define CKvA_DEBUG_PRINT_CERR
#ifdef CKvA_DEBUG_PRINT_CERR
#define CERR(x) std::cerr << x
#else
#define CERR(x) static_cast<void>(0)
#endif
#endif

/*! obtains the asymptotic value of the arc with the given arc number of a curve near the left or right boundary:
 * either the y-coordinate of the horizontal asymptote of the arc, or \c ARR_BOTTOM_BOUNDARY or \c ARR_TOP_BOUNDARY if
 * the arc goes to minus or plus infinity in y. The curve analysis wraps this value in a \c CGAL::Object; this is the
 * only place where it is unwrapped.
 */
template <typename CurveAnalysis_2>
std::variant<CGAL::Arr_parameter_space, typename CurveAnalysis_2::Algebraic_real_1>
asymptotic_value(const CurveAnalysis_2& curve, CGAL::Arr_parameter_space loc,
                 typename CurveAnalysis_2::size_type arcno) {
  auto obj = curve.asymptotic_value_of_arc(loc, arcno);
  if (const auto* ps = CGAL::object_cast<CGAL::Arr_parameter_space>(&obj)) return *ps;
  const auto* y = CGAL::object_cast<typename CurveAnalysis_2::Algebraic_real_1>(&obj);
  CGAL_assertion(y != nullptr);
  return *y;
}

/*! collects main functors for Curved_kernel_via_analysis_2 */
namespace Curved_kernel_via_analysis_2_Functors {

/*! base functor class for inheritance */
template <typename CurvedKernelViaAnalysis_2>
class Curved_kernel_via_analysis_2_functor_base {
public:
  //!\name Public types
  //!@{

  //! this instance's template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the point type
  using Point_2 = typename Curved_kernel_via_analysis_2::Point_2;

  //! the arc type
  using Arc_2 = typename Curved_kernel_via_analysis_2::Arc_2;

  //! type of curve kernel
  using Curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;

  //! type of curve analaysis
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! the x-coordinate type
  using Coordinate_1 = typename Point_2::Coordinate_1;

  // using Y_coordinate_1 = typename Point_2::Y_coordinate_1;

  //!@}

public:
  //!\name Constructors
  //!@{

  /*! constructs base functor
   *
   * \param kernel The kernel instance
   */
  Curved_kernel_via_analysis_2_functor_base(Curved_kernel_via_analysis_2* kernel) : m_curved_kernel(kernel)
  { CGAL_precondition(kernel != nullptr); }

  //!@}

protected:
  //!\name Access members
  //!@{

  /*! returns a pointer to the curved kernel
   * \return Pointer to stored kernel
   */
  Curved_kernel_via_analysis_2* _ckva() const { return m_curved_kernel; }

  /*! obtains the curve kernel of the curved kernel
   */
  Curve_kernel_2& curve_kernel() const { return _ckva()->kernel(); }

  //!@}

protected:
  //!\name Data members
  //!@{

  //! stores pointer to \c Curved_kernel_via_analysis_2
  Curved_kernel_via_analysis_2* m_curved_kernel;

  //!@}
};

#define CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES                                                                            \
  using Point_2 = typename Base::Point_2;                                                                              \
  using Arc_2 = typename Base::Arc_2;                                                                                  \
  using Curve_analysis_2 = typename Base::Curve_analysis_2;                                                            \
  using Coordinate_1 = typename Base::Coordinate_1;                                                                    \
  // using Y_coordinate_1 = typename Base::Y_coordinate_1;

/*! functor to construct a point on a curve */
template <typename CurvedKernelViaAnalysis_2>
class Construct_point_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = Point_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Construct_point_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! constructs an interior point
   *
   * \param x x-coordinate
   * \param c The supporting curve
   * \param arcno Arcnumber on curve
   * \return The constructed point
   */
  Point_2 operator()(const Coordinate_1& x, const Curve_analysis_2& c, int arcno) {
    Point_2 pt(x, c, arcno);
    return pt;
  }

  Point_2 operator()(const Coordinate_1& x, const Arc_2& a) {
    CGAL_precondition(a.is_in_x_range(x));
    Point_2 pt(x, a.curve(), a.arcno(x));
    return pt;
  }

  template <typename T>
  Point_2 operator()(const T& x, const T& y) {
    Point_2 pt(x, y);
    return pt;
  }
};

/*! functor to construct point on an arc */
template <typename CurvedKernelViaAnalysis_2>
class Construct_point_on_arc_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = Point_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Construct_point_on_arc_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! constructs point on an arc
   *
   * \param x x-coordinate of point
   * \param c The supporting curve
   * \param arcno Arcnumber on curve
   * \param arc Can be used to query meta data
   * \return The constructed point
   */
  Point_2 operator()(const Coordinate_1& x, const Curve_analysis_2& c, int arcno, const Arc_2& arc) {
    // avoid compiler warning
    (void)arc;

    //CGAL::IO::set_pretty_mode(std::cerr);
    CERR("Construct_pt_on_arc: " << CGAL::to_double(x) << ", " << arcno << ", " << c.id() << "\narc = " << arc << "\n");

    //CGAL_assertion(c.id() == arc.curve().id());
    //CGAL_assertion(arcno == arc.arcno(x));

    Point_2 pt = this->_ckva()->construct_point_2_object()(x, c, arcno);

    // here we can modify the point wrt "data stored in arc",
    // if we want to
    return pt;
  }
};

/*! functor to construct an x-monotone arc */
template <typename CurvedKernelViaAnalysis_2>
class Construct_arc_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = Arc_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Construct_arc_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  //!\name Constructing non-vertical arcs
  //!@{

  /*! constructs a non-vertical arc with two interior end-points (segment).
   *
   * \param p first endpoint
   * \param q second endpoint
   * \param c The supporting curve
   * \param arcno The arcnumber wrt \c c in the interior of the arc
   * \param arcno_p The arcnumber wrt \c c of the arc at \c p
   * \param arcno_q The arcnumber wrt \c c of the arc at \c q
   * \returns The constructed segment
   *
   * \pre p.x() != q.x()
   */
  Arc_2 operator()(const Point_2& p, const Point_2& q, const Curve_analysis_2& c, int arcno, int arcno_p, int arcno_q) {
    Arc_2 arc(p, q, c, arcno, arcno_p, arcno_q);
    return arc;
  }

  /*! constructs a non-vertical arc with one interior end-point and whose
   * other end approaches the left or right boundary of the parameter space
   * (ray I)
   *
   * \param origin The interior end-point of the ray
   * \param inf_end Defining whether the arc emanates from the left or right
   *        boundary
   * \param c The supporting curve
   * \param arcno The arcnumber wrt \c c in the interior of the arc
   * \param arcno_o The arcnumber wrt \c c of the arc at \c origin
   * \return The constructed ray
   */
  Arc_2 operator()(const Point_2& origin, CGAL::Arr_curve_end inf_end, const Curve_analysis_2& c, int arcno,
                   int arcno_o) {
    Arc_2 arc(origin, inf_end, c, arcno, arcno_o);
    return arc;
  }

  /*! constructs a non-vertical arc with one interior end-point and whose
   * other end approaches a vertical asymptote (ray II)
   *
   * \param origin The interior end-point
   * \param asympt_x The x-coordinate of the vertical asymptote
   * \param inf_end Arc is approaching the bottom or top boundary
   * \param c The supporting curve
   * \param arcno The arcnumber wrt \c c in the interior of the arc
   * \param arcno_o The arcnumber wrt \c c of the arc at \c origin
   * \return The constructed ray
   *
   * \pre origin.x() != asympt_x
   */
  Arc_2 operator()(const Point_2& origin, const Coordinate_1& asympt_x, CGAL::Arr_curve_end inf_end,
                   const Curve_analysis_2& c, int arcno, int arcno_o) {
    Arc_2 arc(origin, asympt_x, inf_end, c, arcno, arcno_o);
    return arc;
  }

  /*! constructs a non-vertical arc with two non-interior ends at
   * the left and right boundary (branch I)
   *
   * \param c The supporting curve
   * \param arcno The arcnumber wrt to \c c in the interior of the arc
   * \return The constructed branch
   */
  Arc_2 operator()(const Curve_analysis_2& c, int arcno) {
    Arc_2 arc(c, arcno);
    return arc;
  }

  /*! constructs a non-vertical arc with two ends approaching vertical
   * asymptotes (branch II).
   *
   * \param asympt_x1 The x-coordinate of the first asymptote
   * \param inf_end1 Arc is approaching the bottom or top boundary at
   *                 \c asympt_x1
   * \param asympt_x2 The x-coordinate of the second asymptote
   * \param inf_end2 Arc is approaching the bottom or top boundary at
   *                 \c asympt_x2
   * \return The constructed branch
   *
   * \pre asympt_x1 != asympt_x2
   */
  Arc_2 operator()(const Coordinate_1& asympt_x1, CGAL::Arr_curve_end inf_end1, const Coordinate_1& asympt_x2,
                   CGAL::Arr_curve_end inf_end2, const Curve_analysis_2& c, int arcno) {
    Arc_2 arc(asympt_x1, inf_end1, asympt_x2, inf_end2, c, arcno);
    return arc;
  }

  /*! constructs a non-vertical arc with one left- or right-boundary end
   * and one end that approaches a vertical asymptote (branch III)
   *
   * \param inf_endx Defining whether the arc emanates from the left or right
   *        boundary
   * \param asympt_x The x-coordinate of the asymptote
   * \param inf_endy Arc is approaching the bottom or top boundary at
   *                 asympt_x
   * \return The constructed branch
   */
  Arc_2 operator()(CGAL::Arr_curve_end inf_endx, const Coordinate_1& asympt_x, CGAL::Arr_curve_end inf_endy,
                   const Curve_analysis_2& c, int arcno) {
    Arc_2 arc(inf_endx, asympt_x, inf_endy, c, arcno);
    return arc;
  }

  //!@}

  //!\name Constructing vertical arcs
  //!@{

  /*! constructs a vertical arc with two interior end-points
   * (vertical segment)
   *
   * \param p The first end-point
   * \param q The second end-point
   * \param c The supporting curve
   * \return The constructed arc
   *
   * \pre p != q && p.x() == q.x()
   * \pre c must have a vertical component at this x
   */
  Arc_2 operator()(const Point_2& p, const Point_2& q, const Curve_analysis_2& c) {
    Arc_2 arc(p, q, c);
    return arc;
  }

  /*! constructs a vertical arc with one interior end-point and
   * one that reaches the bottom or top boundary (vertical ray)
   *
   * \param origin The interior end-point
   * \param inf_end Ray emanates from bottom or top boundary
   * \return The constructed ray
   *
   * \pre c must have a vertical line component at this x
   */
  Arc_2 operator()(const Point_2& origin, CGAL::Arr_curve_end inf_end, const Curve_analysis_2& c) {
    Arc_2 arc(origin, inf_end, c);
    return arc;
  }

  /*! constructs a vertical arc that connects bottom with top boundary
   * (vertical branch)
   *
   * \param x The x-coordinate of the branch
   * \return The constructed branch
   *
   * \pre c must have a vertical line component at this x
   */
  Arc_2 operator()(const Coordinate_1& x, const Curve_analysis_2& c) {
    Arc_2 arc(x, c);
    return arc;
  }

  //!@}
};

/*! functor that checks whether a given arc is vertical */
template <typename CurvedKernelViaAnalysis_2>
class Is_vertical_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = bool;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Is_vertical_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! checks whether the given x-monotone arc \c cv is a vertical segment.
   *
   * \param cv The curve.
   * \return \c true if the curve is a vertical segment, \c false otherwise.
   */
  result_type operator()(const Arc_2& cv) const { return cv.is_vertical(); }
};

/*! functor constructing minimum point of an arc (if interior) */
template <typename CurvedKernelViaAnalysis_2>
class Construct_min_vertex_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = Point_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Construct_min_vertex_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! obtains the minimum end-point of the arc
   *
   * \param cv The arc.
   * \return The minimum end-point.
   *
   * \pre minimum end-point is interior
   */
  result_type operator()(const Arc_2& cv) const { return cv.curve_end(CGAL::ARR_MIN_END); }
};

/*! functor constructing maximum point of an arc (if interior) */
template <typename CurvedKernelViaAnalysis_2>
class Construct_max_vertex_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = Point_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Construct_max_vertex_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! obtains the maximum end-point of the arc.
   *
   * \param cv The arc.
   * \return The maximum end-point.
   *
   * \pre maximum end-point is interior
   */
  result_type operator()(const Arc_2& cv) const { return cv.curve_end(CGAL::ARR_MAX_END); }
};

/*! functor constructing an interior point of on an arc. */
template <typename CurvedKernelViaAnalysis_2>
class Construct_interior_vertex_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = Point_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Construct_interior_vertex_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! obtains an interior point on the arc.
   *
   * \param arc The arc.
   * \return A point on the interior of the arc.
   */
  result_type operator()(const Arc_2& arc) const {
    Point_2 p = compute_interior_vertex(arc);
    CGAL_postcondition(this->_ckva()->is_on_2_object()(p, arc));
    return p;
  }

private:
  result_type compute_interior_vertex(const Arc_2& arc) const {
    using Curve_analysis_2 = typename Curved_kernel_via_analysis_2::Curve_2;

    using Algebraic_curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;
    using Bound = typename Algebraic_curve_kernel_2::Bound;
    using Coordinate_1 = typename Algebraic_curve_kernel_2::Coordinate_1;

    using Poly_rat_1 = CGAL::Polynomial<Bound>;
    using Poly_rat_2 = CGAL::Polynomial<Poly_rat_1>;

    using PT_rat_1 = CGAL::Polynomial_traits_d<Poly_rat_1>;
    using PT_rat_2 = CGAL::Polynomial_traits_d<Poly_rat_2>;

    using FT_2 = CGAL::Fraction_traits<Poly_rat_2>;

    if (! arc.is_vertical()) {
      Bound x_coord = arc.boundary_in_x_range_interior();
      int arcno = arc.arcno();
      const Curve_analysis_2& ca = arc.curve();
      Point_2 p = this->_ckva()->construct_point_on_arc_2_object()(Coordinate_1(x_coord), ca, arcno, arc);
      return p;
    }

    Bound y_coord = 0;
    if (arc.is_finite(ARR_MIN_END)) {
      if (arc.is_finite(ARR_MAX_END)) {
        // We need torefine the interval because there is a chance that
        // the low of the upper point is below the high of the lower point.
        y_coord = this->curve_kernel().bound_between_y_2_object()(arc.curve_end(ARR_MIN_END).xy(),
                                                                     arc.curve_end(ARR_MAX_END).xy());
      }
      else {
        std::pair<Bound, Bound> approx_pair =
          this->curve_kernel().approximate_relative_y_2_object()(arc.curve_end(ARR_MIN_END).xy(), 4);
        y_coord = approx_pair.second + Bound(1);
      }
    }
    else {
      if (arc.is_finite(ARR_MAX_END)) {
        std::pair<Bound, Bound> approx_pair =
          this->curve_kernel().approximate_relative_y_2_object()(arc.curve_end(ARR_MAX_END).xy(), 4);
        y_coord = approx_pair.first - Bound(1);
      }
    }

    /*! \todo Try to remove this polynomial stuff */
    typename PT_rat_1::Construct_polynomial cp1;
    Poly_rat_2 poly2 = typename PT_rat_2::Construct_polynomial()(cp1(-y_coord), cp1(Bound(1)));

    typename FT_2::Denominator_type dummy;
    typename FT_2::Numerator_type curve_poly;
    typename FT_2::Decompose()(poly2, curve_poly, dummy);

    Curve_analysis_2 curve = this->curve_kernel().construct_curve_2_object()(curve_poly);
    Point_2 p = this->_ckva()->construct_point_on_arc_2_object()(arc.x(), curve, 0, arc);
    return p;
  }
};

/*! functor that compares x-coordinates of two interior points */
template <typename CurvedKernelViaAnalysis_2>
class Compare_x_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Compare_x_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! compares the x-coordinates of two points.
   *
   * \param p1 The first point.
   * \param p2 The second point.
   * \return CGAL::LARGER if x(p1)> x(p2);
   *         CGAL::SMALLER if x(p1) \< x(p2);
   *         CGAL::EQUAL if x(p1) = x(p2).
   */
  result_type operator()(const Point_2& p1, const Point_2& p2) const
  { return this->curve_kernel().compare_1_object()(p1.x(), p2.x()); }
};

/*! functor that compares coordinates of two interior points lexicographically */
template <typename CurvedKernelViaAnalysis_2>
class Compare_xy_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Compare_xy_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! compares two points lexigoraphically: by x, then by y.
   * \param p1 The first point.
   * \param p2 The second point.
   * \return CGAL::LARGER if x(p1)> x(p2),
   *                      or if x(p1) = x(p2) and y(p1)> y(p2);
   *         CGAL::SMALLER if x(p1) \< x(p2),
   *                       or if x(p1) = x(p2) and y(p1) \< y(p2);
   *         CGAL::EQUAL if the two points are equal.
   */
  result_type operator()(const Point_2& p1, const Point_2& p2, bool equal_x = false) const {
    // TODO add CGAL_precondition(p1.location() == CGAL::ARR_INTERIOR);
    // TODO add CGAL_precondition(p2.location() == CGAL::ARR_INTERIOR);

    CERR("\ncompare_xy; p1: " << p1 << ";\n p2:" << p2 << "");

    if (p1.id() == p2.id()) {
      result_type res = CGAL::EQUAL;
      CERR("result: " << res << "\n");
      return res;
    }

    result_type res = (this->curve_kernel().compare_xy_2_object()(p1.xy(), p2.xy(), equal_x));

    CERR("result: " << res << "\n");
    return res;
  }
};

/*! functor that computes relative vertical alignment of an interior point and
 * an arc
 */
template <typename CurvedKernelViaAnalysis_2>
class Compare_y_at_x_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Compare_y_at_x_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! returns the relative vertical alignment of a point with an arc
   *
   * \param p The point
   * \param cv The arc
   * \return CGAL::SMALLER if y(p) \< cv(x(p)), i.e.,
   *                       the point is below the arc;
   *         CGAL::LARGER if y(p)> cv(x(p)), i.e.,
   *                      the point is above the arc;
   *         CGAL::EQUAL if p lies on the arc
   *
   * \pre p is in the x-range of cv.
   *
   */
  result_type operator()(const Point_2& p, const Arc_2& cv) const {
    CGAL_precondition(p.is_finite());

    CERR("\ncompare_y_at_x; p: " << p << ";\n cv:" << cv << "\n");

    bool eq_min;
    bool eq_max;
    CGAL_assertion_code(bool in_x_range =) cv.is_in_x_range(p.x(), &eq_min, &eq_max);
    CGAL_assertion(in_x_range);

    auto cmp_xy = this->_ckva()->compare_xy_2_object();

    if (cv.is_vertical()) {
      if (cv.is_finite(CGAL::ARR_MIN_END)) {
        // for vertical arcs we can ask for .xy() member
        if (cmp_xy(p, cv._minpoint(), true) == CGAL::SMALLER) {
          CERR("cmp result: " << CGAL::SMALLER << "\n");
          return CGAL::SMALLER;
        }
      }

      if (cv.is_finite(CGAL::ARR_MAX_END)) {
        if (cmp_xy(p, cv._maxpoint(), true) == CGAL::LARGER) {
          CERR("cmp result: " << CGAL::LARGER << "\n");
          return CGAL::LARGER;
        }
      }
      CERR("cmp result: " << CGAL::EQUAL << "\n");
      return CGAL::EQUAL; // p lies on a vertical arc
    }
    CGAL::Comparison_result res;
    if (eq_min) {
      if (cv._minpoint().is_finite()) res = cmp_xy(p, cv._minpoint(), true);
      else res = (cv.location(CGAL::ARR_MIN_END) == ARR_TOP_BOUNDARY) ? CGAL::SMALLER : CGAL::LARGER;
    }
    else if (eq_max) {
      CGAL_precondition(p.is_finite());
      if (cv._maxpoint().is_finite()) res = cmp_xy(p, cv._maxpoint(), true);
      else res = (cv.location(CGAL::ARR_MAX_END) == ARR_TOP_BOUNDARY) ? CGAL::SMALLER : CGAL::LARGER;
    }
    else {
      Point_2 point_on_s = this->_ckva()->construct_point_on_arc_2_object()(p.x(), cv.curve(), cv.arcno(), cv);
      CGAL_precondition(p.is_finite());
      CGAL_precondition(point_on_s.is_finite());
      res = cmp_xy(p, point_on_s, true);
    }
    CERR("cmp result: " << res << "\n");
    return res;
  }
};

/*! functor that computes the relative vertical alignment of two arcs left of a point
 */
template <typename CurvedKernelViaAnalysis_2>
class Compare_y_at_x_left_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Compare_y_at_x_left_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! compares the relative vertical alignment of two arcs
   * immediately to the left of one of their intersection points.
   *
   * If one of the arcs is vertical (emanating downward from p), it is
   * always considered to be below the other curve.
   *
   * \param cv1 The first arc
   * \param cv2 The second arc
   * \param p The intersection point.
   * \return The relative vertical alignment of cv1 with respect to cv2
   *         immediately to the left of p: CGAL::SMALLER, CGAL::LARGER or
   CGAL::EQUAL.
   *
   * \pre The point p lies on both curves, and both of them must be also be
   *      defined (lexicographically) to its left.
   */
  result_type operator()(const Arc_2& cv1, const Arc_2& cv2, const Point_2& p) const {
    CERR("\ncompare_y_at_x_left(cv2); cv1: " << cv1 << "; cv2: " << cv2 << "; p: " << p << "\n");

    // ensure that p lies on both arcs
    CGAL_precondition(cv1.compare_y_at_x(p) == CGAL::EQUAL);
    CGAL_precondition(cv2.compare_y_at_x(p) == CGAL::EQUAL);

    // check whether both arcs indeed lie to the left of p
    CGAL_precondition((cv1.is_vertical() && cv1.location(CGAL::ARR_MIN_END) == CGAL::ARR_BOTTOM_BOUNDARY) ||
                      cv1._same_arc_compare_xy(cv1._minpoint(), p) == CGAL::SMALLER);
    CGAL_precondition((cv2.is_vertical() && cv2.location(CGAL::ARR_MIN_END) == CGAL::ARR_BOTTOM_BOUNDARY) ||
                      cv2._same_arc_compare_xy(cv2._minpoint(), p) == CGAL::SMALLER);
    if (cv1.is_vertical()) {
      // if both are vertical (they overlap), we return EQUAL
      if (cv2.is_vertical()) return CGAL::EQUAL;
      // a vertical arc is always smaller than the arc extending to the
      // left
      return CGAL::SMALLER;
    }
    // a vertical arc is always smaller than the arc extending to the left;
    // due to the order, we have to return the opposite
    if (cv2.is_vertical()) return CGAL::LARGER;

    // Check singularity in y
    if (cv1.is_singular()) CGAL_error_msg("Handling singularity in y is not yet implemented");

    // vertical line immediately to the left of p: if p lies on boundary
    // get the vertical line over the last interval; otherwise
    // obtain the interval w.r.t. point's x-coordinate (this also valid
    // for discontinuity in y)
    /* if(bndp_x == CGAL::BEFORE_SINGULARITY || bndp_x == CGAL::BEFORE_DISCONTINUITY)
     *   return _compare_arc_numbers(cv2, bndp_x);
     * else
     */

    CGAL::Comparison_result res = cv1._compare_arc_numbers(cv2, CGAL::ARR_INTERIOR, p.x(), CGAL::NEGATIVE);
    CERR("result: " << res << "\n");
    return res;
  }
};

/*! functor that computes the relative vertical alignment of two arcs right of a point
 */
template <typename CurvedKernelViaAnalysis_2>
class Compare_y_at_x_right_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Compare_y_at_x_right_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! compares the relative vertical alignment of two arcs
   * immediately to the right of one of their intersection points.
   *
   * If one of the arcs is vertical (emanating downward from p), it is
   * always considered to be below the other curve.
   *
   * \param cv1 The first arc
   * \param cv2 The second arc
   * \param p The intersection point.
   * \return The relative vertical alignment of cv1 with respect to cv2
   *         immediately to the right of p: CGAL::SMALLER, CGAL::LARGER or
   CGAL::EQUAL.
   *
   * \pre The point p lies on both curves, and both of them must be also be
   *      defined (lexicographically) to its right.
   */
  result_type operator()(const Arc_2& cv1, const Arc_2& cv2, const Point_2& p) const {
    CERR("\ncompare_y_at_x_right; cv1: " << cv1 << "; cv2: " << cv2 << "; p: " << p << "\n");

    // ensure that p lies on both arcs and doesn't lie on the positive
    // boundary
    CGAL_precondition(cv1.compare_y_at_x(p) == CGAL::EQUAL);
    CGAL_precondition(cv2.compare_y_at_x(p) == CGAL::EQUAL);

    // check whether both arcs indeed lie to the left of p
    CGAL_precondition((cv1.is_vertical() && cv1.location(CGAL::ARR_MAX_END) == CGAL::ARR_TOP_BOUNDARY) ||
                      cv1._same_arc_compare_xy(p, cv1._maxpoint()) == CGAL::SMALLER);
    CGAL_precondition((cv2.is_vertical() && cv2.location(CGAL::ARR_MAX_END) == CGAL::ARR_TOP_BOUNDARY) ||
                      cv2._same_arc_compare_xy(p, cv2._maxpoint()) == CGAL::SMALLER);

    if (cv1.is_vertical()) {
      // if both are vertical (they overlap), we return EQUAL
      if (cv2.is_vertical()) return CGAL::EQUAL;

      // a vertical arc is always LARGER than arc extending to the
      // right
      return CGAL::LARGER;
    }
    // a vertical arc is always LARGER than arc extending to the right;
    // due to the order, we have to return the opposite
    if (cv2.is_vertical()) return CGAL::SMALLER;

    if (cv1.is_singular()) { // singularity in y
      CGAL_error_msg("Handling singularity in y is not yet implemented");
    }

    // vertical line immediately to the right of p: if p lies on boundary
    // get the vertical line over the first interval; otherwise
    // obtain the interval w.r.t. point's x-coordinate (this also valid
    // for discontinuity in y)
    /*if(bndp_x == CGAL::AFTER_SINGULARITY ||
      bndp_x == CGAL::AFTER_DISCONTINUITY)
      return _compare_arc_numbers(cv2, bndp_x);
      else*/
    CGAL::Comparison_result res = cv1._compare_arc_numbers(cv2, CGAL::ARR_INTERIOR, p.x(), CGAL::POSITIVE);
    CERR("result: " << res << "\n");
    return res;
  }
};

/*! functor that checks whether a point is in the x-range of an arc */
template <typename CurvedKernelViaAnalysis_2>
class Is_in_x_range_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = bool;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Is_in_x_range_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! checks whether a given point lies within the curve's x-range
   *
   * \param cv The arc
   * \param p the point
   * \return \c true if p lies in arc's x-range; \c false otherwise.
   */
  bool operator()(const Arc_2& cv, const Point_2& p) const { return cv.is_in_x_range(p); }
};

/*! tests two objects, whether they are equal */
template <typename CurvedKernelViaAnalysis_2>
class Equal_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = bool;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Equal_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! checks if the two points are the same
   *
   * \param p1 The first point.
   * \param p2 The second point.
   * \return \c true if the two point are the same; \c false otherwise.
   */
  result_type operator()(const Point_2& p1, const Point_2& p2) const {
    return (this->_ckva()->compare_xy_2_object()(p1, p2) == CGAL::EQUAL);
  }

  /*! checks if the two arcs are the same
   *
   * \param cv1 The first arc
   * \param cv2 The second arc
   * \return \c true if the two curves are the same; \c false otherwise.
   */
  result_type operator()(const Arc_2& cv1, const Arc_2& cv2) const {
    if (cv1.is_identical(cv2)) return true;

    // only one of the arcs is vertical => not equal
    if (cv1.is_vertical() != cv2.is_vertical()) return false;

    Arc_2::simplify(cv1, cv2);
    // distinct supporting curves implies inequality, provided the
    // coprimality condition is satisfied
    if (! cv1.curve().is_identical(cv2.curve())) return false;

    // here either both or none of the arcs are vertical, check for arcnos
    // equality
    if (! cv1.is_vertical() && cv1.arcno() != cv2.arcno()) return false;

    // otherwise compare respective curve ends: supporting curves and
    // arcnos are equal => the curve ends belong to the same arc
    return ((cv1._same_arc_compare_xy(cv1._minpoint(), cv2._minpoint()) == CGAL::EQUAL &&
             cv1._same_arc_compare_xy(cv1._maxpoint(), cv2._maxpoint()) == CGAL::EQUAL));
  }
};

/*! functor that checks whether two arcs overlap */
template <typename CurvedKernelViaAnalysis_2>
class Do_overlap_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = bool;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Do_overlap_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! checks whether two given arcs overlap, i.e., they have infinitely
   * many intersection points
   *
   * \param cv1 The first arc
   * \param cv2 The second arc
   * \return \c true if the curves overlap; \c false otherwise
   */
  bool operator()(const Arc_2& cv1, const Arc_2& cv2) const {
    CERR("\ndo_overlap\n");
    if (cv1.is_identical(cv2)) return true;

    Arc_2::simplify(cv1, cv2);
    // arcs with coprime support do not overlap
    if (! cv1.curve().is_identical(cv2.curve())) return false;

    if (cv1.is_vertical() != cv2.is_vertical()) {
      return false; // only one arc is vertical => can't overlap
    }
    if (cv1.is_vertical()) { // both arcs are vertical
      // check for x-coordinates equality
      if (this->curve_kernel().compare_1_object()(cv1._minpoint().x(), cv2._minpoint().x()) != CGAL::EQUAL)
        return false;

      // compare y-coordinates of min curve ends
      switch (cv1._same_arc_compare_xy(cv1._minpoint(), cv2._minpoint(), true)) {
       case CGAL::EQUAL: return true;  // this->source == cv2->source => overlap !
       case CGAL::SMALLER: // this->source < cv2->source
        // check whether this->target > cv2->source
        return (cv1._same_arc_compare_xy(cv1._maxpoint(), cv2._minpoint(), true) == CGAL::LARGER);

       case CGAL::LARGER: // this->source> cv2->source
        // check whether this->source < cv2->target
        return (cv1._same_arc_compare_xy(cv1._minpoint(), cv2._maxpoint(), true) == CGAL::SMALLER);
      }
    }
    // processing non-vertical arcs
    if (cv1.arcno() != cv2.arcno()) return false;

    /* At this point, we have two non-vertical arcs supported by the same
     * curve with equal arc numbers in their interior. They do overlap if
     * their x-ranges overlap. Compare only x-coordinates */
    switch (cv1._same_arc_compare_xy(cv1._minpoint(), cv2._minpoint(), false, true)) {
     case CGAL::EQUAL: return true;  // this->source == cv2->source => overlap !
     case CGAL::SMALLER: // this->source < cv2->source
      // check whether this->target > cv2->source
      return (cv1._same_arc_compare_xy(cv1._maxpoint(), cv2._minpoint(), false, true) == CGAL::LARGER);
     case CGAL::LARGER: // this->source > cv2->source
      // check whether this->source < cv2->target
      return (cv1._same_arc_compare_xy(cv1._minpoint(), cv2._maxpoint(), false, true) == CGAL::SMALLER);
    }
    CGAL_error_msg("bogus comparison result");
    return false;
  }
};

/*! \class Do_intersect
 * A functor for intersection detection
 */
template <typename CurvedKernelViaAnalysis_2>
class Do_intersect_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  using Multiplicity = std::size_t;
  using Intersection_point = std::pair<Point_2, Multiplicity>;
  using Intersection_result = std::variant<Intersection_point, Arc_2>;

  //! the result type
  using result_type = bool;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Do_intersect_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! determines whether two given \f$x\f$-monotone curves intersect.
   * \param xcv1 the first curve.
   * \param xcv2 the second curve.
   * \return a boolean flag indicating whether the curves intersect.
   * \todo Reimplement without using Intersect_2 to make robust (and efficient) with inexact constructions.
   */
  result_type operator()(const Arc_2& xcv1, const Arc_2& xcv2, bool closed = true) const {
    CERR("\nintersect; cv1: " << xcv1 << ";\n cv2:" << xcv2 << "");
    std::vector<Arc_2> arcs;
    if (xcv1._trim_if_overlapped(xcv2, std::back_inserter(arcs))) {
      arcs.clear();
      return true;
    }

    // process non-overlapping case
    std::vector<Intersection_point> intersections;
    Arc_2::_intersection_points(xcv1, xcv2, std::back_inserter(intersections));
    auto empty = intersections.empty();
    if (closed) {
      intersections.clear();
      return ! empty;
    }

    // Check whether the open curves intersect

    // If the closed curves do not intersect, so do the open curves
    if (empty) return false;

    // If there are more than 2 intersections, return true
    if (intersections.size() > 2) {
      intersections.clear();
      return true;
    }

    auto cmp_xy = this->_ckva()->compare_xy_2_object();

    // checks whether a point is an endpoint of an arc; an arc end at infinity has no point to compare with
    auto is_endpoint = [&](const Arc_2& xcv, const Point_2& q) {
      for (auto ce : {CGAL::ARR_MIN_END, CGAL::ARR_MAX_END})
        if (xcv.is_finite(ce) && (cmp_xy(xcv.curve_end(ce), q) == CGAL::EQUAL)) return true;
      return false;
    };

    // If the first intersection point of the closed curves is not an endpoint of both curves, return true
    const Point_2& p_first = intersections.front().first;
    if (! is_endpoint(xcv1, p_first) || ! is_endpoint(xcv2, p_first)) return true;

    // If there is only one intersection, it is an endpoint; return false
    if (intersections.size() == 1) return false;

    // repeat the above for the last point
    const Point_2& p_last = intersections.back().first;
    return ! is_endpoint(xcv1, p_last) || ! is_endpoint(xcv2, p_last);
  }
};

/*! functor that computes the intersections of two arcs */
template <typename CurvedKernelViaAnalysis_2>
class Intersect_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  using Multiplicity = std::size_t;
  using Intersection_point = std::pair<Point_2, Multiplicity>;
  using Intersection_result = std::variant<Intersection_point, Arc_2>;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Intersect_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  // TODO add operators for non-x-monotone arcs + curves (i.e., all combis)

  /*! finds all intersections of the two given arcs and insert them to the
   * output iterator.
   *
   * The value type of the output iterator is \c Intersection_result, a \c std::variant
   * of an \c Arc_2 object (overlap) and a <tt>std::pair\<Point_2, Multiplicity></tt>,
   * where the multiplicity is that of the zero-dimensional intersection (0 if unknown)
   *
   * \param cv1 The first arc
   * \param cv2 The second arc
   * \param oi The output iterator.
   * \return The past-the-end iterator.
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Arc_2& cv1, const Arc_2& cv2, OutputIterator oi) const {
    CERR("\nintersect; cv1: " << cv1 << ";\n cv2:" << cv2 << "");

    // if arcs overlap, just store their common part, otherwise compute
    // point-wise intersections
    std::vector<Arc_2> arcs;
    if (cv1._trim_if_overlapped(cv2, std::back_inserter(arcs))) {
      for (const auto& item : arcs) *oi++ = item;
      return oi;
    }
    // process non-overlapping case
    std::vector<Intersection_point> vec;
    Arc_2::_intersection_points(cv1, cv2, std::back_inserter(vec));
    for (const auto& item : vec) *oi++ = item;
    return oi;
  }
};

/*! functors that trims an arc */
template <typename CurvedKernelViaAnalysis_2>
class Trim_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = Arc_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Trim_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! returns a trimmed version of an arc
   *
   * \param cv The arc
   * \param p the new first endpoint
   * \param q the new second endpoint
   * \return The trimmed arc
   *
   * \pre p != q
   * \pre both points must be interior and must lie on \c cv
   */
  Arc_2 operator()(const Arc_2& cv, const Point_2& p, const Point_2& q) {
    CERR("trim\n");

    CGAL_precondition(p.location() == CGAL::ARR_INTERIOR);
    CGAL_precondition(q.location() == CGAL::ARR_INTERIOR);

    CGAL_precondition(! this->_ckva()->equal_2_object()(p, q));
    CGAL_precondition(cv.compare_y_at_x(p) == CGAL::EQUAL);
    CGAL_precondition(cv.compare_y_at_x(q) == CGAL::EQUAL);

    return cv._trim(p, q);
  }
};

/*! functor that splits a arc at an interior point */
template <typename CurvedKernelViaAnalysis_2>
class Split_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = void;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Split_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! splits a given arc at a given point into two sub-arcs
   *
   * \param cv The arc to split
   * \param p The split point
   * \param c1 Output: The left resulting subcurve (p is its right endpoint)
   * \param c2 Output: The right resulting subcurve (p is its left endpoint)
   *
   * \pre p lies on cv but is not one of its end-points.
   */
  void operator()(const Arc_2& cv, const Point_2& p, Arc_2& c1, Arc_2& c2) const {
    CGAL_precondition(cv.compare_y_at_x(p) == CGAL::EQUAL);
    // check that p is not an end-point of the arc
    CGAL_precondition(cv._same_arc_compare_xy(cv._minpoint(), p) != CGAL::EQUAL);
    CGAL_precondition(cv._same_arc_compare_xy(cv._maxpoint(), p) != CGAL::EQUAL);

    CERR("\nsplit\n");
    c1 = cv._replace_endpoints(cv._minpoint(), p, -1, (cv.is_vertical() ? -1 : cv.arcno())).first;
    c2 = cv._replace_endpoints(p, cv._maxpoint(), (cv.is_vertical() ? -1 : cv.arcno()), -1).first;
  }
};

/*! functor that computes whether two arcs are mergeable */
template <typename CurvedKernelViaAnalysis_2>
class Are_mergeable_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = bool;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Are_mergeable_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! checks whether two given arcs are mergeable
   *
   * \param cv1 The first arc
   * \param cv2 The second arc
   * \return \c true if the two arcs are mergeable, i.e., they are supported
   * by the same curve and share a common endpoint; \c false otherwise.
   */
  bool operator()(const Arc_2& cv1, const Arc_2& cv2) const {
    CERR("\nare_mergeable\n");

    // if arcs overlap they are not mergeable this also call simplify
    if (cv1.do_overlap(cv2)) return false;

    // touch in at most one point now and supporting curves are simplified
    // both arcs must be either vertical or not
    if (! cv1.curve().is_identical(cv2.curve()) || cv1.is_vertical() != cv2.is_vertical()) return false;

    // if both arcs are non-vertical => they must have equal arcnos
    // to be mergeable
    if (! cv1.is_vertical() && cv1.arcno() != cv2.arcno()) return false;

    // for non-vertical arcs arc numbers are equal => can use same_arc_cmp
    bool max_min = (cv1._same_arc_compare_xy(cv1._maxpoint(), cv2._minpoint()) == CGAL::EQUAL);
    bool min_max = false;
    if (! max_min) { // both cases cannot happen simultaneously
      min_max = (cv1._same_arc_compare_xy(cv1._minpoint(), cv2._maxpoint()) == CGAL::EQUAL);

      // arcs have no common end-point => not mergeable
      if (! min_max) return false;
    }
    // check that the common point is not an event point
    if (cv1.is_vertical()) { // both arcs are vertical
      Point_2 common = (max_min ? cv1._maxpoint() : cv1._minpoint());
      // a common end must be a finite point
      CGAL_precondition(cv1.is_interior(common.location()));
      // check that there are no other non-vertical branches coming
      // through this point

      Curve_analysis_2 ca_2(cv1.curve());
      typename Curve_analysis_2::Status_line_1 cv_line = ca_2.status_line_for_x(common.x());
      CGAL_assertion(cv_line.is_event()); // ??

      // we are not allowed to use number_of_incident_branches()
      // since the common point might be supported by different curve,
      // and therefore its arcno might be not valid for *this arc
      for (int k = 0; k < cv_line.number_of_events(); k++) {
        // use a temporary object for comparison predicate
        typename Point_2::Coordinate_2 tmp(common.x(), cv1.curve(), k);
        if (this->curve_kernel().compare_xy_2_object()(common.xy(), tmp) == CGAL::EQUAL) return false;
      }
    }
    else if (cv1.interval_id() != cv2.interval_id()) return false; // non-vertical case

    return true;
  }
};

/*! functor that merges two arcs */
template <typename CurvedKernelViaAnalysis_2>
class Merge_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = void;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Merge_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! merges two given arcs into a single one
   *
   * \param cv1 The first arc
   * \param cv2 The second arc
   * \param c Output: The resulting arc
   *
   * \pre The two arcs are mergeable, that is they are supported by the
   *      same curve and share a common endpoint.
   */
  void operator()(const Arc_2& cv1, const Arc_2& cv2, Arc_2& c) const {
    CERR("merge\n");
    CGAL_precondition(cv1.are_mergeable(cv2));
    Arc_2::simplify(cv1, cv2);

    Point_2 src;
    Point_2 tgt;
    int arcno_s = -1;
    int arcno_t = -1;
    bool replace_src; // true if cv2 < *this otherwise *this arc < cv2 arc
    // arcs are mergeable => they have one common finite end-point
    replace_src = (cv1._same_arc_compare_xy(cv1._minpoint(), cv2._maxpoint()) == CGAL::EQUAL);
    src = (replace_src ? cv2._minpoint() : cv1._minpoint());
    tgt = (replace_src ? cv1._maxpoint() : cv2._maxpoint());

    if (! cv1.is_vertical()) {
      arcno_s = (replace_src ? cv2.arcno(CGAL::ARR_MIN_END) : cv1.arcno(CGAL::ARR_MIN_END));
      arcno_t = (replace_src ? cv1.arcno(CGAL::ARR_MAX_END) : cv2.arcno(CGAL::ARR_MAX_END));
    }
    Arc_2 arc = cv1._replace_endpoints(src, tgt, arcno_s, arcno_t).first;
    // arc.set_boundaries_after_merge(*this, s); - no need to, since
    // boundaries are stored in Point_2 type and will be copied implicitly

    c = arc;
  }
};

/*! functor that computes whether a point lies on a supporting curve */
template <typename CurvedKernelViaAnalysis_2>
class Is_on_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = bool;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Is_on_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! checks whether \c p lies on \c c
   *
   * \param p The point to test
   * \param c The curve
   * \return \c true if the \c p lies on \c c, \c false otherwise
   */
  result_type operator()(const Point_2& p, const Curve_analysis_2& c) const
  { return (this->curve_kernel().sign_at_2_object()(c, p.xy()) == CGAL::ZERO); }

  /*! checks whether \c p lies on \c cv
   *
   * \param p The point to test
   * \param c The curve arc
   * \return \c true if the \c p lies on \c cv, \c false otherwise
   */
  result_type operator()(const Point_2& p, const Arc_2& cv) const {
    // fix by Michael Hemmer:
    if (cv.is_in_x_range(p.x())) return cv.compare_y_at_x(p) == CGAL::EQUAL;
    return false;

    // This is the old code that seems to interpret the arc as open
    // In particular, it returns false for vertical arcs.
    // (Michael Hemmer)
    //       bool is_left, is_right;
    //       result_type res =
    //         (cv.is_in_x_range(p.x(),&is_left,&is_right)) &&
    //         !(is_left) &&
    //         !(is_right) &&
    //         (cv.compare_y_at_x(p) == CGAL::EQUAL);
    //       return res;
  }
};

/*! functor that decomposes curve into x-monotone arcs and isolated points */
template <typename CurvedKernelViaAnalysis_2>
class Make_x_monotone_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  /*! constructs
   *
   * \param kernel The kernel
   */
  Make_x_monotone_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! decomposes a given arc into list of x-monotone arcs
   * (subcurves) and insert them to the output iterator. Since \c Arc_2
   * is by definition x-monotone, an input arc is passed to the
   * output iterator directly.
   *
   * \param cv The arc
   * \param oi The output iterator, whose value type is \c std::variant<Point_2, Arc_2>
   * The returned objects all hold \c Arc_2 objects
   * \return The past-the-end iterator
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Arc_2& cv, OutputIterator oi) const {
    using Point_2 = typename Curved_kernel_via_analysis_2::Point_2;
    using Arc_2 = typename Curved_kernel_via_analysis_2::Arc_2;
    using Make_x_monotone_result = std::variant<Point_2, Arc_2>;
    *oi++ = Make_x_monotone_result(cv);
    return oi;
  }

  /*! decomposes a given curve into list of x-monotone arcs
   * (subcurves) and isolated points and insert them to the output iterator.
   *
   * \param cv The curve
   * \param oi The output iterator, whose value type is \c std::variant<Point_2, Arc_2>.
   * The returned objects hold either \c Arc_2 or \c Point_2 objects
   * \return The past-the-end iterator
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Curve_analysis_2& cv, OutputIterator oi) const {
    internal::Make_x_monotone_2<Curved_kernel_via_analysis_2> make_x_monotone(this->_ckva());

    return make_x_monotone(cv, oi);
  }

  /*! passes an isolated point to the output iterator as is. This enables the insertion of a range of isolated points
   * into an arrangement, as if they were curves.
   *
   * \param p The point
   * \param oi The output iterator, whose value type is \c std::variant<Point_2, Arc_2>
   * \return The past-the-end iterator
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Point_2& p, OutputIterator oi) const {
    using Make_x_monotone_result = std::variant<Point_2, Arc_2>;
    *oi++ = Make_x_monotone_result(p);
    return oi;
  }
};

// left-right

/*! functor computing parameter space in x for arc */
template <typename CurvedKernelViaAnalysis_2>
class Parameter_space_in_x_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Arr_parameter_space;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Parameter_space_in_x_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! obtains the parameter space in x at the end of an arc.
   *
   * \param cv The arc
   * \param ce the arc end indicator:
   *     ARR_MIN_END - the minimal end of cv
   *     ARR_MAX_END - the maximal end of cv
   * \return the parameter space at the \c ce end of \c cv
   *   ARR_LEFT_BOUNDARY  - the arc approaches the left boundary of the
   *                        parameter space
   *   ARR_INTERIOR       - the arc does not approach a boundary of the
   *                        parameter space
   *   ARR_RIGHT_BOUNDARY - the arc approaches the right boundary of the
   *                        parameter space
   */
  result_type operator()(const Arc_2& cv, CGAL::Arr_curve_end ce) const {
    CGAL::Arr_parameter_space loc = cv.location(ce);
    if (loc == CGAL::ARR_LEFT_BOUNDARY || loc == CGAL::ARR_RIGHT_BOUNDARY) return loc;
    return CGAL::ARR_INTERIOR;
  }

  result_type operator()(const Point_2& pt) const { return pt.location(); }
};

/*! functor that compares ends of arcs near left or right boundary */
template <typename CurvedKernelViaAnalysis_2>
class Compare_y_near_boundary_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Compare_y_near_boundary_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! compares the y-coordinates of 2 arcs at their ends near the left
   * or right boundary of the parameter space
   *
   * \param cv1 the first arc.
   * \param cv2 the second arc.
   * \param ce the arc end indicator.
   * \return CGAL::SMALLER is y(cv1,ce) < y(cv2,ce2) near left/right boundary
   *         CGAL::EQUAL is y(cv1,ce) = y(cv2,ce2) near left/right boundary,
   *         i.e., they overlap
   * \return CGAL::LARGER is y(cv1,ce) > y(cv2,ce2) near left/right boundary
   *
   * \pre the ce ends of the arcs cv1 and cv2 lie either on the left
   * boundary or on the right boundary of the parameter space.
   */
  result_type operator()(const Arc_2& cv1, const Arc_2& cv2, CGAL::Arr_curve_end ce) const {
    CERR("\ncompare_y_near_boundary; cv1: " << cv1 << "; cv2: " << cv2 << "; end: " << ce << "\n");

    CGAL::Comparison_result res = CGAL::EQUAL;

    CGAL::Arr_parameter_space loc1 = cv1.location(ce);
    CGAL_precondition(cv1.is_on_left_right(loc1));
    CGAL_precondition(loc1 == cv2.location(ce));
    // comparing ids is the same as calling is_identical() ??
    if (cv1.id() == cv2.id()) {
      CERR("result: " << res << "\n"); // EQUAL
      return res;
    }

    // both points lie on the left-right-identification, i.e., equalx
    // TODO this interface is NOT official
    using Algebraic_real_1 = typename Curve_analysis_2::Algebraic_real_1;
    auto asym1 = asymptotic_value(cv1.curve(), loc1, cv1.arcno());
    auto asym2 = asymptotic_value(cv2.curve(), loc1, cv2.arcno());
    const auto* ps1 = std::get_if<CGAL::Arr_parameter_space>(&asym1);
    const auto* ps2 = std::get_if<CGAL::Arr_parameter_space>(&asym2);

    if (ps1 && ps2) res = CGAL::EQUAL; // both go to infinity; decided by the arc numbers below
    else if (ps1) res = (*ps1 == CGAL::ARR_BOTTOM_BOUNDARY) ? CGAL::SMALLER : CGAL::LARGER;
    else if (ps2) res = (*ps2 == CGAL::ARR_TOP_BOUNDARY) ? CGAL::SMALLER : CGAL::LARGER;
    else {
      // Remark: Is filtered
      res = this->curve_kernel().compare_1_object()(std::get<Algebraic_real_1>(asym1),
                                                       std::get<Algebraic_real_1>(asym2));
    }

    if (res != EQUAL) {
      CERR("result: " << res << "\n");
      return res;
    }

    CGAL_precondition(cv1.is_on_left_right(loc1));
    CGAL_precondition(loc1 == cv2.location(ce));
    // comparing ids is the same as calling is_identical() ??
    if (cv1.id() == cv2.id()) {
      CGAL::Comparison_result res = CGAL::EQUAL;
      CERR("result: " << res << "\n");
      return res;
    }

    // in this setting same handling as for +/-oo ?
    res = cv1._compare_arc_numbers(cv2, loc1);
    CERR("result: " << res << "\n");
    return res;
  }
};

// bottom-top

template <typename CurvedKernelViaAnalysis_2>
class Parameter_space_in_y_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Arr_parameter_space;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Parameter_space_in_y_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! obtains the parameter space in y at the end of an arc.
   *
   * \param cv The arc
   * \param ce the arc end indicator:
   *     ARR_MIN_END - the minimal end of cv
   *     ARR_MAX_END - the maximal end of cv
   * \return the parameter space at the \c ce end of \c cv
   *   ARR_BOTTOM_BOUNDARY- the arc approaches the bottom boundary of the
   *                        parameter space
   *   ARR_INTERIOR       - the arc does not approach a boundary of the
   *                        parameter space
   *   ARR_TOP_BOUNDARY   - the arc approaches the top boundary of the
   *                        parameter space
   */
  result_type operator()(const Arc_2& cv, CGAL::Arr_curve_end ce) const {
    CGAL::Arr_parameter_space loc = cv.location(ce);
    if (loc == CGAL::ARR_BOTTOM_BOUNDARY || loc == CGAL::ARR_TOP_BOUNDARY) return loc;
    return CGAL::ARR_INTERIOR;
  }

  result_type operator()(const Point_2& pt) const { return pt.location(); }
};

/*! functor that compares x-limits at the top or bottom boundary */
template <typename CurvedKernelViaAnalysis_2>
class Compare_x_on_boundary_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  Compare_x_on_boundary_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! compares the x-limit of a vertical with the x-limit of
   * an arc end near the boundary at bottom or top boundary
   *
   * \param p the point direction.
   * \param cv the arc, the endpoint of which is compared.
   * \param ce the arc-end indicator -
   *            ARR_MIN_END - the minimal end of cv or
   *            ARR_MAX_END - the maximal end of cv.
   * \return the comparison result:
   *         SMALLER - x(p) \< x(cv, ce);
   *         EQUAL   - x(p) = x(cv, ce);
   *         LARGER  - x(p) > x(cv, ce).
   *
   * \pre p lies in the interior of the parameter space.
   * \pre the ce end of the line cv lies on a boundary.
   */
  result_type operator()(const Point_2& p, const Arc_2& cv, CGAL::Arr_curve_end ce) const {
    CERR("\ncompare_x_on_boundary: p: " << p << "\n cv: " << cv << "; curve_end: " << ce << "\n");

    // this curve end has boundary only in y
    CGAL_precondition(cv.is_on_bottom_top(cv.location(ce)));
    if (cv.is_singular()) // the curve end goes to contraction => x-order
      return CGAL::EQUAL; // doesn't matter

    CGAL::Comparison_result res = this->curve_kernel().compare_1_object()(p.x(), cv.curve_end_x(ce));
    CERR("result: " << res << "\n");
    return res;
  }

  /*! compares the x-limits of 2 arcs ends near the top or bottom
   * boundary of the parameter space
   * \param cv1 the first arc.
   * \param ce1 the first arc end indicator -
   *            ARR_MIN_END - the minimal end of cv1 or
   *            ARR_MAX_END - the maximal end of cv1.
   * \param cv2 the second arc.
   * \param ce2 the second arc end indicator -
   *            ARR_MIN_END - the minimal end of cv2 or
   *            ARR_MAX_END - the maximal end of cv2.
   * \return the second comparison result:
   *         SMALLER - x(cv1, ce1) \< x(cv2, ce2);
   *         EQUAL   - x(cv1, ce1) = x(cv2, ce2);
   *         LARGER  - x(cv1, ce1) > x(cv2, ce2).
   *
   * \pre the ce1 end of the arc cv1 lies on a boundary.
   * \pre the ce2 end of the arc cv2 lies on a boundary.
   */
  result_type operator()(const Arc_2& cv1, CGAL::Arr_curve_end ce1, const Arc_2& cv2, CGAL::Arr_curve_end ce2) const {
    CERR("\ncompare_x_on_boundary: cv1: " << cv1 << "\n cv2: " << cv2 << "; end1: " << ce1 << "; end2: " << ce2
                                          << "\n");
    /*CGAL::Arr_boundary_type bnd1 = boundary(end1),
      bnd2 = cv2.boundary(ce2);*/
    CGAL_precondition_code(CGAL::Arr_parameter_space loc1 = cv1.location(ce1));
    CGAL_precondition_code(CGAL::Arr_parameter_space loc2 = cv2.location(ce2));
    CGAL_precondition(cv1.is_on_bottom_top(loc1));
    CGAL_precondition(cv2.is_on_bottom_top(loc2));

    if (cv1.is_singular() != cv2.is_singular()) {
      // only one curve end lies at singularity (another at +/-oo)
      CGAL_error_msg("SINGULARITY + INF comparison is not yet implemented");
    }

    CGAL::Comparison_result res = this->curve_kernel().compare_1_object()(cv1.curve_end_x(ce1), cv2.curve_end_x(ce2));
    CERR("result: " << res << "\n");
    return res;
  }
};

/*! functor that compares x-coordinates near the top or bottom boundary */
template <typename CurvedKernelViaAnalysis_2>
class Compare_x_near_boundary_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  Compare_x_near_boundary_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! compares the x-coordinates of 2 arcs ends near the top or bottom
   * boundary of the parameter space
   * \param cv1 the first arc.
   * \param cv2 the second arc.
   * \param ce the arc end indicator -
   *            ARR_MIN_END - the minimal end of curves or
   *            ARR_MAX_END - the maximal end of curves.
   * \return the second comparison result:
   *         SMALLER - x(cv1, ce) \< x(cv2, ce);
   *         EQUAL   - x(cv1, ce) = x(cv2, ce);
   *         LARGER  - x(cv1, ce) > x(cv2, ce).
   *
   * \pre the ce1 end of the arc cv1 lies on a boundary.
   * \pre the ce2 end of the arc cv2 lies on a boundary.
   * \pre both curve ends are on the same boundary
   */
  result_type operator()(const Arc_2& cv1, const Arc_2& cv2, CGAL::Arr_curve_end ce) const {
    CERR("\ncompare_x_near_boundary: cv1: " << cv1 << "\n cv2: " << cv2 << "; ce: " << ce << "\n");

    CGAL::Arr_parameter_space loc1 = cv1.location(ce);
    CGAL_precondition_code(CGAL::Arr_parameter_space loc2 = cv2.location(ce));
    CGAL_precondition(cv1.is_on_bottom_top(loc1));
    CGAL_precondition(cv2.is_on_bottom_top(loc2));
    CGAL_precondition(cv1.compare_x_on_boundary(ce, cv2, ce) == CGAL::EQUAL);

    CGAL_precondition(loc1 == loc2);

    Coordinate_1 x0(cv1.curve_end_x(ce));
    CGAL::Comparison_result res = cv1._compare_arc_numbers(cv2, CGAL::ARR_INTERIOR, x0,
                                                           (ce == CGAL::ARR_MIN_END ? CGAL::POSITIVE : CGAL::NEGATIVE));
    if ((ce == CGAL::ARR_MAX_END && loc1 == CGAL::ARR_TOP_BOUNDARY) ||
        (ce == CGAL::ARR_MIN_END && loc1 == CGAL::ARR_BOTTOM_BOUNDARY)) {
      res = opposite(res);
    }
    CERR("result: " << res << "\n");
    return res;
  }
};

/*! functor to construct a point on a curve */
template <typename CurvedKernelViaAnalysis_2>
class Compare_endpoints_xy_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Comparison_result;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Compare_endpoints_xy_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! compares endpoints of arc lexicographically
   *
   * \param arc the arc
   * \return The order of endpoints
   */
  CGAL::Comparison_result operator()(const Arc_2& xcv) const {
    return (xcv.is_left_to_right()) ? CGAL::SMALLER : CGAL::LARGER;
    return CGAL::EQUAL;
  }
};

/*! functor to construct a point on a curve */
template <typename CurvedKernelViaAnalysis_2>
class Construct_opposite_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = Arc_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Construct_opposite_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! constructs an arc of opposite direction
   *
   * \param arc the arc to be reversed
   * \return The reversed arc
   */
  Arc_2 operator()(const Arc_2& xcv) const { return xcv.flip(); }
};

/*! functor that computes the x-extreme points of a curve */
template <typename CurvedKernelViaAnalysis_2>
class X_extreme_points_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  using Coordinate_2 = typename Curve_analysis_2::Coordinate_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  X_extreme_points_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*!
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Curve_analysis_2& ca, OutputIterator oi) const {
    using Status_line_1 = typename Curve_analysis_2::Status_line_1;

    int events = ca.number_of_status_lines_with_event();
    for (int i = 0; i < events; i++) {
      Status_line_1 status_line = ca.status_line_at_event(i);
      int lifts = status_line.number_of_events();

      for (int j = 0; j < lifts; j++) {
        std::pair<int, int> arcs = status_line.number_of_incident_branches(j);
        if (arcs.first == 0 || arcs.second == 0) *oi++ = status_line.algebraic_real_2(j);
      }
    }
    return oi;
  }
};

/*! functor that computes the y-extreme points of a curve */
template <typename CurvedKernelViaAnalysis_2>
class Y_extreme_points_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  using Coordinate_2 = typename Curve_analysis_2::Coordinate_2;

  /*! constructs
   *
   * \param kernel The kernel
   */
  Y_extreme_points_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*!
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Curve_analysis_2& ca, OutputIterator oi) const {
    using Status_line_1 = typename Curve_analysis_2::Status_line_1;

    auto& ck = this->curve_kernel();
    Curve_analysis_2 ca_yx = ck.swap_x_and_y_2_object()(ca);
    std::vector<Coordinate_2> y_critical_points;
    this->_ckva()->x_extreme_points_2_object()(ca_yx, std::back_inserter(y_critical_points));
    for (auto it = y_critical_points.begin(); it != y_critical_points.end(); it++) {
      Coordinate_1 curr_x = ck.get_y_2_object()(*it);
      Status_line_1 status_line = ca.status_line_at_exact_x(curr_x);
      int lifts = status_line.number_of_events();
      for (int i = 0; i < lifts; i++) {
        Coordinate_2 lift_xy = status_line.algebraic_real_2(i);
        bool y_coordinate_found;
        while (true) {
          if (ck.upper_boundary_y_2_object()(lift_xy) < ck.lower_boundary_x_2_object()(*it)) {
            y_coordinate_found = false;
            break;
          }
          if (ck.upper_boundary_y_2_object()(lift_xy) >= ck.upper_boundary_x_2_object()(*it)) {
            y_coordinate_found = true;
            break;
          }

          ck.refine_x_2_object()(*it);
        }

        if (y_coordinate_found) {
          *oi++ = Coordinate_2(curr_x, ca, i);
          break;
        }
      }
    }
    return oi;
  }
};

#undef CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

} // namespace Curved_kernel_via_analysis_2_Functors

//! a short name for the namespace of the CKvA functors
namespace CKvA_2_functors = Curved_kernel_via_analysis_2_Functors;

/*! collects the main set of functors of a curved kernel */
template <typename CurvedKernelViaAnalysis_2, typename Dummy = void>
class Curved_kernel_via_analysis_2_functors {
public:
  //!\name Public types
  //!@{

  //! this instance's first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;
  //!@}
  // typedef Curved_kernel_via_analysis_2_functors<CurvedKernelViaAnalysis_2> Functor_base;

private:
  /*! obtains the curved kernel this object is a base of; the functors take a non-const pointer to their kernel,
   * whose caches are mutable.
   */
  Curved_kernel_via_analysis_2* _ckva() const
  { return const_cast<Curved_kernel_via_analysis_2*>(static_cast<const Curved_kernel_via_analysis_2*>(this)); }

public:

  // declares curved kernel functors, for each functor defines a member function
  // returning an instance of this functor bound to this kernel
#define CGAL_CKvA_2_functor_pred(Y, Z)                        \
  using Y = CKvA_2_functors::Y<Curved_kernel_via_analysis_2>; \
  Y Z() const { return Y(_ckva()); }

#define CGAL_CKvA_2_functor_cons(Y, Z) CGAL_CKvA_2_functor_pred(Y, Z)

  CGAL_CKvA_2_functor_pred(Compare_x_2, compare_x_2_object);
  CGAL_CKvA_2_functor_pred(Compare_xy_2, compare_xy_2_object);
  CGAL_CKvA_2_functor_pred(Is_vertical_2, is_vertical_2_object);

  CGAL_CKvA_2_functor_cons(Construct_min_vertex_2, construct_min_vertex_2_object);
  CGAL_CKvA_2_functor_cons(Construct_max_vertex_2, construct_max_vertex_2_object);
  CGAL_CKvA_2_functor_cons(Construct_interior_vertex_2, construct_interior_vertex_2_object);

  CGAL_CKvA_2_functor_pred(Compare_y_at_x_2, compare_y_at_x_2_object);
  CGAL_CKvA_2_functor_pred(Compare_y_at_x_left_2, compare_y_at_x_left_2_object);
  CGAL_CKvA_2_functor_pred(Compare_y_at_x_right_2, compare_y_at_x_right_2_object);
  CGAL_CKvA_2_functor_pred(Equal_2, equal_2_object);
  CGAL_CKvA_2_functor_pred(Is_in_x_range_2, is_in_x_range_2_object);
  CGAL_CKvA_2_functor_pred(Do_overlap_2, do_overlap_2_object);
  CGAL_CKvA_2_functor_cons(Do_intersect_2, do_intersect_2_object);
  CGAL_CKvA_2_functor_cons(Intersect_2, intersect_2_object);
  CGAL_CKvA_2_functor_cons(Trim_2, trim_2_object);
  CGAL_CKvA_2_functor_cons(Split_2, split_2_object);
  CGAL_CKvA_2_functor_pred(Are_mergeable_2, are_mergeable_2_object);
  CGAL_CKvA_2_functor_cons(Merge_2, merge_2_object);

  CGAL_CKvA_2_functor_pred(Is_on_2, is_on_2_object);
  CGAL_CKvA_2_functor_cons(Make_x_monotone_2, make_x_monotone_2_object);

  // left-right
  CGAL_CKvA_2_functor_pred(Parameter_space_in_x_2, parameter_space_in_x_2_object);
  CGAL_CKvA_2_functor_pred(Compare_y_near_boundary_2, compare_y_near_boundary_2_object);

  // bottom-top
  CGAL_CKvA_2_functor_pred(Parameter_space_in_y_2, parameter_space_in_y_2_object);
  CGAL_CKvA_2_functor_pred(Compare_x_on_boundary_2, compare_x_on_boundary_2_object);
  CGAL_CKvA_2_functor_pred(Compare_x_near_boundary_2, compare_x_near_boundary_2_object);

  CGAL_CKvA_2_functor_cons(X_extreme_points_2, x_extreme_points_2_object);
  CGAL_CKvA_2_functor_cons(Y_extreme_points_2, y_extreme_points_2_object);

  CGAL_CKvA_2_functor_cons(Construct_point_2, construct_point_2_object);

  CGAL_CKvA_2_functor_cons(Construct_point_on_arc_2, construct_point_on_arc_2_object);

  CGAL_CKvA_2_functor_cons(Construct_arc_2, construct_arc_2_object);

  CGAL_CKvA_2_functor_pred(Compare_endpoints_xy_2, compare_endpoints_xy_2_object);

  CGAL_CKvA_2_functor_cons(Construct_opposite_2, construct_opposite_2_object);

#undef CGAL_CKvA_2_functor_pred
#undef CGAL_CKvA_2_functor_cons

}; // Curved_kernel_via_analysis_2_functors

} // namespace internal
} // namespace CGAL

#endif
