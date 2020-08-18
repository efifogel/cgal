// Copyright (c) 2005-2007 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// 
//
// Author(s): Ophir Setter          <ophir.setter@post.tau.ac.il>
//

/*!
 * \file   voronoi_of_points_on_sphere.cpp
 *\brief  An example for a Voronoi diagram of points embedded on the sphere.  
 */


#include <iostream>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

#include <CGAL/envelope_voronoi_2.h>
#include <CGAL/envelope_voronoi_2.h>
#include <CGAL/Envelope_voronoi_traits_2/Spherical_voronoi_diagram_traits_2.h>

using namespace std;

typedef CGAL::Exact_predicates_exact_constructions_kernel        Kernel;
typedef CGAL::Spherical_voronoi_diagram_traits_2<Kernel>         Voronoi_traits;
typedef CGAL::Envelope_voronoi_2::Spherical_voronoi_diagram_2<Voronoi_traits>
  Voronoi_diagram;
typedef Voronoi_traits::Site_2                                   Site_2;

int main( int argc, char **argv )
{
  int n;
  std::cin >> n;
  
  std::list<Site_2> sites;
  for (int i = 0; i < n; ++i) {
    Site_2 p;
    cin >> p;
    cout << p << endl;
    sites.push_back(p);
  }

  std::cout << "Number of sites: " << sites.size() << std::endl;
  
  Voronoi_diagram diagram;
  CGAL::voronoi_2 (sites.begin(), sites.end(), diagram);

  std::cout << "Voronoi diagram:" << std::endl <<
    "V = " << diagram.number_of_vertices() << ", E = " << 
    diagram.number_of_edges() << ", F = " << diagram.number_of_faces() << 
    std::endl;
  
  return 0;
}
