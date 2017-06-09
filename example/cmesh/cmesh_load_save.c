/*
  This file is part of t8code.
  t8code is a C library to manage a collection (a forest) of multiple
  connected adaptive space-trees of general element types in parallel.

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

#include <sc_options.h>
#include <t8_cmesh.h>
#include <t8_cmesh_vtk.h>
#include <t8_cmesh_readmshfile.h>

/* TODO: rename this file to t8_something */

static void
t8_cmesh_load_distribute (const char *fileprefix, int num_files, int no_vtk)
{
  t8_cmesh_t          cmesh, cmesh_partition;

  cmesh = t8_cmesh_load_and_distribute (fileprefix, num_files,
                                        sc_MPI_COMM_WORLD, T8_LOAD_SIMPLE,
                                        -1);
  if (cmesh == NULL) {
    t8_errorf ("Error when reading cmesh\n");
    return;
  }
  else {
    t8_debugf ("Successfully loaded cmesh from %s files\n", fileprefix);
    if (!no_vtk) {
      t8_cmesh_vtk_write_file (cmesh, "cmesh_dist_loaded", 1.0);
    }
    t8_cmesh_init (&cmesh_partition);
    t8_cmesh_set_derive (cmesh_partition, cmesh);
    t8_cmesh_set_partition_uniform (cmesh_partition, 0);
    t8_cmesh_commit (cmesh_partition, sc_MPI_COMM_WORLD);
    if (!no_vtk) {
      t8_cmesh_vtk_write_file (cmesh_partition, "cmesh_dist_loaded_partition",
                               1.0);
    }
    t8_cmesh_destroy (&cmesh_partition);
  }
}

static void
t8_cmesh_save_cmesh (const char *mshfile, int dim, int use_metis, int no_vtk)
{
  t8_cmesh_t          cmesh;
  char                filename[BUFSIZ];
  int                 ret, mpirank, mpiret;

  if (mshfile == NULL) {
    cmesh = t8_cmesh_new_hypercube (T8_ECLASS_TET, sc_MPI_COMM_WORLD, 0, 1);
  }
  else {
    t8_cmesh_t          cmesh_partition;
    /* If use_metis is true, the cmesh that we read from the file cannot be partition,
     * we thus pass !use_metis as the partition flag. */
    cmesh =
      t8_cmesh_from_msh_file (mshfile, !use_metis, sc_MPI_COMM_WORLD, dim, 0,
                              use_metis);
    t8_cmesh_init (&cmesh_partition);
    t8_cmesh_set_derive (cmesh_partition, cmesh);
    t8_cmesh_set_partition_uniform (cmesh_partition, 0);
    t8_cmesh_commit (cmesh_partition, sc_MPI_COMM_WORLD);
    cmesh = cmesh_partition;
  }
  mpiret = sc_MPI_Comm_rank (sc_MPI_COMM_WORLD, &mpirank);
  SC_CHECK_MPI (mpiret);
  snprintf (filename, BUFSIZ, "cmesh_saved_%04d.cmesh", mpirank);
  ret = t8_cmesh_save (cmesh, filename);
  if (ret == 0) {
    t8_errorf ("Error when writing to file\n");
  }
  else {
    t8_debugf ("Saved cmesh to %s\n", filename);
  }
  if (!no_vtk) {
    snprintf (filename, BUFSIZ, "cmesh_saved");
    t8_cmesh_vtk_write_file (cmesh, filename, 1.0);
  }
  t8_cmesh_destroy (&cmesh);
}

#if 0
static void
t8_cmesh_load_cmesh ()
{
  t8_cmesh_t          cmesh;
  char                filename[BUFSIZ];
  int                 mpirank, mpiret;

  mpiret = sc_MPI_Comm_rank (sc_MPI_COMM_WORLD, &mpirank);
  SC_CHECK_MPI (mpiret);
  snprintf (filename, BUFSIZ, "cmesh_saved_%04d.cmesh", mpirank);
  cmesh = t8_cmesh_load (filename, sc_MPI_COMM_WORLD);
  if (cmesh != NULL) {
    t8_debugf ("Successfully loaded cmesh from %s\n", filename);
    t8_cmesh_vtk_write_file (cmesh, "cmesh_loaded", 1.0);
    t8_cmesh_destroy (&cmesh);
  }
}
#endif

int
main (int argc, char **argv)
{
  int                 mpiret, n, parsed, dim, no_vtk, helpme = 0;
  int                 use_metis;
  const char         *meshfile, *loadfile;
  sc_options_t       *opt;
  char                usage[BUFSIZ];
  char                help[BUFSIZ];

  snprintf (usage, BUFSIZ, "Usage:\t%s <OPTIONS> <ARGUMENTS>\n\t%s -h\t"
            "for a brief overview of all options.",
            basename (argv[0]), basename (argv[0]));
  snprintf (help, BUFSIZ,
            "This program has two modes. With argument -f <file> -d <dim> it creates a cmesh, "
            "from the file <file>.msh, saves it to a collection of files and loads it again.\n"
            "If the -l <string> and -n <num> arguments are given, the cmesh stored "
            "in the num files string_0000.cmesh,... ,string_num-1.cmesh are read on n processes "
            "and distributed among all processes.\n\n%s\n", usage);

  mpiret = sc_MPI_Init (&argc, &argv);
  SC_CHECK_MPI (mpiret);

  sc_init (sc_MPI_COMM_WORLD, 1, 1, NULL, SC_LP_ESSENTIAL);
  t8_init (SC_LP_DEFAULT);

  opt = sc_options_new (argv[0]);
  sc_options_add_switch (opt, 'h', "help", &helpme,
                         "Display a short help message.");
  sc_options_add_string (opt, 'l', "load", &loadfile, "", "The prefix of the"
                         " .cmesh file to load.");
  sc_options_add_int (opt, 'n', "num-files", &n, -1,
                      "The total number of .cmesh files.");
  sc_options_add_switch (opt, 'o', "no-vtk", &no_vtk,
                         "Do not write vtk output.");
  sc_options_add_string (opt, 'f', "msh-file", &meshfile, "",
                         "The prefix of the .msh file.");
  sc_options_add_int (opt, 'd', "dim", &dim, 2,
                      "The dimension of the msh file.");

  /* TODO: add a parameter to control the number of metis partitions. i.e. -m 4 for 4 partitions or smth. */
  sc_options_add_switch (opt, 'm', "metis", &use_metis,
                         "Use Metis (serial) to repartition the mesh. Only active together with -f.");
  parsed =
    sc_options_parse (t8_get_package_id (), SC_LP_ERROR, opt, argc, argv);
  if (helpme) {
    /* Display help message */
    sc_options_print_usage (t8_get_package_id (), SC_LP_ERROR, opt, NULL);
  }
  /* Check for wrong usage:
   * - Neither meshfile nor loadfile specified
   * - loadfile specified but invalid number of files */
  else if (parsed < 0
           || (strcmp (meshfile, "") == 0 && strcmp (loadfile, "") == 0)
           || (strcmp (loadfile, "") != 0 && n <= 0)
           || dim < 2 || dim > 3) {
    t8_global_errorf ("%s", help);
  }
#ifndef T8_WITH_METIS
  else if (use_metis) {
    t8_global_errorf ("t8code is not compilled with Metis support.\n");
    t8_global_errorf ("Link t8code with Metis to use this feature.\n");
  }
#endif
  else {
    if (strcmp (meshfile, "") != 0) {
      /* If a meshfile was specified, we load it and save the cmesh on disk */
      t8_cmesh_save_cmesh (meshfile, dim, use_metis, no_vtk);
    }
    else {
      /* A load file and a number of processes was given */
      T8_ASSERT (strcmp (loadfile, "") != 0);
      T8_ASSERT (n > 0);
      t8_cmesh_load_distribute (loadfile, n, no_vtk);
    }
  }

  sc_options_destroy (opt);
  sc_finalize ();

  mpiret = sc_MPI_Finalize ();
  SC_CHECK_MPI (mpiret);

  return 0;
}
