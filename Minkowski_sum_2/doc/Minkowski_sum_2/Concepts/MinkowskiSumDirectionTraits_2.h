/*! \ingroup PkgMinkowskiSum2Concepts
 * \cgalConcept
 *
 * The concept `MinkowskiSumDirectionTraits_2` lists the minimal set of
 * geometric types and operations required to compute the Minkowski sum of two
 * convex polygons directly, that is, by merging their edges in the
 * counterclockwise order of their directions. Its types and operations are a
 * subset of the types and operations of the concept `Kernel`, and they must
 * have the same semantics.
 *
 * Every model of the concept `Kernel` is a model of this concept.
 *
 * \cgalHasModelsBegin
 * \cgalHasModels{CGAL::Exact_predicates_exact_constructions_kernel}
 * \cgalHasModels{CGAL::Exact_predicates_inexact_constructions_kernel}
 * \cgalHasModels{CGAL::Arr_segment_traits_2<Kernel>}
 * \cgalHasModels{CGAL::Gps_segment_traits_2<Kernel,Container,ArrSegmentTraits>}
 * \cgalHasModelsEnd
 *
 * \sa `MinkowskiSumConvolutionTraits_2`
 * \sa `MinkowskiSumDecompositionTraits_2`
 * \sa `MinkowskiSumTraits_2`
 */
class MinkowskiSumDirectionTraits_2 {
public:
  /// \name Types
  /// @{

  /*! the point type; it must model the concept `Kernel::Point_2`. */
  typedef unspecified_type Point_2;

  /*! the vector type; it must model the concept `Kernel::Vector_2`. */
  typedef unspecified_type Vector_2;

  /*! the direction type; it must model the concept `Kernel::Direction_2`. */
  typedef unspecified_type Direction_2;

  /// @}

  /// \name Functor Types
  /// @{

  /*! models the concept `Kernel::ConstructVector_2`, including the overload
   * that constructs the vector from the origin to a point.
   */
  typedef unspecified_type Construct_vector_2;

  /*! models the concept `Kernel::ConstructDirection_2`. */
  typedef unspecified_type Construct_direction_2;

  /*! models the concept `Kernel::ConstructTranslatedPoint_2`. */
  typedef unspecified_type Construct_translated_point_2;

  /*! models the concept `Kernel::CounterclockwiseInBetween_2`. */
  typedef unspecified_type Counterclockwise_in_between_2;

  /*! models the concept `Kernel::Equal_2`; only the operator that determines
   * whether two directions are equal is used.
   */
  typedef unspecified_type Equal_2;

  /// @}

  /// \name Accessing Functor Objects
  /// @{

  /*!
   */
  Construct_vector_2 construct_vector_2_object() const;

  /*!
   */
  Construct_direction_2 construct_direction_2_object() const;

  /*!
   */
  Construct_translated_point_2 construct_translated_point_2_object() const;

  /*!
   */
  Counterclockwise_in_between_2 counterclockwise_in_between_2_object() const;

  /*!
   */
  Equal_2 equal_2_object() const;

  /// @}
};
