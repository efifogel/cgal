/* Test the incremental insertion (the zone) of linear curves that overlap
 * existing curves up to the open boundary, e.g., the line y = x and the ray
 * that emanates from (-1,-1) to the left along it.
 *
 * When the left end of the inserted curve is unbounded and coincides with the
 * end of an existing curve, the overlap starts at the vertex at infinity of
 * that curve. The zone used to report the overlap without its left vertex, and
 * the insertion visitor then tried to split the overlapped edge at the left
 * endpoint of the overlap, which does not exist (a precondition violation in
 * Construct_min_vertex_2). Overlaps that reach the open boundary at their
 * right (or top) end, and bounded overlaps, are tested as well.
 *
 * Every set of curves is inserted incrementally in the given order and in the
 * reverse order, and aggregately (the surface sweep); all arrangements must be
 * valid and have the expected sizes.
 */

#include <iostream>
#include <string>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_linear_traits_2.h>
#include <CGAL/Arrangement_2.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Traits_2 = CGAL::Arr_linear_traits_2<Kernel>;
using Point_2 = Kernel::Point_2;
using Segment_2 = Kernel::Segment_2;
using Ray_2 = Kernel::Ray_2;
using Line_2 = Kernel::Line_2;
using X_monotone_curve_2 = Traits_2::X_monotone_curve_2;
using Arrangement_2 = CGAL::Arrangement_2<Traits_2>;
using Size = Arrangement_2::Size;

struct Test_case {
  std::string name;
  std::vector<X_monotone_curve_2> curves;
  Size vertices, vertices_at_infinity, edges, faces;
};

/*! checks an arrangement against the expected sizes.
 */
bool check(const Test_case& tc, const std::string& how, const Arrangement_2& arr) {
  const Size sizes[] = {arr.number_of_vertices(), arr.number_of_vertices_at_infinity(), arr.number_of_edges(),
                        arr.number_of_faces()};
  const Size expected[] = {tc.vertices, tc.vertices_at_infinity, tc.edges, tc.faces};
  bool ok = arr.is_valid();
  for (int i = 0; i < 4; ++i) ok = ok && (sizes[i] == expected[i]);
  if (ok) return true;
  std::cerr << "Error: " << tc.name << " (" << how << "): valid " << arr.is_valid() << "; " << sizes[0]
            << " vertices, " << sizes[1] << " at infinity, " << sizes[2] << " edges, " << sizes[3]
            << " faces; expected " << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3]
            << std::endl;
  return false;
}

int main() {
  const Point_2 o(0, 0), p(1, 1), q(-1, -1), r(-2, -2), s(-3, -3), t(2, 2), u(0, 1), w(1, 0), d(0, -1);
  const std::vector<Test_case> cases = {
    // overlaps that reach the left or bottom boundary
    {"a line and a ray to the left on it", {Line_2(o, p), Ray_2(q, r)}, 1, 2, 2, 2},
    {"a line and the same line", {Line_2(o, p), Line_2(t, Point_2(3, 3))}, 0, 2, 1, 2},
    {"two rays to the left on a line", {Ray_2(o, q), Ray_2(q, r)}, 2, 1, 2, 1},
    {"a vertical line and a vertical ray down on it", {Line_2(o, u), Ray_2(o, d)}, 1, 2, 2, 2},
    {"a line, a ray to the left on it, and another line", {Line_2(o, p), Ray_2(q, r), Line_2(u, w)}, 2, 4, 5, 4},
    // overlaps that reach the right or top boundary, and bounded overlaps
    {"a line and a ray to the right on it", {Line_2(o, p), Ray_2(p, t)}, 1, 2, 2, 2},
    {"a vertical line and a vertical ray up on it", {Line_2(o, u), Ray_2(o, u)}, 1, 2, 2, 2},
    {"a ray to the left and a segment on it", {Ray_2(o, q), Segment_2(s, q)}, 3, 1, 3, 1}};

  // constructs an arrangement and checks it; a failure of CGAL (e.g., a violated precondition) is reported
  auto run = [](const Test_case& tc, const std::string& how, auto construct) {
    try {
      Arrangement_2 arr;
      construct(arr);
      return check(tc, how, arr);
    }
    catch (const CGAL::Failure_exception& e) {
      std::cerr << "Error: " << tc.name << " (" << how << "): " << e.what() << std::endl;
      return false;
    }
  };

  bool ok = true;
  for (const auto& tc : cases) {
    ok = run(tc, "incremental", [&](Arrangement_2& arr) { for (const auto& cv : tc.curves) CGAL::insert(arr, cv); })
         && ok;
    ok = run(tc, "incremental, reverse order", [&](Arrangement_2& arr) {
           for (auto it = tc.curves.rbegin(); it != tc.curves.rend(); ++it) CGAL::insert(arr, *it);
         }) && ok;
    ok = run(tc, "aggregate", [&](Arrangement_2& arr) { CGAL::insert(arr, tc.curves.begin(), tc.curves.end()); })
         && ok;
  }

  std::cout << (ok ? "passed" : "failed") << std::endl;
  return ok ? 0 : 1;
}
