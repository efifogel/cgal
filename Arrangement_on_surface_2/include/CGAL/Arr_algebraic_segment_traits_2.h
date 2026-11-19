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
#include <memory>
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

  //! the type of a shared pointer to the curved kernel
  using Shared_ckva = std::shared_ptr<CKvA_2>;

  /*! constructs default. The functors of this traits use the curved kernel of the calling thread, that is,
   * \c CKvA_2::instance().
   */
  Arr_algebraic_segment_traits_2() = default;

  /*! constructs from a curved kernel, which all functors obtained from this traits use.
   */
  explicit Arr_algebraic_segment_traits_2(Shared_ckva ckva) : m_ckva(std::move(ckva)) {}

  /*! obtains the curved kernel that the functors of this traits use.
   */
  CKvA_2* curved_kernel() const { return m_ckva ? m_ckva.get() : &CKvA_2::instance(); }

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
  Compare_x_2 compare_x_2_object() const { return curved_kernel()->compare_x_2_object(); }

  using Compare_xy_2 = typename CKvA_2::Compare_xy_2;
  Compare_xy_2 compare_xy_2_object() const { return curved_kernel()->compare_xy_2_object(); }

  using Compare_endpoints_xy_2 = typename CKvA_2::Compare_endpoints_xy_2;
  Compare_endpoints_xy_2 compare_endpoints_xy_2_object() const
  { return curved_kernel()->compare_endpoints_xy_2_object(); }

  using Equal_2 = typename CKvA_2::Equal_2;
  Equal_2 equal_2_object() const { return curved_kernel()->equal_2_object(); }

  using Parameter_space_in_y_2 = typename CKvA_2::Parameter_space_in_y_2;
  Parameter_space_in_y_2 parameter_space_in_y_2_object() const
  { return curved_kernel()->parameter_space_in_y_2_object(); }

  using Compare_y_near_boundary_2 = typename CKvA_2::Compare_y_near_boundary_2;
  Compare_y_near_boundary_2 compare_y_near_boundary_2_object() const
  { return curved_kernel()->compare_y_near_boundary_2_object(); }

  using Parameter_space_in_x_2 = typename CKvA_2::Parameter_space_in_x_2;
  Parameter_space_in_x_2 parameter_space_in_x_2_object() const
  { return curved_kernel()->parameter_space_in_x_2_object(); }

  using Compare_x_on_boundary_2 = typename CKvA_2::Compare_x_on_boundary_2;
  Compare_x_on_boundary_2 compare_x_on_boundary_2_object() const
  { return curved_kernel()->compare_x_on_boundary_2_object(); }

  using Compare_x_near_boundary_2 = typename CKvA_2::Compare_x_near_boundary_2;
  Compare_x_near_boundary_2 compare_x_near_boundary_2_object() const
  { return curved_kernel()->compare_x_near_boundary_2_object(); }

  using Construct_min_vertex_2 = typename CKvA_2::Construct_min_vertex_2;
  Construct_min_vertex_2 construct_min_vertex_2_object() const
  { return curved_kernel()->construct_min_vertex_2_object(); }

  using Construct_max_vertex_2 = typename CKvA_2::Construct_max_vertex_2;
  Construct_max_vertex_2 construct_max_vertex_2_object() const
  { return curved_kernel()->construct_max_vertex_2_object(); }

  using Construct_opposite_2 = typename CKvA_2::Construct_opposite_2;
  Construct_opposite_2 construct_opposite_2_object() const { return curved_kernel()->construct_opposite_2_object(); }

  using Is_vertical_2 = typename CKvA_2::Is_vertical_2;
  Is_vertical_2 is_vertical_2_object() const { return curved_kernel()->is_vertical_2_object(); }

  using Compare_y_at_x_2 = typename CKvA_2::Compare_y_at_x_2;
  Compare_y_at_x_2 compare_y_at_x_2_object() const { return curved_kernel()->compare_y_at_x_2_object(); }

  using Compare_y_at_x_left_2 = typename CKvA_2::Compare_y_at_x_left_2;
  Compare_y_at_x_left_2 compare_y_at_x_left_2_object() const
  { return curved_kernel()->compare_y_at_x_left_2_object(); }

  using Compare_y_at_x_right_2 = typename CKvA_2::Compare_y_at_x_right_2;
  Compare_y_at_x_right_2 compare_y_at_x_right_2_object() const
  { return curved_kernel()->compare_y_at_x_right_2_object(); }

  using Intersect_2 = typename CKvA_2::Intersect_2;
  Intersect_2 intersect_2_object() const { return curved_kernel()->intersect_2_object(); }

  using Do_intersect_2 = typename CKvA_2::Do_intersect_2;
  Do_intersect_2 do_intersect_2_object() const { return curved_kernel()->do_intersect_2_object(); }

  using Split_2 = typename CKvA_2::Split_2;
  Split_2 split_2_object() const { return curved_kernel()->split_2_object(); }

  using Are_mergeable_2 = typename CKvA_2::Are_mergeable_2;
  Are_mergeable_2 are_mergeable_2_object() const { return curved_kernel()->are_mergeable_2_object(); }

  using Merge_2 = typename CKvA_2::Merge_2;
  Merge_2 merge_2_object() const { return curved_kernel()->merge_2_object(); }

  using Make_x_monotone_2 = typename CKvA_2::Make_x_monotone_2;
  Make_x_monotone_2 make_x_monotone_2_object() const { return Make_x_monotone_2(curved_kernel()); }

  using Is_on_2 = typename CKvA_2::Is_on_2;
  Is_on_2 is_on_2_object() const { return Is_on_2(curved_kernel()); }

  using Approximate_2 = typename CKvA_2::Approximate_2;
  Approximate_2 approximate_2_object() const { return Approximate_2(curved_kernel()); }

  using Construct_bbox_2 = typename CKvA_2::Construct_bbox_2;
  Construct_bbox_2 construct_bbox_2_object() const { return Construct_bbox_2(curved_kernel()); }

  using Construct_point_2 = typename CKvA_2::Construct_point_2;
  Construct_point_2 construct_point_2_object() const { return Construct_point_2(curved_kernel()); }

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

    // Constructs the x-monotone segment of cv that contains p; it is extended to the left up to start (if given)
    // and to the right up to end (if given) through points that have one branch on each side. Only the arcs of the
    // decomposition of cv into x-monotone arcs that the segment passes through are constructed.
    template <typename OutputIterator>
    OutputIterator x_monotone_segment(const Curve_2& cv, const Point_2& p, const Optional_point_2& start,
                                      const Optional_point_2& end, OutputIterator out) const {
      using Make_x_monotone_result = std::variant<Point_2, X_monotone_curve_2>;
      using Status_line_1 = typename Curve_2::Status_line_1;

      std::list<X_monotone_curve_2> segs;

      auto on_arc = this->_ckva()->is_on_2_object();
      auto trim = this->_ckva()->trim_2_object();
      auto split = this->_ckva()->split_2_object();
      auto left = this->_ckva()->construct_min_vertex_2_object();
      auto right = this->_ckva()->construct_max_vertex_2_object();
      auto equal = this->_ckva()->equal_2_object();
      auto construct_point = this->_ckva()->construct_point_2_object();
      const auto& map_interval_arcno = this->_ckva()->interval_arcno_cache();
      const internal::Make_x_monotone_2<CKvA_2> make_x_monotone(this->_ckva());
      auto new_point = [&](const Algebraic_real_1& x, int j) { return construct_point(x, cv, j); };

      CGAL_precondition(typename CGAL::Polynomial_traits_d<Polynomial_2>::Total_degree()(cv.polynomial_2()) > 0);
      if (typename CGAL::Polynomial_traits_d<Polynomial_2>::Total_degree()(cv.polynomial_2()) < 1) return out;
      const int total_events = cv.number_of_status_lines_with_event();

      // Find the arc that contains p, testing the candidates in the order of the decomposition. The arc is the
      // k-th arc over the i-th interval, or a vertical arc (i = -1).
      std::optional<X_monotone_curve_2> seg_p;
      int interval = -1;
      int arcno = 0;
      auto find_over_interval = [&](int i) {
        const int n = make_x_monotone.number_of_arcs_of_interval(cv, i);
        for (int k = 0; k < n; ++k) {
          auto xcv = make_x_monotone.arc_of_interval(cv, i, k, new_point, new_point);
          if (! on_arc(p, xcv)) continue;
          seg_p = xcv;
          interval = i;
          arcno = k;
          return;
        }
      };
      int index;
      bool is_event;
      cv.x_to_index(p.x(), index, is_event);
      find_over_interval(index);
      if (! seg_p && is_event) {
        std::vector<Make_x_monotone_result> objs;
        make_x_monotone.vertical_arcs_and_isolated_points(cv, index, std::back_inserter(objs));
        for (const auto& obj : objs) {
          const auto* xcv = std::get_if<X_monotone_curve_2>(&obj);
          if (! xcv || ! on_arc(p, *xcv)) continue;
          seg_p = *xcv;
          break;
        }
        if (! seg_p) find_over_interval(index + 1);
      }
      CGAL_precondition(seg_p.has_value()); // p must lie on cv
      if (! seg_p) return out;
      const X_monotone_curve_2& seg = *seg_p;

      bool left_on_arc = start && on_arc(start.value(), seg);
      bool right_on_arc = end && on_arc(end.value(), seg);

      if (left_on_arc && right_on_arc) segs.push_back(trim(seg, start.value(), end.value()));
      if (left_on_arc && (! right_on_arc)) {
        if (! seg.is_finite(CGAL::ARR_MAX_END) || ! equal(start.value(), right(seg))) {
          if (seg.is_finite(CGAL::ARR_MIN_END) && equal(start.value(), left(seg))) segs.push_back(seg);
          else {
            X_monotone_curve_2 split1;
            X_monotone_curve_2 split2;
            split(seg, start.value(), split1, split2);
            segs.push_back(split2);
          }
        }
      }
      if ((! left_on_arc) && right_on_arc) {
        if (! seg.is_finite(CGAL::ARR_MIN_END) || ! equal(left(seg), end.value())) {
          if (seg.is_finite(CGAL::ARR_MAX_END) && equal(end.value(), right(seg))) segs.push_back(seg);
          else {
            X_monotone_curve_2 split1;
            X_monotone_curve_2 split2;
            split(seg, end.value(), split1, split2);
            segs.push_back(split1);
          }
        }
      }
      if ((! left_on_arc) && (! right_on_arc)) segs.push_back(seg);

      // obtains the index of the point at the event to the left (if at_left holds) or to the right of the i-th
      // interval that the k-th arc over the interval ends at, if the point has one branch on each side and the event
      // line is not covered by cv; otherwise, -1
      auto one_one_end = [&](int i, int k, bool at_left) {
        if (i < 0) return -1; // a vertical arc
        const int e = at_left ? i - 1 : i;
        if ((e < 0) || (e == total_events)) return -1; // the arc is unbounded in x
        const Status_line_1& evt_line = cv.status_line_at_event(e);
        if (evt_line.covers_line()) return -1;
        // the interval is to the right (side 0) of the event to its left
        const auto info = map_interval_arcno(evt_line, ! at_left, k);
        if (info.second != CGAL::ARR_INTERIOR) return -1; // the arc approaches an asymptote
        if (evt_line.number_of_incident_branches(info.first) != std::make_pair(1, 1)) return -1;
        return info.first;
      };

      // Extend the segment to the left, arc by arc, through one-one points, until start is reached.
      if (! left_on_arc) {
        X_monotone_curve_2 xcv = seg;
        int i = interval;
        int k = arcno;
        while (true) {
          const int j = one_one_end(i, k, true);
          if (j < 0) {
            CGAL_assertion(! start); // the branch cannot be continued, so start must not be given
            break;
          }
          --i;
          k = make_x_monotone.interval_arcno(cv.status_line_at_event(i), j, true);
          const Point_2 q = left(xcv);
          xcv = make_x_monotone.arc_of_interval(cv, i, k, new_point,
                                                [&q](const Algebraic_real_1&, int) -> const Point_2& { return q; });
          if (start && on_arc(start.value(), xcv)) {
            segs.push_front(trim(xcv, start.value(), right(xcv)));
            break;
          }
          segs.push_front(xcv);
        }
      }

      // Extend the segment to the right, arc by arc, through one-one points, until end is reached.
      if (! right_on_arc) {
        X_monotone_curve_2 xcv = seg;
        int i = interval;
        int k = arcno;
        while (true) {
          const int j = one_one_end(i, k, false);
          if (j < 0) {
            CGAL_assertion(! end); // the branch cannot be continued, so end must not be given
            break;
          }
          k = make_x_monotone.interval_arcno(cv.status_line_at_event(i), j, false);
          ++i;
          const Point_2 q = right(xcv);
          xcv = make_x_monotone.arc_of_interval(cv, i, k,
                                                [&q](const Algebraic_real_1&, int) -> const Point_2& { return q; },
                                                new_point);
          if (end && on_arc(end.value(), xcv)) {
            segs.push_back(trim(xcv, left(xcv), end.value()));
            break;
          }
          segs.push_back(xcv);
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
    OutputIterator
    operator()(const Curve_2& cv, const Point_2& end_left, const Point_2& end_right, OutputIterator out) const {
      // the segment is traced from an end that has a single non-vertical branch towards the other end, or from the
      // lower end of a vertical segment
      const auto bl = branch_numbers_and_vertical(cv, end_left);
      const bool from_left = (bl.first.second == 0 && bl.second) || (bl.first.second == 1 && ! bl.second);
      CGAL_assertion_code(const auto br = branch_numbers_and_vertical(cv, end_right);)
      CGAL_assertion(from_left || (br.first.first == 1 && ! br.second) || (br.first.first == 0 && br.second));
      CGAL_assertion(! ((bl.first.second == 0 && bl.second) || (br.first.first == 0 && br.second)) ||
                     (this->_ckva()->compare_x_2_object()(end_left, end_right) == CGAL::EQUAL));
      const Point_2& p = from_left ? end_left : end_right;
      return x_monotone_segment(cv, p, Optional_point_2(end_left), Optional_point_2(end_right), out);
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
        using FT = CGAL::Fraction_traits<Rational>;
        using Numerator = typename FT::Numerator_type;
        using Denominator = typename FT::Denominator_type;
        using Polynomial_1 = typename CGAL::Polynomial_traits_d<Polynomial_2>::Coefficient_type;

        // the line a x + b y + c = 0 through p and q
        const Rational pxr = px.rational();
        const Rational pyr = py.rational();
        const Rational qxr = qx.rational();
        const Rational qyr = qy.rational();
        typename FT::Decompose decompose;
        Numerator a;
        Numerator b;
        Numerator c;
        Denominator a_den;
        Denominator b_den;
        Denominator c_den;
        decompose(pyr - qyr, a, a_den);
        decompose(qxr - pxr, b, b_den);
        decompose(pxr * qyr - pyr * qxr, c, c_den);
        // the coefficients are brought to a common denominator and divided by their content (b is not 0)
        Numerator a_int = a * b_den * c_den;
        Numerator b_int = b * a_den * c_den;
        Numerator c_int = c * a_den * b_den;
        const Numerator content = CGAL::gcd(CGAL::gcd(a_int, b_int), c_int);
        a_int = CGAL::integral_division(a_int, content);
        b_int = CGAL::integral_division(b_int, content);
        c_int = CGAL::integral_division(c_int, content);
        Polynomial_2 pol(Polynomial_1(c_int, a_int), Polynomial_1(b_int));
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
  { return Construct_x_monotone_segment_2(curved_kernel()); }

  class Construct_curve_2 : public internal::CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base<CKvA_2> {
  public:
    using Base = internal::CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base<CKvA_2>;

    Construct_curve_2(CKvA_2* kernel) : Base(kernel) {}

    Curve_2 operator()(const Polynomial_2& p) const { return this->_ckva()->kernel().construct_curve_2_object()(p); }
  };

  Construct_curve_2 construct_curve_2_object() const { return Construct_curve_2(curved_kernel()); }

private:
  //! the curved kernel used by the functors; if empty, \c CKvA_2::instance() of the calling thread is used
  Shared_ckva m_ckva;
};

} // namespace CGAL

#include <CGAL/enable_warnings.h>

#endif
