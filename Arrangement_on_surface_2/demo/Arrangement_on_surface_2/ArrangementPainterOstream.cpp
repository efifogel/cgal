// Copyright (c) 2012, 2020  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s): Alex Tsui <alextsui05@gmail.com>
//            Saurabh Singh <ssingh@cs.iitr.ac.in>
//            Ahmed Essam <theartful.ae@gmail.com>

#include "ArrangementPainterOstream.h"
#include "ArrangementTypes.h"

#include <QGraphicsView>

namespace CGAL {
namespace Qt {

// Specialization of `ArrangementPainterOstream` for the segment traits.
template <typename Kernel_>
ArrangementPainterOstream<CGAL::Arr_segment_traits_2<Kernel_>>&
ArrangementPainterOstream<CGAL::Arr_segment_traits_2<Kernel_>>::
operator<<(const X_monotone_curve_2& curve) {
  const Point_2& p1 = curve.source();
  const Point_2& p2 = curve.target();
  Segment_2 seg(p1, p2);

  // skip segments outside our view
  QRectF seg_bb = this->convert(seg.bbox());
  if (this->clippingRect.isValid() &&
      ! this->clippingRect.intersects(seg_bb) &&
      ! seg.is_horizontal() && ! seg.is_vertical())
    return *this;

  this->painterOstream << seg;
  return *this;
}

// Specialization of `ArrangementPainterOstream` for the polyline traits.
template <typename SegmentTraits>
ArrangementPainterOstream<CGAL::Arr_polyline_traits_2<SegmentTraits>>&
ArrangementPainterOstream<CGAL::Arr_polyline_traits_2<SegmentTraits>>::
operator<<(const X_monotone_curve_2& curve) {
  for (auto it = curve.subcurves_begin(); it != curve.subcurves_end(); ++it)
    this->painterOstream << *it;
  return *this;
}

// Specialization of `ArrangementPainterOstream` for the conic traits.
template <typename RatKernel, typename AlgKernel, typename NtTraits>
auto ArrangementPainterOstream
<CGAL::Arr_conic_traits_2<RatKernel, AlgKernel, NtTraits>>::
visibleParts(X_monotone_curve_2 curve) -> std::vector<X_monotone_curve_2> {
  // see if we intersect the bottom edge of the viewport
  Point_2 bottomLeft = this->convert(this->clippingRect.bottomLeft());
  Point_2 bottomRight = this->convert(this->clippingRect.bottomRight());
  Point_2 topLeft = this->convert(this->clippingRect.topLeft());
  Point_2 topRight = this->convert(this->clippingRect.topRight());
  X_monotone_curve_2 bottom =
    this->construct_x_monotone_curve_2(bottomLeft, bottomRight);
  X_monotone_curve_2 left =
    this->construct_x_monotone_curve_2(bottomLeft, topLeft);
  X_monotone_curve_2 top =
    this->construct_x_monotone_curve_2(topLeft, topRight);
  X_monotone_curve_2 right =
    this->construct_x_monotone_curve_2(topRight, bottomRight);

  std::vector<CGAL::Object> bottomIntersections;
  std::vector<CGAL::Object> leftIntersections;
  std::vector<CGAL::Object> topIntersections;
  std::vector<CGAL::Object> rightIntersections;
  std::vector<CGAL::Object> intersections;

  this->intersect_2(bottom, curve, bottomIntersections);
  this->intersect_2(left, curve, leftIntersections);
  this->intersect_2(top, curve, topIntersections);
  this->intersect_2(right, curve, rightIntersections);

  this->intersect_2(bottom, curve, intersections);
  this->intersect_2(left, curve, intersections);
  this->intersect_2(top, curve, intersections);
  this->intersect_2(right, curve, intersections);

  this->filterIntersectionPoints(intersections);

  Point_2 leftEndpt = curve.source();
  Point_2 rightEndpt = curve.target();

  if (leftEndpt.x() > rightEndpt.x()) std::swap(leftEndpt, rightEndpt);

  QPointF qendpt1 = this->convert(leftEndpt);
  QPointF qendpt2 = this->convert(rightEndpt);

  std::list<Point_2> pointList;
  for (unsigned int i = 0; i < intersections.size(); ++i) {
    CGAL::Object o = intersections[i];
    std::pair<Intersection_point_2, Multiplicity> pair;
    if (CGAL::assign(pair, o)) {
      Point_2 pt = pair.first;
      pointList.push_back(pt);
    }
  }

  bool includeLeftEndpoint = this->clippingRect.contains(qendpt1);
  bool includeRightEndpoint = this->clippingRect.contains(qendpt2);
  if (includeLeftEndpoint) pointList.push_front(leftEndpt);

  if (includeRightEndpoint) pointList.push_back(rightEndpt);

  // TODO: make ArrangementPainterOstream take traits object
  auto trim = traits.trim_2_object();
  std::vector<X_monotone_curve_2> clippings;
  auto it = pointList.begin();
  for (unsigned int i = 0; i < pointList.size(); i += 2) {
    typename Traits::Point_2 p1 = *it++;
    typename Traits::Point_2 p2 = *it++;
    auto xcv = trim(curve, p1, p2);
    clippings.push_back(xcv);
  }

  return clippings;
}

//
template <typename RatKernel, typename AlgKernel, typename NtTraits>
void ArrangementPainterOstream
<CGAL::Arr_conic_traits_2<RatKernel, AlgKernel, NtTraits>>::
filterIntersectionPoints(std::vector<CGAL::Object>& res) {
  std::vector<std::pair<Intersection_point_2, Multiplicity>> tmp;

  // filter out the non-intersection point results
  for (unsigned int i = 0; i < res.size(); ++i) {
    CGAL::Object obj = res[i];
    std::pair<Intersection_point_2, Multiplicity> pair;
    if (CGAL::assign(pair, obj)) tmp.push_back(pair);
  }
  res.clear();

  // sort the intersection points by x-coord
  Compare_intersection_point_result compare_intersection_point_result;
  std::sort(tmp.begin(), tmp.end(), compare_intersection_point_result);

  // box up the sorted elements
  for (unsigned int i = 0; i < tmp.size(); ++i) {
    std::pair<Intersection_point_2, Multiplicity> pair = tmp[i];
    CGAL::Object o = CGAL::make_object(pair);
    res.push_back(o);
  }
}

//
template <typename RatKernel, typename AlgKernel, typename NtTraits>
ArrangementPainterOstream
<CGAL::Arr_conic_traits_2<RatKernel, AlgKernel, NtTraits>>&
ArrangementPainterOstream
<CGAL::Arr_conic_traits_2<RatKernel, AlgKernel, NtTraits>>::
operator<<(const X_monotone_curve_2& curve) {
  CGAL::Bbox_2 bb = traits.construct_bbox_2_object()(curve);
  QRectF qbb = this->convert(bb);

  // quick cull
  if (this->clippingRect.isValid() && ! this->clippingRect.intersects(qbb))
    return *this;

  // get number of segments
  QGraphicsView* view = this->scene->views().first();
  auto pmin = view->mapFromScene(bb.xmin(), bb.ymin());
  auto pmax = view->mapFromScene(bb.xmax(), bb.ymax());
  double world_xdiff = std::abs(bb.xmax() - bb.xmin());
  double screen_xdiff = std::abs(pmax.x() - pmin.x());
  double error = world_xdiff / screen_xdiff;

  auto paint_curve = [&](auto&& curve_) {
    using Traits_approximate_2 = typename Traits::Approximate_2;
    using Approximate_point_2 = typename Traits_approximate_2::Approximate_point_2;
    std::vector<Approximate_point_2> points;
    auto aprox = traits.approximate_2_object();
    aprox(curve_, error, std::back_inserter(points));
    auto p_curr = points.begin();
    auto end_pts = points.end();
    auto p_next = p_curr + 1;
    do {
      QPointF p1(p_curr->x(), p_curr->y());
      QPointF p2(p_next->x(), p_next->y());
      this->qp->drawLine(p1, p2);
      ++p_curr;
      ++p_next;
    } while (p_next != end_pts);
  };

  if (this->clippingRect.isValid()) {
    if (this->clippingRect.contains(qbb)) {
      paint_curve(curve);
      return *this;
    }
    std::vector<X_monotone_curve_2> parts;
    parts = this->visibleParts(curve);
    for (auto& part : parts) paint_curve(part);
    return *this;
  }
  // draw the whole curve
  paint_curve(curve);

  return *this;
}

// Specialization of `ArrangementPainterOstream` for the Bezier traits.
template <typename RatKernel, typename AlgKernel, typename NtTraits,
          typename BoundingTraits>
std::vector<std::pair<double, double>>
ArrangementPainterOstream
<CGAL::Arr_Bezier_curve_traits_2<RatKernel, AlgKernel, NtTraits,
                                 BoundingTraits>>::
getPoints(const X_monotone_curve_2& curve) {
  std::pair<double, double> param_range = curve.parameter_range();
  auto&& supporting_curve = curve.supporting_curve();

  std::vector<std::pair<double, double>> sampled_points;
  // TODO: get adaptive number of samples
  unsigned int number_of_samples =
    100 * (param_range.second - param_range.first);
  sampled_points.reserve(number_of_samples);

  supporting_curve.sample(
    param_range.first, param_range.second, number_of_samples,
    std::back_inserter(sampled_points));
  return sampled_points;
}

//
template <typename RatKernel, typename AlgKernel, typename NtTraits,
          typename BoundingTraits>
auto ArrangementPainterOstream
<CGAL::Arr_Bezier_curve_traits_2<RatKernel, AlgKernel, NtTraits,
                                 BoundingTraits>>::
operator<<(const X_monotone_curve_2& curve)
  -> ArrangementPainterOstream<Traits>&
{
  auto sampled_points = this->getPoints(curve);
  if (sampled_points.empty()) return *this;

  QPainterPath painterPath;
  painterPath.moveTo(sampled_points[0].first, sampled_points[0].second);

  for (auto& p : sampled_points) painterPath.lineTo(p.first, p.second);

  this->qp->drawPath(painterPath);
  return *this;
}

// Specialization of `ArrangementPainterOstream` for the linear traits.
template <typename Kernel_>
ArrangementPainterOstream<CGAL::Arr_linear_traits_2<Kernel_>>&
ArrangementPainterOstream<CGAL::Arr_linear_traits_2<Kernel_>>::
operator<<(const X_monotone_curve_2& curve) {
  if (curve.is_segment()) {
    Segment_2 seg = curve.segment();

    // skip segments outside our view
    QRectF seg_bb = this->convert(seg.bbox());
    if (this->clippingRect.isValid() &&
        ! this->clippingRect.intersects(seg_bb) &&
        (!seg.is_horizontal() && !seg.is_vertical()))
      return *this;

    this->painterOstream << seg;
  }
  else if (curve.is_ray()) {
    Ray_2 ray = curve.ray();
    QLineF qseg = this->convert(ray);
    if (qseg.isNull()) return *this;    // it's out of view
    Segment_2 seg = this->convert(qseg);
    this->painterOstream << seg;
  }
  else {
    // curve.is_line( )
    Line_2 line = curve.line();
    QLineF qseg = this->convert(line);
    if (qseg.isNull()) return *this;    // it's out of view
    Segment_2 seg = this->convert(qseg);
    this->painterOstream << seg;
  }
  return *this;
}

// Specialization of `ArrangementPainterOstream` for the algebraic traits.
template <typename Coefficient_>
auto ArrangementPainterOstream
<CGAL::Arr_algebraic_segment_traits_2<Coefficient_>>::
getPointsList(const X_monotone_curve_2& curve) -> std::vector<Coord_vec_2> {
  std::vector<Coord_vec_2> points;
  QGraphicsView* view = this->getView();
  if (view == nullptr || view->width() <= 0) return points;

  // approximate within the viewport, extended by a margin, so that the parts
  // of the polyline that run along the sides of the box are not visible
  QRectF viewport = this->viewportRect();
  const double pixel = viewport.width() / view->width();
  const double margin = 8 * pixel;
  viewport.adjust(-margin, -margin, margin, margin);
  const CGAL::Bbox_2 bbox = this->convert(viewport).bbox();

  using Approximate_point_2 = typename Traits::Approximate_2::Approximate_point_2;
  std::vector<Approximate_point_2> pts;
  this->traits.approximate_2_object()(curve, pixel, std::back_inserter(pts), bbox, true);
  if (pts.empty()) return points;
  points.emplace_back();
  points.back().reserve(pts.size());
  for (const auto& p : pts) points.back().emplace_back(p.x(), p.y());
  return points;
}

//
template <typename Coefficient_>
ArrangementPainterOstream<CGAL::Arr_algebraic_segment_traits_2<Coefficient_>>&
ArrangementPainterOstream<CGAL::Arr_algebraic_segment_traits_2<Coefficient_>>::
operator<<(const X_monotone_curve_2& curve) {
  this->paintCurve(curve);
  return *this;
}

//
template <typename Coefficient_>
void ArrangementPainterOstream
<CGAL::Arr_algebraic_segment_traits_2<Coefficient_>>::
paintCurve(const X_monotone_curve_2& curve) {
  for (const auto& vec : this->getPointsList(curve)) {
    if (vec.empty()) continue;
    QPainterPath path;
    path.moveTo(vec.front().first, vec.front().second);
    for (const auto& pt : vec) path.lineTo(pt.first, pt.second);
    this->qp->drawPath(path);
  }
}

// Specialization of `ArrangementPainterOstream` for Arr_rational_function_traits_2
template <typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{ return (v < lo) ? lo : (hi < v) ? hi : v; }

//
template <typename AlgebraicKernel_d_1_>
auto ArrangementPainterOstream
<CGAL::Arr_rational_function_traits_2<AlgebraicKernel_d_1_>>::
operator<<(const X_monotone_curve_2& curve) -> Self& {
  QPainterPath painterPath;
  const QRectF viewport = this->viewportRect();
  // overshoot so that the slope would be more accurate
  double min_y = viewport.top();
  double max_y = viewport.bottom();

  bool disconnected = true;
  bool first_point = true;

  double last_x = 0, last_y = 0;

  // TODO: this is ugly! clean up these conditions
  auto path_filler = [&](double x, double y) {
    if (y > max_y || y < min_y) {
      double x_ = x;
      double y_ = clamp(y, min_y, max_y);

      // make line to the first out of range point
      if (!disconnected) { painterPath.lineTo(x_, y_); }
      // connect between two out of range points when they cross different
      // boundaries
      else if ((last_y == min_y && y_ == max_y) ||
               (last_y == max_y && y_ == min_y))
      {
        painterPath.moveTo(last_x, last_y);
        painterPath.lineTo(x_, y_);
      }
      last_x = x_;
      last_y = y_;

      disconnected = true;
    }
    else if (first_point) {
      painterPath.moveTo(x, y);
      disconnected = false;
    }
    else {
      if (disconnected) {
        painterPath.moveTo(last_x, last_y);
        disconnected = false;
      }
      painterPath.lineTo(x, y);
    }
    first_point = false;
  };

  this->sample_points(curve, path_filler);
  this->qp->drawPath(painterPath);

  return *this;
}

//
template <typename AlgebraicKernel_d_1_>
auto ArrangementPainterOstream
<CGAL::Arr_rational_function_traits_2<AlgebraicKernel_d_1_>>::
getPointsList(const X_monotone_curve_2& curve) -> std::vector<Coord_vec_2> {
  std::vector<Coord_vec_2> points_list;
  Coord_vec_2* cur_list = nullptr;

  const QRectF viewport = this->viewportRect();
  // overshoot so that the slope would be more accurate
  double min_y = viewport.top();
  double max_y = viewport.bottom();

  bool disconnected = true;

  double last_x = 0, last_y = 0;
  bool first_point = false;

  // TODO: this is ugly! clean up these conditions
  auto path_filler = [&](double x, double y) {
    if (y > max_y || y < min_y) {
      double x_ = x;
      double y_ = clamp(y, min_y, max_y);

      if (!disconnected) cur_list->push_back({x_, y_});
      else if ((last_y == min_y && y_ == max_y) ||
               (last_y == max_y && y_ == min_y))
      {
        cur_list->push_back({last_x, last_y});
        cur_list->push_back({x_, y_});
      }
      last_x = x_;
      last_y = y_;

      disconnected = true;
    }
    else {
      if (disconnected) {
        points_list.emplace_back();
        cur_list = &points_list.back();
        if (!first_point) cur_list->push_back({last_x, last_y});
        disconnected = false;
      }
      cur_list->push_back({x, y});
    }
    first_point = false;
  };
  this->sample_points(curve, path_filler);

  return points_list;
}

//
template <typename AlgebraicKernel_d_1_>
template <typename Lambda>
void ArrangementPainterOstream
<CGAL::Arr_rational_function_traits_2<AlgebraicKernel_d_1_>>::
sample_points(const X_monotone_curve_2& curve, Lambda&& lambda) {
  // TODO: cache maximum and minimal points for each curve, and include them
  // in the sampled points
  const QRectF viewport = this->viewportRect();
  qreal min_x = viewport.left();
  qreal max_x = viewport.right();

  auto&& numer = curve._f.numer();
  auto&& denom = curve._f.denom();

  auto eval_at = [&](auto&& x) { return numer.evaluate(x) / denom.evaluate(x); };

  // be conservative and prefer this branch to avoid zero division
  if (curve.left_parameter_space_in_x() == ARR_INTERIOR &&
      curve.left_x().to_interval().second >= min_x)
  {
    min_x = curve.left_x().to_interval().second;
    switch (curve.left_parameter_space_in_y()) {
     case ARR_INTERIOR: {
       auto left_pt = curve.left().to_double();
       lambda(min_x, left_pt.second);
       break;
      }
     case ARR_TOP_BOUNDARY: {
       lambda(min_x, std::numeric_limits<double>::infinity());
       break;
      }
     case ARR_BOTTOM_BOUNDARY: {
       lambda(min_x, -std::numeric_limits<double>::infinity());
       break;
      }
     default: {
       CGAL_error();
     }
    }
  }
  else if (curve.right_parameter_space_in_x() != ARR_INTERIOR ||
           min_x < curve.right_x().to_interval().first)
    lambda(min_x, CGAL::to_double(eval_at(Rational{min_x})));
  else return;  // outside of viewport

  std::pair<double, double> last_pt;

  // be conservative and prefer this branch to avoid zero division
  if (curve.right_parameter_space_in_x() == ARR_INTERIOR &&
      curve.right_x().to_interval().first <= max_x)
  {
    max_x = curve.right_x().to_interval().first;
    switch (curve.right_parameter_space_in_y()) {
     case ARR_INTERIOR: {
       last_pt = {max_x, curve.right().to_double().second};
       break;
      }
     case ARR_TOP_BOUNDARY: {
       last_pt = {max_x, std::numeric_limits<double>::infinity()};
       break;
      }
     case ARR_BOTTOM_BOUNDARY: {
       last_pt = {max_x, -std::numeric_limits<double>::infinity()};
       break;
      }
     default: { CGAL_error(); }
    }
  }
  else if (max_x > min_x)
    last_pt = {max_x, CGAL::to_double(eval_at(Rational{max_x}))};
  else return;  // outside of viewport

  static constexpr int dx_pixel = 1;
  static constexpr int min_num_points = 20;
  QLineF ux_line = this->getView()->transform().map(QLineF{0, 0, 1, 0});
  Rational dx = (CGAL::min)(
    dx_pixel * Rational{1} / ux_line.length(),
    Rational{max_x - min_x} / min_num_points);

  for (Rational cur_x = min_x + dx; cur_x < max_x - dx; cur_x += dx)
    lambda(CGAL::to_double(cur_x), CGAL::to_double(eval_at(cur_x)));

  lambda(last_pt.first, last_pt.second);
}

ARRANGEMENT_DEMO_SPECIALIZE_TRAITS(ArrangementPainterOstream)

} // namespace Qt
} // namespace CGAL
