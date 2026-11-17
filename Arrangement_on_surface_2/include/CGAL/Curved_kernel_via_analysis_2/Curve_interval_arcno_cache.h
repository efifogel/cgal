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
// Author(s) : Pavel Emeliyanenko <asm@mpi-sb.mpg.de>

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_CURVE_INTERVAL_ARCNO_CACHE_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_CURVE_INTERVAL_ARCNO_CACHE_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*!\file include/CGAL/Curved_kernel_via_analysis_2/Curve_interval_arcno_cache.h
 * defines \c Curve_interval_arcno_cache functor
 */

#include <map>
#include <utility>
#include <vector>

#include <CGAL/Arr_enums.h>
#include <CGAL/assertions.h>

namespace CGAL {
namespace internal {

/*! given an arc number over an interval \c i this functor computes a
 * corresponding event arcno (and asymptotic tendency) over a certain vertical
 * line which lies on the given side w.r.t. the interval \c i.
 *
 * For caching issues for each accessed \c Curve_2 object we store
 * \c Interval_arcno_map structure. In its turn, \c Interval_arcno_map
 * for each \c Status_line_1 object stores the precomputed mapping
 * from interval arcnos (on left and right sides) to event arcnos
 */
template <typename CurveKernel_2>
struct Curve_interval_arcno_cache {
  //!\name Public types
  //!@{

  //! this instance's first template parameter
  using Curve_kernel_2 = CurveKernel_2;

  //! type of curve analysis
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! type of x-coordinate
  using Coordinate_1 = typename Curve_kernel_2::Coordinate_1;

  //! type of status line
  using Status_line_1 = typename Curve_analysis_2::Status_line_1;

  //! event arc number descriptor: stores an arc number along with curve's end type (+/-oo or \c ARR_INTERIOR )
  using Arcno_desc = std::pair<int, CGAL::Arr_parameter_space>;

  //! type of first argument
  using first_argument_type = Status_line_1;

  //! type of second argument
  using second_argument_type = bool;

  //! type of third argument
  using third_argument_type = int;

  //! the result type
  using result_type = Arcno_desc;

  //! constructs

  //!@}

  //!\name Constructors
  //!@{

  /*! constructs default. */
  Curve_interval_arcno_cache() : m_last_curve_id(-1) {}

  //!@}

  //!\name Functor invocation
  //!@{

  /*! computes, given an arcno over an interval, a corresponding event
   * arcno on status line \c cv_line lying on the given \c side w.r.t. the
   * interval
   *
   * \param cv_line Status line
   * \param side side = \c true: left side; \c side = \c false: right side
   * \param interval_arcno Arcnumber of interval
   * \return Arcnumber information at event line
   */
  result_type operator()(const Status_line_1& cv_line, bool side, int interval_arcno) const {
    CGAL_precondition(interval_arcno >= 0);
    if (! cv_line.is_event()) // # of arcs over interval is constant
      return std::make_pair(interval_arcno, CGAL::ARR_INTERIOR);

    Curve_analysis_2 ca_2 = cv_line.curve_analysis_2();
    ///////////////////////////////// polynomial or curve id ?
    int curve_id = static_cast<int>(ca_2.polynomial_2().id());
    /////////////////////////////////
    if (m_last_curve_id != curve_id) {
      typename Curve_to_interval_arcno_map::iterator it;
      if (m_last_curve_id != -1) {
        // commit the last changes to the global curve arcno map
        it = m_curve_arcno_map.find(m_last_curve_id);
        CGAL_precondition(it != m_curve_arcno_map.end());
        it->second = m_last_interval_map;
      }
      // modify m_last_interval_map to be pointing to the
      // Interval_arcno_map  corresponding to the given curve_id
      it = m_curve_arcno_map.insert(std::make_pair(curve_id, Interval_arcno_map())).first;
      m_last_interval_map = it->second;
      m_last_curve_id = curve_id;
    }
    // cache status lines by id(); shall we use x-coordinate instead of
    // id to ensure uniqueness ?
    // TODO: shall we use cv_line.index() ?
    typename Interval_arcno_map::const_iterator it = m_last_interval_map.find(cv_line.x().id());
    if (it != m_last_interval_map.end()) {
      const Arcno_vector_pair& tmp = it->second;
      // note: side index must be reversed
      CGAL_precondition(interval_arcno < static_cast<int>(side == 1 ? tmp.first.size() : tmp.second.size()));
      return (side == 1 ? tmp.first[interval_arcno] : tmp.second[interval_arcno]);
    }

    int n_left = ca_2.status_line_of_interval(cv_line.index()).number_of_events();      // # arcs to the left
    int n_right = ca_2.status_line_of_interval(cv_line.index() + 1).number_of_events(); // # arcs to the right

    std::pair<int, int> n_mininf;
    std::pair<int, int> n_maxinf;
    std::pair<int, int> ipair;
    n_mininf = cv_line.number_of_branches_approaching_minus_infinity();
    n_maxinf = cv_line.number_of_branches_approaching_plus_infinity();
    // we must also account for the number of asymptotic arcs approaching this status line
    Arcno_vector_pair vpair;
    vpair.first.resize(n_left);   //+ n_mininf.first + n_maxinf.first);
    vpair.second.resize(n_right); //+ n_mininf.second + n_maxinf.second);

    int i;
    int _arcno;
    int left_i = 0;
    int right_i = 0;
    // process arcs approaching -oo from left and right
    for (; left_i < n_mininf.first; left_i++) vpair.first[left_i] = std::make_pair(left_i, CGAL::ARR_BOTTOM_BOUNDARY);

    for (; right_i < n_mininf.second; right_i++)
      vpair.second[right_i] = std::make_pair(right_i, CGAL::ARR_BOTTOM_BOUNDARY);

    // process all finite events over this vertical line
    for (_arcno = 0; _arcno < cv_line.number_of_events(); _arcno++) {
      ipair = cv_line.number_of_incident_branches(_arcno);
      for (i = 0; i < ipair.first; i++) vpair.first[left_i++] = std::make_pair(_arcno, CGAL::ARR_INTERIOR);

      for (i = 0; i < ipair.second; i++) vpair.second[right_i++] = std::make_pair(_arcno, CGAL::ARR_INTERIOR);
    }
    // process arcs approaching +oo from left and right
    // this is not clear.. why arcnos at +oo are mapped to the same
    // interval arcnos ?
    for (i = 0; i < n_maxinf.first; i++, left_i++) vpair.first[left_i] = std::make_pair(left_i, CGAL::ARR_TOP_BOUNDARY);

    for (i = 0; i < n_maxinf.second; i++, right_i++)
      vpair.second[right_i] = std::make_pair(right_i, CGAL::ARR_TOP_BOUNDARY);

    CGAL_precondition(left_i == static_cast<int>(vpair.first.size()));
    CGAL_precondition(right_i == static_cast<int>(vpair.second.size()));
    m_last_interval_map.insert(std::make_pair(cv_line.x().id(), vpair));
    CGAL_precondition(interval_arcno < static_cast<int>(side == 1 ? vpair.first.size() : vpair.second.size()));
    return (side == 1 ? vpair.first[interval_arcno] : vpair.second[interval_arcno]);
  }

  //!@}

private:
  //!\name Private members
  //!@{

  //! a pair of vectors (right and left side of event-line respectively)
  using Arcno_vector_pair = std::pair<std::vector<Arcno_desc>, std::vector<Arcno_desc>>;

  //! maps from \c Status_line_1 id to a container of interval ->
  //! event arcnos
  // TODO: use hash instead ?? cache status lines by x-coordinate ? (Pavel)
  using Id_type = typename Status_line_1::Algebraic_real_1::Id_type;
  using Interval_arcno_map = std::map<Id_type, Arcno_vector_pair>;

  //! maps from \c Curve_2 id to \c Interval_arcno_map
  using Curve_to_interval_arcno_map = std::map<int, Interval_arcno_map>;

  //! arcno mapping container instance
  mutable Curve_to_interval_arcno_map m_curve_arcno_map;

  //! an id of the last queried \c Curve_2 object
  mutable int m_last_curve_id;

  //! stores an address of the last accessed map
  mutable Interval_arcno_map m_last_interval_map;

  //!@}

}; // struct Curve_interval_arcno_cache

} // namespace internal
} // namespace CGAL

#endif
