// Copyright (c) 2006,2007,2008,2009,2010,2011 Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s): Michael Kerber    <mkerber@mpi-inf.mpg.de>
//
// ============================================================================

#ifndef CGAL_ARR_ALGEBRAIC_SEGMENT_TRAITS_H
#define CGAL_ARR_ALGEBRAIC_SEGMENT_TRAITS_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <algorithm>
#include <iterator>
#include <list>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/disable_warnings.h>
#include <CGAL/Algebraic_kernel_d_1.h>
#include <CGAL/Algebraic_kernel_d/Algebraic_curve_kernel_2.h>
#include <CGAL/Arr_enums.h>
#include <CGAL/assertions.h>
#include <CGAL/enum.h>
#include <CGAL/Fraction_traits.h>
#include <CGAL/Polynomial_traits_d.h>
#include <CGAL/Curved_kernel_via_analysis_2/Curved_kernel_via_analysis_2_functors.h>
#include <CGAL/Curved_kernel_via_analysis_2/Curved_kernel_via_analysis_2_impl.h>

namespace CGAL {

template <typename Coefficient_>
class Arr_algebraic_segment_traits_2 {
public:
  enum Site_of_point { POINT_IN_INTERIOR = 0, MIN_ENDPOINT = -1, MAX_ENDPOINT = 1 };

  // Template argument

  using Coefficient = Coefficient_;

  // 'derivation'
  using Algebraic_kernel_d_1 = CGAL::Algebraic_kernel_d_1<Coefficient>;
  using Algebraic_kernel_d_2 = CGAL::Algebraic_curve_kernel_2<Algebraic_kernel_d_1>;
  using CKvA_2 = CGAL::Curved_kernel_via_analysis_2<Algebraic_kernel_d_2>;
  using Self = CGAL::Arr_algebraic_segment_traits_2<Coefficient>;

  // The traits is stateless; all special member functions are implicit.

  // public types
  using Algebraic_real_1 = typename Algebraic_kernel_d_2::Algebraic_real_1;
  using Bound = typename Algebraic_kernel_d_2::Bound;
  using Polynomial_2 = typename Algebraic_kernel_d_2::Polynomial_2;

  /// public types for AosTraits_2
  using Curve_2 = typename Algebraic_kernel_d_2::Curve_analysis_2;
  using X_monotone_curve_2 = typename CKvA_2::Arc_2;
  using Point_2 = typename CKvA_2::Point_2;

  using Has_left_category = typename CKvA_2::Has_left_category;
  using Has_merge_category = typename CKvA_2::Has_merge_category;

  using Left_side_category = typename CKvA_2::Left_side_category;
  using Bottom_side_category = typename CKvA_2::Bottom_side_category;
  using Top_side_category = typename CKvA_2::Top_side_category;
  using Right_side_category = typename CKvA_2::Right_side_category;

  using Multiplicity = typename CKvA_2::Multiplicity;

  using Compare_x_2 = typename CKvA_2::Compare_x_2;
  Compare_x_2 compare_x_2_object() const { return CKvA_2::instance().compare_x_2_object(); }

  using Compare_xy_2 = typename CKvA_2::Compare_xy_2;
  Compare_xy_2 compare_xy_2_object() const { return CKvA_2::instance().compare_xy_2_object(); }

  using Compare_endpoints_xy_2 = typename CKvA_2::Compare_endpoints_xy_2;
  Compare_endpoints_xy_2 compare_endpoints_xy_2_object() const
  { return CKvA_2::instance().compare_endpoints_xy_2_object(); }

  using Equal_2 = typename CKvA_2::Equal_2;
  Equal_2 equal_2_object() const { return CKvA_2::instance().equal_2_object(); }

  using Parameter_space_in_y_2 = typename CKvA_2::Parameter_space_in_y_2;
  Parameter_space_in_y_2 parameter_space_in_y_2_object() const
  { return CKvA_2::instance().parameter_space_in_y_2_object(); }

  using Compare_y_near_boundary_2 = typename CKvA_2::Compare_y_near_boundary_2;
  Compare_y_near_boundary_2 compare_y_near_boundary_2_object() const
  { return CKvA_2::instance().compare_y_near_boundary_2_object(); }

  using Parameter_space_in_x_2 = typename CKvA_2::Parameter_space_in_x_2;
  Parameter_space_in_x_2 parameter_space_in_x_2_object() const
  { return CKvA_2::instance().parameter_space_in_x_2_object(); }

  using Compare_x_on_boundary_2 = typename CKvA_2::Compare_x_on_boundary_2;
  Compare_x_on_boundary_2 compare_x_on_boundary_2_object() const
  { return CKvA_2::instance().compare_x_on_boundary_2_object(); }

  using Compare_x_near_boundary_2 = typename CKvA_2::Compare_x_near_boundary_2;
  Compare_x_near_boundary_2 compare_x_near_boundary_2_object() const
  { return CKvA_2::instance().compare_x_near_boundary_2_object(); }

  using Construct_min_vertex_2 = typename CKvA_2::Construct_min_vertex_2;
  Construct_min_vertex_2 construct_min_vertex_2_object() const
  { return CKvA_2::instance().construct_min_vertex_2_object(); }

  using Construct_max_vertex_2 = typename CKvA_2::Construct_max_vertex_2;
  Construct_max_vertex_2 construct_max_vertex_2_object() const
  { return CKvA_2::instance().construct_max_vertex_2_object(); }

  using Construct_opposite_2 = typename CKvA_2::Construct_opposite_2;
  Construct_opposite_2 construct_opposite_2_object() const { return CKvA_2::instance().construct_opposite_2_object(); }

  using Is_vertical_2 = typename CKvA_2::Is_vertical_2;
  Is_vertical_2 is_vertical_2_object() const { return CKvA_2::instance().is_vertical_2_object(); }

  using Compare_y_at_x_2 = typename CKvA_2::Compare_y_at_x_2;
  Compare_y_at_x_2 compare_y_at_x_2_object() const { return CKvA_2::instance().compare_y_at_x_2_object(); }

  using Compare_y_at_x_left_2 = typename CKvA_2::Compare_y_at_x_left_2;
  Compare_y_at_x_left_2 compare_y_at_x_left_2_object() const
  { return CKvA_2::instance().compare_y_at_x_left_2_object(); }

  using Compare_y_at_x_right_2 = typename CKvA_2::Compare_y_at_x_right_2;
  Compare_y_at_x_right_2 compare_y_at_x_right_2_object() const
  { return CKvA_2::instance().compare_y_at_x_right_2_object(); }

  using Intersect_2 = typename CKvA_2::Intersect_2;
  Intersect_2 intersect_2_object() const { return CKvA_2::instance().intersect_2_object(); }

  using Do_intersect_2 = typename CKvA_2::Do_intersect_2;
  Do_intersect_2 do_intersect_2_object() const { return CKvA_2::instance().do_intersect_2_object(); }

  using Split_2 = typename CKvA_2::Split_2;
  Split_2 split_2_object() const { return CKvA_2::instance().split_2_object(); }

  using Are_mergeable_2 = typename CKvA_2::Are_mergeable_2;
  Are_mergeable_2 are_mergeable_2_object() const { return CKvA_2::instance().are_mergeable_2_object(); }

  using Merge_2 = typename CKvA_2::Merge_2;
  Merge_2 merge_2_object() const { return CKvA_2::instance().merge_2_object(); }

  using Make_x_monotone_2 = typename CKvA_2::Make_x_monotone_2;
  Make_x_monotone_2 make_x_monotone_2_object() const { return Make_x_monotone_2(&CKvA_2::instance()); }

  using Is_on_2 = typename CKvA_2::Is_on_2;
  Is_on_2 is_on_2_object() const { return Is_on_2(&CKvA_2::instance()); }

  using Construct_point_2 = typename CKvA_2::Construct_point_2;
  Construct_point_2 construct_point_2_object() const { return Construct_point_2(&CKvA_2::instance()); }

  class Construct_x_monotone_segment_2 :
    public internal::CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base<CKvA_2> {
  public:
    using Base = internal::CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base<CKvA_2>;
    using Optional_point_2 = std::optional<Point_2>;

    Construct_x_monotone_segment_2(CKvA_2* kernel) : Base(kernel) {}

  private:
    // helper function, returning the number of incident branches of a point wrt to a curve
    std::pair<std::pair<int, int>, bool> branch_numbers_and_vertical(const Curve_2& cv, const Point_2& p) const {
      auto equal = this->_ckva()->equal_2_object();

      Algebraic_real_1 x = p.x();
      int no;
      bool is_event;
      cv.x_to_index(x, no, is_event);
      if (! is_event) return std::make_pair(std::make_pair(1, 1), false);
      typename Curve_2::Status_line_1 status_line = cv.status_line_at_event(no);
      bool vertical = status_line.covers_line();
      for (int i = 0; i < status_line.number_of_events(); i++) {
        Point_2 curr_point(x, cv, i);
        if (equal(p, curr_point)) return std::make_pair(status_line.number_of_incident_branches(i), vertical);
      }
      return std::make_pair(std::make_pair(0, 0), vertical);
    }

    // abbreviation for convenience
    bool is_one_one(const Curve_2& cv, const Point_2& p) const {
      std::pair<std::pair<int, int>, bool> branches = branch_numbers_and_vertical(cv, p);
      return branches.first.first == 1 && branches.first.second == 1 && ! branches.second;
    }

    template <typename OutputIterator>
    OutputIterator x_monotone_segment(const Curve_2& cv, const Point_2& p, const Optional_point_2& start,
                                      const Optional_point_2& end, OutputIterator out) const {
      using Make_x_monotone_result = std::variant<Point_2, X_monotone_curve_2>;

      // CGAL_assertion(is_one_one(cv,p));

      std::list<X_monotone_curve_2> segs;

      auto on_arc = this->_ckva()->is_on_2_object();
      auto left = this->_ckva()->construct_min_vertex_2_object();
      auto right = this->_ckva()->construct_max_vertex_2_object();
      auto equal = this->_ckva()->equal_2_object();

      std::vector<Make_x_monotone_result> arcs;
      this->_ckva()->make_x_monotone_2_object()(cv, std::back_inserter(arcs));

      // obtains a predicate that holds for x-monotone arcs (not isolated points) that contain q
      auto contains = [&](const Point_2& q) {
        return [&, q](const Make_x_monotone_result& obj) {
          const auto* xcv = std::get_if<X_monotone_curve_2>(&obj);
          return xcv && on_arc(q, *xcv);
        };
      };

      auto it = std::find_if(arcs.begin(), arcs.end(), contains(p));
      CGAL_precondition(it != arcs.end()); // p must lie on cv
      if (it == arcs.end()) return out;
      const auto* it_seg_p = &std::get<X_monotone_curve_2>(*it);

      bool left_on_arc = start && on_arc(start.value(), *it_seg_p);
      bool right_on_arc = end && on_arc(end.value(), *it_seg_p);

      if (left_on_arc && right_on_arc) segs.push_back(it_seg_p->trim(start.value(), end.value()));
      if (left_on_arc && (! right_on_arc)) {
        if (! it_seg_p->is_finite(CGAL::ARR_MAX_END) || ! equal(start.value(), right(*it_seg_p))) {
          if (it_seg_p->is_finite(CGAL::ARR_MIN_END) && equal(start.value(), left(*it_seg_p)))
            segs.push_back(*it_seg_p);
          else {
            X_monotone_curve_2 split1;
            X_monotone_curve_2 split2;
            it_seg_p->split(start.value(), split1, split2);
            segs.push_back(split2);
          }
        }
      }
      if ((! left_on_arc) && right_on_arc) {
        if (! it_seg_p->is_finite(CGAL::ARR_MIN_END) || ! equal(left(*it_seg_p), end.value())) {
          if (it_seg_p->is_finite(CGAL::ARR_MAX_END) && equal(end.value(), right(*it_seg_p)))
            segs.push_back(*it_seg_p);
          else {
            X_monotone_curve_2 split1;
            X_monotone_curve_2 split2;
            it_seg_p->split(end.value(), split1, split2);
            segs.push_back(split1);
          }
        }
      }
      if ((! left_on_arc) && (! right_on_arc)) segs.push_back(*it_seg_p);

      // Extend the segment to the left, arc by arc, through one-one points, until start is reached.
      if (! left_on_arc) {
        auto cur = std::make_reverse_iterator(std::next(it)); // *cur is the arc that contains p
        const X_monotone_curve_2* xcv = it_seg_p;
        while (true) {
          if (! xcv->is_finite(CGAL::ARR_MIN_END) || ! is_one_one(cv, left(*xcv))) {
            CGAL_assertion(! start); // the branch cannot be continued, so start must not be given
            break;
          }
          cur = std::find_if(std::next(cur), arcs.rend(), contains(left(*xcv)));
          CGAL_assertion(cur != arcs.rend());
          if (cur == arcs.rend()) break;
          xcv = &std::get<X_monotone_curve_2>(*cur);
          if (start && on_arc(start.value(), *xcv)) {
            segs.push_front(xcv->trim(start.value(), right(*xcv)));
            break;
          }
          segs.push_front(*xcv);
        }
      }

      // Extend the segment to the right, arc by arc, through one-one points, until end is reached.
      if (! right_on_arc) {
        auto cur = it; // *cur is the arc that contains p
        const X_monotone_curve_2* xcv = it_seg_p;
        while (true) {
          if (! xcv->is_finite(CGAL::ARR_MAX_END) || ! is_one_one(cv, right(*xcv))) {
            CGAL_assertion(! end); // the branch cannot be continued, so end must not be given
            break;
          }
          cur = std::find_if(std::next(cur), arcs.end(), contains(right(*xcv)));
          CGAL_assertion(cur != arcs.end());
          if (cur == arcs.end()) break;
          xcv = &std::get<X_monotone_curve_2>(*cur);
          if (end && on_arc(end.value(), *xcv)) {
            segs.push_back(xcv->trim(left(*xcv), end.value()));
            break;
          }
          segs.push_back(*xcv);
        }
      }

      std::copy(segs.begin(), segs.end(), out);
      return out;
    }

  public:
    //
    template <typename OutputIterator>
    OutputIterator operator()(const Curve_2& cv, const Point_2& p, Site_of_point site_of_p, OutputIterator out) const {
      if (site_of_p == POINT_IN_INTERIOR) return x_monotone_segment(cv, p, std::nullopt, std::nullopt, out);
      else if (site_of_p == MIN_ENDPOINT) return x_monotone_segment(cv, p, Optional_point_2(p), std::nullopt, out);
      CGAL_assertion(site_of_p == MAX_ENDPOINT);
      return x_monotone_segment(cv, p, std::nullopt, Optional_point_2(p), out);
    }

    //
    template <typename OutputIterator>
    OutputIterator operator()(const Curve_2& cv, const Point_2& end_left, const Point_2& end_right,
                              OutputIterator out) const {
      CGAL_assertion((branch_numbers_and_vertical(cv, end_left).first.second == 1 &&
                      ! branch_numbers_and_vertical(cv, end_left).second) ||
                     (branch_numbers_and_vertical(cv, end_right).first.first == 1 &&
                      ! branch_numbers_and_vertical(cv, end_right).second) ||
                     (branch_numbers_and_vertical(cv, end_left).first.second == 0 &&
                      branch_numbers_and_vertical(cv, end_left).second) ||
                     (branch_numbers_and_vertical(cv, end_right).first.first == 0 &&
                      branch_numbers_and_vertical(cv, end_right).second));
      CGAL_assertion_code(if ((branch_numbers_and_vertical(cv, end_left).first.second == 0 &&
                               branch_numbers_and_vertical(cv, end_left).second) ||
                              (branch_numbers_and_vertical(cv, end_right).first.first == 0 &&
                               branch_numbers_and_vertical(cv, end_right).second)) {
        auto equal_x = this->_ckva()->compare_x_2_object(); )
                          CGAL_assertion(equal_x(end_left,end_right) == CGAL::EQUAL);
                          CGAL_assertion_code(});

      if ((branch_numbers_and_vertical(cv, end_left).first.second == 0 &&
           branch_numbers_and_vertical(cv, end_left).second) ||
          (branch_numbers_and_vertical(cv, end_left).first.second == 1 &&
           ! branch_numbers_and_vertical(cv, end_left).second)) {
        return x_monotone_segment(cv, end_left, Optional_point_2(end_left), Optional_point_2(end_right), out);
      }
      else {
        return x_monotone_segment(cv, end_right, Optional_point_2(end_left), Optional_point_2(end_right), out);
      }
    }

    template <typename OutputIterator>
    OutputIterator operator()(const Point_2& p, const Point_2& q, OutputIterator out) const {
      bool same_x = (this->_ckva()->compare_x_2_object()(p, q) == CGAL::EQUAL);
      if (same_x) {
        Algebraic_real_1 x = p.x();
        Polynomial_2 f = Polynomial_2(x.polynomial());
        Curve_2 cv = this->_ckva()->kernel().construct_curve_2_object()(f);
        *out++ = typename CKvA_2::Arc_2(p, q, cv);
        return out;
      }
      Algebraic_real_1 px = this->_ckva()->kernel().compute_x_2_object()(p.xy());
      Algebraic_real_1 py = this->_ckva()->kernel().compute_y_2_object()(p.xy());
      Algebraic_real_1 qx = this->_ckva()->kernel().compute_x_2_object()(q.xy());
      Algebraic_real_1 qy = this->_ckva()->kernel().compute_y_2_object()(q.xy());
      bool p_rat = px.is_rational() && py.is_rational();
      bool q_rat = qx.is_rational() && qy.is_rational();

      if (p_rat && q_rat) {
        using Rational = typename Algebraic_real_1::Rational;

        Rational pxr = px.rational();
        Rational pyr = py.rational();
        Rational qxr = qx.rational();
        Rational qyr = qy.rational();

        using FT = CGAL::Fraction_traits<Rational>;

        typename FT::Decompose decompose;

        using Numerator = typename FT::Numerator_type;
        using Denominator = typename FT::Denominator_type;

        Rational term_at_y = qxr - pxr;
        Rational term_at_x = -qyr + pyr;
        Rational term_at_1 = pxr * qyr - pyr * qxr;
        Denominator denom_curr;
        Numerator term_at_y_int;
        Numerator term_at_x_int;
        Numerator term_at_1_int;
        decompose(term_at_y, term_at_y_int, denom_curr);
        term_at_x = term_at_x * denom_curr;
        term_at_1 = term_at_1 * denom_curr;
        decompose(term_at_x, term_at_x_int, denom_curr);
        term_at_y_int = term_at_y_int * denom_curr;
        term_at_1 = term_at_1 * denom_curr;
        decompose(term_at_1, term_at_1_int, denom_curr);
        term_at_y_int = term_at_y_int * denom_curr;
        term_at_x_int = term_at_x_int * denom_curr;

        using Polynomial_1 = typename CGAL::Polynomial_traits_d<Polynomial_2>::Coefficient_type;

        Polynomial_2 pol(Polynomial_1(term_at_1_int, term_at_x_int), Polynomial_1(term_at_y_int));
        Curve_2 curve = this->_ckva()->kernel().construct_curve_2_object()(pol);

        CGAL_assertion(this->_ckva()->is_on_2_object()(p, curve));
        CGAL_assertion(this->_ckva()->is_on_2_object()(q, curve));
        return this->operator()(curve, p, q, out);
      }
      CGAL_precondition(same_x || (p_rat && q_rat));
      return out;
    }
  };

  Construct_x_monotone_segment_2 construct_x_monotone_segment_2_object() const
  { return Construct_x_monotone_segment_2(&CKvA_2::instance()); }

  /* class Construct_vertical_segment_2 :
   *   public internal::CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base<CKvA_2> {
   *
   * public:
   *   using Base = internal::CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base<CKvA_2>;
   *
   *   Construct_vertical_segment_2(CKvA_2* kernel) : Base(kernel) {}
   *
   * public:
   *   X_monotone_curve_2 operator() (const Point_2& p, const Point_2& q) {
   *     Algebraic_real_1 x = p.x();
   *     Polynomial_2 f = Polynomial_2(x.polynomial());
   *     Curve_2 cv = this->_ckva()->kernel().construct_curve_2_object() (f);
   *     return typename CKvA_2::Arc_2(p,q,cv);
   *   }
   *
   *   template <typename OutputIterator> OutputIterator
   *   operator()(const Point_2& p, Site_of_point site_of_p) const {
   *     Algebraic_real_1 x = p.x();
   *     Polynomial_2 f = Polynomial_2(x.polynomial());
   *     Curve_2 cv = this->_ckva().kernel().construct_curve_2_object() (f);
   *
   *     if (site_of_p == POINT_IN_INTERIOR) return typename CKvA_2::Arc_2(x, cv);
   *     else if (site_of_p == MIN_ENDPOINT) return typename CKvA_2::Arc_2(p, ARR_MIN_END, cv);
   *     CGAL_assertion(site_of_p == MAX_ENDPOINT);
   *     return typename CKvA_2::Arc_2(p, ARR_MAX_END, cv);
   *   }
   * };
   *
   * Construct_vertical_segment_2 construct_vertical_segment_2_object() const
   * { return Construct_vertical_segment_2(&CKvA_2::instance()); }
   */

  class Construct_curve_2 :
    public internal::CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base<CKvA_2> {
  public:
    using Base = internal::CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base<CKvA_2>;

    Construct_curve_2(CKvA_2* kernel) : Base(kernel) {}

    Curve_2 operator()(const Polynomial_2& p) const { return this->_ckva()->kernel().construct_curve_2_object()(p); }
  };

  Construct_curve_2 construct_curve_2_object() const { return Construct_curve_2(&CKvA_2::instance()); }

  // // additional functionality (for not introducing a "general" arc)
  //
  // class Connect_points_2 {
  //   template <typename OutputIterator>
  //   OutputIterator operator()(Curve_2, Point_2, Point_2, ..., OutputIterator) {
  //     while (false || true) *oi++ = X_monotone_curve_2(...);
  //     return oi;
  //   }
  //
  //   template< class OutputIterator >
  //   OutputIterator operator()(Curve_2, Point_2, Point_2, int, ..., OutputIterator) {
  //     while (false || true) *oi++ = X_monotone_curve_2(...);
  //     return oi;
  //   }
  // };
};

} // namespace CGAL

#include <CGAL/enable_warnings.h>

#endif
