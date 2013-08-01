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

const static char * const truenames[] = {
    "T",
    "True",
    "TRUE",
    "true",
    (char *) NULL,
};

const static char * const falsenames[] = {
    "F",
    "False",
    "FALSE",
    "false",
    (char *) NULL,
};





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


#include <wctype.h>

/* This one is not in Rinternals.h, but is used in internet module */
Rboolean isBlankString(const char *s)
{
    if(mbcslocale) {
        wchar_t wc; size_t used; mbstate_t mb_st;
        mbs_init(&mb_st);
        while( (used = Mbrtowc(&wc, s, MB_CUR_MAX, &mb_st)) ) {
            if(!iswspace((wint_t) wc)) return FALSE;
            s += used;
        }
    } else
        while (*s)
            if (!isspace((int)*s++)) return FALSE;
    return TRUE;
}

Rboolean StringBlank(SEXP x)
{
    if (x == R_NilValue) return TRUE;
    else return CHAR(x)[0] == '\0';
}

/* Function to test whether a string is a true value */

Rboolean StringTrue(const char *name)
{
    int i;
    for (i = 0; truenames[i]; i++)
        if (!strcmp(name, truenames[i]))
            return TRUE;
    return FALSE;
}

Rboolean StringFalse(const char *name)
{
    int i;
    for (i = 0; falsenames[i]; i++)
        if (!strcmp(name, falsenames[i]))
            return TRUE;
    return FALSE;
}
