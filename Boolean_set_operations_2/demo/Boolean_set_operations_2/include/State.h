// Copyright (c) 2012  Tel-Aviv University (Israel).
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
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s) : Efi Fogel <efifogel@gmail.com>
//             Kabir Kedia <kabirkedia0111@gmail.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>
//             Apurva Bhatt <response2apurva@gmail.com>

#ifndef CGAL_STATE_H
#define CGAL_STATE_H

/*! \file State.h
 * The states of the undo stack: each one holds the sets of the buckets and of the scratch result after an action.
 */

#include <vector>

#include "Buckets.h"
#include "Curve_set.h"

// The operations recorded on the undo stack.
enum class State_op {
  SO_COMPLEMENT, SO_INTERSECTION, SO_UNION, SO_DIFFERENCE, SYMMETRIC_DIFFERENCE, MINKOWSKI_SUM, SO_RESET, SO_COPY,
  SO_MOVE, SO_CLEAR, DELETE_ALL, SO_DRAW, SO_START, SO_CONSOLE
};

// The state after an action: the sets of the buckets and of the scratch result set.
class State_current {
public:
  /*! constructs the state after a given operation, with empty sets of a given curve family.
   */
  State_current(State_op a_operation, Curve_type a_type);

  /*! obtains the set of a given bucket (or the scratch result set, at result_index).
   */
  Curve_set& set(std::size_t a_index) { return m_curve_sets[a_index]; }

  /*! obtains the scratch result set.
   */
  Curve_set& result_set() { return set(result_index); }

  /*! shows or hides a bucket.
   */
  void toggle_view(std::size_t a_index, bool a_check) {
    if (a_check) set(a_index).gi()->show();
    else set(a_index).gi()->hide();
  }

  /*! clears a given bucket (or the result).
   */
  void clear(std::size_t a_index) { set(a_index).clear(); }

public:
  Curve_set_container m_curve_sets;

  State_op m_operation;
  Curve_type m_type;
};

inline State_current::State_current(State_op a_operation, Curve_type a_type) :
  m_operation(a_operation),
  m_type(a_type) {
  for (std::size_t i = 0; i <= number_of_buckets; ++i)
    m_curve_sets.push_back(Curve_set(m_type, s_pens[i], s_brushes[i]));
}

using State_container = std::vector<State_current>;

#endif
