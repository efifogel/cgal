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
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <QRegularExpression>
#include <QString>
#include <QStringList>

#include "Typedefs.h"
#include "Conic_arcs.h"
#include "Circular_source.h"

/*! obtains the shortest decimal representation that reads back as the same double, so that the program reproduces the
 * exact coordinates.
 */
inline QString to_python(double a_value) {
  char buffer[64];
  auto res = std::to_chars(buffer, buffer + sizeof(buffer), a_value);
  return QString::fromLatin1(buffer, int(res.ptr - buffer));
}

/*! obtains an exact integer as a Python expression.
 */
inline QString to_python(const Conic_integer& a_value) {
  std::ostringstream os;
  os << a_value;
  return QString::fromStdString(os.str());
}

/*! obtains a point (of double coordinates) as a Python expression.
 */
inline QString to_python(const Kernel::Point_2& a_p) {
  return QString("Point_2(%1, %2)").arg(to_python(CGAL::to_double(a_p.x())), to_python(CGAL::to_double(a_p.y())));
}

/*! obtains the boundary of a polygon bounded by segments and circular arcs as a Python expression, from what it was
 * constructed from (see Circular_boundary_source). The program defines polygon() and squared_distance() (see
 * program_header()).
 */
inline QString to_python(const Circular_boundary_source& a_source) {
  QStringList curves;
  for (const auto& piece : a_source) {
    const auto& p = piece.m_points;
    switch (piece.m_kind) {
     case Circular_piece_source::Kind::SEGMENT:
      curves << QString("Curve_2(%1, %2)").arg(to_python(p[0]), to_python(p[1]));
      break;
     case Circular_piece_source::Kind::ARC:
      curves << QString("Curve_2(%1, %2, %3)").arg(to_python(p[0]), to_python(p[1]), to_python(p[2]));
      break;
     case Circular_piece_source::Kind::CIRCLE:
      curves << QString("Curve_2(Circle_2(%1, squared_distance(%1, %2)))").arg(to_python(p[0]), to_python(p[1]));
      break;
    }
  }
  return QString("polygon([%1])").arg(curves.join(", "));
}

/*! obtains the coefficients of a conic as a Python argument list.
 */
inline QString to_python(const Conic_arc_record::Coefficients& a_c) {
  QStringList r_c;
  for (const auto& c : a_c) r_c << to_python(c);
  return r_c.join(", ");
}

/*! obtains a conic arc as a Python expression (see Conic_arc_record). The program defines ctr_curve() (see
 * program_header()).
 */
inline QString to_python(const Conic_arc_record& a_arc) {
  if (a_arc.m_full) return QString("ctr_curve(%1)").arg(to_python(a_arc.m_conic));
  QString orientation = (a_arc.m_orientation == CGAL::CLOCKWISE) ? "Result.CLOCKWISE" :
    (a_arc.m_orientation == CGAL::COUNTERCLOCKWISE) ? "Result.COUNTERCLOCKWISE" : "Result.COLLINEAR";
  return QString("ctr_curve(%1, %2, Point_2(%3, %4), %5, Point_2(%6, %7), %8)")
    .arg(to_python(a_arc.m_conic), orientation, to_python(a_arc.m_source[0]), to_python(a_arc.m_source[1]),
         to_python(a_arc.m_source_conic), to_python(a_arc.m_target[0]), to_python(a_arc.m_target[1]),
         to_python(a_arc.m_target_conic));
}

/*! obtains a conic polygon as a Python expression. The program defines polygon() (see program_header()).
 * \throw std::invalid_argument if the boundary cannot be written exactly (see conic_arc_records()).
 */
inline QString to_python(const Conic_polygon& a_polygon) {
  QStringList arcs;
  for (const auto& arc : conic_arc_records(a_polygon)) arcs << to_python(arc);
  return QString("polygon([%1])").arg(arcs.join(", "));
}

/*! obtains the beginning of a program of a curve family: the import of the CGALPY bindings and the definitions that the
 * statements use. The bindings of a family are named by their kernel and traits, e.g., CGALPY_kerEpec_aos2Cs_bso2.
 */
inline QString program_header(Curve_type a_type) {
  const QString polygon_function =
    "def polygon(curves):\n"
    "  \"\"\"constructs a polygon from its boundary curves, in counterclockwise order.\"\"\"\n"
    "  make_x_monotone = Traits().make_x_monotone_2_object()\n"
    "  return Polygon_2([o for cv in curves for o in make_x_monotone(cv) if isinstance(o, X_monotone_curve_2)])\n";
  QString header = "# Generated by the CGAL 2D Boolean Set Operations demo.\n";
  switch (a_type) {
   case Curve_type::CT_CIRCULAR:
    return header +
      "from CGALPY_kerEpec_aos2Cs_bso2 import Ker, Bso2\n\n"
      "Point_2 = Ker.Point_2\n"
      "Circle_2 = Ker.Circle_2\n"
      "Polygon_set_2 = Bso2.General_polygon_set_2\n"
      "Traits = Polygon_set_2.Traits_2\n"
      "Curve_2 = Traits.Curve_2\n"
      "X_monotone_curve_2 = Traits.X_monotone_curve_2\n"
      "Polygon_2 = Polygon_set_2.Polygon_2\n\n"
      "def squared_distance(p, q):\n"
      "  return (q.x() - p.x()) * (q.x() - p.x()) + (q.y() - p.y()) * (q.y() - p.y())\n\n" + polygon_function + "\n";

   case Curve_type::CT_CONIC:
    return header +
      "from CGALPY_kerEpec_aos2Conic_bso2 import Bso2, Result\n\n"
      "Polygon_set_2 = Bso2.General_polygon_set_2\n"
      "Traits = Polygon_set_2.Traits_2\n"
      "Point_2 = Traits.Point_2\n"
      "X_monotone_curve_2 = Traits.X_monotone_curve_2\n"
      "Polygon_2 = Polygon_set_2.Polygon_2\n"
      "ctr_curve = Traits().construct_curve_2_object()\n\n" + polygon_function + "\n";

   case Curve_type::CT_LINEAR: break;

   default: header += "# The polygons of this curve family are not written to programs yet.\n"; break;
  }
  return header +
    "from CGALPY_kerEpec_aos2Seg_bso2_pol2 import Ker, Pol2, Bso2\n\n"
    "Point_2 = Ker.Point_2\n"
    "Polygon_2 = Pol2.Polygon_2\n"
    "Polygon_set_2 = Bso2.Polygon_set_2\n\n";
}

/*! obtains a linear polygon as a Python expression. (Its curves are segments along the boundary, in order; their
 * sources are the vertices.)
 */
inline QString to_python(const Linear_polygon& a_polygon) {
  QStringList points;
  for (auto cit = a_polygon.curves_begin(); cit != a_polygon.curves_end(); ++cit)
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
  enum class Kind { KI_NAME, KI_POLYGON, POLYGON_SET };
  Kind m_kind = Kind::KI_NAME;
  QString m_name;                                   // Name
  std::vector<std::pair<double, double>> m_points;  // Polygon
  std::shared_ptr<Py_expr> m_arg;                   // Polygon_set (optional)
};

struct Py_statement {
  enum class Kind { KI_ASSIGN, KI_CALL, KI_DELETE };
  Kind m_kind = Kind::KI_CALL;
  QString m_target;              // the assigned, called, or deleted name
  QString m_method;              // Call
  std::vector<Py_expr> m_args;   // Call arguments; Assign: the value
};

/*! obtains an expression as normalized Python text.
 */
inline QString to_python(const Py_expr& a_expr) {
  switch (a_expr.m_kind) {
   case Py_expr::Kind::KI_NAME: return a_expr.m_name;
   case Py_expr::Kind::KI_POLYGON: {
     QStringList points;
     for (const auto& [x, y] : a_expr.m_points)
       points << QString("Point_2(%1, %2)").arg(to_python(x), to_python(y));
     return QString("Polygon_2([%1])").arg(points.join(", "));
   }
   case Py_expr::Kind::POLYGON_SET:
    return QString("Polygon_set_2(%1)")
      .arg(a_expr.m_arg ? to_python(*a_expr.m_arg) : QString());
  }
  return QString();
}

/*! obtains a statement as normalized Python text.
 */
inline QString to_python(const Py_statement& a_statement) {
  switch (a_statement.m_kind) {
   case Py_statement::Kind::KI_ASSIGN:
    return a_statement.m_target + " = " + to_python(a_statement.m_args[0]);
   case Py_statement::Kind::KI_DELETE: return "del " + a_statement.m_target;
   case Py_statement::Kind::KI_CALL: {
     QStringList args;
     for (const auto& arg : a_statement.m_args) args << to_python(arg);
     return QString("%1.%2(%3)").arg(a_statement.m_target, a_statement.m_method, args.join(", "));
   }
  }
  return QString();
}

// A recursive-descent parser for the console language. Throws a std::runtime_error with a message on a syntax error.
class Py_parser {
public:
  /*! constructs from a console line, which is tokenized.
   */
  explicit Py_parser(const QString& a_line) { tokenize(a_line); }

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
  enum class Tok { TO_IDENT, TO_NUMBER, TO_PUNCT, TO_END };
  struct Token { Tok m_kind; QString m_text; };
  std::vector<Token> m_tokens;
  std::size_t m_pos = 0;

  /*! throws a std::runtime_error with a given message.
   */
  [[noreturn]] void error(const QString& a_message) const { throw std::runtime_error(a_message.toStdString()); }

  /*! splits a line into tokens.
   */
  void tokenize(const QString& a_line) {
    static const QRegularExpression number(R"(^(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?)");
    static const QRegularExpression ident(R"(^[A-Za-z_]\w*)");
    int i = 0;
    while (i < a_line.size()) {
      QChar c = a_line[i];
      if (c.isSpace()) { ++i; continue; }
      if (c == '#') break;
      QStringView rest = QStringView(a_line).mid(i);
      auto m = ident.matchView(rest);
      if (m.hasMatch()) {
        m_tokens.push_back({Tok::TO_IDENT, m.captured(0)});
        i += m.capturedLength(0);
        continue;
      }
      m = number.matchView(rest);
      if (m.hasMatch()) {
        m_tokens.push_back({Tok::TO_NUMBER, m.captured(0)});
        i += m.capturedLength(0);
        continue;
      }
      if (QString("()[],.=;-+").contains(c)) {
        m_tokens.push_back({Tok::TO_PUNCT, QString(c)});
        ++i;
        continue;
      }
      error(QString("unexpected character '%1'").arg(c));
    }
    m_tokens.push_back({Tok::TO_END, QString()});
  }

  /*! obtains the token a given number of tokens ahead, without consuming it.
   */
  const Token& peek(std::size_t a_ahead = 0) const { return m_tokens[std::min(m_pos + a_ahead, m_tokens.size() - 1)]; }

  /*! determines whether all tokens are consumed.
   */
  bool at_end() const { return peek().m_kind == Tok::TO_END; }

  /*! consumes a given punctuation token if it is next.
   */
  bool accept(const QString& a_punct) {
    if ((peek().m_kind == Tok::TO_PUNCT) && (peek().m_text == a_punct)) {
      ++m_pos;
      return true;
    }
    return false;
  }

  /*! consumes a given punctuation token, which must be next.
   */
  void expect(const QString& a_punct) { if (! accept(a_punct)) error(QString("expected '%1'%2").arg(a_punct, near())); }

  /*! obtains a description of the position of the next token, for error messages.
   */
  QString near() const { return at_end() ? QString(" at the end") : QString(" before '%1'").arg(peek().m_text); }

  /*! consumes a name.
   */
  QString identifier() {
    if (peek().m_kind != Tok::TO_IDENT) error("expected a name" + near());
    return m_tokens[m_pos++].m_text;
  }

  /*! consumes a (signed) number.
   */
  double number() {
    bool negative = false;
    if (accept("-")) negative = true;
    else accept("+");
    if (peek().m_kind != Tok::TO_NUMBER) error("expected a number" + near());
    double value = m_tokens[m_pos++].m_text.toDouble();
    return negative ? -value : value;
  }

  /*! parses a statement.
   */
  Py_statement statement() {
    Py_statement st;
    QString name = identifier();
    if (name == "del") {
      st.m_kind = Py_statement::Kind::KI_DELETE;
      st.m_target = identifier();
      return st;
    }
    st.m_target = name;
    if (accept("=")) {
      st.m_kind = Py_statement::Kind::KI_ASSIGN;
      st.m_args.push_back(expression());
      return st;
    }
    expect(".");
    st.m_kind = Py_statement::Kind::KI_CALL;
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
      e.m_kind = Py_expr::Kind::POLYGON_SET;
      expect("(");
      if (! accept(")")) {
        e.m_arg = std::make_shared<Py_expr>(expression());
        expect(")");
      }
      return e;
    }
    if (name == "Polygon_2") {
      e.m_kind = Py_expr::Kind::KI_POLYGON;
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
    e.m_kind = Py_expr::Kind::KI_NAME;
    e.m_name = name;
    return e;
  }
};

#endif
