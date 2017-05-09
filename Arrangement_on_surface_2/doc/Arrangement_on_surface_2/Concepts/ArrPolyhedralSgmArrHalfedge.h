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
 * \cgalHasModel `CGAL::Arr_polyhedral_sgm_arr_halfedge<X_monotone_curve_2>`
 */
class ArrPolyhedralSgmArrHalfedge {
public:
  /// \name Types
  /// @{
  typedef unspecified_type              X_monotone_curve_2;
  /// @}

  /// \name Operations
  /// @{
  /*! Add an arrangement to the mask of the original arrangements in the
   * minkowski sum.
   * \param[in] id the id of the added arrangement.
   */
  void add_arr(unsigned int id);

  /*! Return true iff a given arrangement contributed this halfedge
   * while performing the minkowski sum
   */
  bool is_arr(unsigned int id) const;

  /*! Obtain the mask of the ids of the original arrangements that contributed
   * the halfedge while performing the minkowski sum
   */
  unsigned int arr_mask() const;

  /*! Set the arr of an edge with a value.
  * \param[in] id the id to set to.
  */
  void set_arr(unsigned int id);
  /// @}
};
