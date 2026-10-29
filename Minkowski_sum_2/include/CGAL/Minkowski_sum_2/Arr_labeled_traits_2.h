// Copyright (c) 2006  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s): Ron Wein   <wein_r@yahoo.com>

#ifndef CGAL_MINKOWSKI_SUM_2_ARR_LABELED_TRAITS_2_H
#define CGAL_MINKOWSKI_SUM_2_ARR_LABELED_TRAITS_2_H

#include <CGAL/license/Minkowski_sum_2.h>


#include <iterator>
#include <list>
#include <utility>
#include <variant>

#include <CGAL/assertions.h>
#include <CGAL/tags.h>
#include <CGAL/Minkowski_sum_2/Labels.h>

namespace CGAL {

/*! \class
 * A meta-traits class that adds labels to points and to x-monotone curves,
 * such that the comparison of two points, as well as the computation of the
 * intersections between two segments can be easily filtered.
 */
template <typename Traits_>
class Arr_labeled_traits_2 : public Traits_ {
private:
  using Base_traits_2 = Traits_;
  using Traits = Arr_labeled_traits_2<Base_traits_2>;

  using Base_point_2 = typename Base_traits_2::Point_2;
  using Base_x_monotone_curve_2 = typename Base_traits_2::X_monotone_curve_2;

public:
  using Multiplicity = typename Base_traits_2::Multiplicity;

  /*! \class
   * A point extended by a label.
   */
  class Point_2 : public Base_point_2 {
  private:
    Point_label m_label;

  public:
    /*! Default constructor. */
    Point_2() {}

    /*! Constructor from a base point. */
    Point_2(const Base_point_2& p) :
      Base_point_2(p),
      m_label()
    {}

    /*! Constructor from a point an a label. */
    Point_2(const Base_point_2& p, const Point_label& label) :
      Base_point_2(p),
      m_label(label)
    {}

    /*! Perfect forwarding.
     */
    template <typename... Args>
    Point_2(Args... args) :
      Base_point_2(std::forward<Args>(args)...),
      m_label()
    {}

    /*! obtains the label.
     */
    const Point_label& label() const { return (m_label); }
  };

  /*! \class
   * An x-monotone curve extended by a label.
   */
  class X_monotone_curve_2 : public Base_x_monotone_curve_2 {
  private:
    X_curve_label m_label;

  public:
    /*! Default constructor. */
    X_monotone_curve_2() {}

    /*! Constructor from a base x-monotone curve. */
    X_monotone_curve_2(const Base_x_monotone_curve_2& p) :
      Base_x_monotone_curve_2(p),
      m_label()
    {}

    /*! Constructor from an x-monotone curve an a label. */
    X_monotone_curve_2(const Base_x_monotone_curve_2& p, const X_curve_label& label) :
      Base_x_monotone_curve_2(p),
      m_label(label)
    {}

    /*! Get the label (const version). */
    const X_curve_label& label() const { return m_label; }

    /*! Get the label (non-const version). */
    X_curve_label& label() { return m_label; }

    /*! Set the label. */
    void set_label(const X_curve_label& label) { m_label = label; }
  };

  using Has_left_category = typename Base_traits_2::Has_left_category;
  using Has_merge_category = Tag_false;

  /*! Default constructor. */
  Arr_labeled_traits_2() {}

  // Inherited functors:
  using Is_vertical_2 = typename Base_traits_2::Is_vertical_2;
  using Compare_y_at_x_2 = typename Base_traits_2::Compare_y_at_x_2;
  using Compare_y_at_x_right_2 = typename Base_traits_2::Compare_y_at_x_right_2;
  using Equal_2 = typename Base_traits_2::Equal_2;

  /// \name Overridden functors.
  //@{
  class Compare_x_2 {
  private:
    const Base_traits_2& m_base_traits;

    /*! Constructor. */
    Compare_x_2(const Base_traits_2& base_tr) : m_base_traits(base_tr) {}

    friend Traits;

  public:
    /*! Compare the x-coordinates of two points.
     */
    Comparison_result operator()(const Point_2& p1, const Point_2& p2) const {
      // If two points have the same label, they are equal.
      if (p1.label() == p2.label()) return (EQUAL);

      return (m_base_traits.compare_x_2_object()(p1, p2));
    }
  };

  /*! Get a Compare_x_2 functor object. */
  Compare_x_2 compare_x_2_object() const { return (Compare_x_2(*this)); }

  class Compare_xy_2 {
  private:
    const Base_traits_2& m_base_traits;

    /*! Constructor. */
    Compare_xy_2(const Base_traits_2& base_tr) : m_base_traits(base_tr) {}

    friend Traits;

  public:
    /*! Compare two points lexigoraphically: by x, then by y.
     */
    Comparison_result operator()(const Point_2& p1, const Point_2& p2) const {
      // If two points have the same label, they are equal.
      if (p1.label() == p2.label()) return (EQUAL);

      return (m_base_traits.compare_xy_2_object()(p1, p2));
    }
  };

  /*! Obtain a Compare_xy_2 functor object. */
  Compare_xy_2 compare_xy_2_object() const { return Compare_xy_2(*this); }

  class Construct_min_vertex_2 {
  private:
    const Base_traits_2& m_base_traits;

    /*! Constructor. */
    Construct_min_vertex_2(const Base_traits_2& base_tr) : m_base_traits(base_tr) {}

    friend Traits;

  public:
    /*! Obtain the left endpoint of the x-monotone curve.
     */
    Point_2 operator()(const X_monotone_curve_2& cv) const {
      auto base_ctr_min_vertex = m_base_traits.construct_min_vertex_2_object();
      const Base_point_2& pt = base_ctr_min_vertex(cv);

      if ((cv.label().right_count() == 1) && (cv.label().left_count() == 0)) {
        // A curve directed from left to right:
        Point_label label(cv.label().component(), cv.label().index());

        return (Point_2 (pt, label));
      }
      else if ((cv.label().right_count() == 0) && (cv.label().left_count() == 1)) {
        // A curve directed from right to left:
        Point_label label(cv.label().component(), cv.label().is_last() ? 0 : cv.label().index()+1);

        return (Point_2(pt, label));
      }

      // Assign an invalid label to the point.
      return Point_2(pt);
    }
  };

  /*! Get a Construct_min_vertex_2 functor object. */
  Construct_min_vertex_2 construct_min_vertex_2_object () const { return Construct_min_vertex_2(*this); }

  class Construct_max_vertex_2 {
  private:
    const Base_traits_2& m_base_traits;

    /*! Constructor. */
    Construct_max_vertex_2(const Base_traits_2& base_tr) : m_base_traits(base_tr) {}

    friend Traits;

  public:
    /*! obtains the right endpoint of the x-monotone curve.
     */
    Point_2 operator()(const X_monotone_curve_2& cv) const {
      auto base_ctr_max_vertex = m_base_traits.construct_max_vertex_2_object();
      const Base_point_2& pt = base_ctr_max_vertex(cv);

      if ((cv.label().right_count() == 1) && (cv.label().left_count() == 0)) {
        // A curve directed from left to right:
        Point_label label(cv.label().component(), cv.label().is_last() ? 0 : cv.label().index()+1);
        return Point_2(pt, label);
      }
      else if ((cv.label().right_count() == 0) && (cv.label().left_count() == 1)) {
        // A curve directed from right to left:
        Point_label label(cv.label().component(), cv.label().index());
        return Point_2(pt, label);
      }

      // Assign an invalid label to the point.
      return Point_2(pt);
    }
  };

  /*! obtains a Construct_max_vertex_2 functor object. */
  Construct_max_vertex_2 construct_max_vertex_2_object () const { return Construct_max_vertex_2(*this); }

  class Split_2 {
  private:
    const Base_traits_2& m_base_traits;

    /*! Constructor. */
    Split_2(const Base_traits_2& base_tr) : m_base_traits(base_tr) {}

    friend Traits;

  public:
    /*! Split a given x-monotone curve at a given point into two sub-curves.
     */
    void operator()(const X_monotone_curve_2& cv, const Point_2& p,
                    X_monotone_curve_2& c1, X_monotone_curve_2& c2) const {
      // Split the base curve into two.
      m_base_traits.split_2_object()(cv, p, c1, c2);

      // Duplicate the label to both subcurves.
      c1.set_label(cv.label());
      c2.set_label(cv.label());
    }
  };

  /*! Get a Split_2 functor object. */
  Split_2 split_2_object() const { return Split_2(*this); }

  class Intersect_2 {
  private:
    const Base_traits_2& m_base_traits;

  public:
    /*! Constructor. */
    Intersect_2(const Base_traits_2& traits) : m_base_traits(traits) {}

    /*! Find the intersections of the two given curves and insert them to the
     * given output iterator.
     */
    template <typename OutputIterator>
    OutputIterator operator()(const X_monotone_curve_2& cv1, const X_monotone_curve_2& cv2, OutputIterator oi) const {
      using Intersection_base_point = std::pair<Base_point_2, Multiplicity>;
      using Intersection_base_result = std::variant<Intersection_base_point, Base_x_monotone_curve_2>;

      // In case the curves are adjacent in their curve sequence, we do
      // not have to compute their intersection (we already know that they
      // have just one common endpoint).
      if (cv1.label().is_adjacent(cv2.label())) return oi;

      // Compute the intersection.
      std::list<Intersection_base_result> xections;
      m_base_traits.intersect_2_object()(cv1, cv2, std::back_inserter(xections));

      if (xections.empty()) return oi;

      // Attach labels to the intersection objects.
      for (const auto& xection : xections) {
        const Intersection_base_point* base_pt = std::get_if<Intersection_base_point>(&xection);

        if (base_pt != nullptr) {
          // Attach an invalid label to an itersection point.
          *oi++ = std::make_pair(Point_2(base_pt->first), base_pt->second);
          continue;
        }

        const Base_x_monotone_curve_2* base_xcv = std::get_if<Base_x_monotone_curve_2>(&xection);
        CGAL_assertion(base_xcv != nullptr);

        // Attach a merged label to the overlapping curve.
        *oi++ = X_monotone_curve_2(*base_xcv, X_curve_label(cv1.label(), cv2.label()));
      }

      return oi;
    }
  };

  /*! obtains an Intersect_2 functor object. */
  Intersect_2 intersect_2_object() const { return Intersect_2(*this); }

  //@}
};

} //namespace CGAL

#endif
