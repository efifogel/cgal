// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_INTERNAL_NAMED_PARAMETERS_2_H
#define CGAL_MINKOWSKI_SUM_2_INTERNAL_NAMED_PARAMETERS_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <memory>
#include <optional>
#include <type_traits>

#include <CGAL/assertions.h>
#include <CGAL/use.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {

/*! The method that computes the Minkowski sum of two polygons, or the offset
 * of a polygon; see `minkowski_sum_2()`, `approximated_offset_2()`, and
 * `offset_polygon_2()`.
 */
enum class Method {
  AUTOMATIC,            //!< chosen automatically
  DECOMPOSITION,        //!< the decomposition method
  REDUCED_CONVOLUTION,  //!< the reduced convolution method
  FULL_CONVOLUTION      //!< the full convolution method
};

namespace internal {

/*! determines whether a type is a shared pointer.
 */
template <typename T> struct Is_shared_ptr : std::false_type {};
template <typename T> struct Is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

/*! obtains a shared pointer to an object given by a named parameter (e.g.,
 * the geometry traits or a decomposition strategy), or to a default-constructed
 * instance of `Default_`, if the parameter is not given.  The parameter is
 * either
 * - a shared pointer to the object, which is returned as is,
 * - a reference to the object (wrapped by `std::cref()`, or of a type that is
 *   not copyable), which is wrapped by a non-owning shared pointer, as the
 *   referenced object outlives the call, or
 * - the object, which is copied.
 * (The named parameters store a copy of a copyable parameter, but they
 * provide access only to further copies of it.)
 */
template <typename Default_, typename NamedParameters, typename Tag>
auto shared_parameter(const NamedParameters& np, Tag tag) {
  using parameters::get_parameter;
  using parameters::get_parameter_reference;

  if constexpr (parameters::is_default_parameter<NamedParameters, Tag>::value)
    return std::shared_ptr<const Default_>(std::make_shared<Default_>());
  else {
    using Reference = decltype(get_parameter_reference(np, tag));
    using Value = std::remove_cv_t<std::remove_reference_t<Reference>>;
    if constexpr (Is_shared_ptr<Value>::value) {
      std::shared_ptr<const typename Value::element_type> object = get_parameter(np, tag);
      CGAL_precondition(object != nullptr);
      return object;
    }
    else if constexpr (std::is_reference_v<Reference>) {
      Reference object = get_parameter_reference(np, tag);
      return std::shared_ptr<const Value>(std::shared_ptr<void>(), &object);
    }
    else return std::shared_ptr<const Value>(std::make_shared<Value>(get_parameter(np, tag)));
  }
}

/*! obtains a shared pointer to the geometry traits, given by the named
 * parameter `geom_traits`, or to default traits, namely, a default-constructed
 * instance of `Gps_segment_traits_2`; see `shared_parameter()`.
 */
template <typename Kernel_, typename Container_, typename NamedParameters>
auto shared_traits(const NamedParameters& np)
{ return shared_parameter<Gps_segment_traits_2<Kernel_, Container_>>(np, internal_np::geom_traits); }

/*! obtains the hint, given by the named parameter `is_convex`, whether a
 * polygon is convex, if given.  A hint that the polygon is convex is verified
 * if expensive checks are enabled.
 */
template <typename Kernel_, typename Container_, typename NamedParameters>
std::optional<bool> convexity_hint(const Polygon_with_holes_2<Kernel_, Container_>& pwh,
                                   const NamedParameters& np) {
  if constexpr (parameters::is_default_parameter<NamedParameters, internal_np::is_convex_t>::value) {
    CGAL_USE(pwh);
    return std::nullopt;
  }
  else {
    const bool convex = parameters::get_parameter(np, internal_np::is_convex);
    CGAL_expensive_precondition(! convex ||
                                (! pwh.is_unbounded() && (pwh.number_of_holes() == 0) &&
                                 pwh.outer_boundary().is_convex()));
    return convex;
  }
}

/*! determines whether a polygon with holes is convex, that is, whether it is
 * bounded, has no holes, and its outer boundary is convex: as hinted, if a
 * hint is given, or by testing it otherwise.
 */
template <typename Kernel_, typename Container_>
bool is_convex_polygon(const Polygon_with_holes_2<Kernel_, Container_>& pwh, std::optional<bool> hint) {
  if (hint) return *hint;
  return ! pwh.is_unbounded() && (pwh.number_of_holes() == 0) && pwh.outer_boundary().is_convex();
}

/*! obtains the hint whether a simple polygon is convex; see the overload
 * above.
 */
template <typename Kernel_, typename Container_, typename NamedParameters>
std::optional<bool> convexity_hint(const Polygon_2<Kernel_, Container_>& pgn, const NamedParameters& np) {
  if constexpr (parameters::is_default_parameter<NamedParameters, internal_np::is_convex_t>::value) {
    CGAL_USE(pgn);
    return std::nullopt;
  }
  else {
    const bool convex = parameters::get_parameter(np, internal_np::is_convex);
    CGAL_expensive_precondition(! convex || pgn.is_convex());
    return convex;
  }
}

/*! determines whether a simple polygon is convex; see the overload above.
 */
template <typename Kernel_, typename Container_>
bool is_convex_polygon(const Polygon_2<Kernel_, Container_>& pgn, std::optional<bool> hint)
{ return hint ? *hint : pgn.is_convex(); }

/*! determines whether a polygon (with holes) is unbounded.
 */
template <typename Kernel_, typename Container_>
bool is_unbounded_polygon(const Polygon_with_holes_2<Kernel_, Container_>& pwh) { return pwh.is_unbounded(); }

template <typename Kernel_, typename Container_>
bool is_unbounded_polygon(const Polygon_2<Kernel_, Container_>&) { return false; }

} // namespace internal

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
