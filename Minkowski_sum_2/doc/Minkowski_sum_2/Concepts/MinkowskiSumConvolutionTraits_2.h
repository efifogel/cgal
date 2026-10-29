/*! \ingroup PkgMinkowskiSum2Concepts
 * \cgalConcept
 *
 * The concept `MinkowskiSumConvolutionTraits_2` lists the minimal set of
 * geometric types and operations required to compute the Minkowski sum or the
 * Minkowski difference of two polygons with the full convolution method. The
 * convolution cycles of the polygons are inserted into an arrangement, and
 * the Minkowski sum of two convex polygons is computed directly.
 *
 * A model of this concept is a geometry traits class of an arrangement, whose
 * \f$ x\f$-monotone curves are line segments. The curve and the
 * \f$ x\f$-monotone curve constructed from two points must be the line segment
 * that connects them. The type `Point_2` of the refined concepts must be the
 * same type.
 *
 * \cgalRefines{MinkowskiSumDirectionTraits_2,AosDirectionalXMonotoneTraits_2,AosConstructXMonotoneCurveTraits_2,AosConstructCurveTraits_2}
 *
 * The concepts `AosConstructCurveTraits_2` and `AosConstructXMonotoneCurveTraits_2` are required, as the
 * convolution cycles may be represented by polylines (see
 * `CGAL::Minkowski_sum_2::Convolution_curves`), and the traits class
 * `CGAL::Arr_polyline_traits_2<SegmentTraits_2>` requires them from its
 * subcurve traits.
 *
 * \cgalHasModelsBegin
 * \cgalHasModels{CGAL::Arr_segment_traits_2<Kernel>}
 * \cgalHasModels{CGAL::Gps_segment_traits_2<Kernel,Container,ArrSegmentTraits>}
 * \cgalHasModelsEnd
 *
 * \sa `MinkowskiSumDirectionTraits_2`
 * \sa `MinkowskiSumTraits_2`
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2()`
 * \sa `CGAL::Minkowski_sum_2::minkowski_difference_by_full_convolution_2()`
 */
class MinkowskiSumConvolutionTraits_2 {
public:
  /// \name Functor Types
  /// @{

  /*! models the concept `Kernel::Orientation_2`. */
  typedef unspecified_type Orientation_2;

  /// @}

  /// \name Accessing Functor Objects
  /// @{

  /*!
   */
  Orientation_2 orientation_2_object() const;

  /// @}
};
