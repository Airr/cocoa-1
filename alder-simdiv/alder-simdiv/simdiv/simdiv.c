/* simdiv  2009  Jody Hey, Sang Chul Choi and Yong Wang*/


#define GLOBVARS
#include "simdiv.h"

struct BSET emptysetb = { 0 };

void
dosim (void)
{
  int locus, i, j, k, kd;
  double tempsum;
  char tempstr[11];

  for (locus = 0; locus < nloci; locus++)
    {
      if (loadlocusvaluefile)
        {
          umodel = locusmutationmodel[locus];
          hs = locusinheritance[locus];
          seqlength = printseqlength = locuslength[locus];
          if (umodel == HKY)
            {
              ttratio = locusttratio[locus];
              for (i = 0; i < 4; i++)
                {
                  tempsum = 0;
                  for (j = 0; j < 4; j++)
                    {
                      if (absv (j - i) == 2)
                        tempsum += ttratio * bf[j];
                      else if (i != j)
                        tempsum += bf[j];
                    }
                  for (j = 0; j < 4; j++)
                    {
                      if (absv (j - i) == 2)
                        mutmatrix[i][j] = ttratio * bf[j] / tempsum;    // ratio of each type of mutation for four kinds of residues
                      else
                        {
                          if (i != j)
                            mutmatrix[i][j] = bf[j] / tempsum;
                          else
                            mutmatrix[i][j] = 0;
                        }
                    }
                }
              mutmatrix[0][2] += mutmatrix[0][1];       // accumulative ratio
              mutmatrix[0][3] += mutmatrix[0][2];
              mutmatrix[1][2] += mutmatrix[1][0];
              mutmatrix[1][3] += mutmatrix[1][2];
              mutmatrix[2][1] += mutmatrix[2][0];
              mutmatrix[2][3] += mutmatrix[2][1];
              mutmatrix[3][1] += mutmatrix[3][0];
              mutmatrix[3][2] += mutmatrix[3][1];
            }
        }
      if (recrate > 0.0 && umodel != IS)
        {
          printf
            (" recombination rate must be zero for mutation models other than Infinite Sites\n");
          exit (-1);
        }
      loopinit (locus);
      for (i = 0; i < numt - 1; i++)
        if (nowsum > 1)
          {
            clsc (i, locus);
            pool (i);
          }
      if (nowsum > 1)
        clsc (numt - 1, locus);

      /*empty the lists */
      for (i = 0; i < 2 * MAXPOPS; i++)
        {
          nowgnode = linpops[i].l;
          while (nowgnode)
            {
              removegnode (i);
              nowgnode = linpops[i].l;
            }
        }
      //      myassert(count_free + nowsum +numuts + count_others== count_malloc); 

      if (umodel == SW)
        for (i = 0; i < seqlength; i++)
          SWmutations (rootgnode, locus, i);
      if (umodel == JOINT)
        SWmutations (rootgnode, locus, 0);
      if (umodel == INFINITEALLELES)
        IAmutations (rootgnode, locus);
      if (numuts > 0 || numutssw > 0 || numutsIA > 0)
        {
          //calcstats(locus);
          if (ropt[PRINTDATASUMMARY])
            {
              calcstats ();
              if (heylaboptions[FOURGAMETETEST])
                fourgametetest (locus);
            }
        }
      if (!ropt[NODATAONLYSUMMARY])
        {
          if (numuts > 0 || numutssw > 0 || numutsIA > 0)
            printsimdata (locus);
          else
            {
              if (ropt[KEEPZEROVARIATION])
                {
                  if (loadlocusvaluefile)
                    FP "%s  ", locusnames[locus]);
                  else
                    FP "Simulation%d  ", locus);
                  for (i = 0; i < numpops; i++)
                    {
                      //FP"%d   ",num[i]);
                      FP "%d   ", num[i]);
                    }
                  if (!heylaboptions[GAMMADISTRIBUTEDMUTATIONERATES])
                    {
                      FP "%d  I  %f\n", seqlength, hs);
                    }
                  else
                    FP "%d  I  %f  Ratio %12.11f Length %d  #muts %d\n",
                      seqlength, hs, urela[locus], oglen[locus],
                      ogmut[locus]);

                  for (i = 0; i < numpops; i++)
                    {
                      for (j = 1; j <= num[i]; j++)
                        {
                          if (ropt[DIPLOIDGENENAMES])
                            kd = (j + 1) / 2;
                          else
                            kd = j;
                          sprintf (tempstr, "pop%d_%d", i, kd);
                          FP "%s", tempstr);
                          for (k = 0; k < 10 - strlen (tempstr); k++)
                            FP " ");
                          for (k = 0; k < seqlength; k++)
                            FP "T");
                          FP "\n");
                        }
                    }
                }
              else
                {
                  locus--;
                  numloci_nomuts++;
                }
            }
        }
      if (umodel != IS)
        free (tree);
    }
}                               // dosim 

void finish (void)
{
  int i, j, jj;
  if (ropt[PRINTDATASUMMARY])
    {

      FP "\nCounts of mutation models:\n");
      for (i = IS; i <= HKY; i++)
        if (modelcount[i] > 0)
          {
            switch (i)
              {
              case IS:
                FP "IS ");
                break;
              case INFINITEALLELES:
                FP "INFINITEALLELES ");
                break;
              case SW:
                FP "SW ");
                break;
              case JOINT:
                FP "JOINT ");
                break;
              case HKY:
                FP "HKY ");
                break;
              }
            FP "%d ", modelcount[i]);
          }
      FP "\n");
      if (umodel == IS || umodel == JOINT || umodel == HKY)
        FP "\nAverage number of sequence mutations per locus: %.2f \n",
          (double) meannumuts / (double) nloci);
      if (umodel == SW || umodel == JOINT)
        FP "\nAverage number of stepwise mutations per locus: %.2f \n",
          (double) meannumutssw / (double) nloci);
      if (umodel == INFINITEALLELES)
        FP "\nAverage number of infinite allele mutations per locus: %.2f \n",
          (double) meannumutsIA / (double) nloci);
      FP "\n Theta estimates\n");
      if (modelcount[IS] > 0 || modelcount[HKY] > 0 || modelcount[JOINT] > 0)
        {
          FP "   Sequences  - mean of Watterson's estimator\n");
          for (j = 0; j < numpops; j++)
            FP "\tpop%d: %.3f", j, thetaest[j] / (double) nlociseqpoly[j]);
          FP "\n\n");
          FP "   Sequences - mean of pi values \n");
          for (j = 0; j < numpops; j++)
            FP "\tpop%d: %.3f", j, meanpi[j] / (double) nlociseqpoly[j]);
          FP "\n\n");
        }
      if (modelcount[SW] > 0 || modelcount[JOINT] > 0)
        {
          FP "   STRs - stepwise mutation model - mean of squared differences\n");
          for (j = 0; j < numpops; j++)
            FP "\tpop%d: %.3f", j, thetaestsw[j] / (double) nlocistrpoly[j]);
          FP "\n\n");
        }
      if (modelcount[IS] > 0 || modelcount[HKY] > 0 || modelcount[JOINT] > 0)
        {
          FP "\n Tajima D values\n");
          for (j = 0; j < numpops; j++)
            FP "\tpop%d: %.3f", j, meantajd[j] / (double) nlociseqpoly[j]);
          FP "\n\n");
        }

      FP "\n Divergence summaries");
      if (modelcount[IS] > 0 || modelcount[HKY] > 0 || modelcount[JOINT] > 0)
        {
          FP "\n   Sequences");
          FP "\nAbove diagonal:net divergence. Below diagonal: mean pairwise divergence. \n");
          for (j = 0; j < numpops; j++)
            FP "\t%d", j);
          FP "\n");
          for (j = 0; j < numpops; j++)
            {
              FP "%d", j);
              for (jj = 0; jj < numpops; jj++)
                {
                  if (jj == j)
                    FP "\t - ");
                  else
                    FP "\t%.2f", meandiv[j][jj] / nlociseqdiv[j][jj]);
                }
              FP "\n");
            }
          FP "\n");
        }
      if (modelcount[SW] > 0 || modelcount[JOINT] > 0)
        {
          FP "\n   STR - stepwise mutation model  (if any loci have multiple linked STRs, only first one is used for these calculations)");
          FP "\nAbove diagonal:net divergence. Below diagonal: mean pairwise divergence. \n");
          for (j = 0; j < numpops; j++)
            FP "\t%d", j);
          FP "\n");
          for (j = 0; j < numpops; j++)
            {
              FP "%d", j);
              for (jj = 0; jj < numpops; jj++)
                {
                  if (jj == j)
                    FP "\t - ");
                  else
                    FP "\t%.1f", meandivsw[j][jj] / nlocistrdiv[j][jj]);
                }
              FP "\n");
            }
          FP "\n");
        }
      FP "\n");
      if (modelcount[HKY] > 0)
        {
          FP "For HKY loci, mean number of transition events: %.3f  mean number of transversion events: %.3f\n", numutstrans / (float) modelcount[HKY], numutstranv / (float) modelcount[HKY]);
        }
      if (heylaboptions[FOURGAMETETEST])
        fourgametetest (nloci);
      if (recrate > 0.0)
        FP "\nAverage number of recombination events per locus: %.3f",
          (double) meannumrec / (double) nloci);
      FP "\nAverage number of migration events per locus: %.3f\n",
        (double) nummig / (double) nloci);
      FP "Number of loci simulated - but not used - becuase no mutations occured: %d \n", numloci_nomuts);
      FP "\n");
    }
  fclose (outfile);
  free (urela);
  free (oglen);
  free (ogmut);
  if (loadlocusvaluefile)
    {
      for (i = 0; i < nlociallocate; i++)
        free (locusnames[i]);
      free (locusnames);
      for (i = 0; i < nlociallocate; i++)
        free (locussamplesizes[i]);
      free (locussamplesizes);
      for (i = 0; i < nlociallocate; i++)
        free (locusmutationrate[i]);
      free (locusmutationrate);
      free (locusinheritance);
      free (locusmutationmodel);
      free (locuslength);
      free (locusttratio);
    }
  exit (0);
}                               /* finish */

int main (int argc, char *argv[])
{
#ifdef  __MWERKS__
  argc = ccommand (&argv);
  SIOUXSettings.autocloseonquit = _true;
  SIOUXSettings.asktosaveonclose = _false;
#endif
#ifdef HPDBG
  tmpFlag = _CrtSetDbgFlag (_CRTDBG_REPORT_FLAG);
  tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
  tmpFlag |= _CRTDBG_CHECK_CRT_DF;
  tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
  tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
  _CrtSetDbgFlag (tmpFlag);
#endif

  start (argc, argv);
  dosim ();
  finish ();
}                               /* main */
