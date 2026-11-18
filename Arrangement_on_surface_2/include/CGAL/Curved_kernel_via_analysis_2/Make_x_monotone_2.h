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
//             Arno Eigenwillig <arno@mpi-inf.mpg.de>
//             Pavel Emeliyanenko <asm@mpi-sb.mpg.de>
//
// ============================================================================

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_MAKE_X_MONOTONE_2_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_MAKE_X_MONOTONE_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*! \file include/CGAL/Curved_kernel_via_analysis_2/Make_x_monotone_2.h
 * defines \c Make_x_monotone_2 functor
 */

#include <utility>
#include <variant>
#include <vector>

#include <CGAL/Arr_enums.h>

// TODO remove polynomial_traits
#include <CGAL/assertions.h>
#include <CGAL/Polynomial_traits_d.h>

namespace CGAL {
namespace internal {

/*! splits a curve that can be analyzed
 * into connected x-monotone sweepable arcs and isolated points.
 *
 * Arcs are stored as CurvedKernelViaAnalysis_2::Arc_2 objects, and
 * each is either vertical or consists of an x-monotone piece
 * of constant arc number wrt to the curve at every interior x-coordinate.
 * Isolated points are stored as \c CurvedKernelViaAnalysis_2::Point_2 objects.
 *
 * The resulting arcs and points are written to the output iterator as
 * polymorphic \c variant. Past-the-end value of the iterator is returned.
 */
template <typename CurvedKernelViaAnalysis_2,
          typename ConstructArc_2 = typename CurvedKernelViaAnalysis_2::Construct_arc_2>
struct Make_x_monotone_2 {
  //!\name Public types
  //!@{

  //! this instance's first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! this instance's second template parameter
  using Construct_arc_2 = ConstructArc_2;

  //! type of curve kernel
  using Curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;

  //! type of x-coordinate
  using Coordinate_1 = typename Curve_kernel_2::Coordinate_1;

  //! type of xy-coordinate
  using Coordinate_2 = typename Curve_kernel_2::Coordinate_2;

  //! type of curve analysis
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! type of vertical line
  using Status_line_1 = typename Curve_analysis_2::Status_line_1;

  //! type of point on curve
  using Point_2 = typename Curved_kernel_via_analysis_2::Point_2;

  //! type of curve arc
  using Arc_2 = typename Curved_kernel_via_analysis_2::Arc_2;

  //! type of not necessarily x-monotone curve arc
  using Non_x_monotone_arc_2 = typename Curved_kernel_via_analysis_2::Non_x_monotone_arc_2;

  //!@}

  //!\name Constructors
  //!@{

  /*! constructs
   *
   * \param kernel The kernel instance to use
   */
  Make_x_monotone_2(Curved_kernel_via_analysis_2* kernel) : m_curved_kernel(kernel)
  { CGAL_assertion(kernel != nullptr); }

  //!@}

  //!\name Functor invocation
  //!@{

  // TODO add operator for non-x-monotone arc

  /*! splits a curve into x-monotone arcs and isolated points
   *
   * \param curve The input curve
   * \param oi the output iterator for the result. Its dereference type is a
   *           variant that wraps a Point_2 or an X_monotone_curve_2 objects.
   * \return Past-the-end iterator of \c oi
   */
  template <typename OutputIterator>
  OutputIterator operator()(Curve_analysis_2 curve, OutputIterator oi) const {
    using Make_x_monotone_result = std::variant<Point_2, Arc_2>;

    // use CGAL::Total_degree ?
    using Polynomial_2 = typename Curve_analysis_2::Polynomial_2;
    if (typename CGAL::Polynomial_traits_d<Polynomial_2>::Total_degree()(curve.polynomial_2()) < 1) return oi;

    auto construct_point = m_curved_kernel->construct_point_2_object();
    const int total_events = curve.number_of_status_lines_with_event();
    // the points at the events to the left and to the right of the current interval
    std::vector<Point_2> min_pts;
    std::vector<Point_2> max_pts;
    auto min_point = [&](const Coordinate_1&, int j) -> const Point_2& { return min_pts[j]; };
    auto max_point = [&](const Coordinate_1&, int j) -> const Point_2& { return max_pts[j]; };
    // the arcs over the i-th interval are preceded by the vertical arcs and isolated points at the (i-1)-th event
    for (int i = 0; i <= total_events; ++i) {
      if (i > 0) oi = _handle_vertical_and_isolated(curve, curve.status_line_at_event(i - 1), min_pts, oi);
      if (i < total_events) {
        const Status_line_1& evt_line = curve.status_line_at_event(i);
        max_pts.clear();
        for (int j = 0; j < evt_line.number_of_events(); ++j)
          max_pts.push_back(construct_point(evt_line.x(), curve, j));
      }
      const int n = number_of_arcs_of_interval(curve, i);
      for (int k = 0; k < n; ++k) *oi++ = Make_x_monotone_result(arc_of_interval(curve, i, k, min_point, max_point));
      std::swap(min_pts, max_pts);
    }
    return oi;
  }

  /*! returns the number of arcs of a curve over its `i`-th interval, which lies between its `(i-1)`-th and `i`-th
   * events.
   */
  int number_of_arcs_of_interval(const Curve_analysis_2& curve, int i) const {
    const int total_events = curve.number_of_status_lines_with_event();
    if (total_events == 0) return curve.status_line_of_interval(0).number_of_events();
    // the arcs are counted at the event to the right of the interval, or to the left of the last interval
    const bool to_left = (i < total_events);
    const Status_line_1& evt_line = curve.status_line_at_event(to_left ? i : i - 1);
    auto side = [to_left](const std::pair<int, int>& branches) { return to_left ? branches.first : branches.second; };
    int n = side(evt_line.number_of_branches_approaching_minus_infinity()) +
      side(evt_line.number_of_branches_approaching_plus_infinity());
    for (int j = 0; j < evt_line.number_of_events(); ++j) n += side(evt_line.number_of_incident_branches(j));
    CGAL_assertion(n == curve.status_line_of_interval(i).number_of_events());
    return n;
  }

  /*! returns the arc number, over the interval to the left (if `to_left` holds) or to the right of an event, of the
   * branch incident to the `j`-th point of the status line `evt_line` at the event; the point must have a single
   * branch on that side.
   */
  int interval_arcno(const Status_line_1& evt_line, int j, bool to_left) const {
    auto side = [to_left](const std::pair<int, int>& branches) { return to_left ? branches.first : branches.second; };
    CGAL_precondition(side(evt_line.number_of_incident_branches(j)) == 1);
    int k = side(evt_line.number_of_branches_approaching_minus_infinity());
    for (int jj = 0; jj < j; ++jj) k += side(evt_line.number_of_incident_branches(jj));
    CGAL_assertion(m_curved_kernel->interval_arcno_cache()(evt_line, to_left, k) ==
                   std::make_pair(j, CGAL::ARR_INTERIOR));
    return k;
  }

  /*! constructs the `k`-th arc of a curve over its `i`-th interval, which lies between its `(i-1)`-th and `i`-th
   * events.
   *
   * \param min_point a callable that obtains the `j`-th point at the `(i-1)`-th event given the x-coordinate of
   *        the event and `j`
   * \param max_point a callable that obtains the `j`-th point at the `i`-th event given the x-coordinate of the
   *        event and `j`
   */
  template <typename MinPoint, typename MaxPoint>
  Arc_2 arc_of_interval(const Curve_analysis_2& curve, int i, int k, MinPoint min_point, MaxPoint max_point) const {
    auto construct_arc_2 = m_curved_kernel->construct_arc_2_object();
    const auto& map_interval_arcno = m_curved_kernel->interval_arcno_cache();
    auto inf_end = [](CGAL::Arr_parameter_space ps)
    { return (ps == CGAL::ARR_BOTTOM_BOUNDARY) ? CGAL::ARR_MIN_END : CGAL::ARR_MAX_END; };

    const int total_events = curve.number_of_status_lines_with_event();
    if (total_events == 0) return construct_arc_2(curve, k);

    // the arc before the first event
    if (i == 0) {
      const Status_line_1& evt_line = curve.status_line_at_event(0);
      const auto info = map_interval_arcno(evt_line, 1, k);
      if (info.second != CGAL::ARR_INTERIOR)
        return construct_arc_2(CGAL::ARR_MIN_END, evt_line.x(), inf_end(info.second), curve, k);
      return construct_arc_2(max_point(evt_line.x(), info.first), CGAL::ARR_MIN_END, curve, k, info.first);
    }

    // the arc after the last event
    if (i == total_events) {
      const Status_line_1& evt_line = curve.status_line_at_event(total_events - 1);
      const auto info = map_interval_arcno(evt_line, 0, k);
      if (info.second != CGAL::ARR_INTERIOR)
        return construct_arc_2(CGAL::ARR_MAX_END, evt_line.x(), inf_end(info.second), curve, k);
      return construct_arc_2(min_point(evt_line.x(), info.first), CGAL::ARR_MAX_END, curve, k, info.first);
    }

    // the arc between two events
    const Status_line_1& evt_line1 = curve.status_line_at_event(i - 1);
    const Status_line_1& evt_line2 = curve.status_line_at_event(i);
    const auto info1 = map_interval_arcno(evt_line1, 0, k);
    const auto info2 = map_interval_arcno(evt_line2, 1, k);
    if (info1.second != CGAL::ARR_INTERIOR) {
      if (info2.second != CGAL::ARR_INTERIOR)
        return construct_arc_2(evt_line1.x(), inf_end(info1.second), evt_line2.x(), inf_end(info2.second), curve, k);
      return construct_arc_2(max_point(evt_line2.x(), info2.first), evt_line1.x(), inf_end(info1.second), curve, k,
                             info2.first);
    }
    if (info2.second != CGAL::ARR_INTERIOR)
      return construct_arc_2(min_point(evt_line1.x(), info1.first), evt_line2.x(), inf_end(info2.second), curve, k,
                             info1.first);
    return construct_arc_2(min_point(evt_line1.x(), info1.first), max_point(evt_line2.x(), info2.first), curve, k,
                           info1.first, info2.first);
  }

  /*! constructs the vertical arcs and the isolated points of a curve at its `e`-th event.
   *
   * \param oi the output iterator for the result. Its dereference type is a variant that wraps a Point_2 or an
   *           X_monotone_curve_2 objects.
   * \return Past-the-end iterator of \c oi
   */
  template <typename OutputIterator>
  OutputIterator vertical_arcs_and_isolated_points(const Curve_analysis_2& curve, int e, OutputIterator oi) const {
    auto construct_point = m_curved_kernel->construct_point_2_object();
    const Status_line_1& evt_line = curve.status_line_at_event(e);
    std::vector<Point_2> pts;
    if (evt_line.covers_line()) {
      for (int j = 0; j < evt_line.number_of_events(); ++j) pts.push_back(construct_point(evt_line.x(), curve, j));
    }
    return _handle_vertical_and_isolated(curve, evt_line, pts, oi);
  }
  //!@}

private:
  //!\name Private members
  //!@{

  /*! constructs vertical arcs and isolated points at event line
   *
   * \param curve The curve
   * \param cv_line The event line in focus
   * \param pts Points at event line (only used if the event line covers the vertical line)
   * \param oi the output iterator for the result. Its dereference type is a
   *           variant that wraps a Point_2 or an X_monotone_curve_2 objects.
   * \return Past-the-end iterator of \c oi
   */
  template <typename OutputIterator>
  OutputIterator _handle_vertical_and_isolated(const Curve_analysis_2& curve, const Status_line_1& cv_line,
                                               const std::vector<Point_2>& pts, OutputIterator oi) const {
    using Make_x_monotone_result = std::variant<Point_2, Arc_2>;

    auto construct_arc_2 = m_curved_kernel->construct_arc_2_object();

    const Coordinate_1 x = cv_line.x();
    int n = cv_line.number_of_events();
    int j;
    if (cv_line.covers_line()) { // look for vertical arcs
      if (n > 0) {
        // the first vertical ray
        *oi++ = Make_x_monotone_result(construct_arc_2(pts[0], CGAL::ARR_MIN_END, curve));
        for (j = 0; j < n - 1; j++) // interior bounded arcs
          *oi++ = Make_x_monotone_result(construct_arc_2(pts[j], pts[j + 1], curve));
        // the last vertical ray
        *oi++ = Make_x_monotone_result(construct_arc_2(pts[n - 1], CGAL::ARR_MAX_END, curve));
      }
      else // unbounded vertical line
        *oi++ = Make_x_monotone_result(construct_arc_2(x, curve));
      return oi;
    }
    // look for isolated points
    std::pair<int, int> ipair;
    for (j = 0; j < n; j++) {
      ipair = cv_line.number_of_incident_branches(j);
      if (ipair.first == 0 && ipair.second == 0) {
        //std::cout << "isolated point found\n";
        auto construct_point = m_curved_kernel->construct_point_2_object();
        *oi++ = Make_x_monotone_result(construct_point(x, curve, j));
      }
    }
    return oi;
  }

  //!@}

  //!\name Private data
  //!@{

  //! pointer to \c Curved_kernel_via_analysis_2
  Curved_kernel_via_analysis_2* m_curved_kernel;

  //!@}
}; // struct Make_x_monotone

} // namespace internal
} // namespace CGAL

#endif
