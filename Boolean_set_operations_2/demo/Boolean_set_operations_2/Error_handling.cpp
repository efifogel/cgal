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

/*! \file Error_handling.cpp
 * Reporting errors: message boxes for warnings and errors, and the handler of failed CGAL assertions and preconditions.
 */

#include "Error_handling.h"

#include <sstream>
#include <stdexcept>

#include <QMessageBox>
#include <QString>

void show_warning(std::string a_s) { QMessageBox::warning(NULL, "Warning", QString(a_s.c_str())); }

void show_error(std::string a_s) { QMessageBox::critical(NULL, "Critical Error", QString(a_s.c_str())); }

void error(std::string a_s) {
  show_error(a_s);
  throw std::runtime_error(a_s);
}

void error_handler(const char* what, const char* expr, const char* file, int line, const char* msg) {
  if (expr != "first != last" && expr != "_inc_to_right != cv._inc_to_right" && line != 1684 &&
      expr != "is_new == true") {
    std::ostringstream ss;

    ss << "CGAL error: " << what << " violation!" << std::endl
       << "Expr: " << expr << std::endl
       << "File: " << file << std::endl
       << "Line: " << line << std::endl;
    if (msg != 0 || msg != "" || msg != NULL) {
      ss << "Explanation:" << msg << std::endl;
    }
    if (expr == "is_simple_2(first, last, traits)") {
      ss << "\nCompleting Polygon from this point!!!" << std::endl;
    }
    if (msg == "The outer boundary self intersects at edges.") {
      ss << "\nDeleting Polygon causing Error!!!" << std::endl;
    }

    error(ss.str());
  }
}
