// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s) : Efi Fogel       <efif@post.tau.ac.il>

#ifndef CGAL_VARIANT_OUTPUT_ITERATOR_H
#define CGAL_VARIANT_OUTPUT_ITERATOR_H

#include <CGAL/license/Surface_sweep_2.h>

/* A compatible output iterator that accepts a `std::variant` and dispatches its
 * alternatives to different sinks via a visitor.
 *
 * Designed to replace `boost::function_output_iterator` when the algorithm emits
 * heterogeneous output (e.g., `Point_2` or `X_monotone_curve_2`).
 *
 * Several \cgal concepts (e.g., `AosTraits::MakeXMonotone_2`) require an operation that writes
 * results to an `OutputIterator` whose value type is:
 *
 * `std::variant<T1, T2, ...>`
 *
 * However:
 * 1. `boost::function_output_iterator` receives one type
 * 2. Visitors (operator()) are invoked after assignment
 * 3. \cgal freely copies, assigns, and default-constructs output iterators
 * `variant_output_iterator` bridges this gap.
 *
 * The callables passed to `make_variant_output_iterator` must appear in the
 * same order as the alternatives of `Variant`.
 */

#include <variant>
#include <iterator>
#include <utility>

#include <CGAL/assertions.h>

namespace CGAL {

/* A compatible output iterator that accepts a `std::variant` and dispatches
 * its alternatives to different sinks via per-index callables.
 *
 * The callables passed to `make_variant_output_iterator` must appear in the
 * same order as the alternatives of `Variant`.
 */

template <typename Variant, typename F0, typename F1>
class variant_output_iterator {
public:
  using value_type        = void;
  using difference_type   = std::ptrdiff_t;
  using iterator_category = std::output_iterator_tag;
  using pointer           = void;
  using reference         = void;

  variant_output_iterator() = default;

  variant_output_iterator(F0 f0, F1 f1) : m_f0(std::move(f0)), m_f1(std::move(f1)) {}

  variant_output_iterator(const variant_output_iterator&) = default;
  variant_output_iterator& operator=(const variant_output_iterator&) = default;

  variant_output_iterator& operator*() { return *this; }
  variant_output_iterator& operator++() { return *this; }
  variant_output_iterator& operator++(int) { return *this; }

  variant_output_iterator& operator=(const Variant& v) {
    if (v.index() == 0) [[likely]] m_f0(std::get<0>(v));
    else if (v.index() == 1) m_f1(std::get<1>(v));
    else CGAL_error();
    return *this;
  }

  variant_output_iterator& operator=(Variant&& v) {
    if (v.index() == 0) [[likely]] m_f0(std::get<0>(std::move(v)));
    else if (v.index() == 1) m_f1(std::get<1>(std::move(v)));
    else CGAL_error();
    return *this;
  }

private:
  F0 m_f0;
  F1 m_f1;
};

template <typename Variant, typename F0, typename F1>
auto make_variant_output_iterator(F0&& f0, F1&& f1) {
  return variant_output_iterator<Variant, std::decay_t<F0>, std::decay_t<F1>>(std::forward<F0>(f0),
                                                                              std::forward<F1>(f1));
}

} // namespace CGAL

#endif
