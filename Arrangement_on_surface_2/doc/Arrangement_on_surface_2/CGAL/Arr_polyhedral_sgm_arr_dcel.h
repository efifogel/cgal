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

#ifndef CGAL_ARR_POLYHEDRAL_SGM_ARR_DCEL_H
#define CGAL_ARR_POLYHEDRAL_SGM_ARR_DCEL_H

namespace CGAL {

/*! \ingroup PkgArrangement2GaussianMap
 * Extend the arrangement vertex.
 *
 * \cgalModels `ArrPolyhedralSgmArrVertex`
 */
template <typename Point_2_T>
class Arr_polyhedral_sgm_arr_vertex : public CGAL::Arr_vertex_base<Point_2_T> {
public:
  /// \name Types
  /// @{
  typedef Point_2_T                     Point_2;
  /// @}
};

/*! \ingroup PkgArrangement2GaussianMap
 * Extend the arrangement halfedge.
 *
 * \cgalModels `ArrPolyhedralSgmArrHalfedge`
 */
template <typename X_monotone_curve_2_T>
class Arr_polyhedral_sgm_arr_halfedge :
  public CGAL::Arr_halfedge_base<X_monotone_curve_2_T>
{
public:
  /// \name Types
  /// @{
  typedef X_monotone_curve_2_T          X_monotone_curve_2;
  /// @}
};

/*! \ingroup PkgArrangement2GaussianMap
 * Extend the arrangement face.
 *
 * \cgalModels `ArrPolyhedralSgmArrFace`
 */
template <typename Point_3_T>
class Arr_polyhedral_sgm_arr_face : public CGAL::Arr_face_base {
public:
  /// \name Types
  /// @{
  typedef Point_3_T             Point_3;
  /// @}
};

/*! \ingroup PkgArrangement2GaussianMap
 * A DCEL data structure with Gaussian map features.
 *
 * \cgalModels `ArrPolyhedralSgmArrDcel`
 */
template <typename GeometryTraits_2_T>
class Arr_polyhedral_sgm_arr_dcel :
  public CGAL::Arr_dcel_base<Arr_polyhedral_sgm_arr_vertex<typename GeometryTraits_2_T::Point_2>,
                             Arr_polyhedral_sgm_arr_halfedge<typename GeometryTraits_2_T::X_monotone_curve_2>,
                             Arr_polyhedral_sgm_arr_face<typename GeometryTraits_2_T::Point_3> >
{
public:
  /// \name Types
  /// @{
  typedef GeometryTraits_2_T            GeometryTraits_2;
  /// @}
};

}

#endif
