#ifndef SM_VISUALIZOR_H
#define SM_VISUALIZOR_H

#include <CGAL/basic.h>
#include <CGAL/Nef_S2/SM_decorator.h>
#include <CGAL/Nef_S2/SM_triangulator.h>
#include <CGAL/Nef_S2/Sphere_geometry_OGL.h>

#define LGREY CGAL::Color(170,170,200)
#define DGREY CGAL::Color(30,30,50)
CGAL_BEGIN_NAMESPACE

template <typename Map_>
class SM_BooleColor 
{
  typedef typename Map_::SVertex_const_handle   SVertex_const_handle;   
  typedef typename Map_::SHalfedge_const_handle SHalfedge_const_handle;   
  typedef typename Map_::SHalfloop_const_handle SHalfloop_const_handle;   
  typedef typename Map_::SFace_const_handle     SFace_const_handle;   
  typedef typename Map_::Mark Mark;   
public:
  Color color(SVertex_const_handle, Mark m) const
  { return ( m ? CGAL::BLACK : CGAL::WHITE ); }
  Color color(SHalfedge_const_handle, Mark m) const
  { return ( m ? CGAL::BLACK : CGAL::WHITE ); }
  Color color(SHalfloop_const_handle, Mark m) const
  { return ( m ? CGAL::BLACK : CGAL::WHITE ); }
  Color color(SFace_const_handle, Mark m) const
  { return ( m ? DGREY : LGREY ); }
};


/*{\Moptions outfile=SM_visualizor.man }*/
/*{\Manpage {SM_visualizor}{Map_,Sphere_kernel_}
{Drawing plane maps}{V}}*/

template <typename Map_, typename Sphere_kernel_,
	  typename Color_ = SM_BooleColor<Map_> >
class SM_visualizor : public 
  SM_triangulator< SM_decorator<Map_> >
{
/*{\Mdefinition An instance |\Mvar| of the data type |\Mname| is a
decorator to draw the structure of a sphere map into the surface
of a OpenGL sphere. It is generic with respect to the template 
concept.}*/

/*{\Mgeneralization SM_decorator}*/
/*{\Mtypes 3}*/
public:
  typedef Map_    Map;
  typedef Sphere_kernel_ Sphere_kernel;
  typedef SM_visualizor<Map_,Sphere_kernel_,Color_> Self;
  typedef SM_const_decorator<Map_>                  Explorer;
  typedef SM_decorator<Map_>                        Decorator;
  typedef SM_triangulator<Decorator>                Base;

  typedef typename Map_::SVertex_const_handle SVertex_const_handle;   
  typedef typename Map_::SHalfedge_const_handle SHalfedge_const_handle; 
  typedef typename Map_::SFace_const_handle SFace_const_handle;     
  typedef typename Map_::SVertex_const_iterator SVertex_const_iterator;
  typedef typename Map_::SHalfedge_const_iterator SHalfedge_const_iterator;
  typedef typename Map_::SFace_const_iterator SFace_const_iterator;
  typedef typename Map_::Mark Mark;

  typedef typename Sphere_kernel::Sphere_point    Sphere_point;
  typedef typename Sphere_kernel::Sphere_segment  Sphere_segment;
  typedef typename Sphere_kernel::Sphere_circle   Sphere_circle;
  typedef typename Sphere_kernel::Sphere_triangle Sphere_triangle;

  typedef Color_                                  Color_objects;

protected:
  Explorer                E_;
  const Color_objects&    CO_;
  Map                     MT_;
  CGAL::OGL::Unit_sphere& S_;
public:

/*{\Mcreation 4}*/
SM_visualizor(const Map& M, CGAL::OGL::Unit_sphere& S,
	      const Color_objects& C = Color_objects())
/*{\Mcreate creates an instance |\Mvar| of type |\Mname| to visualize
    the vertices, edges, and faces of |D| in an open GL window.}*/
  : Base(M,MT_), E_(&M), CO_(C), MT_(), S_(S)
  { triangulate(); }

/*{\Moperations 2 1}*/

/* |draw_map| draws all object of the referenced sphere map:
   1) edges, loops, and vertices are taken from E_
   2) faces are drawn via the calculated triangulation in MT_  */

void draw_map() const
/*{\Mop draw the whole plane map.}*/
{
  // draw sphere segments underlying edges of E_:
  SHalfedge_const_iterator e;
  CGAL_forall_sedges(e,E_) {
    if ( source(e) == target(e) ) {
      S_.push_back(E_.circle(e), CO_.color(e,E_.mark(e))); 
    } else {
      S_.push_back(Sphere_segment(E_.point(E_.source(e)),
				  E_.point(E_.target(e)),
				  E_.circle(e)),CO_.color(e,E_.mark(e)));
    }
  }
  // draw sphere circles underlying loops of E_:

  if ( E_.has_sloop() )
    S_.push_back(
      Sphere_circle(E_.circle(E_.shalfloop())),
      CO_.color(E_.shalfloop(),E_.mark(E_.shalfloop())));

  // draw points underlying vertices of E_:
  SVertex_const_iterator v;
  CGAL_forall_svertices(v,E_)
    S_.push_back(E_.point(v),CO_.color(v,E_.mark(v)));

  Unique_hash_map<SHalfedge_const_iterator,bool> Done(false);
  CGAL_forall_shalfedges(e,*this) {
    if ( Done[e] ) continue;
    SHalfedge_const_handle en(next(e)),enn(next(en));
    TRACEV(Base::incident_triangle(e));
    TRACEN(incident_mark(e)<<incident_mark(en)<<incident_mark(enn));
    CGAL_assertion(Base::incident_mark(e)==Base::incident_mark(en) &&
		   Base::incident_mark(en)==Base::incident_mark(enn));
    Mark m = Base::incident_mark(e);
    Sphere_triangle t = Base::incident_triangle(e);
    S_.push_back(t, (m ? DGREY : LGREY) );
    Done[e]=Done[en]=Done[enn]=true;
  }

  Done.clear(false);
  CGAL_forall_shalfedges(e,*this) {
    if ( Done[e] ) continue;
    S_.push_back_triangle_edge(Sphere_segment(E_.point(E_.source(e)),
					      E_.point(E_.target(e)),
					      E_.circle(e)));
    Done[e]=Done[twin(e)]=true;
  }

}

/* |draw_triangulation| draws all object of the underlying triangulation:
   1) edges, loops, and vertices are taken from E_
   2) faces are drawn via the calculated triangulation in MT_  */

void draw_triangulation() const
{ 
  // draw sphere segments underlying edges of triangulation:
  SHalfedge_const_iterator e;
  CGAL_forall_sedges(e,*this) {
    S_.push_back(Sphere_segment(point(source(e)),point(target(e)),
				circle(e)),CO_.color(e,mark(e)));
  }

  // draw points underlying vertices of triangulation:
  SVertex_const_iterator v;
  CGAL_forall_svertices(v,*this)
    S_.push_back(point(v),CO_.color(v,mark(v)));

  Unique_hash_map<SHalfedge_const_iterator,bool> Done(false);
  CGAL_forall_shalfedges(e,*this) {
    if ( Done[e] ) continue;
    SHalfedge_const_handle en(next(e)),enn(next(en));
    CGAL_assertion(incident_mark(e)==incident_mark(en)&&
		   incident_mark(en)==incident_mark(enn));
    Mark m = incident_mark(e);
    Sphere_triangle t = incident_triangle(e);
    S_.push_back(t, (m ? DGREY : LGREY) );
    Done[e]=Done[en]=Done[enn]=true;
  }


}

}; // end of SM_visualizor 




CGAL_END_NAMESPACE
#undef CGAL_USING
//#undef LGREY
//#undef DGREY
#endif // SM_VISUALIZOR_H

