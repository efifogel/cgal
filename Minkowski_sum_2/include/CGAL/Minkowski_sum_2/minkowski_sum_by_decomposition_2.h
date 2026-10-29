// Copyright (c) 2006, 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_MINKOWSKI_SUM_BY_DECOMPOSITION_2_H
#define CGAL_MINKOWSKI_SUM_2_MINKOWSKI_SUM_BY_DECOMPOSITION_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Polygon_nop_decomposition_2.h>
#include <CGAL/Small_side_angle_bisector_decomposition_2.h>
#include <CGAL/Polygon_vertical_decomposition_2.h>
#include <CGAL/Minkowski_sum_2/Hole_filter_2.h>
#include <CGAL/Minkowski_sum_2/internal/minkowski_sum_of_convex_polygons_2.h>
#include <CGAL/Minkowski_sum_2/internal/named_parameters_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {
namespace internal {

/*! decomposes a (filtered) polygon with holes into convex polygons, oriented
 * counterclockwise: a bounded polygon without holes is decomposed by
 * `decomp_no_holes`, unless it is convex, in which case it is not decomposed;
 * a polygon with holes, or an unbounded polygon, is decomposed by
 * `decomp_with_holes`.
 */
template <typename Kernel_, typename Container_, typename NoHolesDecomposition, typename WithHolesDecomposition>
std::vector<Polygon_2<Kernel_, Container_>>
decompose_into_convex_polygons(const Polygon_with_holes_2<Kernel_, Container_>& pwh, std::optional<bool> hint,
                               const NoHolesDecomposition& decomp_no_holes,
                               const WithHolesDecomposition& decomp_with_holes) {
  using Kernel = Kernel_;
  using Container = Container_;
  std::vector<Polygon_2<Kernel, Container>> pieces;
  if (! pwh.is_unbounded() && (pwh.number_of_holes() == 0)) {
    const auto& pgn = pwh.outer_boundary();
    if (hint ? *hint : pgn.is_convex()) pieces.push_back(pgn);
    else decomp_no_holes(pgn, std::back_inserter(pieces));
  }
  else decomp_with_holes(pwh, std::back_inserter(pieces));
  CGAL_expensive_postcondition(std::all_of(pieces.begin(), pieces.end(), [](const auto& piece)
                               { return piece.is_convex() && (piece.orientation() == COUNTERCLOCKWISE); }));
  return pieces;
}

/*! merges the pairs of mergeable edges of an arrangement that are incident to
 * a vertex of degree 2.
 */
template <typename Arrangement_>
void merge_mergeable_edges(Arrangement_& arr) {
  const auto* traits = arr.geometry_traits();
  const auto are_mergeable = traits->are_mergeable_2_object();
  const auto merge = traits->merge_2_object();
  for (auto vit = arr.vertices_begin(); vit != arr.vertices_end();) {
    auto curr = vit++;
    if (curr->degree() != 2) continue;
    auto eit = curr->incident_halfedges();
    if (! are_mergeable(eit->curve(), eit->next()->curve())) continue;
    typename Arrangement_::X_monotone_curve_2 cv;
    merge(eit->curve(), eit->next()->curve(), cv);
    arr.merge_edge(eit, eit->next(), cv);
  }
}

/*! computes the union of the pairwise Minkowski sums of two sets of convex
 * polygons, oriented counterclockwise.  Every pairwise sum is computed
 * directly (see `generate_convex_path()`), and their union is computed with a
 * general polygon set.  The directions of the edges of every polygon are
 * extracted once, and its vertices are made accessible by their indices in
 * constant time once (see `Random_access_vertices`).
 * \param traits_ptr a shared pointer to the traits, a model of
 *        `MinkowskiSumDecompositionTraits_2`; the general polygon set refers
 *        to the traits, which outlive it.
 */
template <typename Kernel_, typename Container_, typename Traits_>
Polygon_with_holes_2<Kernel_, Container_>
union_of_convex_sums_2(const std::vector<Polygon_2<Kernel_, Container_>>& pieces1,
                       const std::vector<Polygon_2<Kernel_, Container_>>& pieces2,
                       const std::shared_ptr<const Traits_>& traits_ptr) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Polygon_2 = CGAL::Polygon_2<Kernel, Container>;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel, Container>;
  using Direction_2 = typename Traits_::Direction_2;
  static_assert(std::is_same_v<typename Traits_::Polygon_2, Polygon_2> &&
                std::is_same_v<typename Traits_::Polygon_with_holes_2, Polygon_with_holes_2>,
                "The polygon types of the traits must be the types of the polygons.");

  const Traits_& traits = *traits_ptr;
  auto extract = [&](const std::vector<Polygon_2>& pieces) {
    std::vector<std::vector<Direction_2>> dirs;
    dirs.reserve(pieces.size());
    for (const auto& piece : pieces) dirs.push_back(extract_directions(piece, traits));
    return dirs;
  };
  const auto dirs1 = extract(pieces1);
  const auto dirs2 = extract(pieces2);
  using Vertices = Random_access_vertices<Kernel_, Container_>;
  const std::vector<Vertices> pts1(pieces1.begin(), pieces1.end());
  const std::vector<Vertices> pts2(pieces2.begin(), pieces2.end());

  std::vector<Polygon_2> sums;
  sums.reserve(pieces1.size() * pieces2.size());
  for (std::size_t i = 0; i < pieces1.size(); ++i) {
    for (std::size_t j = 0; j < pieces2.size(); ++j) {
      auto path = generate_convex_path(pts1[i], pts2[j], dirs1[i], dirs2[j], traits);
      path.pop_back();
      sums.emplace_back(path.begin(), path.end());
    }
  }

  General_polygon_set_2<Traits_> gps(traits);
  gps.join(sums.begin(), sums.end());
  merge_mergeable_edges(gps.arrangement());
  std::vector<Polygon_with_holes_2> sum;
  gps.polygons_with_holes(std::back_inserter(sum));
  CGAL_assertion(sum.size() == 1);
  return std::move(sum.front());
}

} // namespace internal

/*! computes the Minkowski sum of two polygons with holes by decomposing each
 * one of them to convex sub-polygons and computing the union of the pairwise
 * Minkowski sums of the sub-polygons.  First, the holes that cannot contribute
 * to the boundary of the sum are removed.  A polygon that has no holes (after
 * the removal) is decomposed by its decomposition strategy, given by the named
 * parameter `decomposition_strategy` (by default,
 * `Small_side_angle_bisector_decomposition_2`), unless it is convex, in which
 * case it is not decomposed; a polygon that has holes is decomposed by its
 * decomposition strategy for polygons with holes, given by the named parameter
 * `decomposition_strategy_with_holes` (by default,
 * `Polygon_vertical_decomposition_2`).  If both polygons are convex, the sum
 * is computed directly.
 * \param pgn1 the first polygon with holes
 * \param pgn2 the second polygon with holes
 * \param np1 the named parameters of the first polygon: `is_convex`,
 *        `decomposition_strategy`, and `decomposition_strategy_with_holes`
 * \param np2 the named parameters of the second polygon: `is_convex`,
 *        `decomposition_strategy`, and `decomposition_strategy_with_holes`
 * \param np the named parameters of the computation: `geom_traits`, a model
 *        of `MinkowskiSumDecompositionTraits_2` or a shared pointer to one
 * \pre `pgn1` and `pgn2` are bounded.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                 const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                 const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                 const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                 const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel, Container>;
  using No_holes_decomposition = Small_side_angle_bisector_decomposition_2<Kernel, Container>;
  using With_holes_decomposition = Polygon_vertical_decomposition_2<Kernel, Container>;

  CGAL_precondition(! pgn1.is_unbounded() && ! pgn2.is_unbounded());
  const auto traits_ptr = internal::shared_traits<Kernel_, Container_>(np);
  const auto& traits = *traits_ptr;
  const auto hint1 = internal::convexity_hint(pgn1, np1);
  const auto hint2 = internal::convexity_hint(pgn2, np2);

  // The sum of two convex polygons is computed directly.
  if (internal::is_convex_polygon(pgn1, hint1) && internal::is_convex_polygon(pgn2, hint2))
    return internal::minkowski_sum_of_convex_polygons_2(pgn1.outer_boundary(), pgn2.outer_boundary(), traits);

  // Remove the holes that cannot contribute to the boundary of the sum.
  Hole_filter_2<Kernel_, Container_> hole_filter;
  Polygon_with_holes_2 filtered_pgn1, filtered_pgn2;
  hole_filter(pgn1, pgn2, filtered_pgn1);
  hole_filter(pgn2, pgn1, filtered_pgn2);

  // Decompose the polygons into convex polygons, and compute the union of the
  // pairwise sums of these polygons.  (A hint that a polygon is convex implies
  // that it has no holes; a hint that it is not convex only causes it to be
  // decomposed.)  A strategy given by a reference (with `std::cref()`) is not
  // copied.
  const auto decomp_no_holes1 =
    internal::shared_parameter<No_holes_decomposition>(np1, internal_np::decomposition_strategy);
  const auto decomp_with_holes1 =
    internal::shared_parameter<With_holes_decomposition>(np1, internal_np::decomposition_strategy_with_holes);
  const auto decomp_no_holes2 =
    internal::shared_parameter<No_holes_decomposition>(np2, internal_np::decomposition_strategy);
  const auto decomp_with_holes2 =
    internal::shared_parameter<With_holes_decomposition>(np2, internal_np::decomposition_strategy_with_holes);
  const auto pieces1 = internal::decompose_into_convex_polygons(filtered_pgn1, hint1, *decomp_no_holes1,
                                                                *decomp_with_holes1);
  const auto pieces2 = internal::decompose_into_convex_polygons(filtered_pgn2, hint2, *decomp_no_holes2,
                                                                *decomp_with_holes2);
  return internal::union_of_convex_sums_2(pieces1, pieces2, traits_ptr);
}

/*! computes the Minkowski sum of two polygons by decomposition; see the overload
 * above.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                 const Polygon_2<Kernel_, Container_>& pgn2,
                                 const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                 const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                 const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Pwh = Polygon_with_holes_2<Kernel, Container>;
  namespace Ms = Minkowski_sum_2;
  return Ms::minkowski_sum_by_decomposition_2(Pwh(pgn1), Pwh(pgn2), np1, np2, np);
}

/*! computes the Minkowski sum of a polygon with holes and a polygon by decomposition; see the overload
 * above.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                 const Polygon_2<Kernel_, Container_>& pgn2,
                                 const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                 const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                 const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Pwh = Polygon_with_holes_2<Kernel, Container>;
  namespace Ms = Minkowski_sum_2;
  return Ms::minkowski_sum_by_decomposition_2(pgn1, Pwh(pgn2), np1, np2, np);
}

/*! computes the Minkowski sum of a polygon and a polygon with holes by decomposition; see the overload
 * above.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_decomposition_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                 const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                 const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                 const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                 const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Pwh = Polygon_with_holes_2<Kernel, Container>;
  namespace Ms = Minkowski_sum_2;
  return Ms::minkowski_sum_by_decomposition_2(Pwh(pgn1), pgn2, np1, np2, np);
}

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
