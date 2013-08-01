/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998-2012   The R Core Team.
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
 *  Contexts:
 *
 *  A linked-list of execution contexts is kept so that control-flow
 *  constructs like "next", "break" and "return" will work.  It is also
 *  used for error returns to top-level.
 *
 *	context[k] -> context[k-1] -> ... -> context[0]
 *	^				     ^
 *	R_GlobalContext			     R_ToplevelContext
 *
 *  Contexts are allocated on the stack as the evaluator invokes itself
 *  recursively.  The memory is reclaimed naturally on return through
 *  the recursions (the R_GlobalContext pointer needs adjustment).
 *
 *  A context contains the following information (and more):
 *
 *	nextcontext	the next level context
 *	cjmpbuf		longjump information for non-local return
 *	cstacktop	the current level of the pointer protection stack
 *	callflag	the context "type"
 *	call		the call (name of function, or expression to
 *			get the function) that effected this
 *			context if a closure, otherwise often NULL.
 *	callfun		the function, if this was a closure.
 *	cloenv		for closures, the environment of the closure.
 *	sysparent	the environment the closure was called from
 *	conexit		code for on.exit calls, to be executed in cloenv
 *			at exit from the closure (normal or abnormal).
 *	cend		a pointer to function which executes if there is
 *			non-local return (i.e. an error)
 *	cenddata	a void pointer to data for cend to use
 *	vmax		the current setting of the R_alloc stack
 *	srcref		the srcref at the time of the call
 *
 *  Context types can be one of:
 *
 *	CTXT_TOPLEVEL	The toplevel context
 *	CTXT_BREAK	target for "break"
 *	CTXT_NEXT	target for "next"
 *	CTXT_LOOP	target for either "break" or "next"
 *	CTXT_RETURN	target for "return" (i.e. a closure)
 *	CTXT_BROWSER	target for "return" to exit from browser
 *	CTXT_CCODE	other functions that need clean up if an error occurs
 *	CTXT_RESTART	a function call to restart was made inside the
 *			closure.
 *
 *	Code (such as the sys.xxx) that looks for CTXT_RETURN must also
 *	look for a CTXT_RESTART and CTXT_GENERIC.
 *	The mechanism used by restart is to change
 *	the context type; error/errorcall then looks for a RESTART and does
 *	a long jump there if it finds one.
 *
 *  A context is created with a call to
 *
 *	void begincontext(RCNTXT *cptr, int flags,
 *			  SEXP syscall, SEXP env, SEXP
 *			  sysp, SEXP promargs, SEXP callfun)
 *
 *  which sets up the context pointed to by cptr in the appropriate way.
 *  When the context goes "out-of-scope" a call to
 *
 *	void endcontext(RCNTXT *cptr)
 *
 *  restores the previous context (i.e. it adjusts the R_GlobalContext
 *  pointer).
 *
 *  The non-local jump to a given context takes place in a call to
 *
 *	void findcontext(int mask, SEXP env, SEXP val)
 *
 *  This causes "val" to be stuffed into a globally accessable place and
 *  then a search to take place back through the context list for an
 *  appropriate context.  The kind of context sort is determined by the
 *  value of "mask".  The value of mask should be the logical OR of all
 *  the context types desired.
 *
 *  The value of "mask" is returned as the value of the setjump call at
 *  the level longjumped to.  This is used to distinguish between break
 *  and next actions.
 *
 *  Contexts can be used as a wrapper around functions that create windows
 *  or open files. These can then be shut/closed gracefully if an error
 *  occurs.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>

/* R_run_onexits - runs the conexit/cend code for all contexts from
   R_GlobalContext down to but not including the argument context.
   This routine does not stop at a CTXT_TOPLEVEL--the code that
   determines the argument is responsible for making sure
   CTXT_TOPLEVEL's are not crossed unless appropriate. */

void attribute_hidden R_run_onexits(RCNTXT *cptr)
{
  return;
}


/* R_restore_globals - restore global variables from a target context
   before a LONGJMP.  The target context itself is not restored here
   since this is done slightly differently in jumpfun below, in
   errors.c:jump_now, and in main.c:ParseBrowser.  Eventually these
   three should be unified so there is only one place where a LONGJMP
   occurs. */

void attribute_hidden R_restore_globals(RCNTXT *cptr)
{
  return;
}


/* jumpfun - jump to the named context */

static void jumpfun(RCNTXT * cptr, int mask, SEXP val)
{
  return;
}


/* begincontext - begin an execution context */

/* begincontext and endcontext are used in dataentry.c and modules */
void begincontext(RCNTXT * cptr, int flags,
		  SEXP syscall, SEXP env, SEXP sysp,
		  SEXP promargs, SEXP callfun)
{
  return;
}


/* endcontext - end an execution context */

void endcontext(RCNTXT * cptr)
{
  return;
}


/* findcontext - find the correct context */

void attribute_hidden findcontext(int mask, SEXP env, SEXP val)
{
  return;
}

void attribute_hidden R_JumpToContext(RCNTXT *target, int mask, SEXP val)
{
  return;
}


/* R_sysframe - look back up the context stack until the */
/* nth closure context and return that cloenv. */
/* R_sysframe(0) means the R_GlobalEnv environment */
/* negative n counts back from the current frame */
/* positive n counts up from the globalEnv */

SEXP attribute_hidden R_sysframe(int n, RCNTXT *cptr)
{
  return 0;
}


/* We need to find the environment that can be returned by sys.frame */
/* (so it needs to be on the cloenv pointer of a context) that matches */
/* the environment where the closure arguments are to be evaluated. */
/* It would be much simpler if sysparent just returned cptr->sysparent */
/* but then we wouldn't be compatible with S. */

int attribute_hidden R_sysparent(int n, RCNTXT *cptr)
{
  return 0;
}

int attribute_hidden framedepth(RCNTXT *cptr)
{
  return 0;
}

SEXP attribute_hidden R_syscall(int n, RCNTXT *cptr)
{
  return 0;
}

SEXP attribute_hidden R_sysfunction(int n, RCNTXT *cptr)
{
  return 0;
}

/* some real insanity to keep Duncan sane */

/* This should find the caller's environment (it's a .Internal) and
   then get the context of the call that owns the environment.  As it
   is, it will restart the wrong function if used in a promise.
   L.T. */
SEXP attribute_hidden do_restart(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* count how many contexts of the specified type are present on the stack */
/* browser contexts are a bit special because they are transient and for  */
/* any closure context with the debug bit set one will be created; so we  */
/* need to count those as well                                            */
int countContexts(int ctxttype, int browser) {
  return 0;
}
  
   
/* functions to support looking up information about the browser */
/* contexts that are in the evaluation stack */

SEXP attribute_hidden do_sysbrowser(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* An implementation of S's frame access functions. They usually count */
/* up from the globalEnv while we like to count down from the currentEnv. */
/* So if the argument is negative count down if positive count up. */
/* We don't want to count the closure that do_sys is contained in, so the */
/* indexing is adjusted to handle this. */

SEXP attribute_hidden do_sys(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_parentframe(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* R_ToplevelExec - call fun(data) within a top level context to
   insure that this functin cannot be left by a LONGJMP.  R errors in
   the call to fun will result in a jump to top level. The return
   value is TRUE if fun returns normally, FALSE if it results in a
   jump to top level. */

Rboolean R_ToplevelExec(void (*fun)(void *), void *data)
{
  return 0;
}



/*
  This is a simple interface for evaluating R expressions
  from C with a guarantee that one will return to the
  point in the code from which the call was made (if it does
  return at all).
  This uses R_TopleveExec to do this.  It is important
  in applications that embed R or wish to make general
  callbacks to R with error handling.

  It is currently hidden with a data structure definition
  and C routine visible only here. The R_tryEval() is the
  only visible aspect. This can be lifted into the header
  files if necessary. (DTL)

  R_tryEval is in Rinternals.h (so public), but not in the API.
 */
typedef struct {
    SEXP expression;
    SEXP val;
    SEXP env;
} ProtectedEvalData;

static void
protectedEval(void *d)
{
  return;
}

SEXP
R_tryEval(SEXP e, SEXP env, int *ErrorOccurred)
{
  return 0;
}

/* Temporary hack to suppress error message printing around a
   R_tryEval call for use in methods_list_dispatch.c; should be
   replaced once we have a way of establishing error handlers from C
   code (probably would want a calling handler if we want to allow
   user-defined calling handlers to enter a debugger, for
   example). LT */
SEXP R_tryEvalSilent(SEXP e, SEXP env, int *ErrorOccurred)
{
  return 0;
}
