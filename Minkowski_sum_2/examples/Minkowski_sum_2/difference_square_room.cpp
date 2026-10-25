//! \file examples/Minkowski_sum_2/difference_square_room.cpp
// Computing the Minkowski difference of a square and a room with an obstacle.

#include <iterator>
#include <vector>

#include <CGAL/Minkowski_sum_2/minkowski_difference_2.h>

#include "bops_linear.h"
#include "pgn_print.h"

int main() {
  // Construct the square.
  Polygon_2 P;
  P.push_back(Point_2(-1, -1));  P.push_back(Point_2(1, -1));
  P.push_back(Point_2(1, 1));    P.push_back(Point_2(-1, 1));

  // Construct the room, a square with a square obstacle (a hole) in it.
  Polygon_2 room;
  room.push_back(Point_2(0, 0));   room.push_back(Point_2(10, 0));
  room.push_back(Point_2(10, 10)); room.push_back(Point_2(0, 10));
  Polygon_2 obstacle;
  obstacle.push_back(Point_2(4, 4)); obstacle.push_back(Point_2(4, 6));
  obstacle.push_back(Point_2(6, 6)); obstacle.push_back(Point_2(6, 4));
  Polygon_with_holes_2 Q(room, &obstacle, &obstacle + 1);

  // Compute the Minkowski difference, the translations of the reflected
  // square that keep it inside the room and away from the obstacle.
  std::vector<Polygon_with_holes_2> diff;
  CGAL::Minkowski_sum_2::minkowski_difference_2(P, Q, std::back_inserter(diff));
  std::cout << "P (-) Q consists of " << diff.size() << " polygon(s) with holes:" << std::endl;
  for (const auto& pwh : diff) print_polygon_with_holes(pwh);
  return 0;
}
