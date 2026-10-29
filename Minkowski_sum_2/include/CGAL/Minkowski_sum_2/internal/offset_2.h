// Copyright (c) 2006, 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_INTERNAL_OFFSET_2_H
#define CGAL_MINKOWSKI_SUM_2_INTERNAL_OFFSET_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <iterator>
#include <list>
#include <type_traits>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Polygon_nop_decomposition_2.h>
#include <CGAL/Small_side_angle_bisector_decomposition_2.h>
#include <CGAL/Minkowski_sum_2/Offset_conv_2.h>
#include <CGAL/Minkowski_sum_2/Offset_decomp_2.h>
#include <CGAL/Minkowski_sum_2/internal/named_parameters_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {
namespace internal {

/*! computes the offset of a polygon (with holes) by a radius, with the method
 * given by the named parameter `method` of `np`: the convolution method
 * (`Method::AUTOMATIC` or `Method::FULL_CONVOLUTION`), or the decomposition
 * method (`Method::DECOMPOSITION`), which applies only to a simple polygon;
 * the polygon is decomposed by the strategy given by the named parameter
 * `decomposition_strategy` of `np1` (by default,
 * `Small_side_angle_bisector_decomposition_2`), unless it is convex.
 * \tparam Result the type of the offset polygon with holes
 * \param base the base offset class, which determines whether the offset is
 *        exact or approximated
 */
template <typename Result, typename Kernel_, typename Container_, typename Base, typename Polygon, typename NT,
          typename NamedParameters1, typename NamedParameters>
Result offset_2(const Base& base, const Polygon& pgn, const NT& r, const NamedParameters1& np1,
                const NamedParameters& np) {
  using parameters::choose_parameter;
  using parameters::get_parameter;
  using Offset_polygon_2 = typename Offset_by_convolution_2<Base>::Offset_polygon_2;

  const Method method = choose_parameter(get_parameter(np, internal_np::method), Method::AUTOMATIC);
  CGAL_precondition(method != Method::REDUCED_CONVOLUTION);
  const auto hint = convexity_hint(pgn, np1);

  Offset_polygon_2 offset_bound;
  std::list<Offset_polygon_2> offset_holes;
  if constexpr (std::is_same_v<Polygon, Polygon_2<Kernel_, Container_>>) {
    if (method == Method::DECOMPOSITION) {
      if (is_convex_polygon(pgn, hint)) {
        Offset_by_decomposition_2<Base, Polygon_nop_decomposition_2<Kernel_, Container_>> offset(base);
        offset(pgn, r, offset_bound, std::back_inserter(offset_holes));
      }
      else {
        // A strategy given by a reference (with `std::cref()`) is not copied.
        const auto decomp = shared_parameter<Small_side_angle_bisector_decomposition_2<Kernel_, Container_>>
          (np1, internal_np::decomposition_strategy);
        using Decomposition = std::decay_t<decltype(*decomp)>;
        Offset_by_decomposition_2<Base, Decomposition> offset(base, decomp.get());
        offset(pgn, r, offset_bound, std::back_inserter(offset_holes));
      }
      return Result(offset_bound, offset_holes.begin(), offset_holes.end());
    }
  }
  else CGAL_precondition(! pgn.is_unbounded() && (method != Method::DECOMPOSITION));

  Offset_by_convolution_2<Base> offset(base);
  offset(pgn, r, offset_bound, std::back_inserter(offset_holes));
  return Result(offset_bound, offset_holes.begin(), offset_holes.end());
}

/*! computes the inset of a simple polygon by a radius, with the convolution
 * method.
 * \param base the base offset class, which determines whether the inset is
 *        exact or approximated
 */
template <typename Base, typename Kernel_, typename Container_, typename NT, typename OutputIterator,
          typename NamedParameters1, typename NamedParameters>
OutputIterator inset_2(const Base& base, const Polygon_2<Kernel_, Container_>& pgn, const NT& r, OutputIterator oi,
                       const NamedParameters1& np1, const NamedParameters& np) {
  using parameters::choose_parameter;
  using parameters::get_parameter;
  CGAL_precondition_code(const Method method =
                           choose_parameter(get_parameter(np, internal_np::method), Method::AUTOMATIC);)
  CGAL_precondition((method == Method::AUTOMATIC) || (method == Method::FULL_CONVOLUTION));
  CGAL_USE(np);
  convexity_hint(pgn, np1);
  Offset_by_convolution_2<Base> offset(base);
  return offset.inset(pgn, r, oi);
}

} // namespace internal
} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
