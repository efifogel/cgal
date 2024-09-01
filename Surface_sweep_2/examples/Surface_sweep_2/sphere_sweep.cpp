//! \file examples/Surface_sweep_2/plane_sweep.cpp
// Computing intersection points among curves using the surface-sweep alg.

#include <list>
#include <cassert>
#include <iostream>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_geodesic_arc_on_sphere_traits_2.h>
#include <CGAL/Arr_spherical_topology_traits_2.h>
#include <CGAL/Surface_sweep_2_algorithms.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Traits = CGAL::Arr_geodesic_arc_on_sphere_traits_2<Kernel>;
using Point = Traits::Point_2;
using X_monotone_curve = Traits::X_monotone_curve_2;
using Curve = Traits::Curve_2;

int main() {
  Traits traits;
  auto ctr_pnt = traits.construct_point_2_object();
  auto ctr_cv = traits.construct_curve_2_object();

  // Construct the input curves.
  Curve curves[] = {
    ctr_cv(ctr_pnt(1, 1, 0), ctr_pnt(1, -1, 0)),
    ctr_cv(ctr_pnt(1, -1, 1), ctr_pnt(1, -1, -1)),
    ctr_cv(ctr_pnt(1, 0, 1), ctr_pnt(1, 0, -1)),
    ctr_cv(ctr_pnt(1, 1, 1), ctr_pnt(1, 1, -1))
  };
  constexpr auto num_curves = sizeof(curves)/sizeof(Curve);

  // Compute all intersection points.
  std::list<Point> pts;
  CGAL::compute_intersection_points(curves, curves + num_curves,
                                    std::back_inserter(pts), true, traits);
  std::cout << "Found " << pts.size() << " intersection points: " << std::endl;
  std::copy(pts.begin(), pts.end(),
            std::ostream_iterator<Point>(std::cout, "\n"));

  // Compute the non-intersecting sub-curves induced by the input curves.
  std::list<X_monotone_curve> sub_segs;
  CGAL::compute_subcurves(curves, curves + num_curves,
                          std::back_inserter(sub_segs), true, traits);
  std::cout << "Found " << sub_segs.size()
            << " interior-disjoint sub-curves." << std::endl;

  assert(CGAL::do_curves_intersect(curves, curves + num_curves, traits));

  return 0;
}
