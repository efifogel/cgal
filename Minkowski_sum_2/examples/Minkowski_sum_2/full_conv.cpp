//! \file examples/Minkowski_sum_2/full_conv.cpp
// Computing the Minkowski sum of a non-convex polygon and an unbounded polygon
// with holes by full convolution.

#include <CGAL/Minkowski_sum_2/minkowski_sum_by_full_convolution_2.h>

#include "bops_linear.h"
#include "pgn_print.h"

int main() {
  // Construct the polygon P, a dart with one reflex vertex, the origin.
  Polygon_2 P;
  P.push_back(Point_2(-4, -5)); P.push_back(Point_2(6, 0));
  P.push_back(Point_2(-4, 5));  P.push_back(Point_2(0, 0));

  // Construct the unbounded polygon with holes Q, the plane minus a square; it
  // has no outer boundary, and its single hole is oriented clockwise.
  Polygon_2 hole;
  hole.push_back(Point_2(0, 0));   hole.push_back(Point_2(0, 30));
  hole.push_back(Point_2(30, 30)); hole.push_back(Point_2(30, 0));
  Polygon_with_holes_2 Q(Polygon_2(), &hole, &hole + 1);

  // Compute and print the Minkowski sum.
  Polygon_with_holes_2 sum = CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2(P, Q);
  std::cout << "P (+) Q = ";
  print_polygon_with_holes(sum);
  return 0;
}
