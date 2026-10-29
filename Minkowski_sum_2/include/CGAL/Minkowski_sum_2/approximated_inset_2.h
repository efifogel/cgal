// Copyright (c) 2006, 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Ron Wein   <wein_r@yahoo.com>
//             Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_APPROXIMATED_INSET_2_H
#define CGAL_MINKOWSKI_SUM_2_APPROXIMATED_INSET_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <CGAL/Polygon_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Minkowski_sum_2/Approx_offset_base_2.h>
#include <CGAL/Minkowski_sum_2/internal/offset_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {

/*! approximates the inset of a simple polygon by a given radius, with the
 * convolution method.
 * \param pgn the polygon
 * \param r the inset radius
 * \param eps the approximation-error bound
 * \param oi the output iterator for the inset polygons, whose value type is
 *        `Gps_circle_segment_traits_2<Kernel>::Polygon_2`
 * \param np1 the named parameters of the polygon: `is_convex`
 * \param np the named parameters of the computation: `method`
 * \return the past-the-end iterator of the output
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator
approximated_inset_2(const Polygon_2<Kernel_, Container_>& pgn, const typename Kernel_::FT& r, const double& eps,
                     OutputIterator oi,
                     const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                     const CGAL_NP_CLASS& np = parameters::default_values())
{ return internal::inset_2(Approx_offset_base_2<Kernel_, Container_>(eps), pgn, r, oi, np1, np); }

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
