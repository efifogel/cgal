// Copyright (c) 2006  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Ron Wein   <wein_r@yahoo.com>

#ifndef CGAL_MINKOWSKI_SUM_2_OFFSET_DECOMP_2_H
#define CGAL_MINKOWSKI_SUM_2_OFFSET_DECOMP_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <list>

#include <CGAL/Minkowski_sum_2/Union_of_curve_cycles_2.h>

namespace CGAL {

/*! \class
 * A class for computing the offset of a given polygon by a given radius,
 * by decomposing the polygon into convex sub-polygons and computing the union
 * off their offsets.
 */
template <typename Base_, typename DecompStrategy_>
class Offset_by_decomposition_2 : private Base_ {
private:
  using Base = Base_;

  using Base::_offset_polygon;

public:
  using Kernel = typename Base::Basic_kernel;
  using NT = typename Base::Basic_NT;
  using Polygon_2 = typename Base::Polygon_2;
  using Offset_polygon_2 = typename Base::Offset_polygon_2;
  using Decomposition_strategy = DecompStrategy_;

private:
  using Polygons_list = std::list<Polygon_2>;
  using Polygons_iterator = typename Polygons_list::iterator;

  using Labeled_traits_2 = typename Base::Labeled_traits_2;
  using Labeled_curve_2 = typename Base::Labeled_curve_2;
  using Curves_list = std::list<Labeled_curve_2>;

  using Union_2 = Union_of_curve_cycles_2<Labeled_traits_2, Offset_polygon_2>;

  const Decomposition_strategy* m_decomp;

public:
  /*! constructs.
   * \param base the base offset class.
   * \param decomp the decomposition strategy; if it is null, a
   *        default-constructed strategy is used.
   */
  Offset_by_decomposition_2(const Base_& base, const Decomposition_strategy* decomp = nullptr) :
    Base (base),
    m_decomp (decomp)
  {}

  /*! computes the offset of a simple polygon by a given radius.
   * Note that as the input polygon may not be convex, its offset may not be
   * simply connected. The result is therefore represented as the outer
   * boundary of the Minkowski sum (which is always a simple offset polygon)
   * and a container of offset polygons, representing the holes in this "outer"
   * polygon.
   * \param traits Arrangement traits that can deal with line segments and
   *               circular arcs.
   * \param pgn The polygon.
   * \param r The offset radius.
   * \param off_bound Output: The outer boundary of the offset polygon.
   * \param off_holes Output: An output iterator for the holes in the offset.
   * \pre The polygon is simple.
   * \return A past-the-end iterator for the holes container.
   */
  template <typename OutputIterator>
  OutputIterator operator()(const Polygon_2& pgn, const NT& r, Offset_polygon_2& off_bound,
                            OutputIterator off_holes) const {
    CGAL_precondition (pgn.is_simple());

    // Decompose the input polygon into convex sub-polygons.
    Decomposition_strategy  default_decomp_strat;
    const Decomposition_strategy& decomp_strat = (m_decomp != nullptr) ? *m_decomp : default_decomp_strat;
    Polygons_list sub_pgns;

    decomp_strat (pgn, std::back_inserter(sub_pgns));

    // Compute the offset of each polygon separately.
    Curves_list boundary_curves;
    unsigned int pgn_id = 1;

    for (auto iter = sub_pgns.begin(); iter != sub_pgns.end(); ++iter) {
      _offset_polygon (*iter, CGAL::COUNTERCLOCKWISE, r, pgn_id, std::back_inserter(boundary_curves));
      pgn_id++;
    }

    // Compute the union of the cycles that represent the offset polygon.
    Union_2 unite;
    off_holes = unite(boundary_curves.begin(), boundary_curves.end(), off_bound, off_holes);
    return off_holes;
  }
};

} //namespace CGAL

#endif
