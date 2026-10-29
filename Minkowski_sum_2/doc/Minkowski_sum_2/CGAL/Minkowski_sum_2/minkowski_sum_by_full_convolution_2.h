namespace CGAL {
namespace Minkowski_sum_2 {

/*! \ingroup PkgMinkowskiSum2Ref
 *
 * computes the Minkowski sum \f$ P \oplus Q\f$ of the two given polygons.
 * The function computes the (full) convolution cycles of the two polygons and
 * extracts the regions having positive winding number with respect to these
 * cycles. This method works very efficiently, regardless of whether `P` and
 * `Q` are convex or non-convex. The outer boundaries and the holes of
 * polygons with holes are treated alike; that is, the convolution cycles of
 * every boundary cycle of `P` and every boundary cycle of `Q` are computed,
 * except for the holes that cannot contribute to the boundary of the sum,
 * which are filtered out first, as done by the reduced convolution method.
 * If both polygons are convex once these holes are removed, their sum is
 * computed directly, by merging their edges in the order of their
 * directions.
 *
 * `PolygonType1` and `PolygonType2` can be any combination of:
 *
 * - `Polygon_2`
 * - `Polygon_with_holes_2`
 *
 * A polygon with holes may be unbounded, that is, without an outer boundary
 * (see `Polygon_with_holes_2::is_unbounded()`). If one summand is unbounded,
 * so is the sum, which is returned as a polygon with holes without an outer
 * boundary; the holes of the other summand do not affect the sum then. If
 * both summands are unbounded, the sum is the entire plane.
 *
 * \pre The outer boundaries of `P` and `Q` (if any) are oriented
 *      counterclockwise, and their holes are oriented clockwise.
 *
 * \param P the first polygon
 * \param Q the second polygon
 * \param np1 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the first polygon
 *        among the ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{is_convex}
 *     \cgalParamDescription{a hint whether the polygon is convex, that is, bounded, without holes, and with a
 *                           convex outer boundary (if both polygons are convex, their sum is computed
 *                           directly)}
 *     \cgalParamType{`bool`}
 *     \cgalParamDefault{none; the convexity of the polygon is tested where needed}
 *     \cgalParamExtra{If the hint is `true`, the polygon is not tested; if expensive checks are enabled
 *                     (`CGAL_CHECK_EXPENSIVE`), its convexity is verified. If the hint is `false`, the polygon
 *                     is not tested either, and it is treated as non-convex (which is always correct, but may
 *                     be slower).}
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
 *     \cgalParamExtra{If the hint is `true`, the polygon is not tested; if expensive checks are enabled
 *                     (`CGAL_CHECK_EXPENSIVE`), its convexity is verified. If the hint is `false`, the polygon
 *                     is not tested either, and it is treated as non-convex (which is always correct, but may
 *                     be slower).}
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
 * \return the Minkowski sum
 *
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_2()`
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2()`
 */
template <typename Kernel, typename Container,
          typename NamedParameters1, typename NamedParameters2, typename NamedParameters>
Polygon_with_holes_2<Kernel, Container>
minkowski_sum_by_full_convolution_2(const PolygonType1<Kernel, Container>& P,
                                    const PolygonType2<Kernel, Container>& Q,
                                    const NamedParameters1& np1 = parameters::default_values(),
                                    const NamedParameters2& np2 = parameters::default_values(),
                                    const NamedParameters& np = parameters::default_values());

} // namespace Minkowski_sum_2
} // namespace CGAL
