namespace CGAL {

/*! \ingroup PkgArrangementOnSurface2TraitsClasses
 *
 * The class `Arr_data_policy_2` is a data policy for the traits class
 * `Arr_data_traits_2`. It attaches data of the types `PointData`,
 * `XMonotoneCurveData`, and `CurveData` to points, \f$x\f$-monotone curves,
 * and curves, respectively, and it does not use the data to expedite any
 * operation. The data of a curve is converted to the data of its
 * \f$x\f$-monotone subcurves using a conversion constructor of
 * `XMonotoneCurveData`; when data are merged, the first data is retained; and
 * an intersection point obtains default-constructed data.
 *
 * The class can also serve as a base class of a data policy that hides some of
 * its member functions.
 *
 * \cgalModels{AosDataPolicy_2}
 *
 * \sa `CGAL::Arr_data_traits_2<GeometryTraits_2,DataPolicy_2>`
 */
template <typename PointData, typename XMonotoneCurveData, typename CurveData = XMonotoneCurveData>
class Arr_data_policy_2 {};

/*! \ingroup PkgArrangementOnSurface2TraitsClasses
 *
 * The class `Arr_data_traits_2` serves as a decorator class that extends the
 * points, the curves, and the \f$x\f$-monotone curves defined by the base
 * traits class (the `GeometryTraits_2` parameter) with extraneous
 * (non-geometric) data fields. The types of the data fields, and the way they
 * are propagated, are determined by the data policy (the `DataPolicy_2`
 * parameter).
 *
 * Every \f$x\f$-monotone curve stores its data and the data of its two
 * endpoints. The data are propagated as follows:
 * - When a curve is subdivided into \f$x\f$-monotone subcurves, its data is
 *   converted and attached to each one of the subcurves; the data of the
 *   endpoints of the subcurves and of isolated points are default constructed.
 * - When an \f$x\f$-monotone curve is split at a point, both subcurves obtain
 *   the data of the curve, and the data of the point becomes the data of their
 *   common endpoint.
 * - An intersection point of two \f$x\f$-monotone curves obtains its data
 *   from the data policy, and an overlapping subcurve of two
 *   \f$x\f$-monotone curves obtains the merged data of the two curves, and the
 *   merged data of the coinciding endpoints of the two curves.
 * - When the endpoint of an \f$x\f$-monotone curve, an isolated point, or an
 *   intersection point coincides with another point during the aggregated
 *   construction of an arrangement or the aggregated insertion of curves into
 *   an arrangement, their data are merged, and the merged data is stored with
 *   the arrangement vertex.
 *
 * When the data policy guarantees that two points are equal, the predicates
 * `Compare_x_2`, `Compare_xy_2`, and `Equal_2` are not evaluated
 * geometrically. When the data policy guarantees that two \f$x\f$-monotone
 * curves are interior disjoint, `Intersect_2` reports their common endpoints,
 * whose data are identical, without evaluating any intersection. When the code
 * is compiled with assertions enabled, these guarantees are verified
 * geometrically.
 *
 * The data of coinciding points is not merged by the incremental insertion
 * functions, by the insertion of isolated points, or by the overlay of
 * arrangements. During the aggregated insertion, the data of an intersection
 * point that coincides with an endpoint of one of the intersecting curves, or
 * with the current event of the surface sweep, is not merged.
 *
 * All other functors are inherited from the base traits class.
 *
 * \tparam GeometryTraits_2 a model of the concept `AosTraits_2`.
 * \tparam DataPolicy_2 a model of the concept `AosDataPolicy_2`.
 *
 * \cgalModels{AosTraits_2}
 *
 * \sa `CGAL::Arr_curve_data_traits_2<Tr,XData,Mrg,CData,Cnv>`
 */
template <typename GeometryTraits_2, typename DataPolicy_2>
class Arr_data_traits_2 : public GeometryTraits_2 {
public:
  /// \name Types
  /// @{

  /// the base traits class.
  typedef GeometryTraits_2 Base_traits_2;

  /// the data policy.
  typedef DataPolicy_2 Data_policy_2;

  /// the type of data attached to points.
  typedef typename Data_policy_2::Point_data Point_data;

  /// the type of data attached to \f$x\f$-monotone curves.
  typedef typename Data_policy_2::X_monotone_curve_data X_monotone_curve_data;

  /// the type of data attached to curves.
  typedef typename Data_policy_2::Curve_data Curve_data;

  /// the base point.
  typedef typename Base_traits_2::Point_2 Base_point_2;

  /// the base \f$x\f$-monotone curve.
  typedef typename Base_traits_2::X_monotone_curve_2 Base_x_monotone_curve_2;

  /// the base curve.
  typedef typename Base_traits_2::Curve_2 Base_curve_2;

  /// a tag that indicates that the traits class attaches data to points.
  typedef Tag_true Has_point_data_category;

  /// @}

  /*! The `Point_2` class nested within the data traits extends the
   * `Base_traits_2::Point_2` type with an extra data field.
   */
  class Point_2 : public Base_point_2 {
  public:
    /// \name Creation
    /// @{

    /*! constructs default. */
    Point_2();

    /*! constructs a point from the given `base` point with a
     * default-constructed data field.
     */
    Point_2(const Base_point_2& base);

    /*! constructs a point from the given `base` point with an attached `data`
     * field.
     */
    Point_2(const Base_point_2& base, const Point_data& data);

    /// @}

    /// \name Access Functions
    /// @{

    /*! returns the data field (a non-const version, which returns a reference
     * to the data object, is also available).
     */
    const Point_data& data() const;

    /*! sets the data field. */
    void set_data(const Point_data& data);

    /// @}
  };

  /*! The `X_monotone_curve_2` class nested within the data traits extends the
   * `Base_traits_2::X_monotone_curve_2` type with an extra data field and with
   * the data fields of its endpoints.
   */
  class X_monotone_curve_2 : public Base_x_monotone_curve_2 {
  public:
    /// \name Creation
    /// @{

    /*! constructs default. */
    X_monotone_curve_2();

    /*! constructs an \f$x\f$-monotone curve from the given `base` curve with
     * default-constructed data fields.
     */
    X_monotone_curve_2(const Base_x_monotone_curve_2& base);

    /*! constructs an \f$x\f$-monotone curve from the given `base` curve with
     * an attached `data` field and default-constructed data fields of its
     * endpoints.
     */
    X_monotone_curve_2(const Base_x_monotone_curve_2& base, const X_monotone_curve_data& data);

    /*! constructs an \f$x\f$-monotone curve from the given `base` curve with
     * an attached `data` field, and with the data fields `min_data` and
     * `max_data` of its left and right endpoints, respectively.
     */
    X_monotone_curve_2(const Base_x_monotone_curve_2& base, const X_monotone_curve_data& data,
                       const Point_data& min_data, const Point_data& max_data);

    /// @}

    /// \name Access Functions
    /// @{

    /*! returns the data field (a non-const version, which returns a reference
     * to the data object, is also available).
     */
    const X_monotone_curve_data& data() const;

    /*! sets the data field. */
    void set_data(const X_monotone_curve_data& data);

    /*! returns the data field of the endpoint at the curve end `ce` (a
     * non-const version, which returns a reference to the data object, is also
     * available).
     */
    const Point_data& end_data(Arr_curve_end ce) const;

    /*! sets the data field of the endpoint at the curve end `ce`. */
    void set_end_data(Arr_curve_end ce, const Point_data& data);

    /// @}
  };

  /*! The `Curve_2` class nested within the data traits extends the
   * `Base_traits_2::Curve_2` type with an extra data field.
   */
  class Curve_2 : public Base_curve_2 {
  public:
    /// \name Creation
    /// @{

    /*! constructs default. */
    Curve_2();

    /*! constructs a curve from the given `base` curve with an uninitialized
     * data field.
     */
    Curve_2(const Base_curve_2& base);

    /*! constructs a curve from the given `base` curve with an attached `data`
     * field.
     */
    Curve_2(const Base_curve_2& base, const Curve_data& data);

    /// @}

    /// \name Access Functions
    /// @{

    /*! returns the data field (a non-const version, which returns a reference
     * to the data object, is also available).
     */
    const Curve_data& data() const;

    /*! sets the data field. */
    void set_data(const Curve_data& data);

    /// @}
  };

  /*! A functor that merges the data of coinciding points.
   */
  class Merge_point_data_2 {
  public:
    /*! returns the point `p` with the merged data of `p` and `q`.
     * \pre `p` and `q` are equal.
     */
    Point_2 operator()(const Point_2& p, const Point_2& q) const;

    /*! returns the point `p` with the merged data of `p` and of an
     * intersection point of `xcv1` and `xcv2`.
     * \pre `p` lies on `xcv1` and on `xcv2`.
     */
    Point_2 operator()(const Point_2& p, const X_monotone_curve_2& xcv1, const X_monotone_curve_2& xcv2) const;
  };

  /// \name Creation
  /// @{

  /*! constructs default. */
  Arr_data_traits_2();

  /*! constructs from the base traits `traits` and the data policy `policy`. */
  Arr_data_traits_2(const Base_traits_2& traits, const Data_policy_2& policy = Data_policy_2());

  /// @}

  /// \name Access Functions
  /// @{

  /*! returns the base traits. */
  const Base_traits_2& base_traits() const;

  /*! returns the data policy. */
  const Data_policy_2& data_policy() const;

  /*! returns a `Merge_point_data_2` functor. */
  Merge_point_data_2 merge_point_data_2_object() const;

  /// @}
};

} // namespace CGAL
