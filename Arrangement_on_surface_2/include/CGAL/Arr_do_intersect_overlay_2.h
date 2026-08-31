// Copyright (c) 2025 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s):      Efi Fogel <efifogel@gmail.com>

#ifndef CGAL_ARR_DO_INTERSECT_OVERLAY_2_H
#define CGAL_ARR_DO_INTERSECT_OVERLAY_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <CGAL/disable_warnings.h>

/*! \file
 *
 * Definition of the global do_intersect_overlay_2() function.
 */

#include <CGAL/Arrangement_on_surface_2.h>
#include <CGAL/No_intersection_surface_sweep_2.h>
#include <CGAL/Surface_sweep_2/Arr_overlay_traits_2.h>
#include <CGAL/Surface_sweep_2/Arr_do_intersect_ss_visitor.h>
#include <CGAL/Surface_sweep_2/Arr_overlay_event.h>
#include <CGAL/Surface_sweep_2/Arr_overlay_subcurve.h>
#include <CGAL/assertions.h>

namespace CGAL {

template <typename Bbox>
inline bool do_regularized_overlap(const Bbox& b1, const Bbox& b2)
{ return (b1.xmin() < b2.xmax()) && (b1.xmax() > b2.xmin()) && (b1.ymin() < b2.ymax()) && (b1.ymax() > b2.ymin()); }

/*! determines whether two input arrangements intersect using the surface-sweeper.
 * with an early abort if an intersection is detected.
 * \tparam GeometryTraitsA_2 the geometry traits of the first arrangement.
 * \tparam GeometryTraitsB_2 the geometry traits of the second arrangement.
 * \tparam TopologyTraitsA the topology traits of the first arrangement.
 * \tparam TopologyTraitsB the topology traits of the second arrangement.
 * \tparam Visitor An overlay-visitor class. The operand arrangements
 *         `arr1` and `arr2` can be templated with different geometry-traits
 *         class and different DCELs (encapsulated in the various
 *         topology-traits classes). The overlay-visitor class defines the
 *         various overlay operations of pairs of DCEL features from
 *         TopologyTraitsA and TopologyTraitsB.
 */
template <typename GeometryTraitsA_2, typename GeometryTraitsB_2, typename TopologyTraitsA, typename TopologyTraitsB>
bool do_intersect_overlay(const Arrangement_on_surface_2<GeometryTraitsA_2, TopologyTraitsA>& arr1,
                          const Arrangement_on_surface_2<GeometryTraitsB_2, TopologyTraitsB>& arr2) {
  // // Quick bounding box rejection check
  // if (! arr1.is_empty() && ! arr2.is_empty()) {
  //   auto bbox1 = arr1.bbox();
  //   auto bbox2 = arr2.bbox();
  //   if (! CGAL::do_regularized_overlap(bbox1, bbox2)) {
  //     // If bounding boxes do not overlap, intersection is only possible if one has an unbounded solid face
  //     auto unf1 = arr1.unbounded_face();
  //     auto unf2 = arr2.unbounded_face();
  //     if (! unf1->data() && ! unf2->data()) return false;
  //   }
  // }

  using Agt2 = GeometryTraitsA_2;
  using Bgt2 = GeometryTraitsB_2;
  using Att = TopologyTraitsA;
  using Btt = TopologyTraitsB;
  using Arr_a = Arrangement_on_surface_2<Agt2, Att>;
  using Arr_b = Arrangement_on_surface_2<Bgt2, Btt>;
  using Allocator = typename Arr_a::Allocator;

  // Some type assertions (not all, but better than nothing).
  using A_point = typename Agt2::Point_2;
  using B_point = typename Bgt2::Point_2;
  using A_xcv = typename Agt2::X_monotone_curve_2;
  using B_xcv = typename Bgt2::X_monotone_curve_2;

  using Gt_adaptor_2 = Arr_traits_basic_adaptor_2<Agt2>;
  using Ovl_gt2 = Arr_overlay_traits_2<Gt_adaptor_2, Arr_a, Arr_b>;
  using Ovl_event = Arr_overlay_event<Ovl_gt2, Arr_a, Allocator>;
  using Ovl_curve = Arr_overlay_subcurve<Ovl_gt2, Ovl_event, Allocator>;
  using Helper = typename TopologyTraitsA::template Overlay_helper<Ovl_gt2, Ovl_event, Ovl_curve, Arr_a, Arr_b>;
  using Do_intersect_ss_visitor = Arr_do_intersect_ss_visitor<Helper, Allocator>;

  using Ovl_x_monotone_curve_2 = typename Ovl_gt2::X_monotone_curve_2;

  // Prepare a vector of extended x-monotone curves that represent all edges
  // in both input arrangements. Each curve is associated with a halfedge
  // directed from right to left.
  typename Arr_a::Halfedge_const_handle invalid_he1;
  typename Arr_b::Halfedge_const_handle invalid_he2;
  std::vector<Ovl_x_monotone_curve_2> xcvs;
  xcvs.reserve(arr1.number_of_edges() + arr2.number_of_edges());
  for (auto eit1 = arr1.edges_begin(); eit1 != arr1.edges_end(); ++eit1) {
    typename Arr_a::Halfedge_const_handle he1 = eit1;
    if (he1->direction() != ARR_RIGHT_TO_LEFT) he1 = he1->twin();
    xcvs.emplace_back(eit1->curve(), he1, invalid_he2);
  }
  for (auto eit2 = arr2.edges_begin(); eit2 != arr2.edges_end(); ++eit2) {
    typename Arr_b::Halfedge_const_handle he2 = eit2;
    if (he2->direction() != ARR_RIGHT_TO_LEFT) he2 = he2->twin();
    xcvs.emplace_back(eit2->curve(), invalid_he1, he2);
  }

  // Obtain an extended traits-class object and define the sweep-line visitor.
  const auto* traits_adaptor = arr1.traits_adaptor();

  /* We would like to avoid copy construction of the geometry traits class.
   * Copy construction is undesired, because it may results with data
   * duplication or even data loss.
   *
   * If the type Ovl_gt2 is the same as the type
   * GeomTraits, use a reference to GeomTraits to avoid constructing a new one.
   * Otherwise, instantiate a local variable of the former and provide
   * the latter as a single parameter to the constructor.
   *
   * Use the form 'A a(*b);' and not ''A a = b;' to handle the case where A has
   * only an implicit constructor, (which takes *b as a parameter).
   */
  std::conditional_t<std::is_same_v<Gt_adaptor_2, Ovl_gt2>, const Ovl_gt2&, Ovl_gt2> ex_traits(*traits_adaptor);

  Do_intersect_ss_visitor visitor(&arr1, &arr2);
  Ss2::No_intersection_surface_sweep_2<Do_intersect_ss_visitor> sweeper(&ex_traits, &visitor);

  sweeper.sweep(xcvs.begin(), xcvs.end());
  xcvs.clear();
  return visitor.do_intersect();
}

} // namespace CGAL

#include <CGAL/enable_warnings.h>

#endif
