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

#ifndef CGAL_MINKOWSKI_SUM_2_UNION_OF_CURVE_CYCLES_2_H
#define CGAL_MINKOWSKI_SUM_2_UNION_OF_CURVE_CYCLES_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <CGAL/Minkowski_sum_2/Union_of_cycles_2.h>

namespace CGAL {

/*! \class
 * An auxiliary class for computing the union of the interiors of curve
 * cycles (convolution cycles).
 */
template <typename Traits_, typename GeneralPolygon_>
class Union_of_curve_cycles_2 : private Union_of_cycles_2<Traits_> {
public:
  using Traits_2 = Traits_;
  using General_polygon_2 = GeneralPolygon_;

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
  /*! \class
   * A circulator adapter that iterates over all curves in a CCB.
   */
  class Ccb_curve_iterator {
  public:
    using Self = Ccb_curve_iterator;
    using Ccb_halfedge_circulator = typename Union_of_cycles_2<Traits_>::Ccb_halfedge_circulator;
    using value_type = typename Union_of_cycles_2<Traits_>::X_monotone_curve_2;
    using iterator_category = std::forward_iterator_tag;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = int;

  private:
    Ccb_halfedge_circulator _first;       // The first halfedge.
    Ccb_halfedge_circulator _circ;        // The current circulator.
    bool _done;                           // Indicates whether we completed a full traversal of the CCB.

  public:

    /*! constructs default. */
    Ccb_curve_iterator() : _done (true) {}

    /*! constructs from a circulator.
     * \param circ A circulator for the first halfedge in the CCB.
     * \param done (true) in order to create a past-the-end iterator.
     */
    Ccb_curve_iterator(Ccb_halfedge_circulator circ, bool done = false) :
      _first(circ),
      _circ(circ),
      _done(done)
    {}

    /*! Dereference operators. */
    reference operator*() const { return _circ->curve(); }

    pointer operator->() const { return (&(_circ->curve())); }

    /*! Equality operators.*/
    bool operator==(const Self& it) const { return (_done == it._done && _circ == it._circ); }

    bool operator!=(const Self& it) const { return (_done != it._done || _circ != it._circ); }

    /*! Increment operators. */
    Self& operator++() {
      if (! _done) {
        --_circ;
        if (_circ == _first) _done = true;
      }
      return *this;
    }

    Self operator++(int) {
      Self temp = *this;
      if (! _done) {
        --_circ;
        if (_circ == _first) _done = true;
      }

      return temp;
    }

  };

  /*! constructs defauot. */
  Union_of_curve_cycles_2 () : Base() {}

  /*! computes the union of the interiors of the curve cycles.
   * \param begin An iterator for the first curve in the range.
   * \param end A past-the-end iterator for the curve range.
   * \param out_bound Output: A generalized polygon representing the outer
   *                          boundary of the union.
   * \param holes Output: An output iterator of the holes in the union.
   * \return A past-the-end iterator for the holes.
   */
  template <typename InputIterator, typename OutputIterator>
  OutputIterator operator()(InputIterator begin, InputIterator end, General_polygon_2& out_bound,
                            OutputIterator holes) const {
    // Construct the arrangement of all segments.
    Arrangement_2 arr;

    this->_construct_arrangement (begin, end, arr);

    // Produce the result. First set the outer boundary of the union, given
    // as the inner boundary of the single hole in the unbounded face.
    Face_iterator fit;
    const Face_handle uf = arr.unbounded_face();
    Inner_ccb_iterator iccb_it = uf->inner_ccbs_begin();
    Ccb_halfedge_circulator  circ;

    circ = *iccb_it;
    out_bound = General_polygon_2(Ccb_curve_iterator(circ), Ccb_curve_iterator(circ, true));

    // Locate the holes in the union: Go over all arrangement faces.
    for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
      CGAL_assertion (fit->data() != this->UNVISITED);

      // If a bounded face has an inside count that equals 0, it forms a hole
      // in the union.
      if (! fit->is_unbounded() && fit->data() == 0) {
        circ = fit->outer_ccb();
        *holes = General_polygon_2(Ccb_curve_iterator(circ), Ccb_curve_iterator(circ, true));
        ++holes;
      }
    }

    return holes;
  }


  /*! computes the inverse of the union of the interiors of the curve cycles,
   * and return the result as a sequence of polygons.
   * \param begin An iterator for the first curve in the range.
   * \param end A past-the-end iterator for the curve range.
   * \param oi Output: An output iterator of the polygons.
   * \return A past-the-end iterator for the polygons.
   */
  template <typename InputIterator, typename OutputIterator>
  OutputIterator inverse(InputIterator begin, InputIterator end, OutputIterator oi) const {
    // Construct the arrangement of all segments.
    Arrangement_2 arr;

    this->_construct_arrangement (begin, end, arr);

    // Go over all arrangement faces, and output each face whose data is negative.
    Face_iterator fit;
    Ccb_halfedge_circulator circ;

    for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
      CGAL_assertion (fit->data() != this->UNVISITED);

      if (fit->data() < 0) {
        circ = fit->outer_ccb();
        *oi = General_polygon_2(Ccb_curve_iterator(circ), Ccb_curve_iterator(circ, true));
        ++oi;
      }
    }

    return oi;
  }
};

} //namespace CGAL

#endif
