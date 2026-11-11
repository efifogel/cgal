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
#include <optional>
#include <string>
#include <vector>

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainterPath>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>

#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Arr_geometry_traits/Conic_intersections_2.h>

#include "Typedefs.h"
#include "Qt6/Conic_Curves.h"

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

  // The tools. The first three add a conic to the boundary under construction; the last two construct a full curve.
  enum Mode { LINE, CIRCLE, ELLIPSE, FULL_CIRCLE, FULL_ELLIPSE };

  /*! constructs from the parent object and the scene.
   */
  Graphics_view_conic_input(QObject* aParent, QGraphicsScene* aScene) :
    GraphicsViewInput(aParent),
    m_scene(aScene),
    m_curves_item(new QGraphicsPathItem),
    m_arcs_item(new QGraphicsPathItem),
    m_preview_item(new QGraphicsPathItem),
    m_markers_item(new QGraphicsPathItem) {
    for (auto* item : items()) {
      item->setZValue(10);
      m_scene->addItem(item);
    }
    set_pen(QPen(QColor(255, 0, 0), 0));
  }

  /*! sets the pen of the curves under construction.
   */
  void set_pen(const QPen& aPen) {
    m_pen = aPen;
    m_pen.setCosmetic(true);
    update_preview();
  }

  /*! sets the tool, and discards the clicks of the previous one.
   */
  void set_mode(Mode aMode) {
    m_mode = aMode;
    m_clicks.clear();
    update_preview();
  }

  /*! discards everything under construction.
   */
  void Reset() {
    m_conics.clear();
    m_vertices.clear();
    m_ccw.clear();
    m_clicks.clear();
    m_candidates.clear();
    m_arc_queue.clear();
    m_closing = false;
    m_state = IDLE;
    update_preview();
  }

  /*! handles the mouse and key events of the scene.
   * \return true if the event is handled.
   */
  bool eventFilter(QObject* aObject, QEvent* aEvent) {
    switch (aEvent->type()) {
     case QEvent::GraphicsSceneMousePress: return mouse_press(static_cast<QGraphicsSceneMouseEvent*>(aEvent));
     case QEvent::GraphicsSceneMouseRelease: return mouse_release(static_cast<QGraphicsSceneMouseEvent*>(aEvent));
     case QEvent::GraphicsSceneMouseMove:
      m_cursor = static_cast<QGraphicsSceneMouseEvent*>(aEvent)->scenePos();
      update_preview();
      return false;
     case QEvent::KeyPress: return key_press(static_cast<QKeyEvent*>(aEvent));
     default: break;
    }
    return QObject::eventFilter(aObject, aEvent);
  }

  /*! asks for the coefficients of a conic, and adds it to the boundary or constructs the full curve.
   */
  void enter_coefficients(QWidget* aParent) {
    QDialog dialog(aParent);
    dialog.setWindowTitle("Conic Coefficients");
    auto* layout = new QFormLayout(&dialog);
    layout->addRow(new QLabel("r x² + s y² + t xy + u x + v y + w = 0"));
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
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    Rational c[6];
    auto check = [&]() {
      bool ok = true;
      for (int i = 0; i < 6; ++i) ok = parse_rational(edits[i]->text(), c[i]) && ok;
      std::string error;
      std::optional<Kind> kind;
      if (ok) kind = classify(c, error);
      kind_label->setText(! ok ? QString("Invalid number") : kind ? kind_name(*kind) : QString(error.c_str()));
      bool is_ellipse = kind && (*kind == Conic_shape::ELLIPSE);
      full_button->setEnabled(is_ellipse);
      if (! is_ellipse && full_button->isChecked()) add_button->setChecked(true);
      buttons->button(QDialogButtonBox::Ok)->setEnabled(kind.has_value());
    };
    for (auto* edit : edits) QObject::connect(edit, &QLineEdit::textChanged, &dialog, check);
    check();
    if (dialog.exec() != QDialog::Accepted) return;
    for (int i = 0; i < 6; ++i) m_last_coefficients[i] = edits[i]->text();
    if (full_button->isChecked()) generate_full(c);
    else add_conic(c);
  }

private:
  enum State { IDLE, PICK_VERTEX, PICK_CLOSING_VERTEX, PICK_ARC };

  /*! A conic: its exact coefficients and its double-precision shape.
   */
  struct Conic {
    Rational m_c[6];
    Conic_shape m_shape;
  };

  /*! obtains the graphics items.
   */
  std::vector<QGraphicsPathItem*> items() const { return {m_curves_item, m_arcs_item, m_preview_item, m_markers_item}; }

  /*! converts a coordinate (0 for x, 1 for y) of a scene point to a rational number exactly.
   */
  static Rational to_rational(QPointF aP, int aI) { return Rational(aI == 0 ? aP.x() : aP.y()); }

  /*! parses an integer, a decimal number, or a fraction exactly.
   * \return false if the text is not a number.
   */
  static bool parse_rational(const QString& aText, Rational& rQ) {
    static const QRegularExpression re("^\\s*([+-]?)(\\d*)(?:\\.(\\d*))?(?:/(\\d+))?\\s*$");
    auto m = re.match(aText);
    if (! m.hasMatch()) return false;
    QString whole = m.captured(2), frac = m.captured(3), den = m.captured(4);
    if (whole.isEmpty() && frac.isEmpty()) return false;
    if (! den.isEmpty() && ! frac.isEmpty()) return false;
    Integer num((whole + frac).isEmpty() ? std::string("0") : (whole + frac).toStdString());
    Integer d = 1;
    for (int i = 0; i < frac.size(); ++i) d *= 10;
    if (! den.isEmpty()) d = Integer(den.toStdString());
    if (d == 0) return false;
    rQ = Rational(num, d);
    if (m.captured(1) == "-") rQ = -rQ;
    return true;
  }

  /*! obtains the name of a kind of conics.
   */
  static QString kind_name(Kind aKind) {
    switch (aKind) {
     case Conic_shape::LINE: return "Line";
     case Conic_shape::ELLIPSE: return "Ellipse";
     case Conic_shape::HYPERBOLA: return "Hyperbola";
     case Conic_shape::PARABOLA: return "Parabola";
    }
    return QString();
  }

  /*! obtains the kind of a non-degenerate conic.
   * \return the kind, or nullopt (and an error message) if the conic is degenerate.
   */
  static std::optional<Kind> classify(const Rational* aC, std::string& rError) {
    const Rational& r = aC[0], &s = aC[1], &t = aC[2], &u = aC[3], &v = aC[4], &w = aC[5];
    if (CGAL::is_zero(r) && CGAL::is_zero(s) && CGAL::is_zero(t)) {
      if (CGAL::is_zero(u) && CGAL::is_zero(v)) {
        rError = "Not a curve";
        return std::nullopt;
      }
      return Conic_shape::LINE;
    }
    // The determinant of [[r, t/2, u/2], [t/2, s, v/2], [u/2, v/2, w]].
    Rational det = r*(s*w - v*v/4) - t/2*(t*w/2 - u*v/4) + u/2*(t*v/4 - s*u/2);
    if (CGAL::is_zero(det)) {
      rError = "Degenerate (a pair of lines or a point)";
      return std::nullopt;
    }
    CGAL::Sign disc = CGAL::sign(4*r*s - t*t);
    if (disc == CGAL::ZERO) return Conic_shape::PARABOLA;
    if (disc == CGAL::NEGATIVE) return Conic_shape::HYPERBOLA;
    if (CGAL::sign((r + s) * det) != CGAL::NEGATIVE) {
      rError = "An ellipse with no real points";
      return std::nullopt;
    }
    return Conic_shape::ELLIPSE;
  }

  /*! constructs a conic from its coefficients.
   * \return the conic, or nullopt (and an error message) if the conic is degenerate.
   */
  static std::optional<Conic> make_conic(const Rational* aC, std::string& rError) {
    auto kind = classify(aC, rError);
    if (! kind) return std::nullopt;
    Conic conic;
    double d[6];
    for (int i = 0; i < 6; ++i) {
      conic.m_c[i] = aC[i];
      d[i] = CGAL::to_double(aC[i]);
    }
    conic.m_shape = Conic_shape(*kind, d);
    return conic;
  }

  /*! obtains the coefficients of the circle with a given center through a given point.
   */
  static void circle(QPointF aCenter, QPointF aP, Rational* rC) {
    Rational cx = to_rational(aCenter, 0), cy = to_rational(aCenter, 1);
    Rational dx = to_rational(aP, 0) - cx, dy = to_rational(aP, 1) - cy;
    rC[0] = 1; rC[1] = 1; rC[2] = 0;
    rC[3] = -2*cx; rC[4] = -2*cy;
    rC[5] = cx*cx + cy*cy - (dx*dx + dy*dy);
  }

  /*! obtains the coefficients of the ellipse with a given center, one of its axes ending at aA, and the other
   * semi-axis the distance of aB from that axis: q*xi^2 + |d|^4*eta^2 = |d|^4*q, where xi and eta are the coordinates
   * along d = aA - aCenter and its normal, and q = (normal . (aB - aCenter))^2.
   * \return false if there is no such ellipse.
   */
  static bool ellipse(QPointF aCenter, QPointF aA, QPointF aB, Rational* rC) {
    Rational cx = to_rational(aCenter, 0), cy = to_rational(aCenter, 1);
    Rational dx = to_rational(aA, 0) - cx, dy = to_rational(aA, 1) - cy;
    Rational bx = to_rational(aB, 0) - cx, by = to_rational(aB, 1) - cy;
    Rational d2 = dx*dx + dy*dy, e = -dy*bx + dx*by, q = e*e;
    if (CGAL::is_zero(d2) || CGAL::is_zero(q)) return false;
    Rational alpha = q, beta = d2*d2;
    // xi = a1 x + b1 y + c1, eta = a2 x + b2 y + c2.
    Rational a1 = dx, b1 = dy, c1 = -(dx*cx + dy*cy);
    Rational a2 = -dy, b2 = dx, c2 = dy*cx - dx*cy;
    rC[0] = alpha*a1*a1 + beta*a2*a2;
    rC[1] = alpha*b1*b1 + beta*b2*b2;
    rC[2] = 2*(alpha*a1*b1 + beta*a2*b2);
    rC[3] = 2*(alpha*a1*c1 + beta*a2*c2);
    rC[4] = 2*(alpha*b1*c1 + beta*b2*c2);
    rC[5] = alpha*c1*c1 + beta*c2*c2 - alpha*beta;
    return true;
  }

  /*! obtains the coefficients of the line through two points.
   * \return false if the points coincide.
   */
  static bool line(QPointF aP, QPointF aQ, Rational* rC) {
    Rational px = to_rational(aP, 0), py = to_rational(aP, 1);
    Rational qx = to_rational(aQ, 0), qy = to_rational(aQ, 1);
    if ((px == qx) && (py == qy)) return false;
    rC[0] = 0; rC[1] = 0; rC[2] = 0;
    rC[3] = qy - py; rC[4] = px - qx; rC[5] = qx*py - px*qy;
    return true;
  }

  /*! determines whether two conics are the same, that is, whether their coefficients are proportional.
   */
  static bool same_conic(const Conic& aC1, const Conic& aC2) {
    int k = 0;
    while (CGAL::is_zero(aC1.m_c[k])) ++k;
    if (CGAL::is_zero(aC2.m_c[k])) return false;
    Rational f = aC2.m_c[k] / aC1.m_c[k];
    for (int i = 0; i < 6; ++i) {
      if (aC2.m_c[i] != f * aC1.m_c[i]) return false;
    }
    return true;
  }

  /*! determines whether two points are equal.
   */
  static bool equal(const Conic_point& aP, const Conic_point& aQ)
  { return Conic_alg_kernel().compare_xy_2_object()(aP, aQ) == CGAL::EQUAL; }

  /*! converts a point to a scene point.
   */
  static QPointF to_qt(const Conic_point& aP) { return QPointF(CGAL::to_double(aP.x()), CGAL::to_double(aP.y())); }

  /*! obtains the (exact) intersection points of two conics.
   */
  static std::vector<Conic_point> intersections(const Conic& aC1, const Conic& aC2) {
    Conic_nt_traits nt;
    Integer a[6], b[6];
    nt.convert_coefficients(aC1.m_c, aC1.m_c + 6, a);
    nt.convert_coefficients(aC2.m_c, aC2.m_c + 6, b);
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
  void message(const QString& aText) { QMessageBox::warning(qobject_cast<QWidget*>(parent()), "Conic Arcs", aText); }

  /*! emits the polygon bounded by the full ellipse of the given coefficients.
   */
  void generate_full(const Rational* aC) {
    std::string error;
    auto conic = make_conic(aC, error);
    if (! conic || (conic->m_shape.kind() != Conic_shape::ELLIPSE)) {
      message(conic ? QString("A full curve must be an ellipse") : QString(error.c_str()));
      return;
    }
    Conic_traits traits;
    Conic_curve cv = traits.construct_curve_2_object()(aC[0], aC[1], aC[2], aC[3], aC[4], aC[5]);
    std::vector<Conic_X_monotone_curve> xcvs;
    if (! append_x_monotone(traits, cv, xcvs)) return;
    emit(generate(CGAL::make_object(Conic_polygon(xcvs.begin(), xcvs.end()))));
  }

  /*! subdivides a conic arc into x-monotone arcs, and appends them to a given vector.
   * \return false if the arc is invalid.
   */
  bool append_x_monotone(const Conic_traits& aTraits, const Conic_curve& aCv,
                         std::vector<Conic_X_monotone_curve>& rXcvs) {
    if (! aCv.is_valid()) {
      message("Failed to construct a conic arc");
      return false;
    }
    std::vector<std::variant<Conic_point, Conic_X_monotone_curve>> objs;
    aTraits.make_x_monotone_2_object()(aCv, std::back_inserter(objs));
    for (const auto& obj : objs) {
      if (auto* xcv = std::get_if<Conic_X_monotone_curve>(&obj)) rXcvs.push_back(*xcv);
    }
    return true;
  }

  /*! adds a conic to the boundary under construction.
   */
  void add_conic(const Rational* aC) {
    std::string error;
    auto conic = make_conic(aC, error);
    if (! conic) {
      message(error.c_str());
      return;
    }
    if (! m_conics.empty() && same_conic(m_conics.back(), *conic)) {
      message("The conic is identical to the previous one");
      return;
    }
    m_conics.push_back(*conic);
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
      message("The conic does not meet the previous conic");
      remove_last_conic();
      return;
    }
    if (m_candidates.size() == 1) choose_vertex(0);
    else m_state = PICK_VERTEX;
    update_preview();
  }

  /*! removes the last conic of the boundary under construction.
   */
  void remove_last_conic() {
    if (m_conics.empty()) return;
    m_conics.pop_back();
    m_ccw.pop_back();
    if (m_vertices.size() >= m_conics.size() && ! m_vertices.empty()) m_vertices.pop_back();
    // The arc of the (now) last conic is no longer determined.
    if (! m_ccw.empty()) m_ccw.back() = -1;
    m_state = IDLE;
    update_preview();
  }

  /*! sets the next vertex to a given candidate.
   */
  void choose_vertex(std::size_t aI) {
    m_vertices.push_back(m_candidates[aI]);
    m_candidates.clear();
    m_state = IDLE;
    // The previous conic now has both of its vertices.
    std::size_t k = m_conics.size() - 2;
    if ((k > 0) && ! check_arc(k)) {
      message("The two vertices of the conic are on different branches");
      remove_last_conic();
    }
    next_arc();
  }

  /*! obtains the endpoints of the arc of the i-th conic.
   * \return false if they are not known yet.
   */
  bool endpoints(std::size_t aI, Conic_point& rS, Conic_point& rT) const {
    std::size_t n = m_conics.size();
    if (aI == 0) {
      if (! m_closing || m_vertices.empty()) return false;
      rS = m_close_vertex;
      rT = m_vertices[0];
      return true;
    }
    if (aI >= m_vertices.size() + ((m_closing && (aI == n - 1)) ? 1 : 0)) return false;
    rS = m_vertices[aI - 1];
    rT = (m_closing && (aI == n - 1)) ? m_close_vertex : m_vertices[aI];
    return true;
  }

  /*! checks that the arc of the i-th conic exists, and queues a choice between the two arcs of an ellipse.
   * \return false if there is no such arc.
   */
  bool check_arc(std::size_t aI) {
    Conic_point s, t;
    endpoints(aI, s, t);
    if (equal(s, t)) return false;
    const auto& shape = m_conics[aI].m_shape;
    if (shape.is_bounded()) {
      m_arc_queue.push_back(aI);
      return true;
    }
    QPolygonF arc;
    QPointF mid;
    return shape.arc(to_qt(s), to_qt(t), true, arc, mid);
  }

  /*! asks for the next arc choice, or completes the polygon if the boundary is closed and no choice is pending.
   */
  void next_arc() {
    if (! m_arc_queue.empty()) m_state = PICK_ARC;
    else if (m_closing) finish();
  }

  /*! closes the boundary: the last vertex is an intersection of the last and the first conics.
   */
  void close() {
    if (m_conics.size() < 2) {
      message("A boundary needs at least two conics");
      return;
    }
    auto points = intersections(m_conics.back(), m_conics.front());
    m_candidates.clear();
    for (const auto& p : points) {
      if (! equal(p, m_vertices.back()) && ! equal(p, m_vertices.front())) m_candidates.push_back(p);
    }
    if (m_candidates.empty()) {
      message("The last conic does not meet the first conic");
      return;
    }
    if (m_candidates.size() == 1) close_at(0);
    else m_state = PICK_CLOSING_VERTEX;
    update_preview();
  }

  /*! closes the boundary at a given candidate.
   */
  void close_at(std::size_t aI) {
    m_close_vertex = m_candidates[aI];
    m_candidates.clear();
    m_closing = true;
    m_state = IDLE;
    if (! check_arc(m_conics.size() - 1) || ! check_arc(0)) {
      message("The two vertices of a conic are on different branches");
      m_closing = false;
      m_arc_queue.clear();
      update_preview();
      return;
    }
    next_arc();
    update_preview();
  }

  /*! obtains the arc of the i-th conic (and its middle point). On an ellipse, aCcw (if not negative) overrides the
   * chosen arc.
   * \return the arc, or nullopt if it is not determined yet.
   */
  std::optional<QPolygonF> arc(std::size_t aI, QPointF* rMid = nullptr, int aCcw = -1) const {
    Conic_point s, t;
    if (! endpoints(aI, s, t)) return std::nullopt;
    const auto& shape = m_conics[aI].m_shape;
    int ccw = (aCcw >= 0) ? aCcw : m_ccw[aI];
    if (shape.is_bounded() && (ccw < 0)) return std::nullopt;
    QPolygonF res;
    QPointF mid;
    if (! shape.arc(to_qt(s), to_qt(t), ccw != 0, res, mid)) return std::nullopt;
    if (rMid) *rMid = mid;
    return res;
  }

  /*! obtains the distance of a point from (the vertices of) a polyline.
   */
  static double distance(const QPolygonF& aPolyline, QPointF aP) {
    double best = std::numeric_limits<double>::max();
    for (const auto& q : aPolyline) best = std::min(best, std::hypot(q.x() - aP.x(), q.y() - aP.y()));
    return best;
  }

  /*! determines which of the two arcs of the ellipse waiting for a choice is nearer to a point.
   * \return 1 for the counterclockwise arc, and 0 for the clockwise one.
   */
  int nearer_arc(QPointF aP) const {
    std::size_t i = m_arc_queue.front();
    auto ccw = arc(i, nullptr, 1), cw = arc(i, nullptr, 0);
    if (! ccw || ! cw) return 1;
    return (distance(*ccw, aP) <= distance(*cw, aP)) ? 1 : 0;
  }

  /*! constructs the arcs and emits the polygon.
   */
  void finish() {
    Conic_traits traits;
    auto ctr = traits.construct_curve_2_object();
    std::vector<Conic_X_monotone_curve> xcvs;
    for (std::size_t i = 0; i < m_conics.size(); ++i) {
      Conic_point s, t;
      endpoints(i, s, t);
      const Conic& conic = m_conics[i];
      Conic_curve cv;
      if (conic.m_shape.kind() == Conic_shape::LINE) cv = ctr(s, t);
      else {
        QPointF mid;
        auto polyline = arc(i, &mid);
        if (! polyline) {
          message("Failed to determine a conic arc");
          return;
        }
        // The arc turns from its source, through its middle, to its target.
        QPointF ps = to_qt(s), pt = to_qt(t);
        double cross = (mid.x() - ps.x()) * (pt.y() - ps.y()) - (mid.y() - ps.y()) * (pt.x() - ps.x());
        CGAL::Orientation orient = (cross > 0) ? CGAL::COUNTERCLOCKWISE : CGAL::CLOCKWISE;
        const Rational* c = conic.m_c;
        cv = ctr(c[0], c[1], c[2], c[3], c[4], c[5], orient, s, t);
      }
      if (! append_x_monotone(traits, cv, xcvs)) return;
    }
    Conic_polygon pgn(xcvs.begin(), xcvs.end());
    Reset();
    emit(generate(CGAL::make_object(pgn)));
  }

  /*! obtains the index of the candidate vertex nearest to a point.
   */
  std::size_t nearest_candidate(QPointF aP) const {
    std::size_t best = 0;
    double best_d = std::numeric_limits<double>::max();
    for (std::size_t i = 0; i < m_candidates.size(); ++i) {
      QPointF q = to_qt(m_candidates[i]);
      double d = std::hypot(q.x() - aP.x(), q.y() - aP.y());
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
  bool mouse_press(QGraphicsSceneMouseEvent* aEvent) {
    QPointF p = aEvent->scenePos();
    m_cursor = p;
    if (aEvent->button() == ::Qt::RightButton) {
      if ((m_state == IDLE) && m_clicks.empty() && ! m_closing) close();
      return true;
    }
    if (aEvent->button() != ::Qt::LeftButton) return false;
    switch (m_state) {
     case PICK_VERTEX: choose_vertex(nearest_candidate(p)); break;
     case PICK_CLOSING_VERTEX: close_at(nearest_candidate(p)); break;
     case PICK_ARC: {
       m_ccw[m_arc_queue.front()] = nearer_arc(p);
       m_arc_queue.erase(m_arc_queue.begin());
       m_state = IDLE;
       next_arc();
       break;
     }
     case IDLE: tool_press(p); break;
    }
    update_preview();
    return true;
  }

  /*! handles a mouse release, which completes a circle, a line, or an ellipse.
   * \return true if the event is handled.
   */
  bool mouse_release(QGraphicsSceneMouseEvent* aEvent) {
    if (aEvent->button() != ::Qt::LeftButton) return false;
    // A circle is defined by pressing at its center and releasing at its boundary.
    if (((m_mode == CIRCLE) || (m_mode == FULL_CIRCLE)) && (m_clicks.size() == 1)) {
      QPointF center = m_clicks[0], p = aEvent->scenePos();
      m_clicks.clear();
      if (std::hypot(p.x() - center.x(), p.y() - center.y()) < 2) {
        update_preview();
        return true;
      }
      Rational c[6];
      circle(center, p, c);
      if (m_mode == FULL_CIRCLE) generate_full(c);
      else add_conic(c);
      update_preview();
    }
    // A line is defined by pressing at one of its points and releasing at another. A release (almost) at the first
    // point defines no line; then a second click defines it instead.
    if ((m_mode == LINE) && (m_clicks.size() == 1)) {
      QPointF p0 = m_clicks[0], p = aEvent->scenePos();
      if (std::hypot(p.x() - p0.x(), p.y() - p0.y()) < 3 * pixel()) return true;
      m_clicks.clear();
      Rational c[6];
      if (line(p0, p, c)) add_conic(c);
      update_preview();
    }
    // An ellipse is defined by clicking at its center, pressing at the end of one semi-axis, and releasing at the
    // width of the other semi-axis. A release (almost) on the line of the first semi-axis defines no ellipse; then a
    // third click defines it instead.
    if (((m_mode == ELLIPSE) || (m_mode == FULL_ELLIPSE)) && (m_clicks.size() == 2)) {
      QPointF center = m_clicks[0], a = m_clicks[1], p = aEvent->scenePos();
      QPointF d = a - center, q = p - center;
      double len = std::hypot(d.x(), d.y());
      if ((len == 0) || (std::abs(d.x()*q.y() - d.y()*q.x()) / len < 3 * pixel())) return true;
      m_clicks.clear();
      Rational c[6];
      if (ellipse(center, a, p, c)) {
        if (m_mode == FULL_ELLIPSE) generate_full(c);
        else add_conic(c);
      }
      update_preview();
    }
    return true;
  }

  /*! records a click of the current tool, and constructs the conic once it has all its clicks.
   */
  void tool_press(QPointF aP) {
    m_clicks.push_back(aP);
    Rational c[6];
    switch (m_mode) {
     case LINE:
      if (m_clicks.size() < 2) return;
      if (line(m_clicks[0], m_clicks[1], c)) add_conic(c);
      m_clicks.clear();
      return;

     case CIRCLE:
     case FULL_CIRCLE: return;          // completed on release

     case ELLIPSE:
     case FULL_ELLIPSE:
      if (m_clicks.size() < 3) return;
      if (ellipse(m_clicks[0], m_clicks[1], m_clicks[2], c)) {
        if (m_mode == FULL_ELLIPSE) generate_full(c);
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
  bool key_press(QKeyEvent* aEvent) {
    if ((aEvent->key() != ::Qt::Key_Escape) && (aEvent->key() != ::Qt::Key_Delete) &&
        (aEvent->key() != ::Qt::Key_Backspace))
      return false;
    if (! m_clicks.empty()) m_clicks.clear();
    else if (m_closing || (m_state != IDLE)) {
      m_closing = false;
      m_arc_queue.clear();
      m_candidates.clear();
      if (m_state == PICK_VERTEX) remove_last_conic();
      m_state = IDLE;
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
  static void add(QPainterPath& rPath, const QPolygonF& aPolyline) {
    if (aPolyline.isEmpty()) return;
    rPath.moveTo(aPolyline.front());
    for (int i = 1; i < aPolyline.size(); ++i) rPath.lineTo(aPolyline[i]);
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
     case LINE: ok = line(m_clicks[0], p, c); break;

     case CIRCLE:
     case FULL_CIRCLE: ok = (p != m_clicks[0]) && (circle(m_clicks[0], p, c), true); break;

     case ELLIPSE:
     case FULL_ELLIPSE:
      // The first semi-axis, from the center, follows the cursor until it is picked. Then the ellipse is shown
      // instead, but while the cursor is on the line of the semi-axis (e.g., right after the second click) there is
      // no ellipse, and the semi-axis is shown again.
      ok = (m_clicks.size() == 2) && ellipse(m_clicks[0], m_clicks[1], p, c);
      if (! ok) {
        path.moveTo(m_clicks[0]);
        path.lineTo((m_clicks.size() == 1) ? p : m_clicks[1]);
        return path;
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
    for (const auto& v : m_vertices) markers.addEllipse(to_qt(v), r, r);
    if (m_closing) markers.addEllipse(to_qt(m_close_vertex), r, r);
    QPainterPath candidates;
    for (const auto& v : m_candidates) candidates.addEllipse(to_qt(v), r, r);
    QPainterPath preview = tool_preview();
    if (m_state == PICK_ARC) {
      int ccw = nearer_arc(m_cursor);
      if (auto a = arc(m_arc_queue.front(), nullptr, ccw)) add(preview, *a);
    }
    QPen dashed = m_pen;
    dashed.setStyle(::Qt::DashLine);
    QPen thick = m_pen;
    thick.setWidthF(2.5);
    m_curves_item->setPen(dashed);
    m_curves_item->setPath(curves);
    m_arcs_item->setPen(thick);
    m_arcs_item->setPath(arcs);
    m_preview_item->setPen((m_state == PICK_ARC) ? thick : m_pen);
    m_preview_item->setPath(preview);
    QPainterPath all_markers = markers;
    all_markers.addPath(candidates);
    m_markers_item->setPen(m_pen);
    m_markers_item->setBrush(m_pen.color());
    m_markers_item->setPath(all_markers);
  }

  QGraphicsScene* m_scene;
  QGraphicsPathItem* m_curves_item;   // the conics (dashed)
  QGraphicsPathItem* m_arcs_item;     // their arcs (thick)
  QGraphicsPathItem* m_preview_item;  // the curve under construction
  QGraphicsPathItem* m_markers_item;  // the vertices and the candidates
  QPen m_pen;

  Mode m_mode = LINE;
  State m_state = IDLE;
  QPointF m_cursor;
  std::vector<QPointF> m_clicks;            // of the current tool
  std::vector<Conic> m_conics;
  std::vector<int> m_ccw;                   // the arc of an ellipse (-1: undecided)
  std::vector<Conic_point> m_vertices;      // m_vertices[i] on conics i, i+1
  std::vector<Conic_point> m_candidates;    // for the next vertex
  std::vector<std::size_t> m_arc_queue;     // ellipses waiting for a choice
  bool m_closing = false;
  Conic_point m_close_vertex;               // on the last and the first conics
  QString m_last_coefficients[6] = {"1", "1", "0", "0", "0", "-400"};
};

} // namespace Qt
} // namespace CGAL

#endif
