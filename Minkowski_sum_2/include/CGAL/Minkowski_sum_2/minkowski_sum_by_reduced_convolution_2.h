// Copyright (c) 2006, 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Ron Wein   <wein_r@yahoo.com>
//             Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_MINKOWSKI_SUM_BY_REDUCED_CONVOLUTION_2_H
#define CGAL_MINKOWSKI_SUM_2_MINKOWSKI_SUM_BY_REDUCED_CONVOLUTION_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <iterator>
#include <list>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Minkowski_sum_2/Hole_filter_2.h>
#include <CGAL/Minkowski_sum_2/Minkowski_sum_reduced_conv_2.h>
#include <CGAL/Minkowski_sum_2/internal/named_parameters_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {

/*! computes the Minkowski sum \f$ P \oplus Q\f$ of two polygons using the
 * reduced convolution method: it computes the reduced convolution of the two
 * polygons and extracts those loops of the convolution that are part of the
 * Minkowski sum.  The convexity hints (the named parameters `is_convex` of
 * `np1` and `np2`) are only verified, and `np` is not used.
 * \param pgn1 the first polygon
 * \param pgn2 the second polygon
 * \param np1 the named parameters of the first polygon
 * \param np2 the named parameters of the second polygon
 * \param np the named parameters of the computation
 * \pre `pgn1` and `pgn2` are oriented counterclockwise.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_reduced_convolution_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                       const Polygon_2<Kernel_, Container_>& pgn2,
                                       const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                       const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                       const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;

  internal::convexity_hint(pgn1, np1);
  internal::convexity_hint(pgn2, np2);
  CGAL_USE(np);

  Minkowski_sum_by_reduced_convolution_2<Kernel, Container> mink_sum;
  Polygon_2<Kernel, Container> sum_bound;
  std::list<Polygon_2<Kernel, Container>> sum_holes;

  if (pgn1.size() > pgn2.size()) mink_sum(pgn1, pgn2, sum_bound, std::back_inserter(sum_holes));
  else mink_sum(pgn2, pgn1, sum_bound, std::back_inserter(sum_holes));

  return Polygon_with_holes_2<Kernel, Container>(sum_bound, sum_holes.begin(), sum_holes.end());
}

/*! computes the Minkowski sum \f$ P \oplus Q\f$ of two polygons with holes
 * using the reduced convolution method; see the overload above.  First, the
 * holes that cannot contribute to the boundary of the sum are removed.
 * \pre The outer boundaries of `pgn1` and `pgn2` (if any) are oriented
 *      counterclockwise, and their holes are oriented clockwise.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_reduced_convolution_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                       const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                       const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                       const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;

  internal::convexity_hint(pgn1, np1);
  internal::convexity_hint(pgn2, np2);
  CGAL_USE(np);

  Hole_filter_2<Kernel, Container> hole_filter;
  Polygon_with_holes_2<Kernel, Container> filtered_pgn1;
  Polygon_with_holes_2<Kernel, Container> filtered_pgn2;
  hole_filter(pgn1, pgn2, filtered_pgn1);
  hole_filter(pgn2, pgn1, filtered_pgn2);

  Minkowski_sum_by_reduced_convolution_2<Kernel, Container> mink_sum;
  Polygon_2<Kernel, Container> sum_bound;
  std::list<Polygon_2<Kernel, Container>> sum_holes;
  mink_sum(filtered_pgn1, filtered_pgn2, sum_bound, std::back_inserter(sum_holes));

  return Polygon_with_holes_2<Kernel, Container>(sum_bound, sum_holes.begin(), sum_holes.end());
}

/*! computes the Minkowski sum \f$ P \oplus Q\f$ of a polygon and a polygon
 * with holes using the reduced convolution method; see the overload above.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_reduced_convolution_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                       const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                       const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                       const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                       const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Kernel = Kernel_;
  using Container = Container_;

  internal::convexity_hint(pgn1, np1);
  internal::convexity_hint(pgn2, np2);
  CGAL_USE(np);

  Hole_filter_2<Kernel, Container> hole_filter;
  Polygon_with_holes_2<Kernel, Container> filtered_pgn2;
  hole_filter(pgn2, pgn1, filtered_pgn2);
  Minkowski_sum_by_reduced_convolution_2<Kernel, Container> mink_sum;
  Polygon_2<Kernel, Container> sum_bound;
  std::list<Polygon_2<Kernel, Container>> sum_holes;
  mink_sum(pgn1, filtered_pgn2, sum_bound, std::back_inserter(sum_holes));
  return Polygon_with_holes_2<Kernel, Container>(sum_bound, sum_holes.begin(), sum_holes.end());
}

/*! computes the Minkowski sum \f$ P \oplus Q\f$ of a polygon with holes and
 * a polygon using the reduced convolution method; see the overload above.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_reduced_convolution_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                       const Polygon_2<Kernel_, Container_>& pgn2,
                                       const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                       const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                       const CGAL_NP_CLASS& np = parameters::default_values())
{ return Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2(pgn2, pgn1, np2, np1, np); }

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
