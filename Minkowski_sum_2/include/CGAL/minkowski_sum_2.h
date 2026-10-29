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
//             Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_H
#define CGAL_MINKOWSKI_SUM_2_H

#include <CGAL/license/Minkowski_sum_2.h>

// The functions that compute Minkowski sums are declared in the namespace
// CGAL::Minkowski_sum_2, each in its own header file:
//   minkowski_sum_2()                        <CGAL/Minkowski_sum_2/minkowski_sum_2.h>
//   minkowski_sum_by_reduced_convolution_2() <CGAL/Minkowski_sum_2/minkowski_sum_by_reduced_convolution_2.h>
//   minkowski_sum_by_full_convolution_2()    <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>
//   minkowski_sum_by_decomposition_2()       <CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>
// This header file includes all of them, and declares the deprecated functions
// of the namespace CGAL that they replace.
#include <CGAL/Minkowski_sum_2/minkowski_sum_2.h>

#ifndef CGAL_NO_DEPRECATED_CODE

#include <iterator>
#include <list>

#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Polygon_nop_decomposition_2.h>
#include <CGAL/Minkowski_sum_2/Hole_filter_2.h>
#include <CGAL/Minkowski_sum_2/Minkowski_sum_conv_2.h>
#include <CGAL/Minkowski_sum_2/Minkowski_sum_decomp_2.h>
#include <CGAL/Minkowski_sum_2/internal/polygon_type_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {
namespace internal {

// Forward declarations, as the functions call one another.
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2);

template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1);

template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1);

template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename Decomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                            const Polygon_2<Kernel_, Container_>& pgn2,
                            const Decomposition_& decomp);

template <typename Kernel_, typename Container_, typename Decomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                            const Polygon_2<Kernel_, Container_>& pgn2,
                            const Decomposition_& decomp,
                            const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                            const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes);

template <typename Kernel_, typename Container_, typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                            const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes,
                            const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                            const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes);

template <typename Kernel_, typename Container_, typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                            const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes,
                            const Gps_segment_traits_2<Kernel_, Container_>& traits);

template <typename Kernel_, typename Container_, typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                            const Polygon_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes);

template <typename Kernel_, typename Container_, typename NoHoleDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                            const Polygon_2<Kernel_, Container_>& pgn2,
                            const NoHoleDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes,
                            const Gps_segment_traits_2<Kernel_, Container_>& traits);

/*
 * Minkowski sum by decomposition (the implementation of the deprecated functions)
 * @{
 */

/* computes the Minkowski sum of two simple polygons by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * Note that as the input polygons may not be convex, their Minkowski sum may
 * not be a simple polygon. The result is therefore represented as a polygon
 * with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2) {
  using Ms = Minkowski_sum_by_decomposition_2<DecompositionStrategy1_, DecompositionStrategy2_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy2, traits);
}

/* computes the Minkowski sum of two simple polygons by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * Note that as the input polygons may not be convex, their Minkowski sum may
 * not be a simple polygon. The result is therefore represented as a polygon
 * with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 *
 * The type of the last argument, namely,
 *   Gps_segment_traits_2<Kernel_, Container_>
 * and the type
 *   const typename Minkowski_sum_by_decomposition_2<DecompositionStrategy1_,
 *                                                   DecompositionStrategy2_,
 *                                                   Container_>::Traits_2>
 * are exchangeable except for in one case, where there is an ambiguity.
 * Thus, we use the former, even though it is less generic, as change to the
 * traits type in Minkowski_sum_by_decomposition_2 would require a similar
 * change here.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Container = Container_;
  using Decomposition_strategy1 = DecompositionStrategy1_;
  using Decomposition_strategy2 = DecompositionStrategy2_;
  using Ms = Minkowski_sum_by_decomposition_2<Decomposition_strategy1, Decomposition_strategy2, Container>;
  Ms mink_sum(decomposition_strategy1, decomposition_strategy2, traits);
  return mink_sum(pgn1, pgn2);
}

/* computes the Minkowski sum of two polygon with holes by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * The result is also represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2) {
  using Ms = Minkowski_sum_by_decomposition_2<DecompositionStrategy1_, DecompositionStrategy2_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy2, traits);
}

/* computes the Minkowski sum of two polygon with holes by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * The result is also represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Decomposition_strategy1 = DecompositionStrategy1_;
  using Decomposition_strategy2 = DecompositionStrategy2_;

  using Ms = Minkowski_sum_by_decomposition_2<Decomposition_strategy1, Decomposition_strategy2, Container>;
  Ms mink_sum(decomposition_strategy1, decomposition_strategy2, traits);
  Hole_filter_2<Kernel, Container> hole_filter;
  Polygon_with_holes_2<Kernel,Container> filtered_pgn1;
  Polygon_with_holes_2<Kernel,Container> filtered_pgn2;
  hole_filter(pgn1, pgn2, filtered_pgn1);
  hole_filter(pgn2, pgn1, filtered_pgn2);
  return mink_sum(filtered_pgn1, filtered_pgn2);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2) {
  using Ms = Minkowski_sum_by_decomposition_2<DecompositionStrategy1_, DecompositionStrategy2_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy2, traits);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The simple polygon.
 * \param[in] pgn2 The polygon with holes.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Decomposition_strategy1 = DecompositionStrategy1_;
  using Decomposition_strategy2 = DecompositionStrategy2_;

  using Ms = Minkowski_sum_by_decomposition_2<Decomposition_strategy1, Decomposition_strategy2, Container>;
  Ms mink_sum(decomposition_strategy1, decomposition_strategy2, traits);
  Hole_filter_2<Kernel, Container> hole_filter;
  Polygon_with_holes_2<Kernel, Container> filtered_pgn2;
  hole_filter(pgn2, pgn1, filtered_pgn2);
  return mink_sum(pgn1, filtered_pgn2);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The second polygon.
 * \param[in] pgn2 The first polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2) {
  using Ms = Minkowski_sum_by_decomposition_2<DecompositionStrategy1_, DecompositionStrategy2_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy2, traits);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The polygon with holes.
 * \param[in] pgn2 The simple polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_, typename DecompositionStrategy2_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const DecompositionStrategy2_& decomposition_strategy2,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits)
{ return sum_by_decomposition_2(pgn2, pgn1, decomposition_strategy2, decomposition_strategy1, traits); }

/* computes the Minkowski sum of two simple polygons by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * Note that as the input polygons may not be convex, their Minkowski sum may
 * not be a simple polygon. The result is therefore represented as a polygon
 * with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1) {
  using Ms = Minkowski_sum_by_decomposition_2<DecompositionStrategy1_, DecompositionStrategy1_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy1, traits);
}

/* computes the Minkowski sum of two simple polygons by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * Note that as the input polygons may not be convex, their Minkowski sum may
 * not be a simple polygon. The result is therefore represented as a polygon
 * with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Container = Container_;
  using Decomposition_strategy1 = DecompositionStrategy1_;

  using Ms = Minkowski_sum_by_decomposition_2<Decomposition_strategy1, Decomposition_strategy1, Container>;
  Ms mink_sum(decomposition_strategy1, decomposition_strategy1, traits);
  return mink_sum(pgn1, pgn2);
}

/* computes the Minkowski sum of two polygon with holes by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * The result is also represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1) {
  using Ms = Minkowski_sum_by_decomposition_2<DecompositionStrategy1_, DecompositionStrategy1_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy1, traits);
}

/* computes the Minkowski sum of two polygon with holes by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * The result is also represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Decomposition_strategy1 = DecompositionStrategy1_;

  using Ms = Minkowski_sum_by_decomposition_2<Decomposition_strategy1, Decomposition_strategy1, Container>;
  Ms mink_sum(decomposition_strategy1, decomposition_strategy1, traits);
  Hole_filter_2<Kernel, Container> hole_filter;
  Polygon_with_holes_2<Kernel, Container> filtered_pgn1;
  Polygon_with_holes_2<Kernel, Container> filtered_pgn2;
  hole_filter(pgn1, pgn2, filtered_pgn1);
  hole_filter(pgn2, pgn1, filtered_pgn2);
  return mink_sum(filtered_pgn1, filtered_pgn2);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1) {
  using Ms = Minkowski_sum_by_decomposition_2<DecompositionStrategy1_, DecompositionStrategy1_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy1, traits);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The simple polygon.
 * \param[in] pgn2 The polygon with holes.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Decomposition_strategy1 = DecompositionStrategy1_;

  using Ms = Minkowski_sum_by_decomposition_2<Decomposition_strategy1, Decomposition_strategy1, Container>;
  Ms mink_sum(decomposition_strategy1, decomposition_strategy1, traits);
  Hole_filter_2<Kernel, Container> hole_filter;
  Polygon_with_holes_2<Kernel,Container> filtered_pgn2;
  hole_filter(pgn2, pgn1, filtered_pgn2);
  return mink_sum(pgn1, filtered_pgn2);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The second polygon.
 * \param[in] pgn2 The first polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1) {
  using Ms = Minkowski_sum_by_decomposition_2<DecompositionStrategy1_, DecompositionStrategy1_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy1, traits);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The polygon with holes.
 * \param[in] pgn2 The simple polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename DecompositionStrategy1_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                       const Polygon_2<Kernel_, Container_>& pgn2,
                       const DecompositionStrategy1_& decomposition_strategy1,
                       const Gps_segment_traits_2<Kernel_, Container_>& traits)
{ return sum_by_decomposition_2(pgn2, pgn1, decomposition_strategy1, decomposition_strategy1, traits); }

/* computes the Minkowski sum of two simple polygons by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * Note that as the input polygons may not be convex, their Minkowski sum may
 * not be a simple polygon. The result is therefore represented as a polygon
 * with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition_strategy A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename Decomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                            const Polygon_2<Kernel_, Container_>& pgn2,
                            const Decomposition_& decomp) {
  typename Minkowski_sum_by_decomposition_2<Decomposition_, Decomposition_, Container_>::Traits_2 traits;
  return sum_by_hole_decomposition_2(pgn1, pgn2, decomp, traits);
}

/* computes the Minkowski sum of two simple polygons by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * Note that as the input polygons may not be convex, their Minkowski sum may
 * not be a simple polygon. The result is therefore represented as a polygon
 * with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename Decomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                            const Polygon_2<Kernel_, Container_>& pgn2,
                            const Decomposition_& decomp,
                            const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Decomposition = Decomposition_;
  using Nop_decomposition = Polygon_nop_decomposition_2<Kernel, Container>;

  if (pgn1.is_convex()) {
    Nop_decomposition decomp_nop;
    if (pgn2.is_convex()) {
      Minkowski_sum_by_decomposition_2<Nop_decomposition, Nop_decomposition, Container>
        mink_sum(decomp_nop, decomp_nop, traits);
      return mink_sum(pgn1, pgn2);
    }
    Minkowski_sum_by_decomposition_2<Nop_decomposition, Decomposition, Container>mink_sum(decomp_nop, decomp, traits);
    return mink_sum(pgn1, pgn2);
  }

  if (pgn2.is_convex()) {
    Nop_decomposition decomp_nop;
    Minkowski_sum_by_decomposition_2<Decomposition, Nop_decomposition, Container>mink_sum(decomp, decomp_nop, traits);
    return mink_sum(pgn1, pgn2);
  }

  Minkowski_sum_by_decomposition_2<Decomposition, Decomposition, Container>mink_sum(decomp, decomp, traits);
  return mink_sum(pgn1, pgn2);
}

/* computes the Minkowski sum of two polygon with holes by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * The result is also represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                            const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes) {
  using Ms = Minkowski_sum_by_decomposition_2<NoHolesDecomposition_, WithHolesDecomposition_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes, traits);
}

/* computes the Minkowski sum of two polygon with holes by decomposing each
 * polygon to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.
 * The result is also represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                            const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes,
                            const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Kernel = Kernel_;
  using Container = Container_;
  using No_holes_decomposition = NoHolesDecomposition_;
  using With_holes_decomposition = WithHolesDecomposition_;
  using Nop_decomposition = Polygon_nop_decomposition_2<Kernel, Container>;

  Hole_filter_2<Kernel, Container> hole_filter;
  Polygon_with_holes_2<Kernel, Container> filtered_pgn1;
  Polygon_with_holes_2<Kernel, Container> filtered_pgn2;
  hole_filter(pgn1, pgn2, filtered_pgn1);
  hole_filter(pgn2, pgn1, filtered_pgn2);

  if (0 == filtered_pgn1.number_of_holes()) {
    const Polygon_2<Kernel, Container>& pnh1 = filtered_pgn1.outer_boundary();
    if (pnh1.is_convex()) {
      // pnh1 is convex
      Nop_decomposition decomp_nop;
      if (0 == filtered_pgn2.number_of_holes()) {
        const Polygon_2<Kernel, Container>& pnh2 = filtered_pgn2.outer_boundary();
        if (pnh2.is_convex()) {
          // pnh2 is convex
          Minkowski_sum_by_decomposition_2<Nop_decomposition, Nop_decomposition, Container>
            mink_sum(decomp_nop, decomp_nop, traits);
          return mink_sum(pnh1, pnh2);
        }
        // pnh2 is concave
        Minkowski_sum_by_decomposition_2<Nop_decomposition, No_holes_decomposition, Container>
          mink_sum(decomp_nop, decomp_no_holes, traits);
        return mink_sum(pnh1, pnh2);
      }
      // pnh2 has holes
      Minkowski_sum_by_decomposition_2<Nop_decomposition, With_holes_decomposition, Container>
        mink_sum(decomp_nop, decomp_with_holes, traits);
      return mink_sum(pnh1, filtered_pgn2);
    }
    // pnh1 is concave
    if (0 == filtered_pgn2.number_of_holes()) {
      const Polygon_2<Kernel, Container>& pnh2 = filtered_pgn2.outer_boundary();
      if (pnh2.is_convex()) {
        // pnh2 is convex
        Nop_decomposition decomp_nop;
        Minkowski_sum_by_decomposition_2<No_holes_decomposition, Nop_decomposition, Container>
          mink_sum(decomp_no_holes, decomp_nop, traits);
        return mink_sum(pnh1, pnh2);
      }
      // pnh2 is concave
      Minkowski_sum_by_decomposition_2<No_holes_decomposition, No_holes_decomposition, Container>
        mink_sum(decomp_no_holes, decomp_no_holes, traits);
      return mink_sum(pnh1, pnh2);
    }
    // pnh2 has holes
    Minkowski_sum_by_decomposition_2<No_holes_decomposition, With_holes_decomposition, Container>
      mink_sum(decomp_no_holes, decomp_with_holes, traits);
    return mink_sum(pnh1, filtered_pgn2);
  }

  // filtered_pgn1 has holes
  if (0 == filtered_pgn2.number_of_holes()) {
    const Polygon_2<Kernel, Container>& pnh2 = filtered_pgn2.outer_boundary();
    if (pnh2.is_convex()) {
      // pnh2 is convex
      Nop_decomposition decomp_nop;
      Minkowski_sum_by_decomposition_2<Nop_decomposition, With_holes_decomposition, Container>
        mink_sum(decomp_nop, decomp_with_holes, traits);
      return mink_sum(pnh2, filtered_pgn1);
    }
    // pnh2 is concave
    Minkowski_sum_by_decomposition_2<No_holes_decomposition, With_holes_decomposition, Container>
      mink_sum(decomp_no_holes, decomp_with_holes, traits);
    return mink_sum(pnh2, filtered_pgn1);
  }
  // pnh2 has holes
  Minkowski_sum_by_decomposition_2<With_holes_decomposition, With_holes_decomposition, Container>
    mink_sum(decomp_with_holes, decomp_with_holes, traits);
  return mink_sum(filtered_pgn1, filtered_pgn2);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \param[in] decomposition A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_,
          typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                            const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes) {
  using Ms = Minkowski_sum_by_decomposition_2<NoHolesDecomposition_, WithHolesDecomposition_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes, traits);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The simple polygon.
 * \param[in] pgn2 The polygon with holes.
 * \param[in] decomposition A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_,typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                            const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes,
                            const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  using Kernel = Kernel_;
  using Container = Container_;
  using No_holes_decomposition = NoHolesDecomposition_;
  using With_holes_decomposition = WithHolesDecomposition_;
  using Nop_decomposition = Polygon_nop_decomposition_2<Kernel, Container>;

  Hole_filter_2<Kernel, Container> hole_filter;
  Polygon_with_holes_2<Kernel,Container> filtered_pgn2;
  hole_filter(pgn2, pgn1, filtered_pgn2);

  if (pgn1.is_convex()) {
    Nop_decomposition decomp_nop;
    if (0 == filtered_pgn2.number_of_holes()) {
      const Polygon_2<Kernel, Container>& pnh2 = filtered_pgn2.outer_boundary();
      if (pnh2.is_convex()) {
        Minkowski_sum_by_decomposition_2<Nop_decomposition, Nop_decomposition, Container>
          mink_sum(decomp_nop, decomp_nop, traits);
        return mink_sum(pgn1, pnh2);
      }
      // pnh2 is concave
      Minkowski_sum_by_decomposition_2<Nop_decomposition, No_holes_decomposition, Container>
        mink_sum(decomp_nop, decomp_no_holes, traits);
      return mink_sum(pgn1, pnh2);
    }
    // pnh2 has holes
    Minkowski_sum_by_decomposition_2<Nop_decomposition, With_holes_decomposition, Container>
      mink_sum(decomp_nop, decomp_with_holes, traits);
    return mink_sum(pgn1, filtered_pgn2);
  }
  // pgn1 is concave
  if (0 == filtered_pgn2.number_of_holes()) {
    const Polygon_2<Kernel, Container>& pnh2 = filtered_pgn2.outer_boundary();
    if (pnh2.is_convex()) {
      // pnh2 is convex
      Nop_decomposition decomp_nop;
      Minkowski_sum_by_decomposition_2<No_holes_decomposition, Nop_decomposition, Container>
        mink_sum(decomp_no_holes, decomp_nop, traits);
      return mink_sum(pgn1, pnh2);
    }
    // pnh2 is concave
    Minkowski_sum_by_decomposition_2<No_holes_decomposition, No_holes_decomposition, Container>
      mink_sum(decomp_no_holes, decomp_no_holes, traits);
    return mink_sum(pgn1, pnh2);
  }
  // pnh2 has holes
  Minkowski_sum_by_decomposition_2<No_holes_decomposition, With_holes_decomposition, Container>
    mink_sum(decomp_no_holes, decomp_with_holes, traits);
  return mink_sum(pgn1, filtered_pgn2);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The second polygon.
 * \param[in] pgn2 The first polygon.
 * \param[in] decomposition A functor for decomposing polygons.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_,typename NoHolesDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                            const Polygon_2<Kernel_, Container_>& pgn2,
                            const NoHolesDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes) {
  using Ms = Minkowski_sum_by_decomposition_2<NoHolesDecomposition_, WithHolesDecomposition_, Container_>;
  typename Ms::Traits_2 traits;
  return sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes, traits);
}

/* computes the Minkowski sum of a simple polygon and a polygon with holes
 * by decomposing each polygon to convex sub-polygons and computing the union
 * of the pairwise Minkowski sums of the sub-polygons.  The result is also
 * represented as a polygon with holes.
 * \param[in] pgn1 The polygon with holes.
 * \param[in] pgn2 The simple polygon.
 * \param[in] decomposition A functor for decomposing polygons.
 * \param[in] traits The traits.
 * \return The resulting polygon with holes, representing the sum.
 */
template <typename Kernel_, typename Container_, typename NoHoleDecomposition_, typename WithHolesDecomposition_>
Polygon_with_holes_2<Kernel_, Container_>
sum_by_hole_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                            const Polygon_2<Kernel_, Container_>& pgn2,
                            const NoHoleDecomposition_& decomp_no_holes,
                            const WithHolesDecomposition_& decomp_with_holes,
                            const Gps_segment_traits_2<Kernel_, Container_>& traits)
{ return sum_by_hole_decomposition_2(pgn2, pgn1, decomp_no_holes, decomp_with_holes, traits); }


} // namespace internal
} // namespace Minkowski_sum_2

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
//   minkowski_sum_2(p, q);
// where p and q are of type Polygon_2<Kernel, Container>, finds both
//   CGAL::Minkowski_sum_2::minkowski_sum_2() and
//   the deprecated CGAL::minkowski_sum_2(),
// if this header file is included. If the deprecated function took parameters
// of type Polygon_2<Kernel, Container>, as it used to, the two would match the
// call equally well (the named-parameter parameters of the new function, which
// have default arguments and no arguments in the call, are ignored when the
// candidates are compared), and the call would be ambiguous. As the deprecated
// function takes parameters of any type, the new function, whose parameters are
// of the more specific polygon types, is preferred. The calls resolve as
// follows:
//   CGAL::minkowski_sum_2(p, q);
//     calls the deprecated function.
//   CGAL::Minkowski_sum_2::minkowski_sum_2(p, q);
//     calls the new function.
//   using namespace CGAL;
//   minkowski_sum_2(p, q);
//     calls the deprecated function (the new function is not found).
//   using namespace CGAL::Minkowski_sum_2;
//   minkowski_sum_2(p, q);
//     calls the new function.
//
// The deprecated functions that take a decomposition strategy have no
// counterparts with the same parameters in the namespace
// CGAL::Minkowski_sum_2, so they keep their original parameter types.

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2()` instead.
 *
 * `Polygon1` and `Polygon2` can be any combination of `Polygon_2<Kernel, Container>` and
 * `Polygon_with_holes_2<Kernel, Container>`; see the comment above for why they are generic.
 */
template <typename Polygon1, typename Polygon2>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2(), declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_reduced_convolution_2.h>")
Minkowski_sum_2::internal::Sum_type_t<Polygon1, Polygon2>
minkowski_sum_by_reduced_convolution_2(const Polygon1& pgn1, const Polygon2& pgn2)
{ return Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2(pgn1, pgn2); }

/*! computes the Minkowski sum of two simple polygons using the (full)
 * convolution method.
 * Note that as the input polygons may not be convex, their Minkowski sum may
 * not be a simple polygon. The result is therefore represented as a polygon
 * with holes.
 * \param[in] pgn1 The first polygon.
 * \param[in] pgn2 The second polygon.
 * \return The resulting polygon with holes, representing the sum.
 *
 * `Polygon1` and `Polygon2` must be `Polygon_2<Kernel, Container>`; see the
 * comment above for why they are generic.
 *
 * \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2()`, declared in
 * `<CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>`, instead.
 */
template <typename Polygon1, typename Polygon2>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2(), declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>")
Minkowski_sum_2::internal::Simple_sum_type_t<Polygon1, Polygon2>
minkowski_sum_by_full_convolution_2(const Polygon1& pgn1, const Polygon2& pgn2) {
  using Polygon_with_holes = Minkowski_sum_2::internal::Simple_sum_type_t<Polygon1, Polygon2>;
  using Kernel = typename Minkowski_sum_2::internal::Polygon_type_2<Polygon1>::Kernel;
  using Container = typename Minkowski_sum_2::internal::Polygon_type_2<Polygon1>::Container;

  Minkowski_sum_by_convolution_2<Kernel, Container> mink_sum;
  Polygon_2<Kernel, Container> sum_bound;
  std::list<Polygon_2<Kernel, Container>> sum_holes;

  if (pgn1.size() > pgn2.size()) mink_sum(pgn1, pgn2, sum_bound, std::back_inserter(sum_holes));
  else mink_sum(pgn2, pgn1, sum_bound, std::back_inserter(sum_holes));
  return (Polygon_with_holes(sum_bound, sum_holes.begin(), sum_holes.end()));
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_2()` instead.
 *
 * `Polygon1` and `Polygon2` can be any combination of `Polygon_2<Kernel, Container>` and
 * `Polygon_with_holes_2<Kernel, Container>`; see the comment above for why they are generic.
 */
template <typename Polygon1, typename Polygon2>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_2(), declared in <CGAL/Minkowski_sum_2/minkowski_sum_2.h>")
Minkowski_sum_2::internal::Sum_type_t<Polygon1, Polygon2>
minkowski_sum_2(const Polygon1& pgn1, const Polygon2& pgn2)
{ return Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2(pgn1, pgn2); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const DecompositionStrategy2_& decomposition_strategy2) {
  return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2,
                                                           decomposition_strategy1, decomposition_strategy2);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const DecompositionStrategy2_& decomposition_strategy2,
                const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, decomposition_strategy2,
                                                           traits);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const DecompositionStrategy2_& decomposition_strategy2) {
  return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2,
                                                           decomposition_strategy1, decomposition_strategy2);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const DecompositionStrategy2_& decomposition_strategy2,
                const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2,
                                                           decomposition_strategy1, decomposition_strategy2, traits);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const DecompositionStrategy2_& decomposition_strategy2) {
  return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2,
                                                           decomposition_strategy1, decomposition_strategy2);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const DecompositionStrategy2_& decomposition_strategy2,
                const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2,
                                                           decomposition_strategy1, decomposition_strategy2, traits);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const DecompositionStrategy2_& decomposition_strategy2) {
  return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2,
                                                           decomposition_strategy1, decomposition_strategy2);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_, typename DecompositionStrategy2_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const DecompositionStrategy2_& decomposition_strategy2,
                const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2,
                                                           decomposition_strategy1, decomposition_strategy2, traits);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1)
{ return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const Gps_segment_traits_2<Kernel_, Container_>& traits)
{ return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, traits); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1)
{ return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const Gps_segment_traits_2<Kernel_, Container_>& traits)
{ return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, traits); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1)
{ return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const Gps_segment_traits_2<Kernel_, Container_>& traits)
{ return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, traits); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1)
{ return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename DecompositionStrategy1_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const DecompositionStrategy1_& decomposition_strategy1,
                const Gps_segment_traits_2<Kernel_, Container_>& traits)
{ return Minkowski_sum_2::internal::sum_by_decomposition_2(pgn1, pgn2, decomposition_strategy1, traits); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_, typename Decomposition_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                 const Polygon_2<Kernel_, Container_>& pgn2,
                                 const Decomposition_& decomp)
{ return Minkowski_sum_2::internal::sum_by_hole_decomposition_2(pgn1, pgn2, decomp); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_, typename Decomposition_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                 const Polygon_2<Kernel_, Container_>& pgn2,
                                 const Decomposition_& decomp,
                                 const Gps_segment_traits_2<Kernel_, Container_>& traits)
{ return Minkowski_sum_2::internal::sum_by_hole_decomposition_2(pgn1, pgn2, decomp, traits); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename NoHolesDecomposition_, typename WithHolesDecomposition_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                 const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                 const NoHolesDecomposition_& decomp_no_holes,
                                 const WithHolesDecomposition_& decomp_with_holes)
{ return Minkowski_sum_2::internal::sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename NoHolesDecomposition_, typename WithHolesDecomposition_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                 const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                 const NoHolesDecomposition_& decomp_no_holes,
                                 const WithHolesDecomposition_& decomp_with_holes,
                                 const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  return Minkowski_sum_2::internal::sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes, traits);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename NoHolesDecomposition_, typename WithHolesDecomposition_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                 const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                 const NoHolesDecomposition_& decomp_no_holes,
                                 const WithHolesDecomposition_& decomp_with_holes)
{ return Minkowski_sum_2::internal::sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename NoHolesDecomposition_, typename WithHolesDecomposition_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                 const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                 const NoHolesDecomposition_& decomp_no_holes,
                                 const WithHolesDecomposition_& decomp_with_holes,
                                 const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  return Minkowski_sum_2::internal::sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes, traits);
}

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename NoHolesDecomposition_, typename WithHolesDecomposition_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                 const Polygon_2<Kernel_, Container_>& pgn2,
                                 const NoHolesDecomposition_& decomp_no_holes,
                                 const WithHolesDecomposition_& decomp_with_holes)
{ return Minkowski_sum_2::internal::sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes); }

/*! \deprecated This function is deprecated since \cgal 6.3; use
 * `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` instead.
 */
template <typename Kernel_, typename Container_,
          typename NoHoleDecomposition_, typename WithHolesDecomposition_>
CGAL_DEPRECATED_MSG("use CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2() with the named parameters "
                    "decomposition_strategy and decomposition_strategy_with_holes of the polygons, declared in "
                    "<CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>")
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                 const Polygon_2<Kernel_, Container_>& pgn2,
                                 const NoHoleDecomposition_& decomp_no_holes,
                                 const WithHolesDecomposition_& decomp_with_holes,
                                 const Gps_segment_traits_2<Kernel_, Container_>& traits) {
  return Minkowski_sum_2::internal::sum_by_hole_decomposition_2(pgn1, pgn2, decomp_no_holes, decomp_with_holes, traits);
}

} // namespace CGAL

#endif // CGAL_NO_DEPRECATED_CODE

#endif
