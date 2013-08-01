/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--2013 The R Core Team
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


/* Notes on so-called 'Large File Support'

   The 'stat' structure returns a file size as 'off_t'.  On some
   32-bit systems this will fail if called on a file > 2GB.  On
   systems with LFS selected (see the notes in connections.c) the call
   is re-mapped to *stat64, which uses off64_t for the file size.

   file.info() returns file sizes as an R double.

   On Windows we need to remap for ourselves.  There are various
   versions of the 'stat' structure (some with 64-bit times and not
   available in the original MSVCRT.dll): we use _stati64 that simply
   replaces off_t by __int64_t.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Defn.h>
#include <Internal.h>
#include <Rinterface.h>
#include <Fileio.h>
#include <ctype.h>			/* toupper */
#include <time.h>			/* for ctime */

# include <errno.h>

/* Machine Constants */

static void
machar(int *ibeta, int *it, int *irnd, int *ngrd, int *machep, int *negep,
       int *iexp, int *minexp, int *maxexp, double *eps,
       double *epsneg, double *xmin, double *xmax);

static void Init_R_Machine(SEXP rho)
{
  return;
}


/*  Platform
 *
 *  Return various platform dependent strings.  This is similar to
 *  "Machine", but for strings rather than numerical values.  These
 *  two functions should probably be amalgamated.
 */
static const char  * const R_OSType = OSTYPE;
static const char  * const R_FileSep = FILESEP;

static void Init_R_Platform(SEXP rho)
{
  return;
}

void attribute_hidden Init_R_Variables(SEXP rho)
{
  return;
}

#ifdef HAVE_LANGINFO_CODESET
/* case-insensitive string comparison (needed for locale check) */
int static R_strieql(const char *a, const char *b)
{
  return 0;
}
#endif

#include <locale.h>
#ifdef HAVE_LANGINFO_CODESET
# include <langinfo.h>
#endif

/* retrieves information about the current locale and
   sets the corresponding variables (known_to_be_utf8,
   known_to_be_latin1, utf8locale, latin1locale and mbcslocale) */
void attribute_hidden R_check_locale(void)
{
  return;
}

/*  date
 *
 *  Return the current date in a standard format.  This uses standard
 *  POSIX calls which should be available on each platform.  We should
 *  perhaps check this in the configure script.
 */
/* BDR 2000/7/20.
 *  time and ctime are in fact ANSI C calls, so we don't check them.
 */
static char *R_Date(void)
{
  return 0;
}

SEXP attribute_hidden do_date(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/*  file.show
 *
 *  Display file(s) so that a user can view it.  The function calls
 *  "R_ShowFiles" which is a platform-dependent hook that arranges
 *  for the file(s) to be displayed.
 */

SEXP attribute_hidden do_fileshow(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/*  file.append
 *
 *  Given two vectors of file names as arguments and arranges for
 *  the second set of files to be appended to the first.
 */

#if defined(BUFSIZ) && (BUFSIZ > 512)
/* OS's buffer size in stdio.h, probably.
   Windows has 512, Solaris 1024, glibc 8192
 */
# define APPENDBUFSIZE BUFSIZ
#else
# define APPENDBUFSIZE 512
#endif

static int R_AppendFile(SEXP file1, SEXP file2)
{
  return 0;
}

SEXP attribute_hidden do_fileappend(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_filecreate(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_fileremove(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for symlink, getpid */
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#ifdef Win32
/* Mingw-w64 defines this to be 0x0502 */
#ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0500 /* for CreateHardLink */
#endif
#include <windows.h>
typedef BOOLEAN (WINAPI *PCSL)(LPWSTR, LPWSTR, DWORD);
static PCSL pCSL = NULL;
const char *formatError(DWORD res);  /* extra.c */
/* Windows does not have link(), but it does have CreateHardLink() on NTFS */
#undef HAVE_LINK
#define HAVE_LINK 1
/* Windows does not have symlink(), but >= Vista does have
   CreateSymbolicLink() on NTFS */
#undef HAVE_SYMLINK
#define HAVE_SYMLINK 1
#endif

/* the Win32 stuff here is not ready for release:

   (i) It needs Windows >= Vista
   (ii) It matters whether 'from' is a file or a dir, and we could only
   know if it exists already.
   (iii) This needs specific privileges which in general only Adminstrators
   have, and which many people report granting in the Policy Editor
   fails to work.
*/
SEXP attribute_hidden do_filesymlink(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}


SEXP attribute_hidden do_filelink(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#ifdef Win32
int Rwin_rename(char *from, char *to);  /* in src/gnuwin32/extra.c */
int Rwin_wrename(const wchar_t *from, const wchar_t *to);
#endif

SEXP attribute_hidden do_filerename(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

# if defined(Unix) && defined(HAVE_PWD_H) && defined(HAVE_GRP_H) \
  && defined(HAVE_GETPWUID) && defined(HAVE_GETGRGID)
#  include <pwd.h>
#  include <grp.h>
#  define UNIX_EXTRAS 1
# endif

#ifdef Win32
# ifndef SCS_64BIT_BINARY
#  define SCS_64BIT_BINARY 6
# endif
#endif

#if defined HAVE_STRUCT_STAT_ST_ATIM_TV_NSEC
# ifdef TYPEOF_STRUCT_STAT_ST_ATIM_IS_STRUCT_TIMESPEC
#  define STAT_TIMESPEC(st, st_xtim) ((st).st_xtim)
# else
#  define STAT_TIMESPEC_NS(st, st_xtim) ((st).st_xtim.tv_nsec)
# endif
#elif defined HAVE_STRUCT_STAT_ST_ATIMESPEC_TV_NSEC
# define STAT_TIMESPEC(st, st_xtim) ((st).st_xtim##espec)
#elif defined HAVE_STRUCT_STAT_ST_ATIMENSEC
# define STAT_TIMESPEC_NS(st, st_xtim) ((st).st_xtim##ensec)
#elif defined HAVE_STRUCT_STAT_ST_ATIM_ST__TIM_TV_NSEC
# define STAT_TIMESPEC_NS(st, st_xtim) ((st).st_xtim.st__tim.tv_nsec)
#endif

SEXP attribute_hidden do_fileinfo(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* No longer required by POSIX, but maybe on earlier OSes */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
#elif HAVE_SYS_NDIR_H
# include <sys/ndir.h>
#elif HAVE_SYS_DIR_H
# include <sys/dir.h>
#elif HAVE_NDIR_H
# include <ndir.h>
#endif

#define CBUFSIZE 2*PATH_MAX+1
static SEXP filename(const char *dir, const char *file)
{
  return 0;
}

#include <tre/tre.h>

static void
list_files(const char *dnp, const char *stem, int *count, SEXP *pans,
	   Rboolean allfiles, Rboolean recursive,
	   const regex_t *reg, int *countmax, PROTECT_INDEX idx,
	   Rboolean idirs, Rboolean allowdots)
{
    return;
}

SEXP attribute_hidden do_listfiles(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

static void list_dirs(const char *dnp, const char *stem, int *count,
		      SEXP *pans, int *countmax, PROTECT_INDEX idx,
		      Rboolean recursive)
{
  return;
}

SEXP attribute_hidden do_listdirs(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_Rhome(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#ifdef Win32
static Rboolean attribute_hidden R_WFileExists(const wchar_t *path)
{
  return 0;
}
#endif

SEXP attribute_hidden do_fileexists(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#define CHOOSEBUFSIZE 1024

#ifndef Win32
SEXP attribute_hidden do_filechoose(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}
#endif

/* needed for access, and perhaps for realpath */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef Win32
extern int winAccessW(const wchar_t *path, int mode);
#endif

/* we require 'access' as from 2.12.0 */
SEXP attribute_hidden do_fileaccess(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#ifdef Win32

static int R_rmdir(const wchar_t *dir)
{
  return 0;
}

/* Junctions and symbolic links are fundamentally reparse points, so
   apparently this is the way to detect them. */
static int isReparsePoint(const wchar_t *name)
{
  return 0;
}

static int delReparsePoint(const wchar_t *name)
{
  return 0;
}

static int R_unlink(wchar_t *name, int recursive, int force)
{
  return 0;
}

void R_CleanTempDir(void)
{
  return;
}
#else
static int R_unlink(const char *name, int recursive, int force)
{
  return 0;
}

/* for use under valgrind on OS X */
void attribute_hidden R_CleanTempDir2(void)
{
  return;
}

#endif


/* Note that wildcards are allowed in 'names' */
#ifdef Win32
# include <dos_wglob.h>
SEXP attribute_hidden do_unlink(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
#else
# if defined(HAVE_GLOB) && defined(HAVE_GLOB_H)
#  include <glob.h>
# endif

SEXP attribute_hidden do_unlink(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
#endif

#if 0
static void chmod_one(const char *name)
{
  return 0;
}

/* recursively fix up permissions: used for R CMD INSTALL and build.
   NB: this overrides umask. */
SEXP attribute_hidden do_dirchmod(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
#endif


SEXP attribute_hidden do_getlocale(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

extern void invalidate_cached_recodings(void);  /* from sysutils.c */

extern void resetICUcollator(void); /* from util.c */

/* Locale specs are always ASCII */
SEXP attribute_hidden do_setlocale(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}



SEXP attribute_hidden do_localeconv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* .Internal function for path.expand */
SEXP attribute_hidden do_pathexpand(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#ifdef Unix
static int var_R_can_use_X11 = -1;

extern Rboolean R_access_X11(void); /* from src/unix/X11.c */

static Rboolean R_can_use_X11(void)
{
  return 0;
}
#endif

/* only actually used on Unix */
SEXP attribute_hidden do_capabilitiesX11(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

SEXP attribute_hidden do_capabilities(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#if defined(HAVE_BSD_NETWORKING) && defined(HAVE_ARPA_INET_H)
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

SEXP attribute_hidden do_nsl(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}
#else
SEXP attribute_hidden do_nsl(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}
#endif

SEXP attribute_hidden do_sysgetpid(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}


/* NB: we save errno immediately after the call here.  This should not
  be necessary on a POSIX OS, but it is on Windows, where it seems
  that on some versions strerror itself changes errno (something
  allowed in C99 but disallowed in POSIX).  Also, something under
  warning() might set errno in a future version.
*/
#ifndef Win32
/* mkdir is defined in <sys/stat.h> */
SEXP attribute_hidden do_dircreate(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
#else /* Win32 */
#include <io.h> /* mkdir is defined here */
SEXP attribute_hidden do_dircreate(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
#endif

/* take file/dir 'name' in dir 'from' and copy it to 'to'
   'from', 'to' should have trailing path separator if needed.
*/
#ifdef Win32
static int do_copy(const wchar_t* from, const wchar_t* name, const wchar_t* to,
		   int over, int recursive, int perms, int depth)
{
  return 0;
}

/* file.copy(files, dir, over, recursive=TRUE, perms), only */
SEXP attribute_hidden do_filecopy(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#else

static int do_copy(const char* from, const char* name, const char* to,
		   int over, int recursive, int perms, int depth)
{
  return 0;
}

/* file.copy(files, dir, recursive), only */
SEXP attribute_hidden do_filecopy(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}
#endif

SEXP attribute_hidden do_l10n_info(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* do_normalizepath moved to util.c in R 2.13.0 */

SEXP attribute_hidden do_syschmod(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_sysumask(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_readlink(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}


SEXP attribute_hidden do_Cstack_info(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#ifdef Win32
static int winSetFileTime(const char *fn, time_t ftime)
{
  return 0;
}
#else
# ifdef HAVE_UTIMES
#  include <sys/time.h>
# elif defined(HAVE_UTIME)
#  include <utime.h>
# endif
#endif

SEXP attribute_hidden
do_setFileTime(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

#ifdef Win32
/* based on ideas in
   http://www.codeproject.com/KB/winsdk/junctionpoints.aspx
*/
typedef struct TMN_REPARSE_DATA_BUFFER
{
    DWORD  ReparseTag;
    WORD   ReparseDataLength;
    WORD   Reserved;
    WORD   SubstituteNameOffset;
    WORD   SubstituteNameLength;
    WORD   PrintNameOffset;
    WORD   PrintNameLength;
    WCHAR  PathBuffer[1024];
} TMN_REPARSE_DATA_BUFFER;

SEXP attribute_hidden do_mkjunction(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}
#endif

/* Formerly src/appl/machar.c:
 * void machar()  -- computes ALL `machine constants' at once.
 * -------------  -- compare with ../nmath/i1mach.c & ../nmath/d1mach.c
 *		     which use the C  <float.h> constants !
 *      algorithm 665, collected algorithms from acm.
 *      this work published in transactions on mathematical software,
 *      vol. 14, no. 4, pp. 303-311.
 *
 *  this fortran 77 subroutine is intended to determine the parameters
 *   of the floating-point arithmetic system specified below.  the
 *   determination of the first three uses an extension of an algorithm
 *   due to m. malcolm, cacm 15 (1972), pp. 949-951, incorporating some,
 *   but not all, of the improvements suggested by m. gentleman and s.
 *   marovich, cacm 17 (1974), pp. 276-277.  an earlier version of this
 *   program was published in the book software manual for the
 *   elementary functions by w. j. cody and w. waite, prentice-hall,
 *   englewood cliffs, nj, 1980.
 *
 *  the program as given here must be modified before compiling.  if
 *   a single (double) precision version is desired, change all
 *   occurrences of cs (  ) in columns 1 and 2 to blanks.
 *
 *  parameter values reported are as follows:
 *
 *       ibeta   - the radix for the floating-point representation
 *       it      - the number of base ibeta digits in the floating-point
 *                 significand
 *       irnd    - 0 if floating-point addition chops
 *                 1 if floating-point addition rounds, but not in the
 *                   ieee style
 *                 2 if floating-point addition rounds in the ieee style
 *                 3 if floating-point addition chops, and there is
 *                   partial underflow
 *                 4 if floating-point addition rounds, but not in the
 *                   ieee style, and there is partial underflow
 *                 5 if floating-point addition rounds in the ieee style,
 *                   and there is partial underflow
 *       ngrd    - the number of guard digits for multiplication with
 *                 truncating arithmetic.  it is
 *                 0 if floating-point arithmetic rounds, or if it
 *                   truncates and only  it  base  ibeta digits
 *                   participate in the post-normalization shift of the
 *                   floating-point significand in multiplication;
 *                 1 if floating-point arithmetic truncates and more
 *                   than  it  base  ibeta  digits participate in the
 *                   post-normalization shift of the floating-point
 *                   significand in multiplication.
 *       machep  - the largest negative integer such that
 *                 1.0+float(ibeta)**machep .ne. 1.0, except that
 *                 machep is bounded below by  -(it+3)
 *       negeps  - the largest negative integer such that
 *                 1.0-float(ibeta)**negeps .ne. 1.0, except that
 *                 negeps is bounded below by  -(it+3)
 *       iexp    - the number of bits (decimal places if ibeta = 10)
 *                 reserved for the representation of the exponent
 *                 (including the bias or sign) of a floating-point
 *                 number
 *       minexp  - the largest in magnitude negative integer such that
 *                 float(ibeta)**minexp is positive and normalized
 *       maxexp  - the smallest positive power of  beta  that overflows
 *       eps     - the smallest positive floating-point number such
 *                 that  1.0+eps .ne. 1.0. in particular, if either
 *                 ibeta = 2  or  irnd = 0, eps = float(ibeta)**machep.
 *                 otherwise,  eps = (float(ibeta)**machep)/2
 *       epsneg  - a small positive floating-point number such that
 *                 1.0-epsneg .ne. 1.0. in particular, if ibeta = 2
 *                 or  irnd = 0, epsneg = float(ibeta)**negeps.
 *                 otherwise,  epsneg = (ibeta**negeps)/2.  because
 *                 negeps is bounded below by -(it+3), epsneg may not
 *                 be the smallest number that can alter 1.0 by
 *                 subtraction.
 *       xmin    - the smallest non-vanishing normalized floating-point
 *                 power of the radix, i.e.,  xmin = float(ibeta)**minexp
 *       xmax    - the largest finite floating-point number.  in
 *                 particular  xmax = (1.0-epsneg)*float(ibeta)**maxexp
 *                 note - on some machines  xmax  will be only the
 *                 second, or perhaps third, largest number, being
 *                 too small by 1 or 2 units in the last digit of
 *                 the significand.
 *
 *     latest revision - april 20, 1987
 *
 *     author - w. j. cody
 *              argonne national laboratory
 *
 */


static void
machar(int *ibeta, int *it, int *irnd, int *ngrd, int *machep, int *negep,
       int *iexp, int *minexp, int *maxexp, double *eps,
       double *epsneg, double *xmin, double *xmax)
{
}
