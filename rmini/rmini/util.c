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

#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>
#include <R_ext/Print.h>
#include <ctype.h>		/* for isspace */

#undef COMPILING_R

#define R_imax2(x, y) ((x < y) ? y : x)
#include <Print.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef Win32
void R_UTF8fixslash(char *s);
static void R_wfixslash(wchar_t *s);
#endif

#ifdef __cplusplus
#include "Clinkage.h"

extern "C" {
#endif
    void F77_SYMBOL(rwarnc)(char *msg, int *nchar);
    void F77_SYMBOL(rexitc)(char *msg, int *nchar);
    
#ifdef __cplusplus
}
#endif

// Various utility functions.

const static struct {
    const char * const str;
    const int type;
}
TypeTable[] = {
    { "NULL",		NILSXP	   },  /* real types */
    { "symbol",		SYMSXP	   },
    { "pairlist",	LISTSXP	   },
    { "closure",	CLOSXP	   },
    { "environment",	ENVSXP	   },
    { "promise",	PROMSXP	   },
    { "language",	LANGSXP	   },
    { "special",	SPECIALSXP },
    { "builtin",	BUILTINSXP },
    { "char",		CHARSXP	   },
    { "logical",	LGLSXP	   },
    { "integer",	INTSXP	   },
    { "double",		REALSXP	   }, /*-  "real", for R <= 0.61.x */
    { "complex",	CPLXSXP	   },
    { "character",	STRSXP	   },
    { "...",		DOTSXP	   },
    { "any",		ANYSXP	   },
    { "expression",	EXPRSXP	   },
    { "list",		VECSXP	   },
    { "externalptr",	EXTPTRSXP  },
    { "bytecode",	BCODESXP   },
    { "weakref",	WEAKREFSXP },
    { "raw",		RAWSXP },
    { "S4",		S4SXP },
    /* aliases : */
    { "numeric",	REALSXP	   },
    { "name",		SYMSXP	   },
    
    { (char *)NULL,	-1	   }
};


SEXPTYPE str2type(const char *s)
{
    int i;
    for (i = 0; TypeTable[i].str; i++) {
        if (!strcmp(s, TypeTable[i].str))
            return (SEXPTYPE) TypeTable[i].type;
    }
    /* SEXPTYPE is an unsigned int, so the compiler warns us w/o the cast. */
    return (SEXPTYPE) -1;
}


SEXP type2str(SEXPTYPE t)
{
    int i;
    
    for (i = 0; TypeTable[i].str; i++) {
        if (TypeTable[i].type == t)
            return mkChar(TypeTable[i].str);
    }
    error(_("type %d is unimplemented in '%s'"), t, "type2str");
    return R_NilValue; /* for -Wall */
}

const char *type2char(SEXPTYPE t)
{
    int i;
    
    for (i = 0; TypeTable[i].str; i++) {
        if (TypeTable[i].type == t)
            return TypeTable[i].str;
    }
    error(_("type %d is unimplemented in '%s'"), t, "type2char");
    return ""; /* for -Wall */
}

#ifdef UNUSED
SEXP type2symbol(SEXPTYPE t)
{
    int i;
    /* for efficiency, a hash table set up to index TypeTable, and
     with TypeTable pointing to both the
     character string and to the symbol would be better */
    for (i = 0; TypeTable[i].str; i++) {
        if (TypeTable[i].type == t)
            return install((const char *)&TypeTable[i].str);
    }
    error(_("type %d is unimplemented in '%s'"), t, "type2symbol");
    return R_NilValue; /* for -Wall */
}
#endif

attribute_hidden
void UNIMPLEMENTED_TYPEt(const char *s, SEXPTYPE t)
{
    int i;
    
    for (i = 0; TypeTable[i].str; i++) {
        if (TypeTable[i].type == t)
            error(_("unimplemented type '%s' in '%s'\n"), TypeTable[i].str, s);
    }
    error(_("unimplemented type (%d) in '%s'\n"), t, s);
}

void UNIMPLEMENTED_TYPE(const char *s, SEXP x)
{
    UNIMPLEMENTED_TYPEt(s, TYPEOF(x));
}
