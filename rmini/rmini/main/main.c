/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998-2013   The R Core Team
 *  Copyright (C) 2002-2005  The R Foundation
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

#include <math.h> /* avoid redefinition of extern in Defn.h */
#include <float.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define __MAIN__
#define R_USE_SIGNALS 1
#include "Defn.h"
#include <Internal.h>
#include "Rinterface.h"
#include "IOStuff.h"
#include "Fileio.h"
#include "Parse.h"
#include "Startup.h"

#include <locale.h>
#include <R_ext/Print.h>

#ifdef ENABLE_NLS
void attribute_hidden nl_Rdummy(void)
{
    return;
}
#endif


/* The 'real' main() program is in Rmain.c on Unix-alikes, and
 src/gnuwin/front-ends/graphappmain.c on Windows, unless of course
 R is embedded */

/* Global Variables:  For convenience, all interpeter global symbols
 * ================   are declared in Defn.h as extern -- and defined here.
 *
 * NOTE: This is done by using some preprocessor trickery.  If __MAIN__
 * is defined as above, there is a sneaky
 *     #define extern
 * so that the same code produces both declarations and definitions.
 *
 * This does not include user interface symbols which are included
 * in separate platform dependent modules.
 */

void Rf_callToplevelHandlers(SEXP expr, SEXP value, Rboolean succeeded,
                             Rboolean visible);

static int ParseBrowser(SEXP, SEXP);


extern void InitDynload(void);

/* Read-Eval-Print Loop [ =: REPL = repl ] with input from a file */

static void R_ReplFile(FILE *fp, SEXP rho)
{
    return;
}

/* Read-Eval-Print loop with interactive input */
static int prompt_type;
static char BrowsePrompt[20];

static const char *R_PromptString(int browselevel, int type)
{
    return 0;
}

/*
 This is a reorganization of the REPL (Read-Eval-Print Loop) to separate
 the loop from the actions of the body. The motivation is to make the iteration
 code (Rf_ReplIteration) available as a separately callable routine
 to avoid cutting and pasting it when one wants a single iteration
 of the loop. This is needed as we allow different implementations
 of event loops. Currently (summer 2002), we have a package in
 preparation that uses Rf_ReplIteration within either the
 Tcl or Gtk event loop and allows either (or both) loops to
 be used as a replacement for R's loop and take over the event
 handling for the R process.
 
 The modifications here are intended to leave the semantics of the REPL
 unchanged, just separate into routines. So the variables that maintain
 the state across iterations of the loop are organized into a structure
 and passed to Rf_ReplIteration() from Rf_ReplConsole().
 */


/**
 (local) Structure for maintaining and exchanging the state between
 Rf_ReplConsole and its worker routine Rf_ReplIteration which is the
 implementation of the body of the REPL.
 
 In the future, we may need to make this accessible to packages
 and so put it into one of the public R header files.
 */
typedef struct {
    ParseStatus    status;
    int            prompt_type;
    int            browselevel;
    unsigned char  buf[CONSOLE_BUFFER_SIZE+1];
    unsigned char *bufp;
} R_ReplState;


/**
 This is the body of the REPL.
 It attempts to parse the first line or expression of its input,
 and optionally request input from the user if none is available.
 If the input can be parsed correctly,
 i) the resulting expression is evaluated,
 ii) the result assigned to .Last.Value,
 iii) top-level task handlers are invoked.
 
 If the input cannot be parsed, i.e. there is a syntax error,
 it is incomplete, or we encounter an end-of-file, then we
 change the prompt accordingly.
 
 The "cursor" for the input buffer is moved to the next starting
 point, i.e. the end of the first line or after the first ;.
 */
int
Rf_ReplIteration(SEXP rho, int savestack, int browselevel, R_ReplState *state)
{
    return 0;
}

static void R_ReplConsole(SEXP rho, int savestack, int browselevel)
{
    return;
}


static unsigned char DLLbuf[CONSOLE_BUFFER_SIZE+1], *DLLbufp;

void R_ReplDLLinit(void)
{
    return;
}

/* FIXME: this should be re-written to use Rf_ReplIteration
 since it gets out of sync with it over time */
int R_ReplDLLdo1(void)
{
    return 0;
}

/* Main Loop: It is assumed that at this point that operating system */
/* specific tasks (dialog window creation etc) have been performed. */
/* We can now print a greeting, run the .First function and then enter */
/* the read-eval-print loop. */

static RETSIGTYPE handleInterrupt(int dummy)
{
    return;
}

/* this flag is set if R internal code is using send() and does not
 want to trigger an error on SIGPIPE (e.g., the httpd code).
 [It is safer and more portable than other methods of handling
 broken pipes on send().]
 */

#ifndef Win32
int R_ignore_SIGPIPE = 0;

static RETSIGTYPE handlePipe(int dummy)
{
    return;
}
#endif


#ifdef Win32
static int num_caught = 0;

static void win32_segv(int signum)
{
    return;
}
#endif

#if defined(HAVE_SIGALTSTACK) && defined(HAVE_SIGACTION) && defined(HAVE_WORKING_SIGACTION) && defined(HAVE_SIGEMPTYSET)

/* NB: this really isn't safe, but suffices for experimentation for now.
 In due course just set a flag and do this after the return.  OTOH,
 if we do want to bail out with a core dump, need to do that here.
 
 2005-12-17 BDR */

static unsigned char ConsoleBuf[CONSOLE_BUFFER_SIZE];
extern void R_CleanTempDir(void);

static void sigactionSegv(int signum, siginfo_t *ip, void *context)
{
    return;
}

#ifndef SIGSTKSZ
# define SIGSTKSZ 8192    /* just a guess */
#endif

#ifdef HAVE_STACK_T
static stack_t sigstk;
#else
static struct sigaltstack sigstk;
#endif
static void *signal_stack;

#define R_USAGE 100000 /* Just a guess */
static void init_signal_handlers(void)
{
    return;
}

#else /* not sigaltstack and sigaction and sigemptyset*/
static void init_signal_handlers(void)
{
    return;
}
#endif


static void R_LoadProfile(FILE *fparg, SEXP env)
{
    return;
}


int R_SignalHandlers = 1;  /* Exposed in R_interface.h */

unsigned int TimeToSeed(void); /* datetime.c */

const char* get_workspace_name();  /* from startup.c */

void attribute_hidden BindDomain(char *R_Home)
{
    return;
}

void setup_Rmainloop(void)
{
    // UnmarkedNodeTemplate
    // R_NilValue
    InitMemory();
    
    // R_UnboundValue
    // R_MissingArg
    // R_RestartToken
    // NA_STRING
    // R_BlankString
    InitNames();
}

void finalize_Rmainloop(void)
{
    FinalizeNames();
    FinalizeMemory();
}

extern SA_TYPE SaveAction; /* from src/main/startup.c */

static void end_Rmainloop(void)
{
    return;
}

void run_Rmainloop(void)
{
    return;
}

void mainloop(void)
{
    return;
}

/*this functionality now appears in 3
 places-jump_to_toplevel/profile/here */

static void printwhere(void)
{
    return;
}

static int ParseBrowser(SEXP CExpr, SEXP rho)
{
    return 0;
}


/* browser(text = "", condition = NULL, expr = TRUE, skipCalls = 0L) */
SEXP attribute_hidden do_browser(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

void R_dot_Last(void)
{
    return;
}

SEXP attribute_hidden do_quit(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


#include <R_ext/Callbacks.h>

static R_ToplevelCallbackEl *Rf_ToplevelTaskHandlers = NULL;

/**
 This is the C-level entry point for registering a handler
 that is to be called when each top-level task completes.
 
 Perhaps we need names to make removing them handlers easier
 since they could be more identified by an invariant (rather than
 position).
 */
R_ToplevelCallbackEl *
Rf_addTaskCallback(R_ToplevelCallback cb, void *data,
                   void (*finalizer)(void *), const char *name, int *pos)
{
    return 0;
}

Rboolean
Rf_removeTaskCallbackByName(const char *name)
{
    return 0;
}

/**
 Remove the top-level task handler/callback identified by
 its position in the list of callbacks.
 */
Rboolean
Rf_removeTaskCallbackByIndex(int id)
{
    return 0;
}


/**
 R-level entry point to remove an entry from the
 list of top-level callbacks. 'which' should be an
 integer and give us the 0-based index of the element
 to be removed from the list.
 
 @see Rf_RemoveToplevelCallbackByIndex(int)
 */
SEXP
R_removeTaskCallback(SEXP which)
{
    return 0;
}

SEXP
R_getTaskCallbackNames(void)
{
    return 0;
}

/**
 Invokes each of the different handlers giving the
 top-level expression that was just evaluated,
 the resulting value from the evaluation, and
 whether the task succeeded. The last may be useful
 if a handler is also called as part of the error handling.
 We also have information about whether the result was printed or not.
 We currently do not pass this to the handler.
 */

/* Flag to ensure that the top-level handlers aren't called recursively.
 Simple state to indicate that they are currently being run. */
static Rboolean Rf_RunningToplevelHandlers = FALSE;

/* This is not used in R and in no header */
void
Rf_callToplevelHandlers(SEXP expr, SEXP value, Rboolean succeeded,
                        Rboolean visible)
{
    return;
}


Rboolean
R_taskCallbackRoutine(SEXP expr, SEXP value, Rboolean succeeded,
                      Rboolean visible, void *userData)
{
    return 0;
}

SEXP
R_addTaskCallback(SEXP f, SEXP data, SEXP useData, SEXP name)
{
    return 0;
}

#undef __MAIN__

#ifndef Win32
/* this is here solely to pull in xxxpr.o */
#include <R_ext/RS.h>
void F77_SYMBOL(intpr) (const char *, int *, int *, int *);
void attribute_hidden dummy12345(void)
{
    return;
}

/* Used in unix/system.c, avoid inlining */
uintptr_t dummy_ii(void)
{
    return 0;
}
#endif
