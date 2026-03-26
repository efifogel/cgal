#include <iostream>
#include <fstream>
#include <vector>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Cartesian_converter.h>

using Epick = CGAL::Exact_predicates_inexact_constructions_kernel;
using Epic_traits = CGAL::Gps_segment_traits_2<Epick>;
using Epic_polygon = CGAL::Polygon_2<Epick>;
using Epic_polygon_with_holes = CGAL::Polygon_with_holes_2<Epick>;

using Epeck = CGAL::Exact_predicates_exact_constructions_kernel;
using Epec_traits = CGAL::Gps_segment_traits_2<Epeck>;
using Epec_polygon = CGAL::Polygon_2<Epeck>;
using Epec_polygon_with_holes = CGAL::Polygon_with_holes_2<Epeck>;

//! convert polygons
template <typename PolygonOut, typename PolygonIn>
PolygonOut convert_polygon(const PolygonIn& polygon_in) {
  using Polygon_out = PolygonOut;
  using Kernel_out = typename Polygon_out::Traits;
  using Polygon_in = PolygonIn;
  using Kernel_in = typename Polygon_in::Traits;

  Polygon_out polygon_out;
  CGAL::Cartesian_converter<Kernel_in, Kernel_out> converter;
  for (auto it = polygon_in.vertices_begin(); it != polygon_in.vertices_end(); ++it)
    polygon_out.push_back(converter(*it));
  return polygon_out;
}

template <typename PolygonWithHolesOut, typename PolygonWithHolesIn>
PolygonWithHolesOut convert_polygon_with_holes(const PolygonWithHolesIn& pwh_in) {
  using Polygon_with_holes_out = PolygonWithHolesOut;
  using Polygon_out = typename Polygon_with_holes_out::Polygon_2;

  // Step A: Convert the outer boundary
  auto outer_out = convert_polygon<Polygon_out>(pwh_in.outer_boundary());

  // Step B: Convert all the holes
  std::vector<Polygon_out> holes_out;
  holes_out.reserve(std::distance(pwh_in.holes_begin(), pwh_in.holes_end()));
  for (auto hit = pwh_in.holes_begin(); hit != pwh_in.holes_end(); ++hit) {
    holes_out.push_back(convert_polygon<Polygon_out>(*hit));
  }

  // Step C: Construct the new exact polygon with holes
  return Polygon_with_holes_out(outer_out, holes_out.begin(), holes_out.end());
}

//!
bool test_one_file(std::ifstream& inp) {
 // Read the polygons
  size_t n = 0;
  if (! (inp >> n)) {
    std::cerr << "Error: Could not read the number of polygons.\n";
    return false;
  }

  std::vector<Epic_polygon> pgns(n);
  std::vector<Epec_polygon> exact_pgns(n);
  for (size_t i = 0; i < n; ++i) {
    inp >> pgns[i];
    exact_pgns[i] = convert_polygon<Epec_polygon>(pgns[i]);
  }

  // Read the polygons with holes
  size_t m = 0;
  if (! (inp >> m)) {
    std::cerr << "Error: Could not read the number of polygons with holes.\n";
    return false;
  }

  std::vector<Epic_polygon_with_holes> pwhs(m);
  std::vector<Epec_polygon_with_holes> exact_pwhs(m);
  for (size_t i = 0; i < m; ++i) {
    inp >> pwhs[i];
    exact_pwhs[i] = convert_polygon_with_holes<Epec_polygon_with_holes>(pwhs[i]);
  }

  // Execute tests
  Epic_traits traits;
  Epec_traits exact_traits;

  std::cout << "--- Test Results ---\n";

  // Test 1.1a: Single range (polygon)
  if (n > 0) {
    bool res = CGAL::do_intersect(pgns.begin(), pgns.end(), traits);
    std::cout << "1. Set of Polygon_2 intersect: " << (res ? "True" : "False") << "\n";
    std::vector<Epec_polygon_with_holes> intersections;
    CGAL::intersection(exact_pgns.begin(), exact_pgns.end(), std::back_inserter(intersections), exact_traits);
    if (intersections.empty() == res) {
      std::cerr << "Error: polygon intersection\n";
      return false;
    }
  }
  else {
    std::cout << "1. Set of Polygon_2 intersect: Skipped (empty set)\n";
  }

  // Test 1.1b: Single range (polygon_with_holes)
  if (m > 0) {
    bool res = CGAL::do_intersect(pwhs.begin(), pwhs.end(), traits);
    std::cout << "2. Set of Polygon_with_holes_2 intersect: " << (res ? "True" : "False") << "\n";
    std::vector<Epec_polygon_with_holes> intersections;
    CGAL::intersection(exact_pwhs.begin(), exact_pwhs.end(), std::back_inserter(intersections), exact_traits);
    if (intersections.empty() == res) {
      std::cerr << "Error: polygon-with-holes intersection\n";
      return false;
    }
  }
  else {
    std::cout << "2. Set of Polygon_with_holes_2 intersect: Skipped (empty set)\n";
  }

  // Test 2.2: Two ranges (polygon and polygon with holes)
  if (n > 0 && m > 0) {
    bool res = CGAL::do_intersect(pgns.begin(), pgns.end(), pwhs.begin(), pwhs.end(), traits);
    std::cout << "3. Cross-intersection between the two sets: "
              << (res ? "True" : "False") << "\n";
    std::vector<Epec_polygon_with_holes> intersections;
    CGAL::intersection(exact_pgns.begin(), exact_pgns.end(), exact_pwhs.begin(), exact_pwhs.end(),
                       std::back_inserter(intersections), exact_traits);
    if (intersections.empty() == res) {
      std::cerr << "Error: mixed polygons and polygon-with-holes intersection\n";
      return false;
    }
  }
  else {
    std::cout << "3. Cross-intersection between the two sets: Skipped (one or both sets empty)\n";
  }

  return true;
}

//!
int main(int argc, char* argv[]) {
   if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file_1.txt> ...\n";
    return -1;
  }

  int success = 0;
  for (int i = 1; i < argc; ++i) {
    std::string str(argv[i]);
    if (str.empty()) continue;

    std::string::iterator itr = str.end();
    --itr;
    while (itr != str.begin()) {
      std::string::iterator tmp = itr;
      --tmp;
      if (*itr == 't') break;

      str.erase(itr);
      itr = tmp;
    }
    if (str.size() <= 1) continue;
    std::ifstream inp(str.c_str());
    if (! inp.is_open()) {
      std::cerr << "Error: Could not open file " << str << std::endl;
      return -1;
    }
    if (! test_one_file(inp)) {
      std::cout << str << ": ERROR" << std::endl;
      ++success;
    }
    else {
      std::cout << str << ": PASSED" << std::endl;
    }
    inp.close();
  }

  return success;
}
