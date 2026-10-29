/*! \ingroup PkgMinkowskiSum2Concepts
 * \cgalConcept
 *
 * The concept `MinkowskiSumTraits_2` lists the geometric types and operations
 * required to compute the Minkowski sum of two polygons with any of the
 * methods that use a geometry traits class, namely, the decomposition method
 * and the full convolution method. As the method may be chosen at run time
 * (see `CGAL::Minkowski_sum_2::Method`), the requirements of both methods
 * apply.
 *
 * \cgalRefines{MinkowskiSumConvolutionTraits_2,MinkowskiSumDecompositionTraits_2}
 *
 * \cgalHasModelsBegin
 * \cgalHasModels{CGAL::Gps_segment_traits_2<Kernel,Container,ArrSegmentTraits>}
 * \cgalHasModelsEnd
 *
 * \sa `MinkowskiSumDirectionTraits_2`
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_2()`
 */
class MinkowskiSumTraits_2 {
public:
};
