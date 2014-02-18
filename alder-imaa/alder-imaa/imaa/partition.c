#line 18038 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "partition.h"

static void inp_free (im_inp *p, int obs, int maxpop);
static im_inp* inp_alloc (int obs, int maxpop, int maxind);
static int get_obs_p (char *pname, int *obs, int *m, int *ni);
static int parse_in_p (im_inp *p, char *pname);
static int write_partitionview (im_inp *p, char *tname);

#line 18499 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int skip_a_line (FILE *fp);
int read_int (FILE *fp);
double read_double (FILE *fp);

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


#line 18053 "/home/goshng/Documents/Projects/IMamp/noweb/imamp.nw"
int
main (int argc, char *argv[])
{
  char *outfilename;
  if (argc != 1)
    {
      fprintf (stderr, "Command: %s [file]\n", argv[0]);
      return 0;
    }
  outfilename = argv[1];
  IMA_inp2partitionview (outfilename);
  return 0;
}

void 
IMA_inp2partitionview (char *fname)
{
  char *pname;
  char *tname;
  int len;
  im_inp *pdata;
  int obs;
  int m;
  int ni;

  len = strlen (fname);
  len += 10;
  pname = (char *) malloc (len * sizeof (char));
  tname = (char *) malloc (len * sizeof (char));
  sprintf (pname, "%s.in.p", fname);
  sprintf (tname, "%s.part", fname);

  get_obs_p (pname, &obs, &m, &ni);

  pdata = inp_alloc (obs, m, ni);
  pdata->nobs = obs;
  pdata->nind = ni;
  pdata->nmaxpop = m;
  pdata->nperiod = 10;
  pdata->uprior = 1.0;
  pdata->tprior = 1.0;

  parse_in_p (pdata, pname);
  write_partitionview (pdata, tname);
  inp_free (pdata, obs, m);

  free (pname);
  pname = NULL;
  free (tname);
  tname = NULL;
}

im_inp*
inp_alloc (int obs, int maxpop, int maxind)
{
  im_inp *p;
  int i;
  int j;
  im_x *x;

  assert (obs > 0);
  assert (maxpop > 0);
  p = (im_inp *) malloc (sizeof (im_inp));
  p->x = (im_x *) malloc (obs * sizeof (im_x));
  x = p->x;
  for (i = 0; i < obs; i++)
    {
      x->ks = (int *) malloc (maxpop * sizeof (int));
      x->ks = memset (x->ks, 0, maxpop * sizeof (int));
      x->is = (int **) malloc (maxpop * sizeof (int *));
      /* y = x->is[0]; */
      for (j = 0; j < maxpop; j++)
        {
          x->is[j] = (int *) malloc (maxind * sizeof (int));
          /* y++; */
        }
      x++;
    }


  return p;
}

void
inp_free (im_inp *p, int obs, int maxpop)
{
  int i;
  int j;

  for (i = 0; i < obs; i++)
    {
      free (p->x[i].ks);
      p->x[i].ks = NULL;
      for (j = 0; j < maxpop; j++)
        {
          free (p->x[i].is[j]);
          p->x[i].is[j] = NULL;
        }
      free (p->x[i].is);
      p->x[i].is = NULL;
    }
  free (p->x);
  p->x = NULL;
  free (p);
  p = NULL;

  return;
}

int
get_obs_p (char *pname, int *obs, int *m, int *ni)
{
  FILE *fp;
  char c;
  int ncol;
  int pi;
  fp = fopen (pname, "r");
  assert (fp != NULL);

  *ni = 0;
  c = 'x';
  while (c != '\n')
    {
      c = fgetc (fp);
      if (c == ')')
        {
          (*ni) += 1;
        }
    }

  *obs = 0;
  *m = -1;
  ncol = 0;
  while (!feof (fp))
    {
/*
      if (c == '\n')
        {
          (*obs) += 1;
          ncol = 0;
        }
*/
      if (c == '\t')
        {
          ncol++;
        }
      if (ncol > 1 && ncol < *ni + 2)
        {
          pi = read_int (fp);
          if (pi > *m)
            {
              *m = pi;
            }
        }
      else
        {
          if (!(ncol < *ni + 2))
            {
              ncol = 0;
              /* ungetc ('\n', fp); */
              (*obs) += 1;
            }
          c = fgetc (fp);      
        }
    }

  fclose (fp);
  return 0;
}

int
parse_in_p (im_inp *p, char *pname)
{
  FILE *fp;
  int i;
  int j;
  int iv;
  int ki;
  double dv;
  fp = fopen (pname, "r");

  skip_a_line (fp);

  for (i = 0; i < p->nobs; i++)
    {
      iv = read_int (fp);
      p->x[i].i = iv;
      dv = read_double (fp);
      p->x[i].logL = dv;
      p->x[i].k = 1;
      for (j = 0; j < p->nind; j++)
        {
          iv = read_int (fp);
          if (iv > p->x[i].k)
            {
              p->x[i].k = iv;
            }
          iv--;
          ki = p->x[i].ks[iv];
          p->x[i].is[iv][ki] = j + 1;
          ki++;
          p->x[i].ks[iv] = ki;
        }
     /* NOTE: for structurama only */
     skip_a_line (fp);
    }

  fclose (fp);
  return 0;
}

int
write_partitionview (im_inp *p, char *tname)
{
  int i, j, k;
  FILE *fp;
  fp = fopen (tname, "w");

  fprintf (fp, "%s\n", tname);
  fprintf (fp, "%d\n", p->nind);
  fprintf (fp, "%d\n", p->nmaxpop);
  fprintf (fp, "%d\n", p->nobs);
  fprintf (fp, "%d\n", p->nperiod);
  fprintf (fp, "%lf\n", p->uprior);
  fprintf (fp, "%lf\n", p->tprior);
  for (i = 0; i < p->nobs; i++)
    {
      fprintf (fp, "%d\n", p->x[i].i);
      fprintf (fp, "%lf\n", p->x[i].logL);
      fprintf (fp, "%d\n", p->x[i].k);
      for (j = 0; j < p->x[i].k; j++)
        {
          fprintf (fp, "%d\n", p->x[i].ks[j]);
          for (k = 0; k < p->x[i].ks[j]; k++)
            {
              fprintf (fp, "%d  ", p->x[i].is[j][k]);
            }
          fprintf (fp, "\n");
        }
    }

  fclose (fp);
  return 0;
}

