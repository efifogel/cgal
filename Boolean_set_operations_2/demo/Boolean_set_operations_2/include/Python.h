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

#ifndef CGAL_PYTHON_H
#define CGAL_PYTHON_H

/*! \file Python.h
 * The console language, a subset of Python that matches the CGALPY bindings: its expressions and statements (Py_expr
 * and Py_statement), their conversion back to Python (to_python()), and its parser (Py_parser).
 */

#include <charconv>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "Typedefs.h"

/*! obtains the shortest decimal representation that reads back as the same double, so that the program reproduces the
 * exact coordinates.
 */
inline QString to_python(double aValue) {
  char buffer[64];
  auto res = std::to_chars(buffer, buffer + sizeof(buffer), aValue);
  return QString::fromLatin1(buffer, int(res.ptr - buffer));
}

/*! obtains a linear polygon as a Python expression. (Its curves are segments along the boundary, in order; their
 * sources are the vertices.)
 */
inline QString to_python(const Linear_polygon& aPolygon) {
  QStringList points;
  for (auto cit = aPolygon.curves_begin(); cit != aPolygon.curves_end(); ++cit)
    points << QString("Point_2(%1, %2)")
      .arg(to_python(CGAL::to_double(cit->source().x())), to_python(CGAL::to_double(cit->source().y())));
  return QString("Polygon_2([%1])").arg(points.join(", "));
}

// The console language: a small subset of Python, matching the CGALPY
// bindings (Polygon_set_2, Polygon_2, Point_2):
//   name = Polygon_set_2()            name = Polygon_set_2(other)
//   name = Polygon_set_2(Polygon_2([Point_2(x, y), ...]))
//   name.op(arg)   name.op(set1, set2)  (op: join, intersection, difference,
//                                         symmetric_difference)
//   name.complement()   name.complement(other)   name.clear()
//   name.is_empty()   name.is_plane()   name.number_of_polygons_with_holes()
//   del name
// Statements may be separated by ';'; '#' starts a comment.
struct Py_expr {
  enum class Kind { Name, Polygon, Polygon_set };
  Kind m_kind = Kind::Name;
  QString m_name;                                   // Name
  std::vector<std::pair<double, double>> m_points;  // Polygon
  std::shared_ptr<Py_expr> m_arg;                   // Polygon_set (optional)
};

struct Py_statement {
  enum class Kind { Assign, Call, Delete };
  Kind m_kind = Kind::Call;
  QString m_target;              // the assigned, called, or deleted name
  QString m_method;              // Call
  std::vector<Py_expr> m_args;   // Call arguments; Assign: the value
};

/*! obtains an expression as normalized Python text.
 */
inline QString to_python(const Py_expr& aExpr) {
  switch (aExpr.m_kind) {
   case Py_expr::Kind::Name: return aExpr.m_name;
   case Py_expr::Kind::Polygon: {
     QStringList points;
     for (const auto& [x, y] : aExpr.m_points)
       points << QString("Point_2(%1, %2)").arg(to_python(x), to_python(y));
     return QString("Polygon_2([%1])").arg(points.join(", "));
   }
   case Py_expr::Kind::Polygon_set:
    return QString("Polygon_set_2(%1)")
      .arg(aExpr.m_arg ? to_python(*aExpr.m_arg) : QString());
  }
  return QString();
}

/*! obtains a statement as normalized Python text.
 */
inline QString to_python(const Py_statement& aStatement) {
  switch (aStatement.m_kind) {
   case Py_statement::Kind::Assign:
    return aStatement.m_target + " = " + to_python(aStatement.m_args[0]);
   case Py_statement::Kind::Delete: return "del " + aStatement.m_target;
   case Py_statement::Kind::Call: {
     QStringList args;
     for (const auto& arg : aStatement.m_args) args << to_python(arg);
     return QString("%1.%2(%3)").arg(aStatement.m_target, aStatement.m_method, args.join(", "));
   }
  }
  return QString();
}

// A recursive-descent parser for the console language. Throws a std::runtime_error with a message on a syntax error.
class Py_parser {
public:
  /*! constructs from a console line, which is tokenized.
   */
  explicit Py_parser(const QString& aLine) { tokenize(aLine); }

  /*! parses the statements of the line.
   */
  std::vector<Py_statement> parse() {
    std::vector<Py_statement> statements;
    while (! at_end()) {
      if (accept(";")) continue;
      statements.push_back(statement());
      if (! at_end()) expect(";");
    }
    return statements;
  }

private:
  enum class Tok { Ident, Number, Punct, End };
  struct Token { Tok m_kind; QString m_text; };
  std::vector<Token> m_tokens;
  std::size_t m_pos = 0;

  /*! throws a std::runtime_error with a given message.
   */
  [[noreturn]] void error(const QString& aMessage) const { throw std::runtime_error(aMessage.toStdString()); }

  /*! splits a line into tokens.
   */
  void tokenize(const QString& aLine) {
    static const QRegularExpression number(R"(^(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?)");
    static const QRegularExpression ident(R"(^[A-Za-z_]\w*)");
    int i = 0;
    while (i < aLine.size()) {
      QChar c = aLine[i];
      if (c.isSpace()) { ++i; continue; }
      if (c == '#') break;
      QStringView rest = QStringView(aLine).mid(i);
      auto m = ident.matchView(rest);
      if (m.hasMatch()) {
        m_tokens.push_back({Tok::Ident, m.captured(0)});
        i += m.capturedLength(0);
        continue;
      }
      m = number.matchView(rest);
      if (m.hasMatch()) {
        m_tokens.push_back({Tok::Number, m.captured(0)});
        i += m.capturedLength(0);
        continue;
      }
      if (QString("()[],.=;-+").contains(c)) {
        m_tokens.push_back({Tok::Punct, QString(c)});
        ++i;
        continue;
      }
      error(QString("unexpected character '%1'").arg(c));
    }
    m_tokens.push_back({Tok::End, QString()});
  }

  /*! obtains the token a given number of tokens ahead, without consuming it.
   */
  const Token& peek(std::size_t aAhead = 0) const { return m_tokens[std::min(m_pos + aAhead, m_tokens.size() - 1)]; }

  /*! determines whether all tokens are consumed.
   */
  bool at_end() const { return peek().m_kind == Tok::End; }

  /*! consumes a given punctuation token if it is next.
   */
  bool accept(const QString& aPunct) {
    if ((peek().m_kind == Tok::Punct) && (peek().m_text == aPunct)) {
      ++m_pos;
      return true;
    }
    return false;
  }

  /*! consumes a given punctuation token, which must be next.
   */
  void expect(const QString& aPunct) { if (! accept(aPunct)) error(QString("expected '%1'%2").arg(aPunct, near())); }

  /*! obtains a description of the position of the next token, for error messages.
   */
  QString near() const { return at_end() ? QString(" at the end") : QString(" before '%1'").arg(peek().m_text); }

  /*! consumes a name.
   */
  QString identifier() {
    if (peek().m_kind != Tok::Ident) error("expected a name" + near());
    return m_tokens[m_pos++].m_text;
  }

  /*! consumes a (signed) number.
   */
  double number() {
    bool negative = false;
    if (accept("-")) negative = true;
    else accept("+");
    if (peek().m_kind != Tok::Number) error("expected a number" + near());
    double value = m_tokens[m_pos++].m_text.toDouble();
    return negative ? -value : value;
  }

  /*! parses a statement.
   */
  Py_statement statement() {
    Py_statement st;
    QString name = identifier();
    if (name == "del") {
      st.m_kind = Py_statement::Kind::Delete;
      st.m_target = identifier();
      return st;
    }
    st.m_target = name;
    if (accept("=")) {
      st.m_kind = Py_statement::Kind::Assign;
      st.m_args.push_back(expression());
      return st;
    }
    expect(".");
    st.m_kind = Py_statement::Kind::Call;
    st.m_method = identifier();
    expect("(");
    if (! accept(")")) {
      do st.m_args.push_back(expression()); while (accept(","));
      expect(")");
    }
    return st;
  }

  /*! parses an expression.
   */
  Py_expr expression() {
    Py_expr e;
    QString name = identifier();
    if (name == "Polygon_set_2") {
      e.m_kind = Py_expr::Kind::Polygon_set;
      expect("(");
      if (! accept(")")) {
        e.m_arg = std::make_shared<Py_expr>(expression());
        expect(")");
      }
      return e;
    }
    if (name == "Polygon_2") {
      e.m_kind = Py_expr::Kind::Polygon;
      expect("(");
      expect("[");
      while (! accept("]")) {
        if (identifier() != "Point_2") error("expected Point_2(x, y)");
        expect("(");
        double x = number();
        expect(",");
        double y = number();
        expect(")");
        e.m_points.emplace_back(x, y);
        if (! accept(",")) { expect("]"); break; }
      }
      expect(")");
      return e;
    }
    if ((name == "Point_2") || (peek().m_text == "("))
      error(QString("unexpected '%1'; expected a polygon set name, "
                    "Polygon_2([...]), or Polygon_set_2(...)").arg(name));
    e.m_kind = Py_expr::Kind::Name;
    e.m_name = name;
    return e;
  }
};

#endif
