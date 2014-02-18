/* simdiv  2009  Jody Hey, Sang Chul Choi and Yong Wang*/

#include "simdiv.h"
#undef GLOBVARS

/* functions included in this file 
 // miscellaneous
void shelltimeinfo(struct timeinfo *lptr, int length);
int cardinality(struct BSET x);
//debugging 
void myassert(unsigned short isit);
void *watch_clsc_malloc(int size);
void watch_clsc_free(void *p);
//string utilities 
void strdelete(char *s,int pos,int len);
char *nextnonspace(char *textline);
char *nextwhite(char *c);
// random number functions
double expo(double c);
long double ldouble_exp(long double value);
int pickpoisson(double param);
double gasdev();
double pickgamma(double beta);
double ran1(long *idum);
double gammln(double xx);
int poidev(double xm);
*/

/* FUNCTIONS */

/*
int cardinality(SET x){
   int count = 0;
   while (!isempty(x)) {
    x ^= (x & -x); ++count;
    }
 return(count);
} */
int
cardinality (struct BSET x)
{
  int count = 0, j;
  for (j = 0; j < SETUNIT; j++)
    {
      while (x.p[j] != emptyset)
        {
          x.p[j] ^= (x.p[j] & -x.p[j]);
          ++count;
        }
    }
  return (count);
}

void
shelltimeinfo (struct timeinfo *lptr, int length)
{
  double aln = 1.442695022, tiny = 1.0e-5;
  struct timeinfo t;
  static int nn, m, lognb2, i, j, k, l;
  lognb2 = (int) floor (log (length) * aln + tiny);
  m = length;
  for (nn = 1; nn <= lognb2; nn++)
    {
      m = m / 2;
      k = length - m;
      for (j = 0; j <= k - 1; j++)
        {
          i = j;
        reloop:
          l = i + m;
          if ((lptr + l)->timeofnode < (lptr + i)->timeofnode)
            {
              t = *(lptr + i);
              *(lptr + i) = *(lptr + l);
              *(lptr + l) = t;
              i = i - m;
              if (i >= 0)
                goto reloop;
            }
        }
    }
}                               /* shell */


/***********************/
/* DEBUGGING FUNCTIONS */
/***********************/

#ifdef USE_MYASSERT
void
myassert (unsigned short isit)
{
  if (!isit)
    {
      fclose (outfile);
      exit (-9);
    }
}
#endif


void dosim (void);
void finish (void);
int main (int argc, char *argv[]);


#ifdef EXTRADEBUG
void *
watch_clsc_malloc (int size)
{
  void *p;
  count_malloc++;
#define malloc(size) malloc(size)
  p = malloc (size);
#define malloc(size) watch_clsc_malloc(size)
  return (p);
}

void
watch_clsc_free (void *p)
{
  count_free++;
#define free free
  free (p);
#define free watch_clsc_free
}
#endif



/***********************/
/* STRING   FUNCTIONS */
/***********************/

/* Delete the substring of length "len" at index "pos" from "s".
   Delete less if out-of-range. */
void
strdelete (char *s, int pos, int len)
{
  int slen;
  if (--pos < 0)
    return;
  slen = (int) strlen (s) - pos;
  if (slen <= 0)
    return;
  s += pos;
  if (slen <= len)
    {
      *s = 0;
      return;
    }
  while ((*s = s[len]))
    s++;
}

char *
nextnonspace (char *textline)
/* finds the next non-space character after the next space */
{
  char *cc;
  if (textline == NULL)
    return NULL;
  cc = textline;
  while (*cc != ' ' && *cc != '\0')
    cc++;
  while (*cc == ' ')
    cc++;
  if (*cc == '\0')
    return NULL;
  else
    return cc;
}

/* find next whitespace, after next non-whitespace */
char *
nextwhite (char *c)
{
  int nonw;
  nonw = !isspace (*c);
  while (*c != '\0')
    {
      while ((nonw == 1 && !isspace (*c)) || (nonw == 0 && isspace (*c)))
        {
          c++;
          if (nonw == 0 && !isspace (*c))
            nonw = 1;
        }
      return c;
    }
  return c;
}                               /* nextwhite */

/***********************/
/* RANDOM NUMBER FUNCTIONS */
/***********************/


/* Generating an exponential*/
double
expo (double c)
{
  if (heylaboptions[BRANCHANDMUTATIONEXPECTIATONS])
    return 1.0 / c;
  else
    return -(1.0 / c) * log (ran1 (idum));
}

long double
ldouble_exp (long double value)
{
/* error trapping, and some code for handling
   an exponent up to about |1730| */
/* it can't really use extreme long double exponents */
#define ln10 2.3025850929940457
  double nval, dval, temp;
  long double output = 1.0;
  int i;
  if (absv (value) < 690.0)
    {
      return (exp (value));
    }
  else
    {
      /*   if (value > 690.0) return(5e299);
         if (value < -690.0) return(3e-300); */
      temp = value / ln10;
      dval = modf (temp, &nval);
      /* can only have a double constant
         so these values are not as extreme as should be */
      if (nval < -1730)
        return (1e-307);
      if (nval > 1730)
        return (1e308);
      if (nval > 0)
        {
          for (i = 1; i <= nval; i++)
            output *= 10.0;
        }
      else
        {
          for (i = -1; i >= nval; i--)
            output /= 10.0;
        }
      output *= (long double) exp (dval * ln10);
      return (output);
    }                           /*endif */
}

int
pickpoisson (double param)
{
  static long double randnum, raised;
  static int i;

  if (heylaboptions[BRANCHANDMUTATIONEXPECTIATONS])
    return INTEGERROUND (param);
  raised = ldouble_exp (-param);
  randnum = ran1 (idum);
  for (i = 0; randnum > raised; i++)
    {
      randnum *= ran1 (idum);
    }
  return (i);
}


double
gasdev ()
{
  static int iset = 0;
  static double gset;
  double fac, rsq, v1, v2;

  if (iset == 0)
    {
      do
        {
          v1 = 2.0 * ran1 (idum) - 1.0;
          v2 = 2.0 * ran1 (idum) - 1.0;
          rsq = v1 * v1 + v2 * v2;
        }
      while (rsq >= 1.0 || rsq == 0.0);
      fac = sqrt (-2.0 * log (rsq) / rsq);
      gset = v1 * fac;
      iset = 1;
      return v2 * fac;
    }
  else
    {
      iset = 0;
      return gset;
    }
}

double
gammln (double xx)
{
  double x, y, tmp, ser;
  static double cof[6] = { 76.18009172947146, -86.50532032941677,
    24.01409824083091, -1.231739572450155,
    0.1208650973866179e-2, -0.5395239384953e-5
  };
  int j;

  y = x = xx;
  tmp = x + 5.5;
  tmp -= (x + 0.5) * log (tmp);
  ser = 1.000000000190015;
  for (j = 0; j <= 5; j++)
    ser += cof[j] / ++y;
  return -tmp + log (2.5066282746310005 * ser / x);
}

#define PI 3.141592654

int
poidev (double xm)
{
  double gammln (double xx);
  double ran1 (long *idum);
  static double sq, alxm, g, oldm = (-1.0);
  double em, t, y;

  if (heylaboptions[BRANCHANDMUTATIONEXPECTIATONS])
    return INTEGERROUND (xm);
  if (xm < 12.0)
    {
      if (xm != oldm)
        {
          oldm = xm;
          g = exp (-xm);
        }
      em = -1;
      t = 1.0;
      do
        {
          ++em;
          t *= ran1 (idum);
        }
      while (t > g);
    }
  else
    {
      if (xm != oldm)
        {
          oldm = xm;
          sq = sqrt (2.0 * xm);
          alxm = log (xm);
          g = xm * alxm - gammln (xm + 1.0);
        }
      do
        {
          do
            {
              y = tan (PI * ran1 (idum));
              em = sq * y + xm;
            }
          while (em < 0.0);
          em = floor (em);
          t = 0.9 * (1.0 + y * y) * exp (em * alxm - gammln (em + 1.0) - g);
        }
      while (ran1 (idum) > t);
    }
//printf("xm %lf  em %i\n",xm, (int) em);
  return (int) em;
}

#undef PI

double
pickgamma (double beta)
{
  static double randnum, i;
  static double d, c, x, v, u;

  beta = fabs (beta);
  if (beta == 0)
    {
      printf ("beta can't be zero!\n");
      exit (0);
    }
  if (beta >= 1)
    {
      d = beta - 1.0 / 3.0;
      c = 1.0 / sqrt (9.0 * d);
      for (;;)
        {
          do
            {
              x = gasdev ();
              v = 1.0 + c * x;
            }
          while (v <= 0.);
          v = v * v * v;
          u = ran1 (idum);
          if (u < 1. - 0.331 * (x * x) * (x * x))
            return (d * v);
          if (log (u) < 0.5 * x * x + d * (1. - v + log (v)))
            return (d * v);
        }
    }
  else
    {
      return (pickgamma (1 + beta) * pow (ran1 (idum), 1 / beta));
    }
}

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-38
#define RNMX (1.0-EPS)
double
ran1 (long *idum)
{
  int j;
  long k;
  static long iy = 0;
  static long iv[NTAB];
  double temp;
  if (*idum <= 0 || !iy)
    {
      if (-(*idum) < 1)
        *idum = 1;
      else
        *idum = -(*idum);
      for (j = NTAB + 7; j >= 0; j--)
        {
          k = (*idum) / IQ;
          *idum = IA * (*idum - k * IQ) - IR * k;
          if (*idum < 0)
            *idum += IM;
          if (j < NTAB)
            iv[j] = *idum;
        }
      iy = iv[0];
    }
  k = (*idum) / IQ;
  *idum = IA * (*idum - k * IQ) - IR * k;
  if (*idum < 0)
    *idum += IM;
  j = iy / NDIV;
  iy = iv[j];
  iv[j] = *idum;
  if ((temp = AM * iy) > RNMX)
    return RNMX;
  else
    return temp;
}

#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX
