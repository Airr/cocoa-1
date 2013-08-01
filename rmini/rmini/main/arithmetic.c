/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996, 1997  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--2012	    The R Core Team.
 *  Copyright (C) 2003-4	    The R Foundation
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


/* interval at which to check interrupts, a guess */
#define NINTERRUPT 10000000


#ifdef __OpenBSD__
/* for definition of "struct exception" in math.h */
# define __LIBM_PRIVATE
#endif
#include <Defn.h>		/*-> Arith.h -> math.h */
#ifdef __OpenBSD__
# undef __LIBM_PRIVATE
#endif

#include <Internal.h>

#define R_MSG_NA	_("NaNs produced")
#define R_MSG_NONNUM_MATH _("non-numeric argument to mathematical function")

#include <Rmath.h>
extern double Rf_gamma_cody(double);

#include "arithmetic.h"

#include <errno.h>

#ifdef HAVE_MATHERR

/* Override the SVID matherr function:
 the main difference here is not to print warnings.
 */
#ifndef __cplusplus
int matherr(struct exception *exc)
{
    return 0;
}
#endif
#endif

#ifndef _AIX
static const double R_Zero_Hack = 0.0;	/* Silence the Sun compiler */
#else
static double R_Zero_Hack = 0.0;
#endif
typedef union
{
    double value;
    unsigned int word[2];
} ieee_double;

/* gcc had problems with static const on AIX and Solaris
 Solaris was for gcc 3.1 and 3.2 under -O2 32-bit on 64-bit kernel */
#ifdef _AIX
#define CONST
#elif defined(sparc) && defined (__GNUC__) && __GNUC__ == 3
#define CONST
#else
#define CONST const
#endif

#ifdef WORDS_BIGENDIAN
static CONST int hw = 0;
static CONST int lw = 1;
#else  /* !WORDS_BIGENDIAN */
static CONST int hw = 1;
static CONST int lw = 0;
#endif /* WORDS_BIGENDIAN */


static double R_ValueOfNA(void)
{
    return 0;
}

int R_IsNA(double x)
{
    return 0;
}

int R_IsNaN(double x)
{
    return 0;
}

/* ISNAN uses isnan, which is undefined by C++ headers
 This workaround is called only when ISNAN() is used
 in a user code in a file with __cplusplus defined */

int R_isnancpp(double x)
{
    return 0;
}


/* Mainly for use in packages */
int R_finite(double x)
{
    return 0;
}


/* Arithmetic Initialization */

void attribute_hidden InitArithmetic()
{
    return;
}

/* Keep these two in step */
/* FIXME: consider using
 tmp = (LDOUBLE)x1 - floor(q) * (LDOUBLE)x2;
 */
static double myfmod(double x1, double x2)
{
    return 0;
}

static double myfloor(double x1, double x2)
{
    return 0;
}

/* some systems get this wrong, possibly depend on what libs are loaded */
static R_INLINE double R_log(double x) {
    return 0;
}

double R_pow(double x, double y) /* = x ^ y */
{
    return 0;
}

static R_INLINE double R_POW(double x, double y) /* handle x ^ 2 inline */
{
    return 0;
}

double R_pow_di(double x, int n)
{
    return 0;
}


/* General Base Logarithms */

/* Note that the behaviour of log(0) required is not necessarily that
 mandated by C99 (-HUGE_VAL), and the behaviour of log(x < 0) is
 optional in C99.  Some systems return -Inf for log(x < 0), e.g.
 libsunmath on Solaris.
 */
static double logbase(double x, double base)
{
    return 0;
}

SEXP R_unary(SEXP, SEXP, SEXP);
SEXP R_binary(SEXP, SEXP, SEXP, SEXP);
static SEXP integer_unary(ARITHOP_TYPE, SEXP, SEXP);
static SEXP real_unary(ARITHOP_TYPE, SEXP, SEXP);
static SEXP real_binary(ARITHOP_TYPE, SEXP, SEXP);
static SEXP integer_binary(ARITHOP_TYPE, SEXP, SEXP, SEXP);

#if 0
static int naflag;
static SEXP lcall;
#endif


/* Unary and Binary Operators */

SEXP attribute_hidden do_arith(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

#define COERCE_IF_NEEDED(v, tp, vpi) do { \
if (TYPEOF(v) != (tp)) { \
int __vo__ = OBJECT(v); \
REPROTECT(v = coerceVector(v, (tp)), vpi); \
if (__vo__) SET_OBJECT(v, 1); \
} \
} while (0)

#define FIXUP_NULL_AND_CHECK_TYPES(v, vpi) do { \
switch (TYPEOF(v)) { \
case NILSXP: REPROTECT(v = allocVector(REALSXP,0), vpi); break; \
case CPLXSXP: case REALSXP: case INTSXP: case LGLSXP: break; \
default: errorcall(lcall, _("non-numeric argument to binary operator")); \
} \
} while (0)

SEXP attribute_hidden R_binary(SEXP call, SEXP op, SEXP x, SEXP y)
{
    return 0;
}

SEXP attribute_hidden R_unary(SEXP call, SEXP op, SEXP s1)
{
    return 0;
}

static SEXP integer_unary(ARITHOP_TYPE code, SEXP s1, SEXP call)
{
    return 0;
}

static SEXP real_unary(ARITHOP_TYPE code, SEXP s1, SEXP lcall)
{
    return 0;
}

/* i1 = i % n1; i2 = i % n2;
 * this macro is quite a bit faster than having real modulo calls
 * in the loop (tested on Intel and Sparc)
 */
#define mod_iterate(n1,n2,i1,i2) for (i=i1=i2=0; i<n; \
i1 = (++i1 == n1) ? 0 : i1,\
i2 = (++i2 == n2) ? 0 : i2,\
++i)



/* The tests using integer comparisons are a bit faster than the tests
 using doubles, but they depend on a two's complement representation
 (but that is almost universal).  The tests that compare results to
 double's depend on being able to accurately represent all int's as
 double's.  Since int's are almost universally 32 bit that should be
 OK. */

#ifndef INT_32_BITS
/* configure checks whether int is 32 bits.  If not this code will
 need to be rewritten.  Since 32 bit ints are pretty much universal,
 we can worry about writing alternate code when the need arises.
 To be safe, we signal a compiler error if int is not 32 bits. */
# error code requires that int have 32 bits
#else
/* Just to be on the safe side, configure ought to check that the
 mashine uses two's complement. A define like
 #define USES_TWOS_COMPLEMENT (~0 == (unsigned) -1)
 might work, but at least one compiler (CodeWarrior 6) chokes on it.
 So for now just assume it is true.
 */
#define USES_TWOS_COMPLEMENT 1

#if USES_TWOS_COMPLEMENT
# define OPPOSITE_SIGNS(x, y) ((x < 0) ^ (y < 0))
# define GOODISUM(x, y, z) (((x) > 0) ? ((y) < (z)) : ! ((y) < (z)))
# define GOODIDIFF(x, y, z) (!(OPPOSITE_SIGNS(x, y) && OPPOSITE_SIGNS(x, z)))
#else
# define GOODISUM(x, y, z) ((double) (x) + (double) (y) == (z))
# define GOODIDIFF(x, y, z) ((double) (x) - (double) (y) == (z))
#endif
#define GOODIPROD(x, y, z) ((double) (x) * (double) (y) == (z))
#define INTEGER_OVERFLOW_WARNING _("NAs produced by integer overflow")
#endif

static SEXP integer_binary(ARITHOP_TYPE code, SEXP s1, SEXP s2, SEXP lcall)
{
    return 0;
}

#define R_INTEGER(robj, i) (double) (INTEGER(robj)[i] == NA_INTEGER ? NA_REAL : INTEGER(robj)[i])

static SEXP real_binary(ARITHOP_TYPE code, SEXP s1, SEXP s2)
{
    return 0;
}


/* Mathematical Functions of One Argument */

static SEXP math1(SEXP sa, double(*f)(double), SEXP lcall)
{
    return 0;
}


SEXP attribute_hidden do_math1(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

/* methods are allowed to have more than one arg */
SEXP attribute_hidden do_trunc(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

/*
 Note that this is slightly different from the do_math1 set,
 both for integer/logical inputs and what it dispatches to for complex ones.
 */

SEXP attribute_hidden do_abs(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

/* Mathematical Functions of Two Numeric Arguments (plus 1 int) */

/* math2_1 and math2_2 and related can be removed  once the byte
 compiler knows how to optimize to .External rather than
 .Internal */

#define if_NA_Math2_set(y,a,b)				\
if      (ISNA (a) || ISNA (b)) y = NA_REAL;	\
else if (ISNAN(a) || ISNAN(b)) y = R_NaN;

static SEXP math2(SEXP sa, SEXP sb, double (*f)(double, double),
                  SEXP lcall)
{
    return 0;
} /* math2() */

static SEXP math2_1(SEXP sa, SEXP sb, SEXP sI,
                    double (*f)(double, double, int), SEXP lcall)
{
    return 0;
} /* math2_1() */

static SEXP math2_2(SEXP sa, SEXP sb, SEXP sI1, SEXP sI2,
                    double (*f)(double, double, int, int), SEXP lcall)
{
    return 0;
} /* math2_2() */

static SEXP math2B(SEXP sa, SEXP sb, double (*f)(double, double, double *),
                   SEXP lcall)
{
    return 0;
} /* math2B() */

#define Math2(A, FUN)	  math2(CAR(A), CADR(A), FUN, call);
#define Math2_1(A, FUN)	math2_1(CAR(A), CADR(A), CADDR(A), FUN, call);
#define Math2_2(A, FUN) math2_2(CAR(A), CADR(A), CADDR(A), CADDDR(A), FUN, call)
#define Math2B(A, FUN)	  math2B(CAR(A), CADR(A), FUN, call);

SEXP attribute_hidden do_math2(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}


/* The S4 Math2 group, round and signif */
/* This is a primitive SPECIALSXP with internal argument matching */
SEXP attribute_hidden do_Math2(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

/* log{2,10} are builtins */
SEXP attribute_hidden do_log1arg(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}


/* This is a primitive SPECIALSXP with internal argument matching */
SEXP attribute_hidden do_log(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}


/* Mathematical Functions of Three (Real) Arguments */

/* math3_1 and math3_2 and related can be removed once the byte
 compiler knows how to optimize to .External rather than
 .Internal */


#define if_NA_Math3_set(y,a,b,c)			        \
if      (ISNA (a) || ISNA (b)|| ISNA (c)) y = NA_REAL;	\
else if (ISNAN(a) || ISNAN(b)|| ISNAN(c)) y = R_NaN;

#define mod_iterate3(n1,n2,n3,i1,i2,i3) for (i=i1=i2=i3=0; i<n; \
i1 = (++i1==n1) ? 0 : i1,				\
i2 = (++i2==n2) ? 0 : i2,				\
i3 = (++i3==n3) ? 0 : i3,				\
++i)

#define SETUP_Math3						\
if (!isNumeric(sa) || !isNumeric(sb) || !isNumeric(sc))	\
errorcall(lcall, R_MSG_NONNUM_MATH);			\
\
na = XLENGTH(sa);						\
nb = XLENGTH(sb);						\
nc = XLENGTH(sc);						\
if ((na == 0) || (nb == 0) || (nc == 0))			\
return(allocVector(REALSXP, 0));			\
n = na;							\
if (n < nb) n = nb;						\
if (n < nc) n = nc;						\
PROTECT(sa = coerceVector(sa, REALSXP));			\
PROTECT(sb = coerceVector(sb, REALSXP));			\
PROTECT(sc = coerceVector(sc, REALSXP));			\
PROTECT(sy = allocVector(REALSXP, n));			\
a = REAL(sa);						\
b = REAL(sb);						\
c = REAL(sc);						\
y = REAL(sy);						\
naflag = 0

#define FINISH_Math3				\
if(naflag) warning(R_MSG_NA);		\
\
if (n == na) DUPLICATE_ATTRIB(sy, sa);	\
else if (n == nb) DUPLICATE_ATTRIB(sy, sb);	\
else if (n == nc) DUPLICATE_ATTRIB(sy, sc);	\
UNPROTECT(4)

static SEXP math3_1(SEXP sa, SEXP sb, SEXP sc, SEXP sI,
                    double (*f)(double, double, double, int), SEXP lcall)
{
    return 0;
} /* math3_1 */

static SEXP math3_2(SEXP sa, SEXP sb, SEXP sc, SEXP sI, SEXP sJ,
                    double (*f)(double, double, double, int, int), SEXP lcall)
{
    return 0;
} /* math3_2 */

static SEXP math3B(SEXP sa, SEXP sb, SEXP sc,
                   double (*f)(double, double, double, double *), SEXP lcall)
{
    return 0;
} /* math3B */

#define Math3_1(A, FUN)	math3_1(CAR(A), CADR(A), CADDR(A), CADDDR(A), FUN, call);
#define Math3_2(A, FUN) math3_2(CAR(A), CADR(A), CADDR(A), CADDDR(A), CAD4R(A), FUN, call)
#define Math3B(A, FUN)  math3B (CAR(A), CADR(A), CADDR(A), FUN, call);

SEXP attribute_hidden do_math3(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
} /* do_math3() */

/* Mathematical Functions of Four (Real) Arguments */

/* This can be removed completely once the byte compiler knows how to
 optimize to .External rather than .Internal */

#define if_NA_Math4_set(y,a,b,c,d)				\
if      (ISNA (a)|| ISNA (b)|| ISNA (c)|| ISNA (d)) y = NA_REAL;\
else if (ISNAN(a)|| ISNAN(b)|| ISNAN(c)|| ISNAN(d)) y = R_NaN;

#define mod_iterate4(n1,n2,n3,n4,i1,i2,i3,i4) for (i=i1=i2=i3=i4=0; i<n; \
i1 = (++i1==n1) ? 0 : i1,					\
i2 = (++i2==n2) ? 0 : i2,					\
i3 = (++i3==n3) ? 0 : i3,					\
i4 = (++i4==n4) ? 0 : i4,					\
++i)

static SEXP math4(SEXP sa, SEXP sb, SEXP sc, SEXP sd,
                  double (*f)(double, double, double, double), SEXP lcall)
{
    return 0;
} /* math4() */

static SEXP math4_1(SEXP sa, SEXP sb, SEXP sc, SEXP sd, SEXP sI, double (*f)(double, double, double, double, int), SEXP lcall)
{
    return 0;
} /* math4_1() */

static SEXP math4_2(SEXP sa, SEXP sb, SEXP sc, SEXP sd, SEXP sI, SEXP sJ,
                    double (*f)(double, double, double, double, int, int), SEXP lcall)
{
    return 0;
} /* math4_2() */


#define CAD3R	CADDDR
/* This is not (yet) in Rinternals.h : */
#define CAD5R(e)	CAR(CDR(CDR(CDR(CDR(CDR(e))))))

#define Math4(A, FUN)   math4  (CAR(A), CADR(A), CADDR(A), CAD3R(A), FUN, call)
#define Math4_1(A, FUN) math4_1(CAR(A), CADR(A), CADDR(A), CAD3R(A), CAD4R(A), \
FUN, call)
#define Math4_2(A, FUN) math4_2(CAR(A), CADR(A), CADDR(A), CAD3R(A), CAD4R(A), \
CAD5R(A), FUN, call)


SEXP attribute_hidden do_math4(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}


#ifdef WHEN_MATH5_IS_THERE/* as in ./arithmetic.h */

/* Mathematical Functions of Five (Real) Arguments */

#define if_NA_Math5_set(y,a,b,c,d,e)					\
if     (ISNA (a)|| ISNA (b)|| ISNA (c)|| ISNA (d)|| ISNA (e))	\
y = NA_REAL;						\
else if(ISNAN(a)|| ISNAN(b)|| ISNAN(c)|| ISNAN(d)|| ISNAN(e))	\
y = R_NaN;

#define mod_iterate5(n1,n2,n3,n4,n5, i1,i2,i3,i4,i5)	\
for (i=i1=i2=i3=i4=i5=0; i<n;				\
i1 = (++i1==n1) ? 0 : i1,			\
i2 = (++i2==n2) ? 0 : i2,			\
i3 = (++i3==n3) ? 0 : i3,			\
i4 = (++i4==n4) ? 0 : i4,			\
i5 = (++i5==n5) ? 0 : i5,			\
++i)

static SEXP math5(SEXP sa, SEXP sb, SEXP sc, SEXP sd, SEXP se, double (*f)())
#define SETUP_Math5							\
if (!isNumeric(sa) || !isNumeric(sb) || !isNumeric(sc) ||		\
!isNumeric(sd) || !isNumeric(se))				\
errorcall(lcall, R_MSG_NONNUM_MATH);				\
\
na = XLENGTH(sa);							\
nb = XLENGTH(sb);							\
nc = XLENGTH(sc);							\
nd = XLENGTH(sd);							\
ne = XLENGTH(se);							\
if ((na == 0) || (nb == 0) || (nc == 0) || (nd == 0) || (ne == 0))	\
return(allocVector(REALSXP, 0));				\
n = na;								\
if (n < nb) n = nb;							\
if (n < nc) n = nc;							\
if (n < nd) n = nd;							\
if (n < ne) n = ne;		/* n = max(na,nb,nc,nd,ne) */		\
PROTECT(sa = coerceVector(sa, REALSXP));				\
PROTECT(sb = coerceVector(sb, REALSXP));				\
PROTECT(sc = coerceVector(sc, REALSXP));				\
PROTECT(sd = coerceVector(sd, REALSXP));				\
PROTECT(se = coerceVector(se, REALSXP));				\
PROTECT(sy = allocVector(REALSXP, n));				\
a = REAL(sa);							\
b = REAL(sb);							\
c = REAL(sc);							\
d = REAL(sd);							\
e = REAL(se);							\
y = REAL(sy);							\
naflag = 0

SETUP_Math5;

mod_iterate5 (na, nb, nc, nd, ne,
              ia, ib, ic, id, ie) {
    //	if ((i+1) % NINTERRUPT == 0) R_CheckUserInterrupt();
	ai = a[ia];
	bi = b[ib];
	ci = c[ic];
	di = d[id];
	ei = e[ie];
	if_NA_Math5_set(y[i], ai,bi,ci,di,ei)
	else {
	    y[i] = f(ai, bi, ci, di, ei);
	    if (ISNAN(y[i])) naflag = 1;
	}
}

#define FINISH_Math5				\
if(naflag) warning(R_MSG_NA);		\
\
if (n == na) DUPLICATE_ATTRIB(sy, sa);	\
else if (n == nb) DUPLICATE_ATTRIB(sy, sb);	\
else if (n == nc) DUPLICATE_ATTRIB(sy, sc);	\
else if (n == nd) DUPLICATE_ATTRIB(sy, sd);	\
else if (n == ne) DUPLICATE_ATTRIB(sy, se);	\
UNPROTECT(6)

FINISH_Math5;

return sy;
} /* math5() */

#define Math5(A, FUN) \
math5(CAR(A), CADR(A), CADDR(A), CAD3R(A), CAD4R(A), FUN);

SEXP attribute_hidden do_math5(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
} /* do_math5() */

#endif /* Math5 is there */

/* This is used for experimenting with parallelized nmath functions -- LT */
CCODE R_get_arith_function(int which)
{
    return 0;
}
