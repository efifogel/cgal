// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_INTERNAL_POLYGON_TYPE_2_H
#define CGAL_MINKOWSKI_SUM_2_INTERNAL_POLYGON_TYPE_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <type_traits>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>

// The traits below are used by the deprecated functions of the namespace CGAL
// (declared in <CGAL/minkowski_sum_2.h>, <CGAL/approximated_offset_2.h>, and
// <CGAL/offset_polygon_2.h>), which accept any type as a polygon parameter.
// They restrict these functions to the polygon types they used to accept: the
// type aliases below are well formed only for these types, so when they are
// used in the return type of a function, the function is removed from the set
// of candidates of a call with other types (SFINAE). See the comment at the
// top of these header files for the reason.

namespace CGAL {
namespace Minkowski_sum_2 {
namespace internal {

/*! the properties of a polygon type; empty if the type is not a polygon.
 */
template <typename T>
struct Polygon_type_2 {};

/*! the properties of a polygon.
 */
template <typename Kernel_, typename Container_>
struct Polygon_type_2<CGAL::Polygon_2<Kernel_, Container_>> {
  using Kernel = Kernel_;
  using Container = Container_;
  using Polygon_with_holes = CGAL::Polygon_with_holes_2<Kernel_, Container_>;
  static constexpr bool has_holes = false;
};

/*! the properties of a polygon with holes.
 */
template <typename Kernel_, typename Container_>
struct Polygon_type_2<CGAL::Polygon_with_holes_2<Kernel_, Container_>> {
  using Kernel = Kernel_;
  using Container = Container_;
  using Polygon_with_holes = CGAL::Polygon_with_holes_2<Kernel_, Container_>;
  static constexpr bool has_holes = true;
};

/*! the kernel of a polygon (with holes).
 */
template <typename Polygon>
using Polygon_kernel_t = typename Polygon_type_2<Polygon>::Kernel;

/*! the kernel of a polygon without holes.
 */
template <typename Polygon>
using Simple_polygon_kernel_t = std::enable_if_t<! Polygon_type_2<Polygon>::has_holes,
                                                 typename Polygon_type_2<Polygon>::Kernel>;

/*! the type of the Minkowski sum of two polygons (with holes) that have the
 * same kernel and container.
 */
template <typename Polygon1, typename Polygon2>
using Sum_type_t = std::enable_if_t<std::is_same_v<typename Polygon_type_2<Polygon1>::Polygon_with_holes,
                                                   typename Polygon_type_2<Polygon2>::Polygon_with_holes>,
                                    typename Polygon_type_2<Polygon1>::Polygon_with_holes>;

/*! the type of the Minkowski sum of two polygons without holes that have the
 * same kernel and container.
 */
template <typename Polygon1, typename Polygon2>
using Simple_sum_type_t = std::enable_if_t<! Polygon_type_2<Polygon1>::has_holes &&
                                           ! Polygon_type_2<Polygon2>::has_holes,
                                           Sum_type_t<Polygon1, Polygon2>>;

} // namespace internal
} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
