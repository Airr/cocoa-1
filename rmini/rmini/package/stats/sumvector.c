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
    int krn = asInteger(skrn);
    double bw = asReal(sbw);
    
    
    x = PROTECT(coerceVector(x, REALSXP));
    y = PROTECT(coerceVector(y, REALSXP));
    xp = PROTECT(coerceVector(xp, REALSXP));
    R_xlen_t nx = XLENGTH(x), np = XLENGTH(xp);
    SEXP yp = PROTECT(allocVector(REALSXP, np));
    
    BDRksmooth(REAL(x), REAL(y), nx, REAL(xp), REAL(yp), np, krn, bw);
    SEXP ans = PROTECT(allocVector(VECSXP, 2));
    SET_VECTOR_ELT(ans, 0, xp);
    SET_VECTOR_ELT(ans, 1, yp);
    SEXP nm = allocVector(STRSXP, 2);
    setAttrib(ans, R_NamesSymbol, nm);
    SET_STRING_ELT(nm, 0, mkChar("x"));
    SET_STRING_ELT(nm, 1, mkChar("y"));
    UNPROTECT(5);
    return ans;
}
