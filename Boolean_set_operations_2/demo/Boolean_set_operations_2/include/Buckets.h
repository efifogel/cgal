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

#include <QBrush>
#include <QColor>
#include <QPen>
#include <QString>

// The buckets are identified by their indices: red, black, blue, brown, yellow, magenta, and aqua. The scratch result
// set follows them. Initially (and at least) the first three buckets exist.
constexpr std::size_t number_of_buckets = 7;
constexpr std::size_t result_index = number_of_buckets;
constexpr std::size_t min_number_of_buckets = 3;

// The pens and the brushes of the buckets and of the scratch result set.
inline QPen sPens[] = {
  QPen(QColor(255, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //red
  QPen(QColor(0, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),     //black
  QPen(QColor(0, 0, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //blue
  QPen(QColor(210, 105, 30), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),//brown
  QPen(QColor(255, 255, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //yellow
  QPen(QColor(255, 0, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //magenta
  QPen(QColor(0, 255, 255), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), //aqua
  QPen(QColor(0, 0, 0, 0), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),   //Result
};

inline QBrush sBrushes[] = {
  QBrush(QColor(255, 0, 0, 75)),           //red
  QBrush(QColor(0, 0, 0, 75)),             //black
  QBrush(QColor(0, 0, 255, 75)),           //blue
  QBrush(QColor(210, 105, 30, 75)),        //brown
  QBrush(QColor(255, 255, 0, 75)),         //yellow
  QBrush(QColor(255, 0, 255, 75)),         //magenta
  QBrush(QColor(0, 255, 255, 75)),         //aqua
  QBrush(QColor(0, 0, 0, 0)),              //Result
};

/*! obtains the name of a bucket in the program (the color of its row), by its index.
 */
inline const char* bucket_name(std::size_t aIndex) {
  static const char* names[] = { "red", "black", "blue", "brown", "yellow", "magenta", "aqua" };
  return (aIndex < number_of_buckets) ? names[aIndex] : "result";
}

/*! obtains a string with its first letter capitalized.
 */
inline QString capitalized(QString aString) {
  if (! aString.isEmpty()) aString[0] = aString[0].toUpper();
  return aString;
}

#endif
