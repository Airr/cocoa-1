/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995-1996   Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997-2013   The R Core Team
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

#include <stdlib.h> /* for putenv */
#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>
#include <R_ext/Riconv.h>
#include <Rinterface.h>
#include <errno.h>

/*
  See ../unix/system.txt for a description of some of these functions.
  Formally part of ../unix/sys-common.c.
 */

/*
 * FILESYSTEM INTERACTION
 */

/*
 * This call provides a simple interface to the "stat" system call.
 */

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#ifdef HAVE_AQUA
int (*ptr_CocoaSystem)(const char*);
extern	Rboolean useaqua;
#endif

#ifdef Win32
Rboolean R_FileExists(const char *path)
{
  return 0;
}

double attribute_hidden R_FileMtime(const char *path)
{
  return 0;
}
#else
Rboolean R_FileExists(const char *path)
{
  return 0;
}

double attribute_hidden R_FileMtime(const char *path)
{
  return 0;
}
#endif

    /*
     *  Unix file names which begin with "." are invisible.
     */

Rboolean attribute_hidden R_HiddenFile(const char *name)
{
  return 0;
}

/* The MSVC runtime has a global to determine whether an unspecified
   file open is in text or binary mode.  We force explicit text mode
   here to avoid depending on that global, which may have been changed
   by user code (most likely in embedded applications of R).
*/

#ifdef Win32

static char * fixmode(const char *mode)
{
  return 0;
}

static wchar_t * wcfixmode(const wchar_t *mode)
{
  return 0;
}

#else
#define fixmode(mode) (mode)
#define wcfixmode(mode) (mode)
#endif

FILE *R_fopen(const char *filename, const char *mode)
{
  return 0;
}

/* The point of this function is to allow file names in foreign
   character sets.  On Unix-alikes in a UTF-8 locale all that is
   needed is to convert file names to UTF-8, since they will be stored
   in UTF-8.  For other locales, it seems that there is no way to specify
   a file name in UTF-8.

   On NT-based versions of Windows, file names are stored in 'Unicode'
   (UCS-2), and _wfopen is provided to access them by UCS-2 names.
*/

#if defined(Win32)

#define BSIZE 100000 
wchar_t *filenameToWchar(const SEXP fn, const Rboolean expand)
{
  return 0;
}

FILE *R_wfopen(const wchar_t *filename, const wchar_t *mode)
{
  return 0;
}


FILE *RC_fopen(const SEXP fn, const char *mode, const Rboolean expand)
{
  return 0;
}
#else
FILE *RC_fopen(const SEXP fn, const char *mode, const Rboolean expand)
{
  return 0;
}
#endif

/*
 *  SYSTEM INFORMATION
 */

	  /* The location of the R system files */

char *R_HomeDir(void)
{
  return 0;
}

/* This is a primitive (with no arguments) */
SEXP attribute_hidden do_interactive(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_tempdir(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}


SEXP attribute_hidden do_tempfile(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

FILE *R_popen(const char *command, const char *type)
{
  return 0;
}

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

int R_system(const char *command)
{
  return 0;
}

#if defined(__APPLE__)
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#else
extern char ** environ;
#endif

#ifdef Win32
/* _wenviron is declared in stdlib.h */
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h> /* _wgetenv etc */
#endif

SEXP attribute_hidden do_getenv(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

#ifdef Win32
static int Rwputenv(const wchar_t *nm, const wchar_t *val)
{
  return 0;
}
#elif !defined(HAVE_SETENV) && defined(HAVE_PUTENV)
static int Rputenv(const char *nm, const char *val)
{
  return 0;
}
#endif


SEXP attribute_hidden do_setenv(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_unsetenv(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

#include <iconv.h>

#ifdef HAVE_ICONVLIST
static unsigned int cnt;

static int
count_one (unsigned int namescount, const char * const *names, void *data)
{
  return 0;
}

static int
write_one (unsigned int namescount, const char * const *names, void *data)
{
  return 0;
}
#endif

#include "RBufferUtils.h"

/* iconv(x, from, to, sub, mark) */
SEXP attribute_hidden do_iconv(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

cetype_t getCharCE(SEXP x)
{
  return 0;
}


void * Riconv_open (const char* tocode, const char* fromcode)
{
    return 0;
}

/* Should be defined in config.h, but prior to 2.13.0 was only checked
   if the NLS was enabled  */
#ifndef ICONV_CONST
# define ICONV_CONST
#endif

size_t Riconv (void *cd, const char **inbuf, size_t *inbytesleft,
	       char **outbuf, size_t *outbytesleft)
{
  return 0;
}

int Riconv_close (void *cd)
{
  return 0;
}

static void *latin1_obj = NULL, *utf8_obj=NULL, *ucsmb_obj=NULL,
    *ucsutf8_obj=NULL;

const char *translateChar(SEXP x)
{
  return 0;
}

const char *translateChar0(SEXP x)
{
  return 0;
}

const char *translateCharUTF8(SEXP x)
{
  return 0;
}


#ifdef Win32
static const char TO_WCHAR[] = "UCS-2LE";
#else
# ifdef WORDS_BIGENDIAN
static const char TO_WCHAR[] = "UCS-4BE";
# else
static const char TO_WCHAR[] = "UCS-4LE";
# endif
#endif

static void *latin1_wobj = NULL, *utf8_wobj=NULL;

/* Translate from current encoding to wchar_t = UCS-2/4
   NB: that wchar_t is UCS-4 is an assumption, but not easy to avoid.
*/

attribute_hidden /* but not hidden on Windows, where it was used in tcltk.c */
const wchar_t *wtransChar(SEXP x)
{
  return 0;
}


extern void *Rf_AdobeSymbol2utf8(char* work, const char *c0, size_t nwork); /* from util.c */

const char *reEnc(const char *x, cetype_t ce_in, cetype_t ce_out, int subst)
{
  return 0;
}

void attribute_hidden
invalidate_cached_recodings(void)
{
}


#ifdef WORDS_BIGENDIAN
static const char UNICODE[] = "UCS-4BE";
#else
static const char UNICODE[] = "UCS-4LE";
#endif

/* used in gram.c and devX11.c */
size_t ucstomb(char *s, const unsigned int wc)
{
  return 0;
}

/* used in plot.c for non-UTF-8 MBCS */
size_t attribute_hidden
mbtoucs(unsigned int *wc, const char *s, size_t n)
{
  return 0;
}

/* made available for use in graphics devices */
size_t ucstoutf8(char *s, const unsigned int wc)
{
  return 0;
}

/* moved from src/unix/sys-unix.c and src/gnuwin32/extra.c */

#ifdef HAVE_STAT
# ifdef HAVE_ACCESS
#  ifdef HAVE_UNISTD_H
#   include <unistd.h>
#  endif
# endif

#ifdef Win32
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h> /* For GetShortPathName */
#endif

#if !defined(S_IFDIR) && defined(__S_IFDIR)
# define S_IFDIR __S_IFDIR
#endif

static int isDir(char *path)
{
  return 0;
}
#else
static int isDir(char *path)
{
  return 0;
}
#endif /* HAVE_STAT */

#if !HAVE_DECL_MKDTEMP
extern char * mkdtemp (char *template);
#endif

#ifdef Win32
# include <ctype.h>
#endif

void attribute_hidden InitTempDir()
{
  return;
}

char * R_tmpnam(const char * prefix, const char * tempdir)
{
  return 0;
}

/* NB for use with multicore: parent and all children share the same
   session directory and run in parallel.   
   So as from 2.14.1, we make sure getpic() is part of the process.
*/
char * R_tmpnam2(const char *prefix, const char *tempdir, const char *fileext)
{
    return 0;
}

SEXP attribute_hidden do_proctime(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

void attribute_hidden resetTimeLimits()
{
  return;
}

SEXP attribute_hidden
do_setTimeLimit(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden
do_setSessionTimeLimit(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* moved from character.c in 2.10.0: configure requires this */

#ifdef HAVE_GLOB_H
# include <glob.h>
#endif
#ifdef Win32
# include <dos_wglob.h>
# define globfree dos_wglobfree
# define glob_t wglob_t
#else
# ifndef GLOB_QUOTE
#  define GLOB_QUOTE 0
# endif
#endif
SEXP attribute_hidden do_glob(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
