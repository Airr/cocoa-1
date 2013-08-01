/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995--2013  The R Core Team.
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
/* -> Errormsg.h */
#include <Startup.h> /* rather cleanup ..*/
#include <Rconnections.h>
#include <Rinterface.h>
#include <R_ext/GraphicsEngine.h> /* for GEonExit */
#include <Rmath.h> /* for imax2 */
#include <R_ext/Print.h>

#ifndef min
#define min(a, b) (a<b?a:b)
#endif

#if defined(__GNUC__) && __GNUC__ >= 3
#define NORET __attribute__((noreturn))
#else
#define NORET
#endif


/* Total line length, in chars, before splitting in warnings/errors */
#define LONGWARN 75

/*
 Different values of inError are used to indicate different places
 in the error handling.
 */
static int inError = 0;
static int inWarning = 0;
static int inPrintWarnings = 0;
static int immediateWarning = 0;

static void try_jump_to_restart(void);
// The next is crucial to the use of NORET attributes.
static void NORET
jump_to_top_ex(Rboolean, Rboolean, Rboolean, Rboolean, Rboolean);
static void signalInterrupt(void);
static char * R_ConciseTraceback(SEXP call, int skip);

/* Interface / Calling Hierarchy :
 
 R__stop()   -> do_error ->   errorcall --> (eventually) jump_to_top_ex
 /
 error
 
 R__warning()-> do_warning   -> warningcall -> if(warn >= 2) errorcall
 /
 warning /
 
 ErrorMessage()-> errorcall   (but with message from ErrorDB[])
 
 WarningMessage()-> warningcall (but with message from WarningDB[]).
 */

static void reset_stack_limit(void *data)
{
    return;
}

void R_CheckStack(void)
{
    return;
}

void R_CheckStack2(size_t extra)
{
    return;
}

void R_CheckUserInterrupt(void)
{
    return;
}

void onintr()
{
    return;
}

/* SIGUSR1: save and quit
 SIGUSR2: save and quit, don't run .Last or on.exit().
 
 These do far more processing than is allowed in a signal handler ....
 */

RETSIGTYPE attribute_hidden onsigusr1(int dummy)
{
}


RETSIGTYPE attribute_hidden onsigusr2(int dummy)
{
}


static void setupwarnings(void)
{
    return;
}

/* Rvsnprintf: like vsnprintf, but guaranteed to null-terminate. */
#ifdef Win32
int trio_vsnprintf(char *buffer, size_t bufferSize, const char *format,
                   va_list args);

static int Rvsnprintf(char *buf, size_t size, const char  *format, va_list ap)
{
    return 0;
}
#else
static int Rvsnprintf(char *buf, size_t size, const char  *format, va_list ap)
{
    return 0;
}
#endif

#define BUFSIZE 8192
void warning(const char *format, ...)
{
    return;
}

/* declarations for internal condition handling */

static void vsignalError(SEXP call, const char *format, va_list ap);
static void vsignalWarning(SEXP call, const char *format, va_list ap);
static void invokeRestart(SEXP, SEXP);

static void reset_inWarning(void *data)
{
    return;
}

#include <rlocale.h>

static int wd(const char * buf)
{
    return 0;
}

static void vwarningcall_dflt(SEXP call, const char *format, va_list ap)
{
    return;
}

static void warningcall_dflt(SEXP call, const char *format,...)
{
    return;
}

void warningcall(SEXP call, const char *format, ...)
{
    return;
}

void warningcall_immediate(SEXP call, const char *format, ...)
{
    return;
}

static void cleanup_PrintWarnings(void *data)
{
    return;
}

attribute_hidden
void PrintWarnings(void)
{
    return;
}

/* Return a constructed source location (e.g. filename#123) from a srcref.  If the srcref
 is not valid "" will be returned.
 */

static SEXP GetSrcLoc(SEXP srcref)
{
    return 0;
}

static char errbuf[BUFSIZE];

const char *R_curErrorBuf() {
    return 0;
}

/* temporary hook to allow experimenting with alternate error mechanisms */
static void (*R_ErrorHook)(SEXP, char *) = NULL;

static void restore_inError(void *data)
{
    return;
}

static void NORET
verrorcall_dflt(SEXP call, const char *format, va_list ap)
{
}

static void NORET errorcall_dflt(SEXP call, const char *format,...)
{
}

#include <stdio.h>
#include <stdarg.h>
// See http://stackoverflow.com/questions/1056411/how-to-pass-variable-number-of-arguments-to-printf-sprintf
void NORET errorcall(SEXP call, const char *format,...)
{
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

SEXP attribute_hidden do_geterrmessage(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

void error(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

static void try_jump_to_restart(void)
{
    return;
}

/* Unwind the call stack in an orderly fashion */
/* calling the code installed by on.exit along the way */
/* and finally longjmping to the innermost TOPLEVEL context */

static void jump_to_top_ex(Rboolean traceback,
                           Rboolean tryUserHandler,
                           Rboolean processWarnings,
                           Rboolean resetConsole,
                           Rboolean ignoreRestartContexts)
{
}

void jump_to_toplevel()
{
    return;
}

/* #define DEBUG_GETTEXT 1 */

/* gettext(domain, string) */
SEXP attribute_hidden do_gettext(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* ngettext(n, msg1, msg2, domain) */
SEXP attribute_hidden do_ngettext(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/* bindtextdomain(domain, dirname) */
SEXP attribute_hidden do_bindtextdomain(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

static SEXP findCall(void)
{
    return 0;
}

SEXP attribute_hidden do_stop(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_warning(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* Error recovery for incorrect argument count error. */
attribute_hidden
void WrongArgCount(const char *s)
{
}


void UNIMPLEMENTED(const char *s)
{
    return;
}

/* ERROR_.. codes in Errormsg.h */
static struct {
    const R_ERROR code;
    const char* const format;
}
const ErrorDB[] = {
    { ERROR_NUMARGS,		N_("invalid number of arguments")	},
    { ERROR_ARGTYPE,		N_("invalid argument type")		},
    
    { ERROR_TSVEC_MISMATCH,	N_("time-series/vector length mismatch")},
    { ERROR_INCOMPAT_ARGS,	N_("incompatible arguments")		},
    
    { ERROR_UNIMPLEMENTED,	N_("unimplemented feature in %s")	},
    { ERROR_UNKNOWN,		N_("unknown error (report this!)")	}
};

static struct {
    R_WARNING code;
    char* format;
}
WarningDB[] = {
    { WARNING_coerce_NA,	N_("NAs introduced by coercion")	},
    { WARNING_coerce_INACC,	N_("inaccurate integer conversion in coercion")},
    { WARNING_coerce_IMAG,	N_("imaginary parts discarded in coercion") },
    
    { WARNING_UNKNOWN,		N_("unknown warning (report this!)")	},
};


attribute_hidden
void ErrorMessage(SEXP call, int which_error, ...)
{
    return;
}

attribute_hidden
void WarningMessage(SEXP call, R_WARNING which_warn, ...)
{
    return;
}

#ifdef UNUSED
/* temporary hook to allow experimenting with alternate warning mechanisms */
static void (*R_WarningHook)(SEXP, char *) = NULL;

void R_SetWarningHook(void (*hook)(SEXP, char *))
{
    return;
}

void R_SetErrorHook(void (*hook)(SEXP, char *))
{
    return;
}

void R_ReturnOrRestart(SEXP val, SEXP env, Rboolean restart)
{
    return;
}

void R_JumpToToplevel(Rboolean restart)
{
    return;
}
#endif

static void R_SetErrmessage(const char *s)
{
    return;
}

static void R_PrintDeferredWarnings(void)
{
    return;
}

attribute_hidden
SEXP R_GetTraceback(int skip)
{
    return 0;
}

SEXP attribute_hidden do_traceback(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

static char * R_ConciseTraceback(SEXP call, int skip)
{
    return 0;
}



static SEXP mkHandlerEntry(SEXP klass, SEXP parentenv, SEXP handler, SEXP rho,
                           SEXP result, int calling)
{
    return 0;
}

/**** rename these??*/
#define IS_CALLING_ENTRY(e) LEVELS(e)
#define ENTRY_CLASS(e) VECTOR_ELT(e, 0)
#define ENTRY_CALLING_ENVIR(e) VECTOR_ELT(e, 1)
#define ENTRY_HANDLER(e) VECTOR_ELT(e, 2)
#define ENTRY_TARGET_ENVIR(e) VECTOR_ELT(e, 3)
#define ENTRY_RETURN_RESULT(e) VECTOR_ELT(e, 4)

#define RESULT_SIZE 3

SEXP attribute_hidden do_addCondHands(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_resetCondHands(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

static SEXP findSimpleErrorHandler(void)
{
    return 0;
}

static void vsignalWarning(SEXP call, const char *format, va_list ap)
{
    return;
}

static void gotoExitingHandler(SEXP cond, SEXP call, SEXP entry)
{
    return;
}

static void vsignalError(SEXP call, const char *format, va_list ap)
{
    return;
}

static SEXP findConditionHandler(SEXP cond)
{
    return 0;
}

SEXP attribute_hidden do_signalCondition(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

static SEXP findInterruptHandler(void)
{
    return 0;
}

static SEXP getInterruptCondition(void)
{
    return 0;
}

static void signalInterrupt(void)
{
    return;
}

void attribute_hidden
R_InsertRestartHandlers(RCNTXT *cptr, Rboolean browser)
{
}

SEXP attribute_hidden do_dfltWarn(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_dfltStop(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/*
 * Restart Handling
 */

SEXP attribute_hidden do_getRestart(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* very minimal error checking --just enough to avoid a segfault */
#define CHECK_RESTART(r) do { \
SEXP __r__ = (r); \
if (TYPEOF(__r__) != VECSXP || LENGTH(__r__) < 2) \
error(_("bad restart")); \
} while (0)

SEXP attribute_hidden do_addRestart(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

#define RESTART_EXIT(r) VECTOR_ELT(r, 1)

static void invokeRestart(SEXP r, SEXP arglist)
{
    return;
}

SEXP attribute_hidden do_invokeRestart(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_addTryHandlers(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_seterrmessage(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

SEXP attribute_hidden
do_printDeferredWarnings(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

SEXP attribute_hidden
do_interruptsSuspended(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

/* These functions are to be used in error messages, and available for others to use in the API
 GetCurrentSrcref returns the first non-NULL srcref after skipping skip of them.  If it
 doesn't find one it returns NULL. */

SEXP
R_GetCurrentSrcref(int skip)
{
    return 0;
}

/* Return the filename corresponding to a srcref, or "" if none is found */

SEXP 
R_GetSrcFilename(SEXP srcref)
{
    return 0;
}

