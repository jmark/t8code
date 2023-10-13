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

#include <t8_geometry/t8_geometry_implementations/t8_geometry_examples.hxx>
#include <t8_geometry/t8_geometry_helpers.h>
#include <t8_vec.h>

void
t8_geometry_squared_disk::t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords,
                                            const size_t num_coords, double *out_coords) const
{
  if (num_coords != 1)
    SC_ABORT ("Error: Batch computation of geometry not yet supported.");

  double n[3]; /* Normal vector. */
  double r[3]; /* Radial vector. */
  double s[3]; /* Radial vector for the corrected coordinates. */
  double p[3]; /* Vector on the plane resp. quad. */

  t8_locidx_t ltreeid = t8_cmesh_get_local_id (cmesh, gtreeid);
  double *tree_vertices = t8_cmesh_get_tree_vertices (cmesh, ltreeid);

  /* Center square. */
  if (gtreeid % 3 == 0) {
    for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
      size_t offset = 3 * i_coord;

      t8_geom_linear_interpolation (ref_coords + offset, tree_vertices, 3, 2, p);

      out_coords[offset + 0] = p[0];
      out_coords[offset + 1] = p[1];
      out_coords[offset + 2] = 0.0;
    }
    
    return;
  }

  {
    n[0] = tree_vertices[0 + 0];
    n[1] = tree_vertices[0 + 1];
    n[2] = tree_vertices[0 + 2];

    /* Normalize vector `n`. */
    const double norm = sqrt (n[0] * n[0] + n[1] * n[1]);
    n[0] = n[0] / norm;
    n[1] = n[1] / norm;
  }

  {
    r[0] = tree_vertices[9 + 0];
    r[1] = tree_vertices[9 + 1];
    r[2] = tree_vertices[9 + 2];

    /* Normalize vector `r`. */
    const double norm = sqrt (r[0] * r[0] + r[1] * r[1]);
    r[0] = r[0] / norm;
    r[1] = r[1] / norm;
  }

  for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
    size_t offset = 3 * i_coord;

    const double x_ref = ref_coords[offset + 0];
    const double y_ref = ref_coords[offset + 1];

    {
      double corr_ref_coords[3];

      /* Correction in order to rectify elements near the corners. */
      corr_ref_coords[0] = tan (0.25*M_PI * x_ref);
      corr_ref_coords[1] = y_ref;
      corr_ref_coords[2] = 0.0;

      /* Compute and normalize vector `s`. */
      t8_geom_linear_interpolation (corr_ref_coords, tree_vertices, 3, 2, s);

      const double norm = sqrt (s[0] * s[0] + s[1] * s[1]);
      s[0] = s[0] / norm;
      s[1] = s[1] / norm;
    }

    t8_geom_linear_interpolation (ref_coords + offset, tree_vertices, 3, 2, p);

    /* Compute intersection of line with a plane. */
    const double R = (p[0] * n[0] + p[1] * n[1]) / (r[0] * n[0] + r[1] * n[1]);

    out_coords[offset + 0] = (1.0 - y_ref) * p[0] + y_ref * R * s[0];
    out_coords[offset + 1] = (1.0 - y_ref) * p[1] + y_ref * R * s[1];
    out_coords[offset + 2] = 0.0;
  }
}

/**
 * Maps a general 2D element with its vertices sitting on a sphere to its curvilinear surface.
 * \param [in]  cmesh      The cmesh in which the point lies.
 * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
 * \param [in]  ref_coords  Array of \a dimension many entries, specifying a point in [0,1]^dimension.
 * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords.
 */
void
t8_geometry_spherical_surface::t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid,
                                                                const double *ref_coords, const size_t num_coords,
                                                                double *out_coords) const
{
  double n[3]; /* Normal vector. */
  double r[3]; /* Radial vector. */
  double p[3]; /* Vector on the plane. */

  const t8_locidx_t ltreeid = t8_cmesh_get_local_id (cmesh, gtreeid);
  const t8_eclass_t eclass = t8_cmesh_get_tree_class (cmesh, ltreeid);

  double *tree_vertices = t8_cmesh_get_tree_vertices (cmesh, ltreeid);

  t8_vec_tri_normal (tree_vertices, tree_vertices + 3, tree_vertices + 6, n);
  t8_vec_normalize (n);

  r[0] = tree_vertices[0];
  r[1] = tree_vertices[1];
  r[2] = tree_vertices[2];
  t8_vec_normalize (r);

  for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
    const size_t offset = 3 * i_coord;

    t8_geom_compute_linear_geometry (eclass, tree_vertices, ref_coords + offset, 1, p);

    const double norm = t8_vec_norm (p);
    const double R = (p[0] * n[0] + p[1] * n[1] + p[2] * n[2]) / (r[0] * n[0] + r[1] * n[1] + r[2] * n[2]) / norm;

    out_coords[offset + 0] = R * p[0];
    out_coords[offset + 1] = R * p[1];
    out_coords[offset + 2] = R * p[2];
  }
}

/**
 * Map the faces of an oktaeder to a spherical surface.
 * \param [in]  cmesh      The cmesh in which the point lies.
 * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
 * \param [in]  ref_coords  Array of \a dimension many entries, specifying a point in [0,1]^dimension.
 * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords.
 */
void
t8_geometry_triangulated_spherical_surface::t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid,
                                                              const double *ref_coords, const size_t num_coords,
                                                              double *out_coords) const
{
  double n[3]; /* Normal vector of the current triangle. */
  double r[3]; /* Radial vector through one of triangle's corners. */
  double p[3]; /* Position vector on the triangle plane. */

  t8_locidx_t ltreeid = t8_cmesh_get_local_id (cmesh, gtreeid);
  double *tree_vertices = t8_cmesh_get_tree_vertices (cmesh, ltreeid);

  t8_vec_tri_normal (tree_vertices, tree_vertices + 3, tree_vertices + 6, n);
  t8_vec_normalize (n);

  r[0] = tree_vertices[0];
  r[1] = tree_vertices[1];
  r[2] = tree_vertices[2];
  t8_vec_normalize (r);

  /* Init output coordinates with zeros. */
  for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
    const size_t offset = 3 * i_coord;

    out_coords[offset + 0] = 0.0;
    out_coords[offset + 1] = 0.0;
    out_coords[offset + 2] = 0.0;
  }

  /* The next three code blocks straighten out the elements near the triangle
   * corners by averaging the rectification with all three corners. */

  /* We average over the three corners of the triangle. */
  const double avg_factor = 1.0 / 3.0;

  /* First triangle corner. */
  {
    double u[3]; /* Position vector. */
    double v[3]; /* First triangle side. */
    double w[3]; /* Second triangle side. */

    u[0] = tree_vertices[0];
    u[1] = tree_vertices[1];
    u[2] = tree_vertices[2];

    v[0] = tree_vertices[3 + 0] - u[0];
    v[1] = tree_vertices[3 + 1] - u[1];
    v[2] = tree_vertices[3 + 2] - u[2];

    w[0] = tree_vertices[6 + 0] - u[0];
    w[1] = tree_vertices[6 + 1] - u[1];
    w[2] = tree_vertices[6 + 2] - u[2];

    /* Reference coordinates from this particular triangle corner. */
    const double u_ref[3] = { 0.0, 0.0, 0.0 };
    const double v_ref[3] = { 1.0, 0.0, 0.0 };
    const double w_ref[3] = { -1.0, 1.0, 0.0 };

    for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
      const size_t offset = 3 * i_coord;

      const double x = ref_coords[offset + 0];
      const double y = ref_coords[offset + 1];

      /* Compute local triangle coordinate. */
      const double vv = u_ref[0] + x * v_ref[0] + y * w_ref[0];
      const double ww = u_ref[1] + x * v_ref[1] + y * w_ref[1];

      /* Correction in order to rectify elements near the corners. */
      const double vv_corr = tan (0.5 * M_PI * (vv - 0.5)) * 0.5 + 0.5;
      const double ww_corr = tan (0.5 * M_PI * (ww - 0.5)) * 0.5 + 0.5;

      /* Compute and apply the corrected mapping. */
      p[0] = u[0] + vv_corr * v[0] + ww_corr * w[0];
      p[1] = u[1] + vv_corr * v[1] + ww_corr * w[1];
      p[2] = u[2] + vv_corr * v[2] + ww_corr * w[2];

      const double norm = sqrt (p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
      const double R
        = (p[0] * n[0] + p[1] * n[1] + p[2] * n[2]) / (r[0] * n[0] + r[1] * n[1] + r[2] * n[2]) / norm * avg_factor;

      /* Note, in `R` there already is the avg. factor `1/3` included. */
      out_coords[offset + 0] = out_coords[offset + 0] + R * p[0];
      out_coords[offset + 1] = out_coords[offset + 1] + R * p[1];
      out_coords[offset + 2] = out_coords[offset + 2] + R * p[2];
    }
  }

  /* Second triangle corner. */
  {
    double u[3]; /* Position vector. */
    double v[3]; /* First triangle side. */
    double w[3]; /* Second triangle side. */

    u[0] = tree_vertices[6 + 0];
    u[1] = tree_vertices[6 + 1];
    u[2] = tree_vertices[6 + 2];

    v[0] = tree_vertices[0 + 0] - u[0];
    v[1] = tree_vertices[0 + 1] - u[1];
    v[2] = tree_vertices[0 + 2] - u[2];

    w[0] = tree_vertices[3 + 0] - u[0];
    w[1] = tree_vertices[3 + 1] - u[1];
    w[2] = tree_vertices[3 + 2] - u[2];

    /* Reference coordinates from this particular triangle corner. */
    const double u_ref[3] = { 1.0, 0.0, 0.0 };
    const double v_ref[3] = { -1.0, 1.0, 0.0 };
    const double w_ref[3] = { 0.0, -1.0, 0.0 };

    for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
      const size_t offset = 3 * i_coord;

      const double x = ref_coords[offset + 0];
      const double y = ref_coords[offset + 1];

      /* Compute local triangle coordinate. */
      const double vv = u_ref[0] + x * v_ref[0] + y * w_ref[0];
      const double ww = u_ref[1] + x * v_ref[1] + y * w_ref[1];

      /* Correction in order to rectify elements near the corners. */
      const double vv_corr = tan (0.5 * M_PI * (vv - 0.5)) * 0.5 + 0.5;
      const double ww_corr = tan (0.5 * M_PI * (ww - 0.5)) * 0.5 + 0.5;

      /* Compute and apply the corrected mapping. */
      p[0] = u[0] + vv_corr * v[0] + ww_corr * w[0];
      p[1] = u[1] + vv_corr * v[1] + ww_corr * w[1];
      p[2] = u[2] + vv_corr * v[2] + ww_corr * w[2];

      const double norm = sqrt (p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
      const double R
        = (p[0] * n[0] + p[1] * n[1] + p[2] * n[2]) / (r[0] * n[0] + r[1] * n[1] + r[2] * n[2]) / norm * avg_factor;

      /* Note, in `R` there already is the avg. factor `1/3` included. */
      out_coords[offset + 0] = out_coords[offset + 0] + R * p[0];
      out_coords[offset + 1] = out_coords[offset + 1] + R * p[1];
      out_coords[offset + 2] = out_coords[offset + 2] + R * p[2];
    }
  }

  /* Third triangle corner. */
  {
    double u[3]; /* Position vector. */
    double v[3]; /* First triangle side. */
    double w[3]; /* Second triangle side. */

    u[0] = tree_vertices[3 + 0];
    u[1] = tree_vertices[3 + 1];
    u[2] = tree_vertices[3 + 2];

    v[0] = tree_vertices[6 + 0] - u[0];
    v[1] = tree_vertices[6 + 1] - u[1];
    v[2] = tree_vertices[6 + 2] - u[2];

    w[0] = tree_vertices[0 + 0] - u[0];
    w[1] = tree_vertices[0 + 1] - u[1];
    w[2] = tree_vertices[0 + 2] - u[2];

    /* Reference coordinates from this particular triangle corner. */
    const double u_ref[3] = { 0.0, 1.0, 0.0 };
    const double v_ref[3] = { 0.0, -1.0, 0.0 };
    const double w_ref[3] = { 1.0, 0.0, 0.0 };

    for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
      const size_t offset = 3 * i_coord;

      const double x = ref_coords[offset + 0];
      const double y = ref_coords[offset + 1];

      /* Compute local triangle coordinate. */
      const double vv = u_ref[0] + x * v_ref[0] + y * w_ref[0];
      const double ww = u_ref[1] + x * v_ref[1] + y * w_ref[1];

      /* Correction in order to rectify elements near the corners. */
      const double vv_corr = tan (0.5 * M_PI * (vv - 0.5)) * 0.5 + 0.5;
      const double ww_corr = tan (0.5 * M_PI * (ww - 0.5)) * 0.5 + 0.5;

      /* Compute and apply the corrected mapping. */
      p[0] = u[0] + vv_corr * v[0] + ww_corr * w[0];
      p[1] = u[1] + vv_corr * v[1] + ww_corr * w[1];
      p[2] = u[2] + vv_corr * v[2] + ww_corr * w[2];

      const double norm = sqrt (p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
      const double R
        = (p[0] * n[0] + p[1] * n[1] + p[2] * n[2]) / (r[0] * n[0] + r[1] * n[1] + r[2] * n[2]) / norm * avg_factor;

      /* Note, in `R` there already is the avg. factor `1/3` included. */
      out_coords[offset + 0] = out_coords[offset + 0] + R * p[0];
      out_coords[offset + 1] = out_coords[offset + 1] + R * p[1];
      out_coords[offset + 2] = out_coords[offset + 2] + R * p[2];
    }
  }
}

/**
 * Maps a general 3D element with its vertices of one face sitting on a sphere to a curvilinear spherical shell.
 * \param [in]  cmesh      The cmesh in which the point lies.
 * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
 * \param [in]  ref_coords  Array of \a dimension many entries, specifying a point in [0,1]^dimension.
 * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords.
 */
void
t8_geometry_spherical_shell::t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid,
                                          const double *ref_coords, const size_t num_coords,
                                         double *out_coords) const
{
  double n[3]; /* Normal vector. */
  double r[3]; /* Radial vector. */
  double p[3]; /* Vector on the plane. */

  const t8_locidx_t ltreeid = t8_cmesh_get_local_id (cmesh, gtreeid);
  const t8_eclass_t eclass = t8_cmesh_get_tree_class (cmesh, ltreeid);

  double *tree_vertices = t8_cmesh_get_tree_vertices (cmesh, ltreeid);

  t8_vec_tri_normal (tree_vertices, tree_vertices + 3, tree_vertices + 6, n);
  t8_vec_normalize (n);

  r[0] = tree_vertices[0];
  r[1] = tree_vertices[1];
  r[2] = tree_vertices[2];
  t8_vec_normalize (r);

  for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
    const size_t offset = 3 * i_coord;

    t8_geom_compute_linear_geometry (eclass, tree_vertices, ref_coords + offset, 1, p);

    const double norm = t8_vec_norm (p);
    const double R = (p[0] * n[0] + p[1] * n[1] + p[2] * n[2]) / (r[0] * n[0] + r[1] * n[1] + r[2] * n[2]) / norm;

    out_coords[offset + 0] = R * p[0];
    out_coords[offset + 1] = R * p[1];
    out_coords[offset + 2] = R * p[2];
  }
}

/**
 * Map the faces of a unit cube to a spherical surface.
 * \param [in]  cmesh      The cmesh in which the point lies.
 * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
 * \param [in]  ref_coords  Array of \a dimension many entries, specifying a point in [0,1]^dimension.
 * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords.
 */
void
t8_geometry_quadrangulated_spherical_surface::t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid,
                                                                const double *ref_coords, const size_t num_coords,
                                                                double *out_coords) const
{
  double n[3]; /* Normal vector. */
  double r[3]; /* Radial vector. */
  double p[3]; /* Vector on the plane. */

  t8_locidx_t ltreeid = t8_cmesh_get_local_id (cmesh, gtreeid);
  double *tree_vertices = t8_cmesh_get_tree_vertices (cmesh, ltreeid);

  {
    const double center_ref[3] = { 0.5, 0.5, 0.0 };
    t8_geom_linear_interpolation (center_ref, tree_vertices, 3, 2, n);

    /* Normalize vector `n`. */
    const double norm = sqrt (n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    n[0] = n[0] / norm;
    n[1] = n[1] / norm;
    n[2] = n[2] / norm;
  }

  r[0] = tree_vertices[0];
  r[1] = tree_vertices[1];
  r[2] = tree_vertices[2];

  {
    /* Normalize vector `r`. */
    const double norm = sqrt (r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
    r[0] = r[0] / norm;
    r[1] = r[1] / norm;
    r[2] = r[2] / norm;
  }

  for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
    const size_t offset = 3 * i_coord;

    {
      double corr_ref_coords[3]; /* Corrected reference coordinates. */

      const double x = ref_coords[offset + 0];
      const double y = ref_coords[offset + 1];
      const double z = ref_coords[offset + 2];

      /* Correction in order to rectify elements near the corners. */
      corr_ref_coords[0] = tan (0.5 * M_PI * (x - 0.5)) * 0.5 + 0.5;
      corr_ref_coords[1] = tan (0.5 * M_PI * (y - 0.5)) * 0.5 + 0.5;
      corr_ref_coords[2] = z;

      t8_geom_linear_interpolation (corr_ref_coords, tree_vertices, 3, 2, p);
    }

    const double R = (p[0] * n[0] + p[1] * n[1] + p[2] * n[2]) / (r[0] * n[0] + r[1] * n[1] + r[2] * n[2]);

    {
      /* Normalize vector `p`. */
      const double norm = sqrt (p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
      p[0] = p[0] / norm;
      p[1] = p[1] / norm;
      p[2] = p[2] / norm;
    }

    out_coords[offset + 0] = R * p[0];
    out_coords[offset + 1] = R * p[1];
    out_coords[offset + 2] = R * p[2];
  }
}

/**
 * Maps six hexaeders arranged into cube to a spherical shell.
 * \param [in]  cmesh      The cmesh in which the point lies.
 * \param [in]  gtreeid    The global tree (of the cmesh) in which the reference point is.
 * \param [in]  ref_coords  Array of \a dimension many entries, specifying a point in [0,1]^dimension.
 * \param [out] out_coords  The mapped coordinates in physical space of \a ref_coords.
 */
void
t8_geometry_cubed_spherical_shell::t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords,
                                                     const size_t num_coords, double *out_coords) const
{
  double n[3]; /* Normal vector. */
  double r[3]; /* Radial vector. */
  double p[3]; /* Vector on the plane. */

  t8_locidx_t ltreeid = t8_cmesh_get_local_id (cmesh, gtreeid);
  double *tree_vertices = t8_cmesh_get_tree_vertices (cmesh, ltreeid);

  {
    const double center_ref[3] = { 0.5, 0.5, 0.0 };
    t8_geom_linear_interpolation (center_ref, tree_vertices, 3, 3, n);

    /* Normalize vector `n`. */
    const double norm = sqrt (n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    n[0] = n[0] / norm;
    n[1] = n[1] / norm;
    n[2] = n[2] / norm;
  }

  r[0] = tree_vertices[0];
  r[1] = tree_vertices[1];
  r[2] = tree_vertices[2];

  {
    /* Normalize vector `r`. */
    const double norm = sqrt (r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
    r[0] = r[0] / norm;
    r[1] = r[1] / norm;
    r[2] = r[2] / norm;
  }

  for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
    const size_t offset = 3 * i_coord;

    {
      double corr_ref_coords[3];

      const double x = ref_coords[offset + 0];
      const double y = ref_coords[offset + 1];
      const double z = ref_coords[offset + 2];

      /* Correction in order to rectify elements near the corners. */
      corr_ref_coords[0] = tan (0.5 * M_PI * (x - 0.5)) * 0.5 + 0.5;
      corr_ref_coords[1] = tan (0.5 * M_PI * (y - 0.5)) * 0.5 + 0.5;
      corr_ref_coords[2] = z;

      t8_geom_linear_interpolation (corr_ref_coords, tree_vertices, 3, 3, p);
    }

    const double R = (p[0] * n[0] + p[1] * n[1] + p[2] * n[2]) / (r[0] * n[0] + r[1] * n[1] + r[2] * n[2]);

    {
      /* Normalize vector `p`. */
      const double norm = sqrt (p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
      p[0] = p[0] / norm;
      p[1] = p[1] / norm;
      p[2] = p[2] / norm;
    }

    out_coords[offset + 0] = R * p[0];
    out_coords[offset + 1] = R * p[1];
    out_coords[offset + 2] = R * p[2];
  }
}

void
t8_geometry_cubed_sphere::t8_geom_evaluate (t8_cmesh_t cmesh, t8_gloidx_t gtreeid, const double *ref_coords,
                                            const size_t num_coords, double *out_coords) const
{
  double n[3]; /* Normal vector. */
  double r[3]; /* Radial vector. */
  double s[3]; /* Radial vector for the corrected coordinates. */
  double p[3]; /* Vector on the plane resp. quad. */

  t8_locidx_t ltreeid = t8_cmesh_get_local_id (cmesh, gtreeid);
  double *tree_vertices = t8_cmesh_get_tree_vertices (cmesh, ltreeid);

  /* Center square. */
  if (gtreeid % 4 == 0) {
    for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
      size_t offset = 3 * i_coord;

      t8_geom_linear_interpolation (ref_coords + offset, tree_vertices, 3, 3, p);

      out_coords[offset + 0] = p[0];
      out_coords[offset + 1] = p[1];
      out_coords[offset + 2] = p[2];
    }
    
    return;
  }

  n[0] = tree_vertices[0 + 0];
  n[1] = tree_vertices[0 + 1];
  n[2] = tree_vertices[0 + 2];
  t8_vec_normalize (n);

  r[0] = tree_vertices[7*3 + 0];
  r[1] = tree_vertices[7*3 + 1];
  r[2] = tree_vertices[7*3 + 2];
  t8_vec_normalize (r);

  for (size_t i_coord = 0; i_coord < num_coords; i_coord++) {
    size_t offset = 3 * i_coord;

    const double x_ref = ref_coords[offset + 0];
    const double y_ref = ref_coords[offset + 1];
    const double z_ref = ref_coords[offset + 2];

    {
      double corr_ref_coords[3];

      /* Correction in order to rectify elements near the corners. */
      corr_ref_coords[0] = tan (0.25*M_PI * x_ref);
      corr_ref_coords[1] = tan (0.25*M_PI * y_ref);
      corr_ref_coords[2] = z_ref;

      /* Compute and normalize vector `s`. */
      t8_geom_linear_interpolation (corr_ref_coords, tree_vertices, 3, 3, s);
      t8_vec_normalize (s);
    }

    t8_geom_linear_interpolation (ref_coords + offset, tree_vertices, 3, 3, p);

    /* Compute intersection of line with a plane. */
    const double R = (p[0] * n[0] + p[1] * n[1] + p[2] * n[2]) / (r[0] * n[0] + r[1] * n[1] + r[2] * n[2]);

    out_coords[offset + 0] = (1.0 - z_ref) * p[0] + z_ref * R * s[0];
    out_coords[offset + 1] = (1.0 - z_ref) * p[1] + z_ref * R * s[1];
    out_coords[offset + 2] = (1.0 - z_ref) * p[2] + z_ref * R * s[2];
  }
}

T8_EXTERN_C_BEGIN ();

void
t8_geometry_destroy (t8_geometry_c **geom)
{
  T8_ASSERT (geom != NULL);

  delete *geom;
  *geom = NULL;
}

/* Satisfy the C interface from t8_geometry_linear.h. */
t8_geometry_c *
t8_geometry_squared_disk_new ()
{
  t8_geometry_squared_disk *geom = new t8_geometry_squared_disk ();
  return (t8_geometry_c *) geom;
}

t8_geometry_c *
t8_geometry_spherical_surface_new ()
{
  t8_geometry_spherical_surface *geom = new t8_geometry_spherical_surface ();
  return (t8_geometry_c *) geom;
}

t8_geometry_c *
t8_geometry_spherical_shell_new ()
{
  t8_geometry_spherical_shell *geom = new t8_geometry_spherical_shell ();
  return (t8_geometry_c *) geom;
}

t8_geometry_c *
t8_geometry_triangulated_spherical_surface_new ()
{
  t8_geometry_triangulated_spherical_surface *geom = new t8_geometry_triangulated_spherical_surface ();
  return (t8_geometry_c *) geom;
}

t8_geometry_c *
t8_geometry_quadrangulated_spherical_surface_new ()
{
  t8_geometry_quadrangulated_spherical_surface *geom = new t8_geometry_quadrangulated_spherical_surface ();
  return (t8_geometry_c *) geom;
}

t8_geometry_c *
t8_geometry_cubed_spherical_shell_new ()
{
  t8_geometry_cubed_spherical_shell *geom = new t8_geometry_cubed_spherical_shell ();
  return (t8_geometry_c *) geom;
}

t8_geometry_c *
t8_geometry_cubed_sphere_new ()
{
  t8_geometry_cubed_sphere *geom = new t8_geometry_cubed_sphere ();
  return (t8_geometry_c *) geom;
}

T8_EXTERN_C_END ();
