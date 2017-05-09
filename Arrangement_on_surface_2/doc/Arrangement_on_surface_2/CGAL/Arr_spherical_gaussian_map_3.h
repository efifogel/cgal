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
// $URL$
// $Id$
//
// Author(s): Efi Fogel         <efif@post.tau.ac.il>
//            Naama mayer       <naamamay@post.tau.ac.il>

#ifndef CGAL_ARR_SPHERICAL_GAUSSIAN_MAP_3_H
#define CGAL_ARR_SPHERICAL_GAUSSIAN_MAP_3_H

namespace CGAL {

/*! \ingroup PkgArrangement2GaussianMap
 * A helper class template that initializes a Gaussian map data structure.
 *
 * \tparam Sgm_T
 * \tparam Traits_T
 */
template <typename Sgm_T, typename Traits_T = typename Sgm_T::Traits>
class Arr_sgm_initializer {
public:
  /// \name Types
  /// @{
  typedef Sgm_T                                         Sgm;
  typedef Traits_T                                      Traits;

  typedef typename Traits::Vector_3                     Vector_3;
  /// @}


  /// \name Creation
  /// @{
  Arr_sgm_initializer(Sgm& sgm);
  /// @}

  /// \name Operations
  /// @{
  /*! Insert a great arc whose angle is less than Pi and is represented by two
   * normals into the SGM. Each normal defines an end point of the greate arc.
   * \param[in] normal1 represents the normal of the source point.
   * \param[in] normal2 represents the normal of the target point.
   */
  Halfedge_handle insert_non_intersecting(const Vector_3& normal1,
                                          const Vector_3& normal2);

  /*! Make x-monotone
   * \param[in] normal1 represents the normal of the source point.
   * \param[in] normal2 represents the normal of the target point.
   * \param[in,out] oi
   */
  template <typename OutputIterator>
  OutputIterator make_x_monotone(const Vector_3& normal1,
                                 const Vector_3& normal2,
                                 OutputIterator oi);

  /*! Insert a great arc whose angle is less than Pi and is represented by two
   * normals into the SGM. Each normal defines an end point of the greate arc.
   * \param[in] normal1 represents the normal of the source point.
   * \param[in] normal2 represents the normal of the target point.
   * \param[in,out] oi
   * \return the handle for the halfedge directed from the endpoint
   * represented by normal1 toward the endpoint represented by normal2
   * \pre the SGM is empty.
   */
  template <typename OutputIterator>
  OutputIterator insert(const Vector_3& normal1, const Vector_3& normal2,
                        OutputIterator oi);

  /*! Insert a great arc whose angle is less than Pi and is represented by two
   * normals into the SGM. Each normal defines an end point of the greate arc.
   * \param[in] normal1 represents the normal of the source point.
   * \param[in] vertex1 the handle of the vertex that is the source of the arc.
   * \param[in] normal2 represents the normal of the target point.
   * \param[in,out] oi
   * \return the handle for the halfedge directed from the endpoint
   * represented by normal1 toward the endpoint represented by normal2
   */
  template <typename OutputIterator>
  OutputIterator insert(const Vector_3& normal1, Vertex_handle vertex1,
                        const Vector_3& normal2,
                        OutputIterator oi);

  /*! Insert a great arc whose angle is less than Pi and is represented by two
   * normals into the SGM. Each normal defines an end point of the greate arc.
   * \param[in] normal1 represents the normal of the source point.
   * \param[in] normal2 represents the normal of the target point.
   * \param[in] vertex2 the handle of the vertex that is the target of the arc.
   * \param[in,out] oi
   * \return the handle for the halfedge directed from the endpoint
   * represented by normal1 toward the endpoint represented by normal2
   */
  template <typename OutputIterator>
  OutputIterator insert(const Vector_3& normal1,
                        const Vector_3& normal2, Vertex_handle vertex2,
                        OutputIterator oi);

  /*! Insert a great arc whose angle is less than Pi and is represented by two
   * normals into the SGM. Each normal defines an end point of the greate arc.
   * \param[in] normal1 represents the normal of the source point.
   * \param[in] vertex1 the handle of the vertex that is the source of the arc.
   * \param[in] normal2 represents the normal of the target point.
   * \param[in] vertex2 the handle of the vertex that is the target of the arc.
   * \param[in,out] oi
   * \return the handle for the halfedge directed from the endpoint
   * represented by normal1 toward the endpoint represented by normal2
   */
  template <typename OutputIterator>
  OutputIterator insert(const Vector_3& normal1, Vertex_handle vertex1,
                        const Vector_3& normal2, Vertex_handle vertex2,
                        OutputIterator oi);
  /// @}
};

/*! \ingroup PkgArrangement2GaussianMap
 * A data structure that represents a Gaussian map of a convex polytope.
 *
 * \tparam GeometryTraits_2_T of the concept `ArrangementTraits_2`.
 * \tparam Dcel_T a model of the concept `ArrPolyhedralSgmArrDcel`.
 *
 * \cgalModels `ArrSphericalGaussianMap_3`
 */
template <typename GeometryTraits_2_T,
          template <class T> class Dcel_T = Arr_default_dcel>
class Arr_spherical_gaussian_map_3 :
  public Arrangement_on_surface_2<GeometryTraits_2_T,
                                  Arr_spherical_topology_traits_2<GeometryTraits_2_T,
                                                                  Dcel_T<GeometryTraits_2_T> > >
{
  /// \name Types
  /// @{
  typedef GeometryTraits_2_T                    Geometry_traits_2;
  typedef Dcel_T                                Dcel;

  typedef Arrangement_on_surface_2<Geometry_traits_2,
	Arr_spherical_topology_traits_2<Geometry_traits_2, Dcel<Geometry_traits_2> > >                                             Base;
  /// @}
};

}

#endif
