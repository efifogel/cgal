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

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_POINT_2_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_POINT_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*!\file include/CGAL/Curved_kernel_via_analysis_2/Point_2.h
 * defines class \c Point_2 that represents a point on a curve that can
 * be analyzed.
 */

#include <iostream>
#include <type_traits>
#include <optional>
#include <utility>
#include <variant>

#include <CGAL/assertions.h>
#include <CGAL/enum.h>
#include <CGAL/Handle_with_policy.h>
#include <CGAL/Arr_enums.h>
#include <CGAL/IO/io.h>
#include <CGAL/Curved_kernel_via_analysis_2/Curved_kernel_via_analysis_2_functors.h>

namespace CGAL {
namespace internal {

// forward class declaration
template <typename CurvedKernelViaAnalysis_2, typename Rep_>
class Point_2;

// forward class declaration
template <typename CurvedKernelViaAnalysis_2>
class Arc_2_rep;

// forward class declaration for befriending
template <typename CurvedKernelViaAnalysis_2, typename Rep_ = Arc_2_rep<CurvedKernelViaAnalysis_2>>
class Arc_2;

/*\!brief
 * representation class for Point_2
 */
template <typename CurvedKernelViaAnalysis_2>
class Point_2_rep {
public:
  //! this instance's template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the class itself
  using Self = Point_2_rep<Curved_kernel_via_analysis_2>;

  //! type of curve kernel
  using Curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;

  //! type of x-coordinate
  using Coordinate_1 = typename Curve_kernel_2::Coordinate_1;

  //! type of a finite point on curve
  using Coordinate_2 = typename Curve_kernel_2::Coordinate_2;

  //! type of curve analysis
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! constructs default
  Point_2_rep() : m_location(CGAL::ARR_INTERIOR) {}

  //! constructs a "finite" point on curve,
  //! implies CGAL::NO_BOUNDARY in x/y
  Point_2_rep(const Coordinate_2& xy) : m_xy(xy), m_location(CGAL::ARR_INTERIOR) {}

  //! constructs a point at +/-oo in x
  Point_2_rep(CGAL::Arr_curve_end inf_end, const Curve_analysis_2& c, int arcno) : m_curve(c), m_arcno(arcno)
  { m_location = (inf_end == CGAL::ARR_MIN_END ? CGAL::ARR_LEFT_BOUNDARY : CGAL::ARR_RIGHT_BOUNDARY); }

  //! constructs a point on curve with y-coordinate at infinity
  Point_2_rep(const Coordinate_1& x, const Curve_analysis_2& c, CGAL::Arr_curve_end inf_end) : m_x(x), m_curve(c)
  { m_location = (inf_end == CGAL::ARR_MIN_END ? CGAL::ARR_BOTTOM_BOUNDARY : CGAL::ARR_TOP_BOUNDARY); }

  //! curve point finite coordinates. They are valid only if boundary in y
  //! is not set (CGAL::NO_BOUNDARY), otherwise only x-coordinate is
  //! accessible, i.e., point is in interior
  std::optional<Coordinate_2> m_xy;

  //! x-coordinate of a curve point
  std::optional<Coordinate_1> m_x;

  //! curve of point at boundary
  std::optional<Curve_analysis_2> m_curve;

  //! arc of point at boundary
  std::optional<int> m_arcno;

  //! location of a point in parameter space
  mutable CGAL::Arr_parameter_space m_location;

  //! store a double approximation of point
  mutable std::optional<std::pair<double, double>> m_doubles;
};

/*! class defines a point on a curve that can be analyzed
 *
 * Only points with finite coordinates can be constructed explicitly
 * (by the user). Points on the boundary use special private constructors to
 * to represent ends of curve arcs on the boundary.
 */
template <typename CurvedKernelViaAnalysis_2, typename Rep_ = internal::Point_2_rep<CurvedKernelViaAnalysis_2>>
class Point_2 : public CGAL::Handle_with_policy<Rep_> {
public:
  //!\name Public types
  //!@{

  //! this instance's first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! this instance's second template parameter
  using Rep = Rep_;

  //! this instance itself
  using Self = Point_2<Curved_kernel_via_analysis_2, Rep>;

  //! type of underlying curve kernel
  using Curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;

  //! type of an x-coordinate
  using Coordinate_1 = typename Curve_kernel_2::Coordinate_1;

  //! type of an xy-coordinate
  using Coordinate_2 = typename Curve_kernel_2::Coordinate_2;

  //! type that analyzes a curve
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! the handle superclass
  using Base = ::CGAL::Handle_with_policy<Rep>;

  //! type of kernel point
  using Kernel_point_2 = typename Curved_kernel_via_analysis_2::Point_2;

  //!@}

#if ! defined(CGAL_NO_ASSERTIONS)
  static const bool s_kernel_point_2_equals_point_2 = std::is_same<Point_2, Kernel_point_2>::value;
#endif

public:
  //!\name Rebind
  //!@{

  /*! an auxiliary structure for rebinding the point with a new rep */
  template <typename NewCKvA_2, typename NewRep>
  class rebind {
  public:
    //! this instance's first template parameter
    using New_curved_kernel_via_analysis_2 = NewCKvA_2;

    //! this instance's second template parameter
    using New_rep = NewRep;

    //! the rebound type
    using Other = Point_2<New_curved_kernel_via_analysis_2, NewRep>;

    //! the rebound point
    using Rebound_point_2 = typename New_curved_kernel_via_analysis_2::Point_2;

    /*! constructs a point of type \c Rebound_point_2 from the point \c pt
     * of type \c Self.
     *
     * All known items of the base class rep will be copied.
     */
    Rebound_point_2 operator()(const Self& pt) {
      New_rep newrep;
      newrep.m_xy = pt.ptr()->m_xy;
      newrep.m_x = pt.ptr()->m_x;
      newrep.m_curve = pt.ptr()->m_curve;
      newrep.m_arcno = pt.ptr()->m_arcno;
      newrep.m_location = pt.ptr()->m_location;
      return Rebound_point_2(newrep);
    }

    // TODO move to SfP_2l
    /*! reverses rebind, i.e., extracts original point type from a
     * rebound instance
     */
    Self operator()(const Rebound_point_2& pt) {
      Rep rep;
      rep.m_xy = pt.ptr()->m_xy;
      rep.m_x = pt.ptr()->m_x;
      rep.m_curve = pt.ptr()->m_curve;
      rep.m_arcno = pt.ptr()->m_arcno;
      rep.m_location = (pt.is_finite()) ? CGAL::ARR_INTERIOR : pt.ptr()->m_location;
      return Self(rep);
    }
  };

  //!@}

public:
  //!\name Standard constructors
  //!@{

  /*! constructs default. */
  Point_2() : Base(Rep()) {}

  /*! constructs copy */
#ifdef DOXYGEN_RUNNING
  Point_2(const Self& p) : Base(static_cast<const Base&>(p)) {}
#endif
  //!@}

public:
  //!\name Usual constructors
  //!@{

  /*! constructs an interior point
   *
   * \param x The x-coordinate
   * \param c The supporting curve
   * \param arcno Arcno of point on \c c
   * \return The constructed point
   */
  Point_2(const Coordinate_1& x, const Curve_analysis_2& c, int arcno) : Base(Rep(Coordinate_2(x, c, arcno))) {}

  /*! constructs an interior point from its coordinates; use \c Construct_point_2 to construct a point from
   * numbers.
   *
   * \param xy The coordinates
   */
  explicit Point_2(const Coordinate_2& xy) : Base(Rep(xy)) {}

  // FUTURE TODO allow to construct without curve,
  // i.e, isolated points on toric identifications -> do it also for arcs
  // FUTURE TODO parameter space in x/y (full set of tasks)

  //!@}

public: // was protected:
  //!\name Special constructors for points on the boundary
  //!@{

  /*! constructs a point with x-coordinate at the left/right boundary
   *
   * \param inf_end Determines whether point is on left or right boundary
   * \param c The supporting curve
   * \param arcno Arcno of point on \c on left/right boundary
   * \return The constructed point
   */
  Point_2(CGAL::Arr_curve_end inf_end, const Curve_analysis_2& c, int arcno) : Base(Rep(inf_end, c, arcno)) {}

  /*! constructs a point on bottom or top boundary
   *
   * \param x The x-coordinate of point
   * \param c The supporting curve
   * \param inf_end Defines whether point is on bottom or top boundary
   * \return The constructed point
   */
  Point_2(const Coordinate_1& x, const Curve_analysis_2& c, CGAL::Arr_curve_end inf_end) : Base(Rep(x, c, inf_end)) {}

  //!@}

protected:
  //!\name Constructors for rebind
  //!@{

  /*! constructs from a given representation */
  /*! constructs for rebind
   *
   * \param rep The representation
   * \return The constructed point
   */
  Point_2(Rep rep) : Base(rep) {}

  //!@}

public:
  //!\name Destructors
  //!@{

  /*! destructs */
  virtual ~Point_2() {}

  //!@}

public:
  //!\name Access members
  //!@{

  /*! obtains the point's x-coordinate (y-coordinate can be undefined)
   *
   * \return The stored x-coordinate
   * \pre the point's x must be finite
   */
  inline const Coordinate_1& x() const {
    CGAL_precondition_msg(this->ptr()->m_xy || this->ptr()->m_x,
                          "Denied access to x-coordinate of the curve end lying at x-infinity");
    return (is_finite() ? (*(this->ptr()->m_xy)).x() : *(this->ptr()->m_x));
  }

  /*! obtains the underlying \c Coordinate_2 object
   *
   * \return The stored xy-coordinate
   * \pre The xy-coordinates must be finite
   */
  inline const Coordinate_2& xy() const {
    CGAL_precondition_msg(bool(this->ptr()->m_xy), "Denied access to the curve end lying at x/y-infinity");
    return *(this->ptr()->m_xy);
  }

  inline const Coordinate_1 y() const { return this->xy().y(); }

  /*! obtains the supporting curve of the point
   *
   * \return supporting curve of point
   */
  inline Curve_analysis_2 curve() const {
    CGAL_precondition_msg(this->ptr()->m_xy || this->ptr()->m_curve,
                          "Denied access to the curve end lying at y-infinity");
    return (is_finite() ? (*(this->ptr()->m_xy)).curve() : *(this->ptr()->m_curve));
  }

  /*! obtains the arc number of the point on a curve
   *
   * \return arcno of point
   * \pre Is not endpoint of a vertical ray or branch
   */
  inline int arcno() const {
    CGAL_precondition_msg(this->ptr()->m_xy || this->ptr()->m_arcno,
                          "Denied access to the curve end lying at y-infinity");
    return (is_finite() ? (*(this->ptr()->m_xy)).arcno() : *(this->ptr()->m_arcno));
  }

  //!@}

public:
  //!\name Methods for location
  //!@{

  /*! sets location of a point in parameter space to \c loc
   *
   * It is supposed that the user thoroughly understands malicious
   * consequences that may result from the misuse of the location
   */
  inline void set_location(CGAL::Arr_parameter_space loc) const { this->ptr()->m_location = loc; }

  /*! obtains the location of the point in parameter space
   *
   * \return location in parameter space
   */
  inline CGAL::Arr_parameter_space location() const { return this->ptr()->m_location; }

  /*! checks if the point lies on left/right boundary
   *
   * \return \c true if it lies on left/right boundary, \c false otherwise
   */
  inline bool is_on_left_right() const
  { return (location() == CGAL::ARR_LEFT_BOUNDARY || location() == CGAL::ARR_RIGHT_BOUNDARY); }

  /*! checks if the point lies on bottom/top boundary
   *
   * \return \c true if it lies on bottom/top boundary, \c false otherwise
   */
  inline bool is_on_bottom_top() const
  { return (location() == CGAL::ARR_BOTTOM_BOUNDARY || location() == CGAL::ARR_TOP_BOUNDARY); }

  /*! checks whether the point is finite
   *
   * \return \c true, if point is finite, \c false otherwise
   */
  inline bool is_finite() const { return bool(this->ptr()->m_xy); }

  //!@}

  //!\name Comparison operators for points in the interior of parameter space
  //!@{

private:
  /*! compares this point with \c q lexicographically, using the curve kernel of the coordinates of the points
   * \pre both points lie in the interior of the parameter space
   */
  CGAL::Comparison_result _compare_xy(const Kernel_point_2& q) const {
    CGAL_precondition(bool(this->ptr()->m_xy));
    CGAL_precondition(bool(q.ptr()->m_xy));
    if (this->id() == q.id()) return CGAL::EQUAL;
    return this->xy().kernel()->compare_xy_2_object()(this->xy(), q.xy());
  }

public:

  //! checks for equality
  inline bool operator==(const Kernel_point_2& q) const { return this->_compare_xy(q) == CGAL::EQUAL; }

  //! checks for inequality
  inline bool operator!=(const Kernel_point_2& q) const { return this->_compare_xy(q) != CGAL::EQUAL; }

  //! checks for less-than in (x,y) lexicographic order
  inline bool operator<(const Kernel_point_2& q) const { return this->_compare_xy(q) == CGAL::SMALLER; }

  //! checks for less-equal in (x,y) lexicographic order
  inline bool operator<=(const Kernel_point_2& q) const { return this->_compare_xy(q) != CGAL::LARGER; }

  //! checks for greater-than in (x,y) lexicographic order
  inline bool operator>(const Kernel_point_2& q) const { return this->_compare_xy(q) == CGAL::LARGER; }

  //! checks for greater-equal in (x,y) lexicographic order
  inline bool operator>=(const Kernel_point_2& q) const { return this->_compare_xy(q) != CGAL::SMALLER; }

  //!@}

  //!\name Approximation
  //!@{

  /*! obtains a pair of doubles approximating the coordinates */
  std::pair<double, double> to_double() const {
    CGAL_precondition(this->location() == CGAL::ARR_INTERIOR);
    if (! this->ptr()->m_doubles) this->ptr()->m_doubles = this->xy().to_double();
    return *(this->ptr()->m_doubles);
  }

  //!@}

public:
  //!\name IO
  //!@{

  /*! writes point to \c os */
  void write(std::ostream& os) const {
    switch (::CGAL::IO::get_mode(os)) {
     case ::CGAL::IO::PRETTY:
      os << "point@" << this->id() << "(";
      os << "sup@" << this->curve().id() << "; ";
      os << "loc=" << this->location() << "; ";
      os << std::flush;
      // write x value
      switch (this->location()) {
       case CGAL::ARR_TOP_BOUNDARY:
       case CGAL::ARR_BOTTOM_BOUNDARY:
       case CGAL::ARR_INTERIOR: {
         os << "x=" << this->x().to_double() << "; ";
         break;
        }
       case CGAL::ARR_LEFT_BOUNDARY: {
         os << "x=-oo; ";
         break;
        }
       case CGAL::ARR_RIGHT_BOUNDARY: {
         os << "x=+oo; ";
         break;
        }
       default: {
         // bogus location
         CGAL_assertion(false);
       }
      }
      os << std::flush;

      // write y value
      switch (this->location()) {
       case CGAL::ARR_INTERIOR: os << "y=" << this->xy().to_double().second << "; "; break;
       case CGAL::ARR_TOP_BOUNDARY: os << "y=+oo; "; break;
       case CGAL::ARR_BOTTOM_BOUNDARY: os << "y=-oo; "; break;
       case CGAL::ARR_LEFT_BOUNDARY:
       case CGAL::ARR_RIGHT_BOUNDARY: {
         auto asym = asymptotic_value(this->curve(), this->location(), this->arcno());
         if (const auto* ps = std::get_if<CGAL::Arr_parameter_space>(&asym))
           os << ((*ps == CGAL::ARR_BOTTOM_BOUNDARY) ? "y=-oo(asym)" : "y=+oo(asym)") << "; ";
         else os << "y=" << CGAL::to_double(std::get<Coordinate_1>(asym)) << "(asym)" << "; ";
         break;
        }
       default: CGAL_assertion(false);
      }
      os << std::flush;
      if (this->ptr()->m_xy || this->ptr()->m_arcno) os << "ARCNO=" << this->arcno();
      else os << "ARCNO=n/a";
      os << ")";
      os << std::flush;
      break;
     case ::CGAL::IO::BINARY:
      std::cerr << "BINARY format not yet implemented" << std::endl;
      break;
     default:
      // ASCII
      os << "Point_2(";
      os << ::CGAL::IO::oformat(this->ptr()->m_xy);
      os << ",";
      os << ::CGAL::IO::oformat(this->ptr()->m_x);
      os << ",";
      os << ::CGAL::IO::oformat(this->ptr()->m_curve);
      os << ",";
      os << ::CGAL::IO::oformat(this->ptr()->m_arcno);
      os << ",";
      os << this->ptr()->m_location;

      os << ")";
    }
  }

  /*! reads point from \c is */
  void read(std::istream& is) {
    CGAL_precondition(CGAL::IO::is_ascii(is));

    Rep rep;

    // read "Point_2("
    swallow(is, 'P');
    swallow(is, 'o');
    swallow(is, 'i');
    swallow(is, 'n');
    swallow(is, 't');
    swallow(is, '_');
    swallow(is, '2');
    swallow(is, '(');

    // read values
    is >> IO::iformat(rep.m_xy);
    swallow(is, ',');
    is >> IO::iformat(rep.m_x);
    swallow(is, ',');
    is >> IO::iformat(rep.m_curve);
    swallow(is, ',');
    is >> IO::iformat(rep.m_arcno);
    swallow(is, ',');
    is >> rep.m_location;

    // read the ')'
    swallow(is, ')');

    *this = Point_2<Curved_kernel_via_analysis_2, Rep>(rep);
  }

  //! @}
  // friends ////////////////////////////////////////////////////////////////

  //! befriending arc classes
  friend class Arc_2<Curved_kernel_via_analysis_2>;

  //friend class Non_x_monotone_arc_2< Curved_kernel_via_analysis_2 >;

  // befriending the functors

#if defined(_MSC_VER)
#define CGAL_BEFRIEND_CKvA_2_FUNCTOR(Z)                                                                                \
  friend typename Curved_kernel_via_analysis_2::Z;                                                                     \
  friend typename CKvA_2_functors::Z<Curved_kernel_via_analysis_2>
#else // defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_COMPILER)
#define CGAL_BEFRIEND_CKvA_2_FUNCTOR(Z)                                                                                \
  friend class CKvA_2_functors::Z<Curved_kernel_via_analysis_2>
#endif // defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_COMPILER)

  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Construct_point_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Compare_x_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Compare_xy_2);

#undef CGAL_BEFRIEND_CKvA_2_FUNCTOR

}; // class Point_2

/*!\relates Point_2
 * writes \c pt to \c os
 */
template <typename CurvedKernelViaAnalysis_2, typename Rep_>
std::ostream& operator<<(std::ostream& os, const Point_2<CurvedKernelViaAnalysis_2, Rep_>& pt) {
  pt.write(os);
  return os;
}

//! reads the objects from stream.
template <typename CurvedKernelViaAnalysis_2, typename Rep_>
std::istream& operator>>(std::istream& is, Point_2<CurvedKernelViaAnalysis_2, Rep_>& pt) {
  CGAL_precondition(CGAL::IO::is_ascii(is));

  //typedef CurvedKernelViaAnalysis_2 Curved_kernel_via_analysis_2;
  //typedef Rep_ Rep;

  pt.read(is);

  return is;
}

} // namespace internal
} // namespace CGAL

#endif
