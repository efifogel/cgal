// Copyright (c) 1997-2002  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source$
// $Revision$ $Date$
// $Name$
//
// Author(s)     : Michael Seel       <seel@mpi-sb.mpg.de>
//                 Miguel Granados    <granados@mpi-sb.mpg.de>
//                 Susan Hert         <hert@mpi-sb.mpg.de>
//                 Lutz Kettner       <kettner@mpi-sb.mpg.de>
//                 Peter Hachenberger <hachenberger@mpi-sb.mpg.de>
#ifndef CGAL_SM_DECORATOR_H 
#define CGAL_SM_DECORATOR_H

#include <CGAL/basic.h>
#include <CGAL/Nef_S2/SM_const_decorator.h>

#undef _DEBUG
#define _DEBUG  23
#include <CGAL/Nef_3/debug.h>
#include <CGAL/Nef_S2/SM_decorator_traits.h>
#include <CGAL/Unique_hash_map.h>
#include <CGAL/IO/Verbose_ostream.h>

CGAL_BEGIN_NAMESPACE

/*{\Moptions print_title=yes }*/ 
/*{\Moptions outfile=SM_decorator.man }*/
/*{\Manpage {SM_decorator}{Sphere_map}
{Topological sphere map decorator}{D}}*/

template <typename Map_>
class SM_decorator
{ 
public:
typedef  Map_              Map;
typedef SM_decorator<Map> Self;
typedef SM_decorator_traits<Map>  Decorator_traits;
/*{\Mdefinition ...}*/

/*{\Mtypes 5}*/

typedef SM_const_decorator<Map>  SM_const_decorator;

typedef typename Map::Sphere_kernel    Sphere_kernel;
/*{\Mtypemember spherical geometry.}*/

typedef typename Map::Sphere_point     Sphere_point;
/*{\Mtypemember embedding vertices.}*/

typedef typename Map::Sphere_segment   Sphere_segment;
/*{\Mtypemember embedding edges.}*/

typedef typename Map::Sphere_circle    Sphere_circle;
/*{\Mtypemember embedding loops.}*/

typedef typename Map::Sphere_direction Sphere_direction;
/*{\Mtypemember embedding directions.}*/

typedef typename Map::Mark   Mark;
/*{\Mtypemember attributes of objects (vertices, edges, faces).}*/

typedef size_t Size_type;
/*{\Mtypemember size type.}*/

enum { BEFORE = -1, AFTER = 1 };
/*{\Menum insertion order labels.}*/

// typedef typename Map::Aff_transformation_3 Aff_transformation_3;

typedef void*  GenPtr;
typedef typename Map::Constructor_parameter     Constructor_parameter;
typedef typename Map::SVertex                   SVertex;
typedef typename Map::SVertex_handle            SVertex_handle;
typedef typename Map::SVertex_iterator          SVertex_iterator;
typedef typename Map::SVertex_const_handle      SVertex_const_handle;
typedef typename Map::SVertex_const_iterator    SVertex_const_iterator;
typedef typename Map::SHalfedge                 SHalfedge;
typedef typename Map::SHalfedge_handle          SHalfedge_handle;
typedef typename Map::SHalfedge_iterator        SHalfedge_iterator;
typedef typename Map::SHalfedge_const_handle    SHalfedge_const_handle;
typedef typename Map::SHalfedge_const_iterator  SHalfedge_const_iterator;
typedef typename Map::SFace                     SFace;
typedef typename Map::SFace_handle              SFace_handle;
typedef typename Map::SFace_iterator            SFace_iterator;
typedef typename Map::SFace_const_handle        SFace_const_handle;
typedef typename Map::SFace_const_iterator      SFace_const_iterator;
typedef typename Map::SHalfloop                 SHalfloop;
typedef typename Map::SHalfloop_handle          SHalfloop_handle;
typedef typename Map::SHalfloop_iterator        SHalfloop_iterator;
typedef typename Map::SHalfloop_const_handle    SHalfloop_const_handle;
typedef typename Map::SHalfloop_const_iterator  SHalfloop_const_iterator;
typedef typename Map::Object_handle             Object_handle;
typedef typename Map::SHalfedge_around_svertex_circulator
                      SHalfedge_around_svertex_circulator;
typedef typename Map::SHalfedge_around_sface_circulator
                      SHalfedge_around_sface_circulator;
typedef typename Map::SFace_cycle_iterator      SFace_cycle_iterator;

/*{\Mtypemember iterating all face cycles of an face |f|.
The iterator has method |bool is_svertex()|, |bool is_shalfedge()|,
|bool is_shalfloop()|, and can be converted to the corresponding
handles |SVertex_handle|, |SHalfedge_handle|, or 
|SHalfloop_handle|.}*/

protected: 
  Constructor_parameter psm_;

public:
/*{\Mcreation 3}*/
SM_decorator() : psm_(0) {}
SM_decorator(const Self& D) : psm_(D.psm_) {}
Self& operator=(const Self& D) { psm_=D.psm_; return *this; }

SM_decorator(Constructor_parameter M) : psm_(M) {}
/*{\Mcreate constructs a plane map decorator exploring |M|.}*/
 
/*{\Moperations 4 4}*/

Constructor_parameter center_vertex() const { return psm_; }
Map* map() const { return psm_->map(); }

SVertex_const_handle source(SHalfedge_const_handle e) const
{ return e->source_; }
SVertex_const_handle target(SHalfedge_const_handle e) const
{ return e->twin_->source_; }
SHalfedge_const_handle twin(SHalfedge_const_handle e) const 
{ return e->twin_; }
SHalfloop_const_handle twin(SHalfloop_const_handle l) const 
{ return l->twin_; }

SVertex_handle source(SHalfedge_handle e) const
/*{\Mop returns the source of |e|.}*/
{ return e->source_; }

SVertex_handle target(SHalfedge_handle e) const
/*{\Mop returns the target of |e|.}*/
{ return e->twin_->source_; }

SHalfedge_handle twin(SHalfedge_handle e) const 
/*{\Mop returns the twin of |e|.}*/
{ return e->twin_; }

SHalfloop_handle twin(SHalfloop_handle l) const 
/*{\Mop returns the twin of |l|.}*/
{ return l->twin_; }

bool is_isolated(SVertex_const_handle v) const
{ return (v->out_sedge_ == SHalfedge_handle()); }

bool is_isolated(SVertex_handle v) const
/*{\Mop returns |true| when |v| is linked to the interior of a face.}*/
{ return (v->out_sedge_ == SHalfedge_handle()); }

SHalfedge_const_handle first_out_edge(SVertex_const_handle v) const
{ return v->out_sedge_; }
SHalfedge_const_handle last_out_edge(SVertex_const_handle v) const
{ return cap(v->out_sedge_); }

SHalfedge_handle first_out_edge(SVertex_handle v) const
/*{\Mop returns one edge with source |v|. It's the starting point for
  the circular iteration over the edges with source |v|.
  \precond |!is_isolated(v)|.}*/
{ return v->out_sedge_; }

SHalfedge_handle last_out_edge(SVertex_handle v) const
/*{\Mop returns one edge with source |v|. \precond |!is_isolated(v)|.}*/
{ return cap(v->out_sedge_); }

SHalfedge_const_handle cyclic_adj_succ(SHalfedge_const_handle e) const
{ return e->sprev_->twin_; }
SHalfedge_const_handle cyclic_adj_pred(SHalfedge_const_handle e) const
{ return e->twin_->snext_; }

SHalfedge_handle cyclic_adj_succ(SHalfedge_handle e) const
/*{\Mop returns the edge after |e| in the cyclic ordered adjacency list of
  |source(e)|.}*/
{ return e->sprev_->twin_; }

SHalfedge_handle cyclic_adj_pred(SHalfedge_handle e) const
/*{\Mop returns the edge before |e| in the cyclic ordered adjacency list of
  |source(e)|.}*/
{ return e->twin_->snext_; }

SHalfedge_const_handle next(SHalfedge_const_handle e) const
{ return e->snext_; }
SHalfedge_const_handle previous(SHalfedge_const_handle e) const
{ return e->sprev_; }

SHalfedge_handle next(SHalfedge_handle e) const
/*{\Mop returns the next edge in the face cycle containing |e|.}*/
{ return e->snext_; }

SHalfedge_handle previous(SHalfedge_handle e) const
/*{\Mop returns the previous edge in the face cycle containing |e|.}*/
{ return e->sprev_; }

SFace_const_handle face(SHalfedge_const_handle e) const
{ return e->incident_sface_; }
SFace_const_handle face(SHalfloop_const_handle l) const
{ return l->incident_sface_; }
SFace_const_handle face(SVertex_const_handle v) const
{ return v->incident_sface_; }

SFace_handle face(SHalfedge_handle e) const
/*{\Mop returns the face incident to |e|.}*/
{ return e->incident_sface_; }

SFace_handle face(SHalfloop_handle l) const
/*{\Mop returns the face incident to |l|.}*/
{ return l->incident_sface_; }

SFace_handle face(SVertex_handle v) const
/*{\Mop returns the face incident to |v|. \precond |is_isolated(v)|.}*/
{ return v->incident_sface_; }

bool has_sloop() const
/*{\Mop returns true iff there is a loop.}*/
{ return psm_->has_sloop(); }

/*{\Mtext \headerline{Iteration} \setopdims{3.3cm}{0cm}}*/
  
SVertex_iterator svertices_begin() const
{ return psm_->svertices_begin(); }
SVertex_iterator svertices_end() const
{ return psm_->svertices_end(); }
SHalfedge_iterator shalfedges_begin() const
{ return psm_->shalfedges_begin(); }
SHalfedge_iterator shalfedges_end() const
{ return psm_->shalfedges_end(); }
SFace_iterator sfaces_begin() const
{ return psm_->sfaces_begin(); }
SFace_iterator sfaces_end() const
{ return psm_->sfaces_end(); }
SHalfloop_iterator shalfloops_begin() const
{ return psm_->shalfloops_begin(); }
SHalfloop_iterator shalfloops_end() const
{ return psm_->shalfloops_end(); }

SHalfloop_handle shalfloop() const
{ return psm_->shalfloop(); }

Size_type number_of_svertices() const 
/*{\Mop returns the number of vertices.}*/
{ return psm_->number_of_svertices(); }

Size_type number_of_shalfedges() const 
/*{\Mop returns the number of halfedges.}*/
{ return psm_->number_of_shalfedges(); }

Size_type number_of_sedges() const 
/*{\Mop returns the number of edges.}*/
{ return number_of_shalfedges()/2; }

Size_type number_of_shalfloops() const 
/*{\Mop returns the number of halfloops.}*/
{ return psm_->number_of_shalfloops(); }

Size_type number_of_sloops() const 
/*{\Mop returns the number of loops.}*/
{ return psm_->number_of_shalfloops()/2; }

Size_type number_of_sfaces() const    
/*{\Mop returns the number of faces.}*/
{ return psm_->number_of_sfaces(); }

SFace_cycle_iterator sface_cycles_begin(SFace_handle f) const
/*{\Mop returns an iterator for all bounding face cycles of |f|.
The iterator is is convertable to |SVertex_handle|, 
|SHalfloop_handle|, or |SHalfedge_handle|.}*/
{ return f->boundary_entry_objects_.begin(); }

SFace_cycle_iterator sface_cycles_end(SFace_handle f) const
/*{\Mop returns the past the end iterator of |f|.}*/
{ return f->boundary_entry_objects_.end(); }

SHalfedge_around_svertex_circulator 
  out_edges(SVertex_handle v) const
/*{\Mop returns a circulator for the cyclic adjacency list of |v|.
\precond the adjacency list is not empty.}*/
{ return SHalfedge_around_svertex_circulator(first_out_edge(v)); }

/*{\Mtext \headerline{Update Operations}}*/

void clear() const
/*{\Mop reintializes |P| to the empty map.}*/
{ psm_->clear(); }

bool is_closed_at_source(SHalfedge_handle e) const
/*{\Mop returns |true| when |prev(e) == twin(e)|.}*/
{ return previous(e) == twin(e); }

bool is_closed_at_target(SHalfedge_handle e) const
/*{\Mop returns |true| when |next(e) == twin(e)|.}*/
{ return next(e) == twin(e); }

SHalfedge_handle cas(SHalfedge_handle e) const 
{ return cyclic_adj_succ(e); } 
SHalfedge_handle cap(SHalfedge_handle e) const
{ return cyclic_adj_pred(e); }

SVertex_handle new_svertex(const Sphere_point& p) const
/*{\Mop creates a new vertex.}*/
{ return map()->new_svertex(p); }

SHalfedge_handle new_shalfedge_pair() const
/*{\Xop creates a new edge pair. No connectivity is provided.}*/
{ return map()->new_shalfedge_pair(); }

SHalfloop_handle new_sloop_pair() const
/*{\Mop creates a new loop pair.
\precond No sloop pair exists in the local graph.}*/ 
{ CGAL_assertion(!has_sloop());
  return map()->new_shalfloop_pair(); }

SFace_handle new_sface() const
/*{\Mop creates a new face.}*/
{ return map()->new_sface(); }

void delete_vertex_only(SVertex_handle v) const
/*{\Mop deletes |v| without any connectivity update.}*/
{ map()->delete_vertex(v); }

void delete_edge_pair_only(SHalfedge_handle e) const
/*{\Mop deletes |e| and its twin without any connectivity update.}*/
{ map()->delete_halfedge_pair(e); }

void delete_halfedge_only(SHalfedge_handle e) const
/*{\Mop deletes |e| without its twin and without any connectivity update.}*/
{ map()->delete_halfedge(e); }

void delete_face_only(SFace_handle f) const
/*{\Mop deletes |f| without any connectivity update.}*/
{ map()->delete_face(f); }

void delete_loop_only() const
/*{\Mop deletes the loop and its twin without any connectivity update.}*/ 
{ CGAL_assertion(psm_->loops_);
  map()->delete_halfloop_pair(psm_->loops_); }

template <typename H>
bool is_boundary_object(H h) const
{ return map()->is_boundary_object(h); }

template <typename H>
void store_boundary_object(H h, SFace_handle f) const { 
  CGAL_assertion(!map()->is_boundary_object(h));
  f->boundary_entry_objects_.push_back(Object_handle(h));
  map()->store_boundary_item(h, --(f->sface_cycles_end()));
}

template <typename H>
void undo_boundary_object(H h, SFace_handle f) const
{ CGAL_assertion(psm_->is_boundary_object(h));
  SFace_cycle_iterator it = psm_->boundary_item(h);
  psm_->undef_boundary_item(h);
  f->boundary_entry_objects_.erase(it);
}

void link_as_face_cycle(SHalfedge_handle e, SFace_handle f) const
/*{\Mop creates a new face cycle of |f| and 
   makes |e| the entry point of it.}*/
{
  SHalfedge_around_sface_circulator hfc(e), hend(hfc);
  CGAL_For_all(hfc,hend) hfc->incident_sface_ = f;
  store_boundary_object(e,f);
} 

void link_as_loop(SHalfloop_handle l, SFace_handle f) const
/*{\Mop creates a new trivial face cycle of |f| and 
   makes |l| the singular object of it.}*/
{ store_boundary_object(l,f); l->incident_sface_ = f; } 

void link_as_isolated_vertex(SVertex_handle v, SFace_handle f) const
/*{\Mop creates a new trivial face cycle of |f|.
   (makes |v| an isolated vertex within |f|).}*/
{ store_boundary_object(v,f); v->incident_sface_ = f; } 

void unlink_as_face_cycle(SHalfedge_handle e) const
/*{\Mop removes the face cycle defined by |e| from |face(e)|.
    Does not update the face links of the corresponding face cycle
    edges. \precond |e| is the entry object of the face cycle.}*/
{ undo_boundary_object(e,face(e)); }
  
void unlink_as_loop(SHalfloop_handle l) const
/*{\Mop removes the trivial face cycle defined by |l| from
   |face(l)|. Does not update |l|'s face link.}*/
{ undo_boundary_object(l,face(l)); }

void unlink_as_isolated_vertex(SVertex_handle v) const
/*{\Mop removes the trivial face cycle defined by |v| from
   |face(v)|. Does not update |v|'s face link.
   \precond |v| is a trivial face cycle of |face(v)|.}*/
{ undo_boundary_object(v,face(v)); }

void clear_face_cycle_entries(SFace_handle f) const
{ map()->reset_object_list(f->boundary_entry_objects_);
  // removes entries of list and the hashed membership
}

SHalfedge_handle new_shalfedge_pair(SVertex_handle v1, 
			      SVertex_handle v2) const
/*{\Mop creates a new pair of edges |(e1,e2)| representing |(v1,v2)| 
  by appending the |ei| to |A(vi)| $(i=1,2)$.}*/
{ SHalfedge_handle e1 = new_shalfedge_pair();
  SHalfedge_handle e2 = twin(e1);
  if (!is_isolated(v1)) 
    set_adjacency_at_source_between(cap(first_out_edge(v1)),e1,
                                    first_out_edge(v1));
  else
    close_tip_at_source(e1,v1);
  if (!is_isolated(v2))
    set_adjacency_at_source_between(cap(first_out_edge(v2)),e2,
                                    first_out_edge(v2));
  else 
    close_tip_at_source(e2,v2);
  return e1;
}


SHalfedge_handle new_shalfedge_pair(SHalfedge_handle e1, 
			      SHalfedge_handle e2,
			      int pos1 = AFTER, int pos2 = AFTER) const
/*{\Mop creates a new pair of edges |(es1,es2)| representing the uedge
  |\{source(e1),source(e2)\}| by inserting the |esi| before or after |ei| 
  into the cyclic adjacency list of |source(ei)| depending on |posi| 
  $(i=1,2)$ from |\Mname::BEFORE|, |\Mname::AFTER|.}*/
{ 
  SHalfedge_handle er = new_shalfedge_pair();
  SHalfedge_handle ero = twin(er);
  if (pos1 < 0) { // before e1
    set_adjacency_at_source_between(cap(e1),er,e1);
    if ( e1 == first_out_edge(source(e1)) )
      set_first_out_edge(source(e1),er);
  } else { // after e1
    set_adjacency_at_source_between(e1,er,cas(e1));
  }
  if (pos2 < 0) { // before e2
    set_adjacency_at_source_between(cap(e2),ero,e2);
    if ( e2 == first_out_edge(source(e2)) )
      set_first_out_edge(source(e2),ero);
  } else { // after e2
    set_adjacency_at_source_between(e2,ero,cas(e2));
  }
  return er;
}

SHalfedge_handle new_shalfedge_pair(SHalfedge_handle e, SVertex_handle v,
                               int pos = AFTER) const
/*{\Mop creates a new pair of edges  |(e1,e2)| representing the uedge
  |\{source(e),v\}| by inserting |e1| before or after |e| into cyclic 
  adjacency list of |source(e)| depending on |pos| from |\Mname::BEFORE|,
  |\Mname::AFTER| and appending |e2| at |A(v)|.}*/
{
  SHalfedge_handle e_new = new_shalfedge_pair();
  SHalfedge_handle e_opp = twin(e_new);
  if (pos < 0) { // before e
    set_adjacency_at_source_between(cap(e),e_new,e);
    if ( e == first_out_edge(source(e)) )
      set_first_out_edge(source(e),e_new);
  } else  // after e
    set_adjacency_at_source_between(e,e_new,cas(e));
  
  if (!is_isolated(v)) {
    SHalfedge_handle e_first = first_out_edge(v);
    set_adjacency_at_source_between(cap(e_first),e_opp,e_first);
  } else
    close_tip_at_source(e_opp,v);
  return e_new;
}


SHalfedge_handle new_shalfedge_pair(SVertex_handle v, SHalfedge_handle e,
                           int pos = AFTER) const
/*{\Mop symmetric to the previous one.}*/
{ return twin(new_shalfedge_pair(e,v,pos)); }

void delete_edge_pair(SHalfedge_handle e) const
/*{\Mop deletes |e| and its twin and maintains the adjacency at its source 
        and its target.}*/
{ remove_from_adj_list_at_source(e);
  remove_from_adj_list_at_source(twin(e));
  delete_edge_pair_only(e);
}

void delete_vertex(SVertex_handle v) const
/*{\Mop deletes |v| and all outgoing edges |A(v)| as well as their twins. 
   Updates the adjacency at the targets of the edges in |A(v)|.}*/
{ 
  if (!is_isolated(v)) {
    SHalfedge_handle e = first_out_edge(v);
    while ( e != cap(e) ) 
      delete_edge_pair(cap(e));  
    delete_edge_pair(e); 
  }
  delete_vertex_only(v);
}

void delete_face(SFace_handle f) const
/*{\Mop deletes the face |f| without consideration of topological 
   linkage.}*/
{ clear_face_cycle_entries(f); delete_face_only(f); }

bool has_outdeg_two(SVertex_handle v) const
/*{\Mop return true when |v| has outdegree two.}*/
// does this work for looping edges?
{ if (is_isolated(v)) return false;
  SHalfedge_handle e1 = first_out_edge(v);
  SHalfedge_handle e2 = last_out_edge(v);
  return (e1!=e2 && e2==cas(e1));
}

void link_as_prev_next_pair(SHalfedge_handle e1, SHalfedge_handle e2) const 
/*{\Xop makes |e1| and |e2| adjacent in the face cycle 
   $\ldots-|e1-e2|-\ldots$.
   Afterwards |e1 = previous(e2)| and |e2 = next(e1)|.}*/
{ e1->snext_ = e2; e2->sprev_ = e1; }

void merge_edge_pairs_at_target(SHalfedge_handle e) const
/*{\Mop merges the edge pairs at |v = target(e)|. |e| and |twin(e)| 
  are preserved, |next(e)|, |twin(next(e))| and |v| are deleted
  in the merger. \precond |v| has outdegree two. The adjacency at 
  |source(e)| and |target(next(e))| is kept consistent.
  If |next(e)| was entry point of |face(e)| then |e| takes this role.
  The same holds for |twin(next(e))| and |face(twin(e))|.}*/
{
  TRACEN("merge_edge_pairs_at_target "<<PH(e));
  SHalfedge_handle en = next(e), eno = twin(en), enn, enno,
               eo = twin(e) ;
  if ( is_closed_at_target(en) ) { enn = eo; enno=e; }
  else { enn = next(en), enno = previous(eno); }
  SVertex_handle v = target(e), vn = target(en);
  CGAL_assertion(has_outdeg_two(v));
  SFace_handle f1 = face(en), f2 = face(eno);
  // transfer the opposite face cycles e-en-enn to e-enn
  if ( enn != eno ) {
    link_as_prev_next_pair(e,enn);
    link_as_prev_next_pair(enno,eo);
  } else {
    link_as_prev_next_pair(e,eo);
  }
  // set vertex of e and deal with vertex-halfedge incidence
  eo->source_ = vn;

  if ( first_out_edge(vn) == eno ) set_first_out_edge(vn,eo);
  if ( is_boundary_object(en) ) 
  { undo_boundary_object(en,f1); store_boundary_object(e,f1); }
  if ( is_boundary_object(eno) )
  { undo_boundary_object(eno,f2); store_boundary_object(eo,f2); }
  delete_vertex_only(v);
  delete_edge_pair_only(en);
  TRACEN("END "<<PH(previous(e))<<PH(e)<<PH(next(e)));
}

void convert_edge_to_loop(SHalfedge_handle e) const
/*{\Mop converts the edge at |v = target(e)| to the unique
  loop |l| of |\Mvar|. |e|, |twin(e)| and |v| are deleted
  in the conversion. \precond there was no loop in |\Mvar|. 
  As |e| was entry point of |face(e)| then |l| takes this role.}*/
{ TRACEN("convert_edge_to_loop "<<PH(e));
  CGAL_assertion( !has_sloop() );
  SHalfloop_handle l = new_sloop_pair();
  SVertex_handle v = target(e);
  SFace_handle f1 = face(e), f2 = face(twin(e));
  CGAL_assertion( is_boundary_object(e) && 
                  is_boundary_object(twin(e)) );
  undo_boundary_object(e,f1); undo_boundary_object(twin(e),f2);
  link_as_loop(l,f1), link_as_loop(twin(l),f2);
  circle(l) = circle(e); circle(twin(l)) = circle(twin(e));
  mark(l) = mark(e);
  delete_vertex_only(v);
  delete_edge_pair_only(e);
}

void flip_diagonal(SHalfedge_handle e) const
{ SHalfedge_handle r = twin(e);
  SHalfedge_handle en = next(e), enn= next(en);
  SHalfedge_handle rn = next(r), rnn= next(rn);
  TRACEN(PH(e)<<PH(en)<<PH(enn));
  TRACEN(PH(r)<<PH(rn)<<PH(rnn));
  CGAL_assertion( next(enn)==e && next(rnn)==r );
  remove_from_adj_list_at_source(e);
  remove_from_adj_list_at_source(r);
  set_adjacency_at_source_between(enn,e,twin(en));
  set_adjacency_at_source_between(rnn,r,twin(rn));
}
     

/*{\Mtext \headerline{Incomplete topological update primitives}}*/

SHalfedge_handle new_shalfedge_pair_at_source
  (SHalfedge_handle e, int pos = AFTER) const
/*{\Xop creates a new pair of edges  |(e1,e2)| representing |(source(e),())| 
  by inserting |e1| before or after |e| into cyclic adjacency list of
  |source(e)| depending on |pos| from |\Mname::BEFORE, \Mname::AFTER|.}*/
{ SHalfedge_handle e_new = new_shalfedge_pair();
  if (pos < 0) set_adjacency_at_source_between(cap(e),e_new,e);
  else         set_adjacency_at_source_between(e,e_new,cas(e));
  return e_new;
}

SHalfedge_handle new_shalfedge_pair_at_source
  (SVertex_handle v, int pos = AFTER) const
/*{\Mop creates a new pair of edges  |(e1,e2)| representing |(v,())| 
  by inserting |e1| at the beginning (BEFORE) or end (AFTER)
  of adjacency list of |v|.}*/
{ SHalfedge_handle e1 = new_shalfedge_pair();
  if (!is_isolated(v)) {
    SHalfedge_handle ef = first_out_edge(v);
    if (pos < 0) { // before e1
      set_adjacency_at_source_between(cap(ef),e1,ef);
      set_first_out_edge(v,e1);
    } else {
      set_adjacency_at_source_between(ef,e1,cas(ef));
    }
  } else
    close_tip_at_source(e1,v);
  return e1;
}

void delete_edge_pair_at_source(SHalfedge_handle e) const
/*{\Mop deletes |e| and its twin and maintains the adjacency at its 
  source.}*/
{ remove_from_adj_list_at_source(e);
  delete_edge_pair_only(e);
}

void link_as_target_and_append(SVertex_handle v, SHalfedge_handle e) const
/*{\Mop makes |v| the target of |e| appends |twin(e)| to the adjacency list
   of |v|.}*/
{ if (!is_isolated(v)) 
    set_adjacency_at_source_between(cap(first_out_edge(v)),twin(e),
      first_out_edge(v));
  else
    close_tip_at_target(e,v);
}

void link_as_source_of(SHalfedge_handle e, SVertex_handle v) const
/*{\Mop makes |source(e) = v| and sets |e| as the first
        out edge if |v| was isolated before.}*/
{ e->source_ = v;
  if (v->out_sedge_ == SHalfedge_handle()) v->out_sedge_ = e; }

void link_as_target_of(SHalfedge_handle e, SVertex_handle v) const
/*{\Mop makes |target(e) = v| and sets |e| as the first
  in edge if |v| was isolated before.}*/
{ link_as_source_of(twin(e),v); }

void set_adjacency_at_source_between(SHalfedge_handle e, SHalfedge_handle en) 
  const 
/*{\Mop makes |e| and |en| neigbors in the cyclic ordered adjacency list 
    around |v=source(e)|. \precond |source(e)==source(en)|.}*/
{ CGAL_assertion(source(e)==source(en));
  link_as_prev_next_pair(twin(en),e);
}

void set_adjacency_at_source_between(SHalfedge_handle e1, 
                                     SHalfedge_handle e_between, 
                                     SHalfedge_handle e2) const 
/*{\Mop inserts |e_between| into the adjacency list around |source(e1)| 
  between |e1| and |e2| and makes |source(e1)| the source of |e_between|. 
  \precond |source(e1)==source(e2)|.}*/
{ 
  e_between->source_ = source(e1);
  set_adjacency_at_source_between(e1,e_between);
  set_adjacency_at_source_between(e_between,e2);
}

void close_tip_at_source(SHalfedge_handle e, SVertex_handle v) const 
/*{\Mop sets |v| as source of |e| and closes the tip by setting the 
  corresponding pointers such that |prev(e) == twin(e)| and
  |next(twin(e)) == e|.}*/
{ link_as_source_of(e,v); 
  link_as_prev_next_pair(twin(e),e); }

void close_tip_at_target(SHalfedge_handle e, SVertex_handle v) const 
/*{\Mop sets |v| as target of |e| and closes the tip by setting the 
  corresponding pointers such that |prev(twin(e)) == e| and 
  |next(e) == twin(e)|.}*/
{ link_as_target_of(e,v);
  link_as_prev_next_pair(e,twin(e)); }


void remove_from_adj_list_at_source(SHalfedge_handle e) const
/*{\Mop removes a halfedge pair |(e,twin(e)| from the adjacency list
of |source(e)|. Afterwards |next(prev(e))==next(twin(e))| and
|first_out_edge(source(e))| is valid if |degree(source(v))>1| before
the operation.}*/
{
  SVertex_handle v = source(e);
  if ( is_closed_at_source(e) ) { // last outgoing
    v->out_sedge_ = SHalfedge_handle();
  } else {
    if (e == first_out_edge(v)) v->out_sedge_ = cap(e);
    set_adjacency_at_source_between(cap(e),cas(e));
  }
}

void set_face(SHalfedge_handle e, SFace_handle f) const
{ e->incident_sface_ = f; }
void set_face(SVertex_handle v, SFace_handle f) const
{ v->incident_sface_ = f; }
void set_first_out_edge(SVertex_handle v, SHalfedge_handle e) const
{ v->out_sedge_ = e; }
void set_prev(SHalfedge_handle e, SHalfedge_handle ep) const
{ e->sprev_ = ep; }
void set_next(SHalfedge_handle e, SHalfedge_handle en) const
{ e->snext_ = en; }
void set_source(SHalfedge_handle e, SVertex_handle v) const
{ e->source_ = v; }



/*{\Mtext \headerline{Associated Information}\restoreopdims}*/

Sphere_point& point(SVertex_handle v) const
/*{\Mop returns the embedding of |v|.}*/
{ return v->point_; }

Sphere_circle& circle(SHalfedge_handle e) const
/*{\Mop returns the plane supporting |e|.}*/
{ return e->circle_; }

Sphere_circle& circle(SHalfloop_handle l) const
/*{\Mop returns the plane supporting |e|.}*/
{ return l->circle_; }

Mark& mark(SVertex_handle v) const
/*{\Mop returns the mark of |v|.}*/
{ return v->mark_; }

Mark& mark(SHalfedge_handle e) const
/*{\Mop returns the mark of |e|.}*/
{ return ( &*e < &*twin(e) ) ? e->mark_ : twin(e)->mark_; }

Mark& mark(SHalfloop_handle l) const
/*{\Mop returns the mark of |l|.}*/
{ return ( &*l < &*twin(l) ) ? l->mark_ : twin(l)->mark_; }

Mark& mark(SFace_handle f) const
/*{\Mop returns the mark of |f|.}*/
{ return f->mark_; }

void unify_marks(SHalfedge_handle e) const
{ if ( &*e < &*twin(e) ) twin(e)->mark_ = e->mark_; 
  else                   e->mark_ = twin(e)->mark_;
}

const Sphere_point& point(SVertex_const_handle v) const
{ return v->point_; }
const Sphere_circle& circle(SHalfedge_const_handle e) const
{ return e->circle_; }
const Sphere_circle& circle(SHalfloop_const_handle l) const
{ return l->circle_; }

const Mark& mark(SVertex_const_handle v) const
{ return v->mark_; }
const Mark& mark(SHalfedge_const_handle e) const
{ return ( &*e < &*twin(e) ) ? e->mark_ : twin(e)->mark_; }
const Mark& mark(SHalfloop_const_handle l) const
{ return ( &*l < &*twin(l) ) ? l->mark_ : twin(l)->mark_; }
const Mark& mark(SFace_const_handle f) const
{ return f->mark_; }

void unify_tmp_marks(SHalfedge_handle e) const
{ if (&*e < &*twin(e)) twin(e)->mark_ = e->mark_; 
  else e->mark_ = twin(e)->mark_; }

void set_marks_in_face_cycle(SHalfedge_handle e, Mark m) const
{ SHalfedge_around_sface_circulator hfc(e), hend(hfc);
  CGAL_For_all(hfc,hend) mark(hfc) = mark(target(hfc)) = m;
}

Mark& mark_of_halfsphere(int i) const
{ CGAL_assertion(i);
  if (i<0) return psm_->m_neg_; 
  return psm_->m_pos_; }

GenPtr& info(SVertex_handle v) const
{ return v->info_; }
GenPtr& info(SHalfedge_handle e) const
{ return e->info_; }
GenPtr& info(SHalfloop_handle l) const
{ return l->info_; }
GenPtr& info(SFace_handle f) const
{ return f->info_; }

const GenPtr& info(SVertex_const_handle v) const
{ return v->info_; }
const GenPtr& info(SHalfedge_const_handle e) const
{ return e->info_; }
const GenPtr& info(SHalfloop_const_handle l) const
{ return l->info_; }
const GenPtr& info(SFace_const_handle f) const
{ return f->info_; }

 
/*{\Mtext \headerline{Iteration}}*/
/*{\Mtext The list of all objects can be accessed via iterator ranges.
For comfortable iteration we also provide iterations macros. 
The iterator range access operations are of the following kind:\\
|SVertex_iterator vertices_begin()/vertices_end()|\\
|SHalfedge_iterator halfedges_begin()/halfedges_end()|\\
|SFace_iterator faces_begin()/faces_end()|

The macros are then |CGAL_forall_svertices_of(v,V)|,
|CGAL_forall_shalfedges_of(e,V)|, |CGAL_forall_sedges_of(e,V)|, 
|CGAL_forall_sfaces_of(f,V)|, |CGAL_forall_sface_cycles_of(fc,F)|.}*/

bool is_valid( Unique_hash_map<SVertex_handle,bool>& sv_hash,
	       Unique_hash_map<SHalfedge_handle,bool>& se_hash,
	       Unique_hash_map<SFace_handle,bool>& sf_hash,
	       bool verb = false, int level = 0) {
    
  Verbose_ostream verr(verb);
  verr << "begin CGAL::SNC_SM_decorator<...>::is_valid( verb=true, "
    "level = " << level << "):" << std::endl;
    
  bool valid = true;

  int count = 0;
  int max = 2 * number_of_svertices() 
    + 2 * number_of_shalfedges()
    + number_of_sfaces()
    + 2;
  
  SVertex_handle sv;
  int isolated_vertices_found = 0;
  CGAL_forall_svertices(sv,*this) {
    valid = valid && (!sv_hash[sv]);
    sv_hash[sv] = true;
    if(is_isolated(sv)) 
      isolated_vertices_found++;
    valid = valid && (++count <= max);
  }

  SHalfedge_iterator she;
  CGAL_forall_shalfedges(she,*this) {
    valid = valid && she->is_valid(verb, level);

    valid = valid && (twin(she) != she);
    valid = valid && (twin(twin(she)) == she);
    valid = valid && (previous(next(she)) == she);
    valid = valid && ((previous(she) != she && next(she) != she) || 
		      (previous(she) == she && next(she) == she));
    valid = valid && (face(she) == face(next(she)));
    valid = valid && (face(she) == face(previous(she)));

    valid = valid && (!se_hash[she]);

    //    Plane_3 pl(point(source(she)),point(target(she)),Point_3(0,0,0));
    //    Sphere_point vct(pl.orthogonal_vector());
    //    valid = valid && (normalized(Sphere_point(circle(she).orthogonal_vector())) == normalized(vct) || 
    //	      normalized(Sphere_point(circle(she).opposite().orthogonal_vector())) == normalized(vct));

    se_hash[she] = true;
    valid = valid && (++count <= max);
  }

  if(has_sloop()) {
    SHalfloop_handle shl = shalfloop();
    valid = valid && shl->is_valid();
    valid = valid && twin(shl)->is_valid();
    valid = valid && (twin(shl) != shl);
    valid = valid && (twin(twin(shl)) == shl); 
  }

  SFace_iterator sf;
  SFace_cycle_iterator sfc;  
  int loop_entries_found = 0;
  int edge_entries_found = 0;
  int vertex_entries_found = 0;
  CGAL_forall_sfaces(sf,*this) {
    valid = valid && sf->is_valid(verb, level);
    valid = valid && (!sf_hash[sf]);
    sf_hash[sf] = true;
    
    CGAL_forall_sface_cycles_of(sfc,sf) {
      if (sfc.is_shalfloop()) 
	loop_entries_found++;
      else if(sfc.is_shalfedge())
	edge_entries_found++;
      else if(sfc.is_svertex())
	vertex_entries_found++;
      valid = valid && (++count <= max);
    }
    
    valid = valid && (++count <= max);
  }

  if(has_sloop())
    valid = valid && (loop_entries_found == 2);
  else
    valid = valid && (loop_entries_found == 0);

  if(number_of_shalfedges() > 0)
    valid = valid && (edge_entries_found > 0);
  else
    valid = valid && (edge_entries_found == 0);

  valid = valid && (vertex_entries_found == isolated_vertices_found);

  verr << "end of CGAL::SNC_SM_decorator<...>::is_valid(): structure is "
       << ( valid ? "valid." : "NOT VALID.") << std::endl;

  return valid;
}

  template <typename Selection>
  void change_marks(const Mark& m, const Selection& SP) {
   
    psm_->mark_ = SP(m, psm_->mark_);

    SVertex_iterator v;
    CGAL_forall_svertices(v,*this)
      mark(v) = SP(m, mark(v));

    SHalfedge_iterator e;
    CGAL_forall_sedges(e,*this)
      mark(e) = SP(m, mark(e));

    SFace_iterator f;
    CGAL_forall_sfaces(f,*this) 
      mark(f) = SP(m, mark(f));
  }

  template <typename Selection>
  void change_marks(const Selection& SP, const Mark& m) {
   
    psm_->mark_ = SP(psm_->mark_, m);

    SVertex_iterator v;
    CGAL_forall_svertices(v,*this)
      mark(v) = SP(mark(v),m);

    SHalfedge_iterator e;
    CGAL_forall_sedges(e,*this)
      mark(e) = SP(mark(e),m);

    SFace_iterator f;
    CGAL_forall_sfaces(f,*this) 
      mark(f) = SP(mark(f),m);
  }

void check_integrity_and_topological_planarity(bool faces=true) const {
  SM_const_decorator C(psm_);
  C.check_integrity_and_topological_planarity(faces);
}

}; // SM_decorator


CGAL_END_NAMESPACE
#endif // CGAL_SM_DECORATOR_H


