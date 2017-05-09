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

/*! \ingroup PkgArrangement2GaussianMap
 * \cgalConcept
 *
 * A model of this concepts is a data structure that represents a Gaussian map
 * of a convex polytope.
 *
 * \cgalRefines `ArrSphericalGaussianMap_3`
 *
 * \cgalHasModel `CGAL::Arr_polyhedral_sgm<GeometryTraits_2,Dcel>`
 */

class ArrPolyhedralSgm {
  /// \name Types
  /// @{
  typedef unspecified_type                              Base;
  typedef unspecified_type                              Polyhedron;

  typedef Polyhedron::HalfedgeDS                        HDS;
  typedef CGAL::Polyhedron_incremental_builder_3<HDS>   Builder;
  typedef Builder::size_type                            size_type;
  /// @}

  /// \name Operations
  /// @{
  /*! Compute the Minkowski sum of 2 objects of type Arr_polyhedral_sgm
   *
   * \tparam PolyhedralSgm_T a model of the concept `ArrPolyhedralSgm`.
   *
   * \param[in] sgm1 the first Arr_polyhedral_sgm object
   * \param[in] sgm2 the second Arr_polyhedral_sgm object
   */
  template <typename PolyhedralSgm_T>
  void minkowski_sum(const PolyhedralSgm& sgm1,
                     const PolyhedralSgm& sgm2);

  /*! Compute the Minkowski sum of two Gaussian map objects.
   *
   * \tparam PolyhedralSgm_T a model of the concept `ArrPolyhedralSgm`.
   * \tparam OverlayTraits_T
   *
   * \param[in] sgm1 the first Gaussian map object.
   * \param[in] sgm2 the second Gaussian map object.
   * \param[in] overlay_traits the overlay traits.
   */
  template <typename PolyhedralSgm_T, typename OverlayTraits_T>
  void minkowski_sum(const PolyhedralSgm_T& sgm1,
                     const PolyhedralSgm_T& sgm2,
                     OverlayTraits_T& overlay_traits);

  /*! Obtain the number of (primal) vertices */
  size_type number_of_vertices() const;

  /*! Obtain the number of (primal) edges
   * \return the number of (primal) edges.
   * Edges that connect vertices of degree 2 are not counted, as they have
   * been introduced only to make non-x-monotone curves x-monotone.
   */
  size_type number_of_edges() const;

  /*! Obtain the number of (primal) facets
   * \return the number of (primal) facets.
   * Vertices of degree 2 are not counted, as they have been introduced only
   * to make non-x-monotone curves x-monotone.
   */
  size_type number_of_facets() const;
  /// @}
};
