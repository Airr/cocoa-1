/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996, 1997  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 2000-12	    The R Core Team.
 *  Copyright (C) 2005		    The R Foundation
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

/* Note: gcc may warn in several places about C99 features as extensions.
   This is a very-long-standing GCC bug, http://gcc.gnu.org/PR7263
   The system <complex.h> header can work around it: some do.
*/

#if 0
/* For testing substitute fns */
#undef HAVE_CARG
#undef HAVE_CABS
#undef HAVE_CPOW
#undef HAVE_CEXP
#undef HAVE_CLOG
#undef HAVE_CSQRT
#undef HAVE_CSIN
#undef HAVE_CCOS
#undef HAVE_CTAN
#undef HAVE_CASIN
#undef HAVE_CACOS
#undef HAVE_CATAN
#undef HAVE_CSINH
#undef HAVE_CCOSH
#undef HAVE_CTANH
#endif

#ifdef __CYGWIN__
/* as of 1.7.8 it had cacos, but it does not work */
#undef HAVE_CACOS
#endif

#ifdef __SUNPRO_C
/* segfaults in Solaris Studio 12.3 */
#undef HAVE_CPOW
#endif

#include <Defn.h>		/* -> ../include/R_ext/Complex.h */
#include <Internal.h>
#include <Rmath.h>

#include "arithmetic.h"		/* complex_*  */
#include <complex.h>

/* interval at which to check interrupts, a guess */
#define NINTERRUPT 10000000


/* GCC has problems with header files on e.g. Solaris.
   That OS defines the imaginary type, but GCC does not.
   Probably needed elsewhere, e.g. AIX, HP-UX (PR#15083)
   And use on Win32/64 suppresses warnings.
   The warning is also seen on Mac OS 10.5, but not later.
*/
#if defined(__GNUC__) && (defined(__sun__) || defined(__hpux__) || defined(Win32))
# undef  I
# define I (__extension__ 1.0iF)
#endif


/* 
   Note: this could use the C11 CMPLX() macro.
   As could mycpow, z_tan and some of the substitutes.
 */
static R_INLINE double complex toC99(Rcomplex *x)
{
  return 0;
}
#define C99_COMPLEX2(x, i) toC99(COMPLEX(x) + i)

static R_INLINE void 
SET_C99_COMPLEX(Rcomplex *x, R_xlen_t i, double complex value)
{
}

SEXP attribute_hidden complex_unary(ARITHOP_TYPE code, SEXP s1, SEXP call)
{
  return 0;
}

static R_INLINE double complex R_cpow_n(double complex X, int k)
{
  return 0;
}

#if defined(Win32)
# undef HAVE_CPOW
#endif
/* reason for this:
  1) X^n  (e.g. for n = +/- 2, 3) is unnecessarily inaccurate in glibc;
     cut-off 65536 : guided from empirical speed measurements

  2) On Mingw (but not Mingw-w64) the system cpow is explicitly linked
     against the (slow) MSVCRT pow, and gets (0+0i)^Y as 0+0i for all Y.

  3) PPC Mac OS X crashes on powers of 0+0i (at least under Rosetta).
  Really 0i^-1 should by Inf+NaNi, but getting that portably seems too hard.
  (C1x's CMPLX will eventually be possible.)
*/

static double complex mycpow (double complex X, double complex Y)
{
  return 0;
}

/* See arithmetic.c */
#define mod_iterate(n1,n2,i1,i2) for (i=i1=i2=0; i<n; \
	i1 = (++i1 == n1) ? 0 : i1,\
	i2 = (++i2 == n2) ? 0 : i2,\
	++i)

SEXP attribute_hidden complex_binary(ARITHOP_TYPE code, SEXP s1, SEXP s2)
{
  return 0;
}

SEXP attribute_hidden do_cmathfuns(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* used in format.c and printutils.c */
#define MAX_DIGITS 22
void attribute_hidden z_prec_r(Rcomplex *r, Rcomplex *x, double digits)
{
  return;
}

/* These substitute functions are rarely used, and not extensively
   tested, e.g. over CRAN.  Please do not change without very good
   reason!

   Currently (Feb 2011) they are used on FreeBSD.
*/

#ifndef HAVE_CLOG
#define clog R_clog
/* FIXME: maybe add full IEC60559 support */
static double complex clog(double complex x)
{
  return 0;
}
#endif

#ifndef HAVE_CSQRT
#define csqrt R_csqrt
/* FreeBSD does have this one */
static double complex csqrt(double complex x)
{
  return 0;
}
#endif

#ifndef HAVE_CEXP
#define cexp R_cexp
/* FIXME: check/add full IEC60559 support */
static double complex cexp(double complex x)
{
  return 0;
}
#endif

#ifndef HAVE_CCOS
#define ccos R_ccos
static double complex ccos(double complex x)
{
  return 0;
}
#endif

#ifndef HAVE_CSIN
#define csin R_csin
static double complex csin(double complex x)
{
  return 0;
}
#endif

#ifndef HAVE_CTAN
#define ctan R_ctan
static double complex ctan(double complex z)
{
  return 0;
}
#endif

#ifndef HAVE_CASIN
#define casin R_casin
static double complex casin(double complex z)
{
  return 0;
}
#endif

#ifndef HAVE_CACOS
#define cacos R_cacos
static double complex cacos(double complex z)
{
  return 0;
}
#endif

#ifndef HAVE_CATAN
#define catan R_catan
static double complex catan(double complex z)
{
  return 0;
}
#endif

#ifndef HAVE_CCOSH
#define ccosh R_ccosh
static double complex ccosh(double complex z)
{
  return 0;
}
#endif

#ifndef HAVE_CSINH
#define csinh R_csinh
static double complex csinh(double complex z)
{
  return 0;
}
#endif

#ifndef HAVE_CTANH
#define ctanh R_ctanh
static double complex ctanh(double complex z)
{
  return 0;
}
#endif

static double complex z_tan(double complex z)
{
  return 0;
}

/* Don't rely on the OS at the branch cuts */

static double complex z_asin(double complex z)
{
  return 0;
}

static double complex z_acos(double complex z)
{
  return 0;
}

static double complex z_atan(double complex z)
{
  return 0;
}

static double complex z_acosh(double complex z)
{
  return 0;
}

static double complex z_asinh(double complex z)
{
  return 0;
}

static double complex z_atanh(double complex z)
{
  return 0;
}

static Rboolean cmath1(double complex (*f)(double complex),
		       Rcomplex *x, Rcomplex *y, R_xlen_t n)
{
  return 0;
}

SEXP attribute_hidden complex_math1(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

static void z_rround(Rcomplex *r, Rcomplex *x, Rcomplex *p)
{
  return;
}

static void z_prec(Rcomplex *r, Rcomplex *x, Rcomplex *p)
{
  return;
}

static void z_logbase(Rcomplex *r, Rcomplex *z, Rcomplex *base)
{
  return;
}

static void z_atan2(Rcomplex *r, Rcomplex *csn, Rcomplex *ccs)
{
  return;
}


	/* Complex Functions of Two Arguments */

typedef void (*cm2_fun)(Rcomplex *, Rcomplex *, Rcomplex *);
SEXP attribute_hidden complex_math2(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

SEXP attribute_hidden do_complex(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

static void R_cpolyroot(double *opr, double *opi, int *degree,
			double *zeror, double *zeroi, Rboolean *fail);

SEXP attribute_hidden do_polyroot(SEXP call, SEXP op, SEXP args, SEXP rho)
{
  return 0;
}

/* Formerly src/appl/cpoly.c:
 *
 *  Copyright (C) 1997-1998 Ross Ihaka
 *  Copyright (C) 1999-2001 R Core Team
 *
 *	cpoly finds the zeros of a complex polynomial.
 *
 *	On Entry
 *
 *	opr, opi      -	 double precision vectors of real and
 *			 imaginary parts of the coefficients in
 *			 order of decreasing powers.
 *
 *	degree	      -	 int degree of polynomial.
 *
 *
 *	On Return
 *
 *	zeror, zeroi  -	 output double precision vectors of
 *			 real and imaginary parts of the zeros.
 *
 *	fail	      -	 output int parameter,	true  only if
 *			 leading coefficient is zero or if cpoly
 *			 has found fewer than degree zeros.
 *
 *	The program has been written to reduce the chance of overflow
 *	occurring. If it does occur, there is still a possibility that
 *	the zerofinder will work provided the overflowed quantity is
 *	replaced by a large number.
 *
 *	This is a C translation of the following.
 *
 *	TOMS Algorithm 419
 *	Jenkins and Traub.
 *	Comm. ACM 15 (1972) 97-99.
 *
 *	Ross Ihaka
 *	February 1997
 */

#include <R_ext/Arith.h> /* for declaration of hypot */
#include <R_ext/Memory.h> /* for declaration of R_alloc */

#include <float.h> /* for FLT_RADIX */

#include <Rmath.h> /* for R_pow_di */

static void calct(Rboolean *);
static Rboolean fxshft(int, double *, double *);
static Rboolean vrshft(int, double *, double *);
static void nexth(Rboolean);
static void noshft(int);

static void polyev(int, double, double,
		   double *, double *, double *, double *, double *, double *);
static double errev(int, double *, double *, double, double, double, double);
static double cpoly_cauchy(int, double *, double *);
static double cpoly_scale(int, double *, double, double, double, double);
static void cdivid(double, double, double, double, double *, double *);

/* Global Variables (too many!) */

static int nn;
static double *pr, *pi, *hr, *hi, *qpr, *qpi, *qhr, *qhi, *shr, *shi;
static double sr, si;
static double tr, ti;
static double pvr, pvi;

static const double eta =  DBL_EPSILON;
static const double are = /* eta = */DBL_EPSILON;
static const double mre = 2. * M_SQRT2 * /* eta, i.e. */DBL_EPSILON;
static const double infin = DBL_MAX;

static void R_cpolyroot(double *opr, double *opi, int *degree,
			double *zeror, double *zeroi, Rboolean *fail)
{
  return;
}


/*  Computes the derivative polynomial as the initial
 *  polynomial and computes l1 no-shift h polynomials.	*/

static void noshft(int l1)
{
  return;
}


/*  Computes l2 fixed-shift h polynomials and tests for convergence.
 *  initiates a variable-shift iteration and returns with the
 *  approximate zero if successful.
 */
static Rboolean fxshft(int l2, double *zr, double *zi)
{
  return 0;
}


/* carries out the third stage iteration.
 */
static Rboolean vrshft(int l3, double *zr, double *zi)
{
  return 0;
}

static void calct(Rboolean *bool)
{
  return;
}

static void nexth(Rboolean bool)
{
  return;
}

/*--------------------- Independent Complex Polynomial Utilities ----------*/

static
void polyev(int n,
	    double s_r, double s_i,
	    double *p_r, double *p_i,
	    double *q_r, double *q_i,
	    double *v_r, double *v_i)
{
  return;
}

static
double errev(int n, double *qr, double *qi,
	     double ms, double mp, double a_re, double m_re)
{
  return 0;
}


static
double cpoly_cauchy(int n, double *pot, double *q)
{
  return 0;
}

static
double cpoly_scale(int n, double *pot,
		   double eps, double BIG, double small, double base)
{
  return 0;
}


static
void cdivid(double ar, double ai, double br, double bi,
	    double *cr, double *ci)
{
  return;
}

/* static double cpoly_cmod(double *r, double *i)
 * --> replaced by hypot() everywhere
*/
