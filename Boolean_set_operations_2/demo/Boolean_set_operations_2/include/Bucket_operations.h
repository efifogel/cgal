// Copyright (c) 2012  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s) : Efi Fogel <efifogel@gmail.com>
//             Kabir Kedia <kabirkedia0111@gmail.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>
//             Apurva Bhatt <response2apurva@gmail.com>

#ifndef CGAL_BUCKET_OPERATIONS_H
#define CGAL_BUCKET_OPERATIONS_H

/*! \file Bucket_operations.h
 * The Boolean operations on the buckets: their names in the program, and their application to sets of any curve family.
 */

#include "Curve_set.h"

// The Boolean operations on buckets.
enum class Bucket_op { BO_COMPLEMENT, BO_INTERSECTION, BO_JOIN, BO_DIFFERENCE, SYMMETRIC_DIFFERENCE };

/*! obtains the name of a Boolean operation in the program (the method of Polygon_set_2).
 */
inline const char* method_name(Bucket_op a_op) {
  switch (a_op) {
   case Bucket_op::BO_COMPLEMENT: return "complement";
   case Bucket_op::BO_INTERSECTION: return "intersection";
   case Bucket_op::BO_JOIN: return "join";
   case Bucket_op::BO_DIFFERENCE: return "difference";
   case Bucket_op::SYMMETRIC_DIFFERENCE: return "symmetric_difference";
  }
  return "";
}

/*! replaces a set with the result of a Boolean operation (other than the complement) on two sets.
 */
inline void apply(Bucket_op a_op, Curve_set& r_set, const Curve_set& a_first, const Curve_set& a_second) {
  switch (a_op) {
   case Bucket_op::BO_INTERSECTION: r_set.intersect(a_first, a_second); break;
   case Bucket_op::BO_JOIN: r_set.join(a_first, a_second); break;
   case Bucket_op::BO_DIFFERENCE: r_set.difference(a_first, a_second); break;
   case Bucket_op::SYMMETRIC_DIFFERENCE: r_set.symmetric_difference(a_first, a_second); break;
   default: break;
  }
}

/*! applies a Boolean operation (other than the complement) to a set and another set, in place.
 */
inline void apply(Bucket_op a_op, Curve_set& r_set, const Curve_set& a_other) {
  switch (a_op) {
   case Bucket_op::BO_INTERSECTION: r_set.intersect(a_other); break;
   case Bucket_op::BO_JOIN: r_set.join(a_other); break;
   case Bucket_op::BO_DIFFERENCE: r_set.difference(a_other); break;
   case Bucket_op::SYMMETRIC_DIFFERENCE: r_set.symmetric_difference(a_other); break;
   default: break;
  }
}

/*! applies a Boolean operation (other than the complement) to a linear set and a polygon, in place.
 */
inline void apply(Bucket_op a_op, Linear_polygon_set& r_set, const Linear_polygon_with_holes& a_polygon) {
  switch (a_op) {
   case Bucket_op::BO_INTERSECTION: r_set.intersection(a_polygon); break;
   case Bucket_op::BO_JOIN: r_set.join(a_polygon); break;
   case Bucket_op::BO_DIFFERENCE: r_set.difference(a_polygon); break;
   case Bucket_op::SYMMETRIC_DIFFERENCE: r_set.symmetric_difference(a_polygon); break;
   default: break;
  }
}

#endif
