/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2012  The R Core Team
 *  Copyright (C) 2002--2005  The R Foundation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
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

/* Code to handle list / vector switch */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>
#include <R_ext/PrtUtil.h> // for IndexWidth
#define imax2(x, y) ((x < y) ? y : x)

#include "RBufferUtils.h"
static R_StringBuffer cbuff = {NULL, 0, MAXELTSIZE};

#define LIST_ASSIGN(x) {SET_VECTOR_ELT(data->ans_ptr, data->ans_length, x); data->ans_length++;}

static SEXP cbind(SEXP, SEXP, SEXPTYPE, SEXP, int);
static SEXP rbind(SEXP, SEXP, SEXPTYPE, SEXP, int);

/* The following code establishes the return type for the */
/* functions  unlist, c, cbind, and rbind and also determines */
/* whether the returned object is to have a names attribute. */

struct BindData {
 int  ans_flags;
 SEXP ans_ptr;
 R_xlen_t ans_length;
 SEXP ans_names;
 int  ans_nnames;
/* int  deparse_level; Initialize to 1. */
};

static int HasNames(SEXP x)
{
  return 0;
}

static void
AnswerType(SEXP x, int recurse, int usenames, struct BindData *data, SEXP call)
{
  return;
}


/* The following functions are used to coerce arguments to */
/* the appropriate type for inclusion in the returned value. */

static void
ListAnswer(SEXP x, int recurse, struct BindData *data, SEXP call)
{
  return;
}

static void
StringAnswer(SEXP x, struct BindData *data, SEXP call)
{
  return;
}

static void
LogicalAnswer(SEXP x, struct BindData *data, SEXP call)
{
  return;
}

static void
IntegerAnswer(SEXP x, struct BindData *data, SEXP call)
{
  return;
}

static void
RealAnswer(SEXP x, struct BindData *data, SEXP call)
{
  return;
}

static void
ComplexAnswer(SEXP x, struct BindData *data, SEXP call)
{
  return;
}

static void
RawAnswer(SEXP x, struct BindData *data, SEXP call)
{
  return;
}

static SEXP NewBase(SEXP base, SEXP tag)
{
  return 0;
}

static SEXP NewName(SEXP base, SEXP tag, int seqno)
{
  return 0;
}

/* also used in coerce.c */
SEXP attribute_hidden ItemName(SEXP names, R_xlen_t i)
{
  return 0;
}

/* NewExtractNames(v, base, tag, recurse):  For c() and	 unlist().
 * On entry, "base" is the naming component we have acquired by
 * recursing down from above.
 *	If we have a list and we are recursing, we append a new tag component
 * to the base tag (either by using the list tags, or their offsets),
 * and then we do the recursion.
 *	If we have a vector, we just create the tags for each element. */

struct NameData {
 int count;
 int seqno;
 int firstpos;
};


static void NewExtractNames(SEXP v, SEXP base, SEXP tag, int recurse,
			     struct BindData *data, struct NameData *nameData)
{
  return;
}

/* Code to extract the optional arguments to c().  We do it this */
/* way, rather than having an interpreted front-end do the job, */
/* because we want to avoid duplication at the top level. */
/* FIXME : is there another possibility? */

static SEXP ExtractOptionals(SEXP ans, int *recurse, int *usenames, SEXP call)
{
  return 0;
}


/* The change to lists based on dotted pairs has meant that it was
   necessary to separate the internal code for "c" and "unlist".
   Although the functions are quite similar, they operate on very
   different data structures.
*/

/* The major difference between the two functions is that the value of
   the "recursive" argument is FALSE by default for "c" and TRUE for
   "unlist".  In addition, "c" takes ... while "unlist" takes a single
   argument.
*/

/* This is a primitive SPECIALSXP */
SEXP attribute_hidden do_c(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_c_dflt(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
} /* do_c */


SEXP attribute_hidden do_unlist(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
} /* do_unlist */


/* cbind(deparse.level, ...) and rbind(deparse.level, ...) : */
/* This is a special .Internal */
SEXP attribute_hidden do_bind(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}


static void SetRowNames(SEXP dimnames, SEXP x)
{
  return;
}

static void SetColNames(SEXP dimnames, SEXP x)
{
  return;
}

/*
 * Apparently i % 0 could occur here (PR#2541).  But it should not,
 * as zero-length vectors are ignored and
 * zero-length matrices must have zero columns,
 * unless the result has zero rows, hence is of length zero and no
 * copying will be done.
 */
static SEXP cbind(SEXP call, SEXP args, SEXPTYPE mode, SEXP rho,
		  int deparse_level)
{
  return 0;
} /* cbind */


static SEXP rbind(SEXP call, SEXP args, SEXPTYPE mode, SEXP rho,
		  int deparse_level)
{
  return 0;
} /* rbind */
