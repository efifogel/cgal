// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Efi Fogel        <efif@post.tau.ac.il>

#ifndef BOOLEAN_SET_OPERATIONS_2_DO_POLYGON_INTERSECT_VISITOR_H
#define BOOLEAN_SET_OPERATIONS_2_DO_POLYGON_INTERSECT_VISITOR_H

#include <CGAL/license/Boolean_set_operations_2.h>

/*! \file
 *
 * Definition of the basic sweep-line visitors, for the usage of the global
 * sweep-line functions.
 */

#include <vector>
#include <type_traits>

#include <CGAL/Surface_sweep_2/Default_visitor.h>

namespace CGAL {

/*! \class Do_polygon_intersect_visitor
 *
 * A plane-sweep visitor that determines whether the curves in a given set intersect.
 */
template <typename GeometryTraits_2, typename Allocator_ = CGAL_ALLOCATOR(int)>
class Do_polygon_intersect_visitor :
    public Surface_sweep_2::Default_visitor<Do_polygon_intersect_visitor
                                            <GeometryTraits_2, Allocator_>, GeometryTraits_2, Allocator_> {
protected:
  bool m_found_x;               // have we found an intersection so far.

public:
  /*! constructs.
   */
  Do_polygon_intersect_visitor() : m_found_x(false) {}

  /*! destructs.
   */
  virtual ~Do_polygon_intersect_visitor() {}

  /*!
   */
  void found_intersection() {
    m_found_x = true;
    this->stop_sweep();
  }

  /*!
   */
  bool do_intersect() { return m_found_x; }
};

} // namespace CGAL

#endif
