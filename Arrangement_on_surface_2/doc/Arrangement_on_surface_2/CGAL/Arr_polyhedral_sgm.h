// Copyright (c) 2006,2007,2008,2009,2010,2011 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Author(s): Efi Fogel         <efif@post.tau.ac.il>

#ifndef CGAL_ARR_POLYHEDRAL_SGM_H
#define CGAL_ARR_POLYHEDRAL_SGM_H

namespace CGAL {

/*! \ingroup PkgArrangement2GaussianMap
 * A helper class that can be used to initialize a Gaussian map from a
 * Polyhedron_3 data structure.
 *
 * \tparam PolyhedralSgm_T a model of the concept `ArrPolyhedralSgm`.
 * \tparam Polyhedron_T a model of the concepts `FaceGraph`.
 * \tparam Visitor_T a model of the concept `ArrPolyhedralSgmInitializerVisitor`.
 */
template <typename PolyhedralSgm_T,
          typename Polyhedron_T,
          typename Visitor_T =
            Arr_polyhedral_sgm_initializer_visitor<PolyhedralSgm_T, Polyhedron_T> >
class Arr_polyhedral_sgm_initializer :
  public Arr_sgm_initializer<typename PolyhedralSgm_T::Base>
{
  /// \name Types
  /// @{
  typedef PolyhedralSgm_T                       PolyhedralSgm;
  typedef Polyhedron_T                          Polyhedron;
  typedef Visitor                               Visitor;
  /// @}

  /// \name Creation
  /// @{
  /*! Initialize the Gaussian map
   * \param[in] polyhedron
   * \param[in] visitor
   * \pre `polyhedron` does not have coplanar facets.
   */
  void operator()(Polyhedron& polyhedron, Visitor* visitor = nullptr);
  /// @}
};

/*! \ingroup PkgArrangement2GaussianMap
 * A data structure that represents a Gaussian map of a convex polytope.
 *
 * \tparam GeometryTraits_2_T of the concept `ArrangementTraits_2`.
 * \tparam Dcel_T a model of the concept `ArrPolyhedralSgmArrDcel`.
 *
 * \cgalModels `ArrPolyhedralSgm`
 */
template <typename GeometryTraits_2_T,
          template <class T> class Dcel_T = Arr_polyhedral_sgm_arr_dcel>
class Arr_polyhedral_sgm :
  public Arr_spherical_gaussian_map_3<GeometryTraits_2_T, Dcel_T>
{
public:
  /// \name Types
  /// @{
  typedef GeometryTraits_2_T                            Geometry_traits_2;

  typedef Arr_spherical_gaussian_map_3<Geometry_traits_2, Dcel>
                                                        Base;
  /// @}
};

}

#endif
