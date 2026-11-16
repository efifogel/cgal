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

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_GENERIC_POINT_2_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_GENERIC_POINT_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*!\file include/CGAL/Curved_kernel_via_analysis_2/Generic_point_2.h
 * defines class \c Generic_point_2
 *
 * adds support for points at infinity to native CKvA_2 object
 */

#include <CGAL/config.h>
#include <CGAL/Handle_with_policy.h>

namespace CGAL {
namespace internal {

//! forward class declaration
template <typename SweepCurvesAdaptor_2, typename Rep_>
class Generic_point_2;

template <typename SweepCurvesAdaptor_2, typename Rep_>
std::ostream& operator<<(std::ostream&, const Generic_point_2<SweepCurvesAdaptor_2, Rep_>&);

template <typename SweepCurvesAdaptor_2>
class Generic_point_2_rep {
public:
  // this instance's template parameter
  using Sweep_curves_adaptor_2 = SweepCurvesAdaptor_2;

  // myself
  using Self = Generic_point_2_rep<Sweep_curves_adaptor_2>;

  // type of a native point object provided by CKvA_2
  using Point_2 = typename Sweep_curves_adaptor_2::Native_point_2;

  // type of a native arc object provided by CKvA_2
  using Arc_2 = typename Sweep_curves_adaptor_2::Native_arc_2;

public:
  // default constructor
  Generic_point_2_rep() : m_point(Point_2()) {}

  // standard constructor : point at infinity
  Generic_point_2_rep(const Arc_2& c, CGAL::Arr_curve_end end) : m_arc(c), m_end(end) {}

  // standard constructor : normal point
  Generic_point_2_rep(const Point_2& p) : m_point(p) {}

  mutable std::optional<Arc_2> m_arc; // supporting arc for points at inf

  // stores respective curve end if this is a point at infinity
  CGAL::Arr_curve_end m_end;

  mutable std::optional<Point_2> m_point; // stores a finite point

  // befriending the handle
  friend class Generic_point_2<Sweep_curves_adaptor_2, Self>;
};

// Boundary_type defined in Arr_enums.h

//! class defines a point on a generic curve
template <typename SweepCurvesAdaptor_2, typename Rep_ = internal::Generic_point_2_rep<SweepCurvesAdaptor_2>>
class Generic_point_2 : public CGAL::Handle_with_policy<Rep_> {
public:
  //!\name publuic typedefs
  //!@{

  //! this instance's first template parameter
  using Sweep_curves_adaptor_2 = SweepCurvesAdaptor_2;

  //! this instance's second template parameter
  using Rep = Rep_;

  //! this instance itself
  using Self = Generic_point_2<Sweep_curves_adaptor_2, Rep>;

  //! type of a native point object provided by CKvA_2
  using Point_2 = typename Sweep_curves_adaptor_2::Native_point_2;

  //! type of a native arc object provided by CKvA_2
  using Arc_2 = typename Sweep_curves_adaptor_2::Native_arc_2;

  //! the handle superclass
  using Base = ::CGAL::Handle_with_policy<Rep>;

  //!@}

public:
  //!\name basic constructors
  //!@{

  /*! constructs default. */
  Generic_point_2() : Base(Rep()) {}

  /*! constructs copy */
#ifdef DOXYGEN_RUNNING
  Generic_point_2(const Self& p) : Base(static_cast<const Base&>(p)) {}
#endif

  /*! constructs an arc from a given representation */
  Generic_point_2(Rep rep) : Base(rep) {}

  //!@}

  //!\name standard constructors
  //!@{

  /*! constructs a finite point */
  explicit Generic_point_2(const Point_2& pt) : Base(Rep(pt)) {}

  //! constructs a 'point at infinity'
  explicit Generic_point_2(const Arc_2& c, CGAL::Arr_curve_end end) : Base(Rep(c, end)) {}

  //!@}
public:
  //!\name access functions
  //!@{

  //! checks whether this point does not lie at infinity
  bool is_finite() const { return (this->ptr()->m_point); }

  //! returns supporting arc of a point lying at infinity
  //!
  //! \pre !is_finite
  Arc_2 arc() const {
    CGAL_precondition(! is_finite());
    return *(this->ptr()->m_arc);
  }

  //! returns respective curve end of a point lying at infinity
  //!
  //! \pre !is_finite
  CGAL::Arr_curve_end curve_end() const {
    CGAL_precondition(! is_finite());
    return this->ptr()->m_end;
  }

  //! returns an embedded point object
  //!
  //! \pre is_finite
  Point_2 point() const {
    CGAL_precondition(is_finite());
    return *(this->ptr()->m_point);
  }

  //! befriending output operator
  // friend std::ostream& operator << <>(std::ostream&, const Self&);

  //!@}
}; // class Generic_point_2

template <typename SweepCurvesAdaptor_2, typename Rep_>
std::ostream& operator<<(std::ostream& os, const Generic_point_2<SweepCurvesAdaptor_2, Rep_>& pt) {
  os << pt.id() << "@";
  if (pt.is_finite()) os << pt.point();
  else os << "inf end: " << pt.curve_end() << "; arc: " << pt.arc();
  return os;
}

template <typename SweepCurvesAdaptor_2, typename Rep_>
std::istream& operator>>(std::istream& is, Generic_point_2<SweepCurvesAdaptor_2, Rep_>& /* pt */) {
  std::cerr << "bogus >> call for generic_point\n";
  return is;
}

} // namespace internal
} // namespace CGAL

#endif
