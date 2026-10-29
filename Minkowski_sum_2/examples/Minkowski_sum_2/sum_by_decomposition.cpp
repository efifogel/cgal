//! \file examples/Minkowski_sum_2/sum_by_decomposition.cpp
// Computing the Minkowski sum of two non-convex polygons read from a file
// using the small-side angle-bisector decomposition strategy.

#include <fstream>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>
#include <CGAL/Small_side_angle_bisector_decomposition_2.h>

#include "pgn_print.h"

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;

int main(int argc, char* argv[]) {
  // Open the input file and read two polygons from it.
  const char* filename = (argc > 1) ? argv[1] : "rooms_star.dat";
  std::ifstream in_file(filename);
  if (! in_file.is_open()) {
    std::cerr << "Failed to open the input file." << std::endl;
    return -1;
  }
  Polygon_2   P, Q;
  in_file >> P >> Q;
  in_file.close();

  // Compute the Minkowski sum using the decomposition approach.
  // Both polygons are decomposed by the small-side angle-bisector strategy.
  CGAL::Small_side_angle_bisector_decomposition_2<Kernel>  ssab_decomp;
  auto np = CGAL::parameters::decomposition_strategy(ssab_decomp);
  Polygon_with_holes_2  sum = CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2(P, Q, np, np);
  std::cout << "P (+) Q = "; print_polygon_with_holes(sum);
  return 0;
}
