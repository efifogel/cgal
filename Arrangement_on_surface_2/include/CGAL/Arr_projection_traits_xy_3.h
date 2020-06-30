// Copyright (c) 1997-2010  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s): Efi Fogel         efifogel@gmail.com
//            Based on Projection_traits_3 developed by
//            Mariette Yvinec, Sebastien Loriot, and Mael Rouxel-Labbé

#ifndef CGAL_ARR_PROJECTION_TRAITS_XY_3_H
#define CGAL_ARR_PROJECTION_TRAITS_XY_3_H

namespace CGAL {

namespace internal {

// Project Point_3 along coordinate dim
template <typename Kernel, int dim>
struct Projector;

//! Project onto xy
template <typename Kernel>
struct Projector<Kernel, 2> {
  typedef typename Kernel::FT                   FT;

  typedef typename Kernel::Point_3              Point_3;
  typedef typename Kernel::Vector_3             Vector_3;

  typedef typename Kernel::Less_x_3             Less_x_2;
  typedef typename Kernel::Less_y_3             Less_y_2;
  typedef typename Kernel::Compare_x_3          Compare_x_2;
  typedef typename Kernel::Compare_y_3          Compare_y_2;
  typedef typename Kernel::Equal_x_3            Equal_x_2;
  typedef typename Kernel::Equal_y_3            Equal_y_2;

  static FT x(const Point_3& p) { return p.x(); }
  static FT y(const Point_3& p) { return p.y(); }
  static FT x(const Vector_3& p) { return p.x(); }
  static FT y(const Vector_3& p) { return p.y(); }

  static Bbox_2 bbox(const Bbox_3& bb)
  { return Bbox_2(bb.xmin(),bb.ymin(), bb.xmax(),bb.ymax()); }

  static const int x_index = 0;

  static const int y_index = 1;
};

template <typename Kernel, int dim>
class Intersect_projected_3 {
public:
  typedef typename Kernel::FT           FT;

  typedef typename Kernel::Point_3      Point;
  typedef typename Kernel::Line_3       Line;

  typedef typename Kernel::Point_2      Point_2;
  typedef typename Kernel::Line_2       Line_2;

private:
  const Kernel& m_kernel;

  FT alpha(const Point_2& p, const Point_2& source, const Point_2& target) const
  {
    FT dx = target.x() - source.x();
    FT dy = target.y() - source.y();
    return (CGAL::abs(dx) > CGAL::abs(dy)) ?
      (p.x() - source.x()) / dx : (p.y() - source.y()) / dy;
  }

public:
  FT x(const Point& p) const { return Projector<Kernel, dim>::x(p); }
  FT y(const Point& p) const { return Projector<Kernel, dim>::y(p); }

  Intersect_projected_3(const Kernel& kernel) : m_kernel(kernel) {}

  Point_2 project(const Point& p) const { return Point_2(x(p), y(p)); }
  Line_2 project(const Line& l) const
  { return Line_2(project(l.point(0)), project(l.point(1))); }

  typename cpp11::result_of<typename Kernel::Intersect_3(Line, Line)>::type
  operator()(const Line& l1, const Line& l2) const
  {
    typedef typename cpp11::result_of<typename Kernel::Intersect_3(Line, Line)>::type result_type;

    Point_2 l1_source = project(l1.point(0));
    Point_2 l1_target = project(l1.point(1));
    Point_2 l2_source = project(l2.point(0));
    Point_2 l2_target = project(l2.point(1));
    Line_2 l1_2(l1_source, l1_target);
    Line_2 l2_2(l2_source, l2_target);
    CGAL_precondition(! l1_2.is_degenerate());
    CGAL_precondition(! l2_2.is_degenerate());

    auto res = m_kernel.intersect_2_object()(l1_2, l2_2);
    if (! res) return result_type();

    // Check if we have a single intersection point.
    const Point_2* pi = boost::get<Point_2>(&*res);
    if (pi != nullptr) {
      // Compute the 3rd coordinate of the projected intersection point onto 3D
      // segments
      FT coords[3];
      auto d1 = l1.point(1)[dim] - l1.point(0)[dim];
      auto d2 = l2.point(1)[dim] - l2.point(0)[dim];
      FT z1 = l1.point(0)[dim] + (alpha(*pi, l1_source, l1_target) * d1);
      FT z2 = l2.point(0)[dim] + (alpha(*pi, l2_source, l2_target) * d2);

      coords[dim] = (z1 + z2) / FT(2);
      coords[Projector<Kernel, dim>::x_index] = pi->x();
      coords[Projector<Kernel, dim>::y_index] = pi->y();

      Point p(coords[0], coords[1], coords[2]);
      CGAL_assertion(x(p) == pi->x() && y(p) == pi->y());
      return result_type(p);
    }

    // The two supporting lines overlap.
    const Line_2* li = boost::get<Line_2>(&*res);
    CGAL_assertion(li != nullptr);

    FT src[3], tgt[3];
    auto d1 = l1.point(1)[dim] - l1.point(0)[dim];
    auto d2 = l1.point(1)[dim] - l1.point(0)[dim];
    //the third coordinate is the midpoint between the points on l1 and l2
    FT z1 = l1.point(0)[dim] + (alpha(li->point(0), l1_source, l1_target) * d1);
    FT z2 = l2.point(0)[dim] + (alpha(li->point(0), l2_source, l2_target) * d2);
    src[dim] = (z1 + z2) / FT(2);
    z1 = l1.point(0)[dim] + (alpha(li->point(1), l1_source, l1_target) * d1);
    z2 = l2.point(0)[dim] + (alpha(li->point(1), l2_source, l2_target) * d2);

    tgt[dim] = (z1 + z2) / FT(2);

    src[Projector<Kernel, dim>::x_index] = li->point(0).x();
    src[Projector<Kernel, dim>::y_index] = li->point(0).y();
    tgt[Projector<Kernel, dim>::x_index] = li->point(1).x();
    tgt[Projector<Kernel, dim>::y_index] = li->point(1).y();
    return result_type(Line(Point(src[0], src[1], src[2]),
                            Point(tgt[0], tgt[1], tgt[2])));
  }
};

//!
template <typename Kernel, int dim>
class Construct_bbox_projected_2 {
public:
  typedef typename Kernel::Point_3      Point;
  typedef Bbox_2                        result_type;

  Bbox_2 operator()(const Point& p) const
  {
    typename Kernel::Construct_bbox_3   bb;
    return Projector<Kernel, dim>::bbox((p));
  }
};

//!
template <typename Kernel, int dim>
class Orientation_projected_3 {
public:
  typedef typename Kernel::FT           FT;
  typedef typename Kernel::Point_2      Point_2;
  typedef typename Kernel::Point_3      Point;

  FT x(const Point& p) const { return Projector<Kernel, dim>::x(p); }
  FT y(const Point& p) const { return Projector<Kernel, dim>::y(p); }

  Point_2 project(const Point& p) const { return Point_2(x(p), y(p)); }

  CGAL::Orientation operator()(const Point& p, const Point& q, const Point& r)
    const
  { return CGAL::orientation(project(p), project(q), project(r)); }
};

//!
template <typename Kernel, int dim>
class Compare_slope_projected_3 {
private:
  const Kernel& m_kernel;

public:
  typedef typename Kernel::FT           FT;
  typedef typename Kernel::Point_3      Point;
  typedef typename Kernel::Line_3       Line;

  typedef typename Kernel::Point_2      Point_2;
  typedef typename Kernel::Line_2       Line_2;

  FT x(const Point& p) const { return Projector<Kernel, dim>::x(p); }
  FT y(const Point& p) const { return Projector<Kernel, dim>::y(p); }

  Compare_slope_projected_3(const Kernel& kernel) : m_kernel(kernel) {}

  Point_2 project(const Point& p) const { return Point_2(x(p), y(p)); }
  Line_2 project(const Line& l) const
  { return Line_2(project(l.point(0)), project(l.point(1))); }

  Comparison_result operator()(const Line& l1, const Line& l2) const
  { return m_kernel.compare_slope_2_object()(project(l1), project(l2)); }
};

//!
template <typename Kernel, int dim>
class Has_on_projected_3 {
private:
  const Kernel& m_kernel;

public:
  typedef typename Kernel::FT           FT;
  typedef typename Kernel::Point_3      Point;
  typedef typename Kernel::Line_3       Line;

  typedef typename Kernel::Point_2      Point_2;
  typedef typename Kernel::Line_2       Line_2;

  FT x(const Point& p) const { return Projector<Kernel, dim>::x(p); }
  FT y(const Point& p) const { return Projector<Kernel, dim>::y(p); }

  Has_on_projected_3(const Kernel& kernel) : m_kernel(kernel) {}

  Point_2 project(const Point& p) const { return Point_2(x(p), y(p)); }
  Line_2 project(const Line& l) const
  { return Line_2(project(l.point(0)), project(l.point(1))); }

  bool operator()(const Line& l, const Point& p) const
  { return m_kernel.has_on_2_object()(project(l), project(p)); }
};

template <typename Kernel, int dim>
class Is_vertical_projected_3 {
private:
  const Kernel& m_kernel;

public:
  typedef typename Kernel::FT           FT;
  typedef typename Kernel::Point_3      Point;
  typedef typename Kernel::Segment_3    Segment;
  typedef typename Kernel::Line_3       Line;

  typedef typename Kernel::Point_2      Point_2;
  typedef typename Kernel::Segment_2    Segment_2;
  typedef typename Kernel::Line_2       Line_2;

  FT x(const Point& p) const { return Projector<Kernel, dim>::x(p); }
  FT y(const Point& p) const { return Projector<Kernel, dim>::y(p); }

  Is_vertical_projected_3(const Kernel& kernel) : m_kernel(kernel) {}

  Point_2 project(const Point& p) const { return Point_2(x(p), y(p)); }

  Segment_2 project(const Segment& s) const
  { return Segment_2(project(s.point(0)), project(s.point(1))); }

  Line_2 project(const Line& l) const
  { return Line_2(project(l.point(0)), project(l.point(1))); }

  bool operator()(const Segment& s) const
  { return m_kernel.is_vertical_2_object()(project(s)); }

  bool operator()(const Line& l) const
  { return m_kernel.is_vertical_2_object()(project(l)); }
};

}

template <typename Kernel>
class Arr_projection_traits_xy_3 : public Kernel {
public:
  typedef typename Kernel::Point_3              Point_2;
  typedef typename Kernel::Segment_3            Segment_2;
  typedef typename Kernel::Ray_3                Ray_2;
  typedef typename Kernel::Line_3               Line_2;

  typedef typename Kernel::Construct_point_3    Construct_point_2;
  typedef typename Kernel::Construct_vertex_3   Construct_vertex_2;
  typedef typename Kernel::Construct_segment_3  Construct_segment_2;
  typedef typename Kernel::Construct_line_3     Construct_line_2;
  typedef typename Kernel::Compare_x_3          Compare_x_2;
  typedef typename Kernel::Compare_y_3          Compare_y_2;
  typedef typename Kernel::Compare_xy_3         Compare_xy_2;
  typedef typename Kernel::Construct_opposite_line_3
    Construct_opposite_line_2;

  //! Obtain a Construct_vertex_2 object.
  Construct_vertex_2 construct_vertex_2_object() const
  { return Construct_vertex_2(); }

  //! Obtain a Construct_segment_2  object.
  Construct_segment_2 construct_segment_2_object() const
  {return Construct_segment_2(); }

  //! Obtain a Construct_line_2 object.
  Construct_line_2 construct_line_2_object() const
  { return Construct_line_2(); }

  //! Obtain a Construct_opposite_line_2 object.
  Construct_opposite_line_2 construct_opposite_line_2_object() const
  { return Construct_opposite_line_2(); }

  //! Obtain a Compare_xy_2 object.
  Compare_xy_2 compare_xy_2_object() const { return Compare_xy_2(); }

  //! Obtain a Compare_x_2 object.
  Compare_x_2 compare_x_2_object() const { return Compare_x_2(); }

  //! Obtain a Compare_y_2 object.
  Compare_y_2 compare_y_2_object() const { return Compare_y_2(); }

  typedef typename internal::Projector<Kernel, 2>::Equal_x_2  Equal_x_2;
  typedef typename internal::Projector<Kernel, 2>::Equal_y_2  Equal_y_2;

  //!
  struct Equal_2 {
    typedef bool result_type;
    bool operator()(const Point_2& p, const Point_2& q) const
    {
      Equal_x_2 eqx;
      Equal_y_2 eqy;
      return eqx(p, q) & eqy(p, q);
    }
  };

  //! Obtain a Equal_2 object.
  Equal_2 equal_2_object() const { return Equal_2(); }

  //!
  class Intersect_2 {
  private:
    const Kernel& m_kernel;

  public:
    Intersect_2(const Kernel& kernel) : m_kernel(kernel) {}

    typename cpp11::result_of<typename Kernel::Intersect_3(Line_2, Line_2)>::type
    operator()(const Line_2& l1, const Line_2& l2)
    { return internal::Intersect_projected_3<Kernel, 2>(m_kernel)(l1, l2); }
  };

  //! Obtain a Intersect_2 object
  Intersect_2 intersect_2_object() const { return Intersect_2(*this); }

  //! Has_on_2
  class Has_on_2 {
  private:
    const Kernel& m_kernel;

  public:
    Has_on_2(const Kernel& kernel) : m_kernel(kernel) {}

    bool operator()(const Line_2& l, const Point_2& p)
    { return internal::Has_on_projected_3<Kernel, 2>(m_kernel)(l, p); }
  };

  //! Obtain a Has_on_2 object.
  Has_on_2 has_on_2_object() const { return Has_on_2(*this); }

  //!
  class Is_vertical_2 {
  private:
    const Kernel& m_kernel;

  public:
    Is_vertical_2(const Kernel& kernel) : m_kernel(kernel) {}

    bool operator()(const Segment_2& s) const
    { return internal::Is_vertical_projected_3<Kernel, 2>(m_kernel)(s); }

    bool operator()(const Line_2& l) const
    { return internal::Is_vertical_projected_3<Kernel, 2>(m_kernel)(l); }
  };

  //! Obtain a Is_vertical_2 object.
  Is_vertical_2 is_vertical_2_object() const { return Is_vertical_2(*this); }

  //!
  class Compare_slope_2 {
  private:
    const Kernel& m_kernel;

  public:
    Compare_slope_2(const Kernel& kernel) : m_kernel(kernel) {}

    Comparison_result operator()(const Line_2& l1, const Line_2& l2) const
    { return internal::Compare_slope_projected_3<Kernel, 2>(m_kernel)(l1, l2); }
  };

  //! Obtain a Compare_slope_2 object.
  Compare_slope_2 compare_slope_2_object() const
  { return Compare_slope_2(*this); }

  //!
  typedef internal::Orientation_projected_3<Kernel, 2>        Orientation_2;

  //! Obtain a Orientation_2 object.
  Orientation_2 orientation_2_object() const { return Orientation_2(); }

  //!
  typedef internal::Construct_bbox_projected_2<Kernel, 2>     Construct_bbox_2;

  //! Obtain a Construct_bbox_2 object.
  Construct_bbox_2 construct_bbox_2_object() const
  { return Construct_bbox_2(); }
};

} //namespace CGAL

#endif // CGAL_PROJECTION_TRAITS_XY_3_H
