/* simdiv  2009  Jody Hey, Sang Chul Choi and Yong Wang*/


#undef GLOBVARS
#include "simdiv.h"



/* LOCAL PROTOTYPES */
int multimut (int oribase);
double tajd (double n, double s, double pi);


/* FUNCTIONS */

/*******************************/
/* CALCULATIONS AND OUTPUT FUNCTIONS */
/******************************/

int
multimut (int oribase)
{
  int mutbase;
  double _uni;
  _uni = ran1 (idum);
  if (oribase == 3)
    _uni *= mutmatrix[3][2];
  else
    _uni *= mutmatrix[oribase][3];
  for (mutbase = 0; mutbase < 4; mutbase++)
    {
      //printf ("(%f,%f)\t",_uni,mutmatrix[oribase][mutbase]);
      if (_uni <= mutmatrix[oribase][mutbase])
        {
          if ((absv (mutbase - oribase)) == 2)
            numutstrans++;
          else
            numutstranv++;
          return (mutbase);
        }
    }
  printf ("error in multimut\n");
  myassert (0);
  exit (-1);
}

double
tajd (double n, double s, double pi)
{
  double a1, a2, b1, b2, e1, e2, c1, c2, t, td, TEMP;
  unsigned short i, j;
  j = (int) n;
  if (j >= 4)
    {
      a1 = 1.0;
      for (i = 2; i < j; i++)
        a1 += 1.0 / i;
      a2 = 1.0;
      for (i = 2; i < j; i++)
        {
          TEMP = 1.0 * i;
          a2 += 1.0 / (TEMP * TEMP);
        }
      b1 = (n + 1.0) / (3.0 * (n - 1.0));
      b2 = 2.0 * (n * n + n + 3.0) / (9.0 * n * (n - 1.0));
      c1 = b1 - 1 / a1;
      c2 = b2 - (n + 2.0) / (a1 * n) + a2 / (a1 * a1);
      e1 = c1 / a1;
      e2 = c2 / (a1 * a1 + a2);
      t = e1 * s + e2 * s * (s - 1);
      myassert (t > 0 && a1 > 0);
      if (s > 0.0 && t > 0.0)
        td = (pi - s / a1) / sqrt (t);
      else
        td = -100.0;
      if (td > -99.0)
        {
          return (td);
        }
      else
        return (-100);
    }
  else
    return (-100);
}                               /* tajd */


/********************/
/* GLOBAL FUNCTIONS */
/********************/

void
printsimdata (int locus)
{
  struct mutation;
  char linestart[11];
  int i, ii, si, j, k, ki, base, basei, kd, jj, dbasei;
  int templength;
  int tempc = 0;
  int basearray[DEFAULTLOCILENGTH];
  char abase, dbase;

  if (umodel != SW && umodel != INFINITEALLELES)
    printf (" # mutations %d", numuts);
  if (umodel == SW || umodel == JOINT)
    printf (" # SW mutations %d", numutssw);
  if (loadlocusvaluefile)
    {
      FP "%s  ", locusnames[locus]);
    }
  else
    FP "Simulation%d  ", locus);
  /* SANGCHUL: ASSIGNMENT */
  if (ropt[ASSIGNMENT] == 1)
    {
      if (ropt[ASSIGNMENTDIFFERNTTREE] == 1)
        {
          for (i = 0; i < npopulation; i++)
            FP "0   ");
        }
      else
        {
          for (i = 0; i < numpops; i++)
            FP "0   ");
        }
    }
  else
    {
      for (i = 0; i < numpops; i++)
        FP "%d   ", num[i]);
    }
  if (umodel == IS || umodel == JOINT)
    {
      if (recrate > 0)
        {
          if (printseqlength == 0)
            templength = DEFAULTLOCILENGTH;
          else
            templength = printseqlength;
          if (umodel == IS)
            FP "  %d  I", templength);
          else
            FP "  %d  J1", templength);
        }
      else
        {
          if (printseqlength == 0 && (umodel == IS || umodel == JOINT))
            templength = numuts;
          else
            templength = seqlength;
          if (umodel == IS)
            {
              if (!heylaboptions[GAMMADISTRIBUTEDMUTATIONERATES])
                {
                  FP "%d  I  %.3f", templength, hs);
                }
              else
                FP "%d  I  %.3f  Ratio %12.11f Length %d  #muts %d",
                  templength, hs, urela[locus], oglen[locus], ogmut[locus]);
            }
          else
            FP "  %d  J1  %.3f", templength, hs);
        }
    }
  else
    templength = seqlength;
  if (umodel == SW)
    FP "  %d  S  %.3f", templength, hs);
  if (umodel == HKY)
    {
      FP "  %d  H  %.3f", templength, hs);
    }
  if (umodel == INFINITEALLELES)
    FP "  1  A  %.3f.3", hs);
  /* SANGCHUL: ASSIGNMENT */
  if (ropt[ASSIGNMENT] == 1)
    {
      FP " A%d\n", numsum);
    }
  else
    {
      FP "\n");
    }    
  if (umodel != SW)             // set up a random sequence according to set base frequencies
    {
      for (j = 0; j < templength; j++)
        {
          basearray[j] = (j >= templength * bf[0])
            + (j >= templength * (bf[0] + bf[1]))
            + (j >= templength * (bf[0] + bf[1] + bf[2]));
        }
      for (i = 0; i < 10; i++)  // jumble 10 times
        for (j = 0; j < templength; j++)        // a full set of swaps
          {
            basei = randint (templength - j) - 1;
            k = basearray[basei];
            basearray[basei] = basearray[j];
            basearray[j] = k;
          }
    }
  for (i = 0 /*1 */ , ii = 0, ki = 1; i < /*=*/ numsum; i++, ki++)
    {
      while (ki > num[ii])
        {
          ii++;
          ki = 1;
        }
      linestart[0] = '\0';
      if (ropt[DIPLOIDGENENAMES])
        kd = (ki + 1) / 2;
      else
        kd = ki;
      sprintf (linestart, "pop%d_%d\0", ii, kd);
      while (strlen (linestart) < 10)
        strcat (linestart, " ");
      FP "%s", linestart);

      if (umodel == SW || umodel == INFINITEALLELES)
        {
          for (si = 0; si < printseqlength; si++)
            FP "%d ", tree[i /*-1*/ ].A[si]);
        }                       /* SW model */
      else
        {
          if (umodel == JOINT)
            {
              FP "%d ", tree[i /*-1*/ ].A[0]);
            }
          if (umodel == JOINT || umodel == IS)
            {
              for (j = 0, jj = 0; j < templength; j++)
                {
                  if (templength == numuts)     //in this case need to only use positions with mutations,  use jj and j 
                    {
                      while (!usedmut[jj])
                        {
                          jj++;
                        }
                      myassert (usedmut[jj] && jj < CHROMELENGTH);
                    }
                  else
                    {
                      jj = j;
                    }
                  dbasei = j % 3;
                  base = basearray[j];
                  switch (base)
                    {
                    case 0:
                      abase = 'A';
                      switch (dbasei)
                        {
                        case 0:
                          dbase = 'T';
                          break;
                        case 1:
                          dbase = 'C';
                          break;
                        case 2:
                          dbase = 'G';
                          break;
                        default:
                          exit (-1);

                        };
                      break;
                    case 1:
                      abase = 'C';
                      switch (dbasei)
                        {
                        case 0:
                          dbase = 'G';
                          break;
                        case 1:
                          dbase = 'A';
                          break;
                        case 2:
                          dbase = 'T';
                          break;
                        default:
                          exit (-1);

                        };
                      break;
                    case 2:
                      abase = 'G';
                      switch (dbasei)
                        {
                        case 0:
                          dbase = 'C';
                          break;
                        case 1:
                          dbase = 'T';
                          break;
                        case 2:
                          dbase = 'A';
                          break;
                        default:
                          exit (-1);

                        };
                      break;
                    case 3:
                      abase = 'T';
                      switch (dbasei)
                        {
                        case 0:
                          dbase = 'A';
                          break;
                        case 1:
                          dbase = 'T';
                          break;
                        case 2:
                          dbase = 'C';
                          break;
                        default:
                          exit (-1);
                        };
                      break;
                    default:
                      exit (-1);
                    }
                  if (usedmut[jj])
                    {
                      //printf(" %d ",jj);
                      nowmut = firstmut;
                      while (nowmut)
                        {
                          if (nowmut->spot == jj)
                            {
                              if (checkintersect (nowmut->d, sampstandard[i]))
                                {
                                  FP "%c", dbase);
                                }
                              else
                                {
                                  FP "%c", abase);
                                }
                              nowmut = NULL;
                            }
                          else
                            {
                              nowmut = nowmut->next;
                            }
                        }
                      jj++;
                    }
                  else
                    FP "%c", abase);
                }
            }                   /*IS model */
          if (umodel == HKY)
            {                   // HKY model
              for (j = 0; j < templength; j++)
                {
                  base = basearray[j];
                  /*base = (j >= templength  * bf[0]) 
                     + (j >= templength  * (bf[0] + bf[1])) 
                     + (j >= templength * (bf[0] + bf[1] + bf[2])) ; */
                  if (usedmut[j])
                    {
                      nowmut = firstmut;
                      while (nowmut)
                        {
                          if ((nowmut->spot == j)
                              && checkintersect (nowmut->d, sampstandard[i]))
                            {
                              tempc++;
                              if (nowmut->to != -1)
                                {
                                  base = nowmut->to;
                                }
                              else
                                {
                                  nowmut->from = base;
                                  base = multimut (base);
                                  nowmut->to = base;
                                }
                            }
                          nowmut = nowmut->next;
                        }
                      switch (base)
                        {
                        case 0:
                          FP "A");
                          break;
                        case 1:
                          FP "C");
                          break;
                        case 2:
                          FP "G");
                          break;
                        case 3:
                          FP "T");
                          break;
                        default:
                          FP "%d", base);
                        }
                    }
                  else
                    {
                      switch (base)
                        {
                        case 0:
                          FP "A");
                          break;
                        case 1:
                          FP "C");
                          break;
                        case 2:
                          FP "G");
                          break;
                        case 3:
                          FP "T");
                          break;
                        default:
                          FP "%d", base);
                        }

                    }
                }

            }                   // HKY
        }
      FP "\n");
    }
  if (umodel == IS || umodel == JOINT || umodel == HKY)
    {
      nowmut = firstmut;
      do
        {
          othermut = nowmut->next;
          free (nowmut);
          nowmut = othermut;
        }
      while (nowmut != NULL);
    }
  //printf("   Finished Locus %d \n",locus); 
  printf ("\n");
}                               /* printsimdata */


//void calcstats(int locus)
void calcstats (void)
{
  struct mutation;
  int i, j, lo[MAXPOPS], hi[MAXPOPS], k, kk;    //c,m;
  int poly[MAXPOPS];
  double a;
  double pi[MAXPOPS], tempthetasw[MAXPOPS];
  double divest[MAXPOPS][MAXPOPS] =
  {
  0, 0};
  int dcount[2];
  double d2sum;
  int ii;
  struct BSET tempbset;

  meannumuts += numuts;
  meannumutssw += numutssw;
  meannumutsIA += numutsIA;
  meannumrec += numrec;
  lo[0] = 0;
  hi[0] = num[0] - 1;
  for (k = 1; k < numpops; k++)
    {
      lo[k] = hi[k - 1] + 1;
      hi[k] = hi[k - 1] + num[k];
    }
  if (umodel == SW || umodel == JOINT)
    {
      for (k = 0; k < numpops; k++)
        if (num[k] > 1)
          {
            nlocistrpoly[k]++;
            d2sum = 0;
            ii = 0;
            for (i = lo[k]; i <= hi[k] - 1; i++)
              for (j = i + 1; j <= hi[k]; j++)
                {
                  d2sum +=
                    (tree[i].A[0] - tree[j].A[0]) * (tree[i].A[0] -
                                                     tree[j].A[0]);
                  ii++;
                }
            d2sum /= (double) (num[k] * (num[k] - 1) / 2);
            tempthetasw[k] = d2sum;
            thetaestsw[k] += d2sum;
            d2sum = 0;
          }
      for (k = 0; k < numpops; k++)
        {
          for (kk = 0, d2sum = 0; kk < numpops; kk++)
            if (kk < k && num[k] > 0 && num[kk] > 0)
              {
                nlocistrdiv[k][kk]++;
                nlocistrdiv[kk][k]++;
                divest[k][kk] = 0;
                for (i = lo[k]; i <= hi[k]; i++)
                  for (j = lo[kk]; j <= hi[kk]; j++)
                    {
                      d2sum +=
                        (tree[i].A[0] - tree[j].A[0]) * (tree[i].A[0] -
                                                         tree[j].A[0]);
                    }
                d2sum /= (double) (num[k] * num[kk]);
                divest[k][kk] = d2sum;
                d2sum = 0;
              }
        }
      for (k = 0; k < numpops; k++)
        {
          for (kk = 0; kk < numpops; kk++)
            if (num[k] > 0 && num[kk] > 0)
              {
                if (kk > k)
                  divest[k][kk] =
                    divest[kk][k] - (tempthetasw[k] + tempthetasw[kk]) / 2;
                meandivsw[k][kk] += divest[k][kk];
              }
        }
    }
  if (umodel != SW && umodel != INFINITEALLELES)
    {
      for (k = 0; k < numpops; k++)
        if (num[k] > 1)
          {
            nlociseqpoly[k]++;
            for (a = 0, j = 1, i = lo[k]; i <= hi[k]; i++, j++)
              a += 1.0 / (double) j;
            for (i = lo[k]; i <= hi[k]; i++)
              {
                nowmut = firstmut;
                poly[k] = 0;
                do
                  {
                    intersectb (nowmut->d, fullsamp[k], tempbset);
                    poly[k] += (checkintersect (nowmut->d, fullsamp[k])
                                && bsetless (tempbset, fullsamp[k]));
                    nowmut = nowmut->next;
                  }
                while (nowmut != NULL);
              }
            thetaest[k] += poly[k] / a;
          }
      for (k = 0; k < numpops; k++)
        {
          pi[k] = 0;
          for (kk = 0; kk < numpops; kk++)
            {
              if (num[k] > 0 && num[kk] > 0)
                nlociseqdiv[k][kk]++;
              divest[k][kk] = 0;
            }
        }
      nowmut = firstmut;
      do
        {
          for (k = 0; k < numpops; k++)
            if (num[k] > 1)
              {
                intersectb (nowmut->d, fullsamp[k], tempbset);
                dcount[0] = cardinality (tempbset);
                pi[k] += dcount[0] * (num[k] - dcount[0]);
                for (kk = 0; kk < numpops; kk++)
                  if (kk < k && num[kk] > 0)
                    {
                      intersectb (nowmut->d, fullsamp[kk], tempbset);
                      dcount[1] = cardinality (tempbset);
                      divest[k][kk] +=
                        dcount[0] * (num[kk] - dcount[1]) + (num[k] -
                                                             dcount[0]) *
                        dcount[1];
                    }
              }
          nowmut = nowmut->next;
        }
      while (nowmut != NULL);
      for (k = 0; k < numpops; k++)
        if (num[k] > 1)
          {
            pi[k] /= (num[k] * (num[k] - 1) / 2.0);
            meanpi[k] += pi[k];
            if (num[k] >= 4 && poly[k] > 0)
              {
                meantajd[k] += tajd (num[k], poly[k], pi[k]);
                nlocitajd[k]++;
              }
          }
      for (k = 0; k < numpops; k++)
        if (num[k] > 0)
          for (kk = 0; kk < numpops; kk++)
            if (kk < k && num[kk] > 0)
              {
                divest[k][kk] /= num[k] * num[kk];
              }
      for (k = 0; k < numpops; k++)
        {
          for (kk = 0; kk < numpops; kk++)
            if (num[k] > 0 && num[kk] > 0)
              {
                if (kk > k)
                  divest[k][kk] = divest[kk][k] - (pi[k] + pi[kk]) / 2;
                meandiv[k][kk] += divest[k][kk];
              }
        }
    }
}                               /* calcstats */



/*
#define BSET_BITS 352
#define singlesetb(bset1,i) // returns a set with only the element i
#define unionb(bset1,bset2,bset3)  // puts the union of bset1 and bset2 into bset3 
#define intersectb(bset1,bset2,bset3)      // puts the intersection of bset1 and bset2 into bset3 
#define  checkintersect(bset1,bset2) // return _true if any overlap,  _false otherwise 
#define bsetless(bset1,bset2) // return _true if bset1 is less than bset2 in at least one unit 
#define setdiffb(bset1,bset2,bset3) //returns a bset that is bitwise exclusive of of bset1 and bset2
#define getlowb(bset,bset1) // gets the lowest value in bset and creates bset1 with it 
#define elementb(bset,i)      //returns true if i in bset
#define forallb(i,bset) 
#define emptyb(bset) // true if bset is empty 
#define notemptyb(bset) // true if bset is not empty 
*/

// since we know which state of polymorphic sites is derived, don't bother with folded distributions
// s4gp_r   pass 4G test  even though there were recombination events
// s4gp_nr  pass 4Gtest  and there happened to not be recombination events
// s4gf     failed 4G test

void fourgametetest (int locus)
{
  static int num4gp_r, num4gp_nr, num4gf;
  static int *numldp_r, *numldp_nr, *numldf;
  static float *ldp_r, *ldp_nr, *ldf;
  static char *gf_str, *gp_nr_str, *gp_r_str;
  static int ***s4gp_r, ***s4gp_nr, ***s4gf;
  int ***s;
  char tempstr[7];
  float *ld;
  int n4g, *nld;
  int i, ii, j, jj, k, si;
  int c00, c01, c10, c11;       //indicators of four types
  boolean pass4g;
  static int *numrp_r, *numrp_nr, *numrf;
  static float *rp_r, *rp_nr, *rf;
  float *r, tempr;
  int *nr;
  struct BSET tempbset1, tempbset2, tempbset3;

  //initiatlize
  k = numpops * (numpops - 1) / 2;
  if (locus == 0)
    {
      num4gp_r = num4gp_nr = num4gf = 0;
      numldp_r = numldp_nr = numldf = 0;
      gf_str = malloc (nloci * 4 * sizeof (char));
      *gf_str = '\0';
      gp_nr_str = malloc (nloci * 4 * sizeof (char));
      *gp_nr_str = '\0';
      gp_r_str = malloc (nloci * 4 * sizeof (char));
      *gp_r_str = '\0';
      if (!loadlocusvaluefile)  // if not then all sample sizes are equal and can do mean ld calculations 
        {
          ldp_r = calloc (numpops, sizeof (float));
          ldp_nr = calloc (numpops, sizeof (float));
          ldf = calloc (numpops, sizeof (float));
          numldp_r = calloc (numpops, sizeof (int));
          numldp_nr = calloc (numpops, sizeof (int));
          numldf = calloc (numpops, sizeof (int));
          rp_r = calloc (numpops, sizeof (float));
          rp_nr = calloc (numpops, sizeof (float));
          rf = calloc (numpops, sizeof (float));
          numrp_r = calloc (numpops, sizeof (int));
          numrp_nr = calloc (numpops, sizeof (int));
          numrf = calloc (numpops, sizeof (int));
          s4gp_r = calloc (k, sizeof (int **));
          s4gp_nr = calloc (k, sizeof (int **));
          s4gf = calloc (k, sizeof (int **));
          for (i = 0, k = 0; i < numpops - 1; i++)
            for (j = i + 1; j < numpops; j++, k++)
              {
                s4gp_r[k] = calloc (1 + num[i], sizeof (int *));
                s4gp_nr[k] = calloc (1 + num[i], sizeof (int *));
                s4gf[k] = calloc (1 + num[i], sizeof (int *));
                for (ii = 0; ii <= num[i]; ii++)
                  {
                    s4gp_r[k][ii] = calloc (1 + num[j], sizeof (int));
                    s4gp_nr[k][ii] = calloc (1 + num[j], sizeof (int));
                    s4gf[k][ii] = calloc (1 + num[j], sizeof (int));
                  }
              }
        }
    }
  if (locus < nloci && numuts > 0)
    {
      pass4g = _true;
      nowmut = firstmut;
      othermut = nowmut->next;
      if (numrec > 0)
        {
          while (othermut != NULL && pass4g == _true)
            {
              setdiffb (fullsampa, othermut->d, tempbset1);
              setdiffb (fullsampa, nowmut->d, tempbset2);
              intersectb (tempbset1, tempbset2, tempbset3);
              c00 = notemptyb (tempbset3);
//                              c00 = !isempty( intersect(setdiff(fullsampa,nowmut->d),setdiff(fullsampa,othermut->d)) ); 
              setdiffb (fullsampa, nowmut->d, tempbset1);
              intersectb (tempbset1, othermut->d, tempbset2);
              c01 = notemptyb (tempbset2);
//                              c01 = !isempty( intersect(setdiff(fullsampa,nowmut->d),othermut->d)); 
              setdiffb (fullsampa, othermut->d, tempbset1);
              intersectb (tempbset1, nowmut->d, tempbset2);
              c10 = notemptyb (tempbset2);
//                              c10 = !isempty( intersect(nowmut->d,setdiff(fullsampa,othermut->d))); 
              intersectb (nowmut->d, othermut->d, tempbset1);
              c11 = notemptyb (tempbset1);
//                              c11 = !isempty( intersect(nowmut->d,othermut->d)); 
              pass4g = pass4g && (c00 + c01 + c10 + c11 < 4);
              othermut = othermut->next;
              if (othermut == NULL)
                {
                  nowmut = nowmut->next;
                  othermut = nowmut->next;
                }
            }
        }
      sprintf (tempstr, "%d \0", locus);
      if (pass4g == _false)
        {
          if (!loadlocusvaluefile)
            {
              s = s4gf;
              ld = ldf;
              nld = numldf;
              r = rf;
              nr = numrf;
            }
          num4gf++;
          strcat (gf_str, tempstr);
        }
      else
        {
          if (numrec == 0)
            {
              if (!loadlocusvaluefile)
                {
                  s = s4gp_nr;
                  ld = ldp_nr;
                  nld = numldp_nr;
                  r = rp_nr;
                  nr = numrp_nr;
                }
              num4gp_nr++;
              strcat (gp_nr_str, tempstr);
            }
          else
            {
              if (!loadlocusvaluefile)
                {
                  s = s4gp_r;
                  ld = ldp_r;
                  nld = numldp_r;
                  r = rp_r;
                  nr = numrp_r;
                }
              num4gp_r++;
              strcat (gp_r_str, tempstr);
            }
        }
      for (i = 0, k = 0; i < numpops - 1; i++)
        for (j = i + 1; j < numpops; j++, k++)
          {
            nowmut = firstmut;
            while (nowmut != NULL)
              {
                intersectb (nowmut->d, fullsamp[i], tempbset1);
                ii = cardinality (tempbset1);
//                                      ii = cardinality(intersect(nowmut->d,fullsamp[i]));
                intersectb (nowmut->d, fullsamp[j], tempbset1);
                jj = cardinality (tempbset1);
//                                      jj = cardinality(intersect(nowmut->d,fullsamp[j]));
                if (!loadlocusvaluefile)
                  s[k][ii][jj]++;
                nowmut = nowmut->next;
              }
          }
      // calculate abs val of LD for sites at intermediate frequencies 
      // old.  not changed since sample size was increased and BSET was defined
      /*for (i = 0; i< numpops; i++)
         {
         templd = 0;
         k = 0;
         nowmut = firstmut; 
         othermut = nowmut->next; 
         while (othermut != NULL)
         {
         c00   = cardinality (intersect (intersect(fullsamp[i],setdiff(fullsampa,nowmut->d))   , intersect(fullsamp[i],setdiff(fullsampa,othermut->d)) )   );
         c10   = cardinality (intersect (intersect(fullsamp[i],nowmut->d) , intersect(fullsamp[i],setdiff(fullsampa,othermut->d))  )   );
         c01   = cardinality (intersect (intersect(fullsamp[i],setdiff(fullsampa,nowmut->d))    , intersect(fullsamp[i],othermut->d) )   );
         c11   = cardinality (intersect (intersect(fullsamp[i],nowmut->d) , intersect(fullsamp[i],othermut->d) )   );
         if (c00 + c10 + c01 + c11 != num[i])
         exit (-3);
         if ((c00 + c01) > 0 &&  (c00 + c01) < num[i] && (c01 + c11) > 0 &&  (c01 + c11) < num[i])
         if ((c00 + c01) > 1 &&  (c00 + c01) < num[i]-1 && (c01 + c11) > 1 &&  (c01 + c11) < num[i] -1)
         {
         k++;
         templd += ((c00 * c11) - (c01 * c10))/ (float) (num[i] * num[i]);
         }
         othermut = othermut->next; 
         if (othermut == NULL)
         {
         nowmut = nowmut->next;
         othermut = nowmut->next; 
         }
         }
         if (k > 0)
         {
         ld[i] += fabs(templd) / (float) k;
         nld[i] = nld[i] + 1;
         }
         } */
      // calculate r - coorelation value of LD  
      for (i = 0; i < numpops; i++)
        {
          tempr = 0;
          k = 0;
          nowmut = firstmut;
          othermut = nowmut->next;
          while (othermut != NULL)
            {
              setdiffb (fullsampa, othermut->d, tempbset1);
              intersectb (fullsamp[i], tempbset1, tempbset1);
              setdiffb (fullsampa, nowmut->d, tempbset2);
              intersectb (fullsamp[i], tempbset2, tempbset2);
              intersectb (tempbset1, tempbset2, tempbset3);
              c00 = cardinality (tempbset3);
//                              c00   = cardinality (intersect (intersect(fullsamp[i],setdiff(fullsampa,nowmut->d))   , intersect(fullsamp[i],setdiff(fullsampa,othermut->d)) )   );
              setdiffb (fullsampa, othermut->d, tempbset1);
              intersectb (fullsamp[i], tempbset1, tempbset1);
              intersectb (fullsamp[i], nowmut->d, tempbset2);
              intersectb (tempbset1, tempbset2, tempbset3);
              c10 = cardinality (tempbset3);
//                              c10   = cardinality (intersect (intersect(fullsamp[i],nowmut->d) , intersect(fullsamp[i],setdiff(fullsampa,othermut->d))  )   );
              intersectb (fullsamp[i], othermut->d, tempbset1);
              setdiffb (fullsampa, nowmut->d, tempbset2);
              intersectb (fullsamp[i], tempbset2, tempbset2);
              intersectb (tempbset1, tempbset2, tempbset3);
              c01 = cardinality (tempbset3);
//                              c01   = cardinality (intersect (intersect(fullsamp[i],setdiff(fullsampa,nowmut->d))    , intersect(fullsamp[i],othermut->d) )   );
              intersectb (fullsamp[i], othermut->d, tempbset1);
              intersectb (fullsamp[i], nowmut->d, tempbset2);
              intersectb (tempbset1, tempbset2, tempbset3);
              c11 = cardinality (tempbset3);
//                              c11   = cardinality (intersect (intersect(fullsamp[i],nowmut->d) , intersect(fullsamp[i],othermut->d) )   );
              if (c00 + c10 + c01 + c11 != num[i])
                exit (-3);
              if ((c00 + c01) > 0 && (c00 + c01) < num[i] && (c01 + c11) > 0
                  && (c01 + c11) < num[i])
                {
                  k++;
                  tempr +=
                    (float)
                    pow ((((c00 * c11) -
                           (c01 * c10)) / (float) (num[i] * num[i])),
                         2) / ((c00 + c01) * (c10 + c11) * (c00 +
                                                            c10) * (c01 +
                                                                    c11) /
                               (float) pow (num[i], 4));
                }
              othermut = othermut->next;
              if (othermut == NULL)
                {
                  nowmut = nowmut->next;
                  othermut = nowmut->next;
                }
            }
          if (k > 0 && !loadlocusvaluefile)
            {
              r[i] += tempr / (float) k;
              nr[i] = nr[i] + 1;
            }
        }


    }
  if (locus == nloci)
    {
      FP "\n");
      for (si = 0; si < 3; si++)
        {
          switch (si)
            {
            case 0:
              n4g = num4gf;
              if (!loadlocusvaluefile)
                {
                  nld = numldf;
                  s = s4gf;
                  ld = ldf;
                  nr = numrf;
                  r = rf;
                }
              FP "FAILURE OF FOUR GAMETE TEST - NUMBER OF LOCI: %d \n", n4g);
              FP "--------------------------------------------------\n");
              FP "  LOCI : %s\n", gf_str);
              break;
            case 1:
              n4g = num4gp_r;
              if (!loadlocusvaluefile)
                {
                  nld = numldp_r;
                  s = s4gp_r;
                  ld = ldp_r;
                  nr = numrp_r;
                  r = rp_r;
                }
              FP " PASS FOUR GAMETE TEST FOR LOCI IN WHICH RECOMBINATION OCCURED - NUMBER OF LOCI: %d \n", num4gp_r);
              FP "-------------------------------------------------------------------------------------\n");
              FP "  LOCI : %s\n", gp_r_str);
              break;
            case 2:
              n4g = num4gp_nr;
              if (!loadlocusvaluefile)
                {
                  nld = numldp_nr;
                  s = s4gp_nr;
                  ld = ldp_nr;
                  nr = numrp_nr;
                  r = rp_nr;
                }
              FP " NO RECOMBINATION OCCURRED (PASS FOUR GAMETE TEST) - NUMBER OF LOCI: %d \n", n4g);
              FP "-------------------------------------------------------------------------\n");
              FP "  LOCI : %s\n", gp_nr_str);
              break;
            }
          FP "\n");
          if (n4g > 0 && !loadlocusvaluefile)
            {
              /*              FP"Mean Linkage Disequilibrium \n");
                 FP" Pop   LD     # Loci measured\n");
                 FP" ---  ------- ------\n");
                 for (i = 0; i< numpops; i++)
                 FP"  %2d % .6f  %d\n",i,ld[i]/ nld[i], nld[i]); */
              FP "\n Mean coorelation - r \n");
              FP " Pop  r      # Loci measured\n");
              FP " ---  ------- ------\n");
              for (i = 0; i < numpops; i++)
                FP "  %2d % .6f  %d\n", i, r[i] / nr[i], nr[i]);
              if (numpops > 1)
                FP " SITE FREQUENCY TABLES FOR PAIRS OF POPULATIONS \n");
              for (i = 0, k = 0; i < numpops - 1; i++)
                for (j = i + 1; j < numpops; j++, k++)
                  {
                    FP "Population %d   sample size: %d  Population %d  sample size :  %d \n", i, num[i], j, num[j]);
                    FP "           Pop %d\n", i);
                    FP "       ");
                    for (ii = 0; ii <= num[i]; ii++)
                      FP "  %2d  ", ii);
                    FP "\n");
                    FP "Pop %2d|", j);
                    for (ii = 0; ii <= num[i]; ii++)
                      FP "------", ii);
                    FP "\n");
                    for (jj = 0; jj <= num[j]; jj++)
                      {
                        FP "   %2d |", jj);
                        for (ii = 0; ii <= num[i]; ii++)
                          FP " %5.1f", s[k][ii][jj] / (float) n4g);
                        FP "\n");
                      }
                    FP "\n");
                  }
            }
        }
      FP "\nMean number of recombination events among loci where recombination occurred:  %.3lf \n", meannumrec / (float) (num4gp_r + num4gf));
      FP "Mean number of recombination events over all Infinite Sites model loci:  %.3lf \n", meannumrec / (float) modelcount[IS]);
      if (!loadlocusvaluefile)  // if not then all sample sizes are equal and can do mean ld calculations 
        {
          free (ldp_r);
          free (ldp_nr);
          free (ldf);
          free (numldp_r);
          free (numldp_nr);
          free (numldf);
          free (rp_r);
          free (rp_nr);
          free (rf);
          free (numrp_r);
          free (numrp_nr);
          free (numrf);
          for (i = 0, k = 0; i < numpops - 1; i++)
            for (j = i + 1; j < numpops; j++, k++)
              {
                for (ii = 0; ii <= num[i]; ii++)
                  {
                    free (s4gp_r[k][ii]);
                    free (s4gp_nr[k][ii]);
                    free (s4gf[k][ii]);
                  }
                free (s4gp_r[k]);
                free (s4gp_nr[k]);
                free (s4gf[k]);
              }
          free (s4gp_r);
          free (s4gp_nr);
          free (s4gf);
        }
      free (gf_str);
      free (gp_nr_str);
      free (gp_r_str);
    }
}                               //fourgamete_sfs
