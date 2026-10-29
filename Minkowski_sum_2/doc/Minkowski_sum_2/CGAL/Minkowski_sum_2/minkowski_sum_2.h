namespace CGAL {
namespace Minkowski_sum_2 {

/*! \ingroup PkgMinkowskiSum2Ref
 *
 * The method that computes the Minkowski sum of two polygons, or the offset
 * of a polygon (see `CGAL::Minkowski_sum_2::approximated_offset_2()` and
 * `CGAL::Minkowski_sum_2::offset_polygon_2()`).
 *
 * It is given by the named parameter `method`.
 *
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_2()`
 */
enum class Method {
  /*! The method is chosen automatically.
   */
  AUTOMATIC,

  /*! The decomposition method; see `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()`.
   */
  DECOMPOSITION,

  /*! The reduced convolution method; see
   * `CGAL::Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2()`.
   */
  REDUCED_CONVOLUTION,

  /*! The full convolution method; see
   * `CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2()`.
   */
  FULL_CONVOLUTION
};

/*!
 * \ingroup PkgMinkowskiSum2Ref
 *
 * computes the Minkowski sum \f$ P \oplus Q\f$ of two given polygons (which
 * may have holes), with the method given by the named parameter `method`
 * (see `CGAL::Minkowski_sum_2::Method`):
 * - `CGAL::Minkowski_sum_2::Method::DECOMPOSITION`: see `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()`;
 * - `CGAL::Minkowski_sum_2::Method::REDUCED_CONVOLUTION`: see
 *   `CGAL::Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2()`;
 * - `CGAL::Minkowski_sum_2::Method::FULL_CONVOLUTION`: see
 *   `CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2()`;
 * - `CGAL::Minkowski_sum_2::Method::AUTOMATIC` (the default): the method is chosen automatically.
 *   The full convolution method is chosen if one of the polygons is
 *   unbounded.  Otherwise, the holes that cannot affect the sum are removed
 *   first; if both polygons are then convex, their sum is computed directly.
 *   Otherwise, the method is chosen by the estimated size of the full
 *   convolution, \f$ E = m + n + l_1 n + l_2 m\f$, where \f$ m\f$ and
 *   \f$ n\f$ are the numbers of vertices of the polygons, and \f$ l_1\f$ and
 *   \f$ l_2\f$ are the numbers of their vertices at which their boundaries do
 *   not turn left (that is, the reflex vertices of the outer boundaries and
 *   the convex vertices of the holes): the full convolution method is chosen
 *   if the convolution is sparse (\f$ E \le 8(m + n)\f$, or
 *   \f$ E \le 2(m + n)\f$ if \f$ \max(l_1, l_2) > 30\f$), the decomposition
 *   method is chosen if the convolution is very large (\f$ E > 40000\f$) and
 *   both polygons have no holes and are highly concave
 *   (\f$ l_1 > 0.3m\f$ and \f$ l_2 > 0.3n\f$), and the reduced convolution
 *   method is chosen otherwise.  The thresholds were determined
 *   experimentally.
 *
 * The named parameters of each polygon apply to that polygon, and are passed
 * on to the method that is applied, which uses those it supports.
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
 *   \cgalParamNBegin{method}
 *     \cgalParamDescription{the method that computes the sum}
 *     \cgalParamType{`CGAL::Minkowski_sum_2::Method`}
 *     \cgalParamDefault{`CGAL::Minkowski_sum_2::Method::AUTOMATIC`}
 *   \cgalParamNEnd
 *   \cgalParamNBegin{geom_traits}
 *     \cgalParamDescription{an instance of a geometry traits class, a reference to one, or a shared pointer to one}
 *     \cgalParamType{a model of the concept `MinkowskiSumTraits_2`, `std::reference_wrapper<const Traits>`, or
 *                    `std::shared_ptr<const Traits>`, where `Traits` is a model of this concept}
 *     \cgalParamDefault{a default-constructed instance of `CGAL::Gps_segment_traits_2<Kernel, Container>`}
 *     \cgalParamExtra{Its types `Point_2`, `Polygon_2`, and `Polygon_with_holes_2` must be `Kernel::Point_2`,
 *                     `Polygon_2<Kernel, Container>`, and `Polygon_with_holes_2<Kernel, Container>`, respectively.
 *                     The traits are used only during the call. An instance is copied; a reference (passed as
 *                     `std::cref(traits)`) and a shared pointer are not, and the instance they refer to may be shared
 *                     with other objects, for example, arrangements. However, if the decomposition method is applied,
 *                     or if the full convolution method is applied and the convolution cycles are represented by
 *                     segments, the traits are copied (see
 *                     `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()` and
 *                     `CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2()`). The traits are not used by the
 *                     reduced convolution method.}
 *   \cgalParamNEnd
 *   \cgalParamNBegin{convolution_curves}
 *     \cgalParamDescription{the representation of the convolution cycles}
 *     \cgalParamType{`CGAL::Minkowski_sum_2::Convolution_curves`}
 *     \cgalParamDefault{`CGAL::Minkowski_sum_2::Convolution_curves::AUTOMATIC`}
 *     \cgalParamExtra{used only by the full convolution method}
 *   \cgalParamNEnd
 * \cgalNamedParamsEnd
 *
 * \return the Minkowski sum
 *
 * \pre The outer boundaries of `P` and `Q` (if any) are oriented
 *      counterclockwise, and their holes are oriented clockwise.
 *
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_by_reduced_convolution_2()`
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()`
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2()`
 */
template <typename Kernel, typename Container,
          typename NamedParameters1, typename NamedParameters2, typename NamedParameters>
Polygon_with_holes_2<Kernel, Container>
minkowski_sum_2(const PolygonType1<Kernel, Container>& P,
                const PolygonType2<Kernel, Container>& Q,
                const NamedParameters1& np1 = parameters::default_values(),
                const NamedParameters2& np2 = parameters::default_values(),
                const NamedParameters& np = parameters::default_values());

} // namespace Minkowski_sum_2
} // namespace CGAL
