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
//             Apurva Bhatt <response2apurva@gmail.com>

#ifndef CGAL_TYPEDEFS_H
#define CGAL_TYPEDEFS_H

#include <iostream>
#include <list>

#include <CGAL/Cartesian.h>
#include <CGAL/Arr_default_dcel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_linear_traits_2.h>
#include <CGAL/Arr_consolidated_curve_data_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>
#include <CGAL/Arrangement_with_history_2.h>
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/Arr_trapezoid_ric_point_location.h>
#include <CGAL/Arr_simple_point_location.h>
#include <CGAL/Arr_walk_along_line_point_location.h>
#include <CGAL/Arr_landmarks_point_location.h>
#include <CGAL/squared_distance_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Bbox_2.h>

#include <CGAL/Arr_observer.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/Arr_segment_traits_2.h>

#include "Qt6/BezierCurves.h"

// Linear polygons
using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;
using Pgn_with_holes_2_container = std::list<Polygon_with_holes_2>;

using Linear_traits = CGAL::Gps_traits_2<CGAL::Arr_segment_traits_2<Kernel>>;
using Linear_polygon_with_holes = Linear_traits::General_polygon_with_holes_2;
using Linear_polygon = Linear_traits::Polygon_2;
using Linear_polygon_set = CGAL::General_polygon_set_2<Linear_traits>;
using Linear_curve = Linear_traits::Curve_2;

using Linear_region_source_container = std::vector<Linear_polygon_with_holes>;
using Linear_boundary_source = std::vector<Linear_curve>;
using Linear_region_source = std::vector<Linear_boundary_source>;

// Polylines (the kernel is the same as the linear one)
using Segment_traits_2 = CGAL::Arr_segment_traits_2<Kernel>;
using Polyline_traits = CGAL::Gps_traits_2<CGAL::Arr_polyline_traits_2<Segment_traits_2>>;
using Polyline_polygon = Polyline_traits::Polygon_2;
using Polyline_polygon_set = CGAL::General_polygon_set_2<Polyline_traits>;
using Polyline_curve = Polyline_traits::Curve_2;
using Polyline_polygon_with_holes = Polyline_traits::General_polygon_with_holes_2;

using Polyline_region_source_container = std::vector<Polyline_polygon_with_holes>;
using Polyline_boundary_source = std::vector<Polyline_curve>;
using Polyline_region_source = std::vector<Polyline_boundary_source>;

// Circular polygons
#ifdef CGAL_USE_GMP
using Base_nt = CGAL::Gmpq;
#else
using Base_nt = CGAL::Quotient<CGAL::MP_Float>;
#endif

using Coord_type = CGAL::Lazy_exact_nt<Base_nt>;
struct Gps_circular_kernel : public CGAL::Cartesian<Coord_type> {};

using Circular_Linear_point = Kernel::Point_2;
using Circular_Linear_polygon = CGAL::Polygon_2<Kernel>;
using Circular_Linear_polygon_with_holes = CGAL::Polygon_with_holes_2<Kernel>;

using Circular_traits = CGAL::Gps_circle_segment_traits_2<Kernel>;
using Circular_curve = Circular_traits::Curve_2;
using Circular_X_monotone_curve = Circular_traits::X_monotone_curve_2;
using Circular_point = Circular_traits::Point_2;
using Circular_polygon = Circular_traits::Polygon_2;

using Circular_polygon_with_holes = Circular_traits::General_polygon_with_holes_2;
using Circular_polygon_set = CGAL::General_polygon_set_2<Circular_traits>;
using Circular_region_source_container = std::vector<Circular_polygon_with_holes>;

// Bezier polygons
using Bezier_nt_traits = CGAL::CORE_algebraic_number_traits;
using Bezier_rational = Bezier_nt_traits::Rational;
using Bezier_algebraic = Bezier_nt_traits::Algebraic;

struct Bezier_rat_kernel : public CGAL::Cartesian<Bezier_rational> {};
struct Bezier_alg_kernel : public CGAL::Cartesian<Bezier_algebraic> {};

struct Bezier_traits :
  public CGAL::Arr_Bezier_curve_traits_2<Bezier_rat_kernel, Bezier_alg_kernel, Bezier_nt_traits> {};

using Bezier_rat_point = Bezier_rat_kernel::Point_2;
using Bezier_curve = Bezier_traits::Curve_2;
using Bezier_X_monotone_curve = Bezier_traits::X_monotone_curve_2;
using Bezier_point = Bezier_traits::Point_2;
using Bezier_gps_traits = CGAL::Gps_traits_2<Bezier_traits>;
using Bezier_polygon = Bezier_gps_traits::General_polygon_2;
using Bezier_polygon_vector = std::vector<Bezier_polygon>;
using Bezier_polygon_with_holes = Bezier_gps_traits::General_polygon_with_holes_2;
using Bezier_polygon_set = CGAL::General_polygon_set_2<Bezier_gps_traits>;

using Bezier_GI = CGAL::Qt::Bezier_set_graphics_item<Bezier_polygon_set>;

using Bezier_boundary_source = std::vector<Bezier_curve>;
using Bezier_region_source = std::vector<Bezier_boundary_source>;
using Bezier_region_source_container = std::vector<Bezier_region_source>;

// Conic polygons
using Conic_nt_traits = CGAL::CORE_algebraic_number_traits;
using Conic_rational = Conic_nt_traits::Rational;
using Conic_algebraic = Conic_nt_traits::Algebraic;
using Conic_integer = Conic_nt_traits::Integer;

struct Conic_rat_kernel : public CGAL::Cartesian<Conic_rational> {};
struct Conic_alg_kernel : public CGAL::Cartesian<Conic_algebraic> {};

using Conic_traits = CGAL::Arr_conic_traits_2<Conic_rat_kernel, Conic_alg_kernel, Conic_nt_traits>;
using Conic_curve = Conic_traits::Curve_2;
using Conic_X_monotone_curve = Conic_traits::X_monotone_curve_2;
using Conic_point = Conic_traits::Point_2;
using Conic_gps_traits = CGAL::Gps_traits_2<Conic_traits>;
using Conic_polygon = Conic_gps_traits::General_polygon_2;
using Conic_polygon_with_holes = Conic_gps_traits::General_polygon_with_holes_2;
using Conic_polygon_set = CGAL::General_polygon_set_2<Conic_gps_traits>;

#endif
