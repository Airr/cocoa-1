/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1997-2012   The R Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <Defn.h>
#include <R_ext/RStartup.h>


/* Remove and process common command-line arguments
 *  Formally part of ../unix/sys-common.c.
 */

/*
  This copies the command line arguments to the Rstart
  structure. The memory is obtained from calloc, etc.
  since these are permanent and it is not intended that
  they be modified. This is why they are copied before
  being processed and removed from the list.

  We might store these as a SEXP. I have no strong opinion
  about this.
 */

/* Permanent copy of the command line arguments and the number
   of them passed to the application.
   These are populated via the routine R_set_command_line_arguments().
*/
static int    NumCommandLineArgs = 0;
static char **CommandLineArgs = NULL;


void
R_set_command_line_arguments(int argc, char **argv)
{
    return;
}


/*
  The .Internal which returns the command line arguments that are stored
  in global variables.
 */
SEXP attribute_hidden
do_commandArgs(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

#ifdef Win32
extern Rboolean R_LoadRconsole;
#endif

void
R_common_command_line(int *pac, char **argv, Rstart Rp)
{
    return;
}
