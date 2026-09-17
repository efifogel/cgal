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

#ifndef CGAL_UNION_OF_CYCLES_2_H
#define CGAL_UNION_OF_CYCLES_2_H

#include <CGAL/license/Minkowski_sum_2.h>


#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arrangement_2.h>

namespace CGAL {

/*! \class
 * An auxiliary base class for computing the union of the interiors of cycles
 * of x-monotone curves.
 */
template <typename Traits_>
class Union_of_cycles_2 {
public:
  using Traits_2 = Traits_;

protected:
  // Traits types:
  using Point_2 = typename Traits_2::Point_2;
  using X_monotone_curve_2 = typename Traits_2::X_monotone_curve_2;

  // Arrangement-related types:
  using Dcel = Arr_face_extended_dcel<Traits_2, int>;
  using Arrangement_2 = CGAL::Arrangement_2<Traits_2, Dcel>;
  using Vertex_handle = typename Arrangement_2::Vertex_handle;
  using Halfedge_handle = typename Arrangement_2::Halfedge_handle;
  using Face_handle = typename Arrangement_2::Face_handle;
  using Vertex_iterator = typename Arrangement_2::Vertex_iterator;
  using Edge_iterator = typename Arrangement_2::Edge_iterator;
  using Halfedge_iterator = typename Arrangement_2::Halfedge_iterator;
  using Face_iterator = typename Arrangement_2::Face_iterator;
  using Inner_ccb_iterator = typename Arrangement_2::Inner_ccb_iterator;
  using Halfedge_around_vertex_circulator = typename Arrangement_2::Halfedge_around_vertex_circulator;
  using Ccb_halfedge_circulator = typename Arrangement_2::Ccb_halfedge_circulator;
  // Data members:
  int UNVISITED;    // A code marking unvisited faces.

public:
  /*! Default constructor. */
  Union_of_cycles_2 () : UNVISITED(-1000000) {}

protected:
  /*! constructs the arrnagement representing the union of the curve cycles,
   * such that every arrangement face is associated with its winding number
   * with respect to the cycles.
   * \param begin An iterator for the first curve in the range.
   * \param end A past-the-end iterator for the curve range.
   * \param arr Output: The arrangement of the curve cycles, where each face
   *                    is associated with its winding number.
   */
  template <typename InputIterator>
  void _construct_arrangement(InputIterator begin, InputIterator end, Arrangement_2& arr) const {
    CGAL_precondition (arr.is_empty());

    // Construct the arrangement of the curves.
    CGAL::insert (arr, begin, end);

    // Go over all faces and mark them as unvisited, by setting their inside count to UNVISITED.
    for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) fit->set_data(UNVISITED);

    // Mark the inside count of the unbounded face as 0, and start a
    // breadth-first search from this face, going over the inner boundary of
    // the single hole (inner CCB) in the unbounded face.
    const Face_handle uf = arr.unbounded_face();
    int next_count = 0;
    Inner_ccb_iterator iccb_it = uf->inner_ccbs_begin();
    Ccb_halfedge_circulator first, circ;
    std::list<Face_handle> queue;

    uf->set_data(0);
    circ = first = *iccb_it;
    do {
      Halfedge_handle he = circ;
      Face_handle f_next = he->twin()->face();
      if (f_next->data() == UNVISITED) {
        next_count = _boundary_count(he->twin());
        f_next->set_data(next_count);
        queue.push_back(f_next);
      }
      else {
        CGAL_assertion(f_next->data() == _boundary_count(he->twin()));
      }

      ++circ;

    } while (circ != first);

    ++iccb_it;

    // Make sure that there is a single hole in the unbounded face.
    CGAL_assertion(iccb_it == uf->inner_ccbs_end());

    // The main breadth-first search loop.
    while (! queue.empty()) {
      Face_handle f_curr = queue.front();
      int curr_count = f_curr->data();
      queue.pop_front();

      // Go over the outer boundary of the current face to visit its edjacent
      // faces.
      circ = first = f_curr->outer_ccb();
      do {
        Halfedge_handle he = circ;
        Face_handle f_next = he->twin()->face();
        if (f_next->data() == UNVISITED) {
          next_count = curr_count + _boundary_count(he->twin());
          f_next->set_data(next_count);
          queue.push_back(f_next);
        }
        else if (f_curr != f_next) {
          CGAL_assertion(f_next->data() == curr_count + _boundary_count(he->twin()));
        }
      } while (++circ != first);

      // Go over the holes (inner CCBs) of the current face.
      for (iccb_it = f_curr->inner_ccbs_begin(); iccb_it != f_curr->inner_ccbs_end(); ++iccb_it) {
        circ = first = *iccb_it;
        do {
          Halfedge_handle he = circ;
          Face_handle f_next = he->twin()->face();
          if (f_next->data() == UNVISITED) {
            next_count = curr_count + _boundary_count(he->twin());
            f_next->set_data (next_count);
            queue.push_back (f_next);
          }
          else if (f_curr != f_next) {
            CGAL_assertion(f_next->data() == curr_count + _boundary_count(he->twin()));
          }
        } while (++circ != first);
      }
    }
  }

private:
  /*! computes the boundary count of the given halfedge, namely the number of
   * curves going in the halfedges direction minus the number of associated
   * curves going in the opposite direction.
   */
  int _boundary_count(Halfedge_handle he) const {
    if ((Arr_halfedge_direction) he->direction() == ARR_LEFT_TO_RIGHT)
      // Halfedge is directed from left to right:
      return (he->curve().label().right_count() - he->curve().label().left_count());
    else
      // Halfedge is directed from right to left:
      return (he->curve().label().left_count() - he->curve().label().right_count());
  }
};

} // namespace CGAL

#endif
