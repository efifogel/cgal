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

#ifndef CGAL_MINKOWSKI_SUM_2_UNION_OF_SEGMENT_CYCLES_2_H
#define CGAL_MINKOWSKI_SUM_2_UNION_OF_SEGMENT_CYCLES_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <CGAL/Minkowski_sum_2/Union_of_cycles_2.h>
#include <CGAL/draw_arrangement_2.h>

namespace CGAL {

/*! \class
 * An auxiliary class for computing the union of the interiors of segment
 * cycles (polygon boundaries or convolution cycles).
 */
template <typename Traits_, typename Polygon_>
class Union_of_segment_cycles_2 : private Union_of_cycles_2<Traits_> {
public:
  using Traits_2 = Traits_;
  using Polygon_2 = Polygon_;

private:
  // Base-class types:
  using Base = Union_of_cycles_2<Traits_2>;
  using Point_2 = typename Base::Point_2;
  using X_monotone_curve_2 = typename Base::X_monotone_curve_2;

  using Arrangement_2 = typename Base::Arrangement_2;
  using Vertex_handle = typename Base::Vertex_handle;
  using Halfedge_handle = typename Base::Halfedge_handle;
  using Face_handle = typename Base::Face_handle;
  using Vertex_iterator = typename Base::Vertex_iterator;
  using Edge_iterator = typename Base::Edge_iterator;
  using Halfedge_iterator = typename Base::Halfedge_iterator;
  using Face_iterator = typename Base::Face_iterator;
  using Inner_ccb_iterator = typename Base::Inner_ccb_iterator;
  using Halfedge_around_vertex_circulator = typename Base::Halfedge_around_vertex_circulator;
  using Ccb_halfedge_circulator = typename Base::Ccb_halfedge_circulator;

public:
  /*! Default constructor. */
  Union_of_segment_cycles_2 () : Base() {}

  /*! computes the union of the interiors of the segment cycles.
   * \param begin An iterator for the first segment in the range.
   * \param end A past-the-end iterator for the segment range.
   * \param out_bound Output: A polygon representing the union boundary.
   * \param holes Output: An output iterator of the holes in the union.
   * \return A past-the-end iterator for the holes.
   */
  template <typename InputIterator, typename OutputIterator>
  OutputIterator operator()(InputIterator begin, InputIterator end, Polygon_2& out_bound, OutputIterator holes) const {
    // Construct the arrangement of all segments.
    Arrangement_2 arr;

    this->_construct_arrangement(begin, end, arr);
    // for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit)
    //   std::cout << "XXXX  " << fit->data() << std::endl;
    // CGAL::draw(arr, "Cycles");

    // Produce the result. First set the outer boundary of the union, given
    // as the inner boundary of the single hole in the unbounded face.
    const Face_handle uf = arr.unbounded_face();
    Inner_ccb_iterator iccb_it = uf->inner_ccbs_begin();
    Ccb_halfedge_circulator first, circ;

    out_bound.erase(out_bound.vertices_begin(), out_bound.vertices_end());

    circ = first = *iccb_it;
    do out_bound.push_back(circ->source()->point());
    while (--circ != first);
    ++iccb_it;

    // Locate the holes in the union: Go over all arrangement faces.
    for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
      CGAL_assertion (fit->data() != this->UNVISITED);

      // If a bounded face has an inside count that equals 0, it forms a hole in the union.
      if (! fit->is_unbounded() && fit->data() == 0) {
        Polygon_2 pgn_hole;
        circ = first = fit->outer_ccb();
        do pgn_hole.push_back(circ->source()->point());
        while (--circ != first);

        // Insert it to the containers of holes in the Minkowski sum.
        *holes = pgn_hole;
        ++holes;
      }
    }

    return holes;
  }
};

} // namespace CGAL

#endif
