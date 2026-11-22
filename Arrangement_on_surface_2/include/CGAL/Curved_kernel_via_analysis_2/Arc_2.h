// Copyright (c) 2007,2008,2009,2010,2011 Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Eric Berberich <eric@mpi-inf.mpg.de>
//             Pavel Emeliyanenko <asm@mpi-sb.mpg.de>

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_ARC_2_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_ARC_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*!\file include/CGAL/Curved_kernel_via_analysis_2/Arc_2.h
 *defines class \c Arc_2 that represents an arc on a curve that
 * can be analyzed.
 */

#include <CGAL/Handle_with_policy.h>

#include <iostream>
#include <type_traits>
#include <optional>

#include <CGAL/Bbox_2.h>
#include <CGAL/Arr_enums.h>
#include <CGAL/assertions.h>
#include <CGAL/IO/io.h>

#ifndef CGAL_CKvA_USE_CACHES
#define CGAL_CKvA_USE_CACHES 1
#endif

#include <CGAL/Curved_kernel_via_analysis_2/Curved_kernel_via_analysis_2_functors.h>
#include <CGAL/Curved_kernel_via_analysis_2/Point_2.h>

namespace CGAL {
namespace internal {

#ifndef CKvA_CERR
//#define CKvA_DEBUG_PRINT_CERR
#ifdef CKvA_DEBUG_PRINT_CERR
#define CKvA_CERR(x) std::cerr << x
#else
#define CKvA_CERR(x) static_cast<void>(0)
#endif
#endif

/*! default representation class for Arc_2 */
template <typename CurvedKernelViaAnalysis_2>
class Arc_2_rep {
public:
  //!\name Public types
  //!@{

  //! this instance's first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! myself
  using Self = Arc_2_rep<Curved_kernel_via_analysis_2>;

  //! type of curve kernel
  using Curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;

  //! type of curve that can be analzed
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! type of a point on a point that can be analyzed
  using Point_2 = typename Curved_kernel_via_analysis_2::Point_2;

  //! type of boundary value in x-range of an arc
  using Bound = typename Curve_kernel_2::Bound;

  //!@}

public:
  //!\name Constructors
  //!@{

  //! constructs default
  Arc_2_rep() : m_arcno(-1), m_arcno_min(-1), m_arcno_max(-1), m_is_vertical(false), m_left_to_right(1) {}

  //! constructs copy
  Arc_2_rep(const Self& s) :
    m_min(s.m_min),
    m_max(s.m_max),
    m_support(s.m_support),
    m_arcno(s.m_arcno),
    m_arcno_min(s.m_arcno_min),
    m_arcno_max(s.m_arcno_max),
    m_is_vertical(s.m_is_vertical),
    m_left_to_right(s.m_left_to_right) {}

  //! constructs
  Arc_2_rep(const Point_2& p, const Point_2& q, const Curve_analysis_2& c, int arcno = -1, int arcno_p = -1,
            int arcno_q = -1, bool is_vertical = false) :
    m_min(p),
    m_max(q),
    m_support(c),
    m_arcno(arcno),
    m_arcno_min(arcno_p),
    m_arcno_max(arcno_q),
    m_is_vertical(is_vertical),
    m_left_to_right(1) {
    // set end-point arcnos from segment's interior
    if (m_arcno_min == -1) m_arcno_min = m_arcno;
    if (m_arcno_max == -1) m_arcno_max = m_arcno;
  }

  //!@}

public:
  //!\name Data members
  //!@{

  //! minimal end-points of an arc
  mutable Point_2 m_min;

  //! maximal end-points of an arc
  mutable Point_2 m_max;

  //! supporting curve
  mutable Curve_analysis_2 m_support;

  //! interior arcno
  mutable int m_arcno;

  //! arcno at min
  mutable int m_arcno_min;

  //! arcno at max
  mutable int m_arcno_max;

  //! indicates whether arc is vertical
  mutable bool m_is_vertical;

  //! stores a direction (left to right, right to left)
  mutable bool m_left_to_right;

  //! stores the index of an interval this arc belongs to
  mutable std::optional<int> m_interval_id;

  //! stores boundary value in x-range of non-vertical interval
  mutable std::optional<Bound> m_boundary_in_interval;

  //! stores a bbox for an arc
  mutable std::optional<CGAL::Bbox_2> m_bbox;

  //!@}
};

/*! class defines an arc on a curve that can be analyzed
 *
 * An arc is either non-vertical or vertical. If it is non-vertical,
 * we can assign a constant arc number to its interior and we may can assign
 * (non-identical) arc numbers to its end-points. It depends on whether
 * the arc touches the boundary of the parameter space or not.
 * If it is vertical, no arc number is available.
 *
 * We distinguish between interior arcs, rays, and branches. An interior arc
 * lies completely in the interior of the parameter space,
 * while a ray has one end that lies on the boundary of the parameter space,
 * and a branch has two end that lie on the boundary.
 *
 */
template <typename CurvedKernelViaAnalysis_2, typename Rep_>
class Arc_2 : public CGAL::Handle_with_policy<Rep_> {
public:
  //!\name Public types
  //!@{

  //! this instance's first template parameter
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! this instance's second template parameter
  using Rep = Rep_;

  //! this instance itself
  using Self = Arc_2<Curved_kernel_via_analysis_2, Rep>;

  //! type of curve kernel
  using Curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;

  //! type of an x-coordinate
  using Coordinate_1 = typename Curve_kernel_2::Coordinate_1;

  //! type of an xy-coordinate
  using Coordinate_2 = typename Curve_kernel_2::Coordinate_2;

  //! type of "rational" value in x-range
  using Bound = typename Curve_kernel_2::Bound;

  //! type of analysis of a pair of curves
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! type of analysis of a pair of curves
  using Curve_pair_analysis_2 = typename Curve_kernel_2::Curve_pair_analysis_2;

  //! type of a kernel point
  using Point_2 = typename Curved_kernel_via_analysis_2::Point_2;
  // Remark: Point_2 is already Kernel_point_2 -> no need to introduce it

  //! type of kernel arc
  using Kernel_arc_2 = typename Curved_kernel_via_analysis_2::Arc_2;

  //! the handle superclass
  using Base = ::CGAL::Handle_with_policy<Rep>;

  //!@}

#if ! defined(CGAL_NO_ASSERTIONS)
  static const bool s_kernel_arc_2_equals_arc_2 = std::is_same<Arc_2, Kernel_arc_2>::value;
#endif

protected:
  //! the functors operate on the representation of arcs
  template <typename CurvedKernelViaAnalysis>
  friend class CKvA_2_functors::Curved_kernel_via_analysis_2_functor_base;

public:
  //!\name Rebind
  //!{

  /*! an auxiliary structure for rebinding the arc with a NewCKvA_2 and a NewRep */
  template <typename NewCKvA_2, typename NewRep>
  class rebind {
  public:
    //! this instance's first template parameter
    using New_curved_kernel_via_analysis_2 = NewCKvA_2;

    //! this instance's second template parameter
    using New_rep = NewRep;

    //! the rebound type
    using Other = Arc_2<New_curved_kernel_via_analysis_2, NewRep>;

    //! surface point type
    using New_point_2 = typename Other::Point_2;

    //! type of rebound arc
    using Rebound_arc_2 = typename New_curved_kernel_via_analysis_2::Arc_2;

    /*! constructs supporting arc of type \c Rebound_arc_2 from the
     * (possible unbounded) \c arc
     * of type \c Self and replaces \c min and \c max point by the
     * given instances.
     *
     * All known items of the base class rep will be copied.
     *
     * \param arc Input arc
     * \param min New endpoint at min
     * \param max New endpoint at max
     * \return An arc of type \c Rebound_arc_2
     */
    Rebound_arc_2 operator()(const Self& arc, const New_point_2& min, const New_point_2& max) {
      New_rep newrep;
      newrep.m_min = min;
      newrep.m_max = max;
      copy_members(arc, newrep);
      return Rebound_arc_2(newrep);
    }

    /*! gives direct access to the minimal or maximal endpoint of an arc,
     * even if it is not interior!
     *
     * \param arc Input arc
     * \param ce Specifies which end is queried
     * \return min or max point (might be not interior!)
     */
    Point_2 operator()(const Self& arc, CGAL::Arr_curve_end ce) {
      if (ce == CGAL::ARR_MIN_END) return arc._minpoint();
      return arc._maxpoint();
    }

    // TODO move to SfA_2l
    /*! reverses rebind, that is, extracts the original arc type from a
     * rebound instance
     *
     * \param arc A rebound arc
     * \return An arc of type \c Self extracted from \c arc
     */
    Self operator()(const Rebound_arc_2& arc) {
      Rep rep;

      rep.m_min = typename New_point_2::Rebind()(arc._minpoint());
      rep.m_max = typename New_point_2::Rebind()(arc._maxpoint());

      rep.m_support = arc.ptr()->m_support;

      rep.m_arcno = arc.ptr()->m_arcno;
      rep.m_arcno_min = arc.ptr()->m_arcno_min;
      rep.m_arcno_max = arc.ptr()->m_arcno_max;

      rep.m_is_vertical = arc.ptr()->m_is_vertical;
      rep.m_left_to_right = arc.ptr()->m_left_to_right;

      rep.m_interval_id = arc.ptr()->m_interval_id;

      rep.m_boundary_in_interval = arc.ptr()->m_boundary_in_interval;

      return Self(rep);
    }

  protected:
    /*! copies main members to a rep
     *
     * \param arc Source arc
     * \param newrep Destination representation
     */
    void copy_members(const Self& arc, New_rep& newrep) {
      newrep.m_support = arc.ptr()->m_support;

      newrep.m_arcno = arc.ptr()->m_arcno;
      newrep.m_arcno_min = arc.ptr()->m_arcno_min;
      newrep.m_arcno_max = arc.ptr()->m_arcno_max;

      newrep.m_is_vertical = arc.ptr()->m_is_vertical;
      newrep.m_left_to_right = arc.ptr()->m_left_to_right;

      newrep.m_interval_id = arc.ptr()->m_interval_id;

      newrep.m_boundary_in_interval = arc.ptr()->m_boundary_in_interval;
    }
  };

  //!}

public:
  //!\name basic constructors
  //!@{

  /*! constructs default. */
  Arc_2() : Base(Rep()) {}

  /*! constructs copy */
#ifdef DOXYGEN_RUNNING
  Arc_2(const Self& a) : Base(static_cast<const Base&>(a)) {}
#endif
  //!@}

public:
  //!\name Constructors for non-vertical arcs
  //!@{

  /*! constructs an arc with two interior end-points (segment).
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
  Arc_2(const Point_2& p, const Point_2& q, const Curve_analysis_2& c, int arcno, int arcno_p, int arcno_q) :
    Base(Rep(p, q, c, arcno, arcno_p, arcno_q)) {
    CGAL_precondition(! p.is_identical(q));
    // preconditions for arc-numbers and event points (should the common
    // parts be moved to a dedicated method ?)
    CGAL_precondition(arcno >= 0);
    CGAL_precondition(arcno_p >= 0);
    CGAL_precondition(arcno_q >= 0);
    // check end-points arcnos validity and coprimality condition
    // for supporting curves
  }

  /*! constructs an arc with one interior end-point and another end
   * at the left or right boundary of the parameter space (ray I).
   *
   * \param origin The interior end-point of the ray
   * \param inf_end Defining whether the arcs emanates from the left or right
   *        boundary
   * \param c The supporting curve
   * \param arcno The arcnumber wrt \c c in the interior of the arc
   * \param arcno_o The arcnumber wrt \c c of the arc at \c origin
   * \return The constructed ray
   */
  Arc_2(const Point_2& origin, CGAL::Arr_curve_end inf_end, const Curve_analysis_2& c, int arcno, int arcno_o) :
    Base(Rep(origin, Point_2(inf_end, c, arcno), c, arcno, arcno_o)) {
    CGAL_precondition(arcno >= 0);
    CGAL_precondition(arcno_o >= 0);
    // check end-points arcnos validity and coprimality condition
    // for supporting curves

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
  Arc_2(const Point_2& origin, const Coordinate_1& asympt_x, CGAL::Arr_curve_end inf_end, const Curve_analysis_2& c,
        int arcno, int arcno_o) :
    Base(Rep(origin, Point_2(asympt_x, c, inf_end), c, arcno, arcno_o)) {
    CGAL_precondition(arcno >= 0);
    CGAL_precondition(arcno_o >= 0);

  }

  /*! constructs a non-vertical arc with two non-interior ends at the
   * left and right boundary (branch I)
   *
   * \param c The supporting curve
   * \param arcno The arcnumber wrt to \c c in the interior of the arc
   * \return The constructed branch
   */
  Arc_2(const Curve_analysis_2& c, int arcno) :
    Base(Rep(Point_2(CGAL::ARR_MIN_END, c, arcno), Point_2(CGAL::ARR_MAX_END, c, arcno), c, arcno)) {
    CGAL_precondition(arcno >= 0);
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
  Arc_2(const Coordinate_1& asympt_x1, CGAL::Arr_curve_end inf_end1, const Coordinate_1& asympt_x2,
        CGAL::Arr_curve_end inf_end2, const Curve_analysis_2& c, int arcno) :
    Base(Rep(Point_2(asympt_x1, c, inf_end1), Point_2(asympt_x2, c, inf_end2), c, arcno)) {
    CGAL_precondition(arcno >= 0);
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
  Arc_2(CGAL::Arr_curve_end inf_endx, const Coordinate_1& asympt_x, CGAL::Arr_curve_end inf_endy,
        const Curve_analysis_2& c, int arcno) :
    Base(Rep(Point_2(inf_endx, c, arcno), Point_2(asympt_x, c, inf_endy), c, arcno)) {
    CGAL_precondition(arcno >= 0);
  }

  //!@}

public:
  //!\name Constructors for vertical arcs
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
  Arc_2(const Point_2& p, const Point_2& q, const Curve_analysis_2& c) : Base(Rep(p, q, c, -1, -1, -1, true)) {
    CGAL_precondition(! p.is_identical(q));
    // check coprimality condition for supporting curves
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
  Arc_2(const Point_2& origin, CGAL::Arr_curve_end inf_end, const Curve_analysis_2& c) :
    Base(Rep(origin, Point_2(origin.x(), c, inf_end), c, -1, -1, -1, true)) {
    // check coprimality condition for supporting curves
  }

  /*! constructs a vertical arc that connects bottom with top boundary
   * (vertical branch)
   *
   * \param x The x-coordinate of the arc
   * \return The constructed branch
   *
   * \pre c must have a vertical line component at this x
   */
  Arc_2(const Coordinate_1& x, const Curve_analysis_2& c) :
    Base(Rep(Point_2(x, c, CGAL::ARR_MIN_END), Point_2(x, c, CGAL::ARR_MAX_END), c, -1, -1, -1, true)) {
  }

  //!@}

protected:
  //!\name Constructor for replace endpoints + rebind
  //!@{

  /*! constructs an arc from a given representation used in rebind
   *
   * \param rep Input representation
   * \return The constructed arc
   */
  Arc_2(Rep rep) : Base(rep) {}

  //!@}

public:
  //!\name Destructors (can be many ?)
  //!@{

  /*! destructs */
  virtual ~Arc_2() {}

  //!@}

public:
  //!\name Parameter space
  //!@{

  /*! obtains the location of the arc's end
   *
   * \param ce The intended end
   * \return The location of arc's \c ce in parameterspace
   */
  CGAL::Arr_parameter_space location(CGAL::Arr_curve_end ce) const {
    if (ce == CGAL::ARR_MIN_END) return _minpoint().location();
    return _maxpoint().location();
  }

  //!@}

  //!\name Access functions
  //!@{

  /*! checks whether a curve-end is finite
   *
   * \param ce The intended end
   * \return \c true, if finite, \c false, otherwise
   */
  bool is_finite(CGAL::Arr_curve_end ce) const {
    const Point_2& pt = (ce == CGAL::ARR_MIN_END ? _minpoint() : _maxpoint());
    return pt.is_finite();
  }

  /*! returns arc's interior curve end
   *
   * \param ce The intended end
   * \return The minimal point of the arc, or the maximal point of the arc
   *
   *  \pre accessed curve end has finite coordinates
   */
  Point_2 curve_end(CGAL::Arr_curve_end ce) const {
    const Point_2& pt = (ce == CGAL::ARR_MIN_END ? _minpoint() : _maxpoint());
#if ! CGAL_ARRANGEMENT_ON_DUPIN_CYCLIDE
    CGAL_precondition(pt.location() == CGAL::ARR_INTERIOR || pt.is_finite());
#endif
    return pt;
  }

  /*! returns x-coordinate of arc's curve end
   *
   * \param ce The intended end
   * \return x-coordinate of arc's end at \c ce
   *
   * \pre accessed curve end has finite x-coordinate
   */
  inline Coordinate_1 curve_end_x(CGAL::Arr_curve_end ce) const {
    CGAL_precondition(! (ce == CGAL::ARR_MIN_END ? _minpoint().is_on_left_right() : _maxpoint().is_on_left_right()));
    return (ce == CGAL::ARR_MIN_END ? _minpoint().x() : _maxpoint().x());
  }

  /*! obtains the supporting curve of the arc
   *
   * \return supporting curve of the arc
   */
  inline const Curve_analysis_2& curve() const { return this->ptr()->m_support; }

  /*! obtains the arc number in the interior
   *
   * \return arc number
   *
   * \pre !is_vertical()
   */
  inline int arcno() const {
    CGAL_precondition(! is_vertical());
    return this->ptr()->m_arcno;
  }

  /*! obtains the arc number of an end of the arc, which may be different from the arc number in its
   * interior
   *
   * \param ce The intended end
   * \return Arc number of intended end
   * \pre !is_vertical()
   */
  inline int arcno(CGAL::Arr_curve_end ce) const {
    CGAL_precondition(! is_vertical());
    return (ce == CGAL::ARR_MIN_END ? this->ptr()->m_arcno_min : this->ptr()->m_arcno_max);
  }

  /*! checks if the arc is vertical
   *
   * \return \c true, if vertical, \c false, otherwise
   */
  inline bool is_vertical() const { return this->ptr()->m_is_vertical; }

  /*! returns x-coordinate of vertical arc
   *
   * \return x-coordinate of line that contains vertical arc
   * \pre is_vertical
   */
  inline const Coordinate_1& x() const {
    CGAL_precondition(is_vertical());
    return _minpoint().x();
  }

  //!@}

  //!\name Direction
  //!@{

  /*! checks if the arc is direction left-to-right (min-to-max)
   *
   * \return \c true, if directed left-to-right, \c false, otherwise
   */
  inline bool is_left_to_right() const { return this->ptr()->m_left_to_right; }

  /*! flips the arc. */
  Arc_2 flip() const {
    Arc_2 opp(*this);
    opp.copy_on_write();
    opp.ptr()->m_left_to_right = ! this->ptr()->m_left_to_right;

    return opp;
  }

  //!@}

public:
  //! \name Shortcuts for code readability
  //!@{

  //! returns true if a parameter encodes an entity in the interior
  inline static bool is_interior(CGAL::Arr_parameter_space loc) { return (loc == CGAL::ARR_INTERIOR); }

  //! returns true if a parameter encodes bottom or top boundary placement
  inline static bool is_on_bottom_top(CGAL::Arr_parameter_space loc)
  { return (loc == CGAL::ARR_BOTTOM_BOUNDARY || loc == CGAL::ARR_TOP_BOUNDARY); }

  //! returns true if a parameter encodes left or right boundary placement
  inline static bool is_on_left_right(CGAL::Arr_parameter_space loc)
  { return (loc == CGAL::ARR_LEFT_BOUNDARY || loc == CGAL::ARR_RIGHT_BOUNDARY); }

  //!@}

protected:
  //!\name Protected helper methods
  //!@{

  /*! obtains the min end-point of this arc (provided for code readability)
   *
   * \return min endpoint of arc (may lie on a boundary!)
   */
  inline const Point_2& _minpoint() const { return this->ptr()->m_min; }

  /*! obtains the max end-point of this arc (provided for code readability)
   *
   * \return max endpoint of arc (may lie on a boundary!)
   */
  inline const Point_2& _maxpoint() const { return this->ptr()->m_max; }

  //!@}

public:
  //!\name IO
  //!@{

  /*! writes the arc to an output stream
   *
   * write arc to \c os
   */
  void write(std::ostream& os) const {
    switch (::CGAL::IO::get_mode(os)) {
    case ::CGAL::IO::PRETTY:
      os << "arc@" << this->id() << "[(sup@" << this->curve().id();
      if (this->is_vertical()) {
        os << ", VERTICAL at x = " << CGAL::to_double(curve_end_x(ARR_MAX_END));
      }
      else {
        os << ", ARCNO=" << this->arcno(CGAL::ARR_MIN_END) << "," << this->arcno() << ","
           << this->arcno(CGAL::ARR_MAX_END);
      }
      os << "; l2r: " << this->is_left_to_right() << "); \n";
      os << "min: " << this->_minpoint() << ";\n";
      os << "max: " << this->_maxpoint() << "]";
      break;

    case ::CGAL::IO::BINARY:
      CGAL_error_msg("the binary format is not supported");

    default:
      // ASCII
      os << "Arc_2(";
      os << this->ptr()->m_min;
      os << ",";
      os << this->ptr()->m_max;
      os << ",";
      os << this->ptr()->m_support;
      os << ",";
      os << this->ptr()->m_arcno;
      os << ",";
      os << this->ptr()->m_arcno_min;
      os << ",";
      os << this->ptr()->m_arcno_max;
      os << ",";
      os << this->ptr()->m_is_vertical;
      os << ",";
      os << this->ptr()->m_left_to_right;
      os << ")";
    }
  }

  /*! reads the arc from an input stream
   *
   * read arc from \c is
   */
  void read(std::istream& is) {
    CGAL_precondition(CGAL::IO::is_ascii(is));

    Rep rep;

    // read "Arc_2("
    swallow(is, 'A');
    swallow(is, 'r');
    swallow(is, 'c');
    swallow(is, '_');
    swallow(is, '2');
    swallow(is, '(');

    Point_2 min;
    Point_2 max;

    // read values
    is >> rep.m_min;
    swallow(is, ',');
    is >> rep.m_max;
    swallow(is, ',');
    is >> rep.m_support;
    swallow(is, ',');
    is >> rep.m_arcno;
    swallow(is, ',');
    is >> rep.m_arcno_min;
    swallow(is, ',');
    is >> rep.m_arcno_max;
    swallow(is, ',');
    is >> rep.m_is_vertical;
    swallow(is, ',');
    is >> rep.m_left_to_right;

    // read the ')'
    swallow(is, ')');

    *this = Arc_2<Curved_kernel_via_analysis_2, Rep>(rep);
  }

  //!@}

#if defined(_MSC_VER)
  // befriending the kernel point
  friend typename Curved_kernel_via_analysis_2::Point_2;

  // befriending the kernel arc
  friend typename Curved_kernel_via_analysis_2::Arc_2;

  // befriending the functors
#define CGAL_BEFRIEND_CKvA_2_FUNCTOR(Z)                                                                                \
  friend typename Curved_kernel_via_analysis_2::Z;                                                                     \
  friend typename CKvA_2_functors::Z<Curved_kernel_via_analysis_2>
#else

  // defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_COMPILER)
  // befriending the kernel point
  //friend class Curved_kernel_via_analysis_2::Point_2;

  // befriending the kernel arc
  //friend class Curved_kernel_via_analysis_2::Arc_2;

  // befriending the functors
#define CGAL_BEFRIEND_CKvA_2_FUNCTOR(Z)                         \
  friend class CKvA_2_functors::Z<Curved_kernel_via_analysis_2>
#endif

  // defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_COMPILER)

  //Curved_kernel_via_analysis_2_functors<Curved_kernel_via_analysis_2> >;

  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Construct_arc_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Is_vertical_2);

  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Construct_min_vertex_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Construct_max_vertex_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Compare_y_at_x_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Compare_y_at_x_left_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Compare_y_at_x_right_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Is_in_x_range_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Equal_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Do_overlap_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Do_intersect_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Intersect_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Trim_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Split_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Are_mergeable_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Merge_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Is_on_2);

  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Parameter_space_in_x_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Compare_y_near_boundary_2);

  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Parameter_space_in_y_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Compare_x_on_boundary_2);
  CGAL_BEFRIEND_CKvA_2_FUNCTOR(Compare_x_near_boundary_2);

#undef CGAL_BEFRIEND_CKvA_2_FUNCTOR

}; // class Arc_2

/*! \relates Arc_2
 * writes the arc to an output stream
 *
 * writes \c arc to \c os
 */
template <typename CurvedKernelViaAnalysis_2, typename Rep_>
inline std::ostream& operator<<(std::ostream& os, const Arc_2<CurvedKernelViaAnalysis_2, Rep_>& arc) {
  arc.write(os);
  return os;
}

//! reads the objects from stream.
template <typename CurvedKernelViaAnalysis_2, typename Rep_>
std::istream& operator>>(std::istream& is, Arc_2<CurvedKernelViaAnalysis_2, Rep_>& arc) {
  CGAL_precondition(CGAL::IO::is_ascii(is));
  arc.read(is);
  return is;
}

} // namespace internal
} // namespace CGAL

#endif
