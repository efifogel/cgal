// Copyright (c) 2026 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s) : Efi Fogel <efifogel@gmail.com>

#ifndef CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_ARC_SIMPLIFICATION_CACHE_H
#define CGAL_CURVED_KERNEL_VIA_ANALYSIS_2_ARC_SIMPLIFICATION_CACHE_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*!\file include/CGAL/Curved_kernel_via_analysis_2/Arc_simplification_cache.h
 * defines \c Arc_simplification_cache
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <utility>

namespace CGAL {
namespace internal {

/*! caches arcs with simplified supporting curves. When the supporting curves of two arcs share a component, the
 * operations on the arcs replace each supporting curve by the component that the arc lies on (see `simplified()` of
 * the functors). The simplified arcs are copies; this cache keeps them, keyed by the arc and the supporting curve of
 * the other arc (an arc is simplified alike against all arcs of that curve), so that each is computed once, while the
 * arcs themselves are not modified.
 *
 * Supporting curves rarely share components; hence, the cache is allocated only when needed (by the curved kernel).
 * It is consulted before the supporting curves are decomposed, for all pairs of arcs with distinct supporting curves;
 * hence, a bit filter on the keys answers most queries of keys that are not cached without searching.
 *
 * An entry keeps its arc alive; an entry with an arc that only the cache refers to can no longer be queried, and it is
 * erased when the number of entries doubles (amortized constant time per insertion). Entries of arcs that others refer
 * to are not erased; hence, the references that the cache returns remain valid as long as those arcs do.
 */
template <typename CurvedKernelViaAnalysis_2>
class Arc_simplification_cache {
public:
  //! the type of the curved kernel
  using Curved_kernel_via_analysis_2 = CurvedKernelViaAnalysis_2;

  //! the type of an arc
  using Arc_2 = typename Curved_kernel_via_analysis_2::Arc_2;

  //! the type of a supporting curve
  using Curve_analysis_2 = typename Curved_kernel_via_analysis_2::Curve_2;

  /*! the simplification of an arc. An empty simplification means that the supporting curve of the arc is the common
   * component, and thus is not simplified.
   */
  using Simplified = std::optional<Arc_2>;

  /*! obtains the simplification of the arc \c cv against the curve \c other, or \c nullptr if it is not cached.
   */
  const Simplified* find(const Arc_2& cv, const Curve_analysis_2& other) const {
    const Key key(cv.id(), other.id());
    const std::size_t bit = filter_bit(key);
    if (! (m_filter[bit / 64] & (std::uint64_t(1) << (bit % 64)))) return nullptr;
    auto it = m_map.find(key);
    return (it == m_map.end()) ? nullptr : &(it->second.simplified);
  }

  /*! caches the simplification of the arc \c cv against the curve \c other.
   *
   * \return the cached simplification; the reference remains valid as long as \c cv does
   */
  const Simplified& insert(const Arc_2& cv, const Curve_analysis_2& other, Simplified simplified) {
    if (m_map.size() >= m_collect_at) collect();
    const Key key(cv.id(), other.id());
    set_filter_bit(key);
    auto res = m_map.emplace(key, Entry{cv, other, std::move(simplified)});
    return res.first->second.simplified;
  }

private:
  //! the key: the ids of the arc and of the other curve (the addresses of their representations); the entry keeps both
  //! alive, so the ids are not reused
  using Key = std::pair<decltype(std::declval<const Arc_2&>().id()),
                        decltype(std::declval<const Curve_analysis_2&>().id())>;

  //! an entry of the cache
  struct Entry {
    Arc_2 arc;
    Curve_analysis_2 other;
    Simplified simplified;
  };

  //! the number of bits of the filter (a power of 2)
  static constexpr std::size_t s_filter_bits = std::size_t(1) << 16;

  //! obtains the bit of the filter of a key (multiplicative hashing; no division)
  static std::size_t filter_bit(const Key& key) {
    const auto a = static_cast<std::uint64_t>(key.first);
    const auto b = static_cast<std::uint64_t>(key.second);
    const std::uint64_t h = (a * 0x9e3779b97f4a7c15ULL) ^ (b * 0xc2b2ae3d27d4eb4fULL);
    return static_cast<std::size_t>(h >> 48);
  }

  //! sets the bit of a key in the filter
  void set_filter_bit(const Key& key) {
    const std::size_t bit = filter_bit(key);
    m_filter[bit / 64] |= std::uint64_t(1) << (bit % 64);
  }

  //! erases the entries that can no longer be queried, as their arcs are referred to only by the cache
  void collect() {
    m_filter.fill(0);
    for (auto it = m_map.begin(); it != m_map.end();) {
      if (it->second.arc.is_shared()) {
        set_filter_bit(it->first);
        ++it;
      }
      else it = m_map.erase(it);
    }
    m_collect_at = std::max(s_min_collect_at, 2 * m_map.size());
  }

  //! the least number of entries that triggers erasing entries that can no longer be queried
  static constexpr std::size_t s_min_collect_at = 64;

  //! the number of entries that triggers erasing entries that can no longer be queried
  std::size_t m_collect_at = s_min_collect_at;

  //! the bit filter: the bit of every cached key is set
  std::array<std::uint64_t, s_filter_bits / 64> m_filter{};

  //! the cache
  std::map<Key, Entry> m_map;
};

} // namespace internal
} // namespace CGAL

#endif
