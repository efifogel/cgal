namespace CGAL {
namespace Minkowski_sum_2 {

/*!
 * \ingroup PkgMinkowskiSum2Ref
 *
 * provides a guaranteed approximation of the inset, or inner offset, of the
 * given polygon `P` by a given radius `r`. Namely, the function computes the
 * set of points inside the polygon whose distance from \f$ P\f$'s boundary is
 * at least \f$ r\f$:
 * \f$ \{ p \in P \;|\; {\rm dist}(p, \partial P) \geq r \}\f$, with the
 * approximation error bounded by `eps`. Note that as the input polygon may not
 * be convex, its inset may comprise several disconnected components. The
 * result is therefore represented as a sequence of generalized polygons, whose
 * edges are either line segments or circular arcs. The output sequence is
 * returned via the output iterator `oi`, whose value type must be
 * `Gps_circle_segment_traits_2<Kernel>::%Polygon_2`.
 *
 * \param P the polygon
 * \param r the inset radius
 * \param eps the approximation-error bound
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
template <typename Kernel, typename Container, typename OutputIterator,
          typename NamedParameters1, typename NamedParameters>
OutputIterator
approximated_inset_2(const Polygon_2<Kernel, Container>& P, const typename Kernel::FT& r, const double& eps,
                     OutputIterator oi,
                     const NamedParameters1& np1 = parameters::default_values(),
                     const NamedParameters& np = parameters::default_values());

} // namespace Minkowski_sum_2
} // namespace CGAL
