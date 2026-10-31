// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef CGAL_ARR_DATA_TRAITS_2_H
#define CGAL_ARR_DATA_TRAITS_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <CGAL/disable_warnings.h>

/*! \file
 * Definition of the Arr_data_policy_2<> and Arr_data_traits_2<> class
 * templates.
 */

#include <list>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/tags.h>
#include <CGAL/enum.h>
#include <CGAL/assertions.h>
#include <CGAL/Arr_tags.h>
#include <CGAL/Arr_enums.h>
#include <CGAL/Arr_geometry_traits/Curve_data_aux.h>

namespace CGAL {

/*! \class
 * A data policy for `Arr_data_traits_2` that attaches data to points and to
 * curves, and does not use the data to expedite any operation. It can be used
 * as is, or as a base class of a policy that overrides some of the functions.
 */
template <typename PointData, typename XMonotoneCurveData, typename CurveData = XMonotoneCurveData>
class Arr_data_policy_2 {
public:
  using Point_data = PointData;
  using X_monotone_curve_data = XMonotoneCurveData;
  using Curve_data = CurveData;

  /*! converts the data of a curve to the data of its x-monotone subcurves. */
  X_monotone_curve_data convert(const Curve_data& data) const { return X_monotone_curve_data(data); }

  /*! merges the data of two overlapping x-monotone curves. */
  X_monotone_curve_data merge_curve_data(const X_monotone_curve_data& data1,
                                         const X_monotone_curve_data& /* data2 */) const
  { return data1; }

  /*! merges the data of two coinciding points. */
  Point_data merge_point_data(const Point_data& data1, const Point_data& /* data2 */) const { return data1; }

  /*! obtains the data of an intersection point of two x-monotone curves. */
  template <typename XMonotoneCurve_2>
  Point_data intersection_point_data(const XMonotoneCurve_2& /* xcv1 */, const XMonotoneCurve_2& /* xcv2 */) const
  { return Point_data(); }

  /*! determines whether the data of two points guarantees that the points are
   * equal.
   */
  bool are_identical(const Point_data& /* data1 */, const Point_data& /* data2 */) const { return false; }

  /*! determines whether two x-monotone curves are guaranteed to have no common
   * point other than common endpoints, whose data are identical.
   */
  template <typename GeometryTraits_2, typename XMonotoneCurve_2>
  bool are_interior_disjoint(const GeometryTraits_2& /* traits */, const XMonotoneCurve_2& /* xcv1 */,
                             const XMonotoneCurve_2& /* xcv2 */) const
  { return false; }
};

/*! \class
 * A generic traits class for maintaining an arrangement of curves whose points
 * and curves have extra data fields. This traits class is templated with an
 * ordinary traits class, which is also used as a base traits class to inherit
 * from, and with a data policy, which determines the types of the data fields
 * and how they are propagated.
 * The data of the endpoints of every x-monotone curve are stored with the
 * curve. The data fields are updated when the curves are converted from
 * Curve_2 to X_monotone_curve_2, when the X_monotone_curve_2 curves are split,
 * when x-monotone curves intersect, and when points coincide. The data policy
 * may also guarantee that two points are equal or that two x-monotone curves
 * are interior disjoint, in which case the corresponding predicates and
 * constructions are not evaluated geometrically.
 * All other functors are inherited from the base ordinary traits class.
 */
template <typename GeometryTraits_2, typename DataPolicy_2>
class Arr_data_traits_2 : public GeometryTraits_2 {
public:
  using Base_traits_2 = GeometryTraits_2;
  using Data_policy_2 = DataPolicy_2;
  using Point_data = typename Data_policy_2::Point_data;
  using X_monotone_curve_data = typename Data_policy_2::X_monotone_curve_data;
  using Curve_data = typename Data_policy_2::Curve_data;

  using Base_point_2 = typename Base_traits_2::Point_2;
  using Base_x_monotone_curve_2 = typename Base_traits_2::X_monotone_curve_2;
  using Base_curve_2 = typename Base_traits_2::Curve_2;

  using Has_left_category = typename Base_traits_2::Has_left_category;
  using Has_merge_category = typename Base_traits_2::Has_merge_category;
  using Has_point_data_category = Tag_true;

  using Left_side_category = typename internal::Arr_complete_left_side_category<Base_traits_2>::Category;
  using Bottom_side_category = typename internal::Arr_complete_bottom_side_category<Base_traits_2>::Category;
  using Top_side_category = typename internal::Arr_complete_top_side_category<Base_traits_2>::Category;
  using Right_side_category = typename internal::Arr_complete_right_side_category<Base_traits_2>::Category;

  using Multiplicity = typename Base_traits_2::Multiplicity;

private:
  using Self = Arr_data_traits_2<Base_traits_2, Data_policy_2>;

public:
  /*! \class
   * A point extended with a data field.
   */
  class Point_2 : public Base_point_2 {
  private:
    Point_data m_data{};

  public:
    /*! constructs default. */
    Point_2() {}

    /*! constructs from a base point; the data is default constructed. */
    Point_2(const Base_point_2& p) : Base_point_2(p) {}

    /*! constructs from a base point and a data object. */
    Point_2(const Base_point_2& p, const Point_data& data) : Base_point_2(p), m_data(data) {}

    /*! obtains the data object (const version). */
    const Point_data& data() const { return m_data; }

    /*! obtains the data object (non-const version). */
    Point_data& data() { return m_data; }

    /*! sets the data object. */
    void set_data(const Point_data& data) { m_data = data; }
  };

  /*! \class
   * An x-monotone curve extended with a data field and with the data fields of
   * its endpoints.
   */
  class X_monotone_curve_2 : public Base_x_monotone_curve_2 {
  private:
    X_monotone_curve_data m_data{};
    Point_data m_min_data{};
    Point_data m_max_data{};

  public:
    /*! constructs default. */
    X_monotone_curve_2() {}

    /*! constructs from a base curve; the data objects are default constructed.
     */
    X_monotone_curve_2(const Base_x_monotone_curve_2& xcv) : Base_x_monotone_curve_2(xcv) {}

    /*! constructs from a base curve and a data object; the data objects of the
     * endpoints are default constructed.
     */
    X_monotone_curve_2(const Base_x_monotone_curve_2& xcv, const X_monotone_curve_data& data) :
      Base_x_monotone_curve_2(xcv),
      m_data(data)
    {}

    /*! constructs from a base curve, a data object, and the data objects of the
     * endpoints.
     */
    X_monotone_curve_2(const Base_x_monotone_curve_2& xcv, const X_monotone_curve_data& data,
                       const Point_data& min_data, const Point_data& max_data) :
      Base_x_monotone_curve_2(xcv),
      m_data(data),
      m_min_data(min_data),
      m_max_data(max_data)
    {}

    /*! obtains the data object (const version). */
    const X_monotone_curve_data& data() const { return m_data; }

    /*! obtains the data object (non-const version). */
    X_monotone_curve_data& data() { return m_data; }

    /*! sets the data object. */
    void set_data(const X_monotone_curve_data& data) { m_data = data; }

    /*! obtains the data object of an endpoint (const version). */
    const Point_data& end_data(Arr_curve_end ce) const { return (ce == ARR_MIN_END) ? m_min_data : m_max_data; }

    /*! obtains the data object of an endpoint (non-const version). */
    Point_data& end_data(Arr_curve_end ce) { return (ce == ARR_MIN_END) ? m_min_data : m_max_data; }

    /*! sets the data object of an endpoint. */
    void set_end_data(Arr_curve_end ce, const Point_data& data) { end_data(ce) = data; }
  };

  // Representation of a curve with an additional data field:
  using Curve_2 = _Curve_data_ex<Base_curve_2, Curve_data>;

private:
  using Intersection_point = std::pair<Point_2, Multiplicity>;
  using Base_intersection_point = std::pair<Base_point_2, Multiplicity>;
  using Base_intersection_result = std::variant<Base_intersection_point, Base_x_monotone_curve_2>;

  //! The data policy.
  Data_policy_2 m_policy;

  static constexpr bool s_open_left = Arr_is_side_open<Left_side_category>::result::value;
  static constexpr bool s_open_right = Arr_is_side_open<Right_side_category>::result::value;
  static constexpr bool s_open_bottom = Arr_is_side_open<Bottom_side_category>::result::value;
  static constexpr bool s_open_top = Arr_is_side_open<Top_side_category>::result::value;

  template <typename T, typename = void>
  struct Has_compare_endpoints_xy_2 : std::false_type {};

  template <typename T>
  struct Has_compare_endpoints_xy_2<T, std::void_t<typename T::Compare_endpoints_xy_2>> : std::true_type {};

  template <typename T, typename = void>
  struct Has_equal_operator : std::false_type {};

  template <typename T>
  struct Has_equal_operator<T, std::void_t<decltype(std::declval<const T&>() == std::declval<const T&>())>> :
    std::true_type {};

public:
  /// \name Construction.
  //@{

  /*! constructs default. */
  Arr_data_traits_2() {}

  /*! constructs from a base-traits class and a data policy. */
  Arr_data_traits_2(const Base_traits_2& traits, const Data_policy_2& policy = Data_policy_2()) :
    Base_traits_2(traits),
    m_policy(policy)
  {}
  //@}

  /// \name Access.
  //@{

  /*! obtains the base traits. */
  const Base_traits_2& base_traits() const { return *this; }

  /*! obtains the data policy. */
  const Data_policy_2& data_policy() const { return m_policy; }
  //@}

private:
  /*! determines whether an end of an x-monotone curve is closed. */
  bool is_closed(const Base_x_monotone_curve_2& xcv, Arr_curve_end ce) const {
    if constexpr (s_open_left || s_open_right) {
      const auto ps_x = base_traits().parameter_space_in_x_2_object()(xcv, ce);
      if (ps_x == ARR_LEFT_BOUNDARY) return ! s_open_left;
      if (ps_x == ARR_RIGHT_BOUNDARY) return ! s_open_right;
    }
    if constexpr (s_open_bottom || s_open_top) {
      const auto ps_y = base_traits().parameter_space_in_y_2_object()(xcv, ce);
      if (ps_y == ARR_BOTTOM_BOUNDARY) return ! s_open_bottom;
      if (ps_y == ARR_TOP_BOUNDARY) return ! s_open_top;
    }
    return true;
  }

  /*! obtains an endpoint of an x-monotone curve. */
  Base_point_2 vertex(const Base_x_monotone_curve_2& xcv, Arr_curve_end ce) const {
    return (ce == ARR_MIN_END) ? base_traits().construct_min_vertex_2_object()(xcv) :
      base_traits().construct_max_vertex_2_object()(xcv);
  }

  /*! obtains the data of an end of an x-monotone curve that is a subcurve of
   * two given x-monotone curves, that is, the merged data of the corresponding
   * ends of the two curves that coincide with it, or the data of an
   * intersection point of the two curves if there are none.
   */
  Point_data overlap_end_data(const Base_x_monotone_curve_2& ovl, Arr_curve_end ce,
                              const X_monotone_curve_2& xcv1, const X_monotone_curve_2& xcv2) const {
    if (! is_closed(ovl, ce)) return Point_data();
    auto equal = base_traits().equal_2_object();
    const auto p = vertex(ovl, ce);
    std::optional<Point_data> data;
    for (const auto* xcv : {&xcv1, &xcv2}) {
      if (! is_closed(*xcv, ce) || ! equal(p, vertex(*xcv, ce))) continue;
      data = data ? m_policy.merge_point_data(*data, xcv->end_data(ce)) : xcv->end_data(ce);
    }
    return data ? *data : m_policy.intersection_point_data(xcv1, xcv2);
  }

  /*! obtains the common endpoints of two x-monotone curves, whose data are
   * identical, sorted in an ascending xy-lexicographic order.
   */
  std::vector<Intersection_point> common_endpoints(const X_monotone_curve_2& xcv1,
                                                   const X_monotone_curve_2& xcv2) const {
    std::vector<Intersection_point> points;
    for (auto ce1 : {ARR_MIN_END, ARR_MAX_END}) {
      for (auto ce2 : {ARR_MIN_END, ARR_MAX_END}) {
        if (! m_policy.are_identical(xcv1.end_data(ce1), xcv2.end_data(ce2))) continue;
        CGAL_assertion(base_traits().equal_2_object()(vertex(xcv1, ce1), vertex(xcv2, ce2)));
        const auto data = m_policy.merge_point_data(xcv1.end_data(ce1), xcv2.end_data(ce2));
        // The multiplicity of a common endpoint is undefined.
        points.emplace_back(Point_2(vertex(xcv1, ce1), data), 0);
        break;
      }
    }
    return points;
  }

public:
  /// \name Overridden functors.
  //@{

  /*! \class Compare_x_2
   * A functor that compares the x-coordinates of two points.
   */
  class Compare_x_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Compare_x_2(const Self& traits) : m_traits(traits) {}

    /*! compares the x-coordinates of two points.
     * \param p1 the first point.
     * \param p2 the second point.
     * \return `EQUAL` if the data of the points guarantee that they are equal;
     *         otherwise, the comparison result of the base functor.
     */
    Comparison_result operator()(const Point_2& p1, const Point_2& p2) const {
      if (m_traits.m_policy.are_identical(p1.data(), p2.data())) {
        CGAL_assertion(m_traits.base_traits().compare_x_2_object()(p1, p2) == EQUAL);
        return EQUAL;
      }
      return m_traits.base_traits().compare_x_2_object()(p1, p2);
    }

    /*! forwards all other comparisons to the base functor. */
    template <typename... Args>
    Comparison_result operator()(const Args&... args) const
    { return m_traits.base_traits().compare_x_2_object()(args...); }
  };

  /*! obtains a `Compare_x_2` functor object. */
  Compare_x_2 compare_x_2_object() const { return Compare_x_2(*this); }

  /*! \class Compare_xy_2
   * A functor that compares two points lexicographically: by x, then by y.
   */
  class Compare_xy_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Compare_xy_2(const Self& traits) : m_traits(traits) {}

    /*! compares two points lexicographically: by x, then by y.
     * \param p1 the first point.
     * \param p2 the second point.
     * \return `EQUAL` if the data of the points guarantee that they are equal;
     *         otherwise, the comparison result of the base functor.
     */
    Comparison_result operator()(const Point_2& p1, const Point_2& p2) const {
      if (m_traits.m_policy.are_identical(p1.data(), p2.data())) {
        CGAL_assertion(m_traits.base_traits().compare_xy_2_object()(p1, p2) == EQUAL);
        return EQUAL;
      }
      return m_traits.base_traits().compare_xy_2_object()(p1, p2);
    }

    /*! forwards all other comparisons to the base functor. */
    template <typename... Args>
    Comparison_result operator()(const Args&... args) const
    { return m_traits.base_traits().compare_xy_2_object()(args...); }
  };

  /*! obtains a `Compare_xy_2` functor object. */
  Compare_xy_2 compare_xy_2_object() const { return Compare_xy_2(*this); }

  /*! \class Equal_2
   * A functor that determines whether two points or two curves are equal.
   */
  class Equal_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Equal_2(const Self& traits) : m_traits(traits) {}

    /*! determines whether two points are equal.
     * \param p1 the first point.
     * \param p2 the second point.
     * \return `true` if the data of the points guarantee that they are equal;
     *         otherwise, the result of the base functor.
     */
    bool operator()(const Point_2& p1, const Point_2& p2) const {
      if (m_traits.m_policy.are_identical(p1.data(), p2.data())) {
        CGAL_assertion(m_traits.base_traits().equal_2_object()(p1, p2));
        return true;
      }
      return m_traits.base_traits().equal_2_object()(p1, p2);
    }

    /*! forwards all other predicates to the base functor. */
    template <typename... Args>
    bool operator()(const Args&... args) const { return m_traits.base_traits().equal_2_object()(args...); }
  };

  /*! obtains an `Equal_2` functor object. */
  Equal_2 equal_2_object() const { return Equal_2(*this); }

  /*! \class Construct_min_vertex_2
   * A functor that obtains the left endpoint of an x-monotone curve.
   */
  class Construct_min_vertex_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Construct_min_vertex_2(const Self& traits) : m_traits(traits) {}

    /*! obtains the left endpoint of an x-monotone curve with its data. */
    Point_2 operator()(const X_monotone_curve_2& xcv) const
    { return Point_2(m_traits.base_traits().construct_min_vertex_2_object()(xcv), xcv.end_data(ARR_MIN_END)); }
  };

  /*! obtains a `Construct_min_vertex_2` functor object. */
  Construct_min_vertex_2 construct_min_vertex_2_object() const { return Construct_min_vertex_2(*this); }

  /*! \class Construct_max_vertex_2
   * A functor that obtains the right endpoint of an x-monotone curve.
   */
  class Construct_max_vertex_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Construct_max_vertex_2(const Self& traits) : m_traits(traits) {}

    /*! obtains the right endpoint of an x-monotone curve with its data. */
    Point_2 operator()(const X_monotone_curve_2& xcv) const
    { return Point_2(m_traits.base_traits().construct_max_vertex_2_object()(xcv), xcv.end_data(ARR_MAX_END)); }
  };

  /*! obtains a `Construct_max_vertex_2` functor object. */
  Construct_max_vertex_2 construct_max_vertex_2_object() const { return Construct_max_vertex_2(*this); }

  /*! \class Make_x_monotone_2
   * A functor for subdividing a curve into x-monotone curves.
   */
  class Make_x_monotone_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Make_x_monotone_2(const Self& traits) : m_traits(traits) {}

    /*! subdivides a given curve into x-monotone subcurves and isolated points,
     * and inserts them into a given output iterator. The data of the curve is
     * converted and attached to each one of the x-monotone subcurves. The data
     * of the isolated points and of the endpoints of the x-monotone subcurves
     * are default constructed.
     * \param cv the curve.
     * \param oi the output iterator for the result. Its value type is a variant
     *           that wraps `Point_2` or `X_monotone_curve_2` objects.
     * \return the past-the-end iterator.
     */
    template <typename OutputIterator>
    OutputIterator operator()(const Curve_2& cv, OutputIterator oi) const {
      using Base_make_x_monotone_result = std::variant<Base_point_2, Base_x_monotone_curve_2>;
      using Make_x_monotone_result = std::variant<Point_2, X_monotone_curve_2>;

      std::list<Base_make_x_monotone_result> base_objects;
      m_traits.base_traits().make_x_monotone_2_object()(cv, std::back_inserter(base_objects));

      const X_monotone_curve_data xdata = m_traits.m_policy.convert(cv.data());
      for (const auto& base_obj : base_objects) {
        if (const auto* bxcv = std::get_if<Base_x_monotone_curve_2>(&base_obj)) {
          *oi++ = Make_x_monotone_result(X_monotone_curve_2(*bxcv, xdata));
          continue;
        }
        const auto* bp = std::get_if<Base_point_2>(&base_obj);
        CGAL_assertion(bp);
        *oi++ = Make_x_monotone_result(Point_2(*bp));
      }
      return oi;
    }
  };

  /*! obtains a `Make_x_monotone_2` functor object. */
  Make_x_monotone_2 make_x_monotone_2_object() const { return Make_x_monotone_2(*this); }

  /*! \class Split_2
   * A functor that splits an x-monotone curve at a point.
   */
  class Split_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Split_2(const Self& traits) : m_traits(traits) {}

    /*! splits a given x-monotone curve at a given point into two subcurves.
     * Both subcurves obtain the data of the curve; the data of the point
     * becomes the data of their common endpoint.
     * \param xcv[in] the curve to split
     * \param p[in] the split point.
     * \param xcv1[out] the left resulting subcurve (`p` is its right endpoint).
     * \param xcv2[out] the right resulting subcurve (`p` is its left endpoint).
     * \pre `p` lies on `xcv` but is not one of its endpoints.
     */
    void operator()(const X_monotone_curve_2& xcv, const Point_2& p,
                    X_monotone_curve_2& xcv1, X_monotone_curve_2& xcv2) const {
      // Copy the data first, as the output curves may alias the input.
      const X_monotone_curve_data data = xcv.data();
      const Point_data min_data = xcv.end_data(ARR_MIN_END);
      const Point_data max_data = xcv.end_data(ARR_MAX_END);
      const Point_data p_data = p.data();

      m_traits.base_traits().split_2_object()(xcv, p, xcv1, xcv2);

      xcv1.set_data(data);
      xcv1.set_end_data(ARR_MIN_END, min_data);
      xcv1.set_end_data(ARR_MAX_END, p_data);
      xcv2.set_data(data);
      xcv2.set_end_data(ARR_MIN_END, p_data);
      xcv2.set_end_data(ARR_MAX_END, max_data);
    }
  };

  /*! obtains a `Split_2` functor object. */
  Split_2 split_2_object() const { return Split_2(*this); }

  /*! \class Intersect_2
   * A functor that computes the intersections of two x-monotone curves.
   */
  class Intersect_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Intersect_2(const Self& traits) : m_traits(traits) {}

    /*! computes the intersections of two given x-monotone curves and inserts
     * them into a given output iterator.
     * If the data policy guarantees that the curves are interior disjoint,
     * only their common endpoints whose data are identical are reported, and
     * the curves are not intersected geometrically. Otherwise, the intersection
     * points obtain data from the data policy, and an overlapping subcurve
     * obtains the merged data of the two curves.
     * \param xcv1 the first curve.
     * \param xcv2 the second curve.
     * \param oi the output iterator.
     * \return the past-the-end iterator.
     */
    template <typename OutputIterator>
    OutputIterator operator()(const X_monotone_curve_2& xcv1, const X_monotone_curve_2& xcv2,
                              OutputIterator oi) const {
      const auto& policy = m_traits.m_policy;
      const auto& base = m_traits.base_traits();

      if (policy.are_interior_disjoint(base, xcv1, xcv2)) {
        const auto points = m_traits.common_endpoints(xcv1, xcv2);
#if ! defined(CGAL_NO_ASSERTIONS)
        // Verify that the curves are indeed interior disjoint.
        std::list<Base_intersection_result> xections;
        base.intersect_2_object()(xcv1, xcv2, std::back_inserter(xections));
        for (const auto& xection : xections) {
          const auto* ip = std::get_if<Base_intersection_point>(&xection);
          CGAL_assertion(ip != nullptr);
          auto is_endpoint = [&](const X_monotone_curve_2& xcv) {
            for (auto ce : {ARR_MIN_END, ARR_MAX_END})
              if (m_traits.is_closed(xcv, ce) && base.equal_2_object()(ip->first, m_traits.vertex(xcv, ce)))
                return true;
            return false;
          };
          CGAL_assertion(is_endpoint(xcv1) && is_endpoint(xcv2));
        }
#endif
        for (const auto& point : points) *oi++ = point;
        return oi;
      }

      std::list<Base_intersection_result> xections;
      base.intersect_2_object()(xcv1, xcv2, std::back_inserter(xections));
      for (const auto& xection : xections) {
        if (const auto* ip = std::get_if<Base_intersection_point>(&xection)) {
          *oi++ = Intersection_point(Point_2(ip->first, policy.intersection_point_data(xcv1, xcv2)), ip->second);
          continue;
        }
        // The intersection object is an overlapping x-monotone curve: merge
        // the data of the two curves and of their coinciding endpoints.
        const auto* ovl = std::get_if<Base_x_monotone_curve_2>(&xection);
        CGAL_assertion(ovl != nullptr);
        *oi++ = X_monotone_curve_2(*ovl, policy.merge_curve_data(xcv1.data(), xcv2.data()),
                                   m_traits.overlap_end_data(*ovl, ARR_MIN_END, xcv1, xcv2),
                                   m_traits.overlap_end_data(*ovl, ARR_MAX_END, xcv1, xcv2));
      }
      return oi;
    }
  };

  /*! obtains an `Intersect_2` functor object. */
  Intersect_2 intersect_2_object() const { return Intersect_2(*this); }

  /*! \class Are_mergeable_2
   * A functor that determines whether two x-monotone curves can be merged.
   */
  class Are_mergeable_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Are_mergeable_2(const Self& traits) : m_traits(traits) {}

    /*! determines whether two given x-monotone curves can be merged, that is,
     * whether the base curves can be merged and the data of the curves are
     * equal.
     * \param xcv1 the first curve.
     * \param xcv2 the second curve.
     * \return `true` if the two curves are mergeable; `false` otherwise.
     */
    bool operator()(const X_monotone_curve_2& xcv1, const X_monotone_curve_2& xcv2) const {
      if (! m_traits.base_traits().are_mergeable_2_object()(xcv1, xcv2)) return false;
      if constexpr (Has_equal_operator<X_monotone_curve_data>::value) return xcv1.data() == xcv2.data();
      else return false;
    }
  };

  /*! obtains an `Are_mergeable_2` functor object. */
  Are_mergeable_2 are_mergeable_2_object() const { return Are_mergeable_2(*this); }

  /*! \class Merge_2
   * A functor that merges two x-monotone curves into one.
   */
  class Merge_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Merge_2(const Self& traits) : m_traits(traits) {}

    /*! merges two given x-monotone curves into a single curve.
     * \param xcv1[in] the first curve.
     * \param xcv2[in] the second curve.
     * \param xcv[out] the merged curve.
     * \pre The two curves are mergeable.
     */
    void operator()(const X_monotone_curve_2& xcv1, const X_monotone_curve_2& xcv2, X_monotone_curve_2& xcv) const {
      // Determine whether xcv1 is to the left of xcv2.
      const bool left = m_traits.is_closed(xcv1, ARR_MAX_END) && m_traits.is_closed(xcv2, ARR_MIN_END) &&
        m_traits.equal_2_object()(m_traits.construct_max_vertex_2_object()(xcv1),
                                  m_traits.construct_min_vertex_2_object()(xcv2));
      const X_monotone_curve_data data = xcv1.data();
      const Point_data min_data = left ? xcv1.end_data(ARR_MIN_END) : xcv2.end_data(ARR_MIN_END);
      const Point_data max_data = left ? xcv2.end_data(ARR_MAX_END) : xcv1.end_data(ARR_MAX_END);

      Base_x_monotone_curve_2 base_xcv;
      m_traits.base_traits().merge_2_object()(xcv1, xcv2, base_xcv);
      xcv = X_monotone_curve_2(base_xcv, data, min_data, max_data);
    }
  };

  /*! obtains a `Merge_2` functor object. */
  Merge_2 merge_2_object() const { return Merge_2(*this); }

  /*! \class Construct_x_monotone_curve_2
   * A functor that constructs an x-monotone curve connecting two points.
   */
  class Construct_x_monotone_curve_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Construct_x_monotone_curve_2(const Self& traits) : m_traits(traits) {}

    /*! obtains an x-monotone curve connecting two given points. The data of
     * the points become the data of the corresponding endpoints of the curve.
     * \param p the first point.
     * \param q the second point.
     * \param data the data of the x-monotone curve.
     * \pre `p` and `q` are not equal.
     * \return an x-monotone curve connecting `p` and `q`.
     */
    X_monotone_curve_2 operator()(const Point_2& p, const Point_2& q,
                                  const X_monotone_curve_data& data = X_monotone_curve_data()) const {
      const auto& base = m_traits.base_traits();
      Base_x_monotone_curve_2 base_xcv = base.construct_x_monotone_curve_2_object()(p, q);
      bool p_is_min;
      if constexpr (Has_compare_endpoints_xy_2<Base_traits_2>::value)
        p_is_min = (base.compare_endpoints_xy_2_object()(base_xcv) == SMALLER);
      else p_is_min = (m_traits.compare_xy_2_object()(p, q) == SMALLER);
      return p_is_min ? X_monotone_curve_2(base_xcv, data, p.data(), q.data()) :
        X_monotone_curve_2(base_xcv, data, q.data(), p.data());
    }
  };

  /*! obtains a `Construct_x_monotone_curve_2` functor object. */
  Construct_x_monotone_curve_2 construct_x_monotone_curve_2_object() const
  { return Construct_x_monotone_curve_2(*this); }

  /*! \class Construct_opposite_2
   * A functor that constructs an opposite x-monotone curve.
   */
  class Construct_opposite_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Construct_opposite_2(const Self& traits) : m_traits(traits) {}

    /*! constructs an opposite x-monotone curve (with swapped source and
     * target) with the same data.
     * \param xcv the curve.
     * \return the opposite curve.
     */
    X_monotone_curve_2 operator()(const X_monotone_curve_2& xcv) const {
      return X_monotone_curve_2(m_traits.base_traits().construct_opposite_2_object()(xcv), xcv.data(),
                                xcv.end_data(ARR_MIN_END), xcv.end_data(ARR_MAX_END));
    }
  };

  /*! obtains a `Construct_opposite_2` functor object. */
  Construct_opposite_2 construct_opposite_2_object() const { return Construct_opposite_2(*this); }

  /*! \class Merge_point_data_2
   * A functor that merges the data of coinciding points.
   */
  class Merge_point_data_2 {
  private:
    const Self& m_traits;

  public:
    /*! constructs. */
    Merge_point_data_2(const Self& traits) : m_traits(traits) {}

    /*! merges the data of two equal points.
     * \param p the first point.
     * \param q the second point.
     * \return the point `p` with the merged data of `p` and `q`.
     * \pre `p` and `q` are equal.
     */
    Point_2 operator()(const Point_2& p, const Point_2& q) const
    { return Point_2(p, m_traits.m_policy.merge_point_data(p.data(), q.data())); }

    /*! merges the data of a point with the data of an intersection point of
     * two x-monotone curves that coincides with it.
     * \param p the point.
     * \param xcv1 the first curve.
     * \param xcv2 the second curve.
     * \return the point `p` with the merged data.
     * \pre `p` lies on `xcv1` and on `xcv2`.
     */
    Point_2 operator()(const Point_2& p, const X_monotone_curve_2& xcv1, const X_monotone_curve_2& xcv2) const {
      const auto& policy = m_traits.m_policy;
      return Point_2(p, policy.merge_point_data(p.data(), policy.intersection_point_data(xcv1, xcv2)));
    }
  };

  /*! obtains a `Merge_point_data_2` functor object. */
  Merge_point_data_2 merge_point_data_2_object() const { return Merge_point_data_2(*this); }
  //@}
};

} // namespace CGAL

#include <CGAL/enable_warnings.h>

#endif
