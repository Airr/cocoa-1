/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995-1998	Robert Gentleman and Ross Ihaka.
 *  Copyright (C) 2000-2012	The R Core Team.
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
 *
 *
 *  print.default()  ->	 do_printdefault (with call tree below)
 *
 *  auto-printing   ->  PrintValueEnv
 *                      -> PrintValueRec
 *                      -> call print() for objects
 *  Note that auto-printing does not call print.default.
 *  PrintValue, R_PV are similar to auto-printing.
 *
 *  do_printdefault
 *	-> PrintDefaults
 *	-> CustomPrintValue
 *	    -> PrintValueRec
 *		-> __ITSELF__  (recursion)
 *		-> PrintGenericVector	-> PrintValueRec  (recursion)
 *		-> printList		-> PrintValueRec  (recursion)
 *		-> printAttributes	-> PrintValueRec  (recursion)
 *		-> PrintExpression
 *		-> printVector		>>>>> ./printvector.c
 *		-> printNamedVector	>>>>> ./printvector.c
 *		-> printMatrix		>>>>> ./printarray.c
 *		-> printArray		>>>>> ./printarray.c
 *
 *  do_prmatrix
 *	-> PrintDefaults
 *	-> printMatrix			>>>>> ./printarray.c
 *
 *
 *  See ./printutils.c	 for general remarks on Printing
 *			 and the Encode.. utils.
 *
 *  Also ./printvector.c,  ./printarray.c
 *
 *  do_sink moved to connections.c as of 1.3.0
 *
 *  <FIXME> These routines are not re-entrant: they reset the
 *  global R_print.
 *  </FIXME>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Defn.h"
#include <Internal.h>
#include <Print.h>
#include "Fileio.h"
#include "Rconnections.h"
#include <S.h>


/* Global print parameter struct: */
R_print_par_t R_print;

static void printAttributes(SEXP, SEXP, Rboolean);
static void PrintSpecial(SEXP);
static void PrintLanguageEtc(SEXP, Rboolean, Rboolean);


#define TAGBUFLEN 256
static char tagbuf[TAGBUFLEN + 5];


/* Used in X11 module for dataentry */
/* NB this is called by R.app even though it is in no public header, so 
   alter there if you alter this */
void PrintDefaults(void)
{
  return;
}

SEXP attribute_hidden do_invisible(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#if 0
SEXP attribute_hidden do_visibleflag(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}
#endif

/* This is *only* called via outdated R_level prmatrix() : */
SEXP attribute_hidden do_prmatrix(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}/* do_prmatrix */

/* .Internal( print.function(f, useSource, ...)) */
SEXP attribute_hidden do_printfunction(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* PrintLanguage() or PrintClosure() : */
static void PrintLanguageEtc(SEXP s, Rboolean useSource, Rboolean isClosure)
{
  return;
}

static
void PrintClosure(SEXP s, Rboolean useSource)
{
  return;
}

static
void PrintLanguage(SEXP s, Rboolean useSource)
{
  return;
}

/* .Internal(print.default(x, digits, quote, na.print, print.gap,
			   right, max, useS4)) */
SEXP attribute_hidden do_printdefault(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}/* do_printdefault */


/* FIXME : We need a general mechanism for "rendering" symbols. */
/* It should make sure that it quotes when there are special */
/* characters and also take care of ansi escapes properly. */

static void PrintGenericVector(SEXP s, SEXP env)
{
  return;
}


static void printList(SEXP s, SEXP env)
{
  return;
}

static void PrintExpression(SEXP s)
{
  return;
}

static void PrintSpecial(SEXP s)
{
  return;
}

/* PrintValueRec -- recursively print an SEXP

 * This is the "dispatching" function for  print.default()
 */
void attribute_hidden PrintValueRec(SEXP s, SEXP env)
{
  return;
}

/* 2000-12-30 PR#715: remove list tags from tagbuf here
   to avoid $a$battr("foo").  Need to save and restore, since
   attributes might be lists with attributes or just have attributes ...
 */
static void printAttributes(SEXP s, SEXP env, Rboolean useSlots)
{
  return;
}/* printAttributes */


/* Print an S-expression using (possibly) local options.
   This is used for auto-printing from main.c */

void attribute_hidden PrintValueEnv(SEXP s, SEXP env)
{
  return;
}


/* Print an S-expression using global options */

void PrintValue(SEXP s)
{
  return;
}


/* Ditto, but only for objects, for use in debugging */

void R_PV(SEXP s)
{
  return;
}


void attribute_hidden CustomPrintValue(SEXP s, SEXP env)
{
  return;
}


/* xxxpr are mostly for S compatibility (as mentioned in V&R).
   The actual interfaces are now in xxxpr.f
 */

attribute_hidden
int F77_NAME(dblep0) (const char *label, int *nchar, double *data, int *ndata)
{
  return 0;
}

attribute_hidden
int F77_NAME(intpr0) (const char *label, int *nchar, int *data, int *ndata)
{
  return 0;
}

attribute_hidden
int F77_NAME(realp0) (const char *label, int *nchar, float *data, int *ndata)
{
  return 0;
}

/* Fortran-callable error routine for lapack */

void F77_NAME(xerbla)(const char *srname, int *info)
{
  return;
}
