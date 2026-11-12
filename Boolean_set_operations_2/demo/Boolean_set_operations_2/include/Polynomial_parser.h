// Copyright (c) 2026  Tel-Aviv University (Israel).
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

#ifndef CGAL_POLYNOMIAL_PARSER_H
#define CGAL_POLYNOMIAL_PARSER_H

/*! \file Polynomial_parser.h
 * The parser of the equations of algebraic curves, e.g., "x^2 + y^2 == 25". The grammar:
 *   equation   := expression ('==' | '=') expression
 *   expression := expression ('+' | '-') expression | expression ('*' | '/') expression | ('-' | '+') expression
 *               | expression '^' expression | '(' expression ')' | 'x' | 'y' | number | 'RN(' integer ',' integer ')'
 *   number     := digits ['.' digits]
 * with the usual precedences; '^' is right associative and binds tighter than the unary '-' (-x^2 is -(x^2)). The
 * divisor must be a nonzero constant and the exponent a non-negative integer constant.
 */

#include "Algebraic_curves.h"

/*! obtains the parser of equations, which is empty if the demo is built without Lexy.
 */
Polynomial_parser polynomial_parser();

#endif
