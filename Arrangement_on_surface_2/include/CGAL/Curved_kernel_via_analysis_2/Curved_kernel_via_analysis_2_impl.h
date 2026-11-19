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

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_IMPL_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_IMPL_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*!\file include/CGAL/Curved_kernel_via_analysis_2.h
 * defines class \c Curved_kernel_via_analysis_2
 *
 * Defines points and arcs supported by curves that can be analyzed.
 */

#include <memory>
#include <utility>

#include <CGAL/tags.h>
#include <CGAL/tss.h>
#include <CGAL/Arr_tags.h>
#include <CGAL/Curved_kernel_via_analysis_2/Point_2.h>
#include <CGAL/Curved_kernel_via_analysis_2/Arc_2.h>
#include <CGAL/Curved_kernel_via_analysis_2/Curve_interval_arcno_cache.h>
#include <CGAL/Curved_kernel_via_analysis_2/Curved_kernel_via_analysis_2_functors.h>

namespace CGAL {
namespace internal {

/*! provides basic types for Curved_kernel_via_analysis_2 */
template <typename NewCKvA, typename BaseCKvA, typename CurveKernel_2,
          template <typename, typename> typename FunctorBase = Curved_kernel_via_analysis_2_functors>
class Curved_kernel_via_analysis_2_base : public FunctorBase<NewCKvA, BaseCKvA> {
public:
  //!\name Global types
  //!@{

  //! type of curve kernel
  using Curve_kernel_2 = CurveKernel_2;

  //! rebinds functor base to a new CKvA
  template <typename X>
  struct rebind {
    using Functor_base = FunctorBase<X, typename BaseCKvA::Curved_kernel_via_analysis_2>;
  };

  //!\name Embedded types to fulfill \c AosTraits_2 concept

  //! type of curve that can be analyzed
  using Curve_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! the multiplicity type
  using Multiplicity = unsigned int;

  //!@}

public:
  //!\name Tags
  //!@{

  //! tag specifies that "to the left of" comparisons are supported
  using Has_left_category = CGAL::Tag_true;

  //! tag specifies that merge and split functors are supported
  using Has_merge_category = CGAL::Tag_true;

  using Left_side_category = Arr_open_side_tag;
  using Bottom_side_category = Arr_open_side_tag;
  using Top_side_category = Arr_open_side_tag;
  using Right_side_category = Arr_open_side_tag;

  //!@}

public:
  //!\name Caching

  //!@{

  //! type of interval arcno cache
  using Curve_interval_arcno_cache = internal::Curve_interval_arcno_cache<Curve_kernel_2>;

  //!@}

public:
  //! \name Constructors
  //!@{

  //! constructs using (a copy of) the curve kernel of the calling thread
  Curved_kernel_via_analysis_2_base() : m_kernel(Curve_kernel_2::get_static_instance()) {}

  /*! constructs using (a copy of) the curve kernel \c kernel, sharing its ownership. The copy shares the caches of
   * \c kernel, which refer back to \c kernel; the shared ownership keeps \c kernel alive.
   */
  explicit Curved_kernel_via_analysis_2_base(std::shared_ptr<Curve_kernel_2> kernel) :
    m_kernel_owner(std::move(kernel)),
    m_kernel(dereference(m_kernel_owner))
  {}

  /*! constructs using a copy of \c kernel. The copy shares the caches of \c kernel, which refer back to \c kernel;
   * hence, \c kernel must outlive this object.
   */
  explicit Curved_kernel_via_analysis_2_base(const Curve_kernel_2& kernel) : m_kernel(kernel) {}

  //!@}

  //!\name underlying curve kernel + caching
  //!@{

  /*! access to static Curve_interval_arcno_cache */
  const Curve_interval_arcno_cache& interval_arcno_cache() const { return m_interval_arcno_cache; }

  /*! obtains the internal Curve_kernel_2 instance. Its functors are obtained through const member functions, and
   * its caches are shared by its copies.
   */
  const Curve_kernel_2& kernel() const { return m_kernel; }

  //!@}

protected:
  //!\name private members
  //!@{

  //! obtains the curve kernel that \c kernel points to
  static const Curve_kernel_2& dereference(const std::shared_ptr<Curve_kernel_2>& kernel) {
    CGAL_precondition(kernel != nullptr);
    return *kernel;
  }

  //! the owner of the curve kernel that the caches of \c m_kernel refer back to; empty if it is not owned
  std::shared_ptr<Curve_kernel_2> m_kernel_owner;

  //! a copy of the curve kernel, which shares its caches (kept by value to save an indirection)
  Curve_kernel_2 m_kernel;

  //! an instance of \c Curve_interval_arcno_cache
  Curve_interval_arcno_cache m_interval_arcno_cache;

  //!@}

public:
  //!\name Static Member to provide CKvA instance
  //!@{

  /*! obtains the default instance of \c Curved_kernel_via_analysis_2 of the calling thread. (A traits that holds
   * its own curved kernel does not use it.)
   *
   * \return the thread-local instance of \c Curved_kernel_via_analysis_2
   */
  static NewCKvA& instance() {
    CGAL_STATIC_THREAD_LOCAL_VARIABLE_0(NewCKvA, s_instance);
    return s_instance;
  }

  //!@}
};

} // namespace internal

template <typename CurveKernel_2>
class Curved_kernel_via_analysis_2
  : public internal::Curved_kernel_via_analysis_2_base<Curved_kernel_via_analysis_2<CurveKernel_2>, void,
                                                       CurveKernel_2> {
public:
  //! type of curve kernel
  using Curve_kernel_2 = CurveKernel_2;

  //! this instance itself
  using Self = Curved_kernel_via_analysis_2<Curve_kernel_2>;

  //! redefine rebind to terminate unrolling nested templates
  template <typename X>
  struct rebind {
    using Functor_base = internal::Curved_kernel_via_analysis_2_functors<X>;
  };

public:
  //!\name Embedded types to fulfill \c AosTraits_2 concept
  //!@{

  using Point_2 = internal::Point_2<Self>;

  using Arc_2 = internal::Arc_2<Self>;

  //! type of weakly x-monotone arc for \c AosTraits_2
  using X_monotone_curve_2 = Arc_2;

  //!@}
protected:
  //!\name Protected base types
  //!@{

  //! class collecting basic types
  using Base_kernel = internal::Curved_kernel_via_analysis_2_base<Self, void, CurveKernel_2>;

  //!@}
public:
  //! \name Constructors
  //!@{

  /*! constructs default; uses the curve kernel of the calling thread, and thus shares its caches. */
  Curved_kernel_via_analysis_2() {}

  /*! constructs from \c kernel, sharing its ownership. For example, `std::make_shared<Curve_kernel_2>()` yields a
   * curved kernel with caches of its own.
   *
   * \param kernel the curve kernel to use internally
   */
  explicit Curved_kernel_via_analysis_2(std::shared_ptr<Curve_kernel_2> kernel) : Base_kernel(std::move(kernel)) {}

  /*! constructs from a copy of \c kernel. The copy shares the caches of \c kernel, which refer back to \c kernel;
   * hence, \c kernel must outlive this object.
   *
   * \deprecated use the constructor that takes a `std::shared_ptr<Curve_kernel_2>`.
   */
  CGAL_DEPRECATED_MSG("the curve kernel must outlive the curved kernel; use the constructor that takes a "
                      "std::shared_ptr<Curve_kernel_2>")
  Curved_kernel_via_analysis_2(const Curve_kernel_2& kernel) : Base_kernel(kernel) {}

  //!@}
};

} // namespace CGAL

#endif
