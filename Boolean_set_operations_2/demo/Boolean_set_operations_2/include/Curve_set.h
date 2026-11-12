// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s) : Efi Fogel <efifogel@gmail.com>
//             Kabir Kedia <kabirkedia0111@gmail.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>
//             Apurva Bhatt <response2apurva@gmail.com>

#ifndef CGAL_CURVE_SET_H
#define CGAL_CURVE_SET_H

/*! \file Curve_set.h
 * The sets of polygons that the buckets hold:
 * - Curve_type: the curve families.
 * - Rep_base and Rep: the representation of a set of a given family, with the graphics item that draws it.
 * - Curve_set: a set of some family; the states of the undo stack share its representation until it changes.
 */

#include <memory>
#include <string>
#include <vector>

#include <QBrush>
#include <QPen>
#include <QRectF>

#include <CGAL/Qt/GraphicsItem.h>

#include "Typedefs.h"
#include "Error_handling.h"
#include "Qt6/Linear_polygons.h"
#include "Qt6/Circular_polygons.h"
#include "Qt6/Polyline_curves.h"
#include "Qt6/Bezier_curves.h"
#include "Qt6/Conic_curves.h"
#include "Qt6/Algebraic_arcs.h"

// The graphics items that draw the sets of the curve families.
using Circular_GI = CGAL::Qt::Circular_set_graphics_item<Circular_polygon_set>;
using Linear_GI = CGAL::Qt::Linear_set_graphics_item<Linear_polygon_set>;
using Polyline_GI = CGAL::Qt::Polyline_set_graphics_item<Polyline_polygon_set>;
using Bezier_GI = CGAL::Qt::Bezier_set_graphics_item<Bezier_polygon_set>;

// The curve families.
enum class Curve_type { CT_LINEAR, CT_CIRCULAR, CT_BEZIER, CT_POLYLINE, CT_CONIC, CT_ALGEBRAIC };
constexpr std::size_t number_of_curve_types = 6;

// The representation of a set of polygons of some curve family.
struct Rep_base {
  /*! destructs.
   */
  virtual ~Rep_base() {}

  /*! obtains the curve family.
   */
  virtual Curve_type type() const = 0;

  /*! constructs a copy of the set (with its own graphics item).
   */
  virtual Rep_base* clone() const = 0;

  /*! assigns another set (of the same family) to the set.
   */
  virtual void assign(const Rep_base& a_other) = 0;

  /*! obtains the graphics item that draws the set.
   */
  virtual CGAL::Qt::GraphicsItem* gi() const = 0;

  /*! obtains the graphics item that draws the set.
   */
  virtual CGAL::Qt::GraphicsItem* gi() = 0;

  /*! sets the pen of the graphics item.
   */
  virtual void set_pen(const QPen& a_pen) = 0;

  /*! sets the brush of the graphics item.
   */
  virtual void set_brush(const QBrush& a_brush) = 0;

  /*! obtains the bounding rectangle of the set.
   */
  virtual QRectF bounding_rect() const { return gi()->boundingRect(); }

  /*! updates the graphics item after the area that unbounded sets are drawn over changed.
   */
  virtual void unbounded_rect_changed() = 0;

  /*! determines whether the set is empty.
   */
  virtual bool is_empty() const = 0;

  /*! clears the set.
   */
  virtual void clear() = 0;

  /*! complements the set.
   */
  virtual void complement() = 0;

  /*! intersects the set with another set.
   */
  virtual void intersect(const Rep_base& a_other) = 0;

  /*! joins the set with another set.
   */
  virtual void join(const Rep_base& a_other) = 0;

  /*! subtracts another set from the set.
   */
  virtual void difference(const Rep_base& a_other) = 0;

  /*! replaces the set with its symmetric difference with another set.
   */
  virtual void symmetric_difference(const Rep_base& a_other) = 0;

  /*! replaces the set with the complement of another set.
   */
  virtual void complement(const Rep_base& a_other) = 0;

  /*! replaces the set with the intersection of two sets.
   */
  virtual void intersect(const Rep_base& a_first, const Rep_base& a_second) = 0;

  /*! replaces the set with the union of two sets.
   */
  virtual void join(const Rep_base& a_first, const Rep_base& a_second) = 0;

  /*! replaces the set with the difference of two sets.
   */
  virtual void difference(const Rep_base& a_first, const Rep_base& a_second) = 0;

  /*! replaces the set with the symmetric difference of two sets.
   */
  virtual void symmetric_difference(const Rep_base& a_first, const Rep_base& a_second) = 0;
};

// The representation of a set of a given curve family, with the graphics item that draws the set.
template <typename GI_, typename Set_, Curve_type type_>
class Rep : public Rep_base {
public:
  using GI = GI_;
  using Set = Set_;
  using Self = Rep<GI, Set, type_>;

  /*! constructs an empty set and its graphics item.
   */
  Rep() : m_GI(new GI(&m_set)) {}

  /*! constructs a copy of a set and its graphics item.
   */
  explicit Rep(const Set& a_set) : m_set(a_set), m_GI(new GI(&m_set)) {}

  Rep(const Rep&) = delete;
  Rep& operator=(const Rep&) = delete;

  /*! destructs the set and its graphics item (which removes the item from its scene).
   */
  ~Rep() override { delete m_GI; }

  /*! obtains the curve family.
   */
  Curve_type type() const override { return type_; }

  /*! constructs a copy of the set (with its own graphics item).
   */
  Rep_base* clone() const override { return new Self(m_set); }

  /*! obtains the set.
   */
  const Set& set() const { return m_set; }

  /*! obtains the set.
   */
  Set& set() { return m_set; }

  /*! obtains the graphics item that draws the set.
   */
  CGAL::Qt::GraphicsItem* gi() const override { return m_GI; }

  /*! obtains the graphics item that draws the set.
   */
  CGAL::Qt::GraphicsItem* gi() override { return m_GI; }

  /*! obtains the bounding rectangle of the curves of the set.
   */
  QRectF bounding_rect() const override { return m_GI->curves_rect(); }

  /*! updates the graphics item after the area that unbounded sets are drawn over changed.
   */
  void unbounded_rect_changed() override { if (m_GI->is_unbounded()) m_GI->modelChanged(); }

  /*! sets the pen of the graphics item.
   */
  void set_pen(const QPen& a_pen) override { m_GI->set_pen(a_pen); }

  /*! sets the brush of the graphics item.
   */
  void set_brush(const QBrush& a_brush) override { m_GI->set_brush(a_brush); }

  /*! determines whether the set is empty.
   */
  bool is_empty() const override { return m_set.is_empty(); }

  /*! clears the set.
   */
  void clear() override { guarded("clear", [&] { m_set.clear(); }); }

  /*! complements the set.
   */
  void complement() override { guarded("complement", [&] { m_set.complement(); }); }

  /*! assigns another set (of the same family) to the set.
   */
  void assign(const Rep_base& a_other) override { guarded("assign", [&] { m_set = cast(a_other).m_set; }); }

  /*! intersects the set with another set.
   */
  void intersect(const Rep_base& a_other) override
  { guarded("intersect", [&] { m_set.intersection(cast(a_other).m_set); }); }

  /*! joins the set with another set.
   */
  void join(const Rep_base& a_other) override { guarded("union", [&] { m_set.join(cast(a_other).m_set); }); }

  /*! subtracts another set from the set.
   */
  void difference(const Rep_base& a_other) override
  { guarded("difference", [&] { m_set.difference(cast(a_other).m_set); }); }

  /*! replaces the set with its symmetric difference with another set.
   */
  void symmetric_difference(const Rep_base& a_other) override
  { guarded("symmetric difference", [&] { m_set.symmetric_difference(cast(a_other).m_set); }); }

  /*! replaces the set with the complement of another set.
   */
  void complement(const Rep_base& a_other) override
  { guarded("complement", [&] { m_set.complement(cast(a_other).m_set); }); }

  /*! replaces the set with the intersection of two sets.
   */
  void intersect(const Rep_base& a_first, const Rep_base& a_second) override
  { guarded("intersect", [&] { m_set.intersection(cast(a_first).m_set, cast(a_second).m_set); }); }

  /*! replaces the set with the union of two sets.
   */
  void join(const Rep_base& a_first, const Rep_base& a_second) override
  { guarded("union", [&] { m_set.join(cast(a_first).m_set, cast(a_second).m_set); }); }

  /*! replaces the set with the difference of two sets.
   */
  void difference(const Rep_base& a_first, const Rep_base& a_second) override
  { guarded("difference", [&] { m_set.difference(cast(a_first).m_set, cast(a_second).m_set); }); }

  /*! replaces the set with the symmetric difference of two sets.
   */
  void symmetric_difference(const Rep_base& a_first, const Rep_base& a_second) override
  { guarded("symmetric difference", [&] { m_set.symmetric_difference(cast(a_first).m_set, cast(a_second).m_set); }); }

  /*! casts a set representation to this type.
   */
  static const Self& cast(const Rep_base& a_other) { return dynamic_cast<const Self&>(a_other); }

  /*! casts a set representation to this type.
   */
  static Self& cast(Rep_base& a_other) { return dynamic_cast<Self&>(a_other); }

private:
  /*! applies an operation to the set; shows an error if it throws.
   */
  template <typename Operation>
  static void guarded(const char* a_name, Operation a_operation) {
    try {
      a_operation();
    }
    catch (...) {
      show_error(std::string("Exception thrown during boolean operation ") + a_name);
    }
  }

  Set m_set;
  GI* m_GI;
};

using Linear_rep = Rep<Linear_GI, Linear_polygon_set, Curve_type::CT_LINEAR>;
using Circular_rep = Rep<Circular_GI, Circular_polygon_set, Curve_type::CT_CIRCULAR>;
using Bezier_rep = Rep<Bezier_GI, Bezier_polygon_set, Curve_type::CT_BEZIER>;
using Polyline_rep = Rep<Polyline_GI, Polyline_polygon_set, Curve_type::CT_POLYLINE>;
using Conic_rep = Rep<CGAL::Qt::Conic_GI, Conic_polygon_set, Curve_type::CT_CONIC>;
using Algebraic_rep = Rep<CGAL::Qt::Algebraic_GI, Algebraic_polygon_set, Curve_type::CT_ALGEBRAIC>;

// A set of polygons of some curve family, drawn with a given pen and brush.
class Curve_set {
  using Rep_ptr = std::shared_ptr<Rep_base>;

public:
  /*! constructs an empty set of a given curve family, drawn with a given pen and brush.
   */
  Curve_set(Curve_type a_type, QPen a_pen, QBrush a_brush) : m_pen(a_pen), m_brush(a_brush) { reset_type(a_type); }

  /*! replaces the set with an empty set of a given curve family.
   */
  void reset_type(Curve_type a_type) {
    m_rep = Rep_ptr(new_rep(a_type));
    m_rep->set_pen(m_pen);
    m_rep->set_brush(m_brush);
  }

  /*! constructs an empty set of a given curve family.
   */
  static Rep_base* new_rep(Curve_type a_type) {
    switch (a_type) {
     case Curve_type::CT_LINEAR: return new Linear_rep();
     case Curve_type::CT_CIRCULAR: return new Circular_rep();
     case Curve_type::CT_BEZIER: return new Bezier_rep();
     case Curve_type::CT_POLYLINE: return new Polyline_rep();
     case Curve_type::CT_CONIC: return new Conic_rep();
     case Curve_type::CT_ALGEBRAIC: return new Algebraic_rep();
    }
    return nullptr;
  }

  /*! replaces the representation of the set with a given one, keeping the pen, the brush, and the visibility.
   */
  void replace_rep(Rep_base* a_rep) {
    bool visible = m_rep->gi()->isVisible();
    m_rep = Rep_ptr(a_rep);
    m_rep->set_pen(m_pen);
    m_rep->set_brush(m_brush);
    m_rep->gi()->setVisible(visible);
  }

  /*! obtains the graphics item that draws the set.
   */
  const CGAL::Qt::GraphicsItem* gi() const { return m_rep->gi(); }

  /*! obtains the graphics item that draws the set.
   */
  CGAL::Qt::GraphicsItem* gi() { return m_rep->gi(); }

  /*! obtains the bounding rectangle of the set.
   */
  QRectF bounding_rect() const { return m_rep->bounding_rect(); }

  /*! updates the graphics item after the area that unbounded sets are drawn over changed.
   */
  void unbounded_rect_changed() { m_rep->unbounded_rect_changed(); }

  /*! determines whether the set is empty.
   */
  bool is_empty() const { return ! m_rep || m_rep->is_empty(); }

  /*! makes the representation of the set its own before the set changes: the states of the undo stack share the
   * representations of the sets that do not change. If the representation is shared, it is replaced with a copy, or
   * with an empty set if the contents are about to be replaced anyway.
   */
  void detach(bool a_copy = true) {
    if (m_rep.use_count() > 1) replace_rep(a_copy ? m_rep->clone() : new_rep(type()));
  }

  /*! clears the set.
   */
  void clear() {
    detach(false);
    m_rep->clear();
  }

  /*! complements the set.
   */
  void complement() {
    detach();
    m_rep->complement();
  }

  /*! assigns another set (of the same family) to the set.
   */
  void assign(const Curve_set& a_other) {
    if (! same_type(a_other) || (&a_other == this)) return;
    detach(false);
    m_rep->assign(*a_other.m_rep);
  }

  /*! intersects the set with another set (of the same family).
   */
  void intersect(const Curve_set& a_other) {
    if (! same_type(a_other)) return;
    detach();
    m_rep->intersect(*a_other.m_rep);
  }

  /*! joins the set with another set (of the same family).
   */
  void join(const Curve_set& a_other) {
    if (! same_type(a_other)) return;
    detach();
    m_rep->join(*a_other.m_rep);
  }

  /*! subtracts another set (of the same family) from the set.
   */
  void difference(const Curve_set& a_other) {
    if (! same_type(a_other)) return;
    detach();
    m_rep->difference(*a_other.m_rep);
  }

  /*! replaces the set with its symmetric difference with another set (of the same family).
   */
  void symmetric_difference(const Curve_set& a_other) {
    if (! same_type(a_other)) return;
    detach();
    m_rep->symmetric_difference(*a_other.m_rep);
  }

  /*! replaces the set with the complement of another set.
   */
  void complement(const Curve_set& a_other) {
    if (! same_type(a_other)) return;
    detach(&a_other == this);
    m_rep->complement(*a_other.m_rep);
  }

  /*! replaces the set with the intersection of two sets (of the same family).
   */
  void intersect(const Curve_set& a_first, const Curve_set& a_second) {
    if (! same_type(a_first) || ! same_type(a_second)) return;
    detach((&a_first == this) || (&a_second == this));
    m_rep->intersect(*a_first.m_rep, *a_second.m_rep);
  }

  /*! replaces the set with the union of two sets (of the same family).
   */
  void join(const Curve_set& a_first, const Curve_set& a_second) {
    if (! same_type(a_first) || ! same_type(a_second)) return;
    detach((&a_first == this) || (&a_second == this));
    m_rep->join(*a_first.m_rep, *a_second.m_rep);
  }

  /*! replaces the set with the difference of two sets (of the same family).
   */
  void difference(const Curve_set& a_first, const Curve_set& a_second) {
    if (! same_type(a_first) || ! same_type(a_second)) return;
    detach((&a_first == this) || (&a_second == this));
    m_rep->difference(*a_first.m_rep, *a_second.m_rep);
  }

  /*! replaces the set with the symmetric difference of two sets (of the same family).
   */
  void symmetric_difference(const Curve_set& a_first, const Curve_set& a_second) {
    if (! same_type(a_first) || ! same_type(a_second)) return;
    detach((&a_first == this) || (&a_second == this));
    m_rep->symmetric_difference(*a_first.m_rep, *a_second.m_rep);
  }

  /*! determines whether another set is of the same curve family.
   */
  bool same_type(const Curve_set& a_other) const
  { return m_rep && a_other.m_rep && (m_rep->type() == a_other.m_rep->type()); }

  /*! sets the brush.
   */
  void change_brush_color(QBrush a_brush) { m_brush = a_brush; }

  /*! sets the pen.
   */
  void change_pen_color(QPen a_pen) { m_pen = a_pen; }

  /*! obtains the representation of the set.
   */
  const Rep_base& rep() const { return *m_rep; }

  /*! obtains the representation of the set (to change it).
   */
  Rep_base& rep() {
    detach();
    return *m_rep;
  }

  /*! obtains the curve family of the set.
   */
  Curve_type type() const { return m_rep->type(); }

  /*! determines whether the set is linear.
   */
  bool is_linear() const { return type() == Curve_type::CT_LINEAR; }

  /*! determines whether the set is circular.
   */
  bool is_circular() const { return type() == Curve_type::CT_CIRCULAR; }

  /*! determines whether the set is Bezier.
   */
  bool is_bezier() const { return type() == Curve_type::CT_BEZIER; }

  /*! determines whether the set is a polyline set.
   */
  bool is_polyline() const { return type() == Curve_type::CT_POLYLINE; }

  /*! determines whether the set is conic.
   */
  bool is_conic() const { return type() == Curve_type::CT_CONIC; }

  /*! obtains the conic representation, or null if the set is not conic.
   */
  const Conic_rep* get_conic_rep() const { return dynamic_cast<const Conic_rep*>(m_rep.get()); }

  /*! obtains the conic representation, or null if the set is not conic.
   */
  Conic_rep* get_conic_rep() {
    detach();
    return dynamic_cast<Conic_rep*>(m_rep.get());
  }

  /*! obtains the conic set.
   */
  const Conic_polygon_set& conic() const { return get_conic_rep()->set(); }

  /*! obtains the conic set.
   */
  Conic_polygon_set& conic() { return get_conic_rep()->set(); }

  /*! obtains the algebraic representation, or null if the set is not algebraic.
   */
  const Algebraic_rep* get_algebraic_rep() const { return dynamic_cast<const Algebraic_rep*>(m_rep.get()); }

  /*! obtains the algebraic representation, or null if the set is not algebraic.
   */
  Algebraic_rep* get_algebraic_rep() {
    detach();
    return dynamic_cast<Algebraic_rep*>(m_rep.get());
  }

  /*! obtains the algebraic set.
   */
  const Algebraic_polygon_set& algebraic() const { return get_algebraic_rep()->set(); }

  /*! obtains the algebraic set.
   */
  Algebraic_polygon_set& algebraic() { return get_algebraic_rep()->set(); }

  /*! obtains the linear representation, or null if the set is not linear.
   */
  const Linear_rep* get_linear_rep() const { return dynamic_cast<const Linear_rep*>(m_rep.get()); }

  /*! obtains the linear representation, or null if the set is not linear.
   */
  Linear_rep* get_linear_rep() {
    detach();
    return dynamic_cast<Linear_rep*>(m_rep.get());
  }

  /*! obtains the linear set.
   */
  const Linear_polygon_set& linear() const { return get_linear_rep()->set(); }

  /*! obtains the linear set.
   */
  Linear_polygon_set& linear() { return get_linear_rep()->set(); }

  /*! obtains the circular representation, or null if the set is not circular.
   */
  const Circular_rep* get_circular_rep() const { return dynamic_cast<const Circular_rep*>(m_rep.get()); }

  /*! obtains the circular representation, or null if the set is not circular.
   */
  Circular_rep* get_circular_rep() {
    detach();
    return dynamic_cast<Circular_rep*>(m_rep.get());
  }

  /*! obtains the circular set.
   */
  const Circular_polygon_set& circular() const { return get_circular_rep()->set(); }

  /*! obtains the circular set.
   */
  Circular_polygon_set& circular() { return get_circular_rep()->set(); }

  /*! obtains the Bezier representation, or null if the set is not Bezier.
   */
  const Bezier_rep* get_bezier_rep() const { return dynamic_cast<const Bezier_rep*>(m_rep.get()); }

  /*! obtains the Bezier representation, or null if the set is not Bezier.
   */
  Bezier_rep* get_bezier_rep() {
    detach();
    return dynamic_cast<Bezier_rep*>(m_rep.get());
  }

  /*! obtains the Bezier set.
   */
  const Bezier_polygon_set& bezier() const { return get_bezier_rep()->set(); }

  /*! obtains the Bezier set.
   */
  Bezier_polygon_set& bezier() { return get_bezier_rep()->set(); }

  /*! obtains the polyline representation, or null if the set is not a polyline set.
   */
  const Polyline_rep* get_polyline_rep() const { return dynamic_cast<const Polyline_rep*>(m_rep.get()); }

  /*! obtains the polyline representation, or null if the set is not a polyline set.
   */
  Polyline_rep* get_polyline_rep() {
    detach();
    return dynamic_cast<Polyline_rep*>(m_rep.get());
  }

  /*! obtains the polyline set.
   */
  const Polyline_polygon_set& polyline() const { return get_polyline_rep()->set(); }

  /*! obtains the polyline set.
   */
  Polyline_polygon_set& polyline() { return get_polyline_rep()->set(); }

  /*! obtains the set of a given type, e.g., Linear_polygon_set.
   */
  template <typename Set>
  Set& get();

public:
  QPen m_pen;
  QBrush m_brush;
  Rep_ptr m_rep;
};

template <> inline Linear_polygon_set& Curve_set::get<Linear_polygon_set>() { return linear(); }
template <> inline Circular_polygon_set& Curve_set::get<Circular_polygon_set>() { return circular(); }
template <> inline Bezier_polygon_set& Curve_set::get<Bezier_polygon_set>() { return bezier(); }
template <> inline Polyline_polygon_set& Curve_set::get<Polyline_polygon_set>() { return polyline(); }
template <> inline Conic_polygon_set& Curve_set::get<Conic_polygon_set>() { return conic(); }
template <> inline Algebraic_polygon_set& Curve_set::get<Algebraic_polygon_set>() { return algebraic(); }

using Curve_set_container = std::vector<Curve_set>;
using Curve_set_const_iterator = Curve_set_container::const_iterator;

#endif
