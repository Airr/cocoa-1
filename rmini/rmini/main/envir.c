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
#include <R_ext/Callbacks.h>

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
    return;
}

static SEXP getActiveValue(SEXP fun)
{
    return 0;
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

/*----------------------------------------------------------------------
 
 String Hashing
 
 This is taken from the second edition of the "Dragon Book" by
 Aho, Ullman and Sethi.
 
 */

/* was extern: used in this file and names.c (for the symbol table).
 
 This hash function seems to work well enough for symbol tables,
 and hash tables get saved as part of environments so changing it
 is a major decision.
 */
int attribute_hidden R_Newhashpjw(const char *s)
{
    return 0;
}

/*----------------------------------------------------------------------
 
 R_HashSet
 
 Hashtable set function.  Sets 'symbol' in 'table' to be 'value'.
 'hashcode' must be provided by user.	Allocates some memory for list
 entries.
 
 */

static void R_HashSet(int hashcode, SEXP symbol, SEXP table, SEXP value,
                      Rboolean frame_locked)
{
    return;
}



/*----------------------------------------------------------------------
 
 R_HashGet
 
 Hashtable get function.  Returns 'value' from 'table' indexed by
 'symbol'.  'hashcode' must be provided by user.  Returns
 'R_UnboundValue' if value is not present.
 
 */

static SEXP R_HashGet(int hashcode, SEXP symbol, SEXP table)
{
    return 0;
}

static Rboolean R_HashExists(int hashcode, SEXP symbol, SEXP table)
{
    return 0;
}



/*----------------------------------------------------------------------
 
 R_HashGetLoc
 
 Hashtable get location function. Just like R_HashGet, but returns
 location of variable, rather than its value. Returns R_NilValue
 if not found.
 
 */

static SEXP R_HashGetLoc(int hashcode, SEXP symbol, SEXP table)
{
    return 0;
}



/*----------------------------------------------------------------------
 
 R_NewHashTable
 
 Hash table initialisation function.  Creates a table of size 'size'
 that increases in size by 'growth_rate' after a threshold is met.
 
 */

static SEXP R_NewHashTable(int size)
{
    return 0;
}

/*----------------------------------------------------------------------
 
 R_NewHashedEnv
 
 Returns a new environment with a hash table initialized with default
 size.  The only non-static hash table function.
 */

SEXP R_NewHashedEnv(SEXP enclos, SEXP size)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 R_HashDelete
 
 Hash table delete function.  Symbols are not removed from the table.
 They have their value set to 'R_UnboundValue'.
 
 */

static SEXP DeleteItem(SEXP symbol, SEXP lst)
{
    return 0;
}

static void R_HashDelete(int hashcode, SEXP symbol, SEXP table)
{
    return;
}



/*----------------------------------------------------------------------
 
 R_HashResize
 
 Hash table resizing function Increase the size of the hash table by
 the growth_rate of the table.	 The vector is reallocated, however
 the lists with in the hash table have their pointers shuffled around
 so that they are not reallocated.
 
 */

static SEXP R_HashResize(SEXP table)
{
    return 0;
} /* end R_HashResize */



/*----------------------------------------------------------------------
 
 R_HashSizeCheck
 
 Hash table size rechecking function.	Compares the load factor
 (size/# of primary slots used)  to a particular threshhold value.
 Returns true if the table needs to be resized.
 
 */

static int R_HashSizeCheck(SEXP table)
{
    return 0;
}



/*----------------------------------------------------------------------
 
 R_HashFrame
 
 Hashing for environment frames.  This function ensures that the
 first frame in the given environment has been hashed.	 Ultimately
 all enironments should be created in hashed form.  At that point
 this function will be redundant.
 
 */

static SEXP R_HashFrame(SEXP rho)
{
    return 0;
}


/* ---------------------------------------------------------------------
 
 R_HashProfile
 
 Profiling tool for analyzing hash table performance.  Returns a
 three element list with components:
 
 size: the total size of the hash table
 
 nchains: the number of non-null chains in the table (as reported by
 HASHPRI())
 
 counts: an integer vector the same length as size giving the length of
 each chain (or zero if no chain is present).  This allows
 for assessing collisions in the hash table.
 */

static SEXP R_HashProfile(SEXP table)
{
    return 0;
}



/*----------------------------------------------------------------------
 
 Environments
 
 The following code implements variable searching for environments.
 
 */


/*----------------------------------------------------------------------
 
 InitGlobalEnv
 
 Create the initial global environment.  The global environment is
 no longer a linked list of environment frames.  Instead it is a
 vector of environments which is searched from beginning to end.
 
 Note that only the first frame of each of these environments is
 searched.  This is intended to make it possible to implement
 namespaces at some (indeterminate) point in the future.
 
 We hash the initial environment.  100 is a magic number discovered
 by Ross.  Change it if you feel inclined.
 
 */

#define USE_GLOBAL_CACHE
#ifdef USE_GLOBAL_CACHE  /* NB leave in place: see below */
/* Global variable caching.  A cache is maintained in a hash table,
 R_GlobalCache.  The entry values are either R_UnboundValue (a
 flushed cache entry), the binding LISTSXP cell from the environment
 containing the binding found in a search from R_GlobalEnv, or a
 symbol if the globally visible binding lives in the base package.
 The cache for a variable is flushed if a new binding for it is
 created in a global frame or if the variable is removed from any
 global frame.
 
 To make sure the cache is valid, all binding creations and removals
 from global frames must go through the interface functions in this
 file.
 
 Initially only the R_GlobalEnv frame is a global frame.  Additional
 global frames can only be created by attach.  All other frames are
 considered local.  Whether a frame is local or not is recorded in
 the highest order bit of the ENVFLAGS field (the gp field of
 sxpinfo).
 
 It is possible that the benefit of caching may be significantly
 reduced if we introduce namespace management.  Since maintaining
 cache integrity is a bit tricky and since it might complicate
 threading a bit (I'm not sure it will but it needs to be thought
 through if nothing else) it might make sense to remove caching at
 that time.  To make that easier, the ifdef's should probably be
 left in place.
 
 L. T. */

#define GLOBAL_FRAME_MASK (1<<15)
#define IS_GLOBAL_FRAME(e) (ENVFLAGS(e) & GLOBAL_FRAME_MASK)
#define MARK_AS_GLOBAL_FRAME(e) \
SET_ENVFLAGS(e, ENVFLAGS(e) | GLOBAL_FRAME_MASK)
#define MARK_AS_LOCAL_FRAME(e) \
SET_ENVFLAGS(e, ENVFLAGS(e) & (~ GLOBAL_FRAME_MASK))

#define INITIAL_CACHE_SIZE 1000

static SEXP R_GlobalCache, R_GlobalCachePreserve;
#endif
static SEXP R_BaseNamespaceName;

void attribute_hidden InitBaseEnv()
{
    return;
}

void attribute_hidden InitGlobalEnv()
{
    return;
}

#ifdef USE_GLOBAL_CACHE
static int hashIndex(SEXP symbol, SEXP table)
{
    return 0;
}

static void R_FlushGlobalCache(SEXP sym)
{
    return;
}

static void R_FlushGlobalCacheFromTable(SEXP table)
{
    return;
}

/**
 Flush the cache based on the names provided by the user defined
 table, specifically returned from calling objects() for that
 table.
 */
static void R_FlushGlobalCacheFromUserTable(SEXP udb)
{
    return;
}

static void R_AddGlobalCache(SEXP symbol, SEXP place)
{
    return;
}

static SEXP R_GetGlobalCache(SEXP symbol)
{
    return 0;
}
#endif /* USE_GLOBAL_CACHE */

/*----------------------------------------------------------------------
 
 unbindVar
 
 Remove a value from an environment. This happens only in the frame
 of the specified environment.
 
 FIXME ? should this also unbind the symbol value slot when rho is
 R_BaseEnv.
 This is only called from eval.c in applydefine and bcEval
 (and applydefine only works for unhashed environments, so not base).
 */

static SEXP RemoveFromList(SEXP thing, SEXP list, int *found)
{
    return 0;
}

void attribute_hidden unbindVar(SEXP symbol, SEXP rho)
{
    return;
}



/*----------------------------------------------------------------------
 
 findVarLocInFrame
 
 Look up the location of the value of a symbol in a
 single environment frame.  Almost like findVarInFrame, but
 does not return the value. R_NilValue if not found.
 
 Callers set *canCache = TRUE or NULL
 */

static SEXP findVarLocInFrame(SEXP rho, SEXP symbol, Rboolean *canCache)
{
    return 0;
}


/*
 External version and accessor functions. Returned value is cast as
 an opaque pointer to insure it is only used by routines in this
 group.  This allows the implementation to be changed without needing
 to change other files.
 */

R_varloc_t R_findVarLocInFrame(SEXP rho, SEXP symbol)
{
    return 0;
}

attribute_hidden
SEXP R_GetVarLocValue(R_varloc_t vl)
{
    return 0;
}

attribute_hidden
SEXP R_GetVarLocSymbol(R_varloc_t vl)
{
    return 0;
}

/* used in methods */
Rboolean R_GetVarLocMISSING(R_varloc_t vl)
{
    return 0;
}

attribute_hidden
void R_SetVarLocValue(R_varloc_t vl, SEXP value)
{
    return;
}


/*----------------------------------------------------------------------
 
 findVarInFrame
 
 Look up the value of a symbol in a single environment frame.	This
 is the basic building block of all variable lookups.
 
 It is important that this be as efficient as possible.
 
 The final argument is usually TRUE and indicates whether the
 lookup is being done in order to get the value (TRUE) or
 simply to check whether there is a value bound to the specified
 symbol in this frame (FALSE).  This is used for get() and exists().
 */

SEXP findVarInFrame3(SEXP rho, SEXP symbol, Rboolean doGet)
{
    return 0;
}

/* This variant of findVarinFrame3 is needed to avoid running active
 binding functions in calls to exists() with mode = "any" */
static Rboolean existsVarInFrame(SEXP rho, SEXP symbol)
{
    return 0;
}

SEXP findVarInFrame(SEXP rho, SEXP symbol)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 findVar
 
 Look up a symbol in an environment.
 
 */

#ifdef USE_GLOBAL_CACHE
/* findGlobalVar searches for a symbol value starting at R_GlobalEnv,
 so the cache can be used. */
static SEXP findGlobalVar(SEXP symbol)
{
    return 0;
}
#endif

SEXP findVar(SEXP symbol, SEXP rho)
{
    return 0;
}



/*----------------------------------------------------------------------
 
 findVar1
 
 Look up a symbol in an environment.  Ignore any values which are
 not of the specified type.
 
 */

SEXP attribute_hidden
findVar1(SEXP symbol, SEXP rho, SEXPTYPE mode, int inherits)
{
    return 0;
}

/*
 *  ditto, but check *mode* not *type*
 */

static SEXP
findVar1mode(SEXP symbol, SEXP rho, SEXPTYPE mode, int inherits,
             Rboolean doGet)
{
    return 0;
}


/*
 ddVal:
 a function to take a name and determine if it is of the form
 ..x where x is an integer; if so x is returned otherwise 0 is returned
 */
static int ddVal(SEXP symbol)
{
    return 0;
}

/*----------------------------------------------------------------------
 ddfindVar
 
 This function fetches the variables ..1, ..2, etc from the first
 frame of the environment passed as the second argument to ddfindVar.
 These variables are implicitly defined whenever a ... object is
 created.
 
 To determine values for the variables we first search for an
 explicit definition of the symbol, them we look for a ... object in
 the frame and then walk through it to find the appropriate values.
 
 If no value is obtained we return R_UnboundValue.
 
 It is an error to specify a .. index longer than the length of the
 ... object the value is sought in.
 
 */

attribute_hidden
SEXP ddfindVar(SEXP symbol, SEXP rho)
{
    return 0;
}



/*----------------------------------------------------------------------
 
 dynamicfindVar
 
 This function does a variable lookup, but uses dynamic scoping rules
 rather than the lexical scoping rules used in findVar.
 
 Return R_UnboundValue if the symbol isn't located and the calling
 function needs to handle the errors.
 
 */

#ifdef UNUSED
SEXP dynamicfindVar(SEXP symbol, RCNTXT *cptr)
{
    return 0;
}
#endif



/*----------------------------------------------------------------------
 
 findFun
 
 Search for a function in an environment This is a specially modified
 version of findVar which ignores values its finds if they are not
 functions.
 
 [ NEEDED: This needs to be modified so that a search for an arbitrary mode can
 be made.  Then findVar and findFun could become same function.]
 
 This could call findVar1.  NB: they behave differently on failure.
 */

SEXP findFun(SEXP symbol, SEXP rho)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 defineVar
 
 Assign a value in a specific environment frame.
 
 */

void defineVar(SEXP symbol, SEXP value, SEXP rho)
{
    return;
}

/*----------------------------------------------------------------------
 
 setVarInFrame
 
 Assign a new value to an existing symbol in a frame.
 Return the symbol if successful and R_NilValue if not.
 
 [ Taken static in 2.4.0: not called for emptyenv or baseenv. ]
 */

static SEXP setVarInFrame(SEXP rho, SEXP symbol, SEXP value)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 setVar
 
 Assign a new value to bound symbol.	 Note this does the "inherits"
 case.  I.e. it searches frame-by-frame for a symbol and binds the
 given value to the first symbol encountered.  If no symbol is
 found then a binding is created in the global environment.
 
 Changed in R 2.4.0 to look in the base environment (previously the
 search stopped befor the base environment, but would (and still
 does) assign into the base namespace if that is on the search and
 the symbol existed there).
 
 */

void setVar(SEXP symbol, SEXP value, SEXP rho)
{
    return;
}



/*----------------------------------------------------------------------
 
 gsetVar
 
 Assignment in the base environment. Here we assign directly into
 the base environment.
 
 */

void gsetVar(SEXP symbol, SEXP value, SEXP rho)
{
    return;
}

/* get environment from a subclass if possible; else return NULL */
#define simple_as_environment(arg) (IS_S4_OBJECT(arg) && (TYPEOF(arg) == S4SXP) ? R_getS4DataSlot(arg, ENVSXP) : R_NilValue)



/*----------------------------------------------------------------------
 
 do_assign : .Internal(assign(x, value, envir, inherits))
 
 */
SEXP attribute_hidden do_assign(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/**
 * do_list2env : .Internal(list2env(x, envir))
 */
SEXP attribute_hidden do_list2env(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 do_remove
 
 There are three arguments to do_remove; a list of names to remove,
 an optional environment (if missing set it to R_GlobalEnv) and
 inherits, a logical indicating whether to look in the parent env if
 a symbol is not found in the supplied env.  This is ignored if
 environment is not specified.
 
 */

static int RemoveVariable(SEXP name, int hashcode, SEXP env)
{
    return 0;
}

SEXP attribute_hidden do_remove(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 do_get
 
 This function returns the SEXP associated with the character
 argument.  It needs the environment of the calling function as a
 default.
 
 get(x, envir, mode, inherits)
 exists(x, envir, mode, inherits)
 
 */


SEXP attribute_hidden do_get(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

static SEXP gfind(const char *name, SEXP env, SEXPTYPE mode,
                  SEXP ifnotfound, int inherits, SEXP enclos)
{
    return 0;
}


/** mget(): get multiple values from an environment
 *
 * .Internal(mget(x, envir, mode, ifnotfound, inherits))
 *
 * @return  a list of the same length as x, a character vector (of names).
 */
SEXP attribute_hidden do_mget(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/*----------------------------------------------------------------------
 
 do_missing
 
 This function tests whether the symbol passed as its first argument
 is a missing argument to the current closure.  rho is the
 environment that missing was called from.
 
 R_isMissing is called on the not-yet-evaluated value of an argument,
 if this is a symbol, as it could be a missing argument that has been
 passed down.  So 'symbol' is the promise value, and 'rho' its
 evaluation argument.
 
 It is also called in arithmetic.c. for e.g. do_log
 */

int attribute_hidden
R_isMissing(SEXP symbol, SEXP rho)
{
    return 0;
}

/* this is primitive and a SPECIALSXP */
SEXP attribute_hidden do_missing(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/*----------------------------------------------------------------------
 
 do_globalenv
 
 Returns the current global environment.
 
 */


SEXP attribute_hidden do_globalenv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/*----------------------------------------------------------------------
 
 do_baseenv
 
 Returns the current base environment.
 
 */


SEXP attribute_hidden do_baseenv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/*----------------------------------------------------------------------
 
 do_emptyenv
 
 Returns the current empty environment.
 
 */


SEXP attribute_hidden do_emptyenv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 do_attach
 
 To attach a list we make up an environment and insert components
 of the list in as the values of this env and install the tags from
 the list as the names.
 
 */

SEXP attribute_hidden do_attach(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}



/*----------------------------------------------------------------------
 
 do_detach
 
 detach the specified environment.  Detachment only takes place by
 position.
 
 */

SEXP attribute_hidden do_detach(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}



/*----------------------------------------------------------------------
 
 do_search
 
 Print out the current search path.
 
 */

SEXP attribute_hidden do_search(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 do_ls
 
 This code implements the functionality of the "ls" and "objects"
 functions.  [ ls(envir, all.names) ]
 
 */

static int FrameSize(SEXP frame, int all)
{
    return 0;
}

static void FrameNames(SEXP frame, int all, SEXP names, int *indx)
{
    return;
}

static void FrameValues(SEXP frame, int all, SEXP values, int *indx)
{
    return;
}

static int HashTableSize(SEXP table, int all)
{
    return 0;
}

static void HashTableNames(SEXP table, int all, SEXP names, int *indx)
{
    return;
}

static void HashTableValues(SEXP table, int all, SEXP values, int *indx)
{
    return;
}

static int BuiltinSize(int all, int intern)
{
    return 0;
}

static void
BuiltinNames(int all, int intern, SEXP names, int *indx)
{
}

static void
BuiltinValues(int all, int intern, SEXP values, int *indx)
{
}

SEXP attribute_hidden do_ls(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* takes a *list* of environments and a boolean indicating whether to get all
 names */
SEXP R_lsInternal(SEXP env, Rboolean all)
{
    return 0;
}

/* transform an environment into a named list */

SEXP attribute_hidden do_env2list(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/*
 * apply a function to all objects in an environment and return the
 * results in a list.
 * Equivalent to lapply(as.list(env, all.names=all.names), FUN, ...)
 */
/* This is a special .Internal */
SEXP attribute_hidden do_eapply(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

int envlength(SEXP rho)
{
    return 0;
}

/*----------------------------------------------------------------------
 
 do_builtins
 
 Return the names of all the built in functions.  These are fetched
 directly from the symbol table.
 
 */

SEXP attribute_hidden do_builtins(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/*----------------------------------------------------------------------
 
 do_pos2env
 
 This function returns the environment at a specified position in the
 search path or the environment of the caller of
 pos.to.env (? but pos.to.env is usually used in arg lists and hence
 is evaluated in the calling environment so this is one higher).
 
 When pos = -1 the environment of the closure that pos2env is
 evaluated in is obtained. Note: this relies on pos.to.env being
 a primitive.
 
 */
static SEXP pos2env(int pos, SEXP call)
{
    return 0;
}

/* this is primitive */
SEXP attribute_hidden do_pos2env(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

static SEXP matchEnvir(SEXP call, const char *what)
{
    return 0;
}

/* This is primitive */
SEXP attribute_hidden
do_as_environment(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

void R_LockEnvironment(SEXP env, Rboolean bindings)
{
    return;
}

Rboolean R_EnvironmentIsLocked(SEXP env)
{
    return 0;
}

SEXP attribute_hidden do_lockEnv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_envIsLocked(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

void R_LockBinding(SEXP sym, SEXP env)
{
    return;
}

void R_unLockBinding(SEXP sym, SEXP env)
{
    return;
}

void R_MakeActiveBinding(SEXP sym, SEXP fun, SEXP env)
{
    return;
}

Rboolean R_BindingIsLocked(SEXP sym, SEXP env)
{
    return 0;
}

Rboolean R_BindingIsActive(SEXP sym, SEXP env)
{
    return 0;
}

Rboolean R_HasFancyBindings(SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_lockBnd(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_bndIsLocked(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_mkActiveBnd(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_bndIsActive(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* This is a .Internal with no wrapper, currently unused in base R */
SEXP attribute_hidden do_mkUnbound(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

void R_RestoreHashCount(SEXP rho)
{
    return;
}

Rboolean R_IsPackageEnv(SEXP rho)
{
    return 0;
}

SEXP R_PackageEnvName(SEXP rho)
{
    return 0;
}

SEXP R_FindPackageEnv(SEXP info)
{
    return 0;
}

Rboolean R_IsNamespaceEnv(SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_isNSEnv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP R_NamespaceEnvSpec(SEXP rho)
{
    return 0;
}

SEXP R_FindNamespace(SEXP info)
{
    return 0;
}

static SEXP checkNSname(SEXP call, SEXP name)
{
    return 0;
}

SEXP attribute_hidden do_regNS(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_unregNS(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_getRegNS(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_getNSRegistry(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_importIntoEnv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


SEXP attribute_hidden do_envprofile(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

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

/* We can reuse the hash structure, but need separate code for get/set
 of values since our keys are char* and not SEXP symbol types.
 
 Experience has shown that it is better to use a different hash function,
 and a power of 2 for the hash size.
 */

/* char_hash_size MUST be a power of 2 and char_hash_mask ==
 char_hash_size - 1 for x & char_hash_mask to be equivalent to x %
 char_hash_size.
 */
static unsigned int char_hash_size = 65536;
static unsigned int char_hash_mask = 65535;

static unsigned int char_hash(const char *s, int len)
{
    return 0;
}

void attribute_hidden InitStringHash()
{
    return;
}

/* #define DEBUG_GLOBAL_STRING_HASH 1 */

/* Resize the global R_StringHash CHARSXP cache */
static void R_StringHash_resize(unsigned int newsize)
{
    return;
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



#ifdef DEBUG_SHOW_CHARSXP_CACHE
/* Call this from gdb with
 
 call do_show_cache(10)
 
 for the first 10 cache chains in use. */
void do_show_cache(int n)
{
    return;
}

void do_write_cache()
{
    return;
}
#endif /* DEBUG_SHOW_CHARSXP_CACHE */













