/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2013  The R Core Team
 *  Copyright (C) 2003, 2004  The R Foundation
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

#define __R_Names__ /* used in Defn.h for extern on R_FunTab */
#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>

#include <Print.h>
#include "arithmetic.h" /* for do_math[1234], do_cmathfuns */

#include <Rinterface.h>

/* initialize the symbol table */
void attribute_hidden InitNames()
{
//    /* allocate the symbol table */
//    if (!(R_SymbolTable = (SEXP *) calloc(HSIZE, sizeof(SEXP))))
//        R_Suicide("couldn't allocate memory for symbol table");
    
    /* R_UnboundValue */
    R_UnboundValue = allocSExp(SYMSXP);
    SET_SYMVALUE(R_UnboundValue, R_UnboundValue);
    SET_PRINTNAME(R_UnboundValue, R_NilValue);
    SET_ATTRIB(R_UnboundValue, R_NilValue);
    /* R_MissingArg */
    R_MissingArg = allocSExp(SYMSXP);
    SET_SYMVALUE(R_MissingArg, R_MissingArg);
    SET_PRINTNAME(R_MissingArg, mkChar(""));
    SET_ATTRIB(R_MissingArg, R_NilValue);
    /* R_RestartToken */
    R_RestartToken = allocSExp(SYMSXP);
    SET_SYMVALUE(R_RestartToken, R_RestartToken);
    SET_PRINTNAME(R_RestartToken, mkChar(""));
    SET_ATTRIB(R_RestartToken, R_NilValue);
    /* String constants (CHARSXP values) */
    /* Note: we don't want NA_STRING to be in the CHARSXP cache, so that
     mkChar("NA") is distinct from NA_STRING */
    /* NA_STRING */
    NA_STRING = allocCharsxp(strlen("NA"));
    strcpy(CHAR_RW(NA_STRING), "NA");
    SET_CACHED(NA_STRING);  /* Mark it */
//    R_print.na_string = NA_STRING;
    /* R_BlankString */
    R_BlankString = mkChar("");
//    /* Initialize the symbol Table */
//    for (int i = 0; i < HSIZE; i++) R_SymbolTable[i] = R_NilValue;
//    /* Set up a set of globals so that a symbol table search can be
//     avoided when matching something like dim or dimnames. */
//    SymbolShortcuts();
//    /*  Builtin Functions */
//    for (int i = 0; R_FunTab[i].name; i++) installFunTab(i);
//    
//    R_initialize_bcode();
}

void attribute_hidden FinalizeNames()
{
    freeVector(R_UnboundValue);
    freeVector(R_MissingArg);
    freeVector(R_RestartToken);
    freeVector(NA_STRING);
    freeVector(R_BlankString);
}

