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
 * \cgalHasModel `CGAL::Arr_polyhedral_sgm_initializer_visitor<PolyhedralSgm,Polyhedron>`
 */
class ArrPolyhedralSgmInitializerVisitor {
public:
  /// \name Types
  /// @{
  typedef unspecified_type                              Polyhedron;
  typedef unspecified_type                              PolyhedronSgm;

  typedef typename Polyhedron::Vertex_const_handle      Polyhedron_vertex_const_handle;
  typedef typename Polyhedron::Halfedge_const_handle    Polyhedron_halfedge_const_handle;
  typedef typename Polyhedron::Facet_const_handle       Polyhedron_facet_const_handle;

  typedef typename PolyhedralSgm::Vertex_handle         Sgm_vertex_handle;
  typedef typename PolyhedralSgm::Halfedge_handle       Sgm_halfedge_handle;
  typedef typename PolyhedralSgm::Face_handle           Sgm_face_handle;
  typedef typename PolyhedralSgm::Face_const_handle     Sgm_face_const_handle;
  /// @}

  /// \name Operations
  /// @{

  /*! Pass information from a polyhedron vertex to its dual---a Gaussian map
   * face.
   * \param[in] src
   * \param[out] trg
   */
  virtual void update_dual_vertex(Polyhedron_vertex_const_handle src,
                                  Sgm_face_handle trg);

  /*! Pass information from a dual vertex (face) to another dual vertex (face)
   * of the same vertex.
   * \param[in] src
   * \param[out] trg
   */
  virtual void update_dual_vertex(Sgm_face_const_handle src,
                                  Sgm_face_handle trg);

  /*! Pass information from a polyhedron facet to its dual---a Gaussian map
   * vertex.
   * \param[in] src
   * \param[out] trg
   */
  virtual void update_dual_face(Polyhedron_facet_const_handle src,
                                Sgm_vertex_handle trg);

  /*! Pass information from a polyhedron halfedge to its dual a---Gaussian map
   * halfedge.
   * \param[in] src
   * \param[out] trg
   */
  virtual void update_dual_halfedge(Polyhedron_halfedge_const_handle src,
                                    Sgm_halfedge_handle trg);
  /// @}
};
