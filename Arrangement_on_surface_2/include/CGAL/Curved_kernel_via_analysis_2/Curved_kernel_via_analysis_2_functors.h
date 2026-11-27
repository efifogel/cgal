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

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/Arr_enums.h>
#include <CGAL/assertions.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/enum.h>
#include <CGAL/Fraction_traits.h>
#include <CGAL/Object.h>
#include <CGAL/Polynomial.h>
#include <CGAL/Polynomial_traits_d.h>
#include <CGAL/Simple_cartesian.h>
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

  //! type of analysis of a pair of curves
  using Curve_pair_analysis_2 = typename Curve_kernel_2::Curve_pair_analysis_2;

  //! the x-coordinate type
  using Coordinate_1 = typename Point_2::Coordinate_1;

  //! the xy-coordinate type
  using Coordinate_2 = typename Curve_kernel_2::Coordinate_2;

  //! the type of rational values
  using Bound = typename Curve_kernel_2::Bound;

  //! type of the cache of the event arc numbers of interval arc numbers
  using Curve_interval_arcno_cache = typename Curved_kernel_via_analysis_2::Curve_interval_arcno_cache;

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
  const Curve_kernel_2& curve_kernel() const { return _ckva()->kernel(); }

  //!@}

  //!\name Arc operations shared by the functors
  //!@{

  /*! obtains the arc number at a given x-coordinate
   *
   * If \c x0 is equal to source's or target's x-coordinate,
   * then the arc number of that point is returned.
   * Otherwise the arc number of the interior is returned.
   *
   * \param x0 queried x-coordinate
   * \returns arcnumber at \c x0
   * \pre !is_vertical()
   * \pre \c x0 must be within arcs's x-range.
   */
  int arcno(const Arc_2& cv, const Coordinate_1& x0) const {
    CGAL_precondition(! cv.is_vertical());
    CGAL_precondition(this->is_in_x_range(cv, x0));

    auto compare_x = this->curve_kernel().compare_1_object();
    if (cv.ptr()->m_arcno_min != cv.ptr()->m_arcno && cv.is_finite(CGAL::ARR_MIN_END) &&
        compare_x(x0, cv._minpoint().x()) == CGAL::EQUAL) {
      return cv.ptr()->m_arcno_min;
    }
    if (cv.ptr()->m_arcno_max != cv.ptr()->m_arcno && cv.is_finite(CGAL::ARR_MAX_END) &&
        compare_x(x0, cv._maxpoint().x()) == CGAL::EQUAL) {
      return cv.ptr()->m_arcno_max;
    }
    return cv.ptr()->m_arcno;
  }

  /*! returns the index of an open interval between two events of the
   * curve the arc belongs to
   *
   * \return interval id of supporting curve for this arc
   * \pre !is_vertical()
   */
  int interval_id(const Arc_2& cv) const {
    CGAL_precondition(! cv.is_vertical());
    if (! cv.ptr()->m_interval_id) cv.ptr()->m_interval_id = this->_compute_interval_id(cv);
    return *(cv.ptr()->m_interval_id);
  }

  /*! returns boundary value in interior of x-range of non-vertical
   * interval
   *
   * \return a rational x-coordinate in the interior of the arc's x-range
   * \pre !is_vertical()
   */
  Bound boundary_in_x_range_interior(const Arc_2& cv) const {
    CGAL_precondition(! cv.is_vertical());
    if (! cv.ptr()->m_boundary_in_interval) {
      cv.ptr()->m_boundary_in_interval = this->_compute_boundary_in_interval(cv);
      CGAL_postcondition_code(typename Curve_analysis_2::Status_line_1 cv_line =
                                cv.curve().status_line_at_exact_x(Coordinate_1(cv->ptr()->m_boundary_in_interval)););
      CGAL_postcondition(cv_line.index() == this->interval_id(cv));
    }
    return *(cv.ptr()->m_boundary_in_interval);
  }

  /*! checks if the given x-value is in the x-range of the arc inclusive.
   *
   * \param x The x-value.
   * \param *eq_min Output: Is this value equal to the x-coordinate of the
   *                       ARR_MIN_END point.
   * \param *eq_max Output: Is this value equal to the x-coordinate of the
   *                       ARR_MAX_END point.
   * \return \c true, if p.x() is in x-range of arc, \c false otherwise
   */
  bool is_in_x_range(const Arc_2& cv, const Coordinate_1& x, bool* eq_min = nullptr, bool* eq_max = nullptr) const {
    if (eq_min != nullptr && eq_max != nullptr) *eq_min = *eq_max = false;

    if (cv.is_vertical()) {
      if (x == cv.x()) {
        if (eq_min != nullptr) *eq_min = true;
        if (eq_max != nullptr) *eq_max = true;
        return true;
      }
      return false;
    }

    // precomputations:
    auto compare_x = this->curve_kernel().compare_1_object();
    CGAL::Comparison_result resmin = CGAL::LARGER;
    CGAL::Arr_parameter_space min_loc = cv.location(CGAL::ARR_MIN_END);
    bool min_has_x =
      (cv.is_finite(CGAL::ARR_MIN_END) || min_loc == CGAL::ARR_BOTTOM_BOUNDARY || min_loc == CGAL::ARR_TOP_BOUNDARY);
    if (min_has_x) {
      resmin = compare_x(x, cv._minpoint().x());
      if (eq_min != nullptr) { // TODO asymptotic end in x-range?
        *eq_min = (resmin == CGAL::EQUAL);
      }
    }

    CGAL::Comparison_result resmax = CGAL::SMALLER;
    CGAL::Arr_parameter_space max_loc = cv.location(CGAL::ARR_MAX_END);
    bool max_has_x =
      (cv.is_finite(CGAL::ARR_MAX_END) || max_loc == CGAL::ARR_BOTTOM_BOUNDARY || max_loc == CGAL::ARR_TOP_BOUNDARY);

    if (max_has_x) {
      resmax = compare_x(x, cv._maxpoint().x());
      if (eq_max != nullptr) { // TODO asymptotic end in x-range?
        *eq_max = (resmax == CGAL::EQUAL);
      }
    }

    bool res = (resmin != CGAL::SMALLER && resmax != CGAL::LARGER);
    return res;
  }

  // TODO do we need this special method ?
  bool is_in_x_range_interior(const Arc_2& cv, const Coordinate_1& x) const {
    bool eq_min;
    bool eq_max;
    if (! this->is_in_x_range(cv, x, &eq_min, &eq_max) || eq_min || eq_max) return false;
    return true;
  }

  /*! computes the multiplicity of intersection
   *
   * The intersection multiplicity of \c cv and \c cv2 at point \c p is
   * returned.
   *
   * \param cv2 The second arc
   * \param p The intersection point
   * \return The multiplicity of the intersection at \c p
   * \pre \c p must be an intersection point.
   */
  int multiplicity_of_intersection(const Arc_2& cv, const Arc_2& cv2, const Point_2& p) const {
    // intersection point must lie in the interior of both arcs
    CGAL_precondition_code( // because of macro stupidity one needs
      bool eq_min1;         // to omit commas in declaration
      bool eq_max1; bool eq_min2; bool eq_max2;);
    CGAL_precondition(this->is_in_x_range(cv, p.x(), &eq_min1, &eq_max1));
    CGAL_precondition(this->is_in_x_range(cv2, p.x(), &eq_min2, &eq_max2));
    CGAL_precondition(cv.is_vertical() || (! eq_min1 && ! eq_max1));
    CGAL_precondition(cv2.is_vertical() || (! eq_min2 && ! eq_max2));

    // there must be an intersection at this point (in_x_range is checked
    // internally by compare_y_at_x() ?
    CGAL_expensive_precondition(this->_ckva()->compare_y_at_x_2_object()(p, cv) == CGAL::EQUAL &&
                                this->_ckva()->compare_y_at_x_2_object()(p, cv2) == CGAL::EQUAL);

    auto [a1, a2] = this->simplified(cv, cv2);
    CGAL_precondition(! a1.curve().is_identical(a2.curve()));
    if (a1.is_vertical() || a2.is_vertical()) {
      CGAL_assertion(! (a1.is_vertical() && a2.is_vertical()));
      return 1;
    }

    Curve_pair_analysis_2 cpa_2 = this->curve_kernel().construct_curve_pair_2_object()(a1.curve(), a2.curve());

    typename Curve_pair_analysis_2::Status_line_1 cpv_line = cpa_2.status_line_for_x(p.x());

    CGAL_precondition(cpv_line.is_intersection());
    int j = cpv_line.event_of_curve(this->arcno(a1, p.x()), a1.curve());
    int mult = cpv_line.multiplicity_of_intersection(j);

    CGAL_postcondition(mult > 0);
    return mult;
  }

  /*! obtains the arcs \c cv1 and \c cv2 with their supporting curves simplified, in case these share a component:
   * each supporting curve is replaced by the component that the arc lies on, so that the supporting curves of the
   * obtained arcs are either identical or coprime. The given arcs keep their supporting curves and arc numbers; a
   * simplified arc is a copy with a representation of its own, kept in the cache of the curved kernel (keyed by the
   * arc and the other supporting curve), so that it is computed once.
   *
   * \return references to the given arcs, or to their simplified copies; they remain valid as long as the given
   *         arcs and the curved kernel do
   */
  std::pair<const Arc_2&, const Arc_2&> simplified(const Arc_2& cv1, const Arc_2& cv2) const {
    using Result = std::pair<const Arc_2&, const Arc_2&>;
    if (cv1.curve().is_identical(cv2.curve())) return Result(cv1, cv2);

    // the simplifications computed before, if any
    const std::optional<Arc_2>* s1 = nullptr;
    const std::optional<Arc_2>* s2 = nullptr;
    if (const auto* cache = this->_ckva()->arc_simplification_cache_if_any()) {
      s1 = cache->find(cv1, cv2.curve());
      s2 = cache->find(cv2, cv1.curve());
      if (s1 != nullptr && s2 != nullptr) return Result(*s1 ? **s1 : cv1, *s2 ? **s2 : cv2);
    }

    std::vector<Curve_analysis_2> parts_of_f;
    std::vector<Curve_analysis_2> parts_of_g;
    std::vector<Curve_analysis_2> common;
    if (! this->curve_kernel().decompose_2_object()(cv1.curve(), cv2.curve(), std::back_inserter(parts_of_f),
                                                    std::back_inserter(parts_of_g), std::back_inserter(common)))
      return Result(cv1, cv2);   // coprime supporting curves (not cached)
    CGAL_assertion((parts_of_f.size() == 1 || parts_of_g.size() == 1) && common.size() == 1);

    // simplifies an arc (unless it lies on the common component), and caches the result
    auto& cache = this->_ckva()->arc_simplification_cache();
    auto simplify_one = [&](const Arc_2& cv, const Curve_analysis_2& other, const std::optional<Arc_2>* cached,
                            const std::vector<Curve_analysis_2>& parts) -> const Arc_2& {
      if (cached == nullptr) {
        std::optional<Arc_2> res;
        if (parts.size() == 1) {
          Arc_2 a(cv);
          a.copy_on_write();
          this->_simplify_by(a, this->curve_kernel().construct_curve_pair_2_object()(parts[0], common[0]));
          res = std::move(a);
        }
        cached = &cache.insert(cv, other, std::move(res));
      }
      return *cached ? **cached : cv;
    };
    const Arc_2& a1 = simplify_one(cv1, cv2.curve(), s1, parts_of_f);
    const Arc_2& a2 = simplify_one(cv2, cv1.curve(), s2, parts_of_g);
    return Result(a1, a2);
  }

  Arc_2 _trim(const Arc_2& cv, const Point_2& p, const Point_2& q) const {
    const bool p_interior = (p.location() == CGAL::ARR_INTERIOR);
    const bool q_interior = (q.location() == CGAL::ARR_INTERIOR);
    if (p_interior && q_interior) {
      const int arcno_p = cv.is_vertical() ? -1 : this->arcno(cv, p.x());
      const int arcno_q = cv.is_vertical() ? -1 : this->arcno(cv, q.x());
      return this->_replace_endpoints(cv, p, q, arcno_p, arcno_q).first;
    }

    if (! p_interior && ! q_interior) return cv;

    Arc_2 left_arc;
    Arc_2 right_arc;
    auto split_2 = this->_ckva()->split_2_object();

    if (! p_interior && q_interior) {
      split_2(cv, q, left_arc, right_arc);
      return left_arc;
    }
    // if (p.location() == CGAL::ARR_INTERIOR &&
    //   q.location() != CGAL::ARR_INTERIOR)

    split_2(cv, p, left_arc, right_arc);
    return right_arc;
  }

  /*! trims this arc and \c cv2 to the common x-range, if it is non-trivial
   *
   * \param cv2 the second arc
   * \param trimmed1 Output: trimmed version of \c cv to joint x-range of
   *                 \c cv and \c cv2
   * \param trimmed1 Output: trimmed version of \c cv2 to joint x-range of
   *                 \c cv and \c cv2
   * \return \c true, if \c cv and \c cv2 share a non-trivial
   *         common x-range, \c false otherwise
   */
  bool trim_by_arc(const Arc_2& cv, const Arc_2& cv2, Arc_2& trimmed1, Arc_2& trimmed2) const {
    CGAL_precondition(true);

    const Arc_2& cv1 = cv;

    Point_2 common_left;
    Point_2 common_right;

    bool joint = this->_joint_x_range(cv1, cv2, common_left, common_right);

    if (! joint) return false;

    auto compare_x = this->curve_kernel().compare_1_object();

    auto construct_point_on_arc_2 = this->_ckva()->construct_point_on_arc_2_object();

    Point_2 left1;
    Point_2 left2;

    if (common_left.location() != CGAL::ARR_LEFT_BOUNDARY) {
      if ((cv1.location(CGAL::ARR_MIN_END) != CGAL::ARR_LEFT_BOUNDARY) &&
          (compare_x(cv1.curve_end_x(CGAL::ARR_MIN_END), common_left.x()) == CGAL::EQUAL)) {
        left1 = cv1._minpoint();
      }
      else {
        left1 = construct_point_on_arc_2(common_left.x(), cv1.curve(), cv1.arcno(), cv1);
      }
      if ((cv2.location(CGAL::ARR_MIN_END) != CGAL::ARR_LEFT_BOUNDARY) &&
          (compare_x(cv2.curve_end_x(CGAL::ARR_MIN_END), common_left.x()) == CGAL::EQUAL)) {
        left2 = cv2._minpoint();
      }
      else {
        left2 = construct_point_on_arc_2(common_left.x(), cv2.curve(), cv2.arcno(), cv2);
      }
    }
    else {
      left1 = cv1._minpoint();
      left2 = cv2._minpoint();
    }

    Point_2 right1;
    Point_2 right2;

    if (common_right.location() != CGAL::ARR_RIGHT_BOUNDARY) {
      if ((cv1.location(CGAL::ARR_MAX_END) != CGAL::ARR_RIGHT_BOUNDARY) &&
          (compare_x(cv1.curve_end_x(CGAL::ARR_MAX_END), common_right.x()) == CGAL::EQUAL)) {
        right1 = cv1._maxpoint();
      }
      else {
        right1 = construct_point_on_arc_2(common_right.x(), cv1.curve(), cv1.arcno(), cv1);
      }
      if ((cv2.location(CGAL::ARR_MAX_END) != CGAL::ARR_RIGHT_BOUNDARY) &&
          (compare_x(cv2.curve_end_x(CGAL::ARR_MAX_END), common_right.x()) == CGAL::EQUAL)) {
        right2 = cv2._maxpoint();
      }
      else {
        right2 = construct_point_on_arc_2(common_right.x(), cv2.curve(), cv2.arcno(), cv2);
      }
    }
    else {
      right1 = cv1._maxpoint();
      right2 = cv2._maxpoint();
    }

    trimmed1 = this->_trim(cv1, left1, right1);
    trimmed2 = this->_trim(cv2, left2, right2);

    return joint;
  }

  /*! ensures lexicographical order of the curve ends
   *
   * must be called once from constructor
   */
  void _fix_curve_ends_order(Arc_2& cv) const {
    CGAL::Comparison_result res = this->_same_arc_compare_xy(cv, cv._minpoint(), cv._maxpoint());
    // curve ends cannot be identical
    CGAL_precondition(res != CGAL::EQUAL);
    if (res == CGAL::LARGER) { // swap curve ends and corresponding arcnos
      std::swap(cv.ptr()->m_min, cv.ptr()->m_max);
      std::swap(cv.ptr()->m_arcno_min, cv.ptr()->m_arcno_max);
    }
    // for non-vertical arcs check arcno constancy in the arc's interior
    // for vertical arcs check that there are no intersection points
    // between curve ends
    this->_check_arc_interior(cv);
  }

  // p.curve() <-> p.arcno()
  // c <-> arcno_on_c
  /*! establishes preconditions that point \c pt lies on the curve
   * \c c with arc number \c arcno_on_c, also checks that point's supporting
   * curve and \c c are coprime
   *
   * \param pt Given point
   * \param arcno_on_c Arcno on curve
   * \param c Supporting curve
   */
  void _check_pt_arcno_and_coprimality(const Arc_2& cv, const Point_2& CGAL_precondition_code(pt),
                                       int CGAL_precondition_code(arcno_on_c),
                                       const Curve_analysis_2& CGAL_precondition_code(c)) const {
    CGAL_precondition_code(
      if (! c.is_identical(pt.curve())) {
        // -1 defines that no arcnos preconditions need to be established
        if (arcno_on_c != -1) {
          typename Curve_pair_analysis_2::Status_line_1 cpv_line;
          Curve_pair_analysis_2 cpa_2 = this->curve_kernel().construct_curve_pair_2_object()(pt.curve(), c);

          cpv_line = cpa_2.status_line_for_x(pt.x());
          CGAL_precondition(cpv_line.event_of_curve(pt.arcno(), pt.curve()) == cpv_line.event_of_curve(arcno_on_c, c));
        }
        std::vector<Curve_analysis_2> dummy[3];
        // ensure that curves are not decomposable
        CGAL_precondition(! this->curve_kernel().decompose_2_object()(c, pt.curve(),
                                                                std::back_inserter(dummy[0]),
                                                                std::back_inserter(dummy[1]),
                                                                std::back_inserter(dummy[2])));
      }
      else if (arcno_on_c != -1) { CGAL_precondition(pt.arcno() == arcno_on_c); });
  }

  /*! establishes preconditions to ensure that there are no event
   * points in the arc's interior (only at source and target) and its arc
   * number is constant
   *
   * \pre before calling this method source and target must be sorted
   * using \c _fix_curve_ends_order()
   */
  void _check_arc_interior(const Arc_2& cv) const {

#if ! (defined(CGAL_KERNEL_NO_PRECONDITIONS) || defined(CGAL_NO_PRECONDITIONS))
    const CGAL::Arr_parameter_space min_loc = cv._minpoint().location();
    const CGAL::Arr_parameter_space max_loc = cv._maxpoint().location();
    if (cv.is_vertical()) {
      Coordinate_1 x0 = cv._minpoint().x();
      typename Curve_analysis_2::Status_line_1 cv_line;
      cv_line = cv.curve().status_line_for_x(x0);
      CGAL_precondition(cv_line.is_event());
      CGAL_precondition(cv_line.covers_line());

      // check that there are no intersections between min and max
      // curve ends
      bool inf_src = (min_loc == CGAL::ARR_BOTTOM_BOUNDARY);
      bool inf_tgt = (max_loc == CGAL::ARR_TOP_BOUNDARY);
      // either no events over this line or the vertical line has at
      // least one finite end
      CGAL_precondition(cv_line.number_of_events() == 0 || ! (inf_src && inf_tgt));

      auto cmp_xy = this->curve_kernel().compare_xy_2_object();

      for (int k = 0; k < cv_line.number_of_events(); k++) {
        Coordinate_2 tmp(x0, cv.curve(), k);
        bool res1 = true;
        bool res2 = true;
        if (! inf_src) res1 = (cmp_xy(cv._minpoint().xy(), tmp, true) == CGAL::SMALLER);
        if (! inf_tgt) res2 = (cmp_xy(tmp, cv._maxpoint().xy(), true) == CGAL::SMALLER);
        CGAL_precondition_msg(! (res1 && res2), "Events are not allowed in the interior of a vertical arc!");
      }
      return;
    }

    typename Curve_analysis_2::Status_line_1 src_line;
    typename Curve_analysis_2::Status_line_1 tgt_line;
    typename Curve_analysis_2::Status_line_1 tmp;
    bool inf_src = (min_loc == CGAL::ARR_LEFT_BOUNDARY);
    bool inf_tgt = (max_loc == CGAL::ARR_RIGHT_BOUNDARY);
    src_line = (inf_src ? cv.curve().status_line_of_interval(0) : cv.curve().status_line_for_x(cv._minpoint().x()));
    tgt_line = (inf_tgt ? cv.curve().status_line_of_interval(cv.curve().number_of_status_lines_with_event())
                        : cv.curve().status_line_for_x(cv._maxpoint().x()));

    int src_idx = src_line.index();
    int tgt_idx = tgt_line.index();
    int diff = tgt_idx - src_idx;
    bool no_events_between = true;
    // it's supposed that arcs are not degenerate but lexicographic
    // order may not be established
    if (src_line.is_event())
      no_events_between = (tgt_line.is_event() ? (diff == 1) : (diff == 0) || (diff == 1));
    else
      no_events_between = (tgt_line.is_event() ? (diff == 0) || (diff == -1) : (diff == 0));

    if (! no_events_between) {
      // iterate through all events between source and target
      // to check that all events points lie above our arc
      int m_src_idx = src_idx + (src_line.is_event() ? 1 : 0);
      int m_tgt_idx = tgt_idx - 1;
      int low = m_src_idx;
      int high = m_tgt_idx;
      int i;
      int j;
      if (low > high) // do we need to check it ?
        std::swap(low, high);
      std::pair<int, int> ipair;
      for (i = low; i <= high; i++) {
        tmp = cv.curve().status_line_at_event(i);
        for (j = 0; j < tmp.number_of_events(); j++) {
          ipair = tmp.number_of_incident_branches(j);
          if (ipair.first != 1 || ipair.second != 1) break;
        }
        // there must be at least one event and cv.arcno() is not LARGER
        // than this event index
        CGAL_precondition(j < tmp.number_of_events() && cv.arcno() <= j);
      }
    }
    // check validity of the curve-ends arcnos

    const Curve_interval_arcno_cache& map_interval_arcno = this->_ckva()->interval_arcno_cache();

    if (src_line.is_event()) {
      CGAL_precondition(map_interval_arcno(src_line, 0, cv.arcno()).first == cv.ptr()->m_arcno_min);
    }
    else {
      CGAL_precondition(cv.arcno() == cv.ptr()->m_arcno_min);
    }
    if (tgt_line.is_event()) {
      CGAL_precondition(map_interval_arcno(tgt_line, 1, cv.arcno()).first == cv.ptr()->m_arcno_max);
    }
    else {
      CGAL_precondition(cv.arcno() == cv.ptr()->m_arcno_max);
    }
#endif
  }

  /*! compares y-coordinates of two arcs over an open (or closed)
   * interval or at exact x-coordinate
   *
   * \c where specifies whether to compare at negative/positive boundary or
   * at finite point. if \c where = ARR_INTERIOR \c perturb defines to
   * compare slightly to the left, on, or to the right of \c x0
   *
   * \param cv2 the second arc
   * \param where the location in parameter space
   * \param x0 The x-coordinate
   * \param perturb determines whether to perturb slightly to the left/right
   * \return the relative vertical alignment
   *
   * \pre !is_on_bottom_top(where)
   */
  CGAL::Comparison_result _compare_arc_numbers(const Arc_2& cv, const Arc_2& cv2, CGAL::Arr_parameter_space where,
                                               Coordinate_1 x0 = Coordinate_1(),
                                               CGAL::Sign perturb = CGAL::ZERO) const {
    CGAL_precondition(! cv.is_on_bottom_top(where));
    CGAL_assertion(true);
    auto [a1, a2] = this->simplified(cv, cv2);
    if (a1.curve().is_identical(a2.curve())) return CGAL::sign(a1.arcno() - a2.arcno());
    return this->_compare_coprime(a1, a2, where, x0, perturb);
  }

  /*! computes vertical ordering of \c cv and \c cv2
   * having coprime supporting curves
   *
   * \param cv2 the second arc
   * \param where the location in parameter space
   * \param x0 The x-coordinate
   * \param perturb determines whether to perturb slightly to the left/right
   * \return the relative vertical alignment
   */
  CGAL::Comparison_result _compare_coprime(const Arc_2& cv, const Arc_2& cv2, CGAL::Arr_parameter_space where,
                                           Coordinate_1 x0, CGAL::Sign perturb) const {
#ifdef CKvA_DEBUG_PRINT_CERR
    CERR("\n_compare_coprime; this: " << cv << "; g: " << cv2.curve().polynomial_2() << "; arcno_on_g: " << cv2.arcno()
                                      << "; where: " << where);
    if (where == CGAL::ARR_INTERIOR) CERR("; x = " << CGAL::to_double(x0));
    CERR("\n");
#endif

    typename Curve_pair_analysis_2::Status_line_1 cpv_line;
    Curve_pair_analysis_2 cpa_2 = this->curve_kernel().construct_curve_pair_2_object()(cv.curve(), cv2.curve());

    if (where == CGAL::ARR_INTERIOR) cpv_line = cpa_2.status_line_for_x(x0, perturb);
    else
      cpv_line =
        cpa_2.status_line_of_interval(where == CGAL::ARR_LEFT_BOUNDARY ? 0 : cpa_2.number_of_status_lines_with_event());

    CGAL::Sign res =
      CGAL::sign(cpv_line.event_of_curve(cv.arcno(), cv.curve()) - cpv_line.event_of_curve(cv2.arcno(), cv2.curve()));
    CERR("result: " << res << "\n");
    return res;
  }

  /* internal comparison of two curve ends "lying" on the same arc
   *
   * since points are supposed to lie on the same arc, converging to the
   * boundary implies equality
   *
   * \param p first endpoint
   * \param q second endpoint
   * \param equal_x \c true indicates to skip the comparison by x
   * \param only_x \c true indicates to report only the comparison by x
   * \returns the result of the queried comparison
   */
  CGAL::Comparison_result _same_arc_compare_xy(const Arc_2& cv, const Point_2& p, const Point_2& q,
                                               bool equal_x = false, bool only_x = false) const {
    CERR("\n_same_arc_compare_xy; this: " << cv << "; p: " << p << "; q: "
                                          << q << "; equal_x: " << equal_x << "; only_x: " << only_x << "\n");

    CGAL::Comparison_result res;

    if (p.is_identical(q)) {
      res = CGAL::EQUAL;
      CERR("result1: " << res << "\n");
      return res;
    }

    CGAL::Arr_parameter_space locp = p.location();
    CGAL::Arr_parameter_space locq = q.location();
    if (! equal_x || only_x) {
      if (! p.is_on_left_right() && ! q.is_on_left_right()) {
        // both xs are finite: require x-comparisons
        res = this->_ckva()->compare_x_2_object()(p, q);
        if (res != CGAL::EQUAL) {
          CERR("result2: " << res << "\n");
          return res;
        }
      }
      else if (locp != locq) {
        CGAL_assertion(p.is_on_left_right() || q.is_on_left_right());
        // at least one of the points lies at infty: suffice to cmp
        // boundaries
        if (locp == CGAL::ARR_LEFT_BOUNDARY) {
          res = CGAL::SMALLER;
          CERR("result3: " << res << "\n");
          return res;
        }
        else if (locp == CGAL::ARR_RIGHT_BOUNDARY) {
          res = CGAL::LARGER;
          CERR("result4: " << res << "\n");
          return res;
        }
        else if (locq == CGAL::ARR_LEFT_BOUNDARY) {
          res = CGAL::LARGER;
          CERR("result5: " << res << "\n");
          return res;
        }
        else if (locq == CGAL::ARR_RIGHT_BOUNDARY) {
          res = CGAL::SMALLER;
          CERR("result6: " << res << "\n");
          return res;
        }
      } // else: proceed to y-comparison
    }
    if (only_x) {
      res = CGAL::EQUAL;
      CERR("result7: " << res << "\n");
      return res;
    }
    if (locp == locq) {
      if (locp != CGAL::ARR_INTERIOR) {
        res = CGAL::EQUAL; // both points are at the same inf in y
        CERR("result8: " << res << "\n");
        return res;
      }
      // compare only y-values;
      res = this->_ckva()->compare_xy_2_object()(p, q, true);
      CERR("result9: " << res << "\n");
      return res;
    }
    // here: locp != locq && one of them is at inf y
    if (locp == CGAL::ARR_INTERIOR) {
      res = (locq == CGAL::ARR_BOTTOM_BOUNDARY ? CGAL::LARGER : CGAL::SMALLER);
      CERR("result10: " << res << "\n");
      return res;
    }
    // here: locp != locq && locp is at infty
    res = (locp == CGAL::ARR_BOTTOM_BOUNDARY ? CGAL::SMALLER : CGAL::LARGER);
    CERR("result11: " << res << "\n");
    return res;
  }

  /*! computes this arc's interval index
   *
   * \pre !is_vertical()
   */
  int _compute_interval_id(const Arc_2& cv) const {
    CGAL_precondition(! cv.is_vertical());
    // we are interested in interval "to the right"
    CGAL::Arr_parameter_space min_loc = cv.location(CGAL::ARR_MIN_END);
    bool min_has_x =
      (cv.is_finite(CGAL::ARR_MIN_END) || min_loc == CGAL::ARR_BOTTOM_BOUNDARY || min_loc == CGAL::ARR_TOP_BOUNDARY);

    if (! min_has_x) return 0;
    // else
    typename Curve_analysis_2::Status_line_1 cv_line = cv.curve().status_line_for_x(cv._minpoint().x(), CGAL::POSITIVE);
    return cv_line.index();
  }

  /*! computes this rational value in the interiors of the arc's x-range
   *
   * \pre !is_vertical()
   */
  Bound _compute_boundary_in_interval(const Arc_2& cv) const {
    CGAL_precondition(! cv.is_vertical());
    // a curve end at negative boundary => 0th interval

    Bound res(0);

    typename Curve_kernel_2::Approximate_relative_1 approx_x;
    typename Curve_kernel_2::Bound_between_1 bound_between_x;

    CGAL::Arr_parameter_space min_loc = cv.location(CGAL::ARR_MIN_END);
    bool min_has_x =
      (cv.is_finite(CGAL::ARR_MIN_END) || min_loc == CGAL::ARR_BOTTOM_BOUNDARY || min_loc == CGAL::ARR_TOP_BOUNDARY);

    CGAL::Arr_parameter_space max_loc = cv.location(CGAL::ARR_MAX_END);
    bool max_has_x =
      (cv.is_finite(CGAL::ARR_MAX_END) || max_loc == CGAL::ARR_BOTTOM_BOUNDARY || max_loc == CGAL::ARR_TOP_BOUNDARY);

    if (min_has_x) {
      Coordinate_1 min_x = cv._minpoint().x();
      if (max_has_x) {
        Coordinate_1 max_x = cv._maxpoint().x();
        res = bound_between_x(min_x, max_x);
      }
      else {
        std::pair<Bound, Bound> min_pair = approx_x(min_x, 4);
        res = min_pair.second + Bound(1);
      }
    }
    else {
      if (max_has_x) {
        Coordinate_1 max_x = cv._maxpoint().x();
        std::pair<Bound, Bound> max_pair = approx_x(max_x, 4);
        res = max_pair.first - Bound(1);
      }
      else {
        // res stays 0
      }
    }
    CGAL_postcondition(this->is_in_x_range_interior(cv, Coordinate_1(res)));
    return res;
  }

  /*! replaces this arc's end-points by \c p1 and \c p2 with arcnos
   * \c arcno1 and \c arcno2.
   *
   * new curve ends are sorted lexicographical in case of need;
   * all preconditions must be checked by the caller
   *
   * \param p1 new first endpoint
   * \param p2 new second endpoint
   * \param arcno1 new first arcno (at \c p1)
   * \param arcno1 new second arcno (at \c p2)
   * \return pair whose first entry represent the refined arc, and whose
   *         second entry reports the lexicographic comparison of p1 and p2
   */
  std::pair<Arc_2, CGAL::Comparison_result> _replace_endpoints(const Arc_2& cv, const Point_2& p1, const Point_2& p2,
                                                               int arcno1 = -1, int arcno2 = -1) const {
    CERR("\n_replace_endpoints\n");

    typename Arc_2::Rep rep(*(cv.ptr()));
    rep.m_min = p1;
    rep.m_max = p2;
    if (! cv.is_vertical()) {
      if (arcno1 >= 0) rep.m_arcno_min = arcno1;
      if (arcno2 >= 0) rep.m_arcno_max = arcno2;
    }

    CGAL::Comparison_result cmp = this->_same_arc_compare_xy(cv, p1, p2);
    if (cmp == CGAL::LARGER) {
      std::swap(rep.m_min, rep.m_max);
      std::swap(rep.m_arcno_min, rep.m_arcno_max);
    }
    /* no need to recompute location since they are set during
           construction of respective curve ends */
    rep.m_is_vertical = cv.ptr()->m_is_vertical;
    rep.m_left_to_right = cv.ptr()->m_left_to_right;

    rep.m_interval_id = std::nullopt;
    rep.m_boundary_in_interval = std::nullopt;

    return std::make_pair(Arc_2(rep), cmp);
  }

  /*! simplifies representation of the arc !! DEPRECATED FUNCTION !!
   *
   * Given a decomposition of the arcs's supporting curve into a pair of two
   * curves \c cpa_2, we search for a curve this arc lies on and reset arc's
   * supporting curve and arcnos appropriately.
   *
   * \param cpa_2 analysis of curve pair that should be used
   *              in simplification
   * \pre \c cpa_2 must correspond to a decomposition of this arc's
   * supporting curve
   */
  void _simplify_by(const Arc_2& cv, const Curve_pair_analysis_2& cpa_2) const {
    using Polynomial_2 = typename Curve_analysis_2::Polynomial_2;
    Polynomial_2 f = cv.curve().polynomial_2();
    CGAL_precondition_code(Polynomial_2 mult =
                             cpa_2.curve_analysis(0).polynomial_2() * cpa_2.curve_analysis(1).polynomial_2();
                           typename CGAL::Polynomial_traits_d<Polynomial_2>::Total_degree deg;);
    // common parts and full parts
    CGAL_precondition(CGAL::resultant(mult, f).degree() < 1);
    CGAL_precondition(mult.degree() == f.degree());
    CGAL_precondition(deg(mult) == deg(f));

    Coordinate_1 x0;
    if (cv.is_vertical()) {
      // processing vertical arcs: search for supporting curve which has
      // vertical line at this x0 (must be exactly 1 curve)
      x0 = cv._minpoint().x();
      Curve_analysis_2 ca_2(cpa_2.curve_analysis(0));
      if (ca_2.status_line_for_x(x0).covers_line()) cv.ptr()->m_support = ca_2;
      else {
        ca_2 = cpa_2.curve_analysis(1);
        CGAL_assertion(ca_2.status_line_for_x(x0).covers_line());
        cv.ptr()->m_support = ca_2;
      }
      return;
    }

    // processing non-vertical arcs
    typename Curve_pair_analysis_2::Status_line_1 cpv_line;
    std::pair<int, int> ipair;
    // preserve original supporting curve
    Curve_analysis_2 orig_curve(cv.curve());

    const CGAL::Arr_parameter_space min_loc = cv._minpoint().location();
    bool inf1_x = (min_loc == CGAL::ARR_LEFT_BOUNDARY);
    bool curve_idx;
    if (! inf1_x) {
      x0 = cv._minpoint().x();
      cpv_line = cpa_2.status_line_for_x(x0, CGAL::POSITIVE);
    }
    else
      cpv_line = cpa_2.status_line_of_interval(0);

    CGAL_precondition_code(
      typename Curve_analysis_2::Status_line_1 cv_line =
        (inf1_x ? orig_curve.status_line_of_interval(0) : orig_curve.status_line_for_x(x0, CGAL::POSITIVE)););
    CGAL_precondition(cpv_line.number_of_events() == cv_line.number_of_events());

    {
      // search for new supporting curve and new arcno
      // since supporting curve was decomposed in two parts, arcno
      // represents y-position here
      ipair = cpv_line.curves_at_event(cv.arcno());
      // this must be 1-curve event
      CGAL_assertion(! (ipair.first != -1 && ipair.second != -1));
      cv.ptr()->m_arcno = (ipair.first != -1 ? ipair.first : ipair.second);
      curve_idx = (ipair.first == -1);
      cv.ptr()->m_support = cpa_2.curve_analysis(curve_idx);
    }
    // search for source arcno
    /////////////// ATTENTION: this only holds for 2D plane topology !!
    ///////////////////////////////////////////////////////////////////
    if (min_loc == CGAL::ARR_INTERIOR) {
      cpv_line = cpa_2.status_line_for_x(x0);
      CGAL_precondition(cpv_line.number_of_events() == orig_curve.status_line_for_x(x0).number_of_events());
      ipair = cpv_line.curves_at_event(cv.ptr()->m_arcno_min);
      if (ipair.first != -1 && ipair.second != -1)
        // choose simpler supporting curve
        cv.ptr()->m_arcno_min = ((curve_idx) ? ipair.second : ipair.first);
      else {
        CGAL_assertion(ipair.first != -1 || ipair.second != -1);
        cv.ptr()->m_arcno_min = (ipair.first != -1 ? ipair.first : ipair.second);
      }
    }
    else // for infinite curve end arcno equals to interior arcno
      cv.ptr()->m_arcno_min = cv.arcno();

    // search for new target arcno
    /////////////// ATTENTION: this only holds for 2D plane topology !!
    ///////////////////////////////////////////////////////////////////
    if (cv._maxpoint().location() == CGAL::ARR_INTERIOR) {
      x0 = cv._maxpoint().x();
      cpv_line = cpa_2.status_line_for_x(x0);
      CGAL_precondition(cpv_line.number_of_events() == orig_curve.status_line_for_x(x0).number_of_events());

      ipair = cpv_line.curves_at_event(cv.ptr()->m_arcno_max);
      if (ipair.first != -1 && ipair.second != -1)
        // choose simpler supporting curve (the one which matches
        //  interior arcno)
        cv.ptr()->m_arcno_max = (curve_idx ? ipair.second : ipair.first);
      else {
        CGAL_assertion(ipair.first != -1 || ipair.second != -1);
        cv.ptr()->m_arcno_max = (ipair.first != -1 ? ipair.first : ipair.second);
      }
    }
    else // for infinite curve end arcno equals to interior arcno
      cv.ptr()->m_arcno_max = cv.arcno();

    // invalidate curve-specific data
    cv.ptr()->m_interval_id = std::nullopt;
    cv.ptr()->m_boundary_in_interval = std::nullopt;
  }

  /*! returns \c true if the two arcs \c cv and \c cv2 overlap,
   * overlapping part(s) are inserted to the output iterator \c oi
   * (of type \c Kernel_arc_2 ); if no overlapping parts found -
   * returns \c false
   *
   * \param cv2 The second arc
   * \param oi Report overlapping parts to this output iterator
   * \return \c true, if there was an overlap, \c false otherwise
   */
  template <typename OutputIterator>
  bool _trim_if_overlapped(const Arc_2& cv, const Arc_2& cv2, OutputIterator oi) const {
    CERR("\n_trim_if_overlapped: this: " << cv << "; and " << cv2 << "\n");
    // one arc is vertical and the other one is not, or x-ranges are not
    // overlapping => quit
    if (cv.is_vertical() != cv2.is_vertical()) return false;

    if (cv.is_vertical()) { // here process vertical case
      // check for x-coordinates equality
      if (this->_ckva()->compare_x_2_object()(cv._minpoint(), cv2._minpoint()) != CGAL::EQUAL) return false;

      auto [a1, a2] = this->simplified(cv, cv2);
      // coprime support => no overlaps
      if (! a1.curve().is_identical(a2.curve())) return false;

      // LARGER source and smaller target
      Point_2 src = (this->_same_arc_compare_xy(a1, a1._minpoint(), a2._minpoint(), true) == CGAL::LARGER ?
                     a1._minpoint() : a2._minpoint());
      Point_2 tgt = (this->_same_arc_compare_xy(a1, a1._maxpoint(), a2._maxpoint(), true) == CGAL::SMALLER ?
                     a1._maxpoint() : a2._maxpoint());
      // vertical arcs do not overlap
      if (this->_same_arc_compare_xy(a1, src, tgt, true) != CGAL::SMALLER) return false;
      // construct a common part
      *oi++ = (this->_replace_endpoints(a1, src, tgt, -1, -1).first);
      return true;
    }
    // ask for joint x-range of two arcs
    // (LARGER source & smaller target curve ends)
    Point_2 src;
    Point_2 tgt;
    if (! this->_joint_x_range(cv, cv2, src, tgt)) return false;

    // If the supporting curves share a component, replace each supporting curve by the part that the arc lies on
    // (the common component or the rest of the curve), as in the vertical case. Then the supporting curves are either
    // identical or coprime (the curves are square free).
    auto [a1, a2] = this->simplified(cv, cv2);
    if (a1.curve().is_identical(a2.curve())) {
      // arcnos are not equal => no overlaps
      if (a1.arcno() != a2.arcno()) return false;
      int a_min = (src.is_on_left_right() ? -1 : this->arcno(a1, src.x()));
      int a_max = (tgt.is_on_left_right() ? -1 : this->arcno(a1, tgt.x()));
      // construct a common  part
      *oi++ = this->_replace_endpoints(a1, src, tgt, a_min, a_max).first;
      return true;
    }

    // coprime supporting curves => no overlaps
    return false;
  }

  /*! computes zero-dimensional intersections of \c cv1 with \c cv2.
   *
   * Intersection points
   * are inserted to the output iterator \c oi as objects of type
   * <tt>std::pair<Point_2, unsigned int></tt> (intersection point +
   * multiplicity)
   *
   * \param cv1 the first arc
   * \param cv2 the second arc
   * \param oi reporting zero-dimensional intersections through this output
   *        iterator
   * \pre !cv1.do_overlap()
   */
  template <typename OutputIterator>
  OutputIterator _intersection_points(const Arc_2& cv1, const Arc_2& cv2, OutputIterator oi) const {
    // handle a special case when two arcs are supported by the same
    // curve => only end-point intersections

    CERR("\nintersection_points\n");
    auto [a1, a2] = this->simplified(cv1, cv2);
    if (a1.curve().is_identical(a2.curve())) return this->_intersect_at_endpoints(a1, a2, oi);

    // else general case: distinct supporting curves
    return this->_intersect_coprime_support(a1, a2, oi);
  }

  /*! computes intersection of two arcs meeting only at their curve ends.
   *
   * Intersection points are returned in the output iterator \c oi as object
   * of type std::pair<Point_2, int> (intersection + multiplicity)
   *
   * \param cv1 the first arc
   * \param cv2 the second arc
   * \param oi reporting zero-dimensional intersections through this output
   *        iterator
   *
   */
  template <typename OutputIterator>
  OutputIterator _intersect_at_endpoints(const Arc_2& cv1, const Arc_2& cv2, OutputIterator oi) const {
    CERR("\n_intersect_at_endpoints\n");

    CGAL_precondition(! this->_ckva()->do_overlap_2_object()(cv1, cv2));
    /* Since cv and cv2 do not overlap and cannot contain singularities
     * in the interior, the only remaining candidates for intersections are
     * their finite endpoints (if any), for vertical arcs as well.
     */

    bool f2_min = (cv2._minpoint().location() == CGAL::ARR_INTERIOR);
    bool f2_max = (cv2._maxpoint().location() == CGAL::ARR_INTERIOR);
    if (! (f2_min || f2_max)) return oi; // neither of curve ends is finite => no intersections

    Point_2 pt;

    CGAL::Arr_curve_end end = CGAL::ARR_MIN_END;

    while (1) {
      CGAL::Arr_parameter_space loc = cv1.location(end);
      if (loc != CGAL::ARR_INTERIOR) goto Lendloop;
      pt = cv1.curve_end(end);
      if ((f2_min && pt == cv2._minpoint()) || (f2_max && pt == cv2._maxpoint())) *oi++ = std::make_pair(pt, 0);

    Lendloop:
      if (end == CGAL::ARR_MAX_END) break;
      end = CGAL::ARR_MAX_END;
    }
    return oi;
  }

  /*! computes a joint x-range of two arcs and returns \c true
   * if arcs' x-ranges overlap; otherwise returns \c false
   *
   * \param cv2 The second arc
   * \param pt_low Output: Point indicating the lower bound of the joint
   *        x-range
   * \param pt_high Output: Point indicating the upper bound of the joint
   *        x-range
   * \return \c true, if arcs overlap, \c false otherwise
   *
   * \pre both arcs are not vertical
   */
  bool _joint_x_range(const Arc_2& cv, const Arc_2& cv2, Point_2& pt_low, Point_2& pt_high) const {
    CERR("\n_joint_x_range\n");

    CGAL_precondition(! cv.is_vertical());
    CGAL_precondition(! cv2.is_vertical());

    Point_2 pt1 = cv._minpoint();
    Point_2 pt2 = cv2._minpoint();
    Point_2 low = pt2;
    Point_2 high;
    auto compare_x = this->_ckva()->compare_x_2_object();
    // find intersection x-range: larger source & smaller target
    if (pt1.location() != CGAL::ARR_LEFT_BOUNDARY) {
      low = ((pt2.location() == CGAL::ARR_LEFT_BOUNDARY) || (compare_x(pt1, pt2) == CGAL::LARGER)) ?
        pt1 : pt2;
    }
    pt1 = cv._maxpoint(), pt2 = cv2._maxpoint(), high = pt2;
    if (pt1.location() != CGAL::ARR_RIGHT_BOUNDARY) {
      high = ((pt2.location() == CGAL::ARR_RIGHT_BOUNDARY) || (compare_x(pt1, pt2) == CGAL::SMALLER)) ?
        pt1 : pt2;
    }
    if (! low.is_on_left_right() && ! high.is_on_left_right() && compare_x(low, high) != CGAL::SMALLER) {
      // disjoint x-ranges
      return false;
    }
    pt_low = low;
    pt_high = high;

    return true;
  }

  /*! computes zero-dimensional
   * intersections of two arcs having coprime supporting curves
   *
   * intersection points are inserted to the output iterator \c oi as objects
   * of type <tt>std::pair<Point_2, unsigned int></tt> (intersection point +
   * multiplicity)
   *
   * \param cv1 the first arc
   * \param cv2 the second arc
   * \param oi reporting zero-dimensional intersections through this output
   *        iterator
   */
  template <typename OutputIterator>
  OutputIterator
  _intersect_coprime_support(const Arc_2& cv1, const Arc_2& cv2, OutputIterator oi) const {
    // vertical arcs: the interesting case is when only one of the arcs is
    // vertical - otherwise there is no intersection (different x-coords),
    // or they overlap (not allowed), or they touch at the end-points
    // (already tested)

    CERR("\n_intersect_coprime_support: " << cv1 << " and " << cv2 << "\n");

    // due to coprimality condition, supporting curves are different =>
    // they have no common vertical line therefore there is no
    // intersection
    if (cv1.is_vertical() && cv2.is_vertical()) return oi;

    auto construct_point_on_arc = this->_ckva()->construct_point_on_arc_2_object();
    if (cv1.is_vertical() || cv2.is_vertical()) {
      const Arc_2& vert = (cv1.is_vertical() ? cv1 : cv2);
      const Arc_2& nonvert = (cv1.is_vertical() ? cv2 : cv1);
      Coordinate_1 x = vert._minpoint().x();
      // vertical arc does not lie within another arc's x-range => no
      // intersections
      bool eq_min = false;
      bool eq_max = false;
      if (! this->is_in_x_range(nonvert, x, &eq_min, &eq_max)) return oi;
      // the non-vertical arc only approaches the vertical line asymptotically, i.e., its end over x lies at
      // infinity => no intersections
      if ((eq_min && ! nonvert.is_finite(CGAL::ARR_MIN_END)) || (eq_max && ! nonvert.is_finite(CGAL::ARR_MAX_END)))
        return oi;

      Point_2 xy = construct_point_on_arc(x, nonvert.curve(), this->arcno(nonvert, x), nonvert);
      if (this->_ckva()->compare_y_at_x_2_object()(xy, vert) == CGAL::EQUAL) *oi++ = std::make_pair(xy, 1);
      return oi;
    }

    Point_2 low_x;
    Point_2 high_x;
    // x-ranges are disjoint => nothing to do
    if (! this->_joint_x_range(cv1, cv2, low_x, high_x)) return oi;
    bool inf_low = low_x.is_on_left_right();
    bool inf_high = high_x.is_on_left_right();
    Curve_analysis_2 f = cv1.curve();
    Curve_analysis_2 g = cv2.curve();
    Curve_pair_analysis_2 cpa_2 = this->curve_kernel().construct_curve_pair_2_object()(f, g);
    int low_idx = 0;
    int high_idx = cpa_2.number_of_status_lines_with_event() - 1;

    bool index_at_event_min = false;
    bool index_at_event_max = false;

    typename Curve_pair_analysis_2::Status_line_1 line;
    if (! inf_low) {
      line = cpa_2.status_line_for_x(low_x.x());
      low_idx = line.index();
      index_at_event_min = line.is_event();
      if (index_at_event_min) {
        // An arc whose minimal end lies on the bottom or top boundary at low_x (a vertical asymptote) has no point
        // over low_x; hence, the arcs do not intersect there (the curves might, through other branches), and the
        // event is skipped.
        if ((cv1._minpoint().is_on_bottom_top() && low_x.x() == cv1._minpoint().x()) ||
            (cv2._minpoint().is_on_bottom_top() && low_x.x() == cv2._minpoint().x())) {
          low_idx++;
          index_at_event_min = false;
        }
      }
    }

    if (! inf_high) {
      line = cpa_2.status_line_for_x(high_x.x());
      high_idx = line.index();
      index_at_event_max = line.is_event();
      if (! index_at_event_max)
        high_idx--;
      else if ((cv1._maxpoint().is_on_bottom_top() && high_x.x() == cv1._maxpoint().x()) ||
               (cv2._maxpoint().is_on_bottom_top() && high_x.x() == cv2._maxpoint().x())) {
        // as above, for a maximal end over high_x
        high_idx--;
        index_at_event_max = false;
      }
    }

    // run over all event points within the joint x-range of two arcs
    // looking whether a particular event is made of both curves, i.e.,
    // grabbing all 2-curve events
    int arcno1;
    int arcno2;
    int mult;

    typename CGAL::Polynomial_traits_d<typename Curve_kernel_2::Polynomial_2>::Total_degree deg;

    bool which_curve = (deg(f.polynomial_2()) < deg(g.polynomial_2()));
    for (int i = low_idx; i <= high_idx; i++) {
      typename Curve_pair_analysis_2::Status_line_1 tmp = cpa_2.status_line_at_event(i);
      if (! tmp.is_intersection()) continue;

      Coordinate_1 x0 = tmp.x();
      if ((i == low_idx && index_at_event_min) || (i == high_idx && index_at_event_max)) {
        arcno1 = this->arcno(cv1, x0);
        arcno2 = this->arcno(cv2, x0);
        mult = 0; // intersection at end-point
      }
      else {
        arcno1 = cv1.arcno();
        arcno2 = cv2.arcno();
        mult = -1; // need to compute
      }

      int pos = tmp.event_of_curve(arcno1, f);
      if (pos != tmp.event_of_curve(arcno2, g)) continue;
      if (mult == -1) mult = tmp.multiplicity_of_intersection(pos);

      // pick up the curve with lower degree
      if (which_curve) {
        Point_2 p = construct_point_on_arc(x0, cv1.curve(), arcno1, cv1);
        *oi++ = std::make_pair(p, mult);
      }
      else {
        Point_2 p = construct_point_on_arc(x0, cv2.curve(), arcno2, cv2);
        *oi++ = std::make_pair(p, mult);
      }
    }
    return oi;
  }

  std::pair<double, double> y_interval_for_curve_end(const Arc_2& arc, CGAL::Arr_curve_end end, long prec) const {
    double PINF = std::numeric_limits<double>::infinity();
    double MINF = -PINF;

    std::pair<Bound, Bound> y_approx;

    const CGAL::Arr_parameter_space loc = arc.location(end);
    switch (loc) {
     case CGAL::ARR_TOP_BOUNDARY: return std::make_pair(PINF, PINF); // early exit
     case CGAL::ARR_BOTTOM_BOUNDARY: return std::make_pair(MINF, MINF); // early exit
     case CGAL::ARR_LEFT_BOUNDARY:
     case CGAL::ARR_RIGHT_BOUNDARY: {
       auto asym = asymptotic_value(arc.curve(), loc, arc.arcno());
       if (const auto* ps = std::get_if<CGAL::Arr_parameter_space>(&asym)) {
         if (*ps == CGAL::ARR_BOTTOM_BOUNDARY) return std::make_pair(MINF, MINF); // early exit
         CGAL_assertion(*ps == CGAL::ARR_TOP_BOUNDARY);
         return std::make_pair(PINF, PINF); // early exit
       }
       y_approx = this->curve_kernel().approximate_absolute_1_object()(std::get<Coordinate_1>(asym), prec);
       break;
      }

     case (CGAL::ARR_INTERIOR): {
       y_approx = this->curve_kernel().approximate_absolute_y_2_object()(arc.curve_end(end).xy(), prec);
       break;
      }

     default: CGAL_error();
    } // switch

    return std::make_pair(CGAL::to_interval(y_approx.first).first, CGAL::to_interval(y_approx.second).second);
  }

  CGAL::Bbox_2 bbox(const Arc_2& cv) const {
    if (! cv.ptr()->m_bbox) {
      double PINF = std::numeric_limits<double>::infinity();
      double MINF = -PINF;

      double xmin;
      double xmax;

      double ymin = PINF; // correct as we modify shrink below
      double ymax = MINF; // correct as we modify shrink below

      // the bounds are rounded outwards, so any precision yields a box that contains the arc; a lower one is cheaper
      // but looser
      long prec = 53;

      std::pair<double, double> y_dapprox;
      auto approximate_x = this->curve_kernel().approximate_absolute_1_object();

      // left end

      // xmin for left (the end is in the interior, or on the bottom or top boundary)
      if (cv.location(CGAL::ARR_MIN_END) != CGAL::ARR_LEFT_BOUNDARY) {
        std::pair<Bound, Bound> x_approx = approximate_x(cv.curve_end_x(CGAL::ARR_MIN_END), prec);

        xmin = CGAL::to_interval(x_approx.first).first;
      }
      else {
        // left end can only lie on LEFT BOUNDARY
        xmin = MINF;
      }

      // ymin/ymax for left
      y_dapprox = this->y_interval_for_curve_end(cv, CGAL::ARR_MIN_END, prec);

      // adapt y-interval
      ymin = (CGAL::min)(ymin, y_dapprox.first);
      ymax = (CGAL::max)(ymax, y_dapprox.second);

      // right end

      // xmax for right (the end is in the interior, or on the bottom or top boundary)
      if (cv.location(CGAL::ARR_MAX_END) != CGAL::ARR_RIGHT_BOUNDARY) {
        std::pair<Bound, Bound> x_approx = approximate_x(cv.curve_end_x(CGAL::ARR_MAX_END), prec);

        xmax = CGAL::to_interval(x_approx.second).second;
      }
      else {
        // right end can only lie on RIGHT BOUNDARY
        xmax = PINF;
      }

      // ymin/ymax for right
      y_dapprox = this->y_interval_for_curve_end(cv, CGAL::ARR_MAX_END, prec);

      // adapt y-interval
      ymin = (CGAL::min)(ymin, y_dapprox.first);
      ymax = (CGAL::max)(ymax, y_dapprox.second);

      // search local extrema on a non-vertical arc

      if (! cv.is_vertical()) {
        // TODO remove algebraic notation (Mult, Solve_2)

        using Multiplicity_type = typename Curve_kernel_2::Multiplicity_type;

        std::vector<std::pair<Coordinate_2, Multiplicity_type>> pts;

        // the candidates are the points of the curve f = 0 where f_x vanishes. A factor of f that f_x shares (a
        // horizontal line, or a multiple factor) is removed from f_x, as the two curves must not share a component.
        using Polynomial_2 = typename Curve_analysis_2::Polynomial_2;
        const Polynomial_2& f = cv.curve().polynomial_2();
        Polynomial_2 fx = CGAL::differentiate(f, 0);
        if (! CGAL::is_zero(fx))
          fx = CGAL::integral_division_up_to_constant_factor(fx, CGAL::gcd_up_to_constant_factor(f, fx));
        if (CGAL::total_degree(fx) > 0)
          this->curve_kernel().solve_2_object()(cv.curve(), this->curve_kernel().construct_curve_2_object()(fx),
                                                std::back_inserter(pts));

        const std::size_t n = pts.size();
        CERR("check candidates for y-extremal points: #" << n);
        for (std::size_t i = 0; i < n; ++i) {
          const Coordinate_2& curr_xy = pts[i].first;

          // keep the candidates that lie on the arc, over the interior of its x-range
          if (this->is_in_x_range_interior(cv, curr_xy.x())) {
            Point_2 curr_pt = this->_ckva()->construct_point_2_object()(curr_xy.x(), curr_xy.curve(), curr_xy.arcno());
            if (this->_ckva()->is_on_2_object()(curr_pt, cv)) {
              CERR("update y coordinates");

              std::pair<Bound, Bound> xy_approx = this->curve_kernel().approximate_absolute_y_2_object()(curr_xy, prec);

              // adapt y-interval
              ymin = (CGAL::min)(ymin, CGAL::to_interval(xy_approx.first).first);
              ymax = (CGAL::max)(ymax, CGAL::to_interval(xy_approx.second).second);
            }
          }
        }
      }
      cv.ptr()->m_bbox = CGAL::Bbox_2(xmin, ymin, xmax, ymax);
    }

    return *(cv.ptr()->m_bbox);
  }

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
    CGAL_precondition(this->is_in_x_range(a, x));
    Point_2 pt(x, a.curve(), this->arcno(a, x));
    return pt;
  }

  template <typename T>
  Point_2 operator()(const T& x, const T& y) {
    Point_2 pt(this->curve_kernel().construct_algebraic_real_2_object()(x, y));
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

    CERR("Construct_pt_on_arc: " << CGAL::to_double(x) << ", " << arcno << ", " << c.id() << "\narc = " << arc << "\n");

    //CGAL_assertion(c.id() == arc.curve().id());
    //CGAL_assertion(arcno == this->arcno(arc, x));

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
    CGAL_precondition(this->_ckva()->compare_x_2_object()(p, q) != CGAL::EQUAL);
    Arc_2 arc(p, q, c, arcno, arcno_p, arcno_q);
    this->_check_pt_arcno_and_coprimality(arc, p, arcno_p, c);
    this->_check_pt_arcno_and_coprimality(arc, q, arcno_q, c);
    this->_fix_curve_ends_order(arc); // lexicographical order of curve ends
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
    this->_check_pt_arcno_and_coprimality(arc, origin, arcno_o, c);
    this->_fix_curve_ends_order(arc); // lexicographical order of curve ends
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
    CGAL_precondition(this->curve_kernel().compare_1_object()(origin.x(), asympt_x) != CGAL::EQUAL);
    Arc_2 arc(origin, asympt_x, inf_end, c, arcno, arcno_o);
    this->_check_pt_arcno_and_coprimality(arc, origin, arcno_o, c);
    this->_fix_curve_ends_order(arc); // lexicographical order of curve ends
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
    this->_fix_curve_ends_order(arc);
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
    CGAL_precondition(this->curve_kernel().compare_1_object()(asympt_x1, asympt_x2) != CGAL::EQUAL);
    Arc_2 arc(asympt_x1, inf_end1, asympt_x2, inf_end2, c, arcno);
    this->_fix_curve_ends_order(arc);
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
    this->_fix_curve_ends_order(arc);
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
    CGAL_precondition(this->_ckva()->compare_x_2_object()(p, q) == CGAL::EQUAL);
    CGAL_precondition(this->_ckva()->compare_xy_2_object()(p, q, true) != CGAL::EQUAL);
    Arc_2 arc(p, q, c);
    this->_check_pt_arcno_and_coprimality(arc, p, -1, c);
    this->_check_pt_arcno_and_coprimality(arc, q, -1, c);
    this->_fix_curve_ends_order(arc);
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
    this->_check_pt_arcno_and_coprimality(arc, origin, -1, c);
    this->_fix_curve_ends_order(arc);
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
    this->_fix_curve_ends_order(arc);
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

    auto construct_point_on_arc = this->_ckva()->construct_point_on_arc_2_object();
    if (! arc.is_vertical()) {
      Bound x_coord = this->boundary_in_x_range_interior(arc);
      int arcno = arc.arcno();
      const Curve_analysis_2& ca = arc.curve();
      Point_2 p = construct_point_on_arc(Coordinate_1(x_coord), ca, arcno, arc);
      return p;
    }

    Bound y_coord = 0;
    auto approximate_y = this->curve_kernel().approximate_relative_y_2_object();
    if (arc.is_finite(ARR_MIN_END)) {
      if (arc.is_finite(ARR_MAX_END)) {
        // We need torefine the interval because there is a chance that
        // the low of the upper point is below the high of the lower point.
        y_coord = this->curve_kernel().bound_between_y_2_object()(arc.curve_end(ARR_MIN_END).xy(),
                                                                     arc.curve_end(ARR_MAX_END).xy());
      }
      else {
        std::pair<Bound, Bound> approx_pair = approximate_y(arc.curve_end(ARR_MIN_END).xy(), 4);
        y_coord = approx_pair.second + Bound(1);
      }
    }
    else {
      if (arc.is_finite(ARR_MAX_END)) {
        std::pair<Bound, Bound> approx_pair = approximate_y(arc.curve_end(ARR_MAX_END).xy(), 4);
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
    Point_2 p = construct_point_on_arc(arc.x(), curve, 0, arc);
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
    CGAL_assertion_code(bool in_x_range =) this->is_in_x_range(cv, p.x(), &eq_min, &eq_max);
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
    CGAL_precondition(this->_ckva()->compare_y_at_x_2_object()(p, cv1) == CGAL::EQUAL);
    CGAL_precondition(this->_ckva()->compare_y_at_x_2_object()(p, cv2) == CGAL::EQUAL);

    // check whether both arcs indeed lie to the left of p
    CGAL_precondition((cv1.is_vertical() && cv1.location(CGAL::ARR_MIN_END) == CGAL::ARR_BOTTOM_BOUNDARY) ||
                      this->_same_arc_compare_xy(cv1, cv1._minpoint(), p) == CGAL::SMALLER);
    CGAL_precondition((cv2.is_vertical() && cv2.location(CGAL::ARR_MIN_END) == CGAL::ARR_BOTTOM_BOUNDARY) ||
                      this->_same_arc_compare_xy(cv2, cv2._minpoint(), p) == CGAL::SMALLER);
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

    // compare the arc numbers over the interval immediately to the left of p
    CGAL::Comparison_result res = this->_compare_arc_numbers(cv1, cv2, CGAL::ARR_INTERIOR, p.x(), CGAL::NEGATIVE);
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
    CGAL_precondition(this->_ckva()->compare_y_at_x_2_object()(p, cv1) == CGAL::EQUAL);
    CGAL_precondition(this->_ckva()->compare_y_at_x_2_object()(p, cv2) == CGAL::EQUAL);

    // check whether both arcs indeed lie to the left of p
    CGAL_precondition((cv1.is_vertical() && cv1.location(CGAL::ARR_MAX_END) == CGAL::ARR_TOP_BOUNDARY) ||
                      this->_same_arc_compare_xy(cv1, p, cv1._maxpoint()) == CGAL::SMALLER);
    CGAL_precondition((cv2.is_vertical() && cv2.location(CGAL::ARR_MAX_END) == CGAL::ARR_TOP_BOUNDARY) ||
                      this->_same_arc_compare_xy(cv2, p, cv2._maxpoint()) == CGAL::SMALLER);

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

    // compare the arc numbers over the interval immediately to the right of p
    CGAL::Comparison_result res = this->_compare_arc_numbers(cv1, cv2, CGAL::ARR_INTERIOR, p.x(), CGAL::POSITIVE);
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
  bool operator()(const Arc_2& cv, const Point_2& p) const { return this->is_in_x_range(cv, p.x()); }
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

    auto [a1, a2] = this->simplified(cv1, cv2);
    // distinct supporting curves implies inequality, provided the
    // coprimality condition is satisfied
    if (! a1.curve().is_identical(a2.curve())) return false;

    // here either both or none of the arcs are vertical, check for arcnos
    // equality
    if (! a1.is_vertical() && a1.arcno() != a2.arcno()) return false;

    // otherwise compare respective curve ends: supporting curves and
    // arcnos are equal => the curve ends belong to the same arc
    return ((this->_same_arc_compare_xy(a1, a1._minpoint(), a2._minpoint()) == CGAL::EQUAL &&
             this->_same_arc_compare_xy(a1, a1._maxpoint(), a2._maxpoint()) == CGAL::EQUAL));
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

    auto [a1, a2] = this->simplified(cv1, cv2);
    // arcs with coprime support do not overlap
    if (! a1.curve().is_identical(a2.curve())) return false;

    if (a1.is_vertical() != a2.is_vertical()) {
      return false; // only one arc is vertical => can't overlap
    }
    if (a1.is_vertical()) { // both arcs are vertical
      // check for x-coordinates equality
      if (this->curve_kernel().compare_1_object()(a1._minpoint().x(), a2._minpoint().x()) != CGAL::EQUAL)
        return false;

      // compare y-coordinates of min curve ends
      switch (this->_same_arc_compare_xy(a1, a1._minpoint(), a2._minpoint(), true)) {
       case CGAL::EQUAL: return true;  // this->source == cv2->source => overlap !
       case CGAL::SMALLER: // this->source < cv2->source
        // check whether this->target > cv2->source
        return (this->_same_arc_compare_xy(a1, a1._maxpoint(), a2._minpoint(), true) == CGAL::LARGER);

       case CGAL::LARGER: // this->source> cv2->source
        // check whether this->source < cv2->target
        return (this->_same_arc_compare_xy(a1, a1._minpoint(), a2._maxpoint(), true) == CGAL::SMALLER);
      }
    }
    // processing non-vertical arcs
    if (a1.arcno() != a2.arcno()) return false;

    /* At this point, we have two non-vertical arcs supported by the same
     * curve with equal arc numbers in their interior. They do overlap if
     * their x-ranges overlap. Compare only x-coordinates */
    switch (this->_same_arc_compare_xy(a1, a1._minpoint(), a2._minpoint(), false, true)) {
     case CGAL::EQUAL: return true;  // this->source == cv2->source => overlap !
     case CGAL::SMALLER: // this->source < cv2->source
      // check whether this->target > cv2->source
      return (this->_same_arc_compare_xy(a1, a1._maxpoint(), a2._minpoint(), false, true) == CGAL::LARGER);
     case CGAL::LARGER: // this->source > cv2->source
      // check whether this->source < cv2->target
      return (this->_same_arc_compare_xy(a1, a1._minpoint(), a2._maxpoint(), false, true) == CGAL::SMALLER);
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

    // arcs with disjoint x-ranges do not intersect
    auto cmp_x = this->_ckva()->compare_x_2_object();
    // determines whether the max end of an arc lies strictly to the left of the min end of another arc
    auto is_left_of = [&](const Arc_2& cv1, const Arc_2& cv2) {
      const Point_2& max1 = cv1._maxpoint();
      const Point_2& min2 = cv2._minpoint();
      if (max1.is_on_left_right() || min2.is_on_left_right()) return false; // an end at x = +-oo
      return cmp_x(max1, min2) == CGAL::SMALLER;
    };
    if (is_left_of(xcv1, xcv2) || is_left_of(xcv2, xcv1)) return false;

    std::vector<Arc_2> arcs;
    if (this->_trim_if_overlapped(xcv1, xcv2, std::back_inserter(arcs))) {
      arcs.clear();
      return true;
    }

    // process non-overlapping case
    std::vector<Intersection_point> intersections;
    this->_intersection_points(xcv1, xcv2, std::back_inserter(intersections));
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

/*! functor that computes the bounding box of an arc */
template <typename CurvedKernelViaAnalysis_2>
class Construct_bbox_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the result type
  using result_type = CGAL::Bbox_2;

  //! constructs
  Construct_bbox_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! computes a bounding box of an arc
   * \param cv the arc
   * \return the bounding box
   */
  result_type operator()(const Arc_2& cv) const { return this->bbox(cv); }
};

/*! functor that approximates points and arcs by points with double coordinates and polylines
 *
 * Arcs are approximated by adaptive subdivision: the polyline is refined at rational x-coordinates, where the
 * points on the arc are computed exactly and then rounded, until every computed point lies within the error bound
 * of the chord that it replaces, with a margin (or until a maximal subdivision depth is reached, e.g., near vertical
 * tangents). The error bound is thus not guaranteed, but met in practice. An arc whose x-range is too narrow to
 * sample in double precision is approximated by its ends and its extreme y-coordinates; the polyline is then not
 * x-monotone.
 */
template <typename CurvedKernelViaAnalysis_2>
class Approximate_2 : public Curved_kernel_via_analysis_2_functor_base<CurvedKernelViaAnalysis_2> {
public:
  //! this instance' first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the base type
  using Base = Curved_kernel_via_analysis_2_functor_base<Curved_kernel_via_analysis_2>;

  CGAL_CKvA_2_GRAB_BASE_FUNCTOR_TYPES

  //! the type of rational values
  using Bound = typename Base::Bound;

  //! the number type of the approximations
  using Approximate_number_type = double;

  //! the kernel of the approximations
  using Approximate_kernel = CGAL::Simple_cartesian<Approximate_number_type>;

  //! the type of approximate points
  using Approximate_point_2 = typename Approximate_kernel::Point_2;

  //! constructs
  Approximate_2(Curved_kernel_via_analysis_2* kernel) : Base(kernel) {}

  /*! obtains an approximation of the x-coordinate (if `i` is 0) or the y-coordinate (if `i` is 1) of a point
   * \pre `p` lies in the interior of the parameter space
   */
  Approximate_number_type operator()(const Point_2& p, int i) const {
    CGAL_precondition((i == 0) || (i == 1));
    const auto approx = p.to_double();
    return (i == 0) ? approx.first : approx.second;
  }

  /*! obtains an approximation of a point
   * \pre `p` lies in the interior of the parameter space
   */
  Approximate_point_2 operator()(const Point_2& p) const {
    const auto approx = p.to_double();
    return Approximate_point_2(approx.first, approx.second);
  }

  /*! approximates a bounded arc by a polyline, the first and last points of which approximate the endpoints of the
   * arc. Nothing is output for an arc that has an end on the boundary of the parameter space; such arcs can be
   * approximated within a bounding box.
   * \param cv the arc
   * \param error the bound on the distance between the arc and the polyline
   * \param oi the output iterator; its value type is `Approximate_point_2`
   * \param l2r whether the points are output from left to right
   * \return the past-the-end iterator of the output
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Arc_2& cv, double error, OutputIterator oi, bool l2r = true) const {
    if (! cv.is_finite(CGAL::ARR_MIN_END) || ! cv.is_finite(CGAL::ARR_MAX_END)) return oi;
    std::vector<Approximate_point_2> pts;
    approximate(cv, error, nullptr, pts);
    return output(pts, oi, l2r);
  }

  /*! approximates an arc clipped to a bounding box by a polyline, with one exception: a maximal portion of the arc
   * that lies in the vertical strip bounded by the left and right sides of the box, but below the bottom side (resp.
   * above the top side) of the box, is not removed. Instead, it is represented by points that lie on the line
   * parallel to the bottom side (resp. top side) at distance \f$h\f$ below (resp. above) it, where \f$h\f$ is the
   * height of the box. Every point inside the box that is adjacent to such a point lies on the bottom (resp. top)
   * side. Thus, the points that lie inside the box form the approximation of the arc clipped to the box, and clamping
   * the \f$y\f$-coordinates of all points to the \f$y\f$-range of the box yields an approximation of the arc portion
   * in the strip clamped to the box.
   * \param cv the arc
   * \param error the bound on the distance between the arc and the polyline
   * \param oi the output iterator; its value type is `Approximate_point_2`
   * \param bbox the bounding box
   * \param l2r whether the points are output from left to right
   * \return the past-the-end iterator of the output
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Arc_2& cv, double error, OutputIterator oi, const CGAL::Bbox_2& bbox,
                            bool l2r = true) const {
    std::vector<Approximate_point_2> pts;
    approximate(cv, error, &bbox, pts);
    return output(to_strip(pts, bbox), oi, l2r);
  }

private:
  //! the minimal and maximal subdivision depth
  static constexpr int s_min_depth = 3;
  static constexpr int s_max_depth = 16;

  //! outputs the points in the requested direction
  template <typename OutputIterator>
  static OutputIterator output(const std::vector<Approximate_point_2>& pts, OutputIterator oi, bool l2r) {
    if (l2r) return std::copy(pts.begin(), pts.end(), oi);
    return std::copy(pts.rbegin(), pts.rend(), oi);
  }

  //! clamps the y-coordinate of a point into the bounding box, if any
  static Approximate_point_2 clamp(const Approximate_point_2& p, const CGAL::Bbox_2* bbox) {
    if (bbox == nullptr) return p;
    return Approximate_point_2(p.x(), (std::min)((std::max)(p.y(), bbox->ymin()), bbox->ymax()));
  }

  /*! converts a polyline of points of an arc, which may lie below or above the bounding box, to the representation
   * of the arc in the vertical strip of the box: inserts the points where the polyline crosses the bottom and top
   * sides of the box, and projects the points below (resp. above) the box onto the line at distance \f$h\f$ below
   * (resp. above) the box, where \f$h\f$ is the height of the box; consecutive projected points that lie on the
   * same line are reduced to the extreme ones.
   */
  static std::vector<Approximate_point_2> to_strip(const std::vector<Approximate_point_2>& pts,
                                                   const CGAL::Bbox_2& bbox) {
    const double ymin = bbox.ymin();
    const double ymax = bbox.ymax();
    const double h = (ymax > ymin) ? (ymax - ymin) : 1.0;
    // -1, 0, or 1 if the point lies below, inside (including the boundary), or above the box
    auto side = [&](const Approximate_point_2& p) { return (p.y() < ymin) ? -1 : (p.y() > ymax) ? 1 : 0; };
    auto project = [&](const Approximate_point_2& p, int s) {
      return (s == 0) ? p : Approximate_point_2(p.x(), (s < 0) ? (ymin - h) : (ymax + h));
    };
    // the point where the segment pq crosses the horizontal line at yc
    auto crossing = [](const Approximate_point_2& p, const Approximate_point_2& q, double yc) {
      if ((p.x() == q.x()) || ! std::isfinite(p.y()) || ! std::isfinite(q.y())) {
        if (std::isfinite(p.y()) == std::isfinite(q.y())) return Approximate_point_2(p.x(), yc);
        return Approximate_point_2(std::isfinite(p.y()) ? p.x() : q.x(), yc);
      }
      const double x = p.x() + (yc - p.y()) * (q.x() - p.x()) / (q.y() - p.y());
      return Approximate_point_2((std::min)((std::max)(x, (std::min)(p.x(), q.x())), (std::max)(p.x(), q.x())), yc);
    };

    std::vector<Approximate_point_2> res;
    res.reserve(pts.size() + 2);
    // appends a point; a projected point between two projected points on the same line is removed
    auto append = [&](const Approximate_point_2& p, int s) {
      const std::size_t n = res.size();
      if ((s != 0) && (n >= 2) && (side(res[n - 1]) == s) && (side(res[n - 2]) == s)) res.back() = p;
      else res.push_back(p);
    };
    for (std::size_t i = 0; i < pts.size(); ++i) {
      const int si = side(pts[i]);
      if (i > 0) {
        const int sp = side(pts[i - 1]);
        // insert the crossings with the sides of the box between the previous point and this one
        if (sp != si) {
          if ((sp < 0) || (si < 0)) {
            const auto c = crossing(pts[i - 1], pts[i], ymin);
            if ((sp > 0) || (si > 0)) {
              const auto d = crossing(pts[i - 1], pts[i], ymax);
              if (sp < 0) { append(c, 0); append(d, 0); }
              else { append(d, 0); append(c, 0); }
            }
            else append(c, 0);
          }
          else append(crossing(pts[i - 1], pts[i], ymax), 0);
        }
      }
      append(project(pts[i], si), si);
    }
    return res;
  }

  //! obtains the approximate y-coordinate of the arc at a rational x-coordinate in the interior of its x-range
  std::optional<double> y_at(const Arc_2& cv, double x) const {
    if (! std::isfinite(x)) return std::nullopt;
    const Coordinate_1 xr{Bound(x)};
    if (! this->is_in_x_range_interior(cv, xr)) return std::nullopt;
    const Point_2 p = this->_ckva()->construct_point_2_object()(xr, cv);
    return p.to_double().second;
  }

  //! obtains the distance between a point and a segment
  static double distance(const Approximate_point_2& p, const Approximate_point_2& a, const Approximate_point_2& b) {
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    const double len2 = dx * dx + dy * dy;
    double t = (len2 > 0) ? ((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / len2 : 0;
    t = (std::min)((std::max)(t, 0.0), 1.0);
    const double ex = a.x() + t * dx - p.x();
    const double ey = a.y() + t * dy - p.y();
    return std::sqrt(ex * ex + ey * ey);
  }

  /*! refines the polyline between two points of the arc and appends it, excluding the first point. The points are
   * not clamped; however, the polyline is refined only as much as needed to approximate the arc clamped to the
   * bounding box (if any).
   */
  void refine(const Arc_2& cv, double error, const CGAL::Bbox_2* bbox, const Approximate_point_2& pl,
              const Approximate_point_2& pr, int depth, std::vector<Approximate_point_2>& pts) const {
    const double xm = 0.5 * (pl.x() + pr.x());
    std::optional<double> ym;
    if (depth < s_max_depth && pl.x() < xm && xm < pr.x()) ym = y_at(cv, xm);
    if (! ym) {
      pts.push_back(pr);
      return;
    }
    const Approximate_point_2 pm(xm, *ym);
    // the arc between two samples may deviate from the chord more than at its middle; hence the margin
    if (depth >= s_min_depth && distance(clamp(pm, bbox), clamp(pl, bbox), clamp(pr, bbox)) <= 0.5 * error) {
      pts.push_back(pr);
      return;
    }
    refine(cv, error, bbox, pl, pm, depth + 1, pts);
    refine(cv, error, bbox, pm, pr, depth + 1, pts);
  }

  /*! obtains the first (if `ce` is the min end) or last point of the polyline, or nothing if the arc does not
   * reach into the bounding box. `x` is the (clipped) x-coordinate of the end of the polyline, and `inner` is the
   * x-coordinate of the other end.
   */
  std::optional<Approximate_point_2> end_point(const Arc_2& cv, CGAL::Arr_curve_end ce, double x, double inner,
                                               const CGAL::Bbox_2* bbox) const {
    // the box cuts the arc only if its side lies in the interior of the x-range of the arc
    const bool clipped = (bbox != nullptr) && ((ce == CGAL::ARR_MIN_END) ? (x == bbox->xmin()) : (x == bbox->xmax())) &&
      this->is_in_x_range_interior(cv, Coordinate_1(Bound(x)));
    if (cv.is_finite(ce) && ! clipped) {
      const auto approx = cv.curve_end(ce).to_double();
      return Approximate_point_2(approx.first, approx.second);
    }
    const CGAL::Arr_parameter_space loc = cv.location(ce);
    if (! clipped && (loc == CGAL::ARR_BOTTOM_BOUNDARY || loc == CGAL::ARR_TOP_BOUNDARY)) {
      // approach the vertical asymptote until the arc leaves the bounding box
      std::optional<Approximate_point_2> last;
      for (int k = 1; k < 53; ++k) {
        const double xk = x + (inner - x) * std::ldexp(1.0, -k);
        if (xk == x) break;
        const auto yk = y_at(cv, xk);
        if (! yk) continue;
        last = Approximate_point_2(xk, *yk);
        if ((loc == CGAL::ARR_BOTTOM_BOUNDARY && *yk <= bbox->ymin()) ||
            (loc == CGAL::ARR_TOP_BOUNDARY && *yk >= bbox->ymax()))
          break;
      }
      return last;
    }
    const auto y = y_at(cv, x);
    if (! y) return std::nullopt;
    return Approximate_point_2(x, *y);
  }

  //! approximates an arc, possibly within a bounding box
  void approximate(const Arc_2& cv, double error, const CGAL::Bbox_2* bbox,
                   std::vector<Approximate_point_2>& pts) const {
    constexpr double inf = std::numeric_limits<double>::infinity();
    if (cv.is_vertical()) {
      const double x = CGAL::to_double(cv.x());
      if (bbox != nullptr && (x < bbox->xmin() || x > bbox->xmax())) return;
      for (auto ce : {CGAL::ARR_MIN_END, CGAL::ARR_MAX_END}) {
        double y;
        if (cv.is_finite(ce)) y = cv.curve_end(ce).to_double().second;
        else y = (cv.location(ce) == CGAL::ARR_BOTTOM_BOUNDARY) ? -inf : inf;
        pts.emplace_back(x, y);
      }
      return;
    }

    // the x-range of the arc, clipped to the bounding box
    double xmin = (cv.location(CGAL::ARR_MIN_END) == CGAL::ARR_LEFT_BOUNDARY) ?
      -inf : CGAL::to_double(cv.curve_end_x(CGAL::ARR_MIN_END));
    double xmax = (cv.location(CGAL::ARR_MAX_END) == CGAL::ARR_RIGHT_BOUNDARY) ?
      inf : CGAL::to_double(cv.curve_end_x(CGAL::ARR_MAX_END));

    // an arc whose ends are at the same or at consecutive doubles in x cannot be sampled; it is approximated through
    // its extreme y-coordinates, which are known from its bounding box
    if ((xmin <= xmax) && (std::nextafter(xmin, inf) >= xmax)) {
      if ((bbox != nullptr) && ((xmax < bbox->xmin()) || (xmin > bbox->xmax()))) return;
      const CGAL::Bbox_2 cv_bbox = this->_ckva()->construct_bbox_2_object()(cv);
      if (cv.is_finite(CGAL::ARR_MIN_END))
        pts.emplace_back(xmin, cv.curve_end(CGAL::ARR_MIN_END).to_double().second);
      pts.emplace_back(xmin, cv_bbox.ymin());
      pts.emplace_back(xmin, cv_bbox.ymax());
      if (cv.is_finite(CGAL::ARR_MAX_END))
        pts.emplace_back(xmax, cv.curve_end(CGAL::ARR_MAX_END).to_double().second);
      return;
    }

    if (bbox != nullptr) {
      xmin = (std::max)(xmin, bbox->xmin());
      xmax = (std::min)(xmax, bbox->xmax());
    }
    if (! (xmin < xmax)) return;

    // an arc that lies above or below the bounding box is represented by points above or below the box; this is
    // known from the bounding box of the arc, which contains the arc and is cached in it, without sampling the arc
    if (bbox != nullptr) {
      const CGAL::Bbox_2 cv_bbox = this->_ckva()->construct_bbox_2_object()(cv);
      const bool above = (cv_bbox.ymin() > bbox->ymax());
      if (above || (cv_bbox.ymax() < bbox->ymin())) {
        const double y = above ? cv_bbox.ymin() : cv_bbox.ymax();
        pts.emplace_back(xmin, y);
        pts.emplace_back(xmax, y);
        return;
      }
    }

    const auto first = end_point(cv, CGAL::ARR_MIN_END, xmin, xmax, bbox);
    const auto last = end_point(cv, CGAL::ARR_MAX_END, xmax, xmin, bbox);
    if (! first || ! last) return;
    pts.push_back(*first);
    refine(cv, error, bbox, *first, *last, 0, pts);
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
    if (this->_trim_if_overlapped(cv1, cv2, std::back_inserter(arcs))) {
      for (const auto& item : arcs) *oi++ = item;
      return oi;
    }
    // process non-overlapping case
    std::vector<Intersection_point> vec;
    this->_intersection_points(cv1, cv2, std::back_inserter(vec));
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
    CGAL_precondition(this->_ckva()->compare_y_at_x_2_object()(p, cv) == CGAL::EQUAL);
    CGAL_precondition(this->_ckva()->compare_y_at_x_2_object()(q, cv) == CGAL::EQUAL);

    return this->_trim(cv, p, q);
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
    CGAL_precondition(this->_ckva()->compare_y_at_x_2_object()(p, cv) == CGAL::EQUAL);
    // check that p is not an end-point of the arc
    CGAL_precondition(this->_same_arc_compare_xy(cv, cv._minpoint(), p) != CGAL::EQUAL);
    CGAL_precondition(this->_same_arc_compare_xy(cv, cv._maxpoint(), p) != CGAL::EQUAL);

    CERR("\nsplit\n");
    c1 = this->_replace_endpoints(cv, cv._minpoint(), p, -1, (cv.is_vertical() ? -1 : cv.arcno())).first;
    c2 = this->_replace_endpoints(cv, p, cv._maxpoint(), (cv.is_vertical() ? -1 : cv.arcno()), -1).first;
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
    if (this->_ckva()->do_overlap_2_object()(cv1, cv2)) return false;

    // touch in at most one point now and supporting curves are simplified
    // both arcs must be either vertical or not
    if (! cv1.curve().is_identical(cv2.curve()) || cv1.is_vertical() != cv2.is_vertical()) return false;

    // if both arcs are non-vertical => they must have equal arcnos
    // to be mergeable
    if (! cv1.is_vertical() && cv1.arcno() != cv2.arcno()) return false;

    // for non-vertical arcs arc numbers are equal => can use same_arc_cmp
    bool max_min = (this->_same_arc_compare_xy(cv1, cv1._maxpoint(), cv2._minpoint()) == CGAL::EQUAL);
    bool min_max = false;
    if (! max_min) { // both cases cannot happen simultaneously
      min_max = (this->_same_arc_compare_xy(cv1, cv1._minpoint(), cv2._maxpoint()) == CGAL::EQUAL);

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
    else if (this->interval_id(cv1) != this->interval_id(cv2)) return false; // non-vertical case

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
    CGAL_precondition(this->_ckva()->are_mergeable_2_object()(cv1, cv2));
    auto [a1, a2] = this->simplified(cv1, cv2);

    Point_2 src;
    Point_2 tgt;
    int arcno_s = -1;
    int arcno_t = -1;
    bool replace_src; // true if cv2 < *this otherwise *this arc < cv2 arc
    // arcs are mergeable => they have one common finite end-point
    replace_src = (this->_same_arc_compare_xy(a1, a1._minpoint(), a2._maxpoint()) == CGAL::EQUAL);
    src = (replace_src ? a2._minpoint() : a1._minpoint());
    tgt = (replace_src ? a1._maxpoint() : a2._maxpoint());

    if (! a1.is_vertical()) {
      arcno_s = (replace_src ? a2.arcno(CGAL::ARR_MIN_END) : a1.arcno(CGAL::ARR_MIN_END));
      arcno_t = (replace_src ? a1.arcno(CGAL::ARR_MAX_END) : a2.arcno(CGAL::ARR_MAX_END));
    }
    Arc_2 arc = this->_replace_endpoints(a1, src, tgt, arcno_s, arcno_t).first;
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
    if (this->is_in_x_range(cv, p.x())) return this->_ckva()->compare_y_at_x_2_object()(p, cv) == CGAL::EQUAL;
    return false;

    // This is the old code that seems to interpret the arc as open
    // In particular, it returns false for vertical arcs.
    // (Michael Hemmer)
    //       bool is_left, is_right;
    //       result_type res =
    //         (this->is_in_x_range(cv, p.x(),&is_left,&is_right)) &&
    //         !(is_left) &&
    //         !(is_right) &&
    //         (this->_ckva()->compare_y_at_x_2_object()(p, cv) == CGAL::EQUAL);
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
      res =
        this->curve_kernel().compare_1_object()(std::get<Algebraic_real_1>(asym1), std::get<Algebraic_real_1>(asym2));
    }

    if (res != EQUAL) {
      CERR("result: " << res << "\n");
      return res;
    }

    // in this setting same handling as for +/-oo ?
    res = this->_compare_arc_numbers(cv1, cv2, loc1);
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
    CGAL_precondition_code(CGAL::Arr_parameter_space loc1 = cv1.location(ce1));
    CGAL_precondition_code(CGAL::Arr_parameter_space loc2 = cv2.location(ce2));
    CGAL_precondition(cv1.is_on_bottom_top(loc1));
    CGAL_precondition(cv2.is_on_bottom_top(loc2));

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
    CGAL_precondition(this->_ckva()->compare_x_on_boundary_2_object()(cv1, ce, cv2, ce) == CGAL::EQUAL);

    CGAL_precondition(loc1 == loc2);

    Coordinate_1 x0(cv1.curve_end_x(ce));
    CGAL::Comparison_result res = this->_compare_arc_numbers(cv1, cv2, CGAL::ARR_INTERIOR, x0,
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
  CGAL_CKvA_2_functor_cons(Construct_bbox_2, construct_bbox_2_object);
  CGAL_CKvA_2_functor_cons(Approximate_2, approximate_2_object);

  CGAL_CKvA_2_functor_pred(Compare_endpoints_xy_2, compare_endpoints_xy_2_object);

  CGAL_CKvA_2_functor_cons(Construct_opposite_2, construct_opposite_2_object);

#undef CGAL_CKvA_2_functor_pred
#undef CGAL_CKvA_2_functor_cons

}; // Curved_kernel_via_analysis_2_functors

} // namespace internal
} // namespace CGAL

#endif
