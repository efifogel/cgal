// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_CONVOLUTION_CURVES_H
#define CGAL_MINKOWSKI_SUM_2_CONVOLUTION_CURVES_H

#include <CGAL/license/Minkowski_sum_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {

/*! The representation of the convolution cycles computed by the full
 * convolution method; it is given by the named parameter `convolution_curves`.
 */
enum class Convolution_curves {
  AUTOMATIC,  //!< chosen by prefer_segments()
  POLYLINES,  //!< polylines
  SEGMENTS    //!< segments
};

} // namespace Minkowski_sum_2
} // namespace CGAL

#endif
