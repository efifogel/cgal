//! \file examples/Surface_sweep_2/plane_sweep.cpp
// Computing intersection points among curves using the surface-sweep alg.

#include <list>
#include <cassert>
#include <iostream>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Surface_sweep_2_algorithms.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Traits = CGAL::Arr_segment_traits_2<Kernel>;
using Point = Traits::Point_2;
using Segment = Traits::Curve_2;

int main() {
  // Construct the input segments.
  Segment segs[] = {
    Segment(Point(1, 5), Point(8, 5)), Segment(Point(1, 1), Point(8, 8)),
    Segment(Point(3, 1), Point(3, 8)), Segment(Point(8, 5), Point(8, 8))
  };
  constexpr auto num_segs = sizeof(segs)/sizeof(Segment);

  // Compute all intersection points.
  std::list<Point> pts;
  CGAL::compute_intersection_points(segs, segs + num_segs,
                                    std::back_inserter(pts));
  std::cout << "Found " << pts.size() << " intersection points: " << std::endl;
  std::copy(pts.begin(), pts.end(),
            std::ostream_iterator<Point>(std::cout, "\n"));

  // Compute the non-intersecting sub-segs induced by the input segs.
  std::list<Segment> sub_segs;
  CGAL::compute_subcurves(segs, segs + num_segs,
                          std::back_inserter(sub_segs));
  std::cout << "Found " << sub_segs.size()
            << " interior-disjoint sub-segs." << std::endl;

  assert(CGAL::do_curves_intersect (segs, segs + num_segs));

  return 0;
}
