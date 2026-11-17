// Copyright (c) 2008,2009,2010,2011 Max-Planck-Institute Saarbruecken (Germany),
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

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_NON_X_MONOTONE_ARC_2_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_NON_X_MONOTONE_ARC_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*!\file include/CGAL/Curved_kernel_via_analysis_2/Non_x_monotone_arc_2.h
 * defines class \c Non_x_monotone_arc_2
 *
 * non x-monotone arc of a generic curve
 */

#include <ostream>
#include <vector>

#include <CGAL/assertions.h>
#include <CGAL/Handle_with_policy.h>

namespace CGAL {
namespace internal {

template <typename CurvedKernelViaAnalysis_2>
class Non_x_monotone_arc_2_rep {
public:
  // this instance's first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  // myself
  using Self = Non_x_monotone_arc_2_rep<Curved_kernel_via_analysis_2>;

  // type of an arc on generic curve
  using Arc_2 = typename Curved_kernel_via_analysis_2::Arc_2;

  // a list of x-monotone arcs
  using Arc_vector = std::vector<Arc_2>;

public:
  /*! constructs default. */
  Non_x_monotone_arc_2_rep() {}

  /*! constructs from one x-monotone arc */
  Non_x_monotone_arc_2_rep(const Arc_2& arc) { m_x_monotone_arcs.push_back(arc); }

  /*! constructs a non x-monotone arc from the list of x-monotone pieces */
  template <typename InputIterator>
  Non_x_monotone_arc_2_rep(InputIterator start, InputIterator end) : m_x_monotone_arcs(std::distance(start, end)) {
    std::copy(start, end, m_x_monotone_arcs.begin());
  }

  //! a list of x-monotone pieces this arc consists of
  mutable Arc_vector m_x_monotone_arcs;

}; // Non_x_monotone_arc_2_rep

/*! class representing a (not necessary x-monotone) curve arc
 *
 * This class represents a not necessarily x-monotone curve arc. The arc is
 * given as a list of connected x-monotone pieces.
 *
 * By constructing a new arc, its validity is checked to ensure that its
 * x-monotone pieces form a single chain
 */
template <typename CurvedKernelViaAnalysis_2, typename Rep_ = Non_x_monotone_arc_2_rep<CurvedKernelViaAnalysis_2>>
class Non_x_monotone_arc_2 : public CGAL::Handle_with_policy<Rep_> {
public:
  //!\name public typedefs
  //!@{

  //! this instance's first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! this instance's second template parameter
  using Rep = Rep_;

  //! this instance itself
  using Self = Non_x_monotone_arc_2<Curved_kernel_via_analysis_2, Rep>;

  //! type of curve kernel
  using Curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;

  //! type of analysis of a pair of curves
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! type of a point on generic curve
  using Point_2 = typename Curved_kernel_via_analysis_2::Point_2;

  //! type of an x-monotone arc on generic curve
  using Arc_2 = typename Curved_kernel_via_analysis_2::Arc_2;

  //! iterator type to range through the list of x-monotone arcs
  using Arc_const_iterator = typename Rep::Arc_vector::const_iterator;

  //! the handle superclass
  using Base = ::CGAL::Handle_with_policy<Rep>;

  //!@}
public:
  //!\name basic constructors
  //!@{

  /*! constructs default. */
  Non_x_monotone_arc_2() : Base(Rep()) {}

  /*! constructs copy */
#ifdef DOXYGEN_RUNNING
  Non_x_monotone_arc_2(const Self& a) : Base(static_cast<const Base&>(a)) {}
#endif

  /*! constructs an arc from one x-monotone piece */
  Non_x_monotone_arc_2(const Arc_2& arc) : Base(Rep(arc)) {}

  /*! constructs an arc from a list x-monotone pieces specified by
   * iterator range <tt>[start; end)</tt>
   *
   * template argument type of \c InputIterator is \c Arc_2
   *
   * \pre the x-monotone arcs must be connected into a single chain
   * \pre either all x-monotone arcs must be vertical or non-vertical
   */
  template <typename InputIterator>
  Non_x_monotone_arc_2(InputIterator start, InputIterator end) : Base(Rep(start, end)) {
    CGAL_precondition(start != end);
    CGAL_precondition_code(InputIterator oi = start; InputIterator next = ++oi; bool vertical = oi->is_vertical();
                           Curve_analysis_2 curve = oi->curve(); for (; next != end; next++) {
                             // ensure that supporting curves are identical for all arcs
                             CGAL_precondition(next->curve().is_identical(curve));
                             // either all arcs are vertical or not
                             CGAL_precondition(vertical == next->is_vertical());
                             /* this condition is hard to check because end-points can be
                 reordered in Arc_2 constructor
                CGAL_precondition(oi->curve_end(CGAL::ARR_MAX_END).
                    compare_xy(next->curve_end(CGAL::ARR_MIN_END)) ==
                            CGAL::EQUAL);*/
                             oi = next;
                           });
  }

  //!@}
public:
  //! \name Access functions
  //!@{

  /*! returns the number of x-monotone arcs this object consists of */
  int number_of_x_monotone_arcs() const { return static_cast<int>(this->ptr()->m_x_monotone_arcs.size()); }

  /*! returns iterator pointing to the first x-monotone arc in the list */
  Arc_const_iterator begin() const { return this->ptr()->m_x_monotone_arcs.begin(); }

  /*! returns iterator pointing to beyond the last x-monotone arc in the list */
  Arc_const_iterator end() const { return this->ptr()->m_x_monotone_arcs.end(); }

#if 1 // not needed
  /*! returns a distinct \c ith x-monotone piece of the arc */
  const Arc_2& x_monotone_arc(int i) const {
    CGAL_precondition(i >= 0);
    CGAL_precondition(i < number_of_x_monotone_arcs());
    return this->ptr()->m_x_monotone_arcs[i];
  }
#endif

  /*! returns the supporting curve */
  Curve_analysis_2 curve() const {
    CGAL_precondition(number_of_x_monotone_arcs() > 0);
    return this->ptr()->m_x_monotone_arcs[0].curve();
  }

  /*! returns \c true if this arc consists of vertical segments */
  bool is_vertical() const {
    CGAL_precondition(number_of_x_monotone_arcs() > 0);
    return this->ptr()->m_x_monotone_arcs[0].is_vertical();
  }

  //!@}
}; // Non_x_monotone_arc_2

/*!\relates Non_x_monotone_arc_2
 * output operator
 */
template <typename CurvedKernelViaAnalysis_2, typename Rep_>
std::ostream& operator<<(std::ostream& os, const Non_x_monotone_arc_2<CurvedKernelViaAnalysis_2, Rep_>& arc) {
  os << "List of x-monotone arcs: [\n";
  int i = 0;

  typename Non_x_monotone_arc_2<CurvedKernelViaAnalysis_2, Rep_>::Arc_const_iterator ait;
  for (ait = arc.begin(); ait != arc.end(); ait++, i++) os << i << ": " << *ait << std::endl;
  os << "]\n\n";
  return os;
}

#if DOXYGEN_RUNNING

/*!\relates Non_x_monotone_arc_2
 * draws an arc to Qt window \c qt
 */
template <typename CurvedKernelViaAnalysis_2, typename Rep_>
CGAL::Qt_widget& operator<<(CGAL::Qt_widget& ws, const Non_x_monotone_arc_2<CurvedKernelViaAnalysis_2, Rep_>& arc) {
  QPainter* ppnt = &ws.get_painter();

  int n_arcs = arc.number_of_x_monotone_arcs();

  QBrush defbrush = ppnt->brush();
  // forbid drawing connection points between x-monotone pieces
  if (n_arcs > 1) ppnt->setBrush(Qt::NoBrush);

  typename Non_x_monotone_arc_2<CurvedKernelViaAnalysis_2, Rep_>::Arc_const_iterator ait;
  for (ait = arc.begin(); ait != arc.end(); ait++) ws << *ait;

  if (n_arcs > 1) {
    ppnt->setBrush(defbrush);

    typeof(arc.x_monotone_arc(0)) const* xarc = &arc.x_monotone_arc(0);
    if (xarc->location(CGAL::ARR_MIN_END) == CGAL::ARR_INTERIOR) ws << xarc->curve_end(CGAL::ARR_MIN_END);

    xarc = &arc.x_monotone_arc(n_arcs - 1);
    if (xarc->location(CGAL::ARR_MAX_END) == CGAL::ARR_INTERIOR) ws << xarc->curve_end(CGAL::ARR_MAX_END);
  }
  return ws;
};

#endif

} // namespace internal
} // namespace CGAL

#endif
