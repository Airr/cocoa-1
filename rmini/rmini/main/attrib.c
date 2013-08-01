/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2013  The R Core Team
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

#include <Defn.h>
#include <Internal.h>
#include <Rmath.h>

static SEXP installAttrib(SEXP, SEXP, SEXP);
static SEXP removeAttrib(SEXP, SEXP);

SEXP comment(SEXP);
static SEXP commentgets(SEXP, SEXP);

static SEXP row_names_gets(SEXP vec , SEXP val)
{
  return 0;
}

/* used in removeAttrib, commentgets and classgets */
static SEXP stripAttrib(SEXP tag, SEXP lst)
{
  return 0;
}

/* NOTE: For environments serialize.c calls this function to find if
   there is a class attribute in order to reconstruct the object bit
   if needed.  This means the function cannot use OBJECT(vec) == 0 to
   conclude that the class attribute is R_NilValue.  If you want to
   rewrite this function to use such a pre-test, be sure to adjust
   serialize.c accordingly.  LT */
SEXP attribute_hidden getAttrib0(SEXP vec, SEXP name)
{
  return 0;
}

SEXP getAttrib(SEXP vec, SEXP name)
{
  return 0;
}

attribute_hidden
SEXP do_shortRowNames(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* This is allowed to change 'out' */
attribute_hidden
SEXP do_copyDFattr(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}


/* 'name' should be 1-element STRSXP or SYMSXP */
SEXP setAttrib(SEXP vec, SEXP name, SEXP val)
{
  return 0;
}

/* This is called in the case of binary operations to copy */
/* most attributes from (one of) the input arguments to */
/* the output.	Note that the Dim and Names attributes */
/* should have been assigned elsewhere. */

void copyMostAttrib(SEXP inp, SEXP ans)
{
  return;
}

/* version that does not preserve ts information, for subsetting */
void copyMostAttribNoTs(SEXP inp, SEXP ans)
{
  return;
}

/* Tweaks here based in part on PR#14934 */
static SEXP installAttrib(SEXP vec, SEXP name, SEXP val)
{
  return 0;
}

static SEXP removeAttrib(SEXP vec, SEXP name)
{
  return 0;
}

static void checkNames(SEXP x, SEXP s)
{
  return;
}


/* Time Series Parameters */

static void badtsp(void)
{
  return;
}

attribute_hidden
SEXP tspgets(SEXP vec, SEXP val)
{
  return 0;
}

static SEXP commentgets(SEXP vec, SEXP comment)
{
  return 0;
}

SEXP attribute_hidden do_commentgets(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_comment(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP classgets(SEXP vec, SEXP klass)
{
  return 0;
}

/* oldClass<-(), primitive */
SEXP attribute_hidden do_classgets(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* oldClass, primitive */
SEXP attribute_hidden do_class(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* character elements corresponding to the syntactic types in the
   grammar */
static SEXP lang2str(SEXP obj, SEXPTYPE t)
{
  return 0;
}

/* the S4-style class: for dispatch required to be a single string;
   for the new class() function;
   if(!singleString) , keeps S3-style multiple classes.
   Called from the methods package, so exposed.
 */
SEXP R_data_class(SEXP obj, Rboolean singleString)
{
  return 0;
}

static SEXP s_dot_S3Class = 0;

static SEXP R_S4_extends_table = 0;

 
static SEXP cache_class(const char *class, SEXP klass) {
  return 0;
}

static SEXP S4_extends(SEXP klass) {
  return 0;
}

/* Version for S3-dispatch */
SEXP attribute_hidden R_data_class2 (SEXP obj)
{
  return 0;
}

/* class() : */
SEXP attribute_hidden R_do_data_class(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* names(object) <- name */
SEXP attribute_hidden do_namesgets(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP namesgets(SEXP vec, SEXP val)
{
  return 0;
}

SEXP attribute_hidden do_names(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_dimnamesgets(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

static SEXP dimnamesgets1(SEXP val1)
{
  return 0;
}


SEXP dimnamesgets(SEXP vec, SEXP val)
{
  return 0;
}

SEXP attribute_hidden do_dimnames(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_dim(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_dimgets(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP dimgets(SEXP vec, SEXP val)
{
  return 0;
}

SEXP attribute_hidden do_attributes(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_levelsgets(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* attributes(object) <- attrs */
SEXP attribute_hidden do_attributesgets(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/*  This code replaces an R function defined as

    attr <- function (x, which)
    {
SEXP attribute_hidden do_attr(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

static void check_slot_assign(SEXP obj, SEXP input, SEXP value, SEXP env) 
{
  return;
}


SEXP attribute_hidden do_attrgets(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}


/* These provide useful shortcuts which give access to */
/* the dimnames for matrices and arrays in a standard form. */

void GetMatrixDimnames(SEXP x, SEXP *rl, SEXP *cl,
		       const char **rn, const char **cn)
{
  return;
}


SEXP GetArrayDimnames(SEXP x)
{
  return 0;
}


/* the code to manage slots in formal classes. These are attributes,
   but without partial matching and enforcing legal slot names (it's
   an error to get a slot that doesn't exist. */


static SEXP pseudo_NULL = 0;

static SEXP s_dot_Data;
static SEXP s_getDataPart;
static SEXP s_setDataPart;

static void init_slot_handling(void) {
  return;
}

static SEXP data_part(SEXP obj) {
  return 0;
}

static SEXP set_data_part(SEXP obj,  SEXP rhs) {
  return 0;
}

SEXP S3Class(SEXP obj)
{
  return 0;
}

/* Slots are stored as attributes to
   provide some back-compatibility
*/

/**
 * R_has_slot() : a C-level test if a obj@<name> is available;
 *                as R_do_slot() gives an error when there's no such slot.
 */
int R_has_slot(SEXP obj, SEXP name) {
#define R_SLOT_INIT							\
    if(!(isSymbol(name) || (isString(name) && LENGTH(name) == 1)))	\
	error(_("invalid type or length for slot name"));		\
    if(!s_dot_Data)							\
	init_slot_handling();						\
    if(isString(name)) name = install(CHAR(STRING_ELT(name, 0)))

    R_SLOT_INIT;
    if(name == s_dot_Data && TYPEOF(obj) != S4SXP)
	return(1);
    /* else */
    return(getAttrib(obj, name) != R_NilValue);
}

/* the @ operator, and its assignment form.  Processed much like $
   (see do_subset3) but without S3-style methods.
*/
/* currently, R_get_slot() ["methods"] is a trivial wrapper for this: */
SEXP R_do_slot(SEXP obj, SEXP name) {
  return 0;
}
#undef R_SLOT_INIT

/* currently, R_set_slot() ["methods"] is a trivial wrapper for this: */
SEXP R_do_slot_assign(SEXP obj, SEXP name, SEXP value) {
  return 0;
}

SEXP attribute_hidden do_AT(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* Return a suitable S3 object (OK, the name of the routine comes from
   an earlier version and isn't quite accurate.) If there is a .S3Class
   slot convert to that S3 class.
   Otherwise, unless type == S4SXP, look for a .Data or .xData slot.  The
   value of type controls what's wanted.  If it is S4SXP, then ONLY
   .S3class is used.  If it is ANYSXP, don't check except that automatic
   conversion from the current type only applies for classes that extend
   one of the basic types (i.e., not S4SXP).  For all other types, the
   recovered data must match the type.
   Because S3 objects can't have type S4SXP, .S3Class slot is not searched
   for in that type object, unless ONLY that class is wanted.
   (Obviously, this is another routine that has accumulated barnacles and
   should at some time be broken into separate parts.)
*/
SEXP attribute_hidden
R_getS4DataSlot(SEXP obj, SEXPTYPE type)
{
  return 0;
}
