/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
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
 *
 *
 *
 *  Environments:
 *
 *  All the action of associating values with symbols happens
 *  in this code.  An environment is (essentially) a list of
 *  environment "frames" of the form
 *
 *	FRAME(envir) = environment frame
 *	ENCLOS(envir) = parent environment
 *	HASHTAB(envir) = (optional) hash table
 *
 *  Each frame is a (tagged) list with
 *
 *	TAG(item) = symbol
 *	CAR(item) = value bound to symbol in this frame
 *	CDR(item) = next value on the list
 *
 *  When the value of a symbol is required, the environment is
 *  traversed frame-by-frame until a value is found.
 *
 *  If a value is not found during the traversal, the symbol's
 *  "value" slot is inspected for a value.  This "top-level"
 *  environment is where system functions and variables reside.
 *
 */

/* R 1.8.0: namespaces are no longer experimental, so the following
 *  are no longer 'experimental options':
 *
 * EXPERIMENTAL_NAMESPACES: When this is defined the variable
 *     R_BaseNamespace holds an environment that has R_GlobalEnv as
 *     its parent.  This environment does not actually contain any
 *     bindings of its own.  Instead, it redirects all fetches and
 *     assignments to the SYMVALUE fields of the base (R_BaseEnv)
 *     environment.  If evaluation occurs in R_BaseNamespace, then
 *     base is searched before R_GlobalEnv.
 *
 * ENVIRONMENT_LOCKING: Locking an environment prevents new bindings
 *     from being created and existing bindings from being removed.
 *
 * FANCY_BINDINGS: This enables binding locking and "active bindings".
 *     When a binding is locked, its value cannot be changed.  It may
 *     still be removed from the environment if the environment is not
 *     locked.
 *
 *     Active bindings contain a function in their value cell.
 *     Getting the value of an active binding calls this function with
 *     no arguments and returns the result.  Assigning to an active
 *     binding calls this function with one argument, the new value.
 *     Active bindings may be useful for mapping external variables,
 *     such as C variables or data base entries, to R variables.  They
 *     may also be useful for making some globals thread-safe.
 *
 *     Bindings are marked as locked or active using bits 14 and 15 in
 *     their gp fields.  Since the save/load code writes out this
 *     field it means the value will be preserved across save/load.
 *     But older versions of R will interpret the entire gp field as
 *     the MISSING field, which may cause confusion.  If we keep this
 *     code, then we will need to make sure that there are no
 *     locked/active bindings in workspaces written for older versions
 *     of R to read.
 *
 * LT */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>
//#include <R_ext/Callbacks.h>

#define IS_USER_DATABASE(rho)  OBJECT((rho)) && inherits((rho), "UserDefinedDatabase")

/* various definitions of macros/functions in Defn.h */

#define FRAME_LOCK_MASK (1<<14)
#define FRAME_IS_LOCKED(e) (ENVFLAGS(e) & FRAME_LOCK_MASK)
#define LOCK_FRAME(e) SET_ENVFLAGS(e, ENVFLAGS(e) | FRAME_LOCK_MASK)
/*#define UNLOCK_FRAME(e) SET_ENVFLAGS(e, ENVFLAGS(e) & (~ FRAME_LOCK_MASK))*/

/* use the same bits (15 and 14) in symbols and bindings */
#define BINDING_VALUE(b) ((IS_ACTIVE_BINDING(b) ? getActiveValue(CAR(b)) : CAR(b)))

#define SYMBOL_BINDING_VALUE(s) ((IS_ACTIVE_BINDING(s) ? getActiveValue(SYMVALUE(s)) : SYMVALUE(s)))
#define SYMBOL_HAS_BINDING(s) (IS_ACTIVE_BINDING(s) || (SYMVALUE(s) != R_UnboundValue))

#define SET_BINDING_VALUE(b,val) do { \
SEXP __b__ = (b); \
SEXP __val__ = (val); \
if (BINDING_IS_LOCKED(__b__)) \
error(_("cannot change value of locked binding for '%s'"), \
CHAR(PRINTNAME(TAG(__b__)))); \
if (IS_ACTIVE_BINDING(__b__)) \
setActiveValue(CAR(__b__), __val__); \
else \
SETCAR(__b__, __val__); \
} while (0)

#define SET_SYMBOL_BINDING_VALUE(sym, val) do { \
SEXP __sym__ = (sym); \
SEXP __val__ = (val); \
if (BINDING_IS_LOCKED(__sym__)) \
error(_("cannot change value of locked binding for '%s'"), \
CHAR(PRINTNAME(__sym__))); \
if (IS_ACTIVE_BINDING(__sym__)) \
setActiveValue(SYMVALUE(__sym__), __val__); \
else \
SET_SYMVALUE(__sym__, __val__); \
} while (0)

static void setActiveValue(SEXP fun, SEXP val)
{
    SEXP arg = LCONS(R_QuoteSymbol, LCONS(val, R_NilValue));
    SEXP expr = LCONS(fun, LCONS(arg, R_NilValue));
    PROTECT(expr);
    eval(expr, R_GlobalEnv);
    UNPROTECT(1);
}

static SEXP getActiveValue(SEXP fun)
{
    SEXP expr = LCONS(fun, R_NilValue);
    PROTECT(expr);
    expr = eval(expr, R_GlobalEnv);
    UNPROTECT(1);
    return expr;
}

/* Macro version of isNull for only the test against R_NilValue */
#define ISNULL(x) ((x) == R_NilValue)

/*----------------------------------------------------------------------
 
 Hash Tables
 
 We use a basic separate chaining algorithm.	A hash table consists
 of SEXP (vector) which contains a number of SEXPs (lists).
 
 The only non-static function is R_NewHashedEnv, which allows code to
 request a hashed environment.  All others are static to allow
 internal changes of implementation without affecting client code.
 */

#define HASHSIZE(x)	     LENGTH(x)
#define HASHPRI(x)	     TRUELENGTH(x)
#define HASHTABLEGROWTHRATE  1.2
#define HASHMINSIZE	     29
#define SET_HASHSIZE(x,v)    SETLENGTH(x,v)
#define SET_HASHPRI(x,v)     SET_TRUELENGTH(x,v)

#define IS_HASHED(x)	     (HASHTAB(x) != R_NilValue)


SEXP mkCharCE(const char *name, cetype_t enc)
{
    size_t len =  strlen(name);
    if (len > INT_MAX)
        error("R character strings are limited to 2^31-1 bytes");
    return mkCharLenCE(name, (int) len, enc);
}

/* no longer used in R but docuented in 2.7.x */
SEXP mkCharLen(const char *name, int len)
{
    return mkCharLenCE(name, len, CE_NATIVE);
}

SEXP mkChar(const char *name)
{
    size_t len =  strlen(name);
    if (len > INT_MAX)
        error("R character strings are limited to 2^31-1 bytes");
    return mkCharLenCE(name, (int) len, CE_NATIVE);
}

/* Global CHARSXP cache and code for char-based hash tables */

/* mkCharCE - make a character (CHARSXP) variable and set its
 encoding bit.  If a CHARSXP with the same string already exists in
 the global CHARSXP cache, R_StringHash, it is returned.  Otherwise,
 a new CHARSXP is created, added to the cache and then returned. */


SEXP mkCharLenCE(const char *name, int len, cetype_t enc)
{
    SEXP cval, chain;
    unsigned int hashcode;
    int need_enc;
    Rboolean embedNul = FALSE, is_ascii = TRUE;
    
    switch(enc){
        case CE_NATIVE:
        case CE_UTF8:
        case CE_LATIN1:
        case CE_BYTES:
        case CE_SYMBOL:
        case CE_ANY:
            break;
        default:
            error(_("unknown encoding: %d"), enc);
    }
    for (int slen = 0; slen < len; slen++) {
        if ((unsigned int) name[slen] > 127) is_ascii = FALSE;
        if (!name[slen]) embedNul = TRUE;
    }
    if (embedNul) {
        error(_("The prematurally terminated string: %s\n"), name);
        return NULL;
//        SEXP c;
//        /* This is tricky: we want to make a reasonable job of
//         representing this string, and EncodeString() is the most
//         comprehensive */
//        c = allocCharsxp(len);
//        memcpy(CHAR_RW(c), name, len);
//        switch(enc) {
//            case CE_UTF8: SET_UTF8(c); break;
//            case CE_LATIN1: SET_LATIN1(c); break;
//            case CE_BYTES: SET_BYTES(c); break;
//            default: break;
//        }
//        if (is_ascii) SET_ASCII(c);
//        error(_("embedded nul in string: '%s'"),
//              EncodeString(c, 0, 0, Rprt_adj_none));
    }
    
    if (enc && is_ascii) enc = CE_NATIVE;
    switch(enc) {
        case CE_UTF8: need_enc = UTF8_MASK; break;
        case CE_LATIN1: need_enc = LATIN1_MASK; break;
        case CE_BYTES: need_enc = BYTES_MASK; break;
        default: need_enc = 0;
    }
    
//    hashcode = char_hash(name, len) & char_hash_mask;
    
    /* Search for a cached value */
    cval = R_NilValue;
// Not use cached values.
//    chain = VECTOR_ELT(R_StringHash, hashcode);
//    for (; !ISNULL(chain) ; chain = CXTAIL(chain)) {
//        SEXP val = CXHEAD(chain);
//        if (TYPEOF(val) != CHARSXP) break; /* sanity check */
//        if (need_enc == (ENC_KNOWN(val) | IS_BYTES(val)) &&
//            LENGTH(val) == len &&  /* quick pretest */
//            memcmp(CHAR(val), name, len) == 0) {
//            cval = val;
//            break;
//        }
//    }
    if (cval == R_NilValue) {
        /* no cached value; need to allocate one and add to the cache */
        PROTECT(cval = allocCharsxp(len));
        memcpy(CHAR_RW(cval), name, len);
        switch(enc) {
            case 0:
                break;          /* don't set encoding */
            case CE_UTF8:
                SET_UTF8(cval);
                break;
            case CE_LATIN1:
                SET_LATIN1(cval);
                break;
            case CE_BYTES:
                SET_BYTES(cval);
                break;
            default:
                error("unknown encoding mask: %d", enc);
        }
        if (is_ascii) SET_ASCII(cval);
//        SET_CACHED(cval);  /* Mark it */
//        /* add the new value to the cache */
//        chain = VECTOR_ELT(R_StringHash, hashcode);
//        if (ISNULL(chain))
//            SET_HASHPRI(R_StringHash, HASHPRI(R_StringHash) + 1);
//        /* this is a destrictive modification */
//        chain = SET_CXTAIL(cval, chain);
//        SET_VECTOR_ELT(R_StringHash, hashcode, chain);
//        
//        /* resize the hash table if necessary with the new entry still
//         protected.
//         Maximum possible power of two is 2^30 for a VECSXP.
//         FIXME: this has changed with long vectors.
//         */
//        if (R_HashSizeCheck(R_StringHash)
//            && char_hash_size < 1073741824 /* 2^30 */)
//            R_StringHash_resize(char_hash_size * 2);
        
        UNPROTECT(1);
    }
    return cval;
}


