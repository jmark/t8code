/*
  This file is part of t8code.
  t8code is a C library to manage a collection (a forest) of multiple
  connected adaptive space-trees of general element classes in parallel.

  Copyright (C) 2015 the developers

  t8code is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  t8code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with t8code; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <p4est_bits.h>
#include <t8_schemes/t8_default/t8_dline_bits.h>
#include <t8_schemes/t8_default/t8_default_common_cxx.hxx>
#include <t8_schemes/t8_default/t8_default_vertex_cxx.hxx>

#include "t8_subelements_quad_cxx.hxx"

/* We want to export the whole implementation to be callable from "C" */
T8_EXTERN_C_BEGIN ();

/* This function is used by other element functions and we thus need to
 * declare it up here */
t8_linearidx_t      t8_element_get_linear_id (const t8_element_t * elem,
                                              int level);

#ifdef T8_ENABLE_DEBUG

#if 0
/* TODO: Used in one assertion in t8_element_nca, but wrongly triggers error there.
         Investigate and decide whether we really need this. */
static int
t8_element_surround_matches (const p4est_quadrant_t * q,
                             const p4est_quadrant_t * r)
{
  return T8_QUAD_GET_TDIM (q) == T8_QUAD_GET_TDIM (r) &&
    (T8_QUAD_GET_TDIM (q) == -1 ||
     (T8_QUAD_GET_TNORMAL (q) == T8_QUAD_GET_TNORMAL (r) &&
      T8_QUAD_GET_TCOORD (q) == T8_QUAD_GET_TCOORD (r)));
}
#endif

#endif /* T8_ENABLE_DEBUG */

int
t8_default_scheme_sub_c::t8_element_maxlevel (void)
{
  return P4EST_QMAXLEVEL;
}

/* *INDENT-OFF* */
t8_eclass_t
t8_default_scheme_sub_c::t8_element_child_eclass (int childid)
/* *INDENT-ON* */

{
  T8_ASSERT (0 <= childid && childid < P4EST_CHILDREN);

  return T8_ECLASS_QUAD;
}

int
t8_default_scheme_sub_c::t8_element_level (const t8_element_t * elem)
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) elem;
  T8_ASSERT (t8_element_is_valid (elem));
  return (int) ((const t8_quad_with_subelements *) pquad_w_sub)->p4q.level;
}

static void
t8_element_copy_surround (const p4est_quadrant_t * q, p4est_quadrant_t * r)
{
  T8_QUAD_SET_TDIM (r, T8_QUAD_GET_TDIM (q));
  if (T8_QUAD_GET_TDIM (q) == 3) {
    T8_QUAD_SET_TNORMAL (r, T8_QUAD_GET_TNORMAL (q));
    T8_QUAD_SET_TCOORD (r, T8_QUAD_GET_TCOORD (q));
  }
}

void
t8_default_scheme_sub_c::t8_element_copy (const t8_element_t * source,
                                           t8_element_t * dest)
{
  const t8_quad_with_subelements *pquad_w_sub_source = (const t8_quad_with_subelements *) source;
  t8_quad_with_subelements *pquad_w_sub_dest = (t8_quad_with_subelements *) dest;

  const p4est_quadrant_t *q = &pquad_w_sub_source->p4q;
  p4est_quadrant_t   *r = &pquad_w_sub_dest->p4q;

  T8_ASSERT (t8_element_is_valid (source));
  T8_ASSERT (t8_element_is_valid (dest));
  if (q == r) {
    /* Do nothing if they are already the same quadrant. */
    return;
  }
  *r = *q;
  t8_element_copy_surround (q, r);
}

int
t8_default_scheme_sub_c::t8_element_compare (const t8_element_t * elem1,
                                              const t8_element_t * elem2)
{
  const t8_quad_with_subelements *pquad_w_sub1 = (const t8_quad_with_subelements *) elem1;
  const t8_quad_with_subelements *pquad_w_sub2 = (const t8_quad_with_subelements *) elem2;

  const p4est_quadrant_t *q = &pquad_w_sub1->p4q;
  const p4est_quadrant_t   *r = &pquad_w_sub2->p4q;

  T8_ASSERT (t8_element_is_valid (elem1));
  T8_ASSERT (t8_element_is_valid (elem2));

  return p4est_quadrant_compare (q, r);
}

void
t8_default_scheme_sub_c::t8_element_parent (const t8_element_t * elem,
                                             t8_element_t * parent)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements *pquad_w_sub_parent = (t8_quad_with_subelements *) parent;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;
  p4est_quadrant_t   *r = &pquad_w_sub_parent->p4q;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (t8_element_is_valid (parent));

  p4est_quadrant_parent (q, r);
  t8_element_copy_surround (q, r);
}

void
t8_default_scheme_sub_c::t8_element_sibling (const t8_element_t * elem,
                                              int sibid,
                                              t8_element_t * sibling)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements *pquad_w_sub_sibling = (t8_quad_with_subelements *) sibling;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;
  p4est_quadrant_t   *r = &pquad_w_sub_sibling->p4q;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (t8_element_is_valid (sibling));

  p4est_quadrant_sibling (q, r, sibid);
  t8_element_copy_surround (q, r);
}

int
t8_default_scheme_sub_c::t8_element_num_faces (const t8_element_t * elem)
{
  T8_ASSERT (t8_element_is_valid (elem));
  return P4EST_FACES;
}

int
t8_default_scheme_sub_c::t8_element_max_num_faces (const t8_element_t * elem)
{
  return P4EST_FACES;
}

int
t8_default_scheme_sub_c::t8_element_num_children (const t8_element_t * elem)
{
  T8_ASSERT (t8_element_is_valid (elem));
  return P4EST_CHILDREN;
}

int
t8_default_scheme_sub_c::t8_element_num_face_children (const t8_element_t *
                                                        elem, int face)
{
  T8_ASSERT (t8_element_is_valid (elem));
  return 2;
}

int
t8_default_scheme_sub_c::t8_element_get_face_corner (const t8_element_t *
                                                      element, int face,
                                                      int corner)
{
  /*
   *   2    f_2    3
   *     x -->-- x
   *     |       |
   *     ^       ^
   * f_0 |       | f_1
   *     x -->-- x
   *   0    f_3    1
   */

  T8_ASSERT (t8_element_is_valid (element));
  T8_ASSERT (0 <= face && face < P4EST_FACES);
  T8_ASSERT (0 <= corner && corner < 2);
  return p4est_face_corners[face][corner];
}

int
t8_default_scheme_sub_c::t8_element_get_corner_face (const t8_element_t *
                                                      element, int corner,
                                                      int face)
{
  T8_ASSERT (t8_element_is_valid (element));
  T8_ASSERT (0 <= corner && corner < P4EST_CHILDREN);
  T8_ASSERT (0 <= face && face < 2);
  return p4est_corner_faces[corner][face];
}

void
t8_default_scheme_sub_c::t8_element_child (const t8_element_t * elem,
                                            int childid, t8_element_t * child)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements *pquad_w_sub_child = (t8_quad_with_subelements *) child;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;
  p4est_quadrant_t   *r = &pquad_w_sub_child->p4q;

  const p4est_qcoord_t shift = P4EST_QUADRANT_LEN (q->level + 1);

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (t8_element_is_valid (child));
  T8_ASSERT (p4est_quadrant_is_extended (q));
  T8_ASSERT (q->level < P4EST_QMAXLEVEL);
  T8_ASSERT (childid >= 0 && childid < P4EST_CHILDREN);
  
  r->x = childid & 0x01 ? (q->x | shift) : q->x;
  r->y = childid & 0x02 ? (q->y | shift) : q->y;
  r->level = q->level + 1;
  
  T8_ASSERT (p4est_quadrant_is_parent (q, r));

  t8_element_copy_surround (q, r);
}

void
t8_default_scheme_sub_c::t8_element_children (const t8_element_t * elem,
                                               int length, t8_element_t * c[])
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements **pquad_w_sub_children = (t8_quad_with_subelements **) c;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;

  int i;

  T8_ASSERT (t8_element_is_valid (elem));
#ifdef T8_ENABLE_DEBUG
  {
    int                 j;
    for (j = 0; j < P4EST_CHILDREN; j++) {
      T8_ASSERT (t8_element_is_valid (c[j]));
    }
  }
#endif
  T8_ASSERT (length == P4EST_CHILDREN);

  /* set coordinates and levels of the children */ 
  p4est_quadrant_children (q, &pquad_w_sub_children[0]->p4q,
                              &pquad_w_sub_children[1]->p4q,
                              &pquad_w_sub_children[2]->p4q,
                              &pquad_w_sub_children[3]->p4q);

  for (i = 0; i < P4EST_CHILDREN; ++i) {
    t8_element_copy_surround (q, &pquad_w_sub_children[i]->p4q); 
  }
}

int
t8_default_scheme_sub_c::t8_element_child_id (const t8_element_t * elem)
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) elem;
  const p4est_quadrant_t *q = &pquad_w_sub->p4q;

  T8_ASSERT (t8_element_is_valid (elem));
  return p4est_quadrant_child_id (q);
}

int
t8_default_scheme_sub_c::t8_element_ancestor_id (const t8_element_t * elem,
                                                  int level)
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) elem;
  const p4est_quadrant_t *q = &pquad_w_sub->p4q;

  return p4est_quadrant_ancestor_id (q, level);
}

int
t8_default_scheme_sub_c::t8_element_is_family (t8_element_t ** fam)
{
  t8_quad_with_subelements **pquad_w_sub_family = (t8_quad_with_subelements **) fam;
#ifdef T8_ENABLE_DEBUG
  int                 i;
  for (i = 0; i < P4EST_CHILDREN; i++) {
    T8_ASSERT (t8_element_is_valid (fam[i]));
  }
#endif
  return p4est_quadrant_is_family (&pquad_w_sub_family[0]->p4q,
                                   &pquad_w_sub_family[1]->p4q,
                                   &pquad_w_sub_family[2]->p4q,
                                   &pquad_w_sub_family[3]->p4q);
}

void
t8_default_scheme_sub_c::t8_element_set_linear_id (t8_element_t * elem,
                                                    int level,
                                                    t8_linearidx_t id)
{
  t8_quad_with_subelements *pquad_w_sub = (t8_quad_with_subelements *) elem;
  p4est_quadrant_t *q = &pquad_w_sub->p4q;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);
  T8_ASSERT (0 <= id && id < ((t8_linearidx_t) 1) << P4EST_DIM * level);

  p4est_quadrant_set_morton (q, level, id);
  T8_QUAD_SET_TDIM (q, 2);
}

t8_linearidx_t
t8_default_scheme_sub_c::t8_element_get_linear_id (const t8_element_t *
                                                      elem, int level)
{
  t8_quad_with_subelements *pquad_w_sub = (t8_quad_with_subelements *) elem;
  p4est_quadrant_t *q = &pquad_w_sub->p4q;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);

  return p4est_quadrant_linear_id (q, level);
}

void
t8_default_scheme_sub_c::t8_element_first_descendant (const t8_element_t *
                                                       elem,
                                                       t8_element_t * desc,
                                                       int level)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements *pquad_w_sub_desc = (t8_quad_with_subelements *) desc;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;
  p4est_quadrant_t *r = &pquad_w_sub_desc->p4q;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (t8_element_is_valid (desc));
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);

  p4est_quadrant_first_descendant (q, r, level);
  T8_QUAD_SET_TDIM (r, 2);
}

void
t8_default_scheme_sub_c::t8_element_last_descendant (const t8_element_t *
                                                      elem,
                                                      t8_element_t * desc,
                                                      int level)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements *pquad_w_sub_desc = (t8_quad_with_subelements *) desc;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;
  p4est_quadrant_t *r = &pquad_w_sub_desc->p4q;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (t8_element_is_valid (desc));
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);

  p4est_quadrant_last_descendant (q, r, level);
  T8_QUAD_SET_TDIM (r, 2);
}

void
t8_default_scheme_sub_c::t8_element_successor (const t8_element_t * elem1,
                                                t8_element_t * elem2,
                                                int level)
{
  const t8_quad_with_subelements *pquad_w_sub_elem1 = (const t8_quad_with_subelements *) elem1;
  t8_quad_with_subelements *pquad_w_sub_elem2 = (t8_quad_with_subelements *) elem2;

  const p4est_quadrant_t *q = &pquad_w_sub_elem1->p4q;
  p4est_quadrant_t *r = &pquad_w_sub_elem2->p4q;

  t8_linearidx_t      id;

  T8_ASSERT (t8_element_is_valid (elem1));
  T8_ASSERT (t8_element_is_valid (elem2));
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);

  id = p4est_quadrant_linear_id (q, level);
  T8_ASSERT (id + 1 < ((t8_linearidx_t) 1) << P4EST_DIM * level);
  p4est_quadrant_set_morton (r, level, id + 1);
  t8_element_copy_surround (q, r);
}

void
t8_default_scheme_sub_c::t8_element_nca (const t8_element_t * elem1,
                                          const t8_element_t * elem2,
                                          t8_element_t * nca)
{
  const t8_quad_with_subelements *pquad_w_sub_elem1 = (const t8_quad_with_subelements *) elem1;
  const t8_quad_with_subelements *pquad_w_sub_elem2 = (const t8_quad_with_subelements *) elem2;
  t8_quad_with_subelements *pquad_w_sub_nca = (t8_quad_with_subelements *) nca;

  const p4est_quadrant_t *q1 = &pquad_w_sub_elem1->p4q;
  const p4est_quadrant_t *q2 = &pquad_w_sub_elem2->p4q;
  p4est_quadrant_t *r = &pquad_w_sub_nca->p4q;

  T8_ASSERT (t8_element_is_valid (elem1));
  T8_ASSERT (t8_element_is_valid (elem2));
#if 0
  /* TODO: This assertions throws an error since it expects a 3D hex.
   *       this does not make sense. investigate. */
  T8_ASSERT (t8_element_surround_matches (q1, q2));
#endif

  p4est_nearest_common_ancestor (q1, q2, r);
  t8_element_copy_surround (q1, r);
}

t8_element_shape_t
  t8_default_scheme_sub_c::t8_element_face_shape (const t8_element_t * elem,
                                                   int face)
{
  T8_ASSERT (t8_element_is_valid (elem));
  return T8_ECLASS_LINE;
}

void
t8_default_scheme_sub_c::t8_element_children_at_face (const t8_element_t *
                                                       elem, int face,
                                                       t8_element_t *
                                                       children[],
                                                       int num_children,
                                                       int *child_indices)
{
  int                 first_child, second_child;

#ifdef T8_ENABLE_DEBUG
  {
    int                 i;
    for (i = 0; i < num_children; i++) {
      T8_ASSERT (t8_element_is_valid (children[i]));
    }
  }
#endif
  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (0 <= face && face < P4EST_FACES);
  T8_ASSERT (num_children == t8_element_num_face_children (elem, face));

  /*
   * Compute the child id of the first and second child at the face.
   *
   *            3
   *
   *      x - - x - - x           This picture shows a refined quadrant
   *      |     |     |           with child_ids and the label for the faces.
   *      | 2   | 3   |           For examle for face 2 (bottom face) we see
   * 0    x - - x - - x   1       first_child = 0 and second_child = 1.
   *      |     |     |
   *      | 0   | 1   |
   *      x - - x - - x
   *
   *            2
   */
  /* TODO: Think about a short and easy bitwise formula. */
  switch (face) {
  case 0:
    first_child = 0;
    second_child = 2;
    break;
  case 1:
    first_child = 1;
    second_child = 3;
    break;
  case 2:
    first_child = 0;
    second_child = 1;
    break;
  case 3:
    first_child = 2;
    second_child = 3;
    break;
  default:
    SC_ABORT_NOT_REACHED ();
  }

  /* From the child ids we now construct the children at the faces. */
  /* We have to revert the order and compute second child first, since
   * the usage allows for elem == children[0].
   */
  this->t8_element_child (elem, second_child, children[1]);
  this->t8_element_child (elem, first_child, children[0]);
  if (child_indices != NULL) {
    child_indices[0] = first_child;
    child_indices[1] = second_child;
  }
}

int
t8_default_scheme_sub_c::t8_element_face_child_face (const t8_element_t *
                                                      elem, int face,
                                                      int face_child)
{
  T8_ASSERT (t8_element_is_valid (elem));
  /* For quadrants the face enumeration of children is the same as for the parent. */
  return face;
}

int
t8_default_scheme_sub_c::t8_element_face_parent_face (const t8_element_t *
                                                       elem, int face)
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) elem;
  const p4est_quadrant_t *q = &pquad_w_sub->p4q;
  int                 child_id;

  T8_ASSERT (t8_element_is_valid (elem));

  if (q->level == 0) {
    return face;
  }
  /* Determine whether face is a subface of the parent.
   * This is the case if the child_id matches one of the faces corners */
  child_id = p4est_quadrant_child_id (q);
  if (child_id == p4est_face_corners[face][0]
      || child_id == p4est_face_corners[face][1]) {
    return face;
  }
  return -1;
}

void
t8_default_scheme_sub_c::t8_element_transform_face (const t8_element_t *
                                                     elem1,
                                                     t8_element_t * elem2,
                                                     int orientation,
                                                     int sign,
                                                     int is_smaller_face)
{
  const t8_quad_with_subelements *pquad_w_sub_elem1 = (const t8_quad_with_subelements *) elem1;
  t8_quad_with_subelements *pquad_w_sub_elem2 = (t8_quad_with_subelements *) elem2;

  const p4est_quadrant_t *qin = &pquad_w_sub_elem1->p4q;
  p4est_quadrant_t *p = &pquad_w_sub_elem2->p4q;

  const p4est_quadrant_t *q;
  p4est_qcoord_t      h = P4EST_QUADRANT_LEN (qin->level);
  p4est_qcoord_t      x = qin->x;       /* temp storage for x coordinate in case elem1 = elem 2 */

  T8_ASSERT (t8_element_is_valid (elem1));
  T8_ASSERT (t8_element_is_valid (elem2));
  T8_ASSERT (0 <= orientation && orientation < P4EST_FACES);

  if (sign) {
    /* The tree faces have the same topological orientation, and
     * thus we have to perform a coordinate switch. */
    /* We use p as storage, since elem1 and elem2 are allowed to
     * point to the same quad */
    q = (const p4est_quadrant_t *) p;
    t8_element_copy_surround (qin, (p4est_quadrant_t *) q);
    ((p4est_quadrant_t *) q)->x = qin->y;
    ((p4est_quadrant_t *) q)->y = x;
    x = q->x;                   /* temp storage in case elem1 = elem 2 */
  }
  else {
    q = qin;
  }

  p->level = q->level;
  /*
   * The faces of the root quadrant are enumerated like this:
   *
   *   v_2      v_3
   *     x -->-- x
   *     |       |
   *     ^       ^
   *     |       |
   *     x -->-- x
   *   v_0      v_1
   *
   * Orientation is the corner number of the bigger face that coincides
   * with the corner v_0 of the smaller face.
   */
  /* If this face is not smaller, switch the orientation:
   *  sign = 0   sign = 1
   *  0 -> 0     0 -> 0
   *  1 -> 2     1 -> 1
   *  2 -> 1     2 -> 2
   *  3 -> 3     3 -> 3
   */
  if (!is_smaller_face && (orientation == 1 || orientation == 2) && !sign) {
    orientation = 3 - orientation;
  }

  switch (orientation) {
  case 0:                      /* Nothing to do */
    p->x = q->x;
    p->y = q->y;
    break;
  case 1:
    p->x = P4EST_ROOT_LEN - q->y - h;
    p->y = x;
    break;
  case 2:
    p->x = q->y;
    p->y = P4EST_ROOT_LEN - x - h;
    break;
  case 3:
    p->x = P4EST_ROOT_LEN - q->x - h;
    p->y = P4EST_ROOT_LEN - q->y - h;
    break;
  default:
    SC_ABORT_NOT_REACHED ();
  }
  T8_QUAD_SET_TDIM (p, 2);
}

int
t8_default_scheme_sub_c::t8_element_extrude_face (const t8_element_t * face,
                                                   const t8_eclass_scheme_c *
                                                   face_scheme,
                                                   t8_element_t * elem,
                                                   int root_face)
{
  t8_quad_with_subelements *pquad_w_sub = (t8_quad_with_subelements *) elem;
  p4est_quadrant_t   *q = &pquad_w_sub->p4q;

  const t8_dline_t   *l = (const t8_dline_t *) face;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (T8_COMMON_IS_TYPE
             (face_scheme, const t8_default_scheme_line_c *));
  T8_ASSERT (face_scheme->eclass == T8_ECLASS_LINE);
  T8_ASSERT (face_scheme->t8_element_is_valid (elem));
  T8_ASSERT (0 <= root_face && root_face < P4EST_FACES);
  
  /*
   * The faces of the root quadrant are enumerated like this:
   *
   *        f_2
   *     x -->-- x
   *     |       |
   *     ^       ^
   * f_0 |       | f_1
   *     x -->-- x
   *        f_3
   *
   * The arrows >,^ denote the orientation of the faces.
   * We need to scale the coordinates since a root line may have a different
   * length than a root quad.
   */
  q->level = l->level;
  switch (root_face) {
  case 0:
    q->x = 0;
    q->y = ((int64_t) l->x * P4EST_ROOT_LEN) / T8_DLINE_ROOT_LEN;
    break;
  case 1:
    q->x = P4EST_LAST_OFFSET (q->level);
    q->y = ((int64_t) l->x * P4EST_ROOT_LEN) / T8_DLINE_ROOT_LEN;
    break;
  case 2:
    q->x = ((int64_t) l->x * P4EST_ROOT_LEN) / T8_DLINE_ROOT_LEN;
    q->y = 0;
    break;
  case 3:
    q->x = ((int64_t) l->x * P4EST_ROOT_LEN) / T8_DLINE_ROOT_LEN;
    q->y = P4EST_LAST_OFFSET (q->level);
    break;
  default:
    SC_ABORT_NOT_REACHED ();
  }
  /* We return the face of q at which we extruded. This is the same number
   * as root_face. */
  return root_face;
}

int
t8_default_scheme_sub_c::t8_element_tree_face (const t8_element_t * elem,
                                                int face)
{
  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (0 <= face && face < P4EST_FACES);
  /* For quadrants the face and the tree face number are the same. */
  return face;
}

/** Construct the first descendant of an element that touches a given face.   */
void
t8_default_scheme_sub_c::t8_element_first_descendant_face (const t8_element_t
                                                            * elem, int face,
                                                            t8_element_t *
                                                            first_desc,
                                                            int level)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements *pquad_w_sub_first_desc = (t8_quad_with_subelements *) first_desc;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;
  p4est_quadrant_t *desc = &pquad_w_sub_first_desc->p4q;

  int                 first_face_corner;

  T8_ASSERT (0 <= face && face < P4EST_FACES);
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);

  /* Get the first corner of q that belongs to face */
  first_face_corner = p4est_face_corners[face][0];
  /* Construce the descendant in that corner */
  p4est_quadrant_corner_descendant (q, desc, first_face_corner, level);
}

/** Construct the last descendant of an element that touches a given face.   */
void
t8_default_scheme_sub_c::t8_element_last_descendant_face (const t8_element_t
                                                           * elem, int face,
                                                           t8_element_t *
                                                           last_desc,
                                                           int level)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements *pquad_w_sub_last_desc = (t8_quad_with_subelements *) last_desc;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;
  p4est_quadrant_t   *desc = &pquad_w_sub_last_desc->p4q;

  int                 last_face_corner;

  T8_ASSERT (0 <= face && face < P4EST_FACES);
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);

  /* Get the last corner of q that belongs to face */
  last_face_corner = p4est_face_corners[face][1];
  /* Construce the descendant in that corner */
  p4est_quadrant_corner_descendant (q, desc, last_face_corner, level);
}

void
t8_default_scheme_sub_c::t8_element_boundary_face (const t8_element_t * elem,
                                                    int face,
                                                    t8_element_t * boundary,
                                                    const t8_eclass_scheme_c *
                                                    boundary_scheme)
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) elem;
  const p4est_quadrant_t *q = &pquad_w_sub->p4q;

  t8_dline_t         *l = (t8_dline_t *) boundary;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (T8_COMMON_IS_TYPE
             (boundary_scheme, const t8_default_scheme_line_c *));
  T8_ASSERT (boundary_scheme->eclass == T8_ECLASS_LINE);
  T8_ASSERT (boundary_scheme->t8_element_is_valid (boundary));
  T8_ASSERT (0 <= face && face < P4EST_FACES);
  /* The level of the boundary element is the same as the quadrant's level */
  l->level = q->level;
  /*
   * The faces of the quadrant are enumerated like this:
   *        f_2
   *     x ---- x
   *     |      |
   * f_0 |      | f_1
   *     x ---- x
   *        f_3
   *
   * If face = 0 or face = 1 then l->x = q->y
   * if face = 2 or face = 3 then l->x = q->x
   */
  l->x = ((face >> 1 ? q->x : q->y) *
          ((int64_t) T8_DLINE_ROOT_LEN) / P4EST_ROOT_LEN);
}

void
t8_default_scheme_sub_c::t8_element_boundary (const t8_element_t * elem,
                                               int min_dim, int length,
                                               t8_element_t ** boundary)
{
  SC_ABORT ("Not implemented\n");
#if 0
#ifdef T8_ENABLE_DEBUG
  int                 per_eclass[T8_ECLASS_COUNT];
#endif
  int                 iface;

  T8_ASSERT (length ==
             t8_eclass_count_boundary (T8_ECLASS_QUAD, min_dim, per_eclass));

  T8_ASSERT (length == P4EST_FACES);
  for (iface = 0; iface < P4EST_FACES; iface++) {
    t8_element_boundary_face (elem, iface, boundary[iface]);
  }
#endif
}

int
t8_default_scheme_sub_c::t8_element_is_root_boundary (const t8_element_t *
                                                       elem, int face)
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) elem;
  const p4est_quadrant_t *q = &pquad_w_sub->p4q;

  p4est_qcoord_t      coord;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (0 <= face && face < P4EST_FACES);

  /* if face is 0 or 1 q->x
   *            2 or 3 q->y
   */
  coord = face >> 1 ? q->y : q->x;
  /* If face is 0 or 2 check against 0.
   * If face is 1 or 3  check against LAST_OFFSET */
  return coord == (face & 1 ? P4EST_LAST_OFFSET (q->level) : 0);
}

int
t8_default_scheme_sub_c::t8_element_face_neighbor_inside (const t8_element_t
                                                           * elem,
                                                           t8_element_t *
                                                           neigh, int face,
                                                           int *neigh_face)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements *pquad_w_sub_neigh = (t8_quad_with_subelements *) neigh;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;
  p4est_quadrant_t   *n = &pquad_w_sub_neigh->p4q;

  T8_ASSERT (t8_element_is_valid (elem));
  T8_ASSERT (t8_element_is_valid (neigh));
  T8_ASSERT (0 <= face && face < P4EST_FACES);

  /* Construct the face neighbor */
  p4est_quadrant_face_neighbor (q, face, n);

  T8_QUAD_SET_TDIM (n, 2);
  /* Compute the face number as seen from q.
   *  0 -> 1    2 -> 3
   *  1 -> 0    3 -> 2
   */
  T8_ASSERT (neigh_face != NULL);

  *neigh_face = p4est_face_dual[face];

  /* return true if neigh is inside the root */
  return p4est_quadrant_is_inside_root (n);
}

void
t8_default_scheme_sub_c::t8_element_anchor (const t8_element_t * elem,
                                             int coord[3])
{
  t8_quad_with_subelements *pquad_w_sub = (t8_quad_with_subelements *) elem;
  p4est_quadrant_t   *q = &pquad_w_sub->p4q;

  T8_ASSERT (t8_element_is_valid (elem));

  coord[0] = q->x;
  coord[1] = q->y;
  coord[2] = 0;
  T8_QUAD_SET_TDIM (q, 2);
}

int
t8_default_scheme_sub_c::t8_element_root_len (const t8_element_t * elem)
{
  return P4EST_ROOT_LEN;
}

void
t8_default_scheme_sub_c::t8_element_vertex_coords_of_subelement (const t8_element_t * t,
                                                                 int vertex, int coords[])
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) t;
  const p4est_quadrant_t *q1 = &pquad_w_sub->p4q;

  int                 len;

  T8_ASSERT (pquad_w_sub->dummy_is_subelement == 1);
  T8_ASSERT (t8_element_is_valid (t));
  T8_ASSERT (0 <= vertex && vertex < 4); /* diese assertion macht für dreieckige subelemente keinen sinn mehr */

  /* get the length of the current quadrant */
  len = P4EST_QUADRANT_LEN (q1->level);

  /* Compute the x and y coordinates of subelement vertices, depending on the subelement type, id and vertex number */
  #if 0
  /* consider the following subelement of type 11:
   *
   *           =len
   *      |-------------|
   *                                     1
   *      x - - - - - - x          x - - x - - x 
   *      |             |          | \ 3 | 4 / |
   *      |             |          | 2 \ | / 5 |                                                                            rearranging binaries:   1   1   1   0
   *      |             |   -->  1 x - - x - - x 1   -->   binary representation: 1011 (be careful, in which order   -->    number subelements:     2   2   2   1
   *      |             |          | 1 /   \ 6 |           the faces are enumerated). Now, consider sub_id : 3              where is our sub?           x -> first sub on upper face
   *      | elem        |          | /   7   \ |
   *      + - - - - - - x          x - - - - - x
   *                                     0
   *           
   * we will use the binary representation of the to automatically determine the vertex coordinates of a given subelement type. */
  /* need to know: face number of anchor node and whether it is split in half and if we are the first or second subelement */
  [face_number, split, sub_face_id] = t8_element_get_location_of_subelement(t); 
  if (face_number == 0) {
    if (split == 0) {
      if (vertex == 0) {
        coords[0] = q1->x;
        coords[1] = q1->y;
      }
      else if (vertex == 1) {
        coords[0] = q1->x;
        coords[1] = q1->y + len;
      }
    }
    else if (split == 1) {
      if (sub_face_id == 1) {
        if (vertex == 0) {
          coords[0] = q1->x;
          coords[1] = q1->y;
        }
        else if (vertex == 1) {
          coords[0] = q1->x;
          coords[1] = q1->y + len * 1/2;
        }
      }
      else if (sub_face_id == 2) {
        if (vertex == 0) {
          coords[0] = q1->x + len * 1/2;
          coords[1] = q1->y + len * 1/2;
        }
        else if (vertex == 1) {
          coords[0] = q1->x + len * 1/2;
          coords[1] = q1->y + len * 1/2 + len * 1/2;
        }
      }
    }
  } /* end of vertex coordinates for subelements that border to face number 1 */
  else if (face_number == 1) {
  }
  else if (face_number == 2) {
  }
  else if (face_number == 3) {
  }
  /* the third vertex equals the center of the element for all subelements */
  if (vertex == 2) {
    coords[0] = q1->x + len * 1/2;
    coords[1] = q1->y + len * 1/2;
  } 
  #endif
  if (pquad_w_sub->subelement_type >= 0 && pquad_w_sub->subelement_type <= 14) {
    /* 
     *            =len
     *      |---------------| 
     *                               V2       V3
     *      x - - - x - - - x         x - - - x   
     *      |       |       |         |       |
     *      |       |       |         |       |
     *      | sub_  | sub_  |   -->   | sub_  |                       
     *      | id    | id    |         | id    |
     *      | 0     | 1     |         | 0     |
     *      + - - - x - - - x         + - - - x
     *                               V0       V1
     */
    if (pquad_w_sub->subelement_id == 0) {
      coords[0] = q1->x + (vertex & 1 ? 1 : 0) * len * 1/2;
      coords[1] = q1->y + (vertex & 2 ? 1 : 0) * len;
    }
    else if (pquad_w_sub->subelement_id == 1) {
      coords[0] = q1->x + (vertex & 1 ? 1 : 0) * len * 1/2 + len * 1/2;
      coords[1] = q1->y + (vertex & 2 ? 1 : 0) * len;
    }
    else {
      T8_ASSERT (printf("No valid subelement id!"));
    }
  }
  else {
    T8_ASSERT (printf("No valid subelement type!"));
  }
}

void
t8_default_scheme_sub_c::t8_element_vertex_coords (const t8_element_t * t,
                                                   int vertex, int coords[])
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) t;
  const p4est_quadrant_t *q1 = &pquad_w_sub->p4q;

  int                 len;

  T8_ASSERT (t8_element_is_valid (t));
  T8_ASSERT (0 <= vertex && vertex < 4);
  /* Get the length of the quadrant */
  len = P4EST_QUADRANT_LEN (q1->level);

  if (pquad_w_sub->dummy_is_subelement == 0) {
    /* Compute the x and y coordinates of the vertex depending on the
     * vertex number */
    coords[0] = q1->x + (vertex & 1 ? 1 : 0) * len;
    coords[1] = q1->y + (vertex & 2 ? 1 : 0) * len;
  }
  else {
    t8_element_vertex_coords_of_subelement (t, vertex, coords);
  }
}

void
t8_default_scheme_sub_c::t8_element_to_subelement (const t8_element_t * elem,
                                                   t8_element_t * c[],
                                                   int type)
{
  const t8_quad_with_subelements *pquad_w_sub_elem = (const t8_quad_with_subelements *) elem;
  t8_quad_with_subelements **pquad_w_sub_subelement = (t8_quad_with_subelements **) c;

  const p4est_quadrant_t *q = &pquad_w_sub_elem->p4q;

  int i;

  /* check that elem is not already a subelement */
  T8_ASSERT (pquad_w_sub_elem->dummy_is_subelement == 0);
  T8_ASSERT (t8_element_is_valid (elem));
#ifdef T8_ENABLE_DEBUG
  {
    int                 j;
    for (j = 0; j < pquad_w_sub_elem->num_subelement_ids; j++) {
      T8_ASSERT (t8_element_is_valid (c[j]));
    }
  }
#endif
  
  /* get the length of a children-quadrant */
  const int8_t        level = (int8_t) (q->level);

  T8_ASSERT (p4est_quadrant_is_extended (q));
  T8_ASSERT (q->level < P4EST_QMAXLEVEL);
  
  #if 0
  int sub_id_counter = 0;
  int num_subelements;

  num_subelements = t8_element_get_number_of_subelements (type);
  /* NOTE determine num_subelements, depending on the subelement type using binary representation */
  /* set the parameter values for different subelements: 
   * all subelement types (up to rotation) are:
   *                               
   *      x - - - - - - x         x - - - - - x        x - - - - - x        x - - - - - x        x - - x - - x 
   *      |             |         | \   2   / |        | \       / |        | \       / |        | \   |   / |
   *      |             |         | 1 \   /   |        |   \   /   |        |   \   /   |        |   \ | /   |
   *      |             |   -->   x - - X   3 |   or   x - - x     |   or   x - - x - - x   or   x - - x - - x    
   *      |             |         | 0 /   \   |        |   / | \   |        |   /   \   |        |   /   \   |
   *      | elem        |         | /   4   \ |        | /   |   \ |        | /       \ |        | /       \ |
   *      + - - - - - - x         x - - - - - x        x - - x - - x        x - - - - - x        x - - - - - x
   *           
   * Sub_ids are counted clockwise, starting with the (lower) left subelement.                    
   * we do not change the p4est quadrant */
  for (sub_id_counter = 0, sub_id_counter < num_subelements, sub_id_counter++) {
    pquad_w_sub_subelement[sub_id_counter]->p4q.x = q->x;
    pquad_w_sub_subelement[sub_id_counter]->p4q.y = q->y;
    pquad_w_sub_subelement[sub_id_counter]->p4q.level = level;
    pquad_w_sub_subelement[sub_id_counter]->dummy_is_subelement = 1;
    pquad_w_sub_subelement[sub_id_counter]->subelement_type = type;
    pquad_w_sub_subelement[sub_id_counter]->subelement_id = sub_id_counter;
    pquad_w_sub_subelement[sub_id_counter]->num_subelement_ids = num_subelements;
  }
  #endif
  if (type >= 0 && type <= 15) {
    /* temporary subelement type:
    *                               
    *      x - - - - - - - x         x - - - x - - - x           
    *      |               |         |       |       |          
    *      |               |         |       |       |    
    *      |               |   -->   | sub_  | sub_  |     
    *      |               |         | id    | id    |   
    *      | elem          |         | 0     | 1     |
    *      + - - - - - - - x         x - - - x - - - x
    *
    * we do not change the p4est quadrant */
    pquad_w_sub_subelement[0]->p4q.x = q->x;
    pquad_w_sub_subelement[0]->p4q.y = q->y;
    pquad_w_sub_subelement[0]->p4q.level = level;
    pquad_w_sub_subelement[0]->dummy_is_subelement = 1;
    pquad_w_sub_subelement[0]->subelement_type = type;
    pquad_w_sub_subelement[0]->subelement_id = 0;
    pquad_w_sub_subelement[0]->num_subelement_ids = 2;
  
    pquad_w_sub_subelement[1]->p4q.x = pquad_w_sub_subelement[0]->p4q.x;
    pquad_w_sub_subelement[1]->p4q.y = pquad_w_sub_subelement[0]->p4q.y;
    pquad_w_sub_subelement[1]->p4q.level = level;
    pquad_w_sub_subelement[1]->dummy_is_subelement = 1;
    pquad_w_sub_subelement[1]->subelement_type = type;
    pquad_w_sub_subelement[1]->subelement_id = 1;
    pquad_w_sub_subelement[1]->num_subelement_ids = 2;
  }
  else {
    T8_ASSERT (printf("No valid subelement type!"));
  }
  for (i = 0; i < pquad_w_sub_elem->num_subelement_ids; ++i) {
    T8_ASSERT (t8_element_is_valid(c[i]));
    t8_element_copy_surround (q, &pquad_w_sub_subelement[i]->p4q); 
  }
}

int
t8_default_scheme_sub_c::t8_element_get_number_of_subelements (int subelement_type)
{
  int num_subelements;
  int digit_sum_of_binary = 0;
  int subelement_type_temp = subelement_type;
  if (8 <= subelement_type_temp) {
    digit_sum_of_binary += 1;
    subelement_type_temp -= 8;
  }
  if (4 <= subelement_type_temp) {
    digit_sum_of_binary += 1;
    subelement_type_temp -= 4;
  }
  if (2 <= subelement_type_temp) {
    digit_sum_of_binary += 1;
    subelement_type_temp -= 2;
  }
  if (1 <= subelement_type_temp) {
    digit_sum_of_binary += 1;
    subelement_type_temp -= 1;
  }
  /* consider subelement_type 13 = 1101 in base two -> there are 4 + (1+1+0+1) = 7 subelements needed to remove hanging nodes */
  num_subelements = P4EST_FACES + digit_sum_of_binary;
  return num_subelements;
}

void
t8_default_scheme_sub_c::t8_element_new (int length, t8_element_t ** elem)
{
  /* allocate memory for a quad with subelements */
  t8_default_scheme_common_c::t8_element_new (length, elem);

  int                 i;
  for (i = 0; i < length; i++) {
    t8_quad_with_subelements *pquad_w_sub = (t8_quad_with_subelements *) elem[i];
    t8_element_init (1, elem[i], 0);
    /* set dimension of quad to 2 */
    T8_QUAD_SET_TDIM ((p4est_quadrant_t *) & pquad_w_sub->p4q, 2);
  }
}

void
t8_default_scheme_sub_c::t8_element_init (int length, t8_element_t * elem,
                                          int new_called)
{
  t8_quad_with_subelements *pquad_w_sub = (t8_quad_with_subelements *) elem;

  int                 i;

  for (i = 0; i < length; i++) {
    /* initalize subelement parameters */
    pquad_w_sub[i].dummy_is_subelement = 0;
    pquad_w_sub[i].subelement_type = -1;
    pquad_w_sub[i].num_subelement_types = -1;
    pquad_w_sub[i].subelement_id = -1;
    pquad_w_sub[i].num_subelement_ids = -1;

    #ifdef T8_ENABLE_DEBUG
    /* In debugging mode we iterate over all length many elements and 
     * set their quad to the level 0 quad with ID 0. */
    if (!new_called) {
      p4est_quadrant_t   *quad = &pquad_w_sub[i].p4q;
      p4est_quadrant_set_morton (quad, 0, 0);
      T8_QUAD_SET_TDIM (quad, 2);
      T8_ASSERT (p4est_quadrant_is_extended (quad));
    }
  }
#endif
}

#ifdef T8_ENABLE_DEBUG
/* *INDENT-OFF* */
/* indent bug, indent adds a second "const" modifier */
int
t8_default_scheme_sub_c::t8_element_is_valid (const t8_element_t * elem) const 
/* *INDENT-ON* */
{
  const t8_quad_with_subelements *pquad_w_sub = (const t8_quad_with_subelements *) elem;
  const p4est_quadrant_t *q = &pquad_w_sub->p4q;

  T8_ASSERT (pquad_w_sub->dummy_is_subelement == 0 || pquad_w_sub->dummy_is_subelement == 1);
  T8_ASSERT (pquad_w_sub->num_subelement_types >= -1 && pquad_w_sub->num_subelement_types <= 10);
  T8_ASSERT (pquad_w_sub->subelement_type >= -1 && pquad_w_sub->subelement_type <= 10);
  T8_ASSERT (pquad_w_sub->num_subelement_ids >= -1 && pquad_w_sub->num_subelement_ids <= 8);
  T8_ASSERT (pquad_w_sub->subelement_id >= -1 && pquad_w_sub->subelement_id <= 8);

  return p4est_quadrant_is_extended (q);
}
#endif

/* Constructor */
t8_default_scheme_sub_c::t8_default_scheme_sub_c (void)
{
  eclass = T8_ECLASS_QUAD;
  element_size = sizeof (t8_pquad_t);
  ts_context = sc_mempool_new (element_size);
}

t8_default_scheme_sub_c::~t8_default_scheme_sub_c ()
{
  /* This destructor is empty since the destructor of the
   * default_common scheme is called automatically and it
   * suffices to destroy the quad_scheme.
   * However we need to provide an implementation of the destructor
   * and hence this empty function. */
}

T8_EXTERN_C_END ();