// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Benchmark for the algebraic segment traits (Arr_algebraic_segment_traits_2)
// of the 2D Arrangements package.
//
// The program measures (i) the construction and decomposition of algebraic
// curves, (ii) arrangement-level operations (aggregate and incremental
// insertion, overlay, point location), and (iii) every operation of the
// traits separately, applied to fixed inputs. It uses only the functors of the
// traits and the documented accessors of its points and curves, so that the
// very same source compiles against a CGAL tree before and after a change of
// the traits internals. Build it once against each tree, run both executables
// with the same options, and compare the CSV files with compare_benchmarks.py
// (see run_comparison.sh).
//
// The input consists of random square-free bivariate polynomials with small
// integer coefficients, generated deterministically from the seed. All inputs
// of the functor benchmarks are prepared once, outside the timed region; every
// prepared input is tried once beforehand, and inputs that violate a
// precondition or reach an unimplemented case are dropped (their number is
// reported). Checksums are derived from exact results only (comparison
// results, counts, multiplicities, arrangement sizes), never from floating-
// point approximations.
//
// By default the caches of the curved kernel are warm: the curves are
// constructed once and reused. With --cold, the curve and arrangement
// benchmarks use, in every run, a traits that holds a fresh curved kernel,
// constructed from a new curve kernel with empty curve and curve-pair caches,
// and construct the curves inside the timed region; this requires a traits
// that can hold its own kernel and is ignored otherwise. (Points and arcs
// still use the static kernel in their member functions.)

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/config.h>

#if ! CGAL_USE_CORE
int main() {
  std::cout << "This benchmark requires CORE; skipping.\n";
  return EXIT_SUCCESS;
}
#else

#include <CGAL/CORE_BigInt.h>
#include <CGAL/Random.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_overlay_2.h>
#include <CGAL/Arr_default_overlay_traits.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arr_walk_along_line_point_location.h>
#include <CGAL/Arr_trapezoid_ric_point_location.h>

#include "bench_harness.hpp"

using Integer = CORE::BigInt;
using Traits = CGAL::Arr_algebraic_segment_traits_2<Integer>;
using Polynomial = Traits::Polynomial_2;
using Curve = Traits::Curve_2;
using Xcv = Traits::X_monotone_curve_2;
using Point = Traits::Point_2;
using Algebraic_real = Traits::Algebraic_real_1;
using Bound = Traits::Bound;
using Multiplicity = Traits::Multiplicity;
using Arr = CGAL::Arrangement_2<Traits>;
using Make_x_monotone_result = std::variant<Point, Xcv>;
using Intersection_result = std::variant<std::pair<Point, Multiplicity>, Xcv>;

constexpr CGAL::Arr_curve_end MIN_END = CGAL::ARR_MIN_END;
constexpr CGAL::Arr_curve_end MAX_END = CGAL::ARR_MAX_END;

// Whether the traits can hold its own curved kernel.
template <typename Tr>
constexpr bool has_own_kernel = std::is_constructible_v<Tr, std::shared_ptr<typename Tr::CKvA_2>>;

/*! constructs a traits; with `cold`, if supported, one that holds a fresh curved kernel with empty caches.
 *
 * A default-constructed curved kernel copies the static curve kernel, and thus shares its caches; hence the
 * curved kernel is constructed from a new curve kernel. The curved kernel stores a copy of the curve kernel,
 * which shares the caches of the original, and the caches refer back to the original; the original must
 * therefore outlive the curved kernel. Both are kept in one block, to which the returned pointer refers.
 */
template <typename Tr>
std::shared_ptr<Tr> make_traits(bool cold) {
  if constexpr (has_own_kernel<Tr>) {
    using CKvA_2 = typename Tr::CKvA_2;
    using Curve_kernel_2 = typename CKvA_2::Curve_kernel_2;
    struct Fresh_kernels {
      Curve_kernel_2 curve_kernel;
      CKvA_2 curved_kernel;
      Fresh_kernels() : curved_kernel(curve_kernel) {}
    };
    if (cold) {
      auto kernels = std::make_shared<Fresh_kernels>();
      return std::make_shared<Tr>(std::shared_ptr<CKvA_2>(kernels, &kernels->curved_kernel));
    }
  }
  return std::make_shared<Tr>();
}

//-----------------------------------------------------------------------------
// Options
//-----------------------------------------------------------------------------
struct Options {
  unsigned int seed = 1;
  std::size_t repetitions = 5;
  std::size_t warmup = 1;
  std::size_t curves = 10;      // number of input curves
  long asymptotic = -1;         // number of curves without the y^degree term; -1: half of them
  int degree = 3;               // maximal total degree of the curves
  int coefficient = 4;          // maximal absolute value of the coefficients
  std::size_t pairs = 1000;     // maximal number of inputs per pairwise benchmark
  std::size_t loops = 20;       // number of passes of a functor benchmark over its inputs
  std::size_t queries = 200;    // number of point-location queries
  double range = 3.0;           // the query points lie in [-range,range]^2
  bool cold = false;            // fresh curved kernel in the curve and arrangement benchmarks
  std::string filter;           // run only benchmarks whose name contains it
  std::string csv;              // optional CSV output file
  bool list = false;
};

static void usage(const char* prog) {
  std::cout <<
    "Usage: " << prog << " [options]\n"
    "  -s, --seed <n>          random seed (default 1)\n"
    "  -r, --repetitions <n>   timed repetitions per benchmark (default 5)\n"
    "  -w, --warmup <n>        untimed warm-up runs per benchmark (default 1)\n"
    "  -n, --curves <n>        number of input curves (default 10)\n"
    "  -a, --asymptotic <n>    number of curves generated without the y^d\n"
    "                          monomial, whose leading coefficient in y thus\n"
    "                          depends on x, which typically yields vertical\n"
    "                          asymptotes (default: half of the curves)\n"
    "  -d, --degree <n>        maximal total degree d of the curves (default 3)\n"
    "  -c, --coefficient <n>   maximal absolute value of the integer\n"
    "                          coefficients (default 4)\n"
    "  -p, --pairs <n>         maximal number of inputs of each pairwise\n"
    "                          benchmark; a random subset is used if there are\n"
    "                          more (default 1000)\n"
    "  -k, --loops <n>         number of passes of each functor benchmark over\n"
    "                          its inputs (default 20); the cheapest ones,\n"
    "                          compare_x_on_boundary and parameter_space, make\n"
    "                          100 and 1000 times as many passes\n"
    "  -q, --queries <n>       number of point-location queries (default 200;\n"
    "                          the naive strategy uses a tenth of them)\n"
    "  -R, --range <x>         the query points lie in [-x,x]^2 (default 3)\n"
    "      --cold              in every run of construct_curves,\n"
    "                          make_x_monotone, insert_aggregate,\n"
    "                          insert_incremental and overlay, use a traits with\n"
    "                          a fresh curved kernel (empty caches) and construct\n"
    "                          the curves while timing\n"
    "  -f, --filter <str>      run only benchmarks whose name contains <str>\n"
    "  -o, --csv <file>        also write the results as CSV to <file>\n"
    "      --list              list the benchmarks and exit\n"
    "  -h, --help              print this message\n";
}

static bool parse(int argc, char* argv[], Options& opt) {
  auto value = [&](int& i) -> const char* {
    if (i + 1 >= argc) {
      std::cerr << "Missing value for " << argv[i] << "\n";
      std::exit(EXIT_FAILURE);
    }
    return argv[++i];
  };
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "-h" || a == "--help") {
      usage(argv[0]);
      std::exit(EXIT_SUCCESS);
    }
    else if (a == "-s" || a == "--seed") opt.seed = std::stoul(value(i));
    else if (a == "-r" || a == "--repetitions") opt.repetitions = std::stoul(value(i));
    else if (a == "-w" || a == "--warmup") opt.warmup = std::stoul(value(i));
    else if (a == "-n" || a == "--curves") opt.curves = std::stoul(value(i));
    else if (a == "-a" || a == "--asymptotic") opt.asymptotic = std::stol(value(i));
    else if (a == "-d" || a == "--degree") opt.degree = std::stoi(value(i));
    else if (a == "-c" || a == "--coefficient") opt.coefficient = std::stoi(value(i));
    else if (a == "-p" || a == "--pairs") opt.pairs = std::stoul(value(i));
    else if (a == "-k" || a == "--loops") opt.loops = std::stoul(value(i));
    else if (a == "-q" || a == "--queries") opt.queries = std::stoul(value(i));
    else if (a == "-R" || a == "--range") opt.range = std::stod(value(i));
    else if (a == "--cold") opt.cold = true;
    else if (a == "-f" || a == "--filter") opt.filter = value(i);
    else if (a == "-o" || a == "--csv") opt.csv = value(i);
    else if (a == "--list") opt.list = true;
    else {
      std::cerr << "Unknown option " << a << "\n";
      usage(argv[0]);
      return false;
    }
  }
  if (opt.asymptotic < 0) opt.asymptotic = static_cast<long>(opt.curves / 2);
  if (opt.repetitions == 0 || opt.curves < 2 || opt.asymptotic > static_cast<long>(opt.curves) ||
      opt.degree < 1 || opt.coefficient < 1 || opt.loops == 0 || opt.range <= 0.0) {
    std::cerr << "Invalid option values\n";
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
// Input generation
//-----------------------------------------------------------------------------
/*! generates a random square-free polynomial of total degree `degree` whose integer coefficients lie in
 * [-c, c]; every monomial of total degree at most `degree` gets a coefficient, which may be zero. If
 * `asymptotic` is true, the monomial y^degree is omitted, so that the leading coefficient in y depends on x,
 * which typically yields vertical asymptotes.
 */
static Polynomial random_polynomial(int degree, int c, bool asymptotic, CGAL::Random& rnd) {
  const Polynomial x = CGAL::shift(Polynomial(1), 1, 0);
  const Polynomial y = CGAL::shift(Polynomial(1), 1, 1);
  typename CGAL::Polynomial_traits_d<Polynomial>::Is_square_free is_square_free;
  while (true) {
    Polynomial p(0);
    bool has_top_degree = false;
    for (int i = 0; i <= degree; ++i) {
      for (int j = 0; i + j <= degree; ++j) {
        const int a = rnd.get_int(-c, c + 1);
        if (a == 0 || (asymptotic && i == 0 && j == degree)) continue;
        if (i + j == degree) has_top_degree = true;
        p += Polynomial(Integer(a)) * CGAL::ipower(x, i) * CGAL::ipower(y, j);
      }
    }
    if (has_top_degree && is_square_free(p)) return p;
  }
}

/*! obtains a rational number strictly inside the x-range of a non-vertical arc whose ends are finite or lie on
 * the left or right boundary.
 */
static Bound interior_x(const Traits& tr, const Xcv& xcv) {
  auto ps_x = tr.parameter_space_in_x_2_object();
  const bool min_finite = (ps_x(xcv, MIN_END) == CGAL::ARR_INTERIOR);
  const bool max_finite = (ps_x(xcv, MAX_END) == CGAL::ARR_INTERIOR);
  if (min_finite && max_finite) {
    typename Traits::Algebraic_kernel_d_1 ak;
    return ak.bound_between_1_object()(xcv.curve_end(MIN_END).x(), xcv.curve_end(MAX_END).x());
  }
  if (max_finite) return Bound(static_cast<int>(std::floor(CGAL::to_double(xcv.curve_end(MAX_END).x()))) - 1);
  if (min_finite) return Bound(static_cast<int>(std::ceil(CGAL::to_double(xcv.curve_end(MIN_END).x()))) + 1);
  return Bound(0);
}

/*! checks whether the x-coordinate of a point lies in the closed x-range of an arc, at an x-coordinate where the
 * arc has a point (i.e., not at a vertical asymptote).
 */
static bool in_x_range(const Traits& tr, const Point& p, const Xcv& xcv) {
  auto ps_x = tr.parameter_space_in_x_2_object();
  auto ps_y = tr.parameter_space_in_y_2_object();
  auto cmp_x = tr.compare_x_2_object();
  auto cmp_x_on_boundary = tr.compare_x_on_boundary_2_object();
  for (auto ce : {MIN_END, MAX_END}) {
    if (ps_x(xcv, ce) != CGAL::ARR_INTERIOR) continue;  // unbounded in x
    CGAL::Comparison_result res;
    if (ps_y(xcv, ce) != CGAL::ARR_INTERIOR) {
      res = cmp_x_on_boundary(p, xcv, ce);
      if (res == CGAL::EQUAL) return false;
    }
    else res = cmp_x(p, xcv.curve_end(ce));
    if ((ce == MIN_END && res == CGAL::SMALLER) || (ce == MAX_END && res == CGAL::LARGER)) return false;
  }
  return true;
}

/*! keeps at most `n` elements of `v`, chosen at random (deterministically from `rnd`).
 */
template <typename T>
static void subsample(std::vector<T>& v, std::size_t n, CGAL::Random& rnd) {
  if (v.size() <= n) return;
  for (std::size_t i = 0; i < n; ++i) std::swap(v[i], v[i + rnd.get_int(0, static_cast<int>(v.size() - i))]);
  v.resize(n);
}

//-----------------------------------------------------------------------------
// Input
//-----------------------------------------------------------------------------
struct Arc_point {
  Xcv xcv;
  Point p;       // a point in the interior of xcv
  Bound x;       // its x-coordinate
};

struct Arc_pair {
  Xcv xcv1;
  Xcv xcv2;
};

struct Arc_pair_at_point {
  Xcv xcv1;
  Xcv xcv2;
  Point p;
};

struct Arc_end_pair {
  Xcv xcv1;
  CGAL::Arr_curve_end ce1;
  Xcv xcv2;
  CGAL::Arr_curve_end ce2;
};

struct Input {
  std::shared_ptr<Traits> traits = std::make_shared<Traits>();
  std::vector<Polynomial> polynomials;
  std::vector<Curve> curves;
  std::vector<Curve> curves1;            // the first half of the curves (overlay)
  std::vector<Curve> curves2;            // the second half of the curves (overlay)
  std::vector<Xcv> xcvs;                 // the x-monotone arcs of all curves
  std::vector<Point> isolated_points;
  std::vector<Point> points;             // the finite vertices of the arrangement
  std::vector<Point> queries;            // rational query points
  std::vector<Arc_point> arc_points;     // arcs with interior points (split, ...)
  std::vector<std::pair<Xcv, Xcv>> halves;  // the two parts of a split arc (merge)
  std::vector<std::pair<Point, Point>> point_pairs;
  std::vector<std::pair<Point, Xcv>> point_arc_pairs;  // the point is in the x-range of the arc
  std::vector<std::pair<Point, Curve>> point_curve_pairs;
  std::vector<Arc_pair> arc_pairs;
  std::vector<Arc_pair_at_point> right_pairs;  // both arcs lie to the right of the point
  std::vector<Arc_pair_at_point> left_pairs;   // both arcs lie to the left of the point
  std::vector<std::pair<Arc_pair, CGAL::Arr_curve_end>> left_right_pairs;  // ends on the left/right boundary
  std::vector<Arc_end_pair> bottom_top_pairs;  // ends on the bottom/top boundary
  std::unique_ptr<Arr> arr;              // the arrangement of all curves
  std::size_t dropped = 0;               // prepared inputs dropped because they failed
};

/*! keeps the elements of `v` for which `f` does not throw.
 */
template <typename T, typename F>
static void keep_valid(std::vector<T>& v, F f, std::size_t& dropped) {
  std::vector<T> res;
  res.reserve(v.size());
  for (const auto& e : v) {
    try {
      f(e);
      res.push_back(e);
    }
    catch (...) { ++dropped; }
  }
  v.swap(res);
}

static void prepare(Input& in, const Options& opt) {
  const Traits& tr = *in.traits;
  CGAL::Random poly_rnd(opt.seed);
  CGAL::Random pair_rnd(opt.seed + 1);
  CGAL::Random query_rnd(opt.seed + 2);

  auto ctr_cv = tr.construct_curve_2_object();
  for (std::size_t i = 0; i < opt.curves; ++i) {
    // spread the asymptotic curves evenly, so that both halves (overlay) get some
    const std::size_t a = static_cast<std::size_t>(opt.asymptotic);
    const bool asymptotic = ((i + 1) * a / opt.curves > i * a / opt.curves);
    in.polynomials.push_back(random_polynomial(opt.degree, opt.coefficient, asymptotic, poly_rnd));
    in.curves.push_back(ctr_cv(in.polynomials.back()));
  }
  in.curves1.assign(in.curves.begin(), in.curves.begin() + in.curves.size() / 2);
  in.curves2.assign(in.curves.begin() + in.curves.size() / 2, in.curves.end());

  // the x-monotone arcs and isolated points
  auto make_x_monotone = tr.make_x_monotone_2_object();
  for (const auto& cv : in.curves) {
    std::vector<Make_x_monotone_result> objs;
    make_x_monotone(cv, std::back_inserter(objs));
    for (const auto& obj : objs) {
      if (const auto* xcv = std::get_if<Xcv>(&obj)) in.xcvs.push_back(*xcv);
      else in.isolated_points.push_back(std::get<Point>(obj));
    }
  }

  // the arrangement and its finite vertices
  in.arr = std::make_unique<Arr>(in.traits.get());
  CGAL::insert(*in.arr, in.curves.begin(), in.curves.end());
  for (auto vit = in.arr->vertices_begin(); vit != in.arr->vertices_end(); ++vit)
    if (! vit->is_at_open_boundary()) in.points.push_back(vit->point());

  // the query points
  auto ctr_pt = tr.construct_point_2_object();
  const int r = static_cast<int>(opt.range * 1000);
  for (std::size_t i = 0; i < opt.queries; ++i) {
    const Bound x = Bound(query_rnd.get_int(-r, r + 1)) / Bound(1000);
    const Bound y = Bound(query_rnd.get_int(-r, r + 1)) / Bound(1000);
    in.queries.push_back(ctr_pt(x, y));
  }

  // arcs with interior points
  auto ps_y = tr.parameter_space_in_y_2_object();
  auto is_vertical = tr.is_vertical_2_object();
  for (const auto& xcv : in.xcvs) {
    if (is_vertical(xcv)) continue;
    if (ps_y(xcv, MIN_END) != CGAL::ARR_INTERIOR || ps_y(xcv, MAX_END) != CGAL::ARR_INTERIOR) continue;
    try {
      const Bound x = interior_x(tr, xcv);
      in.arc_points.push_back({xcv, ctr_pt(Algebraic_real(x), xcv), x});
    }
    catch (...) { ++in.dropped; }
  }
  auto split = tr.split_2_object();
  keep_valid(in.arc_points, [&](const Arc_point& ap) {
      Xcv xcv1;
      Xcv xcv2;
      split(ap.xcv, ap.p, xcv1, xcv2);
      in.halves.emplace_back(xcv1, xcv2);
    }, in.dropped);

  // point pairs
  for (std::size_t i = 0; i < in.points.size(); ++i)
    for (std::size_t j = 0; j < in.points.size(); ++j)
      if (i != j) in.point_pairs.emplace_back(in.points[i], in.points[j]);
  subsample(in.point_pairs, opt.pairs, pair_rnd);

  // point/arc pairs where the point lies in the x-range of the (non-vertical) arc
  std::vector<Point> pts = in.points;
  for (const auto& ap : in.arc_points) pts.push_back(ap.p);
  for (const auto& p : pts)
    for (const auto& xcv : in.xcvs)
      if (! is_vertical(xcv) && in_x_range(tr, p, xcv)) in.point_arc_pairs.emplace_back(p, xcv);
  subsample(in.point_arc_pairs, opt.pairs, pair_rnd);
  auto cmp_y_at_x = tr.compare_y_at_x_2_object();
  keep_valid(in.point_arc_pairs, [&](const auto& pa) { cmp_y_at_x(pa.first, pa.second); }, in.dropped);

  // point/curve pairs
  for (const auto& p : pts)
    for (const auto& cv : in.curves) in.point_curve_pairs.emplace_back(p, cv);
  subsample(in.point_curve_pairs, opt.pairs, pair_rnd);

  // arc pairs
  for (std::size_t i = 0; i < in.xcvs.size(); ++i)
    for (std::size_t j = 0; j < in.xcvs.size(); ++j)
      if (i != j) in.arc_pairs.push_back({in.xcvs[i], in.xcvs[j]});
  subsample(in.arc_pairs, opt.pairs, pair_rnd);
  auto intersect = tr.intersect_2_object();
  keep_valid(in.arc_pairs, [&](const Arc_pair& ap) {
      std::vector<Intersection_result> res;
      intersect(ap.xcv1, ap.xcv2, std::back_inserter(res));
    }, in.dropped);

  // pairs of arcs incident to a common arrangement vertex, on its right or on its left
  for (auto vit = in.arr->vertices_begin(); vit != in.arr->vertices_end(); ++vit) {
    if (vit->is_at_open_boundary() || vit->is_isolated()) continue;
    std::vector<Xcv> right;
    std::vector<Xcv> left;
    auto first = vit->incident_halfedges();
    auto he = first;
    do {
      if (! is_vertical(he->curve())) {
        // the halfedge is directed toward the vertex; its source lies to the right if it is directed right-to-left
        if (he->direction() == CGAL::ARR_RIGHT_TO_LEFT) right.push_back(he->curve());
        else left.push_back(he->curve());
      }
    } while (++he != first);
    for (std::size_t i = 0; i < right.size(); ++i)
      for (std::size_t j = 0; j < right.size(); ++j)
        if (i != j) in.right_pairs.push_back({right[i], right[j], vit->point()});
    for (std::size_t i = 0; i < left.size(); ++i)
      for (std::size_t j = 0; j < left.size(); ++j)
        if (i != j) in.left_pairs.push_back({left[i], left[j], vit->point()});
  }
  subsample(in.right_pairs, opt.pairs, pair_rnd);
  subsample(in.left_pairs, opt.pairs, pair_rnd);
  auto cmp_right = tr.compare_y_at_x_right_2_object();
  auto cmp_left = tr.compare_y_at_x_left_2_object();
  keep_valid(in.right_pairs, [&](const auto& t) { cmp_right(t.xcv1, t.xcv2, t.p); }, in.dropped);
  keep_valid(in.left_pairs, [&](const auto& t) { cmp_left(t.xcv1, t.xcv2, t.p); }, in.dropped);

  // pairs of arc ends on the left or right boundary, and on the bottom or top boundary
  auto ps_x = tr.parameter_space_in_x_2_object();
  for (const auto& xcv1 : in.xcvs) {
    for (const auto& xcv2 : in.xcvs) {
      if (ps_x(xcv1, MIN_END) == CGAL::ARR_LEFT_BOUNDARY && ps_x(xcv2, MIN_END) == CGAL::ARR_LEFT_BOUNDARY)
        in.left_right_pairs.emplace_back(Arc_pair{xcv1, xcv2}, MIN_END);
      if (ps_x(xcv1, MAX_END) == CGAL::ARR_RIGHT_BOUNDARY && ps_x(xcv2, MAX_END) == CGAL::ARR_RIGHT_BOUNDARY)
        in.left_right_pairs.emplace_back(Arc_pair{xcv1, xcv2}, MAX_END);
      for (auto ce1 : {MIN_END, MAX_END})
        for (auto ce2 : {MIN_END, MAX_END})
          if (ps_x(xcv1, ce1) == CGAL::ARR_INTERIOR && ps_y(xcv1, ce1) != CGAL::ARR_INTERIOR &&
              ps_x(xcv2, ce2) == CGAL::ARR_INTERIOR && ps_y(xcv2, ce2) != CGAL::ARR_INTERIOR)
            in.bottom_top_pairs.push_back({xcv1, ce1, xcv2, ce2});
    }
  }
  subsample(in.left_right_pairs, opt.pairs, pair_rnd);
  subsample(in.bottom_top_pairs, opt.pairs, pair_rnd);
  auto cmp_y_near_boundary = tr.compare_y_near_boundary_2_object();
  auto cmp_x_on_boundary = tr.compare_x_on_boundary_2_object();
  keep_valid(in.left_right_pairs,
             [&](const auto& t) { cmp_y_near_boundary(t.first.xcv1, t.first.xcv2, t.second); }, in.dropped);
  keep_valid(in.bottom_top_pairs, [&](const auto& t) { cmp_x_on_boundary(t.xcv1, t.ce1, t.xcv2, t.ce2); },
             in.dropped);
}

//-----------------------------------------------------------------------------
// Benchmarks
//-----------------------------------------------------------------------------
/*! constructs the curves of the input with a given traits.
 */
static std::vector<Curve> construct_curves(const Traits& tr, const std::vector<Polynomial>& polynomials) {
  auto ctr_cv = tr.construct_curve_2_object();
  std::vector<Curve> curves;
  curves.reserve(polynomials.size());
  for (const auto& p : polynomials) curves.push_back(ctr_cv(p));
  return curves;
}

/*! adds a benchmark that applies `f` to every element of `v`, in `loops` passes, and accumulates the results
 * into a checksum.
 */
template <typename T, typename F>
static void add_loop(std::vector<Benchmark>& b, const std::string& name, const std::string& description,
                     const std::vector<T>& v, std::size_t loops, F f) {
  b.push_back({name, description, [&v, loops, f](Stopwatch& sw) mutable -> std::uint64_t {
      std::uint64_t h = v.size();
      sw.start();
      for (std::size_t k = 0; k < loops; ++k)
        for (const auto& e : v) h = mix(h, static_cast<std::uint64_t>(f(e)));
      sw.stop();
      return h;
    }});
}

/*! adds a point-location benchmark.
 */
template <typename PointLocation>
static void add_point_location(std::vector<Benchmark>& b, const std::string& name, Input& in, std::size_t n) {
  b.push_back({"pl_" + name, "locate " + std::to_string(n) + " query points with the " + name + " strategy",
    [&in, n](Stopwatch& sw) -> std::uint64_t {
      PointLocation pl(*in.arr);
      std::uint64_t h = 0;
      sw.start();
      for (std::size_t i = 0; i < n; ++i) h = mix(h, pl.locate(in.queries[i]).index());
      sw.stop();
      return h;
    }});
}

static std::vector<Benchmark> make_benchmarks(Input& in, const Options& opt) {
  std::vector<Benchmark> b;
  const Traits& tr = *in.traits;
  const bool cold = opt.cold && has_own_kernel<Traits>;

  // curves and arrangements
  b.push_back({"construct_curves", "construct the curves (analyses) from the polynomials",
    [&in, cold](Stopwatch& sw) -> std::uint64_t {
      auto trp = make_traits<Traits>(cold);
      sw.start();
      auto curves = construct_curves(*trp, in.polynomials);
      sw.stop();
      return curves.size();
    }});

  b.push_back({"make_x_monotone", "decompose the curves into x-monotone arcs and isolated points",
    [&in, cold](Stopwatch& sw) -> std::uint64_t {
      auto trp = make_traits<Traits>(cold);
      auto make_x_monotone = trp->make_x_monotone_2_object();
      std::uint64_t h = 0;
      sw.start();
      const auto curves = cold ? construct_curves(*trp, in.polynomials) : in.curves;
      for (const auto& cv : curves) {
        std::vector<Make_x_monotone_result> objs;
        make_x_monotone(cv, std::back_inserter(objs));
        h = mix(h, objs.size());
      }
      sw.stop();
      return h;
    }});

  b.push_back({"insert_aggregate", "construct the arrangement of the curves (sweep)",
    [&in, cold](Stopwatch& sw) -> std::uint64_t {
      auto trp = make_traits<Traits>(cold);
      sw.start();
      const auto curves = cold ? construct_curves(*trp, in.polynomials) : in.curves;
      Arr arr(trp.get());
      CGAL::insert(arr, curves.begin(), curves.end());
      sw.stop();
      return size_checksum(arr);
    }});

  b.push_back({"insert_incremental", "insert the curves one by one (zone)",
    [&in, cold](Stopwatch& sw) -> std::uint64_t {
      auto trp = make_traits<Traits>(cold);
      sw.start();
      const auto curves = cold ? construct_curves(*trp, in.polynomials) : in.curves;
      Arr arr(trp.get());
      for (const auto& cv : curves) CGAL::insert(arr, cv);
      sw.stop();
      return size_checksum(arr);
    }});

  b.push_back({"insert_xcurves_aggregate", "construct the arrangement of the x-monotone arcs (sweep)",
    [&in](Stopwatch& sw) -> std::uint64_t {
      sw.start();
      Arr arr(in.traits.get());
      CGAL::insert(arr, in.xcvs.begin(), in.xcvs.end());
      sw.stop();
      return size_checksum(arr);
    }});

  b.push_back({"overlay", "overlay the arrangements of the two halves of the curves",
    [&in, cold](Stopwatch& sw) -> std::uint64_t {
      auto trp = make_traits<Traits>(cold);
      const auto mid = in.polynomials.begin() + in.polynomials.size() / 2;
      const auto curves1 =
        cold ? construct_curves(*trp, std::vector<Polynomial>(in.polynomials.begin(), mid)) : in.curves1;
      const auto curves2 =
        cold ? construct_curves(*trp, std::vector<Polynomial>(mid, in.polynomials.end())) : in.curves2;
      Arr arr1(trp.get());
      Arr arr2(trp.get());
      CGAL::insert(arr1, curves1.begin(), curves1.end());
      CGAL::insert(arr2, curves2.begin(), curves2.end());
      Arr res(trp.get());
      CGAL::Arr_default_overlay_traits<Arr> overlay_traits;
      sw.start();
      CGAL::overlay(arr1, arr2, res, overlay_traits);
      sw.stop();
      return size_checksum(res);
    }});

  add_point_location<CGAL::Arr_naive_point_location<Arr>>(b, "naive", in, in.queries.size() / 10);
  add_point_location<CGAL::Arr_walk_along_line_point_location<Arr>>(b, "walk", in, in.queries.size());
  add_point_location<CGAL::Arr_trapezoid_ric_point_location<Arr>>(b, "trapezoid", in, in.queries.size());

  // the traits functors
  auto cmp = [](CGAL::Comparison_result r) { return static_cast<int>(r) + 1; };

  add_loop(b, "compare_x", "Compare_x_2 on pairs of arrangement vertices", in.point_pairs, opt.loops,
           [cmp, f = tr.compare_x_2_object()](const auto& pp) { return cmp(f(pp.first, pp.second)); });
  add_loop(b, "compare_xy", "Compare_xy_2 on pairs of arrangement vertices", in.point_pairs, opt.loops,
           [cmp, f = tr.compare_xy_2_object()](const auto& pp) { return cmp(f(pp.first, pp.second)); });
  add_loop(b, "compare_y_at_x", "Compare_y_at_x_2 on points in the x-ranges of arcs", in.point_arc_pairs, opt.loops,
           [cmp, f = tr.compare_y_at_x_2_object()](const auto& pa) { return cmp(f(pa.first, pa.second)); });
  add_loop(b, "compare_y_at_x_right", "Compare_y_at_x_right_2 on arcs to the right of a common vertex",
           in.right_pairs, opt.loops,
           [cmp, f = tr.compare_y_at_x_right_2_object()](const auto& t) { return cmp(f(t.xcv1, t.xcv2, t.p)); });
  add_loop(b, "compare_y_at_x_left", "Compare_y_at_x_left_2 on arcs to the left of a common vertex",
           in.left_pairs, opt.loops,
           [cmp, f = tr.compare_y_at_x_left_2_object()](const auto& t) { return cmp(f(t.xcv1, t.xcv2, t.p)); });
  add_loop(b, "compare_y_near_boundary", "Compare_y_near_boundary_2 on arc ends on the left/right boundary",
           in.left_right_pairs, opt.loops, [cmp, f = tr.compare_y_near_boundary_2_object()](const auto& t) {
             return cmp(f(t.first.xcv1, t.first.xcv2, t.second));
           });
  add_loop(b, "compare_x_on_boundary", "Compare_x_on_boundary_2 on arc ends on the bottom/top boundary",
           in.bottom_top_pairs, 100 * opt.loops, [cmp, f = tr.compare_x_on_boundary_2_object()](const auto& t) {
             return cmp(f(t.xcv1, t.ce1, t.xcv2, t.ce2));
           });
  add_loop(b, "parameter_space", "Parameter_space_in_x_2 and _in_y_2 on both ends of all arcs", in.xcvs,
           1000 * opt.loops,
           [ps_x = tr.parameter_space_in_x_2_object(), ps_y = tr.parameter_space_in_y_2_object()](const Xcv& xcv) {
             return ((static_cast<int>(ps_x(xcv, MIN_END)) * 5 + static_cast<int>(ps_y(xcv, MIN_END))) * 5 +
                     static_cast<int>(ps_x(xcv, MAX_END))) * 5 + static_cast<int>(ps_y(xcv, MAX_END)) + 1000;
           });
  add_loop(b, "min_max_vertex", "Construct_min/max_vertex_2 and Compare_xy_2 on arcs with interior points",
           in.arc_points, opt.loops,
           [cmp, min_v = tr.construct_min_vertex_2_object(), max_v = tr.construct_max_vertex_2_object(),
            ps_x = tr.parameter_space_in_x_2_object(), cmp_xy = tr.compare_xy_2_object()](const Arc_point& ap) {
             int h = 0;
             if (ps_x(ap.xcv, MIN_END) == CGAL::ARR_INTERIOR) h += cmp(cmp_xy(min_v(ap.xcv), ap.p));
             if (ps_x(ap.xcv, MAX_END) == CGAL::ARR_INTERIOR) h += 3 * cmp(cmp_xy(max_v(ap.xcv), ap.p));
             return h;
           });
  add_loop(b, "equal", "Equal_2 on pairs of arcs", in.arc_pairs, opt.loops,
           [f = tr.equal_2_object()](const Arc_pair& ap) { return f(ap.xcv1, ap.xcv2) ? 1 : 0; });
  add_loop(b, "is_on", "Is_on_2 on points and curves", in.point_curve_pairs, opt.loops,
           [f = tr.is_on_2_object()](const auto& pc) { return f(pc.first, pc.second) ? 1 : 0; });
  add_loop(b, "intersect", "Intersect_2 on pairs of arcs", in.arc_pairs, opt.loops,
           [f = tr.intersect_2_object()](const Arc_pair& ap) {
             std::vector<Intersection_result> res;
             f(ap.xcv1, ap.xcv2, std::back_inserter(res));
             std::uint64_t h = res.size();
             for (const auto& r : res) {
               if (const auto* ip = std::get_if<std::pair<Point, Multiplicity>>(&r)) h = mix(h, ip->second);
               else h = mix(h, 1000);
             }
             return h;
           });
  add_loop(b, "do_intersect", "Do_intersect_2 on pairs of arcs", in.arc_pairs, opt.loops,
           [f = tr.do_intersect_2_object()](const Arc_pair& ap) { return f(ap.xcv1, ap.xcv2) ? 1 : 0; });
  add_loop(b, "split", "Split_2 on arcs at interior points", in.arc_points, opt.loops,
           [f = tr.split_2_object(), is_vertical = tr.is_vertical_2_object()](const Arc_point& ap) {
             Xcv xcv1;
             Xcv xcv2;
             f(ap.xcv, ap.p, xcv1, xcv2);
             return (is_vertical(xcv1) ? 2 : 0) + (is_vertical(xcv2) ? 1 : 0);
           });
  add_loop(b, "merge", "Are_mergeable_2 and Merge_2 on the two parts of split arcs", in.halves, opt.loops,
           [are_mergeable = tr.are_mergeable_2_object(), merge = tr.merge_2_object()](const auto& h) {
             if (! are_mergeable(h.first, h.second)) return 0;
             Xcv xcv;
             merge(h.first, h.second, xcv);
             return 1;
           });
  add_loop(b, "construct_point_on_arc", "Construct_point_2 from an x-coordinate and an arc", in.arc_points, opt.loops,
           [ctr_pt = tr.construct_point_2_object(), cmp_xy = tr.compare_xy_2_object(),
            cmp](const Arc_point& ap) mutable { return cmp(cmp_xy(ctr_pt(Algebraic_real(ap.x), ap.xcv), ap.p)); });
  add_loop(b, "construct_x_monotone_segment",
           "Construct_x_monotone_segment_2 through interior points of arcs", in.arc_points, opt.loops,
           [f = tr.construct_x_monotone_segment_2_object()](const Arc_point& ap) {
             std::vector<Xcv> segs;
             f(ap.xcv.curve(), ap.p, Traits::POINT_IN_INTERIOR, std::back_inserter(segs));
             return segs.size();
           });

  return b;
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  Options opt;
  if (! parse(argc, argv, opt)) return EXIT_FAILURE;

  Input in;
  prepare(in, opt);
  std::vector<Benchmark> benches = make_benchmarks(in, opt);

  if (opt.list) {
    list_benchmarks(benches);
    return EXIT_SUCCESS;
  }

  const bool cold = opt.cold && has_own_kernel<Traits>;
  std::cout << "CGAL " << CGAL_VERSION_STR << "  traits with its own kernel: "
            << (has_own_kernel<Traits> ? "yes" : "no") << "  caches: " << (cold ? "cold" : "warm")
            << (opt.cold && ! cold ? " (--cold is not supported by this CGAL tree)" : "") << "\n"
            << "seed " << opt.seed << ", repetitions " << opt.repetitions << ", warm-up " << opt.warmup
            << ", curves " << opt.curves << " (" << opt.asymptotic << " asymptotic), degree " << opt.degree
            << ", coefficient " << opt.coefficient << ", pairs " << opt.pairs << ", loops " << opt.loops
            << ", queries " << opt.queries << ", range " << opt.range << "\n"
            << "input: " << in.xcvs.size() << " x-monotone arcs, " << in.isolated_points.size()
            << " isolated points; arrangement: " << in.arr->number_of_vertices() << " vertices, "
            << in.arr->number_of_edges() << " edges, " << in.arr->number_of_faces() << " faces\n"
            << "functor inputs: " << in.point_pairs.size() << " point pairs, " << in.point_arc_pairs.size()
            << " point/arc pairs, " << in.arc_pairs.size() << " arc pairs, " << in.arc_points.size()
            << " arcs with interior points, " << in.right_pairs.size() << "/" << in.left_pairs.size()
            << " right/left pairs, " << in.left_right_pairs.size() << "/" << in.bottom_top_pairs.size()
            << " left-right/bottom-top boundary pairs; " << in.dropped << " dropped\n\n";

  std::ostringstream comment;
  comment << "# cgal=" << CGAL_VERSION_STR << " own_kernel=" << (has_own_kernel<Traits> ? 1 : 0)
          << " cold=" << (cold ? 1 : 0) << " seed=" << opt.seed << " repetitions=" << opt.repetitions
          << " curves=" << opt.curves << " asymptotic=" << opt.asymptotic << " degree=" << opt.degree
          << " coefficient=" << opt.coefficient << " pairs=" << opt.pairs << " loops=" << opt.loops
          << " queries=" << opt.queries << " range=" << opt.range;
  return run_benchmarks(benches, opt.filter, opt.warmup, opt.repetitions, opt.csv, comment.str());
}

#endif // CGAL_USE_CORE
