//! \file examples/Minkowski_sum_2/sum_by_decomposition.cpp
// Computing the Minkowski sum of two non-convex polygons read from a file
// using the small-side angle-bisector decomposition strategy.

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/minkowski_sum_2.h>
#include <CGAL/Polygon_vertical_decomposition_2.h>
#include <CGAL/draw_polygon_2.h>
#include <CGAL/draw_polygon_with_holes_2.h>

#include <iostream>
#include <fstream>

#include "print_utils.h"

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = Kernel::Point_2;
using Polygon_2 = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;

int main(int argc, char* argv[]) {
  // Open the input file.
  const char* filename = (argc > 1) ? argv[1] : "holes.dat";
  std::ifstream in_file(filename);

  if (! in_file.is_open()) {
    std::cerr << "Failed to open the input file." << std::endl;
    return -1;
  }

  // Read the two polygons from the file and compute their Minkowski sum.
  Polygon_with_holes_2 P, Q;

  in_file >> P >> Q;
  in_file.close();

  std::cout << "P = ";
  print_polygon_with_holes(P);

  CGAL::draw(P, "P");
  CGAL::draw(Q, "Q");

  // Compute the Minkowski sum using the decomposition approach.
  CGAL::Polygon_vertical_decomposition_2<Kernel>::Traits_2 traits;
  CGAL::Polygon_vertical_decomposition_2<Kernel> vertical_decomp(traits);
  Polygon_with_holes_2 sum = minkowski_sum_2(P, Q, vertical_decomp, traits);
  // Polygon_with_holes_2 sum = minkowski_sum_by_full_convolution_2(P, Q);
  CGAL::draw(sum, "P + Q");
  std::cout << "P (+) Q = ";
  print_polygon_with_holes(sum);

  return 0;
}
