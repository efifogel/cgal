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
//
// Author(s) : Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//			   Ronnie Gandhi <ronniegandhi19999@gmail.com>

#ifndef CGAL_QT_CONIC_CURVES_H
#define CGAL_QT_CONIC_CURVES_H

#include <cmath>
#include <vector>

#include <QPointF>
#include <QPolygonF>
#include <QRectF>

#include "Typedefs.h"
#include "QT5/PiecewiseSetGraphicsItem.h"

namespace CGAL {
namespace Qt {

// The traits used to approximate and bound conic arcs for drawing.
inline const Conic_traits& conic_drawing_traits() {
  static const Conic_traits s_traits;
  return s_traits;
}

struct Conic_X_monotone_bbox {
  Bbox_2 operator()(Conic_X_monotone_curve const& aXcv) const
  { return conic_drawing_traits().construct_bbox_2_object()(aXcv); }
};

// Draw an x-monotone conic arc as a polyline from its source to its target.
struct Draw_conic_X_monotone_curve {
  template <typename Path>
  void operator()(Conic_X_monotone_curve const& aXcv, Path& aPath,
                  int aIdx) const {
    typedef Conic_traits::Approximate_2::Approximate_point_2 Approx_point;
    std::vector<Approx_point> points;
    conic_drawing_traits().approximate_2_object()
      (aXcv, 0.05, std::back_inserter(points), aXcv.is_directed_right());
    for (std::size_t i = 0; i < points.size(); ++i) {
      QPointF p(points[i].x(), points[i].y());
      if ((aIdx == 0) && (i == 0)) aPath.moveTo(p);
      else aPath.lineTo(p);
    }
  }
};

typedef Piecewise_set_graphics_item_bezier<Conic_polygon_set,
                                           Draw_conic_X_monotone_curve,
                                           Conic_X_monotone_bbox> Conic_GI;

// A non-degenerate conic, r*x^2 + s*y^2 + t*xy + u*x + v*y + w = 0, in
// double precision, with a parametrization for previewing and picking:
// - a line:      p0 + tau*d,
// - an ellipse:  c + a*cos(tau)*e1 + b*sin(tau)*e2,
// - a hyperbola: c + branch*a*cosh(tau)*e1 + b*sinh(tau)*e2 (branch = +-1),
// - a parabola:  tau*e1 + Y(tau)*e2,
// where e2 is e1 rotated by 90 degrees.
class Conic_shape {
public:
  enum Kind { LINE, ELLIPSE, HYPERBOLA, PARABOLA };

  Conic_shape() {}

  Conic_shape(Kind aKind, const double* aC) : m_kind(aKind) {
    for (int i = 0; i < 6; ++i) m_c[i] = aC[i];
    double r = m_c[0], s = m_c[1], t = m_c[2], u = m_c[3], v = m_c[4],
      w = m_c[5];
    if (m_kind == LINE) {
      double n2 = u*u + v*v;
      m_p0 = QPointF(-w*u/n2, -w*v/n2);
      m_e1 = QPointF(-v, u) / std::sqrt(n2);
      return;
    }
    // The eigenvalues and eigenvectors of [[r, t/2], [t/2, s]].
    double m = (r + s) / 2, d = std::hypot((r - s) / 2, t / 2);
    double l1 = m + d, l2 = m - d;
    if (m_kind == PARABOLA) {
      // The nonzero eigenvalue comes first.
      if (std::abs(l1) < std::abs(l2)) std::swap(l1, l2);
    }
    else if (m_kind == HYPERBOLA) {
      // The first axis is the one that crosses the curve.
      double cx, cy;
      center(cx, cy);
      double k0 = value(cx, cy);
      if (-k0 / l1 < 0) std::swap(l1, l2);
    }
    m_e1 = eigenvector(l1);
    m_e2 = QPointF(-m_e1.y(), m_e1.x());
    if (m_kind == PARABOLA) {
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

  Kind kind() const { return m_kind; }
  bool is_bounded() const { return m_kind == ELLIPSE; }

  QPointF point(int aBranch, double aTau) const {
    switch (m_kind) {
     case LINE: return m_p0 + aTau * m_e1;
     case ELLIPSE:
      return m_c0 + m_a*std::cos(aTau)*m_e1 + m_b*std::sin(aTau)*m_e2;
     case HYPERBOLA:
      return m_c0 + aBranch*m_a*std::cosh(aTau)*m_e1 +
        m_b*std::sinh(aTau)*m_e2;
     case PARABOLA:
      return aTau*m_e1 - (m_lambda*aTau*aTau + m_g*aTau + m_c[5])/m_h*m_e2;
    }
    return QPointF();
  }

  // The parameter (and the branch) of a point on the curve.
  void param(QPointF aP, int& rBranch, double& rTau) const {
    rBranch = 1;
    switch (m_kind) {
     case LINE: rTau = dot(aP - m_p0, m_e1); return;
     case ELLIPSE: {
       QPointF q = aP - m_c0;
       rTau = std::atan2(dot(q, m_e2) / m_b, dot(q, m_e1) / m_a);
       return;
     }
     case HYPERBOLA: {
       QPointF q = aP - m_c0;
       rBranch = (dot(q, m_e1) < 0) ? -1 : 1;
       rTau = std::asinh(dot(q, m_e2) / m_b);
       return;
     }
     case PARABOLA: rTau = dot(aP, m_e1); return;
    }
  }

  // Polylines that approximate the curve within the given rectangle.
  std::vector<QPolygonF> curve(QRectF aRect) const {
    std::vector<QPolygonF> res;
    double far = 0;
    QPointF origin = (m_kind == LINE) ? m_p0 :
      (m_kind == PARABOLA) ? QPointF(0, 0) : m_c0;
    for (QPointF corner : {aRect.topLeft(), aRect.topRight(),
                           aRect.bottomLeft(), aRect.bottomRight()})
      far = std::max(far, std::hypot(corner.x() - origin.x(),
                                     corner.y() - origin.y()));
    switch (m_kind) {
     case ELLIPSE: res.push_back(arc(1, 0, 2 * M_PI, 256)); break;
     case LINE: res.push_back(arc(1, -far, far, 2)); break;
     case PARABOLA: res.push_back(arc(1, -far, far, 800)); break;
     case HYPERBOLA: {
       double tau = std::acosh(std::max(1.0, far / m_a)) + 0.1;
       res.push_back(arc(1, -tau, tau, 400));
       res.push_back(arc(-1, -tau, tau, 400));
       break;
     }
    }
    return res;
  }

  QPolygonF arc(int aBranch, double aTau0, double aTau1, int aN) const {
    QPolygonF res;
    for (int i = 0; i <= aN; ++i)
      res << point(aBranch, aTau0 + (aTau1 - aTau0) * i / aN);
    return res;
  }

  // The arc from aSource to aTarget (both on the curve). On an ellipse,
  // aCcw selects the counterclockwise arc (in the parameter space, which is
  // counterclockwise in the plane). Return false if there is no such bounded
  // arc (the points are on different branches of a hyperbola).
  bool arc(QPointF aSource, QPointF aTarget, bool aCcw, QPolygonF& rArc,
           QPointF& rMid) const {
    int b0, b1;
    double t0, t1;
    param(aSource, b0, t0);
    param(aTarget, b1, t1);
    if (b0 != b1) return false;
    if (m_kind == ELLIPSE) {
      if (aCcw) { while (t1 <= t0) t1 += 2 * M_PI; }
      else { while (t1 >= t0) t1 -= 2 * M_PI; }
    }
    int n = (m_kind == LINE) ? 2 :
      std::max(16, int(std::abs(t1 - t0) / (2 * M_PI) * 256));
    if (m_kind == PARABOLA || m_kind == HYPERBOLA) n = 200;
    rArc = arc(b0, t0, t1, n);
    rArc.front() = aSource;
    rArc.back() = aTarget;
    rMid = point(b0, (t0 + t1) / 2);
    return true;
  }

private:
  static double dot(QPointF a, QPointF b) { return a.x()*b.x() + a.y()*b.y(); }

  double value(double x, double y) const {
    return m_c[0]*x*x + m_c[1]*y*y + m_c[2]*x*y + m_c[3]*x + m_c[4]*y + m_c[5];
  }

  // The center of a central conic: [[2r, t], [t, 2s]] c = -(u, v).
  void center(double& rX, double& rY) const {
    double r = m_c[0], s = m_c[1], t = m_c[2], u = m_c[3], v = m_c[4];
    double det = 4*r*s - t*t;
    rX = (-u*2*s + v*t) / det;
    rY = (-v*2*r + u*t) / det;
  }

  QPointF eigenvector(double aL) const {
    double r = m_c[0], s = m_c[1], t = m_c[2];
    QPointF v1(t / 2, aL - r), v2(aL - s, t / 2);
    QPointF v = (dot(v1, v1) > dot(v2, v2)) ? v1 : v2;
    double n = std::sqrt(dot(v, v));
    if (n == 0) return (std::abs(r - aL) < std::abs(s - aL)) ? QPointF(1, 0) :
                  QPointF(0, 1);
    return v / n;
  }

  Kind m_kind = LINE;
  double m_c[6] = {0, 0, 0, 0, 0, 0};
  QPointF m_p0, m_c0, m_e1, m_e2;
  double m_a = 0, m_b = 0, m_lambda = 0, m_g = 0, m_h = 1;
};

} // namespace Qt
} // namespace CGAL

#endif // CGAL_QT_CONIC_CURVES_H
