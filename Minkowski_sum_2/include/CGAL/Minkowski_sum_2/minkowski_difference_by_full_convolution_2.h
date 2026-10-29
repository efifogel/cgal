// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_MINKOWSKI_DIFFERENCE_BY_FULL_CONVOLUTION_2_H
#define CGAL_MINKOWSKI_SUM_2_MINKOWSKI_DIFFERENCE_BY_FULL_CONVOLUTION_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {
namespace internal {

/*! computes the Minkowski difference of two polygons, given the convolution
 * cycles of the first polygon and the boundary cycles, reversed, of the
 * second.
 * \tparam CurveTraits_2 the traits of the curves that represent the
 *         convolution cycles; either the traits given by the user (segments)
 *         or `Arr_polyline_traits_2` of these traits.
 * \param paths the convolution cycles
 * \param ids the identifiers of the vertices of the convolution cycles, or
 *        empty
 * \param winding the winding number of the faces of the difference
 * \param oi the output iterator for the polygons with holes of the difference
 * \param traits_ptr a shared pointer to the traits
 */
template <typename CurveTraits_2, typename Kernel_, typename Container_, typename OutputIterator, typename Traits_>
OutputIterator
minkowski_difference_by_full_convolution_2_impl(const std::vector<std::vector<typename Kernel_::Point_2>>& paths,
                                                const std::vector<std::vector<std::size_t>>& ids,
                                                int winding, OutputIterator oi,
                                                const std::shared_ptr<const Traits_>& traits_ptr) {
  using Arrangement = Convolution_arrangement_2<CurveTraits_2>;

  const auto traits = make_convolution_traits<CurveTraits_2>(traits_ptr);
  Arrangement arr(&traits);
  construct_convolution_arrangement<CurveTraits_2>(paths, ids, arr);

  auto in = [winding](typename Arrangement::Face_const_handle f)
  { return f->data().m_visited && (f->data().m_winding_number == winding); };

  return extract_region<Kernel_, Container_>(arr, in, false, oi, *traits_ptr);
}

} // namespace internal

/*! computes the Minkowski difference \f$ P \ominus Q\f$ of two polygons with
 * holes, the complement of the Minkowski sum of \f$ P\f$ and the complement of
 * \f$ Q\f$, that is, the set of points \f$ x\f$ such that \f$ x - P\f$ is
 * contained in \f$ Q\f$, using the full convolution method.  The outer
 * boundaries and the holes of both polygons are treated alike.  The boundary
 * cycles of \f$ Q\f$ are reversed (the outer boundary becomes clockwise and
 * the holes become counterclockwise), so that they bound the complement of
 * \f$ Q\f$, and the convolution cycles of every boundary cycle of \f$ P\f$
 * and every reversed boundary cycle of \f$ Q\f$ are computed.  The
 * difference consists of the faces of the arrangement of these cycles whose
 * winding number is a value \f$ w\f$ determined as follows.
 *
 * The winding number is the sum of two parts.  The first is the winding number
 * with respect to the (kinetic) convolution of the boundaries, which is the
 * Euler characteristic \f$\chi\f$ of \f$ P \cap (x - \bar{Q})\f$, where
 * \f$\bar{Q}\f$ is the complement of \f$ Q\f$, less its value at infinity.
 * The second is contributed by the vertices at which the boundary of
 * \f$ P\f$ does not turn left (in the direction of its traversal, that is,
 * the reflex vertices of the outer boundary and the convex vertices of the
 * holes): the cone of such a vertex (see `find_all_convolution_edges()`) is
 * the long arc from the direction of its incoming edge to the direction of
 * its outgoing edge, which exceeds the arc of the turn by a full turn; thus,
 * the vertex \f$ v\f$ contributes, in addition, a copy of the reversed
 * boundary of \f$ Q\f$ translated by \f$ v\f$, whose winding number is
 * \f$-1\f$ at \f$ x\f$ if \f$ x - v\f$ is in \f$ Q\f$ and \f$ Q\f$ is
 * bounded, and 0 in the difference otherwise.  In the difference,
 * \f$ x - P\f$ is contained in \f$ Q\f$; hence:
 * - If \f$ P\f$ and \f$ Q\f$ are bounded, the first part is
 *   \f$-\chi(P) = -(1 - h)\f$, where \f$ h\f$ is the number of holes of
 *   \f$ P\f$, and the second is \f$-r\f$, where \f$ r\f$ is the number of
 *   vertices of all the boundary cycles of \f$ P\f$ at which they do not turn
 *   left; thus, \f$ w = -(1 - h + r)\f$.
 * - If \f$ Q\f$ is unbounded, its complement is the union of the regions
 *   \f$ H_1, \ldots, H_m\f$ bounded by its holes; the first part is the sum of
 *   \f$\chi(P \cap (x - H_j))\f$, which is 0, less \f$ m\f$ if \f$ P\f$ is
 *   unbounded as well (as \f$\chi(P \cap (x - H_j)) = 1\f$ at infinity), and
 *   the second is 0; thus, \f$ w = 0\f$ if \f$ P\f$ is bounded, and
 *   \f$ w = -m\f$ otherwise.
 * - If \f$ P\f$ is unbounded and \f$ Q\f$ is bounded, the difference is
 *   empty.
 *
 * The difference is reported as a set of polygons with holes; if it is
 * unbounded, its unbounded component is reported as a polygon with holes
 * without an outer boundary.
 * \param pgn1 the polygon with holes \f$ P\f$
 * \param pgn2 the polygon with holes \f$ Q\f$
 * \param oi the output iterator for the polygons with holes of the
 *        difference, which may be empty or disconnected
 * \param np1 the named parameters of the first polygon: `is_convex` (which is
 *        only verified)
 * \param np2 the named parameters of the second polygon: `is_convex` (which
 *        is only verified)
 * \param np the named parameters of the computation: `geom_traits` and
 *        `convolution_curves`
 * \return the past-the-end iterator of the output
 * \pre The outer boundaries of `pgn1` and `pgn2` (if any) are oriented
 *      counterclockwise, and their holes are oriented clockwise.
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator minkowski_difference_by_full_convolution_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                                          const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                                          OutputIterator oi,
                                                          const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                                          const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                                          const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Polygon_2 = CGAL::Polygon_2<Kernel, Container>;
  using Point_2 = typename Kernel::Point_2;

  // The convexity hints do not affect the difference; they are only verified.
  internal::convexity_hint(pgn1, np1);
  internal::convexity_hint(pgn2, np2);
  const auto traits_ptr = internal::shared_traits<Kernel, Container>(np);
  using Traits = std::decay_t<decltype(*traits_ptr)>;
  using Polyline_traits_2 = CGAL::Arr_polyline_traits_2<Traits>;
  const auto& traits = *traits_ptr;
  const Convolution_curves curves = internal::convolution_curves_parameter(np);

  CGAL_expensive_precondition_code(auto is_well_oriented = [](const Polygon_with_holes_2<Kernel, Container>& pwh) {
    if (! pwh.is_unbounded() && (pwh.outer_boundary().orientation() != COUNTERCLOCKWISE)) return false;
    for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit)
      if (hit->orientation() != CLOCKWISE) return false;
    return true;
  };)
  CGAL_expensive_precondition(is_well_oriented(pgn1) && is_well_oriented(pgn2));

  // An unbounded polygon is not contained in a bounded one.
  if (pgn1.is_unbounded() && ! pgn2.is_unbounded()) return oi;

  // The boundary cycles of P, and the boundary cycles of Q reversed, so that
  // they bound the complement of Q.
  std::vector<const Polygon_2*> cycles1;
  if (! pgn1.is_unbounded()) cycles1.push_back(&pgn1.outer_boundary());
  for (auto hit = pgn1.holes_begin(); hit != pgn1.holes_end(); ++hit) cycles1.push_back(&*hit);
  std::vector<Polygon_2> cycles2;
  if (! pgn2.is_unbounded()) {
    cycles2.push_back(pgn2.outer_boundary());
    cycles2.back().reverse_orientation();
  }
  for (auto hit = pgn2.holes_begin(); hit != pgn2.holes_end(); ++hit) {
    cycles2.push_back(*hit);
    cycles2.back().reverse_orientation();
  }

  // The winding number of the faces of the difference.
  int winding = 0;
  if (! pgn2.is_unbounded()) {
    // The number of vertices of the boundary cycles of P that do not turn left.
    const auto orientation = traits.orientation_2_object();
    int num_non_left = 0;
    for (const auto* cycle : cycles1) {
      if (cycle->is_empty()) continue;
      auto prev = cycle->vertices_circulator();
      auto curr = prev;
      auto next = curr;
      ++next;
      --prev;
      do {
        if (orientation(*prev, *curr, *next) != LEFT_TURN) ++num_non_left;
        prev = curr;
        curr = next;
        ++next;
      } while (curr != cycle->vertices_circulator());
    }
    winding = -(1 - static_cast<int>(pgn1.number_of_holes()) + num_non_left);
  }
  else if (pgn1.is_unbounded()) winding = -static_cast<int>(pgn2.number_of_holes());

  std::vector<std::vector<Point_2>> paths;
  std::vector<std::vector<std::size_t>> ids;
  std::size_t num_vertices = 0;
  std::size_t id_offset = 0;
  for (const auto* cycle1 : cycles1) {
    for (const auto& cycle2 : cycles2) {
      auto cycle_paths = internal::generate_convolution_paths(*cycle1, cycle2, traits_ptr, &ids, id_offset);
      paths.insert(paths.end(), std::make_move_iterator(cycle_paths.begin()),
                   std::make_move_iterator(cycle_paths.end()));
      num_vertices += cycle1->size() + cycle2.size();
      id_offset += cycle1->size() * cycle2.size();
    }
  }

  if (internal::use_segments(paths, num_vertices, curves))
    return internal::minkowski_difference_by_full_convolution_2_impl<Traits, Kernel, Container>(paths, ids, winding,
                                                                                                oi, traits_ptr);
  return internal::minkowski_difference_by_full_convolution_2_impl<Polyline_traits_2, Kernel, Container>(paths, ids,
                                                                                                         winding, oi,
                                                                                                         traits_ptr);
}

/*! computes the Minkowski difference \f$ P \ominus Q\f$ of a polygon and a
 * polygon with holes using the
 * full convolution method; see the overload above.
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator minkowski_difference_by_full_convolution_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                                          const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                                          OutputIterator oi,
                                                          const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                                          const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                                          const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Pwh = Polygon_with_holes_2<Kernel, Container>;
  return minkowski_difference_by_full_convolution_2(Pwh(pgn1), pgn2, oi, np1, np2, np);
}

/*! computes the Minkowski difference \f$ P \ominus Q\f$ of a polygon with
 * holes and a polygon using the
 * full convolution method; see the overload above.
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator minkowski_difference_by_full_convolution_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                                          const Polygon_2<Kernel_, Container_>& pgn2,
                                                          OutputIterator oi,
                                                          const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                                          const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                                          const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Pwh = Polygon_with_holes_2<Kernel, Container>;
  return minkowski_difference_by_full_convolution_2(pgn1, Pwh(pgn2), oi, np1, np2, np);
}

/*! computes the Minkowski difference \f$ P \ominus Q\f$ of two simple
 * polygons using the
 * full convolution method; see the overload above.
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator minkowski_difference_by_full_convolution_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                                          const Polygon_2<Kernel_, Container_>& pgn2,
                                                          OutputIterator oi,
                                                          const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                                          const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                                          const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Pwh = Polygon_with_holes_2<Kernel, Container>;
  return minkowski_difference_by_full_convolution_2(Pwh(pgn1), Pwh(pgn2), oi, np1, np2, np);
}

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
