/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996	Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--2012	The R Core Team.
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


#undef HASHING

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>
#include <Rinterface.h>
#include <Fileio.h>
#include <R_ext/Print.h>


#define ARGUSED(x) LEVELS(x)

static SEXP bcEval(SEXP, SEXP, Rboolean);

/* BC_PROILFING needs to be defined here and in registration.c */
/*#define BC_PROFILING*/
#ifdef BC_PROFILING
static Rboolean bc_profiling = FALSE;
#endif

static int R_Profiling = 0;

#ifdef R_PROFILING

/* BDR 2000-07-15
 Profiling is now controlled by the R function Rprof(), and should
 have negligible cost when not enabled.
 */

/* A simple mechanism for profiling R code.  When R_PROFILING is
 enabled, eval will write out the call stack every PROFSAMPLE
 microseconds using the SIGPROF handler triggered by timer signals
 from the ITIMER_PROF timer.  Since this is the same timer used by C
 profiling, the two cannot be used together.  Output is written to
 the file PROFOUTNAME.  This is a plain text file.  The first line
 of the file contains the value of PROFSAMPLE.  The remaining lines
 each give the call stack found at a sampling point with the inner
 most function first.
 
 To enable profiling, recompile eval.c with R_PROFILING defined.  It
 would be possible to selectively turn profiling on and off from R
 and to specify the file name from R as well, but for now I won't
 bother.
 
 The stack is traced by walking back along the context stack, just
 like the traceback creation in jump_to_toplevel.  One drawback of
 this approach is that it does not show BUILTIN's since they don't
 get a context.  With recent changes to pos.to.env it seems possible
 to insert a context around BUILTIN calls to that they show up in
 the trace.  Since there is a cost in establishing these contexts,
 they are only inserted when profiling is enabled. [BDR: we have since
 also added contexts for the BUILTIN calls to foreign code.]
 
 One possible advantage of not tracing BUILTIN's is that then
 profiling adds no cost when the timer is turned off.  This would be
 useful if we want to allow profiling to be turned on and off from
 within R.
 
 One thing that makes interpreting profiling output tricky is lazy
 evaluation.  When an expression f(g(x)) is profiled, lazy
 evaluation will cause g to be called inside the call to f, so it
 will appear as if g is called by f.
 
 L. T.  */

#ifdef Win32
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>		/* for CreateEvent, SetEvent */
# include <process.h>		/* for _beginthread, _endthread */
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif
# include <signal.h>
#endif /* not Win32 */

static FILE *R_ProfileOutfile = NULL;
static int R_Mem_Profiling=0;
extern void get_current_mem(unsigned long *,unsigned long *,unsigned long *); /* in memory.c */
extern unsigned long get_duplicate_counter(void);  /* in duplicate.c */
extern void reset_duplicate_counter(void);         /* in duplicate.c */
static int R_GC_Profiling = 0;                     /* indicates GC profiling */
static int R_Line_Profiling = 0;                   /* indicates line profiling, and also counts the filenames seen (+1) */
static char **R_Srcfiles;			   /* an array of pointers into the filename buffer */
static size_t R_Srcfile_bufcount;                  /* how big is the array above? */
static SEXP R_Srcfiles_buffer = NULL;              /* a big RAWSXP to use as a buffer for filenames and pointers to them */
static int R_Profiling_Error;		   /* record errors here */

#ifdef Win32
HANDLE MainThread;
HANDLE ProfileEvent;
#endif /* Win32 */

/* Careful here!  These functions are called asynchronously, maybe in the middle of GC,
 so don't do any allocations */

/* This does a linear search through the previously recorded filenames.  If
 this one is new, we try to add it.  FIXME:  if there are eventually
 too many files for an efficient linear search, do hashing. */

static int getFilenum(const char* filename) {
    return 0;
}

/* These, together with sprintf/strcat, are not safe -- we should be
 using snprintf and such and computing needed sizes, but these
 settings are better than what we had. LT */

#define PROFBUFSIZ 10500
#define PROFITEMMAX  500
#define PROFLINEMAX (PROFBUFSIZ - PROFITEMMAX)

/* It would also be better to flush the buffer when it gets full,
 even if the line isn't complete. But this isn't possible if we rely
 on writing all line profiling files first.  With these sizes
 hitting the limit is fairly unlikely, but if we do then the output
 file is wrong. Maybe writing an overflow marker of some sort would
 be better.  LT */

static void lineprof(char* buf, SEXP srcref)
{
    return;
}

/* FIXME: This should be done wih a proper configure test, also making
 sure that the pthreads library is linked in. LT */
#ifndef Win32
#if (defined(__APPLE__) || defined(_REENTRANT) || defined(HAVE_OPENMP)) && \
! defined(HAVE_PTHREAD)
# define HAVE_PTHREAD
#endif
#ifdef HAVE_PTHREAD
# include <pthread.h>
static pthread_t R_profiled_thread;
# endif
#endif

static void doprof(int sig)  /* sig is ignored in Windows */
{
    return;
}

#ifdef Win32
/* Profiling thread main function */
static void __cdecl ProfileThread(void *pwait)
{
    return;
}
#else /* not Win32 */
static void doprof_null(int sig)
{
    return;
}
#endif /* not Win32 */


static void R_EndProfiling(void)
{
    return;
}

static void R_InitProfiling(SEXP filename, int append, double dinterval,
                            int mem_profiling, int gc_profiling,
                            int line_profiling, int numfiles, int bufsize)
{
    return;
}

SEXP do_Rprof(SEXP args)
{
    return 0;
}
#else /* not R_PROFILING */
SEXP do_Rprof(SEXP args)
{
    return 0;
}
#endif /* not R_PROFILING */

/* NEEDED: A fixup is needed in browser, because it can trap errors,
 *	and currently does not reset the limit to the right value. */

void attribute_hidden check_stack_balance(SEXP op, int save)
{
    return;
}


static SEXP forcePromise(SEXP e)
{
    return 0;
}

/* Return value of "e" evaluated in "rho". */

SEXP eval(SEXP e, SEXP rho)
{
    return 0;
}

attribute_hidden
void SrcrefPrompt(const char * prefix, SEXP srcref)
{
    return;
}

/* Apply SEXP op of type CLOSXP to actuals */

static void loadCompilerNamespace(void)
{
    return;
}

static int R_disable_bytecode = 0;

void attribute_hidden R_init_jit_enabled(void)
{
    return;
}

SEXP attribute_hidden R_cmpfun(SEXP fun)
{
    return 0;
}

static SEXP R_compileExpr(SEXP expr, SEXP rho)
{
    return 0;
}

static SEXP R_compileAndExecute(SEXP call, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_enablejit(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_compilepkgs(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* forward declaration */
static SEXP bytecodeExpr(SEXP);

/* this function gets the srcref attribute from a statement block,
 and confirms it's in the expected format */

static R_INLINE SEXP getBlockSrcrefs(SEXP call)
{
    return 0;
}

/* this function extracts one srcref, and confirms the format */
/* It assumes srcrefs has already been validated to be a VECSXP or NULL */

static R_INLINE SEXP getSrcref(SEXP srcrefs, int ind)
{
    return 0;
}

SEXP applyClosure(SEXP call, SEXP op, SEXP arglist, SEXP rho, SEXP suppliedenv)
{
    return 0;
}

/* **** FIXME: This code is factored out of applyClosure.  If we keep
 **** it we should change applyClosure to run through this routine
 **** to avoid code drift. */
static SEXP R_execClosure(SEXP call, SEXP op, SEXP arglist, SEXP rho,
                          SEXP newrho)
{
    return 0;
}

/* **** FIXME: Temporary code to execute S4 methods in a way that
 **** preserves lexical scope. */

/* called from methods_list_dispatch.c */
SEXP R_execMethod(SEXP op, SEXP rho)
{
    return 0;
}

static SEXP EnsureLocal(SEXP symbol, SEXP rho)
{
    return 0;
}


/* Note: If val is a language object it must be protected */
/* to prevent evaluation.  As an example consider */
/* e <- quote(f(x=1,y=2); names(e) <- c("","a","b") */

static SEXP replaceCall(SEXP fun, SEXP val, SEXP args, SEXP rhs)
{
    return 0;
}


static SEXP assignCall(SEXP op, SEXP symbol, SEXP fun,
                       SEXP val, SEXP args, SEXP rhs)
{
    return 0;
}


static R_INLINE Rboolean asLogicalNoNA(SEXP s, SEXP call)
{
    return 0;
}


#define BodyHasBraces(body) \
((isLanguage(body) && CAR(body) == R_BraceSymbol) ? 1 : 0)

#define DO_LOOP_RDEBUG(call, op, args, rho, bgn) do { \
if (bgn && RDEBUG(rho)) { \
SrcrefPrompt("debug", R_Srcref); \
PrintValue(CAR(args)); \
do_browser(call, op, R_NilValue, rho); \
} } while (0)

/* Allocate space for the loop variable value the first time through
 (when v == R_NilValue) and when the value has been assigned to
 another variable (NAMED(v) == 2). This should be safe and avoid
 allocation in many cases. */
#define ALLOC_LOOP_VAR(v, val_type, vpi) do { \
if (v == R_NilValue || NAMED(v) == 2) { \
REPROTECT(v = allocVector(val_type, 1), vpi); \
SET_NAMED(v, 1); \
} \
} while(0)

SEXP attribute_hidden do_if(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_for(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


SEXP attribute_hidden do_while(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


SEXP attribute_hidden do_repeat(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


SEXP attribute_hidden do_break(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


SEXP attribute_hidden do_paren(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_begin(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


SEXP attribute_hidden do_return(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* Declared with a variable number of args in names.c */
SEXP attribute_hidden do_function(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/*
 *  Assignments for complex LVAL specifications. This is the stuff that
 *  nightmares are made of ...	Note that "evalseq" preprocesses the LHS
 *  of an assignment.  Given an expression, it builds a list of partial
 *  values for the exression.  For example, the assignment x$a[3] <- 10
 *  with LHS x$a[3] yields the (improper) list:
 *
 *	 (eval(x$a[3])	eval(x$a)  eval(x)  .  x)
 *
 *  (Note the terminating symbol).  The partial evaluations are carried
 *  out efficiently using previously computed components.
 */

/*
 For complex superassignment  x[y==z]<<-w
 we want x required to be nonlocal, y,z, and w permitted to be local or
 nonlocal.
 */

static SEXP evalseq(SEXP expr, SEXP rho, int forcelocal,  R_varloc_t tmploc)
{
    return 0;
}

/* Main entry point for complex assignments */
/* We have checked to see that CAR(args) is a LANGSXP */

static const char * const asym[] = {":=", "<-", "<<-", "="};

static void tmp_cleanup(void *data)
{
    return;
}

/* This macro stores the current assignment target in the saved
 binding location. It duplicates if necessary to make sure
 replacement functions are always called with a target with NAMED ==
 1. The SET_CAR is intended to protect against possible GC in
 R_SetVarLocValue; this might occur it the binding is an active
 binding. */
#define SET_TEMPVARLOC_FROM_CAR(loc, lhs) do { \
SEXP __lhs__ = (lhs); \
SEXP __v__ = CAR(__lhs__); \
if (NAMED(__v__) == 2) { \
__v__ = duplicate(__v__); \
SET_NAMED(__v__, 1); \
SETCAR(__lhs__, __v__); \
} \
R_SetVarLocValue(loc, __v__); \
} while(0)

/* This macro makes sure the RHS NAMED value is 0 or 2. This is
 necessary to make sure the RHS value returned by the assignment
 expression is correct when the RHS value is part of the LHS
 object. */
#define FIXUP_RHS_NAMED(r) do { \
SEXP __rhs__ = (r); \
if (NAMED(__rhs__) && NAMED(__rhs__) != 2) \
SET_NAMED(__rhs__, 2); \
} while (0)

#define ASSIGNBUFSIZ 32
static R_INLINE SEXP installAssignFcnName(SEXP fun)
{
    return 0;
}

static SEXP applydefine(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* Defunct in 1.5.0
 SEXP attribute_hidden do_alias(SEXP call, SEXP op, SEXP args, SEXP rho)
 {
 return 0;
 }
 */

/*  Assignment in its various forms  */

SEXP attribute_hidden do_set(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/* Evaluate each expression in "el" in the environment "rho".  This is
 a naturally recursive algorithm, but we use the iterative form below
 because it is does not cause growth of the pointer protection stack,
 and because it is a little more efficient.
 */

#define COPY_TAG(to, from) do { \
SEXP __tag__ = TAG(from); \
if (__tag__ != R_NilValue) SET_TAG(to, __tag__); \
} while (0)

/* Used in eval and applyMethod (object.c) for builtin primitives,
 do_internal (names.c) for builtin .Internals
 and in evalArgs.
 
 'n' is the number of arguments already evaluated and hence not
 passed to evalArgs and hence to here.
 */
SEXP attribute_hidden evalList(SEXP el, SEXP rho, SEXP call, int n)
{
    return 0;
} /* evalList() */


/* A slight variation of evaluating each expression in "el" in "rho". */

/* used in evalArgs, arithmetic.c, seq.c */
SEXP attribute_hidden evalListKeepMissing(SEXP el, SEXP rho)
{
    return 0;
}


/* Create a promise to evaluate each argument.	Although this is most */
/* naturally attacked with a recursive algorithm, we use the iterative */
/* form below because it is does not cause growth of the pointer */
/* protection stack, and because it is a little more efficient. */

SEXP attribute_hidden promiseArgs(SEXP el, SEXP rho)
{
    return 0;
}


/* Check that each formal is a symbol */

/* used in coerce.c */
void attribute_hidden CheckFormals(SEXP ls)
{
    return;
}


static SEXP VectorToPairListNamed(SEXP x)
{
    return 0;
}

#define simple_as_environment(arg) (IS_S4_OBJECT(arg) && (TYPEOF(arg) == S4SXP) ? R_getS4DataSlot(arg, ENVSXP) : R_NilValue)

/* "eval": Evaluate the first argument
 in the environment specified by the second argument. */

SEXP attribute_hidden do_eval(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* This is a special .Internal */
SEXP attribute_hidden do_withVisible(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* This is a special .Internal */
SEXP attribute_hidden do_recall(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


static SEXP evalArgs(SEXP el, SEXP rho, int dropmissing, SEXP call, int n)
{
    return 0;
}


/* A version of DispatchOrEval that checks for possible S4 methods for
 * any argument, not just the first.  Used in the code for `[` in
 * do_subset.  Differs in that all arguments are evaluated
 * immediately, rather than after the call to R_possible_dispatch.
 */
attribute_hidden
int DispatchAnyOrEval(SEXP call, SEXP op, const char *generic, SEXP args,
                      SEXP rho, SEXP *ans, int dropmissing, int argsevald)
{
    return 0;
}


/* DispatchOrEval is used in internal functions which dispatch to
 * object methods (e.g. "[" or "[[").  The code either builds promises
 * and dispatches to the appropriate method, or it evaluates the
 * (unevaluated) arguments it comes in with and returns them so that
 * the generic built-in C code can continue.
 
 * To call this an ugly hack would be to insult all existing ugly hacks
 * at large in the world.
 */
attribute_hidden
int DispatchOrEval(SEXP call, SEXP op, const char *generic, SEXP args,
                   SEXP rho, SEXP *ans, int dropmissing, int argsevald)
{
    return 0;
}


/* gr needs to be protected on return from this function */
static void findmethod(SEXP Class, const char *group, const char *generic,
                       SEXP *sxp,  SEXP *gr, SEXP *meth, int *which,
                       char *buf, SEXP rho)
{
    return;
}

attribute_hidden
int DispatchGroup(const char* group, SEXP call, SEXP op, SEXP args, SEXP rho,
                  SEXP *ans)
{
    return 0;
}

/* start of bytecode section */
static int R_bcVersion = 7;
static int R_bcMinVersion = 6;

static SEXP R_AddSym = NULL;
static SEXP R_SubSym = NULL;
static SEXP R_MulSym = NULL;
static SEXP R_DivSym = NULL;
static SEXP R_ExptSym = NULL;
static SEXP R_SqrtSym = NULL;
static SEXP R_ExpSym = NULL;
static SEXP R_EqSym = NULL;
static SEXP R_NeSym = NULL;
static SEXP R_LtSym = NULL;
static SEXP R_LeSym = NULL;
static SEXP R_GeSym = NULL;
static SEXP R_GtSym = NULL;
static SEXP R_AndSym = NULL;
static SEXP R_OrSym = NULL;
static SEXP R_NotSym = NULL;
static SEXP R_SubsetSym = NULL;
static SEXP R_SubassignSym = NULL;
static SEXP R_CSym = NULL;
static SEXP R_Subset2Sym = NULL;
static SEXP R_Subassign2Sym = NULL;
static SEXP R_valueSym = NULL;
static SEXP R_TrueValue = NULL;
static SEXP R_FalseValue = NULL;

#if defined(__GNUC__) && ! defined(BC_PROFILING) && (! defined(NO_THREADED_CODE))
# define THREADED_CODE
#endif

attribute_hidden
void R_initialize_bcode(void)
{
    return;
}

enum {
    BCMISMATCH_OP,
    RETURN_OP,
    GOTO_OP,
    BRIFNOT_OP,
    POP_OP,
    DUP_OP,
    PRINTVALUE_OP,
    STARTLOOPCNTXT_OP,
    ENDLOOPCNTXT_OP,
    DOLOOPNEXT_OP,
    DOLOOPBREAK_OP,
    STARTFOR_OP,
    STEPFOR_OP,
    ENDFOR_OP,
    SETLOOPVAL_OP,
    INVISIBLE_OP,
    LDCONST_OP,
    LDNULL_OP,
    LDTRUE_OP,
    LDFALSE_OP,
    GETVAR_OP,
    DDVAL_OP,
    SETVAR_OP,
    GETFUN_OP,
    GETGLOBFUN_OP,
    GETSYMFUN_OP,
    GETBUILTIN_OP,
    GETINTLBUILTIN_OP,
    CHECKFUN_OP,
    MAKEPROM_OP,
    DOMISSING_OP,
    SETTAG_OP,
    DODOTS_OP,
    PUSHARG_OP,
    PUSHCONSTARG_OP,
    PUSHNULLARG_OP,
    PUSHTRUEARG_OP,
    PUSHFALSEARG_OP,
    CALL_OP,
    CALLBUILTIN_OP,
    CALLSPECIAL_OP,
    MAKECLOSURE_OP,
    UMINUS_OP,
    UPLUS_OP,
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    EXPT_OP,
    SQRT_OP,
    EXP_OP,
    EQ_OP,
    NE_OP,
    LT_OP,
    LE_OP,
    GE_OP,
    GT_OP,
    AND_OP,
    OR_OP,
    NOT_OP,
    DOTSERR_OP,
    STARTASSIGN_OP,
    ENDASSIGN_OP,
    STARTSUBSET_OP,
    DFLTSUBSET_OP,
    STARTSUBASSIGN_OP,
    DFLTSUBASSIGN_OP,
    STARTC_OP,
    DFLTC_OP,
    STARTSUBSET2_OP,
    DFLTSUBSET2_OP,
    STARTSUBASSIGN2_OP,
    DFLTSUBASSIGN2_OP,
    DOLLAR_OP,
    DOLLARGETS_OP,
    ISNULL_OP,
    ISLOGICAL_OP,
    ISINTEGER_OP,
    ISDOUBLE_OP,
    ISCOMPLEX_OP,
    ISCHARACTER_OP,
    ISSYMBOL_OP,
    ISOBJECT_OP,
    ISNUMERIC_OP,
    VECSUBSET_OP,
    MATSUBSET_OP,
    SETVECSUBSET_OP,
    SETMATSUBSET_OP,
    AND1ST_OP,
    AND2ND_OP,
    OR1ST_OP,
    OR2ND_OP,
    GETVAR_MISSOK_OP,
    DDVAL_MISSOK_OP,
    VISIBLE_OP,
    SETVAR2_OP,
    STARTASSIGN2_OP,
    ENDASSIGN2_OP,
    SETTER_CALL_OP,
    GETTER_CALL_OP,
    SWAP_OP,
    DUP2ND_OP,
    SWITCH_OP,
    RETURNJMP_OP,
    STARTVECSUBSET_OP,
    STARTMATSUBSET_OP,
    STARTSETVECSUBSET_OP,
    STARTSETMATSUBSET_OP,
    OPCOUNT
};


SEXP R_unary(SEXP, SEXP, SEXP);
SEXP R_binary(SEXP, SEXP, SEXP, SEXP);
SEXP do_math1(SEXP, SEXP, SEXP, SEXP);
SEXP do_relop_dflt(SEXP, SEXP, SEXP, SEXP);
SEXP do_logic(SEXP, SEXP, SEXP, SEXP);
SEXP do_subset_dflt(SEXP, SEXP, SEXP, SEXP);
SEXP do_subassign_dflt(SEXP, SEXP, SEXP, SEXP);
SEXP do_c_dflt(SEXP, SEXP, SEXP, SEXP);
SEXP do_subset2_dflt(SEXP, SEXP, SEXP, SEXP);
SEXP do_subassign2_dflt(SEXP, SEXP, SEXP, SEXP);

#define GETSTACK_PTR(s) (*(s))
#define GETSTACK(i) GETSTACK_PTR(R_BCNodeStackTop + (i))

#define SETSTACK_PTR(s, v) do { \
SEXP __v__ = (v); \
*(s) = __v__; \
} while (0)

#define SETSTACK(i, v) SETSTACK_PTR(R_BCNodeStackTop + (i), v)

#define SETSTACK_REAL_PTR(s, v) SETSTACK_PTR(s, ScalarReal(v))

#define SETSTACK_REAL(i, v) SETSTACK_REAL_PTR(R_BCNodeStackTop + (i), v)

#define SETSTACK_INTEGER_PTR(s, v) SETSTACK_PTR(s, ScalarInteger(v))

#define SETSTACK_INTEGER(i, v) SETSTACK_INTEGER_PTR(R_BCNodeStackTop + (i), v)

#define SETSTACK_LOGICAL_PTR(s, v) do { \
int __ssl_v__ = (v); \
if (__ssl_v__ == NA_LOGICAL) \
SETSTACK_PTR(s, ScalarLogical(NA_LOGICAL)); \
else \
SETSTACK_PTR(s, __ssl_v__ ? R_TrueValue : R_FalseValue); \
} while(0)

#define SETSTACK_LOGICAL(i, v) SETSTACK_LOGICAL_PTR(R_BCNodeStackTop + (i), v)

typedef union { double dval; int ival; } scalar_value_t;

/* bcStackScalar() checks whether the object in the specified stack
 location is a simple real, integer, or logical scalar (i.e. length
 one and no attributes.  If so, the type is returned as the function
 value and the value is returned in the structure pointed to by the
 second argument; if not, then zero is returned as the function
 value. */
static R_INLINE int bcStackScalar(R_bcstack_t *s, scalar_value_t *v)
{
    return 0;
}

#define DO_FAST_RELOP2(op,a,b) do { \
SKIP_OP(); \
SETSTACK_LOGICAL(-2, ((a) op (b)) ? TRUE : FALSE);	\
R_BCNodeStackTop--; \
NEXT(); \
} while (0)

# define FastRelop2(op,opval,opsym) do { \
scalar_value_t vx; \
scalar_value_t vy; \
int typex = bcStackScalar(R_BCNodeStackTop - 2, &vx); \
int typey = bcStackScalar(R_BCNodeStackTop - 1, &vy); \
if (typex == REALSXP && ! ISNAN(vx.dval)) { \
if (typey == REALSXP && ! ISNAN(vy.dval)) \
DO_FAST_RELOP2(op, vx.dval, vy.dval); \
else if (typey == INTSXP && vy.ival != NA_INTEGER) \
DO_FAST_RELOP2(op, vx.dval, vy.ival); \
} \
else if (typex == INTSXP && vx.ival != NA_INTEGER) { \
if (typey == REALSXP && ! ISNAN(vy.dval)) \
DO_FAST_RELOP2(op, vx.ival, vy.dval); \
else if (typey == INTSXP && vy.ival != NA_INTEGER) { \
DO_FAST_RELOP2(op, vx.ival, vy.ival); \
} \
} \
Relop2(opval, opsym); \
} while (0)

static R_INLINE SEXP getPrimitive(SEXP symbol, SEXPTYPE type)
{
    return 0;
}

static SEXP cmp_relop(SEXP call, int opval, SEXP opsym, SEXP x, SEXP y,
                      SEXP rho)
{
    return 0;
}

static SEXP cmp_arith1(SEXP call, SEXP opsym, SEXP x, SEXP rho)
{
    return 0;
}

static SEXP cmp_arith2(SEXP call, int opval, SEXP opsym, SEXP x, SEXP y,
                       SEXP rho)
{
    return 0;
}

#define Builtin1(do_fun,which,rho) do { \
SEXP call = VECTOR_ELT(constants, GETOP()); \
SETSTACK(-1, CONS(GETSTACK(-1), R_NilValue));		     \
SETSTACK(-1, do_fun(call, getPrimitive(which, BUILTINSXP), \
GETSTACK(-1), rho));		     \
NEXT(); \
} while(0)

#define Builtin2(do_fun,which,rho) do {		     \
SEXP call = VECTOR_ELT(constants, GETOP()); \
SEXP tmp = CONS(GETSTACK(-1), R_NilValue); \
SETSTACK(-2, CONS(GETSTACK(-2), tmp));     \
R_BCNodeStackTop--; \
SETSTACK(-1, do_fun(call, getPrimitive(which, BUILTINSXP),	\
GETSTACK(-1), rho));			\
NEXT(); \
} while(0)

#define NewBuiltin2(do_fun,opval,opsym,rho) do {	\
SEXP call = VECTOR_ELT(constants, GETOP()); \
SEXP x = GETSTACK(-2); \
SEXP y = GETSTACK(-1); \
SETSTACK(-2, do_fun(call, opval, opsym, x, y,rho));	\
R_BCNodeStackTop--; \
NEXT(); \
} while(0)

#define Arith1(opsym) do {		\
SEXP call = VECTOR_ELT(constants, GETOP()); \
SEXP x = GETSTACK(-1); \
SETSTACK(-1, cmp_arith1(call, opsym, x, rho)); \
NEXT(); \
} while(0)


#define Arith2(opval,opsym) NewBuiltin2(cmp_arith2,opval,opsym,rho)
#define Math1(which) Builtin1(do_math1,which,rho)
#define Relop2(opval,opsym) NewBuiltin2(cmp_relop,opval,opsym,rho)

# define DO_FAST_BINOP(op,a,b) do { \
SKIP_OP(); \
SETSTACK_REAL(-2, (a) op (b)); \
R_BCNodeStackTop--; \
NEXT(); \
} while (0)

# define DO_FAST_BINOP_INT(op, a, b) do { \
double dval = ((double) (a)) op ((double) (b)); \
if (dval <= INT_MAX && dval >= INT_MIN + 1) { \
SKIP_OP(); \
SETSTACK_INTEGER(-2, (int) dval); \
R_BCNodeStackTop--; \
NEXT(); \
} \
} while(0)

# define FastBinary(op,opval,opsym) do { \
scalar_value_t vx; \
scalar_value_t vy; \
int typex = bcStackScalar(R_BCNodeStackTop - 2, &vx); \
int typey = bcStackScalar(R_BCNodeStackTop - 1, &vy); \
if (typex == REALSXP) { \
if (typey == REALSXP) \
DO_FAST_BINOP(op, vx.dval, vy.dval); \
else if (typey == INTSXP && vy.ival != NA_INTEGER) \
DO_FAST_BINOP(op, vx.dval, vy.ival); \
} \
else if (typex == INTSXP && vx.ival != NA_INTEGER) { \
if (typey == REALSXP) \
DO_FAST_BINOP(op, vx.ival, vy.dval); \
else if (typey == INTSXP && vy.ival != NA_INTEGER) { \
if (opval == DIVOP) \
DO_FAST_BINOP(op, (double) vx.ival, (double) vy.ival); \
else \
DO_FAST_BINOP_INT(op, vx.ival, vy.ival); \
} \
} \
Arith2(opval, opsym); \
} while (0)

#define BCNPUSH(v) do { \
SEXP __value__ = (v); \
R_bcstack_t *__ntop__ = R_BCNodeStackTop + 1; \
if (__ntop__ > R_BCNodeStackEnd) nodeStackOverflow(); \
__ntop__[-1] = __value__; \
R_BCNodeStackTop = __ntop__; \
} while (0)

#define BCNDUP() do { \
R_bcstack_t *__ntop__ = R_BCNodeStackTop + 1; \
if (__ntop__ > R_BCNodeStackEnd) nodeStackOverflow(); \
__ntop__[-1] = __ntop__[-2]; \
R_BCNodeStackTop = __ntop__; \
} while(0)

#define BCNDUP2ND() do { \
R_bcstack_t *__ntop__ = R_BCNodeStackTop + 1; \
if (__ntop__ > R_BCNodeStackEnd) nodeStackOverflow(); \
__ntop__[-1] = __ntop__[-3]; \
R_BCNodeStackTop = __ntop__; \
} while(0)

#define BCNPOP() (R_BCNodeStackTop--, GETSTACK(0))
#define BCNPOP_IGNORE_VALUE() R_BCNodeStackTop--

#define BCNSTACKCHECK(n)  do { \
if (R_BCNodeStackTop + 1 > R_BCNodeStackEnd) nodeStackOverflow(); \
} while (0)

#define BCIPUSHPTR(v)  do { \
void *__value__ = (v); \
IStackval *__ntop__ = R_BCIntStackTop + 1; \
if (__ntop__ > R_BCIntStackEnd) intStackOverflow(); \
*__ntop__[-1].p = __value__; \
R_BCIntStackTop = __ntop__; \
} while (0)

#define BCIPUSHINT(v)  do { \
int __value__ = (v); \
IStackval *__ntop__ = R_BCIntStackTop + 1; \
if (__ntop__ > R_BCIntStackEnd) intStackOverflow(); \
__ntop__[-1].i = __value__; \
R_BCIntStackTop = __ntop__; \
} while (0)

#define BCIPOPPTR() ((--R_BCIntStackTop)->p)
#define BCIPOPINT() ((--R_BCIntStackTop)->i)

#define BCCONSTS(e) BCODE_CONSTS(e)

static void nodeStackOverflow()
{
    return;
}

#ifdef BC_INT_STACK
static void intStackOverflow()
{
    return;
}
#endif

static SEXP bytecodeExpr(SEXP e)
{
    return 0;
}

SEXP R_PromiseExpr(SEXP p)
{
    return 0;
}

SEXP R_ClosureExpr(SEXP p)
{
    return 0;
}

#ifdef THREADED_CODE
typedef union { void *v; int i; } BCODE;

static struct { void *addr; int argc; } opinfo[OPCOUNT];

#define OP(name,n) \
case name##_OP: opinfo[name##_OP].addr = (__extension__ &&op_##name); \
opinfo[name##_OP].argc = (n); \
goto loop; \
op_##name

#define BEGIN_MACHINE  NEXT(); init: { loop: switch(which++)
#define LASTOP } value = R_NilValue; goto done
#define INITIALIZE_MACHINE() if (body == NULL) goto init

#define NEXT() (__extension__ ({goto *(*pc++).v;}))
#define GETOP() (*pc++).i
#define SKIP_OP() (pc++)

#define BCCODE(e) (BCODE *) INTEGER(BCODE_CODE(e))
#else
typedef int BCODE;

#define OP(name,argc) case name##_OP

#ifdef BC_PROFILING
#define BEGIN_MACHINE  loop: current_opcode = *pc; switch(*pc++)
#else
#define BEGIN_MACHINE  loop: switch(*pc++)
#endif
#define LASTOP  default: error(_("bad opcode"))
#define INITIALIZE_MACHINE()

#define NEXT() goto loop
#define GETOP() *pc++
#define SKIP_OP() (pc++)

#define BCCODE(e) INTEGER(BCODE_CODE(e))
#endif

static R_INLINE SEXP GET_BINDING_CELL(SEXP symbol, SEXP rho)
{
    return 0;
}

static R_INLINE Rboolean SET_BINDING_VALUE(SEXP loc, SEXP value) {
    return 0;
}

static R_INLINE SEXP BINDING_VALUE(SEXP loc)
{
    return 0;
}

#define BINDING_SYMBOL(loc) TAG(loc)

/* Defining USE_BINDING_CACHE enables a cache for GETVAR, SETVAR, and
 others to more efficiently locate bindings in the top frame of the
 current environment.  The index into of the symbol in the constant
 table is used as the cache index.  Two options can be used to chose
 among implementation strategies:
 
 If CACHE_ON_STACK is defined the the cache is allocated on the
 byte code stack. Otherwise it is allocated on the heap as a
 VECSXP.  The stack-based approach is more efficient, but runs
 the risk of running out of stack space.
 
 If CACHE_MAX is defined, then a cache of at most that size is
 used. The value must be a power of 2 so a modulus computation x
 % CACHE_MAX can be done as x & (CACHE_MAX - 1). More than 90%
 of the closures in base have constant pools with fewer than 128
 entries when compiled, to that is a good value to use.
 
 On average about 1/3 of constant pool entries are symbols, so this
 approach wastes some space.  This could be avoided by grouping the
 symbols at the beginning of the constant pool and recording the
 number.
 
 Bindings recorded may become invalid if user code removes a
 variable.  The code in envir.c has been modified to insert
 R_unboundValue as the value of a binding when it is removed, and
 code using cached bindings checks for this.
 
 It would be nice if we could also cache bindings for variables
 found in enclosing environments. These would become invalid if a
 new variable is defined in an intervening frame. Some mechanism for
 invalidating the cache would be needed. This is certainly possible,
 but finding an efficient mechanism does not seem to be easy.   LT */

/* Both mechanisms implemented here make use of the stack to hold
 cache information.  This is not a problem except for "safe" for()
 loops using the STARTLOOPCNTXT instruction to run the body in a
 separate bcEval call.  Since this approach expects loop setup
 information to be passed on the stack from the outer bcEval call to
 an inner one the inner one cannot put things on the stack. For now,
 bcEval takes an additional argument that disables the cache in
 calls via STARTLOOPCNTXT for all "safe" loops. It would be better
 to deal with this in some other way, for example by having a
 specific STARTFORLOOPCNTXT instruction that deals with transferring
 the information in some other way. For now disabling the cache is
 an expedient solution. LT */

#define USE_BINDING_CACHE
# ifdef USE_BINDING_CACHE
/* CACHE_MAX must be a power of 2 for modulus using & CACHE_MASK to work*/
# define CACHE_MAX 128
# ifdef CACHE_MAX
#  define CACHE_MASK (CACHE_MAX - 1)
#  define CACHEIDX(i) ((i) & CACHE_MASK)
# else
#  define CACHEIDX(i) (i)
# endif

# define CACHE_ON_STACK
# ifdef CACHE_ON_STACK
typedef R_bcstack_t * R_binding_cache_t;
#  define GET_CACHED_BINDING_CELL(vcache, sidx) \
(vcache ? vcache[CACHEIDX(sidx)] : R_NilValue)
#  define GET_SMALLCACHE_BINDING_CELL(vcache, sidx) \
(vcache ? vcache[sidx] : R_NilValue)

#  define SET_CACHED_BINDING(cvache, sidx, cell) \
do { if (vcache) vcache[CACHEIDX(sidx)] = (cell); } while (0)
# else
typedef SEXP R_binding_cache_t;
#  define GET_CACHED_BINDING_CELL(vcache, sidx) \
(vcache ? VECTOR_ELT(vcache, CACHEIDX(sidx)) : R_NilValue)
#  define GET_SMALLCACHE_BINDING_CELL(vcache, sidx) \
(vcache ? VECTOR_ELT(vcache, sidx) : R_NilValue)

#  define SET_CACHED_BINDING(vcache, sidx, cell) \
do { if (vcache) SET_VECTOR_ELT(vcache, CACHEIDX(sidx), cell); } while (0)
# endif
#else
typedef void *R_binding_cache_t;
# define GET_CACHED_BINDING_CELL(vcache, sidx) R_NilValue
# define GET_SMALLCACHE_BINDING_CELL(vcache, sidx) R_NilValue

# define SET_CACHED_BINDING(vcache, sidx, cell)
#endif

static R_INLINE SEXP GET_BINDING_CELL_CACHE(SEXP symbol, SEXP rho,
                                            R_binding_cache_t vcache, int idx)
{
    return 0;
}

static void MISSING_ARGUMENT_ERROR(SEXP symbol)
{
    return;
}

#define MAYBE_MISSING_ARGUMENT_ERROR(symbol, keepmiss) \
do { if (! keepmiss) MISSING_ARGUMENT_ERROR(symbol); } while (0)

static void UNBOUND_VARIABLE_ERROR(SEXP symbol)
{
    return;
}

static R_INLINE SEXP FORCE_PROMISE(SEXP value, SEXP symbol, SEXP rho,
                                   Rboolean keepmiss)
{
    return 0;
}

static R_INLINE SEXP FIND_VAR_NO_CACHE(SEXP symbol, SEXP rho, SEXP cell)
{
    return 0;
}

static R_INLINE SEXP getvar(SEXP symbol, SEXP rho,
                            Rboolean dd, Rboolean keepmiss,
                            R_binding_cache_t vcache, int sidx)
{
    return 0;
}

#define INLINE_GETVAR
#ifdef INLINE_GETVAR
/* Try to handle the most common case as efficiently as possible.  If
 smallcache is true then a modulus operation on the index is not
 needed, nor is a check that a non-null value corresponds to the
 requested symbol. The symbol from the constant pool is also usually
 not needed. The test TYPOF(value) != SYMBOL rules out R_MissingArg
 and R_UnboundValue as these are implemented s symbols.  It also
 rules other symbols, but as those are rare they are handled by the
 getvar() call. */
#define DO_GETVAR(dd,keepmiss) do { \
int sidx = GETOP(); \
if (!dd && smallcache) { \
SEXP cell = GET_SMALLCACHE_BINDING_CELL(vcache, sidx); \
/* try fast handling of REALSXP, INTSXP, LGLSXP */ \
/* (cell won't be R_NilValue or an active binding) */ \
value = CAR(cell); \
int type = TYPEOF(value); \
switch(type) { \
case REALSXP: \
case INTSXP: \
case LGLSXP: \
/* may be ok to skip this test: */ \
if (NAMED(value) == 0) \
SET_NAMED(value, 1); \
R_Visible = TRUE; \
BCNPUSH(value); \
NEXT(); \
} \
if (cell != R_NilValue && ! IS_ACTIVE_BINDING(cell)) { \
value = CAR(cell); \
if (TYPEOF(value) != SYMSXP) {	\
if (TYPEOF(value) == PROMSXP) {		\
SEXP pv = PRVALUE(value);		\
if (pv == R_UnboundValue) {		\
SEXP symbol = VECTOR_ELT(constants, sidx);	\
value = FORCE_PROMISE(value, symbol, rho, keepmiss); \
}							\
else value = pv;					\
}							\
else if (NAMED(value) == 0)				\
SET_NAMED(value, 1);				\
R_Visible = TRUE;					\
BCNPUSH(value);						\
NEXT();							\
}								\
}								\
}									\
SEXP symbol = VECTOR_ELT(constants, sidx);				\
R_Visible = TRUE;							\
BCNPUSH(getvar(symbol, rho, dd, keepmiss, vcache, sidx));		\
NEXT();								\
} while (0)
#else
#define DO_GETVAR(dd,keepmiss) do { \
int sidx = GETOP(); \
SEXP symbol = VECTOR_ELT(constants, sidx); \
R_Visible = TRUE; \
BCNPUSH(getvar(symbol, rho, dd, keepmiss, vcache, sidx));	\
NEXT(); \
} while (0)
#endif

#define PUSHCALLARG(v) PUSHCALLARG_CELL(CONS(v, R_NilValue))

#define PUSHCALLARG_CELL(c) do { \
SEXP __cell__ = (c); \
if (GETSTACK(-2) == R_NilValue) SETSTACK(-2, __cell__); \
else SETCDR(GETSTACK(-1), __cell__); \
SETSTACK(-1, __cell__);	       \
} while (0)

static int tryDispatch(char *generic, SEXP call, SEXP x, SEXP rho, SEXP *pv)
{
    return 0;
}

static int tryAssignDispatch(char *generic, SEXP call, SEXP lhs, SEXP rhs,
                             SEXP rho, SEXP *pv)
{
    return 0;
}

#define DO_STARTDISPATCH(generic) do { \
SEXP call = VECTOR_ELT(constants, GETOP()); \
int label = GETOP(); \
value = GETSTACK(-1); \
if (isObject(value) && tryDispatch(generic, call, value, rho, &value)) {\
SETSTACK(-1, value);						\
BC_CHECK_SIGINT(); \
pc = codebase + label; \
} \
else { \
SEXP tag = TAG(CDR(call)); \
SEXP cell = CONS(value, R_NilValue); \
BCNSTACKCHECK(3); \
SETSTACK(0, call); \
SETSTACK(1, cell); \
SETSTACK(2, cell); \
R_BCNodeStackTop += 3; \
if (tag != R_NilValue) \
SET_TAG(cell, CreateTag(tag)); \
} \
NEXT(); \
} while (0)

#define DO_DFLTDISPATCH(fun, symbol) do { \
SEXP call = GETSTACK(-3); \
SEXP args = GETSTACK(-2); \
value = fun(call, symbol, args, rho); \
R_BCNodeStackTop -= 3; \
SETSTACK(-1, value); \
NEXT(); \
} while (0)

#define DO_START_ASSIGN_DISPATCH(generic) do { \
SEXP call = VECTOR_ELT(constants, GETOP()); \
int label = GETOP(); \
SEXP lhs = GETSTACK(-2); \
SEXP rhs = GETSTACK(-1); \
if (NAMED(lhs) == 2) { \
lhs = duplicate(lhs); \
SETSTACK(-2, lhs); \
SET_NAMED(lhs, 1); \
} \
if (isObject(lhs) && \
tryAssignDispatch(generic, call, lhs, rhs, rho, &value)) { \
R_BCNodeStackTop--;	\
SETSTACK(-1, value); \
BC_CHECK_SIGINT(); \
pc = codebase + label; \
} \
else { \
SEXP tag = TAG(CDR(call)); \
SEXP cell = CONS(lhs, R_NilValue); \
BCNSTACKCHECK(3); \
SETSTACK(0, call); \
SETSTACK(1, cell); \
SETSTACK(2, cell); \
R_BCNodeStackTop += 3; \
if (tag != R_NilValue) \
SET_TAG(cell, CreateTag(tag)); \
} \
NEXT(); \
} while (0)

#define DO_DFLT_ASSIGN_DISPATCH(fun, symbol) do { \
SEXP rhs = GETSTACK(-4); \
SEXP call = GETSTACK(-3); \
SEXP args = GETSTACK(-2); \
PUSHCALLARG(rhs); \
value = fun(call, symbol, args, rho); \
R_BCNodeStackTop -= 4; \
SETSTACK(-1, value);	 \
NEXT(); \
} while (0)

#define DO_STARTDISPATCH_N(generic) do { \
int callidx = GETOP(); \
int label = GETOP(); \
value = GETSTACK(-1); \
if (isObject(value)) { \
SEXP call = VECTOR_ELT(constants, callidx); \
if (tryDispatch(generic, call, value, rho, &value)) { \
SETSTACK(-1, value); \
BC_CHECK_SIGINT(); \
pc = codebase + label; \
} \
} \
NEXT(); \
} while (0)

#define DO_START_ASSIGN_DISPATCH_N(generic) do { \
int callidx = GETOP(); \
int label = GETOP(); \
SEXP lhs = GETSTACK(-2); \
if (isObject(lhs)) { \
SEXP call = VECTOR_ELT(constants, callidx); \
SEXP rhs = GETSTACK(-1); \
if (NAMED(lhs) == 2) { \
lhs = duplicate(lhs); \
SETSTACK(-2, lhs); \
SET_NAMED(lhs, 1); \
} \
if (tryAssignDispatch(generic, call, lhs, rhs, rho, &value)) { \
R_BCNodeStackTop--; \
SETSTACK(-1, value); \
BC_CHECK_SIGINT(); \
pc = codebase + label; \
} \
} \
NEXT(); \
} while (0)

#define DO_ISTEST(fun) do { \
SETSTACK(-1, fun(GETSTACK(-1)) ? R_TrueValue : R_FalseValue);	\
NEXT(); \
} while(0)
#define DO_ISTYPE(type) do { \
SETSTACK(-1, TYPEOF(GETSTACK(-1)) == type ? mkTrue() : mkFalse()); \
NEXT(); \
} while (0)
#define isNumericOnly(x) (isNumeric(x) && ! isLogical(x))

#ifdef BC_PROFILING
#define NO_CURRENT_OPCODE -1
static int current_opcode = NO_CURRENT_OPCODE;
static int opcode_counts[OPCOUNT];
#endif

#define BC_COUNT_DELTA 1000

#define BC_CHECK_SIGINT() do { \
if (++evalcount > BC_COUNT_DELTA) { \
R_CheckUserInterrupt(); \
evalcount = 0; \
} \
} while (0)

static void loopWithContext(volatile SEXP code, volatile SEXP rho)
{
    return;
}

static R_INLINE int bcStackIndex(R_bcstack_t *s)
{
    return 0;
}

static R_INLINE void VECSUBSET_PTR(R_bcstack_t *sx, R_bcstack_t *si,
                                   R_bcstack_t *sv, SEXP rho)
{
    return;
}

#define DO_VECSUBSET(rho) do { \
VECSUBSET_PTR(R_BCNodeStackTop - 2, R_BCNodeStackTop - 1, \
R_BCNodeStackTop - 2, rho); \
R_BCNodeStackTop--; \
} while(0)

static R_INLINE SEXP getMatrixDim(SEXP mat)
{
    return 0;
}

static R_INLINE void DO_MATSUBSET(SEXP rho)
{
    return;
}

#define INTEGER_TO_REAL(x) ((x) == NA_INTEGER ? NA_REAL : (x))
#define LOGICAL_TO_REAL(x) ((x) == NA_LOGICAL ? NA_REAL : (x))

static R_INLINE Rboolean setElementFromScalar(SEXP vec, int i, int typev,
                                              scalar_value_t *v)
{
    return 0;
}

static R_INLINE void SETVECSUBSET_PTR(R_bcstack_t *sx, R_bcstack_t *srhs,
                                      R_bcstack_t *si, R_bcstack_t *sv,
                                      SEXP rho)
{
    return;
}

static R_INLINE void DO_SETVECSUBSET(SEXP rho)
{
    return;
}

static R_INLINE void DO_SETMATSUBSET(SEXP rho)
{
    return;
}

#define FIXUP_SCALAR_LOGICAL(callidx, arg, op) do { \
SEXP val = GETSTACK(-1); \
if (TYPEOF(val) != LGLSXP || LENGTH(val) != 1) { \
if (!isNumber(val))	\
errorcall(VECTOR_ELT(constants, callidx), \
_("invalid %s type in 'x %s y'"), arg, op);	\
SETSTACK(-1, ScalarLogical(asLogical(val))); \
} \
} while(0)

static R_INLINE void checkForMissings(SEXP args, SEXP call)
{
    return;
}

#define GET_VEC_LOOP_VALUE(var, pos) do {		\
(var) = GETSTACK(pos);				\
if (NAMED(var) == 2) {				\
(var) = allocVector(TYPEOF(seq), 1);		\
SETSTACK(pos, var);				\
SET_NAMED(var, 1);				\
}							\
} while (0)

static SEXP bcEval(SEXP body, SEXP rho, Rboolean useCache)
{
    return 0;
}

#ifdef THREADED_CODE
SEXP R_bcEncode(SEXP bytes)
{
    return 0;
}

static int findOp(void *addr)
{
    return 0;
}

SEXP R_bcDecode(SEXP code) {
    return 0;
}
#else
SEXP R_bcEncode(SEXP x) { return x; }
SEXP R_bcDecode(SEXP x) { return duplicate(x); }
#endif

SEXP attribute_hidden do_mkcode(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_bcclose(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_is_builtin_internal(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

static SEXP disassemble(SEXP bc)
{
    return 0;
}

SEXP attribute_hidden do_disassemble(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_bcversion(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_loadfile(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

SEXP attribute_hidden do_savefile(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

#ifdef UNUSED
#define R_COMPILED_EXTENSION ".Rc"

/* neither of these functions call R_ExpandFileName -- the caller
 should do that if it wants to */
char *R_CompiledFileName(char *fname, char *buf, size_t bsize)
{
    return 0;
}

FILE *R_OpenCompiledFile(char *fname, char *buf, size_t bsize)
{
    return 0;
}
#endif

SEXP attribute_hidden do_growconst(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

SEXP attribute_hidden do_putconst(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

SEXP attribute_hidden do_getconst(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

#ifdef BC_PROFILING
SEXP R_getbcprofcounts()
{
    return 0;
}

static void dobcprof(int sig)
{
    return;
}

SEXP R_startbcprof()
{
    return 0;
}

static void dobcprof_null(int sig)
{
    return;
}

SEXP R_stopbcprof()
{
    return 0;
}
#endif

/* end of byte code section */

SEXP attribute_hidden do_setnumthreads(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_setmaxnumthreads(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}
