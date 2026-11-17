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

#ifndef CGAL_CKVA_CURVE_RENDERER_TRAITS_H
#define CGAL_CKVA_CURVE_RENDERER_TRAITS_H

#include <CGAL/license/Arrangement_on_surface_2.h>

#include <cstddef>
#include <functional>

#include <CGAL/Coercion_traits.h>
#include <CGAL/Interval_nt.h>
#include <CGAL/number_utils.h>
#include <CGAL/Polynomial.h>
#include <CGAL/Sqrt_extension.h>

#ifdef CGAL_USE_CORE
#include <CGAL/CORE_BigFloat.h>
#include <CGAL/CORE_BigInt.h>
#include <CGAL/CORE_BigRat.h>
#endif

#ifdef CGAL_USE_LEDA
#include <CGAL/leda_bigfloat.h>
#include <CGAL/leda_integer.h>
#include <CGAL/leda_rational.h>
#endif

/*! \file CGAL/Curved_kernel_via_analysis_2/gfx/Curve_renderer_traits.h
 * defines class Curve_renderer_traits.
 *
 * provides specializations of Curve_renderer_traits for different number
 * types compatible with the curve renderer
*/

namespace CGAL {

// transformation routine
namespace internal {

//! this exception is thrown whenever the precision of used number
//! type is not sufficient
class Insufficient_rasterize_precision_exception {};

#ifndef CGAL_MAX_COEFF_TRANSFORM
#define CGAL_MAX_COEFF_TRANSFORM

#ifndef CGAL_ABS
#define CGAL_ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef CGAL_SGN
#define CGAL_SGN(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#endif

template <typename NT>
struct Max_coeff {
  template <typename X>
  NT operator()(const CGAL::Polynomial<X>& p) const {
    typename CGAL::Polynomial<X>::const_iterator it = p.begin();
    Max_coeff<NT> max_coeff;
    NT cur_max(max_coeff(*it));
    while (++it != p.end()) {
      NT tmp(max_coeff(*it));
      if (cur_max < CGAL_ABS(tmp)) cur_max = CGAL_ABS(tmp);
    }
    return cur_max;
  }
  NT operator()(const NT& x) const { return CGAL_ABS(x); }
};

/*! divides an input value by a constant
 *
 * provided that there is a coercion between \c Input and \c Result types
 */
template <typename Result, typename Input>
struct Reduce_by {
  using argument_type = Input;
  using result_type = Result;

  Reduce_by(const Input& denom_) : m_denom(m_cast(denom_)) {}

  Result operator()(const Input& x) { return (m_cast(x) / m_denom); }

  typename CGAL::Coercion_traits<Input, Result>::Cast m_cast;
  Result m_denom;
};

/*! transforms bivaritate polynomial of type \c InputPoly_2 to
 * \c OutputPoly_2 by recursively applying operation \c Op to all of its
 * coefficients
 *
 * <tt>Op: InputPoly_2::Inntermost_coefficient_type ->
 *             OutputPoly_2::Inntermost_coefficient_type</tt>
 */
template <typename OutputPoly_2, typename InputPoly_2, typename Op>
struct Transform {
  using first_argument_type = InputPoly_2;
  using second_argument_type = Op;
  using result_type = OutputPoly_2;

  template <typename X>
  OutputPoly_2 operator()(const CGAL::Polynomial<X>& p, Op op = Op()) const {
    using NT_in = typename InputPoly_2::NT;
    using NT_out = typename OutputPoly_2::NT;
    Transform<NT_out, NT_in, Op> tr;
    auto fn = [&op, &tr](const NT_in& v) { return tr(v, op); };
    return OutputPoly_2(::boost::make_transform_iterator(p.begin(), fn), ::boost::make_transform_iterator(p.end(), fn));
  }

  OutputPoly_2 operator()(const typename Innermost_coefficient_type<InputPoly_2>::Type& x, Op op) const {
    return static_cast<OutputPoly_2>(op(x));
  }
};

#endif // CGAL_MAX_COEFF_TRANSFORM

/*! class template \c Curve_renderer_traits
 *
 * this traits class provides various number type conversions for the
 * curve renderer
 */

template <typename Coeff_, typename Integer_, typename Rational_>
struct Curve_renderer_traits_base {
  //! type of innermost polynomial coefficients
  using Coeff = Coeff_;

  //! an integer number type
  using Integer = Integer_;

  using Rational = Rational_;

  //! conversion from rational to floating-point
  template <typename Float>
  struct Rat_to_float {
    using result_type = Float;

    template <typename X, typename Y, typename ACDE_TAG, typename FP_TAG>
    Float operator()(const Sqrt_extension<X, Y, ACDE_TAG, FP_TAG>& x) const {
      typename CGAL::Coercion_traits<Sqrt_extension<X, Y, ACDE_TAG, FP_TAG>, Float>::Cast cast;
      return cast(x);
    }

    Float operator()(const Rational& x) const { return static_cast<Float>(x); }
  };

  //! provided for convenience when there exists an implicit coercion
  //! between number types
  template <typename To>
  struct Implicit_coercion {
    using result_type = To;

    template <typename From>
    To operator()(const From& x) const {
      return static_cast<To>(x);
    }
  };

  struct Float_to_int {
    using result_type = int;

    template <typename Float>
    int operator()(const Float& x) const {
      return static_cast<int>(std::floor(CGAL::to_double(x)));
    }
    //return static_cast<int>(std::floor(x)); }
  };

  /*! conversion from bivariate polynomial over extended number type to
   * polynomial with coefficients of type \c Coeff
   *
   * valid instantiations of \c Extended number type are \c Rational ,
   * \c FieldWithSqrt , etc. Provided that there is a type coercion between
   * \c Extended and \c Coeff
   */
  struct Convert_poly {
    using result_type = CGAL::Polynomial<CGAL::Polynomial<Coeff>>;

    template <typename Extended>
    inline result_type operator()(const CGAL::Polynomial<CGAL::Polynomial<Extended>>& poly) const {
      //!@todo: use Rat_to_float functor instead of coercion traits ?
      //! need some sort of polymorphism..

      //std::cerr << "calling transform..\n";
      using Cast = typename CGAL::Coercion_traits<Extended, Coeff>::Cast;
      Transform<result_type, CGAL::Polynomial<CGAL::Polynomial<Extended>>, Cast> transform;
      return transform(poly);
    }
  };

  //! converts polynomial coefficients to floating-point representation
  //! \c error_bounds is set if the result is not reliable
  struct Extract_eval {
    using argument_type = Coeff;
    using result_type = Coeff;

    Coeff operator()(const Coeff& x, bool* error_bounds = nullptr) const {
      if (error_bounds != nullptr) *error_bounds = false;
      return x;
    }
  };

  //! computes a 32-bit hash value from floating-point argument
  struct Hash_function {
    using result_type = std::size_t;
    struct long_long {
      long m_low, m_high;
    };

    template <typename Float>
    std::size_t operator()(const Float& key) const {
      const long_long* hk = reinterpret_cast<const long_long*>(&key);
      return (hk->m_low ^ hk->m_high);
    }
  };

  //! makes result exact after inexact operation such as div, sqrt, etc.
  //! (required for multi-precision arithmetic)
  struct Make_exact {
    using result_type = void;

    template <typename Float>
    void operator()(const Float& /*x*/) const {}
  };

  //! compares a given quantity with the precision limit a floating-point
  //! number type, returns \c true if this limit is exceeded
  struct Precision_limit {
    using result_type = bool;

    template <typename Float>
    bool operator()(const Float& /*x*/) const {
      return false; /*(CGAL_ABS(x) <= 1e-16)*/
    }
  };

  //! maximum subdivision level for the curve renderer by exceeding which
  //! an exception of type \c Insufficient_rasterize_precision_exception
  //! will be thrown, this is also limited by \c Integer number type, since
  //! the integer must be able to store numbers up to 2^MAX_SUBDIVISION_LEVEL
  static const unsigned s_max_subdivision_level = 12;
};

template <typename Float, typename Rational>
struct Curve_renderer_traits {};

#ifdef CGAL_USE_CORE
//! specialization for \c CGAL::Interval_nt<true>
template <>
struct Curve_renderer_traits<CGAL::Interval_nt<true>, CORE::BigRat>
  : public Curve_renderer_traits_base<CGAL::Interval_nt<true>, int, CORE::BigRat> {
  using Float = double;

  struct Rat_to_float {
    using result_type = Float;

    template <typename Extended>
    Float operator()(const Extended& x) const {
      return CGAL::to_double(x);
    }
  };

  using Float_to_rat = Implicit_coercion<double>;

  struct Rat_to_integer {
    using argument_type = Rational;
    using result_type = Integer;

    Integer operator()(const Rational& x) const { return static_cast<int>(std::floor(CGAL::to_double(x))); }
  };

  struct Extract_eval {
    using argument_type = Coeff;
    using result_type = Float;

    Float operator()(const Coeff& x, bool* error_bounds = nullptr) const {
      bool err_bnd;
      //             err_bnd = (CGAL_ABS(l) < 1E-15 || CGAL_ABS(u) < 1E-15) ||
      //                 ((l <= 0 && u >= 0));
      Float l = x.inf();
      Float u = x.sup();
      Float mid = (l + u) / 2;
      err_bnd = ((l < 0 && u > 0) || (l == 0 && u == 0));
      if (error_bounds != nullptr) *error_bounds = err_bnd;
      //! ATTENTION!!! if smth is screwed up try to uncomment the line below
      //! this is very crucial for performance & stability
      if (err_bnd) // &&  ABS(mid) < 1E-15)
        return 0;
      //! ATTENTION!!!
      return mid;
    }
  };

  //! compares a given quantity with the precision limit a floating-point
  //! number type, returns \c true if this limit is exceeded
  struct Precision_limit {
    using result_type = bool;

    template <typename Float>
    bool operator()(const Float& x) const {
      return (CGAL_ABS(x) <= 1e-16);
    }
  };

  static const unsigned s_max_subdivision_level = 12;
};

//! specialization for \c CORE::BigFloat
template <>
struct Curve_renderer_traits<CORE::BigFloat, CORE::BigRat>
  : public Curve_renderer_traits_base<CORE::BigFloat, CORE::BigInt, CORE::BigRat> {
  using Float = CORE::BigFloat;

  struct Rat_to_integer {
    using argument_type = Rational;
    using result_type = Integer;

    Integer operator()(const Rational& x) const { return numerator(x) / denominator(x); }
  };

  using Rat_to_float = Rat_to_float<Float>;

  struct Float_to_rat {
    using argument_type = Float;
    using result_type = Rational;

    Rational operator()(const Float& x) const { return x.BigRatValue(); }
  };

  struct Hash_function {
    using argument_type = Float;
    using result_type = std::size_t;

    inline result_type operator()(const Float& key) const {
      const CORE::BigFloatRep& rep = key.getRep();
      std::size_t ret = reinterpret_cast<std::size_t>(&rep);
      return ret;
    }
  };

  struct Make_exact {
    using argument_type = Float;
    using result_type = void;

    inline void operator()(Float& x) const { x.makeExact(); }
  };

  static const unsigned s_max_subdivision_level = 12;
};

//! specialization for \c CORE::BigRat
template <>
struct Curve_renderer_traits<CORE::BigRat, CORE::BigRat>
  : public Curve_renderer_traits_base<CORE::BigRat, CORE::BigInt, CORE::BigRat> {
  using Float = CORE::BigRat;

  using Rat_to_float = Rat_to_float<Float>;

  using Float_to_rat = Implicit_coercion<Float>;

  struct Rat_to_integer {
    using argument_type = Rational;
    using result_type = Integer;

    Integer operator()(const Rational& x) const { return numerator(x) / denominator(x); }
  };

  struct Hash_function {
    using argument_type = Float;
    using result_type = std::size_t;

    inline result_type operator()(const Float& key) const { return std::hash<Float>()(key); }
  };
};
#endif // CGAL_USE_CORE

#ifdef CGAL_USE_LEDA
template <>
struct Curve_renderer_traits<CGAL::Interval_nt<true>, leda::rational>
  : public Curve_renderer_traits_base<CGAL::Interval_nt<true>, int, leda::rational> {
  using Float = double;

  struct Rat_to_float {
    using result_type = Float;

    template <typename Extended>
    Float operator()(const Extended& x) const {
      return CGAL::to_double(x);
    }
  };

  using Float_to_rat = Implicit_coercion<double>;

  struct Rat_to_integer {
    using argument_type = Rational;
    using result_type = Integer;

    Integer operator()(const Rational& x) const {
      return static_cast<int>(std::floor(CGAL::to_double(x)));
      //return static_cast<int>(x.to_double());
    }
  };

  struct Extract_eval {
    using argument_type = Coeff;
    using result_type = Float;

    Float operator()(const Coeff& x, bool* error_bounds = nullptr) const {
      bool err_bnd;
      //             err_bnd = (CGAL_ABS(l) < 1E-15 || CGAL_ABS(u) < 1E-15) ||
      //                 ((l <= 0 && u >= 0));
      Float l = x.inf();
      Float u = x.sup();
      Float mid = (l + u) / 2;
      err_bnd = ((l < 0 && u > 0) || (l == 0 && u == 0));
      if (error_bounds != nullptr) *error_bounds = err_bnd;
      //! ATTENTION!!! if smth is screwed up try to uncomment the line below
      //! this is very crucial for performance & stability
      if (err_bnd) // &&  ABS(mid) < 1E-15)
        return 0;
      //! ATTENTION!!!
      return mid;
    }
  };

  //! compares a given quantity with the precision limit a floating-point
  //! number type, returns \c true if this limit is exceeded
  struct Precision_limit {
    using result_type = bool;

    template <typename Float>
    bool operator()(const Float& x) const {
      return (CGAL_ABS(x) <= 1e-16);
    }
  };
};

//! specialization for \c leda::bigfloat
template <>
struct Curve_renderer_traits<leda::bigfloat, class leda::rational>
  : public Curve_renderer_traits_base<leda::bigfloat, leda::integer, leda::rational> {
  using Float = leda::bigfloat;

  struct Rat_to_integer {
    using argument_type = Rational;
    using result_type = Integer;

    Integer operator()(const Rational& x) const { return leda::floor(x); }
  };

  struct Rat_to_float {
    using result_type = Float;

    template <typename X, typename Y, typename ACDE_TAG, typename FP_TAG>
    Float operator()(const Sqrt_extension<X, Y, ACDE_TAG, FP_TAG>& x) const {
      typename CGAL::Coercion_traits<Sqrt_extension<X, Y, ACDE_TAG, FP_TAG>, Float>::Cast cast;
      return cast(x);
    }
    // no implicit coercion between leda rational and floats, therefore
    // decompose rational to compute the result
    Float operator()(const Rational& x) const {
      return (static_cast<Float>(x.numerator()) / static_cast<Float>(x.denominator()));
    }
  };

  struct Float_to_rat {
    using argument_type = Float;
    using result_type = Rational;

    Rational operator()(const Float& x) const { return x.to_rational(); }
  };

  struct Convert_poly {
    using result_type = CGAL::Polynomial<CGAL::Polynomial<Coeff>>;

    template <typename Extended>
    inline result_type operator()(const CGAL::Polynomial<CGAL::Polynomial<Extended>>& poly) const {
      Transform<result_type, CGAL::Polynomial<CGAL::Polynomial<Extended>>, Rat_to_float> transform;
      return transform(poly);
    }
  };

  struct Hash_function {
    using argument_type = Float;
    using result_type = std::size_t;

    inline result_type operator()(const Float& key) const {
      return static_cast<std::size_t>(key.get_significant_length());
    }
  };
};

//! specialization for \c leda::rational
template <>
struct Curve_renderer_traits<leda::rational, leda::rational>
  : public Curve_renderer_traits_base<leda::rational, leda::integer, leda::rational> {
  using Float = leda::rational;

  using Rat_to_float = Rat_to_float<Float>;

  using Float_to_rat = Implicit_coercion<Float>;

  struct Rat_to_integer {
    using argument_type = Rational;
    using result_type = Integer;

    Integer operator()(const Rational& x) const { return leda::floor(x); }
  };

  struct Hash_function {
    using argument_type = Float;
    using result_type = std::size_t;

    inline result_type operator()(const Float& key) const {
      std::size_t ret = reinterpret_cast<std::size_t>(key.numerator().word_vector());
      return ret;
    }
  };

  struct Make_exact {
    using argument_type = Float;
    using result_type = void;

    inline void operator()(Float& x) const { x.normalize(); }
  };
};

#endif // CGAL_USE_LEDA

} // namespace internal
} // namespace CGAL

#endif
