// Copyright (c) 1997  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Baruch Zukerman <baruchzu@post.tau.ac.il>
//             Efi Fogel       <efifogel@gmail.com>

#ifndef CGAL_GPS_AGG_META_TRAITS_H
#define CGAL_GPS_AGG_META_TRAITS_H

#include <CGAL/license/Boolean_set_operations_2.h>

#include <vector>
#include <variant>
#include <utility>
#include <type_traits>

#include <CGAL/Arr_enums.h>
#include <CGAL/Arr_tags.h>
#include <CGAL/Boolean_set_operations_2/Gps_traits_decorator.h>
#include <CGAL/Boolean_set_operations_2/Curve_with_halfedge.h>
#include <CGAL/Boolean_set_operations_2/Point_with_vertex.h>

namespace CGAL {

template <typename Arrangement_>
class Gps_agg_curve_data : public Curve_with_halfedge<Arrangement_> {
protected:
  using Arrangement = Arrangement_;
  using Halfedge_handle = typename Arrangement::Halfedge_handle;
  using Base = Curve_with_halfedge<Arrangement_>;

  const Arrangement* m_arr; // pointer to the arrangement containing the edge.
  std::size_t m_bc;         // the boundary counter of the halfedge with the same
                            // direction as the curve

  std::size_t m_twin_bc;    // the boundary counter of the halfedge with the
                            // opposite direction to the curve

public:
  Gps_agg_curve_data() :
    Base(),
    m_arr(nullptr),
    m_bc(0),
    m_twin_bc(0)
  {}

  Gps_agg_curve_data(const Arrangement* arr, Halfedge_handle he, std::size_t bc, std::size_t twin_bc) :
    Base(he),
    m_arr(arr),
    m_bc(bc),
    m_twin_bc(twin_bc)
  {}

  std::size_t bc() const { return m_bc; }

  std::size_t twin_bc() const { return m_twin_bc; }

  std::size_t& bc() { return m_bc; }

  std::size_t& twin_bc() { return m_twin_bc; }

  void set_bc(std::size_t bc) { m_bc = bc; }

  void set_twin_bc(std::size_t twin_bc) { m_twin_bc = twin_bc; }

  const Arrangement* arr() const { return m_arr; }
};

template <typename Arrangement_>
class Gps_agg_meta_traits :
  public Gps_traits_decorator<typename Arrangement_::Traits_adaptor_2, Gps_agg_curve_data<Arrangement_>,
                              Point_with_vertex<Arrangement_>> {
  using Arrangement = Arrangement_;
  using Arr = Arrangement;

  using Gt2 = typename Arr::Traits_adaptor_2;

  using Base_x_monotone_curve_2 = typename Gt2::X_monotone_curve_2;
  using Base_point_2 = typename Gt2::Point_2;
  using Base_Construct_min_vertex_2 = typename Gt2::Construct_min_vertex_2;
  using Base_Construct_max_vertex_2 = typename Gt2::Construct_max_vertex_2;
  using Base_Compare_xy_2 = typename Gt2::Compare_xy_2;
  using Base_Split_2 = typename Gt2::Split_2;

public:
  using Multiplicity = typename Gt2::Multiplicity;
  using Curve_data = Gps_agg_curve_data<Arr>;
  using Point_data = Point_with_vertex<Arr>;

private:
  using Self = Gps_agg_meta_traits<Arrangement>;
  using Base = Gps_traits_decorator<Gt2, Curve_data, Point_data>;

public:
  using X_monotone_curve_2 = typename Base::X_monotone_curve_2;
  using Point_2 = typename Base::Point_2;
  using Has_left_category = typename Gt2::Has_left_category;
  using Has_merge_category = typename Gt2::Has_merge_category;

  using Left_side_category = typename Arr::Left_side_category;
  using Bottom_side_category = typename Arr::Bottom_side_category;
  using Top_side_category = typename Arr::Top_side_category;
  using Right_side_category = typename Arr::Right_side_category;

  using Halfedge_handle = typename Arr::Halfedge_handle;
  using Vertex_handle = typename Arr::Vertex_handle;

  Gps_agg_meta_traits() = default;

  Gps_agg_meta_traits(const Gt2& base_tr) : Base(base_tr) {}

  class Intersect_2 {
  private:
    const Self& m_traits;

    /*! constructs. */
    explicit Intersect_2(const Self& traits) : m_traits(traits) {}

    friend Self;

    /*! determines whether the minimal end of a curve lies in the interior of
     * the parameter space. Only open sides are checked, as on oblivious sides
     * every curve end is interior.
     */
    bool is_min_end_interior(const X_monotone_curve_2& xcv) const {
      const auto* base_traits = m_traits.m_base_traits.get();
      if constexpr (! std::is_same_v<Left_side_category, Arr_oblivious_side_tag>) {
        auto ps_in_x = base_traits->parameter_space_in_x_2_object();
        if (ps_in_x(xcv.base(), ARR_MIN_END) != ARR_INTERIOR) return false;
      }
      if constexpr (! std::is_same_v<Bottom_side_category, Arr_oblivious_side_tag> ||
                    ! std::is_same_v<Top_side_category, Arr_oblivious_side_tag>) {
        auto ps_in_y = base_traits->parameter_space_in_y_2_object();
        if (ps_in_y(xcv.base(), ARR_MIN_END) != ARR_INTERIOR) return false;
      }
      return true;
    }

  public:
    template <typename OutputIterator>
    OutputIterator operator()(const X_monotone_curve_2& cv1, const X_monotone_curve_2& cv2, OutputIterator oi) const {
      // Check whether the curves are already in the same arrangement, and thus
      // must be interior-disjoint
      if (cv1.data().arr() == cv2.data().arr()) return oi;

      using Intersection_base_point = std::pair<Base_point_2, Multiplicity>;
      using Intersection_base_result = std::variant<Intersection_base_point, Base_x_monotone_curve_2>;

      const auto* base_traits = m_traits.m_base_traits.get();
      auto base_cmp_xy = base_traits->compare_xy_2_object();
      auto base_cmp_endpoints = base_traits->compare_endpoints_xy_2_object();
      auto base_ctr_min_vertex = base_traits->construct_min_vertex_2_object();
      auto base_intersect = base_traits->intersect_2_object();

      // The curves are ordered by their left endpoints; thus, these must exist.
      CGAL_precondition(is_min_end_interior(cv1) && is_min_end_interior(cv2));
      std::vector<Intersection_base_result> xections;
      if (base_cmp_xy(base_ctr_min_vertex(cv1.base()), base_ctr_min_vertex(cv2.base())) == LARGER)
        base_intersect(cv1.base(), cv2.base(), std::back_inserter(xections));
      else
        base_intersect(cv2.base(), cv1.base(), std::back_inserter(xections));

      // Convert objects that are associated with Base_x_monotone_curve_2 to
      // the extenede X_monotone_curve_2
      for (const auto& xection : xections) {
        const Intersection_base_point* base_pt =
          std::get_if<Intersection_base_point>(&xection);
        if (base_pt != nullptr) {
          Point_2 point_plus(base_pt->first); // the extended point
          *oi++ = std::make_pair(point_plus, base_pt->second);
          continue;
        }

        const Base_x_monotone_curve_2* overlap_cv = std::get_if<Base_x_monotone_curve_2>(&xection);
        CGAL_assertion(overlap_cv != nullptr);
        std::size_t ov_bc;
        std::size_t ov_twin_bc;
        if (base_cmp_endpoints(cv1.base()) == base_cmp_endpoints(cv2.base())) {
          // cv1 and cv2 have the same directions
          ov_bc = cv1.data().bc() + cv2.data().bc();
          ov_twin_bc = cv1.data().twin_bc() + cv2.data().twin_bc();
        }
        else {
          // cv1 and cv2 have opposite directions
          ov_bc = cv1.data().bc() + cv2.data().twin_bc();
          ov_twin_bc = cv1.data().twin_bc() + cv2.data().bc();
        }

        if (base_cmp_endpoints(*overlap_cv) != base_cmp_endpoints(cv1.base())) {
          // overlap_cv, cv1 have opposite directions
          std::swap(ov_bc, ov_twin_bc);
        }

        Curve_data cv_data(cv1.data().arr(), Halfedge_handle(), ov_bc, ov_twin_bc);
        *oi++ = X_monotone_curve_2(*overlap_cv, cv_data);
      }

      return oi;
    }
  };

  /*! obtains an Intersect_2 functor object. */
  Intersect_2 intersect_2_object() const { return Intersect_2(*this); }

  class Split_2 {
  private:
    Base_Split_2 m_base_split;

  public:
    /*! constructs. */
    explicit Split_2(const Base_Split_2& base) : m_base_split(base) {}

    void operator()(const X_monotone_curve_2& cv, const Point_2 & p,
                    X_monotone_curve_2& c1, X_monotone_curve_2& c2) const {
      m_base_split(cv.base(), p.base(), c1.base(), c2.base());
      const Curve_data& cv_data = cv.data();
      Curve_data data(cv_data.arr(), Halfedge_handle(), cv_data.bc(), cv_data.twin_bc());
      c1.set_data(data);
      c2.set_data(data);
    }
  };

  /*! obtains a Split_2 functor object. */
  Split_2 split_2_object() const { return Split_2(this->m_base_traits->split_2_object()); }

  class Construct_min_vertex_2 {
  private:
    Base_Construct_min_vertex_2 m_base;

  public:
    explicit Construct_min_vertex_2(const Base_Construct_min_vertex_2& base) : m_base(base) {}

    /*! obtains the left endpoint of the x-monotone curve (segment).
     * \param cv The curve.
     * \return The left endpoint.
     */
    Point_2 operator()(const X_monotone_curve_2 & cv) const {
      if (cv.data().halfedge() == Halfedge_handle()) return Point_2(m_base(cv.base()));

      CGAL_assertion((Arr_halfedge_direction)cv.data().halfedge()->direction() == ARR_LEFT_TO_RIGHT);
      return Point_2(m_base(cv.base()), cv.data().halfedge()->source());
    }
  };

  /*! obtains a Construct_min_vertex_2 functor object. */
  Construct_min_vertex_2 construct_min_vertex_2_object() const
  { return Construct_min_vertex_2(this->m_base_traits->construct_min_vertex_2_object()); }

  class Construct_max_vertex_2 {
  private:
    Base_Construct_max_vertex_2 m_base;

  public:
    explicit Construct_max_vertex_2(const Base_Construct_max_vertex_2& base) : m_base(base) {}

    /*! obtains the right endpoint of the x-monotone curve (segment).
     * \param cv The curve.
     * \return The right endpoint.
     */
    Point_2 operator()(const X_monotone_curve_2& cv) const {
      if (cv.data().halfedge() == Halfedge_handle()) return Point_2(m_base(cv.base()));

      CGAL_assertion((Arr_halfedge_direction)cv.data().halfedge()->direction() == ARR_LEFT_TO_RIGHT);
      return Point_2(m_base(cv.base()), cv.data().halfedge()->target());
    }
  };

  /*! obtains a Construct_max_vertex_2 functor object. */
  Construct_max_vertex_2 construct_max_vertex_2_object() const
  { return Construct_max_vertex_2(this->m_base_traits->construct_max_vertex_2_object()); }

  class Compare_xy_2 {
  private:
    Base_Compare_xy_2 m_base;

  public:
    explicit Compare_xy_2(const Base_Compare_xy_2& base) : m_base(base) {}

    /*! compares two points lexicographically: first by x, then by y.
     * Points associated with the same arrangement vertex are equal.
     * \param p1 The first point.
     * \param p2 The second point.
     * \return The comparison result.
     */
    Comparison_result operator()(const Point_2& p1, const Point_2& p2) const {
      const Point_data& inf1 = p1.data();
      const Point_data& inf2 = p2.data();
      if ((inf1.vertex() != Vertex_handle()) && (inf1.vertex() == inf2.vertex())) return EQUAL;
      return m_base(p1.base(), p2.base());
    }
  };

  /*! obtains a Compare_xy_2 functor object. */
  Compare_xy_2 compare_xy_2_object() const { return Compare_xy_2(this->m_base_traits->compare_xy_2_object()); }
};

} // namespace CGAL

#endif
