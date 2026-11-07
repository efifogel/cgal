// Test polycurves of geodesic arcs on the sphere that touch or lie on the
// identification curve. The traits on the sphere forbids the invocation of
// several functors with curves that lie on the identification curve and with
// points that lie on the identification curve, e.g., Parameter_space_in_x_2,
// Compare_x_2, and Compare_xy_2. The arrangement code adheres to this rule, and
// the polycurve traits must adhere to it as well when it applies the functors
// of the subcurve traits. The results obtained with polylines of geodesic arcs
// are compared with the results obtained with the geodesic arcs themselves.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_geodesic_arc_on_sphere_traits_2.h>
#include <CGAL/Arr_spherical_topology_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arrangement_on_surface_2.h>
#include <CGAL/Draw_aos/type_utils.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Geodesic_traits = CGAL::Arr_geodesic_arc_on_sphere_traits_2<Kernel>;
using Polyline_traits = CGAL::Arr_polyline_traits_2<Geodesic_traits>;
using Point = Geodesic_traits::Point_2;
using Arc = Geodesic_traits::X_monotone_curve_2;
using Polyline = Polyline_traits::X_monotone_curve_2;
using Geodesic_arrangement =
  CGAL::Arrangement_on_surface_2<Geodesic_traits, CGAL::Arr_spherical_topology_traits_2<Geodesic_traits>>;
using Polyline_arrangement =
  CGAL::Arrangement_on_surface_2<Polyline_traits, CGAL::Arr_spherical_topology_traits_2<Polyline_traits>>;

/* Run a test case; report a failure, including a CGAL failure (e.g., a
 * violated precondition), which is thrown as an exception.
 */
bool run(const std::string& name, const std::function<bool()>& test) {
  try {
    if (test()) return true;
    std::cerr << "Error: " << name << std::endl;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << name << ": exception:\n" << e.what() << std::endl;
  }
  return false;
}

/* Construct closed polylines through points on small circles; the circles lie
 * in planes parallel to the XY-, YZ-, and ZX-planes. Every circle is given by
 * the index of the axis orthogonal to its plane and by the (signed) distance
 * of its plane from the origin. The sample points are offset by half a step so
 * that no point lies on the identification curve.
 */
std::vector<std::vector<Point>> small_circles(const Geodesic_traits& traits, std::size_t n) {
  auto ctr_p = traits.construct_point_2_object();
  std::vector<std::pair<int, double>> circles = {{2, -0.6}, {2, 0.2}, {2, 0.7},  {0, -0.7}, {0, -0.1},
                                                 {0, 0.5},  {1, -0.4}, {1, 0.3}, {1, 0.8}};
  std::vector<std::vector<Point>> loops;
  for (const auto& [axis, c] : circles) {
    const double r = std::sqrt(1.0 - c * c);
    std::vector<Point> points;
    for (std::size_t i = 0; i < n; ++i) {
      const double t = 2.0 * CGAL_PI * (static_cast<double>(i) + 0.5) / static_cast<double>(n);
      std::array<double, 3> v;
      v[axis] = c;
      v[(axis + 1) % 3] = r * std::cos(t);
      v[(axis + 2) % 3] = r * std::sin(t);
      points.push_back(ctr_p(v[0], v[1], v[2]));
    }
    points.push_back(points.front());
    loops.push_back(points);
  }
  return loops;
}

/* Construct the arrangement of the geodesic arcs of the given loops, and
 * optionally the identification curve; obtain the number of faces.
 */
std::size_t geodesic_faces(const std::vector<std::vector<Point>>& loops, bool identification) {
  Geodesic_traits traits;
  Geodesic_arrangement arr(&traits);
  std::vector<Geodesic_traits::Curve_2> arcs;
  for (const auto& loop : loops)
    for (std::size_t i = 0; i + 1 < loop.size(); ++i)
      arcs.push_back(traits.construct_curve_2_object()(loop[i], loop[i + 1]));
  CGAL::insert(arr, arcs.begin(), arcs.end());
  if (identification) CGAL::insert(arr, CGAL::draw_aos::construct_identification_curve(traits));
  return arr.number_of_faces();
}

/* Construct the arrangement of the polylines of the given loops, inserted
 * either incrementally or aggregately, and optionally the identification
 * curve. Check that the number of faces equals the number of faces of the
 * arrangement of the geodesic arcs.
 */
bool test_arrangement(const std::vector<std::vector<Point>>& loops, bool incremental, bool identification) {
  Polyline_traits traits;
  Polyline_arrangement arr(&traits);
  auto ctr_cv = traits.construct_curve_2_object();
  if (incremental) {
    for (const auto& loop : loops) CGAL::insert(arr, ctr_cv(loop.begin(), loop.end()));
  }
  else {
    std::vector<Polyline_traits::Curve_2> curves;
    for (const auto& loop : loops) curves.push_back(ctr_cv(loop.begin(), loop.end()));
    CGAL::insert(arr, curves.begin(), curves.end());
  }
  if (identification) CGAL::insert(arr, CGAL::draw_aos::construct_identification_curve(traits));
  if (! arr.is_valid()) return false;
  auto expected = geodesic_faces(loops, identification);
  if (arr.number_of_faces() == expected) return true;
  std::cerr << "The number of faces is " << arr.number_of_faces() << ", expected " << expected << std::endl;
  return false;
}

/* Construct a point on the small circle at height z near the identification
 * curve, which is the half great circle where y = 0 and x < 0, that is, at
 * azimuth pi.
 */
Point point_at(const Geodesic_traits& traits, double azimuth, double z = 0.5) {
  const double r = std::sqrt(1.0 - z * z);
  return traits.construct_point_2_object()(r * std::cos(azimuth), r * std::sin(azimuth), z);
}

/* Construct a point on the identification curve at height z. */
Point point_on_identification(const Geodesic_traits& traits, double z = 0.5) {
  const double r = std::sqrt(1.0 - z * z);
  return traits.construct_point_2_object()(-r, 0, z);
}

/* Construct the arcs through the given points, and the polyline of them. */
std::pair<std::vector<Arc>, Polyline> arcs_and_polyline(const Polyline_traits& traits,
                                                        const std::vector<Point>& points) {
  auto ctr_arc = traits.subcurve_traits_2()->construct_x_monotone_curve_2_object();
  std::vector<Arc> arcs;
  for (std::size_t i = 0; i + 1 < points.size(); ++i) arcs.push_back(ctr_arc(points[i], points[i + 1]));
  return std::make_pair(arcs, traits.construct_x_monotone_curve_2_object()(arcs.begin(), arcs.end()));
}

/* Obtain the intersection points of the given curves. Return false if the
 * curves overlap.
 */
template <typename Gt>
bool intersection_points(const Gt& traits, const typename Gt::X_monotone_curve_2& xcv1,
                         const typename Gt::X_monotone_curve_2& xcv2, std::vector<Point>& points) {
  using Intersection_point = std::pair<Point, typename Gt::Multiplicity>;
  using Intersection_result = std::variant<Intersection_point, typename Gt::X_monotone_curve_2>;
  std::list<Intersection_result> results;
  traits.intersect_2_object()(xcv1, xcv2, std::back_inserter(results));
  for (const auto& res : results) {
    const auto* ip = std::get_if<Intersection_point>(&res);
    if (ip == nullptr) return false;
    points.push_back(ip->first);
  }
  return true;
}

/* Test the intersection of the identification curve with a polyline that
 * reaches the identification curve. The intersection points must be the
 * intersection points of the identification curve with the arcs.
 */
bool test_intersection(const std::vector<Point>& points, bool polyline_first) {
  Polyline_traits traits;
  const auto& geodesic_traits = *traits.subcurve_traits_2();
  auto [arcs, polyline] = arcs_and_polyline(traits, points);
  auto id_arc = CGAL::draw_aos::construct_identification_curve(geodesic_traits);
  auto id_polyline = CGAL::draw_aos::construct_identification_curve(traits);

  std::vector<Point> expected;
  for (const auto& arc : arcs) {
    bool ok = polyline_first ? intersection_points(geodesic_traits, arc, id_arc, expected) :
      intersection_points(geodesic_traits, id_arc, arc, expected);
    if (! ok) return false;
  }

  std::vector<Point> result;
  bool ok = polyline_first ? intersection_points(traits, polyline, id_polyline, result) :
    intersection_points(traits, id_polyline, polyline, result);
  if (! ok) return false;

  auto equal = geodesic_traits.equal_2_object();
  if (result.size() == expected.size() &&
      std::is_permutation(result.begin(), result.end(), expected.begin(), equal))
    return true;
  std::cerr << "The number of intersection points is " << result.size() << ", expected " << expected.size()
            << std::endl;
  return false;
}

/* Test the comparison of points on the identification curve with a polyline
 * that reaches the identification curve. The results must equal the results
 * of the comparison with the arc that reaches the identification curve.
 */
bool test_compare_y_at_x(const std::vector<Point>& points, const Arc& arc) {
  Polyline_traits traits;
  const auto& geodesic_traits = *traits.subcurve_traits_2();
  auto polyline = arcs_and_polyline(traits, points).second;
  auto cmp_y_at_x = traits.compare_y_at_x_2_object();
  auto geodesic_cmp_y_at_x = geodesic_traits.compare_y_at_x_2_object();
  for (double z : {0.3, 0.5, 0.7}) {
    auto p = point_on_identification(geodesic_traits, z);
    auto res = cmp_y_at_x(p, polyline);
    auto expected = geodesic_cmp_y_at_x(p, arc);
    if (res == expected) continue;
    std::cerr << "Compare_y_at_x_2(p, polyline) = " << res << ", expected " << expected << std::endl;
    return false;
  }
  return true;
}

/* Check whether two x-monotone polylines have the same endpoints. */
bool same_endpoints(const Polyline_traits& traits, const Polyline& xcv1, const Polyline& xcv2) {
  auto equal = traits.subcurve_traits_2()->equal_2_object();
  auto min_vertex = traits.construct_min_vertex_2_object();
  auto max_vertex = traits.construct_max_vertex_2_object();
  return equal(min_vertex(xcv1), min_vertex(xcv2)) && equal(max_vertex(xcv1), max_vertex(xcv2));
}

/* Check whether an x-monotone polyline and an arc have the same endpoints. */
bool same_endpoints(const Polyline_traits& traits, const Polyline& xcv, const Arc& arc) {
  const auto& geodesic_traits = *traits.subcurve_traits_2();
  auto equal = geodesic_traits.equal_2_object();
  return equal(traits.construct_min_vertex_2_object()(xcv), geodesic_traits.construct_min_vertex_2_object()(arc)) &&
    equal(traits.construct_max_vertex_2_object()(xcv), geodesic_traits.construct_max_vertex_2_object()(arc));
}

/* Split the identification curve at the given point on it, and obtain the two
 * arcs.
 */
std::pair<Arc, Arc> split_identification_arc(const Geodesic_traits& traits, const Point& q) {
  Arc arc1, arc2;
  traits.split_2_object()(CGAL::draw_aos::construct_identification_curve(traits), q, arc1, arc2);
  return std::make_pair(arc1, arc2);
}

/* Test the splitting of the identification curve at a point on it. The parts
 * must be the parts of the split identification arc.
 */
bool test_split(const Point& q) {
  Polyline_traits traits;
  auto [arc1, arc2] = split_identification_arc(*traits.subcurve_traits_2(), q);
  Polyline xcv1, xcv2;
  traits.split_2_object()(CGAL::draw_aos::construct_identification_curve(traits), q, xcv1, xcv2);
  return same_endpoints(traits, xcv1, arc1) && same_endpoints(traits, xcv2, arc2);
}

/* Test whether a polyline intersects the identification curve. A polyline
 * that reaches the identification curve meets it at a point in the interior
 * of the identification curve; thus, they intersect whether common endpoints
 * are considered or not.
 */
bool test_do_intersect(const std::vector<Point>& points, bool reaches) {
  Polyline_traits traits;
  auto polyline = arcs_and_polyline(traits, points).second;
  auto id_polyline = CGAL::draw_aos::construct_identification_curve(traits);
  auto do_intersect = traits.do_intersect_2_object();
  for (bool common : {true, false}) {
    bool expected = reaches;
    if ((do_intersect(polyline, id_polyline, common) == expected) &&
        (do_intersect(id_polyline, polyline, common) == expected))
      continue;
    std::cerr << "Do_intersect_2 (common endpoints = " << common << ") != " << expected << std::endl;
    return false;
  }
  return true;
}

/* Test the intersection of two polylines that lie on the identification curve;
 * one consists of the two parts of the identification curve split at the given
 * point, and the other is either the entire identification curve or its lower
 * part. The intersection must be a single curve that overlaps the latter.
 */
bool test_overlap(const Point& q, bool entire) {
  Polyline_traits traits;
  auto [arc1, arc2] = split_identification_arc(*traits.subcurve_traits_2(), q);
  std::vector<Arc> arcs = {arc1, arc2};
  auto ctr_xcv = traits.construct_x_monotone_curve_2_object();
  auto split_polyline = ctr_xcv(arcs.begin(), arcs.end());
  auto other = entire ? CGAL::draw_aos::construct_identification_curve(traits) : ctr_xcv(arc1);

  using Intersection_point = std::pair<Point, Polyline_traits::Multiplicity>;
  using Intersection_result = std::variant<Intersection_point, Polyline>;
  for (bool split_first : {true, false}) {
    std::list<Intersection_result> results;
    if (split_first) traits.intersect_2_object()(split_polyline, other, std::back_inserter(results));
    else traits.intersect_2_object()(other, split_polyline, std::back_inserter(results));
    const Polyline* overlap = (results.size() == 1) ? std::get_if<Polyline>(&results.front()) : nullptr;
    if ((overlap != nullptr) && same_endpoints(traits, *overlap, other)) continue;
    std::cerr << "The intersection is not the overlapping curve; " << results.size() << " results" << std::endl;
    return false;
  }
  return true;
}

/* Construct a closed loop of two polylines that meet at a point on the
 * identification curve and at a point away from it. The first polyline is
 * directed left to right from the point away from the identification curve,
 * through the identification point, that is, the point opposite to the
 * identification curve; it reaches the identification curve from the right
 * side. The second polyline leaves the identification curve from the left side
 * and ends at the first point of the first polyline. The x-ranges of the two
 * polylines in the parameter space overlap, although they share an endpoint on
 * the identification curve.
 */
std::pair<Polyline, Polyline> closing_polylines(const Polyline_traits& traits) {
  const Geodesic_traits& geodesic_traits = *traits.subcurve_traits_2();
  const double z = -0.6;
  auto q = point_on_identification(geodesic_traits, z);
  std::vector<Point> chain = {point_at(geodesic_traits, 4.0, z), point_at(geodesic_traits, 0.36, z),
                              point_at(geodesic_traits, 2.36, z), q};
  auto xcv1 = arcs_and_polyline(traits, chain).second;
  auto xcv2 = arcs_and_polyline(traits, {q, chain.front()}).second;
  return std::make_pair(xcv1, xcv2);
}

/* Insert the two polylines that form a closed loop and meet at a point on the
 * identification curve incrementally, in either order. Check that the
 * arrangement consists of two vertices and two edges.
 */
bool test_closing_polyline(bool closing_last) {
  Polyline_traits traits;
  Polyline_arrangement arr(&traits);
  auto [xcv1, xcv2] = closing_polylines(traits);
  if (closing_last) {
    CGAL::insert(arr, xcv1);
    CGAL::insert(arr, xcv2);
  }
  else {
    CGAL::insert(arr, xcv2);
    CGAL::insert(arr, xcv1);
  }
  if (! arr.is_valid()) return false;
  if ((arr.number_of_vertices() == 2) && (arr.number_of_edges() == 2)) return true;
  std::cerr << "The arrangement has " << arr.number_of_vertices() << " vertices and " << arr.number_of_edges()
            << " edges, expected 2 and 2" << std::endl;
  return false;
}

/* Compute the zone of the polyline that closes the loop in the arrangement of
 * the other polyline. Check that the zone contains the vertex at the far
 * endpoint of the closing polyline.
 */
bool test_closing_zone() {
  Polyline_traits traits;
  Polyline_arrangement arr(&traits);
  auto [xcv1, xcv2] = closing_polylines(traits);
  CGAL::insert(arr, xcv1);
  using Zone_feature = std::variant<Polyline_arrangement::Vertex_handle, Polyline_arrangement::Halfedge_handle,
                                    Polyline_arrangement::Face_handle>;
  std::vector<Zone_feature> features;
  CGAL::zone(arr, xcv2, std::back_inserter(features));
  const auto& p = traits.construct_max_vertex_2_object()(xcv2);
  auto equal = traits.equal_2_object();
  for (const auto& feature : features) {
    auto* vh = std::get_if<Polyline_arrangement::Vertex_handle>(&feature);
    if ((vh != nullptr) && equal((*vh)->point(), p)) return true;
  }
  std::cerr << "The zone does not contain the vertex at the endpoint of the curve" << std::endl;
  return false;
}

/* Insert incrementally an x-monotone curve that lies on the identification
 * curve and a curve that crosses the identification curve in its interior, in
 * either order. The crossing curve is subdivided into two x-monotone curves
 * that end at the identification curve, and the curve on the identification
 * curve must be split at the crossing point. Thus, the arrangement must
 * consist of five vertices and four edges.
 */
template <typename Gt>
bool test_crossing_identification_arc(bool identification_first) {
  using Arrangement = CGAL::Arrangement_on_surface_2<Gt, CGAL::Arr_spherical_topology_traits_2<Gt>>;
  Geodesic_traits geodesic_traits;
  auto ctr_p = geodesic_traits.construct_point_2_object();
  auto crossing = geodesic_traits.construct_curve_2_object()(ctr_p(std::cos(2.9), std::sin(2.9), 0.1),
                                                             ctr_p(std::cos(3.4), std::sin(3.4), -0.1));
  auto on_identification = geodesic_traits.construct_x_monotone_curve_2_object()
    (point_on_identification(geodesic_traits, -0.5), point_on_identification(geodesic_traits, 0.5));
  Gt traits;
  Arrangement arr(&traits);
  typename Gt::Curve_2 cv;
  typename Gt::X_monotone_curve_2 xcv;
  if constexpr (std::is_same_v<Gt, Geodesic_traits>) {
    cv = crossing;
    xcv = on_identification;
  }
  else {
    cv = traits.construct_curve_2_object()(crossing);
    xcv = traits.construct_x_monotone_curve_2_object()(on_identification);
  }
  if (identification_first) {
    CGAL::insert(arr, xcv);
    CGAL::insert(arr, cv);
  }
  else {
    CGAL::insert(arr, cv);
    CGAL::insert(arr, xcv);
  }
  if (! arr.is_valid()) return false;
  if ((arr.number_of_vertices() == 5) && (arr.number_of_edges() == 4)) return true;
  std::cerr << "The arrangement has " << arr.number_of_vertices() << " vertices and " << arr.number_of_edges()
            << " edges, expected 5 and 4" << std::endl;
  return false;
}

/* Convert a geodesic arc to an x-monotone curve of the given traits. */
template <typename Gt>
typename Gt::X_monotone_curve_2 to_x_monotone_curve(const Gt& traits, const Arc& arc) {
  if constexpr (std::is_same_v<Gt, Geodesic_traits>) return arc;
  else return traits.construct_x_monotone_curve_2_object()(arc);
}

/* Insert incrementally an x-monotone curve that lies on the identification
 * curve, and then an x-monotone curve that ends at a point in its interior,
 * either reaching the point from the right boundary or leaving it from the
 * left boundary. The curve on the identification curve must be split at the
 * point. Thus, the arrangement must consist of four vertices and three edges.
 */
template <typename Gt>
bool test_curve_end_on_identification_arc(bool from_right) {
  using Arrangement = CGAL::Arrangement_on_surface_2<Gt, CGAL::Arr_spherical_topology_traits_2<Gt>>;
  Geodesic_traits geodesic_traits;
  auto ctr_arc = geodesic_traits.construct_x_monotone_curve_2_object();
  auto q = point_on_identification(geodesic_traits, 0.0);
  auto arc = from_right ? ctr_arc(point_at(geodesic_traits, 2.9, 0.0), q) : ctr_arc(q, point_at(geodesic_traits, 3.4, 0.0));
  auto on_identification =
    ctr_arc(point_on_identification(geodesic_traits, -0.5), point_on_identification(geodesic_traits, 0.5));
  Gt traits;
  Arrangement arr(&traits);
  CGAL::insert(arr, to_x_monotone_curve(traits, on_identification));
  CGAL::insert(arr, to_x_monotone_curve(traits, arc));
  if (! arr.is_valid()) return false;
  if ((arr.number_of_vertices() == 4) && (arr.number_of_edges() == 3)) return true;
  std::cerr << "The arrangement has " << arr.number_of_vertices() << " vertices and " << arr.number_of_edges()
            << " edges, expected 4 and 3" << std::endl;
  return false;
}

/* Insert incrementally an x-monotone curve that lies on the identification
 * curve and a closed loop (a small circle parallel to the XY-plane) that
 * crosses it, in either order. The loop splits the sphere into two faces.
 * Check that the arrangement has two faces and that the numbers of vertices and
 * edges do not depend on the order of insertion.
 */
template <typename Gt>
bool test_loop_crossing_identification_arc(const std::vector<Point>& loop) {
  using Arrangement = CGAL::Arrangement_on_surface_2<Gt, CGAL::Arr_spherical_topology_traits_2<Gt>>;
  Geodesic_traits geodesic_traits;
  auto on_identification = geodesic_traits.construct_x_monotone_curve_2_object()
    (point_on_identification(geodesic_traits, -0.5), point_on_identification(geodesic_traits, 0.5));
  std::array<std::size_t, 2> vertices, edges;
  bool ok = true;
  for (int identification_first = 0; identification_first < 2; ++identification_first) {
    Gt traits;
    Arrangement arr(&traits);
    std::vector<typename Gt::Curve_2> curves;
    if constexpr (std::is_same_v<Gt, Geodesic_traits>) {
      for (std::size_t i = 0; i + 1 < loop.size(); ++i)
        curves.push_back(traits.construct_curve_2_object()(loop[i], loop[i + 1]));
    }
    else curves.push_back(traits.construct_curve_2_object()(loop.begin(), loop.end()));
    auto xcv = to_x_monotone_curve(traits, on_identification);
    if (identification_first == 1) CGAL::insert(arr, xcv);
    for (const auto& cv : curves) CGAL::insert(arr, cv);
    if (identification_first == 0) CGAL::insert(arr, xcv);
    if (! arr.is_valid()) return false;
    if (arr.number_of_faces() != 2) {
      std::cerr << "The arrangement has " << arr.number_of_faces() << " faces, expected 2" << std::endl;
      ok = false;
    }
    vertices[identification_first] = arr.number_of_vertices();
    edges[identification_first] = arr.number_of_edges();
  }
  if ((vertices[0] == vertices[1]) && (edges[0] == edges[1])) return ok;
  std::cerr << "The arrangement has " << vertices[1] << " vertices and " << edges[1] << " edges, expected "
            << vertices[0] << " and " << edges[0] << std::endl;
  return false;
}

int main() {
  bool ok = true;
  Geodesic_traits geodesic_traits;

  // Arrangements of polylines that cross the identification curve
  auto loops = small_circles(geodesic_traits, 16);
  ok &= run("aggregate insertion", [&]() { return test_arrangement(loops, false, false); });
  ok &= run("incremental insertion", [&]() { return test_arrangement(loops, true, false); });
  ok &= run("aggregate insertion and the identification curve",
            [&]() { return test_arrangement(loops, false, true); });

  // Polylines that form a closed loop and meet on the identification curve
  ok &= run("incremental insertion of a polyline that closes a loop on the identification curve",
            [&]() { return test_closing_polyline(true); });
  ok &= run("incremental insertion of a polyline closed by a loop on the identification curve",
            [&]() { return test_closing_polyline(false); });
  ok &= run("zone of a polyline that closes a loop on the identification curve",
            [&]() { return test_closing_zone(); });

  // A curve on the identification curve and a curve that crosses it
  ok &= run("incremental insertion of an arc on the identification curve and a crossing curve",
            [&]() { return test_crossing_identification_arc<Geodesic_traits>(true); });
  ok &= run("incremental insertion of a crossing curve and an arc on the identification curve",
            [&]() { return test_crossing_identification_arc<Geodesic_traits>(false); });
  ok &= run("incremental insertion of a polyline on the identification curve and a crossing polyline",
            [&]() { return test_crossing_identification_arc<Polyline_traits>(true); });
  ok &= run("incremental insertion of a crossing polyline and a polyline on the identification curve",
            [&]() { return test_crossing_identification_arc<Polyline_traits>(false); });
  ok &= run("incremental insertion of an arc reaching an arc on the identification curve from the right",
            [&]() { return test_curve_end_on_identification_arc<Geodesic_traits>(true); });
  ok &= run("incremental insertion of an arc leaving an arc on the identification curve from the left",
            [&]() { return test_curve_end_on_identification_arc<Geodesic_traits>(false); });
  ok &= run("incremental insertion of a polyline reaching a polyline on the identification curve from the right",
            [&]() { return test_curve_end_on_identification_arc<Polyline_traits>(true); });
  ok &= run("incremental insertion of a polyline leaving a polyline on the identification curve from the left",
            [&]() { return test_curve_end_on_identification_arc<Polyline_traits>(false); });
  // The small circle at z = 0.2 crosses the arc on the identification curve.
  ok &= run("incremental insertion of an arc on the identification curve and a loop of arcs",
            [&]() { return test_loop_crossing_identification_arc<Geodesic_traits>(loops[1]); });
  ok &= run("incremental insertion of a polyline on the identification curve and a closed polyline",
            [&]() { return test_loop_crossing_identification_arc<Polyline_traits>(loops[1]); });

  // Polylines that reach the identification curve at either end, directed
  // either left to right or right to left
  auto q = point_on_identification(geodesic_traits);
  std::vector<Point> before = {point_at(geodesic_traits, 2.6), point_at(geodesic_traits, 2.9), q};
  std::vector<Point> after = {q, point_at(geodesic_traits, 3.4), point_at(geodesic_traits, 3.7)};
  std::vector<std::pair<std::string, std::vector<Point>>> polylines = {
    {"polyline ending at the identification curve", before},
    {"polyline starting at the identification curve", after},
    {"reversed polyline ending at the identification curve", std::vector<Point>(before.rbegin(), before.rend())},
    {"reversed polyline starting at the identification curve", std::vector<Point>(after.rbegin(), after.rend())}};
  if (! geodesic_traits.is_on_y_identification_2_object()(q)) {
    std::cerr << "Error: the point is not on the identification curve" << std::endl;
    return EXIT_FAILURE;
  }
  auto ctr_arc = geodesic_traits.construct_x_monotone_curve_2_object();
  for (const auto& [name, points] : polylines) {
    ok &= run("intersection of a " + name + " with the identification curve",
              [&]() { return test_intersection(points, true); });
    ok &= run("intersection of the identification curve with a " + name,
              [&]() { return test_intersection(points, false); });
    // The arc that reaches the identification curve
    auto arc = (points.front() == q) ? ctr_arc(points[0], points[1]) :
      ctr_arc(points[points.size() - 2], points.back());
    ok &= run("comparison of points on the identification curve with a " + name,
              [&]() { return test_compare_y_at_x(points, arc); });
    ok &= run("do-intersect of a " + name + " and the identification curve",
              [&]() { return test_do_intersect(points, true); });
  }

  // Polylines that lie on the identification curve
  ok &= run("splitting the identification curve", [&]() { return test_split(q); });
  ok &= run("intersection of polylines on the identification curve",
            [&]() { return test_overlap(q, true); });
  ok &= run("intersection of polylines on the identification curve, one partial",
            [&]() { return test_overlap(q, false); });

  // A polyline that does not reach the identification curve
  std::vector<Point> away = {point_at(geodesic_traits, 2.0), point_at(geodesic_traits, 2.3),
                             point_at(geodesic_traits, 2.6)};
  ok &= run("do-intersect of a polyline away from the identification curve",
            [&]() { return test_do_intersect(away, false); });

  if (! ok) return EXIT_FAILURE;
  std::cout << "Passed" << std::endl;
  return EXIT_SUCCESS;
}
