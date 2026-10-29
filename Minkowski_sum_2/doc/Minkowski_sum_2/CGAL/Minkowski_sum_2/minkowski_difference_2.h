namespace CGAL {
namespace Minkowski_sum_2 {

/*! \ingroup PkgMinkowskiSum2Ref
 *
 * computes the Minkowski difference \f$ P \ominus Q\f$ of the two given
 * polygons.  The Minkowski difference of two polygons \f$ P\f$ and \f$ Q\f$ is
 * the complement of the Minkowski sum of \f$ P\f$ and the complement of
 * \f$ Q\f$.
 *
 * The difference may be empty, and it may consist of several polygons with
 * holes, which are inserted into the output iterator `oi`.
 *
 * The difference is the set of points \f$ x\f$ such that the translated copy
 * \f$ x - P\f$ is contained in \f$ Q\f$. The outer boundaries and the holes
 * of the polygons are treated alike.
 *
 * `PolygonType1` and `PolygonType2` can be any combination of:
 *
 * - `Polygon_2`
 * - `Polygon_with_holes_2`
 *
 * A polygon with holes may be unbounded, that is, without an outer boundary
 * (see `Polygon_with_holes_2::is_unbounded()`). If `Q` is unbounded, so is
 * the difference (unless it is empty), and its unbounded component is reported
 * as a polygon with holes without an outer boundary, followed by the bounded
 * components, if any. If `P` is unbounded and `Q` is bounded, the difference is
 * empty.
 *
 * This function defaults to the full convolution method; that is, it calls
 * `CGAL::Minkowski_sum_2::minkowski_difference_by_full_convolution_2()`.
 *
 * \pre The outer boundaries of `P` and `Q` (if any) are oriented
 *      counterclockwise, and their holes are oriented clockwise.
 *
 * \tparam OutputIterator must be an output iterator whose value type is
 *         `Polygon_with_holes_2<Kernel, Container>`.
 *
 * \param P the first polygon
 * \param Q the second polygon
 * \param oi the output iterator for the polygons with holes of the difference
 * \param np1 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the first polygon
 *        among the ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{is_convex}
 *     \cgalParamDescription{a hint whether the polygon is convex, that is, bounded, without holes, and with a
 *                           convex outer boundary}
 *     \cgalParamType{`bool`}
 *     \cgalParamDefault{none; the convexity of the polygon is tested where needed}
 *     \cgalParamExtra{The hint does not affect the difference; if it is `true` and expensive checks are
 *                     enabled (`CGAL_CHECK_EXPENSIVE`), the convexity of the polygon is verified.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \param np2 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the second polygon
 *        among the ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{is_convex}
 *     \cgalParamDescription{a hint whether the polygon is convex, that is, bounded, without holes, and with a
 *                           convex outer boundary}
 *     \cgalParamType{`bool`}
 *     \cgalParamDefault{none; the convexity of the polygon is tested where needed}
 *     \cgalParamExtra{The hint does not affect the difference; if it is `true` and expensive checks are
 *                     enabled (`CGAL_CHECK_EXPENSIVE`), the convexity of the polygon is verified.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \param np an optional sequence of \ref bgl_namedparameters "Named Parameters" of the computation among the
 *        ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{geom_traits}
 *     \cgalParamDescription{an instance of a geometry traits class, a reference to one, or a shared pointer to one}
 *     \cgalParamType{a model of the concept `MinkowskiSumConvolutionTraits_2`,
 *                    `std::reference_wrapper<const Traits>`, or `std::shared_ptr<const Traits>`, where `Traits` is a
 *                    model of this concept}
 *     \cgalParamDefault{a default-constructed instance of `CGAL::Gps_segment_traits_2<Kernel, Container>`}
 *     \cgalParamExtra{Its type `Point_2` must be `Kernel::Point_2`. The traits are used only during the call. An
 *                     instance is copied; a reference (passed as `std::cref(traits)`) and a shared pointer are not,
 *                     and the instance they refer to may be shared with other objects, for example, arrangements.
 *                     However, if the convolution cycles are represented by segments (see `convolution_curves`), the
 *                     traits are copied once to construct the traits of the arrangement of the convolution cycles.}
 *   \cgalParamNEnd
 *   \cgalParamNBegin{convolution_curves}
 *     \cgalParamDescription{the representation of the convolution cycles}
 *     \cgalParamType{`CGAL::Minkowski_sum_2::Convolution_curves`}
 *     \cgalParamDefault{`CGAL::Minkowski_sum_2::Convolution_curves::AUTOMATIC`}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \return the past-the-end iterator of the output
 *
 * \sa `CGAL::Minkowski_sum_2::minkowski_difference_by_full_convolution_2()`
 */
template <typename Kernel, typename Container, typename OutputIterator,
          typename NamedParameters1, typename NamedParameters2, typename NamedParameters>
OutputIterator minkowski_difference_2(const PolygonType1<Kernel, Container>& P,
                                      const PolygonType2<Kernel, Container>& Q,
                                      OutputIterator oi,
                                      const NamedParameters1& np1 = parameters::default_values(),
                                      const NamedParameters2& np2 = parameters::default_values(),
                                      const NamedParameters& np = parameters::default_values());

} // namespace Minkowski_sum_2
} // namespace CGAL
