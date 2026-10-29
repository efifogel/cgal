namespace CGAL {
namespace Minkowski_sum_2 {

/*! \ingroup PkgMinkowskiSum2Ref
 *
 * The representation of the convolution cycles computed by the full
 * convolution method.
 *
 * It is given by the named parameter `convolution_curves`.
 *
 * \sa `CGAL::Minkowski_sum_2::minkowski_sum_by_full_convolution_2()`
 * \sa `CGAL::Minkowski_sum_2::minkowski_difference_by_full_convolution_2()`
 */
enum class Convolution_curves {
  /*! The representation is chosen automatically, based on the convolution
   * cycles: segments are chosen when the cycles fold over themselves densely,
   * and polylines are chosen otherwise.
   */
  AUTOMATIC,

  /*! The convolution cycles are represented by polylines, which is preferable
   * when they intersect sparsely, as is the case when the polygons have few
   * reflex vertices.
   */
  POLYLINES,

  /*! The convolution cycles are represented by segments, which is preferable
   * when they intersect densely, as is the case when the polygons have many
   * reflex vertices.
   */
  SEGMENTS
};

} // namespace Minkowski_sum_2
} // namespace CGAL
