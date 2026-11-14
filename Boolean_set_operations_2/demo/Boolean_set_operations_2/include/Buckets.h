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

#ifndef CGAL_BUCKETS_H
#define CGAL_BUCKETS_H

/*! \file Buckets.h
 * The buckets: their number, their pens and brushes, and their names in the program.
 */

#include <cstddef>
#include <iterator>

#include <QBrush>
#include <QColor>
#include <QPen>
#include <QString>

// The buckets are identified by their indices, in the order of their colors below. The scratch result set follows
// them. Initially (and at least) the first three buckets exist; the user sets the maximum (up to the number of colors).
constexpr std::size_t max_number_of_buckets = 16;
constexpr std::size_t result_index = max_number_of_buckets;
constexpr std::size_t min_number_of_buckets = 3;
constexpr std::size_t default_max_number_of_buckets = 7;

// The name of each bucket (in the program, and capitalized in its row) and its color.
struct Bucket_color {
  const char* m_name;
  QColor m_color;
};

inline const Bucket_color s_bucket_colors[] = {
  {"red", QColor(255, 0, 0)}, {"black", QColor(0, 0, 0)}, {"blue", QColor(0, 0, 255)},
  {"brown", QColor(210, 105, 30)}, {"yellow", QColor(255, 255, 0)}, {"magenta", QColor(255, 0, 255)},
  {"aqua", QColor(0, 255, 255)}, {"orange", QColor(255, 140, 0)}, {"green", QColor(0, 160, 0)},
  {"purple", QColor(128, 0, 128)}, {"gray", QColor(128, 128, 128)}, {"pink", QColor(255, 105, 180)},
  {"navy", QColor(0, 0, 128)}, {"olive", QColor(128, 128, 0)}, {"teal", QColor(0, 128, 128)},
  {"maroon", QColor(128, 0, 0)}};
static_assert(std::size(s_bucket_colors) == max_number_of_buckets);

/*! obtains the pen of a bucket, or (at result_index) of the scratch result set, which is not drawn.
 */
inline QPen bucket_pen(std::size_t a_index) {
  QColor color = (a_index < max_number_of_buckets) ? s_bucket_colors[a_index].m_color : QColor(0, 0, 0, 0);
  return QPen(color, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

/*! obtains the brush of a bucket, or (at result_index) of the scratch result set, which is not drawn.
 */
inline QBrush bucket_brush(std::size_t a_index) {
  if (a_index >= max_number_of_buckets) return QBrush(QColor(0, 0, 0, 0));
  QColor color = s_bucket_colors[a_index].m_color;
  color.setAlpha(75);
  return QBrush(color);
}

/*! obtains the name of a bucket in the program, by its index.
 */
inline const char* bucket_name(std::size_t a_index)
{ return (a_index < max_number_of_buckets) ? s_bucket_colors[a_index].m_name : "result"; }

#endif
