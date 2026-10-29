namespace CGAL {
namespace Minkowski_sum_2 {

/*!
 * \ingroup PkgMinkowskiSum2Ref
 *
 * computes the Minkowski sum \f$ P \oplus Q\f$ of the two given polygons
 * using the decomposition strategy. It decomposes the summands `P` and `Q`
 * into convex sub-polygons \f$ P_1, \ldots, P_k\f$ and
 * \f$ Q_1, \ldots, Q_{\ell}\f$, respectively, and computes the union of
 * pairwise sub-sums (namely \f$ \bigcup_{i,j}{(P_i \oplus Q_j)}\f$).
 *
 * First, the holes that cannot affect the sum are removed (see Section
 * \ref mink_ssec_hole_filter). Then, each summand is decomposed by its own
 * strategies, given by its named parameters: if it remains a polygon with
 * holes, by its strategy `decomposition_strategy_with_holes`; otherwise, by
 * its strategy `decomposition_strategy`, unless it is convex, in which case it
 * is not decomposed (that is, the nop strategy `Polygon_nop_decomposition_2`
 * is applied). If both summands are convex, their sum is computed directly.
 *
 * `PolygonType1` and `PolygonType2` can be any combination of:
 *
 * - `Polygon_2`
 * - `Polygon_with_holes_2`
 *
 * \param P the first polygon
 * \param Q the second polygon
 * \param np1 an optional sequence of \ref bgl_namedparameters "Named Parameters" of the first polygon
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
 *   \cgalParamNBegin{decomposition_strategy}
 *     \cgalParamDescription{the strategy that decomposes the polygon into convex sub-polygons if it has no holes
 *                           (after the holes that cannot affect the sum are removed) and it is not convex}
 *     \cgalParamType{a model of the concept `PolygonConvexDecomposition_2`, `std::reference_wrapper<const Strategy>`,
 *                    or `std::shared_ptr<const Strategy>`, where `Strategy` is a model of this concept}
 *     \cgalParamDefault{`CGAL::Small_side_angle_bisector_decomposition_2<Kernel, Container>`}
 *     \cgalParamExtra{Used only by the decomposition method. An instance is copied; a reference (passed as
 *                     `std::cref(strategy)`) and a shared pointer are not.}
 *   \cgalParamNEnd
 *   \cgalParamNBegin{decomposition_strategy_with_holes}
 *     \cgalParamDescription{the strategy that decomposes the polygon into convex sub-polygons if it has holes (after
 *                           the holes that cannot affect the sum are removed)}
 *     \cgalParamType{a model of the concept `PolygonWithHolesConvexDecomposition_2`,
 *                    `std::reference_wrapper<const Strategy>`, or `std::shared_ptr<const Strategy>`, where `Strategy`
 *                    is a model of this concept}
 *     \cgalParamDefault{`CGAL::Polygon_vertical_decomposition_2<Kernel, Container>`}
 *     \cgalParamExtra{Used only by the decomposition method. An instance is copied; a reference (passed as
 *                     `std::cref(strategy)`) and a shared pointer are not.}
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
 *   \cgalParamNBegin{decomposition_strategy}
 *     \cgalParamDescription{the strategy that decomposes the polygon into convex sub-polygons if it has no holes
 *                           (after the holes that cannot affect the sum are removed) and it is not convex}
 *     \cgalParamType{a model of the concept `PolygonConvexDecomposition_2`, `std::reference_wrapper<const Strategy>`,
 *                    or `std::shared_ptr<const Strategy>`, where `Strategy` is a model of this concept}
 *     \cgalParamDefault{`CGAL::Small_side_angle_bisector_decomposition_2<Kernel, Container>`}
 *     \cgalParamExtra{Used only by the decomposition method. An instance is copied; a reference (passed as
 *                     `std::cref(strategy)`) and a shared pointer are not.}
 *   \cgalParamNEnd
 *   \cgalParamNBegin{decomposition_strategy_with_holes}
 *     \cgalParamDescription{the strategy that decomposes the polygon into convex sub-polygons if it has holes (after
 *                           the holes that cannot affect the sum are removed)}
 *     \cgalParamType{a model of the concept `PolygonWithHolesConvexDecomposition_2`,
 *                    `std::reference_wrapper<const Strategy>`, or `std::shared_ptr<const Strategy>`, where `Strategy`
 *                    is a model of this concept}
 *     \cgalParamDefault{`CGAL::Polygon_vertical_decomposition_2<Kernel, Container>`}
 *     \cgalParamExtra{Used only by the decomposition method. An instance is copied; a reference (passed as
 *                     `std::cref(strategy)`) and a shared pointer are not.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \param np an optional sequence of \ref bgl_namedparameters "Named Parameters" of the computation among the
 *        ones listed below
 *
 * \cgalNamedParamsBegin
 *   \cgalParamNBegin{geom_traits}
 *     \cgalParamDescription{an instance of a geometry traits class, a reference to one, or a shared pointer to one}
 *     \cgalParamType{a model of the concept `MinkowskiSumDecompositionTraits_2`,
 *                    `std::reference_wrapper<const Traits>`, or `std::shared_ptr<const Traits>`, where `Traits` is a
 *                    model of this concept}
 *     \cgalParamDefault{a default-constructed instance of `CGAL::Gps_segment_traits_2<Kernel, Container>`}
 *     \cgalParamExtra{Its types `Point_2`, `Polygon_2`, and `Polygon_with_holes_2` must be `Kernel::Point_2`,
 *                     `Polygon_2<Kernel, Container>`, and `Polygon_with_holes_2<Kernel, Container>`, respectively.
 *                     The traits are used only during the call. An instance is copied; a reference (passed as
 *                     `std::cref(traits)`) and a shared pointer are not, and the instance they refer to may be shared
 *                     with other objects, for example, arrangements. However, the traits are copied once to construct
 *                     the traits of the general polygon set that computes the union of the pairwise sums, and once
 *                     more for every polygon whose validity is checked by the preconditions of the general polygon
 *                     set, if preconditions are checked.}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \return the Minkowski sum
 *
 * \pre `P` and `Q` are bounded, their outer boundaries are oriented
 *      counterclockwise, and their holes are oriented clockwise.
 */
template <typename Kernel, typename Container,
          typename NamedParameters1, typename NamedParameters2, typename NamedParameters>
Polygon_with_holes_2<Kernel, Container>
minkowski_sum_by_decomposition_2(const PolygonType1<Kernel, Container>& P,
                                 const PolygonType2<Kernel, Container>& Q,
                                 const NamedParameters1& np1 = parameters::default_values(),
                                 const NamedParameters2& np2 = parameters::default_values(),
                                 const NamedParameters& np = parameters::default_values());

} // namespace Minkowski_sum_2
} // namespace CGAL
