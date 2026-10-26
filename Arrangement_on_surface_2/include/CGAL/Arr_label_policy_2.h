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

#ifndef CGAL_ARR_LABEL_POLICY_2_H
#define CGAL_ARR_LABEL_POLICY_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <CGAL/disable_warnings.h>

/*! \file
 * Definition of the Arr_label_policy_2<> class template.
 */

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>

#include <boost/container/small_vector.hpp>

#include <CGAL/tags.h>
#include <CGAL/enum.h>
#include <CGAL/Arr_enums.h>
#include <CGAL/Arr_data_traits_2.h>
#include <CGAL/Arr_geometry_traits/Curve_data_aux.h>

namespace CGAL {

/*! \class
 * A data policy for `Arr_data_traits_2` that attaches labels to points, and
 * user data to curves and x-monotone curves. If `MultipleLabelsTag` is
 * `Tag_true`, the data of a point is a sorted set of labels; two points whose
 * label sets share a label are guaranteed to be equal, and the label sets of
 * coinciding points are united. Otherwise, the data of a point is a single
 * (possibly empty) label; two points with the same label are guaranteed to be
 * equal, and a point that coincides with another keeps one of the two labels.
 * If `LinearCurvesTag` is `Tag_true`, the curves are linear, so two x-monotone
 * curves that have a common endpoint, whose labels are identical, and that
 * are not collinear, are guaranteed to be interior disjoint.
 */
template <typename XMonotoneCurveData,
          typename LinearCurvesTag = Tag_false,
          typename Merge_ = _Default_merge_func<XMonotoneCurveData>,
          typename CurveData_ = XMonotoneCurveData,
          typename Convert_ = _Default_convert_func<CurveData_, XMonotoneCurveData>,
          typename Label_ = std::size_t,
          typename MultipleLabelsTag = Tag_true>
class Arr_label_policy_2 {
public:
  using Label = Label_;
  using Label_set = boost::container::small_vector<Label, 2>;

  /*! a single, possibly empty, label. The maximum value of `Label` represents
   * the empty label.
   */
  class Single_label {
    Label m_label;

  public:
    /*! constructs an empty label. */
    Single_label() : m_label((std::numeric_limits<Label>::max)()) {}

    /*! constructs a label. */
    Single_label(Label label) : m_label(label) {}

    /*! determines whether the label is empty. */
    bool is_empty() const { return m_label == (std::numeric_limits<Label>::max)(); }

    /*! obtains the label. */
    Label label() const { return m_label; }

    bool operator==(const Single_label& other) const { return m_label == other.m_label; }
    bool operator!=(const Single_label& other) const { return m_label != other.m_label; }
  };

  using Multiple_labels_tag = MultipleLabelsTag;
  static constexpr bool multiple_labels = std::is_same_v<Multiple_labels_tag, Tag_true>;
  static_assert(multiple_labels || std::is_integral_v<Label>, "A single label must be of an integral type");

  using Point_data = std::conditional_t<multiple_labels, Label_set, Single_label>;
  using X_monotone_curve_data = XMonotoneCurveData;
  using Curve_data = CurveData_;
  using Merge = Merge_;
  using Convert = Convert_;
  using Linear_curves_tag = LinearCurvesTag;

  /*! determines whether two sorted label sets share a label. */
  static bool share(const Label_set& labels1, const Label_set& labels2) {
    auto it1 = labels1.begin();
    auto it2 = labels2.begin();
    while ((it1 != labels1.end()) && (it2 != labels2.end())) {
      if (*it1 == *it2) return true;
      if (*it1 < *it2) ++it1;
      else ++it2;
    }
    return false;
  }

  /*! determines whether two single labels are identical and not empty. */
  static bool share(const Single_label& label1, const Single_label& label2)
  { return ! label1.is_empty() && (label1 == label2); }

  /*! converts the data of a curve to the data of its x-monotone subcurves. */
  X_monotone_curve_data convert(const Curve_data& data) const { return Convert()(data); }

  /*! merges the data of two overlapping x-monotone curves. */
  X_monotone_curve_data merge_curve_data(const X_monotone_curve_data& data1,
                                         const X_monotone_curve_data& data2) const
  { return Merge()(data1, data2); }

  /*! merges the labels of two coinciding points: unites their label sets, or
   * keeps one of their single labels (the first, unless it is empty).
   */
  Point_data merge_point_data(const Point_data& labels1, const Point_data& labels2) const {
    if constexpr (multiple_labels) {
      if (labels2.empty() || (labels1 == labels2)) return labels1;
      if (labels1.empty()) return labels2;
      Point_data labels;
      std::set_union(labels1.begin(), labels1.end(), labels2.begin(), labels2.end(), std::back_inserter(labels));
      return labels;
    }
    else return labels1.is_empty() ? labels2 : labels1;
  }

  /*! obtains the (empty) labels of an intersection point. */
  template <typename XMonotoneCurve_2>
  Point_data intersection_point_data(const XMonotoneCurve_2& /* xcv1 */, const XMonotoneCurve_2& /* xcv2 */) const
  { return Point_data(); }

  /*! determines whether the labels of two points guarantee that the points
   * are equal, that is, whether they share a label.
   */
  bool are_identical(const Point_data& labels1, const Point_data& labels2) const { return share(labels1, labels2); }

  /*! determines whether two x-monotone curves are guaranteed to be interior
   * disjoint, that is, whether the curves are linear, have a common endpoint
   * whose labels are shared, and are not collinear.
   */
  template <typename GeometryTraits_2, typename XMonotoneCurve_2>
  bool are_interior_disjoint(const GeometryTraits_2& traits, const XMonotoneCurve_2& xcv1,
                             const XMonotoneCurve_2& xcv2) const {
    if constexpr (! std::is_same_v<Linear_curves_tag, Tag_true>) return false;
    else {
      auto vertex = [&](const XMonotoneCurve_2& xcv, Arr_curve_end ce) {
        return (ce == ARR_MIN_END) ? traits.construct_min_vertex_2_object()(xcv) :
          traits.construct_max_vertex_2_object()(xcv);
      };
      auto opposite = [](Arr_curve_end ce) { return (ce == ARR_MIN_END) ? ARR_MAX_END : ARR_MIN_END; };
      for (auto ce1 : {ARR_MIN_END, ARR_MAX_END}) {
        for (auto ce2 : {ARR_MIN_END, ARR_MAX_END}) {
          if (! share(xcv1.end_data(ce1), xcv2.end_data(ce2))) continue;
          return traits.orientation_2_object()(vertex(xcv1, opposite(ce1)), vertex(xcv1, ce1),
                                               vertex(xcv2, opposite(ce2))) != COLLINEAR;
        }
      }
      return false;
    }
  }
};

} // namespace CGAL

#include <CGAL/enable_warnings.h>

#endif
