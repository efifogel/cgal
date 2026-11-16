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

#ifndef CGAL_QT6_GRAPHICS_VIEW_CONIC_INPUT_H
#define CGAL_QT6_GRAPHICS_VIEW_CONIC_INPUT_H

#include <cmath>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <QFormLayout>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainterPath>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>

#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Arr_geometry_traits/Conic_intersections_2.h>

#include "Typedefs.h"
#include "Qt6/Conic_curves.h"
#include "Error_handling.h"
#include "Boundary_validity.h"
#include "Polycurve_boundary.h"

namespace CGAL {
namespace Qt {

/*! Constructs polygons bounded by conic arcs. A boundary is a cyclic sequence of conics; every vertex is an
 * intersection point of two consecutive conics, and every conic contributes the arc between its two vertices. Full
 * circles and full ellipses are constructed directly.
 */
class Graphics_view_conic_input : public GraphicsViewInput {
public:
  using Rational = Conic_rational;
  using Integer = Conic_integer;
  using Algebraic = Conic_algebraic;
  using Kind = Conic_shape::Kind;

  // The tools. The first five add a conic to the boundary under construction; the last two construct a full curve.
  enum class Mode { MO_LINE, MO_CIRCLE, MO_ELLIPSE, MO_PARABOLA, MO_HYPERBOLA, FULL_CIRCLE, FULL_ELLIPSE };

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_conic_input(QObject* a_parent, QGraphicsScene* a_scene) :
    GraphicsViewInput(a_parent),
    m_scene(a_scene),
    m_curves_item(new QGraphicsPathItem),
    m_arcs_item(new QGraphicsPathItem),
    m_preview_item(new QGraphicsPathItem),
    m_markers_item(new QGraphicsPathItem),
    m_hollow_markers_item(new QGraphicsPathItem) {
    for (auto* item : items()) {
      item->setZValue(10);
      m_scene->addItem(item);
    }
    set_pen(QPen(QColor(255, 0, 0), 0));
  }

  /*! sets the pen of the curves under construction.
   */
  void set_pen(const QPen& a_pen) {
    m_pen = a_pen;
    m_pen.setCosmetic(true);
    update_preview();
  }

  /*! sets whether the vertices are marked as those of a boundary of polycurves (see update_preview()).
   */
  void set_marked(bool a_marked) {
    m_marked = a_marked;
    update_preview();
  }

  /*! sets the tool, and discards the clicks of the previous one.
   */
  void set_mode(Mode a_mode) {
    m_mode = a_mode;
    m_clicks.clear();
    update_preview();
  }

  /*! discards everything under construction.
   */
  void reset() {
    m_conics.clear();
    m_valid_arcs.clear();
    m_arc_pieces.clear();
    m_vertices.clear();
    m_ccw.clear();
    m_clicks.clear();
    m_candidates.clear();
    m_arc_queue.clear();
    m_closing = false;
    m_state = State::ST_IDLE;
    update_preview();
  }

  /*! handles the mouse and key events of the scene.
   * \return true if the event is handled.
   */
  bool eventFilter(QObject* a_object, QEvent* a_event) {
    switch (a_event->type()) {
     case QEvent::GraphicsSceneMousePress: return mouse_press(static_cast<QGraphicsSceneMouseEvent*>(a_event));
     case QEvent::GraphicsSceneMouseRelease: return mouse_release(static_cast<QGraphicsSceneMouseEvent*>(a_event));
     case QEvent::GraphicsSceneMouseMove:
      m_cursor = static_cast<QGraphicsSceneMouseEvent*>(a_event)->scenePos();
      update_preview();
      return false;
     case QEvent::KeyPress: return key_press(static_cast<QKeyEvent*>(a_event));
     default: break;
    }
    return QObject::eventFilter(a_object, a_event);
  }

  /*! asks for the coefficients of a conic, and adds it to the boundary or constructs the full curve.
   */
  void enter_coefficients() {
    QWidget input;
    auto* layout = new QFormLayout(&input);
    layout->setContentsMargins(0, 0, 0, 0);
    const char* names[6] = {"r", "s", "t", "u", "v", "w"};
    QLineEdit* edits[6];
    for (int i = 0; i < 6; ++i) {
      edits[i] = new QLineEdit(m_last_coefficients[i]);
      edits[i]->setPlaceholderText("e.g., 3, -1.25, or 2/3");
      layout->addRow(names[i], edits[i]);
    }
    auto* kind_label = new QLabel;
    layout->addRow("Curve:", kind_label);
    auto* add_button = new QRadioButton("Add to the boundary");
    auto* full_button = new QRadioButton("Full curve (an ellipse)");
    add_button->setChecked(true);
    layout->addRow(add_button);
    layout->addRow(full_button);

    Rational c[6];
    QPushButton* ok_button = nullptr;
    auto check = [&]() {
      bool ok = true;
      for (int i = 0; i < 6; ++i) ok = parse_rational(edits[i]->text(), c[i]) && ok;
      std::string error;
      std::optional<Kind> kind;
      if (ok) kind = classify(c, error);
      kind_label->setText(! ok ? QString("Invalid number") : kind ? kind_name(*kind) : QString(error.c_str()));
      bool is_ellipse = kind && (*kind == Conic_shape::Kind::KI_ELLIPSE);
      full_button->setEnabled(is_ellipse);
      if (! is_ellipse && full_button->isChecked()) add_button->setChecked(true);
      if (ok_button) ok_button->setEnabled(kind.has_value());
    };
    for (auto* edit : edits) QObject::connect(edit, &QLineEdit::textChanged, &input, check);
    check();
    auto setup = [&](QPushButton* a_ok) {
      ok_button = a_ok;
      check();
    };
    if (! ask_input("Conic Coefficients",
                    "The coefficients (integers, decimals, or fractions) of the conic\n"
                    "r x\u00b2 + s y\u00b2 + t xy + u x + v y + w = 0:", input, setup))
      return;
    for (int i = 0; i < 6; ++i) m_last_coefficients[i] = edits[i]->text();
    if (full_button->isChecked()) generate_full(c);
    else add_conic(c);
  }

private:
  enum class State { ST_IDLE, PICK_VERTEX, PICK_CLOSING_VERTEX, PICK_ARC };

  /*! A conic: its exact coefficients and its double-precision shape.
   */
  struct Conic {
    Rational m_c[6];
    Conic_shape m_shape;
  };

  /*! obtains the graphics items.
   */
  std::vector<QGraphicsPathItem*> items() const
  { return {m_curves_item, m_arcs_item, m_preview_item, m_markers_item, m_hollow_markers_item}; }

  /*! converts a coordinate (0 for x, 1 for y) of a scene point to a rational number exactly.
   */
  static Rational to_rational(QPointF a_p, int a_i) { return Rational(a_i == 0 ? a_p.x() : a_p.y()); }

  /*! parses an integer, a decimal number, or a fraction exactly.
   * \return false if the text is not a number.
   */
  static bool parse_rational(const QString& a_text, Rational& r_q) {
    static const QRegularExpression re("^\\s*([+-]?)(\\d*)(?:\\.(\\d*))?(?:/(\\d+))?\\s*$");
    auto m = re.match(a_text);
    if (! m.hasMatch()) return false;
    QString whole = m.captured(2), frac = m.captured(3), den = m.captured(4);
    if (whole.isEmpty() && frac.isEmpty()) return false;
    if (! den.isEmpty() && ! frac.isEmpty()) return false;
    Integer num((whole + frac).isEmpty() ? std::string("0") : (whole + frac).toStdString());
    Integer d = 1;
    for (int i = 0; i < frac.size(); ++i) d *= 10;
    if (! den.isEmpty()) d = Integer(den.toStdString());
    if (d == 0) return false;
    r_q = Rational(num, d);
    if (m.captured(1) == "-") r_q = -r_q;
    return true;
  }

  /*! obtains the name of a kind of conics.
   */
  static QString kind_name(Kind a_kind) {
    switch (a_kind) {
     case Conic_shape::Kind::KI_LINE: return "Line";
     case Conic_shape::Kind::KI_ELLIPSE: return "Ellipse";
     case Conic_shape::Kind::KI_HYPERBOLA: return "Hyperbola";
     case Conic_shape::Kind::KI_PARABOLA: return "Parabola";
    }
    return QString();
  }

  /*! obtains the kind of a non-degenerate conic.
   * \return the kind, or nullopt (and an error message) if the conic is degenerate.
   */
  static std::optional<Kind> classify(const Rational* a_c, std::string& r_error) {
    const Rational& r = a_c[0], &s = a_c[1], &t = a_c[2], &u = a_c[3], &v = a_c[4], &w = a_c[5];
    if (CGAL::is_zero(r) && CGAL::is_zero(s) && CGAL::is_zero(t)) {
      if (CGAL::is_zero(u) && CGAL::is_zero(v)) {
        r_error = "Not a curve.";
        return std::nullopt;
      }
      return Conic_shape::Kind::KI_LINE;
    }
    // The determinant of [[r, t/2, u/2], [t/2, s, v/2], [u/2, v/2, w]].
    Rational det = r*(s*w - v*v/4) - t/2*(t*w/2 - u*v/4) + u/2*(t*v/4 - s*u/2);
    if (CGAL::is_zero(det)) {
      r_error = "Degenerate (a pair of lines or a point).";
      return std::nullopt;
    }
    CGAL::Sign disc = CGAL::sign(4*r*s - t*t);
    if (disc == CGAL::ZERO) return Conic_shape::Kind::KI_PARABOLA;
    if (disc == CGAL::NEGATIVE) return Conic_shape::Kind::KI_HYPERBOLA;
    if (CGAL::sign((r + s) * det) != CGAL::NEGATIVE) {
      r_error = "An ellipse with no real points.";
      return std::nullopt;
    }
    return Conic_shape::Kind::KI_ELLIPSE;
  }

  /*! constructs a conic from its coefficients.
   * \return the conic, or nullopt (and an error message) if the conic is degenerate.
   */
  static std::optional<Conic> make_conic(const Rational* a_c, std::string& r_error) {
    auto kind = classify(a_c, r_error);
    if (! kind) return std::nullopt;
    Conic conic;
    double d[6];
    for (int i = 0; i < 6; ++i) {
      conic.m_c[i] = a_c[i];
      d[i] = CGAL::to_double(a_c[i]);
    }
    conic.m_shape = Conic_shape(*kind, d);
    return conic;
  }

  /*! obtains the coefficients of the circle with a given center through a given point.
   */
  static void circle(QPointF a_center, QPointF a_p, Rational* r_c) {
    Rational cx = to_rational(a_center, 0), cy = to_rational(a_center, 1);
    Rational dx = to_rational(a_p, 0) - cx, dy = to_rational(a_p, 1) - cy;
    r_c[0] = 1; r_c[1] = 1; r_c[2] = 0;
    r_c[3] = -2*cx; r_c[4] = -2*cy;
    r_c[5] = cx*cx + cy*cy - (dx*dx + dy*dy);
  }

  /*! obtains the coefficients of the ellipse with a given center, one of its axes ending at a_a, and the other
   * semi-axis the distance of a_b from that axis: q*xi^2 + |d|^4*eta^2 = |d|^4*q, where xi and eta are the coordinates
   * along d = a_a - a_center and its normal, and q = (normal . (a_b - a_center))^2.
   * \return false if there is no such ellipse.
   */
  static bool ellipse(QPointF a_center, QPointF a_a, QPointF a_b, Rational* r_c) {
    Rational cx = to_rational(a_center, 0), cy = to_rational(a_center, 1);
    Rational dx = to_rational(a_a, 0) - cx, dy = to_rational(a_a, 1) - cy;
    Rational bx = to_rational(a_b, 0) - cx, by = to_rational(a_b, 1) - cy;
    Rational d2 = dx*dx + dy*dy, e = -dy*bx + dx*by, q = e*e;
    if (CGAL::is_zero(d2) || CGAL::is_zero(q)) return false;
    Rational alpha = q, beta = d2*d2;
    // xi = a1 x + b1 y + c1, eta = a2 x + b2 y + c2.
    Rational a1 = dx, b1 = dy, c1 = -(dx*cx + dy*cy);
    Rational a2 = -dy, b2 = dx, c2 = dy*cx - dx*cy;
    r_c[0] = alpha*a1*a1 + beta*a2*a2;
    r_c[1] = alpha*b1*b1 + beta*b2*b2;
    r_c[2] = 2*(alpha*a1*b1 + beta*a2*b2);
    r_c[3] = 2*(alpha*a1*c1 + beta*a2*c2);
    r_c[4] = 2*(alpha*b1*c1 + beta*b2*c2);
    r_c[5] = alpha*c1*c1 + beta*c2*c2 - alpha*beta;
    return true;
  }

  /*! obtains the coefficients of the parabola with a given vertex, its axis through a_a, and through a_p:
   * xi_p*eta^2 - eta_p^2*xi = 0, where xi and eta are the coordinates along d = a_a - a_vertex and its normal (both
   * scaled by |d|), and (xi_p, eta_p) are those of a_p.
   * \return false if there is no such parabola (a_p is on the axis or on the tangent at the vertex).
   */
  static bool parabola(QPointF a_vertex, QPointF a_a, QPointF a_p, Rational* r_c) {
    Rational vx = to_rational(a_vertex, 0), vy = to_rational(a_vertex, 1);
    Rational dx = to_rational(a_a, 0) - vx, dy = to_rational(a_a, 1) - vy;
    Rational px = to_rational(a_p, 0) - vx, py = to_rational(a_p, 1) - vy;
    Rational xi_p = dx*px + dy*py, eta_p = -dy*px + dx*py;
    if (CGAL::is_zero(xi_p) || CGAL::is_zero(eta_p)) return false;
    Rational alpha = xi_p, beta = eta_p*eta_p;
    // xi = a1 x + b1 y + c1, eta = a2 x + b2 y + c2.
    Rational a1 = dx, b1 = dy, c1 = -(dx*vx + dy*vy);
    Rational a2 = -dy, b2 = dx, c2 = dy*vx - dx*vy;
    r_c[0] = alpha*a2*a2;
    r_c[1] = alpha*b2*b2;
    r_c[2] = 2*alpha*a2*b2;
    r_c[3] = 2*alpha*a2*c2 - beta*a1;
    r_c[4] = 2*alpha*b2*c2 - beta*b1;
    r_c[5] = alpha*c2*c2 - beta*c1;
    return true;
  }

  /*! obtains the coefficients of the hyperbola with a given center, a vertex at a_a, and an asymptote through a_p:
   * eta_p^2*xi^2 - xi_p^2*eta^2 = |d|^4*eta_p^2, where xi and eta are the coordinates along d = a_a - a_center and its
   * normal (both scaled by |d|), and (xi_p, eta_p) are those of a_p.
   * \return false if there is no such hyperbola (a_p is on the axis or on its normal through the center).
   */
  static bool hyperbola(QPointF a_center, QPointF a_a, QPointF a_p, Rational* r_c) {
    Rational cx = to_rational(a_center, 0), cy = to_rational(a_center, 1);
    Rational dx = to_rational(a_a, 0) - cx, dy = to_rational(a_a, 1) - cy;
    Rational px = to_rational(a_p, 0) - cx, py = to_rational(a_p, 1) - cy;
    Rational d2 = dx*dx + dy*dy, xi_p = dx*px + dy*py, eta_p = -dy*px + dx*py;
    if (CGAL::is_zero(d2) || CGAL::is_zero(xi_p) || CGAL::is_zero(eta_p)) return false;
    Rational alpha = eta_p*eta_p, beta = xi_p*xi_p, gamma = d2*d2*alpha;
    // xi = a1 x + b1 y + c1, eta = a2 x + b2 y + c2.
    Rational a1 = dx, b1 = dy, c1 = -(dx*cx + dy*cy);
    Rational a2 = -dy, b2 = dx, c2 = dy*cx - dx*cy;
    r_c[0] = alpha*a1*a1 - beta*a2*a2;
    r_c[1] = alpha*b1*b1 - beta*b2*b2;
    r_c[2] = 2*(alpha*a1*b1 - beta*a2*b2);
    r_c[3] = 2*(alpha*a1*c1 - beta*a2*c2);
    r_c[4] = 2*(alpha*b1*c1 - beta*b2*c2);
    r_c[5] = alpha*c1*c1 - beta*c2*c2 - gamma;
    return true;
  }

  /*! obtains the coefficients of the conic of a tool defined by three points (an ellipse, a parabola, or a
   * hyperbola).
   * \return false if there is no such conic.
   */
  bool three_point_conic(QPointF a_p0, QPointF a_p1, QPointF a_p2, Rational* r_c) const {
    switch (m_mode) {
     case Mode::MO_PARABOLA: return parabola(a_p0, a_p1, a_p2, r_c);
     case Mode::MO_HYPERBOLA: return hyperbola(a_p0, a_p1, a_p2, r_c);
     default: return ellipse(a_p0, a_p1, a_p2, r_c);
    }
  }

  /*! determines whether the tool is defined by three points.
   */
  bool is_three_point_mode() const {
    return (m_mode == Mode::MO_ELLIPSE) || (m_mode == Mode::FULL_ELLIPSE) || (m_mode == Mode::MO_PARABOLA) ||
      (m_mode == Mode::MO_HYPERBOLA);
  }

  /*! obtains the coefficients of the line through two points.
   * \return false if the points coincide.
   */
  static bool line(QPointF a_p, QPointF a_q, Rational* r_c) {
    Rational px = to_rational(a_p, 0), py = to_rational(a_p, 1);
    Rational qx = to_rational(a_q, 0), qy = to_rational(a_q, 1);
    if ((px == qx) && (py == qy)) return false;
    r_c[0] = 0; r_c[1] = 0; r_c[2] = 0;
    r_c[3] = qy - py; r_c[4] = px - qx; r_c[5] = qx*py - px*qy;
    return true;
  }

  /*! determines whether two conics are the same, that is, whether their coefficients are proportional.
   */
  static bool same_conic(const Conic& a_c1, const Conic& a_c2) {
    int k = 0;
    while (CGAL::is_zero(a_c1.m_c[k])) ++k;
    if (CGAL::is_zero(a_c2.m_c[k])) return false;
    Rational f = a_c2.m_c[k] / a_c1.m_c[k];
    for (int i = 0; i < 6; ++i) {
      if (a_c2.m_c[i] != f * a_c1.m_c[i]) return false;
    }
    return true;
  }

  /*! determines whether two points are equal.
   */
  static bool equal(const Conic_point& a_p, const Conic_point& a_q)
  { return Conic_alg_kernel().compare_xy_2_object()(a_p, a_q) == CGAL::EQUAL; }

  /*! converts a point to a scene point.
   */
  static QPointF to_qt(const Conic_point& a_p) { return QPointF(CGAL::to_double(a_p.x()), CGAL::to_double(a_p.y())); }

  /*! obtains the (exact) intersection points of two conics.
   */
  static std::vector<Conic_point> intersections(const Conic& a_c1, const Conic& a_c2) {
    Conic_nt_traits nt;
    Integer a[6], b[6];
    nt.convert_coefficients(a_c1.m_c, a_c1.m_c + 6, a);
    nt.convert_coefficients(a_c2.m_c, a_c2.m_c + 6, b);
    auto degree = [](const Integer* c) {
      return (CGAL::is_zero(c[0]) && CGAL::is_zero(c[1]) && CGAL::is_zero(c[2])) ? 1 : 2;
    };
    int deg_a = degree(a), deg_b = degree(b);
    Algebraic xs[4], ys[4];
    int n_xs =
      compute_resultant_roots(nt, a[0], a[1], a[2], a[3], a[4], a[5], deg_a, b[0], b[1], b[2], b[3], b[4], b[5], deg_b,
                              xs);
    int n_ys =
      compute_resultant_roots(nt, a[1], a[0], a[2], a[4], a[3], a[5], deg_a, b[1], b[0], b[2], b[4], b[3], b[5], deg_b,
                              ys);
    auto on = [&nt](const Integer* c, const Algebraic& x, const Algebraic& y) {
      return CGAL::is_zero(nt.convert(c[0])*x*x + nt.convert(c[1])*y*y + nt.convert(c[2])*x*y + nt.convert(c[3])*x +
                           nt.convert(c[4])*y + nt.convert(c[5]));
    };
    std::vector<Conic_point> res;
    for (int i = 0; i < n_xs; ++i) {
      for (int j = 0; j < n_ys; ++j) {
        if (on(a, xs[i], ys[j]) && on(b, xs[i], ys[j])) res.push_back(Conic_point(xs[i], ys[j]));
      }
    }
    return res;
  }

  /*! shows a warning.
   */
  void message(const QString& a_text) { show_warning("Conic Curves", a_text); }

  /*! emits the polygon bounded by the full ellipse of the given coefficients.
   */
  void generate_full(const Rational* a_c) {
    std::string error;
    auto conic = make_conic(a_c, error);
    if (! conic || (conic->m_shape.kind() != Conic_shape::Kind::KI_ELLIPSE)) {
      message(conic ? QString("A full curve must be an ellipse.") : QString(error.c_str()));
      return;
    }
    Conic_traits traits;
    Conic_curve cv = traits.construct_curve_2_object()(a_c[0], a_c[1], a_c[2], a_c[3], a_c[4], a_c[5]);
    std::vector<Conic_X_monotone_curve> xcvs;
    if (! append_x_monotone(traits, cv, xcvs)) return;
    emit(generate(CGAL::make_object(Conic_polygon(xcvs.begin(), xcvs.end()))));
  }

  /*! subdivides a conic arc into x-monotone arcs, and appends them to a given vector.
   * \return false if the arc is invalid.
   */
  bool append_x_monotone(const Conic_traits& a_traits, const Conic_curve& a_cv,
                         std::vector<Conic_X_monotone_curve>& r_xcvs) {
    if (! a_cv.is_valid()) {
      message("Failed to construct a conic arc.");
      return false;
    }
    std::vector<std::variant<Conic_point, Conic_X_monotone_curve>> objs;
    a_traits.make_x_monotone_2_object()(a_cv, std::back_inserter(objs));
    for (const auto& obj : objs) {
      if (auto* xcv = std::get_if<Conic_X_monotone_curve>(&obj)) r_xcvs.push_back(*xcv);
    }
    return true;
  }

  /*! adds a conic to the boundary under construction.
   */
  void add_conic(const Rational* a_c) {
    std::string error;
    auto conic = make_conic(a_c, error);
    if (! conic) {
      message(error.c_str());
      return;
    }
    if (! m_conics.empty() && same_conic(m_conics.back(), *conic)) {
      message("The conic is identical to the previous one.");
      return;
    }
    m_conics.push_back(*conic);
    m_valid_arcs.clear();
    m_arc_pieces.clear();
    m_ccw.push_back(-1);
    if (m_conics.size() == 1) {
      update_preview();
      return;
    }
    // The new vertex is an intersection of the last two conics, other than the previous vertex.
    auto points = intersections(m_conics[m_conics.size() - 2], m_conics.back());
    m_candidates.clear();
    for (const auto& p : points) {
      if (m_vertices.empty() || ! equal(p, m_vertices.back())) m_candidates.push_back(p);
    }
    if (m_candidates.empty()) {
      message("The conic does not meet the previous conic.");
      remove_last_conic();
      return;
    }
    if (m_candidates.size() == 1) choose_vertex(0);
    else m_state = State::PICK_VERTEX;
    update_preview();
  }

  /*! removes the last conic of the boundary under construction.
   */
  void remove_last_conic() {
    if (m_conics.empty()) return;
    m_conics.pop_back();
    m_valid_arcs.clear();
    m_arc_pieces.clear();
    m_ccw.pop_back();
    if (m_vertices.size() >= m_conics.size() && ! m_vertices.empty()) m_vertices.pop_back();
    // The arc of the (now) last conic is no longer determined.
    if (! m_ccw.empty()) m_ccw.back() = -1;
    m_state = State::ST_IDLE;
    update_preview();
  }

  /*! sets the next vertex to a given candidate.
   */
  void choose_vertex(std::size_t a_i) {
    m_vertices.push_back(m_candidates[a_i]);
    m_valid_arcs.clear();
    m_arc_pieces.clear();
    m_candidates.clear();
    m_state = State::ST_IDLE;
    // The previous conic now has both of its vertices.
    std::size_t k = m_conics.size() - 2;
    if ((k > 0) && ! check_arc(k)) {
      message("The two vertices of the conic are on different branches.");
      remove_last_conic();
    }
    else if ((k > 0) && is_determined(k) && ! is_valid_arc(k)) {
      message("The conic would make the arc of the previous conic intersect the boundary.");
      remove_last_conic();
    }
    next_arc();
  }

  /*! obtains the endpoints of the arc of the i-th conic.
   * \return false if they are not known yet.
   */
  bool endpoints(std::size_t a_i, Conic_point& r_s, Conic_point& r_t) const {
    std::size_t n = m_conics.size();
    if (a_i == 0) {
      if (! m_closing || m_vertices.empty()) return false;
      r_s = m_close_vertex;
      r_t = m_vertices[0];
      return true;
    }
    if (a_i >= m_vertices.size() + ((m_closing && (a_i == n - 1)) ? 1 : 0)) return false;
    r_s = m_vertices[a_i - 1];
    r_t = (m_closing && (a_i == n - 1)) ? m_close_vertex : m_vertices[a_i];
    return true;
  }

  /*! checks that the arc of the i-th conic exists, and queues a choice between the two arcs of an ellipse.
   * \return false if there is no such arc.
   */
  bool check_arc(std::size_t a_i) {
    Conic_point s, t;
    endpoints(a_i, s, t);
    if (equal(s, t)) return false;
    const auto& shape = m_conics[a_i].m_shape;
    if (shape.is_bounded()) {
      m_arc_queue.push_back(a_i);
      return true;
    }
    QPolygonF arc;
    QPointF mid;
    return shape.arc(to_qt(s), to_qt(t), true, arc, mid);
  }

  /*! asks for the next arc choice, or completes the polygon if the boundary is closed and no choice is pending.
   */
  void next_arc() {
    if (! m_arc_queue.empty()) m_state = State::PICK_ARC;
    else if (m_closing) finish();
  }

  /*! closes the boundary: the last vertex is an intersection of the last and the first conics.
   */
  void close() {
    if (m_conics.size() < 2) {
      message("A boundary needs at least two conics.");
      return;
    }
    auto points = intersections(m_conics.back(), m_conics.front());
    m_candidates.clear();
    for (const auto& p : points) {
      if (! equal(p, m_vertices.back()) && ! equal(p, m_vertices.front())) m_candidates.push_back(p);
    }
    if (m_candidates.empty()) {
      message("The last conic does not meet the first conic.");
      return;
    }
    if (m_candidates.size() == 1) close_at(0);
    else m_state = State::PICK_CLOSING_VERTEX;
    update_preview();
  }

  /*! closes the boundary at a given candidate.
   */
  void close_at(std::size_t a_i) {
    m_close_vertex = m_candidates[a_i];
    m_valid_arcs.clear();
    m_arc_pieces.clear();
    m_candidates.clear();
    m_closing = true;
    m_state = State::ST_IDLE;
    if (! check_arc(m_conics.size() - 1) || ! check_arc(0)) {
      message("The two vertices of the conic are on different branches.");
      m_closing = false;
      m_valid_arcs.clear();
      m_arc_pieces.clear();
      m_arc_queue.clear();
      update_preview();
      return;
    }
    for (std::size_t i : {m_conics.size() - 1, std::size_t(0)}) {
      if (is_determined(i) && ! is_valid_arc(i)) {
        message("The polygon cannot be closed: the arc of the last or the first conic would intersect the boundary.");
        m_closing = false;
        m_valid_arcs.clear();
        m_arc_pieces.clear();
        m_arc_queue.clear();
        update_preview();
        return;
      }
    }
    next_arc();
    update_preview();
  }

  /*! obtains the arc of the i-th conic (and its middle point). On an ellipse, a_ccw (if not negative) overrides the
   * chosen arc.
   * \return the arc, or nullopt if it is not determined yet.
   */
  std::optional<QPolygonF> arc(std::size_t a_i, QPointF* r_mid = nullptr, int a_ccw = -1) const {
    Conic_point s, t;
    if (! endpoints(a_i, s, t)) return std::nullopt;
    const auto& shape = m_conics[a_i].m_shape;
    int ccw = (a_ccw >= 0) ? a_ccw : m_ccw[a_i];
    if (shape.is_bounded() && (ccw < 0)) return std::nullopt;
    QPolygonF res;
    QPointF mid;
    if (! shape.arc(to_qt(s), to_qt(t), ccw != 0, res, mid)) return std::nullopt;
    if (r_mid) *r_mid = mid;
    return res;
  }

  /*! obtains the distance of a point from (the vertices of) a polyline.
   */
  static double distance(const QPolygonF& a_polyline, QPointF a_p) {
    double best = std::numeric_limits<double>::max();
    for (const auto& q : a_polyline) best = std::min(best, std::hypot(q.x() - a_p.x(), q.y() - a_p.y()));
    return best;
  }

  /*! determines which of the two arcs of the ellipse waiting for a choice is nearer to a point.
   * \return 1 for the counterclockwise arc, and 0 for the clockwise one.
   */
  int nearer_arc(QPointF a_p) const {
    std::size_t i = m_arc_queue.front();
    auto ccw = arc(i, nullptr, 1), cw = arc(i, nullptr, 0);
    if (! ccw || ! cw) return 1;
    return (distance(*ccw, a_p) <= distance(*cw, a_p)) ? 1 : 0;
  }

  /*! constructs the (exact) arc of the i-th conic. On an ellipse, a_ccw (if not negative) overrides the chosen arc.
   * \return the arc, or nullopt if it is not determined yet.
   */
  std::optional<Conic_curve> exact_arc(std::size_t a_i, int a_ccw = -1) const {
    Conic_point s, t;
    if (! endpoints(a_i, s, t)) return std::nullopt;
    const Conic& conic = m_conics[a_i];
    const Rational* c = conic.m_c;
    Conic_traits traits;
    auto ctr = traits.construct_curve_2_object();
    // A segment is given by the equation of its line (and not only by its endpoints), so it can be written exactly.
    if (conic.m_shape.kind() == Conic_shape::Kind::KI_LINE)
      return ctr(c[0], c[1], c[2], c[3], c[4], c[5], CGAL::COLLINEAR, s, t);
    QPointF mid;
    if (! arc(a_i, &mid, a_ccw)) return std::nullopt;
    // The arc turns from its source, through its middle, to its target.
    QPointF ps = to_qt(s), pt = to_qt(t);
    double cross = (mid.x() - ps.x()) * (pt.y() - ps.y()) - (mid.y() - ps.y()) * (pt.x() - ps.x());
    CGAL::Orientation orient = (cross > 0) ? CGAL::COUNTERCLOCKWISE : CGAL::CLOCKWISE;
    return ctr(c[0], c[1], c[2], c[3], c[4], c[5], orient, s, t);
  }

  /*! determines whether the arc of the i-th conic is determined: its endpoints are known, and, on an ellipse, the arc
   * is chosen.
   */
  bool is_determined(std::size_t a_i) const {
    Conic_point s, t;
    if (! endpoints(a_i, s, t)) return false;
    return ! m_conics[a_i].m_shape.is_bounded() || (m_ccw[a_i] >= 0);
  }

  /*! determines whether the arc of the i-th conic (on an ellipse, the arc given by a_ccw if it is not negative) does
   * not make the boundary self-intersecting: it may meet the other determined arcs only at its endpoints, where they
   * end too (see is_valid_extension()).
   */
  bool is_valid_arc(std::size_t a_i, int a_ccw = -1) const {
    auto key = std::make_pair(a_i, a_ccw);
    auto it = m_valid_arcs.find(key);
    if (it != m_valid_arcs.end()) return it->second;
    bool r_valid = false;
    auto cv = exact_arc(a_i, a_ccw);
    if (cv && cv->is_valid()) {
      Conic_traits traits;
      auto piece = x_monotone_pieces(traits, &*cv, &*cv + 1);
      std::vector<Conic_curve> others;
      for (std::size_t j = 0; j < m_conics.size(); ++j) {
        if ((j == a_i) || ! is_determined(j)) continue;
        if (auto other = exact_arc(j)) others.push_back(*other);
      }
      auto boundary = x_monotone_pieces(traits, others.begin(), others.end());
      Conic_point s, t;
      endpoints(a_i, s, t);
      r_valid = is_valid_extension(traits, boundary, piece, s, &t);
    }
    m_valid_arcs[key] = r_valid;
    return r_valid;
  }

  /*! obtains the x-monotone arcs of the arc of the i-th conic, or nothing if the arc is not determined.
   */
  const std::vector<Conic_X_monotone_curve>& arc_pieces(std::size_t a_i) const {
    auto it = m_arc_pieces.find(a_i);
    if (it != m_arc_pieces.end()) return it->second;
    std::vector<Conic_X_monotone_curve> pieces;
    if (is_determined(a_i)) {
      auto cv = exact_arc(a_i);
      if (cv && cv->is_valid()) pieces = x_monotone_pieces(Conic_traits(), &*cv, &*cv + 1);
    }
    return m_arc_pieces[a_i] = pieces;
  }

  /*! adds the marks of the vertices of a boundary of polycurves under construction, as those of polylines (see
   * Polyline_boundary_pieces_graphics_item): a vertex where the boundary changes direction (see polycurve_boundary())
   * will be a vertex of the polygon, and so are, for now, the vertices next to arcs that are not determined yet. The
   * points where an arc changes direction are marked as well.
   */
  void add_polycurve_marks(QPainterPath& r_filled, QPainterPath& r_hollow) const {
    double r = 4 * pixel(), r_hollow_mark = 3 * pixel();
    auto mark = [&](const Conic_point& a_p, bool a_filled) {
      if (a_filled) r_filled.addEllipse(to_qt(a_p), r, r);
      else r_hollow.addEllipse(to_qt(a_p), r_hollow_mark, r_hollow_mark);
    };
    // A vertex between the arcs of conics i and j.
    auto mark_vertex = [&](const Conic_point& a_p, std::size_t a_i, std::size_t a_j) {
      const auto& in = arc_pieces(a_i);
      const auto& out = arc_pieces(a_j);
      mark(a_p, in.empty() || out.empty() || (curve_direction(in.back()) != curve_direction(out.front())));
    };
    std::size_t n = m_conics.size();
    for (std::size_t i = 0; i < n; ++i) {
      const auto& pieces = arc_pieces(i);
      auto starts = polycurve_starts(pieces);
      for (std::size_t j = 1; j < pieces.size(); ++j) mark(pieces[j].source(), starts[j]);
    }
    for (std::size_t k = 0; k < m_vertices.size(); ++k) {
      if (k + 1 < n) mark_vertex(m_vertices[k], k, k + 1);
      else mark(m_vertices[k], true);
    }
    if (m_closing) mark_vertex(m_close_vertex, n - 1, 0);
  }

  /*! constructs the arcs and emits the polygon.
   */
  void finish() {
    Conic_traits traits;
    auto ctr = traits.construct_curve_2_object();
    std::vector<Conic_X_monotone_curve> xcvs;
    for (std::size_t i = 0; i < m_conics.size(); ++i) {
      auto cv = exact_arc(i);
      if (! cv) {
        message("Failed to determine a conic arc.");
        return;
      }
      if (! append_x_monotone(traits, *cv, xcvs)) return;
    }
    Conic_polygon pgn(xcvs.begin(), xcvs.end());
    reset();
    emit(generate(CGAL::make_object(pgn)));
  }

  /*! obtains the index of the candidate vertex nearest to a point.
   */
  std::size_t nearest_candidate(QPointF a_p) const {
    std::size_t best = 0;
    double best_d = std::numeric_limits<double>::max();
    for (std::size_t i = 0; i < m_candidates.size(); ++i) {
      QPointF q = to_qt(m_candidates[i]);
      double d = std::hypot(q.x() - a_p.x(), q.y() - a_p.y());
      if (d < best_d) {
        best_d = d;
        best = i;
      }
    }
    return best;
  }

  /*! handles a mouse press. A right press closes the boundary; a left press picks a vertex or an arc, or is passed to
   * the current tool.
   * \return true if the event is handled.
   */
  bool mouse_press(QGraphicsSceneMouseEvent* a_event) {
    QPointF p = a_event->scenePos();
    m_cursor = p;
    if (a_event->button() == ::Qt::RightButton) {
      if ((m_state == State::ST_IDLE) && m_clicks.empty() && ! m_closing) close();
      return true;
    }
    if (a_event->button() != ::Qt::LeftButton) return false;
    switch (m_state) {
     case State::PICK_VERTEX: choose_vertex(nearest_candidate(p)); break;
     case State::PICK_CLOSING_VERTEX: close_at(nearest_candidate(p)); break;
     case State::PICK_ARC: {
       // An arc that would make the boundary self-intersecting cannot be picked.
       int ccw = nearer_arc(p);
       if (! is_valid_arc(m_arc_queue.front(), ccw)) break;
       m_ccw[m_arc_queue.front()] = ccw;
       m_valid_arcs.clear();
       m_arc_pieces.clear();
       m_arc_queue.erase(m_arc_queue.begin());
       m_state = State::ST_IDLE;
       next_arc();
       break;
     }
     case State::ST_IDLE: tool_press(p); break;
    }
    update_preview();
    return true;
  }

  /*! handles a mouse release, which completes a circle, a line, an ellipse, a parabola, or a hyperbola.
   * \return true if the event is handled.
   */
  bool mouse_release(QGraphicsSceneMouseEvent* a_event) {
    if (a_event->button() != ::Qt::LeftButton) return false;
    // A circle is defined by pressing at its center and releasing at its boundary.
    if (((m_mode == Mode::MO_CIRCLE) || (m_mode == Mode::FULL_CIRCLE)) && (m_clicks.size() == 1)) {
      QPointF center = m_clicks[0], p = a_event->scenePos();
      m_clicks.clear();
      if (std::hypot(p.x() - center.x(), p.y() - center.y()) < 2) {
        update_preview();
        return true;
      }
      Rational c[6];
      circle(center, p, c);
      if (m_mode == Mode::FULL_CIRCLE) generate_full(c);
      else add_conic(c);
      update_preview();
    }
    // A line is defined by pressing at one of its points and releasing at another. A release (almost) at the first
    // point defines no line; then a second click defines it instead.
    if ((m_mode == Mode::MO_LINE) && (m_clicks.size() == 1)) {
      QPointF p0 = m_clicks[0], p = a_event->scenePos();
      if (std::hypot(p.x() - p0.x(), p.y() - p0.y()) < 3 * pixel()) return true;
      m_clicks.clear();
      Rational c[6];
      if (line(p0, p, c)) add_conic(c);
      update_preview();
    }
    // An ellipse is defined by clicking at its center, pressing at the end of one semi-axis, and releasing at the
    // width of the other semi-axis. A parabola is defined by clicking at its vertex, pressing at a point of its axis,
    // and releasing at a point of the parabola. A hyperbola is defined by clicking at its center, pressing at a
    // vertex, and releasing at a point of an asymptote. A release (almost) on the line of the axis (or, for a parabola
    // or a hyperbola, on its normal through the first click) defines no conic; then a third click defines it instead.
    if (is_three_point_mode() && (m_clicks.size() == 2)) {
      QPointF center = m_clicks[0], a = m_clicks[1], p = a_event->scenePos();
      QPointF d = a - center, q = p - center;
      double len = std::hypot(d.x(), d.y());
      if ((len == 0) || (std::abs(d.x()*q.y() - d.y()*q.x()) / len < 3 * pixel())) return true;
      bool unbounded = (m_mode == Mode::MO_PARABOLA) || (m_mode == Mode::MO_HYPERBOLA);
      if (unbounded && (std::abs(d.x()*q.x() + d.y()*q.y()) / len < 3 * pixel())) return true;
      m_clicks.clear();
      Rational c[6];
      if (three_point_conic(center, a, p, c)) {
        if (m_mode == Mode::FULL_ELLIPSE) generate_full(c);
        else add_conic(c);
      }
      update_preview();
    }
    return true;
  }

  /*! records a click of the current tool, and constructs the conic once it has all its clicks.
   */
  void tool_press(QPointF a_p) {
    m_clicks.push_back(a_p);
    Rational c[6];
    switch (m_mode) {
     case Mode::MO_LINE:
      if (m_clicks.size() < 2) return;
      if (line(m_clicks[0], m_clicks[1], c)) add_conic(c);
      m_clicks.clear();
      return;

     case Mode::MO_CIRCLE:
     case Mode::FULL_CIRCLE: return;          // completed on release

     case Mode::MO_ELLIPSE:
     case Mode::MO_PARABOLA:
     case Mode::MO_HYPERBOLA:
     case Mode::FULL_ELLIPSE:
      if (m_clicks.size() < 3) return;
      if (three_point_conic(m_clicks[0], m_clicks[1], m_clicks[2], c)) {
        if (m_mode == Mode::FULL_ELLIPSE) generate_full(c);
        else add_conic(c);
      }
      m_clicks.clear();
      return;
    }
  }

  /*! handles a key press. Escape, Delete, and Backspace undo one step: they discard the conic under construction (the
   * clicks so far, or the choice of a vertex or an arc) if there is one, and otherwise remove the last conic of the
   * boundary.
   * \return true if the event is handled.
   */
  bool key_press(QKeyEvent* a_event) {
    if ((a_event->key() != ::Qt::Key_Escape) && (a_event->key() != ::Qt::Key_Delete) &&
        (a_event->key() != ::Qt::Key_Backspace))
      return false;
    if (! m_clicks.empty()) m_clicks.clear();
    else if (m_closing || (m_state != State::ST_IDLE)) {
      m_closing = false;
      m_valid_arcs.clear();
      m_arc_pieces.clear();
      m_arc_queue.clear();
      m_candidates.clear();
      if (m_state == State::PICK_VERTEX) remove_last_conic();
      m_state = State::ST_IDLE;
    }
    else remove_last_conic();
    update_preview();
    return true;
  }

  /*! obtains the visible part of the scene.
   */
  QRectF view_rect() const {
    if (m_scene->views().isEmpty()) return m_scene->sceneRect();
    QGraphicsView* view = m_scene->views().first();
    return view->mapToScene(view->viewport()->rect()).boundingRect();
  }

  /*! obtains the size of a pixel in scene units.
   */
  double pixel() const {
    if (m_scene->views().isEmpty()) return 1;
    QGraphicsView* view = m_scene->views().first();
    return 1 / std::max(1e-9, std::abs(view->transform().m11()));
  }

  /*! adds a polyline to a path.
   */
  static void add(QPainterPath& r_path, const QPolygonF& a_polyline) {
    if (a_polyline.isEmpty()) return;
    r_path.moveTo(a_polyline.front());
    for (int i = 1; i < a_polyline.size(); ++i) r_path.lineTo(a_polyline[i]);
  }

  /*! obtains the curve under construction by the current tool.
   */
  QPainterPath tool_preview() const {
    QPainterPath path;
    if (m_clicks.empty()) return path;
    QPointF p = m_cursor;
    Rational c[6];
    bool ok = false;
    switch (m_mode) {
     case Mode::MO_LINE: ok = line(m_clicks[0], p, c); break;

     case Mode::MO_CIRCLE:
     case Mode::FULL_CIRCLE: ok = (p != m_clicks[0]) && (circle(m_clicks[0], p, c), true); break;

     case Mode::MO_ELLIPSE:
     case Mode::MO_PARABOLA:
     case Mode::MO_HYPERBOLA:
     case Mode::FULL_ELLIPSE:
      // The first semi-axis (or the axis of a parabola), from the center (or the vertex), follows the cursor until it
      // is picked. Then the conic is shown instead, but while there is no conic (e.g., right after the second click,
      // when the cursor is on the axis) the axis is shown again.
      ok = (m_clicks.size() == 2) && three_point_conic(m_clicks[0], m_clicks[1], p, c);
      if (! ok) {
        path.moveTo(m_clicks[0]);
        path.lineTo((m_clicks.size() == 1) ? p : m_clicks[1]);
        return path;
      }
      // The asymptotes of a hyperbola, through the center and the cursor and its reflection in the axis.
      if (m_mode == Mode::MO_HYPERBOLA) {
        QPointF center = m_clicks[0], d = m_clicks[1] - center, q = p - center;
        double d2 = d.x()*d.x() + d.y()*d.y();
        QPointF along = d * ((d.x()*q.x() + d.y()*q.y()) / d2);
        for (QPointF a : {q, 2 * along - q}) {
          path.moveTo(center - a);
          path.lineTo(center + a);
        }
      }
      break;
    }
    if (! ok) return path;
    std::string error;
    auto conic = make_conic(c, error);
    if (conic) {
      for (const auto& pl : conic->m_shape.curve(view_rect())) add(path, pl);
    }
    return path;
  }

  /*! redraws everything under construction: the conics, their arcs, the vertices, the candidate vertices, and the
   * curve under construction.
   */
  void update_preview() {
    QRectF rect = view_rect();
    QPainterPath curves, arcs, markers;
    for (std::size_t i = 0; i < m_conics.size(); ++i) {
      for (const auto& pl : m_conics[i].m_shape.curve(rect)) add(curves, pl);
      if (auto a = arc(i)) add(arcs, *a);
    }
    double r = 4 * pixel();
    QPainterPath hollow_markers;
    if (m_marked) add_polycurve_marks(markers, hollow_markers);
    else {
      for (const auto& v : m_vertices) markers.addEllipse(to_qt(v), r, r);
      if (m_closing) markers.addEllipse(to_qt(m_close_vertex), r, r);
    }
    QPainterPath candidates;
    for (const auto& v : m_candidates) candidates.addEllipse(to_qt(v), r, r);
    QPainterPath preview = tool_preview();
    bool valid_pick = true;
    if (m_state == State::PICK_ARC) {
      int ccw = nearer_arc(m_cursor);
      if (auto a = arc(m_arc_queue.front(), nullptr, ccw)) add(preview, *a);
      valid_pick = is_valid_arc(m_arc_queue.front(), ccw);
    }
    QPen dashed = m_pen;
    dashed.setStyle(::Qt::DashLine);
    QPen thick = m_pen;
    thick.setWidthF(2.5);
    m_curves_item->setPen(dashed);
    m_curves_item->setPath(curves);
    m_arcs_item->setPen(thick);
    m_arcs_item->setPath(arcs);
    QPen pick = thick;
    if (! valid_pick) pick.setStyle(::Qt::DashLine);
    m_preview_item->setPen((m_state == State::PICK_ARC) ? pick : m_pen);
    m_preview_item->setPath(preview);
    QPainterPath all_markers = markers;
    all_markers.addPath(candidates);
    m_markers_item->setPen(m_pen);
    m_markers_item->setBrush(m_pen.color());
    m_markers_item->setPath(all_markers);
    QPen hollow_pen = m_pen;
    hollow_pen.setWidthF(1.5);
    m_hollow_markers_item->setPen(hollow_pen);
    m_hollow_markers_item->setBrush(::Qt::white);
    m_hollow_markers_item->setPath(hollow_markers);
  }

  QGraphicsScene* m_scene;
  QGraphicsPathItem* m_curves_item;   // the conics (dashed)
  QGraphicsPathItem* m_arcs_item;     // their arcs (thick)
  QGraphicsPathItem* m_preview_item;  // the curve under construction
  QGraphicsPathItem* m_markers_item;  // the vertices and the candidates
  QGraphicsPathItem* m_hollow_markers_item;  // the vertices of polycurves (see add_polycurve_marks())
  QPen m_pen;

  Mode m_mode = Mode::MO_LINE;
  State m_state = State::ST_IDLE;
  QPointF m_cursor;
  std::vector<QPointF> m_clicks;            // of the current tool
  std::vector<Conic> m_conics;
  std::vector<int> m_ccw;                   // the arc of an ellipse (-1: undecided)
  std::vector<Conic_point> m_vertices;      // m_vertices[i] on conics i, i+1
  std::vector<Conic_point> m_candidates;    // for the next vertex
  std::vector<std::size_t> m_arc_queue;     // ellipses waiting for a choice
  bool m_closing = false;
  Conic_point m_close_vertex;               // on the last and the first conics
  // Whether arcs are valid (see is_valid_arc()), by the index of the conic and the chosen arc; cleared on a change.
  mutable std::map<std::pair<std::size_t, int>, bool> m_valid_arcs;
  // The x-monotone arcs of the arcs (see arc_pieces()), by the index of the conic; cleared on a change.
  mutable std::map<std::size_t, std::vector<Conic_X_monotone_curve>> m_arc_pieces;
  bool m_marked = false;                    // whether the vertices are marked as those of polycurves
  QString m_last_coefficients[6] = {"1", "1", "0", "0", "0", "-400"};
};

} // namespace Qt
} // namespace CGAL

#endif
