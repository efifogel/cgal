//! \file examples/Arrangement_on_surface_2/plane_sweep.cpp
// Computing intersection points among curves using the surface-sweep alg.

#include <list>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Surface_sweep_2_algorithms.h>
#include <CGAL/Arr_projection_traits_xy_3.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel       Kernel_3;
typedef CGAL::Arr_projection_traits_xy_3<Kernel_3>              Kernel;
typedef Kernel::Point_2                                         Point_2;
typedef Kernel::Segment_2                                       Segment_2;

int main()
{
  // Construct the input segments.
  Segment_2 segments[] = {Segment_2 (Point_2 (1, 5, 0), Point_2 (8, 5, 0)),
                          Segment_2 (Point_2 (1, 1, 0), Point_2 (8, 8, 0)),
                          Segment_2 (Point_2 (3, 1, 0), Point_2 (3, 8, 0)),
                          Segment_2 (Point_2 (8, 5, 0), Point_2 (8, 8, 0))};

  // Compute all intersection points.
  std::list<Point_2> pts;

  CGAL::Arr_segment_traits_2<Kernel> traits;
  const bool report_endpoints(false);
  CGAL::compute_intersection_points(segments, segments + 4,
                                    std::back_inserter(pts),
                                    report_endpoints, traits);

  // Print the result.
  std::cout << "Found " << pts.size() << " intersection points: " << std::endl;
  std::copy(pts.begin(), pts.end(),
            std::ostream_iterator<Point_2>(std::cout, "\n"));

  // Compute the non-intersecting sub-segments induced by the input segments.
  std::list<Segment_2> sub_segs;
  const bool mult_overlaps(false);
  CGAL::compute_subcurves(segments, segments + 4,
                          std::back_inserter(sub_segs),
                          mult_overlaps, traits);

  std::cout << "Found " << sub_segs.size()
            << " interior-disjoint sub-segments." << std::endl;

  CGAL_assertion(CGAL::do_curves_intersect(segments, segments + 4, traits));

  return 0;
}
