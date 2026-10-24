// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_BY_FULL_CONVOLUTION_H
#define CGAL_MINKOWSKI_SUM_BY_FULL_CONVOLUTION_H

#include <algorithm>
#include <cstdint>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <vector>
#include <unordered_set>

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_curve_data_traits_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Arrangement_on_curve_1/Circle_traits_1.h>
#include <CGAL/Arrangement_on_curve_1/Identified_topology_traits.h>
#include <CGAL/Arrangement_on_curve_1/Arrangement_on_curve_1.h>
#include <CGAL/Arrangement_on_curve_1/insert.h>
#include <CGAL/Arrangement_on_curve_1/overlay.h>
// #include <CGAL/draw_polygon_2.h>
// #include <CGAL/draw_polygon_with_holes_2.h>
// #include <CGAL/draw_arrangement_2.h>

namespace CGAL {
namespace Minkowski_sum_2 {

// ============================================================================
// FACE DATA FOR WINDING NUMBERS
// ============================================================================

struct Face_data {
  int m_winding_number;
  bool m_visited;

  Face_data() : m_winding_number(-1), m_visited(false) {}
};

// ============================================================================
// CONVOLUTION EDGE DATA STRUCTURES
// ============================================================================

// Result of cone overlap check: which directions can advance from a pair
enum class Cone_overlap_result {
  None,         // cones don't overlap
  First,        // only advancing k1 is valid
  Second,       // only advancing k2 is valid
  Both,		// Both valid in one sub-cone (identical outgoing directions)
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

/*! extract directions.
 */
template <typename Kernel_, typename Container_>
std::vector<typename Kernel_::Direction_2> extract_directions(const Polygon_2<Kernel_, Container_>& pgn,
                                                              const Kernel_& kernel) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Direction_2 = typename Kernel::Direction_2;

  const auto construct_vector = kernel.construct_vector_2_object();
  const auto construct_direction = kernel.construct_direction_2_object();

  std::vector<Direction_2> dirs;
  std::size_t n = pgn.size();
  for (std::size_t i = 0; i < n; ++i) {
    auto vec = construct_vector(pgn[i], pgn[(i + 1) % n]);
    dirs.push_back(construct_direction(vec));
  }
  return dirs;
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

/*! inserts a direction into an arrangement on the circle, given the edge that
 * contains it in its interior.
 */
template <typename Arrangement>
typename Arrangement::Vertex_descriptor
insert_in_edge(Arrangement& arr, typename Arrangement::Edge_descriptor e,
               const typename Arrangement::Point_1& d) {
  const auto& traits = arr.geometry_traits_1();
  if (traits.is_on_x_identification_1_object()(d)) return arr.insert_identification_vertex(d);

  // Without the identification vertex, left_boundary_edge() wraps around the
  // identification point; if d lies in it to the right of its left vertex,
  // left_boundary_edge() must keep the part to the right of d.
  if (! arr.topology_traits().has_identification_vertex() && (e == arr.left_boundary_edge())) {
    auto v_n = arr.left_vertex(e);
    if (traits.compare_x_1_object()(d, get(arr.vertex_point_map(), v_n)) == LARGER) return arr.insert_after(v_n, d);
  }
  return arr.split_edge(e, d);
}

/*! inserts the directions of the edges of a polygon, in the order of
 * traversal, into an empty arrangement on the circle, and stores with every
 * vertex the indices of the polygon edges that have its direction.  The vertex
 * of the next direction is searched starting at the vertex of the current one,
 * in the direction of the turn.  The number of steps is linear in the number
 * of edges when the polygon is convex, but it is proportional to the total
 * absolute turning of the polygon in general.
 * \return `false` if the search takes more than `max_steps` steps, in which
 *         case the arrangement is left partially constructed.
 */
template <typename Arrangement, typename Kernel_>
bool insert_directions_by_traversal(Arrangement& arr, const std::vector<typename Kernel_::Direction_2>& dirs,
                                    const Kernel_& kernel, std::size_t max_steps) {
  CGAL_precondition(arr.empty());
  CGAL_precondition(! dirs.empty());

  const auto equal = kernel.equal_2_object();
  const auto orientation = kernel.orientation_2_object();
  const auto counterclockwise_in_between = kernel.counterclockwise_in_between_2_object();
  const auto pnt_map = arr.vertex_point_map();
  const auto data_map = arr.vertex_data_map();
  std::size_t steps = 0;

  // Locates (and inserts if necessary) d walking counterclockwise from u.
  auto locate_ccw = [&](auto u, const auto& d) -> std::optional<decltype(u)> {
    while (++steps <= max_steps) {
      auto e = arr.right_edge(u);
      auto w = arr.right_vertex(e);
      if (equal(get(pnt_map, w), d)) return w;
      if (counterclockwise_in_between(d, get(pnt_map, u), get(pnt_map, w))) return insert_in_edge(arr, e, d);
      u = w;
    }
    return std::nullopt;
  };

  // Locates (and inserts if necessary) d walking clockwise from u.
  auto locate_cw = [&](auto u, const auto& d) -> std::optional<decltype(u)> {
    while (++steps <= max_steps) {
      auto e = arr.left_edge(u);
      auto w = arr.left_vertex(e);
      if (equal(get(pnt_map, w), d)) return w;
      if (counterclockwise_in_between(d, get(pnt_map, w), get(pnt_map, u))) return insert_in_edge(arr, e, d);
      u = w;
    }
    return std::nullopt;
  };

  auto v = CGAL::Arrangement_on_curve_1::insert(arr, dirs[0]);
  get(data_map, v).push_back(0);
  for (std::size_t k = 1; k < dirs.size(); ++k) {
    const auto& prev_dir = dirs[k - 1];
    const auto& curr_dir = dirs[k];
    if (! equal(prev_dir, curr_dir)) {
      auto res = (orientation(prev_dir.vector(), curr_dir.vector()) == RIGHT_TURN) ?
        locate_cw(v, curr_dir) : locate_ccw(v, curr_dir);
      if (! res) return false;
      v = *res;
    }
    get(data_map, v).push_back(k);
  }
  return true;
}

/*! inserts the directions of the edges of a polygon, in increasing order,
 * into an empty arrangement on the circle, and stores with every vertex the
 * indices of the polygon edges that have its direction.  It takes
 * \f$O(n \log n)\f$ time, where \f$n\f$ is the number of edges.
 */
template <typename Arrangement, typename Kernel_>
void insert_directions_by_sorting(Arrangement& arr, const std::vector<typename Kernel_::Direction_2>& dirs,
                                  const Kernel_& kernel) {
  CGAL_precondition(arr.empty());
  CGAL_precondition(! dirs.empty());

  const auto& traits = arr.geometry_traits_1();
  const auto is_on_x_identification = traits.is_on_x_identification_1_object();
  const auto compare_x = traits.compare_x_1_object();
  const auto equal = kernel.equal_2_object();
  const auto pnt_map = arr.vertex_point_map();
  const auto data_map = arr.vertex_data_map();

  // The identification point precedes all other directions.
  std::vector<std::size_t> indices(dirs.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(), [&](std::size_t i, std::size_t j) {
    if (is_on_x_identification(dirs[j])) return false;
    if (is_on_x_identification(dirs[i])) return true;
    return compare_x(dirs[i], dirs[j]) == SMALLER;
  });

  // Every direction is larger than all directions inserted so far, so it lies
  // in the edge to the right of the last vertex inserted.
  auto v = CGAL::Arrangement_on_curve_1::insert(arr, dirs[indices[0]]);
  get(data_map, v).push_back(indices[0]);
  for (std::size_t i = 1; i < indices.size(); ++i) {
    const auto& d = dirs[indices[i]];
    if (! equal(get(pnt_map, v), d)) v = insert_in_edge(arr, arr.right_edge(v), d);
    get(data_map, v).push_back(indices[i]);
  }
}

/*! inserts the directions of the edges of a polygon into an empty arrangement
 * on the circle, and stores with every vertex the indices of the polygon edges
 * that have its direction.  The directions are inserted in the order of
 * traversal, which takes linear time when the polygon is convex.  If the
 * traversal takes too many steps (because the polygon turns around many
 * times), the directions are inserted in sorted order instead, which takes
 * \f$O(n \log n)\f$ time.
 */
template <typename Arrangement, typename Kernel_>
void insert_directions(Arrangement& arr, const std::vector<typename Kernel_::Direction_2>& dirs,
                       const Kernel_& kernel) {
  CGAL_precondition(arr.empty());
  if (dirs.empty()) return;

  // A traversal of a convex polygon takes at most n steps.  The budget below
  // bounds the total time by the time it takes to sort, up to a constant.
  const std::size_t n = dirs.size();
  std::size_t log_n = 1;
  while ((std::size_t(1) << log_n) < n) ++log_n;
  if (insert_directions_by_traversal(arr, dirs, kernel, 2 * n * log_n)) return;
  arr.clear();
  insert_directions_by_sorting(arr, dirs, kernel);
}

/*! the data of a vertex of the overlay of the two arrangements of directions:
 * the indices of the edges of each polygon that have its direction, or null if
 * there are none.
 */
struct Overlay_vertex_data {
  const std::vector<std::size_t>* m_edges1 = nullptr;
  const std::vector<std::size_t>* m_edges2 = nullptr;
};

/*! an overlay visitor that records the edge indices of the input vertices.
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
  { put(m_arr_r.vertex_data_map(), v_res, Overlay_vertex_data{&edges_a(v_a), &edges_b(v_b)}); }

  void create_vertex_ve(Vertex_const_descriptor_a v_a, Edge_const_descriptor_b, Vertex_descriptor_r v_res)
  { put(m_arr_r.vertex_data_map(), v_res, Overlay_vertex_data{&edges_a(v_a), nullptr}); }

  void create_vertex_ev(Edge_const_descriptor_a, Vertex_const_descriptor_b v_b, Vertex_descriptor_r v_res)
  { put(m_arr_r.vertex_data_map(), v_res, Overlay_vertex_data{nullptr, &edges_b(v_b)}); }

  void create_edge_ee(Edge_const_descriptor_a, Edge_const_descriptor_b, Edge_descriptor_r) {}

private:
  const std::vector<std::size_t>& edges_a(Vertex_const_descriptor_a v) const
  { return get(m_arr_a.vertex_data_map(), v); }

  const std::vector<std::size_t>& edges_b(Vertex_const_descriptor_b v) const
  { return get(m_arr_b.vertex_data_map(), v); }

  const ArrangementA& m_arr_a;
  const ArrangementB& m_arr_b;
  ArrangementR& m_arr_r;
};

/*! finds all convolution edges, creating separate edges for disconnected
 * overlap regions.
 *
 * The directions of the edges of each polygon are inserted into an
 * arrangement on the circle of directions (see `insert_directions()`).  The two
 * arrangements are overlaid, and the vertices of the overlay are swept
 * counterclockwise, starting at the identification point, while the cones of
 * each polygon that contain the current direction are maintained.  The sweep
 * takes \f$O(m + n + k)\f$ time, where \f$m\f$ and \f$n\f$ are the numbers of
 * edges of the polygons and \f$k\f$ is the number of convolution edges.
 * Constructing the arrangement of a polygon takes linear time if the polygon
 * is convex and \f$O(n \log n)\f$ time in general.
 */
template <typename Kernel_>
std::unordered_set<Convolution_edge, Convolution_edge_hash>
find_all_convolution_edges(const std::vector<typename Kernel_::Direction_2>& dirs1,
                           const std::vector<typename Kernel_::Direction_2>& dirs2,
                           const Kernel_& kernel) {
  using Kernel = Kernel_;
  using Direction_2 = typename Kernel::Direction_2;
  using Edge_indices = std::vector<std::size_t>;

  namespace Aoc = CGAL::Arrangement_on_curve_1;
  using Circle_traits = Aoc::Circle_traits_1<Kernel>;
  using Topology_traits = Aoc::Identified_topology_traits<Circle_traits, Edge_indices>;
  using Arrangement = Aoc::Arrangement_on_curve_1<Circle_traits, Topology_traits>;
  using Overlay_topology_traits = Aoc::Identified_topology_traits<Circle_traits, Overlay_vertex_data>;
  using Overlay_arrangement = Aoc::Arrangement_on_curve_1<Circle_traits, Overlay_topology_traits>;

  std::unordered_set<Convolution_edge, Convolution_edge_hash> edges;
  const std::size_t n1 = dirs1.size();
  const std::size_t n2 = dirs2.size();
  if ((n1 == 0) || (n2 == 0)) return edges;

  auto traits = std::make_shared<const Circle_traits>();
  Arrangement arr1(traits);
  Arrangement arr2(traits);
  insert_directions(arr1, dirs1, kernel);
  insert_directions(arr2, dirs2, kernel);

  Overlay_arrangement arr(traits);
  Direction_overlay_visitor<Arrangement, Arrangement, Overlay_arrangement> visitor(arr1, arr2, arr);
  Aoc::overlay(arr1, arr2, arr, visitor);

  // The cones that contain the directions immediately after the
  // identification point.
  const auto equal = kernel.equal_2_object();
  const auto counterclockwise_in_between = kernel.counterclockwise_in_between_2_object();
  const Direction_2& ref = traits->reference_direction();
  auto initial_cones = [&](const std::vector<Direction_2>& dirs) {
    const std::size_t n = dirs.size();
    std::vector<std::size_t> cones;
    for (std::size_t k = 0; k < n; ++k) {
      const auto& prev_dir = dirs[(k + n - 1) % n];
      if (equal(prev_dir, ref) || counterclockwise_in_between(ref, prev_dir, dirs[k])) cones.push_back(k);
    }
    return cones;
  };
  const auto initial_cones1 = initial_cones(dirs1);
  const auto initial_cones2 = initial_cones(dirs2);

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
      const Edge_indices* edges1 = data.m_edges1;
      const Edge_indices* edges2 = data.m_edges2;

      if (edges1) for (auto k1 : *edges1) cones1.erase(k1);
      if (edges2) for (auto k2 : *edges2) cones2.erase(k2);

      if (count_only) {
        const std::size_t m1 = edges1 ? edges1->size() : 0;
        const std::size_t m2 = edges2 ? edges2->size() : 0;
        count += m1 * cones2.size() + m1 * m2 + m2 * cones1.size();
      }
      else {
        if (edges1) {
          for (auto k1 : *edges1) {
            cones2.for_each([&](std::size_t k2) { edges.insert(Convolution_edge(k1, k2, Cone_overlap_result::First)); });
            if (edges2) {
              for (auto k2 : *edges2) edges.insert(Convolution_edge(k1, k2, Cone_overlap_result::Both));
            }
          }
        }
        if (edges2) {
          for (auto k2 : *edges2)
            cones1.for_each([&](std::size_t k1) { edges.insert(Convolution_edge(k1, k2, Cone_overlap_result::Second)); });
        }
      }

      // The cones that start at the last vertex contain the directions
      // immediately after the identification point; they are already in.
      if (e == lb) break;
      if (edges1) for (auto k1 : *edges1) cones1.insert((k1 + 1) % n1);
      if (edges2) for (auto k2 : *edges2) cones2.insert((k2 + 1) % n2);
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
 */
template <typename Kernel_, typename Container_>
std::vector<typename Kernel_::Point_2>
generate_single_path(const Polygon_2<Kernel_, Container_>& pgn1, const Polygon_2<Kernel_, Container_>& pgn2,
                     const std::vector<typename Kernel_::Direction_2>& dirs1,
                     const std::vector<typename Kernel_::Direction_2>& dirs2,
                     const Convolution_edge& start_edge,
                     std::unordered_set<Convolution_edge, Convolution_edge_hash>& remaining_edges,
                     const Kernel_& kernel) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Point_2 = typename Kernel::Point_2;
  using Direction_2 = typename Kernel::Direction_2;
  using Convolution_path = std::vector<Point_2>;

  const auto construct_vector = kernel.construct_vector_2_object();
 const auto construct_translated_point_2 = kernel.construct_translated_point_2_object();

  auto start_k1 = start_edge.m_k1;
  auto start_k2 = start_edge.m_k2;
  auto start_result = start_edge.m_result;

  std::size_t n1 = dirs1.size();
  std::size_t n2 = dirs2.size();
  Convolution_path conv_path;

  std::size_t iteration = 0;
  const std::size_t max_iterations = n1 * n2;

  std::size_t k1 = start_k1;
  std::size_t k2 = start_k2;
  Point_2 start_pt = construct_translated_point_2(pgn1[k1], construct_vector(Point_2(0, 0), pgn2[k2]));
  conv_path.push_back(start_pt);

  // std::cout << start_edge << std::endl;
  remaining_edges.erase(start_edge);

  Direction_2 curr_dir1 = dirs1[k1];
  Direction_2 curr_dir2 = dirs2[k2];
  Direction_2 curr_dir;
  std::size_t k1_next = (k1 + 1) % n1;
  std::size_t k2_next = (k2 + 1) % n2;

  // Initial step: use start_result to determine direction
  if (start_result == Cone_overlap_result::Both) {
    Point_2 next_pt = construct_translated_point_2(pgn1[k1_next], construct_vector(Point_2(0, 0), pgn2[k2_next]));
    conv_path.push_back(next_pt);
    k1 = k1_next;
    k2 = k2_next;
    curr_dir = curr_dir1;
  }
  else if (start_result == Cone_overlap_result::First) {
    Point_2 next_pt = construct_translated_point_2(pgn1[k1_next], construct_vector(Point_2(0, 0), pgn2[k2]));
    conv_path.push_back(next_pt);
    curr_dir = curr_dir1;
    k1 = k1_next;
  }
  else {
    CGAL_assertion(start_result == Cone_overlap_result::Second);
    Point_2 next_pt = construct_translated_point_2(pgn1[k1], construct_vector(Point_2(0, 0), pgn2[k2_next]));
    conv_path.push_back(next_pt);
    curr_dir = curr_dir2;
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

    ++iteration;
    if (iteration > max_iterations) {
      std::cerr << "ERROR: Iteration limit exceeded in convolution path generation" << std::endl;
      break;
    }

    std::size_t k1_next = (k1 + 1) % n1;
    std::size_t k2_next = (k2 + 1) % n2;
    Direction_2 curr_dir1 = dirs1[k1];
    Direction_2 curr_dir2 = dirs2[k2];

    if (found_edge.m_result == Cone_overlap_result::First) {
      // Advance k1
      Point_2 next_pt = construct_translated_point_2(pgn1[k1_next], construct_vector(Point_2(0, 0), pgn2[k2]));
      conv_path.push_back(next_pt);
      curr_dir = curr_dir1;
      k1 = k1_next;
      continue;
    }

    if (found_edge.m_result == Cone_overlap_result::Second) {
      // Advance k2
      Point_2 next_pt = construct_translated_point_2(pgn1[k1], construct_vector(Point_2(0, 0), pgn2[k2_next]));
      conv_path.push_back(next_pt);
      curr_dir = curr_dir2;
      k2 = k2_next;
      continue;
    }

    CGAL_assertion(found_edge.m_result == Cone_overlap_result::Both);
    // Both directions identical - advance both
    Point_2 next_pt = construct_translated_point_2(pgn1[k1_next], construct_vector(Point_2(0, 0), pgn2[k2_next]));
    conv_path.push_back(next_pt);
    k1 = k1_next;
    k2 = k2_next;
    curr_dir = curr_dir1;
  }

  return conv_path;
}

/*! generates all convolution paths for two polygon boundaries.
 */
template <typename Kernel_, typename Container_>
std::vector<std::vector<typename Kernel_::Point_2>>
generate_convolution_paths(const Polygon_2<Kernel_, Container_>& pgn1, const Polygon_2<Kernel_, Container_>& pgn2,
                           const Kernel_& kernel) {
  using Kernel = Kernel_;
  using Container = Container_;
  using Point_2 = typename Kernel::Point_2;
  using Direction_2 = typename Kernel::Direction_2;
  using Convolution_path = std::vector<Point_2>;

  std::vector<Convolution_path> paths;
  std::vector<Direction_2> dirs1 = extract_directions(pgn1, kernel);
  std::vector<Direction_2> dirs2 = extract_directions(pgn2, kernel);

  auto remaining_edges = find_all_convolution_edges(dirs1, dirs2, kernel);

  // Process all edges
  while (! remaining_edges.empty()) {
    Convolution_edge start_edge = *remaining_edges.begin();
    paths.push_back(generate_single_path(pgn1, pgn2, dirs1, dirs2, start_edge, remaining_edges, kernel));
  }
  // std::cout << "Generated " << paths.size() << " paths" << std::endl;
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
// RESULT EXTRACTION
// ============================================================================

template <typename Kernel_, typename Container_, typename ArrangementWithWinding_2>
Polygon_2<Kernel_, Container_> extract_outer_boundary(const ArrangementWithWinding_2& arr) {
  using Kernel = Kernel_;
  using Point_2 = typename Kernel::Point_2;
  using Polygon_2 = CGAL::Polygon_2<Kernel>;

  auto unbounded_face = arr.unbounded_face();
  if (unbounded_face->number_of_inner_ccbs() == 0) return Polygon_2();

  std::vector<Point_2> vertices;
  auto ccb = *unbounded_face->inner_ccbs_begin();
  auto start = ccb;
  do {
    const auto& curve = ccb->curve();
    int curve_data = curve.data();

    // Check if halfedge direction aligns with curve data direction
    bool is_forward = (ccb->direction() == CGAL::ARR_LEFT_TO_RIGHT && curve_data < 0) ||
                      (ccb->direction() == CGAL::ARR_RIGHT_TO_LEFT && curve_data > 0);

    if (is_forward) {
      // Forward direction: add points except last (to avoid duplication with next halfedge)
      auto points_end = std::prev(curve.points_end());
      for (auto pit = curve.points_begin(); pit != points_end; ++pit) vertices.push_back(*pit);
    }
    else {
      // Backward direction: add points in reverse except last (to avoid duplication)
      auto points_begin = curve.points_begin();
      for (auto pit = std::prev(curve.points_end()); pit != points_begin; --pit) vertices.push_back(*pit);
    }
  } while (--ccb != start);
  return Polygon_2(vertices.begin(), vertices.end());
}

/*! extracts holes.
 */
template <typename Kernel_, typename Container_, typename ArrangementWithWinding_2>
std::vector<Polygon_2<Kernel_, Container_>> extract_holes(const ArrangementWithWinding_2& arr) {
  using Kernel = Kernel_;
  using Point_2 = typename Kernel::Point_2;
  using Polygon_2 = CGAL::Polygon_2<Kernel>;

  std::vector<Polygon_2> holes;

  for (auto face = arr.faces_begin(); face != arr.faces_end(); ++face) {
    if (face->is_unbounded() || face->data().m_winding_number != 0) continue;

    std::vector<Point_2> vertices;
    auto ccb = face->outer_ccb();
    auto start = ccb;
    do {
      const auto& curve = ccb->curve();
      int curve_data = curve.data();

      // Check if halfedge direction aligns with curve data direction
      // Inverted since we traverse outer_ccb in reverse
      bool is_forward = (ccb->direction() == CGAL::ARR_LEFT_TO_RIGHT && curve_data > 0) ||
                        (ccb->direction() == CGAL::ARR_RIGHT_TO_LEFT && curve_data < 0);
      if (is_forward) {
        // Forward direction: add points except last (to avoid duplication with next halfedge)
        auto points_end = std::prev(curve.points_end());
        for (auto pit = curve.points_begin(); pit != points_end; ++pit) vertices.push_back(*pit);
      }
      else {
        // Backward direction: add points in reverse except last (to avoid duplication)
        auto points_begin = curve.points_begin();
        for (auto pit = std::prev(curve.points_end()); pit != points_begin; --pit) vertices.push_back(*pit);
      }
    } while (--ccb != start);   // traverse outer_ccb in reverse direction

    holes.push_back(Polygon_2(vertices.begin(), vertices.end()));
  }

  return holes;
}

// ============================================================================
// CURVE DATA TRAITS
// ============================================================================

struct Merge {
  int operator()(const int d1, const int& d2) const { return d1 + d2; }
};

struct Convert {
  int operator()(const bool& d) const { return 0; }
};

  /*! computes the Minkowski sum of two input polygons.
 * \param pgn1 the first summand polygon
 * \param pgn2 the second summand polygon
 */
template <typename Kernel_, typename Container_>
Polygon_with_holes_2<Kernel_, Container_>
new_minkowski_sum_by_full_convolution_2(const Polygon_2<Kernel_, Container_>& pgn1,
                                        const Polygon_2<Kernel_, Container_>& pgn2) {
  using Kernel = Kernel_;
  using Container = Container_;

  using Segment_traits_2 = CGAL::Arr_segment_traits_2<Kernel>;
  using Polyline_traits_2 = CGAL::Arr_polyline_traits_2<Segment_traits_2>;
  using Polyline_data_traits_2 = CGAL::Arr_curve_data_traits_2<Polyline_traits_2, int, Merge, int, Convert>;
  using X_monotone_curve_2 = typename Polyline_data_traits_2::X_monotone_curve_2;
  using Curve_2 = typename Polyline_data_traits_2::Curve_2;
  using Point_2 = typename Polyline_data_traits_2::Point_2;
  using Polygon_2 = CGAL::Polygon_2<Kernel>;
  using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<Kernel>;
  using Polyline_curve_2 = typename Polyline_traits_2::Curve_2;

  Kernel kernel;

  auto paths = generate_convolution_paths(pgn1, pgn2, kernel);
  std::vector<Curve_2> convolution_curves;
  for (std::size_t i = 0; i < paths.size(); ++i)
    convolution_curves.emplace_back(Polyline_curve_2(paths[i].begin(), paths[i].end()), 0);

  std::vector<X_monotone_curve_2> xcvs;
  std::vector<Point_2> points;
  Polyline_data_traits_2 traits;
  make_x_monotone(convolution_curves.begin(), convolution_curves.end(), std::back_inserter(xcvs),
                  std::back_inserter(points), traits);

  for (auto& xcv : xcvs) {
    auto endpoint_order = traits.compare_endpoints_xy_2_object()(xcv);
    int data = (endpoint_order == CGAL::SMALLER) ? 1 : -1;
    xcv.set_data(data);
  }

  using Segment_traits_2 = CGAL::Arr_segment_traits_2<Kernel>;
  using Polyline_traits_2 = CGAL::Arr_polyline_traits_2<Segment_traits_2>;
  using Polyline_data_traits_2 = CGAL::Arr_curve_data_traits_2<Polyline_traits_2, int, Merge, int, Convert>;
  using Dcel_2_with_winding = CGAL::Arr_face_extended_dcel<Polyline_data_traits_2, Face_data>;
  using Arrangement_with_winding_2 = CGAL::Arrangement_2<Polyline_data_traits_2, Dcel_2_with_winding>;
  Arrangement_with_winding_2 arr;
  // Aggregate insertion (a single sweep) is much faster than inserting the
  // curves one at a time, as each incremental insertion traverses a zone.
  CGAL::insert(arr, xcvs.begin(), xcvs.end());
  compute_winding_numbers(arr);
  Polygon_2 outer = extract_outer_boundary<Kernel, Container>(arr);
  auto holes = extract_holes<Kernel, Container>(arr);
  return Polygon_with_holes_2(std::move(outer), std::make_move_iterator(holes.begin()),
                              std::make_move_iterator(holes.end()));
}

}
}

#endif
