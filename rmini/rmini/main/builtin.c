/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995-1998  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1999-2012  The R Core Team.
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

#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>
#include <Print.h>
#include <Fileio.h>
#include <Rconnections.h>

#include <R_ext/RS.h> /* for Memzero */

attribute_hidden
R_xlen_t asVecSize(SEXP x)
{
  return 0;
}

SEXP attribute_hidden do_delayed(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* makeLazy(names, values, expr, eenv, aenv) */
SEXP attribute_hidden do_makelazy(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* This is a primitive SPECIALSXP */
SEXP attribute_hidden do_onexit(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_args(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_formals(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_body(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_bodyCode(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* get environment from a subclass if possible; else return NULL */
#define simple_as_environment(arg) (IS_S4_OBJECT(arg) && (TYPEOF(arg) == S4SXP) ? R_getS4DataSlot(arg, ENVSXP) : arg)


SEXP attribute_hidden do_envir(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_envirgets(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}


/** do_newenv() :  .Internal(new.env(hash, parent, size))
 *
 * @return a newly created environment()
 */
SEXP attribute_hidden do_newenv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_parentenv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_parentenvgets(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_envirName(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#ifdef Win32
# include "rgui_UTF8.h"
#endif
static const char *trChar(SEXP x)
{
  return 0;
}

static void cat_newline(SEXP labels, int *width, int lablen, int ntot)
{
  return;
}

static void cat_sepwidth(SEXP sep, int *width, int ntot)
{
  return;
}

static void cat_printsep(SEXP sep, int ntot)
{
  return;
}

typedef struct cat_info {
    Rboolean wasopen;
    int changedcon;
    Rconnection con;
} cat_info;

static void cat_cleanup(void *data)
{
  return;
}

SEXP attribute_hidden do_cat(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_makelist(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* This is a primitive SPECIALSXP */
SEXP attribute_hidden do_expression(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* vector(mode="logical", length=0) */
SEXP attribute_hidden do_makevector(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}


/* do_lengthgets: assign a length to a vector or a list */
/* (if it is vectorizable). We could probably be fairly */
/* clever with memory here if we wanted to. */

/* used in connections.c */
SEXP xlengthgets(SEXP x, R_xlen_t len)
{
  return 0;
}

/* public older version */
SEXP lengthgets(SEXP x, R_len_t len)
{
  return 0;
}


SEXP attribute_hidden do_lengthgets(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* Expand dots in args, but do not evaluate */
static SEXP expandDots(SEXP el, SEXP rho)
{
  return 0;
}

/* This function is used in do_switch to record the default value and
   to detect multiple defaults, which are not allowed as of 2.13.x */
   
static SEXP setDflt(SEXP arg, SEXP dflt) 
{
  return 0;
}

/* For switch, evaluate the first arg, if it is a character then try
 to match the name with the remaining args, and evaluate the match. If
 the value is missing then take the next non-missing arg as the value.
 Then things like switch(as.character(answer), yes=, YES=1, no=, NO=2,
 3) will work.  But if there is no 'next', return NULL. One arg beyond
 the first is allowed to be unnamed; it becomes the default value if
 there is no match.
 
 If the value of the first arg is not a character string
 then coerce it to an integer k and choose the kth argument from those
 that remain provided 1 < k < nargs.  

 Changed in 2.11.0 to be primitive, so the wrapper does not partially
 match to EXPR, and to return NULL invisibly if it is an error
 condition.

 This is a SPECIALSXP, so arguments need to be evaluated as needed.
  And (see names.c) X=2, so it defaults to a visible value.
*/


SEXP attribute_hidden do_switch(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}
