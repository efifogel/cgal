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

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <limits>
#include <optional>
#include <functional>
#include <type_traits>
#include <vector>


#include <QApplication>
#include <qmessagebox.h>

#include <QMainWindow>
#include <QPointer>
#include <QGraphicsScene>
#include <QToolButton>
#include <QActionGroup>
#include <QPainterPath>
#include <QToolBar>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <cmath>
#include <QSize>
#include <QPen>
#include <QtGui>
#include <QString>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QFontDatabase>
#include <QSaveFile>
#include <charconv>
#include <QLineEdit>
#include <QRegularExpression>
#include <algorithm>
#include <map>
#include <memory>
#include <QInputDialog>
#include <QWheelEvent>
#include <QPainter>
#include <QSlider>
#include <QGraphicsLineItem>
#include <QProgressBar>
#include <QMessageBox>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QTimer>
#include <QDockWidget>
#include <array>
#include <algorithm>
#include <QButtonGroup>
#include <QPushButton>
#include <QDialog>
#include <QBoxLayout>
#include <QStyle>
#include <QProxyStyle>
#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QWindow>
#include <QGuiApplication>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/auto_link/Qt.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Point_2.h>
#include <CGAL/number_utils.h>
#include <CGAL/Cartesian_converter.h>
#include <CGAL/Timer.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/iterator.h>
#include <CGAL/assertions_behaviour.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/General_polygon_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Arr_Bezier_curve_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_circle_segment_traits_2.h>
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Iso_rectangle_2.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/minkowski_sum_2.h>
#include <CGAL/Small_side_angle_bisector_decomposition_2.h>
#include <CGAL/approximated_offset_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/minkowski_sum_2.h>
#include <CGAL/squared_distance_2.h>

#ifdef CGAL_USE_GMP
#include <CGAL/Gmpq.h>
#else
#include <CGAL/MP_Float.h>
#include <CGAL/Quotient.h>
#endif

#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Qt/Converter.h>
#include <CGAL/Qt/DemosMainWindow.h>
#include <CGAL/Qt/utility.h>
#include <CGAL/IO/Dxf_bsop_reader.h>
#include <CGAL/Qt/GraphicsViewNavigation.h>

#include "Qt6/MinkowskiSum.h"
#include "Qt6/Circular_polygons.h"
#include "Qt6/Linear_polygons.h"
#include "Qt6/BezierCurves.h"
#include "Qt6/Polyline_curves.h"
#include "Qt6/Graphics_view_circular_polygon_input.h"
#include "Qt6/Graphics_view_linear_polygon_input.h"
#include "Qt6/Graphics_view_polyline_input.h"
#include "Qt6/GraphicsViewBezierPolygonInput.h"
#include "Qt6/Conic_Curves.h"
#include "Qt6/Graphics_view_conic_input.h"

#include "ui_Boolean_set_operations_2.h"

#include "Typedefs.h"

/*! shows a warning; the program proceeds.
 */
void show_warning(std::string aS);

/*! shows an error; the operation does not proceed.
 */
void show_error(std::string aS);

/*! shows an error and throws a std::runtime_error.
 */
void error(std::string aS);

/*! handles a failed CGAL assertion or precondition by showing an error and throwing a std::runtime_error, except for a
 * few known harmless failures.
 */
void error_handler(const char* what, const char* expr, const char* file, int line, const char* msg);

// Typedefs
using Circular_GI = CGAL::Qt::Circular_set_graphics_item<Circular_polygon_set, Circular_traits>;
using Linear_GI = CGAL::Qt::Linear_set_graphics_item<Linear_polygon_set, Linear_traits>;
using Polyline_GI = CGAL::Qt::Polyline_set_graphics_item<Polyline_polygon_set, Polyline_traits>;

// Functions to show errors

void show_warning(std::string aS) { QMessageBox::warning(NULL, "Warning", QString(aS.c_str())); }

void show_error(std::string aS) { QMessageBox::critical(NULL, "Critical Error", QString(aS.c_str())); }

void error(std::string aS) {
  show_error(aS);
  throw std::runtime_error(aS);
}

void error_handler(const char* what, const char* expr, const char* file, int line, const char* msg) {
  if (expr != "first != last" && expr != "_inc_to_right != cv._inc_to_right" && line != 1684 &&
      expr != "is_new == true") {
    std::ostringstream ss;

    ss << "CGAL error: " << what << " violation!" << std::endl
       << "Expr: " << expr << std::endl
       << "File: " << file << std::endl
       << "Line: " << line << std::endl;
    if (msg != 0 || msg != "" || msg != NULL) {
      ss << "Explanation:" << msg << std::endl;
    }
    if (expr == "is_simple_2(first, last, traits)") {
      ss << "\nCompleting Polygon from this point!!!" << std::endl;
    }
    if (msg == "The outer boundary self intersects at edges.") {
      ss << "\nDeleting Polygon causing Error!!!" << std::endl;
    }

    error(ss.str());
  }
}

// The buckets are identified by their indices: red, black, blue, brown, yellow, magenta, and aqua. The scratch result
// set follows them. Initially (and at least) the first three buckets exist.
constexpr std::size_t number_of_buckets = 7;
constexpr std::size_t result_index = number_of_buckets;
constexpr std::size_t min_number_of_buckets = 3;

// The curve families are identified by numbers: linear 1, circular 2, Bezier 3, polyline 4, and conic 5.

// The pens and the brushes of the buckets and of the scratch result set.
QPen sPens[] = {
  QPen(QColor(255, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //red
  QPen(QColor(0, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),     //black
  QPen(QColor(0, 0, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //blue
  QPen(QColor(210, 105, 30), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),//brown
  QPen(QColor(255, 255, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //yellow
  QPen(QColor(255, 0, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //magenta
  QPen(QColor(0, 255, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //aqua
  QPen(QColor(0, 0, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //Result
};

QBrush sBrushes[] = {
  QBrush(QColor(255, 0, 0, 75)),           //red
  QBrush(QColor(0, 0, 0, 75)),             //black
  QBrush(QColor(0, 0, 255, 75)),           //blue
  QBrush(QColor(210, 105, 30, 75)),        //brown
  QBrush(QColor(255, 255, 0, 75)),         //yellow
  QBrush(QColor(255, 0, 255, 75)),         //magenta
  QBrush(QColor(0, 255, 255, 75)),         //aqua
  QBrush(QColor(0, 0, 0, 0)),              //Result
};

// The curve families.
enum class Curve_type { Linear, Circular, Bezier, Polyline, Conic };
constexpr std::size_t number_of_curve_types = 5;

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
  virtual void assign(const Rep_base& aOther) = 0;

  /*! obtains the graphics item that draws the set.
   */
  virtual CGAL::Qt::GraphicsItem* gi() const = 0;

  /*! obtains the graphics item that draws the set.
   */
  virtual CGAL::Qt::GraphicsItem* gi() = 0;

  /*! sets the pen of the graphics item.
   */
  virtual void set_pen(const QPen& aPen) = 0;

  /*! sets the brush of the graphics item.
   */
  virtual void set_brush(const QBrush& aBrush) = 0;

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
  virtual void intersect(const Rep_base& aOther) = 0;

  /*! joins the set with another set.
   */
  virtual void join(const Rep_base& aOther) = 0;

  /*! subtracts another set from the set.
   */
  virtual void difference(const Rep_base& aOther) = 0;

  /*! replaces the set with its symmetric difference with another set.
   */
  virtual void symmetric_difference(const Rep_base& aOther) = 0;

  /*! replaces the set with the complement of another set.
   */
  virtual void complement(const Rep_base& aOther) = 0;

  /*! replaces the set with the intersection of two sets.
   */
  virtual void intersect(const Rep_base& aFirst, const Rep_base& aSecond) = 0;

  /*! replaces the set with the union of two sets.
   */
  virtual void join(const Rep_base& aFirst, const Rep_base& aSecond) = 0;

  /*! replaces the set with the difference of two sets.
   */
  virtual void difference(const Rep_base& aFirst, const Rep_base& aSecond) = 0;

  /*! replaces the set with the symmetric difference of two sets.
   */
  virtual void symmetric_difference(const Rep_base& aFirst, const Rep_base& aSecond) = 0;
};

// The representation of a set of a given curve family. The graphics item that draws the set is constructed with traits
// if they are given.
template <typename GI_, typename Set_, Curve_type type_, typename Gps_traits = void>
class Rep : public Rep_base {
public:
  using GI = GI_;
  using Set = Set_;
  using Self = Rep<GI, Set, type_, Gps_traits>;

  /*! constructs an empty set and its graphics item.
   */
  Rep() { make_gi(); }

  /*! constructs a copy of a set and its graphics item.
   */
  explicit Rep(const Set& aSet) : m_set(aSet) { make_gi(); }

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
  void set_pen(const QPen& aPen) override { m_GI->set_pen(aPen); }

  /*! sets the brush of the graphics item.
   */
  void set_brush(const QBrush& aBrush) override { m_GI->set_brush(aBrush); }

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
  void assign(const Rep_base& aOther) override { guarded("assign", [&] { m_set = cast(aOther).m_set; }); }

  /*! intersects the set with another set.
   */
  void intersect(const Rep_base& aOther) override
  { guarded("intersect", [&] { m_set.intersection(cast(aOther).m_set); }); }

  /*! joins the set with another set.
   */
  void join(const Rep_base& aOther) override { guarded("union", [&] { m_set.join(cast(aOther).m_set); }); }

  /*! subtracts another set from the set.
   */
  void difference(const Rep_base& aOther) override
  { guarded("difference", [&] { m_set.difference(cast(aOther).m_set); }); }

  /*! replaces the set with its symmetric difference with another set.
   */
  void symmetric_difference(const Rep_base& aOther) override
  { guarded("symmetric difference", [&] { m_set.symmetric_difference(cast(aOther).m_set); }); }

  /*! replaces the set with the complement of another set.
   */
  void complement(const Rep_base& aOther) override
  { guarded("complement", [&] { m_set.complement(cast(aOther).m_set); }); }

  /*! replaces the set with the intersection of two sets.
   */
  void intersect(const Rep_base& aFirst, const Rep_base& aSecond) override
  { guarded("intersect", [&] { m_set.intersection(cast(aFirst).m_set, cast(aSecond).m_set); }); }

  /*! replaces the set with the union of two sets.
   */
  void join(const Rep_base& aFirst, const Rep_base& aSecond) override
  { guarded("union", [&] { m_set.join(cast(aFirst).m_set, cast(aSecond).m_set); }); }

  /*! replaces the set with the difference of two sets.
   */
  void difference(const Rep_base& aFirst, const Rep_base& aSecond) override
  { guarded("difference", [&] { m_set.difference(cast(aFirst).m_set, cast(aSecond).m_set); }); }

  /*! replaces the set with the symmetric difference of two sets.
   */
  void symmetric_difference(const Rep_base& aFirst, const Rep_base& aSecond) override
  { guarded("symmetric difference", [&] { m_set.symmetric_difference(cast(aFirst).m_set, cast(aSecond).m_set); }); }

  /*! casts a set representation to this type.
   */
  static const Self& cast(const Rep_base& aOther) { return dynamic_cast<const Self&>(aOther); }

  /*! casts a set representation to this type.
   */
  static Self& cast(Rep_base& aOther) { return dynamic_cast<Self&>(aOther); }

private:
  /*! constructs the graphics item that draws the set.
   */
  void make_gi() {
    if constexpr (std::is_void_v<Gps_traits>) m_GI = new GI(&m_set);
    else m_GI = new GI(&m_set, m_traits);
  }

  /*! applies an operation to the set; shows an error if it throws.
   */
  template <typename Operation>
  static void guarded(const char* aName, Operation aOperation) {
    try {
      aOperation();
    }
    catch (...) {
      show_error(std::string("Exception thrown during boolean operation ") + aName);
    }
  }

  struct No_traits {};

  GI* m_GI;
  Set m_set;
  std::conditional_t<std::is_void_v<Gps_traits>, No_traits, Gps_traits> m_traits;
};

using Linear_rep = Rep<Linear_GI, Linear_polygon_set, Curve_type::Linear, Linear_traits>;
using Circular_rep = Rep<Circular_GI, Circular_polygon_set, Curve_type::Circular, Circular_traits>;
using Bezier_rep = Rep<Bezier_GI, Bezier_polygon_set, Curve_type::Bezier>;
using Polyline_rep = Rep<Polyline_GI, Polyline_polygon_set, Curve_type::Polyline, Polyline_traits>;
using Conic_rep = Rep<CGAL::Qt::Conic_GI, Conic_polygon_set, Curve_type::Conic>;

// A set of polygons of some curve family, drawn with a given pen and brush.
class Curve_set {
  using Rep_ptr = std::shared_ptr<Rep_base>;

public:
  /*! constructs an empty set of a given curve family, drawn with a given pen and brush.
   */
  Curve_set(Curve_type aType, QPen aPen, QBrush aBrush) : m_pen(aPen), m_brush(aBrush) { reset_type(aType); }

  /*! replaces the set with an empty set of a given curve family.
   */
  void reset_type(Curve_type aType) {
    m_rep = Rep_ptr(new_rep(aType));
    m_rep->set_pen(m_pen);
    m_rep->set_brush(m_brush);
  }

  /*! constructs an empty set of a given curve family.
   */
  static Rep_base* new_rep(Curve_type aType) {
    switch (aType) {
     case Curve_type::Linear: return new Linear_rep();
     case Curve_type::Circular: return new Circular_rep();
     case Curve_type::Bezier: return new Bezier_rep();
     case Curve_type::Polyline: return new Polyline_rep();
     case Curve_type::Conic: return new Conic_rep();
    }
    return nullptr;
  }

  /*! replaces the representation of the set with a given one, keeping the pen, the brush, and the visibility.
   */
  void replace_rep(Rep_base* aRep) {
    bool visible = m_rep->gi()->isVisible();
    m_rep = Rep_ptr(aRep);
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
  void detach(bool aCopy = true) {
    if (m_rep.use_count() > 1) replace_rep(aCopy ? m_rep->clone() : new_rep(type()));
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
  void assign(const Curve_set& aOther) {
    if (! same_type(aOther) || (&aOther == this)) return;
    detach(false);
    m_rep->assign(*aOther.m_rep);
  }

  /*! intersects the set with another set (of the same family).
   */
  void intersect(const Curve_set& aOther) {
    if (! same_type(aOther)) return;
    detach();
    m_rep->intersect(*aOther.m_rep);
  }

  /*! joins the set with another set (of the same family).
   */
  void join(const Curve_set& aOther) {
    if (! same_type(aOther)) return;
    detach();
    m_rep->join(*aOther.m_rep);
  }

  /*! subtracts another set (of the same family) from the set.
   */
  void difference(const Curve_set& aOther) {
    if (! same_type(aOther)) return;
    detach();
    m_rep->difference(*aOther.m_rep);
  }

  /*! replaces the set with its symmetric difference with another set (of the same family).
   */
  void symmetric_difference(const Curve_set& aOther) {
    if (! same_type(aOther)) return;
    detach();
    m_rep->symmetric_difference(*aOther.m_rep);
  }

  /*! replaces the set with the complement of another set.
   */
  void complement(const Curve_set& aOther) {
    if (! same_type(aOther)) return;
    detach(&aOther == this);
    m_rep->complement(*aOther.m_rep);
  }

  /*! replaces the set with the intersection of two sets (of the same family).
   */
  void intersect(const Curve_set& aFirst, const Curve_set& aSecond) {
    if (! same_type(aFirst) || ! same_type(aSecond)) return;
    detach((&aFirst == this) || (&aSecond == this));
    m_rep->intersect(*aFirst.m_rep, *aSecond.m_rep);
  }

  /*! replaces the set with the union of two sets (of the same family).
   */
  void join(const Curve_set& aFirst, const Curve_set& aSecond) {
    if (! same_type(aFirst) || ! same_type(aSecond)) return;
    detach((&aFirst == this) || (&aSecond == this));
    m_rep->join(*aFirst.m_rep, *aSecond.m_rep);
  }

  /*! replaces the set with the difference of two sets (of the same family).
   */
  void difference(const Curve_set& aFirst, const Curve_set& aSecond) {
    if (! same_type(aFirst) || ! same_type(aSecond)) return;
    detach((&aFirst == this) || (&aSecond == this));
    m_rep->difference(*aFirst.m_rep, *aSecond.m_rep);
  }

  /*! replaces the set with the symmetric difference of two sets (of the same family).
   */
  void symmetric_difference(const Curve_set& aFirst, const Curve_set& aSecond) {
    if (! same_type(aFirst) || ! same_type(aSecond)) return;
    detach((&aFirst == this) || (&aSecond == this));
    m_rep->symmetric_difference(*aFirst.m_rep, *aSecond.m_rep);
  }

  /*! determines whether another set is of the same curve family.
   */
  bool same_type(const Curve_set& aOther) const
  { return m_rep && aOther.m_rep && (m_rep->type() == aOther.m_rep->type()); }

  /*! sets the brush.
   */
  void change_brush_color(QBrush aBrush) { m_brush = aBrush; }

  /*! sets the pen.
   */
  void change_pen_color(QPen aPen) { m_pen = aPen; }

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
  bool is_linear() const { return type() == Curve_type::Linear; }

  /*! determines whether the set is circular.
   */
  bool is_circular() const { return type() == Curve_type::Circular; }

  /*! determines whether the set is Bezier.
   */
  bool is_bezier() const { return type() == Curve_type::Bezier; }

  /*! determines whether the set is a polyline set.
   */
  bool is_polyline() const { return type() == Curve_type::Polyline; }

  /*! determines whether the set is conic.
   */
  bool is_conic() const { return type() == Curve_type::Conic; }

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

template <> Linear_polygon_set& Curve_set::get<Linear_polygon_set>() { return linear(); }
template <> Circular_polygon_set& Curve_set::get<Circular_polygon_set>() { return circular(); }
template <> Bezier_polygon_set& Curve_set::get<Bezier_polygon_set>() { return bezier(); }
template <> Polyline_polygon_set& Curve_set::get<Polyline_polygon_set>() { return polyline(); }
template <> Conic_polygon_set& Curve_set::get<Conic_polygon_set>() { return conic(); }

using Curve_set_container = std::vector<Curve_set>;
using Curve_set_const_iterator = Curve_set_container::const_iterator;

// The operations recorded on the undo stack.
enum class State_op { Complement, Intersection, Union, Difference, Symmetric_difference, Minkowski_sum, Reset, Copy,
                      Move, Clear, Delete_all, Draw, Start, Console };

// This container is for returning different color polygons while performing an operation
class State_current {
public:
  /*! constructs the state after a given operation, with empty sets of a given curve family.
   */
  State_current(State_op aOperation, Curve_type aType);

  /*! obtains the set of a given bucket (or the scratch result set, at result_index).
   */
  Curve_set& set(std::size_t aIndex) { return m_curve_sets[aIndex]; }

  /*! obtains the scratch result set.
   */
  Curve_set& result_set() { return set(result_index); }

  /*! shows or hides a bucket.
   */
  void toggle_view(std::size_t aIndex, bool a_check) {
    if (a_check) set(aIndex).gi()->show();
    else set(aIndex).gi()->hide();
  }

  /*! clears a given bucket (or the result).
   */
  void clear(std::size_t aIndex) { set(aIndex).clear(); }

public:
  Curve_set_container m_curve_sets;

  State_op m_operation;
  Curve_type m_type;
};

State_current::State_current(State_op aOperation, Curve_type aType) : m_operation(aOperation), m_type(aType) {
  for (std::size_t i = 0; i <= number_of_buckets; ++i) m_curve_sets.push_back(Curve_set(m_type, sPens[i], sBrushes[i]));
}

using state_container = std::vector<State_current>;
using state_const_iterator = state_container::const_iterator;
using state_iterator = state_container::iterator;

// Lets the user move a floating dock widget by dragging its title bar under Wayland. There, QDockWidget's own drag
// fails, because it grabs the mouse, which Wayland allows only for popup windows ("This plugin supports grabbing the
// mouse only for popup windows"). Instead, ask the compositor to move the window.
class Floating_dock_mover : public QObject {
public:
  /*! constructs from the parent object.
   */
  explicit Floating_dock_mover(QObject* parent) : QObject(parent) {}

protected:
  /*! moves a floating dock widget when its title bar is pressed.
   */
  bool eventFilter(QObject* obj, QEvent* event) override {
    if (event->type() != QEvent::MouseButtonPress) return false;
    auto* dock = qobject_cast<QDockWidget*>(obj);
    if (! dock || ! dock->isFloating()) return false;
    auto* me = static_cast<QMouseEvent*>(event);
    if (me->button() != Qt::LeftButton) return false;
    // Presses on the title-bar buttons go to the buttons themselves; a press outside the content widget is on the title
    // bar.
    QWidget* content = dock->widget();
    if (content && content->geometry().contains(me->position().toPoint()))
      return false;
    QWindow* window = dock->windowHandle();
    return window && window->startSystemMove();
  }
};

// On dark desktop themes the outlines of check box squares and radio button circles are hardly visible. Draw the
// indicator as usual (keeping the check mark or dot), then draw its outline on top: a gray square with slightly rounded
// corners for a check box, and a gray ring for a radio button. The outline turns white while the cursor hovers over the
// button.
class Visible_indicator_style : public QProxyStyle {
public:
  /*! enables hovering over check boxes and radio buttons.
   */
  void polish(QWidget* widget) override {
    QProxyStyle::polish(widget);
    // Track hovering, so that State_MouseOver is set.
    if (qobject_cast<QCheckBox*>(widget) || qobject_cast<QRadioButton*>(widget))
      widget->setAttribute(Qt::WA_Hover);
  }

  /*! draws a primitive element; the indicators of check boxes and radio buttons get a visible outline.
   */
  void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                     const QWidget* widget = nullptr) const override {
    QProxyStyle::drawPrimitive(element, option, painter, widget);
    if ((element != PE_IndicatorCheckBox) && (element != PE_IndicatorRadioButton))
      return;
    bool hover = option->state.testFlag(State_MouseOver) && option->state.testFlag(State_Enabled);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(hover ? QColor(Qt::white) : QColor(0x80, 0x80, 0x80), 1));
    QRectF rect = QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5);
    if (element == PE_IndicatorCheckBox)
      painter->drawRoundedRect(rect, 1.5, 1.5);
    else painter->drawEllipse(rect);
    painter->restore();
  }
};

// A scene that can draw a grid in its background, behind all items. Grid lines that would be denser than a few pixels
// on screen are thinned out (by factors of 10), so that zooming out does not fill the view with lines.
class Grid_scene : public QGraphicsScene {
public:
  /*! determines whether the grid is shown.
   */
  bool grid_visible() const { return m_grid_visible; }

  /*! shows or hides the grid.
   */
  void set_grid_visible(bool aVisible) { m_grid_visible = aVisible; update(); }

  /*! obtains the grid spacing.
   */
  double grid_spacing() const { return m_grid_spacing; }

  /*! sets the grid spacing.
   */
  void set_grid_spacing(double aSpacing) {
    m_grid_spacing = aSpacing;
    update();
  }

  /*! obtains the grid point nearest to a point.
   */
  QPointF snap(const QPointF& aP) const {
    return QPointF(std::round(aP.x() / m_grid_spacing) * m_grid_spacing,
                   std::round(aP.y() / m_grid_spacing) * m_grid_spacing);
  }

protected:
  /*! draws the background and, if it is shown, the grid.
   */
  void drawBackground(QPainter* painter, const QRectF& rect) override {
    QGraphicsScene::drawBackground(painter, rect);
    if (! m_grid_visible || (m_grid_spacing <= 0)) return;
    double scale = std::abs(painter->worldTransform().m11());
    if (scale <= 0) return;
    const double min_pixels = 6;
    double step = m_grid_spacing;
    while (step * scale < min_pixels) step *= 10;
    QVector<QLineF> lines;
    for (double x = std::ceil(rect.left() / step) * step; x <= rect.right(); x += step)
      lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (double y = std::ceil(rect.top() / step) * step; y <= rect.bottom(); y += step)
      lines.append(QLineF(rect.left(), y, rect.right(), y));
    QPen pen(QColor(0xdc, 0xdc, 0xdc), 1);
    pen.setCosmetic(true);
    painter->save();
    painter->setPen(pen);
    painter->drawLines(lines);
    painter->restore();
  }

private:
  bool m_grid_visible = false;
  double m_grid_spacing = 10;
};

// Adjusts mouse positions in a Grid_scene. With snapping enabled, a position
// is moved to the nearest grid point. Otherwise, it is rounded to a multiple
// of the largest power of 2 that does not exceed half a pixel in scene
// coordinates. A double carries 53 bits, while such a multiple has a small
// exact (dyadic) value, so the exact curves constructed from clicked points
// have much smaller coefficients, which makes the exact computations (e.g.,
// the Boolean operations on conics) much faster. The filter is installed on
// the application, so that it sees the scene's mouse events before the
// scene's own event filters (the polygon input tools, which read scenePos()).
class Grid_snapper : public QObject {
public:
  /*! constructs from the scene and the parent object.
   */
  Grid_snapper(Grid_scene* aScene, QObject* aParent) : QObject(aParent), m_scene(aScene) {}

  /*! determines whether snapping to the grid is enabled.
   */
  bool enabled() const { return m_enabled; }

  /*! enables or disables snapping to the grid.
   */
  void set_enabled(bool aEnabled) { m_enabled = aEnabled; }

protected:
  /*! snaps or rounds the position of a mouse event of the scene.
   */
  bool eventFilter(QObject* obj, QEvent* event) override {
    if (obj != m_scene) return false;
    switch (event->type()) {
     case QEvent::GraphicsSceneMousePress:
     case QEvent::GraphicsSceneMouseMove:
     case QEvent::GraphicsSceneMouseRelease:
     case QEvent::GraphicsSceneMouseDoubleClick: {
       auto* me = static_cast<QGraphicsSceneMouseEvent*>(event);
       me->setScenePos(m_enabled ? m_scene->snap(me->scenePos()) : round_to_pixel(me->scenePos(), me->widget()));
       break;
     }
     default: break;
    }
    return false;
  }

private:
  /*! rounds a point to a multiple of the largest power of 2 that does not exceed half a pixel of the view whose
   * viewport is given. Dividing and multiplying by a power of 2 is exact.
   */
  static QPointF round_to_pixel(const QPointF& aP, QWidget* aViewport) {
    auto* view = aViewport ? qobject_cast<QGraphicsView*>(aViewport->parentWidget()) : nullptr;
    if (! view) return aP;
    double scale = std::abs(view->transform().m11());
    if (! (scale > 0) || ! std::isfinite(scale)) return aP;
    double step = std::ldexp(1.0, int(std::floor(std::log2(0.5 / scale))));
    return QPointF(std::round(aP.x() / step) * step, std::round(aP.y() / step) * step);
  }

  Grid_scene* m_scene;
  bool m_enabled = false;
};

// The Boolean operations on buckets.
enum class Bucket_op { Complement, Intersection, Join, Difference, Symmetric_difference };

/*! obtains the name of a Boolean operation in the program (the method of Polygon_set_2).
 */
inline const char* method_name(Bucket_op aOp) {
  switch (aOp) {
   case Bucket_op::Complement: return "complement";
   case Bucket_op::Intersection: return "intersection";
   case Bucket_op::Join: return "join";
   case Bucket_op::Difference: return "difference";
   case Bucket_op::Symmetric_difference: return "symmetric_difference";
  }
  return "";
}

/*! replaces a set with the result of a Boolean operation (other than the complement) on two sets.
 */
inline void apply(Bucket_op aOp, Curve_set& rSet, const Curve_set& aFirst, const Curve_set& aSecond) {
  switch (aOp) {
   case Bucket_op::Intersection: rSet.intersect(aFirst, aSecond); break;
   case Bucket_op::Join: rSet.join(aFirst, aSecond); break;
   case Bucket_op::Difference: rSet.difference(aFirst, aSecond); break;
   case Bucket_op::Symmetric_difference: rSet.symmetric_difference(aFirst, aSecond); break;
   default: break;
  }
}

/*! applies a Boolean operation (other than the complement) to a set and another set, in place.
 */
inline void apply(Bucket_op aOp, Curve_set& rSet, const Curve_set& aOther) {
  switch (aOp) {
   case Bucket_op::Intersection: rSet.intersect(aOther); break;
   case Bucket_op::Join: rSet.join(aOther); break;
   case Bucket_op::Difference: rSet.difference(aOther); break;
   case Bucket_op::Symmetric_difference: rSet.symmetric_difference(aOther); break;
   default: break;
  }
}

/*! applies a Boolean operation (other than the complement) to a linear set and a polygon, in place.
 */
inline void apply(Bucket_op aOp, Linear_polygon_set& rSet, const Linear_polygon_with_holes& aPolygon) {
  switch (aOp) {
   case Bucket_op::Intersection: rSet.intersection(aPolygon); break;
   case Bucket_op::Join: rSet.join(aPolygon); break;
   case Bucket_op::Difference: rSet.difference(aPolygon); break;
   case Bucket_op::Symmetric_difference: rSet.symmetric_difference(aPolygon); break;
   default: break;
  }
}

/*! obtains the name of a bucket in the program (the color of its row), by its index.
 */
inline const char* bucket_name(std::size_t aIndex) {
  static const char* names[] = { "red", "black", "blue", "brown", "yellow", "magenta", "aqua" };
  return (aIndex < number_of_buckets) ? names[aIndex] : "result";
}

/*! obtains a string with its first letter capitalized.
 */
inline QString capitalized(QString aString) {
  if (! aString.isEmpty()) aString[0] = aString[0].toUpper();
  return aString;
}

/*! obtains the shortest decimal representation that reads back as the same double, so that the program reproduces the
 * exact coordinates.
 */
inline QString to_python(double aValue) {
  char buffer[64];
  auto res = std::to_chars(buffer, buffer + sizeof(buffer), aValue);
  return QString::fromLatin1(buffer, int(res.ptr - buffer));
}

/*! obtains a linear polygon as a Python expression. (Its curves are segments along the boundary, in order; their
 * sources are the vertices.)
 */
inline QString to_python(const Linear_polygon& aPolygon) {
  QStringList points;
  for (auto cit = aPolygon.curves_begin(); cit != aPolygon.curves_end(); ++cit)
    points << QString("Point_2(%1, %2)")
      .arg(to_python(CGAL::to_double(cit->source().x())), to_python(CGAL::to_double(cit->source().y())));
  return QString("Polygon_2([%1])").arg(points.join(", "));
}

// The console language: a small subset of Python, matching the CGALPY
// bindings (Polygon_set_2, Polygon_2, Point_2):
//   name = Polygon_set_2()            name = Polygon_set_2(other)
//   name = Polygon_set_2(Polygon_2([Point_2(x, y), ...]))
//   name.op(arg)   name.op(set1, set2)  (op: join, intersection, difference,
//                                         symmetric_difference)
//   name.complement()   name.complement(other)   name.clear()
//   name.is_empty()   name.is_plane()   name.number_of_polygons_with_holes()
//   del name
// Statements may be separated by ';'; '#' starts a comment.
struct Py_expr {
  enum class Kind { Name, Polygon, Polygon_set };
  Kind m_kind = Kind::Name;
  QString m_name;                                   // Name
  std::vector<std::pair<double, double>> m_points;  // Polygon
  std::shared_ptr<Py_expr> m_arg;                   // Polygon_set (optional)
};

struct Py_statement {
  enum class Kind { Assign, Call, Delete };
  Kind m_kind = Kind::Call;
  QString m_target;              // the assigned, called, or deleted name
  QString m_method;              // Call
  std::vector<Py_expr> m_args;   // Call arguments; Assign: the value
};

/*! obtains an expression as normalized Python text.
 */
inline QString to_python(const Py_expr& aExpr) {
  switch (aExpr.m_kind) {
   case Py_expr::Kind::Name: return aExpr.m_name;
   case Py_expr::Kind::Polygon: {
     QStringList points;
     for (const auto& [x, y] : aExpr.m_points)
       points << QString("Point_2(%1, %2)").arg(to_python(x), to_python(y));
     return QString("Polygon_2([%1])").arg(points.join(", "));
   }
   case Py_expr::Kind::Polygon_set:
    return QString("Polygon_set_2(%1)")
      .arg(aExpr.m_arg ? to_python(*aExpr.m_arg) : QString());
  }
  return QString();
}

/*! obtains a statement as normalized Python text.
 */
inline QString to_python(const Py_statement& aStatement) {
  switch (aStatement.m_kind) {
   case Py_statement::Kind::Assign:
    return aStatement.m_target + " = " + to_python(aStatement.m_args[0]);
   case Py_statement::Kind::Delete: return "del " + aStatement.m_target;
   case Py_statement::Kind::Call: {
     QStringList args;
     for (const auto& arg : aStatement.m_args) args << to_python(arg);
     return QString("%1.%2(%3)").arg(aStatement.m_target, aStatement.m_method, args.join(", "));
   }
  }
  return QString();
}

// A recursive-descent parser for the console language. Throws a std::runtime_error with a message on a syntax error.
class Py_parser {
public:
  /*! constructs from a console line, which is tokenized.
   */
  explicit Py_parser(const QString& aLine) { tokenize(aLine); }

  /*! parses the statements of the line.
   */
  std::vector<Py_statement> parse() {
    std::vector<Py_statement> statements;
    while (! at_end()) {
      if (accept(";")) continue;
      statements.push_back(statement());
      if (! at_end()) expect(";");
    }
    return statements;
  }

private:
  enum class Tok { Ident, Number, Punct, End };
  struct Token { Tok m_kind; QString m_text; };
  std::vector<Token> m_tokens;
  std::size_t m_pos = 0;

  /*! throws a std::runtime_error with a given message.
   */
  [[noreturn]] void error(const QString& aMessage) const { throw std::runtime_error(aMessage.toStdString()); }

  /*! splits a line into tokens.
   */
  void tokenize(const QString& aLine) {
    static const QRegularExpression number(R"(^(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?)");
    static const QRegularExpression ident(R"(^[A-Za-z_]\w*)");
    int i = 0;
    while (i < aLine.size()) {
      QChar c = aLine[i];
      if (c.isSpace()) { ++i; continue; }
      if (c == '#') break;
      QStringView rest = QStringView(aLine).mid(i);
      auto m = ident.matchView(rest);
      if (m.hasMatch()) {
        m_tokens.push_back({Tok::Ident, m.captured(0)});
        i += m.capturedLength(0);
        continue;
      }
      m = number.matchView(rest);
      if (m.hasMatch()) {
        m_tokens.push_back({Tok::Number, m.captured(0)});
        i += m.capturedLength(0);
        continue;
      }
      if (QString("()[],.=;-+").contains(c)) {
        m_tokens.push_back({Tok::Punct, QString(c)});
        ++i;
        continue;
      }
      error(QString("unexpected character '%1'").arg(c));
    }
    m_tokens.push_back({Tok::End, QString()});
  }

  /*! obtains the token a given number of tokens ahead, without consuming it.
   */
  const Token& peek(std::size_t aAhead = 0) const { return m_tokens[std::min(m_pos + aAhead, m_tokens.size() - 1)]; }

  /*! determines whether all tokens are consumed.
   */
  bool at_end() const { return peek().m_kind == Tok::End; }

  /*! consumes a given punctuation token if it is next.
   */
  bool accept(const QString& aPunct) {
    if ((peek().m_kind == Tok::Punct) && (peek().m_text == aPunct)) {
      ++m_pos;
      return true;
    }
    return false;
  }

  /*! consumes a given punctuation token, which must be next.
   */
  void expect(const QString& aPunct) { if (! accept(aPunct)) error(QString("expected '%1'%2").arg(aPunct, near())); }

  /*! obtains a description of the position of the next token, for error messages.
   */
  QString near() const { return at_end() ? QString(" at the end") : QString(" before '%1'").arg(peek().m_text); }

  /*! consumes a name.
   */
  QString identifier() {
    if (peek().m_kind != Tok::Ident) error("expected a name" + near());
    return m_tokens[m_pos++].m_text;
  }

  /*! consumes a (signed) number.
   */
  double number() {
    bool negative = false;
    if (accept("-")) negative = true;
    else accept("+");
    if (peek().m_kind != Tok::Number) error("expected a number" + near());
    double value = m_tokens[m_pos++].m_text.toDouble();
    return negative ? -value : value;
  }

  /*! parses a statement.
   */
  Py_statement statement() {
    Py_statement st;
    QString name = identifier();
    if (name == "del") {
      st.m_kind = Py_statement::Kind::Delete;
      st.m_target = identifier();
      return st;
    }
    st.m_target = name;
    if (accept("=")) {
      st.m_kind = Py_statement::Kind::Assign;
      st.m_args.push_back(expression());
      return st;
    }
    expect(".");
    st.m_kind = Py_statement::Kind::Call;
    st.m_method = identifier();
    expect("(");
    if (! accept(")")) {
      do st.m_args.push_back(expression()); while (accept(","));
      expect(")");
    }
    return st;
  }

  /*! parses an expression.
   */
  Py_expr expression() {
    Py_expr e;
    QString name = identifier();
    if (name == "Polygon_set_2") {
      e.m_kind = Py_expr::Kind::Polygon_set;
      expect("(");
      if (! accept(")")) {
        e.m_arg = std::make_shared<Py_expr>(expression());
        expect(")");
      }
      return e;
    }
    if (name == "Polygon_2") {
      e.m_kind = Py_expr::Kind::Polygon;
      expect("(");
      expect("[");
      while (! accept("]")) {
        if (identifier() != "Point_2") error("expected Point_2(x, y)");
        expect("(");
        double x = number();
        expect(",");
        double y = number();
        expect(")");
        e.m_points.emplace_back(x, y);
        if (! accept(",")) { expect("]"); break; }
      }
      expect(")");
      return e;
    }
    if ((name == "Point_2") || (peek().m_text == "("))
      error(QString("unexpected '%1'; expected a polygon set name, "
                    "Polygon_2([...]), or Polygon_set_2(...)").arg(name));
    e.m_kind = Py_expr::Kind::Name;
    e.m_name = name;
    return e;
  }
};

// The main window.
class MainWindow : public CGAL::Qt::DemosMainWindow, public Ui::Boolean_set_operations_2 {
  Q_OBJECT

private:
  Grid_scene m_scene;
  // The cells of the bucket rows of the Operations panel, by column, and the other widgets of the rows.
  using Column = std::array<QAbstractButton*, number_of_buckets>;
  Column m_show_column;                         // Visible
  Column m_draw_column;                         // Draw/Insert
  Column m_complement_column;
  Column m_intersection_column;
  Column m_union_column;
  Column m_difference_column;
  Column m_symmetric_difference_column;
  Column m_minkowski_sum_column;
  Column m_copy_column;
  Column m_move_column;
  Column m_clear_column;
  std::array<QLabel*, number_of_buckets> m_bucket_labels;
  std::array<QFrame*, number_of_buckets> m_bucket_separators;   // the lines below the rows (null for the initial rows)
  std::size_t m_number_of_buckets = min_number_of_buckets;     // the existing buckets are the first ones

  // The program: the statements executed so far, each tagged with the size of the undo stack after its action, so that
  // undo can drop them.
  struct Program_line {
    std::size_t m_state_count;
    QString m_text;
  };
  std::vector<Program_line> m_program;
  QPlainTextEdit* m_log = nullptr;
  bool m_logging = true;      // whether actions are logged

  // The console: its output, its input line and history, and the polygon
  // sets (variables) other than the buckets, e.g., result.
  QPlainTextEdit* m_console_output = nullptr;
  QLineEdit* m_console_input = nullptr;
  QStringList m_console_history;
  int m_console_history_pos = 0;
  std::map<QString, Curve_set> m_variables;
  Curve_type m_curve_type = Curve_type::Linear;     // the curve family of the sets

  // A curve family: its tool-bar action, the input that constructs its curves, and its name; and if its polygons can
  // be loaded, the action that does it, the extension of their files, and the reader.
  struct Curve_family {
    Curve_type m_type;
    const char* m_name;
    QAction* m_insert_action;
    CGAL::Qt::GraphicsViewInput* m_input;
    std::function<void()> m_reset_input;
    QAction* m_open_action;
    const char* m_extension;
    bool (MainWindow::*m_read)(QString);
  };
  std::array<Curve_family, number_of_curve_types> m_families;

  size_t m_color_active;           // the Draw/Insert bucket
  bool empty_warn;
  bool operand_error_warn;
  bool m_grid;
  bool m_pan;
  bool m_reset;
  bool minkowski_sum_operated;
  bool m_disjoint;

  size_t m_state_num;

  // The axes. (Declared after the scene, so that they are deleted first, removing themselves from the scene if shown.)
  std::unique_ptr<QGraphicsLineItem> xAxis = std::make_unique<QGraphicsLineItem>();
  std::unique_ptr<QGraphicsLineItem> yAxis = std::make_unique<QGraphicsLineItem>();

  state_container states_stack;
  // The graphics items in the scene (null once deleted along with their sets).
  std::array<QPointer<CGAL::Qt::GraphicsItem>, number_of_buckets + 1> m_linked_items;

  CGAL::Qt::Graphics_view_linear_polygon_input<Kernel>* m_linear_input;
  CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>* m_circular_input;
  CGAL::Qt::GraphicsViewBezierPolygonInput<Bezier_traits>* m_bezier_input;
  CGAL::Qt::Graphics_view_polyline_input<Kernel>* m_polyline_input;
  CGAL::Qt::Graphics_view_conic_input* m_conic_input;

public:
  /*! constructs the main window: the scene, the panels, the tool bars, and the inputs.
   */
  MainWindow();

private:
  /*! fits the view to the curves of all buckets.
   */
  void zoom_to_fit();

protected slots:
  /*! loads polygons from a file into the Draw/Insert bucket; the file type is determined by its extension.
   */
  void open(QString filename);

public slots:
  /*! adds a polygon constructed by one of the inputs to the Draw/Insert bucket.
   */
  void process_input(CGAL::Object o);

  /*! undoes the last action.
   */
  void undo();

  /*! clears all buckets and starts a new program.
   */
  void new_program();

  /*! shows or hides the axes.
   */
  void toggle_axes();

  /*! adds the complement of the marked bucket to the Draw/Insert bucket.
   */
  void apply_complement(bool aChecked);

  /*! adds the union of the marked buckets to the Draw/Insert bucket.
   */
  void apply_union(bool aChecked);

  /*! adds the intersection of the marked buckets to the Draw/Insert bucket.
   */
  void apply_intersection(bool aChecked);

  /*! adds the difference of the two marked buckets to the Draw/Insert bucket.
   */
  void apply_difference(bool aChecked);

  /*! adds the symmetric difference of the two marked buckets to the Draw/Insert bucket.
   */
  void apply_symmetric_difference(bool aChecked);

  /*! adds the Minkowski sum of the two marked (linear) buckets to the Draw/Insert bucket.
   */
  void apply_minkowski_sum(bool aChecked);






  /*! shows or hides the Operations panel.
   */
  void show_operations_panel(bool aChecked);

  /*! keeps the Operations menu item in sync with the visibility of its panel.
   */
  void operations_panel_visibility_changed(bool visible);

  /*! keeps the Log menu item in sync with the visibility of its panel.
   */
  void log_panel_visibility_changed(bool visible);

  /*! keeps the Console menu item in sync with the visibility of its panel.
   */
  void console_panel_visibility_changed(bool visible);

  /*! shows or hides the Console panel.
   */
  void show_console_panel(bool aChecked);

  /*! shows or hides the Log panel.
   */
  void show_log_panel(bool aChecked);

  /*! loads linear polygons from a file.
   */
  void open_linear();

  /*! loads polygons bounded by segments and circular arcs from a file.
   */
  void open_circular();

  /*! loads Bezier polygons from a file.
   */
  void open_bezier();

  /*! loads polygons bounded by polylines from a file.
   */
  void open_polyline();

  /*! zooms the view in or out.
   */
  void wheelEvent(QWheelEvent* event);

  /*! saves the Draw/Insert bucket to a file.
   */
  void save_draw_bucket();

  /*! adds a bucket (up to seven).
   */
  void add_bucket();

  /*! removes the last bucket (down to three).
   */
  void remove_bucket();

  /*! copies the marked bucket into the Draw/Insert bucket.
   */
  void apply_copy(bool aChecked);

  /*! moves the marked bucket into the Draw/Insert bucket.
   */
  void apply_move(bool aChecked);

  /*! shows a message with a "Don't show this message again" check box. aToggle is the Edit-menu action that controls
   * whether the message is shown; checking the box unchecks it. Unlike a QMessageBox, OK is always on the left and
   * Cancel (when offered) on the right, and neither is the default button. \return the button the user clicked (Escape
   * and the close box count as Cancel when it is offered, and as OK otherwise).
   */
  QMessageBox::StandardButton
  show_message(QMessageBox::Icon aIcon, const QString& aTitle, const QString& aText, QAction* aToggle,
               QMessageBox::StandardButtons aButtons = QMessageBox::Ok);

  /*! warns that the Draw/Insert bucket, where the result is added, is not empty. \return true if the user chooses to
   * continue.
   */
  bool show_not_empty_warning();

  /*! shows an error: the Draw/Insert bucket is a non-empty operand of the operation.
   */
  void show_operand_error();

  /*! determines whether the Draw/Insert bucket is marked in a given operation column.
   */
  bool draw_row_checked(const Column& aColumn) const;

  /*! obtains the buckets (indices) marked in a given operation column.
   */
  std::vector<std::size_t>
  checked_buckets(const Column& aColumn) const;

  /*! unchecks all operation buttons but a given one.
   */
  void uncheck_operations_except(QAbstractButton* aOperation);

  /*! clears the scratch result set.
   */
  void clear_result();

  /*! applies a Boolean operation to the operand buckets, and adds the result to the Draw/Insert bucket. The first two
   * operands are combined with the bivariate operation, and the remaining ones are folded in one at a time. If the
   * Draw/Insert bucket is empty and not an operand, the result is computed directly into it; otherwise into a
   * temporary, which is then added to it. The executed statements are logged.
   */
  void run_operation(Bucket_op aOp, const std::vector<std::size_t>& aOperands, QAbstractButton* aOperation);

  /*! appends statements to the program, and shows them in the Log. Sticky statements (the creation and deletion of
   * buckets, which are not undoable actions) are never dropped by undo.
   */
  void log_statements(const QStringList& aStatements, bool aSticky = false);

  /*! handles the keys of the console input line: Up and Down browse the history, and Ctrl+<key> combinations go to the
   * input line rather than to the operation shortcuts, except Ctrl+Q.
   */
  bool eventFilter(QObject* aObject, QEvent* aEvent) override;

  /*! sets the area that unbounded sets (e.g., complements) are drawn over to the visible area of the view, enlarged a
   * bit so that the edges of the fill never show.
   */
  void update_unbounded_rect();


  /*! obtains the input that constructs curves of the active family.
   */
  QObject* current_input() const;

  /*! stops constructing curves, and discards those under construction.
   */
  void remove_inputs();

  /*! constructs curves with a given input only.
   */
  void install_input(QObject* aInput);



  /*! sets the pen of the curves under construction.
   */
  void set_input_pen(const QPen& aPen);

  /*! obtains a curve family.
   */
  const Curve_family& family(Curve_type aType) const { return m_families[std::size_t(aType)]; }

  /*! switches to a curve family; if there are polygons of another family, asks the user first. \return true if the
   * family is active.
   */
  bool ensure_curve_type(Curve_type aType);

  /*! starts constructing curves of a given family.
   */
  void start_inserting(Curve_type aType);

  /*! obtains an icon of the curve tool bar: a boundary of a segment and an arc, or a circle with its center and radius.
   */
  QIcon curve_tool_icon(const QString& aKind);

  /*! determines whether the bucket with a given index currently exists (is shown).
   */
  bool bucket_exists(std::size_t aIndex) const;

  /*! obtains the index of the bucket with a given name, or number_of_buckets if there is none.
   */
  std::size_t bucket_index(const QString& aName) const;

  /*! obtains the polygon set (an existing bucket or variable) with a given name, or null.
   */
  Curve_set* polygon_set(const QString& aName);

  /*! obtains the polygon of a Polygon_2([...]) expression. It must be simple and counterclockwise oriented (as the
   * bindings require).
   */
  Linear_polygon_with_holes polygon(const Py_expr& aExpr) const;

  /*! executes a console statement; statements that change a polygon set are undoable actions, and are logged. \return
   * the text to print (for queries), or an empty string. Throws a std::runtime_error with a message on an error.
   */
  QString execute(const Py_statement& aStatement);

  /*! obtains the polygon set (an existing bucket or variable) with a given name. Throws a std::runtime_error if there
   * is none.
   */
  Curve_set& existing_set(const QString& aName);

  /*! obtains the polygon set named by an expression. Throws a std::runtime_error if the expression is not a name of a
   * polygon set.
   */
  Curve_set& named_set(const Py_expr& aExpr);

  /*! executes a console statement that deletes a polygon set: del name.
   */
  void execute_delete(const Py_statement& aStatement);

  /*! executes a console statement that assigns a polygon set: name = Polygon_set_2(...).
   */
  void execute_assign(const Py_statement& aStatement);

  /*! executes a console statement that calls a method of a polygon set: name.method(args). \return the text to print
   * (for queries), or an empty string.
   */
  QString execute_call(const Py_statement& aStatement);

  /*! parses and executes a console line.
   */
  void run_console_line(const QString& aLine);

  /*! prints to the console output.
   */
  void console_print(const QString& aText);

  /*! drops the statements of undone actions (those logged at a deeper undo stack than the current one), and refreshes
   * the Log.
   */
  void trim_program_to_states();

  /*! saves the program as a Python script for the CGALPY bindings.
   */
  void save_program();

  /*! keeps at least one bucket unmarked in the intersection or union column (for the result) when a cell is toggled:
   * checking the last unchecked cell is undone. The operation button is enabled only when at least one cell is checked.
   */
  void limit_to_all_but_one(const Column& aColumn, QAbstractButton* aBox, bool aChecked,
                            QAbstractButton* aOperation);

  /*! restores the invariant of limit_to_all_but_one() after a bucket is removed: if all (remaining) buckets are
   * checked, unchecks the last one.
   */
  void keep_one_unchecked(const Column& aColumn);

  /*! keeps at most two buckets marked in the difference, symmetric difference, or Minkowski-sum column when a cell is
   * toggled: checking a third cell is undone. The operation button is enabled only when exactly two cells are checked.
   */
  void limit_to_two_operands(const Column& aColumn, QAbstractButton* aBox, bool aChecked,
                             QAbstractButton* aOperation);

  /*! checks the Draw/Insert bucket, where every operation stores its result. If it is not empty and an operand, shows
   * an error; if it is not empty otherwise, warns and lets the user cancel. When aborting, unchecks the operation
   * button. \return true if the operation is aborted.
   */
  bool abort_for_draw_bucket(QAbstractButton* aOperation, bool aIsOperand);

  /*! shows or hides the scratch result set.
   */
  void show_result(bool a_check);

  /*! shows or hides all buckets.
   */
  void show_all_buckets(bool a_check);

  /*! marks or unmarks all buckets for clearing.
   */
  void mark_all_for_clearing(bool a_check);

  /*! shows or hides a bucket. When the Draw/Insert bucket is hidden, the first visible bucket becomes the Draw/Insert
   * bucket; when no bucket is visible, curves are not constructed.
   */
  void show_bucket(std::size_t aIndex, bool aChecked);

  /*! makes a bucket the Draw/Insert bucket.
   */
  void set_draw_bucket(std::size_t aIndex);

  /*! obtains the number of existing buckets that are visible.
   */
  std::size_t number_of_visible_buckets() const;

  /*! sets the number of existing buckets (the first ones), and shows their rows of the Operations panel.
   */
  void set_number_of_buckets(std::size_t aNumber);

  /*! restores the initial marks of the Operations panel.
   */
  void reset_marks();

  /*! obtains the first bucket marked in a given column, if any.
   */
  std::optional<std::size_t> checked_bucket(const Column& aColumn) const;

  /*! shows information about 2D regularized Boolean set-operations.
   */
  void about_boolean_set_operations();

  /*! shows information about CGAL.
   */
  void about_cgal();

  /*! shows information about 2D Minkowski sums.
   */
  void about_minkowski_sums();

  /*! shows information about the demo.
   */
  void about_demo();

  /*! fits the view to the scene after the window is resized.
   */
  void resizeEvent(QResizeEvent* event);


  /*! clears the scratch result set.
   */
  void delete_result();

  /*! clears all buckets.
   */
  void delete_all();

  /*! clears the marked buckets.
   */
  void apply_clear(bool aChecked);

  /*! starts or stops panning the view by dragging.
   */
  void toggle_pan();

  /*! obtains the (linear) polygon with holes of a given bucket, an operand of a Minkowski sum.
   */
  Polygon_with_holes_2 get_mink_input_polygon(std::size_t aIndex);

  /*! reads linear polygons from a file. \return false if the file cannot be read.
   */
  bool read_linear(QString aFileName);

  /*! reads polygons bounded by segments and circular arcs from a file. \return false if the file cannot be read.
   */
  bool read_circular(QString aFileName);

  /*! reads polygons bounded by polylines from a file. \return false if the file cannot be read.
   */
  bool read_polyline(QString aFileName);

  /*! reads Bezier polygons from a file into the Draw/Insert bucket. \return false if the file cannot be read.
   */
  bool read_bezier(QString aFileName);


signals:
  /*! signals that the model changed.
   */
  void changed();

private:
  /*! adds a polygon of a given curve family, constructed by its input, to the Draw/Insert bucket (as an undoable
   * action), or to the scratch result set while a bounding rectangle is constructed.
   */
  template <typename Set>
  void add_constructed_polygon(Curve_type aType, typename Set::Polygon_2 aPolygon, bool aToResult);

  /*! reads polygons with holes from a file into the Draw/Insert bucket. The file has a format line (DOUBLE for
   * coordinates given as doubles), the number of polygons with holes, and for each one the number of its boundaries
   * followed by the boundaries (the outer one first), each read by a given function. \return false if the file cannot
   * be read.
   */
  template <typename Set, typename Read_boundary>
  bool read_polygons(const QString& aFileName, const char* aKind, Read_boundary aReadBoundary);

  /*! pushes a new state onto the undo stack before an operation is applied. The new state shares the sets of the
   * current one; a set is copied when it first changes (see Curve_set::detach()). (Starting pushes empty sets.)
   */
  void get_new_state(State_op aOperation) {
    if (aOperation == State_op::Start) states_stack.push_back(State_current(aOperation, m_curve_type));
    else {
      State_current state = states_stack.back();
      state.m_operation = aOperation;
      state.m_type = m_curve_type;
      // The sets of another curve family (of a state before the family was switched) are replaced with empty sets.
      for (auto& set : state.m_curve_sets) {
        if (set.type() != m_curve_type) set.reset_type(m_curve_type);
      }
      states_stack.push_back(state);
    }
    m_state_num = states_stack.size();
    actionUndo->setEnabled(true);
    model_changed();
  }

  /*! shows the graphics items of the sets of the current state in the scene, instead of those of the sets they replaced
   * (of other states, or of other curve families).
   */
  void sync_graphics_items() {
    if (states_stack.empty()) return;
    auto& sets = states_stack.back().m_curve_sets;
    for (std::size_t i = 0; i < sets.size(); ++i) {
      CGAL::Qt::GraphicsItem* gi = sets[i].gi();
      if (gi == m_linked_items[i]) continue;
      if (m_linked_items[i]) unlink_gi(m_linked_items[i].data());
      gi->setZValue(double(i));       // the sets are drawn in the order of their indices
      link_gi(gi);
      m_linked_items[i] = gi;
    }
  }

  /*! shows the graphics items of the current state, and emits the changed() signal.
   */
  void model_changed() {
    sync_graphics_items();
    emit(changed());
  }

  /*! asks the user a yes/no question. \return true if the answer is yes.
   */
  bool ask_user_yesno(const QString& aTitle, const QString& aQuestion) {
    return QMessageBox::warning(this, aTitle, aQuestion, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ==
      QMessageBox::Yes;
  }

  /*! shows a warning.
   */
  void ask_user_ok(std::string aTitle, std::string aQuestion)
  { QMessageBox::warning(this, QString(aTitle.c_str()), QString(aQuestion.c_str())); }

  /*! adds a graphics item to the scene and connects it to the changed() signal.
   */
  void link_gi(CGAL::Qt::GraphicsItem* aGI) {
    QObject::connect(this, SIGNAL(changed()), aGI, SLOT(modelChanged()));
    m_scene.addItem(aGI);
  }

  /*! removes a graphics item from the scene and disconnects it from the changed() signal.
   */
  void unlink_gi(CGAL::Qt::GraphicsItem* aGI) {
    m_scene.removeItem(aGI);
    QObject::disconnect(this, SIGNAL(changed()), aGI, SLOT(modelChanged()));
  }

  /*! replaces a set with an empty set of a given curve family.
   */
  void switch_set_type(Curve_set& aSet, Curve_type aType);

  /*! replaces all sets with empty sets of a given curve family.
   */
  void switch_sets_type(Curve_type aType);

  /*! determines whether all sets (the buckets and the result) are empty.
   */
  bool are_all_sets_empty() {
    for (std::size_t i = 0; i <= number_of_buckets; ++i) {
      if (! states_stack.back().set(i).is_empty()) return false;
    }
    return true;
  }





};

MainWindow::MainWindow() :
  DemosMainWindow(),
  m_color_active(0),
  minkowski_sum_operated(false),
  empty_warn(true),
  operand_error_warn(true),
  m_disjoint(false),
  m_pan(false),
  m_grid(false),
  m_reset(false),
  m_state_num(0) {
  CGAL::set_error_handler(error_handler);
  CGAL::set_warning_handler(error_handler);
  CGAL::set_error_behaviour(CGAL::CONTINUE);

  setupUi(this);

  // Make the check boxes and radio buttons of the Operations panel visible on dark themes.
  auto* indicator_style = new Visible_indicator_style;
  indicator_style->setParent(this);
  for (auto* box : sceneDockWidget->findChildren<QCheckBox*>()) box->setStyle(indicator_style);
  for (auto* button : sceneDockWidget->findChildren<QRadioButton*>()) button->setStyle(indicator_style);

  // The Log panel: the program (the executed statements) and a button that saves it as a Python script.
  {
    auto* panel = new QWidget;
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(4, 4, 4, 4);
    m_log = new QPlainTextEdit;
    m_log->setReadOnly(true);
    m_log->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_log->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(m_log);
    auto* buttons = new QHBoxLayout;
    buttons->addStretch();
    auto* save_button = new QPushButton("Save Program...");
    buttons->addWidget(save_button);
    layout->addLayout(buttons);
    if (QWidget* old_panel = infoDockWidget->widget()) old_panel->deleteLater();
    infoDockWidget->setWidget(panel);
    QObject::connect(save_button, &QPushButton::clicked, this, [this]() { save_program(); });
  }

  // The initial buckets.
  log_statements({"red = Polygon_set_2()", "black = Polygon_set_2()", "blue = Polygon_set_2()"}, true);

  // The Console panel: the output, and an input line with a prompt.
  {
    auto* panel = new QWidget;
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(4, 4, 4, 4);
    QFont fixed = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_console_output = new QPlainTextEdit;
    m_console_output->setReadOnly(true);
    m_console_output->setFont(fixed);
    m_console_output->appendPlainText("Type Python-style statements on the polygon sets, e.g.,\n"
      "  red.join(Polygon_2([Point_2(0, 0), Point_2(40, 0), Point_2(0, 30)]))\n" "  blue.intersection(red, black)\n"
      "  result = Polygon_set_2(red)");
    layout->addWidget(m_console_output);
    auto* input_row = new QHBoxLayout;
    auto* prompt = new QLabel(">>>");
    prompt->setFont(fixed);
    input_row->addWidget(prompt);
    m_console_input = new QLineEdit;
    m_console_input->setFont(fixed);
    input_row->addWidget(m_console_input);
    layout->addLayout(input_row);
    if (QWidget* old_panel = consoleDockWidget->widget()) old_panel->deleteLater();
    consoleDockWidget->setWidget(panel);
    QObject::connect(m_console_input, &QLineEdit::returnPressed, this, [this]() {
      QString line = m_console_input->text();
      m_console_input->clear();
      run_console_line(line);
    });
    m_console_input->installEventFilter(this);
  }

  // Grid: show/hide, spacing, and snapping of clicked points to the grid.
  QObject::connect(actionGrid, &QAction::toggled, this, [this](bool a_check) { m_scene.set_grid_visible(a_check); });
  auto* grid_snapper = new Grid_snapper(&m_scene, this);
  qApp->installEventFilter(grid_snapper);
  QObject::connect(actionSnapToGrid, &QAction::toggled, this, [grid_snapper](bool a_check) {
    grid_snapper->set_enabled(a_check);
  });
  // Grid density: step through the spacings below. A denser grid has a smaller spacing. The menu items show the spacing
  // they lead to, and are disabled at the ends of the range.
  static const std::array<double, 7> spacings = {1, 2, 5, 10, 20, 50, 100};
  auto update_density_actions = [this]() {
    auto it = std::find(spacings.begin(), spacings.end(), m_scene.grid_spacing());
    bool denser = (it != spacings.begin());
    bool sparser = (it + 1 != spacings.end());
    actionGridDenser->setEnabled(denser);
    actionGridSparser->setEnabled(sparser);
    actionGridDenser->setText(denser ? QString("&Increase Grid Density (Spacing %1)").arg(*(it - 1)) :
      QString("&Increase Grid Density"));
    actionGridSparser->setText(sparser ? QString("&Decrease Grid Density (Spacing %1)").arg(*(it + 1)) :
      QString("&Decrease Grid Density"));
  };
  auto change_density = [this, update_density_actions](int aDirection) {
    auto it = std::find(spacings.begin(), spacings.end(), m_scene.grid_spacing());
    it += aDirection;
    if ((it < spacings.begin()) || (it >= spacings.end())) return;
    m_scene.set_grid_spacing(*it);
    update_density_actions();
    statusBar()->showMessage(QString("Grid spacing: %1").arg(*it), 3000);
  };
  QObject::connect(actionGridDenser, &QAction::triggered, this, [change_density]() { change_density(-1); });
  QObject::connect(actionGridSparser, &QAction::triggered, this, [change_density]() { change_density(+1); });
  update_density_actions();

  // The cells of the bucket rows, by column, and the other widgets of the rows.
  m_show_column = {showRed, showBlack, showBlue, showBrown, showYellow, showMagenta, showAqua};
  m_draw_column = {drawRed, drawBlack, drawBlue, drawBrown, drawYellow, drawMagenta, drawAqua};
  m_complement_column = {showRedComp, showBlackComp, showBlueComp, showBrownComp, showYellowComp, showMagentaComp,
                         showAquaComp};
  m_intersection_column = {showRedInt, showBlackInt, showBlueInt, showBrownInt, showYellowInt, showMagentaInt,
                           showAquaInt};
  m_union_column = {showRedUnion, showBlackUnion, showBlueUnion, showBrownUnion, showYellowUnion, showMagentaUnion,
                    showAquaUnion};
  m_difference_column = {showRedDiff, showBlackDiff, showBlueDiff, showBrownDiff, showYellowDiff, showMagentaDiff,
                         showAquaDiff};
  m_symmetric_difference_column = {showRedSym_Diff, showBlackSym_Diff, showBlueSym_Diff, showBrownSym_Diff,
                                   showYellowSym_Diff, showMagentaSym_Diff, showAquaSym_Diff};
  m_minkowski_sum_column = {showRedMink_Sum, showBlackMink_Sum, showBlueMink_Sum, showBrownMink_Sum, showYellowMink_Sum,
                            showMagentaMink_Sum, showAquaMink_Sum};
  m_copy_column = {copyRed, copyBlack, copyBlue, copyBrown, copyYellow, copyMagenta, copyAqua};
  m_move_column = {moveRed, moveBlack, moveBlue, moveBrown, moveYellow, moveMagenta, moveAqua};
  m_clear_column = {clearRed, clearBlack, clearBlue, clearBrown, clearYellow, clearMagenta, clearAqua};
  m_bucket_labels = {showRedLabel, showBlackLabel, showBlueLabel, showBrownLabel, showYellowLabel, showMagentaLabel,
                     showAquaLabel};
  m_bucket_separators = {nullptr, nullptr, nullptr, line10, line9, line8, line7};

  // Visible shows or hides a bucket; Draw/Insert selects one bucket (the radio buttons are exclusive).
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    QObject::connect(m_show_column[i], &QAbstractButton::toggled, this,
                     [this, i](bool checked) { show_bucket(i, checked); });
    QObject::connect(m_draw_column[i], &QAbstractButton::toggled, this, [this, i](bool checked) {
      if (checked) set_draw_bucket(i);
    });
  }

  // Exactly one bucket is the operand of the complement operation: make the Complement column exclusive, so clicking
  // the marked cell is a no-op.
  auto* complement_group = new QButtonGroup(this);
  complement_group->setExclusive(true);
  for (auto* box : m_complement_column) complement_group->addButton(box);

  // At most one bucket is copied or moved: checking a cell of the Copy or Move column unchecks the others.
  for (const Column* column : {&m_copy_column, &m_move_column}) {
    for (auto* box : *column) {
      QObject::connect(box, &QAbstractButton::toggled, this, [column, box](bool checked) {
        if (! checked) return;
        for (auto* other : *column) {
          if (other != box) other->setChecked(false);
        }
      });
    }
  }

  // In the Intersection and Union columns at least one bucket stays unchecked, to hold the result, and the operations
  // are enabled only with at least one operand.
  const std::pair<const Column*, QAbstractButton*>
    multi_operand_columns[] = {{&m_intersection_column, actionIntersectionH}, {&m_union_column, actionUnionH}};
  for (const auto& [column, operation] : multi_operand_columns) {
    for (auto* box : *column)
      QObject::connect(box, &QAbstractButton::toggled, this,
                       [this, box, column, operation](bool checked)
                       { limit_to_all_but_one(*column, box, checked, operation); });
    // Set the initial state of the operation.
    limit_to_all_but_one(*column, (*column)[0], false, operation);
  }

  // The Difference, Symmetric Difference, and Minkowski Sum columns have at most two marked cells, and their operations
  // are enabled only with exactly two.
  const std::pair<const Column*, QAbstractButton*> two_operand_columns[] = {
    {&m_difference_column, actionDifferenceH}, {&m_symmetric_difference_column, actionSymmetric_DifferenceH},
    {&m_minkowski_sum_column, actionMinkowski_SumH}};
  for (const auto& [column, operation] : two_operand_columns) {
    for (auto* box : *column)
      QObject::connect(box, &QAbstractButton::toggled, this,
                       [this, box, column, operation](bool checked)
                       { limit_to_two_operands(*column, box, checked, operation); });
    // Set the initial state of the operation.
    limit_to_two_operands(*column, (*column)[0], false, operation);
  }

  // Fit the Visible and Draw/Insert header images into the same icon area as the operation buttons next to them
  // (27x25), keeping their aspect ratio.
  for (auto* label : {VisibleIcon, DrawHeader}) {
    QPixmap pixmap = label->pixmap();
    if (! pixmap.isNull()) label->setPixmap(pixmap.scaled(27, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }

  // The scene and the view.
  m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
  m_scene.setSceneRect(-320, -210, 640, 420);
  this->graphicsView->setScene(&m_scene);
  // Use a white canvas regardless of the desktop color scheme, so that the
  // drawing area stands out from the surrounding widgets. (The scene, rather
  // than the view, paints it, so that the scene can draw the grid over it.)
  m_scene.setBackgroundBrush(Qt::white);
  this->graphicsView->setMouseTracking(true);
  this->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Turn the vertical axis upside down
  this->graphicsView->scale(2.5, -2.5);

  // The navigation adds zooming and translation functionality to the view.
  this->addNavigation(this->graphicsView);
  // Every change of the view (zoom, pan, resize) ends in a repaint, where the area that unbounded sets are drawn over
  // is updated.
  this->graphicsView->viewport()->installEventFilter(this);

  // The menus.
  this->setupStatusBar();
  this->setupOptionsMenu();

  // The recently opened files.
  this->addRecentFiles(this->menuFile, this->actionClose);

  // The first state of the undo stack.
  get_new_state(State_op::Start);
  actionUndo->setEnabled(false);

  this->setWindowState(Qt::WindowMaximized);

  // The inputs, which construct polygons with the mouse.
  m_bezier_input = new CGAL::Qt::GraphicsViewBezierPolygonInput<Bezier_traits>(this, &m_scene);
  m_linear_input = new CGAL::Qt::Graphics_view_linear_polygon_input<Kernel>(this, &m_scene);
  m_circular_input = new CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>(this, &m_scene);
  m_polyline_input = new CGAL::Qt::Graphics_view_polyline_input<Kernel>(this, &m_scene);
  m_conic_input = new CGAL::Qt::Graphics_view_conic_input(this, &m_scene);

  // The curve families. Selecting one in the tool bar starts constructing its curves.
  m_families = {{
    {Curve_type::Linear, "linear", actionInsertLinear, m_linear_input, [this] { m_linear_input->reset(); },
     actionOpenLinear, ".lps", &MainWindow::read_linear},
    {Curve_type::Circular, "circular", actionInsertCircular, m_circular_input, [this] { m_circular_input->reset(); },
     actionOpenDXF, ".cps", &MainWindow::read_circular},
    {Curve_type::Bezier, "Bezier", actionInsertBezier, m_bezier_input, [this] { m_bezier_input->reset(); },
     actionOpenBezier, ".bps", &MainWindow::read_bezier},
    {Curve_type::Polyline, "polyline", actionInsertPolyline, m_polyline_input, [this] { m_polyline_input->reset(); },
     actionOpenPolyline, ".mps", &MainWindow::read_polyline},
    {Curve_type::Conic, "conic", actionInsertConic, m_conic_input, [this] { m_conic_input->reset(); }, nullptr,
     nullptr, nullptr}}};
  for (const auto& family : m_families) {
    Curve_type type = family.m_type;
    QObject::connect(family.m_insert_action, &QAction::toggled, this, [this, type](bool checked) {
      if (checked) start_inserting(type);
    });
  }

  // The curve tool bar, underneath the curve families: the ways to construct curves of the selected family.
  {
    using Circular_input = CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>;
    auto* bar = new QToolBar;
    bar->setIconSize(QSize(24, 24));
    auto* group = new QActionGroup(this);
    auto* arcs = bar->addAction(curve_tool_icon("arcs"), "Line Segments and Circular Arcs");
    arcs->setToolTip("Line Segments and Circular Arcs: click at each vertex; "
                     "press and drag to bend a piece into an arc through the "
                     "cursor; right-click to close (drag to bend the last " "piece)");
    auto* circle = bar->addAction(curve_tool_icon("full circle"), "Circle");
    circle->setToolTip("Circle: press at the center and drag to the radius");
    for (auto* action : {arcs, circle}) {
      action->setCheckable(true);
      group->addAction(action);
    }
    arcs->setChecked(true);
    QObject::connect(circle, &QAction::toggled, this, [this](bool aChecked) {
      m_circular_input->set_mode(aChecked ? Circular_input::Circle_mode : Circular_input::Arcs);
    });

    // The conic tools: conics that bound a polygon, full curves, and a conic given by its coefficients.
    using Conic_input = CGAL::Qt::Graphics_view_conic_input;
    auto* conic_group = new QActionGroup(this);
    const std::vector<std::tuple<QString, QString, Conic_input::Mode, QString>>
      conic_tools = {
      {"line", "Line", Conic_input::LINE,
       "Line: press at one of its points and drag to another"},
      {"circle", "Circle", Conic_input::CIRCLE,
       "Circle: press at the center and drag to the radius"},
      {"ellipse", "Ellipse", Conic_input::ELLIPSE,
       "Ellipse: click at the center, press at the end of an axis, and "
       "drag to the width of the other axis"},
      {"full circle", "Full Circle", Conic_input::FULL_CIRCLE,
       "Full Circle: press at the center and drag to the radius"},
      {"full ellipse", "Full Ellipse", Conic_input::FULL_ELLIPSE,
       "Full Ellipse: click at the center, press at the end of an axis, and "
       "drag to the width of the other axis"}};
    const QString boundary_help = ". Every conic you add meets the previous one at the next vertex (click "
      "at it when there are several); on a circle or an ellipse, click near "
      "the arc to keep; right-click to close the boundary. Escape or Backspace "
      "discards the conic under construction, or else removes the last "
      "conic.";
    std::vector<QAction*> conic_actions;
    for (const auto& [icon, text, mode, tip] : conic_tools) {
      if (mode == Conic_input::FULL_CIRCLE)
        conic_actions.push_back(bar->addSeparator());
      auto* action = bar->addAction(curve_tool_icon(icon), text);
      action->setToolTip((mode < Conic_input::FULL_CIRCLE) ? tip + boundary_help : tip);
      action->setCheckable(true);
      conic_group->addAction(action);
      Conic_input::Mode m = mode;
      QObject::connect(action, &QAction::triggered, this, [this, m]() { m_conic_input->set_mode(m); });
      conic_actions.push_back(action);
    }
    conic_actions[0]->setChecked(true);
    conic_actions.push_back(bar->addSeparator());
    auto* coefficients = bar->addAction(curve_tool_icon("coefficients"), "Coefficients...");
    coefficients->setToolTip("Coefficients: enter a conic " "r x\u00b2 + s y\u00b2 + t xy + u x + v y + w = 0, "
                             "and add it to the boundary or construct the full " "ellipse");
    QObject::connect(coefficients, &QAction::triggered, this, [this]() { m_conic_input->enter_coefficients(this); });
    conic_actions.push_back(coefficients);

    // Keep the height of the bar when it shows no buttons.
    bar->setMinimumHeight(bar->sizeHint().height());
    auto show_tools = [this, arcs, circle, conic_actions]() {
      bool circular = actionInsertCircular->isChecked();
      arcs->setVisible(circular);
      circle->setVisible(circular);
      for (auto* action : conic_actions)
        action->setVisible(actionInsertConic->isChecked());
    };
    show_tools();
    QObject::connect(actionInsertCircular, &QAction::toggled, this, show_tools);
    QObject::connect(actionInsertConic, &QAction::toggled, this, show_tools);
    verticalLayout->insertWidget(1, bar);
  }

  // The polygons constructed by the inputs are added to the Draw/Insert bucket.
  for (const auto& family : m_families) {
    QObject::connect(family.m_input, &CGAL::Qt::GraphicsViewInput::generate, this, &MainWindow::process_input);
  }

  m_scene.installEventFilter(m_linear_input);

  // Under Wayland, let the compositor move floating dock widgets.
  if (QGuiApplication::platformName().startsWith("wayland")) {
    auto* dock_mover = new Floating_dock_mover(this);
    for (auto* dock : findChildren<QDockWidget*>()) dock->installEventFilter(dock_mover);
  }

  QObject::connect(this->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

  // The actions and the widgets of the panels.
  QObject::connect(actionNew, &QAction::triggered, this, &MainWindow::new_program);
  QObject::connect(actionUndo, &QAction::triggered, this, &MainWindow::undo);
  QObject::connect(actionAxis, &QAction::triggered, this, &MainWindow::toggle_axes);
  QObject::connect(actionPAN, &QAction::triggered, this, &MainWindow::toggle_pan);
  QObject::connect(actionRecenter, &QAction::triggered, this, &MainWindow::zoom_to_fit);
  QObject::connect(actionOpenLinear, &QAction::triggered, this, &MainWindow::open_linear);
  QObject::connect(actionOpenDXF, &QAction::triggered, this, &MainWindow::open_circular);
  QObject::connect(actionOpenBezier, &QAction::triggered, this, &MainWindow::open_bezier);
  QObject::connect(actionOpenPolyline, &QAction::triggered, this, &MainWindow::open_polyline);
  QObject::connect(actionSaveCurrentBucket, &QAction::triggered, this, &MainWindow::save_draw_bucket);
  QObject::connect(actionAddColor, &QAction::triggered, this, &MainWindow::add_bucket);
  QObject::connect(actionMinusColor, &QAction::triggered, this, &MainWindow::remove_bucket);
  QObject::connect(actionDeleteAll, &QAction::triggered, this, &MainWindow::delete_all);
  QObject::connect(showColorBucket, &QAction::toggled, this, &MainWindow::show_operations_panel);
  QObject::connect(showConsole, &QAction::toggled, this, &MainWindow::show_console_panel);
  QObject::connect(showInfo, &QAction::toggled, this, &MainWindow::show_log_panel);
  QObject::connect(aboutBolop, &QAction::triggered, this, &MainWindow::about_boolean_set_operations);
  QObject::connect(aboutCGAL, &QAction::triggered, this, &MainWindow::about_cgal);
  QObject::connect(aboutMinkop, &QAction::triggered, this, &MainWindow::about_minkowski_sums);
  QObject::connect(aboutDemo, &QAction::triggered, this, &MainWindow::about_demo);
  QObject::connect(actionComplementH, &QAbstractButton::toggled, this, &MainWindow::apply_complement);
  QObject::connect(actionIntersectionH, &QAbstractButton::toggled, this, &MainWindow::apply_intersection);
  QObject::connect(actionUnionH, &QAbstractButton::toggled, this, &MainWindow::apply_union);
  QObject::connect(actionDifferenceH, &QAbstractButton::toggled, this, &MainWindow::apply_difference);
  QObject::connect(actionSymmetric_DifferenceH, &QAbstractButton::toggled, this,
                   &MainWindow::apply_symmetric_difference);
  QObject::connect(actionMinkowski_SumH, &QAbstractButton::toggled, this, &MainWindow::apply_minkowski_sum);
  QObject::connect(actionCopyH, &QAbstractButton::toggled, this, &MainWindow::apply_copy);
  QObject::connect(actionMoveH, &QAbstractButton::toggled, this, &MainWindow::apply_move);
  QObject::connect(actionClearH, &QAbstractButton::toggled, this, &MainWindow::apply_clear);
  QObject::connect(showResult, &QAbstractButton::toggled, this, &MainWindow::show_result);
  QObject::connect(VisibleHeader, &QAbstractButton::toggled, this, &MainWindow::show_all_buckets);
  QObject::connect(showClear, &QAbstractButton::toggled, this, &MainWindow::mark_all_for_clearing);
  QObject::connect(sceneDockWidget, &QDockWidget::visibilityChanged, this,
                   &MainWindow::operations_panel_visibility_changed);
  QObject::connect(infoDockWidget, &QDockWidget::visibilityChanged, this, &MainWindow::log_panel_visibility_changed);
  QObject::connect(consoleDockWidget, &QDockWidget::visibilityChanged, this,
                   &MainWindow::console_panel_visibility_changed);

  // Opening a recently opened file.
  QObject::connect(this, SIGNAL(openRecentFile(QString)), this, SLOT(open(QString)));

  QObject::connect(actionErrorOperand, &QAction::toggled, this, [this](bool a_check) { operand_error_warn = a_check; });
  QObject::connect(actionWarnNotEmpty, &QAction::toggled, this, [this](bool a_check) { empty_warn = a_check; });

  this->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

  // The axes.
  QPen axis_pen(QBrush(Qt::black), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
  xAxis->setLine(-155000000000000, 0, 155000000000000, 0);
  xAxis->setPen(axis_pen);
  yAxis->setLine(0, -100000000000000, 0, 100000000000000);
  yAxis->setPen(axis_pen);

  m_linear_input->mOngoingPieceGI->set_pen(sPens[0]);
  m_linear_input->mLinearGI->set_pen(sPens[0]);
  m_linear_input->mHandleGI->setPen(sPens[0]);
  actionOpenLinear->setEnabled(true);
  actionOpenDXF->setEnabled(false);
  actionOpenBezier->setEnabled(false);
  actionOpenPolyline->setEnabled(false);
}

void MainWindow::show_result(bool a_check) { states_stack.back().toggle_view(result_index, a_check); }

void MainWindow::show_bucket(std::size_t aIndex, bool aChecked) {
  states_stack.back().toggle_view(aIndex, aChecked);
  m_draw_column[aIndex]->setEnabled(aChecked);
  if (aIndex >= m_number_of_buckets) return;
  auto visible = number_of_visible_buckets();
  if (aChecked) {
    if (visible == 1) m_scene.installEventFilter(current_input());
  }
  else if (visible == 0) remove_inputs();
  else if (aIndex == m_color_active) {
    for (std::size_t i = 0; i < m_number_of_buckets; ++i) {
      if (m_show_column[i]->isChecked()) {
        m_draw_column[i]->setChecked(true);
        break;
      }
    }
  }
  // The header is checked when all buckets are visible.
  QSignalBlocker blocker(VisibleHeader);
  VisibleHeader->setChecked(visible == m_number_of_buckets);
}

void MainWindow::set_draw_bucket(std::size_t aIndex) {
  m_color_active = aIndex;
  set_input_pen(sPens[aIndex]);
}

std::size_t MainWindow::number_of_visible_buckets() const {
  return std::count_if(m_show_column.begin(), m_show_column.begin() + m_number_of_buckets,
                       [](QAbstractButton* box) { return box->isChecked(); });
}

void MainWindow::set_number_of_buckets(std::size_t aNumber) {
  m_number_of_buckets = aNumber;
  const Column* columns[] = {&m_show_column, &m_draw_column, &m_complement_column, &m_intersection_column,
                             &m_union_column, &m_difference_column, &m_symmetric_difference_column,
                             &m_minkowski_sum_column, &m_copy_column, &m_move_column, &m_clear_column};
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    bool exists = (i < aNumber);
    for (const Column* column : columns) (*column)[i]->setVisible(exists);
    m_bucket_labels[i]->setVisible(exists);
    if (m_bucket_separators[i]) m_bucket_separators[i]->setVisible(exists);
  }

  // The vertical lines between the columns span the rows.
  int height = 155 + 30 * int(aNumber - min_number_of_buckets);
  for (auto* line : {line0006, line1, line2, line3, line4, line5, line6, line0007, line06, line061, line062, line063})
    line->setGeometry(QRect(line->x(), 0, 7, height));

  bool can_add = (aNumber < number_of_buckets);
  actionAddColor->setEnabled(can_add);
  actionAddColor->setText(can_add ? "Add a Bucket" : "Maximum Number of Buckets Reached");
  bool can_remove = (aNumber > min_number_of_buckets);
  actionMinusColor->setEnabled(can_remove);
  if (can_remove) {
    actionMinusColor->setText("Remove " + capitalized(bucket_name(aNumber - 1)));
  }
  else actionMinusColor->setText("Minimum Number of Buckets Reached");
}

std::optional<std::size_t> MainWindow::checked_bucket(const Column& aColumn) const {
  auto buckets = checked_buckets(aColumn);
  if (buckets.empty()) return std::nullopt;
  return buckets.front();
}

void MainWindow::show_all_buckets(bool a_check) {
  for (auto* box : m_show_column) box->setChecked(a_check);
  if (a_check) m_draw_column[0]->setChecked(true);
}

void MainWindow::mark_all_for_clearing(bool a_check) {
  for (auto* box : m_clear_column) box->setChecked(a_check);
}

void MainWindow::operations_panel_visibility_changed(bool a_check) {
  if (a_check) showColorBucket->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) showColorBucket->setChecked(false);
}

void MainWindow::log_panel_visibility_changed(bool a_check) {
  if (a_check) showInfo->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) showInfo->setChecked(false);
}

void MainWindow::console_panel_visibility_changed(bool a_check) {
  if (a_check) showConsole->setChecked(true);
  else if (! this->windowState().testFlag(Qt::WindowMinimized)) showConsole->setChecked(false);
}

void MainWindow::show_operations_panel(bool a_check) {
  if (a_check) sceneDockWidget->setVisible(true);
  else sceneDockWidget->setVisible(false);
}

void MainWindow::show_console_panel(bool a_check) {
  if (a_check) consoleDockWidget->setVisible(true);
  else consoleDockWidget->setVisible(false);
}

void MainWindow::show_log_panel(bool a_check) {
  if (a_check) infoDockWidget->setVisible(true);
  else infoDockWidget->setVisible(false);
}

void MainWindow::add_bucket() {
  if (m_number_of_buckets == number_of_buckets) return;
  log_statements({QString("%1 = Polygon_set_2()").arg(bucket_name(m_number_of_buckets))}, true);
  set_number_of_buckets(m_number_of_buckets + 1);
  model_changed();
}

void MainWindow::remove_bucket() {
  if (m_number_of_buckets == min_number_of_buckets) return;
  std::size_t i = m_number_of_buckets - 1;
  log_statements({QString("del %1").arg(bucket_name(i))}, true);
  states_stack.back().clear(i);

  // If the removed bucket is marked for Draw/Insert or Complement, mark the first bucket instead.
  if (m_color_active == i) m_draw_column[0]->setChecked(true);
  if (m_complement_column[i]->isChecked()) m_complement_column[0]->setChecked(true);
  // Unmark the removed bucket as an operand (this disables an operation that no longer has two operands), and as the
  // bucket to copy, move, or clear.
  for (const Column* column : {&m_intersection_column, &m_union_column, &m_difference_column,
                               &m_symmetric_difference_column, &m_minkowski_sum_column, &m_copy_column, &m_move_column,
                               &m_clear_column})
    (*column)[i]->setChecked(false);
  set_number_of_buckets(i);
  // Keep at least one remaining bucket unmarked in the Intersection and Union columns, for the result.
  keep_one_unchecked(m_intersection_column);
  keep_one_unchecked(m_union_column);
  model_changed();
}

void MainWindow::undo() {
  if (m_state_num > 1) {
    // The sets of the previous state are shown again by model_changed().
    states_stack.pop_back();
    if (states_stack.size() == 1) actionUndo->setEnabled(false);
    m_state_num = states_stack.size();
    trim_program_to_states();
    model_changed();
  }
  else {
    show_error("Nothing to Undo!!!");
  }
}

void MainWindow::reset_marks() {
  // The initial marks: all buckets are visible, the first one is the Draw/Insert bucket and the complement operand,
  // and the first two are the operands of the other operations. (Unmark the other operands first, so that marking the
  // first two is not undone.)
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    m_show_column[i]->setChecked(true);
    for (const Column* column : {&m_copy_column, &m_move_column, &m_clear_column})
      (*column)[i]->setChecked(false);
  }
  const Column* operand_columns[] = {&m_intersection_column, &m_union_column, &m_difference_column,
                                     &m_symmetric_difference_column, &m_minkowski_sum_column};
  for (const Column* column : operand_columns) {
    for (std::size_t i = 2; i < number_of_buckets; ++i) (*column)[i]->setChecked(false);
  }
  for (const Column* column : operand_columns) {
    for (std::size_t i = 0; i < 2; ++i) (*column)[i]->setChecked(true);
  }
  m_draw_column[0]->setChecked(true);
  m_complement_column[0]->setChecked(true);
  showResult->setChecked(false);
}

void MainWindow::new_program() {
  if (m_state_num > 1) get_new_state(State_op::Reset);

  // Reset starts a new program, which constructs the initial buckets.
  m_program.clear();
  if (m_log) m_log->clear();
  QStringList program;
  for (std::size_t i = 0; i < min_number_of_buckets; ++i)
    program << QString("%1 = Polygon_set_2()").arg(bucket_name(i));
  log_statements(program, true);

  for (std::size_t i = 0; i <= number_of_buckets; ++i) states_stack.back().clear(i);

  m_scene.setSceneRect(-320, -210, 640, 420);
  this->graphicsView->setScene(&m_scene);
  this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  this->graphicsView->scale(2.5, 2.5);

  m_disjoint = false;

  m_linear_input->reset();
  m_circular_input->reset();
  m_bezier_input->reset();
  m_polyline_input->reset();

  m_pan = false;

  m_color_active = 0;

  m_linear_input->mOngoingPieceGI->set_pen(sPens[0]);
  m_linear_input->mLinearGI->set_pen(sPens[0]);
  m_linear_input->mHandleGI->setPen(sPens[0]);

  sceneDockWidget->setVisible(true);
  consoleDockWidget->setVisible(true);
  infoDockWidget->setVisible(true);

  showColorBucket->setChecked(true);
  showConsole->setChecked(true);
  showInfo->setChecked(true);

  actionCopyH->setChecked(false);
  actionMoveH->setChecked(false);

  actionComplementH->setChecked(false);
  actionUnionH->setChecked(false);
  actionIntersectionH->setChecked(false);
  actionDifferenceH->setChecked(false);
  actionSymmetric_DifferenceH->setChecked(false);
  actionMinkowski_SumH->setChecked(false);

  m_reset = true;
  for (const auto& family : m_families) family.m_insert_action->setChecked(family.m_type == m_curve_type);

  m_reset = false;

  actionDifferenceH->setEnabled(true);

  reset_marks();

  set_number_of_buckets(min_number_of_buckets);
  VisibleHeader->setChecked(true);
  showClear->setChecked(false);
  line11->setVisible(true);

  zoom_to_fit();
  model_changed();
}

void MainWindow::delete_all() {
  actionComplementH->setChecked(false);
  actionIntersectionH->setChecked(false);
  actionUnionH->setChecked(false);
  actionDifferenceH->setChecked(false);
  actionSymmetric_DifferenceH->setChecked(false);
  actionMinkowski_SumH->setChecked(false);

  get_new_state(State_op::Delete_all);
  QStringList program;
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    if (! states_stack.back().set(i).is_empty()) program << QString("%1.clear()").arg(bucket_name(i));
  }
  log_statements(program);

  for (std::size_t i = 0; i < number_of_buckets; ++i) states_stack.back().clear(i);
  delete_result();
  model_changed();
}

void MainWindow::delete_result() {
  states_stack.back().clear(result_index);
  model_changed();
}

void MainWindow::apply_clear(bool aChecked) {
  if (! aChecked) return;
  actionComplementH->setChecked(false);
  actionIntersectionH->setChecked(false);
  actionUnionH->setChecked(false);
  actionDifferenceH->setChecked(false);
  actionSymmetric_DifferenceH->setChecked(false);
  actionMinkowski_SumH->setChecked(false);

  auto buckets = checked_buckets(m_clear_column);
  get_new_state(State_op::Clear);
  QStringList program;
  for (auto i : buckets) program << QString("%1.clear()").arg(bucket_name(i));
  log_statements(program);

  for (auto i : buckets) states_stack.back().clear(i);
  if (buckets.empty()) show_error("Please select color bucket to be cleaned!!!");

  actionClearH->setChecked(false);
  model_changed();
}

void MainWindow::open_linear()
{ open(QFileDialog::getOpenFileName(this, tr("Open Linear Polygon"), "./data", tr("Linear Curve files (*.lps)"))); }

void MainWindow::open_circular() {
  open(QFileDialog::getOpenFileName(this, tr("Open Line segments and circular arcs"), "./data",
                                    tr("Circular curve files (*.cps)")));
}

void MainWindow::open_bezier()
{ open(QFileDialog::getOpenFileName(this, tr("Open Bezier Polygon"), "./data", tr("Bezier Curve files (*.bps)"))); }

void MainWindow::open_polyline()
{ open(QFileDialog::getOpenFileName(this, tr("Open Polyline Polygon"), "./data", tr("Polyline Curve files (*.mps)"))); }

/*! obtains the x-monotone curves of given curves.
 */
template <typename Traits, typename InputIterator>
std::vector<typename Traits::X_monotone_curve_2> x_monotone_curves(InputIterator aBegin, InputIterator aEnd) {
  using Point = typename Traits::Point_2;
  using X_monotone_curve = typename Traits::X_monotone_curve_2;
  Traits traits;
  auto make_x_monotone = traits.make_x_monotone_2_object();
  std::vector<std::variant<Point, X_monotone_curve>> objects;
  for (auto it = aBegin; it != aEnd; ++it) make_x_monotone(*it, std::back_inserter(objects));
  std::vector<X_monotone_curve> xcvs;
  for (const auto& object : objects) {
    if (const auto* xcv = std::get_if<X_monotone_curve>(&object)) xcvs.push_back(*xcv);
  }
  return xcvs;
}

/*! reads the vertices of a boundary: their number followed by their coordinates.
 */
std::vector<Kernel::Point_2> read_vertices(std::istream& aIs) {
  unsigned int n_points;
  aIs >> n_points;
  std::vector<Kernel::Point_2> points;
  for (unsigned int i = 0; i < n_points; ++i) {
    double x, y;
    aIs >> x >> y;
    points.emplace_back(x, y);
  }
  return points;
}

/*! reads the boundary of a linear polygon: its vertices. \return the x-monotone curves of the boundary.
 */
std::vector<Linear_X_monotone_curve> read_linear_boundary(std::istream& aIs, bool /* aDoubleFormat */) {
  auto points = read_vertices(aIs);
  std::vector<Linear_curve> curves;
  for (std::size_t i = 0; i < points.size(); ++i) {
    const auto& p = points[i];
    const auto& q = points[(i + 1) % points.size()];
    if (p != q) curves.emplace_back(p, q);
  }
  return x_monotone_curves<Linear_traits>(curves.begin(), curves.end());
}

/*! reads the boundary of a polygon bounded by polylines: its vertices. \return the x-monotone curves of the boundary.
 */
std::vector<Polyline_X_monotone_curve> read_polyline_boundary(std::istream& aIs, bool /* aDoubleFormat */) {
  auto points = read_vertices(aIs);
  Polyline_traits traits;
  auto construct_segment = traits.subcurve_traits_2()->construct_x_monotone_curve_2_object();
  std::vector<Polyline_curve> curves;
  for (std::size_t i = 0; i < points.size(); ++i) {
    const auto& p = points[i];
    const auto& q = points[(i + 1) % points.size()];
    if (p != q) curves.emplace_back(construct_segment(p, q));
  }
  return x_monotone_curves<Polyline_traits>(curves.begin(), curves.end());
}

/*! reads the boundary of a polygon bounded by segments and circular arcs: the number of its curves followed by the
 * curves, each given by a flag (1 for an arc) followed by its source, a point in its interior (for an arc), and its
 * target. \return the x-monotone curves of the boundary.
 */
std::vector<Circular_X_monotone_curve> read_circular_boundary(std::istream& aIs, bool /* aDoubleFormat */) {
  using Point = Kernel::Point_2;
  unsigned int n_curves;
  aIs >> n_curves;
  std::vector<Circular_curve> curves;
  for (unsigned int i = 0; i < n_curves; ++i) {
    unsigned int is_arc;
    double sx, sy, mx, my, tx, ty;
    aIs >> is_arc >> sx >> sy;
    if (is_arc) aIs >> mx >> my;
    aIs >> tx >> ty;
    if (is_arc) curves.emplace_back(Point(sx, sy), Point(mx, my), Point(tx, ty));
    else curves.emplace_back(Point(sx, sy), Point(tx, ty));
  }
  return x_monotone_curves<Circular_traits>(curves.begin(), curves.end());
}

/*! reads a Bezier curve: the number of its control points followed by their coordinates, as doubles (rounded to
 * 1/1000) or as rationals. Repeated control points and control points collinear with their neighbors are dropped.
 */
Bezier_curve read_bezier_curve(std::istream& is, bool aDoubleFormat) {
  // Read the number of control points.
  unsigned int n;
  is >> n;

  // Read the control points.
  std::vector<Bezier_rat_point> ctrl_pts;

  for (unsigned int k = 0; k < n; k++) {
    Bezier_rat_point p;
    if (aDoubleFormat) {
      double x, y;
      is >> x >> y;
      Bezier_rational rx(static_cast<int> (1000 * x + 0.5), 1000);
      Bezier_rational ry(static_cast<int> (1000 * y + 0.5), 1000);
      p = Bezier_rat_point(rx, ry);
    }
    else {
      is >> p;
    }

    if (k == 0 || ctrl_pts[k - 1] != p) {
      ctrl_pts.push_back(p);
    }
  }
  std::vector<Bezier_rat_point> ctrl_pts2;
  auto beg = ctrl_pts.begin();
  auto end = ctrl_pts.end();
  auto last = end - 1;
  ctrl_pts2.push_back(*beg);
  if (ctrl_pts.size() > 2) {
    auto curr = beg;
    auto next1 = curr + 1;
    auto next2 = next1 + 1;
    do {
      CGAL::Orientation lOrient = orientation(*curr, *next1, *next2);
      if (lOrient != CGAL::COLLINEAR)
        ctrl_pts2.push_back(*next1);
      ++curr;
      ++next1;
      ++next2;
    } while (next2 != end);
  }
  ctrl_pts2.push_back(*last);
  return Bezier_curve(ctrl_pts2.begin(), ctrl_pts2.end());
}

/*! reads the boundary of a Bezier polygon: the number of its curves followed by the curves. \return the x-monotone
 * curves of the boundary.
 */
std::vector<Bezier_X_monotone_curve> read_bezier_boundary(std::istream& aIs, bool aDoubleFormat) {
  unsigned int n_curves;
  aIs >> n_curves;
  std::vector<Bezier_curve> curves;
  for (unsigned int i = 0; i < n_curves; ++i) {
    Bezier_curve curve = read_bezier_curve(aIs, aDoubleFormat);
    if (curve.number_of_control_points() >= 2) curves.push_back(curve);
  }
  return x_monotone_curves<Bezier_traits>(curves.begin(), curves.end());
}

template <typename Set, typename Read_boundary>
bool MainWindow::read_polygons(const QString& aFileName, const char* aKind, Read_boundary aReadBoundary) {
  std::ifstream in_file(qPrintable(aFileName));
  if (! in_file) return false;
  bool rOK = false;
  try {
    std::string format;
    std::getline(in_file, format);
    bool double_format = (format.substr(0, 6) == "DOUBLE");

    unsigned int n_regions;
    in_file >> n_regions;
    for (unsigned int r = 0; r < n_regions; ++r) {
      unsigned int n_boundaries;
      in_file >> n_boundaries;
      for (unsigned int b = 0; b < n_boundaries; ++b) {
        auto xcvs = aReadBoundary(in_file, double_format);
        if (xcvs.empty()) continue;
        typename Set::Polygon_2 pgn(xcvs.begin(), xcvs.end());
        if (pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
        typename Set::Polygon_with_holes_2 pwh(pgn);
        // The first boundary is the outer one; the others are holes.
        if (b == 0) {
          get_new_state(State_op::Draw);
          states_stack.back().set(m_color_active).get<Set>().join(pwh);
        }
        else states_stack.back().set(m_color_active).get<Set>().difference(Set(pwh));
      }
      rOK = true;
    }
  }
  catch (...) {
    show_warning(std::string("Exception occurred during reading of ") + aKind + " polygon set.");
  }
  return rOK;
}

bool MainWindow::read_linear(QString aFileName) {
  return read_polygons<Linear_polygon_set>(aFileName, "linear", read_linear_boundary);
}

bool MainWindow::read_circular(QString aFileName) {
  return read_polygons<Circular_polygon_set>(aFileName, "circular", read_circular_boundary);
}

bool MainWindow::read_polyline(QString aFileName) {
  return read_polygons<Polyline_polygon_set>(aFileName, "polyline", read_polyline_boundary);
}

bool MainWindow::read_bezier(QString aFileName) {
  return read_polygons<Bezier_polygon_set>(aFileName, "Bezier", read_bezier_boundary);
}

/*! saves a polygon set to a file: a format line, the number of polygons with holes, and for each one the number of its
 * boundaries followed by the boundaries (the outer one first), each written by a given function. \return false if the
 * file cannot be written.
 */
template <typename Set, typename Write_boundary>
bool save_polygons(const QString& aFileName, const Set& aSet, Write_boundary aWriteBoundary) {
  std::ofstream out_file(qPrintable(aFileName));
  if (! out_file) return false;
  std::vector<typename Set::Polygon_with_holes_2> pwhs;
  aSet.polygons_with_holes(std::back_inserter(pwhs));
  out_file << "DOUBLE" << std::endl << pwhs.size() << std::endl;
  for (const auto& pwh : pwhs) {
    out_file << " " << (1 + pwh.number_of_holes()) << std::endl;
    aWriteBoundary(out_file, pwh.outer_boundary());
    for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) aWriteBoundary(out_file, *hit);
  }
  return bool(out_file);
}

/*! writes the boundary of a linear polygon: the number of its vertices followed by their coordinates.
 */
void write_linear_boundary(std::ostream& aOs, const Linear_polygon& aBoundary) {
  aOs << "  " << aBoundary.size() << std::endl;
  for (auto cit = aBoundary.curves_begin(); cit != aBoundary.curves_end(); ++cit)
    aOs << "   " << CGAL::to_double(cit->source().x()) << " " << CGAL::to_double(cit->source().y()) << std::endl;
}

/*! writes the boundary of a polygon bounded by polylines: the number of its curves followed by the coordinates of their
 * first points.
 */
void write_polyline_boundary(std::ostream& aOs, const Polyline_polygon& aBoundary) {
  aOs << "  " << aBoundary.size() << std::endl;
  for (auto cit = aBoundary.curves_begin(); cit != aBoundary.curves_end(); ++cit) {
    auto it = cit->points_begin();
    aOs << " " << CGAL::to_double(it->x()) << " " << CGAL::to_double(it->y()) << std::endl;
  }
}

/*! obtains the midpoint of a circular arc.
 */
std::pair<double, double> arc_midpoint(const Circular_X_monotone_curve& aArc) {
  double r = std::sqrt(CGAL::to_double(aArc.supporting_circle().squared_radius()));
  double sx = CGAL::to_double(aArc.source().x());
  double sy = CGAL::to_double(aArc.source().y());
  double tx = CGAL::to_double(aArc.target().x());
  double ty = CGAL::to_double(aArc.target().y());
  double cx = CGAL::to_double(aArc.supporting_circle().center().x());
  double cy = CGAL::to_double(aArc.supporting_circle().center().y());
  double st = std::sqrt((sx - tx) * (sx - tx) + (sy - ty) * (sy - ty));    // the length of the chord
  double tnx = tx - cx;                                                     // the target relative to the center
  double tny = ty - cy;
  double frmst = std::sqrt((4 * r * r) - (st * st));
  if (aArc.supporting_circle().orientation() == CGAL::CLOCKWISE)
    return {(((frmst * tnx) - (st * tny)) / (2 * r)) + cx, (((st * tnx) + (frmst * tny)) / (2 * r)) + cy};
  return {(((tnx * frmst) + (tny * st)) / (2 * r)) + cx, (((tny * frmst) - (st * tnx)) / (2 * r)) + cy};
}

/*! writes the boundary of a polygon bounded by segments and circular arcs: the number of its curves followed by the
 * curves, each given by a flag (1 for an arc) followed by its source, its midpoint (for an arc), and its target.
 */
void write_circular_boundary(std::ostream& aOs, const Circular_polygon& aBoundary) {
  aOs << "  " << aBoundary.size() << std::endl;
  for (auto cit = aBoundary.curves_begin(); cit != aBoundary.curves_end(); ++cit) {
    aOs << "   " << (cit->is_circular() ? 1 : 0) << std::endl;
    aOs << "    " << CGAL::to_double(cit->source().x()) << " " << CGAL::to_double(cit->source().y()) << std::endl;
    if (cit->is_circular()) {
      auto [mx, my] = arc_midpoint(*cit);
      aOs << "    " << mx << " " << my << std::endl;
    }
    aOs << "    " << CGAL::to_double(cit->target().x()) << " " << CGAL::to_double(cit->target().y()) << std::endl;
  }
}

/*! writes the boundary of a Bezier polygon: the number of its curves followed by their control points.
 */
void write_bezier_boundary(std::ostream& out_file, const Bezier_polygon& aBP) {
  using Bezier_rat_point_vector = std::vector<Bezier_rat_point>;

  int cc = aBP.size();
  int lc = cc - 1;
  out_file << "  " << cc << std::endl;
  Bezier_rat_point lFirstP, lPrevP;
  int i = 0;

  for (auto cit = aBP.curves_begin(); cit != aBP.curves_end(); ++cit, ++i) {
    Bezier_rat_point_vector lQ;
    CGAL::Qt::Bezier_helper::clip(*cit, lQ);
    out_file << "   " << lQ.size() << std::endl;

    if (i == 0)
      lFirstP = lQ.front();

    if (i == lc)
      lQ.back() = lFirstP;

    for (auto pit = lQ.begin(); pit != lQ.end(); ++pit) {
      Bezier_rat_point lP = pit == lQ.begin() && i > 0 ? lPrevP : *pit;
      out_file << "    " << CGAL::to_double(lP.x()) << " " << CGAL::to_double(lP.y()) << std::endl;
      lPrevP = lP;
    }
  }
}

void MainWindow::save_draw_bucket() {
  if (m_curve_type == Curve_type::Conic) {
    show_error("Saving conic polygon sets is not supported yet.");
    return;
  }
  const auto& selected = family(m_curve_type);
  QString name = capitalized(selected.m_name);
  QString file_name = QFileDialog::getSaveFileName(this, "Save " + name + " Polygon Set",
                                                   QString("/data/index") + selected.m_extension,
                                                   QString("%1 Curve files (*%2)").arg(name, selected.m_extension));
  if (file_name.isEmpty()) return;
  auto& bucket = states_stack.back().set(m_color_active);
  bool saved = false;
  switch (m_curve_type) {
   case Curve_type::Linear: saved = save_polygons(file_name, bucket.linear(), write_linear_boundary); break;
   case Curve_type::Circular: saved = save_polygons(file_name, bucket.circular(), write_circular_boundary); break;
   case Curve_type::Bezier: saved = save_polygons(file_name, bucket.bezier(), write_bezier_boundary); break;
   case Curve_type::Polyline: saved = save_polygons(file_name, bucket.polyline(), write_polyline_boundary); break;
   default: break;
  }
  if (! saved) show_error("Cannot save " + std::string(selected.m_name) + " polygon set.");
}

void MainWindow::switch_set_type(Curve_set& aSet, Curve_type aType) {
  aSet.reset_type(aType);
  model_changed();
}

void MainWindow::switch_sets_type(Curve_type aType) {
  for (std::size_t i = 0; i <= number_of_buckets; ++i) switch_set_type(states_stack.back().set(i), aType);
}

void MainWindow::open(QString fileName) {
  if (fileName.isEmpty()) return;
  for (const auto& family : m_families) {
    if (! family.m_extension || ! fileName.endsWith(family.m_extension)) continue;
    if (ensure_curve_type(family.m_type) && (this->*family.m_read)(fileName)) {
      model_changed();
      this->addToRecentFiles(fileName);
    }
    return;
  }
}

void MainWindow::apply_complement(bool /* aChecked */) {
  if (! actionComplementH->isChecked()) return;
  auto operands = checked_buckets(m_complement_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionComplementH, is_operand)) return;
  uncheck_operations_except(actionComplementH);
  run_operation(Bucket_op::Complement, operands, actionComplementH);
}

void MainWindow::apply_intersection(bool /* aChecked */) {
  if (! actionIntersectionH->isChecked()) return;
  auto operands = checked_buckets(m_intersection_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionIntersectionH, is_operand)) return;
  uncheck_operations_except(actionIntersectionH);
  run_operation(Bucket_op::Intersection, operands, actionIntersectionH);
}

void MainWindow::apply_difference(bool /* aChecked */) {
  if (! actionDifferenceH->isChecked()) return;
  auto operands = checked_buckets(m_difference_column);
  if (operands.size() != 2) {
    actionDifferenceH->setChecked(false);
    ask_user_ok("Operation Error", "The difference takes exactly two buckets.\n");
    return;
  }
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionDifferenceH, is_operand)) return;
  uncheck_operations_except(actionDifferenceH);
  run_operation(Bucket_op::Difference, operands, actionDifferenceH);
}

void MainWindow::apply_symmetric_difference(bool /* aChecked */) {
  if (! actionSymmetric_DifferenceH->isChecked()) return;
  auto operands = checked_buckets(m_symmetric_difference_column);
  if (operands.size() != 2) {
    actionSymmetric_DifferenceH->setChecked(false);
    ask_user_ok("Operation Error", "The symmetric difference takes exactly two buckets.\n");
    return;
  }
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionSymmetric_DifferenceH, is_operand)) return;
  uncheck_operations_except(actionSymmetric_DifferenceH);
  run_operation(Bucket_op::Symmetric_difference, operands, actionSymmetric_DifferenceH);
}

void MainWindow::apply_union(bool /* aChecked */) {
  if (! actionUnionH->isChecked()) return;
  auto operands = checked_buckets(m_union_column);
  bool is_operand = std::find(operands.begin(), operands.end(), m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionUnionH, is_operand)) return;
  uncheck_operations_except(actionUnionH);
  run_operation(Bucket_op::Join, operands, actionUnionH);
}

void MainWindow::apply_copy(bool aChecked) {
  if (actionCopyH->isChecked()) {
    auto source = checked_bucket(m_copy_column);
    if (abort_for_draw_bucket(actionCopyH, source == m_color_active)) return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    actionUnionH->setChecked(false);
    actionIntersectionH->setChecked(false);
    actionDifferenceH->setChecked(false);
    actionSymmetric_DifferenceH->setChecked(false);
    actionMinkowski_SumH->setChecked(false);
    actionMoveH->setChecked(false);

    get_new_state(State_op::Copy);
    if (source)
      log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), bucket_name(*source))});


    states_stack.back().clear(result_index);

    if (! source) show_warning("Please select a Bucket!!!");
    else if (! states_stack.back().set(*source).is_empty())
      states_stack.back().result_set().assign(states_stack.back().set(*source));

    actionCopyH->setChecked(false);
    lDone = true;
    if (! states_stack.back().result_set().is_empty())
      states_stack.back().set(m_color_active).join(states_stack.back().result_set());
    states_stack.back().clear(result_index);
    this->setCursor(old);
    if (lDone) model_changed();
  }
}

void MainWindow::apply_move(bool aChecked) {
  if (actionMoveH->isChecked()) {
    auto source = checked_bucket(m_move_column);
    if (abort_for_draw_bucket(actionMoveH, source == m_color_active)) return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    actionUnionH->setChecked(false);
    actionIntersectionH->setChecked(false);
    actionDifferenceH->setChecked(false);
    actionSymmetric_DifferenceH->setChecked(false);
    actionMinkowski_SumH->setChecked(false);
    actionCopyH->setChecked(false);

    get_new_state(State_op::Move);
    if (source)
      log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), bucket_name(*source)),
                      QString("%1.clear()").arg(bucket_name(*source))});


    states_stack.back().clear(result_index);

    if (! source) show_warning("Please select a Bucket!!!");
    else if (! states_stack.back().set(*source).is_empty()) {
      auto& source_set = states_stack.back().set(*source);
      states_stack.back().result_set().assign(source_set);
      states_stack.back().set(m_color_active).join(states_stack.back().result_set());
      source_set.clear();
    }

    actionMoveH->setChecked(false);
    states_stack.back().clear(result_index);

    lDone = true;
    this->setCursor(old);
    if (lDone) model_changed();
  }
}

Polygon_with_holes_2 MainWindow::get_mink_input_polygon(std::size_t aIndex) {
  const auto& lps = states_stack.back().set(aIndex).linear();
  if (lps.number_of_polygons_with_holes() > 1) m_disjoint = true;
  if (m_disjoint) return Polygon_with_holes_2();

  // Converts a boundary to a polygon of a given orientation.
  auto to_polygon = [](const Linear_polygon& aBoundary, CGAL::Orientation aOrientation) {
    Polygon_2 pgn;
    for (auto cit = aBoundary.curves_begin(); cit != aBoundary.curves_end(); ++cit)
      pgn.push_back(Kernel::Point_2(CGAL::to_double(cit->source().x()), CGAL::to_double(cit->source().y())));
    if (pgn.orientation() == CGAL::opposite(aOrientation)) pgn.reverse_orientation();
    return pgn;
  };
  std::vector<Linear_polygon_with_holes> pwhs;
  lps.polygons_with_holes(std::back_inserter(pwhs));
  const auto& pwh = pwhs.front();
  std::vector<Polygon_2> holes;
  for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) holes.push_back(to_polygon(*hit, CGAL::CLOCKWISE));
  return Polygon_with_holes_2(to_polygon(pwh.outer_boundary(), CGAL::COUNTERCLOCKWISE), holes.begin(), holes.end());
}

void MainWindow::apply_minkowski_sum(bool aChecked) {
  if (actionMinkowski_SumH->isChecked()) {
    if (abort_for_draw_bucket(actionMinkowski_SumH,
                              draw_row_checked(m_minkowski_sum_column)))
      return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    if (m_curve_type == Curve_type::Linear) {
      actionComplementH->setChecked(false);
      actionUnionH->setChecked(false);
      actionIntersectionH->setChecked(false);
      actionDifferenceH->setChecked(false);
      actionSymmetric_DifferenceH->setChecked(false);

      get_new_state(State_op::Minkowski_sum);
      log_statements({"# Minkowski sum (not supported in the program yet)"});

      // The two checked buckets, in order.
      auto operands = checked_buckets(m_minkowski_sum_column);

      if (operands.size() == 2) {
        Polygon_with_holes_2 p0, p1;
        if (! states_stack.back().set(operands[0]).is_empty()) {
          p0 = get_mink_input_polygon(operands[0]);
          if (! states_stack.back().set(operands[1]).is_empty()) p1 = get_mink_input_polygon(operands[1]);
        }

        if (! m_disjoint) {
          Polygon_with_holes_2 mink_sum_res = CGAL::minkowski_sum_2(p0, p1);
          if (! mink_sum_res.is_unbounded()) {
            m_linear_input->m_is_mink = true;
            m_linear_input->get_minkowski_result(mink_sum_res, p0);
            m_linear_input->m_hole = true;
            m_linear_input->get_minkowski_holes(mink_sum_res, p0);
            states_stack.back().result_set().clear();
            m_linear_input->m_hole = false;
            m_linear_input->m_is_mink = false;
          }
          else ask_user_ok("Minkowski Sum Operation Error", "resultant polygon is unbounded\n");
          lDone = true;
          minkowski_sum_operated = true;
        }
        else {
          undo();
          show_error("DISJOINT POLYGON SET ERROR\n\nCannot perform Minkowski Sum operation since Input Bucket contains "
                     "more than disjoint polygons!!!");
          m_disjoint = false;
        }
      }
      else {
        ask_user_ok("Minkowski Sum Operation Error", "Function supports 2 polygon as input\n");
      }
    }

    actionMinkowski_SumH->setChecked(false);
    this->setCursor(old);
    if (lDone) {
      model_changed();
    }
  }
}

void MainWindow::toggle_axes() {
  if (! m_grid) {
    m_scene.addItem(xAxis.get());
    m_scene.addItem(yAxis.get());
    m_grid = true;
  }
  else {
    m_scene.removeItem(xAxis.get());
    m_scene.removeItem(yAxis.get());
    m_grid = false;
  }
  // Keep the button checked (highlighted) while the axes are shown.
  actionAxis->setChecked(m_grid);
  m_scene.update();
  model_changed();
}

void MainWindow::toggle_pan() {
  if (! m_pan) {
    remove_inputs();
    family(m_curve_type).m_insert_action->setChecked(false);
    this->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    this->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_scene.update();
    m_pan = true;
    model_changed();
  }
  else {
    family(m_curve_type).m_insert_action->setChecked(true);
    m_pan = false;
    model_changed();
  }
}

void MainWindow::wheelEvent(QWheelEvent* event) {

  this->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
  static const double scaleFactor = 1.15;
  static double currentScale = 1.0;  // stores the current scale value.
  static const double scaleMin = .1; // defines the min scale limit.

  if (m_scene.width() >= 1500000 && event->angleDelta().y() > 0) {
    this->graphicsView->setDragMode(QGraphicsView::NoDrag);
    this->graphicsView->setSceneRect(-1500000, -1000000, 3100000, 2000000);
    this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  }
  if (m_scene.width() <= 1500000) {
    if (event->angleDelta().y() > 0) {
      m_scene.setSceneRect(-m_scene.sceneRect().width() * scaleFactor / 2,
                           -m_scene.sceneRect().height() * scaleFactor / 2, m_scene.sceneRect().width() * scaleFactor,
                           m_scene.sceneRect().height() * scaleFactor);
      this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
      this->graphicsView->scale(2.5, 2.5);
    }
    else if (currentScale > scaleMin) {
      m_scene.setSceneRect(-m_scene.sceneRect().width() / (scaleFactor * 2),
                           -m_scene.sceneRect().height() / (scaleFactor * 2), m_scene.sceneRect().width() / scaleFactor,
                           m_scene.sceneRect().height() / scaleFactor);
      this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
      this->graphicsView->scale(2.5, 2.5);
    }
  }
  model_changed();
}

QIcon MainWindow::curve_tool_icon(const QString& aKind) {
  const qreal ratio = 2;
  QPixmap pixmap(QSize(24, 24) * ratio);
  pixmap.setDevicePixelRatio(ratio);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  const QColor green(6, 131, 2);
  painter.setPen(QPen(green, 2));
  painter.setBrush(Qt::NoBrush);
  // Full curves are filled; the curves that bound a polygon show two vertices.
  if (aKind.startsWith("full")) painter.setBrush(QColor(0, 255, 1, 140));
  QList<QPointF> vertices;
  if (aKind == "arcs") {
    QPainterPath path(QPointF(3, 20));
    path.lineTo(7, 9);
    path.arcTo(QRectF(7, 2, 14, 14), 180, -180);
    path.lineTo(17, 20);
    painter.drawPath(path);
    vertices << QPointF(3, 20) << QPointF(7, 9) << QPointF(21, 9)
             << QPointF(17, 20);
  }
  else if (aKind == "line") {
    painter.drawLine(QPointF(2, 20), QPointF(22, 4));
    vertices << QPointF(7, 16) << QPointF(17, 8);
  }
  else if ((aKind == "circle") || (aKind == "full circle")) {
    painter.drawEllipse(QPointF(12, 12), 9, 9);
    if (aKind == "circle") vertices << QPointF(3, 12) << QPointF(18.4, 5.6);
    else {
      painter.setPen(QPen(QColor(64, 64, 64), 1.5, Qt::DashLine));
      painter.drawLine(QPointF(12, 12), QPointF(18.4, 5.6));
      vertices << QPointF(12, 12);
    }
  }
  else if ((aKind == "ellipse") || (aKind == "full ellipse")) {
    painter.translate(12, 12);
    painter.rotate(-30);
    painter.drawEllipse(QPointF(0, 0), 10.5, 6);
    painter.resetTransform();
    if (aKind == "ellipse") vertices << QPointF(3, 17) << QPointF(21, 7);
  }
  else if (aKind == "coefficients") {
    QFont font = painter.font();
    font.setPixelSize(11);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(64, 64, 64));
    painter.drawText(QRectF(0, 0, 24, 24), Qt::AlignCenter, "ax²");
  }
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(255, 7, 0));
  for (const auto& p : vertices) painter.drawEllipse(p, 2, 2);
  return QIcon(pixmap);
}

QObject* MainWindow::current_input() const { return family(m_curve_type).m_input; }

void MainWindow::remove_inputs() {
  for (const auto& family : m_families) {
    m_scene.removeEventFilter(family.m_input);
    family.m_reset_input();
  }
}

void MainWindow::install_input(QObject* aInput) {
  for (const auto& family : m_families) m_scene.removeEventFilter(family.m_input);
  m_scene.installEventFilter(aInput);
}

void MainWindow::set_input_pen(const QPen& aPen) {
  m_linear_input->mOngoingPieceGI->set_pen(aPen);
  m_linear_input->mLinearGI->set_pen(aPen);
  m_linear_input->mHandleGI->setPen(aPen);
  m_circular_input->mOngoingPieceGI->set_pen(aPen);
  m_circular_input->mCircularGI->set_pen(aPen);
  m_circular_input->mHandleGI->setPen(aPen);
  m_bezier_input->mOngoingPieceGI->set_pen(aPen);
  m_bezier_input->mBezierGI->set_pen(aPen);
  m_bezier_input->mHandle0GI->setPen(aPen);
  m_bezier_input->mHandle1GI->setPen(aPen);
  m_polyline_input->mOngoingPieceGI->set_pen(aPen);
  m_polyline_input->mPolylineGI->set_pen(aPen);
  m_polyline_input->mHandleGI->setPen(aPen);
  m_conic_input->set_pen(aPen);
}

bool MainWindow::ensure_curve_type(Curve_type aType) {
  if (m_curve_type == aType) return true;
  QString name = family(aType).m_name;
  if (! are_all_sets_empty() &&
      ! ask_user_yesno(capitalized(name) + " mode switch",
                       QString("You are about to construct or load %1 polygons, but there are polygons of another "
                               "family already loaded.\n"
                               "Both families are not interoperable. In order to proceed, the polygons must be removed "
                               "first.\n"
                               "Once deleted you cannot undo the action.\n"
                               "Yes to remove and proceed?\n").arg(name)))
    return false;
  switch_sets_type(aType);
  for (const auto& other : m_families) {
    if (other.m_type != aType) other.m_reset_input();
  }
  m_curve_type = aType;
  return true;
}

void MainWindow::start_inserting(Curve_type aType) {
  const auto& selected = family(aType);
  this->graphicsView->setDragMode(QGraphicsView::NoDrag);
  if (! ensure_curve_type(aType)) {
    selected.m_insert_action->setChecked(false);
    return;
  }
  if (! m_pan && ! m_reset) {
    // Switching the family discards the history; the first state gets empty sets of the family.
    while (m_state_num > 1) undo();
    if (states_stack.back().set(0).type() != aType) switch_sets_type(aType);
  }
  actionPAN->setChecked(false);
  m_pan = false;
  for (const auto& other : m_families) {
    if (other.m_open_action) other.m_open_action->setEnabled(other.m_type == aType);
    if (other.m_type != aType) other.m_insert_action->setChecked(false);
  }
  install_input(selected.m_input);
  delete_result();
  actionMinkowski_SumH->setEnabled(aType == Curve_type::Linear);
  set_input_pen(sPens[m_color_active]);
}

void MainWindow::zoom_to_fit() {
  std::optional<QRectF> lTotalRect;
  for (auto si = states_stack.back().m_curve_sets.begin(); si != states_stack.back().m_curve_sets.end(); ++si) {
    if (! si->is_empty()) {
      QRectF lRect = si->bounding_rect();
      if (lTotalRect) lTotalRect = *lTotalRect | lRect;
      else lTotalRect = lRect;
    }
  }
  if (lTotalRect) {
    this->graphicsView->setSceneRect(*lTotalRect);
    this->graphicsView->fitInView(*lTotalRect, Qt::KeepAspectRatio);
  }
  model_changed();
}

QMessageBox::StandardButton
MainWindow::show_message(QMessageBox::Icon aIcon, const QString& aTitle, const QString& aText, QAction* aToggle,
                         QMessageBox::StandardButtons aButtons) {
  // Declared before the dialog, so that it outlives the check box using it.
  Visible_indicator_style indicator_style;
  QDialog dialog(this);
  dialog.setWindowTitle(aTitle);

  // The same icons QMessageBox uses.
  QStyle::StandardPixmap pixmap = QStyle::SP_MessageBoxInformation;
  switch (aIcon) {
   case QMessageBox::Warning: pixmap = QStyle::SP_MessageBoxWarning; break;
   case QMessageBox::Critical: pixmap = QStyle::SP_MessageBoxCritical; break;
   case QMessageBox::Question: pixmap = QStyle::SP_MessageBoxQuestion; break;
   default: break;
  }
  int icon_size = style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
  auto* icon_label = new QLabel;
  icon_label->setPixmap(style()->standardIcon(pixmap).pixmap(icon_size));
  icon_label->setAlignment(Qt::AlignTop);
  auto* message_row = new QHBoxLayout;
  message_row->addWidget(icon_label);
  message_row->addSpacing(12);
  message_row->addWidget(new QLabel(aText), 1);

  auto* check_box = new QCheckBox("Don't show this message again");
  // Make the check box visible on dark themes, like those of the Operations panel.
  check_box->setStyle(&indicator_style);

  auto* ok_button = new QPushButton("OK");
  QPushButton* cancel_button = aButtons.testFlag(QMessageBox::Cancel) ? new QPushButton("Cancel") : nullptr;
  auto* button_row = new QHBoxLayout;
  if (cancel_button) {
    button_row->addWidget(ok_button);
    button_row->addStretch();
    button_row->addWidget(cancel_button);
  }
  else {
    button_row->addStretch();
    button_row->addWidget(ok_button);
  }
  for (auto* button : {ok_button, cancel_button}) {
    if (! button) continue;
    button->setAutoDefault(false);
    button->setDefault(false);
  }
  QObject::connect(ok_button, &QPushButton::clicked, &dialog, &QDialog::accept);
  if (cancel_button)
    QObject::connect(cancel_button, &QPushButton::clicked, &dialog, &QDialog::reject);

  auto* layout = new QVBoxLayout(&dialog);
  layout->addLayout(message_row);
  layout->addSpacing(6);
  layout->addWidget(check_box);
  layout->addSpacing(6);
  layout->addLayout(button_row);
  // Fit the dialog to its content (no clipped text, not resizable).
  layout->setSizeConstraint(QLayout::SetFixedSize);

  // Center the dialog over the canvas. (Under native Wayland the compositor
  // places dialogs and ignores the requested position.)
  dialog.adjustSize();
  QWidget* canvas = graphicsView->viewport();
  QPoint center = canvas->mapToGlobal(canvas->rect().center());
  dialog.move(center - dialog.rect().center());

  bool accepted = (dialog.exec() == QDialog::Accepted);
  if (check_box->isChecked()) aToggle->setChecked(false);
  return (cancel_button && ! accepted) ? QMessageBox::Cancel : QMessageBox::Ok;
}

bool MainWindow::show_not_empty_warning() {
  return show_message(QMessageBox::Information, "Warning: Bucket Not Empty", "The Draw/Insert bucket is not empty. "
                      "Add the result to it?", actionWarnNotEmpty, QMessageBox::Ok | QMessageBox::Cancel) ==
    QMessageBox::Ok;
}

void MainWindow::show_operand_error() {
  show_message(QMessageBox::Critical, "Error: Operation Aborted", "The result cannot be stored in a non-empty operand.",
               actionErrorOperand);
}

void MainWindow::
limit_to_all_but_one(const Column& aColumn, QAbstractButton* aBox, bool aChecked,
                     QAbstractButton* aOperation) {
  if (aChecked && ! aBox->isHidden()) {
    bool all_checked = std::all_of(aColumn.begin(), aColumn.end(), [](QAbstractButton* box) {
      return box->isHidden() || box->isChecked();
    });
    if (all_checked) {
      aBox->setChecked(false);  // toggles again, and updates the operation
      return;
    }
  }
  bool any_checked = std::any_of(aColumn.begin(), aColumn.end(), [](QAbstractButton* box) {
    return ! box->isHidden() && box->isChecked();
  });
  aOperation->setEnabled(any_checked);
}

void MainWindow::
keep_one_unchecked(const Column& aColumn) {
  QAbstractButton* last = nullptr;
  for (auto* box : aColumn) {
    if (box->isHidden()) continue;
    if (! box->isChecked()) return;
    last = box;
  }
  if (last) last->setChecked(false);
}

void MainWindow::
limit_to_two_operands(const Column& aColumn, QAbstractButton* aBox, bool aChecked,
                      QAbstractButton* aOperation) {
  auto count = std::count_if(aColumn.begin(), aColumn.end(), [](QAbstractButton* box) { return box->isChecked(); });
  if (aChecked && (count > 2)) {
    aBox->setChecked(false);    // toggles again, and updates the operation
    return;
  }
  aOperation->setEnabled(count == 2);
}

std::vector<std::size_t> MainWindow::
checked_buckets(const Column& aColumn) const {
  std::vector<std::size_t> buckets;
  for (std::size_t i = 0; i < aColumn.size(); ++i) {
    if (! aColumn[i]->isHidden() && aColumn[i]->isChecked())
      buckets.push_back(i);
  }
  return buckets;
}

void MainWindow::uncheck_operations_except(QAbstractButton* aOperation) {
  for (QAbstractButton* button : {static_cast<QAbstractButton*>(actionComplementH),
          static_cast<QAbstractButton*>(actionIntersectionH), static_cast<QAbstractButton*>(actionUnionH),
          static_cast<QAbstractButton*>(actionDifferenceH), static_cast<QAbstractButton*>(actionSymmetric_DifferenceH),
          static_cast<QAbstractButton*>(actionMinkowski_SumH), static_cast<QAbstractButton*>(actionCopyH),
          static_cast<QAbstractButton*>(actionMoveH)}) {
    if (button != aOperation) button->setChecked(false);
  }
}

void MainWindow::clear_result() {
  auto& state = states_stack.back();
  state.clear(result_index);
}

void MainWindow::run_operation(Bucket_op aOp, const std::vector<std::size_t>& aOperands, QAbstractButton* aOperation) {
  if (aOperands.empty() || ((aOp == Bucket_op::Complement) && (aOperands.size() != 1))) {
    aOperation->setChecked(false);
    return;
  }

  State_op code = State_op::Complement;
  switch (aOp) {
   case Bucket_op::Complement: code = State_op::Complement; break;
   case Bucket_op::Intersection: code = State_op::Intersection; break;
   case Bucket_op::Join: code = State_op::Union; break;
   case Bucket_op::Difference: code = State_op::Difference; break;
   case Bucket_op::Symmetric_difference: code = State_op::Symmetric_difference; break;
  }
  const QString name = method_name(aOp);

  QCursor old = this->cursor();
  this->setCursor(Qt::WaitCursor);
  get_new_state(code);
  clear_result();
  auto& state = states_stack.back();
  auto bucket = [&state](std::size_t aIndex) -> Curve_set& { return state.set(aIndex); };
  Curve_set& draw_bucket = state.set(m_color_active);
  bool direct = draw_bucket.is_empty() && (std::find(aOperands.begin(), aOperands.end(), m_color_active) ==
     aOperands.end());
  Curve_set& result = direct ? draw_bucket : state.result_set();
  const QString target = direct ? bucket_name(m_color_active) : "result";
  QStringList program;

  if (aOp == Bucket_op::Complement) {
    result.complement(bucket(aOperands[0]));
    if (! direct) program << "result = Polygon_set_2()";
    program << QString("%1.complement(%2)")
      .arg(target, bucket_name(aOperands[0]));
  }
  else if (aOperands.size() == 1) {
    if (direct) {
      result.join(bucket(aOperands[0]));
      program << QString("%1.join(%2)").arg(target, bucket_name(aOperands[0]));
    }
    else {
      result.assign(bucket(aOperands[0]));
      program << QString("result = Polygon_set_2(%1)")
        .arg(bucket_name(aOperands[0]));
    }
  }
  else {
    apply(aOp, result, bucket(aOperands[0]), bucket(aOperands[1]));
    if (! direct) program << "result = Polygon_set_2()";
    program << QString("%1.%2(%3, %4)").arg(target, name, bucket_name(aOperands[0]), bucket_name(aOperands[1]));
    for (std::size_t i = 2; i < aOperands.size(); ++i) {
      apply(aOp, result, bucket(aOperands[i]));
      program << QString("%1.%2(%3)").arg(target, name, bucket_name(aOperands[i]));
    }
  }

  if (! direct) {
    draw_bucket.join(result);
    program << QString("%1.join(result)").arg(bucket_name(m_color_active));
  }
  clear_result();

  aOperation->setChecked(false);
  this->setCursor(old);
  log_statements(program);
  model_changed();
}

void MainWindow::log_statements(const QStringList& aStatements, bool aSticky) {
  if (! m_logging) return;
  for (const auto& statement : aStatements) {
    m_program.push_back({aSticky ? 0 : states_stack.size(), statement});
    if (m_log) m_log->appendPlainText(statement);
  }
}

void MainWindow::trim_program_to_states() {
  auto it = std::remove_if(m_program.begin(), m_program.end(), [this](const Program_line& aLine) {
    return aLine.m_state_count > states_stack.size();
  });
  bool changed = (it != m_program.end());
  m_program.erase(it, m_program.end());
  if (! changed || ! m_log) return;
  QStringList lines;
  for (const auto& line : m_program) lines << line.m_text;
  m_log->setPlainText(lines.join("\n"));
}

bool MainWindow::bucket_exists(std::size_t aIndex) const { return aIndex < m_number_of_buckets; }

std::size_t MainWindow::bucket_index(const QString& aName) const {
  for (std::size_t i = 0; i < number_of_buckets; ++i) {
    if (aName == bucket_name(i)) return i;
  }
  return number_of_buckets;
}

Curve_set* MainWindow::polygon_set(const QString& aName) {
  std::size_t i = bucket_index(aName);
  if (i < number_of_buckets) return bucket_exists(i) ? &states_stack.back().set(i) : nullptr;
  auto it = m_variables.find(aName);
  return (it == m_variables.end()) ? nullptr : &it->second;
}

Linear_polygon_with_holes MainWindow::polygon(const Py_expr& aExpr) const {
  const auto& pts = aExpr.m_points;
  if (pts.size() < 3)
    throw std::runtime_error("a polygon needs at least 3 vertices");
  CGAL::Polygon_2<Kernel> check;
  for (const auto& [x, y] : pts) check.push_back(Kernel::Point_2(x, y));
  if (! check.is_simple())
    throw std::runtime_error("the polygon is not simple");
  if (check.orientation() != CGAL::COUNTERCLOCKWISE)
    throw std::runtime_error("the polygon is not counterclockwise oriented");
  std::vector<Linear_curve> curves;
  for (std::size_t i = 0; i < pts.size(); ++i) {
    const auto& p = pts[i];
    const auto& q = pts[(i + 1) % pts.size()];
    curves.emplace_back(Kernel::Point_2(p.first, p.second), Kernel::Point_2(q.first, q.second));
  }
  return Linear_polygon_with_holes(Linear_polygon(curves.begin(), curves.end()));
}

QString MainWindow::execute(const Py_statement& aStatement) {
  switch (aStatement.m_kind) {
   case Py_statement::Kind::Delete: execute_delete(aStatement); return QString();
   case Py_statement::Kind::Assign: execute_assign(aStatement); return QString();
   case Py_statement::Kind::Call: break;
  }
  return execute_call(aStatement);
}

Curve_set& MainWindow::existing_set(const QString& aName) {
  Curve_set* set = polygon_set(aName);
  if (! set) throw std::runtime_error(("name '" + aName + "' is not defined").toStdString());
  return *set;
}

Curve_set& MainWindow::named_set(const Py_expr& aExpr) {
  if (aExpr.m_kind != Py_expr::Kind::Name)
    throw std::runtime_error(("expected a polygon set name instead of " + to_python(aExpr)).toStdString());
  return existing_set(aExpr.m_name);
}

void MainWindow::execute_delete(const Py_statement& aStatement) {
  const QString& target = aStatement.m_target;
  std::size_t index = bucket_index(target);
  if (index == number_of_buckets) {
    if (m_variables.erase(target) == 0)
      throw std::runtime_error(("name '" + target + "' is not defined").toStdString());
    log_statements({to_python(aStatement)}, true);
    return;
  }
  // Only the last added bucket can be removed.
  std::size_t last = m_number_of_buckets - 1;
  if ((index < min_number_of_buckets) || (index != last))
    throw std::runtime_error(index < min_number_of_buckets ? "the red, black, and blue buckets cannot be deleted" :
      ("only the last bucket (" + QString(bucket_name(last)) + ") can be deleted").toStdString());
  remove_bucket();      // logs "del <name>"
}

void MainWindow::execute_assign(const Py_statement& aStatement) {
  const QString& target = aStatement.m_target;
  std::size_t index = bucket_index(target);
  const Py_expr& value = aStatement.m_args[0];
  if (value.m_kind != Py_expr::Kind::Polygon_set)
    throw std::runtime_error("only polygon sets can be assigned, e.g., " + (target + " = Polygon_set_2(" +
       (value.m_kind == Py_expr::Kind::Name ? value.m_name : QString()) + ")").toStdString() + " (copies the set)");
  const Py_expr* arg = value.m_arg.get();
  if ((index < number_of_buckets) && ! bucket_exists(index)) {
    // Buckets are added in order.
    std::size_t next = m_number_of_buckets;
    if (index != next)
      throw std::runtime_error(("buckets are added in order; the next one is " + QString(bucket_name(next)))
                               .toStdString());
    m_logging = false;
    add_bucket();
    m_logging = true;
    if (! arg) {       // a new, empty bucket
      log_statements({to_python(aStatement)}, true);
      return;
    }
    log_statements({target + " = Polygon_set_2()"}, true);
  }
  get_new_state(State_op::Console);
  Curve_set* set = polygon_set(target);
  if (! set) set = &m_variables.emplace(target, Curve_set(Curve_type::Linear, QPen(), QBrush())).first->second;
  if (! arg) set->clear();
  else if (arg->m_kind == Py_expr::Kind::Polygon) {
    Linear_polygon_with_holes pgn = polygon(*arg);
    set->clear();
    set->linear().join(pgn);
  }
  else {
    Curve_set& other = named_set(*arg);
    if (&other != set) set->assign(other);
  }
  log_statements({to_python(aStatement)});
}

QString MainWindow::execute_call(const Py_statement& aStatement) {
  const QString& target = aStatement.m_target;
  const QString& method = aStatement.m_method;
  const auto& args = aStatement.m_args;
  Curve_set* set = &existing_set(target);
  auto arity = [&](std::size_t aMin, std::size_t aMax) {
    if ((args.size() < aMin) || (args.size() > aMax))
      throw std::runtime_error((method + "() takes " + ((aMin == aMax) ? QString::number(aMin) :
         QString("%1 or %2").arg(aMin).arg(aMax)) + " argument(s)").toStdString());
  };

  // Queries.
  if (method == "is_empty") {
    arity(0, 0);
    return set->is_empty() ? "True" : "False";
  }
  if (method == "is_plane") {
    arity(0, 0);
    return set->linear().is_plane() ? "True" : "False";
  }
  if (method == "number_of_polygons_with_holes") {
    arity(0, 0);
    return QString::number(set->linear().number_of_polygons_with_holes());
  }

  // Actions; each one is undoable, so the sets are obtained again after the new state is pushed.
  if (method == "clear") {
    arity(0, 0);
    get_new_state(State_op::Console);
    existing_set(target).clear();
  }
  else if (method == "complement") {
    arity(0, 1);
    if (! args.empty() && (&named_set(args[0]) == set))
      throw std::runtime_error("use complement() to complement in place");
    get_new_state(State_op::Console);
    if (args.empty()) existing_set(target).complement();
    else existing_set(target).complement(named_set(args[0]));
  }
  else {
    std::optional<Bucket_op> op;
    for (auto candidate : {Bucket_op::Intersection, Bucket_op::Join, Bucket_op::Difference,
                           Bucket_op::Symmetric_difference}) {
      if (method == method_name(candidate)) op = candidate;
    }
    if (! op) throw std::runtime_error(("unknown method '" + method + "'").toStdString());
    arity(1, 2);
    if (args.size() == 2) {
      // The bivariate form overwrites the target, which must not be an operand.
      if ((&named_set(args[0]) == set) || (&named_set(args[1]) == set))
        throw std::runtime_error(("the target of " + method + "(a, b) must differ from a and b").toStdString());
      get_new_state(State_op::Console);
      apply(*op, existing_set(target), named_set(args[0]), named_set(args[1]));
    }
    else if (args[0].m_kind == Py_expr::Kind::Polygon) {
      Linear_polygon_with_holes pgn = polygon(args[0]);
      get_new_state(State_op::Console);
      apply(*op, existing_set(target).linear(), pgn);
    }
    else {
      named_set(args[0]);      // check the name
      get_new_state(State_op::Console);
      apply(*op, existing_set(target), named_set(args[0]));
    }
  }
  log_statements({to_python(aStatement)});
  return QString();
}

bool MainWindow::eventFilter(QObject* aObject, QEvent* aEvent) {
  if ((aObject == graphicsView->viewport()) && (aEvent->type() == QEvent::Paint))
    update_unbounded_rect();
  if ((aObject == m_console_input) && m_console_input) {
    if (aEvent->type() == QEvent::ShortcutOverride) {
      auto* ke = static_cast<QKeyEvent*>(aEvent);
      if (ke->modifiers().testFlag(Qt::ControlModifier) && (ke->key() != Qt::Key_Q)) {
        aEvent->accept();
        return true;
      }
    }
    if (aEvent->type() == QEvent::KeyPress) {
      auto* ke = static_cast<QKeyEvent*>(aEvent);
      if ((ke->key() == Qt::Key_Up) || (ke->key() == Qt::Key_Down)) {
        int n = m_console_history.size();
        int pos = m_console_history_pos + ((ke->key() == Qt::Key_Up) ? -1 : 1);
        if ((pos >= 0) && (pos <= n)) {
          m_console_history_pos = pos;
          m_console_input->setText((pos < n) ? m_console_history[pos] : QString());
        }
        return true;
      }
    }
  }
  return CGAL::Qt::DemosMainWindow::eventFilter(aObject, aEvent);
}

void MainWindow::update_unbounded_rect() {
  QRectF rect = graphicsView->mapToScene(graphicsView->viewport()->rect()).
    boundingRect();
  double margin = 0.05 * std::max(rect.width(), rect.height());
  rect.adjust(-margin, -margin, margin, margin);
  QRectF& linear_rect = CGAL::Qt::Piecewise_graphics_item_base::unbounded_rect();
  if ((rect == linear_rect) || states_stack.empty()) return;
  linear_rect = rect;
  CGAL::Qt::Piecewise_graphics_item_base_bezier::unbounded_rect() = rect;
  for (auto& curve_set : states_stack.back().m_curve_sets)
    curve_set.unbounded_rect_changed();
}

void MainWindow::console_print(const QString& aText) { if (m_console_output) m_console_output->appendPlainText(aText); }

void MainWindow::run_console_line(const QString& aLine) {
  console_print(">>> " + aLine);
  if (! aLine.trimmed().isEmpty()) {
    m_console_history << aLine;
  }
  m_console_history_pos = m_console_history.size();
  if (m_curve_type != Curve_type::Linear) {
    console_print("Error: the console supports linear polygons only");
    return;
  }
  try {
    auto statements = Py_parser(aLine).parse();
    for (const auto& statement : statements) {
      QString output = execute(statement);
      if (! output.isEmpty()) console_print(output);
    }
  }
  catch (std::exception& e) {
    console_print(QString("Error: ") + e.what());
  }
  model_changed();
}

void MainWindow::save_program() {
  QString file_name = QFileDialog::getSaveFileName(this, "Save Program", "bso_program.py", "Python files (*.py)");
  if (file_name.isEmpty()) return;
  QSaveFile file(file_name);
  if (! file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    show_error("Cannot open " + file_name.toStdString() + " for writing");
    return;
  }
  QTextStream out(&file);
  out << "# Generated by the CGAL 2D Boolean Set Operations demo.\n"
      << "from CGALPY_kerEpec_aos2Seg_bso2_pol2 import Ker, Pol2, Bso2\n\n"
      << "Point_2 = Ker.Point_2\n"
      << "Polygon_2 = Pol2.Polygon_2\n"
      << "Polygon_set_2 = Bso2.Polygon_set_2\n\n";
  for (const auto& line : m_program) out << line.m_text << "\n";
  out.flush();
  if (! file.commit())
    show_error("Cannot write " + file_name.toStdString());
}

bool MainWindow::
draw_row_checked(const Column& aColumn) const
{ return (m_color_active < aColumn.size()) && aColumn[m_color_active]->isChecked(); }

bool MainWindow::abort_for_draw_bucket(QAbstractButton* aOperation, bool aIsOperand) {
  if (states_stack.back().set(m_color_active).is_empty()) return false;
  if (aIsOperand) {
    aOperation->setChecked(false);
    if (operand_error_warn) show_operand_error();
    return true;
  }
  if (empty_warn && ! show_not_empty_warning()) {
    aOperation->setChecked(false);
    return true;
  }
  return false;
}

void MainWindow::resizeEvent(QResizeEvent* e) {
  this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  this->graphicsView->scale(2.5, 2.5);
  model_changed();
}

void MainWindow::about_cgal() {
  QString title = "About CGAL";
  QString html_resource_name = "help/about_CGAL.html";

  QFile about_CGAL(html_resource_name);
  about_CGAL.open(QIODevice::ReadOnly);
  QString about_CGAL_txt = QTextStream(&about_CGAL).readAll();
#ifdef CGAL_VERSION_STR
  QString cgal_version(CGAL_VERSION_STR);
#  ifdef CGAL_FAKE_PUBLIC_RELEASE
  cgal_version.replace(QRegExp("-Ic?.*"), "");
#  endif
  about_CGAL_txt.replace("<!--CGAL_VERSION-->", QString(" (version %1)").arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 550px; min-height: 250px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel* mb_label = mb.findChild<QLabel*>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() | ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void MainWindow::about_minkowski_sums() {
  QString title = "About 2D Minkowski sums";
  QString html_resource_name = "help/About2DMinkowskisums.html";
  QFile about_CGAL(html_resource_name);
  about_CGAL.open(QIODevice::ReadOnly);
  QString about_CGAL_txt = QTextStream(&about_CGAL).readAll();
#ifdef CGAL_VERSION_STR
  QString cgal_version(CGAL_VERSION_STR);
#  ifdef CGAL_FAKE_PUBLIC_RELEASE
  cgal_version.replace(QRegExp("-Ic?.*"), "");
#  endif
  about_CGAL_txt.replace("<!--CGAL_VERSION-->", QString(" (version %1)").arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel* mb_label = mb.findChild<QLabel*>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() | ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void MainWindow::about_boolean_set_operations() {

  QString title = "About 2D Minkowski sums";
  QString html_resource_name = "help/About2DRegularizedBooleanOperations.html";
  QFile about_CGAL(html_resource_name);
  about_CGAL.open(QIODevice::ReadOnly);
  QString about_CGAL_txt = QTextStream(&about_CGAL).readAll();
#ifdef CGAL_VERSION_STR
  QString cgal_version(CGAL_VERSION_STR);
#  ifdef CGAL_FAKE_PUBLIC_RELEASE
  cgal_version.replace(QRegExp("-Ic?.*"), "");
#  endif
  about_CGAL_txt.replace("<!--CGAL_VERSION-->", QString(" (version %1)").arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel* mb_label = mb.findChild<QLabel*>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() | ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void MainWindow::about_demo() {
  QString title = "About Demo";
  QString html_resource_name = "help/aboutThisDemo.html";
  QFile about_CGAL(html_resource_name);
  about_CGAL.open(QIODevice::ReadOnly);
  QString about_CGAL_txt = QTextStream(&about_CGAL).readAll();
#ifdef CGAL_VERSION_STR
  QString cgal_version(CGAL_VERSION_STR);
#  ifdef CGAL_FAKE_PUBLIC_RELEASE
  cgal_version.replace(QRegExp("-Ic?.*"), "");
#  endif
  about_CGAL_txt.replace("<!--CGAL_VERSION-->", QString(" (version %1)").arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel* mb_label = mb.findChild<QLabel*>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() | ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

template <typename Set>
void MainWindow::add_constructed_polygon(Curve_type aType, typename Set::Polygon_2 aPolygon, bool aToResult) {
  if (! ensure_curve_type(aType)) return;
  if (aPolygon.orientation() == CGAL::CLOCKWISE) aPolygon.reverse_orientation();
  typename Set::Polygon_with_holes_2 pwh(aPolygon);
  if (aToResult) {
    states_stack.back().result_set().get<Set>().join(pwh);
    return;
  }
  get_new_state(State_op::Draw);
  states_stack.back().set(m_color_active).get<Set>().join(pwh);
}

void MainWindow::process_input(CGAL::Object o) {
  try {
    if (auto* pgn = CGAL::object_cast<Conic_polygon>(&o))
      add_constructed_polygon<Conic_polygon_set>(Curve_type::Conic, *pgn, false);
    else if (auto* bezier = CGAL::object_cast<std::pair<Bezier_polygon, Bezier_boundary_source>>(&o))
      add_constructed_polygon<Bezier_polygon_set>(Curve_type::Bezier, bezier->first,
                                                  m_bezier_input->is_bounding_rect());
    else if (auto* pgn = CGAL::object_cast<Circular_polygon>(&o))
      add_constructed_polygon<Circular_polygon_set>(Curve_type::Circular, *pgn, m_circular_input->is_bounding_rect());
    else if (auto* pgn = CGAL::object_cast<Polyline_polygon>(&o))
      add_constructed_polygon<Polyline_polygon_set>(Curve_type::Polyline, *pgn, m_polyline_input->is_bounding_rect());
    else if (auto* pgn = CGAL::object_cast<Linear_polygon>(&o)) {
      // A linear polygon may also be a hole to subtract, or a part of a Minkowski sum (not an action on its own).
      if (! ensure_curve_type(Curve_type::Linear)) return;
      Linear_polygon lLI = *pgn;
      if (lLI.orientation() == CGAL::CLOCKWISE) lLI.reverse_orientation();
      Linear_polygon_with_holes lCPWH(lLI);
      if (! m_linear_input->is_bounding_rect() && ! m_linear_input->ishole()) {
        if (! m_linear_input->is_mink()) get_new_state(State_op::Draw);
        states_stack.back().set(m_color_active).linear().join(lCPWH);
        if (! m_linear_input->is_mink())
          log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active), to_python(lLI))});
      }
      else {
        states_stack.back().result_set().linear().join(lCPWH);
        if (m_linear_input->ishole()) {
          log_statements({QString("%1.difference(%2)").arg(bucket_name(m_color_active), to_python(lLI))});
          states_stack.back().set(m_color_active).difference(states_stack.back().result_set());
          states_stack.back().result_set().clear();
        }
      }
    }
  }
  catch (const std::exception& e) {
    undo();
  }
  model_changed();
}

#include "Boolean_set_operations_2.moc"
#include <CGAL/Qt/resources.h>

/*! runs the demo.
 */
int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  app.setOrganizationDomain("geometryfactory.com");
  app.setOrganizationName("GeometryFactory");
  app.setApplicationName("Boolean_operations_2 demo");
  CGAL_QT_INIT_RESOURCES;
  try {
    MainWindow w;
    w.show();
    return app.exec();
  }
  catch (const std::exception& e) {
    std::string s = e.what();
    show_error("Exception thrown during run of the program:\n" + s);
  }
}
