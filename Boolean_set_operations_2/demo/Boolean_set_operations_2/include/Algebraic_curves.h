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

#ifndef CGAL_ALGEBRAIC_CURVES_H
#define CGAL_ALGEBRAIC_CURVES_H

/*! \file Algebraic_curves.h
 * Algebraic curves given by the user as polynomial equations, e.g., "x^2 + 3*y^2 == 10" or "x^2 + RN(1, 2)*y - 1"
 * (an expression p stands for p == 0). The text is parsed by a Polynomial_parser (to be implemented with Boost.Spirit
 * or Lexy) into a Bivariate_polynomial, using the arithmetic below, and the polynomial is converted into a curve of
 * Arr_algebraic_segment_traits_2.
 */

#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include <CGAL/Arithmetic_kernel.h>
#include <CGAL/Fraction_traits.h>
#include <CGAL/Polynomial.h>

/*! The rational numbers of the coefficients.
 */
using Algebraic_rational = CGAL::Arithmetic_kernel::Rational;

/*! A polynomial in x and y with rational coefficients: the coefficient of each monomial x^i*y^j, keyed by (i, j). Zero
 * coefficients are not stored, so the zero polynomial is empty.
 */
using Bivariate_polynomial = std::map<std::pair<unsigned int, unsigned int>, Algebraic_rational>;

/*! A parser of polynomial equations over integer and rational numbers (x/y or RN(x, y)) with +, -, *, ^, and ==,
 * which obtains the polynomial p - q of an equation p == q (and p of an expression p).
 * \return the polynomial, or nothing if the text is not an equation, in which case the error describes why.
 */
using Polynomial_parser = std::function<std::optional<Bivariate_polynomial>(const std::string& a_text,
                                                                            std::string& r_error)>;

// The arithmetic of polynomials, for the actions of the parsers.

/*! obtains a constant polynomial.
 */
inline Bivariate_polynomial constant_polynomial(const Algebraic_rational& a_c) {
  Bivariate_polynomial r_p;
  if (! CGAL::is_zero(a_c)) r_p[{0, 0}] = a_c;
  return r_p;
}

/*! obtains the polynomial x (a_variable 0) or y (a_variable 1).
 */
inline Bivariate_polynomial variable_polynomial(int a_variable) {
  Bivariate_polynomial r_p;
  r_p[(a_variable == 0) ? std::make_pair(1u, 0u) : std::make_pair(0u, 1u)] = Algebraic_rational(1);
  return r_p;
}

/*! adds a polynomial times a scalar to a polynomial.
 */
inline void add_to(Bivariate_polynomial& r_p, const Bivariate_polynomial& a_q, const Algebraic_rational& a_scalar = 1) {
  for (const auto& [monomial, c] : a_q) {
    Algebraic_rational sum = r_p[monomial] + a_scalar * c;
    if (CGAL::is_zero(sum)) r_p.erase(monomial);
    else r_p[monomial] = sum;
  }
}

/*! obtains the sum of two polynomials.
 */
inline Bivariate_polynomial operator+(Bivariate_polynomial a_p, const Bivariate_polynomial& a_q) {
  add_to(a_p, a_q);
  return a_p;
}

/*! obtains the difference of two polynomials.
 */
inline Bivariate_polynomial operator-(Bivariate_polynomial a_p, const Bivariate_polynomial& a_q) {
  add_to(a_p, a_q, -1);
  return a_p;
}

/*! obtains the negation of a polynomial.
 */
inline Bivariate_polynomial operator-(const Bivariate_polynomial& a_p) { return Bivariate_polynomial() - a_p; }

/*! obtains the product of two polynomials.
 */
inline Bivariate_polynomial operator*(const Bivariate_polynomial& a_p, const Bivariate_polynomial& a_q) {
  Bivariate_polynomial r_p;
  for (const auto& [m1, c1] : a_p) {
    for (const auto& [m2, c2] : a_q) {
      Bivariate_polynomial term;
      term[{m1.first + m2.first, m1.second + m2.second}] = c1 * c2;
      add_to(r_p, term);
    }
  }
  return r_p;
}

/*! obtains a polynomial raised to a non-negative integer power.
 */
inline Bivariate_polynomial power(const Bivariate_polynomial& a_p, unsigned int a_exponent) {
  Bivariate_polynomial r_p = constant_polynomial(1);
  for (unsigned int i = 0; i < a_exponent; ++i) r_p = r_p * a_p;
  return r_p;
}

/*! obtains a polynomial as text, e.g., "x^2 + 3*y^2 - 10".
 */
inline std::string to_string(const Bivariate_polynomial& a_p) {
  if (a_p.empty()) return "0";
  // By descending total degree, and then by descending power of x.
  using Key = std::pair<unsigned int, unsigned int>;
  std::map<Key, std::pair<Key, Algebraic_rational>, std::greater<Key>> terms;
  for (const auto& [m, c] : a_p) terms.emplace(Key(m.first + m.second, m.first), std::make_pair(m, c));
  std::ostringstream os;
  bool first = true;
  for (const auto& [degree, term] : terms) {
    const auto& [m, c] = term;
    bool negative = CGAL::is_negative(c);
    Algebraic_rational a = negative ? Algebraic_rational(-c) : c;
    if (first) os << (negative ? "-" : "");
    else os << (negative ? " - " : " + ");
    first = false;
    std::string monomial;
    for (auto [name, e] : {std::make_pair("x", m.first), std::make_pair("y", m.second)}) {
      if (e == 0) continue;
      if (! monomial.empty()) monomial += "*";
      monomial += name;
      if (e > 1) monomial += "^" + std::to_string(e);
    }
    bool one = (a == Algebraic_rational(1));
    if (! one || monomial.empty()) os << a;
    if (! one && ! monomial.empty()) os << "*";
    os << monomial;
  }
  return os.str();
}

/*! converts a polynomial into a curve of Arr_algebraic_segment_traits_2 (or any traits whose curves are constructed
 * from a Polynomial_2). The coefficients are multiplied by the least common multiple of their denominators, which does
 * not change the curve.
 */
template <typename Traits>
typename Traits::Curve_2 algebraic_curve(const Bivariate_polynomial& a_p, const Traits& a_traits) {
  using Polynomial_2 = typename Traits::Polynomial_2;
  using Integer = typename CGAL::Polynomial_traits_d<Polynomial_2>::Innermost_coefficient_type;
  using Decompose = typename CGAL::Fraction_traits<Algebraic_rational>::Decompose;
  Decompose decompose;

  // The least common multiple of the denominators.
  Integer lcm(1);
  for (const auto& [m, c] : a_p) {
    Integer num, den;
    decompose(c, num, den);
    lcm = lcm / CGAL::gcd(lcm, Integer(den)) * Integer(den);
  }

  Polynomial_2 x = CGAL::shift(Polynomial_2(1), 1, 0);
  Polynomial_2 y = CGAL::shift(Polynomial_2(1), 1, 1);
  Polynomial_2 polynomial(0);
  for (const auto& [m, c] : a_p) {
    Integer num, den;
    decompose(c, num, den);
    polynomial += Integer(num) * (lcm / Integer(den)) * CGAL::ipower(x, int(m.first)) * CGAL::ipower(y, int(m.second));
  }
  return a_traits.construct_curve_2_object()(polynomial);
}

#endif
