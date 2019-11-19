// Copyright (c) 2008 INRIA Sophia-Antipolis (France).
// Copyright (c) 2008-2015 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Pierre Alliez, Laurent Rineau, Ilker O. Yaz

// compute self-intersection of a CGAL triangle polyhedron mesh
// original code from Lutz Kettner

#ifndef CGAL_POLYGON_MESH_PROCESSING_SELF_INTERSECTIONS
#define CGAL_POLYGON_MESH_PROCESSING_SELF_INTERSECTIONS

#include <CGAL/license/Polygon_mesh_processing/predicate.h>

#include <CGAL/disable_warnings.h>

#include <CGAL/box_intersection_d.h>
#include <CGAL/intersections.h>
#include <CGAL/Bbox_3.h>

#ifdef CGAL_PMP_SI_DEBUG
#include <CGAL/Real_timer.h>
#endif

#include <CGAL/Kernel/global_functions_3.h>

#include <sstream>
#include <vector>
#include <exception>
#include <boost/range.hpp>

#include <boost/function_output_iterator.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/graph/graph_traits.hpp>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/boost/graph/properties.h>

#include <CGAL/Polygon_mesh_processing/internal/named_function_params.h>
#include <CGAL/Polygon_mesh_processing/internal/named_params_helper.h>

#ifdef CGAL_LINKED_WITH_TBB
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_vector.h>
#endif

#ifdef DOXYGEN_RUNNING
#define CGAL_PMP_NP_TEMPLATE_PARAMETERS NamedParameters
#define CGAL_PMP_NP_CLASS NamedParameters
#endif

namespace CGAL {
namespace internal {

template <class TM,//TriangleMesh
          class Kernel,
          class Box,
          class OutputIterator,
          class VertexPointMap>
struct Intersect_facets
{
// typedefs
  typedef typename Kernel::Segment_3    Segment;
  typedef typename Kernel::Triangle_3   Triangle;
  typedef typename boost::graph_traits<TM>::halfedge_descriptor halfedge_descriptor;
  typedef typename boost::graph_traits<TM>::vertex_descriptor vertex_descriptor;
  typedef typename boost::graph_traits<TM>::face_descriptor face_descriptor;
  typedef typename boost::property_map<TM, boost::vertex_point_t>::const_type Ppmap;

// members
  const TM& m_tmesh;
  const VertexPointMap m_vpmap;
  mutable OutputIterator  m_iterator;

  typename Kernel::Construct_segment_3  segment_functor;
  typename Kernel::Construct_triangle_3 triangle_functor;
  typename Kernel::Do_intersect_3       do_intersect_3_functor;

  Intersect_facets(const TM& tmesh, OutputIterator it, VertexPointMap vpmap, const Kernel& kernel)
    :
    m_tmesh(tmesh),
    m_vpmap(vpmap),
    m_iterator(it),
    segment_functor(kernel.construct_segment_3_object()),
    triangle_functor(kernel.construct_triangle_3_object()),
    do_intersect_3_functor(kernel.do_intersect_3_object())
  {}

  void operator()(const Box* b, const Box* c) const
  {
    halfedge_descriptor h = halfedge(b->info(), m_tmesh);
    halfedge_descriptor g = halfedge(c->info(), m_tmesh);

    vertex_descriptor hv[3], gv[3];
    hv[0] = target(h, m_tmesh);
    hv[1] = target(next(h, m_tmesh), m_tmesh);
    hv[2] = source(h, m_tmesh);

    gv[0] = target(g, m_tmesh);
    gv[1] = target(next(g, m_tmesh), m_tmesh);
    gv[2] = source(g, m_tmesh);

    halfedge_descriptor opp_h;

    // check for shared egde
    for(unsigned int i=0; i<3; ++i){
      opp_h = opposite(h, m_tmesh);
      if(face(opp_h, m_tmesh) == c->info()){
        // there is an intersection if the four points are coplanar and
        // the triangles overlap
        get(m_vpmap, hv[i]);
        get(m_vpmap, hv[(i + 1) % 3]);
        get(m_vpmap, hv[(i + 2) % 3]);
        get(m_vpmap, target(next(opp_h, m_tmesh), m_tmesh));

        if(CGAL::coplanar(get(m_vpmap, hv[i]),
                          get(m_vpmap, hv[(i+1)%3]),
                          get(m_vpmap, hv[(i+2)%3]),
                          get(m_vpmap, target(next(opp_h, m_tmesh), m_tmesh))) &&
           CGAL::coplanar_orientation(get(m_vpmap, hv[(i+2)%3]),
                                      get(m_vpmap, hv[i]),
                                      get(m_vpmap, hv[(i+1)%3]),
                                      get(m_vpmap, target(next(opp_h, m_tmesh), m_tmesh)))
             == CGAL::POSITIVE){
          *m_iterator++ = std::make_pair(b->info(), c->info());
          return;
        } else { // there is a shared edge but no intersection
          return;
        }
      }
      h = next(h, m_tmesh);
    }

    // check for shared vertex --> maybe intersection, maybe not

    halfedge_descriptor v;

    int i(0),j(0);
    bool shared = false;
    for(; i < 3 && (! shared); ++i){
      for(j = 0; j < 3 && (! shared); ++j){
        if(hv[i]==gv[j]){
          shared = true;
          break;
        }
      }
      if (shared) {
        break;
      }
    }
    if(shared){
      // found shared vertex:
      CGAL_assertion(hv[i] == gv[j]);

      // geometric check if the opposite segments intersect the triangles
      Triangle t1 = triangle_functor( get(m_vpmap, hv[0]),
                                      get(m_vpmap, hv[1]),
                                      get(m_vpmap, hv[2]));
      Triangle t2 = triangle_functor( get(m_vpmap, gv[0]),
                                      get(m_vpmap, gv[1]),
                                      get(m_vpmap, gv[2]));

      Segment s1 = segment_functor( get(m_vpmap, hv[(i+1)%3]),
                                    get(m_vpmap, hv[(i+2)%3]) );
      Segment s2 = segment_functor( get(m_vpmap, gv[(j+1)%3]),
                                    get(m_vpmap, gv[(j+2)%3]));

      if(do_intersect_3_functor(t1,s2)){
        *m_iterator++ = std::make_pair(b->info(), c->info());
      } else if(do_intersect_3_functor(t2,s1)){
        *m_iterator++ = std::make_pair(b->info(), c->info());
      }
      return;
    }

    // check for geometric intersection
    Triangle t1 = triangle_functor( get(m_vpmap, hv[0]),
                                    get(m_vpmap, hv[1]),
                                    get(m_vpmap, hv[2]));
    Triangle t2 = triangle_functor( get(m_vpmap, gv[0]),
                                    get(m_vpmap, gv[1]),
                                    get(m_vpmap, gv[2]));
    if(do_intersect_3_functor(t1, t2)){
      *m_iterator++ = std::make_pair(b->info(), c->info());
    }
  } // end operator ()
}; // end struct Intersect_facets

  
#ifdef CGAL_LINKED_WITH_TBB
//TODO: use the same code for the linear pass?
// The functor for doing all geometric tests in parallel
template <class TM,//TriangleMesh
          class FacePairs,
          class Kernel,
          class VertexPointMap>
struct AllPairs
{
// types
  typedef typename Kernel::Segment_3    Segment;
  typedef typename Kernel::Triangle_3   Triangle;
  typedef typename boost::graph_traits<TM>::halfedge_descriptor halfedge_descriptor;
  typedef typename boost::graph_traits<TM>::vertex_descriptor vertex_descriptor;
  typedef typename boost::graph_traits<TM>::face_descriptor face_descriptor;

// data members
  const TM& m_tmesh;
  const VertexPointMap m_vpmap;
  const FacePairs& face_pairs;
  std::vector<int>& dointersect;
  typename Kernel::Construct_segment_3  segment_functor;
  typename Kernel::Construct_triangle_3 triangle_functor;
  typename Kernel::Do_intersect_3       do_intersect_3_functor;


  AllPairs(const FacePairs& face_pairs,
           std::vector<int>& dointersect,
           const TM& tmesh, VertexPointMap vpmap, const Kernel& kernel)
    : m_tmesh(tmesh),
      m_vpmap(vpmap),
      face_pairs(face_pairs),
      dointersect(dointersect),
      triangle_functor(kernel.construct_triangle_3_object()),
      do_intersect_3_functor(kernel.do_intersect_3_object())
  {}

  void operator()(const tbb::blocked_range<std::size_t> &r) const
  {
    for (std::size_t ri = r.begin(); ri != r.end(); ++ri) {
      this->operator()(ri);
    }
  }

  void operator()(std::size_t ri) const
  {
    const std::pair<face_descriptor,face_descriptor>& ff = face_pairs[ri];
    halfedge_descriptor h = halfedge(ff.first, m_tmesh), g = halfedge(ff.second, m_tmesh);

    vertex_descriptor hv[3], gv[3];
    hv[0] = target(h, m_tmesh);
    hv[1] = target(next(h, m_tmesh), m_tmesh);
    hv[2] = source(h, m_tmesh);

    gv[0] = target(g, m_tmesh);
    gv[1] = target(next(g, m_tmesh), m_tmesh);
    gv[2] = source(g, m_tmesh);

    halfedge_descriptor opp_h;

    // check for shared egde
    for(unsigned int i=0; i<3; ++i){
      opp_h = opposite(h, m_tmesh);
      if(face(opp_h, m_tmesh) == ff.second){
        // there is an intersection if the four points are coplanar and
        // the triangles overlap
        get(m_vpmap, hv[i]);
        get(m_vpmap, hv[(i + 1) % 3]);
        get(m_vpmap, hv[(i + 2) % 3]);
        get(m_vpmap, target(next(opp_h, m_tmesh), m_tmesh));

        if(CGAL::coplanar(get(m_vpmap, hv[i]),
                          get(m_vpmap, hv[(i+1)%3]),
                          get(m_vpmap, hv[(i+2)%3]),
                          get(m_vpmap, target(next(opp_h, m_tmesh), m_tmesh))) &&
           CGAL::coplanar_orientation(get(m_vpmap, hv[(i+2)%3]),
                                      get(m_vpmap, hv[i]),
                                      get(m_vpmap, hv[(i+1)%3]),
                                      get(m_vpmap, target(next(opp_h, m_tmesh), m_tmesh)))
             == CGAL::POSITIVE){
          dointersect[ri]=true;
          return;
        } else { // there is a shared edge but no intersection
          return;
        }
      }
      h = next(h, m_tmesh);
    }

    // check for shared vertex --> maybe intersection, maybe not

    halfedge_descriptor v;

    int i(0),j(0);
    bool shared = false;
    for(; i < 3 && (! shared); ++i){
      for(j = 0; j < 3 && (! shared); ++j){
        if(hv[i]==gv[j]){
          shared = true;
          break;
        }
      }
      if (shared) {
        break;
      }
    }
    if(shared){
      // found shared vertex:
      CGAL_assertion(hv[i] == gv[j]);

      // geometric check if the opposite segments intersect the triangles
      Triangle t1 = triangle_functor( get(m_vpmap, hv[0]),
                                      get(m_vpmap, hv[1]),
                                      get(m_vpmap, hv[2]));
      Triangle t2 = triangle_functor( get(m_vpmap, gv[0]),
                                      get(m_vpmap, gv[1]),
                                      get(m_vpmap, gv[2]));

      Segment s1 = segment_functor( get(m_vpmap, hv[(i+1)%3]),
                                    get(m_vpmap, hv[(i+2)%3]) );
      Segment s2 = segment_functor( get(m_vpmap, gv[(j+1)%3]),
                                    get(m_vpmap, gv[(j+2)%3]));

	  dointersect[ri] = (do_intersect_3_functor(t1, s2)) || do_intersect_3_functor(t2, s1);
      return;
    }

    // check for geometric intersection
    Triangle t1 = triangle_functor( get(m_vpmap, hv[0]),
                                    get(m_vpmap, hv[1]),
                                    get(m_vpmap, hv[2]));
    Triangle t2 = triangle_functor( get(m_vpmap, gv[0]),
                                    get(m_vpmap, gv[1]),
                                    get(m_vpmap, gv[2]));
    dointersect[ri] = do_intersect_3_functor(t1, t2);
  }
};


// The functor that filters nothing
template <class OutputIterator>
struct All_faces_filter
{
  mutable OutputIterator  m_iterator;

  All_faces_filter(OutputIterator it) :  m_iterator(it) {}

  template <class Box>
  void operator()(const Box* b, const Box* c) const
  {
     *m_iterator ++ = std::make_pair(b->info(), c->info());
  }
};


#endif

struct Throw_at_output {
  class Throw_at_output_exception: public std::exception
  { };

  template<class T>
  void operator()(const T& /* t */) const {
    throw Throw_at_output_exception();
  }
};

}// namespace internal

namespace Polygon_mesh_processing {

/*!
 * \ingroup PMP_intersection_grp
 * collects intersections between a subset of faces of a triangulated surface mesh.
 * Two faces are said to intersect if the corresponding triangles intersect
 * and the intersection is not an edge nor a vertex incident to both faces.
 *
 * This function depends on the package \ref PkgBoxIntersectionD
 *
 * @pre `CGAL::is_triangle_mesh(tmesh)`
 *
 * @tparam ConcurrencyTag enables sequential versus parallel algorithm.
 *                        Possible values are `Sequential_tag`, `Parallel_tag`, and `Parallel_if_available_tag`.
 * @tparam FaceRange range of `boost::graph_traits<TriangleMesh>::%face_descriptor`,
 *  model of `Range`.
 * Its iterator type is `RandomAccessIterator`.
 * @tparam TriangleMesh a model of `FaceListGraph`
 * @tparam OutputIterator a model of `OutputIterator` holding objects of type
 *   `std::pair<boost::graph_traits<TriangleMesh>::%face_descriptor, boost::graph_traits<TriangleMesh>::%face_descriptor>`
 * @tparam NamedParameters a sequence of \ref pmp_namedparameters "Named Parameters"
 *
 * @param face_range the range of faces to check for self-intersection.
 * @param tmesh the triangulated surface mesh to be checked
 * @param out output iterator to be filled with all pairs of non-adjacent faces that intersect
 * @param np optional sequence of \ref pmp_namedparameters "Named Parameters" among the ones listed below
 *
 * \cgalNamedParamsBegin
 *    \cgalParamBegin{vertex_point_map} the property map with the points associated to the vertices of `pmesh`.
 *   If this parameter is omitted, an internal property map for
 *   `CGAL::vertex_point_t` must be available in `TriangleMesh`\cgalParamEnd
 *    \cgalParamBegin{geom_traits} an instance of a geometric traits class, model of `PMPSelfIntersectionTraits` \cgalParamEnd
 * \cgalNamedParamsEnd
 */
template < class ConcurrencyTag = Sequential_tag,
           class TriangleMesh,
           class FaceRange,
           class OutputIterator,
           class NamedParameters>
OutputIterator
self_intersections( const FaceRange& face_range,
                    const TriangleMesh& tmesh,
                          OutputIterator out,
                    const NamedParameters& np)
{
  CGAL_precondition(CGAL::is_triangle_mesh(tmesh));

  typedef TriangleMesh                                                                   TM;
  typedef typename boost::graph_traits<TM>::face_descriptor                              face_descriptor;

  typedef CGAL::Box_intersection_d::ID_FROM_BOX_ADDRESS                                  Box_policy;
  typedef CGAL::Box_intersection_d::Box_with_info_d<double, 3, face_descriptor, Box_policy> Box;

  typedef typename GetGeomTraits<TM, NamedParameters>::type                              GeomTraits;
  GeomTraits gt = parameters::choose_parameter(parameters::get_parameter(np, internal_np::geom_traits), GeomTraits());

  typedef typename GetVertexPointMap<TM, NamedParameters>::const_type VertexPointMap;
  VertexPointMap vpmap = parameters::choose_parameter(parameters::get_parameter(np, internal_np::vertex_point),
                                                      get_const_property_map(boost::vertex_point, tmesh));

  // make one box per face
  std::vector<Box> boxes;
  boxes.reserve(std::distance(boost::begin(face_range), boost::end(face_range)));

  for(face_descriptor f : face_range)
  {
    typename boost::property_traits<VertexPointMap>::reference
      p = get(vpmap, target(halfedge(f,tmesh),tmesh)),
      q = get(vpmap, target(next(halfedge(f, tmesh), tmesh), tmesh)),
      r = get(vpmap, target(next(next(halfedge(f, tmesh), tmesh), tmesh), tmesh));

    if ( collinear(p, q, r) )
      *out++= std::make_pair(f,f);
    else
      boxes.push_back(Box(p.bbox() + q.bbox() + r.bbox(), f));
  }

  // generate box pointers
  std::vector<const Box*> box_ptr;
  box_ptr.reserve(boxes.size());

  for(Box& b : boxes)
    box_ptr.push_back(&b);

#if !defined(CGAL_LINKED_WITH_TBB)
  CGAL_static_assertion_msg (!(boost::is_convertible<ConcurrencyTag, Parallel_tag>::value),
                             "Parallel_tag is enabled but TBB is unavailable.");
#else
  if (boost::is_convertible<ConcurrencyTag, Parallel_tag>::value)
  {
    // (Parallel version of the code)
    // (A) Sequentially write all pairs of faces with intersecting bbox into a std::vector
    std::ptrdiff_t cutoff = 2000;
   
    typedef tbb::concurrent_vector<std::pair<face_descriptor, face_descriptor> >FacePairs;

    typedef std::back_insert_iterator<FacePairs> FacePairsI;
    FacePairs face_pairs;
    CGAL::internal::All_faces_filter<FacePairsI> all_faces_filter(std::back_inserter(face_pairs));
    CGAL::box_self_intersection_d<ConcurrencyTag>(box_ptr.begin(), box_ptr.end(), all_faces_filter, cutoff);

    // (B) Parallel: perform the geometric tests
    std::vector<int> dointersect(face_pairs.size(), 0);
    CGAL::internal::AllPairs<TM,FacePairs,GeomTraits,VertexPointMap> all_pairs(face_pairs, dointersect, tmesh, vpmap, gt);

    tbb::parallel_for(tbb::blocked_range<std::size_t>(0, face_pairs.size()), all_pairs);

    // (C) Sequentially: Copy from the concurent container to the output iterator
    for(std::size_t i=0; i < dointersect.size(); ++i){
      if(dointersect[i])
        *out ++= face_pairs[i];
    }
    return out;
  }
#endif

  // (Sequential version of the code) compute self-intersections filtered out by boxes
  CGAL::internal::Intersect_facets<TM, GeomTraits, Box, OutputIterator, VertexPointMap>
    intersect_facets(tmesh, out, vpmap, gt);

  std::ptrdiff_t cutoff = 2000;
  CGAL::box_self_intersection_d(box_ptr.begin(), box_ptr.end(),intersect_facets, cutoff);
  return intersect_facets.m_iterator;
}

/// \cond SKIP_IN_MANUAL
template <class ConcurrencyTag = Sequential_tag,
          class TriangleMesh,
          class FaceRange,
          class OutputIterator>
OutputIterator
self_intersections(const FaceRange& face_range,
                   const TriangleMesh& tmesh,
                         OutputIterator out)
{
  return self_intersections<ConcurrencyTag>(face_range, tmesh, out, CGAL::parameters::all_default());
}
/// \endcond


/**
 * \ingroup PMP_intersection_grp
 * collects intersections between all the faces of a triangulated surface mesh.
 * Two faces are said to intersect if the corresponding triangles intersect
 * and the intersection is not an edge nor a vertex incident to both faces.
 *
 * This function depends on the package \ref PkgBoxIntersectionD
 *
 * @pre `CGAL::is_triangle_mesh(tmesh)`
 *
 * @tparam ConcurrencyTag enables sequential versus parallel algorithm.
 *                         Possible values are `Sequential_tag`, `Parallel_tag`, and `Parallel_if_available_tag`.
 * @tparam TriangleMesh a model of `FaceListGraph`
 * @tparam OutputIterator a model of `OutputIterator` holding objects of type
 *   `std::pair<boost::graph_traits<TriangleMesh>::%face_descriptor, boost::graph_traits<TriangleMesh>::%face_descriptor>`
 * @tparam NamedParameters a sequence of \ref pmp_namedparameters "Named Parameters"
 *
 * @param tmesh the triangulated surface mesh to be checked
 * @param out output iterator to be filled with all pairs of non-adjacent faces that intersect.
              In case `tmesh` contains some degenerate faces, for each degenerate face `f` a pair `(f,f)`
              will be put in `out` before any other self intersection between non-degenerate faces.
              These are the only pairs where degenerate faces will be reported.
 * @param np optional sequence of \ref pmp_namedparameters "Named Parameters" among the ones listed below
 *
 * \cgalNamedParamsBegin
 *    \cgalParamBegin{vertex_point_map} the property map with the points associated to the vertices of `pmesh`.
 *   If this parameter is omitted, an internal property map for
 *   `CGAL::vertex_point_t` must be available in `TriangleMesh`\cgalParamEnd
 *    \cgalParamBegin{geom_traits} an instance of a geometric traits class, model of `PMPSelfIntersectionTraits` \cgalParamEnd
 * \cgalNamedParamsEnd
 *
 * @return `out`
 */
template <class ConcurrencyTag = Sequential_tag,
          class TriangleMesh,
          class OutputIterator,
          class CGAL_PMP_NP_TEMPLATE_PARAMETERS>
OutputIterator
self_intersections(const TriangleMesh& tmesh,
                          OutputIterator out,
                   const CGAL_PMP_NP_CLASS& np)
{
  return self_intersections<ConcurrencyTag>(faces(tmesh), tmesh, out, np);
}

/// \cond SKIP_IN_MANUAL
template <class ConcurrencyTag = Sequential_tag, class TriangleMesh, class OutputIterator>
OutputIterator
self_intersections(const TriangleMesh& tmesh, OutputIterator out)
{
  return self_intersections<ConcurrencyTag>(faces(tmesh), tmesh, out, parameters::all_default());
}
/// \endcond

/**
 * \ingroup PMP_intersection_grp
 * tests if a triangulated surface mesh self-intersects.
 * This function depends on the package \ref PkgBoxIntersectionD
 * @pre `CGAL::is_triangle_mesh(tmesh)`
 *
 * @tparam ConcurrencyTag enables sequential versus parallel algorithm.
 *                        Possible values are `Sequential_tag`, `Parallel_tag`, and `Parallel_if_available_tag`.
 * @tparam TriangleMesh a model of `FaceListGraph`
 * @tparam NamedParameters a sequence of \ref pmp_namedparameters "Named Parameters"
 *
 * @param tmesh the triangulated surface mesh to be tested
 * @param np optional sequence of \ref pmp_namedparameters "Named Parameters" among the ones listed below
 *
 * \cgalNamedParamsBegin
 *    \cgalParamBegin{vertex_point_map} the property map with the points associated to the vertices of `tmesh`.
 *   If this parameter is omitted, an internal property map for
 *   `CGAL::vertex_point_t` must be available in `TriangleMesh`\cgalParamEnd
 *    \cgalParamBegin{geom_traits} an instance of a geometric traits class, model of `PMPSelfIntersectionTraits` \cgalParamEnd
 * \cgalNamedParamsEnd
 *
 * @return `true` if `tmesh` self-intersects
 */
template <class ConcurrencyTag = Sequential_tag,
          class TriangleMesh,
          class CGAL_PMP_NP_TEMPLATE_PARAMETERS>
bool does_self_intersect(const TriangleMesh& tmesh,
                         const CGAL_PMP_NP_CLASS& np)
{
  CGAL_precondition(CGAL::is_triangle_mesh(tmesh));

  try
  {
    typedef boost::function_output_iterator<CGAL::internal::Throw_at_output> OutputIterator;
    self_intersections<ConcurrencyTag>(tmesh, OutputIterator(), np);
  }
  catch( CGAL::internal::Throw_at_output::Throw_at_output_exception& )
  { return true; }

  return false;
}

/**
 * \ingroup PMP_intersection_grp
 * tests if a set of faces of a triangulated surface mesh self-intersects.
 * This function depends on the package \ref PkgBoxIntersectionD
 * @pre `CGAL::is_triangle_mesh(tmesh)`
 *
 * @tparam ConcurrencyTag enables sequential versus parallel algorithm.
 *                        Possible values are `Sequential_tag`, `Parallel_tag`, and `Parallel_if_available_tag`.
 * @tparam FaceRange a range of `face_descriptor`
 * @tparam TriangleMesh a model of `FaceListGraph`
 * @tparam NamedParameters a sequence of \ref pmp_namedparameters "Named Parameters"
 *
 * @param face_range the set of faces to test for self-intersection
 * @param tmesh the triangulated surface mesh to be tested
 * @param np optional sequence of \ref pmp_namedparameters "Named Parameters" among the ones listed below
 *
 * \cgalNamedParamsBegin
 *    \cgalParamBegin{vertex_point_map} the property map with the points associated to the vertices of `tmesh`.
 *   If this parameter is omitted, an internal property map for
 *   `CGAL::vertex_point_t` must be available in `TriangleMesh`\cgalParamEnd
 *    \cgalParamBegin{geom_traits} an instance of a geometric traits class, model of `SelfIntersectionTraits` \cgalParamEnd
 * \cgalNamedParamsEnd
 *
 * @return `true` if the faces in `face_range` self-intersect
 */
template <class ConcurrencyTag = Sequential_tag,
          class FaceRange,
          class TriangleMesh,
          class NamedParameters>
bool does_self_intersect(const FaceRange& face_range,
                         const TriangleMesh& tmesh,
                         const NamedParameters& np)
{
  CGAL_precondition(CGAL::is_triangle_mesh(tmesh));

  try
  {
    typedef boost::function_output_iterator<CGAL::internal::Throw_at_output> OutputIterator;
    self_intersections<ConcurrencyTag>(face_range, tmesh, OutputIterator(), np);
  }
  catch( CGAL::internal::Throw_at_output::Throw_at_output_exception& )
  { return true; }

  return false;
}

/// \cond SKIP_IN_MANUAL
template <class ConcurrencyTag = Sequential_tag, class TriangleMesh>
bool does_self_intersect(const TriangleMesh& tmesh)
{
  return does_self_intersect<ConcurrencyTag>(tmesh, CGAL::parameters::all_default());
}

template <class ConcurrencyTag = Sequential_tag, class FaceRange, class TriangleMesh>
bool does_self_intersect(const FaceRange& face_range,
                         const TriangleMesh& tmesh)
{
  return does_self_intersect<ConcurrencyTag>(face_range, tmesh, CGAL::parameters::all_default());
}
/// \endcond

}// end namespace Polygon_mesh_processing

}// namespace CGAL

#include <CGAL/enable_warnings.h>

#endif // CGAL_POLYGON_MESH_PROCESSING_SELF_INTERSECTIONS
