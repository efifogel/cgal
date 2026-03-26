// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Efi Fogel <efifogel@gmail.com>

#ifndef BOOLEAN_SET_OPERATIONS_2_DO_POLYGON_INTERSECT_SURFACE_SWEEP_2_H
#define BOOLEAN_SET_OPERATIONS_2_DO_POLYGON_INTERSECT_SURFACE_SWEEP_2_H

#include <CGAL/license/Boolean_set_operations_2.h>

/*! \file
 */

#include <list>
#include <vector>

#include <CGAL/No_intersection_surface_sweep_2.h>
#include <CGAL/Surface_sweep_2/Random_access_output_iterator.h>
#include <CGAL/algorithm.h>

namespace CGAL {

/*! \class
 *
 * Do_polygon_intersect_surface_sweep_2 is a class that implements the Bentley and Ottmann sweep line algorithm.
 * It detects intersections.
 */

template <typename Visitor_>
class Do_polygon_intersect_surface_sweep_2 : public Surface_sweep_2::No_intersection_surface_sweep_2<Visitor_> {
public:
  using Visitor = Visitor_;

private:
  using Base = Surface_sweep_2::No_intersection_surface_sweep_2<Visitor>;

public:
  using Geometry_traits_2 = typename Base::Geometry_traits_2;
  using Event = typename Base::Event;
  using Subcurve = typename Base::Subcurve;
  using Allocator = typename Base::Allocator;

  using Traits_adaptor_2 = typename Base::Traits_adaptor_2;
  using Point_2 = typename Traits_adaptor_2::Point_2;
  using X_monotone_curve_2 = typename Traits_adaptor_2::X_monotone_curve_2;

  using Event_queue_iterator = typename Base::Event_queue_iterator;
  using Event_subcurve_iterator = typename Event::Subcurve_iterator;

  using Attribute = typename Event::Attribute;

  using Subcurve_container = std::list<Subcurve*>;
  using Subcurve_iterator = typename Subcurve_container::iterator;

  using Status_line_iterator = typename Base::Status_line_iterator;

protected:
  using All_sides_oblivious_category = typename Base::All_sides_oblivious_category;

  // Parameter spaces that are either oblivious or open cannot have points
  // on the boundary, and in particular intersection points. In other words,
  // intersection points of an oblivious or open parameter space is interior
  // by definition.
  using Sides_category = typename Base::Sides_category;

public:
  /*! constructs.
   * \param visitor A pointer to a sweep-line visitor object.
   */
  Do_polygon_intersect_surface_sweep_2(Visitor* visitor) : Base(visitor) {}

  /*! constructs.
   * \param traits A pointer to a sweep-line traits object.
   * \param visitor A pointer to a sweep-line visitor object.
   */
  Do_polygon_intersect_surface_sweep_2(const Geometry_traits_2* traits, Visitor* visitor) : Base(traits, visitor) {}

  /*! destructs. */
  virtual ~Do_polygon_intersect_surface_sweep_2() {}

  /*! runs the sweep-line algorithm on a given range of x-monotone curves.
   * \param curves_begin An iterator for the first curve in the range.
   * \param curves_end A past-the-end iterator for the range.
   * \pre The value-type of CurveInputIterator is X_monotone_curve_2.
   */
  template <typename CurveInputIterator>
  void do_intersect_sweep(CurveInputIterator curves_begin, CurveInputIterator curves_end) {
    this->m_visitor->before_sweep();
    bool overlap = _init_do_intersect_sweep(curves_begin, curves_end);
    if (! overlap) this->_sweep();
    this->_complete_sweep();
    this->m_visitor->after_sweep();
  }

  /*! runs the sweep-line algorithm on a range of x-monotone curves and a range
   * of action event points (if a curve passed through an action point, it will
   * be split).
   * \param curves_begin  An iterator for the first x-monotone curve in the
   *                      range.
   * \param curves_end A past-the-end iterator for this range.
   * \param action_points_begin An iterator for the first point in the range.
   * \param action_points_end A past-the-end iterator for this range.
   * \pre The value-type of XCurveInputIterator is the traits-class
   *      X_monotone_curve_2, and the value-type of PointInputIterator is the
   *      traits-class Point_2.
   */
  template <typename CurveInputIterator, class PointInputIterator>
  void do_intersect_sweep(CurveInputIterator curves_begin,
                          CurveInputIterator curves_end,
                          PointInputIterator action_points_begin,
                          PointInputIterator action_points_end) {
    this->m_visitor->before_sweep();
    bool overlap = _init_do_intersect_sweep(curves_begin, curves_end);
    if (! overlap) overlap = _init_do_intersect_points(action_points_begin, action_points_end, Event::ACTION);
    if (! overlap) this->_sweep();
    this->_complete_sweep();
    this->m_visitor->after_sweep();
  }

protected:
  using Subcurve_vector = typename std::vector<Subcurve*>;

  /*! creates an event object for each input point.
   * \return (true) if an overlap occurred; (false) otherwise.
   */
  template <typename PointInputIterator>
  bool _init_do_intersect_points(PointInputIterator points_begin, PointInputIterator points_end, Attribute type) {
    for (auto pit = points_begin; pit != points_end; ++pit) {
      auto overlap = this->_init_point(*pit, type);
      if (overlap) return true;
    }
    return false;
  }

  /*! creates a Subcurve object and two Event objects for each curve.
   * \return (true) if an overlap occurred; (false) otherwise.
   */
  template <typename CurveInputIterator>
  bool _init_do_intersect_curves(CurveInputIterator curves_begin, CurveInputIterator curves_end) {
    for (auto cit = curves_begin; cit != curves_end; ++cit) {
      auto overlap = this->_init_curve(*cit);
      if (overlap) return true;
    }
    return false;
  }

  /*! initializes the sweep algorithm.
   */
  template <typename CurveInputIterator>
  bool _init_do_intersect_sweep(CurveInputIterator curves_begin, CurveInputIterator curves_end) {
    this->m_num_subcurves = std::distance(curves_begin, curves_end);
    this->_init_structures();
    bool overlap = _init_do_intersect_curves(curves_begin, curves_end);     // initialize the curves
    return overlap;
  }

  /*! completes the sweep (complete data structures). */
  virtual void _complete_sweep();

  /*! Handle the subcurves to the left of the current event point. */
  virtual void _handle_left_curves();

  /*! Handle the subcurves to the right of the current event point. */
  virtual void _handle_right_curves();

  /*! Add a subcurve to the right of an event point.
   * \param event The event point.
   * \param curve The subcurve to add.
   */
  virtual bool _add_curve_to_right(Event* event, Subcurve* curve);

  /*! Compute intersections between the two given curves.
   * If the two curves intersect, create a new event (or use the event that
   * already exits in the intersection point) and insert the curves to the
   * event.
   * \param curve1 The first curve.
   * \param curve2 The second curve.
   */
  bool _do_intersect(Subcurve* c1, Subcurve* c2);

  /*! When a curve is removed from the status line for good, its top and
   * bottom neighbors become neighbors. This method finds these cases and
   * looks for the intersection point, if one exists.
   * \param leftCurve A pointer to the curve that is about to be deleted.
   * \param remove_for_good Whether the aubcurve is removed for good.
   */
  bool _remove_curve_from_status_line(Subcurve* leftCurve);
};

/*! completes the sweep (complete the data structures).
 */
template <typename Vis>
void Do_polygon_intersect_surface_sweep_2<Vis>::_complete_sweep() {
  this->m_queue->clear();
  this->m_statusLine.clear();
  this->_clear();
}

/*! handles the subcurves to the left of the current event point.
 */
template <typename Vis>
void Do_polygon_intersect_surface_sweep_2<Vis>::_handle_left_curves() {
  this->m_is_event_on_above = false;

  if (! this->m_currentEvent->has_left_curves()) {
    // In case the current event has no left subcurves incident to it, we have
    // to locate a place for it in the status line.
    this->_handle_event_without_left_curves(Sides_category());

    if (this->m_is_event_on_above) {
      this->m_visitor->found_intersection();
      return;
    }

    // The event is not located on any subcurve.
    this->m_visitor->before_handle_event(this->m_currentEvent);
    return;
  }

  this->_sort_left_curves();
  this->m_visitor->before_handle_event(this->m_currentEvent);

  // Remove all left subcurves from the status line, and inform the visitor
  // that we are done handling these subcurves.
  Event_subcurve_iterator left_iter = this->m_currentEvent->left_curves_begin();
  while (left_iter != this->m_currentEvent->left_curves_end()) {
    Subcurve* left_sc = *left_iter;
    CGAL_assertion(left_sc->right_event() == this->m_currentEvent);
    this->m_visitor->add_subcurve(left_sc->last_curve(), left_sc);
    ++left_iter;
    if (_remove_curve_from_status_line(left_sc)) return;
  }
}

/*! handles the subcurves to the right of the current event point.
 */
template <typename Vis>
void Do_polygon_intersect_surface_sweep_2<Vis>::_handle_right_curves() {
  for (auto it = this->m_currentEvent->right_curves_begin(); it != this->m_currentEvent->right_curves_end(); ++it) {
    Subcurve* subcurve = *it;
    subcurve->reset_left_event();
  }

  if (! this->m_currentEvent->has_right_curves()) return;

  auto curr = this->m_currentEvent->right_curves_begin();
  auto right_end = this->m_currentEvent->right_curves_end();

  auto sl_it = this->m_statusLine.insert_before(this->m_status_line_insert_hint, *curr);
  Subcurve* sc = *curr;
  sc->set_hint(sl_it);

  if (sl_it != this->m_statusLine.begin()) {
    auto prev = sl_it;
    --prev;
    if (_do_intersect(*prev, *sl_it)) return;
  }

  auto prev = curr;
  ++curr;
  while (curr != right_end) {
    sl_it = this->m_statusLine.insert_before(this->m_status_line_insert_hint, *curr);

    Subcurve* sc = *curr;
    sc->set_hint(sl_it);

    // If the two curves used to be neighbors before, we do not need to
    // intersect them again.
    if (! this->m_currentEvent->are_left_neighbors(*curr, *prev)) {
      if (_do_intersect(*prev, *curr)) return;
    }

    prev = curr;
    ++curr;
  }

  //the next Subcurve at the status line
  ++sl_it;
  if (sl_it != this->m_statusLine.end()) {
    if (_do_intersect(*prev, *sl_it)) return;
  }
}

/*! adds a subcurve to the right of an event point.
 */
template <typename Vis>
bool Do_polygon_intersect_surface_sweep_2<Vis>::_add_curve_to_right(Event* event, Subcurve* curve) {
  std::pair<bool, Event_subcurve_iterator> pair_res = event->add_curve_to_right(curve, this->m_traits);
  if (! pair_res.first) return false;    // no overlap
  this->m_visitor->found_intersection();
  return true;
}

/*! removes a curve from the status line.
 */
template <typename Vis>
bool Do_polygon_intersect_surface_sweep_2<Vis>::_remove_curve_from_status_line(Subcurve* sc) {
  auto sl_it = sc->hint();
  this->m_status_line_insert_hint = sl_it;
  ++(this->m_status_line_insert_hint);
  sc->set_hint(this->m_statusLine.end());

  // The subcurve is removed for good from the status line. We need
  // to check for intersection between its two neighbors (below and above)
  CGAL_assertion(sl_it != this->m_statusLine.end());
  auto last_it = this->m_statusLine.end();
  --last_it;

  if ((sl_it != this->m_statusLine.begin()) && (sl_it != last_it)) {
    auto prev = sl_it;
    --prev;
    auto next = sl_it;
    ++next;
    if (_do_intersect(*prev, *next)) return true;
  }
  this->m_statusLine.erase(sl_it);
  return false;
}

/*! computes intersections between the two given curves.
 */
template <typename Vis>
bool Do_polygon_intersect_surface_sweep_2<Vis>::_do_intersect(Subcurve* c1, Subcurve* c2) {
  CGAL_assertion(c1 != c2);
  auto do_intersect = this->m_traits->do_intersect_2_object();
  if (! do_intersect(c1->last_curve(), c2->last_curve(), false)) return false;
  this->m_visitor->found_intersection();
  return true;
}

} // namespace CGAL

#endif
