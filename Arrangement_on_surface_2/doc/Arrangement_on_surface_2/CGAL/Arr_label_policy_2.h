namespace CGAL {

/*! \ingroup PkgArrangementOnSurface2TraitsClasses
 *
 * The class `Arr_label_policy_2` is a data policy for the traits class
 * `Arr_data_traits_2` that attaches labels to points, and user data to curves
 * and \f$x\f$-monotone curves. It is typically used to label the
 * vertices of input curves, such as the vertices of polygonal cycles, whose
 * points are constructed; when the number type is filtered, comparing such a
 * point with itself geometrically may require an exact evaluation.
 *
 * Two points that share a label are guaranteed to be equal, so points that
 * represent the same input point must be labeled with the same label, and
 * points that are not equal must not share a label. An intersection point
 * obtains no label. The labels of a point are represented in one of two ways,
 * as determined by `MultipleLabelsTag`:
 * - If `MultipleLabelsTag` is `Tag_true`, the data of a point, of the type
 *   `Label_set`, is a sorted set of labels, and the label sets of coinciding
 *   points are united. Thus, a point that coincides with points with
 *   different labels is recognized as equal to each one of them.
 * - If `MultipleLabelsTag` is `Tag_false`, the data of a point, of the type
 *   `Single_label`, is a single, possibly empty, label, and a point that
 *   coincides with another point keeps one of the two labels. This
 *   representation is smaller, and the construction of arrangements is
 *   typically faster, unless many points with different labels coincide.
 *
 * If `LinearCurvesTag` is `Tag_true`, the curves are linear (for example,
 * segments), and two \f$x\f$-monotone curves that have a common endpoint whose
 * labels are shared, and that are not collinear, are guaranteed to be
 * interior disjoint. In this case the base traits class must provide the
 * predicate `Orientation_2` of a kernel, through `orientation_2_object()`.
 *
 * The data of a curve is converted to the data of its \f$x\f$-monotone
 * subcurves using the conversion functor `Convert`, and the data of two
 * overlapping \f$x\f$-monotone curves are merged using the merge functor
 * `Merge`, as in `Arr_curve_data_traits_2`.
 *
 * \tparam XMonotoneCurveData the type of the data of \f$x\f$-monotone curves.
 * \tparam LinearCurvesTag `Tag_true` if the curves are linear, and `Tag_false`
 *         otherwise.
 * \tparam Merge a functor that merges the data of two overlapping
 *         \f$x\f$-monotone curves; see `Arr_curve_data_traits_2`.
 * \tparam CurveData the type of the data of curves.
 * \tparam Convert a functor that converts the data of a curve to the data of
 *         its \f$x\f$-monotone subcurves; see `Arr_curve_data_traits_2`.
 * \tparam Label the type of a label, which must be totally ordered; if
 *         `MultipleLabelsTag` is `Tag_false`, it must be an integral type, and
 *         its maximum value is reserved to represent the empty label.
 * \tparam MultipleLabelsTag `Tag_true` if a point may carry several labels,
 *         and `Tag_false` if it carries a single label.
 *
 * \cgalModels{AosDataPolicy_2}
 *
 * \sa `CGAL::Arr_data_traits_2<GeometryTraits_2,DataPolicy_2>`
 */
template <typename XMonotoneCurveData,
          typename LinearCurvesTag = Tag_false,
          typename Merge = _Default_merge_func<XMonotoneCurveData>,
          typename CurveData = XMonotoneCurveData,
          typename Convert = _Default_convert_func<CurveData, XMonotoneCurveData>,
          typename Label = std::size_t,
          typename MultipleLabelsTag = Tag_true>
class Arr_label_policy_2 {
public:
  /// \name Types
  /// @{

  /// a sorted set of labels, a model of `RandomAccessContainer`.
  typedef unspecified_type Label_set;

  /*! the data of a point: `Label_set` if `MultipleLabelsTag` is `Tag_true`,
   * and `Single_label` otherwise.
   */
  typedef unspecified_type Point_data;

  /// @}

  /*! A single, possibly empty, label.
   */
  class Single_label {
  public:
    /// \name Creation
    /// @{

    /*! constructs an empty label.
     */
    Single_label();

    /*! constructs the label `label`.
     * \pre `label` is not the maximum value of `Label`.
     */
    Single_label(Label label);

    /// @}

    /// \name Operations
    /// @{

    /*! determines whether the label is empty.
     */
    bool is_empty() const;

    /*! obtains the label.
     * \pre the label is not empty.
     */
    Label label() const;

    /// @}
  };
};

} // namespace CGAL
