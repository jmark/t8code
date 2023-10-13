/*
  This file is part of t8code.
  t8code is a C library to manage a collection (a forest) of multiple
  connected adaptive space-trees of general element classes in parallel.

  Copyright (C) 2023 the developers

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

/** \file t8_geometry_example.hxx
 * Various mappings for several cmesh examples.
 */

#ifndef T8_GEOMETRY_EXAMPLES_HXX
#define T8_GEOMETRY_EXAMPLES_HXX

#include <t8.h>
#include <t8_geometry/t8_geometry_with_vertices.hxx>

/** This geometry maps five quads to a disk.
 */
class t8_geometry_squared_disk: public t8_geometry_with_vertices {
 public:
  /* Basic constructor that sets the dimension and the name. */
  t8_geometry_squared_disk (): t8_geometry_with_vertices (2, "t8_squared_disk")
  {
  }

  /**
   * Map five quads to a disk.
   * \param [in]  cmesh      The cmesh in which the point lies.
   * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
   * \param [in]  ref_coords  Array of \a dimension x \a num_coords many entries, specifying a point in /f$ [0,1]^\mathrm{dim} /f$.
   * \param [in]  num_coords  The number of points to map.
   * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords. The length is \a num_coords * 3.
   *
   * This routine expects an input mesh of five squares looking like this:
   *
   *      +----------+
   *      |\___1____/|
   *      | |      | |
   *      |4|  0   |2|
   *      | |______| |
   *      |/   3    \|
   *      +----------+
   *
   * The central quad (id = 0) is mapped as is, while the outer edges of
   * other four quads are stretched onto a circle with a radius determined by
   * the four outer corners (denoted by "+") in the schematic above.
   *
   */
  void
  t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                    double *out_coords) const;

  /* Jacobian, not implemented. */
  void
  t8_geom_evaluate_jacobian (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                             double *jacobian) const
  {
    SC_ABORT_NOT_REACHED ();
  }

  /* Load tree data is inherited from t8_geometry_with_vertices. */
};

/** This geometry maps the faces of an octahedron/icasohedron to a spherical surface.
 */
class t8_geometry_triangulated_spherical_surface: public t8_geometry_with_vertices {
 public:
  /* Basic constructor that sets the dimension and the name. */
  t8_geometry_triangulated_spherical_surface (): t8_geometry_with_vertices (2, "t8_triangulated_spherical_surface")
  {
  }

  /**
   * Map the faces of an octahedron/icasohedron to a spherical surface.
   * \param [in]  cmesh      The cmesh in which the point lies.
   * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
   * \param [in]  ref_coords  Array of \a dimension x \a num_coords many entries, specifying a point in /f$ [0,1]^\mathrm{dim} /f$.
   * \param [in]  num_coords  The number of points to map.
   * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords. The length is \a num_coords * 3.
   *
   * This routine expects an input mesh of triangles arranged into an
   * octahedron/icasohedron.
   *
   */
  void
  t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                    double out_coords[3]) const;

  /* Jacobian, not implemented. */
  void
  t8_geom_evaluate_jacobian (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                             double *jacobian) const
  {
    SC_ABORT_NOT_REACHED ();
  }

  /* Load tree data is inherited from t8_geometry_with_vertices. */
};

/** This geometry maps general 2D faces to a spherical surface.
 */
class t8_geometry_spherical_surface: public t8_geometry_with_vertices {
 public:
  /* Basic constructor that sets the dimension and the name. */
  t8_geometry_spherical_surface (): t8_geometry_with_vertices (2, "t8_spherical_surface")
  {
  }

  /**
   * Maps general 2D faces to a spherical surface.
   * \param [in]  cmesh      The cmesh in which the point lies.
   * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
   * \param [in]  ref_coords  Array of \a dimension x \a num_coords many entries, specifying a point in /f$ [0,1]^\mathrm{dim} /f$.
   * \param [in]  num_coords  The number of points to map.
   * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords. The length is \a num_coords * 3.
   *
   * This routine expects an input mesh of 2D elements with their vertices sitting on a sphere.
   *
   */
  void
  t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                    double out_coords[3]) const;

  /* Jacobian, not implemented. */
  void
  t8_geom_evaluate_jacobian (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                             double *jacobian) const
  {
    SC_ABORT_NOT_REACHED ();
  }

  /* Load tree data is inherited from t8_geometry_with_vertices. */
};

/** This geometry maps the faces of a cube to a spherical surface.
 */
class t8_geometry_quadrangulated_spherical_surface: public t8_geometry_with_vertices {
 public:
  /* Basic constructor that sets the dimension and the name. */
  t8_geometry_quadrangulated_spherical_surface (): t8_geometry_with_vertices (2, "t8_quadrangulated_spherical_surface")
  {
  }

  /**
   * Map the faces of a cube to a spherical surface.
   * \param [in]  cmesh      The cmesh in which the point lies.
   * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
   * \param [in]  ref_coords  Array of \a dimension x \a num_coords many entries, specifying a point in /f$ [0,1]^\mathrm{dim} /f$.
   * \param [in]  num_coords  The number of points to map.
   * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords. The length is \a num_coords * 3.
   *
   * This routine expects an input mesh of six quadrangles arranged into a cube.
   *
   */
  void
  t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                    double out_coords[3]) const;

  /* Jacobian, not implemented. */
  void
  t8_geom_evaluate_jacobian (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                             double *jacobian) const
  {
    SC_ABORT_NOT_REACHED ();
  }

  /* Load tree data is inherited from t8_geometry_with_vertices. */
};

/** This geometry maps six hexaeders arranged as a cube to a spherical shell.
 */
class t8_geometry_cubed_spherical_shell: public t8_geometry_with_vertices {
 public:
  /* Basic constructor that sets the dimension and the name. */
  t8_geometry_cubed_spherical_shell (): t8_geometry_with_vertices (3, "t8_cubed_spherical_shell")
  {
  }

  /**
   * Map the faces of a cube to a spherical surface.
   * \param [in]  cmesh      The cmesh in which the point lies.
   * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
   * \param [in]  ref_coords  Array of \a dimension x \a num_coords many entries, specifying a point in /f$ [0,1]^\mathrm{dim} /f$.
   * \param [in]  num_coords  The number of points to map.
   * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords. The length is \a num_coords * 3.
   *
   * This routine expects an input mesh of six hexaeders arranged into a cube.
   *
   */
  void
  t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                    double out_coords[3]) const;

  /* Jacobian, not implemented. */
  void
  t8_geom_evaluate_jacobian (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                             double *jacobian) const
  {
    SC_ABORT_NOT_REACHED ();
  }

  /* Load tree data is inherited from t8_geometry_with_vertices. */
};

/** This geometry maps 3D elements to a spherical shell.
 */
class t8_geometry_spherical_shell: public t8_geometry_with_vertices {
 public:
  /* Basic constructor that sets the dimension and the name. */
  t8_geometry_spherical_shell (): t8_geometry_with_vertices (3, "t8_spherical_shell")
  {
  }

  /**
   * Map 3D elements to to a spherical surface.
   * \param [in]  cmesh      The cmesh in which the point lies.
   * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
   * \param [in]  ref_coords  Array of \a dimension x \a num_coords many entries, specifying a point in /f$ [0,1]^\mathrm{dim} /f$.
   * \param [in]  num_coords  The number of points to map.
   * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords. The length is \a num_coords * 3.
   *
   * This routine expects an input mesh of 3D elements arranged such that they
   * can be blown out to spherical shell..
   *
   */
  void
  t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                    double out_coords[3]) const;

  /* Jacobian, not implemented. */
  void
  t8_geom_evaluate_jacobian (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords, const size_t num_coords,
                             double *jacobian) const
  {
    SC_ABORT_NOT_REACHED ();
  }

  /* Load tree data is inherited from t8_geometry_with_vertices. */
};

#endif /* T8_GEOMETRY_EXAMPLES_HXX */
