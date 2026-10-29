// Copyright (c) 2006, 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_MINKOWSKI_SUM_2_H
#define CGAL_MINKOWSKI_SUM_2_MINKOWSKI_SUM_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Minkowski_sum_2/Hole_filter_2.h>
#include <CGAL/Minkowski_sum_2/internal/minkowski_sum_of_convex_polygons_2.h>
#include <CGAL/Minkowski_sum_2/internal/named_parameters_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_reduced_convolution_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {
namespace internal {

/*! counts the vertices of a polygon with holes, and the vertices at which its
 * boundary cycles, in the direction of their traversal, do not turn left (the
 * reflex vertices of the outer boundary and the convex vertices of the holes,
 * as well as collinear vertices).
 * \return the pair of the counts
 */
template <typename Kernel_, typename Container_, typename Traits_>
std::pair<std::size_t, std::size_t> count_vertices(const Polygon_with_holes_2<Kernel_, Container_>& pwh,
                                                   const Traits_& traits) {
  const auto orientation = traits.orientation_2_object();
  std::size_t num_vertices = 0;
  std::size_t num_non_left = 0;
  auto count = [&](const Polygon_2<Kernel_, Container_>& pgn) {
    num_vertices += pgn.size();
    if (pgn.size() < 3) return;
    auto prev = pgn.vertices_circulator();
    auto curr = prev;
    auto next = curr;
    ++next;
    --prev;
    do {
      if (orientation(*prev, *curr, *next) != LEFT_TURN) ++num_non_left;
      prev = curr;
      curr = next;
      ++next;
    } while (curr != pgn.vertices_circulator());
  };
  count(pwh.outer_boundary());
  for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) count(*hit);
  return {num_vertices, num_non_left};
}

/*! determines whether a polygon with holes is convex after the removal of the
 * holes that cannot contribute to the boundary of the sum, given the original
 * polygon and its convexity hint, which applies to the original polygon.
 */
template <typename Kernel_, typename Container_>
bool is_filtered_convex(const Polygon_with_holes_2<Kernel_, Container_>& pwh,
                        const Polygon_with_holes_2<Kernel_, Container_>& filtered, std::optional<bool> hint) {
  if (filtered.is_unbounded() || (filtered.number_of_holes() > 0)) return false;
  if (hint && (pwh.number_of_holes() == 0)) return *hint;
  return filtered.outer_boundary().is_convex();
}

/*! chooses the method that computes the Minkowski sum of two bounded polygons
 * with holes, neither of which is convex after the removal of the holes that
 * cannot contribute to the boundary of the sum, given the filtered polygons.
 *
 * The choice is based on the estimated number \f$ E\f$ of the edges of the
 * full convolution.  A vertex at which a boundary cycle of one polygon does
 * not turn left (a reflex vertex of an outer boundary, or a convex vertex of a
 * hole) spans a cone of directions of more than half a turn, which contains
 * many of the directions of the edges of the other polygon; thus, \f$ E\f$ is
 * estimated by \f$ m + n + l_1 n + l_2 m\f$, where \f$ m\f$ and \f$ n\f$ are
 * the numbers of vertices of the polygons and \f$ l_1\f$ and \f$ l_2\f$ are
 * the numbers of their vertices that do not turn left (the estimate is an
 * upper bound, which is typically tight).  The full convolution method is
 * chosen when the convolution is sparse, that is, when \f$ E \le 8(m + n)\f$,
 * or \f$ E \le 2(m + n)\f$ if either polygon has more than 30 vertices that do
 * not turn left (the arrangement of the full convolution of a polygon that
 * folds over itself many times has many intersections); the decomposition
 * method is chosen when the convolution is very large (\f$ E > 40000\f$) and
 * both polygons, which have no holes, are highly concave (more than 30% of
 * their vertices do not turn left); otherwise, the reduced convolution method
 * is chosen.
 *
 * The thresholds were determined empirically (with the Epeck kernel), by
 * comparing the running times of the three methods on 1698 pairs of random
 * polygons: convex and concave polygons with 8 to 300 vertices, of different
 * sizes and scales and of varying ratios of reflex vertices, and polygons with
 * 1 to 16 holes paired with convex polygons, concave polygons, and polygons
 * with holes.  The rule lost 5% in total running time, and 6% in the geometric
 * mean of the running times, compared to an a-posteriori choice of the fastest
 * method for every pair (validated on a second set of 1698 pairs).
 */
template <typename Kernel_, typename Container_, typename Traits_>
Method choose_method(const Polygon_with_holes_2<Kernel_, Container_>& filtered1,
                     const Polygon_with_holes_2<Kernel_, Container_>& filtered2, const Traits_& traits) {
  const auto [m, l1] = count_vertices(filtered1, traits);
  const auto [n, l2] = count_vertices(filtered2, traits);
  const std::size_t num_edges = m + n + l1 * n + l2 * m;
  if ((num_edges > 40000) && (filtered1.number_of_holes() == 0) && (filtered2.number_of_holes() == 0) &&
      (10 * l1 > 3 * m) && (10 * l2 > 3 * n))
    return Method::DECOMPOSITION;
  const std::size_t max_density = (std::max(l1, l2) <= 30) ? 8 : 2;
  return (num_edges <= max_density * (m + n)) ? Method::FULL_CONVOLUTION : Method::REDUCED_CONVOLUTION;
}

/*! computes the Minkowski sum of two polygons, each a polygon or a polygon
 * with holes, with the method given by the named parameter `method`; see
 * `minkowski_sum_2()`.  If the method is chosen automatically, the holes that
 * cannot contribute to the boundary of the sum are removed first; if both
 * polygons are then convex, the sum is computed directly, and otherwise, the
 * method is chosen by `internal::choose_method()`.  The full convolution
 * method is applied if either polygon is unbounded.
 */
template <typename Polygon1, typename Polygon2, typename NamedParameters1, typename NamedParameters2,
          typename NamedParameters>
auto minkowski_sum_2_impl(const Polygon1& pgn1, const Polygon2& pgn2, const NamedParameters1& np1,
                          const NamedParameters2& np2, const NamedParameters& np) {
  using parameters::choose_parameter;
  using parameters::get_parameter;

  Method method = choose_parameter(get_parameter(np, internal_np::method), Method::AUTOMATIC);
  if (method == Method::AUTOMATIC) {
    if (internal::is_unbounded_polygon(pgn1) || internal::is_unbounded_polygon(pgn2)) method = Method::FULL_CONVOLUTION;
    else {
      using Kernel = typename Polygon1::Traits;
      using Container = typename Polygon1::Container;
      using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel, Container>;
      const Polygon_with_holes_2 pwh1(pgn1), pwh2(pgn2);
      Hole_filter_2<Kernel, Container> hole_filter;
      Polygon_with_holes_2 filtered1, filtered2;
      hole_filter(pwh1, pwh2, filtered1);
      hole_filter(pwh2, pwh1, filtered2);
      const auto traits_ptr = internal::shared_traits<Kernel, Container>(np);
      const auto& traits = *traits_ptr;
      if (internal::is_filtered_convex(pwh1, filtered1, internal::convexity_hint(pgn1, np1)) &&
          internal::is_filtered_convex(pwh2, filtered2, internal::convexity_hint(pgn2, np2)))
        return internal::minkowski_sum_of_convex_polygons_2(filtered1.outer_boundary(), filtered2.outer_boundary(),
                                                            traits);
      method = internal::choose_method(filtered1, filtered2, traits);
    }
  }

  switch (method) {
   case Method::DECOMPOSITION: return Minkowski_sum_2::minkowski_sum_by_decomposition_2(pgn1, pgn2, np1, np2, np);
   case Method::REDUCED_CONVOLUTION:
    return Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2(pgn1, pgn2, np1, np2, np);
   default: break;
  }
  return Minkowski_sum_2::minkowski_sum_by_full_convolution_2(pgn1, pgn2, np1, np2, np);
}

} // namespace internal

/*! computes the Minkowski sum of two polygons, with the method given by the named
 * parameter `method` of `np`: decomposition, the reduced convolution method,
 * the full convolution method, or a method chosen automatically (the
 * default).  The result is represented as a polygon with holes.
 * \param pgn1 the first polygon
 * \param pgn2 the second polygon
 * \param np1 the named parameters of the first polygon
 * \param np2 the named parameters of the second polygon
 * \param np the named parameters of the computation
 * \return the Minkowski sum
 *
 * \sa `minkowski_sum_by_reduced_convolution_2()`
 * \sa `minkowski_sum_by_decomposition_2()`
 * \sa `minkowski_sum_by_full_convolution_2()`
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                const CGAL_NP_CLASS& np = parameters::default_values())
{ return internal::minkowski_sum_2_impl(pgn1, pgn2, np1, np2, np); }

/*! computes the Minkowski sum of two polygons with holes, with the method given by the named
 * parameter `method` of `np`: decomposition, the reduced convolution method,
 * the full convolution method, or a method chosen automatically (the
 * default).  The result is represented as a polygon with holes.
 * \param pgn1 the first polygon
 * \param pgn2 the second polygon
 * \param np1 the named parameters of the first polygon
 * \param np2 the named parameters of the second polygon
 * \param np the named parameters of the computation
 * \return the Minkowski sum
 *
 * \sa `minkowski_sum_by_reduced_convolution_2()`
 * \sa `minkowski_sum_by_decomposition_2()`
 * \sa `minkowski_sum_by_full_convolution_2()`
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                const CGAL_NP_CLASS& np = parameters::default_values())
{ return internal::minkowski_sum_2_impl(pgn1, pgn2, np1, np2, np); }

/*! computes the Minkowski sum of a polygon and a polygon with holes, with the method given by the named
 * parameter `method` of `np`: decomposition, the reduced convolution method,
 * the full convolution method, or a method chosen automatically (the
 * default).  The result is represented as a polygon with holes.
 * \param pgn1 the first polygon
 * \param pgn2 the second polygon
 * \param np1 the named parameters of the first polygon
 * \param np2 the named parameters of the second polygon
 * \param np the named parameters of the computation
 * \return the Minkowski sum
 *
 * \sa `minkowski_sum_by_reduced_convolution_2()`
 * \sa `minkowski_sum_by_decomposition_2()`
 * \sa `minkowski_sum_by_full_convolution_2()`
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_2<Kernel_, Container_>& pgn1,
                const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                const CGAL_NP_CLASS& np = parameters::default_values())
{ return internal::minkowski_sum_2_impl(pgn1, pgn2, np1, np2, np); }

/*! computes the Minkowski sum of a polygon with holes and a polygon, with the method given by the named
 * parameter `method` of `np`: decomposition, the reduced convolution method,
 * the full convolution method, or a method chosen automatically (the
 * default).  The result is represented as a polygon with holes.
 * \param pgn1 the first polygon
 * \param pgn2 the second polygon
 * \param np1 the named parameters of the first polygon
 * \param np2 the named parameters of the second polygon
 * \param np the named parameters of the computation
 * \return the Minkowski sum
 *
 * \sa `minkowski_sum_by_reduced_convolution_2()`
 * \sa `minkowski_sum_by_decomposition_2()`
 * \sa `minkowski_sum_by_full_convolution_2()`
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                const Polygon_2<Kernel_, Container_>& pgn2,
                const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                const CGAL_NP_CLASS& np = parameters::default_values())
{ return internal::minkowski_sum_2_impl(pgn1, pgn2, np1, np2, np); }

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
