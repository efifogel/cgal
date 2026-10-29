// Copyright (c) 2006  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Ron Wein   <wein_r@yahoo.com>

#ifndef CGAL_OFFSET_POLYGON_2_H
#define CGAL_OFFSET_POLYGON_2_H

#include <CGAL/license/Minkowski_sum_2.h>

// The functions that compute offsets and insets are declared in the namespace
// CGAL::Minkowski_sum_2, each in its own header file:
//   offset_polygon_2()       <CGAL/Minkowski_sum_2/offset_polygon_2.h>
//   inset_polygon_2()        <CGAL/Minkowski_sum_2/inset_polygon_2.h>
// This header file includes them, and declares the deprecated functions of the
// namespace CGAL that they replace.

#include <CGAL/Minkowski_sum_2/offset_polygon_2.h>
#include <CGAL/Minkowski_sum_2/inset_polygon_2.h>

#ifndef CGAL_NO_DEPRECATED_CODE

#include <type_traits>

#include <CGAL/Minkowski_sum_2/internal/polygon_type_2.h>

// The deprecated functions of the namespace CGAL that have counterparts with
// the same name in the namespace CGAL::Minkowski_sum_2 accept any type as a
// polygon parameter (restricted to the polygon types they used to accept by
// their return types; see <CGAL/Minkowski_sum_2/internal/polygon_type_2.h>),
// instead of a parameter of type Polygon_2<Kernel, Container> or
// Polygon_with_holes_2<Kernel, Container>. This lets the new functions take
// precedence over the deprecated ones when both are candidates of a call.
//
// When a function is called with an unqualified name, the candidates are
// looked up not only in the scopes visible at the call (including the
// namespaces named in using-directives), but also in the namespaces of the
// types of the arguments (argument-dependent lookup). The polygon types are
// declared in the namespace CGAL; thus, after
//   using namespace CGAL::Minkowski_sum_2;
// the call
//   offset_polygon_2(p, r, traits);
// where p is of type Polygon_2<Kernel, Container>, finds both
//   CGAL::Minkowski_sum_2::offset_polygon_2() and
//   the deprecated CGAL::offset_polygon_2(),
// if this header file is included. If the deprecated function took a parameter
// of type Polygon_2<Kernel, Container>, as it used to, the two would match the
// call equally well (the named-parameter parameters of the new function, which
// have default arguments and no arguments in the call, are ignored when the
// candidates are compared), and the call would be ambiguous. As the deprecated
// function takes parameters of any type, the new function, whose parameters are
// of the more specific polygon types, is preferred. The calls resolve as
// follows:
//   CGAL::offset_polygon_2(p, r, traits);
//     calls the deprecated function.
//   CGAL::Minkowski_sum_2::offset_polygon_2(p, r, traits);
//     calls the new function.
//   using namespace CGAL;
//   offset_polygon_2(p, r, traits);
//     calls the deprecated function (the new function is not found).
//   using namespace CGAL::Minkowski_sum_2;
//   offset_polygon_2(p, r, traits);
//     calls the new function.
//
// The deprecated functions that take a decomposition strategy have no
// counterparts with the same parameters in the namespace
// CGAL::Minkowski_sum_2, so they keep their original parameter types.

namespace CGAL {

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::offset_polygon_2()` instead.
 *
 * `Polygon` can be `Polygon_2<ConicTraits::Rat_kernel, Container>` or
 * `Polygon_with_holes_2<ConicTraits::Rat_kernel, Container>`; see the comment above for why it is generic.
 */
template <class Polygon, class ConicTraits>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::offset_polygon_2(), declared in <CGAL/Minkowski_sum_2/offset_polygon_2.h>")
std::enable_if_t<std::is_same_v<Minkowski_sum_2::internal::Polygon_kernel_t<Polygon>, typename ConicTraits::Rat_kernel>,
                 typename Gps_traits_2<ConicTraits>::Polygon_with_holes_2>
offset_polygon_2(const Polygon& pgn, const typename ConicTraits::Rat_kernel::FT& r, const ConicTraits& traits)
{ return Minkowski_sum_2::offset_polygon_2(pgn, r, traits); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::offset_polygon_2()` instead.
 */
template <class ConicTraits, class Container, class DecompositionStrategy>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::offset_polygon_2(), declared in <CGAL/Minkowski_sum_2/offset_polygon_2.h>")
typename Gps_traits_2<ConicTraits>::Polygon_with_holes_2
offset_polygon_2(const Polygon_2<typename ConicTraits::Rat_kernel, Container>& pgn,
                 const typename ConicTraits::Rat_kernel::FT& r, const DecompositionStrategy& decomp,
                 const ConicTraits& traits)
{ return Minkowski_sum_2::offset_polygon_2(pgn, r, traits, parameters::decomposition_strategy(decomp),
                                           parameters::method(Minkowski_sum_2::Method::DECOMPOSITION)); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::inset_polygon_2()` instead.
 *
 * `Polygon` must be `Polygon_2<ConicTraits::Rat_kernel, Container>`; see the comment above for why it is
 * generic.
 */
template <class Polygon, class ConicTraits, class OutputIterator>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::inset_polygon_2(), declared in <CGAL/Minkowski_sum_2/inset_polygon_2.h>")
std::enable_if_t<std::is_same_v<Minkowski_sum_2::internal::Simple_polygon_kernel_t<Polygon>,
                                typename ConicTraits::Rat_kernel>, OutputIterator>
inset_polygon_2(const Polygon& pgn, const typename ConicTraits::Rat_kernel::FT& r, const ConicTraits& traits,
                OutputIterator oi)
{ return Minkowski_sum_2::inset_polygon_2(pgn, r, traits, oi); }

} // namespace CGAL

#endif

#endif
