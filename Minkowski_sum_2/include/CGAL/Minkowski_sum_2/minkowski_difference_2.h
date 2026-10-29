// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_MINKOWSKI_DIFFERENCE_2_H
#define CGAL_MINKOWSKI_SUM_2_MINKOWSKI_DIFFERENCE_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_difference_by_full_convolution_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {

/*! computes the Minkowski difference \f$ P \ominus Q\f$ of two polygons with
 * holes, the complement of the Minkowski sum of \f$ P\f$ and the complement of
 * \f$ Q\f$.  It calls `minkowski_difference_by_full_convolution_2()`.
 * \param pgn1 the polygon with holes \f$ P\f$
 * \param pgn2 the polygon with holes \f$ Q\f$
 * \param oi the output iterator for the polygons with holes of the
 *        difference, which may be empty or disconnected
 * \param np1 the named parameters of the first polygon
 * \param np2 the named parameters of the second polygon
 * \param np the named parameters of the computation
 * \return the past-the-end iterator of the output
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator minkowski_difference_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                      const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                      OutputIterator oi,
                                      const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                      const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                      const CGAL_NP_CLASS& np = parameters::default_values())
{ return minkowski_difference_by_full_convolution_2(pgn1, pgn2, oi, np1, np2, np); }

/*! computes the Minkowski difference \f$ P \ominus Q\f$ of a polygon and a
 * polygon with holes; see the overload above.
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator minkowski_difference_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                      const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                      OutputIterator oi,
                                      const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                      const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                      const CGAL_NP_CLASS& np = parameters::default_values())
{ return minkowski_difference_by_full_convolution_2(pgn1, pgn2, oi, np1, np2, np); }

/*! computes the Minkowski difference \f$ P \ominus Q\f$ of a polygon with
 * holes and a polygon; see the overload above.
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator minkowski_difference_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                      const Polygon_2<Kernel_, Container_>& pgn2,
                                      OutputIterator oi,
                                      const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                      const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                      const CGAL_NP_CLASS& np = parameters::default_values())
{ return minkowski_difference_by_full_convolution_2(pgn1, pgn2, oi, np1, np2, np); }

/*! computes the Minkowski difference \f$ P \ominus Q\f$ of two simple
 * polygons; see the overload above.
 */
template <typename Kernel_, typename Container_, typename OutputIterator,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
OutputIterator minkowski_difference_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                      const Polygon_2<Kernel_, Container_>& pgn2,
                                      OutputIterator oi,
                                      const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                      const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                      const CGAL_NP_CLASS& np = parameters::default_values())
{ return minkowski_difference_by_full_convolution_2(pgn1, pgn2, oi, np1, np2, np); }

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
