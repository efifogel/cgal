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

/*! \file Polynomial_parser.cpp
 * The parser of the equations of algebraic curves, implemented with Lexy.
 */

#include "Polynomial_parser.h"

#ifdef CGAL_USE_LEXY

#include <cstring>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <CGAL/Fraction_traits.h>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>

namespace {

using Algebraic_integer = CGAL::Arithmetic_kernel::Integer;

/*! The value of an expression: a polynomial, or the reason why the expression is not a polynomial (e.g., a division
 * by x).
 */
struct Value {
  Bivariate_polynomial m_polynomial;
  std::string m_error;
};

/*! obtains an error value.
 */
Value error_value(std::string a_error) { return Value{Bivariate_polynomial(), std::move(a_error)}; }

/*! obtains the integer written in decimal digits (possibly preceded by a minus sign).
 */
Algebraic_integer to_integer(std::string a_digits) {
  // Leading zeros would be taken for an octal number.
  std::size_t sign = (! a_digits.empty() && (a_digits[0] == '-')) ? 1 : 0;
  auto first = a_digits.find_first_not_of('0', sign);
  a_digits.erase(sign, ((first == std::string::npos) ? a_digits.size() - 1 : first) - sign);
  std::istringstream is(a_digits);
  Algebraic_integer r_n;
  is >> r_n;
  return r_n;
}

/*! obtains the constant of a constant polynomial, or nothing if the polynomial is not constant.
 */
std::optional<Algebraic_rational> constant(const Bivariate_polynomial& a_p) {
  if (a_p.empty()) return Algebraic_rational(0);
  if ((a_p.size() == 1) && (a_p.begin()->first == std::make_pair(0u, 0u))) return a_p.begin()->second;
  return std::nullopt;
}

/*! The largest exponent, which keeps the expansion of powers short.
 */
constexpr unsigned int max_exponent = 1000;

enum class Op { ADD, SUBTRACT, MULTIPLY, DIVIDE, POWER, NEGATE, PLUS };

/*! applies a binary operator.
 */
Value apply(const Value& a_lhs, Op a_op, const Value& a_rhs) {
  if (! a_lhs.m_error.empty()) return a_lhs;
  if (! a_rhs.m_error.empty()) return a_rhs;
  const auto& p = a_lhs.m_polynomial;
  const auto& q = a_rhs.m_polynomial;
  switch (a_op) {
   case Op::ADD: return Value{p + q, ""};
   case Op::SUBTRACT: return Value{p - q, ""};
   case Op::MULTIPLY: return Value{p * q, ""};
   case Op::DIVIDE: {
     auto c = constant(q);
     if (! c) return error_value("The divisor " + to_string(q) + " is not a constant.");
     if (CGAL::is_zero(*c)) return error_value("Division by zero.");
     Bivariate_polynomial r;
     add_to(r, p, Algebraic_rational(1) / *c);
     return Value{r, ""};
   }
   case Op::POWER: {
     auto c = constant(q);
     using Decompose = CGAL::Fraction_traits<Algebraic_rational>::Decompose;
     Algebraic_integer num, den;
     if (c) Decompose()(*c, num, den);
     if (! c || (den != Algebraic_integer(1)) || CGAL::is_negative(num))
       return error_value("The exponent " + to_string(q) + " is not a non-negative integer.");
     if (num > Algebraic_integer(max_exponent))
       return error_value("The exponent " + to_string(q) + " is larger than " + std::to_string(max_exponent) + ".");
     return Value{power(p, static_cast<unsigned int>(CGAL::to_double(num))), ""};
   }
   default: return error_value("Unknown operator.");
  }
}

/*! applies a unary operator.
 */
Value apply(Op a_op, const Value& a_operand) {
  if (! a_operand.m_error.empty() || (a_op == Op::PLUS)) return a_operand;
  return Value{-a_operand.m_polynomial, ""};
}

namespace grammar {

namespace dsl = lexy::dsl;

constexpr auto name = dsl::identifier(dsl::ascii::alpha, dsl::ascii::word);

/*! An unsigned decimal number, possibly with a fraction, e.g., 12 or 0.25 (which is 1/4).
 */
struct number : lexy::token_production {
  static constexpr auto rule = dsl::capture(dsl::token(dsl::digits<> + dsl::opt(dsl::lit_c<'.'> >> dsl::digits<>)));
  static constexpr auto value = lexy::callback<Value>([](auto a_lexeme) {
    std::string text(a_lexeme.begin(), a_lexeme.end());
    auto point = text.find('.');
    if (point == std::string::npos) return Value{constant_polynomial(Algebraic_rational(to_integer(text))), ""};
    std::string fraction = text.substr(point + 1);
    Algebraic_integer denominator = CGAL::ipower(Algebraic_integer(10), int(fraction.size()));
    Algebraic_rational c = Algebraic_rational(to_integer(text.substr(0, point) + fraction)) /
      Algebraic_rational(denominator);
    return Value{constant_polynomial(c), ""};
  });
};

/*! A signed integer, e.g., -3.
 */
struct integer : lexy::token_production {
  static constexpr auto rule = dsl::capture(dsl::token(dsl::opt(dsl::lit_c<'-'>) + dsl::digits<>));
  static constexpr auto value = lexy::as_string<std::string>;
};

/*! A rational number RN(n, d).
 */
struct rational {
  static constexpr auto rule =
    LEXY_KEYWORD("RN", name) >> dsl::parenthesized(dsl::p<integer> + dsl::comma + dsl::p<integer>);
  static constexpr auto value = lexy::callback<Value>([](const std::string& a_num, const std::string& a_den) {
    Algebraic_integer den = to_integer(a_den);
    if (CGAL::is_zero(den)) return error_value("The denominator of RN(" + a_num + ", " + a_den + ") is 0.");
    return Value{constant_polynomial(Algebraic_rational(to_integer(a_num)) / Algebraic_rational(den)), ""};
  });
};

/*! A variable, x or y.
 */
struct variable {
  static constexpr auto rule = name;
  static constexpr auto value = lexy::callback<Value>([](auto a_lexeme) {
    std::string text(a_lexeme.begin(), a_lexeme.end());
    if (text == "x") return Value{variable_polynomial(0), ""};
    if (text == "y") return Value{variable_polynomial(1), ""};
    return error_value("Unknown name '" + text + "'; the variables are x and y (a product is written with '*').");
  });
};

struct nested_expression : lexy::transparent_production {
  static constexpr auto rule = dsl::recurse<struct expression>;
  static constexpr auto value = lexy::forward<Value>;
};

/*! An expression, by the precedences of its operators.
 */
struct expression : lexy::expression_production {
  struct expected_operand {
    static constexpr auto name = "expected a number, x, y, RN(n, d), or '('";
  };

  static constexpr auto atom = dsl::parenthesized(dsl::p<nested_expression>) | dsl::p<rational> | dsl::p<variable> |
    dsl::p<number> | dsl::error<expected_operand>;

  struct power_op : dsl::infix_op_right {
    static constexpr auto op = dsl::op<Op::POWER>(LEXY_LIT("^"));
    using operand = dsl::atom;
  };

  struct sign_op : dsl::prefix_op {
    static constexpr auto op = dsl::op<Op::NEGATE>(LEXY_LIT("-")) / dsl::op<Op::PLUS>(LEXY_LIT("+"));
    using operand = power_op;
  };

  struct product_op : dsl::infix_op_left {
    static constexpr auto op = dsl::op<Op::MULTIPLY>(LEXY_LIT("*")) / dsl::op<Op::DIVIDE>(LEXY_LIT("/"));
    using operand = sign_op;
  };

  struct sum_op : dsl::infix_op_left {
    static constexpr auto op = dsl::op<Op::ADD>(LEXY_LIT("+")) / dsl::op<Op::SUBTRACT>(LEXY_LIT("-"));
    using operand = product_op;
  };

  using operation = sum_op;

  static constexpr auto value = lexy::callback<Value>(
    lexy::forward<Value>,
    [](const Value& a_lhs, Op a_op, const Value& a_rhs) { return apply(a_lhs, a_op, a_rhs); },
    [](Op a_op, const Value& a_operand) { return apply(a_op, a_operand); });
};

/*! An equation p == q (or p = q), whose polynomial is p - q.
 */
struct equation {
  struct expected_equals {
    static constexpr auto name = "expected an operator or '=='";
  };
  struct expected_end {
    static constexpr auto name = "expected an operator or the end of the equation";
  };

  static constexpr auto whitespace = dsl::ascii::blank;
  static constexpr auto rule = dsl::whitespace(dsl::ascii::blank) + dsl::p<expression> +
    (LEXY_LIT("==") | LEXY_LIT("=") | dsl::error<expected_equals>) + dsl::p<expression> +
    (dsl::eof | dsl::error<expected_end>);
  static constexpr auto value = lexy::callback<Value>([](const Value& a_lhs, const Value& a_rhs) {
    return apply(a_lhs, Op::SUBTRACT, a_rhs);
  });
};

} // namespace grammar

/*! parses an equation.
 */
std::optional<Bivariate_polynomial> parse(const std::string& a_text, std::string& r_error) {
  using Input = lexy::string_input<lexy::utf8_char_encoding>;
  Input input(a_text.data(), a_text.size());

  // The first syntax error, at its column.
  std::string error;
  auto on_error = [&](const auto& /* a_context */, const auto& a_error) {
    if (! error.empty()) return;
    using Error = std::decay_t<decltype(a_error)>;
    std::string message;
    if constexpr (std::is_same_v<Error, lexy::error_for<Input, lexy::expected_literal>> ||
                  std::is_same_v<Error, lexy::error_for<Input, lexy::expected_keyword>>)
      message = "expected '" + std::string(a_error.string(), a_error.length()) + "'";
    else if constexpr (std::is_same_v<Error, lexy::error_for<Input, lexy::expected_char_class>>) {
      std::string name = a_error.name();
      message = "expected " + ((name == "digit.decimal") ? std::string("a digit") : name);
    }
    else message = a_error.message();
    auto column = reinterpret_cast<const char*>(a_error.position()) - a_text.data() + 1;
    error = "Column " + std::to_string(column) + ": " + message + ".";
  };
  auto result = lexy::parse<grammar::equation>(input, lexy::callback(on_error));
  if (! error.empty() || ! result.has_value()) {
    r_error = error.empty() ? std::string("Not an equation.") : error;
    return std::nullopt;
  }
  const Value& value = result.value();
  if (! value.m_error.empty()) {
    r_error = value.m_error;
    return std::nullopt;
  }
  return value.m_polynomial;
}

} // namespace

Polynomial_parser polynomial_parser() { return parse; }

#else

Polynomial_parser polynomial_parser() { return Polynomial_parser(); }

#endif
