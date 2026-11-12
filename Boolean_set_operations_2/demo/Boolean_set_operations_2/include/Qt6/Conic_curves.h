// Copyright (c) 2009  GeometryFactory Sarl (France).
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
//             Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>

#ifndef CGAL_QT6_CONIC_CURVES_H
#define CGAL_QT6_CONIC_CURVES_H

#include <cmath>
#include <vector>

#include <QPointF>
#include <QPolygonF>
#include <QRectF>

#include "Typedefs.h"
#include "Qt6/Piecewise_set_graphics_item.h"

namespace CGAL {
namespace Qt {

/*! obtains the traits used to approximate and bound conic arcs for drawing.
 */
inline const Conic_traits& conic_drawing_traits() {
  static const Conic_traits s_traits;
  return s_traits;
}

/*! Bounds an x-monotone conic arc.
 */
struct Conic_X_monotone_bbox {
  /*! obtains the bounding box of an x-monotone conic arc.
   */
  Bbox_2 operator()(const Conic_X_monotone_curve& a_xcv) const
  { return conic_drawing_traits().construct_bbox_2_object()(a_xcv); }
};

/*! Draws an x-monotone conic arc as a polyline from its source to its target.
 */
struct Draw_conic_X_monotone_curve {
  /*! adds the polyline that approximates an x-monotone conic arc to the path. The first curve (of index 0) starts a
   * new subpath.
   */
  template <typename Path>
  void operator()(const Conic_X_monotone_curve& a_xcv, Path& a_path, int a_idx) const {
    using Approx_point = Conic_traits::Approximate_2::Approximate_point_2;
    std::vector<Approx_point> points;
    conic_drawing_traits().approximate_2_object()(a_xcv, 0.05, std::back_inserter(points), a_xcv.is_directed_right());
    for (std::size_t i = 0; i < points.size(); ++i) {
      QPointF p(points[i].x(), points[i].y());
      if ((a_idx == 0) && (i == 0)) a_path.moveTo(p);
      else a_path.lineTo(p);
    }
  }
};

using Conic_GI = Piecewise_set_graphics_item<Conic_polygon_set, Draw_conic_X_monotone_curve, Conic_X_monotone_bbox>;

/*! A non-degenerate conic, r*x^2 + s*y^2 + t*xy + u*x + v*y + w = 0, in double precision, with a parametrization for
 * previewing and picking:
 * - a line:      p0 + tau*d,
 * - an ellipse:  c + a*cos(tau)*e1 + b*sin(tau)*e2,
 * - a hyperbola: c + branch*a*cosh(tau)*e1 + b*sinh(tau)*e2 (branch = +-1),
 * - a parabola:  tau*e1 + Y(tau)*e2,
 * where e2 is e1 rotated by 90 degrees.
 */
class Conic_shape {
public:
  enum class Kind { KI_LINE, KI_ELLIPSE, KI_HYPERBOLA, KI_PARABOLA };

  /*! constructs default.
   */
  Conic_shape() {}

  /*! constructs from the kind of the conic and its six coefficients.
   */
  Conic_shape(Kind a_kind, const double* a_c) : m_kind(a_kind) {
    for (int i = 0; i < 6; ++i) m_c[i] = a_c[i];
    double r = m_c[0], s = m_c[1], t = m_c[2], u = m_c[3], v = m_c[4], w = m_c[5];
    if (m_kind == Kind::KI_LINE) {
      double n2 = u*u + v*v;
      m_p0 = QPointF(-w*u/n2, -w*v/n2);
      m_e1 = QPointF(-v, u) / std::sqrt(n2);
      return;
    }
    // The eigenvalues and eigenvectors of [[r, t/2], [t/2, s]].
    double m = (r + s) / 2, d = std::hypot((r - s) / 2, t / 2);
    double l1 = m + d, l2 = m - d;
    if (m_kind == Kind::KI_PARABOLA) {
      // The nonzero eigenvalue comes first.
      if (std::abs(l1) < std::abs(l2)) std::swap(l1, l2);
    }
    else if (m_kind == Kind::KI_HYPERBOLA) {
      // The first axis is the one that crosses the curve.
      double cx, cy;
      center(cx, cy);
      double k0 = value(cx, cy);
      if (-k0 / l1 < 0) std::swap(l1, l2);
    }
    m_e1 = eigenvector(l1);
    m_e2 = QPointF(-m_e1.y(), m_e1.x());
    if (m_kind == Kind::KI_PARABOLA) {
      m_lambda = l1;
      m_g = u * m_e1.x() + v * m_e1.y();
      m_h = u * m_e2.x() + v * m_e2.y();
      return;
    }
    double cx, cy;
    center(cx, cy);
    m_c0 = QPointF(cx, cy);
    double k0 = value(cx, cy);
    m_a = std::sqrt(std::abs(k0 / l1));
    m_b = std::sqrt(std::abs(k0 / l2));
  }

  /*! obtains the kind of the conic.
   */
  Kind kind() const { return m_kind; }

  /*! determines whether the conic is bounded (an ellipse).
   */
  bool is_bounded() const { return m_kind == Kind::KI_ELLIPSE; }

  /*! obtains the point of a given parameter on a given branch.
   */
  QPointF point(int a_branch, double a_tau) const {
    switch (m_kind) {
     case Kind::KI_LINE: return m_p0 + a_tau * m_e1;
     case Kind::KI_ELLIPSE: return m_c0 + m_a*std::cos(a_tau)*m_e1 + m_b*std::sin(a_tau)*m_e2;
     case Kind::KI_HYPERBOLA: return m_c0 + a_branch*m_a*std::cosh(a_tau)*m_e1 + m_b*std::sinh(a_tau)*m_e2;
     case Kind::KI_PARABOLA: return a_tau*m_e1 - (m_lambda*a_tau*a_tau + m_g*a_tau + m_c[5])/m_h*m_e2;
    }
    return QPointF();
  }

  /*! obtains the parameter and the branch of a point on the curve.
   */
  void param(QPointF a_p, int& r_branch, double& r_tau) const {
    r_branch = 1;
    switch (m_kind) {
     case Kind::KI_LINE: r_tau = dot(a_p - m_p0, m_e1); return;
     case Kind::KI_ELLIPSE: {
       QPointF q = a_p - m_c0;
       r_tau = std::atan2(dot(q, m_e2) / m_b, dot(q, m_e1) / m_a);
       return;
     }
     case Kind::KI_HYPERBOLA: {
       QPointF q = a_p - m_c0;
       r_branch = (dot(q, m_e1) < 0) ? -1 : 1;
       r_tau = std::asinh(dot(q, m_e2) / m_b);
       return;
     }
     case Kind::KI_PARABOLA: r_tau = dot(a_p, m_e1); return;
    }
  }

  /*! obtains polylines that approximate the curve within a given rectangle.
   */
  std::vector<QPolygonF> curve(QRectF a_rect) const {
    std::vector<QPolygonF> res;
    double far = 0;
    QPointF origin = (m_kind == Kind::KI_LINE) ? m_p0 : (m_kind == Kind::KI_PARABOLA) ? QPointF(0, 0) : m_c0;
    for (QPointF corner : {a_rect.topLeft(), a_rect.topRight(), a_rect.bottomLeft(), a_rect.bottomRight()})
      far = std::max(far, std::hypot(corner.x() - origin.x(), corner.y() - origin.y()));
    switch (m_kind) {
     case Kind::KI_ELLIPSE: res.push_back(arc(1, 0, 2 * M_PI, 256)); break;
     case Kind::KI_LINE: res.push_back(arc(1, -far, far, 2)); break;
     case Kind::KI_PARABOLA: res.push_back(arc(1, -far, far, 800)); break;
     case Kind::KI_HYPERBOLA: {
       double tau = std::acosh(std::max(1.0, far / m_a)) + 0.1;
       res.push_back(arc(1, -tau, tau, 400));
       res.push_back(arc(-1, -tau, tau, 400));
       break;
     }
    }
    return res;
  }

  /*! obtains the polyline through a_n+1 points equally spaced in the parameter range [a_tau0, a_tau1] of a branch.
   */
  QPolygonF arc(int a_branch, double a_tau0, double a_tau1, int a_n) const {
    QPolygonF res;
    for (int i = 0; i <= a_n; ++i) res << point(a_branch, a_tau0 + (a_tau1 - a_tau0) * i / a_n);
    return res;
  }

  /*! obtains the arc from a_source to a_target (both on the curve) and its middle point. On an ellipse, a_ccw selects
   * the counterclockwise arc (in the parameter space, which is counterclockwise in the plane).
   * \return false if there is no such bounded arc (the points are on different branches of a hyperbola).
   */
  bool arc(QPointF a_source, QPointF a_target, bool a_ccw, QPolygonF& r_arc, QPointF& r_mid) const {
    int b0, b1;
    double t0, t1;
    param(a_source, b0, t0);
    param(a_target, b1, t1);
    if (b0 != b1) return false;
    if (m_kind == Kind::KI_ELLIPSE) {
      if (a_ccw) {
        while (t1 <= t0) t1 += 2 * M_PI;
      }
      else {
        while (t1 >= t0) t1 -= 2 * M_PI;
      }
    }
    int n = (m_kind == Kind::KI_LINE) ? 2 : std::max(16, int(std::abs(t1 - t0) / (2 * M_PI) * 256));
    if (m_kind == Kind::KI_PARABOLA || m_kind == Kind::KI_HYPERBOLA) n = 200;
    r_arc = arc(b0, t0, t1, n);
    r_arc.front() = a_source;
    r_arc.back() = a_target;
    r_mid = point(b0, (t0 + t1) / 2);
    return true;
  }

private:
  /*! obtains the dot product of two vectors.
   */
  static double dot(QPointF a, QPointF b) { return a.x()*b.x() + a.y()*b.y(); }

  /*! evaluates the conic polynomial at a point.
   */
  double value(double x, double y) const { return m_c[0]*x*x + m_c[1]*y*y + m_c[2]*x*y + m_c[3]*x + m_c[4]*y + m_c[5]; }

  /*! obtains the center of a central conic: [[2r, t], [t, 2s]] c = -(u, v).
   */
  void center(double& r_x, double& r_y) const {
    double r = m_c[0], s = m_c[1], t = m_c[2], u = m_c[3], v = m_c[4];
    double det = 4*r*s - t*t;
    r_x = (-u*2*s + v*t) / det;
    r_y = (-v*2*r + u*t) / det;
  }

  /*! obtains a unit eigenvector of [[r, t/2], [t/2, s]] of a given eigenvalue.
   */
  QPointF eigenvector(double a_l) const {
    double r = m_c[0], s = m_c[1], t = m_c[2];
    QPointF v1(t / 2, a_l - r), v2(a_l - s, t / 2);
    QPointF v = (dot(v1, v1) > dot(v2, v2)) ? v1 : v2;
    double n = std::sqrt(dot(v, v));
    if (n == 0) return (std::abs(r - a_l) < std::abs(s - a_l)) ? QPointF(1, 0) : QPointF(0, 1);
    return v / n;
  }

  Kind m_kind = Kind::KI_LINE;
  double m_c[6] = {0, 0, 0, 0, 0, 0};
  QPointF m_p0, m_c0, m_e1, m_e2;
  double m_a = 0, m_b = 0, m_lambda = 0, m_g = 0, m_h = 1;
};

} // namespace Qt
} // namespace CGAL

#endif
