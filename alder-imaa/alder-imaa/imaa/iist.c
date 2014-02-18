#line 1649 "/Users/goshng/works/bml/noweb/bml.nw"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "xtrapbits.h"
#include "iist.h"

#line 1657 "/Users/goshng/works/bml/noweb/bml.nw"
int
main (int argc, const char * argv[]) 
{
  /* */
  int i;
  int j;
  int *tasn;
  int nind;
  char *fname_out;
  char *fname_asn;
  char *fname_sumassignment;
  char *fname_inpp;
  char *fname_inppp;
  int len;
  int nrow;
  int ncol;
  int **m;
  double **p;
  int *od;
  int *odp;
  int nk;
  int npops;
  int r;
  int *nod;
  int npi;

  /* Initialized variables. */
  nind = -1;
  npops = -1;

  /* ./iist [[im output file]] [[num. individual]] */
  /* Example: ./iist fischer.out 39 4 */
  len = strlen (argv[1]);
  nind = atoi (argv[2]);
  npops = atoi (argv[3]);

  tasn = malloc (nind * sizeof (int));

  fname_asn = malloc ((len + 4 + 1) * sizeof (char));
  sprintf (fname_asn, "%s.asn", argv[1]);
  fname_sumassignment = malloc ((len + 20 + 1) * sizeof (char));
  sprintf (fname_sumassignment, "%s.in.sum_assignments", argv[1]);

  IMA_io_readsumassignment (fname_sumassignment, nind, tasn);

#ifdef Debug
  printf ("ASN: ");
  for (i = 0; i < nind; i++)
    {
      printf ("[%d] %d, ", i, tasn[i]); 
    }
  printf ("\n");
#endif

  /* Read all posterior samples of assignment parameter. */
  IMA_io_readp (fname_asn, &m, &nrow, &ncol);
  /* This may be different when there are multiple MCMC ouput files. */
  assert (ncol == nind);

  for (i = 0; i < nrow; i++)
    {
      for (j = 0; j < ncol; j++)
        {
          m[i][j]++;
        }
    }

  od = (int *) malloc (npops * sizeof (int));
  odp = (int *) malloc ((npops + 1)* sizeof (int));
  nod = (int *) malloc (24 * sizeof (int));
  memset (nod, 0, 24 * sizeof (int));

  for (i = 0; i < nrow; i++)
    {
      /*******************************************************/
      /* CHECK: see the detail of tasn and m[i] to check od. */
      /*******************************************************/
      IMA_gdistanceOrder (nind, tasn, m[i], npops, od); /* FIXME: npops is fixed to 2 */
      odp[0] = 0;
      for (j = 0; j < npops; j++)
        {
          odp[j + 1] = od[j] + 1; 
        }
      r = IMA_permLexRank (npops, odp);
      nod[r]++;

#ifdef Debug
      printf ("[%4d]: ", i);
      for (j = 0; j < ncol; j++)
        {
          printf ("%d ", m[i][j]);
        }
      printf ("\n");
      fprintf (stdout, "[%4d]: ", i);
      IMA_fprintf (stdout, npops, odp);
#endif

      /* The rank of od? */
      /* Count it. */
      /* od: 1 2 0 3 */
      /* bmlAlign (i, od, p, nk, npops);  */
      /* # of npops can be smaller than estimate. */
      /* imaGsampinfRelabel (i, od); */
    }

  /* Ranks */
  for (i = 0; i < npops + 1; i++)
    {
      odp[i] = i;
    }
  npi = IMA_factorial(npops);
  for (i = 0; i < npi; i++)
    {
      IMA_fprintf (stdout, npops, odp);
      r = IMA_permLexRank (npops, odp);
      assert (r == i);
      fprintf (stdout, "\trank: %d - (c) %d\n", r, nod[i]);
      IMA_permLexSuccessor (npops, odp);
    }
  
  free (od);
  free (odp);

  /* Free all local variables. */
  free (tasn);
  free (fname_sumassignment); 
  free (fname_asn); 

  for (i = 0; i < nrow; i++)
    {
      free (m[i]);
    }
  free (m);

  return 0;
}

int
IMA_fprintf (FILE *fp, int n, int *p)
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



#line 770 "/Users/goshng/works/bml/noweb/bml.nw"
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


#line 688 "/Users/goshng/works/bml/noweb/bml.nw"
int
IMA_io_readp (char *fname, int ***m, int *mr, int *mc)
{
  FILE *fp;
  char c;
  int ncol, nrow;
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
  nrow = -2;
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
  
  (*m) = malloc (nrow * sizeof (int *));
  for (i = 0; i < nrow; i++)
    {
      (*m)[i] = malloc (ncol * sizeof (int));
    }
  
  /* Reposition pointer to the beginning of the file. */
  fseek (fp, 0L, SEEK_SET);

#ifndef ONLYUNCERTAINTY /* Not for structurama run */
  skip_a_line (fp); /* for IM */
#endif

  for (i = 0; i < nrow; i++)
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
  fprintf (stdout, "ncol: %5d\nnrow: %5d\n", ncol, nrow);
  return 0;
}

#line 597 "/Users/goshng/works/bml/noweb/bml.nw"
int 
bmlReadpp (char *fname, double ***m, int *mc, int nind, int npops)
{
  FILE *fp;
  char c;
  int ncol, nrow;
  int i, j;
  double d;
  int gi;
  double lnL;


  int knp;
  
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
  nrow = -2;
#else
  ncol -= 2; /* for IM */
  nrow = -1;
#endif

  knp = ncol - nind;
  assert (knp % npops == 0);

  nrow = -1;
  while (!feof (fp))
    {
      if (c == '\n')
        {
          nrow++;
        }
      c = fgetc(fp);      
    }
  
  (*m) = malloc (nrow * sizeof (double *));
  for (i = 0; i < nrow; i++)
    {
      (*m)[i] = malloc (knp * sizeof (double));
    }
  
  /* Reposition pointer to the beginning of the file. */
  fseek (fp, 0L, SEEK_SET);

#ifndef ONLYUNCERTAINTY /* Not for structurama run */
  skip_a_line (fp); /* for IM */
#endif

  for (i = 0; i < nrow; i++)
    {
#ifdef ONLYUNCERTAINTY /* Structurama run */
      skip_a_line (fp);
#endif
      gi = read_int (fp);
      lnL = read_double (fp);
      for (j = 0; j < nind; j++)
        {
          gi = read_int (fp);
        }
      for (j = 0; j < knp; j++)
        {
          d = read_double (fp);
          (*m)[i][j] = d;
        }
    }
  
  fclose (fp);
  fp = NULL;
  
  *mc = knp;
  fprintf (stdout, "ncol: %5d\nnrow: %5d\n", knp, nrow);
  return 0;

}
#line 958 "/Users/goshng/works/bml/noweb/bml.nw"
int
IMA_gdistanceRemoved (int n, int *Ai, int *Bi, char *Ci)
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
  //assert (m == s); /* FIXME: why this should be? */

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



#line 1340 "/Users/goshng/works/bml/noweb/bml.nw"
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

#line 860 "/Users/goshng/works/bml/noweb/bml.nw"
void 
bmlPrintP (const char *fname_inppp, double **p, int nk, int nrow)
{
  FILE *fp;
  int i;
  int j;

  fp = fopen (fname_inppp, "w");

  if (fp == NULL)
    {
      perror ("inppp cannot created");
    }

  fprintf (fp, "Gen");
  for (j = 0; j < nk; j++)
    {
      fprintf (fp, "\t%d", j);
    }
  fprintf (fp, "\n");

  for (i = 0; i < nrow; i++)
    {
      fprintf (fp, "%d\t", i);
      for (j = 0; j < nk; j++)
        {
          fprintf (fp, "%lf", p[i][j]);
          if (j == nk - 1)
            {
              fprintf (fp, "\n");
            }
          else
            {
              fprintf (fp, "\t");
            }
        }
    }

  fclose (fp);
}

void 
bmlAlign (int gi, int *od, double **p, int nk, int np)
{
  int i;
  int j;
  int pi;
  int pj;
  int nl; /* number of parameters */
  int li;
  /* nk: nl * np */

  double *ap;

  nl = nk / np;
  assert (nk % np == 0);

  ap = malloc (nk * sizeof (double));

  /* pi: 0 1 2 3
   * pj: 1 2 0 3
   */
  printf ("%4d: ", gi);
  for (pi = 0; pi < np; pi++)
    {
      printf ("%d\t", od[pi]);
    }
  printf ("\n");

  for (pi = 0; pi < np; pi++)
    {
      pj = od[pi];
      for (li = 0; li < nl; li++)
        {
          ap[li * np + pj] = p[gi][li * np + pi]; 
        }
    }

  memcpy (p[gi], ap, nk * sizeof (double));
  free (ap);

  return;
}



