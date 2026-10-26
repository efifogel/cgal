/*! \ingroup PkgArrangementOnSurface2ConceptsTraits
 * \cgalConcept
 *
 * A model of the concept `AosDataPolicy_2` determines the types of the data
 * fields that the traits class `Arr_data_traits_2` attaches to points, to
 * curves, and to \f$x\f$-monotone curves, and how these data fields are
 * propagated when curves are subdivided, split, and intersected, and when
 * points coincide.
 *
 * In addition, the data may guarantee that two points are equal or that two
 * \f$x\f$-monotone curves are interior disjoint. In these cases the
 * corresponding predicates and constructions are not evaluated geometrically,
 * which may save considerable time, for example, when the points are
 * constructed and the number type is filtered. Such guarantees must be sound;
 * when the code is compiled with assertions enabled, `Arr_data_traits_2`
 * verifies them geometrically.
 *
 * \cgalHasModelsBegin
 * \cgalHasModels{CGAL::Arr_data_policy_2<PointData,XMonotoneCurveData,CurveData>}
 * \cgalHasModels{CGAL::Arr_label_policy_2<XMonotoneCurveData,LinearCurvesTag,Merge,CurveData,Convert,Label,MultipleLabelsTag>}
 * \cgalHasModelsEnd
 *
 * \sa `CGAL::Arr_data_traits_2<GeometryTraits_2,DataPolicy_2>`
 */
class AosDataPolicy_2 {
public:
  /// \name Types
  /// @{

  /*! the type of the data attached to points. It must be default
   * constructible and copy constructible.
   */
  typedef unspecified_type Point_data;

  /*! the type of the data attached to \f$x\f$-monotone curves. It must be
   * default constructible and copy constructible.
   */
  typedef unspecified_type X_monotone_curve_data;

  /*! the type of the data attached to curves.
   */
  typedef unspecified_type Curve_data;

  /// @}

  /// \name Operations
  /// @{

  /*! converts the data of a curve to the data of each one of its
   * \f$x\f$-monotone subcurves.
   */
  X_monotone_curve_data convert(const Curve_data& data) const;

  /*! merges the data of two overlapping \f$x\f$-monotone curves into the data
   * of their overlapping subcurve.
   */
  X_monotone_curve_data merge_curve_data(const X_monotone_curve_data& data1,
                                         const X_monotone_curve_data& data2) const;

  /*! merges the data of two coinciding points.
   */
  Point_data merge_point_data(const Point_data& data1, const Point_data& data2) const;

  /*! obtains the data of an intersection point of two \f$x\f$-monotone curves.
   * `XMonotoneCurve_2` is the type `Arr_data_traits_2::X_monotone_curve_2`.
   */
  template <typename XMonotoneCurve_2>
  Point_data intersection_point_data(const XMonotoneCurve_2& xcv1, const XMonotoneCurve_2& xcv2) const;

  /*! determines whether the data of two points guarantee that the points are
   * equal. If `true` is returned, the points must be equal. If `false` is
   * returned, the points may or may not be equal. In particular, `false` must
   * be returned for default-constructed data.
   */
  bool are_identical(const Point_data& data1, const Point_data& data2) const;

  /*! determines whether two \f$x\f$-monotone curves are guaranteed to be
   * interior disjoint, that is, to have no common points other than common
   * endpoints. If `true` is returned, the curves must be interior disjoint, and
   * the data of every common endpoint of the curves must be identical (see
   * `are_identical()`). If `false` is returned, the curves may or may not be
   * interior disjoint. `GeometryTraits_2` is the base traits class of
   * `Arr_data_traits_2`, and `XMonotoneCurve_2` is the type
   * `Arr_data_traits_2::X_monotone_curve_2`.
   */
  template <typename GeometryTraits_2, typename XMonotoneCurve_2>
  bool are_interior_disjoint(const GeometryTraits_2& traits, const XMonotoneCurve_2& xcv1,
                             const XMonotoneCurve_2& xcv2) const;

  /// @}
};
