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

/*! \file CGAL/Curved_kernel_via_analysis_2/gfx/Curve_renderer_2.h
 * definition of Curve_renderer_2<>
 * rasterization of algebraic curves
 */

#ifndef CGAL_CKVA_CURVE_RENDERER_2_H
#define CGAL_CKVA_CURVE_RENDERER_2_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#ifndef CGAL_AK_ENABLE_DEPRECATED_INTERFACE
#define CGAL_AK_ENABLE_DEPRECATED_INTERFACE 1
#endif

#include <vector>
#include <stack>
#include <cmath>

#include <CGAL/Polynomial.h>
#include <CGAL/Interval_nt.h>
#include <CGAL/Arithmetic_kernel.h>
#include <CGAL/Algebraic_kernel_d/Bitstream_descartes.h>
#include <CGAL/Algebraic_kernel_d/Bitstream_descartes_rndl_tree_traits.h>
#include <CGAL/Algebraic_kernel_d/Bitstream_coefficient_kernel.h>
#include <CGAL/Curved_kernel_via_analysis_2/gfx/Curve_renderer_internals.h>
#include <CGAL/Curved_kernel_via_analysis_2/gfx/Curve_renderer_traits.h>

// using boost::multi_index::multi_index_container;
// using boost::multi_index::get;
// using boost::multi_index::project;

#ifdef CGAL_CKVA_CR_TIMING
extern CGAL::Timer refine_timer;
#endif

namespace CGAL {

#ifndef CGAL_CURVE_RENDERER_DEFS
#define CGAL_CURVE_RENDERER_DEFS

// subdivision level beyond which visibly coincide branches are not further
// discriminated
#define CGAL_COINCIDE_LEVEL 4

// maximal recursion depth for derivative check
#define CGAL_DER_CHECK_DEPTH 5

// # of pixels to enlarge the drawing window in y-direction: required to skip
// closely located clip-points
#define CGAL_WINDOW_ENLARGE 15

// refine factor for intervals in x-direction (in pixel size)
#define CGAL_REFINE_X 100 // was 1000 previously

// refine factor for intervals in y-direction (in pixel size)
#define CGAL_REFINE_Y 100000

// refine factor for clip-points
#define CGAL_REFINE_CLIP_POINTS 1000

// refine threshold for double point approximation
#define CGAL_REFINE_DOUBLE_APPROX 1e-10

#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
//!@note: points are drawn in any case even if they are outside the window
//#warning approximation hangs for CORE::BigFloat !!
#define CGAL_CKVA_STORE_COORDS(container, pixel)                                                                       \
  if (! std::isnan(pixel.xv)) container.push_back(Coord_2(pixel.xv, pixel.yv));
#else
#define CGAL_CKVA_STORE_COORDS(container, pixel) container.push_back(Coord_2(pixel.m_x, pixel.m_y));
#endif

#endif // CGAL_CURVE_RENDERER_DEFS

namespace {
// map from box sides to subpixel numbers (for h/v directions)
// these are old versions
static const int HV_SUBPIX_MAP[][4] = {
  {1, 3, 0, 2}, // right(0)
  {3, 2, 1, 0}, // top(1)
  {2, 0, 3, 1}, // left(2)
  {0, 1, 2, 3}  // bottom(3)
};

static const int D_SUBPIX_MAP[][4] = {
  {3, 1, 2, 0}, // dir = 0
  {2, 3, 0, 1}, // dir = 1
  {0, 2, 1, 3}, // dir = 2
  {1, 0, 3, 2}  // dir = 3
};
} // anonymous namespace

/*! the class template \c Curve_renderer_2 and its associate functions.
 *
 * The class implements rendering of distinct curve arcs and points as
 * defined by \c CurvedKernelViaAnalysis_2::Arc_2 and
 * \c CurvedKernelViaAnalysis_2::Point_2. \c Coeff_ template parameter
 * defines an underlying number type to be used in polynomial and range
 * evaluations. Valid instantiations are \c double, multi-precision float or
 * exact rational number type. The main float-point number type is defined by
 * \c Curve_renderer_traits<Coeff_>::Float.
 */
template <typename CurvedKernelViaAnalysis_2, typename Coeff_>
class Curve_renderer_2 {
public:
  //! \name public typedefs
  //!@{

  //! this instance's first template argument
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! this instance's second template argument
  using Coeff = Coeff_;

  //! type of embedded curve kernel
  using Curve_kernel_2 = typename Curved_kernel_via_analysis_2::Curve_kernel_2;

  //! type of x-monotone arc
  using Arc_2 = typename Curved_kernel_via_analysis_2::X_monotone_curve_2;

  //! type of a point on curve
  using Point_2 = typename Curved_kernel_via_analysis_2::Point_2;

  //! type of 1-curve analysis
  using Curve_analysis_2 = typename Curve_kernel_2::Curve_analysis_2;

  //!@}
private:
  //! \name private typedefs
  //!@{

  //! type of X_coordinate
  using Coordinate_1 = typename Curve_kernel_2::Coordinate_1;

  //! type of xy-coordinate ? ;)
  using Coordinate_2 = typename Curve_kernel_2::Coordinate_2;

  //! rendering low-level routine
  using Renderer_internals = internal::Curve_renderer_internals<Curve_kernel_2, Coeff>;

  //! exact rational number type
  using Rational = typename Renderer_internals::Rational;

  /// polynomial traits should be used whenever possible
  using Polynomial_traits_2 = typename Renderer_internals::Polynomial_traits_2;

  //! curve renderer type conversion traits
  using Renderer_traits = typename Renderer_internals::Renderer_traits;

  //! coercion between rational and polynomial coefficients number type
  using Rat_coercion_type = typename Renderer_internals::Rat_coercion_type;

  //! approximate y coordinates
  using Approximate_absolute_y_2 = typename Curve_kernel_2::Approximate_absolute_y_2;

  //! bound number type for approximate
  using Bounds = typename Approximate_absolute_y_2::result_type;

  //! specialized integer number type
  using Integer = typename Renderer_traits::Integer;

  //! event line instance type
  using Status_line_1 = typename Curve_analysis_2::Status_line_1;

  //! underlying bivariate polynomial type
  using Polynomial_2 = typename Renderer_internals::Polynomial_2;

  //! underlying univariate polynomial type
  using Poly_dst_1 = typename Renderer_internals::Poly_dst_1;

  //! basic number type used in all computations
  using NT = typename Renderer_internals::NT;

  //! instance of a univariate polynomial
  using Poly_1 = typename Renderer_internals::Poly_1;

  //! instance of a bivariate polynomial
  using Poly_2 = typename Renderer_internals::Poly_2;

  //! conversion from the basic number type to doubles
  typename CGAL::Real_embeddable_traits<NT>::To_double m_to_double;

  //! conversion from the basic number type to integers
  typename Renderer_traits::Rat_to_integer m_rat2integer;

  //! conversion from \c Integer type to built-in integer
  typename Renderer_traits::Float_to_int m_float2int;

  //! conversion from \c Rational type to used number type
  typename Renderer_traits::Rat_to_float m_rat2float;

  //! makes the result exact after inexact operation (applicable only for exact number types
  typename Renderer_traits::Make_exact m_make_exact;

  //! returns \c true when the precision limit for a specified number type is reached
  typename Renderer_traits::Precision_limit m_limit;

  //! maximum level of subdivision depending on specified number type
  static const unsigned s_max_subdivision_level = Renderer_traits::s_max_subdivision_level;

  //! pixel instance type
  using Pixel_2 = internal::Pixel_2_<Integer>;

  //! seed point instance type
  using Seed_point = internal::Seed_point_<Integer>;

  //! support for multiple seed points
  using Seed_stack = std::stack<Seed_point>;

  //! a range of x-coordinates to define bottom/top clip points
  using Clip_point_entry = internal::Clip_point_<Rational, Coordinate_1>;

  //! a container of bottom/top clip points
  using Clip_points = std::vector<Clip_point_entry>;

  //! an integer index container
  using index_vector = std::vector<int>;

  //! map container element's type for maintaining a list of cache instances
  using LRU_entry = std::pair<int, std::size_t>;

  //! LRU list used for cache switching
  using Cache_list = std::vector<LRU_entry>;

  //! internal use only: a stripe defined by bounding polynomials
  //! and coordinates on sides, index 0 - lower boundary, 1 - upper boundary
  struct Stripe {
    Poly_1 m_poly[2];
    NT m_key[2];
    unsigned m_level[2];
  };

  //!@}
public:
  //! \name public methods
  //!@{

  //! constructs default; a curve segment is undefined
  Curve_renderer_2() : m_cache_id(s_invalid_cache_id), m_IA_method(0), m_initialized(false), m_one(1) {
    m_arcno = -1;
    setup(Bbox_2(-1.0, -1.0, 1.0, 1.0), 640, 480);
  }

  //! sets up drawing window and pixel resolution
  void setup(const Bbox_2& bbox_, int res_w_, int res_h_) {
    m_initialized = m_engine.setup(bbox_, res_w_, res_h_);
    m_clip_pts_computed = false; // need to recompute clip points each time the window dimensions are changed
  }

  //! returns currently used supporting algebraic curve
  Curve_analysis_2 curve() const { return *m_support; }

  //! returns the drawing window and resolution
  void get_setup_parameters(CGAL::Bbox_2* pbox, int& res_w_, int& res_h_) const {
    if (pbox != nullptr) *pbox = m_engine.m_window;
    res_w_ = m_engine.m_res_w;
    res_h_ = m_engine.m_res_h;
  }

  //! sets up preferred IA method: 0 - QF, 1 - MAA
  //! returns the old one
  int set_IA_method(int idx) {
    int tmp = m_IA_method;
    m_IA_method = idx;
    return tmp;
  }

  //! destructs
  ~Curve_renderer_2() { m_cache_list.clear(); }

  //!@}
private:
  //! \name Private methods
  //!@{

  //! advances pixel's coordinates by given increments
  void advance_pixel(Pixel_2& pix, int new_dir) {
    int x_inc = internal::directions[new_dir].m_x;
    int y_inc = internal::directions[new_dir].m_y;
    if (pix.m_level == 0) {
      pix.m_x += x_inc;
      pix.m_y += y_inc;
    }
    else {
      Integer x = pix.m_sub_x + x_inc;
      Integer y = pix.m_sub_y + y_inc;
      Integer pow = (m_one << pix.m_level) - 1;
      (x < 0 ? pix.m_x-- : x > pow ? pix.m_x++ : x);
      (y < 0 ? pix.m_y-- : y > pow ? pix.m_y++ : y);
      pix.m_sub_x = x & pow;
      pix.m_sub_y = y & pow;
    }
  }

  //! computes pixel coordinates from rational point
  void get_pixel_coords(const Rational& x, const Rational& y, Pixel_2& pix, Rational* ppix_x = nullptr,
                        Rational* ppix_y = nullptr) {
    Rational p_x = (x - m_engine.m_x_min_r) / m_engine.m_pixel_w_r;
    Rational p_y = (y - m_engine.m_y_min_r) / m_engine.m_pixel_h_r;

#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
    pix.xv = CGAL::to_double(x);
    pix.yv = CGAL::to_double(y);
#endif
    pix.m_x = static_cast<int>(std::floor(CGAL::to_double(p_x)));
    pix.m_y = static_cast<int>(std::floor(CGAL::to_double(p_y)));

    if (ppix_x != nullptr && ppix_y != nullptr) {
      *ppix_x = p_x;
      *ppix_y = p_y;
    }
  }

  //! refines y-coordinate of \c Coordinate_2 to a certain bound
  void refine_xy(const Coordinate_2& xy, const Rational& bound, Rational& low, Rational& up) {
    auto ay = xy.kernel()->approximate_absolute_y_2_object();

    Bounds bnd = ay(xy, -2); // do not approximate
    int prec = 1;
    while (bnd.second - bnd.first > bound) {
      bnd = ay(xy, prec);
      prec++;
    }
    low = bnd.first, up = bnd.second;
  }

  //!@}
private:
  //! \name Private properties
  //!@{

  unsigned m_max_level; //! maximum subdivision level
  unsigned m_current_level;

  int m_arcno; //! arcno ?

  bool m_clip_pts_computed; //! indicates whether clip points are computed

  //! isolating rectangles for lower  and
  //! upper end-points
  Pixel_2 m_isolated_l;
  Pixel_2 m_isolated_h;

  //! an instance of rendering engine
  internal::Curve_renderer_internals<Curve_kernel_2, Coeff> m_engine;

  Curve_analysis_2* m_support; //! supporting 1-curve analysis
  Curve_analysis_2 m_support_[CGAL_N_CACHES];

  //! a set of bottom and top clip points
  Clip_points m_btm_clip;
  Clip_points m_top_clip;
  //! bottom and top polynomials
  Poly_dst_1 m_btm_poly;
  Poly_dst_1 m_top_poly;

  std::size_t m_cache_id;  //! index of currently used cache instance
  Cache_list m_cache_list; //! list of indices of cache instances

  static constexpr std::size_t s_invalid_cache_id = CGAL_N_CACHES;

  Seed_stack m_seed_stack;   //! a stack of seed points
  Seed_point m_current_seed; //! current seed point

  int m_IA_method; //! which IA method to use (0 - QF, 1 - MAA)

  bool m_initialized;       //! indicates whether the renderer has been initialized
                            //! with correct parameters
  const Integer m_one;      //! just "one"
  bool m_branches_coincide; //! indicates that there are several branches
                            //! passing through one neighborhood pixel
  int m_direction_taken;    //! stores a direction taken from the seed point
                            //! during tracking, if it's possible to determine
                            //! 0 - towards lower point, 1 - towards upper
  //!@}

  //!\name public methods
  //!@{

public:
  /*! renders curve arcs (main rendering procedure)
 * returns a list of sequences of coordinates as objects of type \c Coord_2
 * which must be constructible from a pair of ints / doubles
 *
 * An exception \c Insufficient_rasterize_precision_exception is
 * thrown whenever the precision of currently used NT is not enough
 * to correctly render a curve arc. The exception has to be caught
 * outside the renderer to switch to a higher-precision arithmetic
 *
 * \c end_pt1/2 computes end-points (optionally)
 */
  template <typename Coord_2, template <typename, typename> class Container, typename Allocator>
  void draw(const Arc_2& arc, Container<std::vector<Coord_2>, Allocator>& points,
            std::optional<Coord_2>* end_pt1 = nullptr, std::optional<Coord_2>* end_pt2 = nullptr) {
#ifdef CGAL_CKVA_CR_TIMING
    refine_timer.start();
#endif

    if (! m_initialized) return;
    select_cache_entry(arc); // lookup for appropriate cache instance

    Gfx_OUT("\n////////////////////\n resolution: " << engine.res_w << " x " << m_engine.m_res_h << std::endl);
    Gfx_OUT("box: [" << m_engine.m_x_min << "; " << m_engine.m_y_min << "]x[" << m_engine.m_x_max << "; "
                     << m_engine.m_y_max << "]" << std::endl);

#ifndef CGAL_CKVA_RENDER_WITH_REFINEMENT
    Rational lower = m_engine.m_x_min_r, upper = m_engine.m_x_max_r, y_lower = 0, y_upper = 0;
    bool clip_src = true;
    bool clip_tgt = true;
    bool x_outside_window = false;

    CGAL::Arr_parameter_space loc_p1 = arc.location(CGAL::ARR_MIN_END);
    CGAL::Arr_parameter_space loc_p2 = arc.location(CGAL::ARR_MAX_END);

    Rational ref_bound = m_engine.m_pixel_w_r / CGAL_REFINE_X;
    if (loc_p1 != CGAL::ARR_LEFT_BOUNDARY) {
      Gfx_OUT("refining left point\n");
      const Coordinate_1& x0 = arc.curve_end_x(CGAL::ARR_MIN_END);
      typename Curve_kernel_2::Algebraic_kernel_d_1::Approximate_relative_1 approximate_x;

      Bounds bnd = approximate_x(x0, 0);
      while (1) {
        if (bnd.first >= m_engine.m_x_max_r) {
          x_outside_window = true;
          lower = bnd.second;
          break;
        }
        if (bnd.second < m_engine.m_x_min_r)
          break;
        else if (bnd.second - bnd.first <= ref_bound) {
          lower = bnd.second, clip_src = false;
          break;
        }
        bnd = approximate_x(x0, 1);
      }
    }

    // since end-points are sorted lexicographically, no need to check for
    // lower boundary
    if (loc_p2 == CGAL::ARR_TOP_BOUNDARY && arc.is_vertical()) y_upper = m_engine.m_y_max_r;

    if (loc_p2 != CGAL::ARR_RIGHT_BOUNDARY) {
      Gfx_OUT("refining right point\n");
      const Coordinate_1& x0 = arc.curve_end_x(CGAL::ARR_MAX_END);

      typename Curve_kernel_2::Algebraic_kernel_d_1::Approximate_relative_1 approximate_x;

      Bounds bnd = approximate_x(x0, 0); // do not approximate
      while (1) {
        if (bnd.second <= m_engine.m_x_min_r) {
          x_outside_window = true;
          upper = bnd.first;
          break;
        }
        if (bnd.first > m_engine.m_x_max_r) break;
        else if (bnd.second - bnd.first <= ref_bound) {
          upper = bnd.first, clip_tgt = false;
          break;
        }
        bnd = approximate_x(x0, 1);
      }
    }

    if (x_outside_window) {
      return;
    }
#else

    Rational lower, upper, y_lower = 0, y_upper = 0;
    bool clip_src = false;
    bool clip_tgt = false;

    CGAL::Arr_parameter_space loc_p1 = arc.location(CGAL::ARR_MIN_END);
    CGAL::Arr_parameter_space loc_p2 = arc.location(CGAL::ARR_MAX_END);

    Rational ref_bound = m_engine.m_pixel_w_r / CGAL_REFINE_X;
    if (loc_p1 != CGAL::ARR_LEFT_BOUNDARY) {
      const Coordinate_1& x0 = arc.curve_end_x(CGAL::ARR_MIN_END);
      typename Curve_kernel_2::Algebraic_kernel_d_1::Approximate_relative_1 approximate_x;

      Bounds bnd = approximate_x(x0, 0); // do not approximate

      while (bnd.second - bnd.first > ref_bound) {
        bnd = approximate_x(x0, 1);
      }
      lower = bnd.second;
    }
    else {
      lower = m_engine.m_x_min_r;
      clip_src = true;
    }

    // since end-points are sorted lexicographically, no need to check for
    // lower boundary
    if (loc_p2 == CGAL::ARR_TOP_BOUNDARY && arc.is_vertical()) y_upper = m_engine.m_y_max_r;

    if (loc_p2 != CGAL::ARR_RIGHT_BOUNDARY) {
      const Coordinate_1& x0 = arc.curve_end_x(CGAL::ARR_MAX_END);
      typename Curve_kernel_2::Algebraic_kernel_d_1::Approximate_relative_1 approximate_x;
      Bounds bnd = approximate_x(x0, 0); // do not approximate

      while (bnd.second - bnd.first > ref_bound) {
        bnd = approximate_x(x0, 1);
      }
      upper = bnd.first;
      //         while(ubound_x(x0) - lbound_x(x0) > ref_bound)
      //             refine_x(x0);
      //         upper = lbound_x(x0);
    }
    else {
      upper = m_engine.m_x_max_r;
      clip_tgt = true;
    }

    bool x_outside_window = false;
    Rational lower0 = lower;
    Rational upper0 = upper;
    Rational y_lower0(-1);
    Rational y_upper0(-1);

    if (upper <= m_engine.m_x_min_r || lower >= m_engine.m_x_max_r) {
      x_outside_window = true;
      clip_src = clip_tgt = true;
    }

    if (lower < m_engine.m_x_min_r) {
      lower = m_engine.m_x_min_r;
      clip_src = true;
    }
    if (upper > m_engine.m_x_max_r) {
      upper = m_engine.m_x_max_r;
      clip_tgt = true;
    }
#endif

    if (! x_outside_window) {
      Rational height_r = (m_engine.m_y_max_r - m_engine.m_y_min_r) * 2;
      ref_bound = m_engine.m_pixel_h_r / CGAL_REFINE_X;

#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
      ref_bound = (std::min)(ref_bound, Rational(CGAL_REFINE_DOUBLE_APPROX));
#endif
      Gfx_OUT("computing y-coordinates\n");

      if (loc_p1 == CGAL::ARR_INTERIOR || clip_src)
        y_lower = get_endpoint_y(arc, lower, CGAL::ARR_MIN_END, clip_src, ref_bound);

      else if (loc_p1 == CGAL::ARR_BOTTOM_BOUNDARY)
        y_lower = (arc.is_vertical() ? m_engine.m_y_min_r : m_engine.m_y_min_r - height_r);

      else { // endpoints must be sorted lexicographical
        CGAL_precondition(! arc.is_vertical());
        y_lower = m_engine.m_y_max_r + height_r;
      }

      if (loc_p2 == CGAL::ARR_INTERIOR || clip_tgt)
        y_upper = get_endpoint_y(arc, upper, CGAL::ARR_MAX_END, clip_tgt, ref_bound);

      else if (loc_p2 == CGAL::ARR_BOTTOM_BOUNDARY) {
        CGAL_precondition(! arc.is_vertical());
        y_upper = m_engine.m_y_min_r - height_r;
      }
      else
        y_upper = (arc.is_vertical() ? m_engine.m_y_max_r : m_engine.m_y_max_r + height_r);
    }

#ifdef CGAL_CKVA_CR_TIMING
    refine_timer.stop();
#endif

#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
    if (end_pt1 != nullptr && loc_p1 == CGAL::ARR_INTERIOR &&
        (clip_src || y_lower < m_engine.m_y_min_r || y_lower > m_engine.m_y_max_r)) {
      y_lower0 =
        (clip_src ? get_endpoint_y(arc, lower0, CGAL::ARR_MIN_END, false, CGAL_REFINE_DOUBLE_APPROX) : y_lower);
      Gfx_OUT("lower end-point: [" << CGAL::to_double(lower0) << "; " << CGAL::to_double(y_lower0) << "]\n");
      *end_pt1 = Coord_2(CGAL::to_double(lower0), CGAL::to_double(y_lower0));
    }
    if (end_pt2 != nullptr && loc_p2 == CGAL::ARR_INTERIOR &&
        (clip_tgt || y_upper < m_engine.m_y_min_r || y_upper > m_engine.m_y_max_r)) {
      y_upper0 =
        (clip_tgt ? get_endpoint_y(arc, upper0, CGAL::ARR_MAX_END, false, CGAL_REFINE_DOUBLE_APPROX) : y_upper);
      Gfx_OUT("upper end-point: [" << CGAL::to_double(upper0) << "; " << CGAL::to_double(y_upper0) << "]\n");
      *end_pt2 = Coord_2(CGAL::to_double(upper0), CGAL::to_double(y_upper0));
    }
    if (x_outside_window) return;
#endif // CGAL_CKVA_RENDER_WITH_REFINEMENT

    Pixel_2 pix_1, pix_2;

    Gfx_OUT("lower: " << CGAL::to_double(lower) << "; upper: " << CGAL::to_double(upper) << "; y_lower: "
                      << CGAL::to_double(y_lower) << "; y_upper: " << CGAL::to_double(y_upper) << "\n");

    get_pixel_coords(lower, y_lower, pix_1);
    get_pixel_coords(upper, y_upper, pix_2);

#ifndef CGAL_CKVA_RENDER_WITH_REFINEMENT
    if (end_pt1 != nullptr) *end_pt1 = Coord_2(pix_1.m_x, pix_1.m_y);
    if (end_pt2 != nullptr) *end_pt2 = Coord_2(pix_2.m_x, pix_2.m_y);
#endif

    Gfx_OUT("lower pix: (" << pix_1.m_x << "; " << pix_1.m_y << ") upper pix: (" << pix_2.m_x << "; " << pix_2.m_y
                           << ") pixel_w: " << m_engine.m_pixel_w << " pixel_h: " << m_engine.m_pixel_h << std::endl);

    std::vector<Coord_2> rev_points;
    // reserve at least enough space for arc's x-length
    rev_points.reserve(CGAL_ABS(pix_2.m_x - pix_1.m_x));

    if (arc.is_vertical()) {
#ifndef CGAL_CKVA_RENDER_WITH_REFINEMENT
      CGAL_CKVA_STORE_COORDS(rev_points, pix_1);
      CGAL_CKVA_STORE_COORDS(rev_points, pix_2);
#else
      int inc = (pix_2.m_y > pix_1.m_y ? 1 : -1);
      for (; pix_1.m_y != pix_2.m_y; pix_1.m_y += inc) {
        CGAL_CKVA_STORE_COORDS(rev_points, pix_1);
        pix_1.yv += CGAL::to_double(m_engine.m_pixel_h * NT(inc));
      }

#endif
      points.push_back(rev_points);
      return;
    }

    if (! m_clip_pts_computed) {
      Gfx_OUT("computing clip points\n");
      horizontal_clip();
      m_clip_pts_computed = true;
    }

    using Int_pair = std::pair<int, int>;
    using index_pair_vector = std::vector<Int_pair>;
    index_vector btm_idx;
    index_vector top_idx;
    index_pair_vector seg_pts;

    Gfx_OUT("checking bottom clip: y = " << m_engine.m_y_min << "\n");
    segment_clip_points(lower, upper, y_lower, y_upper, m_engine.m_y_min_r, m_btm_poly, arc.arcno(), m_btm_clip,
                        btm_idx);

    Gfx_OUT("checking top clip: y = " << m_engine.m_y_max << "\n");
    segment_clip_points(lower, upper, y_lower, y_upper, m_engine.m_y_max_r, m_top_poly, arc.arcno(), m_top_clip,
                        top_idx);

    index_vector::iterator it1 = btm_idx.begin();
    index_vector::iterator it2 = top_idx.begin();
    while (1) {
      bool push_1 = true;
      if (it1 != btm_idx.end()) {
        if (it2 != top_idx.end()) push_1 = ((m_btm_clip)[*it1].m_left < (m_top_clip)[*it2].m_left);
      }
      else if (it2 != top_idx.end())
        push_1 = false;
      else
        break;
      if (push_1) { // 0 - bottom point, 1 - top point
        seg_pts.push_back(Int_pair(*it1, 0));
        it1++;
      }
      else {
        seg_pts.push_back(Int_pair(*it2, 1));
        it2++;
      }
    }

    m_max_level = 0;
    Pixel_2 start;
    int dir[2];
    int b_taken[2];
    int last_x = -m_engine.m_res_w;
    bool b_coincide;
    Rational pt;
    Rational mid;

    m_branches_coincide = false;
    // make a small tolerance
    bool pt1_inside = (pix_1.m_y >= -1 && pix_1.m_y <= m_engine.m_res_h);
    bool pt2_inside = (pix_2.m_y >= -1 && pix_2.m_y <= m_engine.m_res_h);

    //TODO: use Event1_info::multiplicity(i) - to gather information about roots ?
    if (seg_pts.size() == 1 && pt1_inside && pt2_inside) seg_pts.clear();

    m_arcno = arc.arcno();
    // easy case: no clip-points found
    if (seg_pts.size() == 0) {
      if (! pt1_inside && ! pt2_inside) {
        Gfx_OUT("segment is outside\n");
        return;
      }
      /// WARNING: if x-interval is small while y coordinates are far away from
      /// the window, we can get into the troubles..
      if (pix_2.m_x - pix_1.m_x <= 1) { // it goes away right here
        CGAL_CKVA_STORE_COORDS(rev_points, pix_1);
        CGAL_CKVA_STORE_COORDS(rev_points, pix_2);
        points.push_back(rev_points);
        return;
      }

      Gfx_OUT("NO clip points\n");
      mid = (lower + upper) / 2;
      pt = m_engine.m_x_min_r + (pix_1.m_x * 2 + 5) * m_engine.m_pixel_w_r / 2;
      // segment is almost vertical - start from a middle point
      //if(pt > mid)
      pt = mid;
      if (! get_seed_point(pt, start, dir, b_taken, b_coincide)) {
        Gfx_OUT("generic error occurred..\n");
        return;
      }
      Gfx_OUT("starting pixel found: " << start << " directions: " << dir[0] << " and " << dir[1] << std::endl);
      // only one point list
      draw_lump(rev_points, last_x, pix_1, pix_2);
      points.push_back(rev_points);

      Gfx_OUT("exit normal, max_level: " << m_max_level << std::endl);
      return;
    }

    // clip-points presented
    Rational l;
    Rational r;
    Rational y_clip;
    Coordinate_1 alpha;
    Poly_dst_1* ppoly;
    Clip_point_entry *pclip, *ptmp;

    Gfx_OUT("segment is not completely inside the window..\n");
    index_pair_vector::iterator it = seg_pts.begin();
    index_pair_vector::iterator eend = seg_pts.end();

    Gfx_OUT("\n\nSEGMENT clip points: \n");
    Pixel_2 pix_beg;
    Pixel_2 pix_end;
    bool straight_segment;

    while (it != eend) {
      if (it != seg_pts.begin() && it == eend - 1 && ! pt2_inside) break;
      straight_segment = false;

      int idx = it->first;
      if (it->second == 0) { // bottom points
        pclip = &m_btm_clip[idx];
        ppoly = &m_btm_poly;
        y_clip = m_engine.m_y_min_r;
        Gfx_OUT("bottom clip-point: [");
      }
      else { // top points
        pclip = &m_top_clip[idx];
        ppoly = &m_top_poly;
        y_clip = m_engine.m_y_max_r;
        Gfx_OUT("up clip-point: [");
      }
      l = pclip->m_left;
      r = pclip->m_right;
      Gfx_OUT(CGAL::to_double(l) << "; " << CGAL::to_double(r) << "\n");

      if (last_x != -m_engine.m_res_w) { // compute screen coordinates of a pixel
        Rational last_pt = m_engine.m_x_min_r + static_cast<Rational>(last_x) * m_engine.m_pixel_w_r;
        if (r <= last_pt) { // skip a clip-point if already drawn
          Gfx_OUT("clip point skipped..\n");
          it++;
          continue;
        }
      }

      rev_points.clear();
      if (it == seg_pts.begin() && pt1_inside) {
        Gfx_OUT("starting point is near the left end-point\n");
        //pt = engine.x_min_r + (pix_1.x*2+5)*engine.pixel_w_r/2;
        pt = (lower + l) / 2;
        if (pt <= lower) {
          Gfx_OUT("refining the left end-point\n");

          refine_alg_point(l, r, *ppoly, lower, 1);
          pt = l;
          if (l - lower <= m_engine.m_pixel_w_r * 2) straight_segment = true;
        }
        pix_beg = pix_1;
        get_pixel_coords(l, y_clip, pix_end);
      }
      else if (it == eend - 1 && pt2_inside) {
        Gfx_OUT("starting point is near the right end-point\n");
        //pt = engine.x_min_r + (pix_2.x*2-3)*engine.pixel_w_r/2;
        pt = (r + upper) / 2;
        Gfx_OUT("pt is: " << CGAL::to_double(pt) << "; r is: " << CGAL::to_double(r)
                          << ": pixel_w_r: " << CGAL::to_double(m_engine.m_pixel_w_r) << "\n");

        if (pt >= upper) {
          // ensure that clip-point interval is sufficiently small
          refine_alg_point(l, r, *ppoly, upper, 1);
          pt = r;
          if (upper - r <= m_engine.m_pixel_w_r * 2) straight_segment = true;
        }
        pix_end = pix_2;
        get_pixel_coords(r, y_clip, pix_beg);
      }
      else {
        Gfx_OUT("starting point is between clip-points\n");
        it++;

        if (it == seg_pts.end()) {
          Gfx_OUT("ERROR: clip point missed ?\n");
          break;
        }

        idx = it->first;
        ptmp = (it->second ? &m_top_clip[idx] : &m_btm_clip[idx]);
        pt = pclip->m_alpha.rational_between(ptmp->m_alpha);

        get_pixel_coords(l, y_clip, pix_beg);
        get_pixel_coords(ptmp->m_left, it->second ? m_engine.m_y_max_r : m_engine.m_y_min_r, pix_end);
        if (CGAL_ABS(Rational(ptmp->m_left - l)) <= m_engine.m_pixel_w_r * 2) {
          Coordinate_2 xy(Coordinate_1(pt), *m_support, arc.arcno());
          Rational _;
          refine_xy(xy, m_engine.m_pixel_h_r / CGAL_REFINE_Y, _, mid);
          //                 mid = ubound_y(xy);

          CGAL_CKVA_STORE_COORDS(rev_points, pix_beg);
          get_pixel_coords(pt, mid, pix_beg);
          straight_segment = true;
        }
      }
#ifndef CGAL_CKVA_RENDER_WITH_REFINEMENT
      if (straight_segment) {
        Gfx_OUT("straight subsegment found\n");
        CGAL_CKVA_STORE_COORDS(rev_points, pix_beg);
        CGAL_CKVA_STORE_COORDS(rev_points, pix_end);

        points.push_back(rev_points);
        it++;
        continue;
      }
#endif // !CGAL_CKVA_RENDER_WITH_REFINEMENT

      if (! get_seed_point(pt, start, dir, b_taken, b_coincide)) {
        Gfx_OUT("get_seed_point: a problem occurred..\n");
        it++;
        continue;
      }

      Gfx_OUT("starting pixel found: " << start << " directions: " << dir[0] << " and " << dir[1] << std::endl);
      draw_lump(rev_points, last_x, pix_beg, pix_end);
      points.push_back(rev_points);
      if (it == eend) break;
      it++;
    }
  }

  /*! rasterizes distinct points on curves (overloaded version)
 *
 * \return \c false indicating that the point lies outside the drawing window
 * \c true in case the point coordinates were successfully computed
 */
  template <typename Coord_2>
  bool draw(const Point_2& pt, Coord_2& coord) {
    Gfx_OUT("rasterizing point: " << CGAL::to_double(pt.x()) << std::endl);

    const Coordinate_1& x0 = pt.x();
    Rational ref_bound = m_engine.m_pixel_w_r / 2;
#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
    ref_bound = (std::min)(ref_bound, Rational(CGAL_REFINE_DOUBLE_APPROX));
#endif

    typename Curve_kernel_2::Algebraic_kernel_d_1::Approximate_relative_1 approximate_x;
    Bounds bnd = approximate_x(x0, 0); // do not approximate
    while (bnd.second - bnd.first > ref_bound) {
      bnd = approximate_x(x0, 1);
    }

    //     while(ubound_x(x0) - lbound_x(x0) > ref_bound)
    //         refine_x(x0);
    Rational x_s = bnd.first;
    Rational y_s;
#ifndef CGAL_CKVA_RENDER_WITH_REFINEMENT
    if (x_s < m_engine.m_x_min_r || x_s > m_engine.m_x_max_r) return false;
#endif
    ref_bound = m_engine.m_pixel_h_r / CGAL_REFINE_X;

#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
    ref_bound = (std::min)(ref_bound, Rational(CGAL_REFINE_DOUBLE_APPROX));
#endif

    Coordinate_2 xy(x0, pt.curve(), pt.arcno());
    Rational _;
    refine_xy(xy, ref_bound, _, y_s);

    Pixel_2 pix;
    get_pixel_coords(x_s, y_s, pix);
#ifndef CGAL_CKVA_RENDER_WITH_REFINEMENT
    if (pix.m_x < 0 || pix.m_x >= m_engine.m_res_w || pix.m_y < 0 || pix.m_y >= m_engine.m_res_h) return false;

    coord = Coord_2(pix.m_x, pix.m_y);
#else
    coord = Coord_2(pix.xv, pix.yv);
#endif
    return true;
  }

  //!@}
private:
  //!\name private methods
  //!@{

  //! draws a segment's piece from pix_1 to pix_2, the starting pixel is taken
  //! from the seed point stack
  template <typename Coord_2>
  void draw_lump(std::vector<Coord_2>& rev_points, int& last_x, const Pixel_2& pix_1, const Pixel_2& pix_2) {
    Pixel_2 start;
    Pixel_2 pix;
    Pixel_2 witness;
    Pixel_2 prev_pix;
    Pixel_2 stored_pix;
    Pixel_2 stored_prev;
    int back_dir;
    int new_dir;
    int stored_dir;
    int dir[2];
    int b_taken[2];
    int ux;
    int uy;
    bool b_coincide;
    bool failed;
    start = m_seed_stack.top().m_start;

    bool pix_1_close = (CGAL_ABS(start.m_x - pix_1.m_x) <= 1 && CGAL_ABS(start.m_y - pix_1.m_y) <= 1);
    bool pix_2_close = (CGAL_ABS(start.m_x - pix_2.m_x) <= 1 && CGAL_ABS(start.m_y - pix_2.m_y) <= 1);

    // stores an x-coordinate of the last traced pixel in --> direction:
    // required to "jump" over clip-points
    last_x = -m_engine.m_res_w;

    if (pix_1_close && pix_2_close) { // suppress drawing of trivial segments
      CGAL_CKVA_STORE_COORDS(rev_points, pix_1);
      CGAL_CKVA_STORE_COORDS(rev_points, pix_2);
      return;
    }

    Gfx_OUT("draw_lump: pix_1 = " << pix_1 << "; pix_2 = " << pix_2 << "\n");
    std::vector<Coord_2> points;
    // reserve a list of point coordinates
    points.reserve(CGAL_ABS(pix_2.m_x - pix_1.m_x));

    m_direction_taken = -1;
    bool overstep = (pix_1_close || pix_2_close);
    bool ready = false;

    while (! m_seed_stack.empty()) {
      m_current_seed = m_seed_stack.top();
      m_seed_stack.pop();
      m_branches_coincide = m_current_seed.m_branches_coincide;
      if (m_direction_taken == -1) m_direction_taken = m_current_seed.m_direction_taken;

      witness = m_current_seed.m_start;
      pix = witness;
      m_current_level = witness.m_level;
      pix.m_sub_x = 0;
      pix.m_sub_y = 0;
      pix.m_level = 0;
      back_dir = m_current_seed.m_back_dir;
      stored_dir = back_dir;
      stored_pix = pix;
      stored_prev = pix;

      // store result in a list or reversed list depending on the orientation
      std::vector<Coord_2>& ppoints = (m_direction_taken == 0 ? rev_points : points);

      Gfx_OUT("continuing from a seed point: " << witness << "; back_dir: " << back_dir
                                               << " direction_taken: " << m_direction_taken << std::endl);

      bool is_exit = false;
      while (1) {
        if (pix.m_x < 0 || pix.m_x > m_engine.m_res_w || pix.m_y < -CGAL_WINDOW_ENLARGE ||
            pix.m_y > m_engine.m_res_h + CGAL_WINDOW_ENLARGE) {
          m_branches_coincide = false;
          break;
        }

        CGAL_CKVA_STORE_COORDS(ppoints, pix);
#ifndef CGAL_CKVA_RENDER_WITH_REFINEMENT
        bool bb1 = (m_direction_taken == 0 && pix.m_x <= pix_1.m_x),
             bb2 = (m_direction_taken == 1 && pix.m_x >= pix_2.m_x);
        if ((bb1 || bb2)) {
          //Gfx_OUT("STOP: reached end-point x-coordinate\n");
          m_branches_coincide = false;
          is_exit = true;
          break;
        }
#endif

        bool set_ready = false;
        if (CGAL_ABS(pix.m_x - pix_1.m_x) <= 1 && CGAL_ABS(pix.m_y - pix_1.m_y) <= 1) {
          if (! overstep || ready) {
            pix = pix_1;
            m_branches_coincide = false;
            break;
          }
          set_ready = true;
        }

        if (CGAL_ABS(pix.m_x - pix_2.m_x) <= 1 && CGAL_ABS(pix.m_y - pix_2.m_y) <= 1) {
          if (! overstep || ready) {
            pix = pix_2;
            m_branches_coincide = false;
            break;
          }
          ready = true;
        }
        if (set_ready) ready = true;

        if (! test_neighborhood(pix, back_dir, new_dir)) {
          ux = pix.m_x;
          uy = pix.m_y;
          if (witness == pix) { // witness subpixel is a pixel itself
            if (! subdivide(pix, back_dir, new_dir)) {
              is_exit = true;
              break;
            }
            prev_pix = pix;
            advance_pixel(pix, new_dir);
            back_dir = (new_dir + 4) & 7;
          }
          else {
            back_dir = stored_dir;
            pix = witness;
            prev_pix = witness;
          }

          stored_dir = -1;
          failed = false;
          while (ux == pix.m_x && uy == pix.m_y) {
            if (! failed &&
                ((prev_pix.m_sub_x & 2) != (pix.m_sub_x & 2) || (prev_pix.m_sub_y & 2) != (pix.m_sub_y & 2))) {
              stored_pix = pix;
              pix.m_sub_x >>= 1;
              pix.m_sub_y >>= 1;
              pix.m_level--;
              stored_dir = back_dir;
              stored_prev = prev_pix;
            }

            if (! test_neighborhood(pix, back_dir, new_dir)) {
              if (stored_dir != -1) {
                pix = stored_pix;
                prev_pix = stored_prev;
                back_dir = stored_dir;
                stored_dir = -1;
                failed = true;
                continue;
              }
              if (! subdivide(pix, back_dir, new_dir)) {
                is_exit = true;
                break;
              }
            }
            //Gfx_OUT(pix << " dir = " << new_dir << std::endl);
            prev_pix = pix;
            advance_pixel(pix, new_dir);
            if (is_isolated_pixel(pix)) {
              m_branches_coincide = false;
              is_exit = true;
              break;
            }
            back_dir = (new_dir + 4) & 7;
          }
          if (is_exit) break;

          stored_dir = back_dir;
          witness = pix;
          pix.m_level = 0;
          pix.m_sub_x = 0;
          pix.m_sub_y = 0;
          //Gfx_OUT(witness << " " << prev_pix << std::endl);
        }
        else {
          ux = pix.m_x;
          uy = pix.m_y;
          advance_pixel(pix, new_dir);
          witness = pix;
        }
        back_dir = (new_dir + 4) & 7;
      }

      if (m_branches_coincide) { // oops, need another seed point
        if (m_direction_taken == -1) {
          Gfx_OUT("\n\nFATAL: unknown direction in coincide mode!\n\n");
          return;
        }

        (m_direction_taken == 0) ? pix.m_x -= 1 : pix.m_x += 1;
        if (pix.m_x >= pix_2.m_x - 1 || pix.m_x <= pix_1.m_x + 1) goto Lexit;

        //std::cout << "New seed point required at: " << pix << std::endl;
        if (! get_seed_point(m_engine.m_x_min_r + pix.m_x * m_engine.m_pixel_w_r, start, dir, b_taken, b_coincide)) {
          std::cerr << " wrong seed point found " << std::endl;
          throw internal::Insufficient_rasterize_precision_exception();
        }

        Gfx_OUT("new seed point found: " << start << " directions: " << dir[0] << " and " << dir[1]
                                         << "; taken = " << m_direction_taken << std::endl);

        b_taken[0] = internal::DIR_TAKEN_MAP[dir[0]];
        b_taken[1] = internal::DIR_TAKEN_MAP[dir[1]];

        if (b_taken[0] != -1)
          new_dir = dir[b_taken[0] != m_direction_taken ? 0 : 1];
        else if (b_taken[1] != -1)
          new_dir = dir[b_taken[1] != m_direction_taken ? 1 : 0];
        else {
          std::cerr << "ERROR: wrong backward dir after seed point: " << dir[0] << " " << dir[1] << std::endl;
          throw internal::Insufficient_rasterize_precision_exception();
        }

        m_seed_stack.push(Seed_point(start, new_dir, m_direction_taken, b_coincide));
        continue;
      }
      else if (is_exit) {
      Lexit:
        pix = (m_direction_taken == 0 ? pix_1 : pix_2);
      }

      //!@note: this can cause missing end-points !!
      //! if they proceeded by pt1/2_inside (-1 or res_h+1)
      //if(pix.y >= 0 && pix.y < engine.res_h)
      CGAL_CKVA_STORE_COORDS(ppoints, pix);

      // we were tracing in --> direction: store the pixel where we stopped
      if (m_direction_taken == 1) last_x = pix.m_x;

      if (m_direction_taken != -1) m_direction_taken = 1 - m_direction_taken;
      ready = false;

    } // while(!s_stack.empty())

    std::reverse(rev_points.begin(), rev_points.end());
    // resize rev_points to accommodate the size of points vector
    std::size_t rsize = rev_points.size();
    rev_points.resize(rsize + points.size());
    std::copy(points.begin(), points.end(), rev_points.begin() + rsize);
  }

  //! recursively subdivides pixel into 4 subpixels, returns a new tracking
  //! direction
  bool subdivide(Pixel_2& pix, int back_dir, int& new_dir) {
    //Gfx_OUT("\n\nSubdivision of a pixel" << pix << " with back_dir =  " <<
    //back_dir << std::endl);
    NT inv = NT(1) / NT(m_one << pix.m_level);
    m_make_exact(inv);
    int idx;
    int pref_dir = back_dir >> 1;

    if (pix.m_level >= s_max_subdivision_level) {
      std::cerr << "reached maximum subdivision level: " << pix << std::endl;
      throw internal::Insufficient_rasterize_precision_exception();
    }

    if (m_limit(m_engine.m_pixel_w * inv) || m_limit(m_engine.m_pixel_h * inv)) {
      std::cerr << "too small subpixel size: " << pix << std::endl;
      throw internal::Insufficient_rasterize_precision_exception();
    }

    // if several branches coincide within this pixel we cannot perform
    // a subdivision
    if (m_branches_coincide) return false;

    if (back_dir & 1) { // diagonal direction
      idx = get_subpixel_diag(pix, pref_dir);
      if (idx == -1) {
        std::cerr << "wrong diag subpixel: " << pix << " direction: " << pref_dir << std::endl;
        throw internal::Insufficient_rasterize_precision_exception();
      }
      idx = D_SUBPIX_MAP[pref_dir][idx];
    }
    else {
      idx = get_subpixel_hv(pix, pref_dir);
      if (idx == -1) {
        std::cerr << "wrong h/v subpixel" << pix << " direction: " << pref_dir << std::endl;
        throw internal::Insufficient_rasterize_precision_exception();
      }
      idx = HV_SUBPIX_MAP[pref_dir][idx];
    }

    pix.m_level++;
    if (m_max_level < pix.m_level) m_max_level = pix.m_level;
    if (m_current_level < pix.m_level) m_current_level = pix.m_level;

    pix.m_sub_x = (pix.m_sub_x << 1) + (idx & 1);
    pix.m_sub_y = (pix.m_sub_y << 1) + (idx >> 1);
    //Gfx_DETAILED_OUT("subpixel index: " << idx << " (" << pix.sub_x << "; "
    // << pix.sub_y << ")" << std::endl);
    if (! test_neighborhood(pix, back_dir, new_dir)) return subdivide(pix, back_dir, new_dir);
    //Gfx_DETAILED_OUT("new direction found: " << new_dir << " at a pixel:" <<
    //pix << std::endl);
    return true;
  }

  //! returns a "seed" point of a segment and two possible directions from it
  bool get_seed_point(const Rational& seed, Pixel_2& start, int* dir, int* b_taken, bool& b_coincide) {
    Rational x_s = seed;
    Rational y_s;

    Gfx_OUT("get seed point: " << m_rat2float(seed) << "\n");
    //NOTE dirty HACK: we have no kernel instance ..
    Coordinate_2 xy(Coordinate_1(seed), *m_support, m_arcno);

    Rational _;
    refine_xy(xy, m_engine.m_pixel_h_r / CGAL_REFINE_Y, _, y_s);
    //     y_s = ubound_y(xy);

    Integer lvl = m_one;
    Rational x_seed;
    Rational y_seed;
    get_pixel_coords(x_s, y_s, start, &x_seed, &y_seed);

    Gfx_OUT("y_seed = " << m_rat2float(y_seed) << std::endl);

    // we allow a small tolerance for seed point coordinates due to
    // round off errors
    if (start.m_y < -CGAL_WINDOW_ENLARGE || start.m_y > m_engine.m_res_h + CGAL_WINDOW_ENLARGE) {
      Gfx_OUT("get_seed_point: starting pixel does not fit the window boundaries " << std::endl);
      return false;
    }

    start.m_level = 0;
    start.m_sub_x = 0;
    start.m_sub_y = 0;
    m_current_level = 0;

    //Gfx_OUT("refining starting pixel: " << start << std::endl);
    //     Gfx_OUT("x/y seed: (" << CGAL::to_double(x_seed) << "; " <<
    //         CGAL::to_double(y_seed) << ")" << std::endl);
    b_taken[0] = b_taken[1] = -1;
    b_coincide = false;

    int coincide_level = 0;
    while (! test_pixel(start, (int*)dir, (int*)b_taken, b_coincide)) {
      //Gfx_OUT("refining starting pixel: " << start << std::endl);
      if (start.m_level >= s_max_subdivision_level) {
        std::cerr << "get_seed_point: reached maximum subdivision level " << start.m_level << std::endl;
        throw internal::Insufficient_rasterize_precision_exception();
      }
      //dump_neighborhood(start);

      if (m_limit(m_engine.m_pixel_w / NT(lvl)) || m_limit(m_engine.m_pixel_h / NT(lvl))) {
        std::cerr << "get_seed_point: too small subpixel size: " << start.m_level << std::endl;
        throw internal::Insufficient_rasterize_precision_exception();
      }

      start.m_level++;
      if (start.m_level > m_max_level) m_max_level = start.m_level;
      if (start.m_level > m_current_level) m_current_level = start.m_level;
      lvl <<= 1;

      if (lvl > CGAL_REFINE_Y) {
        Rational _;
        refine_xy(xy, m_engine.m_pixel_h_r / (lvl * 2), _, y_s);
        y_seed = (y_s - m_engine.m_y_min_r) / m_engine.m_pixel_h_r;
      }

      start.m_sub_x = m_rat2integer((x_seed - start.m_x) * lvl);
      start.m_sub_y = m_rat2integer((y_seed - start.m_y) * lvl);

      if (start.m_sub_x < 0) start.m_sub_x = 0;
      start.m_sub_x &= (lvl - 1);
      if (start.m_sub_y < 0) start.m_sub_y = 0;
      start.m_sub_y &= (lvl - 1);

      if (m_current_level >= CGAL_COINCIDE_LEVEL) {
        Pixel_2 test = start;
        test.m_sub_x = start.m_sub_x >> (start.m_level - coincide_level);
        test.m_sub_y = start.m_sub_y >> (start.m_level - coincide_level);
        test.m_level = coincide_level;

        if (test_pixel(test, (int*)dir, (int*)b_taken, b_coincide)) {
          start = test;
          break;
        }
        coincide_level++;
      }
      //Gfx_DETAILED_OUT("\nTesting pixel: " << start << std::endl);
    }

    //Gfx_OUT("directions found: " << dir[0] << "; " << dir[1] << "\n");
    int t0 = internal::DIR_TAKEN_MAP[dir[0]];
    int t1 = internal::DIR_TAKEN_MAP[dir[1]];
    if (t0 != -1 && t0 == t1) {
      Gfx_OUT("get_seed_point: one-side directions found: " << dir[0] << " and " << dir[1] << std::endl);
    }

    if (! m_branches_coincide) {
      if (t0 == -1) {
        if (t1 == -1) {
          // vx = -df/dy; vy = df/dx
          typename Renderer_internals::Coercion::Cast rccast;
          Rat_coercion_type xcs = rccast(x_s);
          Rat_coercion_type ycs = rccast(y_s);

          Rat_coercion_type vx = -m_engine.substitute_xy(*(m_engine.m_rational_fy), x_s, y_s);
          Rat_coercion_type vy = m_engine.substitute_xy(*(m_engine.m_rational_fx), x_s, y_s);

          NT vvx = m_rat2float(vx);
          NT vvy = m_rat2float(vy);
          if (vvy < 0) {
            vvx = -vvx;
          }

          // vx > 0: 2 - right(1), 6 - left(0)
          // vx < 0: 2 - left(0), 6 - right(1)
          bool flag = (vvx > 0);
          if (dir[0] == 2 || dir[1] == 6) {
            // taken 2 = 0 if vx>0, 1 if vx<0
            // taken 6 = 1 if vx>0, 0 if vx<0
            b_taken[0] = ! flag; // !(vx > 0)
            b_taken[1] = flag;   //  vx > 0
          }
          else if (dir[0] == 6 || dir[1] == 2) {
            b_taken[0] = flag;   // vx > 0
            b_taken[1] = ! flag; // !(vx > 0)
          }
        }
        else // t1 != -1
          b_taken[0] = 1 - b_taken[1];
      }
      else if (t1 == -1)
        b_taken[1] = 1 - b_taken[0];

      m_seed_stack.push(Seed_point(start, dir[0], b_taken[0], b_coincide));
      m_seed_stack.push(Seed_point(start, dir[1], b_taken[1], b_coincide));
    }
    if (b_coincide) Gfx_OUT("seed point with coincide branches found" << std::endl);
    return true;
  }

  //! checks whether only one curve branch crosses the pixel, required to
  //! compute a starting witness pixel
  bool test_pixel(const Pixel_2& pix, int* dir, int* b_taken, bool& b_coincide) {
    Stripe box[2]; // 0 - left-right stripe, 1 - bottom-top stripe
    NT lvl = NT(m_one << pix.m_level);
    NT inv = NT(1) / lvl;
    m_make_exact(inv);
    get_boundaries(CGAL_X_RANGE, pix, box[1]);
    get_boundaries(CGAL_Y_RANGE, pix, box[0]);
    NT bottom = box[1].m_key[0]; //top = box[1].key[1],
    NT left = box[0].m_key[0];
    NT lower; //, right = box[0].key[1], lower;
    // bottom(2), top(3), left(0), right(1)
    int n_sign = 0;
    int i;
    int j;
    int n_dir = 0;
    int shift;
    int n_local;
    int new_dir;
    get_polynomials(CGAL_Y_RANGE, box[0]);

    /*
    Gfx_OUT("test pixel: " << pix << "--------------------------------\n");
    dump_neighborhood(pix);
    Gfx_OUT("----------------------------------------------\n\n");*/

    b_coincide = false;
    int n_corner_dir = 0;
    int corner_dir[] = {-1, -1};
    // process subsegments: left/right
    for (i = 0; i < 2; i++) {
      for (j = 0, n_local = 0, lower = bottom; j < 3; j++, lower += inv)

        if (get_range_1(CGAL_Y_RANGE, lower, lower + inv, box[0].m_key[i], box[0].m_poly[i], 3) ||
            m_engine.m_zero_bounds) {
          Gfx_DETAILED_OUT("intersection detected at subsegment: "
                           << j << " side: " << i << "; left(0), right(1), bottom(2), top(3)" << std::endl);

          if (m_engine.m_zero_bounds) {
            bool is_corner = false;
            int diff = m_float2int((lower - pix.m_y) * lvl - pix.m_sub_y);
            int low_sign = m_engine.evaluate_generic(CGAL_Y_RANGE, lower, box[0].m_key[i], box[0].m_poly[i]);

            Gfx_DETAILED_OUT("zero bounds detected, low_sign: " << low_sign << "\n");

            if (diff != 0) {
              if (diff == 1) {
                is_corner =
                  (m_engine.evaluate_generic(CGAL_Y_RANGE, lower + inv, box[0].m_key[i], box[0].m_poly[i]) == 0);
                // in case there is intersection at lower point =>
                // it is counted as normal intersection
                if (! is_corner && low_sign != 0) continue;
              }
              else {
                is_corner = (low_sign == 0);
                if (! is_corner) continue;
              }

              if (is_corner) {
                // compute corner direction
                diff = internal::DIR_MAP[i][diff + 1];
                if (n_corner_dir >= 2) {
                  Gfx_DETAILED_OUT("too many corners\n");
                  return false;
                }
                // corner dirs on vertical segs cannot be identified
                corner_dir[n_corner_dir++] = diff;
                Gfx_DETAILED_OUT("corner direction saved: " << diff << "\n");
              }
            }
            else if (low_sign != 0)
              continue;
          }

          n_local++;
          if (n_local > 1) {
            Gfx_DETAILED_OUT("more than 1 intersection along vertical side " << i << std::endl);
            return false;
          }
          // no need to check the derivative if already coincide mode
          if (! b_coincide)
            if (m_engine.m_first_der &&
                ! recursive_check(CGAL_Y_RANGE, lower, lower + inv, box[0].m_key[i], box[0].m_poly[i])) {
              Gfx_DETAILED_OUT("\nrecursive_check failed" << std::endl);
              if (m_current_level < CGAL_COINCIDE_LEVEL) return false;
              b_coincide = true;
            }
          shift = m_float2int((lower - pix.m_y) * lvl - pix.m_sub_y);
          new_dir = internal::DIR_MAP[i][shift + 1];
          // left side (i = 0) - direction taken = 1
          // right side (i = 1) - direction taken = 0
          b_taken[n_dir] = 1 - i;
          Gfx_DETAILED_OUT("new dir found: " << new_dir << "\n");
          dir[n_dir++] = new_dir;
        }
      n_sign += n_local;
    }

    get_polynomials(CGAL_X_RANGE, box[1]);
    Gfx_DETAILED_OUT("computing bottom/top sides" << std::endl);
    for (i = 0; i < 2; i++) {
      for (j = 0, n_local = 0, lower = left; j < 3; j++, lower += inv) {
        if (get_range_1(CGAL_X_RANGE, lower, lower + inv, box[1].m_key[i], box[1].m_poly[i], 3) ||
            m_engine.m_zero_bounds) {
          Gfx_DETAILED_OUT("intersection detected at subsegment: "
                           << j << " side: " << i + 2 << "; left(0), right(1), bottom(2), top(3) " << std::endl);

          if (m_engine.m_zero_bounds) {
            bool is_corner = false;
            int diff = m_float2int((lower - pix.m_x) * lvl - pix.m_sub_x);
            int low_sign = m_engine.evaluate_generic(CGAL_X_RANGE, lower, box[1].m_key[i], box[1].m_poly[i]);

            Gfx_DETAILED_OUT("zero bounds detected, low_sign: " << low_sign << "\n");

            if (diff != 0) {
              if (diff == 1) {
                is_corner =
                  (m_engine.evaluate_generic(CGAL_X_RANGE, lower + inv, box[1].m_key[i], box[1].m_poly[i]) == 0);
                // in case there is intersection at lower point =>
                // it is counted as normal intersection
                if (! is_corner && low_sign != 0) continue;
              }
              else {
                is_corner = (low_sign == 0);
                if (! is_corner) continue;
              }

              if (is_corner) {
                Gfx_DETAILED_OUT("corner direction detected\n");
                // compute corner direction
                diff = internal::DIR_MAP[i + 2][diff + 1];
                if (n_corner_dir > 0 && (corner_dir[0] == diff || corner_dir[1] == diff)) {
                  Gfx_DETAILED_OUT("corner_dir identified: " << diff << "\n");
                  continue;
                }
              }
            }
            else if (low_sign != 0) continue;
          }

          n_local++;
          n_sign++;
          if (n_local > 1 || n_sign > 2) {
            Gfx_DETAILED_OUT("more than 1 intersection along horizontal side " << i + 2 << std::endl);
            return false;
          }

          if (! b_coincide)
            if (m_engine.m_first_der &&
                ! recursive_check(CGAL_X_RANGE, lower, lower + inv, box[1].m_key[i], box[1].m_poly[i])) {
              Gfx_DETAILED_OUT("\nrecursive_check failed" << std::endl);
              if (m_current_level < CGAL_COINCIDE_LEVEL) return false;
              b_coincide = true;
            }
          shift = m_float2int((lower - pix.m_x) * lvl - pix.m_sub_x);
          new_dir = internal::DIR_MAP[i + 2][shift + 1];
          b_taken[n_dir] = 1 - internal::DIR_TAKEN_MAP[new_dir];

          Gfx_DETAILED_OUT(" new dir found: " << new_dir << "\n");
          dir[n_dir++] = new_dir;
        }
      }
    }
    if (n_sign < 2) {
      Gfx_DETAILED_OUT("ERROR: not enough intersections found" << std::endl);
      return false;
    }
    return true;

    /*Stripe box[2]; // 0 - left-right stripe, 1 - bottom-top stripe
    NT lvl = NT(one << pix.level), inv = NT(1) / lvl, llow[2];
    make_exact(inv);
    get_boundaries(CGAL_X_RANGE,pix,box[1]);
    get_boundaries(CGAL_Y_RANGE,pix,box[0]);
    //NT bottom = box[1].key[0], //top = box[1].key[1],
        //left = box[0].key[0], right = box[0].key[1];
    NT lower;
    // bottom(2), top(3), left(0), right(1)
    int f_der[2];
    int idx[2], ibox, ikey, var, n_sign = 0, i, j, n_dir = 0, shift,
        n_local, new_dir;
    get_polynomials(CGAL_Y_RANGE,box[0]);
    b_coincide = false;
    for(i = 0; i < 4; i++) {
        if(i == 2)
            get_polynomials(CGAL_X_RANGE,box[1]);
        var = (i < 2 ? CGAL_Y_RANGE : CGAL_X_RANGE);
        ibox = i >> 1;
        ikey = i & 1;
        lower = box[1-ibox].key[0];
        for(j = 0, n_local = 0; j < 3; j++, lower += inv)
            if(get_range_1(var,lower,lower+inv,box[ibox].key[ikey],
                box[ibox].poly[ikey])||zero_bounds) {
                Gfx_DETAILED_OUT("intersection detected at subsegment: " << j
                  << " side: " << i << "; left(0), right(1), bottom(2), top(3)"
                    << std::endl);
                if(zero_bounds&&evaluate_modular(var, lower,
                    box[ibox].key[ikey], true) != 0)
                    continue;
                n_local++;
                n_sign++;
                if(n_local > 1||n_sign > 2) {
                  Gfx_DETAILED_OUT("more than 1 intersection along side " << i
                        << std::endl);
                    return false;
                }
                f_der[n_dir] = engine.first_der;
                llow[n_dir] = lower;
                idx[n_dir++] = i;
            }
    }
    if(n_sign < 2) {
        Gfx_DETAILED_OUT("not enough intersections found" << std::endl);
        return false;
    }
    for(i = 0; i < 2; i++) {
        var = (idx[i] < 2 ? CGAL_Y_RANGE : CGAL_X_RANGE);
        ibox = idx[i] >> 1;
        ikey = idx[i] & 1;
        lower = llow[i];
        if(f_der[i] == -1) { // the 1st derivative needs to be computed
            get_range_1(var,lower,lower+inv,box[ibox].key[ikey],
                box[ibox].poly[ikey],2);
            f_der[i] = engine.first_der;
        }
        // no need to check the derivative if already coincide mode
        if(!b_coincide)
        if(f_der[i]&&!recursive_check(var,lower,lower+inv,box[ibox].key[ikey],
            box[ibox].poly[ikey])) {
        Gfx_DETAILED_OUT("\nrecursive_check failed" << std::endl);
            if(current_level < CGAL_COINCIDE_LEVEL)
                return false;
            b_coincide = true;
        }
        if(var == CGAL_Y_RANGE)
            shift = float2int((lower - pix.y)*lvl - pix.sub_y);
        else
            shift = float2int((lower - pix.x)*lvl - pix.sub_x);
        new_dir = internal::DIR_MAP[idx[i]][shift+1];
        b_taken[i] = 1 - internal::DIR_TAKEN_MAP[new_dir];
        dir[i] = new_dir;
        Gfx_DETAILED_OUT(" direction " << i << " found: " << new_dir <<
            " taken: " << b_taken[i] << std::endl);
    }
    return true;*/
  }

  void horizontal_clip() {
    using F_traits = typename CGAL::Fraction_traits<Rational>;
    typename F_traits::Numerator_type num;
    typename F_traits::Denominator_type denom;
    typename F_traits::Decompose decompose;

    m_top_clip.clear();
    m_btm_clip.clear();
    Rational l;
    Rational r;

    typename CGAL::Polynomial_traits_d<Poly_dst_1>::Make_square_free msf;

    decompose(m_engine.m_y_min_r, num, denom);
    m_btm_poly = msf(m_support->polynomial_2().evaluate_homogeneous(num, denom));
    decompose(m_engine.m_y_max_r, num, denom);
    m_top_poly = msf(m_support->polynomial_2().evaluate_homogeneous(num, denom));

    using Isolator = CGAL::internal::Bitstream_descartes<CGAL::internal::Bitstream_descartes_rndl_tree_traits<
      CGAL::internal::Bitstream_coefficient_kernel<typename Poly_dst_1::NT>>>;
    Isolator isolator_btm(m_btm_poly);
    Isolator isolator_top(m_top_poly);

    int n_roots = isolator_btm.number_of_real_roots();
    int i;
    Rational criteria = m_engine.m_pixel_w_r / CGAL_REFINE_CLIP_POINTS;

    for (i = 0; i < n_roots; i++) {
      l = isolator_btm.left_bound(i);
      r = isolator_btm.right_bound(i);
      refine_alg_point(l, r, m_btm_poly, criteria);
      if (l > m_engine.m_x_max_r || r < m_engine.m_x_min_r) continue;
      m_btm_clip.push_back(Clip_point_entry(l, r));
    }

    n_roots = isolator_top.number_of_real_roots();
    for (i = 0; i < n_roots; i++) {
      l = isolator_top.left_bound(i), r = isolator_top.right_bound(i);
      refine_alg_point(l, r, m_top_poly, criteria);
      if (l > m_engine.m_x_max_r || r < m_engine.m_x_min_r) continue;
      m_top_clip.push_back(Clip_point_entry(l, r));
    }
  }

  // low_pix, up_pix and y_clip define segment end-points in pixel space
  void segment_clip_points(const Rational& x_lower, const Rational& x_upper, const Rational& y_lower,
                           const Rational& y_upper, const Rational& y_clip, const Poly_dst_1& poly, int arcno,
                           Clip_points& clip_points, index_vector& clip_indices) {
    int n_arcs;
    int i;
    int j = 0;
    typename Clip_points::iterator it = clip_points.begin();
    Rational low;
    Rational high;
    Rational d;
    Rational l;
    Rational r;
    Gfx_DETAILED_OUT("segment: (" << m_rat2float(x_lower) << "; " << m_rat2float(x_upper) << "); arcno: " << arcno
                                  << "\n");

    while (it != clip_points.end()) {
      l = it->m_left;
      r = it->m_right;
      if (r > x_lower && l < x_upper) {
        Coordinate_1 alpha = (l == r ? Coordinate_1(r) : Coordinate_1(poly, l, r));

        // need precise comparisons in case of tight boundaries
        if (l < x_lower && alpha.compare(x_lower) != ::CGAL::LARGER) {
          it++;
          j++;
          continue;
        }
        if (r > x_upper && alpha.compare(x_upper) != ::CGAL::SMALLER) {
          it++;
          j++;
          continue;
        }
        if (it->m_arcno == -1) {
          Status_line_1 sline = m_support->status_line_for_x(alpha);
          n_arcs = sline.number_of_events();
          for (i = 0; i < n_arcs; i++) {
            Coordinate_2 xy(alpha, *m_support, i);
            Bounds _ = xy.kernel()->approximate_absolute_y_2_object()(xy, -2);
            //                     low = lbound_y(xy);
            //                     high = ubound_y(xy);
            //TODO: no need to refine y-intervals: you need only to check whether
            // polynomial vanishes at y_clip, since algebraic real you specify is exact
            if (((_.first < y_clip) && (_.second > y_clip)) || _.first == y_clip || _.second == y_clip) {
              //event.refine_to(i, engine.pixel_h_r/CGAL_REFINE_Y);
              Rational _1;
              Rational _2;
              refine_xy(xy, m_engine.m_pixel_h_r / CGAL_REFINE_Y, _1, _2);
              if (_1 <= y_clip && _2 >= y_clip) {
                it->m_arcno = i;
                it->m_alpha = alpha;
                Gfx_DETAILED_OUT("clip point assigned to arcno: " << i << "\n");
                break;
              }
            }
          }
        }
        if (it->m_arcno == arcno) {
          bool set = true;
          Coordinate_2 xy(it->m_alpha, *m_support, arcno);
          Bounds _ = xy.kernel()->approximate_absolute_y_2_object()(xy, -2);

          if (r - x_lower <= m_engine.m_pixel_w_r) {
            //                     d = ubound_y(xy) - y_lower;
            d = _.second - y_lower;
            set = (CGAL_ABS(d) > m_engine.m_pixel_h_r);
          }
          else if (x_upper - r <= m_engine.m_pixel_w_r) {
            //                     d = ubound_y(xy) - y_upper;
            d = _.second - y_upper;
            set = (CGAL_ABS(d) > m_engine.m_pixel_h_r);
          }
          if (set) {
            Gfx_DETAILED_OUT("clip-point found\n");
            clip_indices.push_back(j); // store point index
          }
        }
      }
      it++;
      j++;
    }
  }

  //! refines an algebraic point; mode = 0: criteria specifies the length of the refined interval
  //! mode = 1: criteria specifies a point that must be lie outside the interval
  void refine_alg_point(Rational& l, Rational& r, const Poly_dst_1& poly, const Rational& criteria, int mode = 0) {
    CGAL::Sign eval_l;
    CGAL::Sign eval_m;
    Rational mid;
    eval_l = poly.sign_at(l);
    while (1) {
      if (mode == 0) {
        if (r - l < criteria) break;
      }
      else if (l > criteria || r < criteria)
        break;
      mid = (l + r) / 2;
      eval_m = poly.sign_at(mid);
      if (eval_m == CGAL::EQUAL)
        l = r = mid;
      else if (eval_m == eval_l)
        l = mid;
      else
        r = mid;
    }
  }

  //! recursively checks whether only one curve branch crosses a line segment
  bool recursive_check(int var, const NT& beg_, const NT& end_, const NT& key, const Poly_1& poly, int depth = 0) {
    // if polynomial is linear/quadratic and there is sign-change over interval
    // => only one curve branch is guaranteed
    //if(poly.degree() < 3)
    //  return true;

    int val_1;
    int val_2;
    int val_3;
    NT mid = (beg_ + end_) / 2;
    NT key_1;
    NT key_2;
    NT beg = beg_;
    NT end = end_;
    m_make_exact(mid);

    Gfx_DETAILED_OUT("executing recursive check; poly = " << poly << std::endl);
    val_1 = m_engine.evaluate_generic(var, beg, key, poly);
    val_3 = m_engine.evaluate_generic(var, end, key, poly);

    Gfx_DETAILED_OUT("beg: " << val_1 << " end: " << val_3 << std::endl);
    // no sing change: at least two curve branches inside
    if ((val_1 ^ val_3) == 0) return false;

    val_2 = m_engine.evaluate_generic(var, mid, key, poly);
    // select the half with even number of intersections
    if ((val_1 ^ val_2) == 0)
      key_1 = beg;
    else
      key_1 = end;
    if (get_range_1(var, key_1, mid, key, poly)) return false; // at least three intersections over the interval: done
    key_2 = beg + end - key_1;

    get_range_1(var, key_2, mid, key, poly, 3);

    // the first derivative does not straddle zero then only one intersection
    if (! m_engine.m_first_der) return true;
    // if second derivative does not stranddle zero - only 1 first derivative
    //if(engine.second_der == 0)
    //return true;
    if (depth > CGAL_DER_CHECK_DEPTH) return true;
    return recursive_check(var, key_2, mid, key, poly, depth + 1);
  }

  //! computes lower/upper boundaries for pixel's neighborhood
  void get_boundaries(int var, const Pixel_2& pix, Stripe& stripe) {
    int level = pix.m_level;
    int val = pix.m_y;
    Integer sub = pix.m_sub_y;
    Integer cur_sub;
    if (var == CGAL_Y_RANGE) {
      sub = pix.m_sub_x;
      val = pix.m_x;
    }
    cur_sub = sub;
    if (level > 0) {
      cur_sub += 2; // obtain local coordinates for upper boundary
      // for even boundaries raise the level up
      while (level > 0 && ((cur_sub & 1) == 0)) {
        level--;
        cur_sub >>= 1;
      }
    }
    stripe.m_level[1] = level;
    if (level == 0) cur_sub = 2 - cur_sub;
    stripe.m_key[1] = val + NT(cur_sub) / NT(m_one << level);
    m_make_exact(stripe.m_key[1]);
    level = pix.m_level;
    cur_sub = sub - 1;
    while (level > 0 && ((cur_sub & 1) == 0)) {
      level--;
      cur_sub >>= 1;
    }
    stripe.m_level[0] = level;
    stripe.m_key[0] = val + NT(cur_sub) / NT(m_one << level);
    m_make_exact(stripe.m_key[0]);
  }

  //! obtains precached polynomials
  inline void get_polynomials(int var, Stripe& stripe) {
    m_engine.get_precached_poly(var, stripe.m_key[1], stripe.m_level[1], stripe.m_poly[1]);
    m_engine.get_precached_poly(var, stripe.m_key[0], stripe.m_level[0], stripe.m_poly[0]);
  }

  /*! checks 8-pixel neighborhood of a pixel, returns \c true if
   * only one curve branch intersects pixel's neighborhood, \c dir
   * defines backward direction, \c new_dir is a new tracking direction
   *
   * if \c CGAL_CKVA_RENDER_WITH_REFINEMENT is set, in case of success \c pix
   * receives double approximations of intersection point
   */
  bool test_neighborhood(Pixel_2& pix, int dir, int& new_dir) {
    NT lvl = NT(m_one << pix.m_level);
    NT inv = NT(1.0) / lvl;
    m_make_exact(inv);
    int back_dir = dir;
    int idx[5];
    int n_pts = 5;
    int n_sign;
    int shift;
    int i;
    int j;
    int res;
    int var;
    int s_shift = 0;
    int tmp;
    // box[0].key[0] - x-coordinate of left pixel boundary
    // box[0].key[1] - x-coordinate of right pixel boundary
    // box[1].key[0] - y-coordinate of bottom pixel boundary
    // box[1].key[1] - y-coordinate of top pixel boundary
    Stripe box[2]; // 0 - left-right stripe, 1 - bottom-top stripe
    NT p1;
    NT p2;
    NT lower;
    get_boundaries(CGAL_X_RANGE, pix, box[1]);
    get_boundaries(CGAL_Y_RANGE, pix, box[0]);

    struct {      // local point descriptor
      NT m_key;   // respective "key"
      int m_flag; // 0th bit: 0: lower polynomial(left/bottom);
                  //          1: upper polynomial(right/top)
                  // 1st bit: 0: x-range poly; 1: y-range poly
                  // 2nd bit: 0: c is in lower poly; 1: c is in upper poly
                  // (for regular points only)
    } pts[6] = {{box[0].m_key[0], 0}, {box[0].m_key[0], 1}, {box[0].m_key[1], 1 | 4}, {box[0].m_key[1], 0 | 4}};

    int ix = internal::directions[back_dir].m_x;
    int e_dir = back_dir;
    int _3and5 = 0;
    int _3and7 = 0;
    if (back_dir & 1) { // 5 and 3: 4; 7 and 1: 0
      _3and5 = (e_dir == 5 || e_dir == 3);
      _3and7 = e_dir & 2;
      e_dir = _3and5 ? 4 : 0;
    }
    int _4and6 = e_dir >> 2;
    int _2and6 = (e_dir & 2) >> 1;
    int _2and4 = (e_dir == 2) || (e_dir == 4);

    pts[4].m_key = box[_2and6].m_key[_4and6] + (1 - (_4and6 << 1)) * inv;
    pts[4].m_flag = (_2and6 << 1) + _2and4;
    pts[5].m_key = pts[4].m_key;
    pts[5].m_flag = pts[4].m_flag ^ 1; //(_2and6<<1) + (!_2and4); // pts[4].flag^1 ?
    idx[0] = 4;
    idx[3] = 5;
    if (back_dir & 1) (_3and7) ? pts[5].m_key += ix * inv : pts[4].m_key += ix * inv;
    shift = (6 - e_dir) >> 1;
    n_pts = 4;
    idx[1] = (1 + shift) & 3;
    idx[2] = (2 + shift) & 3;
    /*  NT xx, yy;
        if(var == CGAL_X_RANGE) {
            xx = pts[idx[i]].key;
            yy = box[shift].key[fl&1];
        } else {
            yy = pts[idx[i]].key;
            xx = box[shift].key[fl&1];
        }
        xx = xx*20+ofsxx;
        yy = yy*20+100;
        if(i == 0)
            ppnt->moveTo(int(to_double(xx)),700-int(to_double(yy)));
        else
            ppnt->lineTo(int(to_double(xx)),700-int(to_double(yy)));
        prev = curr;
    }*/

    Gfx_DETAILED_OUT("test_neigh for " << pix << "; dir = " << dir << "\n");

    n_sign = 0;
    get_polynomials(CGAL_Y_RANGE, box[0]);
    get_polynomials(CGAL_X_RANGE, box[1]);
    int f_der = -1;
    int corner_dir = -1;
    bool set_coincide = false;
    // bool s_change = false;
    new_dir = -1;
    for (i = 0; i < n_pts - 1; i++) {
      int f1 = pts[idx[i]].m_flag;
      int f2 = pts[idx[i + 1]].m_flag;
      var = CGAL_Y_RANGE;
      p1 = pts[idx[i]].m_key;
      p2 = pts[idx[i + 1]].m_key;
      if ((f1 & 2) + (f2 & 2) == 0) { // both can use x-range polynomials
                                      // different polys in x-range - use y-range instead
        if ((f1 & 1) != (f2 & 1)) {
          shift = (f1 & 4) >> 2;
          p1 = box[1].m_key[0];
          p2 = box[1].m_key[1];
        }
        else { // same polys in x-range
          shift = f1 & 1;
          var = CGAL_X_RANGE;
          shift += 2;
        }
      }
      else {
        if ((f1 & 2) == 0) { // first point is in x-range (convert to y-range)
          shift = f2 & 1;
          p1 = box[1].m_key[f1 & 1];
        }
        else { // second point is in x-range
          shift = f1 & 1;
          p2 = box[1].m_key[f2 & 1];
        }
      }
      if (p1 > p2) {
        NT tmp1 = p1;
        p1 = p2;
        p2 = tmp1;
      }
      res = m_float2int((p2 - p1) * lvl);
      int local_sign = 0;
      for (j = 0; j < res; j++, p1 += inv) {
        NT lkey = box[shift >> 1].m_key[shift & 1];
        const Poly_1& lpoly = box[shift >> 1].m_poly[shift & 1];

        if (get_range_1(var, p1, p1 + inv, lkey, lpoly, 3) || m_engine.m_zero_bounds) {
          Gfx_DETAILED_OUT("range including 0 found at: " << i << "; subseg: " << j
                                                          << "; first der = " << m_engine.m_first_der << std::endl);

          if (m_engine.m_zero_bounds) {
            Gfx_DETAILED_OUT("also with zero_bounds");

            bool is_corner = false;
            int diff = m_float2int(var == CGAL_X_RANGE ? ((p1 - pix.m_x) * lvl - pix.m_sub_x)
                                                       : ((p1 - pix.m_y) * lvl - pix.m_sub_y));
            int low_sign = m_engine.evaluate_generic(var, p1, lkey, lpoly);

            if (diff != 0) {
              // diff == -1: test lower corner: p1
              // diff ==  1: test upper corver: p1+inv
              if (diff == 1) {
                is_corner = (m_engine.evaluate_generic(var, p1 + inv, lkey, lpoly) == 0);
                // in case there is intersection at lower point =>
                // it is counted as normal intersection
                if (! is_corner && low_sign != 0) continue;
              }
              else {
                is_corner = (low_sign == 0);
                if (! is_corner) continue;
              }

              if (is_corner) {
                // compute corner direction
                tmp = internal::DIR_MAP[shift][diff + 1];
                Gfx_DETAILED_OUT("corner direction detected: " << tmp << "\n");

                if (corner_dir == -1)
                  corner_dir = tmp;
                else if (corner_dir != tmp) {
                  Gfx_DETAILED_OUT("different corner \
                                    directions found\n");
                  return false;
                }
                else {
                  // decrement since sign change occurs in the
                  // corner
                  local_sign--;
                  Gfx_DETAILED_OUT("corner found: sign: " << n_sign << "; local: " << local_sign << "\n");
                }
              }
            }
            else if (low_sign != 0)
              continue;
          }
          local_sign++;
          // Gfx_DETAILED_OUT("a curve branch detected at interval: (" << i
          //   << "; " << i+1 << ") segment: " << j << std::endl);
          if (local_sign > 1) {
            Gfx_DETAILED_OUT("more than 1 curve branch detected" << std::endl);
            return false;
          }
          lower = p1;
          s_shift = shift;
          f_der = m_engine.m_first_der;
          // s_change = engine.sign_change;
        }
      }
      n_sign += local_sign;
      if (local_sign == 1) {
        tmp = m_float2int(var == CGAL_X_RANGE ? // bottom/top
                            ((lower - pix.m_x) * lvl - pix.m_sub_x)
                                              : ((lower - pix.m_y) * lvl - pix.m_sub_y));
        tmp = internal::DIR_MAP[s_shift][tmp + 1];
        //             if(new_dir != -1&&new_dir != tmp) // more than 1 direction found
        //                 return false;
        //             else {
        //                 if(!branches_coincide&&
        //                     (current_level < CGAL_COINCIDE_LEVEL))
        //                     return false; // level is too small
        //                 set_coincide = true; // diagonal coincide mode
        //                 //std::cerr << "diagonal coincide mode\n";
        //             }
        new_dir = tmp;
        if (n_sign > 1) {
          Gfx_DETAILED_OUT("test_neigh: too many intersections found");
          return false;
        }
      }
    }

    //Gfx_DETAILED_OUT("processing segment: [" << lower << "; " << lower+inv <<
    //  "]" <<std::endl);
    if (n_sign == 0) {
      Gfx_DETAILED_OUT("ERROR: test_neigh: no sign changes detected..\n");
      return false;
    }
    var = ((s_shift & 2) ? CGAL_X_RANGE : CGAL_Y_RANGE);
    int ibox = s_shift >> 1;
    int ikey = s_shift & 1;
    if (f_der == -1) { // need to compute the first derivative
      get_range_1(var, lower, lower + inv, box[ibox].m_key[ikey], box[ibox].m_poly[ikey], 3);
      f_der = m_engine.m_first_der;
    }

    // if coincide already set - no need to check the derivative
    //     if(!set_coincide && f_der &&
    //         (!s_change || !recursive_check(var,lower,lower+inv,
    //         box[ibox].key[ikey], box[ibox].poly[ikey]))) {
    if (! set_coincide && f_der &&
        ! recursive_check(var, lower, lower + inv, box[ibox].m_key[ikey], box[ibox].m_poly[ikey])) {
      if (! m_branches_coincide && (m_current_level < CGAL_COINCIDE_LEVEL)) //||direction_taken == -1))
        return false;
      if (! m_branches_coincide) Gfx_OUT("Branches coincide at pixel: " << pix << std::endl);
      set_coincide = true;
    }

    int taken = internal::DIR_TAKEN_MAP[new_dir];
    if (taken != -1 && taken != m_direction_taken) {
      Gfx_DETAILED_OUT("\nERROR: wrong direction " << new_dir << " at pixel: " << pix << "; back_dir = " << back_dir
                                                   << "; taken = " << m_direction_taken << std::endl);
      if (back_dir == 2)
        new_dir = 6;
      else if (back_dir == 6)
        new_dir = 2;
      else
        return false;
    }
    if (set_coincide) {
      m_branches_coincide = true;
#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT

      if (var == CGAL_X_RANGE) {
        Gfx_OUT("WARNING: unable to approximate point \
                 in vertical coincide mode!\n");
        //             NT xx = lower+inv/2, yy = box[ibox].key[ikey];
        //             xx = engine.x_min + xx*engine.pixel_w;
        //             yy = engine.y_min + yy*engine.pixel_h;

        pix.xv = NAN; // mark this point as invalid (to be further skipped)
        pix.yv = NAN;
      }
      else {
        NT seed = box[ibox].m_key[ikey];
        seed = m_engine.m_x_min + seed * m_engine.m_pixel_w;

        Gfx_DETAILED_OUT("approximating in coincide mode: " << seed << "\n");
        Coordinate_2 xy(Coordinate_1(Rational(seed)), *m_support, m_arcno);
        Rational yseed;
        Rational _;
        refine_xy(xy, CGAL_REFINE_DOUBLE_APPROX, _, yseed);
        //             Rational yseed = ubound_y(xy);
        pix.xv = CGAL::to_double(seed);
        pix.yv = CGAL::to_double(yseed);
      }
#endif
    }
#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
    else {
      Gfx_DETAILED_OUT("new dir = " << new_dir << "; compute_double_approx for lower = " << lower
                                    << "; key = " << box[ibox].m_key[ikey] << "\n");
      compute_double_approx(var, lower, lower + inv, box[ibox].m_key[ikey], box[ibox].m_poly[ikey], pix);
    }
#endif
    return true;
  }

#ifdef CGAL_CKVA_RENDER_WITH_REFINEMENT
  bool compute_double_approx(int var, const NT& l_, const NT& r_, const NT& key, const Poly_1& poly, Pixel_2& pix) {
    NT l(l_);
    NT r(r_);
    if (l > r) {
      l = r_;
      r = l_;
    }

    bool ret = true;
    int eval1;
    int eval2;
    int mid_eval;
    NT threshold(CGAL_REFINE_DOUBLE_APPROX);
    eval1 = m_engine.evaluate_generic(var, l, key, poly);
    if (eval1 == 0) goto Lexit;

    eval2 = m_engine.evaluate_generic(var, r, key, poly);
    if (eval2 == 0) {
      l = r;
      goto Lexit;
    }

    if (eval1 == eval2) {
      std::cerr << "ERROR: no sign change in compute_double_approx: " << eval1 << " and " << eval2 << "\n";
      l = (l + r) / NT(2);
      m_make_exact(l);
      ret = false;
      goto Lexit;
    }

    while ((r - l) > threshold) {
      NT mid = (l + r) / NT(2);
      m_make_exact(mid);
      //Gfx_DETAILED_OUT("current approx: " << (r - l) << "; threshold = " << threshold << "\n");
      //Gfx_DETAILED_OUT("l = " << l << "; r = " << r << "; mid = " << mid <<  "\n");
      mid_eval = m_engine.evaluate_generic(var, mid, key, poly);
      if (mid_eval == 0) {
        l = r = mid;
        break;
      }
      if (mid_eval == eval1)
        l = mid;
      else
        r = mid;
    }
  Lexit:
    NT x = l;
    NT y = key;
    if (var == CGAL_Y_RANGE) {
      x = key;
      y = l;
    }
    pix.xv = CGAL::to_double(m_engine.m_x_min + x * m_engine.m_pixel_w);
    pix.yv = CGAL::to_double(m_engine.m_y_min + y * m_engine.m_pixel_h);
    return ret;
  }
  /*! \copydoc test_neighborhood
 *  \deprecated please use #test_neighborhood */
  CGAL_DEPRECATED bool test_neighbourhood(Pixel_2& pix, int dir, int& new_dir) {
    return test_neighborhood(pix, new_dir);
  }

#endif // CGAL_CKVA_RENDER_WITH_REFINEMENT

  //! returns whether a polynomial has zero over an interval,
  //! we are not interested in concrete values
  //!
  //! a bit mask \c check indicates which boundaries are to be computed
  //! 0th bit - of a polynomial itself (0th derivative, default)
  //! 1st bit - of the first derivative (sets \c first_der flag)
  //! 2nd bit - of the second derivative
  inline bool get_range_1(int var, const NT& lower, const NT& upper, const NT& key, const Poly_1& poly, int check = 1) {
    if (m_IA_method == 0) return m_engine.get_range_QF_1(var, lower, upper, key, poly, check);

    return m_engine.get_range_MAA_1(var, lower, upper, key, poly, check);
  }

  //! copmutes an isolating interval for y-coordinate of an end-point,
  //! uses caching if possible
  Rational get_endpoint_y(const Arc_2& arc, const Rational& x, CGAL::Arr_curve_end end, bool is_clipped,
                          Rational refine_bound) {
    int arcno;
    Coordinate_2 xy;

    if (arc.location(end) != CGAL::ARR_LEFT_BOUNDARY && arc.location(end) != CGAL::ARR_RIGHT_BOUNDARY && ! is_clipped) {
      arcno = arc.curve_end(end).arcno();
      xy = Coordinate_2(arc.curve_end_x(end), arc.curve_end(end).curve(), arcno);
    }
    else {
      arcno = arc.arcno();
      xy = Coordinate_2(Coordinate_1(x), *m_support, arcno);
      //refine_bound = engine.pixel_h_r / 6; //! use other bound HACK
    }

    // refine the y-interval until its size is smaller than the pixel size
    ////////////////////////// CHANGED REFINE_Y by REFINE_X
    Rational _1;
    Rational _2;
    refine_xy(xy, refine_bound, _1, _2);
    ////////////////////////// CHANGED REFINE_Y by REFINE_X
    return _2; //event.upper_boundary(arcno);
  }

  //! switches to a certain cache instance depending on currently used
  //! algebraic curve
  void select_cache_entry(const Arc_2& arc) {
    int cid = arc.curve().id();
    typename Cache_list::iterator it = m_cache_list.begin();
    while (it != m_cache_list.end()) {
      if (it->first == cid) break;
      it++;
    }

    std::size_t new_id;
    bool new_entry = (it == m_cache_list.end());
    if (new_entry) {
      new_id = m_cache_list.size();
      if (new_id >= CGAL_N_CACHES) {
        new_id = m_cache_list.back().second;
        m_cache_list.pop_back();
      }
      m_cache_list.insert(m_cache_list.begin(), LRU_entry(cid, new_id));
    }
    else { // mark this entry as MRU
      new_id = it->second;
      LRU_entry lru = *it;
      m_cache_list.erase(it);
      m_cache_list.insert(m_cache_list.begin(), lru);
    }

    if (m_cache_id != new_id) m_clip_pts_computed = false;
    m_cache_id = new_id;

    m_support = m_support_ + m_cache_id;
    m_engine.select_cache_entry(m_cache_id);

    while (! m_seed_stack.empty()) m_seed_stack.pop();

    if (new_entry) {
      *m_support = arc.curve();
      m_engine.precompute(m_support->polynomial_2());
    }
  }

  //! returns one of 4 subpixels of a pixel which is crossed by the curve
  //! branch
  //!
  //! subpixels are chosen with the priority \c dir which defines a preferred
  //! direction, i.e. right(0), top(1), left(2) or bottom(3) this is only for h/v
  //! directions
  int get_subpixel_hv(const Pixel_2& pix, int dir) {
    Poly_1 box[4];
    NT inv = NT(1) / NT(m_one << pix.m_level);
    NT inv_2;
    m_make_exact(inv);
    inv_2 = inv / 2;
    m_make_exact(inv_2);
    NT pix_x = pix.m_x + pix.m_sub_x * inv;
    NT pix_y = pix.m_y + pix.m_sub_y * inv;
    NT s_c;
    NT s_key;
    NT inc_c;
    NT inc_key;
    int var;
    int inv_var;
    if (dir & 1) { // 1 or 3 (vertical direction)
      var = CGAL_X_RANGE;
      s_c = pix_x;
      s_key = pix_y;
    }
    else { // 0 or 2 (horizontal direction)
      var = CGAL_Y_RANGE;
      s_c = pix_y;
      s_key = pix_x;
    }
    inc_key = ((dir & 2) - 1) * inv_2;
    inc_c = (1 - 2 * ((dir >> 1) ^ (dir & 1))) * inv_2;
    inv_var = 1 - var;
    s_c = s_c + inv_2 - inc_c;
    s_key = s_key + inv_2 - inc_key;
    m_engine.get_precached_poly(var, s_key, pix.m_level, box[0]);

    //Gfx_DETAILED_OUT("hv subdpixel" << std::endl);
    int p0 = m_engine.evaluate_generic(var, s_c, s_key, box[0]);         // 0
    int p1 = m_engine.evaluate_generic(var, s_c + inc_c, s_key, box[0]); // 1
    if (p0 ^ p1) return 0;

    int p2 = m_engine.evaluate_generic(var, s_c + inc_c * 2, s_key, box[0]); // 2
    if (p2 ^ p1) return 1;

    m_engine.get_precached_poly(inv_var, s_c, pix.m_level, box[1]);
    int p3 = m_engine.evaluate_generic(inv_var, s_key + inc_key, s_c, box[1]); // 3
    if (p3 ^ p0) return 0;

    m_engine.get_precached_poly(inv_var, s_c + inc_c * 2, pix.m_level, box[2]);
    int p4 = m_engine.evaluate_generic(inv_var, s_key + inc_key, s_c + inc_c * 2, box[2]);

    if (p4 ^ p2) return 1;

    int p5 = m_engine.evaluate_generic(inv_var, s_key + inc_key * 2, s_c, box[1]);
    if (p3 ^ p5) return 2;

    int p7 = m_engine.evaluate_generic(inv_var, s_key + inc_key * 2, s_c + inc_c * 2,
                                       box[2]); // 7
    if (p4 ^ p7) return 3;

    m_engine.get_precached_poly(var, s_key + inc_key * 2, pix.m_level, box[3]);
    int p6 = m_engine.evaluate_generic(var, s_c + inc_c, s_key + inc_key * 2,
                                       box[3]); // 6
    if (p5 ^ p6) return 2;
    if (p6 ^ p7) return 3;
    return -1;
  }

  //! computes the same for diagonal directions
  //!
  //! preferred direction: NE(0), NW(1), SW(2), SE(3)
  int get_subpixel_diag(const Pixel_2& pix, int dir) {
    Poly_1 box[4];
    NT inv = NT(1) / NT(m_one << pix.m_level);
    m_make_exact(inv);
    NT inv_2 = inv / 2;
    m_make_exact(inv_2);
    NT pix_x = pix.m_x + pix.m_sub_x * inv;
    NT pix_y = pix.m_y + pix.m_sub_y * inv;
    NT key_1;
    NT key_2;
    NT inc_1;
    NT inc_2;
    int var;
    int inv_var;
    if (dir & 1) { // 1 or 3 (vertical direction)
      var = CGAL_X_RANGE;
      key_1 = pix_x;
      key_2 = pix_y;
    }
    else { // 0 or 2 (horizontal direction)
      var = CGAL_Y_RANGE;
      key_1 = pix_y;
      key_2 = pix_x;
    }
    inc_1 = (2 * ((dir >> 1) ^ (dir & 1)) - 1) * inv_2;
    inc_2 = ((dir & 2) - 1) * inv_2;
    inv_var = 1 - var;
    key_1 = key_1 + inv_2 - inc_1;
    key_2 = key_2 + inv_2 - inc_2;

    m_engine.get_precached_poly(var, key_2, pix.m_level, box[0]);
    int p0 = m_engine.evaluate_generic(var, key_1, key_2, box[0]);         // 0
    int p1 = m_engine.evaluate_generic(var, key_1 + inc_1, key_2, box[0]); // 1
    //Gfx_DETAILED_OUT("p0: " << p0 << "; p1: " << p1 << std::endl);

    if (p0 ^ p1) return 0;

    m_engine.get_precached_poly(inv_var, key_1, pix.m_level, box[1]);
    int p2 = m_engine.evaluate_generic(inv_var, key_2 + inc_2, key_1, box[1]); // 2
    //Gfx_DETAILED_OUT << "p2: " << p2 << std::endl;
    if (p2 ^ p0) return 0;

    int p3 = m_engine.evaluate_generic(var, key_1 + inc_1 * 2, key_2, box[0]); // 3
    //Gfx_DETAILED_OUT << "p3: " << p3 << std::endl;
    if (p3 ^ p1) return 1;

    int p4 = m_engine.evaluate_generic(inv_var, key_2 + inc_2 * 2, key_1, box[1]);
    //Gfx_DETAILED_OUT << "p4: " << p4 << std::endl;
    if (p4 ^ p2) return 2;

    m_engine.get_precached_poly(inv_var, key_1 + inc_1 * 2, pix.m_level, box[2]);
    int p5 = m_engine.evaluate_generic(inv_var, key_2 + inc_2, key_1 + inc_1 * 2, box[2]);
    //Gfx_DETAILED_OUT << "p5: " << p5 << std::endl;
    if (p3 ^ p5) return 1;

    m_engine.get_precached_poly(var, key_2 + inc_2 * 2, pix.m_level, box[3]);
    int p6 = m_engine.evaluate_generic(var, key_1 + inc_1, key_2 + inc_2 * 2,
                                       box[3]); // 6
    //Gfx_DETAILED_OUT << "p6: " << p6 << std::endl;
    if (p4 ^ p6) return 2;

    int p7 = m_engine.evaluate_generic(var, key_1 + inc_1 * 2, key_2 + inc_2 * 2,
                                       box[3]); // 7
    //Gfx_DETAILED_OUT << "p7: " << p7 << std::endl;
    if (p5 ^ p7) return 3;

    if (p6 ^ p7) return 3;
    return -1;
  }

  //! attempts to find an isolating box for an end-point whose upper/lower
  //! boundaries do not intersect with a curve
  bool get_isolating_box(const Rational& x_s, const Rational& y_s, Pixel_2& res) {
    Integer lvl = m_one;
    Rational x_seed;
    Rational y_seed;
    get_pixel_coords(x_s, y_s, res, &x_seed, &y_seed);
    res.m_level = 0;
    res.m_sub_x = 0;
    res.m_sub_y = 0;

    NT inv = NT(1);
    NT bottom = NT(res.m_y);
    NT top = bottom + inv;
    NT left = NT(res.m_x);

    Poly_1 poly_btm;
    Poly_1 poly_top;
    m_engine.get_precached_poly(CGAL_X_RANGE, bottom, 0, poly_btm);
    m_engine.get_precached_poly(CGAL_X_RANGE, top, 0, poly_top);

    while (1) {
      inv = NT(1) / NT(lvl);
      m_make_exact(inv);
      left = NT(res.m_x) + NT(res.m_sub_x) * inv;

      Gfx_DETAILED_OUT("processing pixel: " << res << std::endl);
      if (get_range_1(CGAL_X_RANGE, left, left + inv, bottom, poly_btm) && ! m_engine.m_zero_bounds) {
        Gfx_DETAILED_OUT("bottom boundary intersection found at pixel: " << res << std::endl);
      }
      else {
        if (! get_range_1(CGAL_X_RANGE, left, left + inv, top, poly_top)) {
          Gfx_DETAILED_OUT("no bottom/top intersections found at pixel:" << res << std::endl);
          break;
        }
        else {
          Gfx_DETAILED_OUT("top boundary intersection found at pixel: " << res << std::endl);
        }
      }
      lvl <<= 1;
      /*if(lvl > CGAL_REFINE_Y) {
            event.refine_to(arcno, pixel_h_r/(lvl*2));
            y_seed = (event.lower_boundary(arcno) +
                event.upper_boundary(arcno))/2;
        }*/
      if (res.m_level >= s_max_subdivision_level) {
        std::cerr("get_isolating_box: reached maximum subdivision level " << res.m_level << std::endl);
        return false;
      }
      res.m_level++;
      res.m_sub_x = m_rat2integer((x_seed - res.m_x) * lvl);
      res.m_sub_y = m_rat2integer((y_seed - res.m_y) * lvl);
      if (res.m_sub_x < 0) res.m_sub_x = 0;
      res.m_sub_x &= (lvl - 1);
      if (res.m_sub_y < 0) res.m_sub_y = 0;
      res.m_sub_y &= (lvl - 1);
    }
    return true;
  }

  //! returns true if \c pix is encompassed into one of isolating rectangles
  //! (stopping criteria)
  inline bool is_isolated_pixel(const Pixel_2& /* pix */) {
    /*Integer sub_x;
    if(isolated_l.level != -1u&&isolated_l.y == pix.y&&isolated_l.x == pix.x&&
        pix.level >= isolated_l.level) {
        sub_x = pix.sub_x >> (pix.level - isolated_l.level);
        if(CGAL_ABS(sub_x - isolated_l.sub_x) <= 1)
            return true;
    }
    if(isolated_h.level != -1u&&isolated_h.y == pix.y&&isolated_h.x == pix.x&&
        pix.level >= isolated_h.level) {
        sub_x = pix.sub_x >> (pix.level - isolated_h.level);
        if(CGAL_ABS(sub_x - isolated_h.sub_x) <= 1)
            return true;
    }*/
    return false;
  }

// DEBUG ONLY
#ifdef Gfx_USE_OUT
  void dump_neighborhood(const Pixel_2& pix) {
    CGAL::IO::set_mode(std::cerr, CGAL::IO::PRETTY);
    CGAL::IO::set_mode(std::cout, CGAL::IO::PRETTY);

    Stripe box[2]; // 0 - left-right stripe, 1 - bottom-top stripe
    //NT inv = NT(1) / NT(one << pix.level);
    get_boundaries(CGAL_X_RANGE, pix, box[1]);
    get_boundaries(CGAL_Y_RANGE, pix, box[0]);
    NT bottom = box[1].m_key[0];
    NT top = box[1].m_key[1];
    NT left = box[0].m_key[0];
    NT right = box[0].m_key[1];

    Gfx_OUT("\n\nComputing nighbourhood for pixel: " << pix << std::endl);
    get_polynomials(CGAL_X_RANGE, box[1]);
    get_polynomials(CGAL_Y_RANGE, box[0]);
    NT a;
    NT b;
    NT range;
    NT inc;
    NT val;
    ///////////////////////////////////////////////////////////////////////
    Gfx_OUT("\nevaluate RIGHT side:" << std::endl);
    range = top - bottom;
    inc = range / 3;
    val = bottom;

    if (get_range_1(CGAL_Y_RANGE, bottom, top, right, box[0].m_poly[1]))
      Gfx_OUT("segment RIGHT registered" << std::endl);
    if (get_range_1(CGAL_Y_RANGE, val, val + inc, right, box[0].m_poly[1]))
      Gfx_OUT("segment 0 registered" << std::endl);

    a = m_engine.evaluate_generic(CGAL_Y_RANGE, val, right, box[0].m_poly[1]);
    Gfx_OUT("val = " << a << std::endl);
    val += inc;

    b = m_engine.evaluate_generic(CGAL_Y_RANGE, val, right, box[0].m_poly[1]);
    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 0" << std::endl);
    a = b;
    if (get_range_1(CGAL_Y_RANGE, val, val + inc, right, box[0].m_poly[1]))
      Gfx_OUT("segment 1 registered" << std::endl);
    val += inc;
    b = m_engine.evaluate_generic(CGAL_Y_RANGE, val, right, box[0].m_poly[1]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 1" << std::endl);
    if (get_range_1(CGAL_Y_RANGE, val, val + inc, right, box[0].m_poly[1]))
      Gfx_OUT("segment 2 registered" << std::endl);
    a = b;
    val = top;

    Gfx_OUT("poly right: " << box[0].m_poly[1] << " at (" << val << "; " << right << ")\n");

    b = m_engine.evaluate_generic(CGAL_Y_RANGE, val, right, box[0].m_poly[1]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 2" << std::endl);
    ///////////////////////////////////////////////////////////////////////
    Gfx_OUT("\nevaluate BOTTOM side:" << std::endl);
    range = right - left;
    inc = range / 3;
    val = left;
    if (get_range_1(CGAL_X_RANGE, left, right, bottom, box[1].m_poly[0]))
      Gfx_OUT("segment BOTTOM registered" << std::endl);
    if (get_range_1(CGAL_X_RANGE, val, val + inc, bottom, box[1].m_poly[0]))
      Gfx_OUT("segment 0 registered" << std::endl);

    Gfx_OUT("poly bottom: " << box[1].m_poly[0] << " at (" << val << "; " << bottom << ")\n");

    a = m_engine.evaluate_generic(CGAL_X_RANGE, val, bottom, box[1].m_poly[0]);

    Gfx_OUT("val = " << a << std::endl);
    val += inc;
    b = m_engine.evaluate_generic(CGAL_X_RANGE, val, bottom, box[1].m_poly[0]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 0" << std::endl);
    a = b;
    if (get_range_1(CGAL_X_RANGE, val, val + inc, bottom, box[1].m_poly[0]))
      Gfx_OUT("segment 1 registered" << std::endl);
    val += inc;
    b = m_engine.evaluate_generic(CGAL_X_RANGE, val, bottom, box[1].m_poly[0]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 1" << std::endl);
    a = b;
    if (get_range_1(CGAL_X_RANGE, val, val + inc, bottom, box[1].m_poly[0]))
      Gfx_OUT("segment 2 registered" << std::endl);
    val = right;
    b = m_engine.evaluate_generic(CGAL_X_RANGE, val, bottom, box[1].m_poly[0]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 2" << std::endl);
    ///////////////////////////////////////////////////////////////////////
    Gfx_OUT("\nevaluate LEFT side:" << std::endl);
    range = top - bottom;
    inc = range / 3;
    val = bottom;
    if (get_range_1(CGAL_Y_RANGE, bottom, top, left, box[0].m_poly[0])) Gfx_OUT("segment LEFT registered" << std::endl);
    if (get_range_1(CGAL_Y_RANGE, val, val + inc, left, box[0].m_poly[0])) Gfx_OUT("segment 0 registered" << std::endl);

    Gfx_OUT("poly left: " << box[0].m_poly[0] << " at (" << val << "; " << left << ")\n");
    a = m_engine.evaluate_generic(CGAL_Y_RANGE, val, left, box[0].m_poly[0]);

    Gfx_OUT("val = " << a << std::endl);
    val += inc;
    b = m_engine.evaluate_generic(CGAL_Y_RANGE, val, left, box[0].m_poly[0]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 0" << std::endl);
    a = b;
    if (get_range_1(CGAL_Y_RANGE, val, val + inc, left, box[0].m_poly[0])) Gfx_OUT("segment 1 registered" << std::endl);
    val += inc;
    b = m_engine.evaluate_generic(CGAL_Y_RANGE, val, left, box[0].m_poly[0]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 1" << std::endl);
    a = b;
    if (get_range_1(CGAL_Y_RANGE, val, val + inc, left, box[0].m_poly[0])) Gfx_OUT("segment 2 registered" << std::endl);
    val = top;
    b = m_engine.evaluate_generic(CGAL_Y_RANGE, val, left, box[0].m_poly[0]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 2" << std::endl);
    ///////////////////////////////////////////////////////////////////////
    Gfx_OUT("\nevaluate TOP side:" << std::endl);
    range = right - left;
    inc = range / 3;
    val = left;
    if (get_range_1(CGAL_X_RANGE, left, right, top, box[1].m_poly[1])) Gfx_OUT("segment TOP registered" << std::endl);

    if (get_range_1(CGAL_X_RANGE, val, val + inc, top, box[1].m_poly[1])) Gfx_OUT("segment 0 registered" << std::endl);
    a = m_engine.evaluate_generic(CGAL_X_RANGE, val, top, box[1].m_poly[1]);

    Gfx_OUT("val = " << a << std::endl);
    val += inc;
    b = m_engine.evaluate_generic(CGAL_X_RANGE, val, top, box[1].m_poly[1]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 0" << std::endl);
    a = b;
    if (get_range_1(CGAL_X_RANGE, val, val + inc, top, box[1].m_poly[1])) Gfx_OUT("segment 1 registered" << std::endl);
    val += inc;
    b = m_engine.evaluate_generic(CGAL_X_RANGE, val, top, box[1].m_poly[1]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 1" << std::endl);
    a = b;

    if (get_range_1(CGAL_X_RANGE, val, val + inc, top, box[1].m_poly[1])) Gfx_OUT("segment 2 registered" << std::endl);

    val = right;

    Gfx_OUT("poly top: " << box[1].m_poly[1] << " at (" << val << "; " << top << ")\n");
    b = m_engine.evaluate_generic(CGAL_X_RANGE, val, top, box[1].m_poly[1]);

    Gfx_OUT("val = " << b << std::endl);
    if (a * b < 0) Gfx_OUT("sign change at segment 2" << std::endl);
  }
#else
  void dump_neighborhood(const Pixel_2&) {}
#endif // Gfx_USE_OUT

  CGAL_DEPRECATED void dump_neighbourhood(const Pixel_2& pix) { dump_neighborhood(pix); }

  //!@}
}; // class Curve_renderer_2<>

} // namespace CGAL

#endif
