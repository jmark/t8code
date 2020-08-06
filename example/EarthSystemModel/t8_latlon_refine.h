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

/** file t8_latlon_refine.h
 */

#ifndef T8_LATLON_REFINE_H
#define T8_LATLON_REFINE_H

#include <t8.h>
#include <t8_forest.h>

/* We offer two modes to construct the mesh.
 * T8_LATLON_REFINE: Start with a level 0 mesh and refine it until
 *                   the grid mesh on level L is reached.
 * T8_LATLON_COARSE: Start with the level L uniform mesh and coarsen all
 *                   elements that do not belong to the grid.
 */
enum T8_LATLON_ADAPT_MODE
{
  T8_LATLON_REFINE,
  T8_LATLON_COARSEN
};

/* The data that we pass on to our adapt function and
 * describes the layout of the grid plus adaptation mode.
 */
typedef struct
{
  int                 x_length; /* Number of cells in x dimension. */
  int                 y_length; /* Number of cells in y dimension. */
  int                 max_level;        /* The computed refinement level of a uniform forest to contain an x by y grid. */
  enum T8_LATLON_ADAPT_MODE mode;       /* The adaptation mode to use. */
} t8_latlon_adapt_data_t;

T8_EXTERN_C_BEGIN ();

/* function declarations */

t8_locidx_t         t8_latlon_adapt_callback (t8_forest_t forest,
                                              t8_forest_t forest_from,
                                              t8_locidx_t which_tree,
                                              t8_locidx_t lelement_id,
                                              t8_eclass_scheme_c * ts,
                                              int num_elements,
                                              t8_element_t * elements[]);

void                t8_latlon_refine (int x_length, int y_length,
                                      enum T8_LATLON_ADAPT_MODE mode,
                                      int repartition);

int                 t8_latlon_refine_grid_cuts_elements (const t8_element_t *
                                                         element,
                                                         t8_default_scheme_common_c
                                                         * ts,
                                                         const
                                                         t8_latlon_adapt_data_t
                                                         * adapt_data);
T8_EXTERN_C_END ();

#endif /* !T8_LATLON_REFINE_H */