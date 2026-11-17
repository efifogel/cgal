// Copyright (c) 2004-2008 Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Pavel Emeliyanenko <asm@mpi-sb.mpg.de>
//
// ============================================================================

/*!\file CGAL/Curved_kernel_via_analysis_2/gfx/Curve_renderer_internals.h
 * contains various low-level routines used by \c Curve_renderer_2 and
 * \c Subdivision_1
 *
 * provide caching for polynomials and polynomial evaluations; 1D range
 * analysis using First Quadratic Affine Form (QF) and Modified Affine
 * Arithmetic (MAA), both equipped with recursive derivative information
 */

#ifndef CGAL_CKVA_CURVE_RENDERER_INTERNALS_H
#define CGAL_CKVA_CURVE_RENDERER_INTERNALS_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>
// #include <boost/multi_index_container.hpp>
// #include <boost/multi_index/member.hpp>
// #include <boost/multi_index/hashed_index.hpp>
// #include <boost/multi_index/sequenced_index.hpp>

#include <CGAL/Arithmetic_kernel.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/Coercion_traits.h>
#include <CGAL/Polynomial.h>
#include <CGAL/Polynomial_traits_d.h>

#include <CGAL/Curved_kernel_via_analysis_2/gfx/Curve_renderer_traits.h>

// prints out detailed info of the rendering process
// (only for internal debugging)
#ifndef Gfx_DETAILED_OUT
//#define Gfx_USE_DETAILED_OUT
#ifdef Gfx_USE_DETAILED_OUT
#define Gfx_DETAILED_OUT(x) std::cerr << x
#else
#define Gfx_DETAILED_OUT(x) static_cast<void>(0)
#endif
#endif

// prints out only high-level debug messages
#ifndef Gfx_OUT
// #define Gfx_USE_OUT
#ifdef Gfx_USE_OUT
#define Gfx_OUT(x) std::cerr << x
#else
#define Gfx_OUT(x) static_cast<void>(0)
#endif
#endif

// using boost::multi_index::multi_index_container;
// using boost::multi_index::get;
// using boost::multi_index::project;

namespace CGAL {

namespace internal {

#define CGAL_N_CACHES 2 // maximal number of cache instances

#define CGAL_X_RANGE 0 // polynomial in x-range

#define CGAL_Y_RANGE 1 // polynomial in y-range

// defines subdivision level beyond which univariate polynomials are not
// cached (if 0 cache is off)
#define CGAL_MAX_POLY_CACHE_LEVEL 8

// maximal number of entries in univariate polynomial cache containers
#define CGAL_POLY_CACHE_SIZE 1024 * 1024

// maximal degree of the derivative taken into account during recursive
// derivative range analysis
#define CGAL_RECURSIVE_DER_MAX_DEGREE 7

// 8-pixel neighborhood directions
static const struct {
  int m_x;
  int m_y;
} directions[] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
// map from rectangle sides to pixel directions
// 0 - left side, 1 - right side, 2 - bottom side, 3 - top side
static const int DIR_MAP[][3] = {{5, 4, 3}, {7, 0, 1}, {5, 6, 7}, {3, 2, 1}};
// map from 8 pixel directions to "left" (0) or "right" (1) direction of motion
// vertical directions (2 and 6) are mapped to -1
static const int DIR_TAKEN_MAP[] = {1, 1, -1, 0, 0, 0, -1, 1};

template <typename Integer>
struct Pixel_2_ {
  // pixel coordinates relative to the drawing area
  int m_x;
  int m_y;
  unsigned m_level; // subdivision level: 0 - for pixels,
                    // 1 - for 1/2 pixels, and so on)
#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
  double xv, yv; // double approximation of curve-pixel intersection point
#endif

  Integer m_sub_x, m_sub_y; // subpixel coordinates relative to pixel's boundary
                            // (always 0 for pixels)

  bool operator==(const Pixel_2_& pix) const {
    return (m_x == pix.m_x && m_y == pix.m_y && m_level == pix.m_level && m_sub_x == pix.m_sub_x &&
            m_sub_y == pix.m_sub_y);
  }
};

// structure describing the seed point and backward direction, support for
// multiple seed points
template <typename Integer>
struct Seed_point_ {
  Seed_point_() {}

  Seed_point_(const Pixel_2_<Integer>& start_, int dir_, int taken_, bool coincide_) :
    m_start(start_),
    m_back_dir(dir_),
    m_direction_taken(taken_),
    m_branches_coincide(coincide_) {}

  Pixel_2_<Integer> m_start; // starting pixel
  int m_back_dir;            // backward direction
  int m_direction_taken;
  bool m_branches_coincide;
};

template <typename Rational, typename AlgebraicReal>
struct Clip_point_ { // describes a bottom/top clip point
  Clip_point_() {}
  Clip_point_(Rational left_, Rational right_, int arcno_ = -1) : m_left(left_), m_right(right_), m_arcno(arcno_) {}
  // isolating interval boundaries
  Rational m_left;
  Rational m_right;
  int m_arcno;           // arcno of a segment this point belongs to
  AlgebraicReal m_alpha; // this clip point event line
};

template <typename Integer>
std::ostream& operator<<(std::ostream& os, const Pixel_2_<Integer>& pix) {
  os << " (" << pix.m_x << "/" << pix.m_sub_x << "; " << pix.m_y << "/" << pix.m_sub_y << ") level = " << pix.m_level;
  return os;
}

/*! defines class \c Curve_renderer_internals
 *
 * provides an interface to low-level range analysis and polynomials evaluation
 * methods along with caching mechanism. \c Coeff_ defines internal polynomial
 * coefficients used by the renderer.
 * \c CurveKernel_2 specifies underlying 2D curve kernel
 */
template <typename CurveKernel_2, typename Coeff_>
class Curve_renderer_internals {
public:
  //! \name typedefs
  //!@{

  //! this instance's first template argument
  using Curve_kernel_2 = CurveKernel_2;

  //! this instance's second template argument
  using Coeff = Coeff_;

  //! type of 1-curve analysis
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //! type of supporting polynomial
  using Polynomial_2 = typename Curve_analysis_2::Polynomial_2;

  //! underlying univariate polynomial type
  using Poly_dst_1 = typename Polynomial_2::NT;

  //! rational number type
  using Rational =
    typename ::CGAL::Get_arithmetic_kernel<typename Curve_kernel_2::Coefficient>::Arithmetic_kernel::Rational;

  using Renderer_traits = Curve_renderer_traits<Coeff, Rational>;

  /// polynomial traits should be used whenever possible
  using Polynomial_traits_2 = CGAL::Polynomial_traits_d<Polynomial_2>;

  using Coercion = CGAL::Coercion_traits<Rational, typename Curve_kernel_2::Coefficient>;

  //! coercion between rational and polynomial coefficients number type
  using Rat_coercion_type = typename Coercion::Type;

  //! base number type for all internal computations
  using NT = typename Renderer_traits::Float;

  //! instance of a univariate polynomial
  using Poly_1 = CGAL::Polynomial<Coeff>;

  //! instance of a bivariate polynomial
  using Poly_2 = CGAL::Polynomial<Poly_1>;

  //! container's const iterator (random access)
  using const_iterator_1 = typename Poly_1::const_iterator;

  //! container's const iterator (random access)
  using const_iterator_2 = typename Poly_2::const_iterator;

  //! a univariate rational polynomial
  using Rational_poly_1 = CGAL::Polynomial<Rat_coercion_type>;

  //! a bivariate rational polynomial
  using Rational_poly_2 = CGAL::Polynomial<Rational_poly_1>;

  //! conversion from \c Rational type to used number type
  typename Renderer_traits::Rat_to_float m_rat2float;

  //! conversion from the basic NT to \c Rational
  typename Renderer_traits::Float_to_rat m_float2rat;

  //! makes the result exact after inexact operation (applicable only for exact number types
  typename Renderer_traits::Make_exact m_make_exact;

  //!@}
private:
  //!\name private typedefs
  //!@{

  //! container to store derivative coefficients
  using Derivative_1 = std::vector<NT>;

  //! instance of polynomial derivatives type
  using Derivative_2 = std::vector<CGAL::Polynomial<NT>>;

  //! const_iterator_2 for polynomial derivatives
  using der_iterator_2 = typename Derivative_2::const_iterator;

  //! const_iterator_1 for polynomial derivatives
  using der_iterator_1 = typename std::vector<NT>::const_iterator;

  //! hashed container element's type for univariate polynomials
  using Poly_entry = std::pair<NT, Poly_1>;

  //! hash function to cache univariate polynomials
  //     struct hash_func_poly {
  //         typename Renderer_traits::Hash_function hash;
  //         std::size_t operator()(const NT& key) const {
  //             return hash(key);
  //         }
  //     };
  //
  //! hash function to cache function evaluations
  //     struct hash_func_eval {
  //         typename Renderer_traits::Hash_function hash;
  //         std::size_t operator()(const Eval_hash_key& key) const {
  //             std::size_t h1 = hash(key.first), h2 = hash(key.second);
  //             return (h1 - h2);
  //         }
  //     };

  //! hashed map container with LRU capabilities used to store precomputed
  //! univariate polynomials at fixed x or y coordinates
  //     typedef boost::multi_index::multi_index_container<
  //         Poly_entry,
  //         boost::multi_index::indexed_by<
  //             boost::multi_index::sequenced<>,
  //             boost::multi_index::hashed_unique<
  //                 BOOST_MULTI_INDEX_MEMBER(Poly_entry, NT, first),
  //                 hash_func_poly > > >
  //     Poly_cache;

  //! hashed map used to store precomputed polynomial evaluations
  //     typedef boost::multi_index::multi_index_container<
  //         Eval_entry,
  //         boost::multi_index::indexed_by<
  //             boost::multi_index::sequenced<>,
  //             boost::multi_index::hashed_unique<
  //                 BOOST_MULTI_INDEX_MEMBER(Eval_entry, Eval_hash_key, first),
  //                 hash_func_eval > > >
  //     Eval_cache;

  //!@}
public:
  //!\name public interface
  //!@{

  //! constructs default
  Curve_renderer_internals() : m_window(0.0, 0.0, 0.0, 0.0), m_res_w(0), m_res_h(0) {}

  //! checks whether interval contains zero
  inline bool is_zero(const NT& low, const NT& up) {
    return (low * up < 0); //(low < 0&&up > 0);
  }

  //! evaluates a certain polynomial derivative at x
  //!
  //! \c der_coeffs is a set of derivative coefficients,
  //! \c poly - polynomial coefficients
  NT evaluate_der(const CGAL::Polynomial<NT>& der_coeffs, const Poly_1& poly, const NT& x) {
    typename Renderer_traits::Extract_eval extract;
    const_iterator_1 poly_it = poly.end() - 1;
    der_iterator_1 der_it = der_coeffs.end() - 1;
    NT y(extract(*poly_it--) * (*der_it));
    while (der_it != der_coeffs.begin()) {
      --der_it;
      y = y * x + extract(*poly_it--) * (*der_it);
    }
    return y;
  }

  //! evaluates a polynomial at certain x-coordinate
  static NT evaluate(const Poly_1& poly, const NT& x, bool* error_bounds_ = nullptr) {
    typename Renderer_traits::Extract_eval extract;
    int n = poly.degree() + 1;
    int m = (n - 1) >> 1;
    int odd = n & 1;
    if (error_bounds_ != nullptr) *error_bounds_ = false;
    if (n == 1) return extract(poly.lcoeff(), error_bounds_);
    Coeff cc = static_cast<Coeff>(x);
    const_iterator_1 it1 = poly.end() - 1;
    const_iterator_1 it2 = it1 - (n >> 1);
    // const_iterator_1 beg = poly.begin()+odd;
    Coeff y1 = *it1;
    Coeff y2 = *it2;
    Coeff mul = cc;
    Coeff y;
    // unrolled loop for better instruction pairing
    while (m-- > odd) {
      it1--;
      it2--;
      y1 = y1 * cc + (*it1);
      y2 = y2 * cc + (*it2);
      mul = mul * cc;
    }
    y = y2 + y1 * mul;
    if (odd) y = poly[0] + y * cc;
    //Gfx_OUT("eval results x = " << x << " res: [" << y.lower() <<
    //  "; " << y.upper() << "\n");

    return extract(y, error_bounds_);
  }

  //! evaluates polynomial at a point (x; y)
  //!
  //! y is the outermost variable, x is the innermost
  static Rat_coercion_type substitute_xy(const Rational_poly_2& poly, const Rational& x, const Rational& y) {
    typename Rational_poly_2::const_iterator rit = poly.end() - 1;
    Rat_coercion_type r = rit->evaluate(x);
    Rat_coercion_type yc = typename Coercion::Cast()(y);

    while (rit != poly.begin()) {
      --rit;
      r = r * yc + rit->evaluate(x);
    }
    return r;
  }

  //! destructs
  ~Curve_renderer_internals() { clear_caches(); }

  //!@}
public:
  //!\name public data (can be accessed from hosting curve renderer)
  //!@{

  CGAL::Bbox_2 m_window; //! drawing window
  NT m_x_min;
  NT m_x_max;
  NT m_y_min;
  NT m_y_max;
  //! pixel dimensions w.r.t. resolution
  NT m_pixel_w;
  NT m_pixel_h;

  //! rational versions
  Rational m_x_min_r;
  Rational m_y_min_r;
  Rational m_x_max_r;
  Rational m_y_max_r;
  Rational m_pixel_w_r;
  Rational m_pixel_h_r;

  //! pixel resolution
  int m_res_w;
  int m_res_h;

  Poly_2 *m_coeffs_x, *m_coeffs_y; //! f(x(y)) / f(y(x))
  Derivative_2 *m_der_x, *m_der_y; //! derivative coefficients df/dx (df/dy)

  //! caches of precomputed univariate polynomials in x and y directions
  //Poly_cache *cached_x, *cached_y;

  Rational_poly_2 *m_rational_x, *m_rational_y; //! poly with rational coeffs

  Rational_poly_2 *m_rational_fx, *m_rational_fy; //! partial derivatives

  //! 0 - the 1st (2nd) derivative does not have zero over an
  //! interval; 1 - does have 0; -1 - not computed
  int m_first_der;
  int m_second_der;
  bool m_sign_change; //! detected a sign change over interval

  bool m_zero_bounds; //! indicates that the result of the last range
                      //! evaluation has at least one zero boundary

  // static bool show_dump;    //! for debugging

  //!@}
private:
  //!\name private members for caching support
  //!@{

  //! data structures grouped into arrays to facilitate caching
  Poly_2 m_coeffs_x_[CGAL_N_CACHES];
  Poly_2 m_coeffs_y_[CGAL_N_CACHES];
  Derivative_2 m_der_x_[CGAL_N_CACHES];
  Derivative_2 m_der_y_[CGAL_N_CACHES];
  //Poly_cache cached_x_[CGAL_N_CACHES], cached_y_[CGAL_N_CACHES];
  Rational_poly_2 m_rational_x_[CGAL_N_CACHES];
  Rational_poly_2 m_rational_y_[CGAL_N_CACHES];
  Rational_poly_2 m_rational_fx_[CGAL_N_CACHES];
  Rational_poly_2 m_rational_fy_[CGAL_N_CACHES];

  //!@}
public:
  //! \name public methods
  //!@{

  // template <class CurveKernel_2, class Coeff_>
  // bool Curve_renderer_internals<CurveKernel_2, Coeff_>::show_dump = false;

  //! sets up drawing window and pixel resolution
  //!
  //! returns \c false if parameters are incorrect
  bool setup(const CGAL::Bbox_2& box_, int res_w_, int res_h_) {
    m_window = box_;
    m_x_min = static_cast<NT>(box_.xmin());
    m_y_min = static_cast<NT>(box_.ymin());
    m_x_max = static_cast<NT>(box_.xmax());
    m_y_max = static_cast<NT>(box_.ymax());

    m_res_w = res_w_;
    m_res_h = res_h_;

    if (m_x_min >= m_x_max) {
      std::cerr << "Incorrect setup parameters: "
                << "x_min = " << m_x_min << " >= " << m_x_max << " = x_max" << std::endl;
      return false;
    }
    if (m_y_min >= m_y_max) {
      std::cerr << "Incorrect setup parameters: "
                << "y_min = " << m_y_min << " >= " << m_y_max << " = y_max" << std::endl;
      return false;
    }
    if (m_res_w < 4) {
      std::cerr << "Incorrect setup parameters: " << "res_w = " << m_res_w << " < 4" << std::endl;
      return false;
    }
    if (m_res_h < 4) {
      std::cerr << "Incorrect setup parameters: " << "res_h = " << m_res_h << " < 4" << std::endl;
      return false;
    }
    if (m_res_w > 2048) {
      std::cerr << "Incorrect setup parameters: " << "res_w = " << m_res_w << " > 2048" << std::endl;
      return false;
    }
    if (m_res_h > 2048) {
      std::cerr << "Incorrect setup parameters: " << "res_h = " << m_res_h << " > 2048" << std::endl;
      return false;
    }

    m_pixel_w = (m_x_max - m_x_min) / m_res_w;
    m_pixel_h = (m_y_max - m_y_min) / m_res_h;

    //srand(time(nullptr));
    // from 0.1 to 0.5
    double rmin = 0.1;
    double rmax = 0.5;
    NT sx = m_pixel_w * static_cast<NT>(rmin + (static_cast<double>(rand()) / RAND_MAX) * (rmax - rmin));
    NT sy = m_pixel_h * static_cast<NT>(rmin + (static_cast<double>(rand()) / RAND_MAX) * (rmax - rmin));

    Gfx_OUT("---------------------- sx: " << (sx / m_pixel_w) << "; sy: " << (sy / m_pixel_h) << "\n");
    m_x_min += sx, m_x_max += sx, m_y_min += sy, m_y_max += sy;

    m_x_min_r = Rational(m_x_min), m_y_min_r = Rational(m_y_min);
    m_x_max_r = Rational(m_x_max), m_y_max_r = Rational(m_y_max);

    m_pixel_w_r = (m_x_max_r - m_x_min_r) / m_res_w;
    m_pixel_h_r = (m_y_max_r - m_y_min_r) / m_res_h;

    m_pixel_w = m_rat2float(m_pixel_w_r);
    m_pixel_h = m_rat2float(m_pixel_h_r);
    m_make_exact(m_pixel_w);
    m_make_exact(m_pixel_h);

    //show_dump = false;
    return true;
  }

  //! computes a range of polynomial values \c f([lower,upper]) using
  //! the First Affine Form (AF1) with Recursive Derivative information
  bool get_range_AF1_1(int var, const NT& l_, const NT& r_, const NT& key, const Poly_1& poly, int check) {
    Derivative_2* der = (var == CGAL_X_RANGE ? m_der_x : m_der_y);
    NT l(l_);
    NT r(r_);
    NT l1;
    NT r1;
    NT low;
    NT up;
    NT v1;
    NT v2;
    int eval1;
    int eval2;
    der_iterator_2 der_it_2 = der->end() - 1;
    der_iterator_1 der_it;
    der_iterator_1 der_begin;
    const_iterator_1 cache_it;
    const_iterator_1 begin;

    m_first_der = false, m_sign_change = false;
    if (poly.degree() == 0) {
      m_zero_bounds = false;
      return (poly.lcoeff() == NT(0.0));
    }

    if (l_ == r_) {
      m_zero_bounds = false;
      return false;
    }
    if (l > r) {
      l = r_;
      r = l_;
    }

    eval1 = evaluate_generic(var, l, key, poly);
    eval2 = evaluate_generic(var, r, key, poly);
    bool sign_change = (eval1 * eval2 < 0);
    m_zero_bounds = ((eval1 & eval2) == 0);

    if ((sign_change || m_zero_bounds) && check == 1) return true;
    if (var == CGAL_X_RANGE) {
      l1 = m_x_min + l * m_pixel_w;
      r1 = m_x_min + r * m_pixel_w;
    }
    else {
      l1 = m_y_min + l * m_pixel_h;
      r1 = m_y_min + r * m_pixel_h;
    }

    typename Renderer_traits::Extract_eval extract;
    unsigned index = CGAL_RECURSIVE_DER_MAX_DEGREE;
    if (index >= der->size()) {
      low = up = extract(poly.lcoeff()) * (*der_it_2).lcoeff();
    }
    else {
      der_it_2 = der->begin() + index;
      low = 1;
      up = -1;
    }

    NT x0 = (l1 + r1) / 2;
    NT x1 = (r1 - l1) / 2;
    m_make_exact(x0);
    m_make_exact(x1);
    NT x0_abs = CGAL_ABS(x0);
    NT x1_abs = CGAL_ABS(x1);

    while (der_it_2 != der->begin()) {
      --der_it_2;
      // iterate through derivative coefficients
      der_it = der_it_2->end() - 1;
      der_begin = der_it_2->begin();
      cache_it = poly.end() - 1; // iterate through precomputed y-values
      // if a derivative does not straddle zero we can
      // calculate the exact boundaries for f(x)
      if (low * up >= 0) {
        v1 = v2 = extract(*cache_it--) * (*der_it);
        // calculate the ith derivative at xa and xb
        while (der_it != der_begin) {
          --der_it;

          NT cc1 = extract(*cache_it--) * (*der_it);
          v1 = v1 * l1 + cc1;
          v2 = v2 * r1 + cc1;
        }
        low = v1, up = v2;
      }
      else { // use affine arithmetic to compute bounds

        NT y0 = extract(*cache_it) * (*der_it);
        NT y1(0);
        NT e1(0);
        cache_it--;
        while (der_it != der_begin) {
          --der_it;

          e1 = x0_abs * e1 + x1_abs * (CGAL_ABS(y1) + CGAL_ABS(e1));
          y1 = y1 * x0 + x1 * y0;
          y0 = x0 * y0 + extract(*cache_it) * (*der_it);
          cache_it--;
        }
        NT spread = CGAL_ABS(y1) + CGAL_ABS(e1);
        low = y0 - spread, up = y0 + spread;
      }
      if (der_it_2 == der->begin() && check == 3) {
        m_first_der = //(eval1*eval2 < 0);
          is_zero(low, up);
        if (sign_change || m_zero_bounds) return true;
      }
    }
    if (low * up < 0) {
      cache_it = poly.end() - 1;
      begin = poly.begin();

      NT y0 = extract(*cache_it);
      NT y1(0);
      NT e1(0);
      while (cache_it != begin) {
        --cache_it;
        e1 = x0_abs * e1 + x1_abs * (CGAL_ABS(y1) + CGAL_ABS(e1));
        y1 = y1 * x0 + x1 * y0;
        y0 = x0 * y0 + extract(*cache_it);
      }

      NT spread = CGAL_ABS(y1) + CGAL_ABS(e1);
      low = y0 - spread, up = y0 + spread;

      eval1 = CGAL_SGN(low), eval2 = CGAL_SGN(up);
    }
    m_zero_bounds = ((eval1 & eval2) == 0);
    if (eval1 * eval2 < 0) {
      //std::cerr << "eval1 = " << low << "; eval2 = " << up << std::endl;
      return true;
    }
    return false;
  }

  //! evaluates a univariate polynomial over an interval using
  //! First Quadratic Affine Form
  bool get_range_QF_1(int var, const NT& l_, const NT& r_, const NT& key, const Poly_1& poly, int check = 1) {
    Derivative_2* der = (var == CGAL_X_RANGE ? m_der_x : m_der_y);
    NT l(l_);
    NT r(r_);
    NT l1;
    NT r1;
    NT low;
    NT up;
    NT v1;
    NT v2;
    int eval1;
    int eval2;
    der_iterator_2 der_it_2 = der->end() - 1;
    der_iterator_1 der_it;
    der_iterator_1 der_begin;
    const_iterator_1 cache_it;
    const_iterator_1 begin;

    m_first_der = false, m_sign_change = false;
    if (poly.degree() == 0) {
      m_zero_bounds = false;
      return (poly.lcoeff() == NT(0.0));
    }
    if (l_ == r_) {
      m_zero_bounds = false;
      return false;
    }
    if (l > r) {
      l = r_;
      r = l_;
    }

    eval1 = evaluate_generic(var, l, key, poly);
    eval2 = evaluate_generic(var, r, key, poly);
    m_sign_change = (eval1 * eval2 < 0);

    m_zero_bounds = ((eval1 & eval2) == 0);
    if ((m_sign_change || m_zero_bounds) && check == 1) return true;

    if (var == CGAL_X_RANGE) {
      l1 = m_x_min + l * m_pixel_w;
      r1 = m_x_min + r * m_pixel_w;
    }
    else {
      l1 = m_y_min + l * m_pixel_h;
      r1 = m_y_min + r * m_pixel_h;
    }

    typename Renderer_traits::Extract_eval extract;
    unsigned index = CGAL_RECURSIVE_DER_MAX_DEGREE;
    if (index >= der->size()) {
      low = up = extract(poly.lcoeff()) * (*der_it_2).lcoeff();
    }
    else {
      der_it_2 = der->begin() + index;
      low = 1;
      up = -1;
    }

    NT x0 = (l1 + r1) / 2;
    NT x1 = (r1 - l1) / 2;
    m_make_exact(x0);
    m_make_exact(x1);
    NT x0_abs = CGAL_ABS(x0);
    NT xsum_abs = CGAL_ABS(x1) + x0_abs;

    while (der_it_2 != der->begin()) {
      --der_it_2;
      // iterate through derivative coefficients
      der_it = (*der_it_2).end() - 1;
      der_begin = (*der_it_2).begin();
      cache_it = poly.end() - 1; // iterate through precomputed y-values

      // if a derivative does not straddle zero we can obtain exact bounds
      // by evaluating a polynomial at end-points
      if (low * up >= 0) {
        v1 = v2 = extract(*cache_it--) * (*der_it);
        // calculate the ith derivative at xa and xb
        while (der_it != der_begin) {
          --der_it;

          NT cc1 = extract(*cache_it--) * (*der_it);
          v1 = v1 * l1 + cc1;
          v2 = v2 * r1 + cc1;
        }
        low = v1;
        up = v2;
      }
      else { // use Quadratic Form to compute the bounds

        NT y0 = extract(*cache_it) * (*der_it);
        NT y1(0);
        NT z1(0);
        NT e1(0);
        cache_it--;
        while (der_it != der_begin) {
          --der_it;

          e1 = xsum_abs * e1 + CGAL_ABS(NT(x1 * z1));
          z1 = x0 * z1 + x1 * y1;
          y1 = y1 * x0 + x1 * y0;
          y0 = x0 * y0 + extract(*cache_it) * (*der_it);
          cache_it--;
        }
        NT spread = CGAL_ABS(y1) + e1;
        low = spread;
        up = spread;
        if (z1 > 0)
          up = up + z1;
        else
          low = low - z1;
        low = y0 - low;
        up = y0 + up;
      }

      if (der_it_2 == der->begin() && check == 3) {
        m_first_der = //(eval1*eval2 < 0);//
          is_zero(low, up);
        if (m_sign_change || m_zero_bounds) return true;
      }
    }

    if (low * up < 0) {
      cache_it = poly.end() - 1;
      begin = poly.begin();
      NT y0 = extract(*cache_it);
      NT y1(0);
      NT z1(0);
      NT e1(0);
      while (cache_it != begin) {
        --cache_it;
        e1 = xsum_abs * e1 + CGAL_ABS(NT(x1 * z1));
        z1 = x0 * z1 + x1 * y1;
        y1 = y1 * x0 + x1 * y0;
        y0 = x0 * y0 + extract(*cache_it);
      }
      NT spread = CGAL_ABS(y1) + e1;
      low = spread;
      up = spread;
      if (z1 > 0)
        up = up + z1;
      else
        low = low - z1;
      low = y0 - low;
      up = y0 + up;
      eval1 = CGAL_SGN(low);
      eval2 = CGAL_SGN(up);
    }
    //if(eval1*eval2 < 0)
    //  std::cerr << "eval1 = " << low << "; eval2 = " << up << std::endl;
    m_zero_bounds = ((eval1 & eval2) == 0);
    return (eval1 * eval2 < 0);
  }

  //! evaluates a univariate polynomial over an interval using
  //! Modified Affine Arithmetic
  bool get_range_MAA_1(int var, const NT& l_, const NT& r_, const NT& key, const Poly_1& poly, int check = 1) {
    Derivative_2* der = (var == CGAL_X_RANGE) ? m_der_x : m_der_y;
    // stores precomputed polynomial derivatives and binomial coeffs
    Derivative_1 der_cache; //(der->size()+1, NT(0))
    Derivative_1 binom;     //(der->size()+1, NT(0));
    NT l(l_);
    NT r(r_);
    NT l1;
    NT r1;
    NT low = NT(1);
    NT up = NT(-1);
    NT v1;
    NT v2;
    NT v;
    int eval1;
    int eval2;

    m_first_der = false, m_sign_change = false;
    if (poly.degree() == 0) {
      m_zero_bounds = false;
      return (poly.lcoeff() == NT(0.0));
    }
    if (l_ == r_) {
      m_first_der = false;
      return false;
    }
    if (l > r) {
      l = r_;
      r = l_;
    }
    eval1 = evaluate_generic(var, l, key, poly);
    eval2 = evaluate_generic(var, r, key, poly);

    m_sign_change = (eval1 * eval2 < 0);
    m_zero_bounds = ((eval1 & eval2) == 0);

    if ((m_sign_change || m_zero_bounds) && check == 1) return true;

    if (var == CGAL_X_RANGE) {
      l1 = m_x_min + l * m_pixel_w;
      r1 = m_x_min + r * m_pixel_w;
    }
    else {
      l1 = m_y_min + l * m_pixel_h;
      r1 = m_y_min + r * m_pixel_h;
    }
    NT x0 = (l1 + r1) / 2;
    NT x1 = (r1 - l1) / 2;
    m_make_exact(x0);
    m_make_exact(x1);

    int d = 0;
    v1 = evaluate(poly, x0);
    //der_cache[d] = v1;
    der_cache.push_back(v1);
    v = x1, d++;
    der_iterator_2 der_it_2;
    for (der_it_2 = der->begin(); der_it_2 != der->end(); der_it_2++) {
      v1 = evaluate_der((*der_it_2), poly, x0);
      der_cache.push_back(v1);
      //der_cache[d] = v1; // replace by push_backs ?
      //binom[d] = v;
      binom.push_back(v);
      d++;
      //v *= x1/d;
      //make_exact(v);
      v *= x1; // ONLY when derivative coefficients are normalized
    }

    typename Renderer_traits::Extract_eval extract;
    unsigned index = CGAL_RECURSIVE_DER_MAX_DEGREE;
    if (index >= der->size()) {
      low = up = extract(poly.lcoeff()) * (der->end() - 1)->lcoeff();
    }
    else {
      der_it_2 = der->begin() + index;
      low = 1;
      up = -1;
    }
    // assume we have an array of derivatives:
    // der_cache: {f^(0); f^(1); f^(2); ...}
    // and binomial coefficients: [h; h^2/2; h^3/6; ... h^d/d!]
    der_iterator_1 eval_it = der_cache.end() - 1;
    der_iterator_1 local_it;
    der_iterator_1 binom_it;
    der_iterator_1 eval_end = der_cache.end();
    d = poly.degree();
    der_it_2 = der->end() - 1;
    while (1) { //der_it!=end) {
      if (low * up < 0) {
        v2 = *eval_it;
        local_it = eval_it + 1;
        binom_it = binom.begin();
        low = v2, up = v2;
        while ((local_it) != eval_end) { // calculate derivative bounds
          v1 = (*local_it++) * (*binom_it++);
          if (v1 >= 0) { // derivative index is odd
            up += v1;
            low -= v1;
          }
          else {
            up -= v1;
            low += v1;
          }
          if (local_it == eval_end) break;
          // derivative index is even
          v1 = (*local_it++) * (*binom_it++);
          (v1 > 0) ? up += v1 : low += v1;
        }
        eval1 = CGAL_SGN(low);
        eval2 = CGAL_SGN(up);
      }
      else if (d > 0) {
        low = evaluate_der((*der_it_2), poly, l1);
        up = evaluate_der((*der_it_2), poly, r1);
        eval1 = CGAL_SGN(low);
        eval2 = CGAL_SGN(up);
      }
      else if (d == 0) {
        ; //Gfx_DETAILED_OUT("MAA bounds: sign change\n");
      }

      if (d == 1 && check == 3) {
        m_first_der = (eval1 * eval2 < 0);
        if (m_sign_change || m_zero_bounds) return true;
      }
      else if (d == 0) {
        m_zero_bounds = ((eval1 & eval2) == 0);
        return (eval1 * eval2 < 0);
      }
      d--;
      der_it_2--;
      if (eval_it == der_cache.begin()) break;
      eval_it--;
    }
    return true;
  }

  //! fixes one coordinate of a bivariate polynomial, uses caching
  //! if appropriate
  void get_precached_poly(int var, const NT& key, int /* level */, Poly_1& poly) {
    NT key1;
    //Poly_cache *cached = cached_x;
    Poly_2* coeffs = m_coeffs_x;
    //typename boost::multi_index::nth_index_iterator<Poly_cache,1>::type it;
    bool not_cached = true; /*(level >= CGAL_MAX_POLY_CACHE_LEVEL)*/
    bool not_found = false;

    if (var == CGAL_Y_RANGE) {
      //cached = cached_y;
      coeffs = m_coeffs_y;
      key1 = m_x_min + key * m_pixel_w;
    }
    else
      key1 = m_y_min + key * m_pixel_h;

    //     if(!not_cached) {
    //         typename boost::multi_index::nth_index<Poly_cache,1>::type&
    //             idx = cached->get<1>();
    //         it = idx.find(key1); //*4
    //         not_found = (it == idx.end());
    //     }

    if (not_cached || not_found) {
      auto fn = [&key1](const Poly_1& poly) { return evaluate(poly, key1); };
      poly = Poly_1(::boost::make_transform_iterator(coeffs->begin(), fn),
                    ::boost::make_transform_iterator(coeffs->end(), fn));
      if (not_cached) return;
      // all available space consumed: drop the least recently used entry
      //         if(cached->size() >= CGAL_POLY_CACHE_SIZE)
      //             cached->pop_back();
      //         cached->push_front(Poly_entry(key1,poly)); //*4
      //         return;
    }
    //     cached->relocate(cached->begin(), cached->project<0>(it));
    //     poly = (*it).second;
  }

  //! computes the sign of univariate polynomial at (x; y).
  //!
  //! if computation with the current precision is not reliable, the sign is
  //! recomputed with modular or exact rational arithmetic
  int evaluate_generic(int var, const NT& c, const NT& key, const Poly_1& poly) {
    NT c1;

    c1 = (var == CGAL_X_RANGE) ? m_x_min + c * m_pixel_w : m_y_min + c * m_pixel_h;

    bool error_bounds_;
    int sign;
    NT res = evaluate(poly, c1, &error_bounds_);

    sign = (error_bounds_) ? evaluate_rational(var, c, key) : CGAL_SGN(res);

    return sign;
  }

  //! evaluates a polynomial at (x, y) using exact rational arithmetic
  int evaluate_rational(int var, const NT& c, const NT& key) {
    //Rational_poly_1 poly;
    Rational c_r = m_float2rat(c);
    Rational key_r = m_float2rat(key);
    c_r = (var == CGAL_X_RANGE) ? (m_x_min_r + c_r * m_pixel_w_r) : (m_y_min_r + c_r * m_pixel_h_r);

    Rational_poly_2* rat = m_rational_x;
    if (var == CGAL_Y_RANGE) {
      rat = m_rational_y;
      key_r = m_x_min_r + key_r * m_pixel_w_r;
    }
    else
      key_r = m_y_min_r + key_r * m_pixel_h_r;

    Rat_coercion_type res = substitute_xy(*rat, key_r, c_r);
    return CGAL_SGN(res);
  }

  //! precomputes polynomials and derivative coefficients
  void precompute(const Polynomial_2& in) {
    using Coeff_src = typename Polynomial_traits_2::Innermost_coefficient_type;

    Max_coeff<Coeff_src> max_coeff;
    Coeff_src max_c = max_coeff(in);
    /////// magic symbol ////////////
    // somehow relates to double precision fix
    // std::cerr << ' ';

    using Reduce_op = Reduce_by<Rat_coercion_type, Coeff_src>;
    Transform<Rational_poly_2, Polynomial_2, Reduce_op> transform;
    Reduce_op op(max_c);

    using RP_traits = CGAL::Polynomial_traits_d<Rational_poly_2>;
    *m_rational_y = transform(in, op); // divides by maximal coefficient
    *m_rational_x = typename RP_traits::Swap()(*m_rational_y, 0, 1);

    // rational fx and fy must have y outermost var and x innermost
    *m_rational_fx = typename RP_traits::Differentiate()(*m_rational_y, 0);
    *m_rational_fy = typename RP_traits::Differentiate()(*m_rational_y, 1);

    typename Renderer_traits::Convert_poly convert_poly;
    ////////////////////////////////////////////////////////
    /////// ATTENTION: need to call makeExact for bigfloats after conversion
    ////////////////////////////////////////////////////////
    *m_coeffs_y = convert_poly(*m_rational_y);
    *m_coeffs_x = typename CGAL::Polynomial_traits_d<Poly_2>::Swap()(*m_coeffs_y, 0, 1);

    int degree_x = m_coeffs_x->degree();
    int degree_y = m_coeffs_y->degree();
    /*    cached_x->clear();
    cached_y->clear(); */
    m_der_x->clear();
    m_der_y->clear();
    int i;
    int j;
    int maxdeg = (degree_x > degree_y ? degree_x : degree_y);
    std::vector<NT> X(maxdeg, NT(0));
    m_der_x->reserve(degree_x);
    m_der_y->reserve(degree_y);

    NT det(1.0);
    for (i = 0; i < degree_x; i++) {
      if (i != 0) det = X[0];
      for (j = 1; j <= degree_x - i; j++) {
        // divide by the lowest coefficient ?
        X[j - 1] = (i == 0 ? NT(j) : X[j] * NT(j) / det);
        m_make_exact(X[j - 1]);
      }
      m_der_x->push_back(CGAL::Polynomial<NT>(X.begin(), (X.begin() + degree_x - i)));
    }

    det = NT(1.0);
    for (i = 0; i < degree_y; i++) {
      if (i != 0) det = X[0];
      for (j = 1; j <= degree_y - i; j++) {
        // divide by the lowest coefficient ?
        X[j - 1] = (i == 0 ? NT(j) : X[j] * NT(j) / det);
        m_make_exact(X[j - 1]);
      }
      m_der_y->push_back(CGAL::Polynomial<NT>(X.begin(), (X.begin() + degree_y - i)));
    }
  }

  //! activates the cache entry \c cache_id
  void select_cache_entry(std::size_t cache_id) {
    m_coeffs_x = m_coeffs_x_ + cache_id;
    m_coeffs_y = m_coeffs_y_ + cache_id;
    m_rational_x = m_rational_x_ + cache_id;
    m_rational_y = m_rational_y_ + cache_id;
    m_rational_fx = m_rational_fx_ + cache_id;
    m_rational_fy = m_rational_fy_ + cache_id;
    m_der_x = m_der_x_ + cache_id;
    m_der_y = m_der_y_ + cache_id;
    /*    cached_x = cached_x_ + cache_id;
    cached_y = cached_y_ + cache_id;*/
  }

  //! empties all cache instances
  void clear_caches() {
    for (unsigned i = 0; i < CGAL_N_CACHES; i++) {
      /*        cached_x_[i].clear();
        cached_y_[i].clear(); */
      m_der_x_[i].clear();
      m_der_y_[i].clear();
    }
  }

  //!@}

}; // Curve_renderer_internals

} // namespace internal
} // namespace CGAL

#endif
