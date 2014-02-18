#line 18416 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "xtrapbits.h"
#include "uncertainty.h"

#line 18991 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int IMA_gdistanceRemoved (int n, int *Ai, int *Bi, int *perms, int nperm, char *Ci);
int IMA_gdistanceOrderNEW (int n, int *Ai, int *Bi, int m, int *od);
int IMA_gdistanceOrder (int n, int *Ai, int *Bi, int m, int *od);
int IMA_gdistance (int n, int *Ai, int *Bi);
int IMA_gdistanceSimple (int n, int *Ai, int *Bi);
int IMA_max_intarray (int n, int *A);

#line 18424 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int IMA_convertStr2Asn (int ncol, int *A, char *trueassignment);
int IMA_io_readp (char *fname, int ***m, int *mr, int *mc, int nburnin);
int IMA_gusfield_distanceRemoved (int *A, int **m, int nrow, int ncol, 
                                  int *perms, int nperm,
                                  double *r);
int IMA_gusfield_distance (int *A, int **m, int nrow, int ncol); 
int IMA_gusfield_squaredDistance (int *A, int **m, int nrow, int ncol); 
double IMA_gusfield_varianceDistance (int *A, int **m, int nrow, int ncol, double mean); 
static int skip_a_line (FILE *fp);
static int read_int (FILE *fp);
static double read_double (FILE *fp);
//int IMA_gdistanceRemoved (int n, int *Ai, int *Bi, char *Ci);
int IMA_gdistanceRemoved (int n, int *Ai, int *Bi, int *perms, int nperm, char *Ci);
int IMA_gdistance (int n, int *Ai, int *Bi);
int IMA_max_intarray (int n, int *A);
int IMA_permLexSuccessor (int n, int *p);
int IMA_permLexRank (int n, int *p);
int IMA_permLexUnrank (int *p, int n, int r);
int IMA_permFprintf (FILE *fp, int n, int *p);
int IMA_factorial (int n);

char *trueassignment;

int
main (int argc, char *argv[])
{
  char *outfilename;
  int nburnin;

  outfilename = NULL;
  trueassignment = NULL;
  if (argc == 2)
    {
      outfilename = argv[1];
      nburnin = 0;
    }
  else if (argc == 3)
    {
      outfilename = argv[1];
      nburnin = atoi (argv[2]);
    }
  else if (argc == 4)
    {
      outfilename = argv[1];
      nburnin = atoi (argv[2]);
      trueassignment = argv[3];
    }

  if (outfilename != NULL)
    {
      if (trueassignment != NULL)
        {
          if (*trueassignment == 'x')
            {
              imaIMTreeUncertainty (outfilename, ".asn", nburnin); /* FIXME: .in.p? */
            }
          else
            {
              imaStructuramaSummary (outfilename, nburnin);
            }
        }
      else
        {
          imaStructuramaUncertainty (outfilename, nburnin);
        }
    }

  return 0;
}

#line 18585 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
void 
imaStructuramaSummary (char *fname, int nburnin)
{
  FILE *fp;
  int lenpname;
  char *pname;
  int **m;
  int nrow;
  int ncol;
  int nsample;
  int i;
  int *A;
  int *B;
  int total_distance;
  double mean_distance;
  double normalized_distance;
  double variance_distance;
  double squared_distance;
  double meansquared_distance;
  int nind;
  int b_distance; /* distance between true and estimate */

  lenpname = 20 + strlen (fname);

  pname = malloc (lenpname * sizeof (char));
  sprintf (pname, "%s.in.p", fname);

  IMA_io_readp (pname, &m, &nrow, &ncol, nburnin);
  nsample = nrow - nburnin; 
  nind = strlen (trueassignment);
  if (nind == ncol)
    {
      A = malloc (ncol * sizeof (int));
      IMA_convertStr2Asn (ncol, A, trueassignment);

      total_distance = IMA_gusfield_distance (A, m, nsample, ncol); 
      mean_distance = ((double) total_distance) / ((double) nsample);
      normalized_distance = mean_distance / ncol;
      variance_distance = IMA_gusfield_varianceDistance (A, m, nsample, ncol, mean_distance); 
      squared_distance = IMA_gusfield_squaredDistance (A, m, nsample, ncol); 
      meansquared_distance = ((double) squared_distance) / ((double) nsample);
      sprintf (pname, "%s.sum", fname);
      fp = fopen (pname, "w");
      fprintf (fp, "%d\n", total_distance);
      fprintf (fp, "%g\n", mean_distance);
      fprintf (fp, "%g\n", normalized_distance);
      fprintf (fp, "%g\n", variance_distance);
      fprintf (fp, "%g\n", squared_distance);
      fprintf (fp, "%g\n", meansquared_distance);

      B = malloc (ncol * sizeof (int));
      sprintf (pname, "%s.in.sum_assignments", fname);
      IMA_io_readsumassignment (pname, ncol, B);
      b_distance = IMA_gdistance (ncol, A, B);
      fprintf (fp, "%d\n", b_distance);
      free (A);
      A = NULL;
      free (B);
      B = NULL;

      fclose (fp);
      fp = NULL;
    }
  free (A);
  A = NULL;
  for (i = 0; i < nsample; i++)
    {
      free (m[i]);
      m[i] = NULL;
    }
  free (m);
  m = NULL;
  free (pname);
  pname = NULL;
  return;
}

void 
IMA_io_readsumassignment (char *fname, int n, int *A)
{
  FILE *fp;
  char c;
  int i, d;
  
  fp = fopen (fname, "r");
  for (i = 0; i < 10; i++)
    {
      skip_a_line (fp);
    }
  for (i = 0; i < n; i++)
    {
      c = 'x';
      while (c != ')')
        {
          c = fgetc (fp);
        }
      d = read_int (fp);
      A[i] = d;
    }
  fclose (fp);
  fp = NULL;
 
  return;
}

#line 18697 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int
IMA_convertStr2Asn (int ncol, int *a, char *s)
{
  int len;
  int i;
  char str[2];

  len = strlen (s);
  assert (len == ncol);

  str[1] = '\0';
  for (i = 0; i < len; i++)
    {
      str[0] = s[i];
      a[i] = atoi (str);
    }
  return 1;
}

#line 18733 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
/*
static int
skip_a_line (FILE *fp)
{
  char c;
  c = 'x';
  while (c != '\n')
    {
      c = fgetc(fp);
    }
  return 0;
}
*/

int 
IMA_io_readp_header (char *fname, char **name, int nc)
{
  FILE *fp;
  char c;
  int i;
  int ii;
  char bf[100];
  
  fp = fopen (fname, "r");
  
  /* Count the number of columns. */
  i = 0;
  while (i < 2)
    {
      c = fgetc(fp);
      if (c == '\t')
        {
          i++;
        }
    }
  ii = 0;
  i = 0;
  while (c != '\n' && ii < nc)
    {
      c = fgetc(fp);
      if (c == '\t' || c == '\n')
        {
          bf[i++] = '\0';
          name[ii++] = strdup (bf);
          i = 0;
        }
      else
        {
          bf[i++] = c;
        }
    }
  fclose (fp);
  fp = NULL;
  return 0; 
}

int
IMA_io_readp (char *fname, int ***m, int *mr, int *mc, int nburnin)
{
  FILE *fp;
  char c;
  int ncol, nrow;
  int nsample;
  int i, j;
  double lnL;
  int gi;
  
  fp = fopen (fname, "r");
  
  /* Count the number of columns. */
  c = fgetc(fp);
  ncol = 1;
  while (c != '\n')
    {
      if (c == '\t')
        {
          ncol++;
        }
      c = fgetc(fp);
    }

#ifdef ONLYUNCERTAINTY /* Structurama run */
  ncol -= 3;
  nrow = -1;
#else
  ncol -= 2; /* for IM */
  nrow = -1;
#endif

  while (!feof (fp))
    {
      if (c == '\n')
        {
          nrow++;
        }
      c = fgetc(fp);      
    }
  
  nsample = nrow - nburnin;
  (*m) = malloc (nsample * sizeof (int *));
  for (i = 0; i < nsample; i++)
    {
      (*m)[i] = malloc (ncol * sizeof (int));
    }
  
  /* Reposition pointer to the beginning of the file. */
  fseek (fp, 0L, SEEK_SET);

#ifndef ONLYUNCERTAINTY /* Not for structurama run */
  skip_a_line (fp); /* for IM */
#endif

  for (i = 0; i < nburnin; i++)
    {
      skip_a_line (fp);
    } 

  /* FIXME: Check this out! */

  for (i = 0; i < nsample; i++)
    {
#ifdef ONLYUNCERTAINTY /* Structurama run */
      skip_a_line (fp);
#endif
      gi = read_int (fp);
      lnL = read_double (fp);
      for (j = 0; j < ncol; j++)
        {
          gi = read_int (fp);
          (*m)[i][j] = gi;
        }
    }
  
  fclose (fp);
  fp = NULL;
  
  *mr = nrow;
  *mc = ncol;
  /* fprintf (stdout, "ncol: %5d\nnrow: %5d\n", ncol, nrow); */
  return 0;
}

/* (nrow - nburnin) many samples are used. */
int
IMA_gusfield_distanceRemoved (int *A, int **m, int nrow, int ncol, 
                              int *perms, int nperm,
                              double *r)
{
  int d;
  int i, j;
  char *c;
  
  c = malloc (ncol * sizeof (char));
  d = 0;
  for (i = 0; i < nrow; i++)
    {
      d += IMA_gdistanceRemoved (ncol, A, m[i], perms, nperm, c);
      for (j = 0; j < ncol; j++)
        {
          if (c[j] == 'T')
            {
              r[j] += 1.0;
            }
        }
    }
  for (j = 0; j < ncol; j++)
    {
      r[j] /= ((double) nrow);
    }
  /* printf ("Burnin: %d, Used: %d\n", nburnin, nrow - nburnin);  */

  free (c);
  c = NULL;
  return d;
}

int
IMA_gusfield_distance (int *A, int **m, int nrow, int ncol)
{
  int d;
  int i;
  
  d = 0;
  for (i = 0; i < nrow; i++)
    {
      d += IMA_gdistance (ncol, A, m[i]);
    }
  return d;
}

int
IMA_gusfield_squaredDistance (int *A, int **m, int nrow, int ncol)
{
  int d, d1;
  int i;
  
  d = 0;
  for (i = 0; i < nrow; i++)
    {
      d1 = IMA_gdistance (ncol, A, m[i]);
      d1 *= d1;
      d += d1;
    }
  return d;
}


double
IMA_gusfield_varianceDistance (int *A, int **m, int nrow, int ncol, double mean)
{
  double v;
  double xi;
  int i;
  
  v = 0.0;
  for (i = 0; i < nrow; i++)
    {
      xi = (double) IMA_gdistance (ncol, A, m[i]);
      v += ((xi - mean) * (xi - mean));
    }
  v /= ((double) (nrow - 1.0)); 
  return v;
}

#line 19187 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int
IMA_gdistanceRemoved (int n, int *Ai, int *Bi, int *perms, int nperm, char *Ci)
{
  int rk; /* rank for the best permutation */
  int idummy;
  int i, j, k;
  int d, d1, d2;
  int r, s;
  UByteP *ar;
  UByteP *bs;
  UByteP c;
  int sP;
  int g;
  int *pi;
  int npi; /* number of permutations */
  int *A;
  int *B;

  if (perms == NULL)
    {
      idummy = nperm;
    }

  /* Find r and s. */
  r = IMA_max_intarray (n, Ai);
  s = IMA_max_intarray (n, Bi);
  if (r > s)
    {
      A = Bi;
      B = Ai;
      i = s;
      s = r;
      r = i;
    }
  else
    {
      A = Ai;
      B = Bi;
    }
  assert (!(r > s));
  sP = (n - 1) / 8 + 1;
  BitPowerSetNew (ar, sP, r, i);
  BitPowerSetNew (bs, sP, s, i);
  BitSetNew (c, sP);
  /* pi is initialized to be the first of permutations. */
  pi = malloc ((s + 1) * sizeof (int));
  for (i = 0; i < s + 1; i++)
    {
      pi[i] = i;
    }
  
  /* Set a and b. */
  for (i = 0; i < n; i++)
    {
      g = A[i];
      assert (!(g > r));
      BitTrue (ar[g - 1], i);
      g = B[i];
      assert (!(g > s));
      BitTrue (bs[g - 1], i);
    }
  
  d = n;
  npi = IMA_factorial(s);
  i = 0;
  rk = i;
  while (i++ < npi)
    {
      d2 = 0;
      for (j = 1; j < s + 1; j++)
        {
          k = pi[j] - 1;
          if (j < r + 1)
            {
              BitDifference (c, ar[j - 1], bs[k], sP, idummy);
              BitNTrues (d1, c, sP, idummy);
              d2 += d1;              
            }
        }
      if (d2 < d)
        {
          rk = i;
          d = d2;
        }
      IMA_permLexSuccessor(s, pi);
    }
  
  assert (rk > 0);
  IMA_permLexUnrank(pi, s, rk - 1);
  memset (Ci, 'F', n * sizeof (char));
  for (j = 1; j < s + 1; j++)
    {
      k = pi[j] - 1;
      if (j < r + 1)
        {
          BitDifference (c, ar[j - 1], bs[k], sP, idummy);
          for (i = 0; i < n; i++)
            {
              if (BitIsTrue (c, i))
                {
                  Ci[i] = 'T';
                }
            }          
        }          
    }  
  
  free (pi);
  pi = NULL;
  BitSetDelete (c);
  BitPowerSetDelete (ar, r, i);
  BitPowerSetDelete (bs, s, i);
  return d;
}

#line 19317 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
/* Return a swap order not the distance between two assignment. */
int
IMA_gdistanceOrder (int n, int *Ai, int *Bi, int m, int *od)
{
  int idummy;
  int i, j, k;
  int d, d1, d2;
  int r, s;
  UByteP *ar;
  UByteP *bs;
  UByteP c;
  int sP;
  int g;
  int *pi;
  int npi; /* number of permutations */
  int *A;
  int *B;
  int v;
  
  /* Find r and s. */
  r = IMA_max_intarray (n, Ai);
  s = IMA_max_intarray (n, Bi);
  if (r > s)
    {
      A = Bi;
      B = Ai;
      i = s;
      s = r;
      r = i;
    }
  else
    {
      A = Ai;
      B = Bi;
    }
  assert (!(r > s));
  /* r is not greater than s. */
  assert (m == s); /* FIXME: why this should be? */

  sP = (n - 1) / 8 + 1;
  BitPowerSetNew (ar, sP, r, i);
  BitPowerSetNew (bs, sP, s, i);
  BitSetNew (c, sP);
  /* pi is initialized to be the first of permutations. */
  pi = malloc ((s + 1) * sizeof (int));
  for (i = 0; i < s + 1; i++)
    {
      pi[i] = i;
    }
  
  /* Set a and b. */
  for (i = 0; i < n; i++)
    {
      g = A[i];
      assert (!(g > r));
      BitTrue (ar[g - 1], i);
      g = B[i];
      assert (!(g > s));
      BitTrue (bs[g - 1], i);
    }
  
  d = n;
  npi = IMA_factorial(s);
  assert (npi > 0);
  i = 0;
  v = 0;
  while (i++ < npi)
    {
      d2 = 0;
      for (j = 1; j < s + 1; j++)
        {
          k = pi[j] - 1;
          if (j < r + 1)
            {
              BitDifference (c, ar[j - 1], bs[k], sP, idummy);
              BitNTrues (d1, c, sP, idummy);
              d2 += d1;              
            }
        }

      if (d2 < d)
        {
          d = d2;
          v = i;
        }
      IMA_permLexSuccessor(s, pi);
    }

  IMA_permLexUnrank (pi, s, v - 1); /* Unrank is zero-indexed. */
  d2 = 0;
  for (j = 1; j < s + 1; j++)
    {
      k = pi[j] - 1;
      od[j - 1] = k;
      if (j < r + 1)
        {
          BitDifference (c, ar[j - 1], bs[k], sP, idummy);
          BitNTrues (d1, c, sP, idummy);
          d2 += d1;              
        }
    }
  assert (d == d2);
  
  free (pi);
  pi = NULL;
  BitSetDelete (c);
  BitPowerSetDelete (ar, r, i);
  BitPowerSetDelete (bs, s, i);
  return v;
}
#line 19437 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
/* Return a swap order not the distance between two assignment. */
int
IMA_gdistanceOrderNEW (int n, int *Ai, int *Bi, int m, int *od)
{
  int i, j, k;
  int d, d2;
  int sP;
  int *pi;
  int npi; /* number of permutations */
  int v;
  int *nuBi;
  
  /* FIXME: why this should be? If the sampled assignment does not have all of
   * labels (npops is 4, and assignment only is done for 3 or 2 population),
   * then this will crash the program. We need to consider this case. We could
   * swap or relabel within the assigned populations. */

  nuBi = malloc (n * sizeof (int)); 
  
  sP = (n - 1) / 8 + 1;

  pi = malloc ((m + 1) * sizeof (int));
  for (i = 0; i < m + 1; i++)
    {
      pi[i] = i;
    }
  
  d = n;
  npi = IMA_factorial(m); /* This number should be that of possible permutation. */
  assert (npi > 0);
  i = 0;
  v = 0;
  while (i++ < npi)
    {
      /* Permute Bi using pi. */
      /* pi starts with index 1 not 0. Bi is also based on index 1 not 0. 
       * pi = {0, 1, 2, 3, 4} or {0, 3, 1, 2, 4}.
       * Bi = {1, 1, 3, 3, 3} not {0, 0, 2, 2, 2}
       */
      for (j = 0; j < n; j++)
        {
          nuBi[j] = pi[Bi[j]];
        }

      /* Compare Ai and permuted Bi. */ 
      d2 = IMA_gdistanceSimple (n, Ai, nuBi);

      if (d2 < d)
        {
          d = d2;
          v = i;
        }
      IMA_permLexSuccessor(m, pi);
    }

  IMA_permLexUnrank (pi, m, v - 1); /* Unrank is zero-indexed. */
  d2 = 0;
  for (j = 1; j < m + 1; j++)
    {
      k = pi[j] - 1;
      od[j - 1] = k;
    }
 
  free (nuBi);
  nuBi = NULL;
  free (pi);
  pi = NULL;
  return v;
}

int
IMA_gdistance (int n, int *Ai, int *Bi)
{
  int idummy;
  int i, j, k;
  int d, d1, d2;
  int r, s;
  UByteP *ar;
  UByteP *bs;
  UByteP c;
  int sP;
  int g;
  int *pi;
  int npi; /* number of permutations */
  int *A;
  int *B;
  
  /* Find r and s. */
  r = IMA_max_intarray (n, Ai);
  s = IMA_max_intarray (n, Bi);
  if (r > s)
    {
      A = Bi;
      B = Ai;
      i = s;
      s = r;
      r = i;
    }
  else
    {
      A = Ai;
      B = Bi;
    }
  assert (!(r > s));
  sP = (n - 1) / 8 + 1;
  BitPowerSetNew (ar, sP, r, i);
  BitPowerSetNew (bs, sP, s, i);
  BitSetNew (c, sP);
  /* pi is initialized to be the first of permutations. */
  pi = malloc ((s + 1) * sizeof (int));
  for (i = 0; i < s + 1; i++)
    {
      pi[i] = i;
    }
  
  /* Set a and b. */
  for (i = 0; i < n; i++)
    {
      g = A[i];
      assert (!(g > r));
      BitTrue (ar[g - 1], i);
      g = B[i];
      assert (!(g > s));
      BitTrue (bs[g - 1], i);
    }
  
  d = n;
  npi = IMA_factorial(s);
  assert (npi > 0);
  i = 0;
  while (i++ < npi)
    {
      d2 = 0;
      for (j = 1; j < s + 1; j++)
        {
          k = pi[j] - 1;
          if (j < r + 1)
            {
              BitDifference (c, ar[j - 1], bs[k], sP, idummy);
              BitNTrues (d1, c, sP, idummy);
              d2 += d1;              
            }
        }
      if (d2 < d)
        {
          d = d2;
        }
      IMA_permLexSuccessor(s, pi);
    }
  
  free (pi);
  pi = NULL;
  BitSetDelete (c);
  BitPowerSetDelete (ar, r, i);
  BitPowerSetDelete (bs, s, i);
  return d;
}

int
IMA_gdistanceSimple (int n, int *Ai, int *Bi)
{
  int d;
  int i;

  assert (n > 0);
  d = 0;
  for (i = 0; i < n; i++)
    {
      if (Ai[i] != Bi[i])
        {
          d++;
        }
    }
  return d;
}

int
IMA_max_intarray (int n, int *A)
{
  int i;
  int m;
  m = A[0];
  for (i = 1; i < n; i++)
    {
      if (m < A[i])
        {
          m = A[i];
        }
    }
  return m;
}
#line 19637 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int
IMA_permFprintf (FILE *fp, int n, int *p)
{
  int i;
  
  for (i = 1; i < n + 1; i++)
    {
      if (i == n)
        {
          fprintf (fp, "%2d\n", p[i]);
        }
      else
        {
          fprintf (fp, "%2d ", p[i]);          
        }
    }
  return 0;
}

/* Algorithm 2.14 of Kreher and Stinson */
int
IMA_permLexSuccessor (int n, int *p)
{
  int i, j, t, h;
  int *rho;
  
  rho = malloc ((n + 1) * sizeof (int));
  p[0] = 0;
  i = n - 1;
  while (p[i + 1] < p[i])
    {
      i = i - 1;
    }
  if (i == 0)
    {
      free (rho);
      rho = NULL;
      return 1; /* undefined */
    }
  j = n;
  while (p[j] < p[i])
    {
      j = j - 1;
    }
  t = p[j];
  p[j] = p[i];
  p[i] = t;
  for (h = i + 1; h < n + 1; h++)
    {
      rho[h] = p[h];
    }
  for (h = i + 1; h < n + 1; h++)
    {
      p[h] = rho[n + i + 1 - h];
    }
  free (rho);
  rho = NULL;
  return 0;
}

int
IMA_permLexRank (int n, int *p)
{
  int r, i, j;
  int *rho;
  
  rho = malloc ((n + 1) * sizeof (int));
  r = 0;
  memcpy (rho, p, (n + 1) * sizeof (int));
  for (j = 1; j < n + 1; j++)
    {
      r = r + (rho[j] - 1) * IMA_factorial (n - j);
      for (i = j + 1; i < n + 1; i++)
        {
          if (rho[i] > rho[j])
            {
              rho[i] = rho[i] - 1;
            }
        }
    }
  free (rho);
  rho = NULL;
  return r;
}

int
IMA_permLexUnrank (int *p, int n, int r)
{
  int i, j, d;
  
  p[n] = 1;
  for (j = 1; j < n; j++)
    {
      d = (r % IMA_factorial(j + 1)) / IMA_factorial(j);
      r = r - d * IMA_factorial(j);
      p[n - j] = d + 1;
      for (i = n - j + 1; i < n + 1; i++)
        {
          if (p[i] > d)
            {
              p[i] = p[i] + 1;
            }
        }
    }
  return 0;
}

int
IMA_factorial (int n)
{
  assert (!(n < 0));
  assert (n < 13);
  if (n == 0)
    {
      return 1;
    }
  if (n == 1)
    {
      return 1;
    }
  if (n == 2)
    {
      return 2;
    }
  else
    {
      return n * IMA_factorial (n - 1);
    }
}

#line 16436 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
void 
imaStructuramaUncertainty (char *fname, int nburnin)
{
  int d;
  FILE *fp;
  int lenpname;
  char *pname;
  int **m;
  int nrow;
  int ncol;
  int i;
  int *B;
  char **indname;
  int nsample;

  double *au;

  lenpname = 20 + strlen (fname);

  pname = malloc (lenpname * sizeof (char));
  sprintf (pname, "%s.in.p", fname);
  IMA_io_readp (pname, &m, &nrow, &ncol, nburnin);
  nsample = nrow - nburnin;

  /* Must be called after we know ncol, or number of individuals. */
  indname = malloc (ncol * sizeof (char *));
  IMA_io_readp_header (pname, indname, ncol);

  B = malloc (ncol * sizeof (int));
  au = malloc (ncol * sizeof (double));
  memset (au, 0, ncol * sizeof (double));
  sprintf (pname, "%s.in.sum_assignments", fname);
  IMA_io_readsumassignment (pname, ncol, B);
  d = IMA_gusfield_distanceRemoved (B, m, nsample, ncol, NULL, -1, au);

  sprintf (pname, "%s.uct", fname);
  fp = fopen (pname, "w");

  fprintf (fp, "t = {");
  for (i = 0; i < ncol; i++)
    {
      fprintf (fp, "{\"%s\", %d, %lf}", indname[i], B[i], au[i]);
      if (i < ncol - 1)
        {
          fprintf (fp, ", ");
        }
    }
  fprintf (fp, "}\n");
  fclose (fp);
  fp = NULL;
  free (au);
  au = NULL;
  
  for (i = 0; i < ncol; i++)
    {
      free (indname[i]);
      indname[i] = NULL;
    }
  free (indname);
  indname = NULL;

  free (B);
  B = NULL;
  for (i = 0; i < nsample; i++)
    {
      free (m[i]);
      m[i] = NULL;
    }
  free (m);
  m = NULL;
  free (pname);
  pname = NULL;


  return;
}




#line 16521 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
void 
imaIMTreeUncertainty (char *fname, const char *ext, int nburnin)
{
  int d;
  FILE *fp;
  int lenpname;
  char *pname;
  int **m;
  int nrow;
  int ncol;
  int i;
  int *B;
  char **indname;
  int nsample;

  double *au;

  lenpname = 20 + strlen (fname);

  pname = malloc (lenpname * sizeof (char));
  sprintf (pname, "%s%s", fname, ext);
  IMA_io_readp (pname, &m, &nrow, &ncol, nburnin);
  nsample = nrow - nburnin;

  /* Must be called after we know ncol, or number of individuals. */
  indname = malloc (ncol * sizeof (char *));
  IMA_io_readp_header (pname, indname, ncol);

  B = malloc (ncol * sizeof (int));
  au = malloc (ncol * sizeof (double));
  memset (au, 0, ncol * sizeof (double));
  sprintf (pname, "%s.in.sum_assignments", fname);
  IMA_io_readsumassignment (pname, ncol, B);
  d = IMA_gusfield_distanceRemoved (B, m, nsample, ncol, NULL, -1, au);

  sprintf (pname, "%s.uct", fname);
  fp = fopen (pname, "w");

  fprintf (fp, "t = {");
  for (i = 0; i < ncol; i++)
    {
      fprintf (fp, "{\"%s\", %d, %lf}", indname[i], B[i], au[i]);
      if (i < ncol - 1)
        {
          fprintf (fp, ", ");
        }
    }
  fprintf (fp, "}\n");
  fclose (fp);
  fp = NULL;
  free (au);
  au = NULL;
  
  for (i = 0; i < ncol; i++)
    {
      free (indname[i]);
      indname[i] = NULL;
    }
  free (indname);
  indname = NULL;

  free (B);
  B = NULL;
  for (i = 0; i < nsample; i++)
    {
      free (m[i]);
      m[i] = NULL;
    }
  free (m);
  m = NULL;
  free (pname);
  pname = NULL;


  return;
}



#line 18504 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int
skip_a_line (FILE *fp)
{
  char c;
  c = 'x';
  while (c != '\n')
    {
      c = fgetc(fp);
    }
  return 0;
}

int
read_int (FILE *fp)
{
  int v;
  char c;
  char *buf;
  int len;
  int i;
  
  len = 5;
  buf = malloc (len * sizeof (char));
  
  c = fgetc (fp);
  i = 0;
  while (c != '\t' && c != '\n')
    {
      buf[i++] = c;
      if (i == len)
        {
          len += 5;
          buf = realloc (buf, len * sizeof (char));
        }
      c = fgetc (fp);
    }
  buf[i] = '\0';
  v = atoi (buf);
  
  free (buf);
  buf = NULL;
  return v;
}

double 
read_double (FILE *fp)
{
  double v;
  char c;
  char *buf;
  int len;
  int i;
  
  len = 5;
  buf = malloc (len * sizeof (char));
  
  c = fgetc (fp);
  i = 0;
  while (c != '\t' && c != '\n')
    {
      buf[i++] = c;
      if (i == len)
        {
          len += 5;
          buf = realloc (buf, len * sizeof (char));
        }
      c = fgetc (fp);
    }
  buf[i] = '\0';
  v = atof (buf);
  
  free (buf);
  buf = NULL;
  
  return v;
}


