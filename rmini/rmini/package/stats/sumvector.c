/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1998-2012	The R Foundation
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

#include <R.h>			/* for NA_REAL, includes math.h */
#include <Rinternals.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) dgettext ("stats", String)
#else
#define _(String) (String)
#endif

SEXP sumvector(SEXP x, SEXP y)
{
    int xi = asInteger(x);
    int yi = asInteger(y);
    int zi = xi + yi;
    SEXP z = PROTECT(allocVector(INTSXP, 1));
    INTEGER(z)[0] = zi;
    UNPROTECT(1);
    return z;
}
