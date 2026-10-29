namespace CGAL {
namespace Minkowski_sum_2 {

/*!
 * \ingroup PkgMinkowskiSum2Ref
 *
 * computes the Minkowski sum \f$ P \oplus Q\f$ of the two given polygons
 * using the reduced convolution method. The function computes the reduced
 * convolution \cgalCite{cgal:bl-frmsurc-11} of the two polygons and extracts
 * those loops of the convolution that are part of the Minkowski sum. This
 * method works very efficiently, regardless of whether `P` and `Q` are convex
 * or non-convex. In general, it is more efficient than the full convolution
 * method, except for degenerate cases where the output polygon has many holes.
 * First, the holes that cannot affect the sum are removed (see Section
 * \ref mink_ssec_hole_filter).
 *
 * `PolygonType1` and `PolygonType2` can be any combination of:
 *
 * - `Polygon_2`
 * - `Polygon_with_holes_2`
 *
 * \param P the first polygon
 * \param Q the second polygon
 * \param np1 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the first polygon among the
 *        ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{is_convex}
 *     \cgalParamDescription{a hint whether the polygon is convex}
 *     \cgalParamType{`bool`}
 *     \cgalParamDefault{none}
 *     \cgalParamExtra{The hint does not affect the sum; if it is `true` and expensive checks are enabled
 *                     (`CGAL_CHECK_EXPENSIVE`), the convexity of the polygon is verified.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \param np2 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the second polygon, as
 *        `np1`
 * \param np an optional sequence of \ref bgl_namedparameters "Named Parameters" of the computation (none is
 *        used)
 *
 * \return the Minkowski sum
 *
 * \pre The outer boundaries of `P` and `Q` (if any) are oriented
 *      counterclockwise, and their holes are oriented clockwise.
 *
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_2()`
 */
template <typename Kernel, typename Container,
          typename NamedParameters1, typename NamedParameters2, typename NamedParameters>
Polygon_with_holes_2<Kernel, Container>
minkowski_sum_by_reduced_convolution_2(const PolygonType1<Kernel, Container>& P,
                                       const PolygonType2<Kernel, Container>& Q,
                                       const NamedParameters1& np1 = parameters::default_values(),
                                       const NamedParameters2& np2 = parameters::default_values(),
                                       const NamedParameters& np = parameters::default_values());

} // namespace Minkowski_sum_2
} // namespace CGAL
