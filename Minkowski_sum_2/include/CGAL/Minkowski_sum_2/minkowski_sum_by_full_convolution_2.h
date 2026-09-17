// Copyright (c) 2026  Tel-Aviv University (Israel).
// All rights reserved.
// This file is part of CGAL (www.cgal.org).
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
// Author(s) : Efi Fogel  <efifogel@gmail.com>

#ifndef CGAL_MINKOWSKI_SUM_BY_FULL_CONVOLUTION_H
#define CGAL_MINKOWSKI_SUM_BY_FULL_CONVOLUTION_H

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
    auto a = std::hash<std::size_t>()(e.m_k1);
    auto b = std::hash<std::size_t>()(e.m_k2);
    auto c = std::hash<int>()(static_cast<int>(e.m_result));
    return a ^ (b << 1) ^ (c << 2);
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

/*! checks which directions have overlapping normal cones
 * prev_dir1, curr_dir1: incoming and outgoing at k1
 * prev_dir2, curr_dir2: incoming and outgoing at k2
 */
template <typename Kernel_>
Cone_overlap_result cones_overlap_which(const typename Kernel_::Direction_2& prev_dir1,
                                        const typename Kernel_::Direction_2& curr_dir1,
                                        const typename Kernel_::Direction_2& prev_dir2,
                                        const typename Kernel_::Direction_2& curr_dir2,
                                        const Kernel_& kernel) {
  const auto equal = kernel.equal_2_object();
  const auto counterclockwise_in_between = kernel.counterclockwise_in_between_2_object();

  // 1. If outgoing directions identical
  if (equal(curr_dir1, curr_dir2)) return Cone_overlap_result::Both;

  // 2. If incoming directions identical
  if (equal(prev_dir1, prev_dir2)) {
    return (counterclockwise_in_between(curr_dir1, prev_dir2, curr_dir2)) ?
      Cone_overlap_result::First : Cone_overlap_result::Second;
  }

  // 3. Otherwise, check cone containment
  bool res1 = counterclockwise_in_between(curr_dir1, prev_dir2, curr_dir2);
  bool res2 = counterclockwise_in_between(curr_dir2, prev_dir1, curr_dir1);

  if (! res1 && ! res2) return Cone_overlap_result::None;
  if (res1 && ! res2) return Cone_overlap_result::First;
  if (! res1 && res2) return Cone_overlap_result::Second;
  return Cone_overlap_result::Either;
}

/*! finds all pairs (k1, k2) whose normal cones have interior overlap Find all
 * convolution edges, creating separate edges for disconnected overlap regions
 */
template <typename Kernel_>
std::unordered_set<Convolution_edge, Convolution_edge_hash>
find_all_convolution_edges(const std::vector<typename Kernel_::Direction_2>& dirs1,
                           const std::vector<typename Kernel_::Direction_2>& dirs2,
                          const Kernel_& kernel) {
  using Kernel = Kernel_;
  using Direction_2 = typename Kernel::Direction_2;

  std::unordered_set<Convolution_edge, Convolution_edge_hash> edges;
  std::size_t n1 = dirs1.size();
  std::size_t n2 = dirs2.size();

  for (std::size_t k1 = 0; k1 < n1; ++k1) {
    for (std::size_t k2 = 0; k2 < n2; ++k2) {
      std::size_t prev_k1 = (k1 - 1 + n1) % n1;
      std::size_t prev_k2 = (k2 - 1 + n2) % n2;

      Direction_2 prev_dir1 = dirs1[prev_k1];
      Direction_2 curr_dir1 = dirs1[k1];
      Direction_2 prev_dir2 = dirs2[prev_k2];
      Direction_2 curr_dir2 = dirs2[k2];

      // Check which advance directions are valid from this pair
      Cone_overlap_result result = cones_overlap_which(prev_dir1, curr_dir1, prev_dir2, curr_dir2, kernel);

      if (result == Cone_overlap_result::None) continue;

      if (result == Cone_overlap_result::Either) {
        edges.insert(Convolution_edge(k1, k2, Cone_overlap_result::First));
        edges.insert(Convolution_edge(k1, k2, Cone_overlap_result::Second));
        continue;
      }

      edges.insert(Convolution_edge(k1, k2, result));
    }
  }

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
  for (auto& xcv : xcvs) CGAL::insert(arr, xcv);
  compute_winding_numbers(arr);
  Polygon_2 outer = extract_outer_boundary<Kernel, Container>(arr);
  auto holes = extract_holes<Kernel, Container>(arr);
  return Polygon_with_holes_2(std::move(outer), std::make_move_iterator(holes.begin()),
                              std::make_move_iterator(holes.end()));
}

}
}

#endif
