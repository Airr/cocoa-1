/*----For random number generator------------------------------------------------------------------------------------------------*/
int ix, iy, iz;

/*----initialize the random seed (by STRUCTURE)----------------------------------------------------------------------------------*/
int
Randomize (int seed)
{
  if (seed == -1)
    seed = (int) time (NULL);
  srand (seed);
  return (seed);
}

/*----generate a random real number between 0 and 1 (by Ayres1998)---------------------------------------------------------------*/
double
WichHill ()
{
  ix = (171 * ix) % 30269;
  iy = (172 * iy) % 30307;
  iz = (170 * iz) % 30323;
  return (fmod ((ix / 30269.0) + (iy / 30307.0) + (iz / 30323.0), 1.0));
}

/*----generate a random real number ---------------------------------------------------------------------------------------------*/
double
RandomReal (double low, double high)
{
  return (low + WichHill () * (high - low));
}

/*----generate a random integer -------------------------------------------------------------------------------------------------*/
int
RandomInteger (int low, int high)
{
  int k;

  k = (int) (WichHill () * (high - low + 1));
  return (low + k);
}

/*----generate a real number from Uniform (0,1) (by STRUCTURE with some modifications)-------------------------------------------*/
double
rnd ()
{
  double value;

  do
    value = WichHill ();
  while ((value == 0.0) || (value == 1.0));

  return value;
}

/*----generate a real number from Gamma (n, lambda) (by STRUCTURE), E[x] = n/lambda, V[x] = n/lambda^2 --------------------------*/
double
RGamma (double n, double lambda)
{
  double ar;
  double w;

  double x = 0.0;
  if (n < 1)
    {
      const double E = 2.71828182;
      const double b = (n + E) / E;
      double p = 0.0;
    one:
      p = b * rnd ();
      if (p > 1)
        goto two;
      x = exp (log (p) / n);
      if (x > -log (rnd ()))
        goto one;
      goto three;
    two:
      x = -log ((b - p) / n);
      if (((n - 1) * log (x)) < log (rnd ()))
        goto one;
    three:;
    }
  else
    {
      if (n == 1.0)
        {                       /* exponential random variable, from Ripley, 1987, P230  */
          double a = 0.0;
          double u, u0, ustar;
        ten:
          u = rnd ();
          u0 = u;
        twenty:
          ustar = rnd ();
          if (u < ustar)
            goto thirty;
          u = rnd ();
          if (u < ustar)
            goto twenty;
          a++;
          goto ten;
        thirty:
          return (a + u0) / lambda;
        }
      else
        {
          double static nprev = 0.0;
          double static c1 = 0.0;
          double static c2 = 0.0;
          double static c3 = 0.0;
          double static c4 = 0.0;
          double static c5 = 0.0;
          double u1;
          double u2;
          if (n != nprev)
            {
              c1 = n - 1.0;
              ar = 1.0 / c1;
              c2 = ar * (n - 1 / (6 * n));
              c3 = 2 * ar;
              c4 = c3 + 2;
              if (n > 2.5)
                c5 = 1 / sqrt (n);
            }
        four:
          u1 = rnd ();
          u2 = rnd ();
          if (n <= 2.5)
            goto five;
          u1 = u2 + c5 * (1 - 1.86 * u1);
          if ((u1 <= 0) || (u1 >= 1))
            goto four;
        five:
          w = c2 * u2 / u1;
          if (c3 * u1 + w + 1.0 / w < c4)
            goto six;
          if (c3 * log (u1) - log (w) + w >= 1)
            goto four;
        six:
          x = c1 * w;
          nprev = n;
        }
    }

  return x / lambda;
}


/*----generate real numbers from N (0,1) ( by STRUCTURE )------------------------------------------------------------------------*/
double
snorm ()                        /*was snorm(void) -- JKP */
{
  static double a[32] = {
    0.0, 3.917609E-2, 7.841241E-2, 0.11777, 0.1573107, 0.1970991, 0.2372021,
    0.2776904,
    0.3186394, 0.36013, 0.4022501, 0.4450965, 0.4887764, 0.5334097, 0.5791322,
    0.626099, 0.6744898, 0.7245144, 0.7764218, 0.8305109, 0.8871466,
    0.9467818,
    1.00999, 1.077516, 1.150349, 1.229859, 1.318011, 1.417797, 1.534121,
    1.67594,
    1.862732, 2.153875
  };
  static double d[31] = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.2636843, 0.2425085, 0.2255674, 0.2116342,
    0.1999243,
    0.1899108, 0.1812252, 0.1736014, 0.1668419, 0.1607967, 0.1553497,
    0.1504094,
    0.1459026, 0.14177, 0.1379632, 0.1344418, 0.1311722, 0.128126, 0.1252791,
    0.1226109, 0.1201036, 0.1177417, 0.1155119, 0.1134023, 0.1114027,
    0.1095039
  };
  static double t[31] = {
    7.673828E-4, 2.30687E-3, 3.860618E-3, 5.438454E-3, 7.0507E-3, 8.708396E-3,
    1.042357E-2, 1.220953E-2, 1.408125E-2, 1.605579E-2, 1.81529E-2,
    2.039573E-2,
    2.281177E-2, 2.543407E-2, 2.830296E-2, 3.146822E-2, 3.499233E-2,
    3.895483E-2,
    4.345878E-2, 4.864035E-2, 5.468334E-2, 6.184222E-2, 7.047983E-2,
    8.113195E-2,
    9.462444E-2, 0.1123001, 0.136498, 0.1716886, 0.2276241, 0.330498,
    0.5847031
  };
  static double h[31] = {
    3.920617E-2, 3.932705E-2, 3.951E-2, 3.975703E-2, 4.007093E-2, 4.045533E-2,
    4.091481E-2, 4.145507E-2, 4.208311E-2, 4.280748E-2, 4.363863E-2,
    4.458932E-2,
    4.567523E-2, 4.691571E-2, 4.833487E-2, 4.996298E-2, 5.183859E-2,
    5.401138E-2,
    5.654656E-2, 5.95313E-2, 6.308489E-2, 6.737503E-2, 7.264544E-2,
    7.926471E-2,
    8.781922E-2, 9.930398E-2, 0.11556, 0.1404344, 0.1836142, 0.2790016,
    0.7010474
  };
  static long inr;
  static double snorm, u, ss, ustar, anr, w, y, tt;
  u = rnd ();                   /* was ranf--JKP */
  ss = 0.0;
  if (u > 0.5)
    ss = 1.0;
  u += (u - ss);
  u = 32.0 * u;
  inr = (long) (u);
  if (inr == 32)
    inr = 31;
  if (inr == 0)
    goto S100;
/*
                                START CENTER
*/
  ustar = u - (double) inr;
  anr = *(a + inr - 1);
S40:
  if (ustar <= *(t + inr - 1))
    goto S60;
  w = (ustar - *(t + inr - 1)) ** (h + inr - 1);
S50:
/*
                                EXIT   (BOTH CASES)
*/
  y = anr + w;
  snorm = y;
  if (ss == 1.0)
    snorm = -y;
  return snorm;
S60:
/*
                                CENTER CONTINUED
*/
  u = rnd ();                   /*was ranf--JKP */
  w = u * (*(a + inr) - anr);
  tt = (0.5 * w + anr) * w;
  goto S80;
S70:
  tt = u;
  ustar = rnd ();               /*was ranf--JKP */
S80:
  if (ustar > tt)
    goto S50;
  u = rnd ();                   /*was ranf--JKP */
  if (ustar >= u)
    goto S70;
  ustar = rnd ();               /*was ranf--JKP */
  goto S40;
S100:
/*
                                START TAIL
*/
  inr = 6;
  anr = *(a + 31);
  goto S120;
S110:
  anr += *(d + inr - 1);
  inr += 1;
S120:
  u += u;
  if (u < 1.0)
    goto S110;
  u -= 1.0;
S140:
  w = u ** (d + inr - 1);
  tt = (0.5 * w + anr) * w;
  goto S160;
S150:
  tt = u;
S160:
  ustar = rnd ();               /*was ranf--JKP */
  if (ustar > tt)
    goto S50;
  u = rnd ();                   /*was ranf--JKP */
  if (ustar >= u)
    goto S150;
  u = rnd ();                   /*was ranf--JKP */
  goto S140;
}


/*----generate real number from N(mu, sd) (by STRUCTURE)-------------------------------------------------------------------------*/
double
RNormal (double mu, double sd)
     /* Returns Normal rv with mean mu, variance sigsq.   
        Uses snorm function of Brown and Lovato.  By JKP */
{
  return (mu + sd * snorm ());
}


/*----calculate log of the gamma function (by STRUCTURE)-------------------------------------------------------------------------*/
double
mylgamma (double zpo)
{
/* LGAMMA function

   double_value = lgamma(<double_value > 0.>)

   returns the natural log of the gamma function

Uses Lanczos-type approximation to ln(gamma) for zpo > 0.   
Reference:                                          
 Lanczos, C. 'A precision approximation of the gamma   
    function', J. SIAM Numer. Anal., B, 1, 86-96, 1964.   

Original was in FORTRAN
Accuracy: About 14 significant digits except for small regions   
          in the vicinity of 1 and 2.  
Programmer: Alan Miller   
          CSIRO Division of Mathematics & Statistics   
Latest revision - 17 April 1988   
  
Translated and modified into C by Peter Beerli 1997 
Tested against Mathematica's Log[Gamma[x]]
*/

  double alg[9] = { 0.9999999999995183, 676.5203681218835,
    -1259.139216722289, 771.3234287757674, -176.6150291498386,
    12.50734324009056, -0.1385710331296526, 9.934937113930748e-6,
    1.659470187408462e-7
  };
  double lnsqrt2pi = 0.9189385332046727;
  double result;
  long jlg;
  double tmp;
  if (zpo <= 0.)
    {
      fprintf (stderr, "lgamma function failed with wrong input (%lf)\n",
               zpo);
      exit (-1);
    }
  result = 0.;
  tmp = zpo + 7.;
  for (jlg = 9; jlg >= 2; --jlg)
    {
      result += alg[jlg - 1] / tmp;
      tmp -= 1.;
    }
  result += alg[0];
  result =
    log (result) + lnsqrt2pi - (zpo + 6.5) + (zpo - 0.5) * log (zpo + 6.5);
  return result;
}


/*----Returns a random number between 0 and total-1, according the probalities list, *probs.-------------------------------------*/
    /*sum is the sum of the probabirities (by STRUCTURE with some modifications) */
int
PickAnOption (int total, double sum, const double *probs)
{
  int option;
  double random;
  double sumsofar = 0.0;

  random = RandomReal (0, sum); /*Get uniform random real in this range */
  for (option = 0; option < total; option++)    /*Figure out which popn this is */
    {
      sumsofar += probs[option];
      if (random <= sumsofar)
        break;
    }

  return option;

}

int
PickAnOptionL (int total, double *probs)
{
  int option;
  double random, sum, max, value;
  double sumsofar = 0.0;

  max = probs[0];
  if (max = 0.0)
    max = probs[1];
  for (option = 1; option < total; option++)
    {
      value = probs[option];
      if (value > max && value != 0.0)
        max = value;
    }

  sum = 0.0;
  for (option = 0; option < total; option++)
    {
      value = probs[option];
      if (value != 0.0)
        {
          value -= max;
          probs[option] = exp (value);
        }
      sum += probs[option];
    }

  random = RandomReal (0, sum); /*Get uniform random real in this range */
  for (option = 0; option < total; option++)    /*Figure out which popn this is */
    {
      sumsofar += probs[option];
      if (random <= sumsofar)
        break;
    }

  return option;

}

/*----Return 0 with probability a/(a+b) and return 1 with probability b/(a+b) ---------------------------------------------------*/
int
PickAnOptionS (const double a, const double b)
{
  int option;
  double random, sum;

  sum = a + b;
  random = RandomReal (0, sum);
  if (random < a)
    {
      option = 0;
    }
  else
    {
      option = 1;
    }

  return option;

}



/*----calculate likelihood-------------------------------------------------------------------------------------------------------*/

        /* for algorithms that integrate P. calculate log likelihood based on allele counts of other individuals */
double
Likelihoodi (int ploidy, int xa, int xb, const int *count, const int missing,
             const int maxallele, const double lambda)
{
  int aa, countsum = 0;
  double temp = 1.0, ml;

  if (missing == 0)
    {
      if (ploidy == 2)
        {
          if (xa == xb)
            {
              temp *= (lambda + (double) count[xa - 1] + 1.0);
              temp *= (lambda + (double) count[xb - 1]);
              for (aa = 0; aa < maxallele; aa++)
                countsum += count[aa];
              ml = (double) maxallele *lambda;
              temp /= (ml + 1.0 + (double) countsum);
              temp /= (ml + (double) countsum);
              return (temp);
            }
          else
            {
              temp *= 2.0;
              temp *= (lambda + (double) count[xa - 1]);
              temp *= (lambda + (double) count[xb - 1]);
              for (aa = 0; aa < maxallele; aa++)
                countsum += count[aa];
              ml = (double) maxallele *lambda;
              temp /= (ml + 1.0 + (double) countsum);
              temp /= (ml + (double) countsum);
              return (temp);
            }
        }
      else
        {
          temp *= (lambda + count[xa - 1]);
          for (aa = 0; aa < maxallele; aa++)
            countsum += count[aa];
          temp /= ((double) maxallele * lambda + (double) countsum);
          return (temp);
        }
    }
  else
    {
      return (1.0);
    }
}

        /* for algorithms that integrate P. calculate log likelihood without allele counts of other individuals */
double
Likelihoodp (int ploidy, int xa, int xb, const int missing,
             const int maxallele, const double lambda)
{
  double temp = 1.0, ml;

  if (missing == 0)
    {
      if (ploidy == 2)
        {
          if (xa == xb)
            {
              temp *= (lambda + 1.0);
              temp *= lambda;
              ml = (double) maxallele *lambda;
              temp /= (ml + 1.0);
              temp /= (ml);
              return (temp);
            }
          else
            {
              temp *= 2.0;
              temp *= (lambda * lambda);
              ml = (double) maxallele *lambda;
              temp /= (ml + 1.0);
              temp /= (ml);
              return (temp);
            }
        }
      else
        {
          temp *= lambda;
          temp /= ((double) maxallele * lambda);
          return (temp);
        }
    }
  else
    {
      return (1.0);
    }
}

/*----Convert the input data file -----------------------------------------------------------------------------------------------*/
void
Convert (int *x, const int maxalleleinput, const int nl, const int ni,
         const int ploidy)
{
  int locusc, allelec, indc, min, max, mm, pp, nn;

  for (locusc = 0; locusc < nl; locusc++)
    {
      min = maxalleleinput;
      max = 0;
      for (allelec = ploidy * locusc;
           allelec <= (ploidy * locusc + ploidy - 1); allelec++)
        {
          for (indc = 0; indc < ni; indc++)
            {
              if (x[indc * nl * ploidy + allelec] > 0
                  && x[indc * nl * ploidy + allelec] < min)
                min = x[indc * nl * ploidy + allelec];
              if (x[indc * nl * ploidy + allelec] > max)
                max = x[indc * nl * ploidy + allelec];
            }
        }

      nn = 1;
      for (mm = min; mm <= max; mm++)
        {
          pp = 0;
          for (allelec = ploidy * locusc;
               allelec <= (ploidy * locusc + ploidy - 1); allelec++)
            {
              for (indc = 0; indc < ni; indc++)
                {
                  if (x[indc * nl * ploidy + allelec] == mm)
                    {
                      x[indc * nl * ploidy + allelec] = nn;
                      pp++;
                    }
                }
            }
          if (pp > 0)
            nn++;
        }
    }
}

/*----Shuffle an array------------------------------------------------------------------------------------------------------------*/
void
Shuffle (int *asl, int length)
{
  int isl, jsl, target;

  for (isl = 0; isl < length; isl++)
    {
      jsl = asl[isl];
      target = RandomInteger (0, length - 1);
      asl[isl] = asl[target];
      asl[target] = jsl;
    }
}

/*----Sort an array---------------------------------------------------------------------------------------------------------------*/
void
Sort (int *aso, int n)
{
  int iso, jso, temps;
  for (iso = 0; iso < n - 1; iso++)
    for (jso = 1; jso < n - iso; jso++)
      if (aso[jso - 1] > aso[jso])
        {
          temps = aso[jso - 1];
          aso[jso - 1] = aso[jso];
          aso[jso] = temps;
        }

}

/*----Initialize Z and Nic--------------------------------------------------------------------------------------------------------*/
int
InitializeZ (int *z, int ni, double alpha)
{

  int i, j, k, *nic;
  double *prob, sum;

  nic = calloc (ni, sizeof (int));
  prob = calloc (ni, sizeof (double));

  z[0] = 0;
  k = 1;
  nic[0] = 1;

  for (i = 1; i < ni; i++)
    {
      sum = 0.0;
      for (j = 0; j < k; j++)
        {
          prob[j] = (double) nic[j];
          sum += prob[j];
        }
      prob[k] = alpha;
      sum += prob[k];

      z[i] = PickAnOption (k + 1, sum, prob);

      if (z[i] == k)
        {
          nic[k] = 1;
          k++;
        }
      else
        {
          nic[z[i]]++;
        }
    }

  free (nic);
  free (prob);
  return (k);

}

/*----alert when allocation of memory is failed-----------------------------------------------------------------------------------*/
void
alert (void)
{
  puts ("memory allocation failure. try again");
}


/* Split populations in the SAMS sampler -----------------------------------------------------------------------------------------*/
double
SAMSsplit (int ni, int nl, int *maxallele, int missingallele, int *x,
           int *missing, double *lambda, int *z, int is, int js, int si,
           int *niss, int *nic, int *propz, double alpha, int ploidy)
{
  /*  is, js; selected individuals
     si ;new population
     niss; ni in si and sj
     propz; proposal of z
   */

  int aa, bb, locuss, inds, pos, pos2, pos3, pos4;
  int *memins;
  int s, *counts, *countsi, *countsj;
  double likeratio, transprob, likesi, likesj, temp;


  s = z[js];                    /* s is the population to be splited */
  niss[0] = 1;
  niss[1] = 1;                  /* ni in si and sj */

  /* allele count in s ( to be splited ), si ( new population ) and sj ( to be remained ) */
  counts = calloc (nl * missingallele, sizeof (int));
  if (counts == NULL)
    alert ();
  countsi = calloc (nl * missingallele, sizeof (int));
  if (countsi == NULL)
    alert ();
  countsj = calloc (nl * missingallele, sizeof (int));
  if (countsj == NULL)
    alert ();

  likeratio = 0.0;
  for (locuss = 0; locuss < nl; locuss++)
    {
      pos = maxallele[locuss];
      pos2 = locuss * missingallele;
      pos3 = is * ploidy * nl + locuss * ploidy;
      pos4 = js * ploidy * nl + locuss * ploidy;

      /* Likelihood in si and sj and allele count in si and sj */
      likeratio +=
        log (Likelihoodi
             (ploidy, x[pos3], x[pos3 + ploidy - 1], countsi + pos2,
              missing[is * nl + locuss], pos, lambda[locuss]));
      countsi[pos2 + x[pos3] - 1]++;
      if (ploidy == 2)
        countsi[pos2 + x[pos3 + 1] - 1]++;
      likeratio +=
        log (Likelihoodi
             (ploidy, x[pos4], x[pos4 + ploidy - 1], countsj + pos2,
              missing[js * nl + locuss], pos, lambda[locuss]));
      countsj[pos2 + x[pos4] - 1]++;
      if (ploidy == 2)
        countsj[pos2 + x[pos4 + 1] - 1]++;

      /* Likelihood in s and allele count in s */
      likeratio -=
        log (Likelihoodi
             (ploidy, x[pos3], x[pos3 + ploidy - 1], counts + pos2,
              missing[is * nl + locuss], pos, lambda[locuss]));
      counts[pos2 + x[pos3] - 1]++;
      if (ploidy == 2)
        counts[pos2 + x[pos3 + 1] - 1]++;
      likeratio -=
        log (Likelihoodi
             (ploidy, x[pos4], x[pos4 + ploidy - 1], counts + pos2,
              missing[js * nl + locuss], pos, lambda[locuss]));
      counts[pos2 + x[pos4] - 1]++;
      if (ploidy == 2)
        counts[pos2 + x[pos4 + 1] - 1]++;
    }

  transprob = 0.0;
  if (nic[s] > 2)               /* in the case where other members than is and js exist */
    {
      /* members in s except for is and js */
      memins = malloc (sizeof (int) * (nic[s] - 2));
      if (memins == NULL)
        alert ();

      aa = 0;
      bb = 0;
      while (aa < ni)
        {
          if (z[aa] == s && aa != is && aa != js)
            {
              memins[bb] = aa;
              bb++;
            }
          if (bb == nic[s] - 2)
            break;
          aa++;
        }

      /* Shuffle memins */
      Shuffle (memins, nic[s] - 2);

      /* Assign individuals in memins into si or sj */
      for (inds = 0; inds < (nic[s] - 2); inds++)
        {
          likesi = 1.0;
          likesj = 1.0;

          /* Assign memins [inds] */
          for (locuss = 0; locuss < nl; locuss++)
            {
              pos = memins[inds] * ploidy * nl + locuss * ploidy;
              pos2 = memins[inds] * nl + locuss;
              pos3 = locuss * missingallele;

              likesi *=
                Likelihoodi (ploidy, x[pos], x[pos + ploidy - 1],
                             countsi + pos3, missing[pos2], maxallele[locuss],
                             lambda[locuss]);
              likesj *=
                Likelihoodi (ploidy, x[pos], x[pos + ploidy - 1],
                             countsj + pos3, missing[pos2], maxallele[locuss],
                             lambda[locuss]);
              likeratio -=
                log (Likelihoodi
                     (ploidy, x[pos], x[pos + ploidy - 1], counts + pos3,
                      missing[pos2], maxallele[locuss], lambda[locuss]));

            }

          likesi *= niss[0];
          likesj *= niss[1];

          aa = PickAnOptionS (likesi, likesj);
          if (aa == 0)
            {
              propz[memins[inds]] = si;
              transprob += log (likesi / (likesi + likesj));
              likesi /= niss[0];
              likeratio += log (likesi);

              for (locuss = 0; locuss < nl; locuss++)
                {
                  pos =
                    locuss * missingallele + x[memins[inds] * ploidy * nl +
                                               ploidy * locuss] - 1;
                  countsi[pos]++;
                  counts[pos]++;
                  if (ploidy == 2)
                    {
                      pos2 =
                        locuss * missingallele +
                        x[memins[inds] * ploidy * nl + ploidy * locuss + 1] -
                        1;
                      countsi[pos2]++;
                      counts[pos2]++;
                    }
                }

              niss[0]++;

            }
          else
            {
              transprob += log (likesj / (likesi + likesj));
              likesj /= niss[1];
              likeratio += log (likesj);

              for (locuss = 0; locuss < nl; locuss++)
                {
                  pos =
                    locuss * missingallele + x[memins[inds] * ploidy * nl +
                                               ploidy * locuss] - 1;
                  countsj[pos]++;
                  counts[pos]++;
                  if (ploidy == 2)
                    {
                      pos2 =
                        locuss * missingallele +
                        x[memins[inds] * ploidy * nl + ploidy * locuss + 1] -
                        1;
                      countsj[pos2]++;
                      counts[pos2]++;
                    }
                }

              niss[1]++;

            }

        }                       /* end of inds repeat */

      free (memins);

    }                           /* end of if ( nic [s] > 2 ) */

  free (counts);
  free (countsi);
  free (countsj);

  /* calculate MH prob. */
  temp =
    mylgamma ((double) niss[0]) + mylgamma ((double) niss[1]) -
    mylgamma ((double) nic[s]) + log (alpha);
  return (likeratio + temp - transprob);

}



/* Merge populations in the SAMS sampler -----------------------------------------------------------------------------------------*/
double
SAMSmerge (int ni, int nl, int *maxallele, int missingallele, int *x,
           int *missing, double *lambda, int *z, int is, int js, int *niss,
           int *nic, double alpha, int ploidy)
{
  /* si; population to be deleted 
     sj; population to be remained 
   */

  int aa, bb, locuss, inds, pos, pos2, pos3, pos4;
  int *memins;
  int si, sj, *counts, *countsi, *countsj;
  double likeratio, transprob, likesi, likesj, temp;

  si = z[is];
  sj = z[js];
  niss[0] = 1;
  niss[1] = 1;

  /* allele count in sj ( to be remained), si ( to be deleted ) and s ( merged population ) */
  countsj = calloc (nl * missingallele, sizeof (int));
  if (countsj == NULL)
    alert ();
  countsi = calloc (nl * missingallele, sizeof (int));
  if (countsi == NULL)
    alert ();
  counts = calloc (nl * missingallele, sizeof (int));
  if (counts == NULL)
    alert ();

  likeratio = 0.0;
  for (locuss = 0; locuss < nl; locuss++)
    {
      pos = maxallele[locuss];
      pos2 = locuss * missingallele;
      pos3 = is * ploidy * nl + locuss * ploidy;
      pos4 = js * ploidy * nl + locuss * ploidy;

      /* Likelihood in si and sj and allele count in si and sj */
      likeratio -=
        log (Likelihoodi
             (ploidy, x[pos3], x[pos3 + ploidy - 1], countsi + pos2,
              missing[is * nl + locuss], pos, lambda[locuss]));
      countsi[pos2 + x[pos3] - 1]++;
      if (ploidy == 2)
        countsi[pos2 + x[pos3 + 1] - 1]++;
      likeratio -=
        log (Likelihoodi
             (ploidy, x[pos4], x[pos4 + ploidy - 1], countsj + pos2,
              missing[js * nl + locuss], pos, lambda[locuss]));
      countsj[pos2 + x[pos4] - 1]++;
      if (ploidy == 2)
        countsj[pos2 + x[pos4 + 1] - 1]++;

      /* Likelihood in s and allele count in s */
      likeratio +=
        log (Likelihoodi
             (ploidy, x[pos3], x[pos3 + ploidy - 1], counts + pos2,
              missing[is * nl + locuss], pos, lambda[locuss]));
      counts[pos2 + x[pos3] - 1]++;
      if (ploidy == 2)
        counts[pos2 + x[pos3 + 1] - 1]++;
      likeratio +=
        log (Likelihoodi
             (ploidy, x[pos4], x[pos4 + ploidy - 1], counts + pos2,
              missing[js * nl + locuss], pos, lambda[locuss]));
      counts[pos2 + x[pos4] - 1]++;
      if (ploidy == 2)
        counts[pos2 + x[pos4 + 1] - 1]++;
    }

  transprob = 0.0;
  if (nic[si] > 1 || nic[sj] > 1)       /* in the case where other members than is and js exist */
    {

      /* members in si and sj except for is and js */
      memins = malloc (sizeof (int) * (nic[si] + nic[sj] - 2));
      if (memins == NULL)
        alert ();

      aa = 0;
      bb = 0;
      while (aa < ni)
        {
          if ((z[aa] == si || z[aa] == sj) && aa != is && aa != js)
            {
              memins[bb] = aa;
              bb++;
            }
          if (bb == (nic[si] + nic[sj] - 2))
            break;
          aa++;
        }

      /* Shuffle memins */
      Shuffle (memins, nic[si] + nic[sj] - 2);

      /* calculate densities of assignment individuals in memins into si or sj */
      for (inds = 0; inds < (nic[si] + nic[sj] - 2); inds++)
        {
          likesi = 1.0;
          likesj = 1.0;

          /* Assign memins [inds] into z[memins[inds]] */
          for (locuss = 0; locuss < nl; locuss++)
            {
              pos = memins[inds] * ploidy * nl + locuss * ploidy;
              pos2 = memins[inds] * nl + locuss;
              pos3 = locuss * missingallele;

              likesi *=
                Likelihoodi (ploidy, x[pos], x[pos + ploidy - 1],
                             countsi + pos3, missing[pos2], maxallele[locuss],
                             lambda[locuss]);
              likesj *=
                Likelihoodi (ploidy, x[pos], x[pos + ploidy - 1],
                             countsj + pos3, missing[pos2], maxallele[locuss],
                             lambda[locuss]);
              likeratio +=
                log (Likelihoodi
                     (ploidy, x[pos], x[pos + ploidy - 1], counts + pos3,
                      missing[pos2], maxallele[locuss], lambda[locuss]));
            }

          likesi *= niss[0];
          likesj *= niss[1];

          if (z[memins[inds]] == si)
            {
              transprob += log (likesi / (likesi + likesj));
              likesi /= niss[0];
              likeratio -= log (likesi);

              for (locuss = 0; locuss < nl; locuss++)
                {
                  pos =
                    locuss * missingallele + x[memins[inds] * ploidy * nl +
                                               ploidy * locuss] - 1;
                  countsi[pos]++;
                  counts[pos]++;
                  if (ploidy == 2)
                    {
                      pos2 =
                        locuss * missingallele +
                        x[memins[inds] * ploidy * nl + ploidy * locuss + 1] -
                        1;
                      countsi[pos2]++;
                      counts[pos2]++;
                    }
                }

              niss[0]++;

            }
          else
            {
              transprob += log (likesj / (likesi + likesj));
              likesj /= niss[1];
              likeratio -= log (likesj);

              for (locuss = 0; locuss < nl; locuss++)
                {
                  pos =
                    locuss * missingallele + x[memins[inds] * ploidy * nl +
                                               ploidy * locuss] - 1;
                  countsj[pos]++;
                  counts[pos]++;
                  if (ploidy == 2)
                    {
                      pos2 =
                        locuss * missingallele +
                        x[memins[inds] * ploidy * nl + ploidy * locuss + 1] -
                        1;
                      countsj[pos2]++;
                      counts[pos2]++;
                    }
                }

              niss[1]++;
            }

        }                       /* end of inds repeat */

      free (memins);

    }                           /* end of if ( nic [si] > 1 && ...) */

  free (counts);
  free (countsi);
  free (countsj);

  /* calculate MH prob. */
  temp =
    mylgamma ((double) (nic[si] + nic[sj])) - mylgamma ((double) niss[0]) -
    mylgamma ((double) niss[1]) - log (alpha);
  return (likeratio + temp + transprob);

}



/* Gibbs sampler (when the individual is not a singleton) ------------------------------------------------------------------------*/
int
GibbsNotSingle (int indg, int currk, int *x, int nl, int *count,
                int missingallele, int *missing, int *maxallele,
                double *lambda, int *nic, double alpha, int ploidy, int pn,
                int nm)
{
  int popg, locusg, aa, indgpn, x1, x2;
  double *clia, sum;

  clia = malloc (sizeof (double) * (currk + 1));
  if (clia == NULL)
    alert ();
  for (popg = 0; popg <= currk; popg++)
    clia[popg] = 1.0;
  sum = 0.0;
  indgpn = indg * pn;

  for (locusg = 0; locusg < nl; locusg++)
    {
      x1 = x[indgpn + locusg * ploidy];
      x2 = x[indgpn + locusg * ploidy + ploidy - 1];
      for (popg = 0; popg < currk; popg++)
        {
          clia[popg] *=
            Likelihoodi (ploidy, x1, x2,
                         count + popg * nm + locusg * missingallele,
                         missing[indg * nl + locusg], maxallele[locusg],
                         lambda[locusg]);
        }

      clia[currk] *=
        Likelihoodp (ploidy, x1, x2, missing[indg * nl + locusg],
                     maxallele[locusg], lambda[locusg]);
    }

  for (popg = 0; popg < currk; popg++)
    {
      clia[popg] *= nic[popg];
    }
  clia[currk] *= alpha;

  for (popg = 0; popg <= currk; popg++)
    sum += clia[popg];

  /* assign indg. according to clia */
  aa = PickAnOption ((currk + 1), sum, clia);
  free (clia);
  return (aa);

}



/* Gibbs sampler (when the individual is a singleton) ----------------------------------------------------------------------------*/
int
GibbsSingle (int indg, int currk, int *x, int nl, int *z, int *count,
             int missingallele, int *missing, int *maxallele, double *lambda,
             int *nic, double alpha, int ploidy, int pn, int nm)
{
  int popg, locusg, aa, x1, x2, indgpn;
  double *clia, sum;

  clia = malloc (sizeof (double) * (currk));
  if (clia == NULL)
    alert ();
  for (popg = 0; popg < currk; popg++)
    clia[popg] = 1.0;
  sum = 0.0;
  indgpn = indg * pn;

  for (locusg = 0; locusg < nl; locusg++)
    {
      x1 = x[indgpn + locusg * ploidy];
      x2 = x[indgpn + locusg * ploidy + ploidy - 1];
      for (popg = 0; popg < currk; popg++)
        {
          clia[popg] *=
            Likelihoodi (ploidy, x1, x2,
                         count + popg * nm + locusg * missingallele,
                         missing[indg * nl + locusg], maxallele[locusg],
                         lambda[locusg]);
        }
    }

  for (popg = 0; popg < currk; popg++)
    if (popg == z[indg])
      {
        clia[popg] *= alpha;
      }
    else
      {
        clia[popg] *= nic[popg];
      }

  for (popg = 0; popg < currk; popg++)
    sum += clia[popg];

  /* assign indg. according to clia */
  aa = PickAnOption (currk, sum, clia);
  free (clia);
  return (aa);

}


/* Update alpha ------------------------------------------------------------------------------------------------------------------*/
void
UpdateAlpha (double *esalpha, double sd, int currk, double shape,
             double scale, int ni)
{
  double propalpha, mhprob;
  int aa;

  propalpha = RNormal (*esalpha, sd);
  if (propalpha > 0.0)
    {
      mhprob = 0.0;
      for (aa = 1; aa < ni; aa++)
        mhprob += log ((*esalpha + (double) aa) / (propalpha + (double) aa));
      mhprob +=
        (((double) currk + shape - 2.0) * log (propalpha / *esalpha) -
         scale * (propalpha - *esalpha));

      if (mhprob > log (rnd ()))
        *esalpha = propalpha;
    }
}


/* Update Lambda -----------------------------------------------------------------------------------------------------------------*/
void
UpdateLambdaSingle (double *eslambda, double sd, double lambdalim, int currk,
                    int nl, int missingallele, int *maxallele, int *count,
                    int nm)
{
  double proplambda, clikeli, plikeli, mhprob, temp, temp2;
  int pop, locus, allele, ii, jj, kk;

  proplambda = RNormal (eslambda[0], sd);
  clikeli = 0.0;
  plikeli = 0.0;

  if (proplambda > 1e-20 && proplambda < lambdalim)
    {
      for (pop = 0; pop < currk; pop++)
        {
          for (locus = 0; locus < nl; locus++)
            {
              ii = maxallele[locus];
              kk = 0;
              for (allele = 0; allele < ii; allele++)
                {
                  jj = count[pop * nm + locus * missingallele + allele];
                  clikeli += mylgamma (eslambda[0] + jj);
                  plikeli += mylgamma (proplambda + jj);
                  kk += jj;
                }

              temp = eslambda[0] * (double) ii;
              temp2 = proplambda * (double) ii;
              clikeli += mylgamma (temp);
              plikeli += mylgamma (temp2);
              clikeli -= mylgamma (temp + (double) kk);
              plikeli -= mylgamma (temp2 + (double) kk);
              clikeli -= (mylgamma (eslambda[0]) * (double) ii);
              plikeli -= (mylgamma (proplambda) * (double) ii);
            }
        }

      mhprob = plikeli - clikeli;

      if (mhprob > log (rnd ()))
        {
          for (locus = 0; locus < nl; locus++)
            eslambda[locus] = proplambda;
        }

    }
}



void
UpdateLambdaUnique (double *eslambda, double sd, double lambdalim, int currk,
                    int nl, int missingallele, int *maxallele, int *count,
                    int nm)
{
  double proplambda, clikeli, plikeli, mhprob, temp, temp2;
  int pop, locus, allele, ii, jj, kk;


  for (locus = 0; locus < nl; locus++)
    {
      proplambda = RNormal (eslambda[locus], sd);
      clikeli = 0.0;
      plikeli = 0.0;
      ii = maxallele[locus];

      if (proplambda > 1e-20 && proplambda < lambdalim)
        {
          for (pop = 0; pop < currk; pop++)
            {
              kk = 0;
              for (allele = 0; allele < ii; allele++)
                {
                  jj = count[pop * nm + locus * missingallele + allele];
                  clikeli += mylgamma (eslambda[locus] + jj);
                  plikeli += mylgamma (proplambda + jj);
                  kk += jj;
                }

              temp = eslambda[locus] * (double) ii;
              temp2 = proplambda * (double) ii;
              clikeli += mylgamma (temp);
              plikeli += mylgamma (temp2);
              clikeli -= mylgamma (temp + (double) kk);
              plikeli -= mylgamma (temp2 + (double) kk);
              clikeli -= (mylgamma (eslambda[locus]) * (double) ii);
              plikeli -= (mylgamma (proplambda) * (double) ii);
            }

          mhprob = plikeli - clikeli;

          if (mhprob > log (rnd ()))
            eslambda[locus] = proplambda;
        }
    }
}




/* calculation of the mean partition (proposed by Huelsenbeck et al. 2007) ------------------------------------------------------------*/

   /*Cost matrix is generated using eq.16 in Konovalov et al. 2005 and solved by the hungarian algorithm.
      Original C++ source code of the hungarian algorithm is provided by 
      http://www.topcoder.com/tc?module=Static&d1=tutorials&d2=hungarianAlgorithm 
      Here it is converted to C */

#define Nv 50                   /* max number of vertices in one part */
#define INF 100000000           /* just infinity */
#define whichmax(a, b) ((a) > (b) ? (a) : (b))
#define whichmin(a, b) ((a) < (b) ? (a) : (b))

int cost[Nv][Nv];               /* cost matrix */
int n, max_match;               /* n workers and n jobs */
int lx[Nv], ly[Nv];             /* labels of X and Y parts */
int xy[Nv];                     /* xy[x] - vertex that is matched with x, */
int yx[Nv];                     /* yx[y] - vertex that is matched with y */
int Sh[Nv], Th[Nv];             /* sets Sh and Th in algorithm */
int slack[Nv];                  /* as in the algorithm description */
int slackx[Nv];                 /* slackx[y] such a vertex, that */
                         /* l(slackx[y]) + l(y) - w(slackx[y],y) = slack[y] */
int prev[Nv];                   /* array for memorizing alternating paths */

/*----------------------------------------------------------------------------------------------------*/
void
add_to_tree (int x, int prevx)
/* x - current vertex,prevx - vertex from X before x in the alternating path, */
/* so we add edges (prevx, xy[x]), (xy[x], x) */
{
  int y;

  Sh[x] = 1;                    /* add x to Sh */
  prev[x] = prevx;              /* we need this when augmenting */
  for (y = 0; y < n; y++)       /* update slacks, because we add new vertex to Sh */
    if (lx[x] + ly[y] - cost[x][y] < slack[y])
      {
        slack[y] = lx[x] + ly[y] - cost[x][y];
        slackx[y] = x;
      }
}

/*----------------------------------------------------------------------------------------------------*/
void
augment (void)                  /* main function of the algorithm */
{

  int x, y, root;               /* just counters and root vertex */
  int q[Nv], wr = 0, rd = 0;    /* q - queue for bfs, wr,rd - write and read */
  int cx, cy, ty;
  int i, j;
  int delta = INF;

  if (max_match == n)
    return;                     /* check wether matching is already perfect */
  /* pos in queue */
  memset (Sh, 0, sizeof (Sh));  /* init set Sh */
  memset (Th, 0, sizeof (Th));  /* init set Th */
  memset (prev, -1, sizeof (prev));     /* init set prev - for the alternating tree */


  for (x = 0; x < n; x++)       /* finding root of the tree */
    if (xy[x] == -1)
      {
        q[wr++] = root = x;
        prev[x] = -2;
        Sh[x] = 1;
        break;
      }

  for (y = 0; y < n; y++)       /* initializing slack array */
    {
      slack[y] = lx[root] + ly[y] - cost[root][y];
      slackx[y] = root;
    }

  /* second part of augment() function */
  while (1)                     /* main cycle */
    {
      while (rd < wr)           /* building tree with bfs cycle */
        {
          x = q[rd++];          /* current vertex from X part */
          for (y = 0; y < n; y++)       /* iterate through all edges in equality graph */
            if (cost[x][y] == lx[x] + ly[y] && !Th[y])
              {
                if (yx[y] == -1)
                  break;        /* an exposed vertex in Y found, so */
                /* augmenting path exists! */
                Th[y] = 1;      /* else just add y to Th, */
                q[wr++] = yx[y];        /* add vertex yx[y], which is matched */
                /* with y, to the queue */
                add_to_tree (yx[y], x); /* add edges (x,y) and (y,yx[y]) to the tree */
              }
          if (y < n)
            break;              /* augmenting path found! */
        }
      if (y < n)
        break;                  /* augmenting path found! */

      /* augmenting path not found, so improve labeling. "update_labels" */
      for (j = 0; j < n; j++)   /* calculate delta using slack */
        if (!Th[j])
          delta = whichmin (delta, slack[j]);
      for (i = 0; i < n; i++)   /* update X labels */
        if (Sh[i])
          lx[i] -= delta;
      for (j = 0; j < n; j++)   /* update Y labels */
        if (Th[j])
          ly[j] += delta;
      for (j = 0; j < n; j++)   /* update slack array */
        if (!Th[j])
          slack[j] -= delta;

      wr = rd = 0;
      for (y = 0; y < n; y++)
        /* in this cycle we add edges that were added to the equality graph as a
           result of improving the labeling, we add edge (slackx[y], y) to the tree if
           and only if !Th[y] &&  slack[y] == 0, also with this edge we add another one
           (y, yx[y]) or augment the matching, if y was exposed */
        if (!Th[y] && slack[y] == 0)
          {
            if (yx[y] == -1)    /* exposed vertex in Y found - augmenting path exists! */
              {
                x = slackx[y];
                break;
              }
            else
              {
                Th[y] = 1;      /* else just add y to Th, */
                if (!Sh[yx[y]])
                  {
                    q[wr++] = yx[y];    /* add vertex yx[y], which is matched with */
                    /* y, to the queue */
                    add_to_tree (yx[y], slackx[y]);     /* and add edges (x,y) and (y, */
                    /* yx[y]) to the tree */
                  }
              }
          }
      if (y < n)
        break;                  /* augmenting path found! */
    }

  if (y < n)                    /* we found augmenting path! */
    {
      max_match++;              /* increment matching */
      /* in this cycle we inverse edges along augmenting path */

      for (cx = x, cy = y; cx != -2; cx = prev[cx], cy = ty)
        {
          ty = xy[cx];
          yx[cy] = cx;
          xy[cx] = cy;
        }

      augment ();               /* recall function, go to step 1 of the algorithm */
    }
}


/*----------------------------------------------------------------------------------------------------*/
int
hungarian (void)
{
  int ret = 0;                  /* weight of the optimal matching */
  int x, y;

  max_match = 0;                /* number of vertices in current matching */
  memset (xy, -1, sizeof (xy));
  memset (yx, -1, sizeof (yx));

  memset (lx, 0, sizeof (lx));  /* step 0, "init_labels" */
  memset (ly, 0, sizeof (ly));

  for (x = 0; x < n; x++)
    for (y = 0; y < n; y++)
      lx[x] = whichmax (lx[x], cost[x][y]);

  augment ();                   /* steps 1-3 */
  for (x = 0; x < n; x++)       /* forming answer there */
    ret += cost[x][xy[x]];

  return (ret);
}


/*----Calculate the partition distance between A and B which has Ne elements--------------------------*/
int
Partdis (const int Ne, const int *A, const int *B)
{
  int ii, jj, kk, Pos, sum, Dis;
  int *Abs = NULL, *Bbsbar = NULL;

  /* n represents the number of clusters in A and B */
  for (ii = 0, n = 0; ii < Ne; ii++)
    {
      if (n < (int) A[ii])
        n = (int) A[ii];
      if (n < (int) B[ii])
        n = (int) B[ii];
    }
  n += 1;                       /* because the cluster number in A and B starts with 0 */

  if (n > Nv)
    {
      printf ("Number of clusters in sampled partitions is larger than %d\n",
              Nv);
      return (0);
    }

  Abs = calloc (Ne * n, sizeof (int));
  if (Abs == NULL)
    alert ();
  Bbsbar = calloc (Ne * n, sizeof (int));
  if (Bbsbar == NULL)
    alert ();

  for (ii = 0; ii < n; ii++)
    for (jj = 0; jj < Ne; jj++)
      {
        Pos = ii * Ne + jj;
        if (A[jj] == (int) ii)
          Abs[Pos] = 1;
        if (B[jj] != (int) ii)
          Bbsbar[Pos] = 1;
      }

  for (ii = 0; ii < n; ii++)
    {
      for (jj = 0; jj < n; jj++)
        {
          sum = 0;
          for (kk = 0; kk < Ne; kk++)
            sum += (Abs[jj * Ne + kk] * Bbsbar[ii * Ne + kk]);

          cost[ii][jj] = -sum;
        }
    }

  Dis = hungarian ();
  Dis *= -1;

  free (Abs);
  Abs = NULL;
  free (Bbsbar);
  Bbsbar = NULL;

  return (Dis);

}

/*----Main ( calculate the mean partition distance )--------------------------------------------------*/
void
Meanpartdis (int *sampledz, int ne, int ns, int *meanpartition)
{
  int ii, jj;
  int *Meanp, *propMeanp, kmeanp = 0, kk, min;
  int Distsum, propDistsum = 0, ele = 0, Move = 0, Notmove = 0;
  int poplabel, labelupdate;

  Distsum = ne * ns;

  Meanp = malloc (sizeof (int) * ne);
  if (Meanp == NULL)
    alert ();
  propMeanp = malloc (sizeof (int) * ne);
  if (propMeanp == NULL)
    alert ();
  for (ii = 0; ii < ne; ii++)
    {
      Meanp[ii] = sampledz[ii];
    }


  do
    {
      kmeanp = 0;
      for (ii = 0; ii < ne; ii++)
        {
          if (Meanp[ii] > kmeanp)
            kmeanp = Meanp[ii];
          propMeanp[ii] = Meanp[ii];
        }
      kmeanp += 1;

      for (kk = 0; kk <= kmeanp; kk++)
        {
          propMeanp[ele] = kk;

          propDistsum = 0;
          for (jj = 0; jj < ns; jj++)
            {
              propDistsum += Partdis (ne, propMeanp, sampledz + (jj * ne));

            }

          if (propDistsum < Distsum)
            {
              Distsum = propDistsum;
              for (jj = 0; jj < ne; jj++)
                {
                  Meanp[jj] = propMeanp[jj];
                }
              Move += 1;
              Notmove = 0;
            }
        }

      ele += 1;

      if (Move == 0)
        {
          Notmove += 1;
        }
      else
        {
          Move = 0;
        }
      if (ele == ne)
        ele = 0;

    }
  while (Notmove < (ne - 1));


  min = Meanp[0];
  for (ii = 1; ii < ne; ii++)
    if (Meanp[ii] < min)
      min = Meanp[ii];

  jj = 0;
  poplabel = 1;
  do
    {
      labelupdate = 0;
      for (ii = 0; ii < ne; ii++)
        if (Meanp[ii] == min)
          {
            meanpartition[ii] = poplabel;
            jj++;
            labelupdate++;
          }

      min++;

      if (labelupdate > 0)
        poplabel++;

    }
  while (jj < ne);



  free (propMeanp);
  free (Meanp);

}


/* Calculate the co-assignment probs ---------------------------------------------------------------------------------------------------*/
void
Coassignprobs (int *sampledz, int ni, int ns, double *coaprob)
{
  int ii, jj, ss, rr;

  for (ss = 0; ss < ns; ss++)
    {
      rr = 0;
      for (ii = 0; ii < (ni - 1); ii++)
        {
          for (jj = ii + 1; jj < ni; jj++)
            {
              if (sampledz[ss * ni + ii] == sampledz[ss * ni + jj])
                coaprob[rr] += 1.0;
              rr++;
            }
        }
    }

  for (ii = 0; ii < rr; ii++)
    coaprob[ii] /= (double) ns;

}
