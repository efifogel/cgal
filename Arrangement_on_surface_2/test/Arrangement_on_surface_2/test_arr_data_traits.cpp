// Tests Arr_data_traits_2: the construction of arrangements of labeled
// segments, the merging of point data and of curve data, and the shortcuts
// that the labels enable, by comparing the arrangements with arrangements
// constructed with the base traits.

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_data_traits_2.h>
#include <CGAL/Arr_label_policy_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Random.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point = Kernel::Point_2;
using Vector = Kernel::Vector_2;
using Segment_traits = CGAL::Arr_segment_traits_2<Kernel>;
using Labels = std::vector<int>;

// merges two sorted label containers.
Labels unite(const Labels& labels1, const Labels& labels2) {
  Labels labels;
  std::set_union(labels1.begin(), labels1.end(), labels2.begin(), labels2.end(), std::back_inserter(labels));
  return labels;
}

// determines whether two sorted label containers share a label.
bool share(const Labels& labels1, const Labels& labels2) {
  auto it1 = labels1.begin();
  auto it2 = labels2.begin();
  while ((it1 != labels1.end()) && (it2 != labels2.end())) {
    if (*it1 == *it2) return true;
    if (*it1 < *it2) ++it1;
    else ++it2;
  }
  return false;
}

// A policy that labels every input point and every input segment; equal
// labels guarantee equal points, and segments that share an endpoint label and
// are not collinear are interior disjoint.
struct Label_policy : public CGAL::Arr_data_policy_2<Labels, Labels> {
  Labels merge_curve_data(const Labels& labels1, const Labels& labels2) const { return unite(labels1, labels2); }
  Labels merge_point_data(const Labels& labels1, const Labels& labels2) const { return unite(labels1, labels2); }
  bool are_identical(const Labels& labels1, const Labels& labels2) const { return share(labels1, labels2); }

  template <typename GeometryTraits_2, typename XMonotoneCurve_2>
  bool are_interior_disjoint(const GeometryTraits_2& traits, const XMonotoneCurve_2& xcv1,
                             const XMonotoneCurve_2& xcv2) const {
    for (auto ce1 : {CGAL::ARR_MIN_END, CGAL::ARR_MAX_END}) {
      for (auto ce2 : {CGAL::ARR_MIN_END, CGAL::ARR_MAX_END}) {
        if (! share(xcv1.end_data(ce1), xcv2.end_data(ce2))) continue;
        const auto& s = (ce1 == CGAL::ARR_MIN_END) ? xcv1.left() : xcv1.right();
        const auto& p = (ce1 == CGAL::ARR_MIN_END) ? xcv1.right() : xcv1.left();
        const auto& q = (ce2 == CGAL::ARR_MIN_END) ? xcv2.right() : xcv2.left();
        return traits.orientation_2_object()(p, s, q) != CGAL::COLLINEAR;
      }
    }
    return false;
  }
};

using Data_traits = CGAL::Arr_data_traits_2<Segment_traits, Label_policy>;
using Data_arrangement = CGAL::Arrangement_2<Data_traits>;
using Arrangement = CGAL::Arrangement_2<Segment_traits>;
using Data_point = Data_traits::Point_2;
using Data_x_monotone_curve = Data_traits::X_monotone_curve_2;

// A labeled input point.
struct Input_point {
  Point point;
  int label;
};

// generates closed polygonal cycles whose vertices are constructed points
// (sums of points and vectors) on a coarse grid, so that vertices of different
// cycles coincide, and segments overlap.
std::vector<std::vector<Input_point>> random_cycles(std::size_t num_cycles, std::size_t size, int grid,
                                                     CGAL::Random& rnd) {
  std::vector<std::vector<Input_point>> cycles;
  int label = 0;
  for (std::size_t c = 0; c < num_cycles; ++c) {
    std::vector<Input_point> cycle;
    while (cycle.size() < size) {
      Point p = Point(rnd.get_int(0, grid), rnd.get_int(0, grid)) + Vector(rnd.get_int(0, 2), rnd.get_int(0, 2));
      if (! cycle.empty() && (p == cycle.back().point)) continue;
      cycle.push_back(Input_point{p, label++});
    }
    if (cycle.front().point == cycle.back().point) cycle.pop_back();
    cycles.push_back(cycle);
  }
  return cycles;
}

// constructs the labeled segments of the given cycles.
std::vector<Data_x_monotone_curve> labeled_segments(const std::vector<std::vector<Input_point>>& cycles) {
  const Data_traits traits;
  auto ctr = traits.construct_x_monotone_curve_2_object();
  std::vector<Data_x_monotone_curve> xcvs;
  int label = 0;
  for (const auto& cycle : cycles) {
    for (std::size_t i = 0; i < cycle.size(); ++i) {
      const auto& p = cycle[i];
      const auto& q = cycle[(i + 1) % cycle.size()];
      xcvs.push_back(ctr(Data_point(p.point, Labels{p.label}), Data_point(q.point, Labels{q.label}),
                         Labels{label++}));
    }
  }
  return xcvs;
}

// constructs the plain segments of the given cycles.
std::vector<Segment_traits::X_monotone_curve_2> plain_segments(const std::vector<std::vector<Input_point>>& cycles) {
  std::vector<Segment_traits::X_monotone_curve_2> xcvs;
  for (const auto& cycle : cycles)
    for (std::size_t i = 0; i < cycle.size(); ++i)
      xcvs.emplace_back(cycle[i].point, cycle[(i + 1) % cycle.size()].point);
  return xcvs;
}

// verifies that the arrangements have the same combinatorial structure, that
// the data of every vertex are the labels of the input points that coincide
// with it, and that the data of every edge are the labels of the input
// segments that contain it.
bool verify(const std::string& name, const Data_arrangement& data_arr, const Arrangement& arr,
            const std::vector<std::vector<Input_point>>& cycles) {
  bool ok = (data_arr.number_of_vertices() == arr.number_of_vertices()) &&
    (data_arr.number_of_edges() == arr.number_of_edges()) && (data_arr.number_of_faces() == arr.number_of_faces());
  if (! ok) std::cerr << "  different arrangements" << std::endl;

  for (auto vit = data_arr.vertices_begin(); vit != data_arr.vertices_end(); ++vit) {
    Labels expected;
    for (const auto& cycle : cycles)
      for (const auto& ip : cycle)
        if (ip.point == vit->point()) expected.push_back(ip.label);
    std::sort(expected.begin(), expected.end());
    if (vit->point().data() != expected) {
      std::cerr << "  wrong data of vertex " << vit->point() << std::endl;
      ok = false;
    }
  }

  Segment_traits traits;
  auto is_in_x_range = traits.is_in_x_range_2_object();
  auto compare_y_at_x = traits.compare_y_at_x_2_object();
  for (auto eit = data_arr.edges_begin(); eit != data_arr.edges_end(); ++eit) {
    const auto& xcv = eit->curve();
    const Point mid = CGAL::midpoint(xcv.left(), xcv.right());
    Labels expected;
    int label = 0;
    for (const auto& cycle : cycles) {
      for (std::size_t i = 0; i < cycle.size(); ++i, ++label) {
        Segment_traits::X_monotone_curve_2 seg(cycle[i].point, cycle[(i + 1) % cycle.size()].point);
        if (is_in_x_range(seg, mid) && (compare_y_at_x(mid, seg) == CGAL::EQUAL)) expected.push_back(label);
      }
    }
    if (xcv.data() != expected) {
      std::cerr << "  wrong data of edge " << xcv << std::endl;
      ok = false;
    }
    // The data of the endpoints of the edge must be contained in the data of
    // its vertices (which may also contain the data of coinciding points).
    const auto& left = (eit->source()->point() == xcv.left()) ? eit->source() : eit->target();
    const auto& right = (left == eit->source()) ? eit->target() : eit->source();
    const auto& left_data = left->point().data();
    const auto& right_data = right->point().data();
    const auto& min_data = xcv.end_data(CGAL::ARR_MIN_END);
    const auto& max_data = xcv.end_data(CGAL::ARR_MAX_END);
    if (! std::includes(left_data.begin(), left_data.end(), min_data.begin(), min_data.end()) ||
        ! std::includes(right_data.begin(), right_data.end(), max_data.begin(), max_data.end())) {
      std::cerr << "  wrong data of the endpoints of edge " << xcv << std::endl;
      ok = false;
    }
  }
  std::cout << (ok ? "Passed: " : "Failed: ") << name << " (" << data_arr.number_of_vertices() << " vertices, "
            << data_arr.number_of_edges() << " edges)" << std::endl;
  return ok;
}

// constructs the arrangement of all cycles at once.
bool test_construction(const std::string& name, const std::vector<std::vector<Input_point>>& cycles) {
  const auto xcvs = labeled_segments(cycles);
  Data_arrangement data_arr;
  CGAL::insert(data_arr, xcvs.begin(), xcvs.end());
  const auto segs = plain_segments(cycles);
  Arrangement arr;
  CGAL::insert(arr, segs.begin(), segs.end());
  return verify(name, data_arr, arr, cycles);
}

// constructs the arrangement of the first half of the cycles, and inserts the
// second half into it.
bool test_insertion(const std::string& name, const std::vector<std::vector<Input_point>>& cycles) {
  const auto xcvs = labeled_segments(cycles);
  std::size_t half = 0;
  for (std::size_t c = 0; c < cycles.size() / 2; ++c) half += cycles[c].size();
  Data_arrangement data_arr;
  CGAL::insert(data_arr, xcvs.begin(), xcvs.begin() + half);
  CGAL::insert(data_arr, xcvs.begin() + half, xcvs.end());
  const auto segs = plain_segments(cycles);
  Arrangement arr;
  CGAL::insert(arr, segs.begin(), segs.end());
  return verify(name, data_arr, arr, cycles);
}

// tests the functors directly.
bool test_functors() {
  const Data_traits traits;
  auto ctr = traits.construct_x_monotone_curve_2_object();
  const Data_point a(Point(0, 0), Labels{0}), b(Point(2, 0), Labels{1}), c(Point(4, 0), Labels{2});
  const Data_point d(Point(2, 2), Labels{3}), e(Point(1, 0), Labels{4});
  bool ok = true;

  // Equal labels decide equality without geometry; different labels do not.
  ok = ok && (traits.compare_xy_2_object()(a, Data_point(Point(0, 0), Labels{0})) == CGAL::EQUAL);
  ok = ok && (traits.compare_xy_2_object()(a, Data_point(Point(0, 0), Labels{9})) == CGAL::EQUAL);
  ok = ok && (traits.compare_xy_2_object()(a, b) == CGAL::SMALLER);
  ok = ok && traits.equal_2_object()(a, Data_point(Point(0, 0)));

  // The endpoint data follow the xy-order of the endpoints.
  const auto ba = ctr(b, a, Labels{10});
  ok = ok && (ba.end_data(CGAL::ARR_MIN_END) == Labels{0}) && (ba.end_data(CGAL::ARR_MAX_END) == Labels{1});
  ok = ok && (traits.construct_min_vertex_2_object()(ba).data() == Labels{0});
  const auto opp = traits.construct_opposite_2_object()(ba);
  ok = ok && (opp.data() == Labels{10}) && (opp.end_data(CGAL::ARR_MIN_END) == Labels{0});

  // Adjacent non-collinear segments report their common endpoint only.
  const auto bd = ctr(b, d, Labels{11});
  std::vector<std::variant<std::pair<Data_point, std::size_t>, Data_x_monotone_curve>> xs;
  traits.intersect_2_object()(ba, bd, std::back_inserter(xs));
  ok = ok && (xs.size() == 1);
  if (ok) {
    const auto* ip = std::get_if<std::pair<Data_point, std::size_t>>(&xs.front());
    ok = (ip != nullptr) && (ip->first == Point(2, 0)) && (ip->first.data() == Labels{1});
  }

  // Overlapping segments: the overlap obtains the merged curve data, and the
  // data of the coinciding endpoints.
  const auto ac = ctr(a, c, Labels{12});
  const auto eb = ctr(e, Data_point(Point(2, 0), Labels{5}), Labels{13});
  xs.clear();
  traits.intersect_2_object()(ac, ba, std::back_inserter(xs));
  ok = ok && (xs.size() == 1);
  if (ok) {
    const auto* ovl = std::get_if<Data_x_monotone_curve>(&xs.front());
    ok = (ovl != nullptr) && (ovl->data() == (Labels{10, 12})) && (ovl->end_data(CGAL::ARR_MIN_END) == Labels{0}) &&
      (ovl->end_data(CGAL::ARR_MAX_END) == Labels{1});
  }
  xs.clear();
  traits.intersect_2_object()(eb, ba, std::back_inserter(xs));
  ok = ok && (xs.size() == 1);
  if (ok) {
    const auto* ovl = std::get_if<Data_x_monotone_curve>(&xs.front());
    ok = (ovl != nullptr) && (ovl->end_data(CGAL::ARR_MIN_END) == Labels{4}) &&
      (ovl->end_data(CGAL::ARR_MAX_END) == (Labels{1, 5}));
  }

  // Splitting: the split point becomes the common endpoint of the subcurves.
  Data_x_monotone_curve xcv1, xcv2;
  traits.split_2_object()(ac, b, xcv1, xcv2);
  ok = ok && (xcv1.data() == Labels{12}) && (xcv2.data() == Labels{12}) &&
    (xcv1.end_data(CGAL::ARR_MIN_END) == Labels{0}) && (xcv1.end_data(CGAL::ARR_MAX_END) == Labels{1}) &&
    (xcv2.end_data(CGAL::ARR_MIN_END) == Labels{1}) && (xcv2.end_data(CGAL::ARR_MAX_END) == Labels{2});

  // Merging point data.
  ok = ok && (traits.merge_point_data_2_object()(a, Data_point(Point(0, 0), Labels{7})).data() == (Labels{0, 7}));

  std::cout << (ok ? "Passed: " : "Failed: ") << "functors" << std::endl;
  return ok;
}

// tests the default policy with polylines, which are subdivided into
// x-monotone polylines.
bool test_default_policy() {
  using Polyline_traits = CGAL::Arr_polyline_traits_2<Segment_traits>;
  using Traits = CGAL::Arr_data_traits_2<Polyline_traits, CGAL::Arr_data_policy_2<int, int>>;
  Traits traits;
  Polyline_traits polyline_traits;
  auto ctr = polyline_traits.construct_curve_2_object();
  std::vector<Point> points1 = {Point(0, 0), Point(4, 4), Point(0, 4), Point(4, 0)};
  std::vector<Point> points2 = {Point(0, 2), Point(4, 2), Point(2, 6)};
  std::vector<Traits::Curve_2> cvs = {Traits::Curve_2(ctr(points1.begin(), points1.end()), 1),
                                      Traits::Curve_2(ctr(points2.begin(), points2.end()), 2)};
  CGAL::Arrangement_2<Traits> arr(&traits);
  CGAL::insert(arr, cvs.begin(), cvs.end());
  CGAL::Arrangement_2<Polyline_traits> base_arr;
  std::vector<Polyline_traits::Curve_2> base_cvs = {ctr(points1.begin(), points1.end()),
                                                    ctr(points2.begin(), points2.end())};
  CGAL::insert(base_arr, base_cvs.begin(), base_cvs.end());
  bool ok = (arr.number_of_vertices() == base_arr.number_of_vertices()) &&
    (arr.number_of_edges() == base_arr.number_of_edges()) && (arr.number_of_faces() == base_arr.number_of_faces());
  for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit)
    if ((eit->curve().data() != 1) && (eit->curve().data() != 2)) ok = false;
  std::cout << (ok ? "Passed: " : "Failed: ") << "default policy with polylines" << std::endl;
  return ok;
}

// A policy that marks intersection points with a special label.
struct Intersection_label_policy : public Label_policy {
  template <typename XMonotoneCurve_2>
  Labels intersection_point_data(const XMonotoneCurve_2& /* xcv1 */, const XMonotoneCurve_2& /* xcv2 */) const
  { return Labels{-1}; }
};

// tests the merging of the data of an intersection point that coincides with
// an endpoint of another segment, in construction and in insertion.
bool test_intersection_point_data() {
  using Traits = CGAL::Arr_data_traits_2<Segment_traits, Intersection_label_policy>;
  using Arr = CGAL::Arrangement_2<Traits>;
  using Pnt = Traits::Point_2;
  const Traits traits;
  auto ctr = traits.construct_x_monotone_curve_2_object();
  const std::vector<Traits::X_monotone_curve_2> xcvs = {
    ctr(Pnt(Point(2, 2), Labels{5}), Pnt(Point(6, 2), Labels{6}), Labels{2}),
    ctr(Pnt(Point(0, 0), Labels{1}), Pnt(Point(4, 4), Labels{2}), Labels{0}),
    ctr(Pnt(Point(0, 4), Labels{3}), Pnt(Point(4, 0), Labels{4}), Labels{1})
  };
  auto data_at = [](const Arr& arr, const Point& p) {
    for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit)
      if (vit->point() == p) return vit->point().data();
    return Labels{-2};
  };
  Arr arr1(&traits);
  CGAL::insert(arr1, xcvs.begin(), xcvs.end());
  Arr arr2(&traits);
  CGAL::insert(arr2, xcvs.begin(), xcvs.begin() + 1);
  CGAL::insert(arr2, xcvs.begin() + 1, xcvs.end());
  const bool ok = (data_at(arr1, Point(2, 2)) == (Labels{-1, 5})) && (data_at(arr2, Point(2, 2)) == (Labels{-1, 5}));
  std::cout << (ok ? "Passed: " : "Failed: ") << "intersection point data" << std::endl;
  return ok;
}

// tests the merging of the data of an intersection point at the endpoint of
// one of the two intersecting segments (a weak intersection), in construction
// and in insertion.
bool test_weak_intersection_point_data() {
  using Traits = CGAL::Arr_data_traits_2<Segment_traits, Intersection_label_policy>;
  using Arr = CGAL::Arrangement_2<Traits>;
  using Pnt = Traits::Point_2;
  const Traits traits;
  auto ctr = traits.construct_x_monotone_curve_2_object();
  const std::vector<Traits::X_monotone_curve_2> xcvs = {
    ctr(Pnt(Point(0, 0), Labels{1}), Pnt(Point(4, 0), Labels{2}), Labels{0}),
    ctr(Pnt(Point(1, -1), Labels{3}), Pnt(Point(2, 0), Labels{4}), Labels{1})
  };
  auto data_at = [](const Arr& arr, const Point& p) {
    for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit)
      if (vit->point() == p) return vit->point().data();
    return Labels{-2};
  };
  Arr arr1(&traits);
  CGAL::insert(arr1, xcvs.begin(), xcvs.end());
  Arr arr2(&traits);
  CGAL::insert(arr2, xcvs.begin(), xcvs.begin() + 1);
  CGAL::insert(arr2, xcvs.begin() + 1, xcvs.end());
  const bool ok = (data_at(arr1, Point(2, 0)) == (Labels{-1, 4})) && (data_at(arr2, Point(2, 0)) == (Labels{-1, 4}));
  std::cout << (ok ? "Passed: " : "Failed: ") << "weak intersection point data" << std::endl;
  return ok;
}

// tests Arr_label_policy_2 for segments: the arrangement of labeled cycles is
// compared with the arrangement constructed with the base traits. With
// multiple labels, the label set of every vertex must consist of the labels of
// the input points that coincide with it; with a single label, the label of
// every vertex must be one of these labels, or empty if there are none.
template <typename MultipleLabelsTag>
bool test_label_policy(const std::string& name, const std::vector<std::vector<Input_point>>& cycles) {
  using Policy = CGAL::Arr_label_policy_2<int, CGAL::Tag_true, CGAL::_Default_merge_func<int>, int,
                                          CGAL::_Default_convert_func<int, int>, std::size_t, MultipleLabelsTag>;
  using Traits = CGAL::Arr_data_traits_2<Segment_traits, Policy>;
  using Arr = CGAL::Arrangement_2<Traits>;
  using Pnt = typename Traits::Point_2;
  const Traits traits;
  auto ctr = traits.construct_x_monotone_curve_2_object();
  std::vector<typename Traits::X_monotone_curve_2> xcvs;
  for (const auto& cycle : cycles) {
    for (std::size_t i = 0; i < cycle.size(); ++i) {
      const auto& p = cycle[i];
      const auto& q = cycle[(i + 1) % cycle.size()];
      xcvs.push_back(ctr(Pnt(p.point, {std::size_t(p.label)}), Pnt(q.point, {std::size_t(q.label)}), 0));
    }
  }
  Arr data_arr(&traits);
  CGAL::insert(data_arr, xcvs.begin(), xcvs.end());
  const auto segs = plain_segments(cycles);
  Arrangement arr;
  CGAL::insert(arr, segs.begin(), segs.end());
  bool ok = (data_arr.number_of_vertices() == arr.number_of_vertices()) &&
    (data_arr.number_of_edges() == arr.number_of_edges()) && (data_arr.number_of_faces() == arr.number_of_faces());
  for (auto vit = data_arr.vertices_begin(); vit != data_arr.vertices_end(); ++vit) {
    typename Policy::Label_set expected;
    for (const auto& cycle : cycles)
      for (const auto& ip : cycle)
        if (ip.point == vit->point()) expected.push_back(std::size_t(ip.label));
    std::sort(expected.begin(), expected.end());
    const auto& data = vit->point().data();
    if constexpr (Policy::multiple_labels) {
      if (data != expected) ok = false;
    }
    else if (expected.empty() ? ! data.is_empty() :
             (data.is_empty() || ! std::binary_search(expected.begin(), expected.end(), data.label())))
      ok = false;
  }
  std::cout << (ok ? "Passed: " : "Failed: ") << name << std::endl;
  return ok;
}

int main() {
  std::size_t failures = 0;
  if (! test_functors()) ++failures;
  if (! test_default_policy()) ++failures;
  if (! test_intersection_point_data()) ++failures;
  if (! test_weak_intersection_point_data()) ++failures;

  // A triangle and a square that share a vertex and an edge part.
  const std::vector<std::vector<Input_point>> simple = {
    {{Point(0, 0), 0}, {Point(4, 0), 1}, {Point(2, 3), 2}},
    {{Point(2, 0), 3}, {Point(6, 0), 4}, {Point(6, 3), 5}, {Point(2, 3), 6}}
  };
  if (! test_construction("triangle and square", simple)) ++failures;
  if (! test_insertion("triangle and square, insertion", simple)) ++failures;

  CGAL::Random rnd(17);
  for (int i = 0; i < 20; ++i) {
    const auto cycles = random_cycles(2 + rnd.get_int(0, 4), 3 + rnd.get_int(0, 10), 3 + rnd.get_int(0, 6), rnd);
    if (! test_construction("random " + std::to_string(i), cycles)) ++failures;
    if (! test_insertion("random " + std::to_string(i) + ", insertion", cycles)) ++failures;
    if (! test_label_policy<CGAL::Tag_true>("random " + std::to_string(i) + ", label policy", cycles)) ++failures;
    if (! test_label_policy<CGAL::Tag_false>("random " + std::to_string(i) + ", single-label policy", cycles))
      ++failures;
  }

  std::cout << failures << " failure(s)" << std::endl;
  return (failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
