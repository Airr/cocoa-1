#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "dpart.h"


/* DPART

  This code can be compiled on Windows, but has not been compiled on any other platforms */


/* Fixed values */
#define Ver "06"                /* version of this program */
#define	parafilename  "DPART_parameters.txt"    /* name of the parameter file */
#define inputfilenames	  "DPART_inputfiles.txt"        /* inputfile names */
#define	Mfn  51                 /* max length of the input data file name */
#define	Maxalleleinput  999     /* max allele in the input data file */
#define	Missingalleleinput -9   /* missing alleles in the input data file (should be negative) */
#define	Nw 10                   /* number of windows to output the frequency of the acceptance rate of the SAMS sampler */


/* Parameters not included in the parameter file */
#define	seed -1                 /* random seed. Use time when -1 */

#define	SAMSfreq 4              /* frequency of the SAMS sampler */
#define Gibbsfreq 1             /* frequency of the Gibbs sampler */

#define Shape 1.0               /* shape of the gamma prior for Alpha */
#define Scale 1.0               /* scale of the gamma prior for Alpha. E[Alpha] = Shape / Scale, V[Alpha] = Shape / Scale^2 */
#define Sdforalpha 0.02         /* Sd of the proposal distribution of Alpha */

#define Lambdalim 10.0          /* Upper limit of the uniform prior of Lambda, U (0.0, Lambdalim) */
#define	Sdforlambda 0.02        /* Sd of the propsal ditribution of Lambda */


/* Values defined by the parameter file*/
int Ninput;                     /* number of input files */
int Ploidy;                     /* ploidy level. only 1 or 2 is allowed */
int Ni;                         /* number of individuals */
int Nl;                         /* number of loci */
double Alpha;                   /* value of the mass parameter for dirichlet process mixture model.
                                   When Alpha = 0, Alpha is estimated */
double Lambda;                  /* value of the hyperparameter for the prior distribution of allele frequencies.
                                   When Lambda = 0, A unique value of Lambda is estimated for each locus. 
                                   When Lambda = -1, A single value of Lambda is estimated for all loci.
                                   When Lambda = -2, Lambda is fixed to 1/Jl where Jl is the number of alleles at locus l. */
int Burnin;                     /* length of bunrnin */
int Ite;                        /* length of iterations after burnin */
int Thi;                        /* thninng. the number of samples is Ite/Thi */
int Run;                        /* number of runs of analysis */
int Meanpartcal;                /* determine whether calculate the mean partition */

char Labeltemp[Mfn];


/* Input and output files */
FILE *fp;
char *Input, *Output;

/* Used random seed*/
int usedseed;

/* Generic use */
int ii, jj, kk, mm, max, locus, allele, ind, pop, aupop, *pointer;
double sum, temp;
int file, run, mcmc;

/* Positions in arrays. */
int Pos, Pos2, Pos3, Pos4, Pos5;

/* Ploidy * Nl, Nl * Missingallele */
int PN, NM;

/* Data */
int *Xi;                        /* Genotype data (obserbed data) */
int *Missing;                   /* missing genotypes in Xi */
int *Maxallele;                 /* max allele of each locus */
int Maxal;                      /* max allele over all loci */
int Missingallele;              /* missing allele in Xi */
int Xcheck;                     /* for inputfile check */

/* Number of populations */
int Inik;                       /* Initial number of populations in the cold chain */
int Currk;                      /* number of populations in the current state */
int Newk;                       /* number of populations in the proposed state */

/* Estimated parametars */
int *Zi;                        /* populations where individuals belong */
int *meanPart;                  /* mean partition */
double *Coaprob;                /* co-assignment probabilities */
int *Nic;                       /* number of individuals in each population */
int *Count;                     /* allele count at each locus in each population */

/* Hyperparametars */
double EsAlpha;                 /* estimated mass parameter of the dirichlet process prior */
double *EsLambda;               /* estimated Lambda */

/* Sampling */
int Startsamp;                  /* iteration where sampling starts */
int Nswb;                       /* number of samples */
int *sampledZ;                  /* sampled Zi */
double meanAlpha;               /* mean Alpha */
double *meanLambda;             /* average Lambda */

/* auxiliary objects */
int *Nica;                      /* used when update Nic */
int *Counta;                    /* used when update Count */

/* Metropolis-Hastings probability */
double MHprob;

/* For the SAMS sampler */
int i;                          /* selected individual in SAMS sampling */
int j;                          /* selected individual in SAMS sampling */
int Si;                         /* population where i belong  */
int Sj;                         /* population where j belongs */
int Niss[2];                    /* number of individuals in Si and Sj */
int *propZ;                     /* proposal of Zi */

/* Acceptance rate */
int *Acsplit;                   /* record how many times split is accepted in the cold chain */
int *Acmerge;                   /* record how many times split is proposed in the cold chain */
int *Ocsplit;                   /* record how many times merge is accepted in the cold chain */
int *Ocmerge;                   /* record how many times merge is proposed in the cold chain */
int Sw;                         /* size of window which records the acceptance rate. calculated as (Burnin + Ite) / Nw */

/* SAMSfreq + Gibbsfreq */
int SG;


/* main --------------------------------------------------------------------------------------------------------------------------*/
int
main (void)
{
  /* Start message */
  putchar ('\n');
  printf ("-------------------------------------------------\n");
  printf ("                DPART ver%s\n", Ver);
  printf ("-------------------------------------------------\n");
  putchar ('\n');

  /* Open the parameter file */
  if ((fp = fopen (parafilename, "r")) == NULL)
    {
      printf ("can not open %s\n", parafilename);
      return (0);
    }
  else
    {
      fscanf (fp, "%s%d", Labeltemp, &Ninput);
      fscanf (fp, "%s%d", Labeltemp, &Ploidy);
      fscanf (fp, "%s%d", Labeltemp, &Ni);
      fscanf (fp, "%s%d", Labeltemp, &Nl);
      fscanf (fp, "%s%lf", Labeltemp, &Alpha);
      fscanf (fp, "%s%lf", Labeltemp, &Lambda);
      fscanf (fp, "%s%d", Labeltemp, &Burnin);
      fscanf (fp, "%s%d", Labeltemp, &Ite);
      fscanf (fp, "%s%d", Labeltemp, &Thi);
      fscanf (fp, "%s%d", Labeltemp, &Run);
      fscanf (fp, "%s%d", Labeltemp, &Meanpartcal);

      /* print the parameters */
      putchar ('\n');
      printf ("Ninput : %d\n", Ninput);
      printf ("Ploidy : %d\n", Ploidy);
      printf ("Ni : %d\n", Ni);
      printf ("Nl : %d\n", Nl);
      printf ("Alpha : %f\n", Alpha);
      printf ("Lambda : %f\n", Lambda);
      printf ("Burnin : %d\n", Burnin);
      printf ("Iterations : %d\n", Ite);
      printf ("Thinning : %d\n", Thi);
      printf ("Run : %d\n", Run);
      printf ("Meanpartcal : %d\n", Meanpartcal);
      putchar ('\n');

      printf ("Total length of MCMC iterations is %d\n", Burnin + Ite);
      printf ("The number of MCMC samples is %d\n", Ite / Thi);
      putchar ('\n');

      fclose (fp);
    }

  /* Check the parameter values */
  if (Ploidy != 1 && Ploidy != 2)
    {
      printf ("Ploidy error. only 1 or 2 is allowed\n");
      return (0);
    }

  if (Alpha < 0)
    {
      printf ("Alpha should be >= 0\n");
      return (0);
    }

  if (Lambda < 0 && Lambda != -1 && Lambda != -2)
    {
      printf ("Lambda should be either -1, -2, or >=0\n");
      return (0);
    }


  for (file = 0; file < Ninput; file++)
    {

      /* Open the input data file */
      if ((fp = fopen (inputfilenames, "r")) == NULL)
        {
          printf ("can not open %s\n", inputfilenames);
          return (0);
        }
      else
        {

          Input = malloc (sizeof (char) * (Mfn + 4));
          if (Input == NULL)
            alert ();
          for (ii = 0; ii < file; ii++)
            fscanf (fp, "%s", Labeltemp);
          fscanf (fp, "%s", Input);

          fclose (fp);

          if ((fp = fopen (Input, "r")) == NULL)
            {
              printf ("can not open %s\n", Input);
              free (Input);
              return (0);
            }
          else
            {

              jj = Ni * Nl * Ploidy;
              Xi = calloc (jj, sizeof (int));
              if (Xi == NULL)
                alert ();
              for (ii = 0; ii < jj; ii++)
                {
                  fscanf (fp, "%d", &Xi[ii]);

                  if (Xi[ii] > Maxalleleinput)
                    {
                      printf ("Errors in %s\n", Input);
                      printf ("Allele labels > %d are not allowed\n",
                              Maxalleleinput);
                      putchar ('\n');
                      return (0);
                    }

                  if (Xi[ii] <= 0 && Xi[ii] != Missingalleleinput)
                    {
                      printf ("Errors in %s\n", Input);
                      printf
                        ("The number of allele copies in the input data file may be less than %d * %d * %d\n",
                         Ni, Nl, Ploidy);
                      printf ("or irregular allele labels may be included\n");
                      printf
                        ("Note that missing genotypes should be represented by %d\n",
                         Missingalleleinput);

                      putchar ('\n');
                      return (0);
                    }
                }

              /* Check the input data */
              fscanf (fp, "%d", &Xcheck);
              if (Xcheck != 0)
                {
                  printf ("Errors in %s\n", Input);
                  printf
                    ("The number of allele copies in the input data file is more than %d * %d * %d\n",
                     Ni, Nl, Ploidy);
                  putchar ('\n');
                  return (0);
                }

              fclose (fp);

              /* Initialize the seed */
              usedseed = Randomize (seed);
              ix = (rand () % 30000) + 1;
              iy = (rand () % 30000) + 1;
              iz = (rand () % 30000) + 1;

              /* Output the parameters */
              Output = malloc (sizeof (char) * (Mfn + 20));
              if (Output == NULL)
                alert ();
              sprintf (Output, "%s_%s", Input, "parameters.txt");

              fp = fopen (Output, "w");
              fprintf (fp, "DPART ver %s\n", Ver);
              fprintf (fp, "\n");
              fprintf (fp, "Inputfile : %s\n", Input);
              fprintf (fp, "Ploidy : %d\n", Ploidy);
              fprintf (fp, "Ni : %d\n", Ni);
              fprintf (fp, "Nl : %d\n", Nl);
              fprintf (fp, "Alpha : %f\n", Alpha);
              fprintf (fp, "Lambda : %f\n", Lambda);
              fprintf (fp, "Burnin: %d\n", Burnin);
              fprintf (fp, "Iterations : %d\n", Ite);
              fprintf (fp, "Thinning : %d\n", Thi);
              fprintf (fp, "Run : %d\n", Run);
              fprintf (fp, "Meanpartcal : %d\n", Meanpartcal);
              fprintf (fp, "Random seed : %d\n", usedseed);
              fprintf (fp, "SAMSfreq : %d\n", SAMSfreq);
              fprintf (fp, "Gibbsfreq : %d\n", Gibbsfreq);

              if (Alpha == 0.0)
                {
                  fprintf (fp, "Shape : %f\n", Shape);
                  fprintf (fp, "Scale : %f\n", Scale);
                  fprintf (fp, "Sdforalpha : %f\n", Sdforalpha);
                }

              if (Lambda == 0.0)
                {
                  fprintf (fp, "Lambdalimit : %f\n", Lambdalim);
                  fprintf (fp, "Sdforlambda : %f\n", Sdforlambda);
                }

              fclose (fp);
              free (Output);

              /* Convert the Input data file */
              Convert (Xi, Maxalleleinput, Nl, Ni, Ploidy);

              /* Max allele in each locus and over loci */
              Maxal = 0;
              Maxallele = malloc (sizeof (int) * Nl);
              if (Maxallele == NULL)
                alert ();

              for (locus = 0; locus < Nl; locus++)
                for (allele = Ploidy * locus;
                     allele <= (Ploidy * locus + Ploidy - 1); allele++)
                  for (ind = 0; ind < Ni; ind++)
                    if (Xi[ind * Nl * Ploidy + allele] > Maxallele[locus])
                      {
                        Maxallele[locus] = Xi[ind * Nl * Ploidy + allele];
                        if (Maxallele[locus] > Maxal)
                          Maxal = Maxallele[locus];
                      }

              /* Convert missing alleles into Missingallele */
              Missingallele = Maxal + 1;

              for (locus = 0; locus < Nl; locus++)
                for (allele = Ploidy * locus;
                     allele <= (Ploidy * locus + Ploidy - 1); allele++)
                  for (ind = 0; ind < Ni; ind++)
                    if (Xi[ind * Nl * Ploidy + allele] == Missingalleleinput)
                      Xi[ind * Nl * Ploidy + allele] = Missingallele;

              /* Missing genotypes in Xi */
              Missing = calloc (Ni * Nl, sizeof (int));
              if (Missing == NULL)
                alert ();

              for (ind = 0; ind < Ni; ind++)
                for (locus = 0; locus < Nl; locus++)
                  if (Xi[Ploidy * Nl * ind + Ploidy * locus] ==
                      Xi[Ploidy * Nl * ind + Ploidy * locus + Ploidy - 1])
                    if (Xi[Ploidy * Nl * ind + Ploidy * locus] ==
                        Missingallele)
                      Missing[Nl * ind + locus] = 1;

              /* Iteration that begin sampling parameters */
              Startsamp = (Burnin / Thi) * Thi + Thi;

              /* Number of sampled iterations */
              Nswb = (Burnin + Ite) / Thi - Burnin / Thi;

              /* Window size to record the acceptance of split and merge in the SAMS sampler */
              Sw = (Burnin + Ite) / Nw;
              if ((Burnin + Ite) % Nw > 0)
                Sw++;

              /* Sum of SAMSfreq and Gibbsfreq */
              SG = SAMSfreq + Gibbsfreq;

              /* Ploidy * Nl and Nl * Missingallele */
              PN = Ploidy * Nl;
              NM = Nl * Missingallele;

              printf ("-------------------------------------------------\n");
              printf ("Analysis starts; %s\n", Input);
              putchar ('\n');

              /* Run analyses */
              for (run = 1; run <= Run; run++)
                {
                  printf ("Run %d\n", run);
                  putchar ('\n');

                  /* Initialize Alpha */
                  if (Alpha == 0.0)
                    {
                      EsAlpha = Shape / Scale;
                    }
                  else
                    {
                      EsAlpha = Alpha;
                    }

                  /* Initialize Lambda */
                  EsLambda = calloc (Nl, sizeof (double));
                  if (EsLambda == NULL)
                    alert ();
                  for (locus = 0; locus < Nl; locus++)
                    {
                      if (Lambda > 0)
                        EsLambda[locus] = Lambda;
                      if (Lambda == -1 || Lambda == 0)
                        EsLambda[locus] = 1.0;
                      if (Lambda == -2)
                        EsLambda[locus] = 1.0 / (double) Maxallele[locus];
                    }

                  /* Initialize Z and Nic */
                  Zi = calloc (Ni, sizeof (int));
                  if (Zi == NULL)
                    alert ();
                  Inik = InitializeZ (Zi, Ni, EsAlpha);
                  Currk = Inik;

                  Nic = calloc (Currk, sizeof (int));
                  if (Nic == NULL)
                    alert ();
                  for (ii = 0; ii < Ni; ii++)
                    {
                      Nic[Zi[ii]]++;
                    }

                  sampledZ = malloc (sizeof (int) * (Ni * Nswb));
                  if (sampledZ == NULL)
                    alert ();

                  if (SAMSfreq > 0)
                    {
                      Acsplit = calloc (Nw, sizeof (int));
                      if (Acsplit == NULL)
                        alert ();
                      Acmerge = calloc (Nw, sizeof (int));
                      if (Acmerge == NULL)
                        alert ();
                      Ocsplit = calloc (Nw, sizeof (int));
                      if (Ocsplit == NULL)
                        alert ();
                      Ocmerge = calloc (Nw, sizeof (int));
                      if (Ocmerge == NULL)
                        alert ();
                    }

                  if (Alpha == 0.0)
                    {
                      meanAlpha = 0.0;
                    }

                  if (Lambda == 0.0)
                    {
                      meanLambda = calloc (Nl, sizeof (double));
                      if (meanLambda == NULL)
                        alert ();
                    }

                  if (Lambda == -1)
                    {
                      meanLambda = calloc (1, sizeof (double));
                      if (meanLambda == NULL)
                        alert ();
                    }


                  /* Count number of allele copies */
                  Count = calloc (Currk * NM, sizeof (int));
                  if (Count == NULL)
                    alert ();
                  for (ind = 0; ind < Ni; ind++)
                    {
                      for (locus = 0; locus < Nl; locus++)
                        {
                          Count[Zi[ind] * NM + locus * Missingallele +
                                Xi[ind * PN + Ploidy * locus] - 1]++;
                          if (Ploidy == 2)
                            Count[Zi[ind] * NM + locus * Missingallele +
                                  Xi[ind * PN + Ploidy * locus + 1] - 1]++;
                        }
                    }

                  printf ("MCMC starts\n");
                  printf
                    ("-------------------------------------------------\n");
                  putchar ('\n');

                  /* MCMC iterations start */
                  for (mcmc = 1; mcmc <= (Burnin + Ite); mcmc++)
                    {

                      /* Print mcmc */
                      if (mcmc % 500 == 0)
                        {
                          printf ("%d\n", mcmc);
                          putchar ('\n');
                        }

                      if ((mcmc - 1) % SG < SAMSfreq)
                        {
                          /* use SAMS sampler */

                          /* Select two individuals, i and j */
                          i = RandomInteger (0, Ni - 1);
                          while (1)
                            {
                              j = RandomInteger (0, Ni - 1);
                              if (j != i)
                                break;
                            }

                          if (Zi[i] == Zi[j])   /* split */
                            {

                              Ocsplit[(mcmc - 1) / Sw] += 1;

                              propZ = malloc (sizeof (int) * Ni);
                              if (propZ == NULL)
                                alert ();       /* proposal of Zi */
                              for (ind = 0; ind < Ni; ind++)
                                propZ[ind] = Zi[ind];

                              propZ[i] = Currk;

                              MHprob =
                                SAMSsplit (Ni, Nl, Maxallele, Missingallele,
                                           Xi, Missing, EsLambda, Zi, i, j,
                                           Currk, Niss, Nic, propZ, EsAlpha,
                                           Ploidy);

                              if (MHprob > log (rnd ()))
                                {

                                  Newk = Currk + 1;

                                  /* update Nic */
                                  kk = sizeof (int) * Currk;
                                  Nica = malloc (kk);
                                  if (Nica == NULL)
                                    alert ();
                                  memcpy (Nica, Nic, kk);
                                  free (Nic);
                                  Nic = malloc (sizeof (int) * Newk);
                                  if (Nic == NULL)
                                    alert ();
                                  memcpy (Nic, Nica, kk);

                                  Nic[Zi[j]] = Niss[1];
                                  Nic[Currk] = Niss[0];

                                  free (Nica);

                                  /* update Zi */
                                  for (ind = 0; ind < Ni; ind++)
                                    Zi[ind] = propZ[ind];

                                  /* update Currk */
                                  Currk += 1;

                                  /* update Count */
                                  free (Count);
                                  Count = calloc (Currk * NM, sizeof (int));
                                  if (Count == NULL)
                                    alert ();
                                  for (ind = 0; ind < Ni; ind++)
                                    {
                                      for (locus = 0; locus < Nl; locus++)
                                        {
                                          Count[Zi[ind] * NM +
                                                locus * Missingallele +
                                                Xi[ind * PN +
                                                   Ploidy * locus] - 1]++;
                                          if (Ploidy == 2)
                                            Count[Zi[ind] * NM +
                                                  locus * Missingallele +
                                                  Xi[ind * PN +
                                                     Ploidy * locus + 1] -
                                                  1]++;
                                        }
                                    }

                                  /* record */
                                  Acsplit[(mcmc - 1) / Sw] += 1;

                                }       /* end of if ( MHprob >... ) */

                              free (propZ);

                            }   /* end of split */
                          else  /* merge */
                            {

                              Ocmerge[(mcmc - 1) / Sw] += 1;

                              MHprob =
                                SAMSmerge (Ni, Nl, Maxallele, Missingallele,
                                           Xi, Missing, EsLambda, Zi, i, j,
                                           Niss, Nic, EsAlpha, Ploidy);

                              if (MHprob > log (rnd ()))
                                {
                                  Newk = Currk - 1;
                                  Si = Zi[i];
                                  Sj = Zi[j];

                                  /* update Nic */
                                  kk = sizeof (int) * Currk;
                                  Nica = malloc (kk);
                                  if (Nica == NULL)
                                    alert ();
                                  memcpy (Nica, Nic, kk);
                                  Nica[Sj] += Nic[Si];
                                  free (Nic);
                                  Nic = malloc (sizeof (int) * Newk);
                                  if (Nic == NULL)
                                    alert ();

                                  for (pop = 0; pop < Currk; pop++)
                                    {
                                      if (pop < Si)
                                        Nic[pop] = Nica[pop];
                                      if (pop > Si)
                                        Nic[pop - 1] = Nica[pop];
                                    }

                                  free (Nica);

                                  /* update Zi */
                                  for (ind = 0; ind < Ni; ind++)
                                    {
                                      if (Zi[ind] == Si)
                                        Zi[ind] = Sj;
                                      if (Zi[ind] > Si)
                                        Zi[ind] -= 1;
                                    }

                                  /* update Currk */
                                  Currk -= 1;

                                  /* update Count */
                                  free (Count);
                                  Count = calloc (Currk * NM, sizeof (int));
                                  if (Count == NULL)
                                    alert ();

                                  for (ind = 0; ind < Ni; ind++)
                                    {
                                      for (locus = 0; locus < Nl; locus++)
                                        {
                                          Count[Zi[ind] * NM +
                                                locus * Missingallele +
                                                Xi[ind * PN +
                                                   Ploidy * locus] - 1]++;
                                          if (Ploidy == 2)
                                            Count[Zi[ind] * NM +
                                                  locus * Missingallele +
                                                  Xi[ind * PN +
                                                     Ploidy * locus + 1] -
                                                  1]++;
                                        }
                                    }

                                  /* record */
                                  Acmerge[(mcmc - 1) / Sw] += 1;

                                }       /* end of if ( MHprob >... ) */

                            }   /* end of merge */


                        }       /* end of if ( mcmc - 1 ) % SG < SAMSfreq */
                      else
                        {

                          /* perform Gibbs sampling */


                          /* Update Zi in each ind. */
                          for (ind = 0; ind < Ni; ind++)
                            {
                              /* remove ind from Nic */
                              Nic[Zi[ind]] -= 1;

                              /* remove alleles of ind from Count */
                              for (locus = 0; locus < Nl; locus++)
                                {
                                  Count[Zi[ind] * NM + locus * Missingallele +
                                        Xi[ind * PN + Ploidy * locus] - 1]--;
                                  if (Ploidy == 2)
                                    Count[Zi[ind] * NM +
                                          locus * Missingallele +
                                          Xi[ind * PN + Ploidy * locus + 1] -
                                          1]--;
                                }

                              /* assign ind */
                              if (Nic[Zi[ind]] > 0)     /* in the case where ind is not a singleton */
                                {

                                  ii =
                                    GibbsNotSingle (ind, Currk, Xi, Nl, Count,
                                                    Missingallele, Missing,
                                                    Maxallele, EsLambda, Nic,
                                                    EsAlpha, Ploidy, PN, NM);

                                  if (ii < Currk)
                                    {
                                      /* printf ("1 "); */

                                      Zi[ind] = ii;
                                      Nic[ii] += 1;
                                      for (locus = 0; locus < Nl; locus++)
                                        {
                                          Count[Zi[ind] * NM +
                                                locus * Missingallele +
                                                Xi[ind * PN +
                                                   Ploidy * locus] - 1]++;
                                          if (Ploidy == 2)
                                            Count[Zi[ind] * NM +
                                                  locus * Missingallele +
                                                  Xi[ind * PN +
                                                     Ploidy * locus + 1] -
                                                  1]++;
                                        }
                                    }
                                  else
                                    {
                                      /* printf ("2 "); */

                                      /* generate a new population */
                                      Newk = Currk + 1;
                                      Zi[ind] = (Newk - 1);

                                      /* Copy Nic */
                                      kk = sizeof (int) * Currk;
                                      Nica = malloc (kk);
                                      if (Nica == NULL)
                                        alert ();
                                      memcpy (Nica, Nic, kk);

                                      free (Nic);
                                      Nic = malloc (sizeof (int) * Newk);
                                      if (Nic == NULL)
                                        alert ();
                                      memcpy (Nic, Nica, kk);
                                      Nic[Currk] = 1;

                                      /* Copy Count */
                                      kk = sizeof (int) * (Currk * NM);
                                      Counta = malloc (kk);
                                      if (Counta == NULL)
                                        alert ();
                                      memcpy (Counta, Count, kk);

                                      free (Count);
                                      Count =
                                        calloc (Newk * NM, sizeof (int));
                                      if (Count == NULL)
                                        alert ();
                                      memcpy (Count, Counta, kk);

                                      for (locus = 0; locus < Nl; locus++)
                                        {
                                          Count[Currk * NM +
                                                locus * Missingallele +
                                                Xi[ind * PN +
                                                   Ploidy * locus] - 1]++;
                                          if (Ploidy == 2)
                                            Count[Currk * NM +
                                                  locus * Missingallele +
                                                  Xi[ind * PN +
                                                     Ploidy * locus + 1] -
                                                  1]++;
                                        }

                                      /* Update Currk */
                                      Currk = Newk;

                                      /* Free auxiliary parameters */
                                      free (Nica);
                                      free (Counta);

                                    }   /* end of if ( ii<Currk ) */
                                }
                              else
                                {       /* in the case where ind is a singleton */

                                  ii =
                                    GibbsSingle (ind, Currk, Xi, Nl, Zi,
                                                 Count, Missingallele,
                                                 Missing, Maxallele, EsLambda,
                                                 Nic, EsAlpha, Ploidy, PN,
                                                 NM);

                                  if (ii == Zi[ind])
                                    {
                                      /* printf ("3 "); */

                                      Nic[ii] += 1;
                                      for (locus = 0; locus < Nl; locus++)
                                        {
                                          Count[Zi[ind] * NM +
                                                locus * Missingallele +
                                                Xi[ind * PN +
                                                   Ploidy * locus] - 1]++;
                                          if (Ploidy == 2)
                                            Count[Zi[ind] * NM +
                                                  locus * Missingallele +
                                                  Xi[ind * PN +
                                                     Ploidy * locus + 1] -
                                                  1]++;
                                        }
                                    }
                                  else
                                    {
                                      /* printf ("4 "); */

                                      /* delete one population */
                                      Newk = Currk - 1;

                                      /* Update Nic */
                                      Nic[ii]++;

                                      /* Copy Nic */
                                      kk = sizeof (int) * Currk;
                                      Nica = malloc (kk);
                                      if (Nica == NULL)
                                        alert ();
                                      memcpy (Nica, Nic, kk);
                                      free (Nic);
                                      Nic = malloc (sizeof (int) * Newk);
                                      if (Nic == NULL)
                                        alert ();

                                      for (pop = 0; pop < Currk; pop++)
                                        {
                                          if (pop < Zi[ind])
                                            Nic[pop] = Nica[pop];
                                          if (pop > Zi[ind])
                                            Nic[pop - 1] = Nica[pop];
                                        }

                                      /* Update Count */
                                      for (locus = 0; locus < Nl; locus++)
                                        {
                                          Count[ii * NM +
                                                locus * Missingallele +
                                                Xi[ind * PN +
                                                   Ploidy * locus] - 1]++;
                                          if (Ploidy == 2)
                                            Count[ii * NM +
                                                  locus * Missingallele +
                                                  Xi[ind * PN +
                                                     Ploidy * locus + 1] -
                                                  1]++;
                                        }

                                      /* Copy Count */
                                      kk = sizeof (int) * (Currk * NM);
                                      Counta = malloc (kk);
                                      if (Counta == NULL)
                                        alert ();
                                      memcpy (Counta, Count, kk);

                                      free (Count);
                                      Count =
                                        calloc (Newk * NM, sizeof (int));
                                      if (Count == NULL)
                                        alert ();

                                      for (pop = 0; pop < Currk; pop++)
                                        {
                                          if (pop < Zi[ind])
                                            {
                                              for (locus = 0; locus < Nl;
                                                   locus++)
                                                for (allele = 0;
                                                     allele < Missingallele;
                                                     allele++)
                                                  Count[pop * NM +
                                                        locus *
                                                        Missingallele +
                                                        allele] =
                                                    Counta[pop * NM +
                                                           locus *
                                                           Missingallele +
                                                           allele];
                                            }
                                          if (pop > Zi[ind])
                                            {
                                              for (locus = 0; locus < Nl;
                                                   locus++)
                                                for (allele = 0;
                                                     allele < Missingallele;
                                                     allele++)
                                                  Count[(pop - 1) * NM +
                                                        locus *
                                                        Missingallele +
                                                        allele] =
                                                    Counta[pop * NM +
                                                           locus *
                                                           Missingallele +
                                                           allele];
                                            }
                                        }

                                      /* Update Zi */
                                      mm = Zi[ind];
                                      Zi[ind] = ii;
                                      for (ii = 0; ii < Ni; ii++)
                                        if (Zi[ii] > mm)
                                          Zi[ii] = Zi[ii] - 1;


                                      /* Update Currk */
                                      Currk = Newk;

                                      /* Free auxiliary parameters */
                                      free (Nica);
                                      free (Counta);

                                    }   /* end of if ( ii == Zi [ind]  ...) */

                                }       /* end of if ( Nic >...) */

                            }   /* end of ind repeats */

                        }       /* end of if( ( mcmc - 1 ) % SG < SAMSfreq ) */


                      /* Update Alpha */
                      if (Alpha == 0.0)
                        UpdateAlpha (&EsAlpha, Sdforalpha, Currk, Shape,
                                     Scale, Ni);

                      /* Update Lambda */
                      if (Lambda == -1)
                        UpdateLambdaSingle (EsLambda, Sdforlambda, Lambdalim,
                                            Currk, Nl, Missingallele,
                                            Maxallele, Count, NM);

                      if (Lambda == 0)
                        UpdateLambdaUnique (EsLambda, Sdforlambda, Lambdalim,
                                            Currk, Nl, Missingallele,
                                            Maxallele, Count, NM);



                      /* Record the sampled parameters */
                      if (mcmc % Thi == 0 && mcmc > Burnin)
                        {
                          ii = (mcmc - Startsamp) / Thi;
                          for (ind = 0; ind < Ni; ind++)
                            sampledZ[ii * Ni + ind] = Zi[ind];
                          if (Alpha == 0.0)
                            meanAlpha += EsAlpha;
                          if (Lambda == -1)
                            meanLambda[0] += EsLambda[0];
                          if (Lambda == 0.0)
                            for (locus = 0; locus < Nl; locus++)
                              meanLambda[locus] += EsLambda[locus];
                        }

                    }           /* end of mcmc */


                  /* Ouptput results */
                  /* sampledZ */
                  Output = malloc (sizeof (char) * (Mfn + 20));
                  if (Output == NULL)
                    alert ();
                  sprintf (Output, "%s_Z_run%d.txt", Input, run);
                  fp = fopen (Output, "w");
                  fprintf (fp, "Start with K=%d\n", Inik);
                  for (ii = 0; ii < Nswb; ii++)
                    {
                      for (ind = 0; ind < Ni; ind++)
                        fprintf (fp, "%3u", sampledZ[ii * Ni + ind] + 1);
                      fprintf (fp, "\n");
                    }
                  fclose (fp);
                  free (Output);

                  /* sampledAlpha */
                  if (Alpha == 0.0)
                    {
                      Output = malloc (sizeof (char) * (Mfn + 25));
                      if (Output == NULL)
                        alert ();
                      sprintf (Output, "%s_meanAlpha_run%d.txt", Input, run);
                      fp = fopen (Output, "w");
                      fprintf (fp, "%.6f\n", meanAlpha / (double) Nswb);
                      fclose (fp);
                      free (Output);
                    }

                  /* sampledLambda */
                  if (Lambda == 0.0)
                    {
                      Output = malloc (sizeof (char) * (Mfn + 25));
                      if (Output == NULL)
                        alert ();
                      sprintf (Output, "%s_meanLambda_run%d.txt", Input, run);
                      fp = fopen (Output, "w");
                      for (locus = 0; locus < Nl; locus++)
                        fprintf (fp, "%.6f\n",
                                 meanLambda[locus] / (double) Nswb);
                      fclose (fp);
                      free (Output);
                      free (meanLambda);
                    }

                  if (Lambda == -1)
                    {
                      Output = malloc (sizeof (char) * (Mfn + 25));
                      if (Output == NULL)
                        alert ();
                      sprintf (Output, "%s_meanLambda_run%d.txt", Input, run);
                      fp = fopen (Output, "w");
                      fprintf (fp, "%.6f\n", meanLambda[0] / (double) Nswb);
                      fclose (fp);
                      free (Output);
                      free (meanLambda);
                    }

                  if (Lambda == -2)
                    {
                      Output = malloc (sizeof (char) * (Mfn + 25));
                      if (Output == NULL)
                        alert ();
                      sprintf (Output, "%s_Lambda_run%d.txt", Input, run);
                      fp = fopen (Output, "w");
                      for (locus = 0; locus < Nl; locus++)
                        fprintf (fp, "%.6f\n", EsLambda[locus]);
                      fclose (fp);
                      free (Output);
                    }

                  /* Acceptance rate */
                  if (SAMSfreq > 0)
                    {
                      Output = malloc (sizeof (char) * (Mfn + 25));
                      if (Output == NULL)
                        alert ();
                      sprintf (Output, "%s_Acrate_run%d.txt", Input, run);
                      fp = fopen (Output, "w");

                      fprintf (fp, "MCMC iteration ");
                      for (ii = 1; ii < Nw; ii++)
                        fprintf (fp, "%8d ", ii * Sw);
                      fprintf (fp, "%8d\n", Burnin + Ite);

                      fprintf (fp, "split (freq)  ");
                      for (ii = 0; ii < Nw; ii++)
                        fprintf (fp, "%6d   ", Acsplit[ii]);
                      fprintf (fp, "\n");

                      fprintf (fp, "split (rate)     ");
                      for (ii = 0; ii < Nw; ii++)
                        {
                          if (Ocsplit[ii] == 0)
                            {
                              fprintf (fp, "%.4f   ", 0.0);
                            }
                          else
                            {
                              fprintf (fp, "%.4f   ",
                                       (double) Acsplit[ii] / Ocsplit[ii]);
                            }
                        }
                      fprintf (fp, "\n");

                      fprintf (fp, "merge (freq)  ");
                      for (ii = 0; ii < Nw; ii++)
                        fprintf (fp, "%6d   ", Acmerge[ii]);
                      fprintf (fp, "\n");

                      fprintf (fp, "merge (rate)     ");
                      for (ii = 0; ii < Nw; ii++)
                        {
                          if (Ocmerge[ii] == 0)
                            {
                              fprintf (fp, "%.4f   ", 0.0);
                            }
                          else
                            {
                              fprintf (fp, "%.4f   ",
                                       (double) Acmerge[ii] / Ocmerge[ii]);
                            }
                        }
                      fprintf (fp, "\n");

                      free (Acsplit);
                      free (Acmerge);
                      free (Ocsplit);
                      free (Ocmerge);

                      fprintf (fp,
                               "The chain starts with K=%d and ends with K=%d\n",
                               Inik, Currk);
                      fclose (fp);
                      free (Output);
                    }

                  /* Free */
                  free (Zi);
                  free (Nic);
                  free (EsLambda);

                  /* Co-assignment probabilities */
                  Coaprob = calloc (Ni * (Ni - 1) / 2, sizeof (double));
                  Coassignprobs (sampledZ, Ni, Nswb, Coaprob);

                  Output = malloc (sizeof (char) * (Mfn + 30));
                  if (Output == NULL)
                    alert ();
                  sprintf (Output, "%s_coaprob_run%d.txt", Input, run);
                  fp = fopen (Output, "w");

                  fprintf (fp, "co-assignment probabilities\n");
                  fprintf (fp,
                           "Individual numbers and corresponding probability\n");
                  kk = 0;
                  for (ii = 0; ii < (Ni - 1); ii++)
                    {
                      for (jj = ii + 1; jj < Ni; jj++)
                        {
                          fprintf (fp, "%d %d %f\n", ii + 1, jj + 1,
                                   Coaprob[kk]);
                          kk++;
                        }
                    }

                  free (Output);
                  free (Coaprob);

                  /* Mean partition */
                  if (Meanpartcal)
                    {
                      printf
                        ("calculating the mean partition of run %d of %s",
                         run, Input);
                      meanPart = malloc (sizeof (int) * Ni);
                      if (meanPart == NULL)
                        alert ();
                      Meanpartdis (sampledZ, Ni, Nswb, meanPart);
                      printf ("...done\n");
                      putchar ('\n');

                      Output = malloc (sizeof (char) * (Mfn + 30));
                      if (Output == NULL)
                        alert ();
                      sprintf (Output, "%s_meanpartition_run%d.txt", Input,
                               run);
                      fp = fopen (Output, "w");
                      for (ii = 0; ii < Ni; ii++)
                        fprintf (fp, "%d\n", meanPart[ii]);
                      fclose (fp);

                      free (meanPart);
                      free (Output);
                    }

                  free (sampledZ);
                  free (Count);

                }               /* end of run */

              free (Xi);
              free (Missing);
              free (Maxallele);

            }                   /* if ( ( fp = fopen ( Input, "r" ) ) == NULL ) else */

          free (Input);

        }                       /* if ( ( fp = fopen( inputfilenames, "r" ) ) == NULL ) else */

    }                           /* end of file repeat */


  /* End */
  printf ("Job is finished\n");
  return (0);

}
