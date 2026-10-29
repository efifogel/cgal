/*! \ingroup PkgMinkowskiSum2Concepts
 * \cgalConcept
 *
 * The concept `MinkowskiSumDecompositionTraits_2` lists the minimal set of
 * geometric types and operations required to compute the Minkowski sum of two
 * polygons with the decomposition method. The polygons are decomposed into
 * convex sub-polygons, the Minkowski sums of the pairs of sub-polygons are
 * computed directly, and their union is computed with a general polygon set.
 *
 * A model of this concept is a geometry traits class of a general polygon
 * set, whose \f$ x\f$-monotone curves are line segments. The type `Point_2` of
 * the refined concepts must be the same type.
 *
 * \cgalRefines{MinkowskiSumDirectionTraits_2,GeneralPolygonSetTraits_2}
 *
 * \cgalHasModelsBegin
 * \cgalHasModels{CGAL::Gps_segment_traits_2<Kernel,Container,ArrSegmentTraits>}
 * \cgalHasModelsEnd
 *
 * \sa `MinkowskiSumDirectionTraits_2`
 * \sa `MinkowskiSumTraits_2`
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_by_decomposition_2()`
 */
class MinkowskiSumDecompositionTraits_2 {
public:
};
