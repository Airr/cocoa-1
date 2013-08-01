/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1999--2012  The R Core Team
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


/* =========
 * Printing:
 * =========
 *
 * All printing in R is done via the functions Rprintf and REprintf
 * or their (v) versions Rvprintf and REvprintf.
 * These routines work exactly like (v)printf(3).  Rprintf writes to
 * ``standard output''.	 It is redirected by the sink() function,
 * and is suitable for ordinary output.	 REprintf writes to
 * ``standard error'' and is useful for error messages and warnings.
 * It is not redirected by sink().
 *
 *  See ./format.c  for the  format_FOO_  functions which provide
 *	~~~~~~~~~~  the	 length, width, etc.. that are used here.
 *  See ./print.c  for do_printdefault, do_prmatrix, etc.
 *
 *
 * Here, the following UTILITIES are provided:
 *
 * The utilities EncodeLogical, EncodeInteger, EncodeReal
 * and EncodeString can be used to convert R objects to a form suitable
 * for printing.  These print the values passed in a formatted form
 * or, in the case of NA values, an NA indicator.  EncodeString takes
 * care of printing all the standard ANSI escapes \a, \t \n etc.
 * so that these appear in their backslash form in the string.	There
 * is also a routine called Rstrlen which computes the length of the
 * string in its escaped rather than literal form.
 *
 * Finally there is a routine called EncodeElement which will encode
 * a single R-vector element.  This is used in deparse and write.table.
 */

/* if ESC_BARE_QUOTE is defined, " in an unquoted string is replaced
 by \".  " in a quoted string is always replaced by \". */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Rmath.h>
#include <Print.h>
#include <R_ext/RS.h>
#include <Rconnections.h>

#include "RBufferUtils.h"


#if !defined(__STDC_ISO_10646__) && (defined(__APPLE__) || defined(__FreeBSD__))
/* This may not be 100% true (see the comment in rlocales.h),
 but it seems true in normal locales */
# define __STDC_ISO_10646__
#endif

#ifdef Win32
int trio_vsnprintf(char *buffer, size_t bufferSize, const char *format,
                   va_list args);
# define vsnprintf trio_vsnprintf
#endif

extern int R_OutputCon; /* from connections.c */

#ifndef min
#define min(a, b) (((a)<(b))?(a):(b))
#endif

#define BUFSIZE 8192  /* used by Rprintf etc */

/* Only if ierr < 0 or not is currently used */
attribute_hidden
R_size_t R_Decode2Long(char *p, int *ierr)
{
    return 0;
}

/* There is no documented (or enforced) limit on 'w' here,
 so use snprintf */
#define NB 1000
const char *EncodeLogical(int x, int w)
{
    return 0;
}

const char *EncodeInteger(int x, int w)
{
    return 0;
}

attribute_hidden
const char *EncodeRaw(Rbyte x)
{
    return 0;
}

attribute_hidden
const char *EncodeEnvironment(SEXP x)
{
    return 0;
}

const char *EncodeReal(double x, int w, int d, int e, char cdec)
{
    return 0;
}

attribute_hidden
const char *EncodeReal2(double x, int w, int d, int e)
{
    return 0;
}

void z_prec_r(Rcomplex *r, Rcomplex *x, double digits);

const char
*EncodeComplex(Rcomplex x, int wr, int dr, int er, int wi, int di, int ei,
               char cdec)
{
    return 0;
}

/* <FIXME>
 encodeString and Rstrwid assume that the wchar_t representation
 used to hold multibyte chars is Unicode.  This is usually true, and
 we warn if it is not known to be true.  Potentially looking at
 wchar_t ranges as we do is incorrect, but that is even less likely to
 be problematic.
 
 On Windows with surrogate pairs it will not be canonical, but AFAIK
 they do not occur in any MBCS (so it would only matter if we implement
 UTF-8, and then only if Windows has surrogate pairs switched on,
 which Western versions at least do not.).
 */

#include <rlocale.h> /* redefines isw* functions */

#ifdef Win32
#include "rgui_UTF8.h"
#endif

/* strlen() using escaped rather than literal form,
 and allowing for embedded nuls.
 In MBCS locales it works in characters, and reports in display width.
 Also used in printarray.c.
 */
attribute_hidden
int Rstrwid(const char *str, int slen, cetype_t ienc, int quote)
{
    return 0;
}

attribute_hidden
int Rstrlen(SEXP s, int quote)
{
    return 0;
}

/* Here w is the minimum field width
 If 'quote' is non-zero the result should be quoted (and internal quotes
 escaped and NA strings handled differently).
 
 EncodeString is called from EncodeElements, cat() (for labels when
 filling), to (auto)print character vectors, arrays, names and
 CHARSXPs.  It is also called by do_encodeString, but not from
 format().
 */

attribute_hidden
const char *EncodeString(SEXP s, int w, int quote, Rprt_adj justify)
{
    return 0;
}

/* EncodeElement is called by cat(), write.table() and deparsing. */

/* NB this is called by R.app even though it is in no public header, so
 alter there if you alter this */
const char *EncodeElement(SEXP x, int indx, int quote, char dec)
{
    return 0;
}

void Rprintf(const char *format, ...)
{
    return;
}

/*
 REprintf is used by the error handler do not add
 anything unless you're sure it won't
 cause problems
 */
void REprintf(const char *format, ...)
{
    return;
}

#if defined(HAVE_VASPRINTF) && !HAVE_DECL_VASPRINTF
int vasprintf(char **strp, const char *fmt, va_list ap)
#ifdef __cplusplus
throw ()
#endif
       ;
#endif

# define R_BUFSIZE BUFSIZE
attribute_hidden
void Rcons_vprintf(const char *format, va_list arg)
{
    return;
}

void Rvprintf(const char *format, va_list arg)
{
    return;
}

/*
 REvprintf is part of the error handler.
 Do not change it unless you are SURE that
 your changes are compatible with the
 error handling mechanism.
 
 It is also used in R_Suicide on Unix.
 */

void REvprintf(const char *format, va_list arg)
{
    return;
}

int attribute_hidden IndexWidth(R_xlen_t n)
{
    return 0;
}

void attribute_hidden VectorIndex(R_xlen_t i, int w)
{
    return;
}
