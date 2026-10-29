//! \file examples/Minkowski_sum_2/sum_of_holes.cpp
// Computing the Minkowski sum of two polygons with holes read from a file
// using the vertical decomposition strategy and the full convolution method.

#include <iostream>
#include <iterator>
#include <vector>
#include <fstream>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_decomposition_2.h>
#include <CGAL/Polygon_vertical_decomposition_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>
#include <CGAL/draw_polygon_2.h>
#include <CGAL/draw_polygon_with_holes_2.h>

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
  auto np = CGAL::parameters::decomposition_strategy_with_holes(vertical_decomp);
  Polygon_with_holes_2 sum =
    CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2(P, Q, np, np, CGAL::parameters::geom_traits(traits));
  CGAL::draw(sum, "P + Q");
  std::cout << "P (+) Q = ";
  print_polygon_with_holes(sum);

  // Compute the Minkowski sum using the full convolution method, which
  // requires the outer boundaries to be oriented counterclockwise and the
  // holes to be oriented clockwise, and compare the two sums.
  Polygon_with_holes_2 conv_sum = CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2(P, Q);
  std::vector<Polygon_with_holes_2> sym_diff;
  CGAL::symmetric_difference(sum, conv_sum, std::back_inserter(sym_diff));
  std::cout << "The full convolution method computes "
            << (sym_diff.empty() ? "the same sum." : "a different sum!") << std::endl;

  return sym_diff.empty() ? 0 : 1;
}
