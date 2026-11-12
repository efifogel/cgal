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
enum class Bucket_op { Complement, Intersection, Join, Difference, Symmetric_difference };

/*! obtains the name of a Boolean operation in the program (the method of Polygon_set_2).
 */
inline const char* method_name(Bucket_op aOp) {
  switch (aOp) {
   case Bucket_op::Complement: return "complement";
   case Bucket_op::Intersection: return "intersection";
   case Bucket_op::Join: return "join";
   case Bucket_op::Difference: return "difference";
   case Bucket_op::Symmetric_difference: return "symmetric_difference";
  }
  return "";
}

/*! replaces a set with the result of a Boolean operation (other than the complement) on two sets.
 */
inline void apply(Bucket_op aOp, Curve_set& rSet, const Curve_set& aFirst, const Curve_set& aSecond) {
  switch (aOp) {
   case Bucket_op::Intersection: rSet.intersect(aFirst, aSecond); break;
   case Bucket_op::Join: rSet.join(aFirst, aSecond); break;
   case Bucket_op::Difference: rSet.difference(aFirst, aSecond); break;
   case Bucket_op::Symmetric_difference: rSet.symmetric_difference(aFirst, aSecond); break;
   default: break;
  }
}

/*! applies a Boolean operation (other than the complement) to a set and another set, in place.
 */
inline void apply(Bucket_op aOp, Curve_set& rSet, const Curve_set& aOther) {
  switch (aOp) {
   case Bucket_op::Intersection: rSet.intersect(aOther); break;
   case Bucket_op::Join: rSet.join(aOther); break;
   case Bucket_op::Difference: rSet.difference(aOther); break;
   case Bucket_op::Symmetric_difference: rSet.symmetric_difference(aOther); break;
   default: break;
  }
}

/*! applies a Boolean operation (other than the complement) to a linear set and a polygon, in place.
 */
inline void apply(Bucket_op aOp, Linear_polygon_set& rSet, const Linear_polygon_with_holes& aPolygon) {
  switch (aOp) {
   case Bucket_op::Intersection: rSet.intersection(aPolygon); break;
   case Bucket_op::Join: rSet.join(aPolygon); break;
   case Bucket_op::Difference: rSet.difference(aPolygon); break;
   case Bucket_op::Symmetric_difference: rSet.symmetric_difference(aPolygon); break;
   default: break;
  }
}

#endif
