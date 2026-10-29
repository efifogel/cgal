// Copyright (c) 2006, 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Ron Wein   <wein_r@yahoo.com>
//             Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_OFFSET_POLYGON_2_H
#define CGAL_MINKOWSKI_SUM_2_OFFSET_POLYGON_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Minkowski_sum_2/Exact_offset_base_2.h>
#include <CGAL/Minkowski_sum_2/internal/offset_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {

/*! computes the exact offset of a simple polygon by a given radius, that is,
 * its Minkowski sum with a disc of the radius, with the convolution method or
 * the decomposition method, as determined by the named parameter `method` of
 * `np`.  The conic traits, which determine the type of the result, are a
 * mandatory argument.
 * \param pgn the polygon
 * \param r the offset radius
 * \param traits the conic traits
 * \param np1 the named parameters of the polygon: `is_convex` and
 *        `decomposition_strategy`
 * \param np the named parameters of the computation: `method`
 * \return the offset polygon
 */
template <typename ConicTraits, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS>
typename Gps_traits_2<ConicTraits>::Polygon_with_holes_2
offset_polygon_2(const Polygon_2<typename ConicTraits::Rat_kernel, Container_>& pgn,
                 const typename ConicTraits::Rat_kernel::FT& r, const ConicTraits& traits,
                 const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                 const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Result = typename Gps_traits_2<ConicTraits>::Polygon_with_holes_2;
  CGAL_USE(traits);
  return internal::offset_2<Result, typename ConicTraits::Rat_kernel, Container_>
    (Exact_offset_base_2<ConicTraits, Container_>(), pgn, r, np1, np);
}

/*! computes the exact offset of a polygon with holes by a given radius, with
 * the convolution method; see the overload above.  It offsets the outer
 * boundary of the polygon and insets its holes.
 * \pre `pwh` is bounded.
 */
template <typename ConicTraits, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS>
typename Gps_traits_2<ConicTraits>::Polygon_with_holes_2
offset_polygon_2(const Polygon_with_holes_2<typename ConicTraits::Rat_kernel, Container_>& pwh,
                 const typename ConicTraits::Rat_kernel::FT& r, const ConicTraits& traits,
                 const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                 const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Result = typename Gps_traits_2<ConicTraits>::Polygon_with_holes_2;
  CGAL_USE(traits);
  return internal::offset_2<Result, typename ConicTraits::Rat_kernel, Container_>
    (Exact_offset_base_2<ConicTraits, Container_>(), pwh, r, np1, np);
}

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
