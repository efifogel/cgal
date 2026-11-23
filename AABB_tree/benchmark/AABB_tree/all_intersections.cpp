// Benchmark of line queries against a triangle mesh:
//  - all_intersections: AABB_tree::all_intersections() of a line and the faces,
//  - line_mesh: the intersection of a line and the volume bounded by a closed,
//    outward oriented, triangle mesh, computed by several variants.
//
// Run with --help for the command line options.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <boost/program_options.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Interval_nt.h>
#include <CGAL/Exact_rational.h>
#include <CGAL/Cartesian_converter.h>
#include <CGAL/Filtered_predicate.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/Side_of_triangle_mesh.h>
#include <CGAL/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/bbox.h>
#include <CGAL/Intersections_3/Bbox_3_Segment_3.h>
#include <CGAL/Random.h>
#include <CGAL/Real_timer.h>
#include <CGAL/assertions.h>

namespace PMP = CGAL::Polygon_mesh_processing;

using Epick = CGAL::Exact_predicates_inexact_constructions_kernel;
using Epeck = CGAL::Exact_predicates_exact_constructions_kernel;

using Mesh = CGAL::Surface_mesh<Epick::Point_3>;
using Vertex_index = Mesh::Vertex_index;
using Halfedge_index = Mesh::Halfedge_index;
using Face_index = Mesh::Face_index;

template <typename K> using Point_map = Mesh::Property_map<Vertex_index, typename K::Point_3>;

template <typename K, typename CacheDatum = CGAL::Tag_false>
using Tree = CGAL::AABB_tree<CGAL::AABB_traits_3<K, CGAL::AABB_face_graph_triangle_primitive<Mesh, Point_map<K>,
                                                                                            CGAL::Tag_true,
                                                                                            CacheDatum>>>;

template <typename K> using Output = std::variant<typename K::Point_3, typename K::Segment_3>;

// A line query given by two distinct points.
using Query = std::pair<Epick::Point_3, Epick::Point_3>;

//////////////////////////////////////////////////////////////////////////////
// The original procedure (the baseline)
//////////////////////////////////////////////////////////////////////////////

/*! determines whether the line through `a` and `b`, translated by an
 * infinitesimal generic vector, hits the triangle `(p0, p1, p2)`.
 *
 * The line meets the closed triangle iff the non-zero signs of
 * orientation(a, b, p_j, p_j+1) agree. The translation by (e, e^2, e^3), where
 * e is infinitesimal, replaces a zero sign by the sign of the first non-zero
 * coordinate of (b - a) x (p_j+1 - p_j). A triangle coplanar with the line is
 * never hit.
 */
template <typename Kernel>
bool perturbed_line_hits_triangle(const typename Kernel::Point_3& a, const typename Kernel::Point_3& b,
                                  const std::array<typename Kernel::Point_3, 3>& p, const Kernel& /* kernel */) {
  CGAL::Sign s[3];
  for (int j = 0; j < 3; ++j) s[j] = CGAL::orientation(a, b, p[j], p[(j + 1) % 3]);
  if ((s[0] == CGAL::ZERO) && (s[1] == CGAL::ZERO) && (s[2] == CGAL::ZERO)) return false;
  for (int j = 0; j < 3; ++j) {
    if (s[j] != CGAL::ZERO) continue;
    auto c = CGAL::cross_product(b - a, p[(j + 1) % 3] - p[j]);
    s[j] = CGAL::sign(c.x());
    if (s[j] == CGAL::ZERO) s[j] = CGAL::sign(c.y());
    if (s[j] == CGAL::ZERO) s[j] = CGAL::sign(c.z());
  }
  return (s[0] != CGAL::ZERO) && (s[0] == s[1]) && (s[1] == s[2]);
}

/*! An event along the line: a point intersection of the line and a facet, or
 * an endpoint of a segment intersection (the line lies in the plane of the
 * facet and meets it in a segment).
 */
template <typename Kernel>
struct Event {
  typename Kernel::Point_3 point;
  int delta;       // +1 = the first endpoint of a segment intersection along the line,
                   // -1 = the last one, 0 = a point intersection
  bool perturbed;  // a point intersection: does the translated line hit the facet?
};

/*! adds the event(s) of the intersection `x` of the line through `a` and `b`
 * and the facet with the points `tri`. The endpoints of a segment
 * intersection are ordered along the line.
 */
template <typename Intersection, typename OutputIterator, typename Kernel>
OutputIterator add_events(const Intersection& x, const typename Kernel::Point_3& a, const typename Kernel::Point_3& b,
                          const std::array<typename Kernel::Point_3, 3>& tri, OutputIterator oi,
                          const Kernel& kernel) {
  using Point = typename Kernel::Point_3;
  using Segment = typename Kernel::Segment_3;
  if (const Point* p = std::get_if<Point>(&x)) {
    *oi++ = Event<Kernel>{ *p, 0, perturbed_line_hits_triangle(a, b, tri, kernel) };
    return oi;
  }
  const Segment& s = std::get<Segment>(x);
  CGAL_assertion(! s.is_degenerate());
  bool forward = CGAL::sign((s.target() - s.source()) * (b - a)) == CGAL::POSITIVE;
  *oi++ = Event<Kernel>{ forward ? s.source() : s.target(), +1, false };
  *oi++ = Event<Kernel>{ forward ? s.target() : s.source(), -1, false };
  return oi;
}

/*! converts the intersections of the line and the facets of the mesh into
 * events.
 */
template <typename InputIterator, typename PolygonMesh, typename VertexPointMap, typename OutputIterator,
          typename Kernel>
OutputIterator make_events(InputIterator begin, InputIterator end, const typename Kernel::Line_3& line,
                           const PolygonMesh& mesh, const VertexPointMap& vpm, OutputIterator oi,
                           const Kernel& kernel) {
  using Point = typename Kernel::Point_3;
  const Point a = line.point(0);
  const Point b = line.point(1);
  for (auto it = begin; it != end; ++it) {
    auto h = halfedge(it->second, mesh);
    std::array<Point, 3> tri = { get(vpm, source(h, mesh)), get(vpm, target(h, mesh)),
                                 get(vpm, target(next(h, mesh), mesh)) };
    oi = add_events(it->first, a, b, tri, oi, kernel);
  }
  return oi;
}

/*! sorts the events along the line.
 */
template <typename RandomAccessIterator, typename Kernel>
void sort_intersections(RandomAccessIterator begin, RandomAccessIterator end, const typename Kernel::Line_3& line,
                        const Kernel& /* kernel */) {
  const auto dir = line.to_vector();
  const auto ref = line.point();
  std::sort(begin, end, [&](const auto& e1, const auto& e2) { return (e1.point - ref) * dir < (e2.point - ref) * dir; });
}

/*! processes the events of the line into the intersection of the line and the
 * closed volume bounded by the mesh: a sorted sequence of disjoint maximal
 * segments and isolated points.
 *
 * The events at the same point form a group. Every point of a group is in the
 * volume (it is on the boundary). The status of the open interval between two
 * consecutive groups is determined as follows:
 * - It is on the boundary iff it is covered by a segment intersection; the
 *   number of covering segment intersections is maintained with the deltas.
 * - Otherwise, it is inside iff the line, translated by an infinitesimal
 *   generic vector, is inside there. The translated line crosses the boundary
 *   at a group iff it hits an odd number of the facets of the point
 *   intersections of the group (a facet interior, the two facets incident to
 *   an edge, or the facets incident to a vertex). (The signs of the inner
 *   products of the direction and the normals of the facets do not determine
 *   this at a vertex: at a saddle vertex, a crossing line and a touching line
 *   can both meet incident facets of both signs.)
 *
 * \pre the events are sorted along the line.
 */
template <typename InputIterator, typename OutputIterator>
OutputIterator process_intersections(InputIterator begin, InputIterator end, OutputIterator oi,
                                     unsigned int verbose_level) {
  using Event_ = typename std::iterator_traits<InputIterator>::value_type;
  using Point = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<Event_>().point)>>;
  using Segment = typename CGAL::Kernel_traits<Point>::Kernel::Segment_3;

  int num_covering = 0;         // the number of segment intersections that cover the current interval
  bool shifted_inside = false;  // is the translated line inside in the current interval?
  bool inside = false;          // is the current interval in the volume?
  Point start;
  for (auto it = begin; it != end; ++it) {
    const Point& p = it->point;

    // Consume the group of all events at the point.
    bool crossing = false;
    for (auto git = it; ; it = git) {
      if (git->perturbed) crossing = ! crossing;
      num_covering += git->delta;
      if ((++git == end) || (git->point != p)) break;
    }
    CGAL_assertion(num_covering >= 0);
    if (crossing) shifted_inside = ! shifted_inside;
    bool inside_after = (num_covering > 0) || shifted_inside;

    if (! inside) {
      if (verbose_level > 2) std::cout << "Entrance point " << p << std::endl;
      start = p;
    }
    if (! inside_after) {
      if (verbose_level > 2) std::cout << "Exit point " << p << std::endl;
      if (inside) {
        if (verbose_level > 1) std::cout << "Segment intersection" << std::endl;
        *oi++ = Segment(start, p);
      }
      else {
        if (verbose_level > 1) std::cout << "Point intersection" << std::endl;
        *oi++ = p;
      }
    }
    inside = inside_after;
  }

  CGAL_assertion(! inside);
  CGAL_assertion(num_covering == 0);
  CGAL_assertion(! shifted_inside);
  return oi;
}

//////////////////////////////////////////////////////////////////////////////
// Building blocks of the optimized variants
//////////////////////////////////////////////////////////////////////////////

// Sorts events along the line of the query. The points lie exactly on the
// line, so comparing one coordinate along which the line is not constant
// suffices.
template <typename Events>
void sort_events(Events& events, const Query& query) {
  const auto& [a, b] = query;
  double d[3] = { b.x() - a.x(), b.y() - a.y(), b.z() - a.z() };
  int axis = 0;
  for (int i = 1; i < 3; ++i) if (std::abs(d[i]) > std::abs(d[axis])) axis = i;
  // The difference of two distinct doubles is never rounded to 0; its sign is
  // exact (also when it overflows).
  const bool increasing = d[axis] > 0;

  std::sort(events.begin(), events.end(),
            [&](const auto& e1, const auto& e2) {
              CGAL::Comparison_result res = (axis == 0) ? CGAL::compare_x(e1.point, e2.point) :
                (axis == 1) ? CGAL::compare_y(e1.point, e2.point) : CGAL::compare_z(e1.point, e2.point);
              return (res != CGAL::EQUAL) && ((res == CGAL::SMALLER) == increasing);
            });
}

// A line given by two points with a fast conservative test against a box. The
// slab test runs in doubles; the parameter range is inflated to cover the
// rounding errors, so a box that the exact line meets is never rejected.
//
// Error analysis: each slab parameter t = fl(fl(c - a_i) * fl(1 / fl(b_i -
// a_i))) has a relative error of at most ~4u (u = 2^-53) unless an operation
// overflows or underflows. An underflow causes an absolute error below 1e-307,
// covered by the absolute term; a non-finite value (overflow, or 0 * inf)
// makes the test accept the box.
class Fast_line {
public:
  explicit Fast_line(const Query& query) {
    const auto& [a, b] = query;
    for (int i = 0; i < 3; ++i) {
      m_a[i] = a[i];
      double d = b[i] - a[i];
      m_zero[i] = (d == 0);  // exact: the difference of distinct doubles is not 0
      m_inv[i] = m_zero[i] ? 0 : 1 / d;
      if (! std::isfinite(d) || ! std::isfinite(m_inv[i])) m_unsafe = true;
    }
  }

  bool intersects(const CGAL::Bbox_3& bb) const {
    if (m_unsafe) return true;
    double lo = -std::numeric_limits<double>::infinity();
    double hi = std::numeric_limits<double>::infinity();
    for (int i = 0; i < 3; ++i) {
      if (m_zero[i]) {
        if ((m_a[i] < bb.min(i)) || (m_a[i] > bb.max(i))) return false;
        continue;
      }
      double t1 = (bb.min(i) - m_a[i]) * m_inv[i];
      double t2 = (bb.max(i) - m_a[i]) * m_inv[i];
      if (! std::isfinite(t1) || ! std::isfinite(t2)) return true;
      if (t1 > t2) std::swap(t1, t2);
      lo = (std::max)(lo, t1);
      hi = (std::min)(hi, t2);
    }
    constexpr double eps = 1e-14;
    constexpr double tiny = 1e-300;
    return (lo - eps * std::abs(lo) - tiny) <= (hi + eps * std::abs(hi) + tiny);
  }

private:
  double m_a[3];
  double m_inv[3];
  bool m_zero[3];
  bool m_unsafe = false;
};

// A line given by two points with the statically filtered line-box test of
// CGAL: the slab test of do_intersect_bbox_segment_aux() with both ends
// unbounded, on the two input points. The test is exact when it is certain;
// an uncertain result accepts the box.
class Filtered_line {
public:
  explicit Filtered_line(const Query& query) : m_a(query.first), m_b(query.second) {}

  bool intersects(const CGAL::Bbox_3& bb) const {
    CGAL::Uncertain<bool> res =
      CGAL::Intersections::internal::do_intersect_bbox_segment_aux<double, false, false, true>(
        m_a.x(), m_a.y(), m_a.z(), m_b.x(), m_b.y(), m_b.z(), bb);
    return is_indeterminate(res) || res.sup();
  }

private:
  Epick::Point_3 m_a;
  Epick::Point_3 m_b;
};

// Traversal traits that visit every face whose box may meet the line.
template <typename Tree_, typename Leaf, typename Line = Fast_line>
class Conservative_traversal {
  using Primitive = typename Tree_::Primitive;
  using Node = CGAL::AABB_node<typename Tree_::AABB_traits>;

public:
  explicit Conservative_traversal(Leaf& leaf) : m_leaf(leaf) {}
  constexpr bool go_further() const { return true; }
  void intersection(const Line&, const Primitive& primitive) { m_leaf(primitive.id()); }
  bool do_intersect(const Line& line, const Node& node) const { return line.intersects(node.bbox()); }

private:
  Leaf& m_leaf;
};

// Kernels and converters for the filtered predicates below.
using Interval_kernel = CGAL::Simple_cartesian<CGAL::Interval_nt_advanced>;
using Exact_kernel = CGAL::Simple_cartesian<CGAL::Exact_rational>;
using To_interval = CGAL::Cartesian_converter<Epick, Interval_kernel>;
using To_exact = CGAL::Cartesian_converter<Epick, Exact_kernel>;

// The sign of orientation(a + w, b + w, p, q) for an infinitesimal generic
// translation w = (e, e^2, e^3), assuming orientation(a, b, p, q) = 0, that is,
// the sign of the first non-zero coordinate of (b - a) x (q - p).
template <typename K>
struct Perturbed_orientation_ {
  auto operator()(const typename K::Point_3& a, const typename K::Point_3& b,
                  const typename K::Point_3& p, const typename K::Point_3& q) const {
    auto c = CGAL::cross_product(b - a, q - p);
    auto s = CGAL::sign(c.x());
    if (s != CGAL::ZERO) return s;
    s = CGAL::sign(c.y());
    if (s != CGAL::ZERO) return s;
    return CGAL::sign(c.z());
  }
};
using Perturbed_orientation =
  CGAL::Filtered_predicate<Perturbed_orientation_<Exact_kernel>, Perturbed_orientation_<Interval_kernel>,
                           To_exact, To_interval>;

// Let t_i be the parameter of the intersection of the line (a, b) and the
// plane of the triangle (p_i, q_i, r_i), and let den_i be the inner product of
// the direction of the line and the normal of the triangle. Compares
// t_1 * den_1 * den_2 and t_2 * den_1 * den_2.
template <typename K>
struct Compare_along_line_ {
  auto operator()(const typename K::Point_3& a, const typename K::Point_3& b,
                  const typename K::Point_3& p1, const typename K::Point_3& q1, const typename K::Point_3& r1,
                  const typename K::Point_3& p2, const typename K::Point_3& q2, const typename K::Point_3& r2) const {
    auto d = b - a;
    auto n1 = CGAL::cross_product(q1 - p1, r1 - p1);
    auto n2 = CGAL::cross_product(q2 - p2, r2 - p2);
    return CGAL::compare((n1 * (p1 - a)) * (n2 * d), (n2 * (p2 - a)) * (n1 * d));
  }
};
using Compare_along_line =
  CGAL::Filtered_predicate<Compare_along_line_<Exact_kernel>, Compare_along_line_<Interval_kernel>,
                           To_exact, To_interval>;

// Given a line (a, b) in the plane of the triangle (p, q, r), returns the side
// of the direction b - a with respect to the line (p, q) in that plane:
// POSITIVE = the side of r, ZERO = parallel to (p, q), NEGATIVE = the other
// side. It is the sign of ((q - p) x (b - a)) * ((q - p) x (r - p)).
template <typename K>
struct Coplanar_side_ {
  auto operator()(const typename K::Point_3& p, const typename K::Point_3& q, const typename K::Point_3& r,
                  const typename K::Point_3& a, const typename K::Point_3& b) const {
    auto e = q - p;
    return CGAL::sign(CGAL::cross_product(e, b - a) * CGAL::cross_product(e, r - p));
  }
};
using Coplanar_side =
  CGAL::Filtered_predicate<Coplanar_side_<Exact_kernel>, Coplanar_side_<Interval_kernel>, To_exact, To_interval>;

// The same as perturbed_line_hits_triangle(), but with exact filtered
// predicates on the input points (which are doubles) only.
inline bool perturbed_line_hits_triangle(const Epick::Point_3& a, const Epick::Point_3& b,
                                         const std::array<Epick::Point_3, 3>& p) {
  static const Perturbed_orientation perturbed_orientation;
  CGAL::Sign s[3];
  for (int j = 0; j < 3; ++j) s[j] = CGAL::orientation(a, b, p[j], p[(j + 1) % 3]);
  if ((s[0] == CGAL::ZERO) && (s[1] == CGAL::ZERO) && (s[2] == CGAL::ZERO)) return false;
  for (int j = 0; j < 3; ++j) if (s[j] == CGAL::ZERO) s[j] = perturbed_orientation(a, b, p[j], p[(j + 1) % 3]);
  return (s[0] != CGAL::ZERO) && (s[0] == s[1]) && (s[1] == s[2]);
}

//////////////////////////////////////////////////////////////////////////////
// Benchmark infrastructure
//////////////////////////////////////////////////////////////////////////////

struct Context {
  Mesh mesh;
  Point_map<Epick> kpoints;
  Point_map<Epeck> epoints;
  std::vector<Query> queries;
  std::vector<Epeck::Line_3> elines;
};

const Point_map<Epick>& points(const Context& ctx, const Epick&) { return ctx.kpoints; }
const Point_map<Epeck>& points(const Context& ctx, const Epeck&) { return ctx.epoints; }

struct Result {
  std::string name;
  double setup_time = 0;
  double query_time = 0;
  std::size_t num_points = 0;
  std::size_t num_segments = 0;
  double length = 0;
  long num_invalid = -1;  // the number of queries with an invalid output; -1 = not validated
};

inline Epeck::Point_3 to_epeck(const Epick::Point_3& p) { return Epeck::Point_3(p.x(), p.y(), p.z()); }
inline const Epeck::Point_3& to_epeck(const Epeck::Point_3& p) { return p; }

using Side_of_mesh = CGAL::Side_of_triangle_mesh<Mesh, Epeck, Point_map<Epeck>>;

// Computes the reference intersection of the line of the i-th query and the
// closed volume, independently of the variants: the exact line is intersected
// with every face (no tree; CGAL's Epeck intersection), all intersection
// points and segment endpoints are sorted along the line (the lexicographic
// order is monotone along a line), and the open interval between every two
// consecutive points is classified by the exact point location of its
// midpoint (Side_of_triangle_mesh, Epeck): it is in the volume iff the
// midpoint is not on the unbounded side. Every point of the boundary is in the
// volume. The result is the sorted sequence of the maximal segments and the
// isolated points.
std::vector<Output<Epeck>> reference(const Context& ctx, const Side_of_mesh& side,
                                     const std::vector<Epeck::Triangle_3>& triangles, std::size_t i) {
  using P = Epeck::Point_3;
  const auto& line = ctx.elines[i];
  std::vector<P> pts;
  for (const auto& t : triangles) {
    if (! CGAL::do_intersect(line, t)) continue;
    auto x = CGAL::intersection(line, t);
    CGAL_assertion(x.has_value());
    if (const P* p = std::get_if<P>(&*x)) pts.push_back(*p);
    else {
      const auto& seg = std::get<Epeck::Segment_3>(*x);
      pts.push_back(seg.source());
      pts.push_back(seg.target());
    }
  }
  const bool increasing = CGAL::compare_xyz(line.point(0), line.point(1)) == CGAL::SMALLER;
  std::sort(pts.begin(), pts.end(), [&](const P& p, const P& q) {
    auto res = CGAL::compare_xyz(p, q);
    return (res != CGAL::EQUAL) && ((res == CGAL::SMALLER) == increasing);
  });
  pts.erase(std::unique(pts.begin(), pts.end()), pts.end());

  std::vector<Output<Epeck>> result;
  bool inside = false;
  P start;
  for (std::size_t k = 0; k < pts.size(); ++k) {
    bool inside_after = (k + 1 < pts.size()) && (side(CGAL::midpoint(pts[k], pts[k + 1])) != CGAL::ON_UNBOUNDED_SIDE);
    if (! inside) start = pts[k];
    if (! inside_after) {
      if (inside) result.push_back(Epeck::Segment_3(start, pts[k]));
      else result.push_back(pts[k]);
    }
    inside = inside_after;
  }
  return result;
}

// The reference outputs of all queries; computed only when validating.
std::vector<std::vector<Output<Epeck>>> references;

// Checks that the output of one query is identical to the reference: the same
// sequence of points and segments with equal (exact) endpoints.
template <typename K, typename InputIterator>
bool validate(InputIterator begin, InputIterator end, const std::vector<Output<Epeck>>& ref) {
  if (static_cast<std::size_t>(std::distance(begin, end)) != ref.size()) return false;
  auto rit = ref.begin();
  for (auto it = begin; it != end; ++it, ++rit) {
    if (const auto* s = std::get_if<typename K::Segment_3>(&*it)) {
      const auto* rs = std::get_if<Epeck::Segment_3>(&*rit);
      if (! rs || (to_epeck(s->source()) != rs->source()) || (to_epeck(s->target()) != rs->target())) return false;
    }
    else {
      const auto* rp = std::get_if<Epeck::Point_3>(&*rit);
      if (! rp || (to_epeck(std::get<typename K::Point_3>(*it)) != *rp)) return false;
    }
  }
  return true;
}

template <typename K>
void finalize(Result& res, const std::vector<Output<K>>& outputs, const std::vector<std::size_t>& offsets);

// Runs the query function on all queries and collects the statistics of the
// output; the statistics are computed after the timing.
template <typename K, typename F>
Result run(const std::string& name, double setup_time, std::size_t num_queries, F&& query) {
  std::vector<Output<K>> outputs;
  outputs.reserve(4 * num_queries);
  std::vector<std::size_t> offsets;
  offsets.reserve(num_queries + 1);
  CGAL::Real_timer timer;
  timer.start();
  for (std::size_t i = 0; i < num_queries; ++i) {
    offsets.push_back(outputs.size());
    query(i, std::back_inserter(outputs));
  }
  timer.stop();
  offsets.push_back(outputs.size());

  Result res{name, setup_time, timer.time()};
  finalize<K>(res, outputs, offsets);
  return res;
}

// Runs the queries in parallel; each thread processes a contiguous range of
// queries with its own query object.
template <typename K, typename MakeQuery>
Result run_parallel(const std::string& name, double setup_time, std::size_t num_queries, unsigned int num_threads,
                    MakeQuery make_query) {
  std::vector<std::vector<Output<K>>> outputs(num_threads);
  std::vector<std::vector<std::size_t>> offsets(num_threads);
  CGAL::Real_timer timer;
  timer.start();
  std::vector<std::thread> threads;
  for (unsigned int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&, t]() {
      auto query = make_query();
      std::size_t begin = num_queries * t / num_threads;
      std::size_t end = num_queries * (t + 1) / num_threads;
      outputs[t].reserve(4 * (end - begin));
      for (std::size_t i = begin; i < end; ++i) {
        offsets[t].push_back(outputs[t].size());
        query(i, std::back_inserter(outputs[t]));
      }
    });
  }
  for (auto& thread : threads) thread.join();
  timer.stop();

  std::vector<Output<K>> all_outputs;
  std::vector<std::size_t> all_offsets;
  for (unsigned int t = 0; t < num_threads; ++t) {
    for (auto o : offsets[t]) all_offsets.push_back(all_outputs.size() + o);
    all_outputs.insert(all_outputs.end(), outputs[t].begin(), outputs[t].end());
  }
  all_offsets.push_back(all_outputs.size());
  Result res{name + " (" + std::to_string(num_threads) + " threads)", setup_time, timer.time()};
  finalize<K>(res, all_outputs, all_offsets);
  return res;
}

// Computes the statistics of the output and validates it if requested.
template <typename K>
void finalize(Result& res, const std::vector<Output<K>>& outputs, const std::vector<std::size_t>& offsets) {
  const std::size_t num_queries = offsets.size() - 1;
  for (const auto& x : outputs) {
    if (const auto* s = std::get_if<typename K::Segment_3>(&x)) {
      ++res.num_segments;
      res.length += std::sqrt(CGAL::to_double(s->squared_length()));
    }
    else ++res.num_points;
  }
  if (! references.empty()) {
    res.num_invalid = 0;
    for (std::size_t i = 0; i < num_queries; ++i)
      if (! validate<K>(outputs.begin() + offsets[i], outputs.begin() + offsets[i + 1], references[i]))
        ++res.num_invalid;
  }
}

template <typename K, typename CacheDatum = CGAL::Tag_false>
std::pair<Tree<K, CacheDatum>*, double> build_tree(const Context& ctx) {
  CGAL::Real_timer timer;
  timer.start();
  auto* tree = new Tree<K, CacheDatum>(faces(ctx.mesh).first, faces(ctx.mesh).second, ctx.mesh, points(ctx, K()));
  tree->build();
  timer.stop();
  return { tree, timer.time() };
}

template <typename K>
std::array<typename K::Point_3, 3> face_points(const Context& ctx, Face_index f) {
  const auto& pm = points(ctx, K());
  Halfedge_index h = ctx.mesh.halfedge(f);
  return { pm[ctx.mesh.source(h)], pm[ctx.mesh.target(h)], pm[ctx.mesh.target(ctx.mesh.next(h))] };
}

//////////////////////////////////////////////////////////////////////////////
// The variants of the line-mesh intersection
//////////////////////////////////////////////////////////////////////////////

// The original procedure (Epeck; with Epick the intersection points, their
// equality tests, and the sort are inexact, and the output is invalid).
Result baseline(const Context& ctx, const std::string& name) {
  using Line_intersection = Tree<Epeck>::Intersection_and_primitive_id<Epeck::Line_3>::Type;
  auto [tree, setup_time] = build_tree<Epeck>(ctx);
  const auto& ls = ctx.elines;
  std::vector<Line_intersection> face_intersections;
  std::vector<Event<Epeck>> events;
  Epeck kernel;
  auto res = run<Epeck>(name, setup_time, ctx.queries.size(), [&](std::size_t i, auto oi) {
    face_intersections.clear();
    tree->all_intersections(ls[i], std::back_inserter(face_intersections));
    events.clear();
    make_events(face_intersections.begin(), face_intersections.end(), ls[i], ctx.mesh, ctx.epoints,
                std::back_inserter(events), kernel);
    sort_intersections(events.begin(), events.end(), ls[i], kernel);
    process_intersections(events.begin(), events.end(), oi, 0);
  });
  delete tree;
  return res;
}

// Adds the event(s) of the exact intersection x of the line of the i-th query
// and the face f. The perturbed test uses exact filtered predicates on the
// input points (doubles).
template <typename Intersection>
void add_epeck_events(const Context& ctx, std::size_t i, const Intersection& x, Face_index f,
                      std::vector<Event<Epeck>>& events) {
  if (const auto* p = std::get_if<Epeck::Point_3>(&x)) {
    const auto& [a, b] = ctx.queries[i];
    events.push_back({ *p, 0, perturbed_line_hits_triangle(a, b, face_points<Epick>(ctx, f)) });
    return;
  }
  // A segment (rare): the line lies in the plane of the face.
  const auto& seg = std::get<Epeck::Segment_3>(x);
  CGAL_assertion(! seg.is_degenerate());
  bool forward = CGAL::sign((seg.target() - seg.source()) * ctx.elines[i].to_vector()) == CGAL::POSITIVE;
  events.push_back({ forward ? seg.source() : seg.target(), +1, false });
  events.push_back({ forward ? seg.target() : seg.source(), -1, false });
}

// Epeck tree; the perturbed test is computed with exact filtered predicates on
// the input points and the events are sorted by a coordinate (predicates only,
// no constructions in the comparator).
template <typename CacheDatum>
Result sorted(const Context& ctx, const std::string& name) {
  using T = Tree<Epeck, CacheDatum>;
  using Line_intersection = typename T::template Intersection_and_primitive_id<Epeck::Line_3>::Type;
  auto [tree, setup_time] = build_tree<Epeck, CacheDatum>(ctx);
  std::vector<Line_intersection> face_intersections;
  std::vector<Event<Epeck>> events;
  auto res = run<Epeck>(name, setup_time, ctx.queries.size(), [&](std::size_t i, auto oi) {
    face_intersections.clear();
    tree->all_intersections(ctx.elines[i], std::back_inserter(face_intersections));
    events.clear();
    for (const auto& x : face_intersections) add_epeck_events(ctx, i, x.first, x.second, events);
    sort_events(events, ctx.queries[i]);
    process_intersections(events.begin(), events.end(), oi, 0);
  });
  delete tree;
  return res;
}

// Epick tree traversed with the fast conservative line-box test; Epeck
// intersections.
Result hybrid_fast(const Context& ctx, const std::string& name) {
  using T = Tree<Epick>;
  auto [tree, setup_time] = build_tree<Epick>(ctx);
  std::vector<Event<Epeck>> events;
  auto res = run<Epeck>(name, setup_time, ctx.queries.size(), [&](std::size_t i, auto oi) {
    events.clear();
    auto leaf = [&](Face_index f) {
      auto [p, q, r] = face_points<Epeck>(ctx, f);
      auto x = CGAL::intersection(ctx.elines[i], Epeck::Triangle_3(p, q, r));
      if (x) add_epeck_events(ctx, i, *x, f, events);
    };
    Conservative_traversal<T, decltype(leaf)> traversal(leaf);
    tree->traversal(Fast_line(ctx.queries[i]), traversal);
    sort_events(events, ctx.queries[i]);
    process_intersections(events.begin(), events.end(), oi, 0);
  });
  delete tree;
  return res;
}

// A contact of the line and the closed face; the contact is either in the
// interior of the face, on an edge, or at a vertex.
struct Contact {
  int dim;                // 2 = face, 1 = edge, 0 = vertex
  std::size_t id;         // the index of the face, edge, or vertex
  Face_index face;        // the face that reported the contact
  CGAL::Sign sign;        // the sign of the inner product of the direction and the normal
  bool perturbed;         // does the infinitesimally translated line hit the face?
};

// Epick tree traversed with the fast line-box test. The contacts are
// classified with exact Epick predicates on the input points only; Epeck
// points are constructed only for the output.
class Line_mesh_by_predicates {
  using T = Tree<Epick>;

public:
  Line_mesh_by_predicates(const Context& ctx, const T& tree) : m_ctx(ctx), m_tree(tree) {}

  // Computes the intersection of the line of the i-th query and the volume.
  template <typename OutputIterator>
  void operator()(std::size_t i, OutputIterator oi) {
    const Mesh& mesh = m_ctx.mesh;
    const auto& [a, b] = m_ctx.queries[i];
    m_contacts.clear();

    // The line meets the closed triangle (p0, p1, p2) transversally iff the
    // non-zero signs of orientation(a, b, p_j, p_j+1) agree and not all are 0.
    // The common sign is the sign of the inner product of the direction of the
    // line and the normal of the triangle. The zero signs determine the
    // feature: one zero = an edge, two zeros = a vertex.
    auto leaf = [&](Face_index f) {
      Halfedge_index h[3];
      h[0] = mesh.halfedge(f);
      h[1] = mesh.next(h[0]);
      h[2] = mesh.next(h[1]);
      const Epick::Point_3* p[3];
      for (int j = 0; j < 3; ++j) p[j] = &m_ctx.kpoints[mesh.source(h[j])];
      CGAL::Sign s[3];
      CGAL::Sign common = CGAL::ZERO;
      int num_zeros = 0;
      for (int j = 0; j < 3; ++j) {
        s[j] = CGAL::orientation(a, b, *p[j], *p[(j + 1) % 3]);
        if (s[j] == CGAL::ZERO) ++num_zeros;
        else if (common == CGAL::ZERO) common = s[j];
        else if (s[j] != common) return;
      }
      if (num_zeros == 3) return;     // the line and the face are coplanar

      bool perturbed = true;
      for (int j = 0; j < 3; ++j)
        if ((s[j] == CGAL::ZERO) && (m_perturbed_orientation(a, b, *p[j], *p[(j + 1) % 3]) != common))
          perturbed = false;

      if (num_zeros == 0) m_contacts.push_back({ 2, std::size_t(f.idx()), f, common, true });
      else if (num_zeros == 1) {
        int j = (s[0] == CGAL::ZERO) ? 0 : (s[1] == CGAL::ZERO) ? 1 : 2;
        m_contacts.push_back({ 1, std::size_t(mesh.edge(h[j]).idx()), f, common, perturbed });
      }
      else {
        // The zero edges j+1 and j+2 share the source of h[j+2].
        int j = (s[0] != CGAL::ZERO) ? 0 : (s[1] != CGAL::ZERO) ? 1 : 2;
        m_contacts.push_back({ 0, std::size_t(mesh.source(h[(j + 2) % 3]).idx()), f, common, perturbed });
      }
    };
    Conservative_traversal<T, decltype(leaf)> traversal(leaf);
    m_tree.traversal(Fast_line(m_ctx.queries[i]), traversal);

    // Merge the m_contacts of each feature; the line crosses the boundary of
    // the volume at the feature iff the translated line hits an odd number of
    // the faces incident to the feature.
    std::sort(m_contacts.begin(), m_contacts.end(),
              [](const Contact& c1, const Contact& c2)
              { return (c1.dim != c2.dim) ? (c1.dim < c2.dim) : (c1.id < c2.id); });
    m_features.clear();
    for (const auto& c : m_contacts) {
      if (m_features.empty() || (m_features.back().dim != c.dim) || (m_features.back().id != c.id))
        m_features.push_back(c);
      else if (c.perturbed) m_features.back().perturbed = ! m_features.back().perturbed;
    }

    // Sort the m_features along the line.
    std::sort(m_features.begin(), m_features.end(), [&](const Contact& c1, const Contact& c2) {
      auto [p1, q1, r1] = face_points<Epick>(m_ctx, c1.face);
      auto [p2, q2, r2] = face_points<Epick>(m_ctx, c2.face);
      CGAL::Comparison_result res = m_compare_along_line(a, b, p1, q1, r1, p2, q2, r2);
      if (c1.sign != c2.sign) res = CGAL::opposite(res);
      return res == CGAL::SMALLER;
    });

    // Construct the output: the maximal segments and the isolated points of
    // the intersection of the line and the closed volume. Every feature point
    // is in the volume. The open interval after a feature is in the volume iff
    // the translated line is inside there, or the line runs along the surface
    // there (an interval along the surface starts at a feature: the end of a
    // maximal part of the line along the surface is a point of a face that is
    // not coplanar with the line).
    const auto& line = m_ctx.elines[i];
    auto point = [&](const Contact& c) -> Epeck::Point_3 {
      if (c.dim == 0) return m_ctx.epoints[Vertex_index(static_cast<Mesh::size_type>(c.id))];
      auto [p, q, r] = face_points<Epeck>(m_ctx, c.face);
      auto x = CGAL::intersection(line, Epeck::Plane_3(p, q, r));
      CGAL_assertion(x && std::get_if<Epeck::Point_3>(&*x));
      return *std::get_if<Epeck::Point_3>(&*x);
    };
    bool shifted_inside = false;  // is the translated line inside in the current interval?
    bool inside = false;          // is the current interval in the volume?
    Epeck::Point_3 start;
    for (const auto& c : m_features) {
      if (c.perturbed) shifted_inside = ! shifted_inside;
      bool inside_after = shifted_inside || along_surface_after(c, a, b);
      if (! inside) start = point(c);
      if (! inside_after) {
        if (inside) *oi++ = Epeck::Segment_3(start, point(c));
        else *oi++ = start;
      }
      inside = inside_after;
    }
    CGAL_assertion(! inside);
    CGAL_assertion(! shifted_inside);
  }

private:
  // Determines whether the line (a, b) runs along the surface right after the
  // feature, that is, whether the points c + t (b - a) for all small t > 0 lie
  // in a face that is incident to the feature and coplanar with the line.
  bool along_surface_after(const Contact& c, const Epick::Point_3& a, const Epick::Point_3& b) const {
    if (c.dim == 2) return false;  // the line crosses the face transversally
    const Mesh& mesh = m_ctx.mesh;
    const auto& pm = m_ctx.kpoints;
    auto coplanar = [&](const Epick::Point_3& p, const Epick::Point_3& q, const Epick::Point_3& r) {
      return (CGAL::orientation(p, q, r, a) == CGAL::COPLANAR) && (CGAL::orientation(p, q, r, b) == CGAL::COPLANAR);
    };
    if (c.dim == 1) {
      // The line crosses the edge (p, q) at an interior point; it runs into
      // the incident face (p, q, r) iff the face is coplanar with the line and
      // the direction points to the side of r.
      Halfedge_index h0 = mesh.halfedge(Mesh::Edge_index(static_cast<Mesh::size_type>(c.id)));
      for (Halfedge_index h : { h0, mesh.opposite(h0) }) {
        const auto& p = pm[mesh.source(h)];
        const auto& q = pm[mesh.target(h)];
        const auto& r = pm[mesh.target(mesh.next(h))];
        if (coplanar(p, q, r) && (m_coplanar_side(p, q, r, a, b) == CGAL::POSITIVE)) return true;
      }
      return false;
    }
    // The line passes through the vertex v; it runs into the incident face
    // (u, v, w) iff the face is coplanar with the line and the direction is in
    // the closed angle of the face at v, from v->w counterclockwise to v->u.
    Vertex_index vi(static_cast<Mesh::size_type>(c.id));
    const auto& v = pm[vi];
    for (Halfedge_index h : CGAL::halfedges_around_target(mesh.halfedge(vi), mesh)) {
      const auto& u = pm[mesh.source(h)];
      const auto& w = pm[mesh.target(mesh.next(h))];
      if (! coplanar(u, v, w)) continue;
      if ((m_coplanar_side(v, w, u, a, b) != CGAL::NEGATIVE) && (m_coplanar_side(v, u, w, a, b) != CGAL::NEGATIVE))
        return true;
    }
    return false;
  }

  const Context& m_ctx;
  const T& m_tree;
  Perturbed_orientation m_perturbed_orientation;
  Compare_along_line m_compare_along_line;
  Coplanar_side m_coplanar_side;
  std::vector<Contact> m_contacts;
  std::vector<Contact> m_features;
};

Result predicates(const Context& ctx, const std::string& name) {
  auto [tree, setup_time] = build_tree<Epick>(ctx);
  Line_mesh_by_predicates query(ctx, *tree);
  auto res = run<Epeck>(name, setup_time, ctx.queries.size(), query);
  delete tree;
  return res;
}

// The same as predicates(), with the queries distributed among threads.
Result predicates_mt(const Context& ctx, const std::string& name, unsigned int num_threads) {
  auto [tree, setup_time] = build_tree<Epick>(ctx);
  auto res = run_parallel<Epeck>(name, setup_time, ctx.queries.size(), num_threads,
                                 [&]() { return Line_mesh_by_predicates(ctx, *tree); });
  delete tree;
  return res;
}

//////////////////////////////////////////////////////////////////////////////
// all_intersections() benchmark
//////////////////////////////////////////////////////////////////////////////

void print_all_intersections(const std::string& name, double setup_time, double time, std::size_t n,
                             std::size_t num_intersections);

// The library path: AABB_tree::all_intersections() of the exact (Epeck) line
// and the faces.
void bench_all_intersections(const Context& ctx) {
  using Line_intersection = Tree<Epeck>::Intersection_and_primitive_id<Epeck::Line_3>::Type;
  auto [tree, setup_time] = build_tree<Epeck>(ctx);
  std::vector<Line_intersection> face_intersections;
  std::size_t num_intersections = 0;
  CGAL::Real_timer timer;
  timer.start();
  for (const auto& line : ctx.elines) {
    face_intersections.clear();
    tree->all_intersections(line, std::back_inserter(face_intersections));
    num_intersections += face_intersections.size();
  }
  timer.stop();
  delete tree;
  print_all_intersections("Epeck all_intersections", setup_time, timer.time(), ctx.elines.size(), num_intersections);
}

void print_all_intersections(const std::string& name, double setup_time, double time, std::size_t n,
                             std::size_t num_intersections) {
  std::cout << std::setw(40) << std::left << name
            << " tree: " << std::setw(10) << setup_time
            << " queries: " << std::setw(10) << time
            << " queries/s: " << std::setw(10) << std::size_t(n / time)
            << " intersections: " << num_intersections << "\n";
}

// Determines whether the line (a, b) meets the closed triangle p, exactly:
// the non-zero signs of orientation(a, b, p_j, p_j+1) agree; if all are zero
// (the line and the triangle are coplanar), Epeck decides.
inline bool line_meets_triangle(const Epick::Point_3& a, const Epick::Point_3& b,
                                const std::array<Epick::Point_3, 3>& p) {
  CGAL::Sign common = CGAL::ZERO;
  for (int j = 0; j < 3; ++j) {
    CGAL::Sign s = CGAL::orientation(a, b, p[j], p[(j + 1) % 3]);
    if (s == CGAL::ZERO) continue;
    if (common == CGAL::ZERO) common = s;
    else if (s != common) return false;
  }
  if (common != CGAL::ZERO) return true;
  return CGAL::do_intersect(Epeck::Line_3(to_epeck(a), to_epeck(b)),
                            Epeck::Triangle_3(to_epeck(p[0]), to_epeck(p[1]), to_epeck(p[2])));
}

// The search for the faces that the exact line meets: the library path
// (Epeck tree, all_intersected_primitives()) vs. the Epick tree traversed with
// the fast conservative line-box test and an exact test at the leaves on the
// input points. Both are exact and report the same faces.
void bench_traversal(const Context& ctx) {
  const std::size_t n = ctx.queries.size();
  {
    auto [tree, setup_time] = build_tree<Epeck>(ctx);
    std::vector<Face_index> fs;
    std::size_t num_intersections = 0;
    CGAL::Real_timer timer;
    timer.start();
    for (const auto& line : ctx.elines) {
      fs.clear();
      tree->all_intersected_primitives(line, std::back_inserter(fs));
      num_intersections += fs.size();
    }
    timer.stop();
    delete tree;
    print_all_intersections("Epeck all_intersected_primitives", setup_time, timer.time(), n, num_intersections);
  }
  auto epick_traversal = [&](auto* line_tag, const std::string& name) {
    using Line = std::remove_pointer_t<decltype(line_tag)>;
    using T = Tree<Epick>;
    auto [tree, setup_time] = build_tree<Epick>(ctx);
    std::size_t num_intersections = 0;
    CGAL::Real_timer timer;
    timer.start();
    for (std::size_t i = 0; i < n; ++i) {
      const auto& [a, b] = ctx.queries[i];
      auto leaf = [&](Face_index f) { if (line_meets_triangle(a, b, face_points<Epick>(ctx, f))) ++num_intersections; };
      Conservative_traversal<T, decltype(leaf), Line> traversal(leaf);
      tree->traversal(Line(ctx.queries[i]), traversal);
    }
    timer.stop();
    delete tree;
    print_all_intersections(name, setup_time, timer.time(), n, num_intersections);
  };
  epick_traversal(static_cast<Fast_line*>(nullptr), "Epick Fast_line + exact test");
  epick_traversal(static_cast<Filtered_line*>(nullptr), "Epick Filtered_line + exact test");
}

//////////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////////

// Generates random lines, each through two random points in the bounding box,
// through two random distinct vertices of the mesh, through a random vertex
// or the midpoint of a random edge and a random point in the enlarged bounding
// box (often touching the solid at that point only), or through two random
// points with integer coordinates in the bounding box (on meshes with integer
// coordinates, many such lines lie in planes of faces or along edges).
std::vector<Query> generate_queries(const Mesh& mesh, const std::string& type, std::size_t n, CGAL::Random& rng) {
  std::vector<Query> queries;
  queries.reserve(n);
  if (type == "vertices") {
    const auto nv = static_cast<int>(mesh.number_of_vertices());
    while (queries.size() < n) {
      const auto& a = mesh.point(Vertex_index(rng.get_int(0, nv)));
      const auto& b = mesh.point(Vertex_index(rng.get_int(0, nv)));
      if (a != b) queries.emplace_back(a, b);
    }
    return queries;
  }
  auto bb = PMP::bbox(mesh);
  auto random_point = [&]() {
    return Epick::Point_3(rng.get_double(bb.xmin(), bb.xmax()),
                          rng.get_double(bb.ymin(), bb.ymax()),
                          rng.get_double(bb.zmin(), bb.zmax()));
  };
  // A random point in the bounding box enlarged by its size on every side
  // (often outside the solid, also when the solid fills its bounding box).
  auto random_far_point = [&]() {
    double dx = bb.xmax() - bb.xmin(), dy = bb.ymax() - bb.ymin(), dz = bb.zmax() - bb.zmin();
    return Epick::Point_3(rng.get_double(bb.xmin() - dx, bb.xmax() + dx),
                          rng.get_double(bb.ymin() - dy, bb.ymax() + dy),
                          rng.get_double(bb.zmin() - dz, bb.zmax() + dz));
  };
  if ((type == "vertex-point") || (type == "edge-point")) {
    const auto nv = static_cast<int>(mesh.number_of_vertices());
    const auto ne = static_cast<int>(mesh.number_of_edges());
    while (queries.size() < n) {
      Epick::Point_3 a;
      if (type == "vertex-point") a = mesh.point(Vertex_index(rng.get_int(0, nv)));
      else {
        // The midpoint of a random edge (on the edge exactly when the
        // coordinates are, e.g., integers).
        auto h = mesh.halfedge(Mesh::Edge_index(rng.get_int(0, ne)));
        const auto& p = mesh.point(mesh.source(h));
        const auto& q = mesh.point(mesh.target(h));
        a = Epick::Point_3((p.x() + q.x()) / 2, (p.y() + q.y()) / 2, (p.z() + q.z()) / 2);
      }
      auto b = random_far_point();
      if (a != b) queries.emplace_back(a, b);
    }
    return queries;
  }
  if (type == "grid") {
    auto coord = [&](double lo, double hi) {
      return static_cast<double>(rng.get_int(static_cast<int>(std::floor(lo)), static_cast<int>(std::ceil(hi)) + 1));
    };
    while (queries.size() < n) {
      Epick::Point_3 a(coord(bb.xmin(), bb.xmax()), coord(bb.ymin(), bb.ymax()), coord(bb.zmin(), bb.zmax()));
      Epick::Point_3 b(coord(bb.xmin(), bb.xmax()), coord(bb.ymin(), bb.ymax()), coord(bb.zmin(), bb.zmax()));
      if (a != b) queries.emplace_back(a, b);
    }
    return queries;
  }
  while (queries.size() < n) {
    auto a = random_point();
    auto b = random_point();
    if (a != b) queries.emplace_back(a, b);
  }
  return queries;
}

const std::vector<std::string> all_variants =
  { "baseline", "sorted", "cached", "hybrid-fast", "predicates", "predicates-mt" };

int main(int argc, char* argv[]) {
  namespace po = boost::program_options;

  std::string filename;
  std::string bench;
  std::string line_type;
  std::string variants_str;
  std::size_t num_queries;
  unsigned int seed;
  unsigned int num_threads;

  po::options_description desc("Options");
  desc.add_options()
    ("help,h", "print this help message")
    ("bench,b", po::value<std::string>(&bench)->default_value("all_intersections"),
     "operation to benchmark: all_intersections, line_mesh, or both")
    ("variants,V", po::value<std::string>(&variants_str)->default_value("all"),
     "comma separated line_mesh variants: baseline, sorted, cached, hybrid-fast, predicates, "
     "predicates-mt, or all")
    ("threads,j", po::value<unsigned int>(&num_threads)->default_value(std::max(1u, std::thread::hardware_concurrency())),
     "number of threads of the predicates-mt variant")
    ("lines,l", po::value<std::string>(&line_type)->default_value("random"),
     "line queries: random (through two random points in the bounding box), vertices (through two mesh vertices), "
     "vertex-point / edge-point (through a mesh vertex / edge midpoint and a random point in the enlarged bounding "
     "box), "
     "or grid (through two random points with integer coordinates in the bounding box)")
    ("input-file,i", po::value<std::string>(&filename)->default_value(CGAL::data_file_path("meshes/elephant.off")),
     "input triangle mesh")
    ("queries,n", po::value<std::size_t>(&num_queries)->default_value(10000), "number of line queries")
    ("seed,s", po::value<unsigned int>(&seed)->default_value(0), "seed of the random number generator")
    ("validate", "validate the line_mesh outputs with an independent (slow) point location");

  po::positional_options_description pos;
  pos.add("input-file", 1);

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
    po::notify(vm);
  }
  catch (const po::error& e) {
    std::cerr << "Error: " << e.what() << "\n" << desc << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("help")) {
    std::cout << "Usage: " << argv[0] << " [options] [input-file]\n" << desc << std::endl;
    return EXIT_SUCCESS;
  }

  auto error = [&](const std::string& option, const std::string& value) {
    std::cerr << "Error: invalid value of --" << option << ": " << value << "\n" << desc << std::endl;
    return EXIT_FAILURE;
  };

  bool bench_all_intersections_op = (bench == "all_intersections") || (bench == "both");
  bool bench_line_mesh = (bench == "line_mesh") || (bench == "both");
  if (! bench_all_intersections_op && ! bench_line_mesh) return error("bench", bench);


  if ((line_type != "random") && (line_type != "vertices") && (line_type != "vertex-point") && (line_type != "edge-point") &&
      (line_type != "grid")) return error("lines", line_type);

  std::vector<std::string> variants;
  if (variants_str == "all") variants = all_variants;
  else {
    std::stringstream ss(variants_str);
    std::string v;
    while (std::getline(ss, v, ',')) {
      if (std::find(all_variants.begin(), all_variants.end(), v) == all_variants.end()) return error("variants", v);
      variants.push_back(v);
    }
  }

  Context ctx;
  if (! CGAL::IO::read_polygon_mesh(filename, ctx.mesh) || ! CGAL::is_triangle_mesh(ctx.mesh)) {
    std::cerr << "Error: cannot read the triangle mesh " << filename << std::endl;
    return EXIT_FAILURE;
  }
  Mesh& mesh = ctx.mesh;

  // Precomputed data: Epeck points.
  ctx.kpoints = mesh.points();
  ctx.epoints = mesh.add_property_map<Vertex_index, Epeck::Point_3>("v:epeck_point").first;
  for (auto v : vertices(mesh)) {
    const auto& p = mesh.point(v);
    ctx.epoints[v] = Epeck::Point_3(p.x(), p.y(), p.z());
  }

  CGAL::Random rng(seed);
  ctx.queries = generate_queries(mesh, line_type, num_queries, rng);
  for (const auto& [a, b] : ctx.queries) {
    ctx.elines.emplace_back(Epeck::Point_3(a.x(), a.y(), a.z()), Epeck::Point_3(b.x(), b.y(), b.z()));
  }

  std::cout << "Mesh: " << filename << ", faces: " << num_faces(mesh) << ", lines: " << line_type
            << ", queries: " << num_queries << ", seed: " << seed << "\n\n";

  if (bench_all_intersections_op) {
    std::cout << "=== all_intersections ===\n";
    bench_all_intersections(ctx);
    bench_traversal(ctx);
    std::cout << std::endl;
  }

  if (bench_line_mesh) {
    if (! CGAL::is_closed(mesh))
      std::cerr << "Warning: the mesh is not closed; the line-mesh intersections are meaningless." << std::endl;

    if (vm.count("validate")) {
      CGAL::Real_timer timer;
      timer.start();
      Side_of_mesh side(mesh, ctx.epoints);
      std::vector<Epeck::Triangle_3> triangles;
      for (auto f : faces(mesh)) {
        auto [p, q, r] = face_points<Epeck>(ctx, f);
        triangles.emplace_back(p, q, r);
      }
      references.reserve(ctx.queries.size());
      for (std::size_t i = 0; i < ctx.queries.size(); ++i) references.push_back(reference(ctx, side, triangles, i));
      timer.stop();
      std::cout << "Reference outputs computed in " << timer.time() << " s\n\n";
    }

    std::vector<Result> results;
    for (const auto& v : variants) {
      if (v == "baseline") results.push_back(baseline(ctx, v));
      else if (v == "sorted") results.push_back(sorted<CGAL::Tag_false>(ctx, v));
      else if (v == "cached") results.push_back(sorted<CGAL::Tag_true>(ctx, v));
      else if (v == "hybrid-fast") results.push_back(hybrid_fast(ctx, v));
      else if (v == "predicates") results.push_back(predicates(ctx, v));
      else if (v == "predicates-mt") results.push_back(predicates_mt(ctx, v, std::max(1u, num_threads)));
    }

    std::cout << "=== line_mesh ===\n"
              << std::setw(28) << std::left << "variant"
              << std::setw(12) << "tree (s)" << std::setw(12) << "queries (s)" << std::setw(12) << "queries/s"
              << std::setw(10) << "points" << std::setw(10) << "segments" << std::setw(18) << "total length"
              << std::setw(11) << "check" << "invalid\n";
    const Result& ref = results.front();
    for (const auto& r : results) {
      bool same = (r.num_points == ref.num_points) && (r.num_segments == ref.num_segments) &&
        (std::abs(r.length - ref.length) <= 1e-9 * (std::max)(1.0, ref.length));
      std::cout << std::setw(28) << std::left << r.name
                << std::setw(12) << r.setup_time << std::setw(12) << r.query_time
                << std::setw(12) << std::size_t(num_queries / r.query_time)
                << std::setw(10) << r.num_points << std::setw(10) << r.num_segments
                << std::setw(18) << std::setprecision(12) << r.length << std::setprecision(6)
                << std::setw(11) << (same ? "same" : "DIFFERENT")
                << ((r.num_invalid < 0) ? std::string("-") : std::to_string(r.num_invalid)) << "\n";
    }
    std::cout << "(check: compared with the first variant; invalid: the number of queries with an invalid output)"
              << std::endl;
  }

  return EXIT_SUCCESS;
}
