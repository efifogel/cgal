namespace CGAL {
namespace Minkowski_sum_2 {

/*!
 * \ingroup PkgMinkowskiSum2Ref
 *
 * computes the inset, or inner offset, of the given polygon `P` by a given
 * radius `r`, namely, the set of points inside the polygon whose distance from
 * \f$ P\f$'s boundary is at least \f$ r\f$:
 * \f$ \{ p \in P \;|\; {\rm dist}(p, \partial P) \geq r \}\f$. Note that as
 * the input polygon may not be convex, its inset may comprise several
 * disconnected components. The result is therefore represented as a sequence
 * of generalized polygons, such that the edges of each polygon correspond to
 * line segments and circular arcs, both are special types of conic arcs, as
 * represented by the `traits` class. The output sequence is returned via the
 * output iterator `oi`, whose value type must be `Gps_traits_2::Polygon_2`.
 *
 * \param P the polygon
 * \param r the inset radius
 * \param traits the conic traits
 * \param oi the output iterator for the inset polygons
 * \param np1 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the polygon among the ones
 *        listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{is_convex}
 *     \cgalParamDescription{a hint whether the polygon is convex}
 *     \cgalParamType{`bool`}
 *     \cgalParamDefault{none; the convexity of the polygon is tested where needed}
 *     \cgalParamExtra{The hint does not affect the inset; if it is `true` and expensive checks are
 *                     enabled (`CGAL_CHECK_EXPENSIVE`), the convexity of the polygon is verified.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \param np an optional sequence of \ref bgl_namedparameters "Named Parameters" of the computation among the
 *        ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{method}
 *     \cgalParamDescription{the method that computes the inset}
 *     \cgalParamType{`CGAL::Minkowski_sum_2::Method`}
 *     \cgalParamDefault{`CGAL::Minkowski_sum_2::Method::AUTOMATIC`}
 *     \cgalParamExtra{Only the convolution method is supported (`CGAL::Minkowski_sum_2::Method::AUTOMATIC`
 *                     or `CGAL::Minkowski_sum_2::Method::FULL_CONVOLUTION`).}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \return the past-the-end iterator of the output
 *
 * \pre `P` is a simple polygon.
 */
template <typename ConicTraits, typename Container, typename OutputIterator,
          typename NamedParameters1, typename NamedParameters>
OutputIterator
inset_polygon_2(const Polygon_2<typename ConicTraits::Rat_kernel, Container>& P,
                const typename ConicTraits::Rat_kernel::FT& r, const ConicTraits& traits, OutputIterator oi,
                const NamedParameters1& np1 = parameters::default_values(),
                const NamedParameters& np = parameters::default_values());

} // namespace Minkowski_sum_2
} // namespace CGAL
