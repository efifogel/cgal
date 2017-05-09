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
 * \cgalHasModel `CGAL::Arr_polyhedral_sgm_arr_face<Point_3>`
 */
class ArrPolyhedralSgmArrFace {
public:
  /// \name Types
  /// @{
  typedef unspecified_type              Point_3;
  /// @}

  /// \name Operations
  /// @{
  /*! Set the 3D point of the original polyhedron.
   */
  void set_point(const Point_3& point);

  /*! Obtain the 3D point of the original polyhedron.
   */
  const Point_3& point() const;

  /*! Determine whether the point has been set already.
   */
  bool is_set() const;

  /*! Resets the flag.
   */
  void set_is_set(bool flag);
  /// @}
};
