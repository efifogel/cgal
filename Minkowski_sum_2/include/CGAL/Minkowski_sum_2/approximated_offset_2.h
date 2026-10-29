// Copyright (c) 2006, 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Ron Wein   <wein_r@yahoo.com>
//             Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_APPROXIMATED_OFFSET_2_H
#define CGAL_MINKOWSKI_SUM_2_APPROXIMATED_OFFSET_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Minkowski_sum_2/Approx_offset_base_2.h>
#include <CGAL/Minkowski_sum_2/internal/offset_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {

/*! approximates the offset of a simple polygon by a given radius, that is, its
 * Minkowski sum with a disc of the radius, with the convolution method or the
 * decomposition method, as determined by the named parameter `method` of
 * `np`.
 * \param pgn the polygon
 * \param r the offset radius
 * \param eps the approximation-error bound
 * \param np1 the named parameters of the polygon: `is_convex` and
 *        `decomposition_strategy`
 * \param np the named parameters of the computation: `method`
 * \return the approximated offset polygon
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS>
typename Gps_circle_segment_traits_2<Kernel_>::Polygon_with_holes_2
approximated_offset_2(const Polygon_2<Kernel_, Container_>& pgn, const typename Kernel_::FT& r, const double& eps,
                      const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                      const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Result = typename Gps_circle_segment_traits_2<Kernel_>::Polygon_with_holes_2;
  return internal::offset_2<Result, Kernel_, Container_>(Approx_offset_base_2<Kernel_, Container_>(eps), pgn, r,
                                                         np1, np);
}

/*! approximates the offset of a polygon with holes by a given radius, with
 * the convolution method; see the overload above.  It offsets the outer
 * boundary of the polygon and insets its holes.
 * \pre `pwh` is bounded.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS>
typename Gps_circle_segment_traits_2<Kernel_>::Polygon_with_holes_2
approximated_offset_2(const Polygon_with_holes_2<Kernel_, Container_>& pwh, const typename Kernel_::FT& r,
                      const double& eps,
                      const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                      const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Result = typename Gps_circle_segment_traits_2<Kernel_>::Polygon_with_holes_2;
  return internal::offset_2<Result, Kernel_, Container_>(Approx_offset_base_2<Kernel_, Container_>(eps), pwh, r,
                                                         np1, np);
}

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
