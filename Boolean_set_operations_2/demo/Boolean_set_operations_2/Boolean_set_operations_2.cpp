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
// WARRANTY OF describingIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s) : Kabir Kedia<kabirkedia0111@gmail.com>
//             Ronnie Gandhi<ronniegandhi19999@gmail.com>
//             Apurva Bhatt <response2apurva@gmail.com>
//             Efi Fogel <efifogel@gmail.com>

//TODO:- add error handling

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <limits>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <boost/shared_ptr.hpp>

#include <QApplication>
#include <qmessagebox.h>
// #include <QDesktopWidget>

#include <QMainWindow>
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

#include <CGAL/auto_link/Qt.h>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/Converter.h>

#include <CGAL/basic.h>
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

#include "QT5/MinkowskiSum.h"   //for future supports
#include "QT5/Circular_polygons.h"
#include "QT5/Linear_polygons.h"
#include "QT5/BezierCurves.h"
#include "QT5/Polyline_curves.h"
#include "QT5/Graphics_view_circular_polygon_input.h"
#include "QT5/Graphics_view_linear_polygon_input.h"
#include "QT5/Graphics_view_polyline_input.h"
#include "QT5/GraphicsViewBezierPolygonInput.h"
#include "QT5/Graphics_view_minkowski_input.h"
#include "QT5/Conic_Curves.h"
#include "QT5/Graphics_view_conic_input.h"
#include "QT5/General_polygon_2.h"
#include "QT5/General_polygon_set_2.h"
#include "QT5/General_polygon_set_on_surface_2.h"
#include "QT5/Gps_circle_segment_traits_2.h"
#include "QT5/Gps_segment_traits_2.h"
#include "QT5/Gps_traits_2.h"
#include "QT5/connect_holes.h"
#include "QT5/Polygon_set_2.h"

#include "ui_Boolean_set_operations_2.h"

#include "Typedefs.h"

// Forward Declarations
void show_warning(std::string aS);

void show_error(std::string aS);

void error(std::string aS);

void error_handler(char const *what, char const *expr, char const *file,
                   int line, char const *msg);


// Circular_polygon linearPart_2_circ(Circular_Linear_polygon const& pgn);
// Circular_polygon_with_holes linearPart_2_circ(Circular_Linear_polygon_with_holes const& pwh);

// Typedefs
typedef CGAL::Qt::Circular_set_graphics_item <Circular_polygon_set,
Circular_traits> Circular_GI;
typedef CGAL::Qt::Linear_set_graphics_item <Linear_polygon_set,
Linear_traits> Linear_GI;
typedef CGAL::Qt::Polyline_set_graphics_item <Polyline_polygon_set,
Polyline_traits> Polyline_GI;


//typedef CGAL::Qt::Bezier_set_graphics_item<Bezier_polygon_set,
//                                         Bezier_traits>      Bezier_GI;


// Functions to show errors

//show warnings but proceed
void show_warning(std::string aS) { QMessageBox::warning(NULL, "Warning", QString(aS.c_str())); }

//show errors in a message box and don't proceed
void show_error(std::string aS) { QMessageBox::critical(NULL, "Critical Error", QString(aS.c_str())); }

//show errors in a message box and then crash.
// Needs to be replaced by show error or a reset button
void error(std::string aS) {
  show_error(aS);
  throw std::runtime_error(aS);
}

void error_handler(char const *what, char const *expr, char const *file,
                   int line, char const *msg) {
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

enum {
  BLUE_GROUP, RED_GROUP, BLACK_GROUP, BROWN_GROUP, YELLOW_GROUP,
  MAGENTA_GROUP, AQUA_GROUP, RESULT_GROUP
};

//A way to maintain 5 category of polygons namely linear,circular,bezier, polyline(to be added later)
//enum generates errors so, we will use LINEAR_TYPE = 1 ,CIRCULAR_TYPE = 2 ,BEZIER_TYPE = 3 ,POLYLINE_TYPE = 4 and CONIC_TYPE = 5(Not this year)

//drawing tools for different colors
QPen sPens[] = {
  QPen(QColor(255, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //blue
  QPen(QColor(0, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),     //red
  QPen(QColor(0, 0, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //black
  QPen(QColor(210, 105, 30), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),//brown
  QPen(QColor(255, 255, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //yellow
  QPen(QColor(255, 0, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //magenta
  QPen(QColor(0, 255, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //aqua
  QPen(QColor(0, 0, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //Result
};

QBrush sBrushes[] = {
  QBrush(QColor(255, 0, 0, 75)),           //blue
  QBrush(QColor(0, 0, 0, 75)),             //red
  QBrush(QColor(0, 0, 255, 75)),           //black
  QBrush(QColor(210, 105, 30, 75)),        //brown
  QBrush(QColor(255, 255, 0, 75)),         //yellow
  QBrush(QColor(255, 0, 255, 75)),         //magenta
  QBrush(QColor(0, 255, 255, 75)),         //aqua
  QBrush(QColor(0, 0, 0, 0)),              //Result
};

//A base call for rep class
struct Rep_base {
  virtual ~Rep_base() {}

  virtual int type() const = 0;

  virtual CGAL::Qt::GraphicsItem *gi() const = 0;

  virtual CGAL::Qt::GraphicsItem *gi() = 0;

  virtual void set_pen(QPen const &aPen) = 0;

  virtual void set_brush(QBrush const &aBrush) = 0;

  virtual QRectF bounding_rect() const { return gi()->boundingRect(); }

  // The area that unbounded sets are drawn over has changed.
  virtual void unbounded_rect_changed() = 0;

  virtual bool is_empty() const = 0;

  virtual void clear() = 0;

  virtual void complement() = 0;

  virtual void assign(Rep_base const &aOther) = 0;

  virtual void intersect(Rep_base const &aOther) = 0;

  virtual void join(Rep_base const &aOther) = 0;

  virtual void difference(Rep_base const &aOther) = 0;

  virtual void symmetric_difference(Rep_base const &aOther) = 0;

  // Bivariate operations: *this = aFirst op aSecond.
  virtual void complement(Rep_base const &aOther) = 0;

  virtual void intersect(Rep_base const &aFirst, Rep_base const &aSecond) = 0;

  virtual void join(Rep_base const &aFirst, Rep_base const &aSecond) = 0;

  virtual void difference(Rep_base const &aFirst, Rep_base const &aSecond) = 0;

  virtual void symmetric_difference(Rep_base const &aFirst,
                                    Rep_base const &aSecond) = 0;
};

//Class for initializing Rep class.
//Used for linear and circular
template<typename GI_, typename Set_, typename Gps_traits>
class Rep : public Rep_base {
public:
  typedef GI_ GI;
  typedef Set_ Set;

  typedef Rep<GI, Set, Gps_traits> Self;

  Rep() { m_GI = new GI(&m_set, m_traits); }

  Set const &set() const { return m_set; }

  Set &set() { return m_set; }

  virtual CGAL::Qt::GraphicsItem *gi() const { return m_GI; }

  virtual CGAL::Qt::GraphicsItem *gi() { return m_GI; }

  virtual QRectF bounding_rect() const { return m_GI->curves_rect(); }

  virtual void unbounded_rect_changed()
  { if (m_GI->is_unbounded()) m_GI->modelChanged(); }

  virtual void set_pen(QPen const &aPen) { m_GI->setPen(aPen); }

  virtual void set_brush(QBrush const &aBrush) { m_GI->setBrush(aBrush); }

  virtual bool is_empty() const { return m_set.is_empty(); }

  //virtual functions for error handling of every operation
  virtual void clear() {
    try {
      m_set.clear();
    }
    catch (...) {
      show_error("Exception thrown during boolean operation clear");
    }
  }

  virtual void complement() {
    try {
      m_set.complement();
    }
    catch (...) {
      show_error("Exception thrown during boolean operation complement");
    }
  }

  virtual void assign(Rep_base const &aOther) {
    try {
      m_set = cast(aOther).m_set;
    }
    catch (...) {
      show_error("Exception thrown during boolean operation assign");
    }
  }

  virtual void intersect(Rep_base const &aOther) {
    try {
      m_set.intersection(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation intersect");
    }
  }

  virtual void join(Rep_base const &aOther) {
    try {
      m_set.join(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation union");
    }
  }

  virtual void difference(Rep_base const &aOther) {
    try {
      m_set.difference(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation difference");
    }
  }

  virtual void symmetric_difference(Rep_base const &aOther) {
    try {
      m_set.symmetric_difference(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation symmetric difference");
    }
  }

  virtual void complement(Rep_base const &aOther) {
    try {
      m_set.complement(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation complement");
    }
  }

  virtual void intersect(Rep_base const &aFirst, Rep_base const &aSecond) {
    try {
      m_set.intersection(cast(aFirst).m_set, cast(aSecond).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation intersect");
    }
  }

  virtual void join(Rep_base const &aFirst, Rep_base const &aSecond) {
    try {
      m_set.join(cast(aFirst).m_set, cast(aSecond).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation union");
    }
  }

  virtual void difference(Rep_base const &aFirst, Rep_base const &aSecond) {
    try {
      m_set.difference(cast(aFirst).m_set, cast(aSecond).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation difference");
    }
  }

  virtual void symmetric_difference(Rep_base const &aFirst,
                                    Rep_base const &aSecond) {
    try {
      m_set.symmetric_difference(cast(aFirst).m_set, cast(aSecond).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation symmetric difference");
    }
  }

  static Self const &cast(Rep_base const &aOther) { return dynamic_cast<Self const &>(aOther); }

  static Self &cast(Rep_base &aOther) { return dynamic_cast<Self &>(aOther); }

private:
  //For maintaining all drawing operations
  GI *m_GI;
  //Storage for all polygons of one type. It is used as a base to perform all boolean operations
  Set m_set;
protected:
  //pass it
  Gps_traits m_traits;
};

//Implementing Bezier's rep class
template<class GI_, class Set_>
class Rep_o : public Rep_base {
public:
  typedef GI_ GI;
  typedef Set_ Set;

  typedef Rep_o<GI, Set> Self;

  Rep_o() { m_GI = new GI(&m_set); }

  Set const &set() const { return m_set; }

  Set &set() { return m_set; }

  virtual CGAL::Qt::GraphicsItem *gi() const { return m_GI; }

  virtual CGAL::Qt::GraphicsItem *gi() { return m_GI; }

  virtual QRectF bounding_rect() const { return m_GI->curves_rect(); }

  virtual void unbounded_rect_changed()
  { if (m_GI->is_unbounded()) m_GI->modelChanged(); }

  virtual void set_pen(QPen const &aPen) { m_GI->setPen(aPen); }

  virtual void set_brush(QBrush const &aBrush) { m_GI->setBrush(aBrush); }

  virtual bool is_empty() const { return m_set.is_empty(); }

  virtual void clear() {
    try {
      m_set.clear();
    }
    catch (...) {
      show_error("Exception thrown during boolean operation clear");
    }
  }

  virtual void complement() {
    try {
      m_set.complement();
    }
    catch (...) {
      show_error("Exception thrown during boolean operation complement");
    }
  }

  virtual void assign(Rep_base const &aOther) {
    try {
      m_set = cast(aOther).m_set;
    }
    catch (...) {
      show_error("Exception thrown during boolean operation assign");
    }
  }

  virtual void intersect(Rep_base const &aOther) {
    try {
      m_set.intersection(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation intersect");
    }
  }

  virtual void join(Rep_base const &aOther) {
    try {
      m_set.join(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation union");
    }
  }

  virtual void difference(Rep_base const &aOther) {
    try {
      m_set.difference(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation difference");
    }
  }

  virtual void symmetric_difference(Rep_base const &aOther) {
    try {
      m_set.symmetric_difference(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation symmetric difference");
    }
  }

  virtual void complement(Rep_base const &aOther) {
    try {
      m_set.complement(cast(aOther).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation complement");
    }
  }

  virtual void intersect(Rep_base const &aFirst, Rep_base const &aSecond) {
    try {
      m_set.intersection(cast(aFirst).m_set, cast(aSecond).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation intersect");
    }
  }

  virtual void join(Rep_base const &aFirst, Rep_base const &aSecond) {
    try {
      m_set.join(cast(aFirst).m_set, cast(aSecond).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation union");
    }
  }

  virtual void difference(Rep_base const &aFirst, Rep_base const &aSecond) {
    try {
      m_set.difference(cast(aFirst).m_set, cast(aSecond).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation difference");
    }
  }

  virtual void symmetric_difference(Rep_base const &aFirst,
                                    Rep_base const &aSecond) {
    try {
      m_set.symmetric_difference(cast(aFirst).m_set, cast(aSecond).m_set);
    }
    catch (...) {
      show_error("Exception thrown during boolean operation symmetric difference");
    }
  }

  static Self const &cast(Rep_base const &aOther) { return dynamic_cast<Self const &>(aOther); }

  static Self &cast(Rep_base &aOther) { return dynamic_cast<Self &>(aOther); }

private:
  GI *m_GI;
  Set m_set;
};

//A class for connecting linear polygon GUI and this file
class Linear_rep : public Rep<Linear_GI, Linear_polygon_set, Linear_traits> {
  typedef Rep<Linear_GI, Linear_polygon_set, Linear_traits> Base;
public:
  Linear_rep() : Base() {}

  virtual int type() const { return 1; }
};

//A class for connecting circular polygon GUI and this file
class Circular_rep : public Rep<Circular_GI, Circular_polygon_set, Circular_traits> {
  typedef Rep<Circular_GI, Circular_polygon_set, Circular_traits> Base;
public:
  Circular_rep() : Base() {}

  virtual int type() const { return 2; }
};

//A class for connecting polyline polygon GUI and this file
class Polyline_rep : public Rep<Polyline_GI, Polyline_polygon_set, Polyline_traits> {
  typedef Rep<Polyline_GI, Polyline_polygon_set, Polyline_traits> Base;
public:
  Polyline_rep() : Base() {}

  virtual int type() const { return 4; }
};

//A class for connecting bezier polygon GUI and this file
class Bezier_rep : public Rep_o<Bezier_GI, Bezier_polygon_set> {
  typedef Rep_o<Bezier_GI, Bezier_polygon_set> Base;
public:
  Bezier_rep() : Base() {}

  virtual int type() const { return 3; }
};

//A class for connecting conic polygon GUI and this file
class Conic_rep : public Rep_o<CGAL::Qt::Conic_GI, Conic_polygon_set> {
  typedef Rep_o<CGAL::Qt::Conic_GI, Conic_polygon_set> Base;
public:
  Conic_rep() : Base() {}

  virtual int type() const { return 5; }
};

// a container which deletes an object when last shared_ptr gets deleted or re-initiated
class Curve_set {
  typedef boost::shared_ptr <Rep_base> Rep_ptr;

public:
  //constructor
  Curve_set(int aType, QPen aPen, QBrush aBrush) : m_pen(aPen), m_brush(aBrush) { reset_type(aType); }

  void reset_type(int aType) {
    //setting shared_ptr for respective polygon
    if (aType == 1) m_rep = Rep_ptr(new Linear_rep());
    else if (aType == 2) m_rep = Rep_ptr(new Circular_rep());
    else if (aType == 3) m_rep = Rep_ptr(new Bezier_rep());
    else if (aType == 4) m_rep = Rep_ptr(new Polyline_rep());
    else if (aType == 5) m_rep = Rep_ptr(new Conic_rep());

    //setting pen and brush
    m_rep->set_pen(m_pen);
    m_rep->set_brush(m_brush);
  }

  CGAL::Qt::GraphicsItem const *gi() const { return m_rep->gi(); }

  CGAL::Qt::GraphicsItem *gi() { return m_rep->gi(); }

  QRectF bounding_rect() const { return m_rep->bounding_rect(); }

  void unbounded_rect_changed() { m_rep->unbounded_rect_changed(); }

  bool is_empty() const { return !m_rep || m_rep->is_empty(); }

  void clear() { m_rep->clear(); }

  //boolean operations
  void complement() { m_rep->complement(); }

  void assign(Curve_set const &aOther) {
    if (is_linear() && aOther.is_linear())
      get_linear_rep()->assign(*aOther.get_linear_rep());
    else if (is_circular() && aOther.is_circular())
      get_circular_rep()->assign(*aOther.get_circular_rep());
    else if (is_bezier() && aOther.is_bezier())
      get_bezier_rep()->assign(*aOther.get_bezier_rep());
    else if (is_polyline() && aOther.is_polyline())
      get_polyline_rep()->assign(*aOther.get_polyline_rep());
    else if (is_conic() && aOther.is_conic())
      get_conic_rep()->assign(*aOther.get_conic_rep());
  }

  void intersect(Curve_set const &aOther) {
    if (is_linear() && aOther.is_linear())
      get_linear_rep()->intersect(*aOther.get_linear_rep());
    else if (is_circular() && aOther.is_circular())
      get_circular_rep()->intersect(*aOther.get_circular_rep());
    else if (is_bezier() && aOther.is_bezier())
      get_bezier_rep()->intersect(*aOther.get_bezier_rep());
    else if (is_polyline() && aOther.is_polyline())
      get_polyline_rep()->intersect(*aOther.get_polyline_rep());
    else if (is_conic() && aOther.is_conic())
      get_conic_rep()->intersect(*aOther.get_conic_rep());
  }

  void join(Curve_set const &aOther) {
    if (is_linear() && aOther.is_linear())
      get_linear_rep()->join(*aOther.get_linear_rep());
    else if (is_circular() && aOther.is_circular())
      get_circular_rep()->join(*aOther.get_circular_rep());
    else if (is_bezier() && aOther.is_bezier())
      get_bezier_rep()->join(*aOther.get_bezier_rep());
    else if (is_polyline() && aOther.is_polyline())
      get_polyline_rep()->join(*aOther.get_polyline_rep());
    else if (is_conic() && aOther.is_conic())
      get_conic_rep()->join(*aOther.get_conic_rep());
  }

  void difference(Curve_set const &aOther) {
    if (is_linear() && aOther.is_linear())
      get_linear_rep()->difference(*aOther.get_linear_rep());
    else if (is_circular() && aOther.is_circular())
      get_circular_rep()->difference(*aOther.get_circular_rep());
    else if (is_bezier() && aOther.is_bezier())
      get_bezier_rep()->difference(*aOther.get_bezier_rep());
    else if (is_polyline() && aOther.is_polyline())
      get_polyline_rep()->difference(*aOther.get_polyline_rep());
    else if (is_conic() && aOther.is_conic())
      get_conic_rep()->difference(*aOther.get_conic_rep());
  }

  void symmetric_difference(Curve_set const &aOther) {
    if (is_linear() && aOther.is_linear())
      get_linear_rep()->symmetric_difference(*aOther.get_linear_rep());
    else if (is_circular() && aOther.is_circular())
      get_circular_rep()->symmetric_difference(*aOther.get_circular_rep());
    else if (is_bezier() && aOther.is_bezier())
      get_bezier_rep()->symmetric_difference(*aOther.get_bezier_rep());
    else if (is_polyline() && aOther.is_polyline())
      get_polyline_rep()->symmetric_difference(*aOther.get_polyline_rep());
    else if (is_conic() && aOther.is_conic())
      get_conic_rep()->symmetric_difference(*aOther.get_conic_rep());
  }

  // Bivariate operations: *this = aFirst op aSecond (all of the same type).
  void complement(Curve_set const &aOther) {
    if (same_type(aOther)) m_rep->complement(*aOther.m_rep);
  }

  void intersect(Curve_set const &aFirst, Curve_set const &aSecond) {
    if (same_type(aFirst) && same_type(aSecond))
      m_rep->intersect(*aFirst.m_rep, *aSecond.m_rep);
  }

  void join(Curve_set const &aFirst, Curve_set const &aSecond) {
    if (same_type(aFirst) && same_type(aSecond))
      m_rep->join(*aFirst.m_rep, *aSecond.m_rep);
  }

  void difference(Curve_set const &aFirst, Curve_set const &aSecond) {
    if (same_type(aFirst) && same_type(aSecond))
      m_rep->difference(*aFirst.m_rep, *aSecond.m_rep);
  }

  void symmetric_difference(Curve_set const &aFirst,
                            Curve_set const &aSecond) {
    if (same_type(aFirst) && same_type(aSecond))
      m_rep->symmetric_difference(*aFirst.m_rep, *aSecond.m_rep);
  }

  bool same_type(Curve_set const &aOther) const
  { return m_rep && aOther.m_rep && (m_rep->type() == aOther.m_rep->type()); }

  void change_brush_color(QBrush aBrush) {
    m_brush = aBrush;
  }

  void change_pen_color(QPen aPen) {
    m_pen = aPen;
  }

  //Minkowski Sum handled separately
  //see its need keep it for now
  const Rep_base &rep() const { return *m_rep; }

  Rep_base &rep() { return *m_rep; }

  bool is_linear() const { return m_rep->type() == 1; }

  bool is_circular() const { return m_rep->type() == 2; }

  bool is_bezier() const { return m_rep->type() == 3; }

  bool is_polyline() const { return m_rep->type() == 4; }

  bool is_conic() const { return m_rep->type() == 5; }

  const Conic_rep *get_conic_rep() const
  { return dynamic_cast<Conic_rep const *>(boost::get_pointer(m_rep)); }

  Conic_rep *get_conic_rep()
  { return dynamic_cast<Conic_rep *>(boost::get_pointer(m_rep)); }

  const Conic_polygon_set &conic() const { return get_conic_rep()->set(); }

  Conic_polygon_set &conic() { return get_conic_rep()->set(); }

  //to get rep for linear polygons
  const Linear_rep *get_linear_rep() const { return dynamic_cast<Linear_rep const *>(boost::get_pointer(m_rep)); }

  Linear_rep *get_linear_rep() { return dynamic_cast<Linear_rep *>(boost::get_pointer(m_rep)); }

  //to get Linear_polygon_set
  const Linear_polygon_set &linear() const { return get_linear_rep()->set(); }

  Linear_polygon_set &linear() { return get_linear_rep()->set(); }

  //to get rep for circular polygons
  const Circular_rep *get_circular_rep() const { return dynamic_cast<Circular_rep const *>(boost::get_pointer(m_rep)); }

  Circular_rep *get_circular_rep() { return dynamic_cast<Circular_rep *>(boost::get_pointer(m_rep)); }

  //to get Circular_polygon_set
  const Circular_polygon_set &circular() const { return get_circular_rep()->set(); }

  Circular_polygon_set &circular() { return get_circular_rep()->set(); }

  //get rep for bezier polygons
  Bezier_rep const *get_bezier_rep() const { return dynamic_cast<Bezier_rep const *>( boost::get_pointer(m_rep)); }

  Bezier_rep *get_bezier_rep() { return dynamic_cast<Bezier_rep *        >( boost::get_pointer(m_rep)); }

  //to get Bezier_polygon_set
  const Bezier_polygon_set &bezier() const { return get_bezier_rep()->set(); }

  Bezier_polygon_set &bezier() { return get_bezier_rep()->set(); }

  //to get rep for polylines
  const Polyline_rep *get_polyline_rep() const {
    return dynamic_cast<Polyline_rep const *>( boost::get_pointer(m_rep));
  }

  Polyline_rep *get_polyline_rep() { return dynamic_cast<Polyline_rep *>( boost::get_pointer(m_rep)); }

  //to get Polyline_polygon_set
  const Polyline_polygon_set &polyline() const { return get_polyline_rep()->set(); }

  Polyline_polygon_set &polyline() { return get_polyline_rep()->set(); }

public:
  //drawing tools
  QPen m_pen;
  QBrush m_brush;
  // a container which deletes an object when last shared_ptr gets deleted or
  // re-initiated
  boost::shared_ptr <Rep_base> m_rep;
};

typedef std::vector <Curve_set> Curve_set_container;
typedef Curve_set_container::const_iterator Curve_set_const_iterator;
typedef Curve_set_container::iterator Curve_set_iterator;

// This container is for returning different color polygons while performing an operation
class State_current {
  /* COMPLEMENT_OP = 0
  // INTERSECTION_OP = 1
  // UNION_OP = 2
  // DIFFERENCE_OP = 3
  // SYMMETRIC_DIFFERENCE_OP = 4
  // MINKOWSKI_SUM_OP = 5
  // COPY_OP = 6
  // MOVE_OP = 7
  // CLEAR_OP = 9
  // DELETE_ALL_OP = 10
  // START_OP = 11*/

public:
  State_current(size_t operation_name, size_t aType);

public:
  Curve_set &set(size_t aGroup) { return m_curve_sets[aGroup]; }
  /*gets which group is currently active now
    size_t active_group() const { return m_color_active; }
    size_t complement_group() const {return m_color_complement; } //see if needed
  */

  //sets the current active group
  Curve_set &active_set(size_t m_color_active) { return set(m_color_active); }

  Curve_set &move_set(size_t m_color_move) { return set(m_color_move); }

  //setting curve
  Curve_set &blue_set() { return set(BLUE_GROUP); }

  Curve_set &red_set() { return set(RED_GROUP); }

  Curve_set &black_set() { return set(BLACK_GROUP); }

  Curve_set &brown_set() { return set(BROWN_GROUP); }

  Curve_set &yellow_set() { return set(YELLOW_GROUP); }

  Curve_set &magenta_set() { return set(MAGENTA_GROUP); }

  Curve_set &aqua_set() { return set(AQUA_GROUP); }

  Curve_set &result_set() { return set(RESULT_GROUP); }

  void ToggleView(size_t aGROUP, bool a_check) {
    if (a_check) set(aGROUP).gi()->show();
    else set(aGROUP).gi()->hide();
  }

  //returns linear containers
  Linear_region_source_container &blue_linear_sources() { return m_blue_linear_sources; }

  Linear_region_source_container &red_linear_sources() { return m_red_linear_sources; }

  Linear_region_source_container &black_linear_sources() { return m_black_linear_sources; }

  Linear_region_source_container &brown_linear_sources() { return m_brown_linear_sources; }

  Linear_region_source_container &yellow_linear_sources() { return m_yellow_linear_sources; }

  Linear_region_source_container &magenta_linear_sources() { return m_magenta_linear_sources; }

  Linear_region_source_container &aqua_linear_sources() { return m_aqua_linear_sources; }

  Linear_region_source_container &result_linear_sources() { return m_result_linear_sources; }

  //returns circular containers
  Circular_region_source_container &blue_circular_sources() { return m_blue_circular_sources; }

  Circular_region_source_container &red_circular_sources() { return m_red_circular_sources; }

  Circular_region_source_container &black_circular_sources() { return m_black_circular_sources; }

  Circular_region_source_container &brown_circular_sources() { return m_brown_circular_sources; }

  Circular_region_source_container &yellow_circular_sources() { return m_yellow_circular_sources; }

  Circular_region_source_container &magenta_circular_sources() { return m_magenta_circular_sources; }

  Circular_region_source_container &aqua_circular_sources() { return m_aqua_circular_sources; }

  Circular_region_source_container &result_circular_sources() { return m_result_circular_sources; }

  //returns bezier containers
  Bezier_region_source_container &blue_bezier_sources() { return m_blue_bezier_sources; }

  Bezier_region_source_container &red_bezier_sources() { return m_red_bezier_sources; }

  Bezier_region_source_container &black_bezier_sources() { return m_black_bezier_sources; }

  Bezier_region_source_container &brown_bezier_sources() { return m_brown_bezier_sources; }

  Bezier_region_source_container &yellow_bezier_sources() { return m_yellow_bezier_sources; }

  Bezier_region_source_container &magenta_bezier_sources() { return m_magenta_bezier_sources; }

  Bezier_region_source_container &aqua_bezier_sources() { return m_aqua_bezier_sources; }

  Bezier_region_source_container &result_bezier_sources() { return m_result_bezier_sources; }

  //returns polyline containers
  Polyline_region_source_container &blue_polyline_sources() { return m_blue_polyline_sources; }

  Polyline_region_source_container &red_polyline_sources() { return m_red_polyline_sources; }

  Polyline_region_source_container &black_polyline_sources() { return m_black_polyline_sources; }

  Polyline_region_source_container &brown_polyline_sources() { return m_brown_polyline_sources; }

  Polyline_region_source_container &yellow_polyline_sources() { return m_yellow_polyline_sources; }

  Polyline_region_source_container &magenta_polyline_sources() { return m_magenta_polyline_sources; }

  Polyline_region_source_container &aqua_polyline_sources() { return m_aqua_polyline_sources; }

  Polyline_region_source_container &result_polyline_sources() { return m_result_polyline_sources; }

  //choosing colors in case of linear polygons
  Linear_region_source_container &active_linear_sources(size_t m_color_active) {
    switch (m_color_active) {
     case 0:return m_blue_linear_sources;
     case 1:return m_red_linear_sources;
     case 2:return m_black_linear_sources;
     case 3:return m_brown_linear_sources;
     case 4:return m_yellow_linear_sources;
     case 5:return m_magenta_linear_sources;
     case 6:return m_aqua_linear_sources;
     case 7:return m_result_linear_sources;

     default:break;
    }
    CGAL_warning_msg(true, "Should not reach here!");
    return m_blue_linear_sources;
  }

  //choosing colors in case of circular polygons
  Circular_region_source_container &active_circular_sources(size_t m_color_active) {
    switch (m_color_active) {
     case 0:return m_blue_circular_sources;
     case 1:return m_red_circular_sources;
     case 2:return m_black_circular_sources;
     case 3:return m_brown_circular_sources;
     case 4:return m_yellow_circular_sources;
     case 5:return m_magenta_circular_sources;
     case 6:return m_aqua_circular_sources;
     case 7:return m_result_circular_sources;

     default:break;
    }
    CGAL_warning_msg(true, "Should not reach here!");
    return m_blue_circular_sources;
  }

  //choosing colors in case of bezier polygons
  Bezier_region_source_container &active_bezier_sources(size_t m_color_active) {
    switch (m_color_active) {
     case 0:return m_blue_bezier_sources;
     case 1:return m_red_bezier_sources;
     case 2:return m_black_bezier_sources;
     case 3:return m_brown_bezier_sources;
     case 4:return m_yellow_bezier_sources;
     case 5:return m_magenta_bezier_sources;
     case 6:return m_aqua_bezier_sources;
     case 7:return m_result_bezier_sources;

     default:break;
    }
    CGAL_warning_msg(true, "Should not reach here!");
    return m_blue_bezier_sources;
  }

  //choosing colors in case of polyline polygons
  Polyline_region_source_container &active_polyline_sources(size_t m_color_active) {
    switch (m_color_active) {
     case 0:return m_blue_polyline_sources;
     case 1:return m_red_polyline_sources;
     case 2:return m_black_polyline_sources;
     case 3:return m_brown_polyline_sources;
     case 4:return m_yellow_polyline_sources;
     case 5:return m_magenta_polyline_sources;
     case 6:return m_aqua_polyline_sources;
     case 7:return m_result_polyline_sources;

     default:break;
    }
    CGAL_warning_msg(true, "Should not reach here!");
    return m_blue_polyline_sources;
  }

public:
  Curve_set_container m_curve_sets;
  //container for curves
  Linear_region_source_container m_blue_linear_sources;
  Linear_region_source_container m_red_linear_sources;
  Linear_region_source_container m_black_linear_sources;
  Linear_region_source_container m_brown_linear_sources;
  Linear_region_source_container m_yellow_linear_sources;
  Linear_region_source_container m_magenta_linear_sources;
  Linear_region_source_container m_aqua_linear_sources;
  Linear_region_source_container m_result_linear_sources;

  Circular_region_source_container m_blue_circular_sources;
  Circular_region_source_container m_red_circular_sources;
  Circular_region_source_container m_black_circular_sources;
  Circular_region_source_container m_brown_circular_sources;
  Circular_region_source_container m_yellow_circular_sources;
  Circular_region_source_container m_magenta_circular_sources;
  Circular_region_source_container m_aqua_circular_sources;
  Circular_region_source_container m_result_circular_sources;

  Bezier_region_source_container m_blue_bezier_sources;
  Bezier_region_source_container m_red_bezier_sources;
  Bezier_region_source_container m_black_bezier_sources;
  Bezier_region_source_container m_brown_bezier_sources;
  Bezier_region_source_container m_yellow_bezier_sources;
  Bezier_region_source_container m_magenta_bezier_sources;
  Bezier_region_source_container m_aqua_bezier_sources;
  Bezier_region_source_container m_result_bezier_sources;

  Polyline_region_source_container m_blue_polyline_sources;
  Polyline_region_source_container m_red_polyline_sources;
  Polyline_region_source_container m_black_polyline_sources;
  Polyline_region_source_container m_brown_polyline_sources;
  Polyline_region_source_container m_yellow_polyline_sources;
  Polyline_region_source_container m_magenta_polyline_sources;
  Polyline_region_source_container m_aqua_polyline_sources;
  Polyline_region_source_container m_result_polyline_sources;

  //size_t m_state_num;
  size_t m_operation;
  size_t m_type;
};

//defining the State_current constructor
State_current::State_current(size_t operation_name, size_t aType) : m_operation(operation_name), m_type(aType) {
  //for setting Curve_set of aGroup type an int representing a set of polygon of a specific type

  //default setups with Linear
  m_curve_sets.push_back(Curve_set(m_type, sPens[BLUE_GROUP], sBrushes[BLUE_GROUP]));
  m_curve_sets.push_back(Curve_set(m_type, sPens[RED_GROUP], sBrushes[RED_GROUP]));
  m_curve_sets.push_back(Curve_set(m_type, sPens[BLACK_GROUP], sBrushes[BLACK_GROUP]));
  m_curve_sets.push_back(Curve_set(m_type, sPens[BROWN_GROUP], sBrushes[BROWN_GROUP]));
  m_curve_sets.push_back(Curve_set(m_type, sPens[YELLOW_GROUP], sBrushes[YELLOW_GROUP]));
  m_curve_sets.push_back(Curve_set(m_type, sPens[MAGENTA_GROUP], sBrushes[MAGENTA_GROUP]));
  m_curve_sets.push_back(Curve_set(m_type, sPens[AQUA_GROUP], sBrushes[AQUA_GROUP]));
  m_curve_sets.push_back(Curve_set(m_type, sPens[RESULT_GROUP], sBrushes[RESULT_GROUP]));
}

typedef std::vector <State_current> state_container;
typedef state_container::const_iterator state_const_iterator;
typedef state_container::iterator state_iterator;

// Lets the user move a floating dock widget by dragging its title bar under
// Wayland. There, QDockWidget's own drag fails, because it grabs the mouse,
// which Wayland allows only for popup windows ("This plugin supports grabbing
// the mouse only for popup windows"). Instead, ask the compositor to move the
// window.
class Floating_dock_mover : public QObject {
public:
  explicit Floating_dock_mover(QObject* parent) : QObject(parent) {}

protected:
  bool eventFilter(QObject* obj, QEvent* event) override {
    if (event->type() != QEvent::MouseButtonPress) return false;
    auto* dock = qobject_cast<QDockWidget*>(obj);
    if (! dock || ! dock->isFloating()) return false;
    auto* me = static_cast<QMouseEvent*>(event);
    if (me->button() != Qt::LeftButton) return false;
    // Presses on the title-bar buttons go to the buttons themselves; a press
    // outside the content widget is on the title bar.
    QWidget* content = dock->widget();
    if (content && content->geometry().contains(me->position().toPoint()))
      return false;
    QWindow* window = dock->windowHandle();
    return window && window->startSystemMove();
  }
};

// On dark desktop themes the outlines of check box squares and radio button
// circles are hardly visible. Draw the indicator as usual (keeping the check
// mark or dot), then draw its outline on top: a gray square with slightly
// rounded corners for a check box, and a gray ring for a radio button. The
// outline turns white while the cursor hovers over the button.
class Visible_indicator_style : public QProxyStyle {
public:
  void polish(QWidget* widget) override {
    QProxyStyle::polish(widget);
    // Track hovering, so that State_MouseOver is set.
    if (qobject_cast<QCheckBox*>(widget) ||
        qobject_cast<QRadioButton*>(widget))
      widget->setAttribute(Qt::WA_Hover);
  }

  void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                     QPainter* painter,
                     const QWidget* widget = nullptr) const override {
    QProxyStyle::drawPrimitive(element, option, painter, widget);
    if ((element != PE_IndicatorCheckBox) &&
        (element != PE_IndicatorRadioButton))
      return;
    bool hover = option->state.testFlag(State_MouseOver) &&
      option->state.testFlag(State_Enabled);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(hover ? QColor(Qt::white) : QColor(0x80, 0x80, 0x80),
                         1));
    QRectF rect = QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5);
    if (element == PE_IndicatorCheckBox)
      painter->drawRoundedRect(rect, 1.5, 1.5);
    else painter->drawEllipse(rect);
    painter->restore();
  }
};

// A scene that can draw a grid in its background, behind all items. Grid
// lines that would be denser than a few pixels on screen are thinned out (by
// factors of 10), so that zooming out does not fill the view with lines.
class Grid_scene : public QGraphicsScene {
public:
  bool grid_visible() const { return m_grid_visible; }
  void set_grid_visible(bool aVisible) { m_grid_visible = aVisible; update(); }

  double grid_spacing() const { return m_grid_spacing; }
  void set_grid_spacing(double aSpacing) {
    m_grid_spacing = aSpacing;
    update();
  }

  // The grid point nearest to aP.
  QPointF snap(const QPointF& aP) const {
    return QPointF(std::round(aP.x() / m_grid_spacing) * m_grid_spacing,
                   std::round(aP.y() / m_grid_spacing) * m_grid_spacing);
  }

protected:
  void drawBackground(QPainter* painter, const QRectF& rect) override {
    QGraphicsScene::drawBackground(painter, rect);
    if (! m_grid_visible || (m_grid_spacing <= 0)) return;
    double scale = std::abs(painter->worldTransform().m11());
    if (scale <= 0) return;
    const double min_pixels = 6;
    double step = m_grid_spacing;
    while (step * scale < min_pixels) step *= 10;
    QVector<QLineF> lines;
    for (double x = std::ceil(rect.left() / step) * step; x <= rect.right();
         x += step)
      lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (double y = std::ceil(rect.top() / step) * step; y <= rect.bottom();
         y += step)
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
  Grid_snapper(Grid_scene* aScene, QObject* aParent) :
    QObject(aParent), m_scene(aScene) {}

  bool enabled() const { return m_enabled; }
  void set_enabled(bool aEnabled) { m_enabled = aEnabled; }

protected:
  bool eventFilter(QObject* obj, QEvent* event) override {
    if (obj != m_scene) return false;
    switch (event->type()) {
     case QEvent::GraphicsSceneMousePress:
     case QEvent::GraphicsSceneMouseMove:
     case QEvent::GraphicsSceneMouseRelease:
     case QEvent::GraphicsSceneMouseDoubleClick: {
       auto* me = static_cast<QGraphicsSceneMouseEvent*>(event);
       me->setScenePos(m_enabled ? m_scene->snap(me->scenePos()) :
                       round_to_pixel(me->scenePos(), me->widget()));
       break;
     }
     default: break;
    }
    return false;
  }

private:
  // Round aP to a multiple of the largest power of 2 that does not exceed
  // half a pixel of the view whose viewport is aViewport. Dividing and
  // multiplying by a power of 2 is exact.
  static QPointF round_to_pixel(const QPointF& aP, QWidget* aViewport) {
    auto* view = aViewport ?
      qobject_cast<QGraphicsView*>(aViewport->parentWidget()) : nullptr;
    if (! view) return aP;
    double scale = std::abs(view->transform().m11());
    if (! (scale > 0) || ! std::isfinite(scale)) return aP;
    double step = std::ldexp(1.0, int(std::floor(std::log2(0.5 / scale))));
    return QPointF(std::round(aP.x() / step) * step,
                   std::round(aP.y() / step) * step);
  }

  Grid_scene* m_scene;
  bool m_enabled = false;
};

// The Boolean operations on buckets.
enum class Bucket_op { Complement, Intersection, Join, Difference,
                       Symmetric_difference };

// The names of the buckets in the program, by bucket index (the colors the
// rows display).
inline const char* bucket_name(std::size_t aIndex) {
  static const char* names[] =
    { "red", "black", "blue", "brown", "yellow", "magenta", "aqua" };
  return (aIndex < 7) ? names[aIndex] : "result";
}

// The shortest decimal representation that reads back as the same double, so
// that the program reproduces the exact coordinates.
inline QString to_python(double aValue) {
  char buffer[64];
  auto res = std::to_chars(buffer, buffer + sizeof(buffer), aValue);
  return QString::fromLatin1(buffer, int(res.ptr - buffer));
}

// A linear polygon as a Python expression. (Its curves are segments along
// the boundary, in order; their sources are the vertices.)
inline QString to_python(Linear_polygon const& aPolygon) {
  QStringList points;
  for (auto cit = aPolygon.curves_begin(); cit != aPolygon.curves_end(); ++cit)
    points << QString("Point_2(%1, %2)")
      .arg(to_python(CGAL::to_double(cit->source().x())),
           to_python(CGAL::to_double(cit->source().y())));
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

// The statement (or expression) as normalized Python text.
inline QString to_python(Py_expr const& aExpr) {
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

inline QString to_python(Py_statement const& aStatement) {
  switch (aStatement.m_kind) {
   case Py_statement::Kind::Assign:
    return aStatement.m_target + " = " + to_python(aStatement.m_args[0]);
   case Py_statement::Kind::Delete: return "del " + aStatement.m_target;
   case Py_statement::Kind::Call: {
     QStringList args;
     for (const auto& arg : aStatement.m_args) args << to_python(arg);
     return QString("%1.%2(%3)").arg(aStatement.m_target, aStatement.m_method,
                                     args.join(", "));
   }
  }
  return QString();
}

// A recursive-descent parser for the console language. Throws a
// std::runtime_error with a message on a syntax error.
class Py_parser {
public:
  explicit Py_parser(const QString& aLine) { tokenize(aLine); }

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

  [[noreturn]] void error(const QString& aMessage) const
  { throw std::runtime_error(aMessage.toStdString()); }

  void tokenize(const QString& aLine) {
    static const QRegularExpression number(
      R"(^(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?)");
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

  const Token& peek(std::size_t aAhead = 0) const
  { return m_tokens[std::min(m_pos + aAhead, m_tokens.size() - 1)]; }
  bool at_end() const { return peek().m_kind == Tok::End; }
  bool accept(const QString& aPunct) {
    if ((peek().m_kind == Tok::Punct) && (peek().m_text == aPunct)) {
      ++m_pos;
      return true;
    }
    return false;
  }
  void expect(const QString& aPunct) {
    if (! accept(aPunct))
      error(QString("expected '%1'%2").arg(aPunct, near()));
  }
  QString near() const {
    return at_end() ? QString(" at the end") :
      QString(" before '%1'").arg(peek().m_text);
  }
  QString identifier() {
    if (peek().m_kind != Tok::Ident) error("expected a name" + near());
    return m_tokens[m_pos++].m_text;
  }
  double number() {
    bool negative = false;
    if (accept("-")) negative = true;
    else accept("+");
    if (peek().m_kind != Tok::Number) error("expected a number" + near());
    double value = m_tokens[m_pos++].m_text.toDouble();
    return negative ? -value : value;
  }

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

//The main page for displaying the window and all the icons
class MainWindow : public CGAL::Qt::DemosMainWindow,
                   public Ui::Boolean_set_operations_2 {
  Q_OBJECT// removing it gives error ui not declared

private:
  Grid_scene m_scene;
  // The Intersection and Union columns (one cell per bucket).
  std::array<QAbstractButton*, 7> m_intersection_column;
  std::array<QAbstractButton*, 7> m_union_column;
  std::array<QAbstractButton*, 7> m_complement_column;
  std::array<QAbstractButton*, 7> m_difference_column;
  std::array<QAbstractButton*, 7> m_symmetric_difference_column;

  // The program: the statements executed so far, each tagged with the size
  // of the undo stack after its action, so that undo can drop them.
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
  //keep it intact for now check it out
  bool m_circular_active;
  bool m_bezier_active;
  bool m_polyline_active;
  bool m_conic_active = false;

  //to determine which type is currently active
  size_t m_color_active;
  size_t m_color_move;
  bool empty_warn;
  bool operand_error_warn;
  size_t m_color_visible;
  size_t m_color_cm;
  size_t m_color_complement;
  size_t m_color_copy;
  size_t m_color_move_ext;
  bool m_blue_int;
  bool m_red_int;
  bool m_black_int;
  bool m_brown_int;
  bool m_yellow_int;
  bool m_magenta_int;
  bool m_aqua_int;
  bool m_grid;
  bool m_pan;
  bool m_reset;

  bool m_blue_union;
  bool m_red_union;
  bool m_black_union;
  bool m_brown_union;
  bool m_yellow_union;
  bool m_magenta_union;
  bool m_aqua_union;

  bool m_blue_sym_diff;
  bool m_red_sym_diff;
  bool m_black_sym_diff;
  bool m_brown_sym_diff;
  bool m_yellow_sym_diff;
  bool m_magenta_sym_diff;
  bool m_aqua_sym_diff;

  bool m_blue_mink;
  bool m_red_mink;
  bool m_black_mink;
  bool m_brown_mink;
  bool m_yellow_mink;
  bool m_magenta_mink;
  bool m_aqua_mink;

  bool m_visible_black;
  bool m_visible_brown;
  bool m_visible_yellow;
  bool m_visible_magenta;
  bool m_visible_aqua;

  bool m_clear_blue;
  bool m_clear_red;
  bool m_clear_black;
  bool m_clear_brown;
  bool m_clear_yellow;
  bool m_clear_magenta;
  bool m_clear_aqua;

  bool minkowski_sum_operated;
  bool m_disjoint;

  size_t m_state_num;

  QGraphicsLineItem *xAxis = new QGraphicsLineItem();
  QGraphicsLineItem *yAxis = new QGraphicsLineItem();

  //typedefs of classes used to draw circular and linear polygon

  state_container states_stack;

  CGAL::Qt::Graphics_view_linear_polygon_input <Kernel> *m_linear_input;
  CGAL::Qt::Graphics_view_circular_polygon_input <Kernel> *m_circular_input;
  CGAL::Qt::GraphicsViewBezierPolygonInput <Bezier_traits> *m_bezier_input;
  CGAL::Qt::Graphics_view_polyline_input <Kernel> *m_polyline_input;
  CGAL::Qt::Graphics_view_conic_input *m_conic_input;

public:
  MainWindow();

private:
  void zoomToFit();

protected
slots:
  //for file handling
  void open(QString
            filename);

  //defining QT slots for GUI integration
public
slots:
  //for all the operations such as union, complement, difference

  void processInput(CGAL::Object o);

  void on_actionUndo_triggered();

  void on_actionNew_triggered();

  void on_actionAxis_triggered();

  void on_actionRecenter_triggered();

  void on_actionComplementH_toggled(bool aChecked);

  void on_actionUnionH_toggled(bool aChecked);

  void on_actionIntersectionH_toggled(bool aChecked);

  void on_actionDifferenceH_toggled(bool aChecked);

  void on_actionSymmetric_DifferenceH_toggled(bool aChecked);

  void on_actionMinkowski_SumH_toggled(bool aChecked);

  void on_actionInsertLinear_toggled(bool aChecked);

  void on_actionInsertCircular_toggled(bool aChecked);
  void on_actionInsertConic_toggled(bool aChecked);

  void on_actionInsertBezier_toggled(bool aChecked);

  void on_actionInsertPolyline_toggled(bool aChecked);

  void on_showColorBucket_toggled(bool aChecked);

  void on_sceneDockWidget_visibilityChanged(bool visible);

  void on_infoDockWidget_visibilityChanged(bool visible);

  void on_consoleDockWidget_visibilityChanged(bool visible);

  void on_showConsole_toggled(bool aChecked);

  void on_showInfo_toggled(bool aChecked);

  void on_actionOpenLinear_triggered();

  void on_actionOpenDXF_triggered();

  void on_actionOpenBezier_triggered();

  void on_actionOpenPolyline_triggered();

  void wheelEvent(QWheelEvent *event);

  void on_actionSaveCurrentBucket_triggered();

  void on_actionAddColor_triggered();

  void on_actionMinusColor_triggered();

  void on_actionCopyH_toggled(bool aChecked);

  void on_actionMoveH_toggled(bool aChecked);

  QMessageBox::StandardButton
  show_message(QMessageBox::Icon aIcon, const QString& aTitle,
               const QString& aText, QAction* aToggle,
               QMessageBox::StandardButtons aButtons = QMessageBox::Ok);
  bool show_not_empty_warning();
  void show_operand_error();
  bool draw_row_checked(const std::array<QAbstractButton*, 7>& aColumn) const;
  std::vector<std::size_t>
  checked_buckets(const std::array<QAbstractButton*, 7>& aColumn) const;
  void uncheck_operations_except(QAbstractButton* aOperation);
  void clear_result();
  void run_operation(Bucket_op aOp, const std::vector<std::size_t>& aOperands,
                     QAbstractButton* aOperation);
  void log_statements(const QStringList& aStatements, bool aSticky = false);
  bool eventFilter(QObject* aObject, QEvent* aEvent) override;
  void update_unbounded_rect();
  int active_type() const;
  QObject* current_input() const;
  void remove_inputs();
  void install_input(QObject* aInput);
  bool ensure_conic_mode();
  QAction* family_action(int aType) const;
  void set_conic_pen(const QPen& aPen);
  QIcon curve_tool_icon(const QString& aKind);
  bool bucket_exists(std::size_t aIndex) const;
  std::size_t bucket_index(const QString& aName) const;
  Curve_set* polygon_set(const QString& aName);
  Linear_polygon_with_holes polygon(Py_expr const& aExpr) const;
  QString execute(Py_statement const& aStatement);
  void run_console_line(const QString& aLine);
  void console_print(const QString& aText);
  void trim_program_to_states();
  void save_program();
  void limit_to_all_but_one(const std::array<QAbstractButton*, 7>& aColumn,
                            QAbstractButton* aBox, bool aChecked,
                            QAbstractButton* aOperation);
  void keep_one_unchecked(const std::array<QAbstractButton*, 7>& aColumn);
  void limit_to_two_operands(const std::array<QAbstractButton*, 7>& aColumn,
                             QAbstractButton* aBox, bool aChecked,
                             QAbstractButton* aOperation);
  bool abort_for_draw_bucket(QAbstractButton* aOperation, bool aIsOperand);

  //showing colors when clicked on the checkbox
  void on_showBlue_toggled(bool a_check);

  void on_showRed_toggled(bool a_check);

  void on_showBlack_toggled(bool a_check);

  void on_showBrown_toggled(bool a_check);

  void on_showYellow_toggled(bool a_check);

  void on_showMagenta_toggled(bool a_check);

  void on_showAqua_toggled(bool a_check);

  void on_showResult_toggled(bool a_check);

  void on_VisibleHeader_toggled(bool a_check);

  void on_showClear_toggled(bool a_check);

  void on_copyBlue_toggled(bool aCheck);

  void on_copyRed_toggled(bool aCheck);

  void on_copyBlack_toggled(bool aCheck);

  void on_copyBrown_toggled(bool aCheck);

  void on_copyYellow_toggled(bool aCheck);

  void on_copyMagenta_toggled(bool aCheck);

  void on_copyAqua_toggled(bool aCheck);

  void on_moveBlue_toggled(bool aCheck);

  void on_moveRed_toggled(bool aCheck);

  void on_moveBlack_toggled(bool aCheck);

  void on_moveBrown_toggled(bool aCheck);

  void on_moveYellow_toggled(bool aCheck);

  void on_moveMagenta_toggled(bool aCheck);

  void on_moveAqua_toggled(bool aCheck);

  void on_showBlueComp_toggled(bool aCheck);

  void on_showRedComp_toggled(bool aCheck);

  void on_showBlackComp_toggled(bool aCheck);

  void on_showBrownComp_toggled(bool aCheck);

  void on_showYellowComp_toggled(bool aCheck);

  void on_showMagentaComp_toggled(bool aCheck);

  void on_showAquaComp_toggled(bool aCheck);

  void on_showBlueInt_toggled(bool aCheck);

  void on_showRedInt_toggled(bool aCheck);

  void on_showBlackInt_toggled(bool aCheck);

  void on_showBrownInt_toggled(bool aCheck);

  void on_showYellowInt_toggled(bool aCheck);

  void on_showMagentaInt_toggled(bool aCheck);

  void on_showAquaInt_toggled(bool aCheck);

  //union
  void on_showBlueUnion_toggled(bool aCheck);

  void on_showRedUnion_toggled(bool aCheck);

  void on_showBlackUnion_toggled(bool aCheck);

  void on_showBrownUnion_toggled(bool aCheck);

  void on_showYellowUnion_toggled(bool aCheck);

  void on_showMagentaUnion_toggled(bool aCheck);

  void on_showAquaUnion_toggled(bool aCheck);

  void on_showBlueDiff_toggled(bool aCheck);

  void on_showRedDiff_toggled(bool aCheck);

  void on_showBlackDiff_toggled(bool aCheck);

  void on_showBrownDiff_toggled(bool aCheck);

  void on_showYellowDiff_toggled(bool aCheck);

  void on_showMagentaDiff_toggled(bool aCheck);

  void on_showAquaDiff_toggled(bool aCheck);

  //difference
  void on_showBlueSym_Diff_toggled(bool aCheck);

  void on_showRedSym_Diff_toggled(bool aCheck);

  void on_showBlackSym_Diff_toggled(bool aCheck);

  void on_showBrownSym_Diff_toggled(bool aCheck);

  void on_showYellowSym_Diff_toggled(bool aCheck);

  void on_showMagentaSym_Diff_toggled(bool aCheck);

  void on_showAquaSym_Diff_toggled(bool aCheck);

  //minkowski sum
  void on_showBlueMink_Sum_toggled(bool aCheck);

  void on_showRedMink_Sum_toggled(bool aCheck);

  void on_showBlackMink_Sum_toggled(bool aCheck);

  void on_showBrownMink_Sum_toggled(bool aCheck);

  void on_showYellowMink_Sum_toggled(bool aCheck);

  void on_showMagentaMink_Sum_toggled(bool aCheck);

  void on_showAquaMink_Sum_toggled(bool aCheck);

  void on_drawBlue_toggled(bool a_check);

  void on_drawRed_toggled(bool a_check);

  void on_drawBlack_toggled(bool a_check);

  void on_drawBrown_toggled(bool a_check);

  void on_drawYellow_toggled(bool a_check);

  void on_drawMagenta_toggled(bool a_check);

  void on_drawAqua_toggled(bool a_check);

  //about demo,cgal
  void on_aboutBolop_triggered();

  void on_aboutCGAL_triggered();

  void on_aboutMinkop_triggered();

  void on_aboutDemo_triggered();

  void resizeEvent(QResizeEvent *event);

  void exception_handler();

  void on_actionDeleteResult();

  void on_actionDeleteAll_triggered();

  void on_actionClearH_toggled(bool aChecked);

  void on_actionPAN_triggered();

  Polygon_with_holes_2 getMinkInputPolygon(size_t color);

  //for reading polygons from a file
  bool read_linear(QString aFileName, Linear_polygon_set &rSet,
                   Linear_region_source_container &rSources);

  bool read_circular(QString aFileName, Circular_polygon_set &rSet,
                     Circular_region_source_container &rSources);

  bool read_polyline(QString aFilename, Polyline_polygon_set &rSet,
                     Polyline_region_source_container &rSources);

  bool read_bezier(QString aFileName);

  Bezier_curve read_bezier_curve(std::istream &is, bool aDoubleFormat);

signals:
  void changed();

private:
  void get_new_state(size_t operation_name) {
    //operation_name
    // COMPLEMENT_OP = 0
    // INTERSECTION_OP = 1
    // UNION_OP = 2
    // DIFFERENCE_OP = 3
    // SYMMETRIC_DIFFERENCE_OP = 4
    // MINKOWSKI_SUM_OP = 5
    // RESET_OP = 6
    // COPY_OP = 7
    // MOVE_OP = 8
    if (operation_name == 12) {
      State_current m_state = State_current(operation_name, active_type());
      states_stack.push_back(m_state);
      for (auto si = states_stack.back().m_curve_sets.begin();
           si != states_stack.back().m_curve_sets.end(); ++si)
      { link_GI(si->gi()); }
    }
    else {
      State_current m_state_x = State_current(operation_name, active_type());

      for (int i = 0; i < 8; i++) {
        m_state_x.m_curve_sets[i].assign(states_stack.back().m_curve_sets[i]);
      }

      modelChanged();

      for (auto si = states_stack.back().m_curve_sets.begin();
           si != states_stack.back().m_curve_sets.end(); ++si) { unlink_GI(si->gi()); }

      modelChanged();

      states_stack.push_back(m_state_x);

      for (auto si = states_stack.back().m_curve_sets.begin();
           si != states_stack.back().m_curve_sets.end(); ++si) { link_GI(si->gi()); }

      modelChanged();
    }

    m_state_num = states_stack.size();
    actionUndo->setEnabled(true);

    // for confirming the num of each state during debugging
    //show_warning("State num: "+to_string(states_stack.size()));
    modelChanged();
  }

  void modelChanged() { emit(changed()); }

  //warning message for user
  bool ask_user_yesno(const char* aTitle, const char* aQuestion) {
    return QMessageBox::warning(this, aTitle, QString(aQuestion),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No) == QMessageBox::Yes;
  }

  //warning message for user
  void ask_user_ok(std::string aTitle, std::string aQuestion) {
    QMessageBox::warning(this, QString(aTitle.c_str()), QString(aQuestion.c_str()));
  }

  //functions for setting the color of polygon
  void SetViewBlue(bool a_check) { showBlue->setChecked(a_check); }

  void SetViewRed(bool a_check) { showRed->setChecked(a_check); }

  void SetViewBlack(bool a_check) { showBlack->setChecked(a_check); }

  void SetViewBrown(bool a_check) { showBrown->setChecked(a_check); }

  void SetViewYellow(bool a_check) { showYellow->setChecked(a_check); }

  void SetViewMagenta(bool a_check) { showMagenta->setChecked(a_check); }

  void SetViewAqua(bool a_check) { showAqua->setChecked(a_check); }

  void SetViewResult(bool a_check) { showResult->setChecked(a_check); }

  //changes the set of polygons of a specific type

  void link_GI(CGAL::Qt::GraphicsItem *aGI) {
    QObject::connect(this, SIGNAL(changed()), aGI, SLOT(modelChanged()));
    m_scene.addItem(aGI);
  }

  void unlink_GI(CGAL::Qt::GraphicsItem *aGI) {
    m_scene.removeItem(aGI);
    QObject::disconnect(this, SIGNAL(changed()), aGI, SLOT(modelChanged()));
  }

  void switch_set_type(Curve_set &aSet, int aType);

  void switch_sets_type(int aType);

  bool ensure_circular_mode();

  bool ensure_bezier_mode();

  bool ensure_linear_mode();

  bool ensure_polyline_mode();

  //bool ensure_mink_mode();

};

//constructor for MainWindow
MainWindow::MainWindow() :
  DemosMainWindow(),
  m_bezier_active(false), //default
  m_circular_active(false), //default
  m_polyline_active(false), //default
  m_color_active(0), //default
  m_color_move(1111), //default
  m_color_copy(1111),//default
  m_color_move_ext(111),//default
  m_color_cm(1111), //default
  m_color_visible(7), //default
  m_color_complement(0), //default
  m_blue_int(true), //default
  minkowski_sum_operated(false), //default
  m_red_int(true), //default
  m_black_int(false), //default
  m_brown_int(false), //default
  m_yellow_int(false), //default
  m_magenta_int(false), //default
  m_aqua_int(false), //default
  m_blue_union(true), //default
  m_red_union(true), //default
  m_black_union(false), //default
  m_brown_union(false), //default
  m_yellow_union(false), //default
  m_magenta_union(false), //default
  m_aqua_union(false), //default
  m_blue_sym_diff(true), //default
  m_red_sym_diff(true), //default
  m_black_sym_diff(false), //default
  m_brown_sym_diff(false), //default
  m_yellow_sym_diff(false), //default
  m_magenta_sym_diff(false), //default
  m_aqua_sym_diff(false), //default
  m_blue_mink(true), //default
  m_red_mink(true), //default
  m_black_mink(false), //default
  m_brown_mink(false), //default
  m_yellow_mink(false), //default
  m_magenta_mink(false), //default
  m_aqua_mink(false), //default
  m_visible_black(true), //default
  m_visible_brown(false), //default
  m_visible_yellow(false), //default
  m_visible_magenta(false), //default
  m_visible_aqua(false), //default
  empty_warn(true), // default
  operand_error_warn(true), // default
  m_disjoint(false), //default
  m_pan(false),
  m_grid(false),
  m_reset(false),
  m_state_num(0) {
  CGAL::set_error_handler(error_handler);
  CGAL::set_warning_handler(error_handler);
  CGAL::set_error_behaviour(CGAL::CONTINUE);

  setupUi(this);

  // Make the check boxes and radio buttons of the Operations panel visible on
  // dark themes.
  auto* indicator_style = new Visible_indicator_style;
  indicator_style->setParent(this);
  for (auto* box : sceneDockWidget->findChildren<QCheckBox*>())
    box->setStyle(indicator_style);
  for (auto* button : sceneDockWidget->findChildren<QRadioButton*>())
    button->setStyle(indicator_style);

  // The Log panel: the program (the executed statements) and a button that
  // saves it as a Python script.
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
    QObject::connect(save_button, &QPushButton::clicked, this,
                     [this]() { save_program(); });
  }

  // The initial buckets.
  log_statements({"red = Polygon_set_2()", "black = Polygon_set_2()",
                  "blue = Polygon_set_2()"}, true);

  // The Console panel: the output, and an input line with a prompt.
  {
    auto* panel = new QWidget;
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(4, 4, 4, 4);
    QFont fixed = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_console_output = new QPlainTextEdit;
    m_console_output->setReadOnly(true);
    m_console_output->setFont(fixed);
    m_console_output->appendPlainText(
      "Type Python-style statements on the polygon sets, e.g.,\n"
      "  red.join(Polygon_2([Point_2(0, 0), Point_2(40, 0), Point_2(0, 30)]))\n"
      "  blue.intersection(red, black)\n"
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
    if (QWidget* old_panel = consoleDockWidget->widget())
      old_panel->deleteLater();
    consoleDockWidget->setWidget(panel);
    QObject::connect(m_console_input, &QLineEdit::returnPressed, this, [this]() {
      QString line = m_console_input->text();
      m_console_input->clear();
      run_console_line(line);
    });
    m_console_input->installEventFilter(this);
  }

  // Grid: show/hide, spacing, and snapping of clicked points to the grid.
  QObject::connect(actionGrid, &QAction::toggled, this,
                   [this](bool a_check) { m_scene.set_grid_visible(a_check); });
  auto* grid_snapper = new Grid_snapper(&m_scene, this);
  qApp->installEventFilter(grid_snapper);
  QObject::connect(actionSnapToGrid, &QAction::toggled, this,
                   [grid_snapper](bool a_check)
                   { grid_snapper->set_enabled(a_check); });
  // Grid density: step through the spacings below. A denser grid has a
  // smaller spacing. The menu items show the spacing they lead to, and are
  // disabled at the ends of the range.
  static const std::array<double, 7> spacings = {1, 2, 5, 10, 20, 50, 100};
  auto update_density_actions = [this]() {
    auto it = std::find(spacings.begin(), spacings.end(),
                        m_scene.grid_spacing());
    bool denser = (it != spacings.begin());
    bool sparser = (it + 1 != spacings.end());
    actionGridDenser->setEnabled(denser);
    actionGridSparser->setEnabled(sparser);
    actionGridDenser->setText(denser ?
      QString("&Increase Grid Density (Spacing %1)").arg(*(it - 1)) :
      QString("&Increase Grid Density"));
    actionGridSparser->setText(sparser ?
      QString("&Decrease Grid Density (Spacing %1)").arg(*(it + 1)) :
      QString("&Decrease Grid Density"));
  };
  auto change_density = [this, update_density_actions](int aDirection) {
    auto it = std::find(spacings.begin(), spacings.end(),
                        m_scene.grid_spacing());
    it += aDirection;
    if ((it < spacings.begin()) || (it >= spacings.end())) return;
    m_scene.set_grid_spacing(*it);
    update_density_actions();
    statusBar()->showMessage(QString("Grid spacing: %1").arg(*it), 3000);
  };
  QObject::connect(actionGridDenser, &QAction::triggered, this,
                   [change_density]() { change_density(-1); });
  QObject::connect(actionGridSparser, &QAction::triggered, this,
                   [change_density]() { change_density(+1); });
  update_density_actions();

  // Exactly one bucket is the operand of the complement operation: make the
  // Complement column exclusive, so clicking the marked cell is a no-op.
  auto* complement_group = new QButtonGroup(this);
  complement_group->setExclusive(true);
  for (auto* box : {showBlueComp, showRedComp, showBlackComp, showBrownComp,
                    showYellowComp, showMagentaComp, showAquaComp})
    complement_group->addButton(box);

  // In the Intersection and Union columns at least one bucket stays
  // unchecked, to hold the result, and the operations are enabled only with
  // at least one operand.
  m_complement_column = {showBlueComp, showRedComp, showBlackComp,
                         showBrownComp, showYellowComp, showMagentaComp,
                         showAquaComp};
  m_difference_column = {showBlueDiff, showRedDiff, showBlackDiff,
                         showBrownDiff, showYellowDiff, showMagentaDiff,
                         showAquaDiff};
  m_symmetric_difference_column =
    {showBlueSym_Diff, showRedSym_Diff, showBlackSym_Diff, showBrownSym_Diff,
     showYellowSym_Diff, showMagentaSym_Diff, showAquaSym_Diff};
  m_intersection_column = {showBlueInt, showRedInt, showBlackInt, showBrownInt,
                           showYellowInt, showMagentaInt, showAquaInt};
  m_union_column = {showBlueUnion, showRedUnion, showBlackUnion,
                    showBrownUnion, showYellowUnion, showMagentaUnion,
                    showAquaUnion};
  const std::pair<const std::array<QAbstractButton*, 7>*, QAbstractButton*>
    multi_operand_columns[] = {{&m_intersection_column, actionIntersectionH},
                               {&m_union_column, actionUnionH}};
  for (const auto& [column, operation] : multi_operand_columns) {
    for (auto* box : *column)
      QObject::connect(box, &QAbstractButton::toggled, this,
                       [this, box, column, operation](bool checked) {
                         limit_to_all_but_one(*column, box, checked,
                                              operation);
                       });
    // Set the initial state of the operation.
    limit_to_all_but_one(*column, (*column)[0], false, operation);
  }

  // The Difference, Symmetric Difference, and Minkowski Sum columns have at
  // most two marked cells, and their operations are enabled only with exactly
  // two.
  using Column = std::array<QAbstractButton*, 7>;
  const std::pair<Column, QAbstractButton*> two_operand_columns[] = {
    {{showBlueDiff, showRedDiff, showBlackDiff, showBrownDiff,
      showYellowDiff, showMagentaDiff, showAquaDiff},
     actionDifferenceH},
    {{showBlueSym_Diff, showRedSym_Diff, showBlackSym_Diff, showBrownSym_Diff,
      showYellowSym_Diff, showMagentaSym_Diff, showAquaSym_Diff},
     actionSymmetric_DifferenceH},
    {{showBlueMink_Sum, showRedMink_Sum, showBlackMink_Sum, showBrownMink_Sum,
      showYellowMink_Sum, showMagentaMink_Sum, showAquaMink_Sum},
     actionMinkowski_SumH}
  };
  for (const auto& [column, operation] : two_operand_columns) {
    for (auto* box : column)
      QObject::connect(box, &QAbstractButton::toggled, this,
                       [this, box, column, operation](bool checked) {
                         limit_to_two_operands(column, box, checked,
                                               operation);
                       });
    // Set the initial state of the operation.
    limit_to_two_operands(column, column[0], false, operation);
  }

  // Fit the Visible and Draw/Insert header images into the same icon area as
  // the operation buttons next to them (27x25), keeping their aspect ratio.
  for (auto* label : {VisibleIcon, DrawHeader}) {
    QPixmap pixmap = label->pixmap();
    if (! pixmap.isNull())
      label->setPixmap(pixmap.scaled(27, 25, Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
  }

  //
  // Setup the m_scene and the view
  //
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


  //this->on_actionInsertLinear_triggered();

  // Turn the vertical axis upside down
  this->graphicsView->scale(2.5, -2.5);

  //adding basic setups

  //todo:need to be given finishing touch
  //The navigation adds zooming and translation functionality to the
  //QGraphicsView.
  this->addNavigation(this->graphicsView);
  // Every change of the view (zoom, pan, resize) ends in a repaint, where the
  // area that unbounded sets are drawn over is updated.
  this->graphicsView->viewport()->installEventFilter(this);

  //setting the menus
  this->setupStatusBar();
  this->setupOptionsMenu();

  //adding files to select out of polygon
  this->addRecentFiles(this->menuFile, this->actionClose);


  /*  operation_name
  // COMPLEMENT_OP = 0
  // INTERSECTION_OP = 1
  // UNION_OP = 2
  // DIFFERENCE_OP = 3
  // SYMMETRIC_DIFFERENCE_OP = 4
  // MINKOWSKI_SUM_OP = 5
  // RESET_OP = 6
  // COPY_OP = 7
  // MOVE_OP = 8
  // CLEAR_OP = 9
  // DELETEALL_OP = 10
  // START_OP = 11
  */

  //initializing and pushing first state in States_stack
  get_new_state(12);
  actionUndo->setEnabled(false);

  this->setWindowState(Qt::WindowMaximized);

  //! \todo what is it?
  // actionUse_OpenGL->setVisible(false);

  //initializing classes to draw respective polygons using mouse
  m_bezier_input = new CGAL::Qt::GraphicsViewBezierPolygonInput<Bezier_traits>(this, &m_scene);
  m_linear_input = new CGAL::Qt::Graphics_view_linear_polygon_input<Kernel>(this, &m_scene);
  m_circular_input = new CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>(this, &m_scene);
  m_polyline_input = new CGAL::Qt::Graphics_view_polyline_input<Kernel>(this, &m_scene);
  m_conic_input = new CGAL::Qt::Graphics_view_conic_input(this, &m_scene);

  // The curve tool bar, underneath the curve families: the ways to construct
  // curves of the selected family.
  {
    using Circular_input =
      CGAL::Qt::Graphics_view_circular_polygon_input<Kernel>;
    auto* bar = new QToolBar;
    bar->setIconSize(QSize(24, 24));
    auto* group = new QActionGroup(this);
    auto* arcs = bar->addAction(curve_tool_icon("arcs"),
                                "Line Segments and Circular Arcs");
    arcs->setToolTip("Line Segments and Circular Arcs: click at each vertex; "
                     "press and drag to bend a piece into an arc through the "
                     "cursor; right-click to close (drag to bend the last "
                     "piece)");
    auto* circle = bar->addAction(curve_tool_icon("full circle"), "Circle");
    circle->setToolTip("Circle: press at the center and drag to the radius");
    for (auto* action : {arcs, circle}) {
      action->setCheckable(true);
      group->addAction(action);
    }
    arcs->setChecked(true);
    QObject::connect(circle, &QAction::toggled, this, [this](bool aChecked) {
      m_circular_input->set_mode(aChecked ? Circular_input::Circle_mode :
                                 Circular_input::Arcs);
    });

    // The conic tools: conics that bound a polygon, full curves, and a conic
    // given by its coefficients.
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
    const QString boundary_help =
      ". Every conic you add meets the previous one at the next vertex (click "
      "at it when there are several); on a circle or an ellipse, click near "
      "the arc to keep; right-click to close the boundary. Escape or Backspace "
      "discards the conic under construction, or else removes the last "
      "conic.";
    std::vector<QAction*> conic_actions;
    for (const auto& [icon, text, mode, tip] : conic_tools) {
      if (mode == Conic_input::FULL_CIRCLE)
        conic_actions.push_back(bar->addSeparator());
      auto* action = bar->addAction(curve_tool_icon(icon), text);
      action->setToolTip((mode < Conic_input::FULL_CIRCLE) ?
                         tip + boundary_help : tip);
      action->setCheckable(true);
      conic_group->addAction(action);
      Conic_input::Mode m = mode;
      QObject::connect(action, &QAction::triggered, this,
                       [this, m]() { m_conic_input->set_mode(m); });
      conic_actions.push_back(action);
    }
    conic_actions[0]->setChecked(true);
    conic_actions.push_back(bar->addSeparator());
    auto* coefficients =
      bar->addAction(curve_tool_icon("coefficients"), "Coefficients...");
    coefficients->setToolTip("Coefficients: enter a conic "
                             "r x\u00b2 + s y\u00b2 + t xy + u x + v y + w = 0, "
                             "and add it to the boundary or construct the full "
                             "ellipse");
    QObject::connect(coefficients, &QAction::triggered, this,
                     [this]() { m_conic_input->enter_coefficients(this); });
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


  //connecting GUI and the code base
  QObject::connect(m_linear_input, SIGNAL(generate(CGAL::Object)), this,
                   SLOT(processInput(CGAL::Object)));
  //QObject::connect(m_linear_input, SIGNAL(generate(std::variant<Linear_polygon>)), this,
  //                 SLOT(processInput(std::variant<Linear_polygon>)));
  QObject::connect(m_circular_input, SIGNAL(generate(CGAL::Object)), this,
                   SLOT(processInput(CGAL::Object)));
  QObject::connect(m_bezier_input, SIGNAL(generate(CGAL::Object)), this,
                   SLOT(processInput(CGAL::Object)));
  QObject::connect(m_conic_input, SIGNAL(generate(CGAL::Object)), this,
                   SLOT(processInput(CGAL::Object)));
  QObject::connect(m_polyline_input, SIGNAL(generate(CGAL::Object)), this,
                   SLOT(processInput(CGAL::Object)));

  m_scene.installEventFilter(m_linear_input);

  // Under Wayland, let the compositor move floating dock widgets.
  if (QGuiApplication::platformName().startsWith("wayland")) {
    auto* dock_mover = new Floating_dock_mover(this);
    for (auto* dock : findChildren<QDockWidget*>())
      dock->installEventFilter(dock_mover);
  }

  QObject::connect(this->actionQuit, SIGNAL(triggered()), this,
                   SLOT(close()));

  //for file handling
  QObject::connect(this, SIGNAL(openRecentFile(QString)), this,
                   SLOT(open(QString)));

  //drawing polygons of various colors
  QObject::connect(drawBlue, SIGNAL(toggled(bool)), this,
                   SLOT(on_drawBlue_toggled(bool)));
  QObject::connect(drawRed, SIGNAL(toggled(bool)), this,
                   SLOT(on_drawRed_toggled(bool)));
  QObject::connect(drawBlack, SIGNAL(toggled(bool)), this,
                   SLOT(on_drawBlack_toggled(bool)));
  QObject::connect(drawBrown, SIGNAL(toggled(bool)), this,
                   SLOT(on_drawBrown_toggled(bool)));
  QObject::connect(drawYellow, SIGNAL(toggled(bool)), this,
                   SLOT(on_drawYellow_toggled(bool)));
  QObject::connect(drawMagenta, SIGNAL(toggled(bool)), this,
                   SLOT(on_drawMagenta_toggled(bool)));
  QObject::connect(drawAqua, SIGNAL(toggled(bool)), this,
                   SLOT(on_drawAqua_toggled(bool)));

  QObject::connect(showBlue, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlue_toggled(bool)));
  QObject::connect(showRed, SIGNAL(toggled(bool)), this,
                   SLOT(on_showRed_toggled(bool)));
  QObject::connect(showBlack, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlack_toggled(bool)));
  QObject::connect(showBrown, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBrown_toggled(bool)));
  QObject::connect(showYellow, SIGNAL(toggled(bool)), this,
                   SLOT(on_showYellow_toggled(bool)));
  QObject::connect(showMagenta, SIGNAL(toggled(bool)), this,
                   SLOT(on_showMagenta_toggled(bool)));
  QObject::connect(showAqua, SIGNAL(toggled(bool)), this,
                   SLOT(on_showAqua_toggled(bool)));
  QObject::connect(showResult, SIGNAL(toggled(bool)), this,
                   SLOT(on_showResult_toggled(bool)));

  /* QObject::connect(sceneDockWidget, SIGNAL(visibilityChanged(bool)), this,
  //                  SLOT(on_sceneDockWidget_visibilityChanged(bool)));
  // QObject::connect(infoDockWidget, SIGNAL(visibilityChanged(bool)), this,
  //                  SLOT(on_infoDockWidget_visibilityChanged(bool)));
  // QObject::connect(consoleDockWidget, SIGNAL(visibilityChanged(bool)), this,
  SLOT(on_consoleDockWidget_visibilityChanged(bool)));*/

  //showing all the side panels
  //todo::implement the console and the information channel quickly
  QObject::connect(showColorBucket, SIGNAL(toggled(bool)), this,
                   SLOT(on_showColorBucket_toggled(bool)));
  QObject::connect(showConsole, SIGNAL(toggled(bool)), this,
                   SLOT(on_showConsole_toggled(bool)));
  QObject::connect(actionErrorOperand, &QAction::toggled, this,
                   [this](bool a_check) { operand_error_warn = a_check; });
  QObject::connect(actionWarnNotEmpty, &QAction::toggled, this,
                   [this](bool a_check) { empty_warn = a_check; });
  QObject::connect(showInfo, SIGNAL(toggled(bool)), this,
                   SLOT(on_showInfo_toggled(bool)));

  QObject::connect(showClear, SIGNAL(toggled(bool)), this,
                   SLOT(on_showClear_toggled(bool)));
  QObject::connect(VisibleHeader, SIGNAL(toggled(bool)), this,
                   SLOT(on_VisibleHeader_toggled(bool)));

  //copy paste for all polygons(not a boolean operation)
  QObject::connect(copyBlue, SIGNAL(toggled(bool)), this,
                   SLOT(on_copyBlue_toggled(bool)));
  QObject::connect(copyRed, SIGNAL(toggled(bool)), this,
                   SLOT(on_copyRed_toggled(bool)));
  QObject::connect(copyBlack, SIGNAL(toggled(bool)), this,
                   SLOT(on_copyBlack_toggled(bool)));
  QObject::connect(copyBrown, SIGNAL(toggled(bool)), this,
                   SLOT(on_copyBrown_toggled(bool)));
  QObject::connect(copyYellow, SIGNAL(toggled(bool)), this,
                   SLOT(on_copyYellow_toggled(bool)));
  QObject::connect(copyMagenta, SIGNAL(toggled(bool)), this,
                   SLOT(on_copyMagenta_toggled(bool)));
  QObject::connect(copyAqua, SIGNAL(toggled(bool)), this,
                   SLOT(on_copyAqua_toggled(bool)));


  //move button usage (not an operation)
  QObject::connect(moveBlue, SIGNAL(toggled(bool)), this,
                   SLOT(on_moveBlue_toggled(bool)));
  QObject::connect(moveRed, SIGNAL(toggled(bool)), this,
                   SLOT(on_moveRed_toggled(bool)));
  QObject::connect(moveBlack, SIGNAL(toggled(bool)), this,
                   SLOT(on_moveBlack_toggled(bool)));
  QObject::connect(moveBrown, SIGNAL(toggled(bool)), this,
                   SLOT(on_moveBrown_toggled(bool)));
  QObject::connect(moveYellow, SIGNAL(toggled(bool)), this,
                   SLOT(on_moveYellow_toggled(bool)));
  QObject::connect(moveMagenta, SIGNAL(toggled(bool)), this,
                   SLOT(on_moveMagenta_toggled(bool)));
  QObject::connect(moveAqua, SIGNAL(toggled(bool)), this,
                   SLOT(on_moveAqua_toggled(bool)));


  //complement operation
  QObject::connect(showBlueComp, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlueComp_toggled(bool)));
  QObject::connect(showRedComp, SIGNAL(toggled(bool)), this,
                   SLOT(on_showRedComp_toggled(bool)));
  QObject::connect(showBlackComp, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlackComp_toggled(bool)));
  QObject::connect(showBrownComp, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBrownComp_toggled(bool)));
  QObject::connect(showYellowComp, SIGNAL(toggled(bool)), this,
                   SLOT(on_showYellowComp_toggled(bool)));
  QObject::connect(showMagentaComp, SIGNAL(toggled(bool)), this,
                   SLOT(on_showMagentaComp_toggled(bool)));
  QObject::connect(showAquaComp, SIGNAL(toggled(bool)), this,
                   SLOT(on_showAquaComp_toggled(bool)));

  //intersection operation
  QObject::connect(showBlueInt, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlueInt_toggled(bool)));
  QObject::connect(showRedInt, SIGNAL(toggled(bool)), this,
                   SLOT(on_showRedInt_toggled(bool)));
  QObject::connect(showBlackInt, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlackInt_toggled(bool)));
  QObject::connect(showBrownInt, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBrownInt_toggled(bool)));
  QObject::connect(showYellowInt, SIGNAL(toggled(bool)), this,
                   SLOT(on_showYellowInt_toggled(bool)));
  QObject::connect(showMagentaInt, SIGNAL(toggled(bool)), this,
                   SLOT(on_showMagentaInt_toggled(bool)));
  QObject::connect(showAquaInt, SIGNAL(toggled(bool)), this,
                   SLOT(on_showAquaInt_toggled(bool)));


  //union operation
  QObject::connect(showBlueUnion, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlueUnion_toggled(bool)));
  QObject::connect(showRedUnion, SIGNAL(toggled(bool)), this,
                   SLOT(on_showRedUnion_toggled(bool)));
  QObject::connect(showBlackUnion, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlackUnion_toggled(bool)));
  QObject::connect(showBrownUnion, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBrownUnion_toggled(bool)));
  QObject::connect(showYellowUnion, SIGNAL(toggled(bool)), this,
                   SLOT(on_showYellowUnion_toggled(bool)));
  QObject::connect(showMagentaUnion, SIGNAL(toggled(bool)), this,
                   SLOT(on_showMagentaUnion_toggled(bool)));
  QObject::connect(showAquaUnion, SIGNAL(toggled(bool)), this,
                   SLOT(on_showAquaUnion_toggled(bool)));

  //difference operation
  QObject::connect(showBlueDiff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlueDiff_toggled(bool)));
  QObject::connect(showRedDiff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showRedDiff_toggled(bool)));
  QObject::connect(showBlackDiff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlackDiff_toggled(bool)));
  QObject::connect(showBrownDiff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBrownDiff_toggled(bool)));
  QObject::connect(showYellowDiff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showYellowDiff_toggled(bool)));
  QObject::connect(showMagentaDiff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showMagentaDiff_toggled(bool)));
  QObject::connect(showAquaDiff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showAquaDiff_toggled(bool)));


  //sym_difference operation
  QObject::connect(showBlueSym_Diff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlueSym_Diff_toggled(bool)));
  QObject::connect(showRedSym_Diff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showRedSym_Diff_toggled(bool)));
  QObject::connect(showBlackSym_Diff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlackSym_Diff_toggled(bool)));
  QObject::connect(showBrownSym_Diff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBrownSym_Diff_toggled(bool)));
  QObject::connect(showYellowSym_Diff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showYellowSym_Diff_toggled(bool)));
  QObject::connect(showMagentaSym_Diff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showMagentaSym_Diff_toggled(bool)));
  QObject::connect(showAquaSym_Diff, SIGNAL(toggled(bool)), this,
                   SLOT(on_showAquaSym_Diff_toggled(bool)));


  //minkowski_sum
  QObject::connect(showBlueMink_Sum, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlueMink_Sum_toggled(bool)));
  QObject::connect(showRedMink_Sum, SIGNAL(toggled(bool)), this,
                   SLOT(on_showRedMink_Sum_toggled(bool)));
  QObject::connect(showBlackMink_Sum, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBlackMink_Sum_toggled(bool)));
  QObject::connect(showBrownMink_Sum, SIGNAL(toggled(bool)), this,
                   SLOT(on_showBrownMink_Sum_toggled(bool)));
  QObject::connect(showYellowMink_Sum, SIGNAL(toggled(bool)), this,
                   SLOT(on_showYellowMink_Sum_toggled(bool)));
  QObject::connect(showMagentaMink_Sum, SIGNAL(toggled(bool)), this,
                   SLOT(on_showMagentaMink_Sum_toggled(bool)));
  QObject::connect(showAquaMink_Sum, SIGNAL(toggled(bool)), this,
                   SLOT(on_showAquaMink_Sum_toggled(bool)));

  //clear
  this->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

  // axis
  QPen *dashedLine{new QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)};
  // dashedLine->setCosmetic(true);
  dashedLine->setWidth(0);
  xAxis->setLine(-155000000000000, 0, 155000000000000, 0);
  xAxis->setPen(*dashedLine);
  yAxis->setLine(0, -100000000000000, 0, 100000000000000);
  yAxis->setPen(*dashedLine);

  /* m_scene.addLine(0,-10000000,0,10000000, QPen(Qt::black));
  // m_scene.addLine(-15500000,0,15500000,0, QPen(Qt::black));

  // // Add the vertical lines first, paint them red
  // for (int x=-15500000; x<=15500000; x+=50)
  //     m_scene.addLine(x,-10000000,x,10000000, QPen(Qt::black));

  // // Now add the horizontal lines, paint them green
  // for (int y=-10000000; y<=10000000; y+=50)
  /     m_scene.addLine(-15500000,y,15500000,y, QPen(Qt::black));
  */

  m_linear_input->mOngoingPieceGI->setPen(sPens[0]);
  m_linear_input->mLinearGI->setPen(sPens[0]);
  m_linear_input->mHandleGI->setPen(sPens[0]);
  actionOpenLinear->setEnabled(true);
  actionOpenDXF->setEnabled(false);
  actionOpenBezier->setEnabled(false);
  actionOpenPolyline->setEnabled(false);

  states_stack.back().blue_set().clear();
  states_stack.back().red_set().clear();
  states_stack.back().black_set().clear();
  states_stack.back().brown_set().clear();
  states_stack.back().yellow_set().clear();
  states_stack.back().magenta_set().clear();
  states_stack.back().aqua_set().clear();
}

//started defining Main Window class functions
void MainWindow::on_showBlue_toggled(bool a_check) {
  states_stack.back().ToggleView(BLUE_GROUP, a_check);
  if (a_check) {
    if (m_color_visible == 0) {
      m_scene.installEventFilter(current_input());
    }
    m_color_visible++;
    drawBlue->setEnabled(true);
  } else {
    m_color_visible--;
    if (showRed->isChecked()) drawRed->setChecked(true);
    else if (showBlack->isChecked()) drawBlack->setChecked(true);
    else if (showBrown->isChecked()) drawBrown->setChecked(true);
    else if (showYellow->isChecked()) drawYellow->setChecked(true);
    else if (showMagenta->isChecked()) drawMagenta->setChecked(true);
    else if (showAqua->isChecked()) drawAqua->setChecked(true);
    else {
      remove_inputs();
    }
    drawBlue->setEnabled(false);
  }
  if (m_color_visible == 7) VisibleHeader->setChecked(true);
  //else VisibleHeader->setChecked(false);
}

void MainWindow::on_showRed_toggled(bool a_check) {
  states_stack.back().ToggleView(RED_GROUP, a_check);
  if (a_check) {
    if (m_color_visible == 0) {
      m_scene.installEventFilter(current_input());
    }
    m_color_visible++;
    drawRed->setEnabled(true);
  } else {
    m_color_visible--;
    if (showBlue->isChecked()) drawBlue->setChecked(true);
    else if (showBlack->isChecked()) drawBlack->setChecked(true);
    else if (showBrown->isChecked()) drawBrown->setChecked(true);
    else if (showYellow->isChecked()) drawYellow->setChecked(true);
    else if (showMagenta->isChecked()) drawMagenta->setChecked(true);
    else if (showAqua->isChecked()) drawAqua->setChecked(true);
    else {
      remove_inputs();
    }
    drawRed->setEnabled(false);
  }
  if (m_color_visible == 7) VisibleHeader->setChecked(true);
  // else VisibleHeader->setChecked(false);
}

void MainWindow::on_showBlack_toggled(bool a_check) {
  states_stack.back().ToggleView(BLACK_GROUP, a_check);
  if (a_check) {
    if (m_color_visible == 0) {
      m_scene.installEventFilter(current_input());
    }
    m_color_visible++;
    drawBlack->setEnabled(true);
  } else {
    m_color_visible--;
    if (showBlue->isChecked()) drawBlue->setChecked(true);
    else if (showRed->isChecked()) drawRed->setChecked(true);
    else if (showBrown->isChecked()) drawBrown->setChecked(true);
    else if (showYellow->isChecked()) drawYellow->setChecked(true);
    else if (showMagenta->isChecked()) drawMagenta->setChecked(true);
    else if (showAqua->isChecked()) drawAqua->setChecked(true);
    else {
      remove_inputs();
    }
    drawBlack->setEnabled(false);
  }
  if (m_color_visible == 7) VisibleHeader->setChecked(true);
  //else VisibleHeader->setChecked(false);
}

void MainWindow::on_showBrown_toggled(bool a_check) {
  states_stack.back().ToggleView(BROWN_GROUP, a_check);
  if (a_check) {
    if (m_color_visible == 0) {
      m_scene.installEventFilter(current_input());
    }
    m_color_visible++;
    drawBrown->setEnabled(true);
  } else {
    m_color_visible--;
    if (showBlue->isChecked()) drawBlue->setChecked(true);
    else if (showRed->isChecked()) drawRed->setChecked(true);
    else if (showBlack->isChecked()) drawBlack->setChecked(true);
    else if (showYellow->isChecked()) drawYellow->setChecked(true);
    else if (showMagenta->isChecked()) drawMagenta->setChecked(true);
    else if (showAqua->isChecked()) drawAqua->setChecked(true);
    else {
      remove_inputs();
    }
    drawBrown->setEnabled(false);
  }
  if (m_color_visible == 7) VisibleHeader->setChecked(true);
  //else VisibleHeader->setChecked(false);
}

void MainWindow::on_showYellow_toggled(bool a_check) {
  states_stack.back().ToggleView(YELLOW_GROUP, a_check);
  if (a_check) {
    if (m_color_visible == 0) {
      m_scene.installEventFilter(current_input());
    }
    m_color_visible++;
    drawYellow->setEnabled(true);
  } else {
    m_color_visible--;
    if (showBlue->isChecked()) drawBlue->setChecked(true);
    else if (showRed->isChecked()) drawRed->setChecked(true);
    else if (showBlack->isChecked()) drawBlack->setChecked(true);
    else if (showBrown->isChecked()) drawBrown->setChecked(true);
    else if (showMagenta->isChecked()) drawMagenta->setChecked(true);
    else if (showAqua->isChecked()) drawAqua->setChecked(true);
    else {
      remove_inputs();
    }
    drawYellow->setEnabled(false);
  }
  if (m_color_visible == 7) VisibleHeader->setChecked(true);
  //else VisibleHeader->setChecked(false);
}

void MainWindow::on_showMagenta_toggled(bool a_check) {
  states_stack.back().ToggleView(MAGENTA_GROUP, a_check);
  if (a_check) {
    if (m_color_visible == 0) {
      m_scene.installEventFilter(current_input());
    }
    m_color_visible++;
    drawMagenta->setEnabled(true);
  } else {
    m_color_visible--;
    if (showBlue->isChecked()) drawBlue->setChecked(true);
    else if (showRed->isChecked()) drawRed->setChecked(true);
    else if (showBlack->isChecked()) drawBlack->setChecked(true);
    else if (showBrown->isChecked()) drawBrown->setChecked(true);
    else if (showYellow->isChecked()) drawYellow->setChecked(true);
    else if (showAqua->isChecked()) drawAqua->setChecked(true);
    else {
      remove_inputs();
    }
    drawMagenta->setEnabled(false);
  }
  if (m_color_visible == 7) VisibleHeader->setChecked(true);
  //else VisibleHeader->setChecked(false);
}

void MainWindow::on_showAqua_toggled(bool a_check) {
  states_stack.back().ToggleView(AQUA_GROUP, a_check);
  if (a_check) {
    if (m_color_visible == 0) {
      m_scene.installEventFilter(current_input());
    }
    m_color_visible++;
    drawAqua->setEnabled(true);
  } else {
    m_color_visible--;
    if (showBlue->isChecked()) drawBlue->setChecked(true);
    else if (showRed->isChecked()) drawRed->setChecked(true);
    else if (showBlack->isChecked()) drawBlack->setChecked(true);
    else if (showBrown->isChecked()) drawBrown->setChecked(true);
    else if (showYellow->isChecked()) drawYellow->setChecked(true);
    else if (showMagenta->isChecked()) drawMagenta->setChecked(true);
    else {
      remove_inputs();
    }
    drawAqua->setEnabled(false);
  }
  if (m_color_visible == 7) VisibleHeader->setChecked(true);
  //else VisibleHeader->setChecked(false);
}

void MainWindow::on_showResult_toggled(bool a_check) { states_stack.back().ToggleView(RESULT_GROUP, a_check); }

void MainWindow::on_VisibleHeader_toggled(bool a_check) {
  if (a_check) {
    showRed->setChecked(true);
    showBlue->setChecked(true);
    showBlack->setChecked(true);
    showBrown->setChecked(true);
    showYellow->setChecked(true);
    showMagenta->setChecked(true);
    showAqua->setChecked(true);
    drawBlue->setChecked(true);
  } else {
    showRed->setChecked(false);
    showBlue->setChecked(false);
    showBlack->setChecked(false);
    showBrown->setChecked(false);
    showYellow->setChecked(false);
    showMagenta->setChecked(false);
    showAqua->setChecked(false);
  }
}

//clear button
void MainWindow::on_showClear_toggled(bool a_check) {
  if (a_check) {
    clearRed->setChecked(true);
    clearBlue->setChecked(true);
    clearBrown->setChecked(true);
    clearYellow->setChecked(true);
    clearMagenta->setChecked(true);
    clearAqua->setChecked(true);
    clearBlack->setChecked(true);
  } else {
    clearRed->setChecked(false);
    clearBlue->setChecked(false);
    clearBrown->setChecked(false);
    clearYellow->setChecked(false);
    clearMagenta->setChecked(false);
    clearAqua->setChecked(false);
    clearBlack->setChecked(false);
  }
}

void MainWindow::on_sceneDockWidget_visibilityChanged(bool a_check) {
  if (a_check) showColorBucket->setChecked(true);
  else if (!this->windowState().testFlag(Qt::WindowMinimized)) {
    //sceneDockWidget->setVisible(false);
    showColorBucket->setChecked(false);
  }
}

void MainWindow::on_infoDockWidget_visibilityChanged(bool a_check) {
  if (a_check) showInfo->setChecked(true);
  else if (!this->windowState().testFlag(Qt::WindowMinimized)) {
    //infoDockWidget->setVisible(false);
    showInfo->setChecked(false);
  }
}

void MainWindow::on_consoleDockWidget_visibilityChanged(bool a_check) {
  if (a_check) showConsole->setChecked(true);
  else if (!this->windowState().testFlag(Qt::WindowMinimized)) {
    //consoleDockWidget->setVisible(false);
    showConsole->setChecked(false);
  }
}

void MainWindow::on_showColorBucket_toggled(bool a_check) {
  if (a_check) sceneDockWidget->setVisible(true);
  else sceneDockWidget->setVisible(false);
}

void MainWindow::on_showConsole_toggled(bool a_check) {
  if (a_check) consoleDockWidget->setVisible(true);
  else consoleDockWidget->setVisible(false);
}

void MainWindow::on_showInfo_toggled(bool a_check) {
  if (a_check) infoDockWidget->setVisible(true);
  else infoDockWidget->setVisible(false);
}

//complement operation button clicked
void MainWindow::on_showBlueComp_toggled(bool aCheck) {
  if (aCheck) {
    m_color_complement = 0;
    showRedComp->setChecked(false);
    showBlackComp->setChecked(false);
    showBrownComp->setChecked(false);
    showYellowComp->setChecked(false);
    showMagentaComp->setChecked(false);
    showAquaComp->setChecked(false);
  } else {
    showBlueComp->setChecked(false);
  }
}

void MainWindow::on_showRedComp_toggled(bool aCheck) {
  if (aCheck) {
    m_color_complement = 1;
    showBlueComp->setChecked(false);
    showBlackComp->setChecked(false);
    showBrownComp->setChecked(false);
    showYellowComp->setChecked(false);
    showMagentaComp->setChecked(false);
    showAquaComp->setChecked(false);
  } else {
    showRedComp->setChecked(false);
  }
}

void MainWindow::on_showBlackComp_toggled(bool aCheck) {
  if (aCheck) {
    m_color_complement = 2;
    showRedComp->setChecked(false);
    showBlueComp->setChecked(false);
    showBrownComp->setChecked(false);
    showYellowComp->setChecked(false);
    showMagentaComp->setChecked(false);
    showAquaComp->setChecked(false);
  }
  else {
    showBlackComp->setChecked(false);
  }
}

void MainWindow::on_showBrownComp_toggled(bool aCheck) {
  if (aCheck) {
    m_color_complement = 3;
    showRedComp->setChecked(false);
    showBlackComp->setChecked(false);
    showBlueComp->setChecked(false);
    showYellowComp->setChecked(false);
    showMagentaComp->setChecked(false);
    showAquaComp->setChecked(false);
  } else {
    showBrownComp->setChecked(false);
  }
}

void MainWindow::on_showYellowComp_toggled(bool aCheck) {
  if (aCheck) {
    m_color_complement = 4;
    showRedComp->setChecked(false);
    showBlackComp->setChecked(false);
    showBrownComp->setChecked(false);
    showBlueComp->setChecked(false);
    showMagentaComp->setChecked(false);
    showAquaComp->setChecked(false);
  } else {
    showYellowComp->setChecked(false);
  }
}

void MainWindow::on_showMagentaComp_toggled(bool aCheck) {
  if (aCheck) {
    m_color_complement = 5;
    showRedComp->setChecked(false);
    showBlackComp->setChecked(false);
    showBrownComp->setChecked(false);
    showYellowComp->setChecked(false);
    showBlueComp->setChecked(false);
    showAquaComp->setChecked(false);
  } else {
    showMagentaComp->setChecked(false);
  }
}

void MainWindow::on_showAquaComp_toggled(bool aCheck) {
  if (aCheck) {
    m_color_complement = 6;
    showRedComp->setChecked(false);
    showBlackComp->setChecked(false);
    showBrownComp->setChecked(false);
    showYellowComp->setChecked(false);
    showMagentaComp->setChecked(false);
    showBlueComp->setChecked(false);
  }
  else {
    showAquaComp->setChecked(false);
  }
}

//copy button clicked
void MainWindow::on_copyBlue_toggled(bool aCheck) {
  if (aCheck) {
    m_color_copy = 0;
    copyRed->setChecked(false);
    copyBlack->setChecked(false);
    copyBrown->setChecked(false);
    copyYellow->setChecked(false);
    copyMagenta->setChecked(false);
    copyAqua->setChecked(false);
  }
  else {
    copyBlue->setChecked(false);
  }
}

void MainWindow::on_copyRed_toggled(bool aCheck) {
  if (aCheck) {
    m_color_copy = 1;
    copyBlue->setChecked(false);
    copyBlack->setChecked(false);
    copyBrown->setChecked(false);
    copyYellow->setChecked(false);
    copyMagenta->setChecked(false);
    copyAqua->setChecked(false);
  }
  else {
    copyRed->setChecked(false);
  }
}

void MainWindow::on_copyBlack_toggled(bool aCheck) {
  if (aCheck) {
    m_color_copy = 2;
    copyRed->setChecked(false);
    copyBlue->setChecked(false);
    copyBrown->setChecked(false);
    copyYellow->setChecked(false);
    copyMagenta->setChecked(false);
    copyAqua->setChecked(false);
  }
  else {
    copyBlack->setChecked(false);
  }
}

void MainWindow::on_copyBrown_toggled(bool aCheck) {
  if (aCheck) {
    m_color_copy = 3;
    copyRed->setChecked(false);
    copyBlack->setChecked(false);
    copyBlue->setChecked(false);
    copyYellow->setChecked(false);
    copyMagenta->setChecked(false);
    copyAqua->setChecked(false);
  }
  else {
    copyBrown->setChecked(false);
  }
}

void MainWindow::on_copyYellow_toggled(bool aCheck) {
  if (aCheck) {
    m_color_copy = 4;
    copyRed->setChecked(false);
    copyBlack->setChecked(false);
    copyBrown->setChecked(false);
    copyBlue->setChecked(false);
    copyMagenta->setChecked(false);
    copyAqua->setChecked(false);
  } else {
    copyYellow->setChecked(false);
  }
}

void MainWindow::on_copyMagenta_toggled(bool aCheck) {
  if (aCheck) {
    m_color_copy = 5;
    copyRed->setChecked(false);
    copyBlack->setChecked(false);
    copyBrown->setChecked(false);
    copyYellow->setChecked(false);
    copyBlue->setChecked(false);
    copyAqua->setChecked(false);
  } else {
    copyMagenta->setChecked(false);
  }
}

void MainWindow::on_copyAqua_toggled(bool aCheck) {
  if (aCheck) {
    m_color_copy = 6;
    copyRed->setChecked(false);
    copyBlack->setChecked(false);
    copyBrown->setChecked(false);
    copyYellow->setChecked(false);
    copyMagenta->setChecked(false);
    copyBlue->setChecked(false);
  }
  else {
    copyAqua->setChecked(false);
  }
}

//move function clicked
void MainWindow::on_moveBlue_toggled(bool aCheck) {
  if (aCheck) {
    m_color_move_ext = 0;
    moveRed->setChecked(false);
    moveBlack->setChecked(false);
    moveBrown->setChecked(false);
    moveYellow->setChecked(false);
    moveMagenta->setChecked(false);
    moveAqua->setChecked(false);
  } else {
    moveBlue->setChecked(false);
  }
}

void MainWindow::on_moveRed_toggled(bool aCheck) {
  if (aCheck) {
    m_color_move_ext = 1;
    moveBlue->setChecked(false);
    moveBlack->setChecked(false);
    moveBrown->setChecked(false);
    moveYellow->setChecked(false);
    moveMagenta->setChecked(false);
    moveAqua->setChecked(false);
  } else {
    moveRed->setChecked(false);
  }
}

void MainWindow::on_moveBlack_toggled(bool aCheck) {
  if (aCheck) {
    m_color_move_ext = 2;
    moveRed->setChecked(false);
    moveBlue->setChecked(false);
    moveBrown->setChecked(false);
    moveYellow->setChecked(false);
    moveMagenta->setChecked(false);
    moveAqua->setChecked(false);
  }
  else {
    moveBlack->setChecked(false);
  }
}

void MainWindow::on_moveBrown_toggled(bool aCheck) {
  if (aCheck) {
    m_color_move_ext = 3;
    moveRed->setChecked(false);
    moveBlack->setChecked(false);
    moveBlue->setChecked(false);
    moveYellow->setChecked(false);
    moveMagenta->setChecked(false);
    moveAqua->setChecked(false);
  }
  else {
    copyBrown->setChecked(false);
  }
}

void MainWindow::on_moveYellow_toggled(bool aCheck) {
  if (aCheck) {
    m_color_move_ext = 4;
    moveRed->setChecked(false);
    moveBlack->setChecked(false);
    moveBrown->setChecked(false);
    moveBlue->setChecked(false);
    moveMagenta->setChecked(false);
    moveAqua->setChecked(false);
  }
  else {
    moveYellow->setChecked(false);
  }
}

void MainWindow::on_moveMagenta_toggled(bool aCheck) {
  if (aCheck) {
    m_color_move_ext = 5;
    moveRed->setChecked(false);
    moveBlack->setChecked(false);
    moveBrown->setChecked(false);
    moveYellow->setChecked(false);
    moveBlue->setChecked(false);
    moveAqua->setChecked(false);
  } else {
    moveMagenta->setChecked(false);
  }
}

void MainWindow::on_moveAqua_toggled(bool aCheck) {
  if (aCheck) {
    m_color_move_ext = 6;
    moveRed->setChecked(false);
    moveBlack->setChecked(false);
    moveBrown->setChecked(false);
    moveYellow->setChecked(false);
    moveMagenta->setChecked(false);
    moveBlue->setChecked(false);
  } else {
    moveAqua->setChecked(false);
  }
}

//show button checkbox clicked
void MainWindow::on_showBlueInt_toggled(bool aCheck) {
  if (aCheck) {
    m_blue_int = true;
  } else {
    m_blue_int = false;
  }
}

void MainWindow::on_showRedInt_toggled(bool aCheck) {
  if (aCheck) {
    m_red_int = true;
  } else {
    m_red_int = false;
  }

}

void MainWindow::on_showBlackInt_toggled(bool aCheck) {
  if (aCheck) {
    m_black_int = true;
  } else {
    m_black_int = false;
  }

}

void MainWindow::on_showBrownInt_toggled(bool aCheck) {
  if (aCheck) {
    m_brown_int = true;
  } else {
    m_brown_int = false;
  }

}

void MainWindow::on_showYellowInt_toggled(bool aCheck) {

  if (aCheck) {
    m_yellow_int = true;
  } else {
    m_yellow_int = false;
  }
}

void MainWindow::on_showMagentaInt_toggled(bool aCheck) {
  if (aCheck) {
    m_magenta_int = true;
  } else {
    m_magenta_int = false;
  }

}

void MainWindow::on_showAquaInt_toggled(bool aCheck) {
  if (aCheck) {
    m_aqua_int = true;
  } else {
    m_aqua_int = false;
  }
}

//union operation button clicked
void MainWindow::on_showBlueUnion_toggled(bool aCheck) {
  if (aCheck) {
    m_blue_union = true;
  } else {
    m_blue_union = false;
  }
}

void MainWindow::on_showRedUnion_toggled(bool aCheck) {
  if (aCheck) {
    m_red_union = true;
  } else {
    m_red_union = false;
  }

}

void MainWindow::on_showBlackUnion_toggled(bool aCheck) {

  if (aCheck) {
    m_black_union = true;
  } else {
    m_black_union = false;
  }
}

void MainWindow::on_showBrownUnion_toggled(bool aCheck) {

  if (aCheck) {
    m_brown_union = true;
  } else {
    m_brown_union = false;
  }
}

void MainWindow::on_showYellowUnion_toggled(bool aCheck) {

  if (aCheck) {
    m_yellow_union = true;
  } else {
    m_yellow_union = false;
  }
}

void MainWindow::on_showMagentaUnion_toggled(bool aCheck) {
  if (aCheck) {
    m_magenta_union = true;
  } else {
    m_magenta_union = false;
  }
}

void MainWindow::on_showAquaUnion_toggled(bool aCheck) {
  if (aCheck) {
    m_aqua_union = true;
  } else {
    m_aqua_union = false;
  }
}

//difference button clicked
void MainWindow::on_showBlueDiff_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionDifferenceH->setEnabled(false);
    } else {
      actionDifferenceH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;

    if (count == 2) {
      actionDifferenceH->setEnabled(true);
    } else {
      actionDifferenceH->setEnabled(false);
    }
  }
}

void MainWindow::on_showRedDiff_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showAquaDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionDifferenceH->setEnabled(false);
    } else {
      actionDifferenceH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showAquaDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count == 2) {
      actionDifferenceH->setEnabled(true);
    } else {
      actionDifferenceH->setEnabled(false);
    }
  }
}

void MainWindow::on_showBlackDiff_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionDifferenceH->setEnabled(false);
    } else {
      actionDifferenceH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count == 2) {
      actionDifferenceH->setEnabled(true);
    } else {
      actionDifferenceH->setEnabled(false);
    }
  }
}

void MainWindow::on_showBrownDiff_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionDifferenceH->setEnabled(false);
    } else {
      actionDifferenceH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count == 2) {
      actionDifferenceH->setEnabled(true);
    } else {
      actionDifferenceH->setEnabled(false);
    }
  }
}

void MainWindow::on_showYellowDiff_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionDifferenceH->setEnabled(false);
    } else {
      actionDifferenceH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count == 2) {
      actionDifferenceH->setEnabled(true);
    } else {
      actionDifferenceH->setEnabled(false);
    }
  }
}

void MainWindow::on_showMagentaDiff_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionDifferenceH->setEnabled(false);
    } else {
      actionDifferenceH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showAquaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count == 2) {
      actionDifferenceH->setEnabled(true);
    } else {
      actionDifferenceH->setEnabled(false);
    }
  }
}

void MainWindow::on_showAquaDiff_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionDifferenceH->setEnabled(false);
    } else {
      actionDifferenceH->setEnabled(true);
    }
  }
  else {
    size_t count = 0;
    if (showRedDiff->isChecked()) count++;
    if (showBlackDiff->isChecked()) count++;
    if (showBrownDiff->isChecked()) count++;
    if (showYellowDiff->isChecked()) count++;
    if (showMagentaDiff->isChecked()) count++;
    if (showBlueDiff->isChecked()) count++;

    if (count == 2) {
      actionDifferenceH->setEnabled(true);
    } else {
      actionDifferenceH->setEnabled(false);
    }
  }
}

//symmetric difference button clicked
void MainWindow::on_showBlueSym_Diff_toggled(bool aCheck) {
  if (aCheck) {
    m_blue_sym_diff = true;
  } else {
    m_blue_sym_diff = false;
  }
}

void MainWindow::on_showRedSym_Diff_toggled(bool aCheck) {

  if (aCheck) {
    m_red_sym_diff = true;
  } else {
    m_red_sym_diff = false;
  }
}

void MainWindow::on_showBlackSym_Diff_toggled(bool aCheck) {
  if (aCheck) {
    m_black_sym_diff = true;
  } else {
    m_black_sym_diff = false;
  }
}

void MainWindow::on_showBrownSym_Diff_toggled(bool aCheck) {
  if (aCheck) {
    m_brown_sym_diff = true;
  } else {
    m_brown_sym_diff = false;
  }
}

void MainWindow::on_showYellowSym_Diff_toggled(bool aCheck) {
  if (aCheck) {
    m_yellow_sym_diff = true;
  } else {
    m_yellow_sym_diff = false;
  }
}

void MainWindow::on_showMagentaSym_Diff_toggled(bool aCheck) {
  if (aCheck) {
    m_magenta_sym_diff = true;
  } else {
    m_magenta_sym_diff = false;
  }
}

void MainWindow::on_showAquaSym_Diff_toggled(bool aCheck) {
  if (aCheck) {
    m_aqua_sym_diff = true;
  } else {
    m_aqua_sym_diff = false;
  }
}

//Minkowski button clicked
void MainWindow::on_showBlueMink_Sum_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionMinkowski_SumH->setEnabled(false);
    } else {
      actionMinkowski_SumH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;

    if (count == 2) {
      actionMinkowski_SumH->setEnabled(true);
    } else {
      actionMinkowski_SumH->setEnabled(false);
    }
  }
}

void MainWindow::on_showRedMink_Sum_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionMinkowski_SumH->setEnabled(false);
    } else {
      actionMinkowski_SumH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count == 2) {
      actionMinkowski_SumH->setEnabled(true);
    } else {
      actionMinkowski_SumH->setEnabled(false);
    }
  }
}

void MainWindow::on_showBlackMink_Sum_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionMinkowski_SumH->setEnabled(false);
    } else {
      actionMinkowski_SumH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count == 2) {
      actionMinkowski_SumH->setEnabled(true);
    } else {
      actionMinkowski_SumH->setEnabled(false);
    }
  }
}

void MainWindow::on_showBrownMink_Sum_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionMinkowski_SumH->setEnabled(false);
    } else {
      actionMinkowski_SumH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count == 2) {
      actionMinkowski_SumH->setEnabled(true);
    } else {
      actionMinkowski_SumH->setEnabled(false);
    }
  }
}

void MainWindow::on_showYellowMink_Sum_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionMinkowski_SumH->setEnabled(false);
    } else {
      actionMinkowski_SumH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count == 2) {
      actionMinkowski_SumH->setEnabled(true);
    } else {
      actionMinkowski_SumH->setEnabled(false);
    }
  }
}

void MainWindow::on_showMagentaMink_Sum_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionMinkowski_SumH->setEnabled(false);
    } else {
      actionMinkowski_SumH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showAquaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count == 2) {
      actionMinkowski_SumH->setEnabled(true);
    } else {
      actionMinkowski_SumH->setEnabled(false);
    }
  }
}

void MainWindow::on_showAquaMink_Sum_toggled(bool aCheck) {
  if (aCheck) {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count >= 2 or count == 0) {
      actionMinkowski_SumH->setEnabled(false);
    } else {
      actionMinkowski_SumH->setEnabled(true);
    }
  } else {
    size_t count = 0;
    if (showRedMink_Sum->isChecked()) count++;
    if (showBlackMink_Sum->isChecked()) count++;
    if (showBrownMink_Sum->isChecked()) count++;
    if (showYellowMink_Sum->isChecked()) count++;
    if (showMagentaMink_Sum->isChecked()) count++;
    if (showBlueMink_Sum->isChecked()) count++;

    if (count == 2) {
      actionMinkowski_SumH->setEnabled(true);
    } else {
      actionMinkowski_SumH->setEnabled(false);
    }
  }
}

//////////###################### Add Color Plus Button ###########################////////////
//done in GSoC 2020

//add color button clicked
void MainWindow::on_actionAddColor_triggered() {
  //these colors are not present by default
  if (!m_visible_brown) {
    m_visible_brown = true;
    log_statements({"brown = Polygon_set_2()"}, true);
    showBrown->setVisible(true);
    drawBrown->setVisible(true);
    clearBrown->setVisible(true);
    showBrownComp->setVisible(true);
    showBrownDiff->setVisible(true);
    showBrownUnion->setVisible(true);
    showBrownInt->setVisible(true);
    showBrownSym_Diff->setVisible(true);
    showBrownMink_Sum->setVisible(true);
    copyBrown->setVisible(true);
    moveBrown->setVisible(true);
    showBrownLabel->setVisible(true);

    line10->setVisible(true);

    line1->setGeometry(QRect(115, 0, 7, 185));
    line2->setGeometry(QRect(155, 0, 7, 185));
    line3->setGeometry(QRect(200, 0, 7, 185));
    line4->setGeometry(QRect(245, 0, 7, 185));
    line5->setGeometry(QRect(290, 0, 7, 185));
    line6->setGeometry(QRect(335, 0, 7, 185));
    line06->setGeometry(QRect(425, 0, 7, 185));
    line061->setGeometry(QRect(470, 0, 7, 185));
    line062->setGeometry(QRect(515, 0, 7, 185));
    line063->setGeometry(QRect(560, 0, 7, 185));
    line0007->setGeometry(QRect(380, 0, 7, 185));
    line0006->setGeometry(QRect(70, 0, 7, 185));

    actionMinusColor->setEnabled(true);
    actionMinusColor->setText("Remove Brown");
  } else if (!m_visible_yellow) {
    m_visible_yellow = true;
    log_statements({"yellow = Polygon_set_2()"}, true);
    showYellow->setVisible(true);
    drawYellow->setVisible(true);
    clearYellow->setVisible(true);
    showYellowComp->setVisible(true);
    showYellowDiff->setVisible(true);
    showYellowUnion->setVisible(true);
    showYellowInt->setVisible(true);
    showYellowSym_Diff->setVisible(true);
    showYellowMink_Sum->setVisible(true);
    copyYellow->setVisible(true);
    moveYellow->setVisible(true);
    showYellowLabel->setVisible(true);

    line9->setVisible(true);

    line1->setGeometry(QRect(115, 0, 7, 215));
    line2->setGeometry(QRect(155, 0, 7, 215));
    line3->setGeometry(QRect(200, 0, 7, 215));
    line4->setGeometry(QRect(245, 0, 7, 215));
    line5->setGeometry(QRect(290, 0, 7, 215));
    line6->setGeometry(QRect(335, 0, 7, 215));
    line06->setGeometry(QRect(425, 0, 7, 215));
    line061->setGeometry(QRect(470, 0, 7, 215));
    line062->setGeometry(QRect(515, 0, 7, 215));
    line063->setGeometry(QRect(560, 0, 7, 215));
    line0007->setGeometry(QRect(380, 0, 7, 215));
    line0006->setGeometry(QRect(70, 0, 7, 215));

    actionMinusColor->setText("Remove Yellow");
  } else if (!m_visible_magenta) {
    m_visible_magenta = true;
    log_statements({"magenta = Polygon_set_2()"}, true);
    showMagenta->setVisible(true);
    drawMagenta->setVisible(true);
    clearMagenta->setVisible(true);
    showMagentaComp->setVisible(true);
    showMagentaDiff->setVisible(true);
    showMagentaUnion->setVisible(true);
    showMagentaInt->setVisible(true);
    showMagentaSym_Diff->setVisible(true);
    showMagentaMink_Sum->setVisible(true);
    copyMagenta->setVisible(true);
    moveMagenta->setVisible(true);
    showMagentaLabel->setVisible(true);

    line8->setVisible(true);

    line1->setGeometry(QRect(115, 0, 7, 245));
    line2->setGeometry(QRect(155, 0, 7, 245));
    line3->setGeometry(QRect(200, 0, 7, 245));
    line4->setGeometry(QRect(245, 0, 7, 245));
    line5->setGeometry(QRect(290, 0, 7, 245));
    line6->setGeometry(QRect(335, 0, 7, 245));
    line06->setGeometry(QRect(425, 0, 7, 245));
    line061->setGeometry(QRect(470, 0, 7, 245));
    line062->setGeometry(QRect(515, 0, 7, 245));
    line063->setGeometry(QRect(560, 0, 7, 245));
    line0007->setGeometry(QRect(380, 0, 7, 245));
    line0006->setGeometry(QRect(70, 0, 7, 245));

    actionMinusColor->setText("Remove Magenta");
  } else if (!m_visible_aqua) {
    m_visible_aqua = true;
    log_statements({"aqua = Polygon_set_2()"}, true);
    showAqua->setVisible(true);
    drawAqua->setVisible(true);
    clearAqua->setVisible(true);
    showAquaComp->setVisible(true);
    showAquaDiff->setVisible(true);
    showAquaUnion->setVisible(true);
    showAquaInt->setVisible(true);
    showAquaSym_Diff->setVisible(true);
    showAquaMink_Sum->setVisible(true);
    copyAqua->setVisible(true);
    moveAqua->setVisible(true);
    showAquaLabel->setVisible(true);

    line7->setVisible(true);

    line1->setGeometry(QRect(115, 0, 7, 275));
    line2->setGeometry(QRect(155, 0, 7, 275));
    line3->setGeometry(QRect(200, 0, 7, 275));
    line4->setGeometry(QRect(245, 0, 7, 275));
    line5->setGeometry(QRect(290, 0, 7, 275));
    line6->setGeometry(QRect(335, 0, 7, 275));
    line06->setGeometry(QRect(425, 0, 7, 275));
    line061->setGeometry(QRect(470, 0, 7, 275));
    line062->setGeometry(QRect(515, 0, 7, 275));
    line063->setGeometry(QRect(560, 0, 7, 275));
    line0007->setGeometry(QRect(380, 0, 7, 275));
    line0006->setGeometry(QRect(70, 0, 7, 275));

    actionAddColor->setEnabled(false);
    actionAddColor->setText("Maximum Number of Buckets Reached");
    actionMinusColor->setText("Remove Aqua");
  }

  modelChanged();
}

//////#########Remove Color Minus
//minus color button clicked
void MainWindow::on_actionMinusColor_triggered() {
  if (!(m_visible_yellow || m_visible_magenta || m_visible_aqua)) {
    log_statements({"del brown"}, true);
    states_stack.back().m_curve_sets[3].clear();
    //states_stack.back().m_curve_sets[7].clear();

    states_stack.back().brown_circular_sources().clear();
    states_stack.back().brown_linear_sources().clear();
    states_stack.back().brown_bezier_sources().clear();
    states_stack.back().brown_polyline_sources().clear();

    m_visible_brown = false;
    showBrown->setVisible(false);
    drawBrown->setVisible(false);
    clearBrown->setVisible(false);
    showBrownComp->setVisible(false);
    showBrownDiff->setVisible(false);
    showBrownUnion->setVisible(false);
    showBrownInt->setVisible(false);
    showBrownSym_Diff->setVisible(false);
    showBrownMink_Sum->setVisible(false);
    copyBrown->setVisible(false);
    moveBrown->setVisible(false);
    showBrownLabel->setVisible(false);

    // If the removed bucket is marked for Draw/Insert or Complement,
    // mark the first bucket instead.
    if (m_color_active == 3) drawBlue->setChecked(true);
    if (m_color_complement == 3) showBlueComp->setChecked(true);
    // Unmark the removed bucket as an operand of the two-operand operations;
    // this disables an operation that no longer has two operands.
    showBrownDiff->setChecked(false);
    showBrownSym_Diff->setChecked(false);
    showBrownMink_Sum->setChecked(false);
    // Unmark it as an intersection or union operand, and keep at least one
    // remaining bucket unmarked, for the result.
    showBrownInt->setChecked(false);
    showBrownUnion->setChecked(false);
    keep_one_unchecked(m_intersection_column);
    keep_one_unchecked(m_union_column);

    line10->setVisible(false);

    line1->setGeometry(QRect(115, 0, 7, 155));
    line2->setGeometry(QRect(155, 0, 7, 155));
    line3->setGeometry(QRect(200, 0, 7, 155));
    line4->setGeometry(QRect(245, 0, 7, 155));
    line5->setGeometry(QRect(290, 0, 7, 155));
    line6->setGeometry(QRect(335, 0, 7, 155));
    line06->setGeometry(QRect(425, 0, 7, 155));
    line061->setGeometry(QRect(470, 0, 7, 155));
    line062->setGeometry(QRect(515, 0, 7, 155));
    line063->setGeometry(QRect(560, 0, 7, 155));
    line0007->setGeometry(QRect(380, 0, 7, 155));
    line0006->setGeometry(QRect(70, 0, 7, 155));

    actionMinusColor->setText("Minimum Number of Buckets Reached");
    actionMinusColor->setEnabled(false);
  } else if (!(m_visible_magenta || m_visible_aqua)) {
    log_statements({"del yellow"}, true);
    states_stack.back().m_curve_sets[4].clear();

    states_stack.back().yellow_circular_sources().clear();
    states_stack.back().yellow_linear_sources().clear();
    states_stack.back().yellow_bezier_sources().clear();
    states_stack.back().yellow_polyline_sources().clear();

    m_visible_yellow = false;
    showYellow->setVisible(false);
    drawYellow->setVisible(false);
    clearYellow->setVisible(false);
    showYellowComp->setVisible(false);
    showYellowDiff->setVisible(false);
    showYellowUnion->setVisible(false);
    showYellowInt->setVisible(false);
    showYellowSym_Diff->setVisible(false);
    showYellowMink_Sum->setVisible(false);
    copyYellow->setVisible(false);
    moveYellow->setVisible(false);
    showYellowLabel->setVisible(false);

    // If the removed bucket is marked for Draw/Insert or Complement,
    // mark the first bucket instead.
    if (m_color_active == 4) drawBlue->setChecked(true);
    if (m_color_complement == 4) showBlueComp->setChecked(true);
    // Unmark the removed bucket as an operand of the two-operand operations;
    // this disables an operation that no longer has two operands.
    showYellowDiff->setChecked(false);
    showYellowSym_Diff->setChecked(false);
    showYellowMink_Sum->setChecked(false);
    // Unmark it as an intersection or union operand, and keep at least one
    // remaining bucket unmarked, for the result.
    showYellowInt->setChecked(false);
    showYellowUnion->setChecked(false);
    keep_one_unchecked(m_intersection_column);
    keep_one_unchecked(m_union_column);

    line9->setVisible(false);

    line1->setGeometry(QRect(115, 0, 7, 185));
    line2->setGeometry(QRect(155, 0, 7, 185));
    line3->setGeometry(QRect(200, 0, 7, 185));
    line4->setGeometry(QRect(245, 0, 7, 185));
    line5->setGeometry(QRect(290, 0, 7, 185));
    line6->setGeometry(QRect(335, 0, 7, 185));
    line06->setGeometry(QRect(425, 0, 7, 185));
    line061->setGeometry(QRect(470, 0, 7, 185));
    line062->setGeometry(QRect(515, 0, 7, 185));
    line063->setGeometry(QRect(560, 0, 7, 185));
    line0007->setGeometry(QRect(380, 0, 7, 185));
    line0006->setGeometry(QRect(70, 0, 7, 185));

    actionMinusColor->setText("Remove Brown");
  } else if (!m_visible_aqua) {
    log_statements({"del magenta"}, true);
    states_stack.back().m_curve_sets[5].clear();

    states_stack.back().magenta_circular_sources().clear();
    states_stack.back().magenta_linear_sources().clear();
    states_stack.back().magenta_bezier_sources().clear();
    states_stack.back().magenta_polyline_sources().clear();

    m_visible_magenta = false;
    showMagenta->setVisible(false);
    drawMagenta->setVisible(false);
    clearMagenta->setVisible(false);
    showMagentaComp->setVisible(false);
    showMagentaDiff->setVisible(false);
    showMagentaUnion->setVisible(false);
    showMagentaInt->setVisible(false);
    showMagentaSym_Diff->setVisible(false);
    showMagentaMink_Sum->setVisible(false);
    copyMagenta->setVisible(false);
    moveMagenta->setVisible(false);
    showMagentaLabel->setVisible(false);

    // If the removed bucket is marked for Draw/Insert or Complement,
    // mark the first bucket instead.
    if (m_color_active == 5) drawBlue->setChecked(true);
    if (m_color_complement == 5) showBlueComp->setChecked(true);
    // Unmark the removed bucket as an operand of the two-operand operations;
    // this disables an operation that no longer has two operands.
    showMagentaDiff->setChecked(false);
    showMagentaSym_Diff->setChecked(false);
    showMagentaMink_Sum->setChecked(false);
    // Unmark it as an intersection or union operand, and keep at least one
    // remaining bucket unmarked, for the result.
    showMagentaInt->setChecked(false);
    showMagentaUnion->setChecked(false);
    keep_one_unchecked(m_intersection_column);
    keep_one_unchecked(m_union_column);

    line8->setVisible(false);

    line1->setGeometry(QRect(115, 0, 7, 215));
    line2->setGeometry(QRect(155, 0, 7, 215));
    line3->setGeometry(QRect(200, 0, 7, 215));
    line4->setGeometry(QRect(245, 0, 7, 215));
    line5->setGeometry(QRect(290, 0, 7, 215));
    line6->setGeometry(QRect(335, 0, 7, 215));
    line06->setGeometry(QRect(425, 0, 7, 215));
    line061->setGeometry(QRect(470, 0, 7, 215));
    line062->setGeometry(QRect(515, 0, 7, 215));
    line063->setGeometry(QRect(560, 0, 7, 215));
    line0007->setGeometry(QRect(380, 0, 7, 215));
    line0006->setGeometry(QRect(70, 0, 7, 215));

    actionMinusColor->setText("Remove Yellow");
  } else {
    log_statements({"del aqua"}, true);
    states_stack.back().m_curve_sets[6].clear();

    states_stack.back().aqua_circular_sources().clear();
    states_stack.back().aqua_linear_sources().clear();
    states_stack.back().aqua_bezier_sources().clear();
    states_stack.back().aqua_polyline_sources().clear();

    m_visible_aqua = false;
    showAqua->setVisible(false);
    drawAqua->setVisible(false);
    clearAqua->setVisible(false);
    showAquaComp->setVisible(false);
    showAquaDiff->setVisible(false);
    showAquaUnion->setVisible(false);
    showAquaInt->setVisible(false);
    showAquaSym_Diff->setVisible(false);
    showAquaMink_Sum->setVisible(false);
    copyAqua->setVisible(false);
    moveAqua->setVisible(false);
    showAquaLabel->setVisible(false);

    // If the removed bucket is marked for Draw/Insert or Complement,
    // mark the first bucket instead.
    if (m_color_active == 6) drawBlue->setChecked(true);
    if (m_color_complement == 6) showBlueComp->setChecked(true);
    // Unmark the removed bucket as an operand of the two-operand operations;
    // this disables an operation that no longer has two operands.
    showAquaDiff->setChecked(false);
    showAquaSym_Diff->setChecked(false);
    showAquaMink_Sum->setChecked(false);
    // Unmark it as an intersection or union operand, and keep at least one
    // remaining bucket unmarked, for the result.
    showAquaInt->setChecked(false);
    showAquaUnion->setChecked(false);
    keep_one_unchecked(m_intersection_column);
    keep_one_unchecked(m_union_column);

    line7->setVisible(false);

    line1->setGeometry(QRect(115, 0, 7, 245));
    line2->setGeometry(QRect(155, 0, 7, 245));
    line3->setGeometry(QRect(200, 0, 7, 245));
    line4->setGeometry(QRect(245, 0, 7, 245));
    line5->setGeometry(QRect(290, 0, 7, 245));
    line6->setGeometry(QRect(335, 0, 7, 245));
    line06->setGeometry(QRect(425, 0, 7, 245));
    line061->setGeometry(QRect(470, 0, 7, 245));
    line062->setGeometry(QRect(515, 0, 7, 245));
    line063->setGeometry(QRect(560, 0, 7, 245));
    line0007->setGeometry(QRect(380, 0, 7, 245));
    line0006->setGeometry(QRect(70, 0, 7, 245));

    actionAddColor->setEnabled(true);
    actionAddColor->setText("Add a Bucket");
    actionMinusColor->setText("Remove Magenta");
  }

  modelChanged();
}

void MainWindow::on_actionUndo_triggered() {
  if (m_state_num > 1) {
    /*operation_name
    // COMPLEMENT_OP = 0
    // INTERSECTION_OP = 1
    // UNION_OP = 2
    // DIFFERENCE_OP = 3
    // SYMMETRIC_DIFFERENCE_OP = 4
    // MINKOWSKI_SUM_OP = 5
    // RESET_OP = 6
    // COPY_OP = 7
    // MOVE_OP = 8
    // CLEAR_OP = 9
    // DELETEALL_OP = 10
    // DRAWPOL_OP = 11
    START_OP = 12
    */

    //unlink GI for last state
    for (auto si = states_stack.back().m_curve_sets.begin();
         si != states_stack.back().m_curve_sets.end(); ++si) { unlink_GI(si->gi()); }

    switch (states_stack.back().m_operation) {
     case 0:if (actionComplementH->isChecked()) actionComplementH->setChecked(false);
      break;
     case 1:if (actionIntersectionH->isChecked()) actionIntersectionH->setChecked(false);
      break;
     case 2:if (actionUnionH->isChecked()) actionUnionH->setChecked(false);
      break;
     case 3:if (actionDifferenceH->isChecked()) actionDifferenceH->setChecked(false);
      break;
     case 4:if (actionSymmetric_DifferenceH->isChecked()) actionSymmetric_DifferenceH->setChecked(false);
      break;
     case 5:if (actionMinkowski_SumH->isChecked()) actionMinkowski_SumH->setChecked(false);
      break;
     case 10:if (actionComplementH->isChecked()) actionComplementH->setChecked(false);
      break;
     case 11:if (actionComplementH->isChecked()) actionComplementH->setChecked(false);
      break;
    }

    modelChanged();

    // pop last state
    states_stack.pop_back();

    for (auto si = states_stack.back().m_curve_sets.begin();
         si != states_stack.back().m_curve_sets.end(); ++si) { link_GI(si->gi()); }

    if (states_stack.size() == 1) {
      actionUndo->setEnabled(false);
    }

    m_state_num = states_stack.size();
    trim_program_to_states();

    // for confirming the num of each state during debugging
    // show_error("State num: "+to_string(states_stack.size()));
    modelChanged();
  } else {
    show_error("Nothing to Undo!!!");
  }
}

void MainWindow::on_actionNew_triggered() {

  if (m_state_num > 1) get_new_state(6);

  // Reset starts a new program, which constructs the existing buckets.
  m_program.clear();
  if (m_log) m_log->clear();
  QStringList program;
  for (std::size_t i = 0; i < 7; ++i)
    if (bucket_exists(i))
      program << QString("%1 = Polygon_set_2()").arg(bucket_name(i));
  log_statements(program, true);

  for (Curve_set_iterator si = states_stack.back().m_curve_sets.begin();
       si != states_stack.back().m_curve_sets.end(); ++si)
    si->clear();

  m_scene.setSceneRect(-320, -210, 640, 420);
  this->graphicsView->setScene(&m_scene);
  this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  this->graphicsView->scale(2.5, 2.5);

  states_stack.back().result_set().clear();
  states_stack.back().blue_set().clear();
  states_stack.back().black_set().clear();
  states_stack.back().brown_set().clear();
  states_stack.back().red_set().clear();
  states_stack.back().yellow_set().clear();
  states_stack.back().magenta_set().clear();
  states_stack.back().aqua_set().clear();

  states_stack.back().blue_circular_sources().clear();
  states_stack.back().red_circular_sources().clear();
  states_stack.back().black_circular_sources().clear();
  states_stack.back().brown_circular_sources().clear();
  states_stack.back().yellow_circular_sources().clear();
  states_stack.back().magenta_circular_sources().clear();
  states_stack.back().aqua_circular_sources().clear();
  states_stack.back().result_circular_sources().clear();

  states_stack.back().blue_linear_sources().clear();
  states_stack.back().red_linear_sources().clear();
  states_stack.back().black_linear_sources().clear();
  states_stack.back().brown_linear_sources().clear();
  states_stack.back().yellow_linear_sources().clear();
  states_stack.back().magenta_linear_sources().clear();
  states_stack.back().result_linear_sources().clear();

  states_stack.back().blue_bezier_sources().clear();
  states_stack.back().red_bezier_sources().clear();
  states_stack.back().black_bezier_sources().clear();
  states_stack.back().brown_bezier_sources().clear();
  states_stack.back().yellow_bezier_sources().clear();
  states_stack.back().magenta_bezier_sources().clear();
  states_stack.back().result_bezier_sources().clear();

  states_stack.back().blue_polyline_sources().clear();
  states_stack.back().red_polyline_sources().clear();
  states_stack.back().black_polyline_sources().clear();
  states_stack.back().brown_polyline_sources().clear();
  states_stack.back().yellow_polyline_sources().clear();
  states_stack.back().magenta_polyline_sources().clear();
  states_stack.back().result_polyline_sources().clear();

  SetViewBlue(true);
  SetViewRed(true);
  SetViewBlack(true);
  SetViewBrown(true);
  SetViewYellow(true);
  SetViewMagenta(true);
  SetViewAqua(true);
  SetViewResult(true);

  actionAddColor->setEnabled(true);
  actionMinusColor->setEnabled(false);

  m_disjoint = false;

  m_linear_input->Reset();
  m_circular_input->Reset();
  m_bezier_input->Reset();
  m_polyline_input->Reset();

  m_pan = false;

  m_color_active = 0;
  m_color_move = 1111;
  m_color_cm = 1111;
  m_color_visible = 7;

  m_linear_input->mOngoingPieceGI->setPen(sPens[0]);
  m_linear_input->mLinearGI->setPen(sPens[0]);
  m_linear_input->mHandleGI->setPen(sPens[0]);

  sceneDockWidget->setVisible(true);
  consoleDockWidget->setVisible(true);
  infoDockWidget->setVisible(true);

  showColorBucket->setChecked(true);
  showConsole->setChecked(true);
  showInfo->setChecked(true);

  actionCopyH->setChecked(false);
  actionMoveH->setChecked(false);
  //actionAxis->setChecked(false);

  actionComplementH->setChecked(false);
  actionUnionH->setChecked(false);
  actionIntersectionH->setChecked(false);
  actionDifferenceH->setChecked(false);
  actionSymmetric_DifferenceH->setChecked(false);
  actionMinkowski_SumH->setChecked(false);

  m_reset = true;
  for (int type = 1; type <= 5; ++type)
    family_action(type)->setChecked(type == active_type());

  m_reset = false;

  // actionMinkowski_SumH -> setEnabled(true);
  actionDifferenceH->setEnabled(true);

  m_color_copy = 111; //default
  m_color_move_ext = 111; //default
  m_color_complement = 0; //default
  m_blue_int = true; //default
  m_red_int = true; //default
  m_black_int = false; //default
  m_brown_int = false; //default
  m_yellow_int = false; //default
  m_magenta_int = false; //default
  m_aqua_int = false; //default

  m_blue_union = true; //default
  m_red_union = true; //default
  m_black_union = false; //default
  m_brown_union = false; //default
  m_yellow_union = false; //default
  m_magenta_union = false; //default
  m_aqua_union = false; //default

  m_blue_sym_diff = true; //default
  m_red_sym_diff = true; //default
  m_black_sym_diff = false; //default
  m_brown_sym_diff = false; //default
  m_yellow_sym_diff = false; //default
  m_magenta_sym_diff = false; //default
  m_aqua_sym_diff = false; //default

  m_blue_mink = true; //default
  m_red_mink = true; //default
  m_black_mink = false; //default
  m_brown_mink = false; //default
  m_yellow_mink = false; //default
  m_magenta_mink = false; //default
  m_aqua_mink = false; //default

  drawBlue->setChecked(true);

  showBlue->setChecked(true);
  showRed->setChecked(true);
  showBlack->setChecked(true);
  showBrown->setChecked(true);
  showYellow->setChecked(true);
  showMagenta->setChecked(true);
  showAqua->setChecked(true);
  showResult->setChecked(false);

  clearBlue->setChecked(false);
  clearRed->setChecked(false);
  clearBlack->setChecked(false);
  clearBrown->setChecked(false);
  clearYellow->setChecked(false);
  clearMagenta->setChecked(false);
  clearAqua->setChecked(false);

  clearBrown->setVisible(false);
  clearYellow->setVisible(false);
  clearMagenta->setVisible(false);
  clearAqua->setVisible(false);

  showBrownLabel->setVisible(false);
  showYellowLabel->setVisible(false);
  showMagentaLabel->setVisible(false);
  showAquaLabel->setVisible(false);

  showBlueComp->setChecked(true);
  showRedComp->setChecked(false);
  showBlackComp->setChecked(false);
  showBrownComp->setChecked(false);
  showYellowComp->setChecked(false);
  showMagentaComp->setChecked(false);
  showAquaComp->setChecked(false);

  copyBlue->setChecked(false);
  copyRed->setChecked(false);
  copyBlack->setChecked(false);
  copyBrown->setChecked(false);
  copyYellow->setChecked(false);
  copyMagenta->setChecked(false);
  copyAqua->setChecked(false);

  moveBlue->setChecked(false);
  moveRed->setChecked(false);
  moveBlack->setChecked(false);
  moveBrown->setChecked(false);
  moveYellow->setChecked(false);
  moveMagenta->setChecked(false);
  moveAqua->setChecked(false);

  showBlackInt->setChecked(false);
  showBrownInt->setChecked(false);
  showYellowInt->setChecked(false);
  showMagentaInt->setChecked(false);
  showAquaInt->setChecked(false);
  showBlueInt->setChecked(true);
  showRedInt->setChecked(true);

  showBlackUnion->setChecked(false);
  showBrownUnion->setChecked(false);
  showYellowUnion->setChecked(false);
  showMagentaUnion->setChecked(false);
  showAquaUnion->setChecked(false);
  showBlueUnion->setChecked(true);
  showRedUnion->setChecked(true);

  showBlackDiff->setChecked(false);
  showBrownDiff->setChecked(false);
  showYellowDiff->setChecked(false);
  showMagentaDiff->setChecked(false);
  showAquaDiff->setChecked(false);
  showBlueDiff->setChecked(true);
  showRedDiff->setChecked(true);

  showBlackSym_Diff->setChecked(false);
  showBrownSym_Diff->setChecked(false);
  showYellowSym_Diff->setChecked(false);
  showMagentaSym_Diff->setChecked(false);
  showAquaSym_Diff->setChecked(false);
  showBlueSym_Diff->setChecked(true);
  showRedSym_Diff->setChecked(true);

  showBlackMink_Sum->setChecked(false);
  showBrownMink_Sum->setChecked(false);
  showYellowMink_Sum->setChecked(false);
  showMagentaMink_Sum->setChecked(false);
  showAquaMink_Sum->setChecked(false);
  showBlueMink_Sum->setChecked(true);
  showRedMink_Sum->setChecked(true);

  m_visible_black = true;
  showBlack->setVisible(true);
  drawBlack->setVisible(true);
  showBlackComp->setVisible(true);
  showBlackDiff->setVisible(true);
  showBlackUnion->setVisible(true);
  showBlackInt->setVisible(true);
  showBlackSym_Diff->setVisible(true);
  showBlackMink_Sum->setVisible(true);
  copyBlack->setVisible(true);
  moveBlack->setVisible(true);

  m_visible_brown = false;
  showBrown->setVisible(false);
  drawBrown->setVisible(false);
  showBrownComp->setVisible(false);
  showBrownDiff->setVisible(false);
  showBrownUnion->setVisible(false);
  showBrownInt->setVisible(false);
  showBrownSym_Diff->setVisible(false);
  showBrownMink_Sum->setVisible(false);
  copyBrown->setVisible(false);
  moveBrown->setVisible(false);

  m_visible_yellow = false;
  showYellow->setVisible(false);
  drawYellow->setVisible(false);
  showYellowComp->setVisible(false);
  showYellowDiff->setVisible(false);
  showYellowUnion->setVisible(false);
  showYellowInt->setVisible(false);
  showYellowSym_Diff->setVisible(false);
  showYellowMink_Sum->setVisible(false);
  copyYellow->setVisible(false);
  moveYellow->setVisible(false);

  m_visible_magenta = false;
  showMagenta->setVisible(false);
  drawMagenta->setVisible(false);
  showMagentaComp->setVisible(false);
  showMagentaDiff->setVisible(false);
  showMagentaUnion->setVisible(false);
  showMagentaInt->setVisible(false);
  showMagentaSym_Diff->setVisible(false);
  showMagentaMink_Sum->setVisible(false);
  copyMagenta->setVisible(false);
  moveMagenta->setVisible(false);

  m_visible_aqua = false;
  showAqua->setVisible(false);
  drawAqua->setVisible(false);
  showAquaComp->setVisible(false);
  showAquaDiff->setVisible(false);
  showAquaUnion->setVisible(false);
  showAquaInt->setVisible(false);
  showAquaSym_Diff->setVisible(false);
  showAquaMink_Sum->setVisible(false);
  copyAqua->setVisible(false);
  moveAqua->setVisible(false);

  actionAddColor->setEnabled(true);
  actionAddColor->setText("Add a Bucket");
  actionMinusColor->setText("Minimum Number of Buckets Reached");
  VisibleHeader->setChecked(true);
  showClear->setChecked(false);

  line7->setVisible(false);
  line8->setVisible(false);
  line9->setVisible(false);
  line10->setVisible(false);
  line11->setVisible(true);

  line1->setGeometry(QRect(115, 0, 7, 155));
  line2->setGeometry(QRect(155, 0, 7, 155));
  line3->setGeometry(QRect(200, 0, 7, 155));
  line4->setGeometry(QRect(245, 0, 7, 155));
  line5->setGeometry(QRect(290, 0, 7, 155));
  line6->setGeometry(QRect(335, 0, 7, 155));
  line06->setGeometry(QRect(425, 0, 7, 155));
  line061->setGeometry(QRect(470, 0, 7, 155));
  line062->setGeometry(QRect(515, 0, 7, 155));
  line063->setGeometry(QRect(560, 0, 7, 155));
  line0007->setGeometry(QRect(380, 0, 7, 155));
  line0006->setGeometry(QRect(70, 0, 7, 155));

  zoomToFit();
  modelChanged();
}

void MainWindow::on_actionDeleteAll_triggered() {
  bool lDone = false;
  //bool lProceed = states_stack.back().result_set().is_empty() ? true : ask_user_yesno("Store result","All polygons will be deleted\n continue anyway?\n");

  //ask: what is the use of if(true)
  if (true) {
    actionComplementH->setChecked(false);
    actionIntersectionH->setChecked(false);
    actionUnionH->setChecked(false);
    actionDifferenceH->setChecked(false);
    actionSymmetric_DifferenceH->setChecked(false);
    actionMinkowski_SumH->setChecked(false);
    actionComplementH->setChecked(false);
    actionComplementH->setChecked(false);

    get_new_state(10);
    {
      QStringList program;
      for (std::size_t i = 0; i < 7; ++i)
        if (! states_stack.back().set(i).is_empty())
          program << QString("%1.clear()").arg(bucket_name(i));
      log_statements(program);
    }

    /*operation_name
    // COMPLEMENT_OP = 0
    // INTERSECTION_OP = 1
    // UNION_OP = 2
    // DIFFERENCE_OP = 3
    // SYMMETRIC_DIFFERENCE_OP = 4
    // MINKOWSKI_SUM_OP = 5
    // RESET_OP = 6
    // COPY_OP = 7
    // MOVE_OP = 8
    // CLEAR_OP = 9
    // DELETEALL_OP = 10
    // START_OP = 11*/

    states_stack.back().blue_set().clear();
    states_stack.back().blue_circular_sources().clear();
    states_stack.back().blue_bezier_sources().clear();
    states_stack.back().blue_linear_sources().clear();
    states_stack.back().blue_polyline_sources().clear();
    states_stack.back().red_set().clear();
    states_stack.back().red_circular_sources().clear();
    states_stack.back().red_bezier_sources().clear();
    states_stack.back().red_linear_sources().clear();
    states_stack.back().red_polyline_sources().clear();
    states_stack.back().black_set().clear();
    states_stack.back().black_circular_sources().clear();
    states_stack.back().black_bezier_sources().clear();
    states_stack.back().black_linear_sources().clear();
    states_stack.back().black_polyline_sources().clear();
    states_stack.back().brown_set().clear();
    states_stack.back().brown_circular_sources().clear();
    states_stack.back().brown_bezier_sources().clear();
    states_stack.back().brown_linear_sources().clear();
    states_stack.back().brown_polyline_sources().clear();
    states_stack.back().yellow_set().clear();
    states_stack.back().yellow_circular_sources().clear();
    states_stack.back().yellow_bezier_sources().clear();
    states_stack.back().yellow_linear_sources().clear();
    states_stack.back().yellow_polyline_sources().clear();
    states_stack.back().magenta_set().clear();
    states_stack.back().magenta_circular_sources().clear();
    states_stack.back().magenta_bezier_sources().clear();
    states_stack.back().magenta_linear_sources().clear();
    states_stack.back().magenta_polyline_sources().clear();
    states_stack.back().aqua_set().clear();
    states_stack.back().aqua_circular_sources().clear();
    states_stack.back().aqua_bezier_sources().clear();
    states_stack.back().aqua_linear_sources().clear();
    states_stack.back().aqua_polyline_sources().clear();

    on_actionDeleteResult();
  }
  lDone = true;
  if (lDone) modelChanged();
}

void MainWindow::on_actionDeleteResult() {
  bool lDone = false;
  bool lProceed;

  /*if(states_stack.back().result_set().is_empty()||states_stack.back().result_set().is_empty()||states_stack.back().result_set().is_empty()||states_stack.back().result_set().is_empty()||states_stack.back().result_set().is_empty()||states_stack.back().result_set().is_empty()||states_stack.back().result_set().is_empty())
    {
    bool lProceed = ask_user_yesno("Store result","Result will be deleted\n continue anyway?\n");
    }*/

  if (true) {
    states_stack.back().result_set().clear();
    states_stack.back().result_circular_sources().clear();
    states_stack.back().result_bezier_sources().clear();
    states_stack.back().result_linear_sources().clear();
    states_stack.back().result_polyline_sources().clear();
    lDone = true;
  }
  if (lDone) modelChanged();
}

void MainWindow::on_actionClearH_toggled(bool aChecked) {
  if (aChecked) {
    bool lDone = false;
    bool lProceed;

    actionComplementH->setChecked(false);
    actionIntersectionH->setChecked(false);
    actionUnionH->setChecked(false);
    actionDifferenceH->setChecked(false);
    actionSymmetric_DifferenceH->setChecked(false);
    actionMinkowski_SumH->setChecked(false);
    actionComplementH->setChecked(false);
    actionComplementH->setChecked(false);

    get_new_state(9);
    {
      QStringList program;
      std::size_t i = 0;
      for (auto* box : {clearBlue, clearRed, clearBlack, clearBrown,
                        clearYellow, clearMagenta, clearAqua}) {
        if (box->isChecked() && bucket_exists(i))
          program << QString("%1.clear()").arg(bucket_name(i));
        ++i;
      }
      log_statements(program);
    }

    /*operation_name
    // COMPLEMENT_OP = 0
    // INTERSECTION_OP = 1
    // UNION_OP = 2
    // DIFFERENCE_OP = 3
    // SYMMETRIC_dIFFERENCE_OP = 4
    // MINKOWSKI_SUM_OP = 5
    // RESET_OP = 6
    // COPY_OP = 7
    // MOVE_OP = 8
    // CLEAR_OP = 9
    // DELETEALL_OP = 10
    // START_OP = 11*/

    if (clearBlue->isChecked()) {
      states_stack.back().blue_set().clear();
      states_stack.back().blue_linear_sources().clear();
      states_stack.back().blue_circular_sources().clear();
      states_stack.back().blue_bezier_sources().clear();
      states_stack.back().blue_polyline_sources().clear();
    }
    if (clearBlack->isChecked()) {
      states_stack.back().black_set().clear();
      states_stack.back().black_linear_sources().clear();
      states_stack.back().black_circular_sources().clear();
      states_stack.back().black_bezier_sources().clear();
      states_stack.back().black_polyline_sources().clear();
    }
    if (clearRed->isChecked()) {
      states_stack.back().red_set().clear();
      states_stack.back().red_linear_sources().clear();
      states_stack.back().red_circular_sources().clear();
      states_stack.back().red_bezier_sources().clear();
      states_stack.back().red_polyline_sources().clear();
    }
    if (clearBrown->isChecked()) {
      states_stack.back().brown_set().clear();
      states_stack.back().brown_linear_sources().clear();
      states_stack.back().brown_circular_sources().clear();
      states_stack.back().brown_bezier_sources().clear();
      states_stack.back().brown_polyline_sources().clear();
    }
    if (clearYellow->isChecked()) {
      states_stack.back().yellow_set().clear();
      states_stack.back().yellow_linear_sources().clear();
      states_stack.back().yellow_circular_sources().clear();
      states_stack.back().yellow_bezier_sources().clear();
      states_stack.back().yellow_polyline_sources().clear();
    }
    if (clearMagenta->isChecked()) {
      states_stack.back().magenta_set().clear();
      states_stack.back().magenta_linear_sources().clear();
      states_stack.back().magenta_circular_sources().clear();
      states_stack.back().magenta_bezier_sources().clear();
      states_stack.back().magenta_polyline_sources().clear();
    }
    if (clearAqua->isChecked()) {
      states_stack.back().aqua_set().clear();
      states_stack.back().aqua_linear_sources().clear();
      states_stack.back().aqua_circular_sources().clear();
      states_stack.back().aqua_bezier_sources().clear();
      states_stack.back().aqua_polyline_sources().clear();
    }

    if (!(clearBlue->isChecked()) && !(clearRed->isChecked()) && !(clearBlack->isChecked()) &&
        !(clearBrown->isChecked()) && !(clearYellow->isChecked()) && !(clearMagenta->isChecked()) &&
        !(clearAqua->isChecked()))
      show_error("Please select color bucket to be cleaned!!!");

    actionClearH->setChecked(false);
    lDone = true;

    if (lDone) modelChanged();
  }
}

//draw polygons
//todo::redefine it's parameters i.e remove bool from params
void MainWindow::on_drawBlue_toggled(bool /* a_check */) {
  m_color_active = 0;

  if (states_stack.back().blue_set().is_linear()) {
    m_linear_input->mOngoingPieceGI->setPen(sPens[0]);
    m_linear_input->mLinearGI->setPen(sPens[0]);
    m_linear_input->mHandleGI->setPen(sPens[0]);
  } else if (states_stack.back().blue_set().is_circular()) {
    m_circular_input->mOngoingPieceGI->setPen(sPens[0]);
    m_circular_input->mCircularGI->setPen(sPens[0]);
    m_circular_input->mHandleGI->setPen(sPens[0]);
  } else if (states_stack.back().blue_set().is_bezier()) {
    m_bezier_input->mOngoingPieceGI->setPen(sPens[0]);
    m_bezier_input->mBezierGI->setPen(sPens[0]);
    m_bezier_input->mHandle0GI->setPen(sPens[0]);
    m_bezier_input->mHandle1GI->setPen(sPens[0]);
  } else {
    m_polyline_input->mOngoingPieceGI->setPen(sPens[0]);
    m_polyline_input->mPolylineGI->setPen(sPens[0]);
    m_polyline_input->mHandleGI->setPen(sPens[0]);
  }
  set_conic_pen(sPens[0]);

}

void MainWindow::on_drawRed_toggled(bool /* a_check */) {
  m_color_active = 1;

  if (states_stack.back().red_set().is_linear()) {
    m_linear_input->mOngoingPieceGI->setPen(sPens[1]);
    m_linear_input->mLinearGI->setPen(sPens[1]);
    m_linear_input->mHandleGI->setPen(sPens[1]);
  } else if (states_stack.back().red_set().is_circular()) {
    m_circular_input->mOngoingPieceGI->setPen(sPens[1]);
    m_circular_input->mCircularGI->setPen(sPens[1]);
    m_circular_input->mHandleGI->setPen(sPens[1]);
  } else if (states_stack.back().red_set().is_bezier()) {
    m_bezier_input->mOngoingPieceGI->setPen(sPens[1]);
    m_bezier_input->mBezierGI->setPen(sPens[1]);
    m_bezier_input->mHandle0GI->setPen(sPens[1]);
    m_bezier_input->mHandle1GI->setPen(sPens[1]);
  } else {
    m_polyline_input->mOngoingPieceGI->setPen(sPens[1]);
    m_polyline_input->mPolylineGI->setPen(sPens[1]);
    m_polyline_input->mHandleGI->setPen(sPens[1]);
  }
  set_conic_pen(sPens[1]);

}

void MainWindow::on_drawBlack_toggled(bool /* a_check */) {
  m_color_active = 2;

  if (states_stack.back().black_set().is_linear()) {
    m_linear_input->mOngoingPieceGI->setPen(sPens[2]);
    m_linear_input->mLinearGI->setPen(sPens[2]);
    m_linear_input->mHandleGI->setPen(sPens[2]);
  } else if (states_stack.back().black_set().is_circular()) {
    m_circular_input->mOngoingPieceGI->setPen(sPens[2]);
    m_circular_input->mCircularGI->setPen(sPens[2]);
    m_circular_input->mHandleGI->setPen(sPens[2]);
  } else if (states_stack.back().black_set().is_bezier()) {
    m_bezier_input->mOngoingPieceGI->setPen(sPens[2]);
    m_bezier_input->mBezierGI->setPen(sPens[2]);
    m_bezier_input->mHandle0GI->setPen(sPens[2]);
    m_bezier_input->mHandle1GI->setPen(sPens[2]);
  } else {
    m_polyline_input->mOngoingPieceGI->setPen(sPens[2]);
    m_polyline_input->mPolylineGI->setPen(sPens[2]);
    m_polyline_input->mHandleGI->setPen(sPens[2]);
  }
  set_conic_pen(sPens[2]);

}

void MainWindow::on_drawBrown_toggled(bool /* a_check */) {
  m_color_active = 3;

  if (states_stack.back().brown_set().is_linear()) {
    m_linear_input->mOngoingPieceGI->setPen(sPens[3]);
    m_linear_input->mLinearGI->setPen(sPens[3]);
    m_linear_input->mHandleGI->setPen(sPens[3]);
  } else if (states_stack.back().brown_set().is_circular()) {
    m_circular_input->mOngoingPieceGI->setPen(sPens[3]);
    m_circular_input->mCircularGI->setPen(sPens[3]);
    m_circular_input->mHandleGI->setPen(sPens[3]);
  } else if (states_stack.back().brown_set().is_bezier()) {
    m_bezier_input->mOngoingPieceGI->setPen(sPens[3]);
    m_bezier_input->mBezierGI->setPen(sPens[3]);
    m_bezier_input->mHandle0GI->setPen(sPens[3]);
    m_bezier_input->mHandle1GI->setPen(sPens[3]);
  } else {
    m_polyline_input->mOngoingPieceGI->setPen(sPens[3]);
    m_polyline_input->mPolylineGI->setPen(sPens[3]);
    m_polyline_input->mHandleGI->setPen(sPens[3]);
  }
  set_conic_pen(sPens[3]);

}

void MainWindow::on_drawYellow_toggled(bool /* a_check */) {
  m_color_active = 4;

  if (states_stack.back().yellow_set().is_linear()) {
    m_linear_input->mOngoingPieceGI->setPen(sPens[4]);
    m_linear_input->mLinearGI->setPen(sPens[4]);
    m_linear_input->mHandleGI->setPen(sPens[4]);
  } else if (states_stack.back().yellow_set().is_circular()) {
    m_circular_input->mOngoingPieceGI->setPen(sPens[4]);
    m_circular_input->mCircularGI->setPen(sPens[4]);
    m_circular_input->mHandleGI->setPen(sPens[4]);
  } else if (states_stack.back().yellow_set().is_bezier()) {
    m_bezier_input->mOngoingPieceGI->setPen(sPens[4]);
    m_bezier_input->mBezierGI->setPen(sPens[4]);
    m_bezier_input->mHandle0GI->setPen(sPens[4]);
    m_bezier_input->mHandle1GI->setPen(sPens[4]);
  } else {
    m_polyline_input->mOngoingPieceGI->setPen(sPens[4]);
    m_polyline_input->mPolylineGI->setPen(sPens[4]);
    m_polyline_input->mHandleGI->setPen(sPens[4]);
  }
  set_conic_pen(sPens[4]);

}

void MainWindow::on_drawMagenta_toggled(bool /* a_check */) {
  m_color_active = 5;

  if (states_stack.back().magenta_set().is_linear()) {
    m_linear_input->mOngoingPieceGI->setPen(sPens[5]);
    m_linear_input->mLinearGI->setPen(sPens[5]);
    m_linear_input->mHandleGI->setPen(sPens[5]);
  } else if (states_stack.back().magenta_set().is_circular()) {
    m_circular_input->mOngoingPieceGI->setPen(sPens[5]);
    m_circular_input->mCircularGI->setPen(sPens[5]);
    m_circular_input->mHandleGI->setPen(sPens[5]);
  } else if (states_stack.back().magenta_set().is_bezier()) {
    m_bezier_input->mOngoingPieceGI->setPen(sPens[5]);
    m_bezier_input->mBezierGI->setPen(sPens[5]);
    m_bezier_input->mHandle0GI->setPen(sPens[5]);
    m_bezier_input->mHandle1GI->setPen(sPens[5]);
  } else {
    m_polyline_input->mOngoingPieceGI->setPen(sPens[5]);
    m_polyline_input->mPolylineGI->setPen(sPens[5]);
    m_polyline_input->mHandleGI->setPen(sPens[5]);
  }
  set_conic_pen(sPens[5]);

}

void MainWindow::on_drawAqua_toggled(bool /* a_check */) {
  m_color_active = 6;

  if (states_stack.back().aqua_set().is_linear()) {
    m_linear_input->mOngoingPieceGI->setPen(sPens[6]);
    m_linear_input->mLinearGI->setPen(sPens[6]);
    m_linear_input->mHandleGI->setPen(sPens[6]);
  } else if (states_stack.back().aqua_set().is_circular()) {
    m_circular_input->mOngoingPieceGI->setPen(sPens[6]);
    m_circular_input->mCircularGI->setPen(sPens[6]);
    m_circular_input->mHandleGI->setPen(sPens[6]);
  } else if (states_stack.back().aqua_set().is_bezier()) {
    m_bezier_input->mOngoingPieceGI->setPen(sPens[6]);
    m_bezier_input->mBezierGI->setPen(sPens[6]);
    m_bezier_input->mHandle0GI->setPen(sPens[6]);
    m_bezier_input->mHandle1GI->setPen(sPens[6]);
  } else {
    m_polyline_input->mOngoingPieceGI->setPen(sPens[6]);
    m_polyline_input->mPolylineGI->setPen(sPens[6]);
    m_polyline_input->mHandleGI->setPen(sPens[6]);
  }
  set_conic_pen(sPens[6]);

}

//extra utilities
void MainWindow::on_actionRecenter_triggered() { zoomToFit(); }

void MainWindow::on_actionOpenLinear_triggered() {
  open(QFileDialog::getOpenFileName(this,
                                    tr("Open Linear Polygon"), "./data",
                                    tr("Linear Curve files (*.lps)")));
}

void MainWindow::on_actionOpenDXF_triggered() {
  open(QFileDialog::getOpenFileName(this,
                                    tr("Open Line segments and circular arcs"), "./data",
                                    tr("Circular curve files (*.cps)")));
}

void MainWindow::on_actionOpenBezier_triggered() {
  open(QFileDialog::getOpenFileName(this,
                                    tr("Open Bezier Polygon"), "./data",
                                    tr("Bezier Curve files (*.bps)")));
}

void MainWindow::on_actionOpenPolyline_triggered() {
  open(QFileDialog::getOpenFileName(this,
                                    tr("Open Polyline Polygon"), "./data",
                                    tr("Polyline Curve files (*.mps)")));
}

bool MainWindow::read_linear(QString aFileName, Linear_polygon_set& rSet,
                             Linear_region_source_container& rSources) {
  bool rOK = false;

  std::ifstream in_file(qPrintable(aFileName));
  if (in_file) {
    try {
      typedef Linear_traits Gps_traits;
      typedef typename Gps_traits::X_monotone_curve_2 Linear_X_monotone_curve;
      typedef typename Gps_traits::Curve_2 Linear_curve;
      typedef std::vector <Linear_curve> Linear_curve_vector;
      typedef typename Gps_traits::X_monotone_curve_2 Linear_X_monotone_curve;
      typedef typename Gps_traits::Polygon_2 Linear_polygon;
      typedef typename Gps_traits::Point_2 Linear_point;
      typedef typename Kernel::Point_2 Point;
      typedef typename Kernel::FT FT;
      Linear_curve_vector mLinearPolygonPieces;

      std::string format;
      std::getline(in_file, format);

      bool lDoubleFormat = (format.length() >= 6 && format.substr(0, 6) == "DOUBLE");

      //number of linear polygon with holes
      unsigned int n_regions;
      in_file >> n_regions;

      for (unsigned int r = 0; r < n_regions; ++r) {
        //number of linear polygon including holes
        unsigned int n_boundaries;
        in_file >> n_boundaries;

        std::vector <Linear_polygon> Linear_polygons;

        for (unsigned int r = 0; r < n_boundaries; ++r) {
          // number of points in linear polygon
          unsigned int n_points;
          in_file >> n_points;
          std::vector <Point> points;

          std::vector <Linear_X_monotone_curve> xcvs;
          Gps_traits traits;
          auto make_x_monotone = traits.make_x_monotone_2_object();
          using Pnt = typename Gps_traits::Point_2;
          using Xcv = typename Gps_traits::X_monotone_curve_2;
          using Make_x_monotone_result = std::variant<Pnt, Xcv>;

          for (unsigned int j = 0; j < n_points; ++j) {
            double x, y;
            in_file >> x >> y;
            points.push_back(Point(x, y));
            if (j > 0) {
              mLinearPolygonPieces.push_back(Linear_curve(points[points.size() - 2],
                                                          points[points.size() - 1]));
            }
          }
          points.clear();

          for (auto it = mLinearPolygonPieces.begin();
               it != mLinearPolygonPieces.end(); ++it) {
            std::vector<Make_x_monotone_result> x_objs;

            make_x_monotone(*it, std::back_inserter(x_objs));
            for (auto i = 0; i < x_objs.size(); ++i) {
              auto *xcv = std::get_if<Linear_X_monotone_curve>(&x_objs[i]);
              CGAL_assertion(xcv != nullptr);
              xcvs.push_back(*xcv);
            }
          }

          if (xcvs.size() > 0) {
            Linear_point const &first_point = xcvs.front().source();
            Linear_point const &last_point = xcvs.back().target();
            FT fxs = first_point.x();
            FT fys = first_point.y();
            FT lxs = last_point.x();
            FT lys = last_point.y();
            xcvs.push_back(Linear_X_monotone_curve(Point(lxs, lys), Point(fxs, fys)));
            //for (auto &i:xcvs)
            //cout<<i<<endl;

            Linear_polygon lp(xcvs.begin(), xcvs.end());
            CGAL::Orientation orient = lp.orientation();
            if (orient == CGAL::CLOCKWISE) {
              lp.reverse_orientation();
            }
            Linear_polygon_with_holes lCPWH(lp);

            if (r == 0) {
              get_new_state(11);
              states_stack.back().active_set(m_color_active).linear().join(lCPWH);
              states_stack.back().active_linear_sources(m_color_active).push_back(lCPWH);
            }
            else {
              states_stack.back().result_set().clear();
              states_stack.back().result_linear_sources().clear();
              states_stack.back().result_set().linear().join(lCPWH);
              states_stack.back().result_linear_sources().push_back(lCPWH);
              states_stack.back().active_set(m_color_active).difference(states_stack.back().result_set());
              states_stack.back().result_set().clear();
              states_stack.back().result_linear_sources().clear();
            }
            mLinearPolygonPieces.clear();
          }
        }
        rOK = true;
      }
    }
    catch (...) {
      show_warning("Exception ocurred during reading of linear polygon set.");
    }
  }
  return rOK;
}

bool MainWindow::read_circular(QString aFileName, Circular_polygon_set& rSet,
                               Circular_region_source_container& rSources) {
  bool rOK = false;

  std::ifstream in_file(qPrintable(aFileName));
  if (in_file) {
    try {
      typedef CGAL::Gps_circle_segment_traits_2 <Kernel> Gps_traits;
      typedef typename Gps_traits::Curve_2 Circular_curve;
      typedef typename Gps_traits::X_monotone_curve_2 Circular_X_monotone_curve;
      typedef typename Gps_traits::Polygon_2 Circular_polygon;
      typedef typename Circular_polygon::Point_2 Arc_point;
      typedef typename Kernel::FT FT;
      typedef typename Kernel::Vector_2 Vector;
      typedef typename Kernel::Point_2 Point;
      typedef typename Kernel::Circle_2 Circle;

      typedef std::vector <Circular_curve> Circular_curve_vector;

      Circular_curve_vector mCircularPolygonPieces;

      std::string format;
      std::getline(in_file, format);

      bool lDoubleFormat = (format.length() >= 6 && format.substr(0, 6) == "DOUBLE");

      //number of linear polygon with holes
      unsigned int n_regions;
      in_file >> n_regions;

      for (unsigned int r = 0; r < n_regions; ++r) {
        //number of linear polygon including holes
        unsigned int n_boundaries;
        in_file >> n_boundaries;

        std::vector <Circular_polygon> Circular_polygons;

        for (unsigned int r = 0; r < n_boundaries; ++r) {
          // number of points in linear polygon
          unsigned int n_points;
          in_file >> n_points;
          std::vector <Point> points;

          std::vector <Circular_X_monotone_curve> xcvs;
          Gps_traits traits;
          auto make_x_monotone = traits.make_x_monotone_2_object();
          using Pnt = typename Gps_traits::Point_2;
          using Xcv = typename Gps_traits::X_monotone_curve_2;
          using Make_x_monotone_result = std::variant<Pnt, Xcv>;

          for (unsigned int j = 0; j < n_points; ++j) {
            unsigned int bulge;
            in_file >> bulge;
            if (bulge) {
              double sx, sy, mx, my, tx, ty;

              in_file >> sx >> sy;
              in_file >> mx >> my;
              in_file >> tx >> ty;

              mCircularPolygonPieces.push_back(Circular_curve(Point(sx, sy),
                                                              Point(mx, my),
                                                              Point(tx, ty)));
            }
            else  //3576.382857 3576.382064
            {
              double sx, sy, tx, ty;

              in_file >> sx >> sy;
              in_file >> tx >> ty;

              mCircularPolygonPieces.push_back(Circular_curve(Point(sx, sy), Point(tx, ty)));
            }
          }

          if (mCircularPolygonPieces.size() > 0) {
            Gps_traits traits;
            auto make_x_monotone = traits.make_x_monotone_2_object();;

            std::vector <Circular_X_monotone_curve> xcvs;
            for (auto it = mCircularPolygonPieces.begin(); it != mCircularPolygonPieces.end(); ++it) {
              std::vector<Make_x_monotone_result> x_objs;
              make_x_monotone(*it, std::back_inserter(x_objs));

              auto *xcv = std::get_if<Circular_X_monotone_curve>(&x_objs[0]);
              //if ((*xcv).is_linear() && mCircularPolygonPieces.size() == 1) return;
              for (auto i = 0; i < x_objs.size(); ++i) {
                auto *xcv = std::get_if<Circular_X_monotone_curve>(&x_objs[i]);
                CGAL_assertion(xcv != nullptr);
                xcvs.push_back(*xcv);
              }
            }
            if (xcvs.size() > 0) {
              Circular_polygon cp(xcvs.begin(), xcvs.end());

              CGAL::Orientation orient = cp.orientation();

              if (orient == CGAL::CLOCKWISE) {
                cp.reverse_orientation();
              }

              Circular_polygon_with_holes lCPWH(cp);

              if (r == 0) {
                // processInput(CGAL::make_object(cp));
                get_new_state(11);
                states_stack.back().active_set(m_color_active).circular().join(lCPWH);
                states_stack.back().active_circular_sources(m_color_active).push_back(lCPWH);
              }
              else {
                states_stack.back().result_set().clear();
                states_stack.back().result_circular_sources().clear();
                states_stack.back().result_set().circular().join(lCPWH);
                states_stack.back().result_circular_sources().push_back(lCPWH);
                states_stack.back().active_set(m_color_active).difference(states_stack.back().result_set());
                states_stack.back().result_set().clear();
                states_stack.back().result_circular_sources().clear();
              }
            }
            mCircularPolygonPieces.clear();
          }
        }
        rOK = true;
      }
    }
    catch (...) {
      show_warning("Exception occured during reading of circular polygon set.");
    }
  }
  return rOK;
}

bool MainWindow::read_polyline(QString aFileName, Polyline_polygon_set& rSet,
                               Polyline_region_source_container& rSources) {
  bool rOK = false;

  std::ifstream in_file(qPrintable(aFileName));
  if (in_file) {
    try {
      typedef Polyline_traits Gps_traits;
      typedef typename Gps_traits::X_monotone_curve_2 Polyline_X_monotone_curve;
      typedef typename Gps_traits::Curve_2 Polyline_curve;
      typedef typename Gps_traits::X_monotone_curve_2 Polyline_X_monotone_curve;
      typedef typename Gps_traits::Polygon_2 Polyline_polygon;
      typedef typename Gps_traits::Point_2 Polyline_point;
      typedef typename Kernel::Point_2 Point;
      typedef typename Kernel::FT FT;
      typedef std::vector <Polyline_curve> Polyline_curve_vector;

      typedef CGAL::Arr_segment_traits_2 <Kernel> Segment_traits_2;
      typedef CGAL::Gps_traits_2 <CGAL::Arr_polycurve_traits_2<Segment_traits_2>> Base;
      typedef typename Base::X_monotone_subcurve_2 X_monotone_subcurve_2;

      Polyline_curve_vector mPolylinePolygonPieces;

      std::string format;
      std::getline(in_file, format);

      bool lDoubleFormat = (format.length() >= 6 && format.substr(0, 6) == "DOUBLE");

      unsigned int n_regions;
      in_file >> n_regions;

      for (unsigned int r = 0; r < n_regions; ++r) {

        unsigned int n_boundaries;
        in_file >> n_boundaries;

        std::vector <Polyline_polygon> Polyline_polygons;

        for (unsigned int r = 0; r < n_boundaries; ++r) {

          unsigned int n_points;
          in_file >> n_points;
          std::vector<Point> points;

          std::vector<Polyline_X_monotone_curve> xcvs;
          Gps_traits traits;
          auto make_x_monotone = traits.make_x_monotone_2_object();
          using Pnt = typename Gps_traits::Point_2;
          using Xcv = typename Gps_traits::X_monotone_curve_2;
          using Make_x_monotone_result = std::variant<Pnt, Xcv>;

          for (unsigned int j = 0; j < n_points; ++j) {
            double x, y;
            in_file >> x >> y;
            points.push_back(Point(x, y));
            int pt_pos=0;
            if (points.size() > 1) {
              Gps_traits x;
              X_monotone_subcurve_2 seg = x.subcurve_traits_2()->
                construct_x_monotone_curve_2_object()(points[points.size()-2],
                                                      points[points.size()-1]);
              //cout<<points[points.size()-2]<<" "<<points[points.size() - 1]<<endl;

              mPolylinePolygonPieces.push_back(Polyline_curve(seg));
            }
          }
          points.clear();

          for (auto it = mPolylinePolygonPieces.begin();
               it != mPolylinePolygonPieces.end(); ++it) {
            std::vector<Make_x_monotone_result> x_objs;

            make_x_monotone(*it, std::back_inserter(x_objs));
            for (auto i = 0; i < x_objs.size(); ++i) {
              auto* xcv = std::get_if<Polyline_X_monotone_curve>(&x_objs[i]);
              CGAL_assertion(xcv != nullptr);
              xcvs.push_back(*xcv);
            }
          }
          if (xcvs.size() > 0) {
            Polyline_point const &first_point = xcvs.front()[0].source();
            Polyline_point const &last_point = xcvs.back()[xcvs.back().number_of_subcurves() - 1].target();
            FT fxs = first_point.x();
            FT fys = first_point.y();
            FT lxs = last_point.x();
            FT lys = last_point.y();

            Gps_traits x;
            X_monotone_subcurve_2 seg = x.subcurve_traits_2()->
              construct_x_monotone_curve_2_object()(Point(lxs, lys), Point(fxs, fys));

            xcvs.push_back(Polyline_X_monotone_curve(seg));
            //for(auto &i:xcvs)
            //cout<<i<<endl;

            Polyline_polygon pp(xcvs.begin(), xcvs.end());
            CGAL::Orientation orient = pp.orientation();
            Polyline_polygon_with_holes PPWH(pp);
            if (orient == CGAL::CLOCKWISE) {
              pp.reverse_orientation();
            }
            if (r == 0) {
              get_new_state(11);
              states_stack.back().active_set(m_color_active).polyline().join(PPWH);
              states_stack.back().active_polyline_sources(m_color_active).push_back(PPWH);
            }
            else {
              states_stack.back().result_set().clear();
              states_stack.back().result_polyline_sources().clear();
              states_stack.back().result_set().polyline().join(PPWH);
              states_stack.back().result_polyline_sources().push_back(PPWH);
              states_stack.back().active_set(m_color_active).difference(states_stack.back().result_set());
              states_stack.back().result_set().clear();
              states_stack.back().result_polyline_sources().clear();
            }
            mPolylinePolygonPieces.clear();
          }
        }
        rOK = true;
      }
      //cout << "end of read polyline" << endl<<endl;
    }
    catch (...) {
      show_warning("Exception ocurred during reading of linear polygon set.");
    }
  }
  return rOK;
}

bool MainWindow::read_bezier(QString aFileName) {

  bool rOK = false;

  std::ifstream in_file(qPrintable(aFileName));

  if (in_file) {
    try {
      std::string format;
      std::getline(in_file, format);

      bool lDoubleFormat = (format.length() >= 6 && format.substr(0, 6) == "DOUBLE");

      // Read the number of bezier polygon with holes
      unsigned int n_regions;
      in_file >> n_regions;

      for (unsigned int r = 0; r < n_regions; ++r) {
        Bezier_polygon_vector bezier_polygons;
        Bezier_region_source br_source;
        // Read the number of bezier curves.
        unsigned int n_boundaries;
        in_file >> n_boundaries;

        for (unsigned int b = 0; b < n_boundaries; ++b) {
          Bezier_boundary_source bb_source;

          // Read the number of bezier curves.
          unsigned int n_curves;
          in_file >> n_curves;

          // Read the curves one by one, and construct the general polygon these
          // curve form (the outer boundary and the holes inside it).

          std::vector <Bezier_X_monotone_curve> xcvs;

          for (unsigned int k = 0; k < n_curves; ++k) {
            // Read the current curve and subdivide it into x-monotone subcurves.

            using Pnt = typename Bezier_traits::Point_2;
            using Xcv = typename Bezier_traits::X_monotone_curve_2;
            using Make_x_monotone_result = std::variant<Pnt, Xcv>;
            Bezier_X_monotone_curve xcv;
            Bezier_traits traits;
            std::vector<Make_x_monotone_result> x_objs;
            auto make_x_monotone = traits.make_x_monotone_2_object();

            Bezier_curve b = read_bezier_curve(in_file, lDoubleFormat);
            if (b.number_of_control_points() >= 2) {
              bb_source.push_back(b);
              make_x_monotone(b, std::back_inserter(x_objs));

              for (auto i = 0; i < x_objs.size(); ++i) {
                auto *xcv = std::get_if<Bezier_X_monotone_curve>(&x_objs[i]);
                CGAL_assertion(xcv != nullptr);
                xcvs.push_back(*xcv);
              }
            }
          }

          Bezier_polygon pgn(xcvs.begin(), xcvs.end());

          CGAL::Orientation orient = pgn.orientation();
          //TRACE( "  Orientation: " << orient ) ;
          if (orient == CGAL::CLOCKWISE) pgn.reverse_orientation();

          if (b == 0) {
            get_new_state(11);
            states_stack.back().active_set(m_color_active).bezier().join(Bezier_polygon_with_holes(pgn));
            Bezier_region_source br;
            br.push_back(bb_source);
            states_stack.back().active_bezier_sources(m_color_active).push_back(br);
          } else {
            states_stack.back().result_set().clear();
            states_stack.back().result_bezier_sources().clear();
            states_stack.back().result_set().bezier().join(Bezier_polygon_with_holes(pgn));
            Bezier_region_source br;
            br.push_back(bb_source);
            states_stack.back().result_bezier_sources().push_back(br);
            states_stack.back().active_set(m_color_active).difference(states_stack.back().result_set());
            states_stack.back().result_set().clear();
            states_stack.back().result_bezier_sources().clear();
          }
        }
        rOK = true;
      }
    }
    catch (const std::exception &e) {
      // std::string s = e.what();
      // show_error(s);
      on_actionUndo_triggered();
    }
  }
  return rOK;
}

Bezier_curve MainWindow::read_bezier_curve(std::istream& is,
                                           bool aDoubleFormat) {
  // Read the number of control points.
  unsigned int n;
  is >> n;

  // Read the control points.
  std::vector <Bezier_rat_point> ctrl_pts;

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
  std::vector <Bezier_rat_point> ctrl_pts2;
  typedef std::vector<Bezier_rat_point>::const_iterator cp_const_iterator;
  cp_const_iterator beg = ctrl_pts.begin();
  cp_const_iterator end = ctrl_pts.end();
  cp_const_iterator last = end - 1;
  ctrl_pts2.push_back(*beg);
  if (ctrl_pts.size() > 2) {
    cp_const_iterator curr = beg;
    cp_const_iterator next1 = curr + 1;
    cp_const_iterator next2 = next1 + 1;
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

//save linear polygon
bool save_linear(QString aFileName, Linear_polygon_set &rSet) {
  bool rOK = false;

  Linear_polygon_set lps;
  typedef typename Kernel::Point_2 Point;
  std::list <Polygon_2> pgns;

  std::ofstream out_file(qPrintable(aFileName));
  if (out_file) {
    out_file << "DOUBLE" << std::endl;
    std::vector <Linear_polygon_with_holes> lpwh_container;
    rSet.polygons_with_holes(std::back_inserter(lpwh_container));

    out_file << lpwh_container.size() << std::endl;

    for (auto rit = lpwh_container.begin();
         rit != lpwh_container.end(); ++rit) {
      Linear_polygon_with_holes lpwh = *rit;

      out_file << " " << (1 + lpwh.number_of_holes()) << std::endl;

      int cc = lpwh.outer_boundary().size();
      int lc = cc - 1;
      out_file << "  " << cc << std::endl;
      int i = 0;

      for (auto cit = lpwh.outer_boundary().curves_begin();
           cit != lpwh.outer_boundary().curves_end(); ++cit, ++i) {
        auto pt = cit->source();
        out_file << "   " << CGAL::to_double(pt.x()) << " " << CGAL::to_double(pt.y()) << std::endl;
      }

      for (auto hit = lpwh.holes_begin(); hit != lpwh.holes_end(); ++hit) {
        int cc = hit->size();
        int lc = cc - 1;
        out_file << "  " << cc << std::endl;
        int i = 0;

        for (auto cit = hit->curves_begin(); cit != hit->curves_end();
             ++cit, ++i) {
          auto pt = cit->source();
          out_file << "   " << CGAL::to_double(pt.x()) << " "
                   << CGAL::to_double(pt.y()) << std::endl;
        }
      }
      rOK = true;
    }
  }
  return rOK;
}

bool save_circular(QString aFileName, Circular_polygon_set &rSet)
{
  bool rOK = false;
  std::ofstream out_file(qPrintable(aFileName));

  if (out_file) {
    out_file << "DOUBLE" << std::endl;
    std::vector <Circular_polygon_with_holes> cpwh_container;
    rSet.polygons_with_holes(std::back_inserter(cpwh_container));
    out_file << cpwh_container.size() << std::endl;

    for (auto rit = cpwh_container.begin();
         rit != cpwh_container.end(); ++rit) {
      Circular_polygon_with_holes cpwh = *rit;
      out_file << " " << (1 + cpwh.number_of_holes()) << std::endl;
      int cc = cpwh.outer_boundary().size();
      int lc = cc - 1;
      out_file << "  " << cc << std::endl;
      int i = 0;

      for (auto cit = cpwh.outer_boundary().curves_begin();
           cit != cpwh.outer_boundary().curves_end(); ++cit, ++i) {
        auto pt = cit->source();
        if (cit->is_circular()) {
          out_file << "   1" << std::endl;
          out_file << "    " << CGAL::to_double(pt.x()) << " "
                   << CGAL::to_double(pt.y()) << std::endl;

          double r, mx, my, tx, ty, cx, cy, sx, sy, st, tnx, tny, frmst;
          r = sqrt(CGAL::to_double(cit->supporting_circle().squared_radius())); //radius r
          sx = CGAL::to_double(cit->source().x()); // source x()
          sy = CGAL::to_double(cit->source().y()); // source y()
          tx = CGAL::to_double(cit->target().x()); // target x()
          ty = CGAL::to_double(cit->target().y()); // target y()
          cx = CGAL::to_double(cit->supporting_circle().center().x()); // center x()
          cy = CGAL::to_double(cit->supporting_circle().center().y()); // center y()
          st = sqrt((sx - tx) * (sx - tx) + (sy - ty) * (sy - ty)); //Source to target euclides distance ST
          tnx = tx - cx; // origin shifting to center
          tny = ty - cy; // origin shifting to center
          frmst = sqrt((4 * r * r) - (st * st)); // intermediate value

          if (cit->supporting_circle().orientation() == CGAL::CLOCKWISE) {
            mx = (((frmst * tnx) - (st * tny)) / (2 * r)) + cx;
            my = (((st * tnx) + (frmst * tny)) / (2 * r)) + cy;
          } else {
            mx = (((tnx * frmst) + (tny * st)) / (2 * r)) + cx;
            my = (((tny * frmst) - (st * tnx)) / (2 * r)) + cy;
          }

          out_file << "    " << mx << " " << my << std::endl;
        }
        else {
          out_file << "   0" << std::endl;
          out_file << "    " << CGAL::to_double(pt.x()) << " "
                   << CGAL::to_double(pt.y()) << std::endl;
        }
        pt = cit->target();
        out_file << "    " << CGAL::to_double(pt.x()) << " "
                 << CGAL::to_double(pt.y()) << std::endl;
      }

      for (auto hit = cpwh.holes_begin(); hit != cpwh.holes_end(); ++hit) {
        int cc = hit->size();
        int lc = cc - 1;
        out_file << "  " << cc << std::endl;
        int i = 0;

        for (auto cit = hit->curves_begin(); cit != hit->curves_end();
             ++cit, ++i) {
          auto pt = cit->source();
          if (cit->is_circular()) {
            out_file << "   1" << std::endl;
            out_file << "    " << CGAL::to_double(pt.x()) << " "
                     << CGAL::to_double(pt.y()) << std::endl;

            double r, mx, my, tx, ty, cx, cy, sx, sy, st, tnx, tny, frmst;
            r = sqrt(CGAL::to_double(cit->supporting_circle().squared_radius())); //radius r
            sx = CGAL::to_double(cit->source().x()); // source x()
            sy = CGAL::to_double(cit->source().y()); // source y()
            tx = CGAL::to_double(cit->target().x()); // target x()
            ty = CGAL::to_double(cit->target().y()); // target y()
            cx = CGAL::to_double(cit->supporting_circle().center().x()); // center x()
            cy = CGAL::to_double(cit->supporting_circle().center().y()); // center y()
            st = sqrt((sx - tx) * (sx - tx)
                      + (sy - ty) * (sy - ty)); //Source to target euclides distance ST
            tnx = tx - cx; // origin shifting to center
            tny = ty - cy; // origin shifting to center
            frmst = sqrt((4 * r * r) - (st * st)); // intermediate value

            if (cit->supporting_circle().orientation() == CGAL::CLOCKWISE) {
              mx = (((frmst * tnx) - (st * tny)) / (2 * r)) + cx;
              my = (((st * tnx) + (frmst * tny)) / (2 * r)) + cy;
            } else {
              mx = (((tnx * frmst) + (tny * st)) / (2 * r)) + cx;
              my = (((tny * frmst) - (st * tnx)) / (2 * r)) + cy;
            }

            out_file << "    " << mx << " " << my << std::endl;
          }
          else {
            out_file << "   0" << std::endl;
            out_file << "    " << CGAL::to_double(pt.x()) << " "
                     << CGAL::to_double(pt.y()) << std::endl;
          }
          pt = cit->target();
          out_file << "    " << CGAL::to_double(pt.x()) << " " << CGAL::to_double(pt.y()) << std::endl;
        }
      }
      rOK = true;
    }
  }
  return rOK;
}

bool save_polyline(QString aFileName, Polyline_polygon_set &rSet) {
  bool rOK = false;

  Polyline_polygon_set pps;
  typedef typename Kernel::Point_2 Point;
  std::list <Polygon_2> pgns;

  std::ofstream out_file(qPrintable(aFileName));
  if (out_file) {
    out_file << "DOUBLE" << std::endl;
    std::vector <Polyline_polygon_with_holes> lpwh_container;
    rSet.polygons_with_holes(std::back_inserter(lpwh_container));

    out_file << lpwh_container.size() << std::endl;

    for (auto rit = lpwh_container.begin(); rit != lpwh_container.end(); ++rit)
    {
      Polyline_polygon_with_holes lpwh = *rit;

      out_file << " " << (1 + lpwh.number_of_holes()) << std::endl;

      int cc = lpwh.outer_boundary().size();
      int lc = cc - 1;
      out_file << "  " << cc << std::endl;
      int i = 0;

      for (auto cit = lpwh.outer_boundary().curves_begin();
           cit != lpwh.outer_boundary().curves_end(); ++cit, ++i) {
        auto it = cit->points_begin();
        out_file << " " << CGAL::to_double((*it).x()) << " "
                 << CGAL::to_double((*it).y()) << std::endl;
        /*while (it != cit->points_end()) {
          auto pt = *it;
          cout<<pt<<endl;
          (*it)++;
          }
          cout<<endl;*/
      }

      for (auto hit = lpwh.holes_begin(); hit != lpwh.holes_end(); ++hit) {
        int cc = hit->size();
        int lc = cc - 1;
        out_file << "  " << cc << std::endl;
        int i = 0;

        for (auto cit = hit->curves_begin(); cit != hit->curves_end();
             ++cit, ++i) {
          auto it = cit->points_begin();
          out_file << " " << CGAL::to_double((*it).x()) << " "
                   << CGAL::to_double((*it).y()) << std::endl;
          /*while (it != cit->points_end()) {
            auto pt = *it;
            out_file << "   " << CGAL::to_double(pt.x()) << " " << CGAL::to_double(pt.y()) << std::endl;
            *it++;
            }*/
        }
      }
      rOK = true;
    }
  }
  return rOK;
}

//bezier file in saved in two parts
void save_bezier_polygon(std::ostream &out_file, Bezier_polygon const &aBP) {
  typedef std::vector<Bezier_rat_point> Bezier_rat_point_vector;

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

bool save_bezier_result(QString aFileName, Bezier_polygon_set const& aSet) {
  bool rOK = false;
  std::ofstream out_file(qPrintable(aFileName));
  if (out_file) {
    out_file << "DOUBLE" << std::endl;
    std::vector <Bezier_polygon_with_holes> bpwh_container;
    aSet.polygons_with_holes(std::back_inserter(bpwh_container));
    out_file << bpwh_container.size() << std::endl;

    for (auto rit = bpwh_container.begin();
         rit != bpwh_container.end(); ++rit) {
      Bezier_polygon_with_holes bpwh = *rit;
      out_file << " " << (1 + bpwh.number_of_holes()) << std::endl;
      save_bezier_polygon(out_file, bpwh.outer_boundary());
      for (auto hit = bpwh.holes_begin(); hit != bpwh.holes_end(); ++hit)
        save_bezier_polygon(out_file, *hit);

      rOK = true;
    }
  }
  return rOK;
}

//"file:save active bucket" button pressed
void MainWindow::on_actionSaveCurrentBucket_triggered() {
  if (m_circular_active) {
    if (!save_circular(QFileDialog::getSaveFileName(this,
                                                    tr("Save Result Circular Polygon Set"), "/data/index.cps",
                                                    tr("Circular Curve files (*.cps)")),
                       states_stack.back().active_set(m_color_active).circular()
                       )
        ) {
      show_error("Cannot save circular polygon set.");
    }
  }
  else if (m_bezier_active) {
    if (!save_bezier_result(QFileDialog::getSaveFileName(this,
                                                         tr("Save Result Bezier Polygon Set"), "/data/index.bps",
                                                         tr("Bezier Curve files (*.bps)")),
                            states_stack.back().active_set(m_color_active).bezier()
                            )
        ) {
      show_error("Cannot save bezier polygon set.");
    }
  }
  else if (m_conic_active) {
    show_error("Saving conic polygon sets is not supported yet.");
  }
  else if (m_polyline_active) {
    if (!save_polyline(QFileDialog::getSaveFileName(this,
                                                    tr("Save Result Polyline Polygon Set"), "/data/index.mps",
                                                    tr("Polyline Curve files (*.mps)")),
                       states_stack.back().active_set(m_color_active).polyline()
                       )
        ) {
      show_error("Cannot save polyline polygon set.");
    }
  }
  else {
    if (!save_linear(QFileDialog::getSaveFileName(this,
                                                  tr("Save Result Linear Polygon Set"), "/data/index.lps",
                                                  tr("Linear Curve files (*.lps)")),
                     states_stack.back().active_set(m_color_active).linear()
                     )
        ) {
      show_error("Cannot save linear polygon set.");
    }
  }
}

//check out
void MainWindow::switch_set_type(Curve_set &aSet, int aType)
{
  unlink_GI(aSet.gi());
  aSet.reset_type(aType);
  link_GI(aSet.gi());
  modelChanged();
}

void MainWindow::switch_sets_type(int aType) {
  switch_set_type(states_stack.back().blue_set(), aType);
  switch_set_type(states_stack.back().red_set(), aType);
  switch_set_type(states_stack.back().black_set(), aType);
  switch_set_type(states_stack.back().brown_set(), aType);
  switch_set_type(states_stack.back().yellow_set(), aType);
  switch_set_type(states_stack.back().magenta_set(), aType);
  switch_set_type(states_stack.back().aqua_set(), aType);
  switch_set_type(states_stack.back().result_set(), aType);
}

//switching from one polygon to another
bool MainWindow::ensure_circular_mode() {
  if (! m_circular_active) {
    bool lProceed = states_stack.back().blue_set().is_empty() && states_stack.back().red_set().is_empty() &&
      states_stack.back().black_set().is_empty() && states_stack.back().brown_set().is_empty() &&
      states_stack.back().yellow_set().is_empty() && states_stack.back().magenta_set().is_empty() &&
      states_stack.back().aqua_set().is_empty() && states_stack.back().result_set().is_empty();

    if (! lProceed)
      lProceed = ask_user_yesno("Circular mode switch",
                                "You are about to load a circular poygon, but there are linear/bezier/polyline curves already loaded.\n" \
                                "Both types are not interoperable. In order to proceed, the polygons must be removed first.\n" \
                                "Once deleted you cannot undo the action.\n" \
                                "Yes to remove and proceed?\n");

    if (lProceed) {
      switch_sets_type(2);
      m_linear_input->Reset();
      m_bezier_input->Reset();
      m_polyline_input->Reset();
      m_circular_active = true;
      m_conic_active = false;
      m_conic_input->Reset();
      m_bezier_active = false;
      m_polyline_active = false;
    }
  }
  return m_circular_active;
}

bool MainWindow::ensure_bezier_mode() {
  if (! m_bezier_active) {
    bool lProceed = states_stack.back().blue_set().is_empty() && states_stack.back().red_set().is_empty() &&
      states_stack.back().black_set().is_empty() && states_stack.back().brown_set().is_empty() &&
      states_stack.back().yellow_set().is_empty() && states_stack.back().magenta_set().is_empty() &&
      states_stack.back().aqua_set().is_empty() && states_stack.back().result_set().is_empty();

    if (!lProceed)
      lProceed = ask_user_yesno("Bezier mode switch",
                                "You are about to load a Bezier curve, but there are linear/bezier/polyline polygons already loaded.\n" \
                                "Both types are not interoperable. In order to proceed, the polygons must be removed first.\n" \
                                "Once deleted you cannot undo the action.\n" \
                                "Yes to remove and proceed?\n");

    if (lProceed) {
      switch_sets_type(3);
      m_circular_input->Reset();
      m_linear_input->Reset();
      m_polyline_input->Reset();
      m_bezier_active = true;
      m_conic_active = false;
      m_conic_input->Reset();
      m_circular_active = false;
      m_polyline_active = false;
    }
  }
  return m_bezier_active;
}

bool MainWindow::ensure_linear_mode() {
  if (active_type() != 1) {
    bool lProceed = states_stack.back().blue_set().is_empty() && states_stack.back().red_set().is_empty() &&
      states_stack.back().black_set().is_empty() && states_stack.back().brown_set().is_empty() &&
      states_stack.back().yellow_set().is_empty() && states_stack.back().magenta_set().is_empty() &&
      states_stack.back().aqua_set().is_empty() && states_stack.back().result_set().is_empty();

    if (! lProceed)
      lProceed = ask_user_yesno("Linear mode switch",
                                "You are about to load a linear poygon, but there are circular/bezier/polyline polygons already loaded.\n" \
                                "Both types are not interoperable. In order to proceed, the polygons must be removed first.\n" \
                                "Once deleted you cannot undo the action.\n"
                                "Yes to remove and proceed?\n");

    // cout<<"Linear Mode"<<endl;
    if (lProceed) {
      //cout<<"fault 1"<<endl;
      switch_sets_type(1);
      m_circular_input->Reset();
      m_bezier_input->Reset();
      m_polyline_input->Reset();
      m_circular_active = false;
      m_bezier_active = false;
      //cout<<"fault 3"<<endl;
      m_polyline_active = false;
      m_conic_active = false;
      m_conic_input->Reset();
      //cout<<"End of ensure_linear_polygon()"<<endl;
    }
    //cout<<"end of this function"<<endl;
  }
  return active_type() == 1;
}

bool MainWindow::ensure_polyline_mode() {
  if (!m_polyline_active) {
    bool lProceed = states_stack.back().blue_set().is_empty() && states_stack.back().red_set().is_empty() &&
      states_stack.back().black_set().is_empty() && states_stack.back().brown_set().is_empty() &&
      states_stack.back().yellow_set().is_empty() && states_stack.back().magenta_set().is_empty() &&
      states_stack.back().aqua_set().is_empty() && states_stack.back().result_set().is_empty();

    if (!lProceed)
      lProceed = ask_user_yesno("Polyline mode switch",
                                "You are about to load a polyline polygon, but there are circular/bezier/linear polygons already loaded.\n" \
                                "Both types are not interoperable. In order to proceed, the polygons must be removed first.\n" \
                                "Once deleted you cannot undo the action.\n" \
                                "Yes to remove and proceed?\n");

    if (lProceed) {
      switch_sets_type(4);
      m_circular_input->Reset();
      m_bezier_input->Reset();
      m_linear_input->Reset();
      m_circular_active = false;
      m_bezier_active = false;
      m_polyline_active = true;
      m_conic_active = false;
      m_conic_input->Reset();
    }
  }
  return m_polyline_active;
}

//load polygons from file that is "file:open" clicked
void MainWindow::open(QString fileName) {
  if (!fileName.isEmpty()) {
    bool lRead = false;
    if (fileName.endsWith(".lps")) {
      if (ensure_linear_mode())
        lRead = read_linear(fileName, states_stack.back().active_set(m_color_active).linear(),
                            states_stack.back().active_linear_sources(m_color_active));
    }
    else if (fileName.endsWith(".cps")) {
      if (ensure_circular_mode())
        lRead = read_circular(fileName, states_stack.back().active_set(m_color_active).circular(),
                              states_stack.back().active_circular_sources(m_color_active));
    }
    else if (fileName.endsWith(".bps")) {
      if (ensure_bezier_mode())
        lRead = read_bezier(fileName);
    }
    else if (fileName.endsWith(".mps")) {
      if (ensure_polyline_mode())
        lRead = read_polyline(fileName, states_stack.back().active_set(m_color_active).polyline(),
                              states_stack.back().active_polyline_sources(m_color_active));
    }
    if (lRead) {
      modelChanged();
      // zoomToFit();
      this->addToRecentFiles(fileName);
    }
  }
}

void MainWindow::on_actionInsertCircular_toggled(bool aChecked) {
  if (aChecked) {
    this->graphicsView->setDragMode(QGraphicsView::NoDrag);
    if (ensure_circular_mode()) {
      if (!m_pan && !m_reset) {
        while (m_state_num > 1) {
          on_actionUndo_triggered();
        }
      }

      actionPAN->setChecked(false);
      m_pan = false;
      actionOpenLinear->setEnabled(false);
      actionOpenDXF->setEnabled(true);
      actionOpenBezier->setEnabled(false);
      actionOpenPolyline->setEnabled(false);
      actionInsertLinear->setChecked(false);
      actionInsertBezier->setChecked(false);
      actionInsertPolyline->setChecked(false);
      actionInsertConic->setChecked(false);
      install_input(m_circular_input);
      on_actionDeleteResult();
      actionMinkowski_SumH->setEnabled(false);

      m_circular_input->mOngoingPieceGI->setPen(sPens[m_color_active]);
      m_circular_input->mCircularGI->setPen(sPens[m_color_active]);
      m_circular_input->mHandleGI->setPen(sPens[m_color_active]);
    } else {
      actionInsertCircular->setChecked(false);
    }
  }
}

void MainWindow::on_actionInsertBezier_toggled(bool aChecked) {
  if (aChecked) {
    this->graphicsView->setDragMode(QGraphicsView::NoDrag);
    if (ensure_bezier_mode()) {
      if (!m_pan && !m_reset) {
        while (m_state_num > 1) {
          on_actionUndo_triggered();
        }
      }

      actionPAN->setChecked(false);
      m_pan = false;
      actionOpenLinear->setEnabled(false);
      actionOpenDXF->setEnabled(false);
      actionOpenBezier->setEnabled(true);
      actionOpenPolyline->setEnabled(false);
      actionInsertLinear->setChecked(false);
      actionInsertCircular->setChecked(false);
      actionInsertPolyline->setChecked(false);
      actionInsertConic->setChecked(false);
      install_input(m_bezier_input);
      on_actionDeleteResult();

      actionMinkowski_SumH->setEnabled(false);

      m_bezier_input->mOngoingPieceGI->setPen(sPens[m_color_active]);
      m_bezier_input->mBezierGI->setPen(sPens[m_color_active]);
      m_bezier_input->mHandle0GI->setPen(sPens[m_color_active]);
      m_bezier_input->mHandle1GI->setPen(sPens[m_color_active]);
    } else {
      actionInsertBezier->setChecked(false);
    }
  }
}

void MainWindow::on_actionInsertPolyline_toggled(bool aChecked) {
  if (aChecked) {
    this->graphicsView->setDragMode(QGraphicsView::NoDrag);
    if (ensure_polyline_mode()) {
      if (!m_pan && !m_reset) {
        while (m_state_num > 1) {
          on_actionUndo_triggered();
        }
      }

      actionPAN->setChecked(false);
      m_pan = false;
      actionOpenLinear->setEnabled(false);
      actionOpenDXF->setEnabled(false);
      actionOpenBezier->setEnabled(false);
      actionOpenPolyline->setEnabled(true);
      actionInsertLinear->setChecked(false);
      actionInsertBezier->setChecked(false);

      actionInsertCircular->setChecked(false);
      actionInsertConic->setChecked(false);
      install_input(m_polyline_input);
      on_actionDeleteResult();
      actionMinkowski_SumH->setEnabled(false);

      m_polyline_input->mOngoingPieceGI->setPen(sPens[m_color_active]);
      m_polyline_input->mPolylineGI->setPen(sPens[m_color_active]);
      m_polyline_input->mHandleGI->setPen(sPens[m_color_active]);
    } else {
      actionInsertPolyline->setChecked(false);
    }
  }
}

void MainWindow::on_actionInsertLinear_toggled(bool aChecked) {
  if (aChecked) {
    this->graphicsView->setDragMode(QGraphicsView::NoDrag);
    if (ensure_linear_mode()) {
      if (!m_pan && !m_reset) {
        while (m_state_num > 1) {
          on_actionUndo_triggered();
        }
      }
      actionPAN->setChecked(false);
      m_pan = false;
      actionOpenLinear->setEnabled(true);
      actionOpenDXF->setEnabled(false);
      actionOpenBezier->setEnabled(false);
      actionOpenPolyline->setEnabled(false);
      actionInsertCircular->setChecked(false);
      actionInsertBezier->setChecked(false);
      actionInsertPolyline->setChecked(false);
      actionInsertConic->setChecked(false);
      install_input(m_linear_input);
      on_actionDeleteResult();

      actionMinkowski_SumH->setEnabled(true);
    }
    else {
      actionInsertLinear->setChecked(false);
    }
  }
}

//complement
void MainWindow::on_actionComplementH_toggled(bool /* aChecked */) {
  if (! actionComplementH->isChecked()) return;
  auto operands = checked_buckets(m_complement_column);
  bool is_operand = std::find(operands.begin(), operands.end(),
                              m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionComplementH, is_operand)) return;
  uncheck_operations_except(actionComplementH);
  run_operation(Bucket_op::Complement, operands, actionComplementH);
}

//intersection
void MainWindow::on_actionIntersectionH_toggled(bool /* aChecked */) {
  if (! actionIntersectionH->isChecked()) return;
  auto operands = checked_buckets(m_intersection_column);
  bool is_operand = std::find(operands.begin(), operands.end(),
                              m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionIntersectionH, is_operand)) return;
  uncheck_operations_except(actionIntersectionH);
  run_operation(Bucket_op::Intersection, operands, actionIntersectionH);
}

//blue - red
//difference
void MainWindow::on_actionDifferenceH_toggled(bool /* aChecked */) {
  if (! actionDifferenceH->isChecked()) return;
  auto operands = checked_buckets(m_difference_column);
  if (operands.size() != 2) {
    actionDifferenceH->setChecked(false);
    ask_user_ok("Operation Error", "The difference takes exactly two buckets.\n");
    return;
  }
  bool is_operand = std::find(operands.begin(), operands.end(),
                              m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionDifferenceH, is_operand)) return;
  uncheck_operations_except(actionDifferenceH);
  run_operation(Bucket_op::Difference, operands, actionDifferenceH);
}

//symmetric difference
void MainWindow::on_actionSymmetric_DifferenceH_toggled(bool /* aChecked */) {
  if (! actionSymmetric_DifferenceH->isChecked()) return;
  auto operands = checked_buckets(m_symmetric_difference_column);
  if (operands.size() != 2) {
    actionSymmetric_DifferenceH->setChecked(false);
    ask_user_ok("Operation Error", "The symmetric difference takes exactly two buckets.\n");
    return;
  }
  bool is_operand = std::find(operands.begin(), operands.end(),
                              m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionSymmetric_DifferenceH, is_operand)) return;
  uncheck_operations_except(actionSymmetric_DifferenceH);
  run_operation(Bucket_op::Symmetric_difference, operands, actionSymmetric_DifferenceH);
}

//union
void MainWindow::on_actionUnionH_toggled(bool /* aChecked */) {
  if (! actionUnionH->isChecked()) return;
  auto operands = checked_buckets(m_union_column);
  bool is_operand = std::find(operands.begin(), operands.end(),
                              m_color_active) != operands.end();
  if (abort_for_draw_bucket(actionUnionH, is_operand)) return;
  uncheck_operations_except(actionUnionH);
  run_operation(Bucket_op::Join, operands, actionUnionH);
}

//copy
void MainWindow::on_actionCopyH_toggled(bool aChecked) {
  if (actionCopyH->isChecked()) {
    if (abort_for_draw_bucket(actionCopyH,
                              m_color_copy == m_color_active))
      return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    actionUnionH->setChecked(false);
    actionIntersectionH->setChecked(false);
    actionDifferenceH->setChecked(false);
    actionSymmetric_DifferenceH->setChecked(false);
    actionMinkowski_SumH->setChecked(false);
    actionMoveH->setChecked(false);

    get_new_state(7);
    if (m_color_copy < 7)
      log_statements({QString("%1.join(%2)").arg(bucket_name(m_color_active),
                                                 bucket_name(m_color_copy))});

    /*operation_name
    // COMPLEMENT_OP = 0
    // INTERSECTION_OP = 1
    // UNION_OP = 2
    // DIFFERENCE_OP = 3
    // SYMMETRIC_dIFFERENCE_OP = 4
    // MINKOWSKI_SUM_OP = 5
    // RESET_OP = 6
    // COPY_OP = 7
    // MOVE_OP = 8
    // CLEAR_OP = 9
    // DELETEALL_OP = 10
    // START_OP = 11*/

    states_stack.back().result_set().clear();
    states_stack.back().result_linear_sources().clear();
    states_stack.back().result_circular_sources().clear();
    states_stack.back().result_bezier_sources().clear();
    states_stack.back().result_polyline_sources().clear();

    switch (m_color_copy) {
     case 0:
      if (!states_stack.back().blue_set().is_empty())
        states_stack.back().result_set().assign(states_stack.back().blue_set());
      break;
     case 1:
      if (!states_stack.back().red_set().is_empty())
        states_stack.back().result_set().assign(states_stack.back().red_set());
      break;
     case 2:
      if (!states_stack.back().black_set().is_empty())
        states_stack.back().result_set().assign(states_stack.back().black_set());
      break;
     case 3:
      if (!states_stack.back().brown_set().is_empty())
        states_stack.back().result_set().assign(states_stack.back().brown_set());
      break;
     case 4:
      if (!states_stack.back().yellow_set().is_empty())
        states_stack.back().result_set().assign(states_stack.back().yellow_set());
      break;
     case 5:
      if (!states_stack.back().magenta_set().is_empty())
        states_stack.back().result_set().assign(states_stack.back().magenta_set());
      break;
     case 6:
      if (!states_stack.back().aqua_set().is_empty())
        states_stack.back().result_set().assign(states_stack.back().aqua_set());
      break;
     default:show_warning("Please select a Bucket!!!");
      break;
    }

    actionCopyH->setChecked(false);
    lDone = true;
    m_color_cm = 0; //copy
    if (!states_stack.back().result_set().is_empty())
      states_stack.back().active_set(m_color_active).join(states_stack.back().result_set());
    states_stack.back().result_set().clear();
    states_stack.back().result_linear_sources().clear();
    states_stack.back().result_circular_sources().clear();
    states_stack.back().result_bezier_sources().clear();
    states_stack.back().result_polyline_sources().clear();
    this->setCursor(old);
    if (lDone) modelChanged();
  }
}

//move
void MainWindow::on_actionMoveH_toggled(bool aChecked) {
  if (actionMoveH->isChecked()) {
    if (abort_for_draw_bucket(actionMoveH,
                              m_color_move_ext == m_color_active))
      return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    actionUnionH->setChecked(false);
    actionIntersectionH->setChecked(false);
    actionDifferenceH->setChecked(false);
    actionSymmetric_DifferenceH->setChecked(false);
    actionMinkowski_SumH->setChecked(false);
    actionCopyH->setChecked(false);

    get_new_state(8);
    if (m_color_move_ext < 7)
      log_statements({QString("%1.join(%2)")
                      .arg(bucket_name(m_color_active),
                           bucket_name(m_color_move_ext)),
                      QString("%1.clear()").arg(bucket_name(m_color_move_ext))});

    /*operation_name
    // COMPLEMENT_OP = 0
    // INTERSECTION_OP = 1
    // UNION_OP = 2
    // DIFFERENCE_OP = 3
    // SYMMETRIC_dIFFERENCE_OP = 4
    // MINKOWSKI_SUM_OP = 5
    // RESET_OP = 6
    // COPY_OP = 7
    // MOVE_OP = 8
    // CLEAR_OP = 9
    // DELETEALL_OP = 10
    // START_OP = 11*/

    states_stack.back().result_set().clear();
    states_stack.back().result_linear_sources().clear();
    states_stack.back().result_circular_sources().clear();
    states_stack.back().result_bezier_sources().clear();
    states_stack.back().result_polyline_sources().clear();

    switch (m_color_move_ext) {
     case 0:
      if (!states_stack.back().blue_set().is_empty()) {
        states_stack.back().result_set().assign(states_stack.back().blue_set());
        states_stack.back().active_set(m_color_active).join(states_stack.back().result_set());
        states_stack.back().blue_set().clear();
      }
      break;
     case 1:
      if (!states_stack.back().red_set().is_empty()) {
        states_stack.back().result_set().assign(states_stack.back().red_set());
        states_stack.back().active_set(m_color_active).join(states_stack.back().result_set());
        states_stack.back().red_set().clear();
      }
      break;
     case 2:
      if (!states_stack.back().black_set().is_empty()) {
        states_stack.back().result_set().assign(states_stack.back().black_set());
        states_stack.back().active_set(m_color_active).join(states_stack.back().result_set());
        states_stack.back().black_set().clear();
      }
      break;
     case 3:
      if (!states_stack.back().brown_set().is_empty()) {
        states_stack.back().result_set().assign(states_stack.back().brown_set());
        states_stack.back().active_set(m_color_active).join(states_stack.back().result_set());
        states_stack.back().brown_set().clear();
      }
      break;
     case 4:
      if (!states_stack.back().yellow_set().is_empty()) {
        states_stack.back().result_set().assign(states_stack.back().yellow_set());
        states_stack.back().active_set(m_color_active).join(states_stack.back().result_set());
        states_stack.back().yellow_set().clear();
      }
      break;
     case 5:
      if (!states_stack.back().magenta_set().is_empty()) {
        states_stack.back().result_set().assign(states_stack.back().magenta_set());
        states_stack.back().active_set(m_color_active).join(states_stack.back().result_set());
        states_stack.back().magenta_set().clear();
      }
      break;
     case 6:
      if (!states_stack.back().aqua_set().is_empty()) {
        states_stack.back().result_set().assign(states_stack.back().aqua_set());
        states_stack.back().active_set(m_color_active).join(states_stack.back().result_set());
        states_stack.back().aqua_set().clear();
      }
      break;
     default:show_warning("Please select a Bucket!!!");
      break;
    }

    actionMoveH->setChecked(false);
    states_stack.back().result_set().clear();
    states_stack.back().result_linear_sources().clear();
    states_stack.back().result_circular_sources().clear();
    states_stack.back().result_bezier_sources().clear();
    states_stack.back().result_polyline_sources().clear();

    lDone = true;
    m_color_cm = 1; //copy
    this->setCursor(old);
    if (lDone) modelChanged();
  }
}

//minkowski polygon(only for linear polygon)
Polygon_with_holes_2 MainWindow::getMinkInputPolygon(size_t color) {
  Linear_polygon_set lps;
  typedef std::vector <Linear_polygon_with_holes> Pgn_with_holes_container;
  typedef typename Kernel::Point_2 Point;
  std::list <Polygon_2> pgns;

  switch (color) {
   case 0:lps = states_stack.back().blue_set().linear();
    break;
   case 1:lps = states_stack.back().red_set().linear();
    break;
   case 2:lps = states_stack.back().black_set().linear();
    break;
   case 3:lps = states_stack.back().brown_set().linear();
    break;
   case 4:lps = states_stack.back().yellow_set().linear();
    break;
   case 5:lps = states_stack.back().magenta_set().linear();
    break;
   case 6:lps = states_stack.back().aqua_set().linear();
    break;
  }

  Pgn_with_holes_container res(lps.number_of_polygons_with_holes());
  res.begin() = lps.polygons_with_holes(res.begin());
  typename Pgn_with_holes_container::const_iterator it;

  Linear_polygon_with_holes temp;
  Polygon_with_holes_2 pwh;
  Polygon_2 p;
  //show_warning("Number of polygons with holes: "+to_string(lps.number_of_polygons_with_holes()));

  if (lps.number_of_polygons_with_holes() > 1) {
    m_disjoint = true;
  }

  if (!m_disjoint) {
    for (it = res.begin(); it != res.end(); ++it) {
      temp = *it;
      for (auto vit = temp.outer_boundary().curves_begin(); vit != temp.outer_boundary().curves_end(); vit++) {
        auto pt = vit->source();
        //show_warning(to_string(CGAL::to_double(pt.x()))+" , "+to_string(CGAL::to_double(pt.y())));
        p.push_back(Point(CGAL::to_double(pt.x()), CGAL::to_double(pt.y())));
      }
      if (p.orientation() == CGAL::CLOCKWISE) {
        p.reverse_orientation();
      }

      pgns.push_back(p);

      for (auto hi = temp.holes_begin(); hi != temp.holes_end(); ++hi) {
        Polygon_2 hole;
        auto vti = *hi;
        for (auto hit = vti.curves_begin(); hit != vti.curves_end(); hit++) {
          auto pti = hit->source();
          hole.push_back(Point(CGAL::to_double(pti.x()),
                               CGAL::to_double(pti.y())));
        }
        if (hole.orientation() == CGAL::COUNTERCLOCKWISE) {
          hole.reverse_orientation();
        }
        pgns.push_back(hole);
      }
    }
    std::list<Polygon_2>::iterator pit = pgns.begin();
    return Polygon_with_holes_2(pgns.front(), ++pit, pgns.end());
  }
  return pwh;
}

//minkowski button clicked
void MainWindow::on_actionMinkowski_SumH_toggled(bool aChecked) {
  if (actionMinkowski_SumH->isChecked()) {
    if (abort_for_draw_bucket(
          actionMinkowski_SumH,
          draw_row_checked({showBlueMink_Sum, showRedMink_Sum, showBlackMink_Sum,
                           showBrownMink_Sum, showYellowMink_Sum, showMagentaMink_Sum,
                           showAquaMink_Sum})))
      return;
    bool lDone = false;
    QCursor old = this->cursor();
    this->setCursor(Qt::WaitCursor);

    if (active_type() == 1) {
      actionComplementH->setChecked(false);
      actionUnionH->setChecked(false);
      actionIntersectionH->setChecked(false);
      actionDifferenceH->setChecked(false);
      actionSymmetric_DifferenceH->setChecked(false);

      get_new_state(5);
      log_statements({"# Minkowski sum (not supported in the program yet)"});

      /*operation_name
      // COMPLEMENT_OP = 0
      // INTERSECTION_OP = 1
      // UNION_OP = 2
      // DIFFERENCE_OP = 3
      // SYMMETRIC_dIFFERENCE_OP = 4
      // MINKOWSKI_SUM_OP = 5
      // RESET_OP = 6
      // COPY_OP = 7
      // MOVE_OP = 8
      // CLEAR_OP = 9
      // DELETEALL_OP = 10
      // START_OP = 11*/


      size_t count = 0;
      if (showBlueMink_Sum->isChecked()) count++;
      if (showRedMink_Sum->isChecked()) count++;
      if (showBlackMink_Sum->isChecked()) count++;
      if (showBrownMink_Sum->isChecked()) count++;
      if (showYellowMink_Sum->isChecked()) count++;
      if (showMagentaMink_Sum->isChecked()) count++;
      if (showAquaMink_Sum->isChecked()) count++;

      if (count == 2) {
        size_t color1 = 111;
        size_t color2 = 1111;

        if (showBlueMink_Sum->isChecked()) color1 = 0;
        if (showRedMink_Sum->isChecked()) {
          if (color1 < 1) color2 = 1;
          else color1 = 1;
        }

        if (showBlackMink_Sum->isChecked()) {
          if (color1 < 2) color2 = 2;
          else color1 = 2;
        }

        if (showBrownMink_Sum->isChecked()) {
          if (color1 < 3) color2 = 3;
          else color1 = 3;
        }

        if (showYellowMink_Sum->isChecked()) {
          if (color1 < 4) color2 = 4;
          else color1 = 4;
        }

        if (showMagentaMink_Sum->isChecked()) {
          if (color1 < 5) color2 = 5;
          else color1 = 5;
        }

        if (showAquaMink_Sum->isChecked()) {
          color2 = 6;
        }

        typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
        typedef Kernel::Point_2 Point_2;

        Polygon_with_holes_2 p0, p1;

        // Compute the Minkowski sum using the decomposition approach.
        CGAL::Small_side_angle_bisector_decomposition_2 <Kernel> ssab_decomp;

        if (color1 == 0 && !states_stack.back().blue_set().is_empty()) {
          p0 = getMinkInputPolygon(color1);
          if (color2 == 1 && !states_stack.back().red_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 2 && !states_stack.back().black_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 3 && !states_stack.back().brown_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 4 && !states_stack.back().yellow_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 5 && !states_stack.back().magenta_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 6 && !states_stack.back().aqua_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
        }
        else if (color1 == 1 && !states_stack.back().red_set().is_empty()) {
          p0 = getMinkInputPolygon(color1);
          if (color2 == 2 && !states_stack.back().black_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 3 && !states_stack.back().brown_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 4 && !states_stack.back().yellow_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 5 && !states_stack.back().magenta_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
          else if (color2 == 6 && !states_stack.back().aqua_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
        }
        else if (color1 == 2 && !states_stack.back().black_set().is_empty()) {
          p0 = getMinkInputPolygon(color1);
          if (color2 == 3 && !states_stack.back().brown_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          } else if (color2 == 4 && !states_stack.back().yellow_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          } else if (color2 == 5 && !states_stack.back().magenta_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          } else if (color2 == 6 && !states_stack.back().aqua_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
        } else if (color1 == 3 && !states_stack.back().brown_set().is_empty()) {
          p0 = getMinkInputPolygon(color1);
          if (color2 == 4 && !states_stack.back().yellow_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          } else if (color2 == 5 && !states_stack.back().magenta_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          } else if (color2 == 6 && !states_stack.back().aqua_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
        } else if (color1 == 4 && !states_stack.back().yellow_set().is_empty()) {
          p0 = getMinkInputPolygon(color1);
          if (color2 == 5 && !states_stack.back().magenta_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          } else if (color2 == 6 && !states_stack.back().aqua_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
        } else if (color1 == 5 && !states_stack.back().magenta_set().is_empty()) {
          p0 = getMinkInputPolygon(color1);
          if (color2 == 6 && !states_stack.back().aqua_set().is_empty()) {
            p1 = getMinkInputPolygon(color2);
          }
        }

        if (! m_disjoint) {
          Polygon_with_holes_2 mink_sum_res = CGAL::minkowski_sum_2(p0, p1);
          if (!mink_sum_res.is_unbounded()) {
            m_linear_input->m_is_mink = true;
            m_linear_input->get_Minkowski_result(mink_sum_res, p0);
            m_linear_input->m_hole = true;
            m_linear_input->get_Minkowski_holes(mink_sum_res, p0);
            states_stack.back().result_set().clear();
            states_stack.back().result_linear_sources().clear();
            m_linear_input->m_hole = false;
            m_linear_input->m_is_mink = false;
          } else ask_user_ok("Minkowski Sum Operation Error", "resultant polygon is unbounded\n");
          lDone = true;
          minkowski_sum_operated = true;
        }
        else {
          on_actionUndo_triggered();
          show_error(
                     "DISJOINT POLYGON SET ERROR\n\nCannot perform Minkowski Sum operation since Input Bucket contains more than disjoint polygons!!!");
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
      //zoomToFit();
      modelChanged();
    }
  }
}

//axis button clicked
void MainWindow::on_actionAxis_triggered() {
  if (! m_grid) {
    m_scene.addItem(xAxis);
    m_scene.addItem(yAxis);
    m_grid = true;
  }
  else {
    m_scene.removeItem(xAxis);
    m_scene.removeItem(yAxis);
    m_grid = false;
  }
  // Keep the button checked (highlighted) while the axes are shown.
  actionAxis->setChecked(m_grid);
  m_scene.update();
  modelChanged();
}

//pan button
void MainWindow::on_actionPAN_triggered() {
  if (! m_pan) {
    //get_new_state(7);
    remove_inputs();
    family_action(active_type())->setChecked(false);
    this->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    this->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_scene.update();
    m_pan = true;
    modelChanged();
  }
  else {
    family_action(active_type())->setChecked(true);
    m_pan = false;
    modelChanged();
  }
}

//add colors(represented by wheel)
void MainWindow::wheelEvent(QWheelEvent *event) {

  this->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
  static const double scaleFactor = 1.15;
  static double currentScale = 1.0;  // stores the current scale value.
  static const double scaleMin = .1; // defines the min scale limit.

  if (m_scene.width() >= 1500000 && event->angleDelta().y() > 0) {
    // currentScale = 0.0;
    this->graphicsView->setDragMode(QGraphicsView::NoDrag);
    this->graphicsView->setSceneRect(-1500000, -1000000, 3100000, 2000000);
    this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  }
  if (m_scene.width() <= 1500000) {
    if (event->angleDelta().y() > 0) {
      m_scene.setSceneRect(-m_scene.sceneRect().width() * scaleFactor / 2,
                           -m_scene.sceneRect().height() * scaleFactor / 2,
                           m_scene.sceneRect().width() * scaleFactor,
                           m_scene.sceneRect().height() * scaleFactor);
      this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
      this->graphicsView->scale(2.5, 2.5);
      // show_warning(to_string(m_scene.sceneRect().height())+" "+to_string(m_scene.sceneRect().width()));
    } else if (currentScale > scaleMin) {
      m_scene.setSceneRect(-m_scene.sceneRect().width() / (scaleFactor * 2),
                           -m_scene.sceneRect().height() / (scaleFactor * 2),
                           m_scene.sceneRect().width() / scaleFactor,
                           m_scene.sceneRect().height() / scaleFactor);
      this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
      this->graphicsView->scale(2.5, 2.5);
    }
  }
  modelChanged();
}

// The icons of the curve tool bar: a boundary of a segment and an arc, or a
// circle with its center and radius.
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

// The curve family being constructed: 1 linear, 2 circular, 3 Bezier,
// 4 polyline, 5 conic.
int MainWindow::active_type() const {
  if (m_conic_active) return 5;
  if (m_polyline_active) return 4;
  if (m_bezier_active) return 3;
  if (m_circular_active) return 2;
  return 1;
}

QAction* MainWindow::family_action(int aType) const {
  switch (aType) {
   case 2: return actionInsertCircular;
   case 3: return actionInsertBezier;
   case 4: return actionInsertPolyline;
   case 5: return actionInsertConic;
   default: return actionInsertLinear;
  }
}

// The input that constructs curves of the active family.
QObject* MainWindow::current_input() const {
  switch (active_type()) {
   case 2: return m_circular_input;
   case 3: return m_bezier_input;
   case 4: return m_polyline_input;
   case 5: return m_conic_input;
   default: return m_linear_input;
  }
}

// Stop constructing curves, and discard those under construction.
void MainWindow::remove_inputs() {
  for (QObject* input : {static_cast<QObject*>(m_linear_input),
                         static_cast<QObject*>(m_circular_input),
                         static_cast<QObject*>(m_bezier_input),
                         static_cast<QObject*>(m_polyline_input),
                         static_cast<QObject*>(m_conic_input)})
    m_scene.removeEventFilter(input);
  m_linear_input->Reset();
  m_circular_input->Reset();
  m_bezier_input->Reset();
  m_polyline_input->Reset();
  m_conic_input->Reset();
}

// Construct curves with the given input only.
void MainWindow::install_input(QObject* aInput) {
  for (QObject* input : {static_cast<QObject*>(m_linear_input),
                         static_cast<QObject*>(m_circular_input),
                         static_cast<QObject*>(m_bezier_input),
                         static_cast<QObject*>(m_polyline_input),
                         static_cast<QObject*>(m_conic_input)})
    m_scene.removeEventFilter(input);
  m_scene.installEventFilter(aInput);
}

void MainWindow::set_conic_pen(const QPen& aPen) { m_conic_input->set_pen(aPen); }

bool MainWindow::ensure_conic_mode() {
  if (! m_conic_active) {
    bool lProceed = true;
    for (auto& curve_set : states_stack.back().m_curve_sets)
      lProceed = lProceed && curve_set.is_empty();
    if (! lProceed)
      lProceed = ask_user_yesno("Conic mode switch",
                                "You are about to construct a conic polygon, but there are polygons of another type already loaded.\n" \
                                "Both types are not interoperable. In order to proceed, the polygons must be removed first.\n" \
                                "Once deleted you cannot undo the action.\n" \
                                "Yes to remove and proceed?\n");
    if (lProceed) {
      switch_sets_type(5);
      m_linear_input->Reset();
      m_circular_input->Reset();
      m_bezier_input->Reset();
      m_polyline_input->Reset();
      m_circular_active = false;
      m_bezier_active = false;
      m_polyline_active = false;
      m_conic_active = true;
    }
  }
  return m_conic_active;
}

void MainWindow::on_actionInsertConic_toggled(bool aChecked) {
  if (! aChecked) return;
  this->graphicsView->setDragMode(QGraphicsView::NoDrag);
  if (! ensure_conic_mode()) {
    actionInsertConic->setChecked(false);
    return;
  }
  if (!m_pan && !m_reset) {
    while (m_state_num > 1) on_actionUndo_triggered();
  }
  actionPAN->setChecked(false);
  m_pan = false;
  actionOpenLinear->setEnabled(false);
  actionOpenDXF->setEnabled(false);
  actionOpenBezier->setEnabled(false);
  actionOpenPolyline->setEnabled(false);
  actionInsertLinear->setChecked(false);
  actionInsertCircular->setChecked(false);
  actionInsertBezier->setChecked(false);
  actionInsertPolyline->setChecked(false);
  install_input(m_conic_input);
  on_actionDeleteResult();
  actionMinkowski_SumH->setEnabled(false);
  set_conic_pen(sPens[m_color_active]);
}

//zoom to fit button
void MainWindow::zoomToFit() {
    std::optional <QRectF> lTotalRect;
    for (auto si = states_stack.back().m_curve_sets.begin(); si != states_stack.back().m_curve_sets.end(); ++si) {
        if (!si->is_empty()) {
            QRectF lRect = si->bounding_rect();
            if (lTotalRect) lTotalRect = *lTotalRect | lRect;
            else lTotalRect = lRect;
        }
    }
    if (lTotalRect) {
        this->graphicsView->setSceneRect(*lTotalRect);
        this->graphicsView->fitInView(*lTotalRect, Qt::KeepAspectRatio);
    }
    modelChanged();
}

// Show a message with a "Don't show this message again" check box.
// aToggle is the Edit-menu action that controls whether the message is shown;
// checking the box unchecks it (and its handler updates the flag).
// This is a plain dialog rather than a QMessageBox, because QMessageBox
// arranges its buttons according to the platform: here OK is always on the
// left and Cancel (when offered) on the right, and the two look alike (neither
// is the default button).
// Return the button the user clicked (Escape and the close box count as
// Cancel when it is offered, and as OK otherwise).
QMessageBox::StandardButton
MainWindow::show_message(QMessageBox::Icon aIcon, const QString& aTitle,
                         const QString& aText, QAction* aToggle,
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
  // Make the check box visible on dark themes, like those of the Operations
  // panel.
  check_box->setStyle(&indicator_style);

  auto* ok_button = new QPushButton("OK");
  QPushButton* cancel_button = aButtons.testFlag(QMessageBox::Cancel) ?
    new QPushButton("Cancel") : nullptr;
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
    QObject::connect(cancel_button, &QPushButton::clicked,
                     &dialog, &QDialog::reject);

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

// Warning: the Draw/Insert bucket, where the result is added, is not empty.
// Uses the (blue) information icon, to set it apart from the (red) error.
// Return true if the user chooses to continue.
bool MainWindow::show_not_empty_warning() {
  return show_message(QMessageBox::Information, "Warning: Bucket Not Empty",
                      "The Draw/Insert bucket is not empty. "
                      "Add the result to it?", actionWarnNotEmpty,
                      QMessageBox::Ok | QMessageBox::Cancel) ==
    QMessageBox::Ok;
}

// Error: the Draw/Insert bucket is a non-empty operand of the operation.
void MainWindow::show_operand_error() {
  show_message(QMessageBox::Critical, "Error: Operation Aborted",
               "The result cannot be stored in a non-empty operand.",
               actionErrorOperand);
}

// Intersection and union take one or more operands, but at least one bucket
// must remain for the result. Called when a cell (aBox) of the operation
// column (aColumn) is toggled: checking the last unchecked cell (of the
// existing buckets) is undone (a no-op). The operation button (aOperation)
// is enabled only when at least one cell (of the existing buckets) is
// checked.
void MainWindow::
limit_to_all_but_one(const std::array<QAbstractButton*, 7>& aColumn,
                     QAbstractButton* aBox, bool aChecked,
                     QAbstractButton* aOperation) {
  if (aChecked && ! aBox->isHidden()) {
    bool all_checked =
      std::all_of(aColumn.begin(), aColumn.end(), [](QAbstractButton* box)
                  { return box->isHidden() || box->isChecked(); });
    if (all_checked) {
      aBox->setChecked(false);  // toggles again, and updates the operation
      return;
    }
  }
  bool any_checked =
    std::any_of(aColumn.begin(), aColumn.end(), [](QAbstractButton* box)
                { return ! box->isHidden() && box->isChecked(); });
  aOperation->setEnabled(any_checked);
}

// Restore the invariant above after a bucket is removed: if all (remaining)
// buckets are checked, uncheck the last one.
void MainWindow::
keep_one_unchecked(const std::array<QAbstractButton*, 7>& aColumn) {
  QAbstractButton* last = nullptr;
  for (auto* box : aColumn) {
    if (box->isHidden()) continue;
    if (! box->isChecked()) return;
    last = box;
  }
  if (last) last->setChecked(false);
}

// Difference, symmetric difference, and Minkowski sum take exactly two
// operands. Called when a cell (aBox) of the operation column (aColumn) is
// toggled: checking a third cell is undone (a no-op); unchecking is always
// allowed. The operation button (aOperation) is enabled only when exactly two
// cells are checked.
void MainWindow::
limit_to_two_operands(const std::array<QAbstractButton*, 7>& aColumn,
                      QAbstractButton* aBox, bool aChecked,
                      QAbstractButton* aOperation) {
  auto count = std::count_if(aColumn.begin(), aColumn.end(),
                             [](QAbstractButton* box)
                             { return box->isChecked(); });
  if (aChecked && (count > 2)) {
    aBox->setChecked(false);    // toggles again, and updates the operation
    return;
  }
  aOperation->setEnabled(count == 2);
}

// The buckets (indices) marked in the given operation column.
std::vector<std::size_t> MainWindow::
checked_buckets(const std::array<QAbstractButton*, 7>& aColumn) const {
  std::vector<std::size_t> buckets;
  for (std::size_t i = 0; i < aColumn.size(); ++i)
    if (! aColumn[i]->isHidden() && aColumn[i]->isChecked())
      buckets.push_back(i);
  return buckets;
}

// Uncheck all operation buttons but the given one.
void MainWindow::uncheck_operations_except(QAbstractButton* aOperation) {
  for (QAbstractButton* button :
         {static_cast<QAbstractButton*>(actionComplementH),
          static_cast<QAbstractButton*>(actionIntersectionH),
          static_cast<QAbstractButton*>(actionUnionH),
          static_cast<QAbstractButton*>(actionDifferenceH),
          static_cast<QAbstractButton*>(actionSymmetric_DifferenceH),
          static_cast<QAbstractButton*>(actionMinkowski_SumH),
          static_cast<QAbstractButton*>(actionCopyH),
          static_cast<QAbstractButton*>(actionMoveH)})
    if (button != aOperation) button->setChecked(false);
}

// Clear the scratch result set.
void MainWindow::clear_result() {
  auto& state = states_stack.back();
  state.result_set().clear();
  state.result_linear_sources().clear();
  state.result_circular_sources().clear();
  state.result_bezier_sources().clear();
  state.result_polyline_sources().clear();
}

// Apply a Boolean operation to the operand buckets, and add the result to the
// Draw/Insert bucket. The first two operands are combined with the bivariate
// operation (target = first op second), and the remaining ones are folded in
// one at a time. The complement is exact (unbounded). If the Draw/Insert
// bucket is empty and not an operand, the result is computed directly into
// it; otherwise into a temporary (result), which is then added to it. (The
// bivariate operations overwrite their target, and the target must not alias
// an operand.) The executed statements are logged, in Python (CGALPY) form.
void MainWindow::run_operation(Bucket_op aOp,
                               const std::vector<std::size_t>& aOperands,
                               QAbstractButton* aOperation) {
  if (aOperands.empty() ||
      ((aOp == Bucket_op::Complement) && (aOperands.size() != 1))) {
    aOperation->setChecked(false);
    return;
  }

  // The operation codes of the undo stack.
  std::size_t code = 0;
  QString name;
  switch (aOp) {
   case Bucket_op::Complement: code = 0; name = "complement"; break;
   case Bucket_op::Intersection: code = 1; name = "intersection"; break;
   case Bucket_op::Join: code = 2; name = "join"; break;
   case Bucket_op::Difference: code = 3; name = "difference"; break;
   case Bucket_op::Symmetric_difference:
    code = 4; name = "symmetric_difference"; break;
  }

  QCursor old = this->cursor();
  this->setCursor(Qt::WaitCursor);
  get_new_state(code);
  clear_result();
  auto& state = states_stack.back();
  auto bucket = [&state](std::size_t aIndex) -> Curve_set&
                { return state.set(aIndex); };
  Curve_set& draw_bucket = state.active_set(m_color_active);
  bool direct = draw_bucket.is_empty() &&
    (std::find(aOperands.begin(), aOperands.end(), m_color_active) ==
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
    auto apply2 = [&](Curve_set const& aFirst, Curve_set const& aSecond) {
      switch (aOp) {
       case Bucket_op::Intersection: result.intersect(aFirst, aSecond); break;
       case Bucket_op::Join: result.join(aFirst, aSecond); break;
       case Bucket_op::Difference: result.difference(aFirst, aSecond); break;
       case Bucket_op::Symmetric_difference:
        result.symmetric_difference(aFirst, aSecond); break;
       default: break;
      }
    };
    auto apply1 = [&](Curve_set const& aOther) {
      switch (aOp) {
       case Bucket_op::Intersection: result.intersect(aOther); break;
       case Bucket_op::Join: result.join(aOther); break;
       case Bucket_op::Difference: result.difference(aOther); break;
       case Bucket_op::Symmetric_difference:
        result.symmetric_difference(aOther); break;
       default: break;
      }
    };
    apply2(bucket(aOperands[0]), bucket(aOperands[1]));
    if (! direct) program << "result = Polygon_set_2()";
    program << QString("%1.%2(%3, %4)").arg(target, name,
                                             bucket_name(aOperands[0]),
                                             bucket_name(aOperands[1]));
    for (std::size_t i = 2; i < aOperands.size(); ++i) {
      apply1(bucket(aOperands[i]));
      program << QString("%1.%2(%3)").arg(target, name,
                                          bucket_name(aOperands[i]));
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
  modelChanged();
}

// Append statements to the program, and show them in the Log. Sticky
// statements (the creation and deletion of buckets, which are not undoable
// actions) are never dropped by undo.
void MainWindow::log_statements(const QStringList& aStatements, bool aSticky) {
  if (! m_logging) return;
  for (const auto& statement : aStatements) {
    m_program.push_back({aSticky ? 0 : states_stack.size(), statement});
    if (m_log) m_log->appendPlainText(statement);
  }
}

// Drop the statements of undone actions (those logged at a deeper undo stack
// than the current one), and refresh the Log.
void MainWindow::trim_program_to_states() {
  auto it = std::remove_if(m_program.begin(), m_program.end(),
                           [this](Program_line const& aLine)
                           { return aLine.m_state_count > states_stack.size(); });
  bool changed = (it != m_program.end());
  m_program.erase(it, m_program.end());
  if (! changed || ! m_log) return;
  QStringList lines;
  for (const auto& line : m_program) lines << line.m_text;
  m_log->setPlainText(lines.join("\n"));
}

// Whether the bucket with the given index currently exists (is shown).
bool MainWindow::bucket_exists(std::size_t aIndex) const {
  switch (aIndex) {
   case 0: case 1: case 2: return true;
   case 3: return m_visible_brown;
   case 4: return m_visible_yellow;
   case 5: return m_visible_magenta;
   case 6: return m_visible_aqua;
   default: return false;
  }
}

// The index of the bucket with the given name, or 7 if there is none.
std::size_t MainWindow::bucket_index(const QString& aName) const {
  for (std::size_t i = 0; i < 7; ++i)
    if (aName == bucket_name(i)) return i;
  return 7;
}

// The polygon set (an existing bucket or variable) with the given name, or
// null.
Curve_set* MainWindow::polygon_set(const QString& aName) {
  std::size_t i = bucket_index(aName);
  if (i < 7) return bucket_exists(i) ? &states_stack.back().set(i) : nullptr;
  auto it = m_variables.find(aName);
  return (it == m_variables.end()) ? nullptr : &it->second;
}

// The polygon of a Polygon_2([...]) expression. It must be simple and
// counterclockwise oriented (as the bindings require).
Linear_polygon_with_holes MainWindow::polygon(Py_expr const& aExpr) const {
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
    curves.emplace_back(Kernel::Point_2(p.first, p.second),
                        Kernel::Point_2(q.first, q.second));
  }
  return Linear_polygon_with_holes(Linear_polygon(curves.begin(),
                                                  curves.end()));
}

// Execute one console statement. Return the text to print (for queries), or
// an empty string. Throws a std::runtime_error with a message on an error.
// Statements that change a polygon set are undoable actions, and are logged.
QString MainWindow::execute(Py_statement const& aStatement) {
  using Kind = Py_statement::Kind;
  const QString& target = aStatement.m_target;
  std::size_t index = bucket_index(target);
  bool is_bucket = (index < 7);

  auto set_of = [this](Py_expr const& aExpr) -> Curve_set& {
    if (aExpr.m_kind != Py_expr::Kind::Name)
      throw std::runtime_error(("expected a polygon set name instead of " +
                                to_python(aExpr)).toStdString());
    Curve_set* set = polygon_set(aExpr.m_name);
    if (! set)
      throw std::runtime_error(("name '" + aExpr.m_name +
                                "' is not defined").toStdString());
    return *set;
  };
  auto log = [this, &aStatement]() {
    log_statements({to_python(aStatement)});
  };

  // del name
  if (aStatement.m_kind == Kind::Delete) {
    if (! is_bucket) {
      if (m_variables.erase(target) == 0)
        throw std::runtime_error(("name '" + target +
                                  "' is not defined").toStdString());
      log_statements({to_python(aStatement)}, true);
      return QString();
    }
    // Only the last added bucket can be removed.
    std::size_t last = 2;
    for (std::size_t i = 3; i < 7; ++i) if (bucket_exists(i)) last = i;
    if ((index < 3) || (index != last))
      throw std::runtime_error(index < 3 ?
        "the red, black, and blue buckets cannot be deleted" :
        ("only the last bucket (" + QString(bucket_name(last)) +
         ") can be deleted").toStdString());
    on_actionMinusColor_triggered();      // logs "del <name>"
    return QString();
  }

  // name = Polygon_set_2(...)
  if (aStatement.m_kind == Kind::Assign) {
    const Py_expr& value = aStatement.m_args[0];
    if (value.m_kind != Py_expr::Kind::Polygon_set)
      throw std::runtime_error("only polygon sets can be assigned, e.g., " +
        (target + " = Polygon_set_2(" +
         (value.m_kind == Py_expr::Kind::Name ? value.m_name : QString()) +
         ")").toStdString() + " (copies the set)");
    const Py_expr* arg = value.m_arg.get();
    if (is_bucket && ! bucket_exists(index)) {
      // Buckets are added in order.
      std::size_t next = 3;
      while ((next < 7) && bucket_exists(next)) ++next;
      if (index != next)
        throw std::runtime_error(("buckets are added in order; the next one "
                                  "is " + QString(bucket_name(next)))
                                 .toStdString());
      m_logging = false;
      on_actionAddColor_triggered();
      m_logging = true;
      if (! arg) {       // a new, empty bucket
        log_statements({to_python(aStatement)}, true);
        return QString();
      }
      log_statements({target + " = Polygon_set_2()"}, true);
    }
    get_new_state(13);
    Curve_set* set = polygon_set(target);
    if (! set) {
      set = &m_variables.emplace(target, Curve_set(1, QPen(), QBrush()))
        .first->second;
    }
    if (! arg) set->clear();
    else if (arg->m_kind == Py_expr::Kind::Polygon) {
      Linear_polygon_with_holes pgn = polygon(*arg);
      set->clear();
      set->linear().join(pgn);
    }
    else {
      Curve_set& other = set_of(*arg);
      if (&other != set) set->assign(other);
    }
    log();
    return QString();
  }

  // name.method(args)
  Curve_set* set = polygon_set(target);
  if (! set)
    throw std::runtime_error(("name '" + target + "' is not defined")
                             .toStdString());
  const QString& method = aStatement.m_method;
  const auto& args = aStatement.m_args;
  auto arity = [&](std::size_t aMin, std::size_t aMax) {
    if ((args.size() < aMin) || (args.size() > aMax))
      throw std::runtime_error((method + "() takes " +
        ((aMin == aMax) ? QString::number(aMin) :
         QString("%1 or %2").arg(aMin).arg(aMax)) + " argument(s)")
        .toStdString());
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

  if (method == "clear") {
    arity(0, 0);
    get_new_state(13);
    polygon_set(target)->clear();
    log();
    return QString();
  }
  if (method == "complement") {
    arity(0, 1);
    if (! args.empty()) {
      Curve_set& other = set_of(args[0]);
      if (&other == set)
        throw std::runtime_error("use complement() to complement in place");
    }
    get_new_state(13);
    set = polygon_set(target);
    if (args.empty()) set->complement();
    else set->complement(set_of(args[0]));
    log();
    return QString();
  }

  enum class Op { Join, Intersection, Difference, Symmetric_difference };
  Op op;
  if (method == "join") op = Op::Join;
  else if (method == "intersection") op = Op::Intersection;
  else if (method == "difference") op = Op::Difference;
  else if (method == "symmetric_difference") op = Op::Symmetric_difference;
  else
    throw std::runtime_error(("unknown method '" + method + "'").toStdString());
  arity(1, 2);

  if (args.size() == 2) {
    // The bivariate form overwrites the target, which must not be an operand.
    if ((&set_of(args[0]) == set) || (&set_of(args[1]) == set))
      throw std::runtime_error(("the target of " + method + "(a, b) must "
                                "differ from a and b").toStdString());
    get_new_state(13);
    set = polygon_set(target);
    Curve_set& a = set_of(args[0]);
    Curve_set& b = set_of(args[1]);
    switch (op) {
     case Op::Join: set->join(a, b); break;
     case Op::Intersection: set->intersect(a, b); break;
     case Op::Difference: set->difference(a, b); break;
     case Op::Symmetric_difference: set->symmetric_difference(a, b); break;
    }
    log();
    return QString();
  }

  // One argument: a polygon set or a polygon.
  if (args[0].m_kind == Py_expr::Kind::Polygon) {
    Linear_polygon_with_holes pgn = polygon(args[0]);
    get_new_state(13);
    auto& lset = polygon_set(target)->linear();
    switch (op) {
     case Op::Join: lset.join(pgn); break;
     case Op::Intersection: lset.intersection(pgn); break;
     case Op::Difference: lset.difference(pgn); break;
     case Op::Symmetric_difference: lset.symmetric_difference(pgn); break;
    }
    log();
    return QString();
  }
  set_of(args[0]);      // check the name
  get_new_state(13);
  set = polygon_set(target);
  Curve_set& other = set_of(args[0]);
  switch (op) {
   case Op::Join: set->join(other); break;
   case Op::Intersection: set->intersect(other); break;
   case Op::Difference: set->difference(other); break;
   case Op::Symmetric_difference: set->symmetric_difference(other); break;
  }
  log();
  return QString();
}

// The console input line: Up and Down browse the history, and Ctrl+<key>
// combinations go to the input line rather than to the operation shortcuts
// (Ctrl+C is complement, Ctrl+D difference, Ctrl+U union, ...), except Ctrl+Q.
bool MainWindow::eventFilter(QObject* aObject, QEvent* aEvent) {
  if ((aObject == graphicsView->viewport()) &&
      (aEvent->type() == QEvent::Paint))
    update_unbounded_rect();
  if ((aObject == m_console_input) && m_console_input) {
    if (aEvent->type() == QEvent::ShortcutOverride) {
      auto* ke = static_cast<QKeyEvent*>(aEvent);
      if (ke->modifiers().testFlag(Qt::ControlModifier) &&
          (ke->key() != Qt::Key_Q)) {
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
          m_console_input->setText((pos < n) ? m_console_history[pos] :
                                   QString());
        }
        return true;
      }
    }
  }
  return CGAL::Qt::DemosMainWindow::eventFilter(aObject, aEvent);
}

// Set the area that unbounded sets (e.g., complements) are drawn over to the
// visible area of the view, enlarged a bit so that the edges of the fill
// never show.
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

// Print to the console output.
void MainWindow::console_print(const QString& aText) {
  if (m_console_output) m_console_output->appendPlainText(aText);
}

// Parse and execute a console line.
void MainWindow::run_console_line(const QString& aLine) {
  console_print(">>> " + aLine);
  if (! aLine.trimmed().isEmpty()) {
    m_console_history << aLine;
  }
  m_console_history_pos = m_console_history.size();
  if (active_type() != 1) {
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
  modelChanged();
}

// Save the program as a Python script for the CGALPY bindings.
void MainWindow::save_program() {
  QString file_name =
    QFileDialog::getSaveFileName(this, "Save Program", "bso_program.py",
                                 "Python files (*.py)");
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

// Whether the Draw/Insert bucket is marked in the given operation column.
bool MainWindow::
draw_row_checked(const std::array<QAbstractButton*, 7>& aColumn) const {
  return (m_color_active < aColumn.size()) &&
    aColumn[m_color_active]->isChecked();
}

// Every operation stores its result in the Draw/Insert bucket. If that bucket
// is not empty:
// - If it is one of the operands, abort the operation and show an error
//   (unless the user turned the error off).
// - Otherwise, warn (unless the user turned the warning off), and abort the
//   operation if the user cancels.
// When aborting, uncheck the operation button. Return true if aborted.
bool MainWindow::abort_for_draw_bucket(QAbstractButton* aOperation,
                                       bool aIsOperand) {
  if (states_stack.back().active_set(m_color_active).is_empty()) return false;
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

//resize button
void MainWindow::resizeEvent(QResizeEvent *e) {
  this->graphicsView->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
  this->graphicsView->scale(2.5, 2.5);
  modelChanged();
}

//all the information in the help/about section
void MainWindow::on_aboutCGAL_triggered() {
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
  about_CGAL_txt.replace("<!--CGAL_VERSION-->",
                         QString(" (version %1)")
                         .arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 550px; min-height: 250px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel *mb_label = mb.findChild<QLabel *>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() |
                                      ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  } else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void MainWindow::on_aboutMinkop_triggered() {
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
  about_CGAL_txt.replace("<!--CGAL_VERSION-->",
                         QString(" (version %1)")
                         .arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel *mb_label = mb.findChild<QLabel *>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() |
                                      ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  }
  else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void MainWindow::on_aboutBolop_triggered() {

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
  about_CGAL_txt.replace("<!--CGAL_VERSION-->",
                         QString(" (version %1)")
                         .arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel *mb_label = mb.findChild<QLabel *>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() |
                                      ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  } else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

void MainWindow::on_aboutDemo_triggered() {
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
  about_CGAL_txt.replace("<!--CGAL_VERSION-->",
                         QString(" (version %1)")
                         .arg(cgal_version));
#endif
  QMessageBox mb;
  mb.setStyleSheet("QLabel{min-width: 700px; min-height: 300px;}");
  mb.setWindowTitle(title);
  mb.setText(about_CGAL_txt);
  mb.addButton(QMessageBox::Ok);

  QLabel *mb_label = mb.findChild<QLabel *>("qt_msgbox_label");
  if (mb_label) {
    mb_label->setTextInteractionFlags(mb_label->textInteractionFlags() |
                                      ::Qt::LinksAccessibleByMouse |
                                      ::Qt::LinksAccessibleByKeyboard);
  } else {
    std::cerr << "Cannot find child \"qt_msgbox_label\" in QMessageBox\n"
              << "  with Qt version " << QT_VERSION_STR << "!\n";
  }
  mb.exec();
}

//exception handling while switching modes
void MainWindow::exception_handler() {
  try {
    if (ensure_linear_mode()) {
      m_linear_input->Reset();
      m_linear_input->mState = m_linear_input->Start;
    } else if (ensure_circular_mode()) {
      m_circular_input->Reset();
      m_circular_input->mState = m_circular_input->Start;
    } else if (ensure_bezier_mode()) {
      m_bezier_input->Reset();
      m_bezier_input->mState = m_bezier_input->Start;
    } else if (ensure_polyline_mode()) {
      m_polyline_input->Reset();
      m_polyline_input->mState = m_polyline_input->Start;
    }
  }
  catch (const std::exception &e) {
    //todo::add catch statement
  }
}

//process all the inputs
void MainWindow::processInput(CGAL::Object o) {
  std::pair <Bezier_polygon, Bezier_boundary_source> lBI;
  Linear_polygon lLI;
  Circular_polygon lCI;
  Polyline_polygon lPI;
  Conic_polygon lConicPI;
  //cout<<"in process input"<<endl;
  try {
    if (CGAL::assign(lConicPI, o)) {
      if (ensure_conic_mode()) {
        if (lConicPI.orientation() == CGAL::CLOCKWISE)
          lConicPI.reverse_orientation();
        get_new_state(11);
        states_stack.back().active_set(m_color_active).conic().
          join(Conic_polygon_with_holes(lConicPI));
      }
    }
    else if (CGAL::assign(lBI, o)) {
      if (ensure_bezier_mode()) {
        CGAL::Orientation o = lBI.first.orientation();
        if (o == CGAL::CLOCKWISE)
          lBI.first.reverse_orientation();

        if (!m_bezier_input->isboundingRect()) {
          get_new_state(11);
          states_stack.back().active_set(m_color_active).bezier().join(Bezier_polygon_with_holes(lBI.first));
          Bezier_region_source br;
          br.push_back(lBI.second);
          states_stack.back().active_bezier_sources(m_color_active).push_back(br);
        } else {
          states_stack.back().result_set().bezier().join(Bezier_polygon_with_holes(lBI.first));
          Bezier_region_source br;
          br.push_back(lBI.second);
          states_stack.back().result_bezier_sources().push_back(br);
        }
      }
    } else if (CGAL::assign(lLI, o)) {
      if (ensure_linear_mode()) {
        CGAL::Orientation orient = lLI.orientation();
        if (orient == CGAL::CLOCKWISE) {
          lLI.reverse_orientation();
        }
        Linear_polygon_with_holes lCPWH(lLI);

        if (!m_linear_input->isboundingRect() && !m_linear_input->ishole()) {
          if (!m_linear_input->is_mink()) {
            get_new_state(11);
          }
          states_stack.back().active_set(m_color_active).linear().join(lCPWH);
          states_stack.back().active_linear_sources(m_color_active).push_back(lCPWH);
          if (! m_linear_input->is_mink())
            log_statements({QString("%1.join(%2)")
                            .arg(bucket_name(m_color_active), to_python(lLI))});
        } else {
          states_stack.back().result_set().linear().join(lCPWH);
          states_stack.back().result_linear_sources().push_back(lCPWH);
          if (m_linear_input->ishole()) {
            log_statements({QString("%1.difference(%2)")
                            .arg(bucket_name(m_color_active), to_python(lLI))});
            states_stack.back().active_set(m_color_active).difference(states_stack.back().result_set());
            states_stack.back().result_set().clear();
            states_stack.back().result_linear_sources().clear();
          }
        }
      }
    } else if (CGAL::assign(lCI, o)) {
      if (ensure_circular_mode()) {
        CGAL::Orientation o = lCI.orientation();
        if (o == CGAL::CLOCKWISE)
          lCI.reverse_orientation();

        Circular_polygon_with_holes lCPWH(lCI);

        if (!m_circular_input->isboundingRect()) {
          get_new_state(11);
          states_stack.back().active_set(m_color_active).circular().join(lCPWH);
          states_stack.back().active_circular_sources(m_color_active).push_back(lCPWH);
        } else {
          states_stack.back().result_set().circular().join(lCPWH);
          states_stack.back().result_circular_sources().push_back(lCPWH);
        }
      }
    } else if (CGAL::assign(lPI, o)) {
      if (ensure_polyline_mode()) {
        CGAL::Orientation o = lPI.orientation();
        if (o == CGAL::CLOCKWISE)
          lPI.reverse_orientation();

        Polyline_polygon_with_holes lPPWH(lPI);

        if (!m_polyline_input->isboundingRect()) {
          get_new_state(11);
          states_stack.back().active_set(m_color_active).polyline().join(lPPWH);
          states_stack.back().active_polyline_sources(m_color_active).push_back(lPPWH);
        } else {
          states_stack.back().result_set().polyline().join(lPPWH);
          states_stack.back().result_polyline_sources().push_back(lPPWH);
        }
      }
    }
  }
  catch (const std::exception &e) {
    on_actionUndo_triggered();
  }
  modelChanged();
}

#include "Boolean_set_operations_2.moc"
#include <CGAL/Qt/resources.h>

//the main function
int main(int argc, char *argv[]) {
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
  catch (const std::exception &e) {
    std::string s = e.what();
    show_error("Exception thrown during run of the program:\n" + s);
  }
}
