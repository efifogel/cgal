namespace CGAL {
namespace Minkowski_sum_2 {

/*!
 * \ingroup PkgMinkowskiSum2Ref
 *
 * provides a guaranteed approximation of the offset of the given polygon `P`
 * by a given radius `r`, namely, the Minkowski sum \f$ P \oplus B_r\f$, where
 * \f$ B_r\f$ is a disc of radius `r` centered at the origin. The function
 * actually outputs a set \f$ S\f$ that contains the Minkowski sum, such that
 * the approximation error is bounded by `eps`. Note that as the input polygon
 * may not be convex, its offset may not be a simple polygon. The result is
 * therefore represented as a polygon with holes, whose edges are either line
 * segments or circular arcs.
 *
 * By default, the offset is computed with the convolution method. If the
 * named parameter `method` is `CGAL::Minkowski_sum_2::Method::DECOMPOSITION`,
 * the polygon is decomposed into convex sub-polygons
 * \f$ P_1, \ldots, P_k\f$, unless it is convex, and the union of the
 * sub-offsets (namely \f$ \bigcup_{i}{(P_i \oplus B_r)}\f$) is computed.
 *
 * \param P the polygon
 * \param r the offset radius
 * \param eps the approximation-error bound
 * \param np1 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the polygon among the ones
 *        listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{is_convex}
 *     \cgalParamDescription{a hint whether the polygon is convex}
 *     \cgalParamType{`bool`}
 *     \cgalParamDefault{none; the convexity of the polygon is tested where needed}
 *     \cgalParamExtra{If the polygon is hinted to be convex (or is found to be convex), it is not
 *                     decomposed by the decomposition method; if the hint is `true` and expensive checks are
 *                     enabled (`CGAL_CHECK_EXPENSIVE`), the convexity of the polygon is verified.}
 *   \cgalParamNEnd
 *   \cgalParamNBegin{decomposition_strategy}
 *     \cgalParamDescription{the strategy that decomposes the polygon into convex sub-polygons, unless it is convex}
 *     \cgalParamType{a model of the concept `PolygonConvexDecomposition_2`, `std::reference_wrapper<const Strategy>`,
 *                    or `std::shared_ptr<const Strategy>`, where `Strategy` is a model of this concept}
 *     \cgalParamDefault{`CGAL::Small_side_angle_bisector_decomposition_2<Kernel, Container>`}
 *     \cgalParamExtra{Used only by the decomposition method. An instance is copied; a reference (passed as
 *                     `std::cref(strategy)`) and a shared pointer are not.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \param np an optional sequence of \ref bgl_namedparameters "Named Parameters" of the computation among the
 *        ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{method}
 *     \cgalParamDescription{the method that computes the offset}
 *     \cgalParamType{`CGAL::Minkowski_sum_2::Method`}
 *     \cgalParamDefault{`CGAL::Minkowski_sum_2::Method::AUTOMATIC`}
 *     \cgalParamExtra{`CGAL::Minkowski_sum_2::Method::AUTOMATIC` and
 *                     `CGAL::Minkowski_sum_2::Method::FULL_CONVOLUTION` apply the convolution method;
 *                     `CGAL::Minkowski_sum_2::Method::DECOMPOSITION` applies the decomposition method, which
 *                     supports only simple polygons. `CGAL::Minkowski_sum_2::Method::REDUCED_CONVOLUTION` is
 *                     not supported.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \return the approximated offset polygon
 *
 * \pre `P` is a simple polygon.
 */
template <typename Kernel, typename Container,
          typename NamedParameters1, typename NamedParameters>
typename Gps_circle_segment_traits_2<Kernel>::Polygon_with_holes_2
approximated_offset_2(const Polygon_2<Kernel, Container>& P, const typename Kernel::FT& r, const double& eps,
                      const NamedParameters1& np1 = parameters::default_values(),
                      const NamedParameters& np = parameters::default_values());

/*!
 * \ingroup PkgMinkowskiSum2Ref
 *
 * provides a guaranteed approximation of the offset of the given polygon with
 * holes `pwh` by a given radius `r`, such that the approximation error is
 * bounded by `eps`. It does so by offsetting the outer boundary of `pwh` and
 * insetting its holes, with the convolution method. The result is represented
 * as a generalized polygon with holes, whose edges are line segments and
 * circular arcs.
 *
 * \param pwh the polygon with holes
 * \param r the offset radius
 * \param eps the approximation-error bound
 * \param np1 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the polygon among the ones
 *        listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{is_convex}
 *     \cgalParamDescription{a hint whether the polygon is convex}
 *     \cgalParamType{`bool`}
 *     \cgalParamDefault{none; the convexity of the polygon is tested where needed}
 *     \cgalParamExtra{The hint does not affect the offset; if it is `true` and expensive checks are
 *                     enabled (`CGAL_CHECK_EXPENSIVE`), the convexity of the polygon is verified.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \param np an optional sequence of \ref bgl_namedparameters "Named Parameters" of the computation among the
 *        ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{method}
 *     \cgalParamDescription{the method that computes the offset}
 *     \cgalParamType{`CGAL::Minkowski_sum_2::Method`}
 *     \cgalParamDefault{`CGAL::Minkowski_sum_2::Method::AUTOMATIC`}
 *     \cgalParamExtra{Only the convolution method is supported (`CGAL::Minkowski_sum_2::Method::AUTOMATIC`
 *                     or `CGAL::Minkowski_sum_2::Method::FULL_CONVOLUTION`).}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \return the approximated offset polygon
 *
 * \pre `pwh` is bounded (it has an outer boundary).
 */
template <typename Kernel, typename Container, typename NamedParameters1, typename NamedParameters>
typename Gps_circle_segment_traits_2<Kernel>::Polygon_with_holes_2
approximated_offset_2(const Polygon_with_holes_2<Kernel, Container>& pwh, const typename Kernel::FT& r,
                      const double& eps,
                      const NamedParameters1& np1 = parameters::default_values(),
                      const NamedParameters& np = parameters::default_values());

} // namespace Minkowski_sum_2
} // namespace CGAL
