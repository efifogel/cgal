// Copyright (c) 2026  Tel-Aviv University (Israel).
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

#ifndef CGAL_ALGEBRAIC_POLYGONS_H
#define CGAL_ALGEBRAIC_POLYGONS_H

/*! \file Algebraic_polygons.h
 * The polygons bounded by algebraic curves, and the approximation of their arcs for drawing.
 */

#include <vector>

#include <QPointF>

#include <CGAL/Bbox_2.h>

#include "Typedefs.h"
#include "Algebraic_curves.h"

/*! obtains the polygons bounded by the curve of a polynomial: the boundaries of the bounded faces of the arrangement
 * of the curve, in counterclockwise order. Their union is the region the curve encloses (e.g., the disk of a circle,
 * or the outer disk of two concentric circles); unbounded parts of the curve, isolated points, and arcs that have the
 * same face on both sides are ignored.
 */
std::vector<Algebraic_polygon> algebraic_polygons(const Bivariate_polynomial& a_polynomial);

/*! obtains points along an x-monotone arc, from its source to its target, which approximate it for drawing.
 */
std::vector<QPointF> algebraic_arc_points(const Algebraic_X_monotone_curve& a_xcv);

/*! obtains the bounding box of an x-monotone arc.
 */
CGAL::Bbox_2 algebraic_arc_bbox(const Algebraic_X_monotone_curve& a_xcv);

#endif
