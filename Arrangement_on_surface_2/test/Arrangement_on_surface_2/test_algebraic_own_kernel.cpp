// Copyright (c) 2026 Tel-Aviv University (Israel).
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Tests a traits of algebraic segments that holds a curved kernel with a curve kernel of its own: the caches are
// separate from those of the curve kernel of the thread (which default-constructed traits use), and the curved kernel
// keeps the curve kernel alive (the caches of a curve kernel refer back to it), even when no one else refers to it.

#include <CGAL/config.h>
#include <iostream>

#if ! defined(CGAL_USE_CORE)

int main() {
  std::cout << "This test requires CORE; it is skipped." << std::endl;
  return 0;
}

#else

#include <memory>
#include <vector>

#include <CGAL/CORE_BigInt.h>
#include <CGAL/Arr_algebraic_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>

using Traits = CGAL::Arr_algebraic_segment_traits_2<CORE::BigInt>;
using CKvA_2 = Traits::CKvA_2;
using Curve_kernel_2 = CKvA_2::Curve_kernel_2;
using Polynomial_2 = Traits::Polynomial_2;
using Arrangement_2 = CGAL::Arrangement_2<Traits>;

/*! constructs the arrangement of a few curves, and returns its number of edges.
 */
std::size_t construct(const Traits& traits) {
  Polynomial_2 x = CGAL::shift(Polynomial_2(1), 1, 0);
  Polynomial_2 y = CGAL::shift(Polynomial_2(1), 1, 1);
  std::vector<Traits::Curve_2> curves;
  for (const auto& p : {x * x + y * y - 4, y - x * x * x + x, x * y - 1})
    curves.push_back(traits.construct_curve_2_object()(p));
  Arrangement_2 arr(&traits);
  CGAL::insert(arr, curves.begin(), curves.end());
  return arr.is_valid() ? arr.number_of_edges() : 0;
}

int main() {
  bool ok = true;
  auto& static_cache = Curve_kernel_2::get_static_instance().curve_cache_2();
  const auto static_size = static_cache.size();

  // a curved kernel with a curve kernel of its own, to which no one else refers
  auto ckva = std::make_shared<CKvA_2>(std::make_shared<Curve_kernel_2>());
  Traits traits(ckva);
  const std::size_t edges = construct(traits);
  if (&ckva->kernel().curve_cache_2() == &static_cache) {
    std::cerr << "Error: the curve kernel shares the caches of the curve kernel of the thread" << std::endl;
    ok = false;
  }
  if (static_cache.size() != static_size) {
    std::cerr << "Error: the curve cache of the thread is modified" << std::endl;
    ok = false;
  }
  if (ckva->kernel().curve_cache_2().size() == 0) {
    std::cerr << "Error: the curve cache of the curve kernel is not used" << std::endl;
    ok = false;
  }

  // a copy of the curved kernel shares the caches
  CKvA_2 copy(*ckva);
  if (&copy.kernel().curve_cache_2() != &ckva->kernel().curve_cache_2()) {
    std::cerr << "Error: a copy of the curved kernel does not share the caches" << std::endl;
    ok = false;
  }

  // the default-constructed traits uses the curve kernel of the thread, with the same result
  Traits default_traits;
  if (&default_traits.curved_kernel()->kernel().curve_cache_2() != &static_cache) {
    std::cerr << "Error: a default-constructed traits does not use the curve kernel of the thread" << std::endl;
    ok = false;
  }
  const std::size_t default_edges = construct(default_traits);
  if (edges == 0 || edges != default_edges) {
    std::cerr << "Error: " << edges << " edges, but " << default_edges << " with the default traits" << std::endl;
    ok = false;
  }

  std::cout << (ok ? "passed" : "failed") << std::endl;
  return ok ? 0 : 1;
}

#endif
