namespace AosTraits {

/*! \ingroup PkgArrangementOnSurface2ConceptsFunctionObjects
 * \cgalConcept
 *
 * \cgalRefines{AdaptableTernaryFunction}
 *
 * \cgalHasModelsBegin
 * \cgalHasModels{AosBasicTraits_2::Compare_y_at_x_left_2}
 * \cgalHasModelsEnd
 */
class CompareYAtXLeft_2 {
public:
  /// \name Operations
  /// A model of this concept must provide:
  /// @{

  /*! accepts two \f$x\f$-monotone curves `xc1` and `xc2` and a point `p` that
   * lies on both curves, and returns `CGAL::SMALLER`, `CGAL::EQUAL`, or
   * `CGAL::LARGER` according to the relative position of the two curves
   * immediately to the left of `p`.
   * - If one of the curves is vertical and extends downward from `p`, it is
   *   considered to lie <I>below</I> the other curve.
   * - `CGAL::EQUAL` is returned if and only if the two curves coincide
   *   immediately to the left of `p`, that is, they overlap there.
   *
   * \pre `p` lies on both `xc1` and `xc2`, and both curves are defined
   *      (lexicographically) to its left.
   * \pre `p` does not lie on a boundary side of the parameter space.
   */
  Comparison_result operator()(AosTraits::X_monotone_curve_2 xc1,
                               AosTraits::X_monotone_curve_2 xc2,
                               AosTraits::Point_2 p);

  /// @}
}; /* end AosTraits::CompareYAtXLeft_2 */

}
