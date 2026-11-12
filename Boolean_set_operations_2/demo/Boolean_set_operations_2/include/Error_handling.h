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

#ifndef CGAL_ERROR_HANDLING_H
#define CGAL_ERROR_HANDLING_H

/*! \file Error_handling.h
 * Reporting errors: message boxes for warnings and errors, and the handler of failed CGAL assertions and preconditions.
 */

#include <string>

/*! shows a warning; the program proceeds.
 */
void show_warning(std::string a_s);

/*! shows an error; the operation does not proceed.
 */
void show_error(std::string a_s);

/*! shows an error and throws a std::runtime_error.
 */
void error(std::string a_s);

/*! handles a failed CGAL assertion or precondition by showing an error and throwing a std::runtime_error, except for a
 * few known harmless failures.
 */
void error_handler(const char* what, const char* expr, const char* file, int line, const char* msg);

#endif
