// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_2_MINKOWSKI_SUM_BY_FULL_CONVOLUTION_2_H
#define CGAL_MINKOWSKI_SUM_2_MINKOWSKI_SUM_BY_FULL_CONVOLUTION_2_H

#include <CGAL/license/Minkowski_sum_2.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/Named_function_parameters.h>
#include <CGAL/Gps_segment_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_curve_data_traits_2.h>
#include <CGAL/Arr_data_traits_2.h>
#include <CGAL/Arr_label_policy_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Iterator_range.h>
#include <CGAL/Unique_hash_map.h>
#include <CGAL/unordered_flat_set.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Minkowski_sum_2/Convolution_curves.h>
#include <CGAL/Minkowski_sum_2/Hole_filter_2.h>
#include <CGAL/Minkowski_sum_2/internal/minkowski_sum_of_convex_polygons_2.h>
#include <CGAL/Minkowski_sum_2/internal/named_parameters_2.h>
#include <CGAL/Arrangement_on_curve_1/Direction_geom_traits_2_adaptor_1.h>
#include <CGAL/Arrangement_on_curve_1/Identified_topology_traits.h>
#include <CGAL/Arrangement_on_curve_1/Arrangement_on_curve_1.h>
#include <CGAL/Arrangement_on_curve_1/insert.h>
#include <CGAL/Arrangement_on_curve_1/overlay.h>

namespace CGAL {
namespace Minkowski_sum_2 {
namespace internal {

// ============================================================================
// FACE DATA FOR WINDING NUMBERS
// ============================================================================

struct Face_data {
  int m_winding_number;
  bool m_visited;
  // The component of the extracted region that contains the face (see
  // `extract_region()`), or -1.
  mutable int m_component;

  Face_data() : m_winding_number(-1), m_visited(false), m_component(-1) {}
};

// ============================================================================
// CONVOLUTION EDGE DATA STRUCTURES
// ============================================================================

// Result of cone overlap check: which directions can advance from a pair
enum class Cone_overlap_result {
  None,         // cones don't overlap
  First,        // only advancing k1 is valid
  Second,       // only advancing k2 is valid
  Both,         // Both valid in one sub-cone (identical outgoing directions)
  Either        // Either valid in two disconnected sub-cones
};

struct Convolution_edge {
  std::size_t m_k1;
  std::size_t m_k2;
  Cone_overlap_result m_result;

  Convolution_edge(std::size_t k1, std::size_t k2, Cone_overlap_result result) :
    m_k1(k1), m_k2(k2), m_result(result)
  {}

  bool operator==(const Convolution_edge& other) const
  { return m_k1 == other.m_k1 && m_k2 == other.m_k2 && m_result == other.m_result; }

  bool operator<(const Convolution_edge& other) const {
    if (m_k1 != other.m_k1) return m_k1 < other.m_k1;
    if (m_k2 != other.m_k2) return m_k2 < other.m_k2;
    return static_cast<int>(m_result) < static_cast<int>(other.m_result);
  }

  // Stream insertion operator
  friend std::ostream& operator<<(std::ostream& os, const Convolution_edge& edge) {
    os << "{" << edge.m_k1 << ", " << edge.m_k2 << ", ";
    switch(edge.m_result) {
     case Cone_overlap_result::First: os << "First"; break;
     case Cone_overlap_result::Second: os << "Second"; break;
     case Cone_overlap_result::Both: os << "Both"; break;
     case Cone_overlap_result::Either: os << "Either"; break;
     default: os << "None"; break;
    }
    os << "}";
    return os;
  }
};

/*!
 */
struct Convolution_edge_hash {
  // The hash values are well mixed, so hash tables need not mix them further.
  using is_avalanching = std::true_type;

  std::size_t operator()(const Convolution_edge& e) const {
    // Pack the fields into one integer (injectively, as long as the indices
    // are smaller than 2^29), and mix its bits as in splitmix64.  Combining the
    // hash values of the fields with shifts and xors maps many edges to few
    // hash values.
    std::uint64_t x = (std::uint64_t(e.m_k1) << 32) ^ (std::uint64_t(e.m_k2) << 3) ^
      std::uint64_t(static_cast<int>(e.m_result));
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return static_cast<std::size_t>(x ^ (x >> 31));
  }
};

//! a set of convolution edges.
using Convolution_edge_set = CGAL::unordered_flat_set<Convolution_edge, Convolution_edge_hash>;

// ============================================================================
// X-MONOTONE DECOMPOSITION
// ============================================================================

template <typename CurveInputIterator, typename XCurveOutputIterator, typename PointOutputIterator, typename Traits>
void make_x_monotone(CurveInputIterator begin, CurveInputIterator end, XCurveOutputIterator it_xcv,
                     PointOutputIterator it_pt, const Traits& traits) {
  using Point_2 = typename Traits::Point_2;
  using X_monotone_curve_2 = typename Traits::X_monotone_curve_2;
  using Variant = std::variant<Point_2, X_monotone_curve_2>;
  auto out = CGAL::make_variant_output_iterator<Variant>(
    std::function([it_pt](const Point_2& pt) mutable { *it_pt++ = pt; }),
    std::function([it_xcv](const X_monotone_curve_2& xcv) mutable { *it_xcv++ = xcv; }));
  auto mk_x_monotone = traits.make_x_monotone_2_object();
  for (auto it = begin; it != end; ++it) mk_x_monotone(*it, out);
}

/*! extracts the turns of a polygon: the k-th turn is the orientation of the
 * k-th vertex and its neighbors, that is, the turn from the direction of edge
 * k-1 (the incoming edge) to the direction of edge k (the outgoing edge).  It
 * is computed from the vertices, which is cheaper than computing it from the
 * directions.
 */
template <typename Kernel_, typename Container_, typename Traits_>
std::vector<Orientation> extract_turns(const Polygon_2<Kernel_, Container_>& pgn, const Traits_& traits) {
  const auto orientation = traits.orientation_2_object();
  std::vector<Orientation> turns;
  if (pgn.is_empty()) return turns;
  auto prev = pgn.vertices_circulator();
  auto curr = prev;
  auto next = curr;
  ++next;
  --prev;
  do {
    turns.push_back(orientation(*prev, *curr, *next));
    prev = curr;
    curr = next;
    ++next;
  } while (curr != pgn.vertices_circulator());
  return turns;
}

// ============================================================================
// CONVOLUTION EDGES VIA ARRANGEMENTS ON THE CIRCLE OF DIRECTIONS
// ============================================================================

/* The cone of vertex k of a polygon is the open counterclockwise arc of
 * directions from dirs[k-1] (the direction of the incoming edge) to dirs[k]
 * (the direction of the outgoing edge); if the two are equal, the cone is the
 * entire circle except for this direction.  The pair (k1, k2) induces the
 * convolution edges
 * - Both,   if dirs1[k1] = dirs2[k2],
 * - First,  if dirs1[k1] lies in the cone of vertex k2 of the second polygon, and
 * - Second, if dirs2[k2] lies in the cone of vertex k1 of the first polygon.
 */

/*! a set of cones, identified by the indices of their polygon vertices, that
 * supports insertion, removal, and traversal in time proportional to the
 * number of cones traversed.
 */
class Cone_set {
public:
  explicit Cone_set(std::size_t n) : m_next(n + 1, n), m_prev(n + 1, n), m_in(n, false), m_head(n), m_size(0) {}

  std::size_t size() const { return m_size; }

  void insert(std::size_t k) {
    CGAL_precondition(! m_in[k]);
    m_in[k] = true;
    ++m_size;
    m_next[k] = m_next[m_head];
    m_prev[k] = m_head;
    m_prev[m_next[m_head]] = k;
    m_next[m_head] = k;
  }

  void erase(std::size_t k) {
    CGAL_precondition(m_in[k]);
    m_in[k] = false;
    --m_size;
    m_next[m_prev[k]] = m_next[k];
    m_prev[m_next[k]] = m_prev[k];
  }

  template <typename Function>
  void for_each(Function f) const { for (auto k = m_next[m_head]; k != m_head; k = m_next[k]) f(k); }

private:
  std::vector<std::size_t> m_next;
  std::vector<std::size_t> m_prev;
  std::vector<bool> m_in;
  std::size_t m_head;           // the sentinel
  std::size_t m_size;
};

//! the index of no edge.
constexpr std::size_t no_edge = std::numeric_limits<std::size_t>::max();

/*! the data of a vertex of an arrangement of the directions of the edges of a
 * polygon: the edges that have its direction, which form a singly linked list
 * whose links are stored in an array indexed by the edges (see
 * `Direction_arrangement_builder`).
 */
struct Edge_chain {
  std::size_t m_first = no_edge;        // the first edge, or `no_edge`
  std::size_t m_size = 0;               // the number of edges
};

/*! builds an arrangement on the circle of the directions of the edges of a
 * polygon, and stores with every vertex the chain of the edges that have its
 * direction.  The geometry traits of the arrangement provide `Equal_1`, in
 * addition to the functors required by `AocIdentifiedBoundaryTraits_1`.
 */
template <typename Arrangement>
class Direction_arrangement_builder {
public:
  using Direction_2 = typename Arrangement::Point_1;
  using Vertex_descriptor = typename Arrangement::Vertex_descriptor;
  using Edge_descriptor = typename Arrangement::Edge_descriptor;

  /*! constructs.
   * \param next the links of the chains of edges, indexed by the edges.
   */
  Direction_arrangement_builder(Arrangement& arr, const std::vector<Direction_2>& dirs,
                                const std::vector<Orientation>& turns, std::vector<std::size_t>& next) :
    m_arr(arr), m_dirs(dirs), m_turns(turns), m_next(next),
    m_compare_x(arr.geometry_traits_1().compare_x_1_object()),
    m_is_on_identification(arr.geometry_traits_1().is_on_x_identification_1_object())
  {}

  /*! inserts the directions into the (empty) arrangement.  The directions are
   * inserted in the order of traversal, which takes linear time when the
   * polygon is convex.  If the traversal takes too many steps (because the
   * polygon turns around many times), the directions are inserted in sorted
   * order instead, which takes \f$O(n \log n)\f$ time.
   */
  void operator()() {
    CGAL_precondition(m_arr.empty());
    if (m_dirs.empty()) return;

    // A traversal of a convex polygon takes at most n steps.  The budget below
    // bounds the total time by the time it takes to sort, up to a constant.
    const std::size_t n = m_dirs.size();
    std::size_t log_n = 1;
    while ((std::size_t(1) << log_n) < n) ++log_n;
    if (insert_by_traversal(2 * n * log_n)) return;
    m_arr.clear();
    insert_by_sorting();
  }

  /*! inserts the directions into the (empty) arrangement in the order of
   * traversal.  The vertex of the next direction is searched starting at the
   * vertex of the current one, in the direction of the turn.  The number of
   * steps is linear in the number of edges when the polygon is convex, but it
   * is proportional to the total absolute turning of the polygon in general.
   * \return `false` if the search takes more than `max_steps` steps, in which
   *         case the arrangement is left partially constructed.
   */
  bool insert_by_traversal(std::size_t max_steps) {
    const auto equal = m_arr.geometry_traits_1().equal_1_object();
    std::size_t steps = 0;

    // locates (and inserts if necessary) d walking from u counterclockwise or
    // clockwise.
    auto walk = [&](Vertex_descriptor u, const Direction_2& d, bool ccw) -> std::optional<Vertex_descriptor> {
      while (++steps <= max_steps) {
        auto e = ccw ? m_arr.right_edge(u) : m_arr.left_edge(u);
        Comparison_result left;
        switch (locate(e, d, &left)) {
         case Location::LEFT_VERTEX: return m_arr.left_vertex(e);
         case Location::RIGHT_VERTEX: return m_arr.right_vertex(e);
         case Location::INTERIOR: return insert_in_edge(e, d, left);
         case Location::OUTSIDE: break;
        }
        u = ccw ? m_arr.right_vertex(e) : m_arr.left_vertex(e);
      }
      return std::nullopt;
    };

    // inserts d into the arrangement, given the vertex of the previous
    // direction and the turn from the previous direction to d.
    auto insert = [&](Vertex_descriptor v, const Direction_2& d, Orientation turn) -> std::optional<Vertex_descriptor> {
      if (m_is_on_identification(d)) return insert_identification_point(d);
      return walk(v, d, turn != RIGHT_TURN);
    };

    auto v = m_is_on_identification(m_dirs[0]) ? insert_identification_point(m_dirs[0]) : m_arr.insert_empty(m_dirs[0]);
    add(v, 0);
    for (std::size_t k = 1; k < m_dirs.size(); ++k) {
      // A collinear turn is either no turn or a turn by 180 degrees.
      if ((m_turns[k] != COLLINEAR) || ! equal(m_dirs[k - 1], m_dirs[k])) {
        auto res = insert(v, m_dirs[k], m_turns[k]);
        if (! res) return false;
        v = *res;
      }
      add(v, k);
    }
    return true;
  }

  /*! inserts the directions into the (empty) arrangement in increasing order
   * (with the identification point, if it is one of the directions, last).  It
   * takes \f$O(n \log n)\f$ time.
   */
  void insert_by_sorting() {
    std::vector<std::size_t> indices, on_identification;
    for (std::size_t k = 0; k < m_dirs.size(); ++k)
      (m_is_on_identification(m_dirs[k]) ? on_identification : indices).push_back(k);
    std::sort(indices.begin(), indices.end(),
              [&](std::size_t i, std::size_t j) { return m_compare_x(m_dirs[i], m_dirs[j]) == SMALLER; });

    // Every direction is larger than all directions inserted so far, so it is
    // inserted after the last vertex inserted.
    Vertex_descriptor v {};
    for (std::size_t i = 0; i < indices.size(); ++i) {
      const auto& d = m_dirs[indices[i]];
      if (i == 0) v = m_arr.insert_empty(d);
      else if (m_compare_x(m_dirs[indices[i - 1]], d) != EQUAL) v = m_arr.insert_after(v, d);
      add(v, indices[i]);
    }
    if (on_identification.empty()) return;
    v = insert_identification_point(m_dirs[on_identification.front()]);
    for (auto k : on_identification) add(v, k);
  }

private:
  // The location of a direction with respect to an edge.
  enum class Location { LEFT_VERTEX, INTERIOR, RIGHT_VERTEX, OUTSIDE };

  // adds edge k to the chain of vertex v.
  void add(Vertex_descriptor v, std::size_t k) {
    auto& chain = get(m_arr.vertex_data_map(), v);
    m_next[k] = chain.m_first;
    chain.m_first = k;
    ++chain.m_size;
  }

  bool is_identification_vertex(Vertex_descriptor v) const {
    const auto& topo = m_arr.topology_traits();
    return topo.has_identification_vertex() && (v == topo.identification_vertex());
  }

  // locates the direction d, which is not the identification point, with
  // respect to the edge e (whose ends are counterclockwise from its left end to
  // its right end) using the order of the arrangement, in which the
  // identification vertex precedes all other vertices at the left end of
  // left_boundary_edge() and follows them at the right end of
  // right_boundary_edge().  If `left` is not null, it is set to the
  // comparison of the left vertex of e with d.
  Location locate(Edge_descriptor e, const Direction_2& d, Comparison_result* left = nullptr) const {
    const auto pnt_map = m_arr.vertex_point_map();
    auto u = m_arr.left_vertex(e);
    auto w = m_arr.right_vertex(e);
    const auto c_ud = is_identification_vertex(u) ? SMALLER : m_compare_x(get(pnt_map, u), d);
    if (left) *left = c_ud;
    if (c_ud == EQUAL) return Location::LEFT_VERTEX;
    if (u == w) return Location::INTERIOR;      // the only vertex; e is a loop
    const auto c_dw = is_identification_vertex(w) ? SMALLER : m_compare_x(d, get(pnt_map, w));
    if (c_dw == EQUAL) return Location::RIGHT_VERTEX;
    // Without the identification vertex, left_boundary_edge() wraps around
    // the identification point.
    const bool wraps = ! m_arr.topology_traits().has_identification_vertex() && (e == m_arr.left_boundary_edge());
    const bool interior = wraps ? ((c_ud == SMALLER) || (c_dw == SMALLER)) : ((c_ud == SMALLER) && (c_dw == SMALLER));
    return interior ? Location::INTERIOR : Location::OUTSIDE;
  }

  // inserts the direction d, which is not the identification point, into the
  // interior of the edge e; `left` is the comparison of the left vertex of e
  // with d.
  Vertex_descriptor insert_in_edge(Edge_descriptor e, const Direction_2& d, Comparison_result left) {
    // If e is a loop (at the only vertex), or if it is left_boundary_edge()
    // and it wraps around the identification point (as the identification
    // vertex does not exist), and d follows its left vertex, then d must
    // become the left end of left_boundary_edge().
    auto u = m_arr.left_vertex(e);
    const bool loop = (u == m_arr.right_vertex(e));
    const bool wraps = ! m_arr.topology_traits().has_identification_vertex() && (e == m_arr.left_boundary_edge());
    if ((left == SMALLER) && (loop || wraps)) return m_arr.insert_after(u, d);
    return m_arr.split_edge(e, d);
  }

  // inserts the identification point, unless it is already there.
  Vertex_descriptor insert_identification_point(const Direction_2& d) {
    const auto& topo = m_arr.topology_traits();
    if (topo.has_identification_vertex()) return m_arr.topology_traits().identification_vertex();
    return m_arr.empty() ? m_arr.insert_empty(d) : m_arr.insert_identification_vertex(d);
  }

  Arrangement& m_arr;
  const std::vector<Direction_2>& m_dirs;
  const std::vector<Orientation>& m_turns;
  std::vector<std::size_t>& m_next;
  typename Arrangement::Geometry_traits_1::Compare_x_1 m_compare_x;
  typename Arrangement::Geometry_traits_1::Is_on_x_identification_1 m_is_on_identification;
};

/*! the data of a vertex of the overlay of the two arrangements of directions:
 * the chains of the edges of each polygon that have its direction (empty if
 * there are none).
 */
struct Overlay_vertex_data {
  Edge_chain m_edges1;
  Edge_chain m_edges2;
};

/*! an overlay visitor that records the edge chains of the input vertices.
 */
template <typename ArrangementA, typename ArrangementB, typename ArrangementR>
class Direction_overlay_visitor {
public:
  using Vertex_const_descriptor_a = typename ArrangementA::Vertex_const_descriptor;
  using Vertex_const_descriptor_b = typename ArrangementB::Vertex_const_descriptor;
  using Vertex_descriptor_r = typename ArrangementR::Vertex_descriptor;

  using Edge_const_descriptor_a = typename ArrangementA::Edge_const_descriptor;
  using Edge_const_descriptor_b = typename ArrangementB::Edge_const_descriptor;
  using Edge_descriptor_r = typename ArrangementR::Edge_descriptor;

  Direction_overlay_visitor(const ArrangementA& arr_a, const ArrangementB& arr_b, ArrangementR& arr_r) :
    m_arr_a(arr_a), m_arr_b(arr_b), m_arr_r(arr_r)
  {}

  void create_vertex_vv(Vertex_const_descriptor_a v_a, Vertex_const_descriptor_b v_b, Vertex_descriptor_r v_res)
  { put(m_arr_r.vertex_data_map(), v_res, Overlay_vertex_data{edges_a(v_a), edges_b(v_b)}); }

  void create_vertex_ve(Vertex_const_descriptor_a v_a, Edge_const_descriptor_b, Vertex_descriptor_r v_res)
  { put(m_arr_r.vertex_data_map(), v_res, Overlay_vertex_data{edges_a(v_a), Edge_chain()}); }

  void create_vertex_ev(Edge_const_descriptor_a, Vertex_const_descriptor_b v_b, Vertex_descriptor_r v_res)
  { put(m_arr_r.vertex_data_map(), v_res, Overlay_vertex_data{Edge_chain(), edges_b(v_b)}); }

  void create_edge_ee(Edge_const_descriptor_a, Edge_const_descriptor_b, Edge_descriptor_r) {}

private:
  const Edge_chain& edges_a(Vertex_const_descriptor_a v) const { return get(m_arr_a.vertex_data_map(), v); }
  const Edge_chain& edges_b(Vertex_const_descriptor_b v) const { return get(m_arr_b.vertex_data_map(), v); }

  const ArrangementA& m_arr_a;
  const ArrangementB& m_arr_b;
  ArrangementR& m_arr_r;
};

/*! finds all convolution edges, creating separate edges for disconnected
 * overlap regions.
 *
 * The directions of the edges of each polygon are inserted into an
 * arrangement on the circle of directions (see `Direction_arrangement_builder`),
 * whose identification point is the direction of the first edge of the first
 * polygon.  The geometry traits of the arrangements adapt the traits (see
 * `Arrangement_on_curve_1::Direction_geom_traits_2_adaptor_1`); they order the
 * directions using `Counterclockwise_in_between_2` alone.
 * The two arrangements are overlaid, and the vertices of the overlay are
 * swept counterclockwise, starting at the identification point, while the
 * cones of each polygon that contain the current direction are maintained.
 * The sweep takes \f$O(m + n + k)\f$ time, where \f$m\f$ and \f$n\f$ are the
 * numbers of edges of the polygons and \f$k\f$ is the number of convolution
 * edges.  Constructing the arrangement of a polygon takes linear time if the
 * polygon is convex and \f$O(n \log n)\f$ time in general.
 * \param dirs1, dirs2 the directions of the edges of the polygons.
 * \param turns1, turns2 the turns of the polygons (see `extract_turns()`).
 * \param traits_ptr a shared pointer to the traits
 */
template <typename Traits_>
Convolution_edge_set
find_all_convolution_edges(const std::vector<typename Traits_::Direction_2>& dirs1,
                           const std::vector<Orientation>& turns1,
                           const std::vector<typename Traits_::Direction_2>& dirs2,
                           const std::vector<Orientation>& turns2,
                           const std::shared_ptr<const Traits_>& traits_ptr) {
  namespace Aoc = CGAL::Arrangement_on_curve_1;
  using Direction_traits = Aoc::Direction_geom_traits_2_adaptor_1<Traits_>;
  // The arrangements are only built and traversed, never shrunk; thus, they
  // store their vertices and edges in vectors (whose storage is reserved in
  // advance), which is faster than storing them in lists.
  using Topology_traits = Aoc::Identified_topology_traits<Direction_traits, Edge_chain, void, true>;
  using Arrangement = Aoc::Arrangement_on_curve_1<Direction_traits, Topology_traits>;
  using Overlay_topology_traits = Aoc::Identified_topology_traits<Direction_traits, Overlay_vertex_data, void, true>;
  using Overlay_arrangement = Aoc::Arrangement_on_curve_1<Direction_traits, Overlay_topology_traits>;

  Convolution_edge_set edges;
  const std::size_t n1 = dirs1.size();
  const std::size_t n2 = dirs2.size();
  if ((n1 == 0) || (n2 == 0)) return edges;

  // The arrangements and their overlay share the traits, as the overlay
  // compares the directions of both arrangements.
  auto direction_traits = std::make_shared<const Direction_traits>(traits_ptr, dirs1[0]);
  Arrangement arr1(direction_traits);
  Arrangement arr2(direction_traits);
  arr1.reserve(n1);
  arr2.reserve(n2);
  std::vector<std::size_t> next1(n1), next2(n2);
  Direction_arrangement_builder<Arrangement>(arr1, dirs1, turns1, next1)();
  Direction_arrangement_builder<Arrangement>(arr2, dirs2, turns2, next2)();

  Overlay_arrangement arr(direction_traits);
  Direction_overlay_visitor<Arrangement, Arrangement, Overlay_arrangement> visitor(arr1, arr2, arr);
  Aoc::overlay(arr1, arr2, arr, visitor);

  // The cones that contain the directions immediately after the
  // identification point.  The cone of vertex k, from dirs[k-1] to dirs[k],
  // contains them iff it wraps around the identification point, that is, iff
  // dirs[k] does not follow dirs[k-1] in the order of the vertices of the
  // arrangement (which starts after the identification point and ends at the
  // identification vertex, if it exists); this includes the case where
  // dirs[k-1] is the identification point and the case where the two
  // directions are equal (in which case the cone is the entire circle except
  // for this direction).
  auto initial_cones = [](const Arrangement& a, const std::vector<std::size_t>& next, std::size_t n) {
    std::vector<std::size_t> rank(n);
    const auto data_map = a.vertex_data_map();
    const auto lb = a.left_boundary_edge();
    std::size_t r = 0;
    for (auto e = lb; ; ++r) {
      auto v = a.right_vertex(e);
      for (auto k = get(data_map, v).m_first; k != no_edge; k = next[k]) rank[k] = r;
      e = a.right_edge(v);
      if (e == lb) break;
    }
    std::vector<std::size_t> cones;
    for (std::size_t k = 0; k < n; ++k) if (rank[k] <= rank[(k + n - 1) % n]) cones.push_back(k);
    return cones;
  };
  const auto initial_cones1 = initial_cones(arr1, next1, n1);
  const auto initial_cones2 = initial_cones(arr2, next2, n2);

  // Sweeps the vertices of the overlay counterclockwise; the identification
  // vertex, if it exists, is the last.  At the vertex of direction d, the cones
  // that end at d (those of the edges of direction d) are removed first; then,
  // the remaining cones contain d.  The cones that start at d (those of the
  // vertices following the edges of direction d) are inserted last.  When
  // `count_only` is true, the convolution edges are only counted.
  const auto data_map = arr.vertex_data_map();
  const auto lb = arr.left_boundary_edge();
  auto sweep = [&](bool count_only) {
    Cone_set cones1(n1);
    Cone_set cones2(n2);
    for (auto k1 : initial_cones1) cones1.insert(k1);
    for (auto k2 : initial_cones2) cones2.insert(k2);
    std::size_t count = 0;
    auto e = lb;
    while (true) {
      auto v = arr.right_vertex(e);
      e = arr.right_edge(v);
      const auto& data = get(data_map, v);
      const Edge_chain& edges1 = data.m_edges1;
      const Edge_chain& edges2 = data.m_edges2;

      for (auto k1 = edges1.m_first; k1 != no_edge; k1 = next1[k1]) cones1.erase(k1);
      for (auto k2 = edges2.m_first; k2 != no_edge; k2 = next2[k2]) cones2.erase(k2);

      if (count_only)
        count += edges1.m_size * cones2.size() + edges1.m_size * edges2.m_size + edges2.m_size * cones1.size();
      else {
        for (auto k1 = edges1.m_first; k1 != no_edge; k1 = next1[k1]) {
          cones2.for_each([&](std::size_t k2) { edges.emplace(k1, k2, Cone_overlap_result::First); });
          for (auto k2 = edges2.m_first; k2 != no_edge; k2 = next2[k2])
            edges.emplace(k1, k2, Cone_overlap_result::Both);
        }
        for (auto k2 = edges2.m_first; k2 != no_edge; k2 = next2[k2])
          cones1.for_each([&](std::size_t k1) { edges.emplace(k1, k2, Cone_overlap_result::Second); });
      }

      // The cones that start at the last vertex contain the directions
      // immediately after the identification point; they are already in.
      if (e == lb) break;
      for (auto k1 = edges1.m_first; k1 != no_edge; k1 = next1[k1]) cones1.insert((k1 + 1) % n1);
      for (auto k2 = edges2.m_first; k2 != no_edge; k2 = next2[k2]) cones2.insert((k2 + 1) % n2);
    }
    return count;
  };

  // Counting first avoids rehashing, which dominates when there are many
  // convolution edges.
  edges.reserve(sweep(true));
  sweep(false);
  return edges;
}

/*! generates a single convolution path.
 * \param pts1, pts2 the vertices of the polygons, accessed by their indices
 *        in constant time (see `Random_access_vertices`).
 * \param ids if not null, the identifiers of the vertices of the path are
 *        appended to it; the identifier of the vertex \f$ p_i + q_j\f$ is
 *        `id_offset` + \f$ i n_2 + j\f$, where \f$ n_2\f$ is the number of
 *        vertices of the second polygon.
 */
template <typename Kernel_, typename Container_, typename Traits_>
std::vector<typename Kernel_::Point_2>
generate_single_path(const Random_access_vertices<Kernel_, Container_>& pts1,
                     const Random_access_vertices<Kernel_, Container_>& pts2,
                     const std::vector<typename Traits_::Direction_2>& dirs1,
                     const std::vector<typename Traits_::Direction_2>& dirs2,
                     const Convolution_edge& start_edge,
                     Convolution_edge_set& remaining_edges,
                     const Traits_& traits,
                     std::vector<std::size_t>* ids = nullptr, std::size_t id_offset = 0) {
  using Point_2 = typename Kernel_::Point_2;
  using Convolution_path = std::vector<Point_2>;

  const auto construct_vector = traits.construct_vector_2_object();
  const auto construct_translated_point_2 = traits.construct_translated_point_2_object();

  auto start_k1 = start_edge.m_k1;
  auto start_k2 = start_edge.m_k2;
  auto start_result = start_edge.m_result;

  std::size_t n1 = dirs1.size();
  std::size_t n2 = dirs2.size();
  Convolution_path conv_path;

  // Every iteration consumes a convolution edge; thus, the number of
  // iterations is bounded by the number of remaining edges.
  CGAL_assertion_code(std::size_t iteration = 0;)
  CGAL_assertion_code(const std::size_t max_iterations = remaining_edges.size();)

  std::size_t k1 = start_k1;
  std::size_t k2 = start_k2;
  Point_2 start_pt = construct_translated_point_2(pts1[k1], construct_vector(ORIGIN, pts2[k2]));
  conv_path.push_back(start_pt);
  if (ids) ids->push_back(id_offset + k1 * n2 + k2);

  remaining_edges.erase(start_edge);

  std::size_t k1_next = (k1 + 1) % n1;
  std::size_t k2_next = (k2 + 1) % n2;

  // Initial step: use start_result to determine direction
  if (start_result == Cone_overlap_result::Both) {
    Point_2 next_pt = construct_translated_point_2(pts1[k1_next], construct_vector(ORIGIN, pts2[k2_next]));
    conv_path.push_back(next_pt);
    if (ids) ids->push_back(id_offset + k1_next * n2 + k2_next);
    k1 = k1_next;
    k2 = k2_next;
  }
  else if (start_result == Cone_overlap_result::First) {
    Point_2 next_pt = construct_translated_point_2(pts1[k1_next], construct_vector(ORIGIN, pts2[k2]));
    conv_path.push_back(next_pt);
    if (ids) ids->push_back(id_offset + k1_next * n2 + k2);
    k1 = k1_next;
  }
  else {
    CGAL_assertion(start_result == Cone_overlap_result::Second);
    Point_2 next_pt = construct_translated_point_2(pts1[k1], construct_vector(ORIGIN, pts2[k2_next]));
    conv_path.push_back(next_pt);
    if (ids) ids->push_back(id_offset + k1 * n2 + k2_next);
    k2 = k2_next;
  }

  // Continue while edge exist for current pair
  while (true) {
    Convolution_edge found_edge(k1, k2, Cone_overlap_result::None);
    auto it = remaining_edges.end();

    for (auto possible_result : {Cone_overlap_result::First, Cone_overlap_result::Second, Cone_overlap_result::Both}) {
      it = remaining_edges.find(Convolution_edge(k1, k2, possible_result));
      if (it != remaining_edges.end()) {
        found_edge = *it;
        break;
      }
    }

    if (it == remaining_edges.end()) break;
    remaining_edges.erase(it);

    CGAL_assertion(++iteration < max_iterations);

    std::size_t k1_next = (k1 + 1) % n1;
    std::size_t k2_next = (k2 + 1) % n2;

    if (found_edge.m_result == Cone_overlap_result::First) {
      // Advance k1
      Point_2 next_pt = construct_translated_point_2(pts1[k1_next], construct_vector(ORIGIN, pts2[k2]));
      conv_path.push_back(next_pt);
      if (ids) ids->push_back(id_offset + k1_next * n2 + k2);
      k1 = k1_next;
      continue;
    }

    if (found_edge.m_result == Cone_overlap_result::Second) {
      // Advance k2
      Point_2 next_pt = construct_translated_point_2(pts1[k1], construct_vector(ORIGIN, pts2[k2_next]));
      conv_path.push_back(next_pt);
      if (ids) ids->push_back(id_offset + k1 * n2 + k2_next);
      k2 = k2_next;
      continue;
    }

    CGAL_assertion(found_edge.m_result == Cone_overlap_result::Both);
    // Both directions identical - advance both
    Point_2 next_pt = construct_translated_point_2(pts1[k1_next], construct_vector(ORIGIN, pts2[k2_next]));
    conv_path.push_back(next_pt);
    if (ids) ids->push_back(id_offset + k1_next * n2 + k2_next);
    k1 = k1_next;
    k2 = k2_next;
  }

  return conv_path;
}

/*! removes the translated copies of either polygon from a set of convolution
 * edges.  If the edges (k1, k2, First) exist for all k1, the cone of vertex k2
 * of the second polygon contains all the directions of the first polygon (the
 * vertex is reflex, or its incident edges are collinear, and its turn contains
 * no such direction); these edges form the boundary of the first polygon
 * translated by the vertex, a closed cycle.  Symmetrically, the edges
 * (k1, k2, Second) for all k2 form the boundary of the second polygon
 * translated by vertex k1 of the first.  Removing such a cycle leaves the
 * remaining edges a union of closed cycles, and decreases the winding number
 * by one exactly in the interior of the translated copy, which is contained in
 * the sum.
 *
 * Let \f$ K = Q \cap (x - P)\f$.  The winding number at \f$ x\f$ is
 * \f$\chi(K)\f$ plus the number of translated copies (one for every reflex
 * or collinear vertex of either polygon) that contain \f$ x\f$.  If one of
 * the polygons is convex (and has no holes), \f$ K\f$ has no holes, so
 * \f$\chi(K)\f$ is the number of the connected components of \f$ K\f$,
 * which is positive in the sum; thus, the copies may be removed without
 * affecting the set of points whose winding number is positive.  Otherwise,
 * for two simple polygons, the removal is not proven to be safe, but it has
 * been verified experimentally, and the former implementation of the full
 * convolution method, which supports only simple polygons, removes the copies
 * unconditionally.  For two polygons with holes, neither of which is convex,
 * the removal is not safe: a translated copy of the outer boundary of one
 * polygon need not be contained in the sum, and the clockwise cycles of the
 * holes contribute negative winding numbers, so removing a copy may lose
 * points of the sum (as experiments confirm).  The removal is never safe when
 * the winding numbers of the faces must be exact, as in the Minkowski
 * difference.
 *
 * Only copies of counterclockwise cycles are removed.  A copy of a clockwise
 * cycle, such as a hole, translated by a vertex of the other polygon
 * decreases the winding number by one in its interior, which need not be
 * contained in the sum; removing it may thus add points to the sum.  (If one
 * of the polygons is convex, no such copy occurs, as the cone of a convex
 * vertex cannot contain all the directions of a cycle.)
 * \param edges the convolution edges
 * \param n1 the number of vertices of the first polygon
 * \param n2 the number of vertices of the second polygon
 * \param remove1 whether to remove the copies of the first polygon
 * \param remove2 whether to remove the copies of the second polygon
 */
inline void remove_translated_copies(Convolution_edge_set& edges, std::size_t n1, std::size_t n2,
                                     bool remove1 = true, bool remove2 = true) {
  std::vector<std::size_t> num_first(n2, 0);
  std::vector<std::size_t> num_second(n1, 0);
  for (const auto& edge : edges) {
    if (edge.m_result == Cone_overlap_result::First) ++num_first[edge.m_k2];
    else if (edge.m_result == Cone_overlap_result::Second) ++num_second[edge.m_k1];
  }
  for (std::size_t k2 = 0; remove1 && (k2 < n2); ++k2) {
    if (num_first[k2] != n1) continue;
    for (std::size_t k1 = 0; k1 < n1; ++k1) edges.erase(Convolution_edge(k1, k2, Cone_overlap_result::First));
  }
  for (std::size_t k1 = 0; remove2 && (k1 < n1); ++k1) {
    if (num_second[k1] != n2) continue;
    for (std::size_t k2 = 0; k2 < n2; ++k2) edges.erase(Convolution_edge(k1, k2, Cone_overlap_result::Second));
  }
}

/*! generates all convolution paths for two polygon boundaries.
 * \param ids if not null, the identifiers of the vertices of every path are
 *        appended to it (see `generate_single_path()`); equal identifiers
 *        identify the same vertex \f$ p_i + q_j\f$, which may occur more than
 *        once in the paths.
 * \param remove_copies indicates whether to remove the translated copies of
 *        the counterclockwise polygons from the convolution cycles (see
 *        `remove_translated_copies()`); this is valid for the sum of simple
 *        polygons, and of polygons with holes one of which is convex, but not
 *        for the difference.
 */
template <typename Kernel_, typename Container_, typename Traits_>
std::vector<std::vector<typename Kernel_::Point_2>>
generate_convolution_paths(const Polygon_2<Kernel_, Container_>& pgn1, const Polygon_2<Kernel_, Container_>& pgn2,
                           const std::shared_ptr<const Traits_>& traits_ptr,
                           std::vector<std::vector<std::size_t>>* ids = nullptr, std::size_t id_offset = 0,
                           bool remove_copies = false) {
  const Traits_& traits = *traits_ptr;
  using Point_2 = typename Kernel_::Point_2;
  using Direction_2 = typename Traits_::Direction_2;
  using Convolution_path = std::vector<Point_2>;

  std::vector<Convolution_path> paths;
  std::vector<Direction_2> dirs1 = extract_directions(pgn1, traits);
  std::vector<Direction_2> dirs2 = extract_directions(pgn2, traits);
  // The vertices are accessed by their indices in constant time, also if the
  // container of the polygons is not random access; see
  // `Random_access_vertices`.
  const Random_access_vertices<Kernel_, Container_> pts1(pgn1), pts2(pgn2);

  const auto turns1 = extract_turns(pgn1, traits);
  const auto turns2 = extract_turns(pgn2, traits);

  // The convolution of two strictly convex polygons oriented
  // counterclockwise (all of whose turns are left turns) is a single cycle.
  auto is_strictly_convex = [](const std::vector<Orientation>& turns)
  { return std::all_of(turns.begin(), turns.end(), [](Orientation o) { return o == LEFT_TURN; }); };
  if (is_strictly_convex(turns1) && is_strictly_convex(turns2)) {
    if (ids) paths.push_back(generate_convex_path(pts1, pts2, dirs1, dirs2, traits, &ids->emplace_back(), id_offset));
    else paths.push_back(generate_convex_path(pts1, pts2, dirs1, dirs2, traits));
    return paths;
  }

  auto remaining_edges = find_all_convolution_edges(dirs1, turns1, dirs2, turns2, traits_ptr);
  if (remove_copies)
    remove_translated_copies(remaining_edges, dirs1.size(), dirs2.size(), pgn1.orientation() == COUNTERCLOCKWISE,
                             pgn2.orientation() == COUNTERCLOCKWISE);

  // Start a path at every edge that no path generated so far contains.  The
  // edges are traversed in a copy of the set, as obtaining the first element
  // of a hash table with open addressing, which `begin()` does, may take time
  // proportional to its capacity.
  const std::vector<Convolution_edge> all_edges(remaining_edges.begin(), remaining_edges.end());
  for (const auto& start_edge : all_edges) {
    if (remaining_edges.empty()) break;
    if (remaining_edges.find(start_edge) == remaining_edges.end()) continue;
    std::vector<std::size_t>* path_ids = nullptr;
    if (ids) path_ids = &ids->emplace_back();
    paths.push_back(generate_single_path(pts1, pts2, dirs1, dirs2, start_edge, remaining_edges, traits, path_ids,
                                         id_offset));
  }
  return paths;
}

// ============================================================================
// WINDING NUMBER COMPUTATION
// ============================================================================

template <typename ArrangementWithWinding_2>
void compute_winding_numbers(ArrangementWithWinding_2& arr) {
  using Arrangement_with_winding_2 = ArrangementWithWinding_2;
  using Face_handle = typename Arrangement_with_winding_2::Face_handle;

  auto unbounded_face = arr.unbounded_face();

  std::queue<Face_handle> queue;
  queue.push(unbounded_face);
  unbounded_face->data().m_winding_number = 0;
  unbounded_face->data().m_visited = true;

  while (! queue.empty()) {
    auto face = queue.front();
    queue.pop();
    auto current_winding = face->data().m_winding_number;

    if (face->has_outer_ccb()) {
      auto ccb = face->outer_ccb();
      do {
        auto halfedge = ccb;
        auto opposite = halfedge->twin();
        auto adjacent_face = opposite->face();
        if (adjacent_face->data().m_visited) continue;

        auto curve_data = halfedge->curve().data();
        if (halfedge->direction() != CGAL::ARR_RIGHT_TO_LEFT) curve_data = -curve_data;
        adjacent_face->data().m_winding_number = current_winding + curve_data;
        adjacent_face->data().m_visited = true;
        queue.push(adjacent_face);
      } while (++ccb != face->outer_ccb());
    }

    for (auto hole = face->inner_ccbs_begin(); hole != face->inner_ccbs_end(); ++hole) {
      auto ccb = *hole;
      do {
        auto halfedge = ccb;
        auto opposite = halfedge->twin();
        auto adjacent_face = opposite->face();
        if (adjacent_face->data().m_visited) continue;

        auto curve_data = halfedge->curve().data();
        if (halfedge->direction() != CGAL::ARR_RIGHT_TO_LEFT) curve_data = -curve_data;
        adjacent_face->data().m_winding_number = current_winding + curve_data;
        adjacent_face->data().m_visited = true;
        queue.push(adjacent_face);
      } while (++ccb != *hole);
    }
  }
}

// ============================================================================
// CURVE REPRESENTATION
// ============================================================================

// The convolution cycles are represented either by segments or by polylines;
// the choice is made at run time (see Convolution_curves and prefer_segments()
// below).  The traits of the curves that represent them are either the traits
// given by the user, `Traits_`, whose curves are segments, or
// `Arr_polyline_traits_2<Traits_>`.

/*! determines whether the traits of the curves that represent the convolution
 * cycles are the traits of polylines.
 */
template <typename CurveTraits_2>
struct Is_polyline_traits : std::false_type {};

template <typename SubcurveTraits_2>
struct Is_polyline_traits<Arr_polyline_traits_2<SubcurveTraits_2>> : std::true_type {};

/*! applies `f` to the polyline that represents a convolution path.
 */
template <typename SubcurveTraits_2, typename PointRange, typename Function>
void for_each_curve(const Arr_polyline_traits_2<SubcurveTraits_2>& traits, const PointRange& path, Function f)
{ f(traits.construct_curve_2_object()(path.begin(), path.end())); }

/*! obtains the points of a polycurve, ordered from its source to its target.
 */
template <typename Subcurve_2, typename Point_2>
auto curve_points(const CGAL::internal::Polycurve_2<Subcurve_2, Point_2>& cv)
{ return CGAL::make_range(cv.points_begin(), cv.points_end()); }

/*! appends the points of an x-monotone polycurve, except for its last point
 * (in the traversal direction), to `vertices`.
 * \param is_forward indicates whether the curve is traversed from its source
 *        to its target.
 */
template <typename XMonotoneCurve_2, typename Point_2>
void append_curve_points(const XMonotoneCurve_2& cv, bool is_forward, std::vector<Point_2>& vertices) {
  const auto& points = curve_points(cv);
  if (is_forward) vertices.insert(vertices.end(), points.begin(), std::prev(points.end()));
  else for (auto pit = std::prev(points.end()); pit != points.begin(); --pit) vertices.push_back(*pit);
}

// ============================================================================
// RESULT EXTRACTION
// ============================================================================

/*! extracts the region of an arrangement that consists of the faces that
 * satisfy a predicate, as a set of polygons with holes, one per component of
 * the region (where two faces are in the same component if they share an
 * edge).  The outer boundaries are oriented counterclockwise, and the holes
 * are oriented clockwise.  If the region contains the unbounded face, the
 * component of the unbounded face is unbounded; it is extracted as a polygon
 * with holes without an outer boundary.  The data of the faces must be of the type
 * `Face_data`, whose field `m_component` is used to record the components (it
 * must be -1 initially).
 * \param arr the arrangement
 * \param in the predicate, which accepts a face handle
 * \param connected indicates whether the region is known to be connected (and
 *        to consist of most of the faces, as does a Minkowski sum), in which
 *        case its components are not computed
 * \param oi the output iterator for the polygons with holes
 * \param traits the traits, which provide `Compare_xy_2` and `Orientation_2`
 */
template <typename Kernel_, typename Container_, typename Arrangement, typename Predicate,
          typename OutputIterator, typename Traits_>
OutputIterator extract_region(const Arrangement& arr, Predicate in, bool connected, OutputIterator oi,
                              const Traits_& traits) {
  using Polygon_2 = CGAL::Polygon_2<Kernel_, Container_>;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel_, Container_>;
  using Point_2 = typename Kernel_::Point_2;
  using Face_const_handle = typename Arrangement::Face_const_handle;
  using Halfedge_const_handle = typename Arrangement::Halfedge_const_handle;
  using Ccb_halfedge_const_circulator = typename Arrangement::Ccb_halfedge_const_circulator;

  // Applies `f` to every halfedge on the boundary of the face `g`.
  auto for_each_ccb_halfedge = [](Face_const_handle g, auto f) {
    auto apply = [&](Ccb_halfedge_const_circulator ccb) {
      auto curr = ccb;
      do f(Halfedge_const_handle(curr)); while (++curr != ccb);
    };
    if (g->has_outer_ccb()) apply(g->outer_ccb());
    for (auto hit = g->inner_ccbs_begin(); hit != g->inner_ccbs_end(); ++hit) apply(*hit);
  };

  // Count the faces of the region, unless it is connected (and consists of
  // most of the faces).
  std::size_t num_in = 0;
  if (! connected) {
    for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) if (in(Face_const_handle(fit))) ++num_in;
    if (num_in == 0) return oi;
  }

  // Assign the faces of the region to components (by breadth-first search
  // through the edges shared by faces of the region), unless the region is
  // connected.
  auto component = [&](Face_const_handle f) -> int { return connected ? 0 : f->data().m_component; };
  int num_components = connected ? 1 : 0;
  if (! connected) {
    for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
      Face_const_handle f = fit;
      if (! in(f) || (f->data().m_component >= 0)) continue;
      const int c = num_components++;
      std::queue<Face_const_handle> queue;
      f->data().m_component = c;
      queue.push(f);
      while (! queue.empty()) {
        auto g = queue.front();
        queue.pop();
        for_each_ccb_halfedge(g, [&](Halfedge_const_handle e) {
          auto h = e->twin()->face();
          if (in(h) && (h->data().m_component < 0)) {
            h->data().m_component = c;
            queue.push(h);
          }
        });
      }
    }
  }

  // Collect the boundary halfedges, which have the region on their left.  A
  // boundary halfedge lies on the boundary of a face of the region, and its
  // twin lies on the boundary of a face not in the region; the faces of the
  // smaller of these two sets are scanned.
  std::vector<Halfedge_const_handle> boundary;
  const bool scan_in = ! connected && (2 * num_in <= arr.number_of_faces());
  for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
    Face_const_handle g = fit;
    if (in(g) != scan_in) continue;
    for_each_ccb_halfedge(g, [&](Halfedge_const_handle e) {
      if (in(e->twin()->face()) != scan_in) boundary.push_back(scan_in ? e : e->twin());
    });
  }

  // Trace the boundary cycles of the region.  At the target vertex of a
  // boundary halfedge, the next boundary halfedge is found by rotating around
  // the vertex through the faces of the region.
  using Curve_traits_2 = typename Arrangement::Geometry_traits_2::Base_traits_2;
  const auto compare_xy = traits.compare_xy_2_object();
  const auto orientation = traits.orientation_2_object();
  std::vector<std::vector<Polygon_2>> outers(num_components), holes(num_components);
  CGAL::Unique_hash_map<Halfedge_const_handle, bool> visited(false);
  for (auto h : boundary) {
    if (visited[h]) continue;
    std::vector<Point_2> vertices;
    auto g = h;
    do {
      visited[g] = true;
      // The points of the curve of g, from its source to its target, except
      // for its target; the curve of a segment has no other points than the
      // points of the vertices of g.
      if constexpr (Is_polyline_traits<Curve_traits_2>::value) {
        const auto compare_endpoints_xy = arr.geometry_traits()->compare_endpoints_xy_2_object();
        const bool is_forward =
          ((g->direction() == ARR_LEFT_TO_RIGHT) == (compare_endpoints_xy(g->curve()) == SMALLER));
        append_curve_points(g->curve(), is_forward, vertices);
      }
      else vertices.push_back(g->source()->point());
      auto next = g->next();
      while (in(next->twin()->face())) next = next->twin()->next();
      g = next;
    } while (g != h);

    // The cycle, with the region on its left, is the outer boundary of its
    // component iff it is oriented counterclockwise, which is determined by
    // the turn at its lexicographically smallest vertex (the signed area,
    // whose exact evaluation is expensive, is used only if this turn is
    // degenerate).
    const std::size_t n = vertices.size();
    std::size_t k_min = 0;
    for (std::size_t k = 1; k < n; ++k) if (compare_xy(vertices[k], vertices[k_min]) == SMALLER) k_min = k;
    auto turn = orientation(vertices[(k_min + n - 1) % n], vertices[k_min], vertices[(k_min + 1) % n]);
    Polygon_2 cycle(vertices.begin(), vertices.end());
    if (turn == COLLINEAR) turn = (cycle.area() > 0) ? LEFT_TURN : RIGHT_TURN;
    const int c = component(h->face());
    if (turn == LEFT_TURN) outers[c].push_back(std::move(cycle));
    else holes[c].push_back(std::move(cycle));
  }

  // Every component is bounded by one outer boundary, except for the
  // component of the unbounded face, which has none (a connected region may
  // be empty, or the entire plane).
  const Face_const_handle unbounded_face = arr.unbounded_face();
  if (boundary.empty()) {
    if (in(unbounded_face)) *oi++ = Polygon_with_holes_2();
    return oi;
  }
  for (int c = 0; c < num_components; ++c) {
    CGAL_assertion(outers[c].size() <= 1);
    if (outers[c].empty()) {
      CGAL_assertion(in(unbounded_face) && (component(unbounded_face) == c));
      *oi++ = Polygon_with_holes_2(Polygon_2(), std::make_move_iterator(holes[c].begin()),
                                   std::make_move_iterator(holes[c].end()));
      continue;
    }
    *oi++ = Polygon_with_holes_2(std::move(outers[c].front()), std::make_move_iterator(holes[c].begin()),
                                 std::make_move_iterator(holes[c].end()));
  }
  return oi;
}

// ============================================================================
// CURVE DATA TRAITS
// ============================================================================

struct Merge {
  int operator()(const int d1, const int& d2) const { return d1 + d2; }
};

struct Convert {
  int operator()(const bool& /* d */) const { return 0; }
};

/*! the traits of the curves of the arrangement of convolution cycles, which
 * attach a winding number to every curve.  The traits of segments (the traits
 * given by the user) also label the vertices of the convolution cycles. The labels guarantee that
 * the common endpoint of consecutive segments, which is a constructed point, is
 * not compared with itself geometrically (which the filters of a lazy kernel
 * cannot decide), and that consecutive segments, which are not collinear, are
 * not intersected. A vertex is labeled by a single label, its identifier
 * (see `generate_single_path()`), which all its occurrences share; sets of
 * labels, which also record the coincidences of distinct vertices, make the
 * points and segments larger and the construction slower, and save few exact
 * evaluations, even for degenerate input.
 */
template <typename CurveTraits_2>
struct Convolution_data_traits {
  using type = CGAL::Arr_data_traits_2<CurveTraits_2,
                                       Arr_label_policy_2<int, Tag_true, Merge, int, Convert, std::size_t, Tag_false>>;
};

/*! the traits of the polylines of the arrangement of convolution cycles, which
 * are extended with curve data only.
 */
template <typename SubcurveTraits_2>
struct Convolution_data_traits<Arr_polyline_traits_2<SubcurveTraits_2>> {
  using type = CGAL::Arr_curve_data_traits_2<Arr_polyline_traits_2<SubcurveTraits_2>, int, Merge, int, Convert>;
};

//! the traits of the curves of the arrangement of convolution cycles.
template <typename CurveTraits_2>
using Convolution_data_traits_2 = typename Convolution_data_traits<CurveTraits_2>::type;

//! the arrangement of convolution cycles, whose faces store winding numbers.
template <typename CurveTraits_2>
using Convolution_arrangement_2 =
  CGAL::Arrangement_2<Convolution_data_traits_2<CurveTraits_2>,
                      CGAL::Arr_face_extended_dcel<Convolution_data_traits_2<CurveTraits_2>, Face_data>>;

/*! constructs the x-monotone polylines of convolution paths.
 */
template <typename SubcurveTraits_2, typename Point_2, typename OutputIterator>
void construct_convolution_curves(const Arr_curve_data_traits_2<Arr_polyline_traits_2<SubcurveTraits_2>,
                                                                int, Merge, int, Convert>& traits,
                                  const std::vector<std::vector<Point_2>>& paths,
                                  const std::vector<std::vector<std::size_t>>& /* ids */, OutputIterator oi) {
  using DataTraits_2 = Arr_curve_data_traits_2<Arr_polyline_traits_2<SubcurveTraits_2>, int, Merge, int, Convert>;
  using Curve_2 = typename DataTraits_2::Curve_2;
  std::vector<Curve_2> convolution_curves;
  for (const auto& path : paths)
    for_each_curve(traits, path, [&](auto&& cv) { convolution_curves.emplace_back(std::move(cv), 0); });

  std::vector<typename DataTraits_2::Point_2> points;
  make_x_monotone(convolution_curves.begin(), convolution_curves.end(), oi, std::back_inserter(points), traits);
}

/*! constructs the segments of convolution paths. Every vertex of a path is
 * labeled by its identifier, so that all the occurrences of a vertex are
 * labeled alike; if no identifiers are given, every occurrence is labeled by
 * its position. As a path is closed, its last segment ends at its first
 * vertex.
 */
template <typename Traits_, typename Policy_, typename Point_2, typename OutputIterator>
void construct_convolution_curves(const Arr_data_traits_2<Traits_, Policy_>& traits,
                                  const std::vector<std::vector<Point_2>>& paths,
                                  const std::vector<std::vector<std::size_t>>& ids, OutputIterator oi) {
  using Data_traits_2 = Arr_data_traits_2<Traits_, Policy_>;
  using Labeled_point_2 = typename Data_traits_2::Point_2;
  using Label = typename Policy_::Label;

  CGAL_precondition(ids.empty() || (ids.size() == paths.size()));
  auto ctr_xcv = traits.construct_x_monotone_curve_2_object();
  Label label = 0;
  for (std::size_t c = 0; c < paths.size(); ++c) {
    const auto& path = paths[c];
    if (path.size() < 2) continue;
    CGAL_assertion(path.front() == path.back());
    CGAL_assertion(ids.empty() || ((ids[c].size() == path.size()) && (ids[c].front() == ids[c].back())));
    const std::size_t n = path.size() - 1;
    auto label_of = [&](std::size_t i) { return ids.empty() ? label + Label(i % n) : Label(ids[c][i]); };
    const Labeled_point_2 first(path.front(), {label_of(0)});
    Labeled_point_2 prev = first;
    for (std::size_t i = 1; i <= n; ++i) {
      const Labeled_point_2 curr = (i == n) ? first : Labeled_point_2(path[i], {label_of(i)});
      *oi++ = ctr_xcv(prev, curr, 0);
      prev = curr;
    }
    label += Label(n);
  }
}

/*! constructs the traits of the curves of the arrangement of convolution
 * cycles from the given traits, which must outlive the result (the traits of
 * polylines refer to them).
 * \tparam CurveTraits_2 the traits of the curves that represent the
 *         convolution cycles; either `Traits_` (segments) or
 *         `Arr_polyline_traits_2<Traits_>`.
 */
template <typename CurveTraits_2, typename Traits_>
Convolution_data_traits_2<CurveTraits_2> make_convolution_traits(const std::shared_ptr<const Traits_>& traits_ptr) {
  using Data_traits_2 = Convolution_data_traits_2<CurveTraits_2>;
  static_assert(std::is_same_v<CurveTraits_2, Traits_> ||
                std::is_same_v<CurveTraits_2, Arr_polyline_traits_2<Traits_>>);
  if constexpr (Is_polyline_traits<CurveTraits_2>::value) return Data_traits_2(CurveTraits_2(traits_ptr.get()));
  else return Data_traits_2(*traits_ptr);
}

/*! constructs the arrangement of convolution cycles and computes the winding
 * numbers of its faces.
 * \tparam CurveTraits_2 the traits of the curves that represent the
 *         convolution cycles; either the traits given by the user (segments)
 *         or `Arr_polyline_traits_2` of these traits.
 * \param paths the convolution cycles
 * \param ids the identifiers of the vertices of the convolution cycles, or
 *        empty
 * \param arr the resulting arrangement, which must be empty
 */
template <typename CurveTraits_2, typename Point_2>
void construct_convolution_arrangement(const std::vector<std::vector<Point_2>>& paths,
                                       const std::vector<std::vector<std::size_t>>& ids,
                                       Convolution_arrangement_2<CurveTraits_2>& arr) {
  using Data_traits_2 = Convolution_data_traits_2<CurveTraits_2>;
  using X_monotone_curve_2 = typename Data_traits_2::X_monotone_curve_2;

  const auto& traits = *arr.geometry_traits();

  std::vector<X_monotone_curve_2> xcvs;
  construct_convolution_curves(traits, paths, ids, std::back_inserter(xcvs));

  for (auto& xcv : xcvs) {
    auto endpoint_order = traits.compare_endpoints_xy_2_object()(xcv);
    int data = (endpoint_order == CGAL::SMALLER) ? 1 : -1;
    xcv.set_data(data);
  }

  // Aggregate insertion (a single sweep) is much faster than inserting the
  // curves one at a time, as each incremental insertion traverses a zone.
  CGAL::insert(arr, xcvs.begin(), xcvs.end());
  compute_winding_numbers(arr);
}

/*! computes the Minkowski sum of two input polygons, given their convolution
 * cycles.
 * \tparam CurveTraits_2 the traits of the curves that represent the
 *         convolution cycles; either the traits given by the user (segments)
 *         or `Arr_polyline_traits_2` of these traits.
 * \param paths the convolution cycles of the two summand polygons
 * \param ids the identifiers of the vertices of the convolution cycles, or
 *        empty
 * \param traits_ptr a shared pointer to the traits
 */
template <typename CurveTraits_2, typename Kernel_, typename Container_, typename Traits_>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_full_convolution_2_impl(const std::vector<std::vector<typename Kernel_::Point_2>>& paths,
                                         const std::vector<std::vector<std::size_t>>& ids,
                                         const std::shared_ptr<const Traits_>& traits_ptr) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel, Container>;
  using Arrangement = Convolution_arrangement_2<CurveTraits_2>;

  const auto traits = make_convolution_traits<CurveTraits_2>(traits_ptr);
  Arrangement arr(&traits);
  construct_convolution_arrangement<CurveTraits_2>(paths, ids, arr);
  // The sum consists of the faces whose winding numbers are positive; it is
  // connected.
  auto in = [](typename Arrangement::Face_const_handle f)
  { return f->data().m_visited && (f->data().m_winding_number > 0); };
  std::vector<Polygon_with_holes_2> components;
  extract_region<Kernel, Container>(arr, in, true, std::back_inserter(components), *traits_ptr);
  CGAL_assertion(components.size() <= 1);
  return components.empty() ? Polygon_with_holes_2() : std::move(components.front());
}

/*! computes the Minkowski sum of a bounded polygon and an unbounded polygon
 * with holes, given the convolution cycles of the outer boundary of the former
 * and the holes of the latter.  The points that are not in the sum are those
 * whose winding number with respect to these cycles is `winding`; they form
 * bounded, simply connected regions, the holes of the (unbounded and connected)
 * sum.
 * \tparam CurveTraits_2 the traits of the curves that represent the
 *         convolution cycles; either the traits given by the user (segments)
 *         or `Arr_polyline_traits_2` of these traits.
 * \param paths the convolution cycles
 * \param ids the identifiers of the vertices of the convolution cycles, or
 *        empty
 * \param winding the winding number of the faces that are not in the sum
 * \param traits_ptr a shared pointer to the traits
 */
template <typename CurveTraits_2, typename Kernel_, typename Container_, typename Traits_>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_with_unbounded_2_impl(const std::vector<std::vector<typename Kernel_::Point_2>>& paths,
                                    const std::vector<std::vector<std::size_t>>& ids, int winding,
                                    const std::shared_ptr<const Traits_>& traits_ptr) {
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel_, Container_>;
  using Arrangement = Convolution_arrangement_2<CurveTraits_2>;

  const auto traits = make_convolution_traits<CurveTraits_2>(traits_ptr);
  Arrangement arr(&traits);
  construct_convolution_arrangement<CurveTraits_2>(paths, ids, arr);
  auto in = [winding](typename Arrangement::Face_const_handle f)
  { return ! f->data().m_visited || (f->data().m_winding_number != winding); };
  std::vector<Polygon_with_holes_2> components;
  extract_region<Kernel_, Container_>(arr, in, true, std::back_inserter(components), *traits_ptr);
  CGAL_assertion((components.size() == 1) && components.front().is_unbounded());
  return std::move(components.front());
}

/*! obtains the representation of the convolution cycles, given by the named
 * parameter `convolution_curves`, or `Convolution_curves::AUTOMATIC`.
 */
template <typename NamedParameters>
Convolution_curves convolution_curves_parameter(const NamedParameters& np) {
  using parameters::choose_parameter;
  using parameters::get_parameter;
  return choose_parameter(get_parameter(np, internal_np::convolution_curves), Convolution_curves::AUTOMATIC);
}


/*! counts the maximal x-monotone chains of closed convolution paths, that is,
 * the number of x-monotone polylines that represent them. A chain ends where
 * the lexicographic xy-order of the consecutive vertices of a path reverses.
 */
template <typename Point_2>
std::size_t number_of_x_monotone_chains(const std::vector<std::vector<Point_2>>& paths) {
  std::size_t num_chains = 0;
  for (const auto& path : paths) {
    if (path.size() < 3) continue;
    // The last vertex of a path is its first vertex.
    const std::size_t n = path.size() - 1;
    std::size_t num_turns = 0;
    auto prev = CGAL::compare_xy(path[n - 1], path[0]);
    for (std::size_t i = 0; i < n; ++i) {
      const auto curr = CGAL::compare_xy(path[i], path[i + 1]);
      if (curr != prev) ++num_turns;
      prev = curr;
    }
    num_chains += std::max<std::size_t>(num_turns, 1);
  }
  return num_chains;
}

/*! determines whether to represent the convolution cycles of two polygons
 * with m and n vertices, respectively, by segments rather than by polylines.
 * A polyline saves the overhead the surface sweep incurs per curve on all but
 * one segment of each x-monotone chain; however, computing the intersections
 * of two polylines is more expensive than computing those of two segments.
 * Thus, polylines are preferable when the cycles intersect sparsely or their
 * x-monotone chains are long, while segments are preferable when the cycles
 * fold over themselves densely. The density of the folding is measured by the
 * ratio between the number E of convolution edges and m + n (E = m + n for
 * convex polygons), and the length of the chains by the ratio between E and
 * the number C of x-monotone chains; segments are preferred if E > 4(m + n),
 * and in addition, either E > 20(m + n) (the folding is so dense that it
 * outweighs long chains, as typical of large convex-concave pairs), or
 * E < 40C and, unless the chains are short (E < 15C), E > 500 (for few
 * edges, the overhead per curve of the sweep is outweighed by long chains).
 * (The chains, whose counting requires a pass over the cycles, are counted
 * only if neither of the conditions on the density decides.)
 * The thresholds were determined empirically (with the Epeck kernel), by
 * comparing the running times of both representations on 1192 pairs of
 * random polygons with 15 to 400 vertices, of equal and of different sizes,
 * and varying ratios of reflex vertices; the rule lost 1% in total running
 * time, and 2% in the geometric mean of the running times, compared to an
 * a-posteriori choice of the faster representation for every pair.
 * \param paths the convolution cycles
 * \param num_vertices the total number of vertices of the two polygons (m + n);
 *        for polygons with holes, the sum of m + n over the pairs of convolved
 *        boundary cycles
 */
template <typename Point_2>
bool prefer_segments(const std::vector<std::vector<Point_2>>& paths, std::size_t num_vertices) {
  std::size_t num_edges = 0;
  for (const auto& path : paths)
    if (path.size() > 1) num_edges += path.size() - 1;
  if (num_edges <= 4 * num_vertices) return false;
  if (num_edges > 20 * num_vertices) return true;
  const std::size_t num_chains = number_of_x_monotone_chains(paths);
  return (num_edges < 40 * num_chains) && ((num_edges > 500) || (num_edges < 15 * num_chains));
}

/*! determines whether to represent convolution cycles by segments (rather
 * than by polylines), as requested, or as determined by `prefer_segments()`
 * if the representation is to be chosen automatically.
 */
template <typename Point_2>
bool use_segments(const std::vector<std::vector<Point_2>>& paths, std::size_t num_vertices,
                  Convolution_curves curves) {
  if (curves == Convolution_curves::AUTOMATIC) return prefer_segments(paths, num_vertices);
  return curves == Convolution_curves::SEGMENTS;
}

/*! computes the Minkowski sum of a bounded polygon with holes \f$ B\f$ and an
 * unbounded polygon with holes \f$ U\f$, that is, the plane minus the holes
 * \f$ H_1, \ldots, H_k\f$ of \f$ U\f$.  A point \f$ x\f$ is not in the sum
 * iff \f$ x - B\f$ is contained in the union of the holes; as \f$ B\f$ is
 * connected and the holes are simply connected and pairwise disjoint, this
 * holds iff \f$ x - \hat{B}\f$ is contained in one hole \f$ H_i\f$, where
 * \f$ \hat{B}\f$ is the region bounded by the outer boundary of \f$ B\f$.
 * Thus, the holes of \f$ B\f$ do not affect the sum, which is the complement
 * of the union of the Minkowski differences \f$ \hat{B} \ominus H_i\f$ (the
 * complements of \f$ \hat{B}\f$ plus the complement of \f$ H_i\f$; see
 * `minkowski_difference_by_full_convolution_2()`). Their union is computed
 * as in the difference: the convolution cycles of the outer boundary of
 * \f$ B\f$ and the (clockwise) holes of \f$ U\f$ are computed, without
 * removing translated copies, and the faces whose winding number is
 * \f$-(1 + r)\f$, where \f$ r\f$ is the number of vertices of the outer
 * boundary of \f$ B\f$ that are not convex, are not in the sum.  The sum is
 * unbounded and connected, and its holes, the differences, are simply
 * connected.  A hole of \f$ U\f$ that cannot contain a translated copy of
 * \f$ \hat{B}\f$, as determined by the hole filter, is removed first.
 * \param bounded the bounded polygon with holes \f$ B\f$
 * \param unbounded the unbounded polygon with holes \f$ U\f$
 * \param curves the representation of the convolution cycles
 * \param traits_ptr a shared pointer to the traits
 */
template <typename Kernel_, typename Container_, typename Traits_>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_with_unbounded_2(const Polygon_with_holes_2<Kernel_, Container_>& bounded,
                               const Polygon_with_holes_2<Kernel_, Container_>& unbounded,
                               Convolution_curves curves, const std::shared_ptr<const Traits_>& traits_ptr) {
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel_, Container_>;
  using Polyline_traits_2 = CGAL::Arr_polyline_traits_2<Traits_>;

  CGAL_precondition(! bounded.is_unbounded() && unbounded.is_unbounded());
  const auto& outer = bounded.outer_boundary();

  // Remove the holes of the unbounded polygon that cannot contain a translated
  // copy of the bounded one; if none remains, the sum is the entire plane.
  Hole_filter_2<Kernel_, Container_> hole_filter;
  Polygon_with_holes_2 filtered;
  hole_filter(unbounded, outer, filtered);
  if (filtered.number_of_holes() == 0) return Polygon_with_holes_2();

  // The number of vertices of the outer boundary that are not convex.
  // The vertices are traversed with circulators, as indexing a polygon whose
  // container is not random access takes linear time.
  const auto orientation = traits_ptr->orientation_2_object();
  const std::size_t n = outer.size();
  int num_non_convex = 0;
  auto prev = outer.vertices_circulator();
  auto curr = prev;
  auto next = curr;
  ++next;
  --prev;
  do {
    if (orientation(*prev, *curr, *next) != LEFT_TURN) ++num_non_convex;
    prev = curr;
    curr = next;
    ++next;
  } while (curr != outer.vertices_circulator());
  const int winding = -(1 + num_non_convex);

  std::vector<std::vector<typename Kernel_::Point_2>> paths;
  std::vector<std::vector<std::size_t>> ids;
  std::size_t num_vertices = 0;
  std::size_t id_offset = 0;
  for (auto hit = filtered.holes_begin(); hit != filtered.holes_end(); ++hit) {
    auto hole_paths = generate_convolution_paths(outer, *hit, traits_ptr, &ids, id_offset);
    paths.insert(paths.end(), std::make_move_iterator(hole_paths.begin()), std::make_move_iterator(hole_paths.end()));
    num_vertices += n + hit->size();
    id_offset += n * hit->size();
  }

  if (use_segments(paths, num_vertices, curves))
    return minkowski_sum_with_unbounded_2_impl<Traits_, Kernel_, Container_>(paths, ids, winding, traits_ptr);
  return minkowski_sum_with_unbounded_2_impl<Polyline_traits_2, Kernel_, Container_>(paths, ids, winding, traits_ptr);
}

} // namespace internal

/*! computes the Minkowski sum of two input polygons with holes.  The outer
 * boundaries and the holes are treated alike: the convolution cycles of every
 * boundary cycle of the first polygon and every boundary cycle of the second
 * polygon are computed, and the sum consists of the faces of the arrangement
 * of all these cycles whose winding numbers are positive.  First, the holes
 * that cannot contribute to the boundary of the sum are removed, as done by
 * the reduced convolution method; a hole is removed if the bounding box of the
 * outer boundary of the other polygon does not fit in its bounding box, as the
 * sum then covers the hole.  If both polygons are convex, the sum is computed
 * directly; see `minkowski_sum_of_convex_polygons_2()`.  A polygon with holes
 * may be unbounded (without an outer boundary); see
 * `minkowski_sum_with_unbounded_2()`.  The sum of two unbounded polygons is
 * the entire plane.
 * \param pgn1 the first summand polygon with holes
 * \param pgn2 the second summand polygon with holes
 * \param np1 the named parameters of the first polygon: `is_convex`
 * \param np2 the named parameters of the second polygon: `is_convex`
 * \param np the named parameters of the computation: `geom_traits` and
 *        `convolution_curves`
 * \pre The outer boundaries of `pgn1` and `pgn2` (if any) are oriented
 *      counterclockwise, and their holes are oriented clockwise.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_full_convolution_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                    const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                    const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                    const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                    const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Polygon_2 = CGAL::Polygon_2<Kernel_, Container_>;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel_, Container_>;

  // The boundary cycles of a polygon with holes.
  auto boundary_cycles = [](const Polygon_with_holes_2& pwh) {
    std::vector<const Polygon_2*> cycles{&pwh.outer_boundary()};
    for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) cycles.push_back(&*hit);
    return cycles;
  };
  CGAL_expensive_precondition_code(auto is_well_oriented = [](const Polygon_with_holes_2& pwh) {
    if (! pwh.is_unbounded() && (pwh.outer_boundary().orientation() != COUNTERCLOCKWISE)) return false;
    for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit)
      if (hit->orientation() != CLOCKWISE) return false;
    return true;
  };)
  CGAL_expensive_precondition(is_well_oriented(pgn1) && is_well_oriented(pgn2));

  const auto traits_ptr = internal::shared_traits<Kernel_, Container_>(np);
  using Traits = std::decay_t<decltype(*traits_ptr)>;
  using Polyline_traits_2 = CGAL::Arr_polyline_traits_2<Traits>;
  const auto& traits = *traits_ptr;
  const Convolution_curves curves = internal::convolution_curves_parameter(np);
  const auto hint1 = internal::convexity_hint(pgn1, np1);
  const auto hint2 = internal::convexity_hint(pgn2, np2);

  // The sum of two convex polygons is computed directly.
  if (internal::is_convex_polygon(pgn1, hint1) && internal::is_convex_polygon(pgn2, hint2))
    return internal::minkowski_sum_of_convex_polygons_2(pgn1.outer_boundary(), pgn2.outer_boundary(), traits);

  // The sum of two unbounded polygons is the entire plane.
  if (pgn1.is_unbounded() && pgn2.is_unbounded()) return Polygon_with_holes_2();
  if (pgn1.is_unbounded()) return internal::minkowski_sum_with_unbounded_2(pgn2, pgn1, curves, traits_ptr);
  if (pgn2.is_unbounded()) return internal::minkowski_sum_with_unbounded_2(pgn1, pgn2, curves, traits_ptr);

  // Remove the holes that cannot contribute to the boundary of the sum.
  Hole_filter_2<Kernel_, Container_> hole_filter;
  Polygon_with_holes_2 filtered_pgn1, filtered_pgn2;
  hole_filter(pgn1, pgn2, filtered_pgn1);
  hole_filter(pgn2, pgn1, filtered_pgn2);

  // The number of vertices passed to prefer_segments() is summed over the
  // pairs of cycles, so that it equals the number of convolution edges when
  // all the cycles are convex, as it does for two simple polygons.
  // The identifiers of the vertices of the cycles of different pairs are
  // distinct.
  std::vector<std::vector<typename Kernel_::Point_2>> paths;
  std::vector<std::vector<std::size_t>> ids;
  std::size_t num_vertices = 0;
  std::size_t id_offset = 0;
  // A (filtered) polygon is convex if it has no holes and its outer boundary
  // is convex.  The hint applies to the original polygon; thus, it is used
  // only if the original polygon has no holes (a polygon hinted not to be
  // convex is then not tested).
  auto is_convex = [](const Polygon_with_holes_2& pwh, const Polygon_with_holes_2& filtered,
                      std::optional<bool> hint) {
    if (filtered.number_of_holes() > 0) return false;
    return (hint && (pwh.number_of_holes() == 0)) ? *hint : filtered.outer_boundary().is_convex();
  };
  const bool is_convex1 = is_convex(pgn1, filtered_pgn1, hint1);
  const bool is_convex2 = is_convex(pgn2, filtered_pgn2, hint2);

  // If both polygons are convex once their holes are removed, the sum is
  // computed directly.
  if (is_convex1 && is_convex2)
    return internal::minkowski_sum_of_convex_polygons_2(filtered_pgn1.outer_boundary(), filtered_pgn2.outer_boundary(),
                                              traits);

  // The translated copies of the boundary cycles are removed if neither
  // (filtered) polygon has holes, as for simple polygons, or if one of them is
  // convex; otherwise, removing them may lose points of the sum. See
  // remove_translated_copies().
  const bool remove_copies = ((filtered_pgn1.number_of_holes() == 0) && (filtered_pgn2.number_of_holes() == 0)) ||
    is_convex1 || is_convex2;
  for (const auto* cycle1 : boundary_cycles(filtered_pgn1)) {
    for (const auto* cycle2 : boundary_cycles(filtered_pgn2)) {
      auto cycle_paths =
        internal::generate_convolution_paths(*cycle1, *cycle2, traits_ptr, &ids, id_offset, remove_copies);
      paths.insert(paths.end(), std::make_move_iterator(cycle_paths.begin()),
                   std::make_move_iterator(cycle_paths.end()));
      num_vertices += cycle1->size() + cycle2->size();
      id_offset += cycle1->size() * cycle2->size();
    }
  }

  if (internal::use_segments(paths, num_vertices, curves))
    return internal::minkowski_sum_by_full_convolution_2_impl<Traits, Kernel_, Container_>(paths, ids, traits_ptr);
  return internal::minkowski_sum_by_full_convolution_2_impl<Polyline_traits_2, Kernel_, Container_>(paths, ids,
                                                                                                    traits_ptr);
}

/*! computes the Minkowski sum of two input polygons; see the overload above.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_full_convolution_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                    const Polygon_2<Kernel_, Container_>& pgn2,
                                    const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                    const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                    const CGAL_NP_CLASS& np = parameters::default_values()) {
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel_, Container_>;
  return minkowski_sum_by_full_convolution_2(Polygon_with_holes_2(pgn1), Polygon_with_holes_2(pgn2), np1, np2, np);
}

/*! computes the Minkowski sum of an input polygon with holes and an input
 * polygon; see the overload above.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_full_convolution_2(const Polygon_with_holes_2<Kernel_, Container_>& pgn1,
                                    const Polygon_2<Kernel_, Container_>& pgn2,
                                    const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                    const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                    const CGAL_NP_CLASS& np = parameters::default_values())
{ return minkowski_sum_by_full_convolution_2(pgn1, Polygon_with_holes_2<Kernel_, Container_>(pgn2), np1, np2, np); }

/*! computes the Minkowski sum of an input polygon and an input polygon with
 * holes; see the overload above.
 */
template <typename Kernel_, typename Container_,
          typename CGAL_NP_TEMPLATE_PARAMETERS_1, typename CGAL_NP_TEMPLATE_PARAMETERS_2,
          typename CGAL_NP_TEMPLATE_PARAMETERS>
Polygon_with_holes_2<Kernel_, Container_>
minkowski_sum_by_full_convolution_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                    const Polygon_with_holes_2<Kernel_, Container_>& pgn2,
                                    const CGAL_NP_CLASS_1& np1 = parameters::default_values(),
                                    const CGAL_NP_CLASS_2& np2 = parameters::default_values(),
                                    const CGAL_NP_CLASS& np = parameters::default_values())
{ return minkowski_sum_by_full_convolution_2(Polygon_with_holes_2<Kernel_, Container_>(pgn1), pgn2, np1, np2, np); }

}
}

#endif
