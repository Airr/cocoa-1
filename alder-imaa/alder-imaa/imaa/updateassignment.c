/* IMa2 2007-2011 Jody Hey, Rasmus Nielsen and Sang Chul Choi */
#include <ctype.h>
#include <search.h>
#include "config-common.h"
#undef GLOBVARS
#include "imamp.h"
#include "update_gtree_common.h"
#include "utilities.h"
#include "updateassignment.h"
#include "imastructure.h"

extern char *outfilename;
extern char *trueassignment;
extern char *individualfilename;

extern char popnames[][NAMELENGTH];
/* Class Edgemiginfo */
static struct edgemiginfo *saoems;
static struct edgemiginfo *sanems;
static struct edgemiginfo *saosms;
static struct edgemiginfo *sansms;

/* Class Nnminus1 */
static double *snnminus1;

/* Class Sbf */
static im_bfupdate sbfold;
static im_bfupdate sbfnew;

/* Class Ginfo */
static im_ginfo *saGiold;
static im_ginfo *saGioldz1;
static im_ginfo *saGioldz2;
static im_ginfo *saGinew;
static im_ginfo *saGinewz1;
static im_ginfo *saGinewz2;

/* Member sasigned */
static double **sassigned;

/* Class Individuals */
static int snind;
static int snind_known;
static int snind_unknown;
static int *sngenes_ind;
static int ssizePi;
static int **sind2gi;        
static int *sinds;
static int sci;
static char** sindividualName;
static int sindividualNameSize;
static int* sindividualAssignment;

/* Class Ptree */
static im_popntree saC;            /* _s_tatic _a_ssignment _C_hain */

/* Class Saved */
static im_savedlocus *saT;         /* _s_tatic _a_ssignment _T_ree */

static int snasn;
static int *saasn;
static void imaInitEdgemiginfo ();
static void imaFreeEdgemiginfo ();
static void imaIndInit ();
static void imaIndFree ();
static void imaIndSet ();
static void imaIndSetPairs ();
static void imaGtreeReseti ();
static void imaGtreeCopyi ();
static int imaIndN (int k);
void imaIndReadIndividualData ();
static void imaIndReadIndividualFile (const char *);
static int imaUtilityExistString (char** a, int n, const char* b);
static void imaInitNnminus1 ();
static void imaFreeNnminus1 ();
static int imaSbfInit ();
static int imaSbfFree ();
static void imaGinfoInit (im_ginfo **g);
static void imaGinfoFree (im_ginfo **g);
static void imaGinfoReset (im_ginfo *g);
static void imaInitSagi ();
static void imaFreeSagi ();
static double imaGtreeBranchLength (int ci, int li, int ei);
static int imaGtreeSister (int ci, int li, int ei);
static int imaGtreePop (int ci, int li, int ei);
static double imaGtreeTime (int ci, int li, int ei);
static int imaGtreeNmig (int ci, int li, int ei);
static void imaGnodePrint_node (FILE *fp, struct edge *gtree, int e);
static void imaGnodePrint (FILE *fp, struct edge *gtree, int e);
static void imaGnodePrint_nodeseq (FILE *fp, struct edge *gtree, int e, int si);
void imaGnodePrintseq (FILE *fp, struct edge *gtree, int e, int si);
static int findperiodoftime (int ci, double ktime);
static double imaAsnValue_mean (int ci, int li);
static void imaPtreeSetIsland ();
static void imaPtreeSet ();
static void imaPtreeUnset ();
static void imaPnodePrint_node (FILE *fp, struct popedge *ptree, int e);
static void imaPnodePrint (FILE *fp, struct popedge *ptree, int e);
static void imaSavedSaveLocusInfo (int ci, int li);
static void imaSavedRestoreLocusInfo (int ci, int li);
static void   imaSavedReset (int li);
static void   imaSavedInit ();
static void   imaSavedFree ();
static int    imaSavedSaveBranch (int ci, int li, int ei);
static void   imaSavedRestore (int ci, int li);
/* assertgenealogy */
#ifndef NDEBUG
static void assertgenealogylocdiploid (int ci, int li);
static void assertgenealogyindividual (int ci);
static void assertgenealogyedgelabel (int ci, int li, int ei);
static void assertgenealogylabel (int ci);
static void assertgenealogyloclabel (int ci, int li);
static void assertgenealogyultrametric (int ci);
static void assertgenealogylocultrametric (int ci, int li);
static void assertgenealogylocbranch (int ci, int li);
static void assertgenealogybranch (int ci);
static void IMA_check_popn_up (int kprevperiod, int prevpop, int kperiod, int pop);
static void IMA_check_popn_move (int notpop, int kperiod, int pop);
static void assertgenealogyasn (int ci);
#endif /* NDEBUG */
static void imaStringRemoveSpaces (char *s1, const char *s2);
static double relabel (int ci, int *mpart);
static void IMA_edge_fillmiginfo (int ci, int li, int ei, 
                                  struct edgemiginfo *em);
void IMA_edge_copynewmig (int ci, int li, struct edgemiginfo *em);
static double imaRandomPickpopn (int *topop, int ci, int pop, int apop);
static double imaRandomPickpopnLocus (int *topop, int ci, int li, int pop, int apop);
static double addmigration_relabel (int ci, int li, int newpop, 
                                    int oldmigcount,
                                    int *newmigcount,
                                    struct edgemiginfo *olde, 
                                    struct edgemiginfo *newe);

static void copy_probcalc_all (struct probcalc *dest, struct probcalc *srce);
static double bfmove (int ci);
static void bfmove_chooseA1 (int ci, int li, im_bfupdate *pbf);
static double bfmove_selectA1 (int ci, int li, im_bfupdate *pbf, double lq);
static int bfmove_chooseAnEdge (int ci, int li, im_bfupdate *pbf, double ct, int pc);
static int bfmove_selectAnEdge (int ci, int li, im_bfupdate *pbf, im_bfupdate *qbf);
static double bfmove_nextI (int ci, int li,
                            double lq, im_event *ek, 
                            int np, double rates[], 
                            im_ginfo *gk, 
                            im_bfupdate *pbf,
                            int *ki,
                            int z1,
                            char *is_coalesced1,
                            int *pc1);
static int bfmove_ratesI (double (*r)[2 * MAXPOPS],
                          im_ginfo *gk,
                          im_ginfo *gkz1,
                          im_bfupdate *pbf,
                          int ki,
                          int pc1);
static void bfmove_ratesIII (double (*rates)[2 * MAXPOPS], im_ginfo *gk, im_bfupdate *pbf,
                             int ki,
                             int pc1, int pc3);
static void IMA_genealogy_detach (int ci, int li, im_bfupdate *pbf,
                                  int z1);
static int IMA_sbf_c (int ci, int li, int edge, int pi, 
                      int *seqz,
                      int *n, int **l, int *a);
static int IMA_sbf_m (int edge, int pi, int pj,
                      int *n, int **l, int *a);
static int IMA_sbf_nsum (im_bfupdate *pbf,
                         int pc1, int pc3);
static void IMA_sbf_lineages (im_bfupdate *pbf, int ci, int li, int z1);
static void IMA_intervals_collectsbfnew (int ci, int li, im_edgemiginfo *newm3);
static double bfq (im_bfupdate *pbf, im_bfupdate *qbf, 
                   int ci, int li, 
                   im_ginfo *gk,
                   im_ginfo *gkz1);
static double IMA_choose_migtopop (int *topop, int ki, int pi, im_ginfo *gk);
static void IMA_edgemiginfo_copymig (im_edgemiginfo *em, int ci, int li, int ei);
static void IMA_genealogy_copymig (int ci, int li, int ei, im_edgemiginfo *em);
static void IMA_genealogy_appendmig (int ci, int li, int sis, im_edgemiginfo *em, 
                                     int edge, int down);
static int IMA_intervals_collect (int ci, int li, double upt, double dnt);
                                  
static int IMA_genealogy_derivetheta (char w, int ci, int li, int z1);
int IMA_ginfo_computetheta (im_ginfo *gk);
#ifdef DELETETHIS
static int ran_discrete (int n, ...);
#endif /* DELETETHIS */
static int ran_discretea (int n, double pr[]); 
static int ran_discreteloga (int n, double pr[]); 
static int normalizeLoga (int n, double pr[]); 
static double bflocusmove (int ci,int li);
static int tptExistCoalescentPeriod (int ci, int rperiod);
static int tptChooseAnIngroup (int ci, int rperiod, int *popx, int *popa);
static int Phylogeny_tptChooseAnIngroup (int ci, int rperiod, int *popx, int *popa);
static int Phylogeny_tptChooseAllPopulations (int ci, int rperiod, 
                                       int *popal, int *popbl, int *popcl,
                                       int *popau, int *popbu, int *popcu);
static int tptOutgroup (int ci, int rperiod);
static int Phylogeny_sister (int ci, int pi);
static double tptmove (int ci, int rperiod, int popx, int popa, int popb, int popc);
static int imaGtreeSwap (int ci, int li, int ei, 
                         int popx, int popa, int popb, int popc,
                         double upt, double dnt);
static double treemoveI (int ci, int rperiod, int popx, int popa, int popb, int popc);
static double treemoveII (int ci, int rperiod, 
                          int popal, int popbl, int popcl,
                          int popau, int popbu, int popcu);
static int Phylogeny_choose_a_period (int ci, int * rperiod);
static int Phylogeny_isCoalescent (int ci, double upt, double dnt, int b1, int b2);
static int Phylogeny_isCoalescent2 (int ci, double upt, double dnt, 
                                    int b1, int b2, int b3);
static int Phylogeny_isMigration (int ci, double upt, double dnt, int b1, int b2);
static int tptChooseCoalescentPeriod (int ci);
static int Genealogy_skipMig (int ci, int li, int ei, double t);
static int Migration_above_startpopn (int ci, int li, int ei, int mi, int ki);
static int Migration_startpopn (int ci, int li, int ei, int mi);
static char*  IMA_getIndividualName (int i);
static int    IMA_getGenotype (int ii, int li, int *ig1, int *ig2);
static int imaPtreeNsispop (int ci);
static int imaPtreeSispop (int ci, int **s);
int IMA_rgf_setSnasn (int nind, int np);
int IMA_convertStr2Asn (int ncol, int *a, char *s);
int IMA_io_readp_header (char *fname, char **name, int mc);
int IMA_io_readp (char *fname, int ***m, int *mr, int *mc, int nburnin);
int IMA_gusfield_distanceRemoved (int *A, int **m, int nrow, int ncol, 
                                  int *perms, int nperm,
                                  double *r);
int IMA_gusfield_distance (int *A, int **m, int nrow, int ncol); 
int IMA_gusfield_squaredDistance (int *A, int **m, int nrow, int ncol); 
double IMA_gusfield_varianceDistance (int *A, int **m, int nrow, int ncol, double mean); 
/*
static int skip_a_line (FILE *fp);
*/

int IMA_gdistanceRemoved (int n, int *Ai, int *Bi, int *perms, int nperm, char *Ci);
int IMA_gdistanceOrderNEW (int n, int *Ai, int *Bi, int m, int *od);
int IMA_gdistanceOrder (int n, int *Ai, int *Bi, int m, int *od);
int IMA_gdistance (int n, int *Ai, int *Bi);
int IMA_gdistanceSimple (int n, int *Ai, int *Bi);
int IMA_max_intarray (int n, int *A);


int IMA_permLexSuccessor (int n, int *p);
int IMA_permLexRank (int n, int *p);
int IMA_permLexUnrank (int *p, int n, int r);
int IMA_permFprintf (FILE *fp, int n, int *p);
int IMA_factorial (int n);
static int im_event_cmp (const void *a, const void *b);

void 
recordassignment_header (char *fname, int ci, int bIsPerChain)
{
  int li;
  int ii;
  int ei;
  int ki;
  int li2;
  char strName[FNSIZE];
  char *assignfilename;
  int lenname;
  FILE *assignfile = NULL;
  FILE *namefile = NULL;
  FILE *fpassigngenealogy = NULL;

  /* we find individuals for each locus. */
  lenname = 30 + strlen (fname);
  assignfilename = malloc (lenname * sizeof (char));

  if (bIsPerChain == 0)
    {
      strcpy (assignfilename, fname);
      strcat (assignfilename, ".in.p");
      assignfile = fopen (assignfilename, "w");

      strcpy (assignfilename, fname);
      strcat (assignfilename, ".tex");
      namefile = fopen (assignfilename, "w");

      strcpy (assignfilename, fname);
      strcat (assignfilename, ".asn");
    }
  else
    {
      sprintf (assignfilename, "%s.%s.asn", fname, C[ci]->name);
    }
  fpassigngenealogy = fopen (assignfilename, "w");

  if (bIsPerChain == 0)
    {
      if (assignfile == NULL || namefile == NULL)
        {
          IM_err (IMERR_CREATEFILEFAIL, 
                  "Assignment output file cannot be created: %s", 
                  assignfilename);
        }
      fprintf (assignfile, "Gen");
      fprintf (assignfile, "\tlnL");
    }

  fprintf (fpassigngenealogy, "Gen");
  if (bIsPerChain == 1)
    {
      fprintf (fpassigngenealogy, "\tbeta");
      for (ki = 0; ki < numsplittimes; ki++)
        {
          fprintf (fpassigngenealogy, "\tt%d", ki);
        }   
    }
  fprintf (fpassigngenealogy, "\tlnL");

  if (bIsPerChain == 0)
    {
      fprintf (namefile, "\\begin{tabular}{l}\n");
    }
  for (ii = 0; ii < snind; ii++)
    {
      li = 0;
      ei = sind2gi[ii][li];
      if (!(ei < 0))
        {
          imaStringRemoveSpaces (strName, L[li].gNames[ei]);
          if (bIsPerChain == 0)
            {
              fprintf (assignfile, "\tP(%s)", strName);
              fprintf (namefile, "%s\\tabularnewline\n", strName);
            }
          fprintf (fpassigngenealogy, "\tP(%s)", strName);
        }
      else
        {
          /* If the individual does not have genes at first locus, we look for
           * it from the rest of loci. */
          for (li2 = 1; li2 < nloci; li2++)
            {
              ei = sind2gi[ii][li2];
              if (!(ei < 0))
                {
                  imaStringRemoveSpaces (strName, L[li2].gNames[ei]);
                  if (bIsPerChain == 0)
                    {
                      fprintf (assignfile, "\tP(%s)", strName);
                      fprintf (namefile, "%s\\tabularnewline\n", strName);
                    }
                  fprintf (fpassigngenealogy, "\tP(%s)", strName);
                  break;
                }
            }
        }
    }

  if (bIsPerChain == 0)
    {
      fprintf (namefile, "\\end{tabular}\n");
      fprintf (assignfile, "\n");
    }
  fprintf (fpassigngenealogy, "\n");

  if (bIsPerChain == 0)
    {
      fclose (assignfile);
      assignfile = NULL;
      fclose (namefile);
      namefile = NULL;
    }
  fclose (fpassigngenealogy);
  fpassigngenealogy = NULL;
  free (assignfilename);
  assignfilename = NULL;
  return;
}
void 
imaAdmixturePrintHeader (char *fname, int ci, int bIsPerChain)
{
  int li;
  int ei;
  int ki;
  int c;
  char strName[FNSIZE];
  char *assignfilename;
  int lenname;
  FILE *assignfile = NULL;
  FILE *namefile = NULL;
  FILE *fpassigngenealogy = NULL;

  /* we find individuals for each locus. */
  lenname = 30 + strlen (fname);
  assignfilename = malloc (lenname * sizeof (char));

  if (bIsPerChain == 0)
    {
      strcpy (assignfilename, fname);
      strcat (assignfilename, ".in.p");
      assignfile = fopen (assignfilename, "w");

      strcpy (assignfilename, fname);
      strcat (assignfilename, ".tex");
      namefile = fopen (assignfilename, "w");

      strcpy (assignfilename, fname);
      strcat (assignfilename, ".asn");
    }
  else
    {
      sprintf (assignfilename, "%s.%s.asn", fname, C[ci]->name);
    }
  fpassigngenealogy = fopen (assignfilename, "w");

  if (bIsPerChain == 0)
    {
      if (assignfile == NULL || namefile == NULL)
        {
          IM_err (IMERR_CREATEFILEFAIL, 
                  "Assignment output file cannot be created: %s", 
                  assignfilename);
        }
      fprintf (assignfile, "Gen");
      fprintf (assignfile, "\tlnL");
    }

  fprintf (fpassigngenealogy, "Gen");
  if (bIsPerChain == 1)
    {
      fprintf (fpassigngenealogy, "\tbeta");
      for (ki = 0; ki < numsplittimes; ki++)
        {
          fprintf (fpassigngenealogy, "\tt%d", ki);
        }   
    }
  fprintf (fpassigngenealogy, "\tlnL");

  if (bIsPerChain == 0)
    {
      fprintf (namefile, "\\begin{tabular}{l}\n");
    }

  c = 0; 
  for (li = 0; li < nloci; li++)
    {
      for (ei = 0; ei < L[li].numgenes; ei++)
        {
          imaStringRemoveSpaces (strName, L[li].gNames[ei]);
          sprintf (strName, "%s_%04d", strName, li);
          if (bIsPerChain == 0)
            {
              fprintf (assignfile, "\tP(%s)", strName);
              fprintf (namefile, "%s\\tabularnewline\n", strName);
            }
          fprintf (fpassigngenealogy, "\tP(%s)", strName);
          c++;
        }
    }
  assert (c == total_numgenes);

  if (bIsPerChain == 0)
    {
      fprintf (namefile, "\\end{tabular}\n");
      fprintf (assignfile, "\n");
    }
  fprintf (fpassigngenealogy, "\n");

  if (bIsPerChain == 0)
    {
      fclose (assignfile);
      assignfile = NULL;
      fclose (namefile);
      namefile = NULL;
    }
  fclose (fpassigngenealogy);
  fpassigngenealogy = NULL;
  free (assignfilename);
  assignfilename = NULL;
  return;
}

int 
IMA_ninds ()
{
  assert (snind > 0);
  return snind;
}

int 
IMA_nindsunknown ()
{
  assert (!(snind_unknown < 0));
  return snind_unknown;
}

void 
recordassignment (char *fname, int ci, int gen, int bIsPerChain)
{
  int li;
  int ii;
  int ei;
  int ki;
  FILE *assignfile = NULL;
  FILE *fpassigngenealogy = NULL;
  char assignfilename[FNSIZE];
  double pdg;
  int *a = NULL;
  int *pa;

  a = malloc (snind * sizeof (int));

  if (bIsPerChain == 0)
    {
      strcpy (assignfilename, fname);
      strcat (assignfilename, ".in.p");
      assignfile = fopen (assignfilename, "a");
      if (assignfile == NULL)
        { 
          IM_err (IMERR_APPENDFILEFAIL, "Assignment output file cannot be opened: %s", assignfilename);
        }
      fprintf (assignfile, "%d", gen);

      strcpy (assignfilename, fname);
      strcat (assignfilename, ".asn");
    }
  else
    {
      sprintf (assignfilename, "%s.%s.asn", fname, C[ci]->name);
    }

  fpassigngenealogy = fopen (assignfilename, "a");
  if (fpassigngenealogy == NULL)
    { 
      IM_err (IMERR_APPENDFILEFAIL, "Raw assignment output file cannot be opened: %s", assignfilename);
    }
  fprintf (fpassigngenealogy, "%d", gen);
  if (bIsPerChain == 1)
    {
      fprintf (fpassigngenealogy, "\t%lf", beta[ci]);
      for (ki = 0; ki < numsplittimes; ki++)
        {
          fprintf (fpassigngenealogy, "\t%lf", C[ci]->tvals[ki]);
        }   
    }

  pdg = C[ci]->allpcalc.pdg;
  if (bIsPerChain == 0)
    {
      fprintf (assignfile, "\t%lf", pdg);
    }
  fprintf (fpassigngenealogy, "\t%lf", pdg);

  /* how do I know individuals of a locus? */
  pa = a;
  for (ii = 0; ii < snind; ii++)
    {
      li = saC.indlist[ii][0].li;
      ei = saC.indlist[ii][0].ei;
      *pa = C[ci]->G[li].gtree[ei].pop;
      pa++;
    }

  /* A raw assignment is printed out without RGF conversion. */
  pa = a;
  for (ii = 0; ii < snind; ii++)
    {
      fprintf (fpassigngenealogy, "\t%d", *pa);
      pa++;
    }
  fprintf (fpassigngenealogy, "\n");

  /* An assignment is printed out with RGF conversion. */
  if (bIsPerChain == 0)
    {
      IMA_rgs_convert (a, snind);
      pa = a;
      for (ii = 0; ii < snind; ii++)
        {
          fprintf (assignfile, "\t%d", *pa);
          pa++;
        }
      fprintf (assignfile, "\n");
    }

  if (bIsPerChain == 0)
    {
      fclose (assignfile);
      assignfile = NULL;
    }
  fclose (fpassigngenealogy);
  fpassigngenealogy = NULL;
  free (a);
  a = NULL;
 
  return;
}

void 
imaAdmixturePrintChain (char *fname, int ci, int gen, int bIsPerChain)
{
  int li;
  int ei;
  int ki;
  FILE *assignfile = NULL;
  FILE *fpassigngenealogy = NULL;
  char assignfilename[FNSIZE];
  double pdg;
  int *a = NULL;
  int *pa;

  a = malloc (total_numgenes * sizeof (int));

  if (bIsPerChain == 0)
    {
      strcpy (assignfilename, fname);
      strcat (assignfilename, ".in.p");
      assignfile = fopen (assignfilename, "a");
      if (assignfile == NULL)
        { 
          IM_err (IMERR_APPENDFILEFAIL, "Assignment output file cannot be opened: %s", assignfilename);
        }
      fprintf (assignfile, "%d", gen);

      strcpy (assignfilename, fname);
      strcat (assignfilename, ".asn");
    }
  else
    {
      sprintf (assignfilename, "%s.%s.asn", fname, C[ci]->name);
    }

  fpassigngenealogy = fopen (assignfilename, "a");
  if (fpassigngenealogy == NULL)
    { 
      IM_err (IMERR_APPENDFILEFAIL, "Raw assignment output file cannot be opened: %s", assignfilename);
    }
  fprintf (fpassigngenealogy, "%d", gen);
  if (bIsPerChain == 1)
    {
      fprintf (fpassigngenealogy, "\t%lf", beta[ci]);
      for (ki = 0; ki < numsplittimes; ki++)
        {
          fprintf (fpassigngenealogy, "\t%lf", C[ci]->tvals[ki]);
        }   
    }

  pdg = C[ci]->allpcalc.pdg;
  if (bIsPerChain == 0)
    {
      fprintf (assignfile, "\t%lf", pdg);
    }
  fprintf (fpassigngenealogy, "\t%lf", pdg);

  /* Raw population labels are stored for the reference to gene trees. */
  pa = a;
  for (li = 0; li < nloci; li++)
    {
      for (ei = 0; ei < L[li].numgenes; ei++)
        {
          *pa = C[ci]->G[li].gtree[ei].pop;
          pa++;
        }
    }

  /* The raw assignment is printed out without RGF conversion. */
  pa = a;
  for (li = 0; li < nloci; li++)
    {
      for (ei = 0; ei < L[li].numgenes; ei++)
        {
          fprintf (fpassigngenealogy, "\t%d", *pa);
          pa++;
        }
    }
  fprintf (fpassigngenealogy, "\n");

  /* An assignment is printed out with RGF conversion. */
  if (bIsPerChain == 0)
    {
      IMA_rgs_convert (a, total_numgenes);
      pa = a;
      for (li = 0; li < nloci; li++)
        {
          for (ei = 0; ei < L[li].numgenes; ei++)
            {
              fprintf (assignfile, "\t%d", *pa);
              pa++;
            }
        }
      fprintf (assignfile, "\n");
    }

  if (bIsPerChain == 0)
    {
      fclose (assignfile);
      assignfile = NULL;
    }
  fclose (fpassigngenealogy);
  fpassigngenealogy = NULL;
  free (a);
  a = NULL;
 
  return;
}

void 
recordassignmentloci (char *fname, int ci)
{
  static int gen = 0;
#ifdef DEBUG
  int i;
#endif /* DEBUG */

  recordassignment (fname, ci, gen, 0);
#ifdef DEBUG
  if (numchains > 1)
    {
      for (i = 0; i < numchains; i++)
        {
          recordassignment (fname, i, gen, 1);
        }
    }
#endif /* DEBUG */
  gen++;
  return;
}
void 
imaAdmixturePrint (char *fname, int ci)
{
  static int gen = 0;
#ifdef DEBUG
  int i;
#endif /* DEBUG */

  imaAdmixturePrintChain (fname, ci, gen, 0);
#ifdef DEBUG
  if (numchains > 1)
    {
      for (i = 0; i < numchains; i++)
        {
          imaAdmixturePrintChain (fname, i, gen, 1);
        }
    }
#endif /* DEBUG */
  gen++;
  return;
}
int
imaParserReadString (FILE* fp, char* s, int sn)
{
  int c;
  int i;
  c = fgetc(fp);
  while (isspace(c))
    {
      c = fgetc(fp);
    }
  i = 0;
  while (!isspace(c) && i < sn)
    {
      s[i++] = c;
      c = fgetc(fp);
    }
  s[i] = '\0';
  if (c == '\n')
    {
      i = 1;
    }
  else
    {
      i = 0;
    }
  return i;
}

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

int
updateassignmentrelabel (int ci)
{
  int accp;                   /* accept (1) or reject (0)            */
  double logweight;           /* MH ratio                            */
  double logU;                /* log uniform                         */
  double tpw;                 /* log [p(G*)/p(G)]                    */
  double migweight;           /* hastings ratio for migration update */
  struct genealogy *G;        /* locus li                            */
  int mpart;
  int li;
  int nmigaftertreeweight;

  /* declarations must precede any statement for compatibility with Visual C++
   * compiler */
  G = NULL;
  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      assertgenealogy (ci);
    }

  if (ci == 0)
    {
      for (li = 0; li < nloci; li++)
        {
          L[li].a_rec->upinf[IM_UPDATE_ASSIGNMENT_RELABEL].tries++;
        }
    }
  Cupinf[ci].upinf[IM_UPDATE_ASSIGNMENT_RELABEL].tries++;

  /* save the current state of a chain */
  for (li = 0; li < nloci; li++)
    {
      imaSavedReset (li);
      imaSavedSaveLocusInfo (ci, li);
      copy_treeinfo (&saT[li].savedgweight, &C[ci]->G[li].gweight);
    }
  copy_treeinfo (&saC.savedallgweight, &C[ci]->allgweight);
  copy_probcalc (&saC.savedallpcalc, &C[ci]->allpcalc);

  /* the key proposal function of relabel method */
  migweight = relabel (ci, &mpart);

  if (migweight < REJECTINFINITESITESCONSTANT + 1.0)
    {
      /* Reject it. */
      accp = 0;
    }
  else
    {
      accp = 1;
      /* compute Metropolis-ratio */
      setzero_genealogy_weights (&C[ci]->allgweight);
      for (li = 0; li < nloci; li++)
        {
          setzero_genealogy_weights (&C[ci]->G[li].gweight);
          treeweight (ci, li);
          sum_treeinfo (&C[ci]->allgweight, &C[ci]->G[li].gweight);
        }
      /* check if the number of migration events after the call of treeweight is the
       * same as the number of migration events from update */
      nmigaftertreeweight = 0;
      for (li = 0; li < nloci; li++)
        {
           nmigaftertreeweight += C[ci]->G[li].mignum; 
        }
      assert (nmigaftertreeweight == mpart);
      /* We use allgweight to compute allpcalc */
      initialize_integrate_tree_prob (ci, &C[ci]->allgweight, &C[ci]->allpcalc);

      /* Log[p(G*)] - Log[p(G)] is [[tpw]] */
      tpw = C[ci]->allpcalc.probg - saC.savedallpcalc.probg;

      logU = log(uniform ());
      logweight = beta[ci] * tpw + migweight;
      assert (beta[ci] * tpw + migweight > -1e200
              && beta[ci] * tpw + migweight < 1e200);
    }

  if (accp == 1 && (logweight > 0.0 || logweight > logU))
    {
      /* accept the update */
      accp = 1;
      if (ci == 0)
        {
          for (li = 0; li < nloci; li++)
            {
              L[li].a_rec->upinf[IM_UPDATE_ASSIGNMENT_RELABEL].accp++;
            }
        }
      Cupinf[ci].upinf[IM_UPDATE_ASSIGNMENT_RELABEL].accp++;
      C[ci]->nasn[sbfold.pop]--;
      C[ci]->nasn[sbfnew.pop]++;
    }
  else
    {
      accp = 0;
      for (li = 0; li < nloci; li++)
        {
          /* restore edges changed during update */
          imaSavedRestore (ci, li); 
          imaSavedRestoreLocusInfo (ci, li);
          copy_treeinfo (&C[ci]->G[li].gweight, &saT[li].savedgweight);
        }
      copy_probcalc (&C[ci]->allpcalc, &saC.savedallpcalc);
      copy_treeinfo (&C[ci]->allgweight, &saC.savedallgweight);
    }

  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      assertgenealogy (ci);
    }
  return accp;
}

double
relabel (int ci, int *mpart)
{
  int li;
  int ei;
  double logprob;
  int newpop;
  int oldpop;
  int oldmigcount[MAXLOCI];
  int newmigcount[MAXLOCI];
  struct genealogy *G;
  struct edge *gtree; 
  int ii;
  int ij;
  int nei;
  double t;
  int ki;
  int minki;
  int ni;
  int down;

  /* Pick an individual to relabel. */
  if (modeloptions[NOMIGRATION] == 0)
    {
      ii = snind_known + randposint (snind_unknown);
    }
  else
    {
      ni = 0;
      for (ii = snind_known; ii < snind; ii++)
        {
          nei = sngenes_ind[ii];
          for (ij = 0; ij < nei; ij++)
            {
              li = saC.indlist[ii][ij].li;
              ei = saC.indlist[ii][ij].ei;
              down = C[ci]->G[li].gtree[ei].down;
              if (C[ci]->G[li].gtree[down].pop == C[ci]->G[li].gtree[ei].pop) 
                {
                  break;
                }
            }
          if (ij == nei)
            {
              sinds[ni] = ii;
              ni++;
            }
        }
      if (ni == 0)
        {
          logprob = REJECTINFINITESITESCONSTANT;
          return logprob;
        }
      else
        {
          ii = sinds[randposint (ni)]; 
        }
    }

  /* relabel the selected genes: newpop is chosen with logprob */
  /* we need a list of edge mig info structures */
  li = saC.indlist[ii][0].li;
  ei = saC.indlist[ii][0].ei;
  G = &C[ci]->G[li];
  gtree = G->gtree;
  oldpop = gtree[ei].pop;
  sbfold.pop = oldpop;

  if (modeloptions[NOMIGRATION] == 0)
    {
      logprob = imaRandomPickpopn (&newpop, ci, oldpop, C[ci]->rootpop);
    }
  else
    {
      /* We can only move to descendents of the ancestor. */
      minki = npops;
      nei = sngenes_ind[ii];
      for (ij = 0; ij < nei; ij++)
        {
          li = saC.indlist[ii][ij].li;
          ei = saC.indlist[ii][ij].ei;
          t = C[ci]->G[li].gtree[ei].time;
          ki = findperiodoftime (ci, t);
          if (minki > ki)
            {
              minki = ki;
            }
        }
      /* pop and minki are used to propose a new pop. */
      newpop = oldpop;
      while (newpop == oldpop)
        {
          newpop = randposint (npops);
          if (saC.popndown[newpop][minki] != saC.popndown[oldpop][minki])
            {
              newpop = oldpop;
            }
        }
      logprob = 0.0;
    }
  sbfnew.pop = newpop;

  nei = sngenes_ind[ii];
  for (ij = 0; ij < nei; ij++)
    {
      li = saC.indlist[ii][ij].li;
      ei = saC.indlist[ii][ij].ei;
      imaSavedSaveBranch (ci, li, ei);
    }

  for (ij = 0; ij < nei; ij++)
    {
      li = saC.indlist[ii][ij].li;
      ei = saC.indlist[ii][ij].ei;

      /* This should work with NOMIGRATION. */
      IMA_edge_fillmiginfo (ci, li, ei, &saoems[ij]); 
    }

  for (li = 0; li < nloci; li++)
    {
      oldmigcount[li] = C[ci]->G[li].mignum;
    }

  for (ij = 0; ij < nei; ij++)
    {
      li = saC.indlist[ii][ij].li;
      ei = saC.indlist[ii][ij].ei;
      C[ci]->G[li].gtree[ei].mig[0].mt = -1;
      if (modeloptions[NOMIGRATION] == 0)
        {
          logprob += addmigration_relabel (ci, li, newpop, 
                                           oldmigcount[li],
                                           &newmigcount[li],
                                           &saoems[ij], &sanems[ij]);
        }
      else
        {
          /* We check if edge's old pop and new pop share the same ancestral
           * population.  
           * FIXME: this will lead to high rejection rates. We may figure out
           * which individual can move to other population. We may have to pick
           * individuals that can move.
           */
          oldmigcount[li] = 0;
          newmigcount[li] = 0;
          t = C[ci]->G[li].gtree[ei].time;
          ki = findperiodoftime (ci, t);
          if (saC.popndown[oldpop][ki] != saC.popndown[newpop][ki])
            {
              /* Proposed label cannot move. */
              logprob = REJECTINFINITESITESCONSTANT;
              break;
            }
        }
      oldmigcount[li] = newmigcount[li];
    }
 
  *mpart = 0;
  if (modeloptions[NOMIGRATION] == 0)
    {
      for (li = 0; li < nloci; li++)
        {
          *mpart += oldmigcount[li];
        }

      for (ij = 0; ij < nei; ij++)
        {
          li = saC.indlist[ii][ij].li;
          ei = saC.indlist[ii][ij].ei;
          assert (sanems[ij].edgeid == ei);
          assert (sanems[ij].li == li);

          /* This should work with NOMIGRATION. */
          IMA_edge_copynewmig (ci, li, &sanems[ij]);
        }
    }

  if (logprob > REJECTINFINITESITESCONSTANT)
    {
      for (ij = 0; ij < nei; ij++)
        {
          li = saC.indlist[ii][ij].li;
          ei = saC.indlist[ii][ij].ei;
          gtree = C[ci]->G[li].gtree;
          gtree[ei].pop = newpop;
        }
    }

  return logprob;
}
void
IMA_edge_fillmiginfo (int ci, int li, int ei, 
                      struct edgemiginfo *em)
{
  int i;
  int j;
  int down;                        /* down edge             */
  struct genealogy *G;             /* locus li              */
  struct edge *gtree;              /* genealogy of locus li */

  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  down = gtree[ei].down;

  /*******************************************************/
  /* oldedgemig                                          */
  /*******************************************************/
  IMA_reset_edgemiginfo (em);
  em->li = li;
  em->sisid = -1; /* do we need sisid? */
  em->edgeid = ei;
  em->upt = imaGtreeTime (ci, li, ei);
  em->dnt = gtree[ei].time;
  em->pop = gtree[ei].pop;
  em->temppop = gtree[ei].pop;
  em->fpop = gtree[down].pop;
  /* b, e, mtimeavail, and mtall are set. */
  fillmiginfoperiods (ci, em);
  /* migration events are copied */
  em->mig[0].mt = -1.0;
  for (j = 0; j <= em->b; j++)
    {
      em->mp[j] = 0;
    }
  em->mpall = 0;

  i = 0;
  j = em->b;
  while (gtree[ei].mig[i].mt > -0.5)
    {
      while (gtree[ei].mig[i].mt > C[ci]->tvals[j])
        {
          j++;
          em->mp[j] = 0;
        }
      em->mp[j]++;
      em->mpall++;
      em->mig[i] = gtree[ei].mig[i];
      i++;
    }
  em->mig[i].mt = -1;

  return;
}
void
IMA_edge_copynewmig (int ci, int li, struct edgemiginfo *em)
{
  int i;
  int ei;
  struct edge *gtree;              /* genealogy of locus li */
  
  ei = em->edgeid;
  gtree = C[ci]->G[li].gtree;

  i = 0;
  while (em->mig[i].mt > 0)
    {
      checkmigt (i, &gtree[ei]);
      copymig (&(gtree[ei].mig[i]), &(em->mig[i]));
      i++;
    }
  gtree[ei].mig[i].mt = -1;
  return;
}


double
imaRandomPickpopn (int *topop, int ci, int pop, int apop)
{
  double v;
  int n;
  int ip;
  int pi;
  int nzero;

  assert (assignmentoptions[POPULATIONASSIGNMENTASN] == 1);

  *topop = pop;
  n = saC.npopnlist[apop][0];
  assert (!(apop < npops));
  while (*topop == pop)
    {
      ip = randposint (n);
      *topop = saC.popnlist[apop][0][ip];
    }
  assert (n > 1);
  /* v = -log ((double) (n - 1)); */

  nzero = 0;
  for (pi = 0; pi < npops; pi++)
    {
      if (C[ci]->nasn[pi] == 0)
        {
          nzero++;
        }
    }
  assert (C[ci]->nasn[pop] > 0);
  v = 0.0;
  if (C[ci]->nasn[pop] == 1 && nzero > 0 && C[ci]->nasn[*topop] > 0)
    {
      v = log ((double) (nzero + 1.0));
    }
  else if (C[ci]->nasn[pop] > 1 && nzero > 1 && C[ci]->nasn[*topop] == 0)
    {
      v = -log ((double) (nzero));
    }

  return v;
}

double
imaRandomPickpopnLocus (int *topop, int ci, int li, int pop, int apop)
{
  double v;
  int n;
  int ip;
  int pi;
  int nzero;

  *topop = pop;
  n = saC.npopnlist[apop][0];
  assert (!(apop < npops));
  while (*topop == pop)
    {
      ip = randposint (n);
      *topop = saC.popnlist[apop][0][ip];
    }
  assert (n > 1);
  /* v = -log ((double) (n - 1)); */

  nzero = 0;
  for (pi = 0; pi < npops; pi++)
    {
      if (C[ci]->nasn[pi + li * npops] == 0)
        {
          nzero++;
        }
    }
  assert (C[ci]->nasn[pop + li * npops] > 0);
  v = 0.0;
  if (C[ci]->nasn[pop + li * npops] == 1 && nzero > 0 && C[ci]->nasn[*topop + li * npops] > 0)
    {
      v = log ((double) (nzero + 1.0));
    }
  else if (C[ci]->nasn[pop + li * npops] > 1 && nzero > 1 && C[ci]->nasn[*topop + li * npops] == 0)
    {
      v = -log ((double) (nzero));
    }

  return v;
}

double
addmigration_relabel (int ci, int li, int newpop, 
                      int oldmigcount,
                      int *newmigcount,
                      struct edgemiginfo *olde, 
                      struct edgemiginfo *newe)
{
  int edge;
  double weight;
  double mproposenum, mproposedenom;
  double mparamf, mparamb;
  struct edge *gtree; 
  int mcount;

  gtree = C[ci]->G[li].gtree;
  assert (C[ci]->G[li].mignum >= 0 );
  /* determine current migration rate to use for update */
  
  /* To reflect Jody's comment: we use overall migration rates of a genealogy.
   * This may help assignment update a little bit. 
   * mparamf = calcmrate (olde->mpall, olde->mtall); */
  mparamf = calcmrate (C[ci]->G[li].mignum, C[ci]->G[li].tlength);

  IMA_reset_edgemiginfo (newe);
  assert(olde->edgeid < L[li].numgenes);
  newe->edgeid = edge = olde->edgeid;
  /* We need li to call checkmig in function simmpath. */
  newe->li = olde->li = li;
  newe->pop = newpop;
  newe->temppop = newpop;
  assert (olde->fpop == gtree[gtree[edge].down].pop);
  newe->fpop = olde->fpop;
  newe->upt = 0.0; /* because we only deal with external branches */
  newe->dnt = gtree[edge].time;
  newe->mig[0].mt = -1;
  fillmiginfoperiods (ci, newe);
  /* newe->mpall = mwork (ci, newe, newe->e, mparamf); TURN ON THIS LATER */
  assert (newe->mtall > 0);    // some of the edge must occur before the last split time

  fprintf (stderr, "relabel should not be called for the moment\n");
  /* mproposedenom = getmprob (ci, mparamf, newe, NULL); TURN ON THIS LATER */
  assert (mproposedenom > -1e200 && mproposedenom < 1e200);

  /* calculate probability of reverse update    */
  mcount = oldmigcount - olde->mpall + newe->mpall;
  
  /* find the migation rate for the backward update */
  /* To reflect Jody's comment: we use overall migration rates of a genealogy.
   * This may help assignment update a little bit. 
   * mparamb = calcmrate (newe->mpall, newe->mtall); */
  mparamb = calcmrate (mcount, C[ci]->G[li].tlength);
  assert (mparamb > 0);

  /* mproposenum = getmprob (ci, mparamb, olde, NULL); TURN ON THIS LATER */
  assert (mproposenum > -1e200 && mproposenum < 1e200);
  weight = mproposenum - mproposedenom;
  *newmigcount = mcount;
  return weight;
}
void
copy_probcalc_all (struct probcalc *dest, struct probcalc *srce)
{
  memcpy (dest->qintegrate, srce->qintegrate,
          numpopsizeparams * sizeof (double));
  if (nummigrateparams > 0)
    {
      memcpy (dest->mintegrate, srce->mintegrate,
              nummigrateparams * sizeof (double));
    }
  dest->pdg = srce->pdg;
  dest->probg = srce->probg;
  return;
}

int updateassignmentrelabellocus (int ci,int li)
{
  ci = li = 0;
  return 0;
}

int
updateassignmentbf (int ci)
{
  int accp;
  double lweight;
  double logU;
  double mweight;
  double hweight;
  struct genealogy *G;
  int li;

  /* I should check this before entering MCMC. For now I add this code for
   * the purpose of comment on the code of this function. */
  if (L[0].model != INFINITESITES)
    {
      IM_errloc (AT, 
                 "BF99 update is implemented only for infinite-sites data");
    }

  if (snind_unknown == 0)
    {
      return 0;
    }

  G = NULL;
  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      assertgenealogy (ci);
    }

  /* Count up the number of tries of MCMC cycles. */
  if (ci == 0)
    {
      for (li = 0; li < nloci; li++)
        {
          L[li].a_rec->upinf[IM_UPDATE_ASSIGNMENT_BF].tries++;
        }
    }
  Cupinf[ci].upinf[IM_UPDATE_ASSIGNMENT_BF].tries++;

  /* Save the current state of a chain */
  for (li = 0; li < nloci; li++)
    {
      imaSavedReset (li);
      imaSavedSaveLocusInfo (ci, li);
      copy_treeinfo (&saT[li].savedgweight, &C[ci]->G[li].gweight);
    }
  copy_treeinfo (&saC.savedallgweight, &C[ci]->allgweight);
  copy_probcalc_all (&saC.savedallpcalc, &C[ci]->allpcalc);

  /* Propose a new state of genealogy and assignment. */
  hweight = bfmove (ci);

  /* Find all internal node sequences and mutations of a full genealogy. */
  /* This is for debugging purposes.                                     */
  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == INFINITESITES
          && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          accp = IMA_genealogy_findIntSeq (ci, li);
          assert (accp == 1);
        }
    }

  /* Compute Metropolis-ratio. */
  setzero_genealogy_weights (&C[ci]->allgweight);
  for (li = 0; li < nloci; li++)
    {
      setzero_genealogy_weights (&C[ci]->G[li].gweight);
      treeweight (ci, li);
      sum_treeinfo (&C[ci]->allgweight, &C[ci]->G[li].gweight);
    }
  initialize_integrate_tree_prob (ci, &C[ci]->allgweight, &C[ci]->allpcalc);

  /* Compute likelihood. */
  /* This has to be called after treeweight. */
  if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
    {
      bflikelihood (ci);
    }
  else
    {
      C[ci]->allpcalc.pdg = 0.0;
    }

  logU = log (uniform ());
  mweight = (C[ci]->allpcalc.probg - saC.savedallpcalc.probg) 
            + gbeta * (C[ci]->allpcalc.pdg - saC.savedallpcalc.pdg);
  lweight = beta[ci] * mweight + hweight;
  assert (beta[ci] * mweight + hweight > -1e200
          && beta[ci] * mweight + hweight < 1e200);

/* check values of acceptance probability: 
  printf ("pdg: %.3lf - %.3lf (%.3lf), pg: %.3lf - %.3lf (%.3lf), hweight: %.3lf, lweight: %.3lf vs. logU: %.3lf, [%d]\n", 
          C[ci]->allpcalc.pdg, saC.savedallpcalc.pdg, C[ci]->allpcalc.pdg - saC.savedallpcalc.pdg,
          C[ci]->allpcalc.probg, saC.savedallpcalc.probg, C[ci]->allpcalc.probg - saC.savedallpcalc.probg,
          hweight, lweight, logU, lweight > logU);
*/
 
  accp = 0;

  /* Find all internal node sequences and mutations of a full genealogy. */
  /* This is for debugging purposes.                                     */
  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == INFINITESITES
          && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          accp = IMA_genealogy_findIntSeq (ci, li);
          assert (accp == 1);
        }
      else if (L[li].model == HKY || L[li].model == STEPWISE)
        {
          /* No code. */
        }
    }

  if (lweight > 0.0 || lweight > logU)
    {
      /* accept the update */
      accp = 1;
      if (ci == 0)
        {
          for (li = 0; li < nloci; li++)
            {
              L[li].a_rec->upinf[IM_UPDATE_ASSIGNMENT_BF].accp++;
            }
        }
      Cupinf[ci].upinf[IM_UPDATE_ASSIGNMENT_BF].accp++;
      C[ci]->nasn[sbfold.pop]--;
      C[ci]->nasn[sbfnew.pop]++;

      /* Always revert to an original. */
      for (li = 0; li < nloci; li++)
        {
          if (L[li].model == HKY)
            {
              copyfraclike (ci, li);
              storescalefactors (ci, li);
            }
        }
    }
  else
    {
      for (li = 0; li < nloci; li++)
        {
          /* restore edges changed during update */
          imaSavedRestore (ci, li); 
          imaSavedRestoreLocusInfo (ci, li);
          copy_treeinfo (&C[ci]->G[li].gweight, &saT[li].savedgweight);
        }
      copy_treeinfo (&C[ci]->allgweight, &saC.savedallgweight);
      copy_probcalc_all (&C[ci]->allpcalc, &saC.savedallpcalc);
    }

  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == HKY)
        {
          restorescalefactors (ci, li);
        }
    }

  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      assertgenealogy (ci);
    }
  return accp;
}

int 
bflikelihood (int ci)
{
  int li;
  int ai;
  struct genealogy *G = NULL;

  assert (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0);

  C[ci]->allpcalc.pdg = 0.0;
  for (li = 0; li < nloci; li++)
    {
      G = &(C[ci]->G[li]);
      switch (L[li].model)
        {
        case HKY:
          if (assignmentoptions[JCMODEL] == 1)
            {
              G->pdg = likelihoodJC (ci, li, G->uvals[0]);
            }
          else
            {
              G->pdg = likelihoodHKY (ci, li, G->uvals[0], G->kappaval, -1, -1, -1, -1);
            }
          G->pdg_a[0] = G->pdg;
          break;
        case INFINITESITES:
          /* We need both pdg and pdb_a to be set. */
          G->pdg = likelihoodIS (ci, li, G->uvals[0]);
          G->pdg_a[0] = G->pdg;
          break;
        case STEPWISE:
          G->pdg = 0.0;
          for (ai = 0; ai < L[li].nlinked; ai++)
          {
            G->pdg_a[ai] = likelihoodSW (ci, li, ai, G->uvals[ai], 1.0);
            G->pdg += G->pdg_a[ai];
          }
          break;
        case JOINT_IS_SW:
          assert (0);
          ai = 0;
          G->pdg_a[ai] = likelihoodIS (ci, li, G->uvals[0]);
          for (ai = 1; ai < L[li].nlinked; ai++)
          {
            /* ui = L[li].uii[ai];       */
            /* assert (pdgnew[ui] == 0); */
            G->pdg_a[ai] = likelihoodSW (ci, li, ai, G->uvals[ai], 1.0);
            G->pdg += G->pdg_a[ai];
          }
          break;
        }
      C[ci]->allpcalc.pdg += G->pdg;
    }

  return 0; 
}

double
bfmove (int ci)
{
  /* iterator variables */
  int li;
  int ei;
  int pj;
  int ii;
  int ij;
  int ej;
  int i;
  int mi;
  int si;
  /* numbers of counts */
  int nei;
  int ngenes;
  int nsum;
  int np;
  int ki;
  int prevki;
  int oldpop;
  int newpop;
  struct genealogy *G;
  struct edge *gtree; 
  int z1;
  int z3;
  int pc1;
  int pc3;
  char is_coalesced1;
  char is_coalesced3;
  im_edgemiginfo *m1;
  im_edgemiginfo *m3;
  double totalrate;
  double rates[2 * MAXPOPS];
  double newct;
  int down1;
  int re;
  double dnt_genealogy;
  int ri;
  int nsim;
  int ti;
  double lhweight;
  im_event *ek;
  im_ginfo *gk;
  im_ginfo *gkz1;
  double prevt;
  double dt;
  double u;
  double rt;
  int nr;
  double lqstar;
  double lq;

  lhweight = 0.0;

  /* Find all internal node sequences and mutations of a full genealogy. */
  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == INFINITESITES 
          && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          i = IMA_genealogy_findIntSeq (ci, li);
          assert (i == 1);
        }
    }

  /* Pick an individual to relabel. */
  ii = snind_known + randposint (snind_unknown);
  nei = sngenes_ind[ii];
  for (ij = 0; ij < nei; ij++)
    {
      li = saC.indlist[ii][ij].li;
      ei = saC.indlist[ii][ij].ei;
      imaSavedSaveBranch (ci, li, ei);
    }

  /* Prepare all migration events storage. */
  for (li = 0; li < nloci; li++)
    {
      IMA_reset_edgemiginfo (&saoems[2 * li]);
      IMA_reset_edgemiginfo (&saoems[2 * li + 1]);
      IMA_reset_edgemiginfo (&saosms[2 * li]);
      IMA_reset_edgemiginfo (&sanems[2 * li]);
      IMA_reset_edgemiginfo (&sanems[2 * li + 1]);
      IMA_reset_edgemiginfo (&sansms[2 * li]);
    }
  for (ij = 0; ij < nei; ij++)
    {
      li = saC.indlist[ii][ij].li;
      ei = saC.indlist[ii][ij].ei;
      ej = saoems[2 * li].edgeid;
      if (ej < 0)
        {
          IMA_edgemiginfo_copymig (&saoems[2 * li], ci, li, ei);
        }
      else
        {
          assert (0);
        }
    }

  /* Label all the genes. */
  li = saC.indlist[ii][0].li;
  ei = saC.indlist[ii][0].ei;
  oldpop = C[ci]->G[li].gtree[ei].pop;
  lhweight = imaRandomPickpopn (&newpop, ci, oldpop, C[ci]->rootpop);
  sbfold.pop = oldpop;
  sbfnew.pop = newpop;
  for (ij = 0; ij < nei; ij++)
    {
      li = saC.indlist[ii][ij].li;
      ei = saC.indlist[ii][ij].ei;
      C[ci]->G[li].gtree[ei].pop = newpop;
    }

  /* Simulate branches for each genealogy. */
  for (li = 0; li < nloci; li++)
    {
      ngenes = L[li].numgenes;
      G = &C[ci]->G[li];
      gtree = G->gtree;

      /* If no genes in locus [[li]] belong to individual [[ii]]. */
      /* Decide old branches. -> wrong words!     */
      /* We do not have a longer branch any more. */
      if (saoems[2 * li].edgeid < 0)
        {
          /* No long branches any more. */
          /* assert (0); We can delete this line but I do not allow partial
          genes of an individual. */
          continue;
        }

      assert (saoems[2 * li].upt == 0.0);
      sbfold.m1 = &saoems[2 * li];
      z1 = sbfold.m1->edgeid;

      m1 = &sanems[2 * li]; /* sbfnew.m1 could be m1. */
      m1->li = li;
      m1->edgeid = z1;
      m1->upt = 0.0;
      m1->pop = newpop;
      dnt_genealogy = imaGtreeTime (ci, li, G->root);

      /* Detach active lineages. */
      sbfold.is_finiterootbranch = 'F'; /* This is the 1st occurrence.(2 more) */
      IMA_genealogy_detach (ci, li, &sbfold, z1);

      /* Get intervals of events of a partial genealogy. */
      assert (!gsl_fcmp(dnt_genealogy, G->roottime, 1e-7));
      IMA_intervals_collect (ci, li, 0.0, G->roottime);

      /* Derive parameters from a partial genealogy. */
      IMA_genealogy_derivetheta ('o', ci, li, z1);
      gk = &saGiold[li];
      gkz1 = &saGioldz1[li];

      nsim = 0;
simulation:
      nsim++;

#ifdef DEBUG
      if (L[li].model != INFINITESITES)
        {
          assert (L[li].model == HKY || L[li].model == STEPWISE);
          assert (nsim == 1);
        }
#endif /* DEBUG */

      /* Prepare the number of lineages with which active lineages can coalesce. */
      IMA_sbf_lineages (&sbfold, ci, li, z1);

      /* Initialize active lineages. */
      m1->dnt = -1.0;
      m1->fpop = -1;
      m1->mpall = 0;
      m1->mig[0].mt = -1.0;
      m1->mig[0].mp = -1;
      sbfnew.sis1 = -1;
      sbfnew.m1 = NULL; /* sbfnew.m1 is the same as &sanems[2 * li]    */
      sbfnew.m3 = NULL; /* but we do not know this is null or not yet. */
      sbfnew.is_finiterootbranch = 'F';
      sbfnew.canz31 = 'F'; /* both canz31 must be the same? */
      sbfold.canz31 = 'F';
      sbfold.m3 = NULL; /* but we do not know this is null or not yet. */
      is_coalesced1 = 'F';

      ti = 0;
      prevt = 0.0;
      ki = 0;
      pc1 = gtree[z1].pop;
      assert (pc1 == newpop);

      /* Start simulation down towards the genealogy. */
      ek = sbfold.e1; 
      while (is_coalesced1 == 'F')
        {
          dt = ek->t - prevt;
          assert (dt > 0.0);
          while (dt > 0.0 && is_coalesced1 == 'F')
            {
              np = bfmove_ratesI (&rates, gk, gkz1,
                                  &sbfold, ki,
                                  pc1);
              totalrate = 0.0;
              for (pj = 0; pj < np; pj++)
                {
                  assert (!(rates[pj] < 0.0));
                  totalrate += rates[pj];
                }
              if (totalrate > 0.0)
                {
                  u = uniform ();
                  rt = -log (u) / totalrate;
                  dt -= rt; 
                }
              else if (totalrate == 0.0)
                {
                  /* assert (0); */
                  /* assert (0); I do not know when this would happen: Answer -
                   * This can happen when there is no lineage to attach an
                   * active lineage in the last period. I am not sure whether
                   * this could continue to happen once we have zero total rate.
                   * If that is the case, we may let this happen and resimulate
                   * once we get to the beyond of the bottom of a partial
                   * genealogy. */
                  dt = -1.0;
                }
              else
                {
                  assert (0);
                }
              if (dt < 0)
                {
                  /* move to next interval: we hit and exit the current while,
                   * then we increase ti and ek */
                  continue;
                }
              else
                {
                  newct = ek->t - dt;
                }
              re = ran_discretea (np, rates);

              /* Case (A). Only one lineage z1 coalesces. */
              if ((np == 1 && is_coalesced1 == 'F')
                  || (np == 2 && is_coalesced1 == 'F' && re == 0))
                {
                  is_coalesced1 = 'T';
                  /* We need a list of edges: n and l.                     */
                  /* We computes discrete probabilities for all the edges. */
                  i = bfmove_chooseAnEdge (ci, li, &sbfold, newct, pc1);
                  if (L[li].model == INFINITESITES)
                    {
                      sbfnew.sis1 = sbfold.lz1[pc1][i];
                    }
                  else if (L[li].model == HKY || L[li].model == STEPWISE)
                    {
                      sbfnew.sis1 = sbfold.l[pc1][i];
                    }
                  else
                    {
                      assert (0);
                    }
                  /* sbfnew.down1 = sbfold.down1; */
                  sbfnew.m1 = m1;
                  m1->mig[m1->mpall].mt = -1.0;
                  m1->dnt = newct;
                  m1->fpop = pc1;
                }
              /* Case (F). Lineage z1 migrates */
              else if (np == 2 && is_coalesced1 == 'F' && re == 1)
                {
                  IMA_choose_migtopop (&ri, ki, pc1, gk);
                  assert (pc1 != ri);
                  pc1 = ri;
                  mi = m1->mpall;
                  m1->mig[mi].mt = newct;
                  m1->mig[mi].mp = pc1;
                  m1->mpall++;
                }
              else
                {
                  assert (0);
                }
            }
          if (is_coalesced1 == 'T')
            {
              break;
            }

          /* New time is further back in time than the bottom of an event. */
          bfmove_nextI (ci, li, 0.0, ek, np, rates, 
                        NULL, &sbfold, /* &sbfnew, */
                        &ki,
                        z1,
                        &is_coalesced1,
                        &pc1);
          prevt = ek->t;
          ti++;
          if (ti < sbfold.n1)
            {
              ek++;
            }
          else
            {
              break;
            }
        }

      /* Simulation beyond the bottom of a genealogy. */
      sbfnew.m3 = NULL;
      if (is_coalesced1 == 'F') 
        {
          assert (sbfold.n1 == ti);
          /* Should we do two different actions for the last event type? */
          assert (sbfold.e1[ti - 1].type == 'c' 
                  || sbfold.e1[ti - 1].type == 'e');
          assert (sbfold.e1[ti - 1].t == prevt);
          assert (findperiodoftime (ci, prevt) == ki);

          is_coalesced3 = 'F';
          newct = prevt;
          pc3 = sbfold.e1[ti - 1].pj;
          assert (!(pc1 < 0));
          assert (!(pc3 < 0));
          assert (sbfold.e1[ti - 1].ei == G->root);
          z3 = G->root;
          m3 = &sansms[2 * li];
          m3->li = li;
          m3->edgeid = z3;
          m3->upt = prevt;
          m3->pop = pc3;

          sbfold.canz31 = 'T';
          if (L[li].model == INFINITESITES
              && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
            {
              for (si = 0; si < L[li].numsites; si++)
                {
                  if (G->mut[si] == z3)
                    {
                      continue;
                    }
                  if (gtree[z3].seq[si] != sbfold.seqz1[si])
                    {
                      sbfold.canz31 = 'F';
                      break;
                    }
                }
            }
          else if (L[li].model == HKY || L[li].model == STEPWISE)
            {
              /* No code: we should attach an active lineage. */
            }

          nsum = IMA_sbf_nsum (&sbfold, pc1, pc3);
          assert (nsum == 2); /* Do not consider diploid data type. */
          if (is_coalesced1 == 'F' && sbfold.canz31 == 'F')
            {
              m1->dnt = -1.0;
              m1->fpop = -1;
              m1->mpall = 0;
              m1->mig[0].mt = -1.0;
              goto simulation;
            }
          assert (sbfold.canz31 == 'T');

          /* ki is set before. */
          while (nsum == 2)
            {
              assert (is_coalesced1 == 'F');
              assert (is_coalesced3 == 'F');
              assert (pc1 < 2 * npops - 1);
              assert (!(pc1 < 0));
              assert (pc3 < 2 * npops - 1);
              assert (!(pc3 < 0));
              pc1 = saC.popndown[pc1][ki];
              pc3 = saC.popndown[pc3][ki];

              bfmove_ratesIII (&rates, gk, &sbfold, ki, pc1, pc3);
              nr = 3;
              totalrate = 0.0;
              for (ri = 0; ri < nr; ri++)
                {
                  totalrate += rates[ri];
                }
              assert (totalrate > 0.0);
              u = uniform ();
              rt = -log (u) / totalrate;
              prevki = ki;
              newct = newct + rt;
              ki = findperiodoftime (ci, newct);
              if (prevki == ki)
                {
                  /* No Code */
                }
              else if (prevki < ki)
                {
                  /* ki can be larger than prevki by more than one. */
                  newct = C[ci]->tvals[prevki];
                  ki = prevki + 1;
                  continue;
                }
              else
                {
                  assert (0);
                }
              re = ran_discretea (nr, rates);
              switch (re)
                {
                case 0:
                  IMA_choose_migtopop (&ri, ki, pc1, gk);
                  pc1 = ri;
                  mi = m1->mpall;
                  m1->mig[mi].mt = newct;
                  m1->mig[mi].mp = pc1;
                  m1->mpall++;
                  break;
                case 1:
                  IMA_choose_migtopop (&ri, ki, pc3, gk);
                  pc3 = ri;
                  mi = m3->mpall;
                  m3->mig[mi].mt = newct;
                  m3->mig[mi].mp = pc3;
                  m3->mpall++;
                  break;
                case 2: /* Lineages z1 coalesces with z3. */
                  is_coalesced3 = 'T';
                  is_coalesced1 = 'T';
                  /* What are all these about? */
                  sbfnew.sis1 = z3;
                  /* sbfnew.down1 = sbfold.down1; */
                  sbfnew.m1 = m1;
                  m1->mig[m1->mpall].mt = -1.0;
                  m1->dnt = newct;
                  m1->fpop = pc1;
                  sbfnew.m3 = m3;
                  m3->mig[m3->mpall].mt = -1.0;
                  m3->dnt = newct;
                  m3->fpop = pc3;
                  assert (pc1 == pc3);
                  bfmove_chooseA1 (ci, li, &sbfnew);
                  nsum = 1;
                  break;
                }
            }
        }

      if (sbfold.is_finiterootbranch == 'F')
        {
          assert (sbfold.m3 == NULL);
        }
      /* Get [[sbfnew]] and [[saGinew]]. */
      IMA_intervals_collectsbfnew (ci, li, sbfnew.m3);
      IMA_genealogy_derivetheta ('n', ci, li, z1);
      if (L[li].model == INFINITESITES)
        {
          /* No code. */
        }
      else if (L[li].model == HKY || L[li].model == STEPWISE)
        {
          /* FIXME: What if there is no or single inactive lineage? */
          bfmove_selectAnEdge (ci, li, &sbfnew, &sbfold);
          /* bfmove_selectA1 (ci, li, &sbfold); */
        }

      /* Compute Hasting ratio. */
      /* G     -> Gp     -> Gstar */
      lqstar = bfq (&sbfold, &sbfnew, ci, li, gk, gkz1);
      /* Gstar -> Gstarp -> G     */
      lq = bfq (&sbfnew, &sbfold, ci, li, &saGinew[li], &saGinewz1[li]);
      lhweight += (lq - lqstar);

      /* Reattach the branch or branches. */
      down1 = gtree[sbfnew.m1->edgeid].down;
      if (sbfnew.m3 != NULL)
        {
          assert (sbfnew.sis1 == G->root);
          IMA_genealogy_appendmig (ci, li, 
                                   sbfnew.sis1, sbfnew.m3, 
                                   sbfnew.m1->edgeid, down1);
        }
      else
        {
          IMA_genealogy_bisectsis (ci, li, 
                                   sbfnew.sis1, sbfnew.m1->edgeid, 
                                   down1, 
                                   sbfnew.m1, 
                                   sbfnew.seqz1);
        }
      IMA_genealogy_copymig (ci, li, sbfnew.m1->edgeid, sbfnew.m1);
      gtree[G->root].time = TIMEMAX;
      gtree[G->root].down = -1;
      gtree[G->root].mig[0].mt = -1.0;

      /* Update locus information. */ 
      gtree[z1].exist = 'T';
      down1 = gtree[z1].down;
      gtree[down1].exist = 'T';
      G->roottime = imaGtreeTime (ci, li, G->root); 
      sbfold.n1 = 0;
      sbfold.n2 = 0;
      sbfold.n3 = 0;
      sbfnew.n1 = 0;
      sbfnew.n2 = 0;
      sbfnew.n3 = 0;

      if (L[li].model == INFINITESITES 
          && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          i = IMA_genealogy_findIntSeq (ci, li);
          assert (i == 1);
        }
      else if (L[li].model == HKY) 
        {
          /* No code. */
        }
      else if (L[li].model == STEPWISE)
        {
          /* No code. */
          assert (sbfnew.A1 >= L[li].minA[0] && sbfnew.A1 <= L[li].maxA[0]);
          gtree[down1].A[0] = sbfnew.A1;
        }
      /* printf ("[%d] root time - %lf\n", step, G->roottime);  */
    }

  return lhweight;
}

void
bfmove_chooseA1 (int ci, int li, im_bfupdate *pbf)
{
  double u;
  struct genealogy *G;
  struct edge *gtree; 
  int nA;
  int bi;
  int newA;
  double dlikeA;
  double sisupt;
  int sis;
  int edge;
  double ct;
  int vj;
  double sisnewtlen;
  int d;
  double bessiv;

  if (L[li].model == STEPWISE)
    {
      G = &C[ci]->G[li];
      gtree = G->gtree;

      /**********************************************************/
      /*                  SOMEWHAT COMPLICATED                  */
      /**********************************************************/
      /* All unlinked loci! */
      edge = pbf->m1->edgeid;
      sis = pbf->m3->edgeid;
      sisupt = imaGtreeTime (ci, li, sis);
      sisnewtlen = pbf->m3->dnt - sisupt;
      ct = pbf->m1->dnt;
      
      u = G->uvals[0];
      nA = L[li].maxA[0] - L[li].minA[0] + 1;
      for (bi = 0; bi < nA; bi++)
        {
          newA = L[li].minA[0] + bi;
          dlikeA = 0.0; /* FIXME: */

          d = gtree[sis].A[0] - newA;
          bessiv = bessi (d, sisnewtlen * u);
          assert (bessiv > 0.0);
          dlikeA += (-(sisnewtlen * u) + log (bessiv));

          d = gtree[edge].A[0] - newA;
          bessiv = bessi (d, ct * u);
          assert (bessiv > 0.0);
          dlikeA += (-(ct * u) + log (bessiv));

          /* A rooting edge does not have a likelihood under SW model. */

          pbf->likes11[0][bi] = dlikeA;
        }
      normalizeLoga (nA, pbf->likes11[0]);
      vj = ran_discreteloga (nA, pbf->likes11[0]);
      pbf->likei11 = vj;
      pbf->A1 = L[li].minA[0] + vj;
      pbf->likei1 = 0;
      pbf->likes1[0] = 0.0;
      /**********************************************************/
      /*                  SOMEWHAT COMPLICATED                  */
      /**********************************************************/
    }
  return;
}

double
bfmove_selectA1 (int ci, int li, im_bfupdate *pbf, double lq)
{
  double u;
  struct genealogy *G;
  struct edge *gtree; 
  int nA;
  int bi;
  int newA;
  double dlikeA;
  double sisupt;
  int sis;
  int edge;
  double ct;
  double sisnewtlen;
  int d;
  double bessiv;

  if (L[li].model == STEPWISE)
    {
      assert (pbf->m3 != NULL);

      G = &C[ci]->G[li];
      gtree = G->gtree;

      /**********************************************************/
      /*                  SOMEWHAT COMPLICATED                  */
      /**********************************************************/
      /* All unlinked loci! */
      edge = pbf->m1->edgeid;
      sis = pbf->m3->edgeid;
      sisupt = imaGtreeTime (ci, li, sis);
      sisnewtlen = pbf->m3->dnt - sisupt;
      ct = pbf->m1->dnt;
      
      u = G->uvals[0];
      nA = L[li].maxA[0] - L[li].minA[0] + 1;
      for (bi = 0; bi < nA; bi++)
        {
          newA = L[li].minA[0] + bi;
          dlikeA = 0.0; /* FIXME: */

          d = gtree[sis].A[0] - newA;
          bessiv = bessi (d, sisnewtlen * u);
          assert (bessiv > 0.0);
          dlikeA += (-(sisnewtlen * u) + log (bessiv));

          d = gtree[edge].A[0] - newA;
          bessiv = bessi (d, ct * u);
          assert (bessiv > 0.0);
          dlikeA += (-(ct * u) + log (bessiv));

          /* A rooting edge does not have a likelihood under SW model. */
          pbf->likes11[0][bi] = dlikeA;

          if (pbf->A1 == L[li].minA[0] + bi)
            {
              pbf->likei11 = bi;
            }
        }
      normalizeLoga (nA, pbf->likes11[0]);
      lq += pbf->likes11[0][pbf->likei11];
      assert (pbf->A1 == L[li].minA[0] + pbf->likei11);
      /**********************************************************/
      /*                  SOMEWHAT COMPLICATED                  */
      /**********************************************************/
    }
  return lq;
}

int 
bfmove_chooseAnEdge (int ci, int li, im_bfupdate *pbf, double ct, int pc)
{
  int i;
  int vi;
  int sis;
  struct genealogy *G;
  struct edge *gtree; 
  int edge;
  int down;
  int downdown;
  int up;
  double sisupt;
  double sisdnt;
  double sisoldtl;
  double sisnewtl;
  double downtl;
  double u;
  int nA;
  int bi;
  int newA;
  double dlikeA;
  int d;
  double bessiv;
  int iszero;
  int vj;

  G = &C[ci]->G[li];
  gtree = G->gtree;
  edge = pbf->m1->edgeid;
  down = gtree[edge].down;

  if (L[li].model == INFINITESITES)
    {
      assert (pbf->nz1[pc] > 0);
      vi = randposint (pbf->nz1[pc]);
    }
  else if (L[li].model == HKY || L[li].model == STEPWISE)
    {
      assert (pbf->n[pc] > 0);
#ifdef NOTCONSIDERINGDATA
      vi = randposint (pbf->n[pc]);
      break;
#endif /* NOTCONSIDERINGDATA */
      if (pbf->n[pc] > 0)
        {
          /* Compute all likelihoods for possible full genealogies. */
          
          /* We attach an active lineage to one of lineages. Each of the lineages is
           * a candidate for lineage [[sis]]. We freely attach it, 
           * compute likelihood, and detach it. We keep doing this until we fill all
           * likelihoods for all full possible genealogies. */
          for (i = 0; i < pbf->n[pc]; i++)
            {
              sis = pbf->l[pc][i]; /* FIXME: l is not set yet. */
              /* Attach it. */ 
              /* Attaching may be done using a function. */
              downdown = gtree[sis].down;
              sisupt = imaGtreeTime (ci, li, sis);
              assert (sisupt < ct); 

              sisdnt = gtree[sis].time;
              if (G->root == sis)
                {
                  assert (pbf->n[pc] == 1);
                  /* sisdnt is not ... */
                  /* sisdnt = gtree[sis].time; */
                }
              else
                {
                  assert (ct < sisdnt);
                }

              if (G->root == sis)
                {
                  assert (downdown == -1);
                  G->root = down;
                  G->roottime = ct;
                }
              else
                {
                  if (gtree[downdown].up[IM_EDGE_LEFT] == sis)
                    {
                      gtree[downdown].up[IM_EDGE_LEFT] = down;
                    }
                  else
                    {
                      gtree[downdown].up[IM_EDGE_RIGHT] = down;
                    }
                }
              gtree[down].down = downdown;
              gtree[sis].down = down;
              gtree[edge].down = down;
              gtree[down].up[IM_EDGE_LEFT] = sis;
              gtree[down].up[IM_EDGE_RIGHT] = edge;
              gtree[edge].time = ct;
              gtree[sis].time = ct;
              gtree[down].time = sisdnt;
              sisoldtl = sisdnt - sisupt;
              sisnewtl = ct - sisupt;
              downtl = sisdnt - ct;

              /* Compute likelihood. */
              if (L[li].model == HKY)
                {
                  pbf->likes1[i] = likelihoodHKY (ci, li, 
                                                  G->uvals[0], G->kappaval, 
                                                  -1, -1, -1, -1);
                }
              else if (L[li].model == STEPWISE)
                {
                  /**********************************************************/
                  /*                  SOMEWHAT COMPLICATED                  */
                  /**********************************************************/
                  /* All unlinked loci! */
                  u = G->uvals[0];
                  pbf->likes1[i] = 0.0;
                  nA = L[li].maxA[0] - L[li].minA[0] + 1;
                  for (bi = 0; bi < nA; bi++)
                    {
                      newA = L[li].minA[0] + bi;
                      dlikeA = 0.0; /* FIXME: */
                      if (G->root != down)
                        {
                          dlikeA -= gtree[sis].dlikeA[0];
                        }
                      else
                        {
                          assert (downdown == -1);
                        }

                      d = gtree[sis].A[0] - newA;
                      bessiv = bessi (d, sisnewtl * u);
                      if (!(bessiv > 0.0))
                        {
                          iszero = 1;
                          assert (0);
                          /* Impossible situation: IM_BESSI_MIN; */
                        }
                      else
                        {
                          dlikeA += (-(sisnewtl * u) + log (bessiv));
                        }
                      d = gtree[edge].A[0] - newA;
                      bessiv = bessi (d, ct * u);
                      if (!(bessiv > 0.0))
                        {
                          iszero = 1;
                          assert (0);
                          /* Impossible situation: IM_BESSI_MIN; */
                        }
                      else
                        {
                          dlikeA += (-(ct * u) + log (bessiv));
                        }
 
                      if (!(downdown < 0))
                        {
                          d = newA - gtree[downdown].A[0];
                          bessiv = bessi (d, downtl * u);
                          if (!(bessiv > 0.0))
                            {
                              iszero = 1;
                              assert (0);
                              /* Impossible situation: IM_BESSI_MIN; */
                            }
                          else
                            {
                              dlikeA += (-(downtl * u) + log (bessiv));
                            }
                        }
                      pbf->likes11[i][bi] = dlikeA;
                      if (bi == 0)
                        {
                          pbf->likes1[i] = dlikeA;
                        }
                      else
                        {
                          LogSum2 (pbf->likes1[i], pbf->likes1[i], dlikeA);
                        }
                    }
                  /**********************************************************/
                  /*                  SOMEWHAT COMPLICATED                  */
                  /**********************************************************/
                }

              /* Detach it. */
              up = sis;
              if (G->root == down)
                {
                  assert (downdown == -1);
                  gtree[up].down = -1;
                  G->root = up;
                  gtree[up].time = sisdnt;
                }
              else
                {
                  assert (!(downdown < 0));
                  gtree[up].down = downdown;
                  if (gtree[downdown].up[IM_EDGE_LEFT] == down)
                    {
                      gtree[downdown].up[IM_EDGE_LEFT] = up;
                    }
                  else
                    {
                      gtree[downdown].up[IM_EDGE_RIGHT] = up;
                    }
                  gtree[up].time = gtree[down].time;
                }
            }

          if (pbf->n[pc] > 1)
            {
              normalizeLoga (pbf->n[pc], pbf->likes1);
              vi = ran_discreteloga (pbf->n[pc], pbf->likes1);
              pbf->likei1 = vi;
            }
          else if (pbf->n[pc] == 1)
            {
              vi = 0;
              pbf->likes1[0] = 0.0;
              pbf->likei1 = vi;
            }
          else
            {
              assert (0);
            }

          if (L[li].model == STEPWISE)
            {
              nA = L[li].maxA[0] - L[li].minA[0] + 1;
              normalizeLoga (nA, pbf->likes11[vi]);
              vj = ran_discreteloga (nA, pbf->likes11[vi]);
              pbf->likei11 = vj;
            }
        }
      else if (pbf->n[pc] == 1)
        {
          assert (0); /* Never be called. */
          vi = 0;
          pbf->likes1[0] = 0.0;
          pbf->likei1 = vi;
  
          /* FIXME: likes11[0] is the array that we need to fill. */
          if (L[li].model == STEPWISE)
            {
              nA = L[li].maxA[0] - L[li].minA[0] + 1;
              normalizeLoga (nA, pbf->likes11[vi]);
              vj = ran_discreteloga (nA, pbf->likes11[vi]);
              pbf->likei11 = vj;
            }
        }
      else
        {
          assert (0);
        }
    }
  else
    {
      assert (0);
    }

  return vi;
}

int 
bfmove_selectAnEdge (int ci, int li, im_bfupdate *pbf, im_bfupdate *qbf)
{
  /* pbf: sbfnew, qbf: sbfold */
  int pc;
  double ct;
  int ki;
  int mi;
  int pop;
  int i;
  int n;
  int vi;
  int sis;
  struct genealogy *G;
  struct edge *gtree; 
  int edge;
  int down;
  int downdown;
  int up;
  double sisupt;
  double sisdnt;
  double upt;
  double dnt;
  int ngnodes;
  int ngenes;
  char iscross;
  int ei;
  int bi;
  int nA;
  double dlikeA;
  int d;
  double u;
  int newA;
  double sisoldtl;
  double sisnewtl;
  double downtl;
  double bessiv;
  int iszero;

  G = &C[ci]->G[li];
  gtree = G->gtree;
  edge = pbf->m1->edgeid;
  down = gtree[edge].down;
  ct = qbf->m1->dnt;
  ki = findperiodoftime (ci, ct);
  pc = qbf->m1->fpop;

  ngnodes = L[li].numlines;
  ngenes = L[li].numgenes;

  /* Find all edges that are crossed by time [[ct]]. Exclude edges that are not
   * a part of a partial genealogy. */
  pbf->n[pc] = 0;
  if (qbf->is_finiterootbranch == 'T')
    {
      pbf->n[pc] = 1;
      pbf->l[pc][0] = G->root;
    }
  else
    {
      for (ei = 0; ei < ngnodes; ei++)
        {
          /* Exclude a gene at an active lineage. */
          if (gtree[ei].exist == 'F')
            {
              continue; 
            }
          assert (!(gtree[ei].pop < 0));
          assert (gtree[ei].pop < 2 * MAXPOPS - 1);
          if (ei < ngenes)
            {
              upt = 0.0;
            }
          else
            {
              up = gtree[ei].up[0];
              upt = gtree[up].time;
            }
          dnt = gtree[ei].time;
          iscross = 'F';
          if (upt < ct && ct < dnt)
            {
              pop = gtree[ei].pop;
              if (gtree[ei].mig[0].mt < 0.0)
                {
                  pop = saC.popndown[pop][ki];
                  if (pop == pc)
                    {
                      iscross = 'T';
                    }
                }
              else
                {
                  mi = 0;
                  while (gtree[ei].mig[mi].mt > -0.5)
                    {
                      dnt = gtree[ei].mig[mi].mt;
                      if (upt < ct && ct < dnt)
                        {
                          pop = saC.popndown[pop][ki];
                          if (pop == pc)
                            {
                              iscross = 'T';
                            }
                          break;
                        }
                      upt = dnt;
                      pop = gtree[ei].mig[mi].mp;
                      mi++;
                    }
                  if (iscross == 'F')
                    {
                      dnt = gtree[ei].time;
                      if (upt < ct && ct < dnt)
                        {
                          pop = saC.popndown[pop][ki];
                          if (pop == pc)
                            {
                              iscross = 'T';
                            }
                        }
                    }
                }
            }
          if (iscross == 'T')
            {
              n = pbf->n[pc];
              pbf->l[pc][n] = ei;
              pbf->n[pc]++;
            }
        }
    }

  if (L[li].model == INFINITESITES)
    {
      assert (0);
    }
  else if (L[li].model == HKY || L[li].model == STEPWISE)
    {
      assert (pbf->n[pc] > 0);
#ifdef NOTCONSIDERINGDATA
      vi = randposint (pbf->n[pc]);
      break;
#endif /* NOTCONSIDERINGDATA */
      if (pbf->n[pc] > 0)
        {
          /* Compute all likelihoods for possible full genealogies. */
          
          /* We attach an active lineage to one of lineages. Each of the lineages is
           * a candidate for lineage [[sis]]. We freely attach it, 
           * compute likelihood, and detach it. We keep doing this until we fill all
           * likelihoods for all full possible genealogies. */
          for (i = 0; i < pbf->n[pc]; i++)
            {
              sis = pbf->l[pc][i]; /* FIXME: l is not set yet. */
              /* Attach it. */ 
              /* Attaching may be done using a function. */
              downdown = gtree[sis].down;
              sisupt = imaGtreeTime (ci, li, sis);
              assert (sisupt < ct); 

              sisdnt = gtree[sis].time;
              if (G->root == sis)
                {
                  assert (pbf->n[pc] == 1);
                  /* sisdnt is not ... */
                  /* sisdnt = gtree[sis].time; */
                }
              else
                {
                  assert (ct < sisdnt);
                }

              if (G->root == sis)
                {
                  assert (downdown == -1);
                  G->root = down;
                  G->roottime = ct;
                }
              else
                {
                  if (gtree[downdown].up[IM_EDGE_LEFT] == sis)
                    {
                      gtree[downdown].up[IM_EDGE_LEFT] = down;
                    }
                  else
                    {
                      gtree[downdown].up[IM_EDGE_RIGHT] = down;
                    }
                }
              gtree[down].down = downdown;
              gtree[sis].down = down;
              gtree[edge].down = down;
              gtree[down].up[IM_EDGE_LEFT] = sis;
              gtree[down].up[IM_EDGE_RIGHT] = edge;
              gtree[edge].time = ct;
              gtree[sis].time = ct;
              gtree[down].time = sisdnt;
              sisoldtl = sisdnt - sisupt;
              sisnewtl = ct - sisupt;
              downtl = sisdnt - ct;

              /* Compute likelihood. */
              if (L[li].model == HKY)
                {
                  pbf->likes1[i] = likelihoodHKY (ci, li, 
                                                  G->uvals[0], G->kappaval, 
                                                  -1, -1, -1, -1);
                }
              else if (L[li].model == STEPWISE)
                {
                  /**********************************************************/
                  /*                  SOMEWHAT COMPLICATED                  */
                  /**********************************************************/
                  /* All unlinked loci! */
                  u = G->uvals[0];
                  pbf->likes1[i] = 0.0;
                  nA = L[li].maxA[0] - L[li].minA[0] + 1;
                  for (bi = 0; bi < nA; bi++)
                    {
                      newA = L[li].minA[0] + bi;
                      dlikeA = 0.0; /* FIXME: */
                      if (G->root != down)
                        {
                          dlikeA -= gtree[sis].dlikeA[0];
                        }
                      else
                        {
                          assert (downdown == -1);
                        }

                      d = gtree[sis].A[0] - newA;
                      bessiv = bessi (d, sisnewtl * u);
                      if (!(bessiv > 0.0))
                        {
                          iszero = 1;
                          assert (0);
                          /* Impossible situation: IM_BESSI_MIN; */
                        }
                      else
                        {
                          dlikeA += (-(sisnewtl * u) + log (bessiv));
                        }
                      d = gtree[edge].A[0] - newA;
                      bessiv = bessi (d, ct * u);
                      if (!(bessiv > 0.0))
                        {
                          iszero = 1;
                          assert (0);
                          /* Impossible situation: IM_BESSI_MIN; */
                        }
                      else
                        {
                          dlikeA += (-(ct * u) + log (bessiv));
                        }
 
                      if (!(downdown < 0))
                        {
                          d = newA - gtree[downdown].A[0];
                          bessiv = bessi (d, downtl * u);
                          if (!(bessiv > 0.0))
                            {
                              iszero = 1;
                              assert (0);
                              /* Impossible situation: IM_BESSI_MIN; */
                            }
                          else
                            {
                              dlikeA += (-(downtl * u) + log (bessiv));
                            }
                        }
                      pbf->likes11[i][bi] = dlikeA;
                      if (bi == 0)
                        {
                          pbf->likes1[i] = dlikeA;
                        }
                      else
                        {
                          LogSum2 (pbf->likes1[i], pbf->likes1[i], dlikeA);
                        }
                    }
                  /**********************************************************/
                  /*                  SOMEWHAT COMPLICATED                  */
                  /**********************************************************/
                }

              if (sis == qbf->sis1) /* qbf: sbfold, where is sbfold.sis1 set? */
                {
                  /* IMA_genealogy_detach (ci, li, &sbfold, z1): sets sis1. */
                  pbf->likei1 = i;
                  if (L[li].model == STEPWISE)
                    {
                      pbf->likei11 = gtree[down].A[0] - L[li].minA[0];
                    }
                }

              /* Detach it. */
              up = sis;
              if (G->root == down)
                {
                  assert (downdown == -1);
                  gtree[up].down = -1;
                  G->root = up;
                  gtree[up].time = sisdnt;
                }
              else
                {
                  assert (!(downdown < 0));
                  gtree[up].down = downdown;
                  if (gtree[downdown].up[IM_EDGE_LEFT] == down)
                    {
                      gtree[downdown].up[IM_EDGE_LEFT] = up;
                    }
                  else
                    {
                      gtree[downdown].up[IM_EDGE_RIGHT] = up;
                    }
                  gtree[up].time = gtree[down].time;
                }
            }

          /* We need to set likei11. */
          /* vi = ran_discreteloga (pbf->n[pc], pbf->likes1); */

                  if (pbf->n[pc] > 1)
                    {
                          normalizeLoga (pbf->n[pc], pbf->likes1);
                    }
          else if (pbf->n[pc] == 1)
                    {
              pbf->likes1[0] = 0.0;
              pbf->likei1 = 0;
            }
          else
            {
              assert (0);
            }

          if (L[li].model == STEPWISE)
                    {
              nA = L[li].maxA[0] - L[li].minA[0] + 1;
              assert (!(pbf->likei11 < 0));
              normalizeLoga (nA, pbf->likes11[pbf->likei1]);
            }
        }
      else if (pbf->n[pc] == 1)
        {
          assert (0);
          pbf->likes1[0] = 0.0;
          pbf->likei1 = 0;
        }
    }
  else
    {
      assert (0);
    }

  return vi;
}


double 
bfmove_nextI (int ci, int li, 
              double lq, im_event *ek, 
              int np, double rates[], 
              im_ginfo *gk, 
              im_bfupdate *pbf,
              int *ki,
              int z1,
              char *is_coalesced1,
              int *pc1)
{
  int pi;
  int pj;
  int pk;
  int pl;
  int edge;
  int n1;
  int n2;
  struct genealogy *G;
  struct edge *gtree; 
  
  G = &C[ci]->G[li];
  gtree = G->gtree;

  switch (ek->type)
    {
    case 'a': /* Active lineage coalesces. */
      assert (np == 1 || np == 2);
      assert (rates[0] > 0.0);
      assert (ek->ei == z1);
      assert (*is_coalesced1 == 'F');
      lq += log (rates[0]);
      if (L[li].model == INFINITESITES)
        {
          assert (pbf->nz1[*pc1] > 0);
          lq -= log ((double) pbf->nz1[*pc1]);
        }
      else if (L[li].model == HKY)
        {
          assert (pbf->n[*pc1] > 0);
          lq += pbf->likes1[pbf->likei1]; /* This has been normalized. */
        }
      else if (L[li].model == STEPWISE)
        {
          assert (pbf->n[*pc1] > 0);
          lq += pbf->likes1[pbf->likei1]; /* This has been normalized. */
          lq += pbf->likes11[pbf->likei1][pbf->likei11]; /* This has been normalized. */
        }
      else
        {
          assert (0);
        }
      *is_coalesced1 = 'T';
      break;
    case 'p': /* Active lineage [[z1]] migrates.  */
      pj = ek->pj;
      assert (ek->ei == z1);
      assert (gk->mms[*pc1][pj] > 0.0);
      lq += log (gk->mms[*pc1][pj]);
      *pc1 = pj;
      break;
    case 'e':
      assert (pbf->is_finiterootbranch == 'T');
      break;
    case 'c':
      pi = ek->pj;
      edge = ek->ei;
      assert (*is_coalesced1 == 'F');
      if (*is_coalesced1 == 'F')
        {
          if (L[li].model == INFINITESITES)
            {
              /* NOTE: We do not update sbfold.l!!!           */
              /* We could have called IMA_sbf_c with n and l. */
              pbf->n[pi]--;
              if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
                {
                  IMA_sbf_c (ci, li, edge, pi, pbf->seqz1,
                             pbf->nz1, pbf->lz1, pbf->a);
                }
              else
                {
                  IMA_sbf_c (ci, li, edge, pi, NULL,
                             pbf->nz1, pbf->lz1, pbf->a);
                }
            }
          else if (L[li].model == HKY || L[li].model == STEPWISE)
            {
              IMA_sbf_c (ci, li, edge, pi, NULL, 
                         pbf->n, pbf->l, pbf->a);
            }
        }
      break;
    case 'm':
      edge = ek->ei;
      pi = ek->pi;
      pj = ek->pj;
      assert (*is_coalesced1 == 'F');
      if (L[li].model == INFINITESITES)
        {
          pbf->n[pi]--;
          pbf->n[pj]++;
          IMA_sbf_m (edge, pi, pj, 
                     pbf->nz1, pbf->lz1, pbf->a);
        }
      else if (L[li].model == HKY || L[li].model == STEPWISE)
        {
          IMA_sbf_m (edge, pi, pj, 
                     pbf->n, pbf->l, pbf->a);
        }
      break;
    case 's':
      assert (ek->p == *ki);
      (*ki)++;
      pj = C[ci]->droppops[*ki][IM_EDGE_LEFT];
      pk = C[ci]->droppops[*ki][IM_EDGE_RIGHT];
      pl = C[ci]->addpop[*ki];
      pbf->n[pl] = pbf->n[pj] + pbf->n[pk];
      n1 = pbf->n[pj];
      n2 = pbf->n[pk];
      if (n1 > 0)
        {
          memcpy (pbf->l[pl], pbf->l[pj], n1 * sizeof (int));
        }
      if (n2 > 0)
        {
          memcpy (&pbf->l[pl][n1], pbf->l[pk], n2 * sizeof (int));
        }
      pbf->n[pj] = 0;
      pbf->n[pk] = 0;
      assert (*is_coalesced1 == 'F');
      *pc1 = saC.popndown[*pc1][*ki];
      assert (L[li].model == INFINITESITES 
              || L[li].model == HKY
              || L[li].model == STEPWISE);
      /* Change the number and sets as well. */
      n1 = pbf->nz1[pj];
      n2 = pbf->nz1[pk];
      pbf->nz1[pl] = n1 + n2;
      if (n1 > 0)
        {
          memcpy (pbf->lz1[pl], pbf->lz1[pj], n1 * sizeof (int));
        }
      if (n2 > 0)
        {
          memcpy (&pbf->lz1[pl][n1], pbf->lz1[pk], n2 * sizeof (int));
        }
      pbf->nz1[pj] = 0;
      pbf->nz1[pk] = 0;
      break;
    }
  return lq;
}
int 
bfmove_ratesI (double (*rates)[2 * MAXPOPS], 
               im_ginfo *gk, 
               im_ginfo *gkz1, 
               im_bfupdate *pbf,
               int ki,
               int pc1)
{
  int np;
  int li;

  np = 0;
  li = pbf->m1->li;

  if (L[li].model == INFINITESITES)
    {
      if (gkz1->thetas[pc1] == 0.0)
        {
          assert (pbf->nz1[pc1] == 0);
          (*rates)[np++] = 0.0;
        }
      else
        {
          assert (gkz1->thetas[pc1] > 0.0);
          (*rates)[np++] = 2 * pbf->nz1[pc1] / gkz1->thetas[pc1];
        }
    }
  else if (L[li].model == HKY || L[li].model == STEPWISE)
    {
      if (gk->thetas[pc1] == 0.0)
        {
          assert (pbf->n[pc1] == 0);
          (*rates)[np++] = 0.0;
        }
      else
        {
          assert (gk->thetas[pc1] > 0.0);
          (*rates)[np++] = 2 * pbf->n[pc1] / gk->thetas[pc1];
        }
    }
  else
    {
      assert (0);
    }
  if (ki < lastperiodnumber)
    {
      assert (gk->ms[ki][pc1] > 0.0);
      (*rates)[np++] = gk->ms[ki][pc1];
    }

  return np;
}

void
bfmove_ratesIII (double (*rates)[2 * MAXPOPS], im_ginfo *gk, im_bfupdate *pbf,
                 int ki,
                 int pc1, int pc3)
{
  if (ki < lastperiodnumber)
    {
      (*rates)[0] = gk->ms[ki][pc1];
      (*rates)[1] = gk->ms[ki][pc3];
    }
  else
    {
      (*rates)[0] = 0.0;
      (*rates)[1] = 0.0;
    }
  (*rates)[2] = 0.0; 
  assert (pbf->canz31 == 'T');
  if (pc1 == pc3) 
    {
      if (gk->thetas[pc3] == 0.0)
        {
          /* FIXME: is this a rightful max? */
          (*rates)[2] = 2 * (2 - 1) / itheta[pc3].pr.max; 
        }
      else
        {
          (*rates)[2] = 2 * (2 - 1) / gk->thetas[pc3];
        }
    }
  return; 
}
void 
IMA_genealogy_detach (int ci, int li, im_bfupdate *pbf,
                      int z1)
{
  struct genealogy *G;
  struct edge *gtree; 
  double dnt_genealogy;
  int si;
  int down1;
  int downdown1;
  double sisupt;

  /* For SW model
  double bessiv;
  int d;
  double sislen;
  */

  
  G = &C[ci]->G[li];
  gtree = G->gtree;

  pbf->is_finiterootbranch = 'F'; /* This is the 2nd occurrence. (1 more) */
  dnt_genealogy = imaGtreeTime (ci, li, G->root);
  assert (!gsl_fcmp(dnt_genealogy, G->roottime, 1e-7));
  assert (pbf->m1->edgeid == z1);
  down1 = gtree[z1].down;

  
  if (L[li].model == INFINITESITES) /* FIXME: WHAT ABOUT NO DATA RUN? */
    {
      memcpy (pbf->seqz1, gtree[down1].seq, L[li].numsites * sizeof (int));
    }
  else if (L[li].model == HKY || L[li].model == STEPWISE)
    {
      /* No code. */
    }
  else
    {
      assert (0);
    }

  pbf->sis1 = imaGtreeSister (ci, li, z1);
  assert (!(pbf->sis1 < 0));

  sisupt = imaGtreeTime (ci, li, pbf->sis1);

  downdown1 = gtree[down1].down;
  imaSavedSaveBranch (ci, li, z1);
  imaSavedSaveBranch (ci, li, pbf->sis1);
  imaSavedSaveBranch (ci, li, down1);
  if (!(downdown1 < 0))
    {
      imaSavedSaveBranch (ci, li, downdown1);
    }
  pbf->is_finiterootbranch = 'F'; /* This is the 3rd and last occurrence. */
  if (downdown1 == -1)
    {
      assert (G->root == down1);
      G->root = pbf->sis1;
      gtree[pbf->sis1].down = -1;
      pbf->is_finiterootbranch = 'T';
      /**************************************************/
      /* We keep gtree[sis1].time                       */
      /* We keep migration events along the sister.     */
      /* We keep coalescent event at the original root  */
      /* by naming it ``end.''                          */
      /* We do not change roottime.                     */
      /**************************************************/
    }
  else
    {
      IMA_genealogy_join (gtree, pbf->sis1, down1, downdown1);
    }

  if (L[li].model == INFINITESITES)
    {
      /* Move mutations on down branch to sister. */
      if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          for (si = 0; si < L[li].numsites; si++)
            {
              if (G->mut[si] == down1)
                {
                  G->mut[si] = pbf->sis1;
                }
            }
        }
    }
  else if (L[li].model == HKY)
    {
      /* No code. */
    }
  else if (L[li].model == STEPWISE)
    {
      pbf->A1 = gtree[down1].A[0];
/* 
      if (downdown1 == -1)
            {
                }
          else
            {
                  d = gtree[pbf->sis1].A[0] - gtree[downdown1].A[0];
                  sislen = gtree[pbf->sis1].time - sisupt;
                  bessiv = bessi (d, sislen * G->uvals[0]);
                  if (!(bessiv > 0.0))
                        {
                          assert (0); 
                        }
                  else
                        {
                      gtree[pbf->sis1].dlikeA[0] = (-(sislen * G->uvals[0]) + log (bessiv));
                        }
                }
*/
    }
  else
    {
      assert (0);
    }

  gtree[z1].exist = 'F';
  gtree[down1].exist = 'F';

  return;
}
int
IMA_genealogy_findIntSeq (int ci, int li)
{
  int si;
  int ei;
  int edge;
  int sis;
  int down;
  int ledge;
  int redge;
  int nmut;
  int ngenes;
  int ngnodes;
  char flag;
  struct genealogy *G;
  struct edge *gtree; 
  int accp;

  assert (L[li].model == INFINITESITES);
  G = &C[ci]->G[li];
  gtree = G->gtree;
  ngenes = L[li].numgenes;
  ngnodes = L[li].numlines;
  /* Reset seq and mut of a full genealogy */
  for (si = 0; si < L[li].numsites; si++)
    {
      for (ei = ngenes; ei < ngnodes; ei++)
        {
          gtree[ei].seq[si] = -1; 
        }
      G->mut[si] = -1;
    }
  for (si = 0; si < L[li].numsites; si++)
    {
      for (ei = 0; ei < ngenes; ei++)
        {
          edge = ei;
          flag = 'T';
          while (flag == 'T')
            {
              flag = 'F';
              down = gtree[edge].down;
              if (down == -1)
                {
                  assert (G->root == edge);
                  break;
                }
              if ((sis = gtree[down].up[IM_EDGE_LEFT]) == edge)
                {
                  sis = gtree[down].up[IM_EDGE_RIGHT];
                }
              assert (gtree[edge].seq[si] == 0
                      || gtree[edge].seq[si] == 1
                      || gtree[edge].seq[si] == 2);
              assert (!(gtree[down].seq[si] == -1 && gtree[edge].seq[si] == -1));
              if (gtree[down].seq[si] != gtree[edge].seq[si])
                {
                  if (gtree[edge].seq[si] == 2)
                    {
                      if (gtree[sis].seq[si] != -1)
                        {
                          gtree[down].seq[si] = gtree[sis].seq[si];
                        }
                      else
                        {
                          gtree[down].seq[si] = 2;
                        }
                    }
                  else if ((gtree[sis].seq[si] != -1 && gtree[sis].seq[si] != 2)
                           && (gtree[sis].seq[si] != gtree[edge].seq[si]))
                    {
                      gtree[down].seq[si] = 2;
                      /* assert (G->mut[si] == -1 || G->mut[si] == down); */
                      G->mut[si] = down;
                    }
                  else
                    {
                      gtree[down].seq[si] = gtree[edge].seq[si];
                    }
                  flag = 'T';
                }
              else if (gtree[down].seq[si] == 2)
                {
                  assert (gtree[edge].seq[si] == 2);
                  if (gtree[sis].seq[si] != -1 && gtree[sis].seq[si] != 2)
                    {
                      gtree[down].seq[si] = gtree[sis].seq[si];
                      flag = 'T';
                    }
                }
              edge = down;
            }
        }
    }
  /* Find a single edge with a mutation event at site [[si]]. */
  /* All edges except one must be labelled either 0 or 1. The only
   * edge is labelled 2. The edge is can be found at G->mut[si]. */
  accp = 1;
  for (si = 0; si < L[li].numsites; si++)
    {
      nmut = 0; 
      for (ei = ngenes; ei < ngnodes; ei++)
        {
          assert (gtree[ei].seq[si] == 0
                  || gtree[ei].seq[si] == 1
                  || gtree[ei].seq[si] == 2);
          if (gtree[ei].seq[si] == 2)
            {
              nmut++;
              ledge = gtree[ei].up[IM_EDGE_LEFT];
              redge = gtree[ei].up[IM_EDGE_RIGHT];
              /* assert (gtree[ledge].seq[si] != gtree[redge].seq[si]); */
              if (gtree[ledge].seq[si] == gtree[redge].seq[si])
                {
                  nmut = 999;
                  break;
                }
              if (ei == G->root)
                {
                  gtree[ei].seq[si] = 0;
                  if (gtree[ledge].seq[si] == 0)
                    {
                      G->mut[si] = redge;
                    }
                  else
                    {
                      G->mut[si] = ledge;
                    }
                }
              else
                {
                  down = gtree[ei].down;
                  assert (!(down < 0));
                  assert (down < ngnodes);
                  if (gtree[ledge].seq[si] == gtree[down].seq[si])
                    {
                      G->mut[si] = redge;
                      gtree[ei].seq[si] = gtree[ledge].seq[si];
                    }
                  else
                    {
                      G->mut[si] = ledge;
                      gtree[ei].seq[si] = gtree[redge].seq[si];
                    }
                }
            }
        }
      /* assert (nmut == 1); */
      if (nmut != 1)
        {
          accp = 0;
          break;
        }
    }
  return accp;
}

int 
IMA_sbf_c (int ci, int li, int edge, int pi, 
           int *seqz,
           int *n, int **l, int *a)
{
  int j;
  int ei;
  char can_coalesce;
  int si;
  int ni;
  int nl;
  struct genealogy *G;
  struct edge *gtree;
  int ledge;
  int redge;

  if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 1)
    {
      assert (seqz == NULL);
    }

  G = &C[ci]->G[li];
  gtree = G->gtree;
  ledge = gtree[edge].up[IM_EDGE_LEFT];
  redge = gtree[edge].up[IM_EDGE_RIGHT];

  /* Remove edges (both left and right) that coalesce together if there is. It
   * is possible that they are absent in [[l]]. */
  nl = n[pi];
  j = 0;
  for (ei = 0; ei < nl; ei++)
    {
      if (l[pi][ei] == ledge || l[pi][ei] == redge)
        {
          /* assert ledge could have coalesced with z. */
        }
      else
        {
          a[j] = l[pi][ei];
          j++;
        }
    }
  assert (j == nl || j == nl - 1 || j == nl - 2);
  if (j < nl)
    {
      if (j > 0)
        {
          memcpy (l[pi], a, j * sizeof (int));
        }
      n[pi] = j;
    }

  /* Add edge if it can coalesce with active lineage z. */
  can_coalesce = 'T';
  if (seqz != NULL)
    {
      for (si = 0; si < L[li].numsites; si++)
        {
          if (G->mut[si] == edge)
            {
              continue;
            }
          if (gtree[edge].seq[si] != seqz[si])
            {
              can_coalesce = 'F';
            }
        }
    }
  if (can_coalesce == 'T') /* edge and z can coalesce together? */
    {
      ni = n[pi];
      l[pi][ni] = edge;
      n[pi]++;
    }
  return 0;
} 

int 
IMA_sbf_m (int edge, int pi, int pj,
           int *n, int **l, int *a)
{
  int j;
  int ei;
  int ni;
  int nl;

  assert (pi != pj);

  /* Remove an edge if it exists in pi. */
  nl = n[pi];
  j = 0;
  for (ei = 0; ei < nl; ei++)
    {
      if (l[pi][ei] == edge)
        {
          /* assert ledge could have coalesced with z1. */
        }
      else
        {
          a[j] = l[pi][ei];
          j++;
        }
    }
  assert (j == nl || j == nl - 1);
  if (j < nl)
    {
      /* Add the edge to population pj. */
      if (j > 0)
        {
          memcpy (l[pi], a, j * sizeof (int));
        }
      n[pi] = j;
      ni = n[pj];
      for (ei = 0; ei < ni; ei++)
        {
          assert (l[pj][ei] != edge);
        }
      l[pj][ni] = edge;
      n[pj]++;
    }
  else if (j == nl)
    {
      /* No Code! */
    }
  else
    {
      assert (0);
    }
  return 0;
}

int
IMA_sbf_nsum (im_bfupdate *pbf,
              int pc1, int pc3)
{
  int nsum;
  int pi;

  /* Make sure that we have one inactive lineage. */
  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      if (pi == pc3)
        {
          assert (pbf->n[pc3] == 1);
        }
      else
        {
          assert (pbf->n[pi] == 0);
        }
    }
  pbf->n[pc1]++;

  nsum = 0;
  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      nsum += pbf->n[pi];
    }
  assert (nsum == 2);

  return nsum;
}

void 
IMA_sbf_lineages (im_bfupdate *pbf, int ci, int li, int z1)
{
  int n;
  int p;
  int np;
  int ni;
  int si;
  int ei;
  int edge;
  struct edge *gtree;
  struct genealogy *G;
  int ngenes;
  char can_coalesce;

  assert (modeloptions[PARAMETERSBYPERIOD] == 0);
  for (p = 0; p < numpopsizeparams; p++)
    {
      pbf->n[p] = 0;
      pbf->nz1[p] = 0; /* Only for infinite-sites model */
    }
  ngenes = L[li].numgenes;
  G = &C[ci]->G[li];
  gtree = G->gtree;
  for (ei = 0; ei < ngenes; ei++)
    {
      /* Exclude a gene at an active lineage. */
      if (ei == z1) 
        {
          continue; 
        }
      assert (!(gtree[ei].pop < 0));
      assert (gtree[ei].pop < 2 * npops - 1);
      p = gtree[ei].pop;
      /* Add gene [[ei]] to population [[p]]. */
      n = pbf->n[p];
      pbf->l[p][n] = ei;
      pbf->n[p]++;
    }

  /* Number of lineages in populations for active lineages to be able to
   * coalesce. */
  if (L[li].model == INFINITESITES)
    {
      for (p = 0; p < numpopsizeparams; p++)
        {
          np = pbf->n[p];
          for (ni = 0; ni < np; ni++)
            {
              edge = pbf->l[p][ni];
              can_coalesce = 'T';
              
              if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
                {
                  for (si = 0; si < L[li].numsites; si++)
                    {
                      if (G->mut[si] == edge)
                        {
                          continue;
                        }
                      if (gtree[edge].seq[si] != pbf->seqz1[si])
                        {
                          can_coalesce = 'F';
                          break;
                        }
                    }
                }

              if (can_coalesce == 'T') /* If edge and z1 can coalesce together? */
                {
                  n = pbf->nz1[p];
                  pbf->lz1[p][n] = edge;
                  pbf->nz1[p]++;
                }
            }
        }
    }
  return;
}
void 
IMA_intervals_collectsbfnew (int ci, int li, im_edgemiginfo *newm3)
{
  double ct;
  int pc;

  im_event *ek;
  int mi;
  int mj;
  int ei;
  int si;
  int ki;
  int pi;
  int z3;
  struct edge *gtree;
  struct genealogy *G;
  double t;
  double roottime;
  char m3type;
  int down1;

  G = &C[ci]->G[li];
  gtree = C[ci]->G[li].gtree;
  if (sbfnew.sis1 == G->root)
    {
      sbfnew.is_finiterootbranch = 'T';
    }
  else
    {
      sbfnew.is_finiterootbranch = 'F';
    }
  
  down1 = gtree[sbfnew.m1->edgeid].down;
  if (L[li].model == INFINITESITES)
    {
      if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          assert (L[li].numsites > 0); /* FIXME: Crash! */
          memcpy (sbfnew.seqz1, gtree[down1].seq, L[li].numsites * sizeof (int)); 
        }
    }
  else if (L[li].model == HKY)
    {
      assert (L[li].numsites > 0); /* FIXME: Crash! */
      /* No code. */
    }
  else if (L[li].model == STEPWISE)
    {
      /* No code. */
    }
  else
    {
      assert (0);
    }

  ct = sbfnew.m1->dnt;
  pc = sbfnew.m1->fpop;

  /***************************************************************************/
  /*                             sbfold.m3                                   */
  /***************************************************************************/
  /* Fill sbfold.m3 when detaching an external root-connecting branch and    */
  /* attaching it to an inactive lineage.                                    */
  if (newm3 == NULL && sbfold.is_finiterootbranch == 'T')
    {
      ei = G->root;
      roottime = imaGtreeTime (ci, li, ei);

      sbfold.m3 = &saosms[2 * li];
      sbfold.m3->li = li;
      sbfold.m3->edgeid = ei;
      if (ct < roottime)
        {
          m3type = 'A';
          sbfold.m3->upt = roottime;
          sbfold.m3->pop = gtree[ei].pop;
        }
      else
        {
          m3type = 'B'; 
          sbfold.m3->upt = ct;
          sbfold.m3->pop = pc;
        }
      assert (ct < gtree[ei].time);
      assert (gtree[ei].time < TIMEMAX - 1.0);
      sbfold.m3->dnt = gtree[ei].time;

      mi = 0;
      mj = 0;
      while (gtree[ei].mig[mi].mt > -0.5)
        {
          if (gtree[ei].mig[mi].mt < ct)
            { 
              mi++;
              continue;
            }
          sbfold.m3->mig[mj] = gtree[ei].mig[mi];
          mi++;
          mj++;
        }
      sbfold.m3->mig[mj].mt = -1.0;
      sbfold.m3->mpall = mj;
      if (mj == 0)
        {
          pi = sbfold.m3->pop;
        }
      else
        {
          pi = sbfold.m3->mig[mj - 1].mp;
        }
      ki = findperiodoftime (ci, sbfold.m3->dnt);
      sbfold.m3->fpop = saC.popndown[pi][ki];
      if (ct < roottime)
        {
          /* No Code! */
        }
      else
        {
          assert (sbfnew.m1->dnt == sbfold.m3->upt);
        }
      assert (sbfnew.m1->dnt < sbfold.m3->dnt);

      /* Why are we doing this here? */
      /* Set sbfnew's canz31? */
      z3 = G->root;
      sbfnew.canz31 = 'T';
      if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          if (L[li].model == INFINITESITES)
            {
              for (si = 0; si < L[li].numsites; si++)
                {
                  if (G->mut[si] == z3)
                    {
                      continue;
                    }
                  if (gtree[z3].seq[si] != sbfnew.seqz1[si])
                    {
                      sbfnew.canz31 = 'F';
                    }
                }
            }
          else if (L[li].model == HKY || L[li].model == STEPWISE)
            {
              /* No code. */
            }
        }
      assert (sbfnew.canz31 == 'T');
    }
  else
    {
      sbfold.m3 = NULL;
    }

  /***************************************************************************/
  /*                                 sbfnew                                  */
  /***************************************************************************/

  /* Copy e1 from sbfold's to sbfnew's. */
  if (sbfnew.nmax1 < sbfold.n1)
    {
      sbfnew.nmax1 = sbfold.n1;
      sbfnew.e1 = realloc (sbfnew.e1, sbfnew.nmax1 * sizeof (im_event));
    }
  assert (sbfold.n1 > 0);
  memcpy (sbfnew.e1, sbfold.e1, sbfold.n1 * sizeof (im_event));
  sbfnew.n1 = sbfold.n1;

  /* Case I */
  if (newm3 != NULL)
    {
      /* sbfnew is lengthened by newm3. */
      /* if there is end event, we remove it */
      /* we can add ``end'' event. */
      if (sbfnew.e1[sbfnew.n1 - 1].type == 'e')
        {
          sbfnew.n1--;
        }
       
      if (sbfnew.nmax1 < sbfnew.n1 + newm3->mpall + numsplittimes + 1)
        {
          sbfnew.nmax1 = sbfnew.n1 + newm3->mpall + numsplittimes + 1;
          sbfnew.e1 = realloc (sbfnew.e1, sbfnew.nmax1 * sizeof (im_event));
        }
      /* ek = &sbfnew.e1[sbfnew.n1]; */
      ek = sbfnew.e1 + sbfnew.n1;
      mi = 0;
      while (newm3->mig[mi].mt > -0.5)
        {
          t = newm3->mig[mi].mt;
          ek->type = 'm';
          ek->t = t;
          ek->p = findperiodoftime (ci, t);
          if (mi == 0)
            {
              ek->pi = newm3->pop;
            }
          else
            {
              ek->pi = newm3->mig[mi - 1].mp;
            }
          assert (!(ek->pi < 0));
          assert (ek->pi < numpopsizeparams);
          ek->pi = saC.popndown[ek->pi][ek->p];
          ek->pj = newm3->mig[mi].mp;
          assert (!(ek->pi < 0));
          assert (ek->pi < numpopsizeparams);
          assert (!(ek->pj < 0));
          assert (ek->pj < numpopsizeparams);
          ek->ei = newm3->edgeid;
          ek++;
          mi++;
        }
      assert (newm3->mpall == mi); 
      /* end event */
      t = newm3->dnt;
      ek->type = 'e';
      ek->t = t;
      ek->p = findperiodoftime (ci, t);
      if (mi == 0)
        {
          ek->pi = newm3->pop;
        }
      else
        {
          ek->pi = newm3->mig[mi - 1].mp;
        }
      assert (!(ek->pi < 0));
      assert (ek->pi < numpopsizeparams);
      ek->pj = saC.popndown[ek->pi][ek->p];
      assert (!(ek->pj < 0));
      assert (ek->pj < numpopsizeparams);
      ek->ei = newm3->edgeid;
      sbfnew.n1 += newm3->mpall + 1;

      /* We need to sort the events. */
      ek = sbfnew.e1 + sbfnew.n1;
      for (ki = 0; ki < numsplittimes; ki++)
        {
          t = C[ci]->tvals[ki];
          if (newm3->upt < t && t < newm3->dnt)
            {
              ek->type = 's'; 
              ek->t = t;
              ek->p = ki;    /* Split time happens right before the period line. */
              ek->pi = ki;   /* from 0 (pi) to 1 (pj) for the first split time.  */
              ek->pj = ki + 1;
              ek->ei = -1;
              ek++;
              sbfnew.n1++;
            }
        }
      qsort (sbfnew.e1, (size_t) sbfnew.n1, sizeof (im_event), im_event_cmp);
    }
  /* Case II */
  else if (newm3 == NULL && sbfold.is_finiterootbranch == 'T')
    {
      assert (sbfold.m3 != NULL);
      /* sbfnew is shortened by oldm3. */
      /* if there is end event, we remove it */
      /* we may add ``end'' event. */
      if (sbfnew.e1[sbfnew.n1 - 1].type == 'e')
        {
          sbfnew.n1--;
        }
      /* Remove events below oldm3 */
      if (m3type == 'A')
        {
           while (sbfnew.e1[sbfnew.n1 - 1].t > sbfold.m3->upt) 
            {
              if (sbfnew.e1[sbfnew.n1 - 1].type == 'c'
                  && sbfnew.e1[sbfnew.n1 - 1].ei == sbfold.m3->edgeid)
                {
                  break;
                }
              sbfnew.n1--;
            }
        }
      else if (m3type == 'B')
        {
          while (sbfnew.e1[sbfnew.n1 - 1].t > sbfold.m3->upt) 
            {
              assert (!(sbfnew.e1[sbfnew.n1 - 1].type == 'c'
                        && sbfnew.e1[sbfnew.n1 - 1].ei == sbfold.m3->edgeid));
              sbfnew.n1--;
            }
          ek = sbfnew.e1 + sbfnew.n1;
          ek->ei = sbfold.m3->edgeid;
          ek->type = 'e';
          ek->t = sbfold.m3->upt;
          ek->p = findperiodoftime (ci, ek->t);
          mi = 0;
          t = gtree[ek->ei].mig[mi].mt;
          if (t > -0.5 && t < ek->t)
            {
              mi++;
              t = gtree[ek->ei].mig[mi].mt;
            }
          if (mi == 0)
            {
              ek->pi = gtree[ek->ei].pop;
            }
          else
            {
              ek->pi = gtree[ek->ei].mig[mi - 1].mp;
            }
          ek->pj = sbfold.m3->pop;
          sbfnew.n1++;
        }
      else
        {
          assert (0);
        }
      /* No need to sort */
    }
  return;
}

double 
bfq (im_bfupdate *pbf, im_bfupdate *qbf, 
     int ci, int li, 
     im_ginfo *gk,
     im_ginfo *gkz1)
{
  double lq;
  char is_coalesced1;
  char is_coalesced3;
  int pc1;
  double prevt;
  double dt;
  int ki;
  int ti;
  int ti3;
  int pj;
  int np;
  int mi;
  double totalrate;
  im_event *ek;
  double lasteventtime;
  double rates[2 * MAXPOPS];
  int nsum;
  double t;
  struct edge *gtree;
  struct genealogy *G;
  int ngenes;
  double newct;
  int pc3;
  int ri;
  int nr;
  int z1;
  int z3;
  double r;
  char eventAadded;

  G = &C[ci]->G[li];
  gtree = G->gtree;
  ngenes = L[li].numgenes;

  /***************************************************************************/
  /* Prepare [[e2]] and possibly [[e3]].                                     */
  /***************************************************************************/
  /* Type 'a': lineage 1 coalesces. 
   *      'p': lineage 1 migrates.
   *      'r': lineage 3 migrates. */
  lasteventtime = pbf->e1[pbf->n1 - 1].t;

  if (pbf->nmax2 < pbf->n1)
    {
      pbf->nmax2 = pbf->n1;
      pbf->e2 = realloc (pbf->e2, pbf->nmax2 * sizeof (im_event));
    }
  pbf->e2 = memcpy (pbf->e2, pbf->e1, pbf->n1 * sizeof (im_event));
  pbf->n2 = pbf->n1;
  if (pbf->nmax2 < pbf->n2 + qbf->m1->mpall + 1)
    {
      pbf->nmax2 = pbf->n2 + qbf->m1->mpall + 1;
      pbf->e2 = realloc (pbf->e2, pbf->nmax2 * sizeof (im_event));
    }
  ek = pbf->e2 + pbf->n2;
  for (mi = 0; mi < qbf->m1->mpall; mi++)
    {
      t = qbf->m1->mig[mi].mt;
      assert (t > -0.5);
      if (t > lasteventtime)
        {
          break;
        }
      ek->type = 'p';
      ek->t = t;
      ek->p = findperiodoftime (ci, t);
      if (mi == 0)
        {
          ek->pi = qbf->m1->pop;
        }
      else
        {
          ek->pi = qbf->m1->mig[mi - 1].mp;
        }
      ek->pi = saC.popndown[ek->pi][ek->p];
      ek->pj = qbf->m1->mig[mi].mp;
      ek->ei = qbf->m1->edgeid;
      ek++;
      pbf->n2++;
    }
  if (qbf->m1->dnt < lasteventtime)
    {
      /* Active lineage [[z1]] coalesces. */
      ek->type = 'a';
      ek->t = qbf->m1->dnt;
      ek->p = -1;
      ek->pi = -1;
      ek->pj = qbf->m1->fpop;
      ek->ei = qbf->m1->edgeid;
      ek++;
      pbf->n2++;
      eventAadded = 'y';
    }
  else
    {
      eventAadded = 'n';
    }
  qsort (pbf->e2, (size_t) pbf->n2, sizeof (im_event), im_event_cmp);

  /* old3 = two or three active lineages   */
  /* qbf->m1->mpall and a coalescent       */
  /* qbf->m3->mpall and a coalescent       */
  pbf->n3 = 0;
  if (qbf->m3 != NULL)
    {
      if (pbf->nmax3 < qbf->m1->mpall + 1)
        {
          pbf->nmax3 = qbf->m1->mpall + 1;
          pbf->e3 = realloc (pbf->e3, pbf->nmax3 * sizeof (im_event));
        }
      ek = pbf->e3;
      for (mi = 0; mi < qbf->m1->mpall; mi++)
        {
          t = qbf->m1->mig[mi].mt; 
          assert (t > -0.5);
          if (t < lasteventtime)
            {
              continue;
            }
          ek->type = 'p';
          ek->t = t;
          ek->p = findperiodoftime (ci, t);
          if (mi == 0)
            {
              ek->pi = qbf->m1->pop;
            }
          else
            {
              ek->pi = qbf->m1->mig[mi - 1].mp;
            }
          ek->pi = saC.popndown[ek->pi][ek->p];
          ek->pj = qbf->m1->mig[mi].mp;
          ek->ei = qbf->m1->edgeid;
          ek++;
          pbf->n3++;
        }
      assert (qbf->m1->dnt > lasteventtime);
      assert (eventAadded = 'n');
      ek->type = 'a';
      ek->t = qbf->m1->dnt;
      ek->p = -1;
      ek->pi = -1;
      ek->pj = qbf->m1->fpop;
      ek->ei = qbf->m1->edgeid;
      ek++;
      pbf->n3++;

      if (pbf->nmax3 < pbf->n3 + qbf->m3->mpall + 1)
        {
          pbf->nmax3 = pbf->n3 + qbf->m3->mpall + 1;
          pbf->e3 = realloc (pbf->e3, pbf->nmax3 * sizeof (im_event));
        }
      ek = pbf->e3 + pbf->n3;
      for (mi = 0; mi < qbf->m3->mpall; mi++)
        {
          t = qbf->m3->mig[mi].mt;
          assert (t > lasteventtime);
          assert (t > -0.5);
          ek->type = 'r';
          ek->t = t;
          ek->p = findperiodoftime (ci, t);
          if (mi == 0)
            {
              ek->pi = qbf->m3->pop;
            }
          else
            {
              ek->pi = qbf->m3->mig[mi - 1].mp;
            }
          ek->pi = saC.popndown[ek->pi][ek->p];
          ek->pj = qbf->m3->mig[mi].mp;
          ek->ei = qbf->m3->edgeid;
          ek++;
          pbf->n3++;
        }
      if (pbf->nmax3 < pbf->n3 + numsplittimes + 1)
        {
          pbf->nmax3 = pbf->n3 + numsplittimes + 1;
          pbf->e3 = realloc (pbf->e3, pbf->nmax3 * sizeof (im_event));
        }
      ek = pbf->e3 + pbf->n3;
      for (ki = 0; ki < numsplittimes; ki++)
        {
          t = C[ci]->tvals[ki];
          if (qbf->m3->upt < t && t < qbf->m3->dnt)
            {
              ek->type = 's'; 
              ek->t = t;
              ek->p = ki;        /* Split time happens right before the period line. */
              ek->pi = ki;       /* from 0 (pi) to 1 (pj) for the first split time.  */
              ek->pj = ki + 1;
              ek++;
              pbf->n3++;
            }
        }
      qsort (pbf->e3, (size_t) pbf->n3, sizeof (im_event), im_event_cmp);
      assert (pbf->e3[pbf->n3 - 1].type == 'a');
    }
  /***************************************************************************/
  /* Prepare [[e2]] and possibly [[e3]].                                     */
  /***************************************************************************/

  /* Add probabilities before the bottom of a partial genealogy */
  /* pbf:sbfold, and qbf:sbfnew.                                */
  z1 = qbf->m1->edgeid;
  assert (!(z1 < 0));
  is_coalesced1 = 'F';
  pc1 = qbf->m1->pop;
  assert (!(pc1 < 0));
  IMA_sbf_lineages (pbf, ci, li, z1);

  ki = 0;
  ti = 0;
  lq = 0.0; 
  prevt = 0.0;
  ek = pbf->e2;
  while (is_coalesced1 == 'F') 
    {
      dt = ek->t - prevt;
      np = bfmove_ratesI (&rates, gk, gkz1,
                          pbf, ki, 
                          pc1);
      totalrate = 0.0;
      for (pj = 0; pj < np; pj++)
        {
          assert (!(rates[pj] < 0.0));
          totalrate += rates[pj];
        }
      if (totalrate == 0.0)
        {
          /* assert (0); I do not know when this would happen. We have to fix
           * it. */
          assert (ek->type == 'c'
                  || ek->type == 'm'
                  || ek->type == 's'
                  || ek->type == 'e');
        }
      lq -= totalrate * dt;
      lq = bfmove_nextI (ci, li, lq, ek, np, rates, 
                         gk, pbf, /* qbf, */
                         &ki,
                         z1,
                         &is_coalesced1,
                         &pc1);
      prevt = ek->t;
      ti++;
      if (ti < pbf->n2)
        {
          ek++;
        }
      else
        {
          break;
        }
    }

  /* Add probabilities after the bottom of a partial genealogy. */
  if (is_coalesced1 == 'F')
    {
      assert (pbf->m3 == NULL);
      assert (qbf->m3 != NULL);
      assert (pbf->n3 > 0);
      assert (pbf->n2 == ti);
      assert (pbf->e2[ti - 1].type != 's');
      assert (pbf->e2[ti - 1].type == 'e' || pbf->e2[ti - 1].type == 'c');
      assert (pbf->e2[ti - 1].t == prevt);
      assert (findperiodoftime (ci, prevt) == ki);

      is_coalesced3 = 'F';
      newct = prevt;
      pc3 = pbf->e2[ti - 1].pj;
      z3 = pbf->e2[ti - 1].ei;
      assert (z3 == qbf->m3->edgeid);
      assert (!(pc1 < 0));
      assert (!(pc3 < 0));
      assert (qbf->m3->upt == newct);
      assert (qbf->m3->li == li);
      assert (qbf->m3->edgeid == G->root);
      assert (qbf->m3->pop == pc3);

      /* Set canz31? */
      assert (is_coalesced1 == 'F');
      assert (pbf->canz31 == 'T');
      
      nsum = IMA_sbf_nsum (pbf, pc1, pc3);
      assert (nsum == 2);

      ti3 = 0;
      ek = pbf->e3;
      while (nsum == 2)
        {
          dt = ek->t - prevt;

          bfmove_ratesIII (&rates, gk, pbf, ki, pc1, pc3);
          nr = 3;
          totalrate = 0.0;
          for (ri = 0; ri < nr; ri++)
            {
              totalrate += rates[ri];
            }
          assert (totalrate > 0.0);
          assert (dt > 0.0);
          lq -= totalrate * dt;
          switch (ek->type)
            {
            case 's': /* split event. */
              assert (ek->pi == ki);
              ki++;
              assert (ek->pj == ki);
              pc1 = saC.popndown[pc1][ki];
              pc3 = saC.popndown[pc3][ki];
              break;
            case 'p': /* one left active lineage migrates. */
              pj = ek->pj;
              assert (ek->ei == z1);      /* lineage 1 coalesces. */
              assert (rates[0] > 0.0);
              assert (gk->mms[pc1][pj] > 0.0);
              assert (gk->ms[ki][pc1] > 0.0);
              r = gk->mms[pc1][pj];
              lq += log (r);
              pc1 = pj;
              break;
            case 'r': /* Lineage 3 migrates. */
              pj = ek->pj;
              assert (ek->ei == z3);
              assert (rates[1] > 0.0); 
              assert (gk->mms[pc3][pj] > 0.0);
              assert (gk->ms[ki][pc3] > 0.0);
              lq += log (gk->mms[pc3][pj]);
              pc3 = pj;
              break;
            case 'a': /* Two lineages coalesce. */
              is_coalesced3 = 'T';
              assert (ek->ei == z1);
              assert (is_coalesced1 == 'F');
              is_coalesced1 = 'T';
              assert (rates[2] > 0.0); 
              r = rates[2];
              lq += log (r);
              lq = bfmove_selectA1 (ci, li, qbf, lq);
              nsum = 1;
              break;
            default:
              assert (0);
              break;
            }
          prevt = ek->t;
          ti3++;
          ek++;
        }
      assert (ti3 == pbf->n3);
    }

  return lq;
}

double 
IMA_choose_migtopop (int *topop, int ki, int pi, im_ginfo *gk)
{
  double pr[MAXPOPS];
  int npminus1;
  int ni;
  int pj;
  int i;
  double total;

  total = 0.0;
  npminus1 = npops - ki - 1;
  for (ni = 0; ni < npminus1; ni++)
    {
      pj = saC.popnmig[ki][pi][ni];
      assert (gk->mms[pi][pj] > 0.0);
      pr[ni] = gk->mms[pi][pj];
      total += pr[ni];
    }
  assert (gk->ms[ki][pi] == total); /* FIXME03: failed with -O3 option compilation */

  i = ran_discretea (npminus1, pr);
  *topop = saC.popnmig[ki][pi][i];
  return pr[i] / total;
}

void
IMA_edgemiginfo_copymig (im_edgemiginfo *em, int ci, int li, int ei)
{
  int mi;
  int down;
  struct edge *gtree;

  gtree = C[ci]->G[li].gtree;
  down = gtree[ei].down;
  assert (!(down < 0));
  em->li = li;
  em->edgeid = ei;
  em->upt = imaGtreeTime (ci, li, ei);
  em->dnt = gtree[ei].time;
  em->pop = gtree[ei].pop;
  em->fpop = gtree[down].pop;
  mi = 0;
  while (gtree[ei].mig[mi].mt > -0.5)
    {
      em->mig[mi] = gtree[ei].mig[mi];
      mi++;
    }
  em->mig[mi].mt = -1;
  em->mpall = mi;

  return;
}


void
IMA_genealogy_copymig (int ci, int li, int ei, im_edgemiginfo *em)
{
  int mi;
  struct edge *gtree;

  gtree = C[ci]->G[li].gtree;

  assert (em->li == li);
  assert (em->edgeid == ei);
  mi = 0;
  while (em->mig[mi].mt > -0.5)
    {
      checkmigt (mi, &gtree[ei]);
      gtree[ei].mig[mi] = em->mig[mi];
      mi++;
    }
  gtree[ei].mig[mi].mt = -1.0;
  assert (em->mpall == mi);

  return;
}

void
IMA_genealogy_appendmig (int ci, int li, int sis, im_edgemiginfo *em, 
                         int edge, int down)
{
  double sisupt;
  double sisdnt;
  int mj;
  int mi;
  struct genealogy *G;
  struct edge *gtree;

  assert (em->li == li);
  assert (em->edgeid == sis);

  G = &C[ci]->G[li];
  gtree = G->gtree;
  imaSavedSaveBranch (ci, li, sis);
  sisupt = imaGtreeTime (ci, li, sis);
  sisdnt = gtree[sis].time;
  assert (G->root == sis);
  G->root = down;
  assert (G->roottime < em->dnt);
  G->roottime = em->dnt;
  gtree[down].down = -1;
  gtree[sis].down = down;
  gtree[edge].down = down;
  gtree[down].up[IM_EDGE_LEFT] = sis;
  gtree[down].up[IM_EDGE_RIGHT] = edge;
  gtree[edge].time = em->dnt;
  gtree[sis].time = em->dnt;
  gtree[down].time = TIMEMAX; 
  /* A partial genealogy becomes a full genealogy although it may have another
   * active lineage that needs to be attached. */

  mi = 0;
  while (gtree[sis].mig[mi].mt > -0.5 && gtree[sis].mig[mi].mt < em->upt)
    {
      mi++;
    }
  mj = 0;
  while (em->mig[mj].mt > -0.5)
    {
      checkmigt (mi, &gtree[sis]);
      gtree[sis].mig[mi] = em->mig[mj];
      mi++;
      mj++;
    }
  gtree[sis].mig[mi].mt = -1.0;
  assert (em->mpall == mj);
  gtree[down].mig[0].mt = -1.0;
  gtree[down].pop = em->fpop;

  return;
}

void 
IMA_genealogy_join (im_edge *gtree, int up, int down, int downdown)
{
  int mi;
  int mj;

  gtree[up].down = downdown;
  if (gtree[downdown].up[IM_EDGE_LEFT] == down)
    {
      gtree[downdown].up[IM_EDGE_LEFT] = up;
    }
  else
    {
      gtree[downdown].up[IM_EDGE_RIGHT] = up;
    }

  mi = 0;
  while (gtree[up].mig[mi].mt > -0.5)
    {
      mi++;
    }
  mj = 0;
  while (gtree[down].mig[mj].mt > -0.5)
    {
      checkmig (mi + 1, &(gtree[up].mig), &(gtree[up].cmm));
      gtree[up].mig[mi] = gtree[down].mig[mj];
      mi++;
      mj++;
    }
  gtree[up].mig[mi].mt = -1;
  gtree[up].time = gtree[down].time;
  return;
}

void 
IMA_genealogy_absorbdown (im_edge *gtree, int up, int down)
{
  int mi;
  int mj;

  mi = 0;
  while (gtree[up].mig[mi].mt > -0.5)
    {
      mi++;
    }
  mj = 0;
  while (gtree[down].mig[mj].mt > -0.5)
    {
      checkmig (mi + 1, &(gtree[up].mig), &(gtree[up].cmm));
      gtree[up].mig[mi] = gtree[down].mig[mj];
      mi++;
      mj++;
    }
  gtree[up].mig[mi].mt = -1;
  gtree[up].time = gtree[down].time;
  return;
}

void 
IMA_genealogy_bisectsis (int ci, int li, 
                         int sis, int edge, int down, 
                         im_edgemiginfo *em, 
                         int *seqz)
{
  double sisupt;
  double sisdnt;
  double ct;
  int nowpop;
  int downdown;
  int mj;
  int mi;
  int ki;
  int si;
  struct genealogy *G;
  struct edge *gtree;

  /* I do not like that I have to keep calling these two lines. I wanted to
   * avoid using them. It turned out that I have to change some basic structures
   * */
  G = &C[ci]->G[li];
  gtree = G->gtree;
  downdown = gtree[sis].down;
  imaSavedSaveBranch (ci, li, sis);
  if (!(downdown < 0))
    {
      imaSavedSaveBranch (ci, li, downdown);
    }

  sisupt = imaGtreeTime (ci, li, sis);
  sisdnt = gtree[sis].time;
  ct = em->dnt;
  assert (sisupt < ct && ct < sisdnt);
  if (G->root == sis)
  {
    assert (downdown == -1);
    G->root = down;
    G->roottime = ct;
  }
  else
  {
    if (gtree[downdown].up[IM_EDGE_LEFT] == sis)
      {
        gtree[downdown].up[IM_EDGE_LEFT] = down;
      }
    else
      {
        gtree[downdown].up[IM_EDGE_RIGHT] = down;
      }
  }
  gtree[down].down = downdown;

  gtree[sis].down = down;
  gtree[edge].down = down;
  gtree[down].up[IM_EDGE_LEFT] = sis;
  gtree[down].up[IM_EDGE_RIGHT] = edge;

  /* Population label and migration events */
  gtree[edge].time = ct;
  gtree[sis].time = ct;
  gtree[down].time = sisdnt;
  mi = 0;
  while (gtree[sis].mig[mi].mt > -0.5 
         && gtree[sis].mig[mi].mt < ct)
    {
      mi++;
    }
  if (mi > 0)
    {
      nowpop = gtree[sis].mig[mi - 1].mp;
    }
  else
    {
      nowpop = gtree[sis].pop;
    }

  ki = findperiodoftime (ci, ct);
  nowpop = saC.popndown[nowpop][ki];
  assert (em->fpop == nowpop);
  gtree[down].pop = em->fpop;

  mj = 0;
  while (gtree[sis].mig[mj + mi].mt > -0.5)
    {
      checkmigt (mj, &gtree[down]);
      gtree[down].mig[mj] = gtree[sis].mig[mj + mi];
      assert (nowpop != gtree[down].mig[mj].mp);
      nowpop = gtree[down].mig[mj].mp;
      mj++;
    }
  gtree[sis].mig[mi].mt = -1.0;
  gtree[down].mig[mj].mt = -1.0;

  if (L[li].model == INFINITESITES)
    {
      if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          /* Allocate proper things at the internal node sequence. */
          for (si = 0; si < L[li].numsites; si++)
            {
              if (G->mut[si] == sis)
                {
                  if (gtree[sis].seq[si] == seqz[si])
                    {
                      G->mut[si] = down;
                    }
                }
            }
        }
    }
  else if (L[li].model == HKY || L[li].model == STEPWISE)
    {
      /* No code. */
    }
  else
    {
      assert (0);
    }
  return;
}

int
IMA_intervals_collect (int ci, int li, double upt, double dnt)
{
  struct genealogy *G;
  struct edge *gtree;
  im_event *ek;
  int enmax;
  int ki;
  int mi;
  int ei;
  int a;
  int ngnodes;
  int ngenes;
  double t;

  G = &C[ci]->G[li];
  gtree = G->gtree;
  ngnodes = L[li].numlines;
  ngenes = L[li].numgenes;

  a = 0;
  ek = sbfold.e1;
  enmax = sbfold.nmax1;
  for (ei = ngenes; ei < ngnodes; ei++)
    {
      if (gtree[ei].exist == 'F')
        {
          continue;
        }
      t = imaGtreeTime (ci, li, ei);
      assert (upt < t);
      ek->type = 'c'; 
      ek->t = t;
      ek->p = -1;
      ek->pi = -1;
      ek->pj = gtree[ei].pop;
      ek->ei = ei;
      ek++;
      a++;
    }
  for (ki = 0; ki < numsplittimes; ki++)
    {
      t = C[ci]->tvals[ki];
      if (upt < t && t < dnt)
        {
          ek->type = 's'; 
          ek->t = t;
          ek->p = ki;    /* Split time happens right before the period line. */
          ek->pi = ki;   /* from 0 (pi) to 1 (pj) for the first split time.  */
          ek->pj = ki + 1;
          ek->ei = -1;
          ek++;
          a++;
        }
    }
  for (ei = 0; ei < ngnodes; ei++)
    {
      if (gtree[ei].exist == 'F')
        {
          continue;
        }
      mi = 0;
      assert (gtree[ei].cmm > 0);
      t = gtree[ei].mig[mi].mt;
      while (t > -0.5)
        {
          assert (upt < t);
          if (!(a < enmax - 1))
            {
              enmax += 5;
              sbfold.e1 = realloc (sbfold.e1, enmax * sizeof (im_event));
              sbfold.nmax1 = enmax;

              /* IMPORTANT! The pointer must repoint to a right place. */
              ek = &sbfold.e1[a]; 
            }

          ek->type = 'm';
          ek->t = t;
          ek->p = findperiodoftime (ci, t);
          if (mi == 0)
            {
              ek->pi = gtree[ei].pop;
            }
          else
            {
              ek->pi = gtree[ei].mig[mi - 1].mp;
            }
          ek->pi = saC.popndown[ek->pi][ek->p];
          ek->pj = gtree[ei].mig[mi].mp;
          ek->ei = ei;
          ek++;
          a++;

          mi++;
          t = gtree[ei].mig[mi].mt;
        }
      if (ei == G->root)
        {
          if (sbfold.is_finiterootbranch == 'T')
            {
              t = gtree[ei].time;
              assert (t < TIMEMAX - 1.0);
              if (!(a < enmax - 1))
                {
                  enmax += 5;
                  sbfold.e1 = realloc (sbfold.e1, enmax * sizeof (im_event));
                  sbfold.nmax1 = enmax;

                  /* IMPORTANT! The pointer must repoint to a right place. */
                  ek = &sbfold.e1[a];
                }

              ek->type = 'e';
              ek->t = t;
              ek->p = findperiodoftime (ci, t);
              if (mi == 0)
                {
                  ek->pi = gtree[ei].pop;
                }
              else
                {
                  ek->pi = gtree[ei].mig[mi - 1].mp;
                }
              ek->pi = saC.popndown[ek->pi][ek->p];
              ek->pj = ek->pi;
              ek->ei = ei;
              ek++;
              a++;
            }
        }
    }

  assert (a > 0);
  sbfold.n1 = a;
  qsort (sbfold.e1, (size_t) a, sizeof (im_event), im_event_cmp);
  assert (upt < sbfold.e1[0].t);
  
  return a;
}
int
IMA_genealogy_derivetheta (char w, int ci, int li, int z1)
{
  im_event *ek;
  double prevt;
  double tau;
  double h2term;
  int pi;
  int pj;
  int ki;
  int ai;
  int pii;
  double fm1;
  double fm1z1;
  int ngenes;
  int npminus1; 
  int ninterval;
  struct edge *gtree;
  int ni;
  int pk;
  int pl;
  im_ginfo *gk;
  im_ginfo *gkz1;
  im_bfupdate *pbf;
  int n1;
  int n2;
  int edge;

  assert (assignmentoptions[POPULATIONASSIGNMENT] == 1);
  gtree = C[ci]->G[li].gtree;

  if (w == 'o')
    {
      pbf = &sbfold;
      gk = &saGiold[li];
      gkz1 = &saGioldz1[li];
    }
  else if (w == 'n')
    {
      pbf = &sbfnew;
      gk = &saGinew[li];
      gkz1 = &saGinewz1[li];
    }
  else
    {
      assert (0);
    }

  ek = pbf->e1;
  ninterval = pbf->n1;
  ngenes = L[li].numgenes;
  IMA_sbf_lineages (pbf, ci, li, z1);
  imaGinfoReset (gk);
  imaGinfoReset (gkz1);

  h2term = 1 / (2 * L[li].hval);
  prevt = 0.0;
  ki = 0;
  npminus1 = npops - ki - 1;
  for (ai = 0; ai < ninterval; ai++)
    {
      tau = ek->t - prevt;
      assert (tau > 0.0);
      /* For all pop'n during period ki */
      for (pii = 0; pii < npops - ki; pii++) 
        {
          pi = C[ci]->plist[ki][pii];

          /* Jody's code does not have plus one or +1. When we have only single
           * lineage, we should have positve coalescent rate. 1 * (1 - 1) is
           * zero, and we would have 0 rate of coalescent for a single lineage.
           * That may be why we add one to the number of lineages. The idea is
           * that we should consider the active lineage. */
          gk->fc[pi] += snnminus1[pbf->n[pi] + 1] * tau * h2term; 
          gkz1->fc[pi] += snnminus1[pbf->nz1[pi] + 1] * tau * h2term; 
          assert (gk->fc[pi] < DBL_MAX);
          assert (gkz1->fc[pi] < DBL_MAX);
          if (modeloptions[NOMIGRATION] == 0 && ki < lastperiodnumber)
            {
              fm1 = (pbf->n[pi] + 1) * tau;
              fm1z1 = (pbf->nz1[pi] + 1) * tau;
              for (ni = 0; ni < npminus1; ni++)
                {
                  pj = saC.popnmig[ki][pi][ni];
                  gk->fm[pi][pj] += fm1;
                  gkz1->fm[pi][pj] += fm1z1;
                }
            }
        }

      switch (ek->type)
        {
        case 'c':
          pi = ek->pj;
          gk->cc[pi]++;
          gkz1->cc[pi]++;
          edge = ek->ei;
          if (L[li].model == INFINITESITES)
            {
              /* NOTE: We do not update sbfold.l!!! */
              pbf->n[pi]--;
              if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
                {
                  IMA_sbf_c (ci, li, edge, pi, pbf->seqz1,
                             pbf->nz1, pbf->lz1, pbf->a);
                }
              else
                {
                  IMA_sbf_c (ci, li, edge, pi, NULL,
                             pbf->nz1, pbf->lz1, pbf->a);
                }
            }
          else if (L[li].model == HKY || L[li].model == STEPWISE)
            {
              /* NOTE: We do not update sbfold.nz1 .lz1 !!! */
              IMA_sbf_c (ci, li, edge, pi, NULL, 
                         pbf->n, pbf->l, pbf->a);
            }
          break;
        case 'm':
          edge = ek->ei;
          pi = ek->pi;
          pj = ek->pj;
          gk->cm[pi][pj]++;
          gkz1->cm[pi][pj]++;
          if (L[li].model == INFINITESITES)
            {
              pbf->n[pi]--;
              pbf->n[pj]++;
              IMA_sbf_m (edge, pi, pj, 
                         pbf->nz1, pbf->lz1, pbf->a);
            }
          else if (L[li].model == HKY || L[li].model == STEPWISE)
            {
              IMA_sbf_m (edge, pi, pj, 
                         pbf->n, pbf->l, pbf->a);
            }
          break;
        case 's':
          assert (ek->p == ki);
          ki++;
          pj = C[ci]->droppops[ki][IM_EDGE_LEFT];
          pk = C[ci]->droppops[ki][IM_EDGE_RIGHT];
          pl = C[ci]->addpop[ki];
          pbf->n[pl] = pbf->n[pj] + pbf->n[pk];
          n1 = pbf->n[pj];
          n2 = pbf->n[pk];
          if (n1 > 0)
            {
              memcpy (pbf->l[pl], pbf->l[pj], n1 * sizeof (int));
            }
          if (n2 > 0)
            {
              memcpy (&pbf->l[pl][n1], pbf->l[pk], n2 * sizeof (int));
            }
          pbf->n[pj] = 0;
          pbf->n[pk] = 0;
          npminus1--;
          assert (!(npminus1 < 0));
          if (!(z1 < 0))
            {
              /* *pc1 = saC.popndown[*pc1][ki]; */
            }
          assert (L[li].model == INFINITESITES 
                  || L[li].model == HKY
                  || L[li].model == STEPWISE);
          if (!(z1 < 0))
            {
              /* FIXME: We do not use gkz1 or nz1, lz1 for HKY.
               * We should replace the above if-statement with 
               * if (L[li].model == INFINITESITES)
               */

              /* Change the number and sets as well. */
              n1 = pbf->nz1[pj];
              n2 = pbf->nz1[pk];
              pbf->nz1[pl] = n1 + n2;
              if (n1 > 0)
                {
                  memcpy (pbf->lz1[pl], pbf->lz1[pj], n1 * sizeof (int));
                }
              if (n2 > 0)
                {
                  memcpy (&pbf->lz1[pl][n1], pbf->lz1[pk], n2 * sizeof (int));
                }
              pbf->nz1[pj] = 0;
              pbf->nz1[pk] = 0;
            }
          break;
        case 'e':
          assert (ai == ninterval - 1);
          break;
        }
      prevt = ek->t;
      ek++;
    }

  IMA_ginfo_computetheta (gk);
  IMA_ginfo_computetheta (gkz1);
  
  return 0;
}

int
IMA_ginfo_computetheta (im_ginfo *gk)
{  
  int pi;
  int pj;
  int ti;
  int ni;
  int npminus1;
  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      if (gk->cc[pi] > 0)         /* Case (A) */
        {
          gk->thetas[pi] = 2 * gk->fc[pi] / (gk->cc[pi] + 1);
        }
      else
        {
          if (gk->fc[pi] > 0.0)   /* Case (B) */
            {
              assert (gk->cc[pi] == 0);
              gk->thetas[pi] = 2 * gk->fc[pi] / (1);
            }
          else                    /* Case (C) */
            {
              gk->thetas[pi] = 0.0;
            }
        }
      for (pj = 0; pj < numpopsizeparams; pj++)
        {
          if (pi == pj)
            {
              continue;
            }
          if (gk->cm[pi][pj] > 0) /* Case (D) */
            {
              assert (gk->fm[pi][pj] > 0.0);
              gk->mms[pi][pj] = (gk->cm[pi][pj] + 1) / gk->fm[pi][pj];
            }
          else                   /* Case (E) and (F) */
            {
              assert (gk->cm[pi][pj] == 0);
              if (gk->fm[pi][pj] > 0.0)
                {
                  gk->mms[pi][pj] = (1) / gk->fm[pi][pj];
                }
              else
                {
                  gk->mms[pi][pj] = BFMPRIORMIN;
                }
            }
        }
    }

  /* We need number of periods where migrations are allowed. */
  /* npops - 1 for tree model and 1 for tree model.          */
  /* lastperiodnumber is for this                            */
  for (ti = 0; ti < lastperiodnumber; ti++)
    {
      for (pi = 0; pi < numpopsizeparams; pi++)
        {
          if (saC.popnmig[ti][pi] != NULL)
            {
              gk->ms[ti][pi] = 0.0;
              npminus1 = npops - ti - 1;
              for (ni = 0; ni < npminus1; ni++)
                {
                  pj = saC.popnmig[ti][pi][ni];
                  gk->ms[ti][pi] += gk->mms[pi][pj];
                }
            }
        }
    }
  return 0;
}



#ifdef DELETETHIS
int
ran_discrete (int n, ...)
{
  register int i;
  va_list ap;
  double s;
  double a;
  double l[10];
  double u;
  int v;

  assert (n > 1);
  assert (n <= 10);
  va_start (ap, n);
  a = va_arg (ap, double);
  s = a;
  l[0] = a;
  for (i = 1; i < n; i++)
    {
      a = va_arg (ap, double);
      l[i] = a;
      s += a;
    }
  va_end (ap);

  u = s * uniform ();

  s = 0.0;
  for (i = 0; i < n; i++)
    {
      s += l[i];
      if (u < s)
        {
          v = i;
          break;
        }
    }
  return v;
}
#endif /* DELETETHIS */

int
ran_discretea (int n, double pr[])
{
  register int i;
  int v;
  double s;
  double u;

  s = 0;
  for (i = 0; i < n; i++)
    {
      s += pr[i]; 
    }
  u = s * uniform ();
  s = 0.0;
  for (i = 0; i < n; i++)
    {
      s += pr[i];
      if (u < s)
        {
          v = i;
          break;
        }
    }
  return v;

}

int
ran_discreteloga (int n, double pr[])
{
  register int i;
  int v;
  double s;
  double t;
  double u;

  t = pr[0];
  for (i = 1; i < n; i++)
    {
      LogSum2 (t, t, pr[i]);
    }
  u = log (uniform ()) + t;

  s = pr[0];
  if (u < s)
    {
      v = 0;
    }
  else
    {
      for (i = 1; i < n; i++)
        {
          LogSum2 (s, s, pr[i]);
          if (u < s)
            {
              v = i;
              break;
            }
        }
    }
  return v;
}

int
normalizeLoga (int n, double pr[])
{
  register int i;
  double t;

  t = pr[0];
  for (i = 1; i < n; i++)
    {
      LogSum2 (t, t, pr[i]);
    }
  
  for (i = 0; i < n; i++)
    {
      pr[i] -= t;
    }

  return 0;
}


double
likelihoodJC (int ci, int li, double u)
{
  double l_k[4];
  double v;
  int si;
  im_edge *gtree; 

  gtree = C[ci]->G[li].gtree;
  v = 0.0;
  for (si = 0; si < L[li].numsites; si++)
    {
      computeLk (&l_k, u, li, gtree, C[ci]->G[li].root, si);
      v += logsum (4, l_k[0], l_k[1], l_k[2], l_k[3]);
      v -= M_LN2 * 2;
    }

  return v;
}

/* Function: computeLk
 * l_k: return value for the conditional likelihood
 * u  : branch length scaler
 * li : locus index
 * t  : tree
 * k  : node index
 * si : site index */
int
computeLk (double (*l_k)[4], double u, int li, im_edge *t, int k, int si)
{
  int s;
  int x;
  int y;
  int left;
  int right;
  int base;
  int basel;
  int baser;
  double l_kl[4] = { 0.0, 0.0, 0.0, 0.0};
  double l_kr[4] = { 0.0, 0.0, 0.0, 0.0};
  double l_kleft;
  double l_kright;
  double A;
  double tl;
  double tr;

  left = t[k].up[0];
  right = t[k].up[1];
  base = -1;
  if (left < 0 && right < 0)
    {
      assert (!(k < 0));
      assert (k < L[li].numgenes);
      base = L[li].seq[k][si];
    }
  else
    {
      assert (!(left < 0 || right < 0));
      basel = computeLk (&l_kl, u, li, t, left, si);
      baser = computeLk (&l_kr, u, li, t, right, si);
      tl = u * IMA_edge_length (t, left);
      tr = u * IMA_edge_length (t, right);

      for (s = 0; s < 4; s++)
        {
          l_kleft = 0.0;
          if (basel == -1)
            {
              for (x = 0; x < 4; x++)
                {
                  A = PijJC (x, s, tl) + l_kl[x];
                  LogSum2 (l_kleft, l_kleft, A);
                }
            }
          else
            {
              assert (basel == 0 || basel == 1 || basel == 2 || basel == 3);
              l_kleft = PijJC (basel, s, tl);
            }

          l_kright = 0.0;
          if (baser == -1)
            {
              for (y = 0; y < 4; y++)
                {
                  A = PijJC (y, s, tr) + l_kr[y];
                  LogSum2 (l_kright, l_kright, A);
                }
            }
          else
            {
              assert (baser == 0 || baser == 1 || baser == 2 || baser == 3);
              l_kright = PijJC (baser, s, tr);
            }
          (*l_k)[s] = l_kleft + l_kright;
        }
    }

  return base; 
}

double
PijJC (int i, int j, double t)
{
  double v;
  double A;

  if (i == j)
    {
      A = -4.0 * t / 3.0;
      LogDiff (v, 0.0, A);
      v -= M_LN2 * 2;
    }
  else
    {
      A = -4.0 * t / 3.0 + log (3.0);
      LogSum2 (v, 0.0, A);
      v -= M_LN2 * 2;
    }
  return v;
}

double
IMA_edge_length (im_edge *t, int ei)
{
  double upt;
  int up0;

  up0 = t[ei].up[0];
  if (up0 < 0)
    {
      upt = 0.0;
    }
  else
    {
      upt = t[up0].time;
    }
  return t[ei].time - upt;
}

int 
im_event_cmp (const void *a, const void *b)
{
  const im_event *A = (const im_event *) a;
  const im_event *B = (const im_event *) b;
  if (A->t > B->t)
    return 1;
  else if (A->t < B->t)
    return -1;
  else
    return 0;
}
int
updateassignmentbflocus (int ci,int li)
{
  int accp;
  double lweight;
  double logU;
  double mweight;
  double hweight;
  struct genealogy *G = &(C[ci]->G[li]);

  if (L[li].model != INFINITESITES)
    {
      IM_errloc (AT, 
                 "BF99 update is implemented only for infinite-sites data");
    }

  if (snind_unknown == 0)
    {
      return 0;
    }

  G = &C[ci]->G[li];
  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      /* assertgenealogylocus (ci,li); */
    }

  /* Count up the number of tries of MCMC cycles. */
  if (ci == 0)
    {
      L[li].a_rec->upinf[IM_UPDATE_ASSIGNMENT_BF].tries++;
    }
  Cupinf[ci].upinf[IM_UPDATE_ASSIGNMENT_BF].tries++;

  /* Save the current state of a chain */
  imaSavedReset (li);
  imaSavedSaveLocusInfo (ci, li);
  copy_treeinfo (&saT[li].savedgweight, &C[ci]->G[li].gweight);
  copy_treeinfo (&saC.savedallgweight, &C[ci]->allgweight);
  copy_probcalc_all (&saC.savedallpcalc, &C[ci]->allpcalc);

  /* Propose a new state of genealogy and assignment. */
  hweight = bflocusmove (ci,li);

  /* Find all internal node sequences and mutations of a full genealogy. */
  /* This is for debugging purposes.                                     */
  if (L[li].model == INFINITESITES
      && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
    {
      accp = IMA_genealogy_findIntSeq (ci, li);
      assert (accp == 1);
    }

  /* Compute Metropolis-ratio. */
  setzero_genealogy_weights (&C[ci]->G[li].gweight);
  treeweight (ci, li);
  sum_subtract_treeinfo (&C[ci]->allgweight, &G->gweight,
                         &saT[li].savedgweight);
                         /* &holdgweight_updategenealogy); CHECKTHIS */

  initialize_integrate_tree_prob (ci, &C[ci]->allgweight, &C[ci]->allpcalc);

  /* Compute likelihood. */
  /* This has to be called after treeweight. */
  if (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
    {
      bflocuslikelihood (ci,li);
    }
  else
    {
      C[ci]->allpcalc.pdg = 0.0;
    }

  logU = log (uniform ());
  mweight = (C[ci]->allpcalc.probg - saC.savedallpcalc.probg) 
            + gbeta * (C[ci]->allpcalc.pdg - saC.savedallpcalc.pdg);
  lweight = beta[ci] * mweight + hweight;
  assert (beta[ci] * mweight + hweight > -1e200
          && beta[ci] * mweight + hweight < 1e200);

  accp = 0;

  /* Find all internal node sequences and mutations of a full genealogy. */
  /* This is for debugging purposes.                                     */
  if (L[li].model == INFINITESITES
      && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
    {
      accp = IMA_genealogy_findIntSeq (ci, li);
      assert (accp == 1);
    }
  else if (L[li].model == HKY || L[li].model == STEPWISE)
    {
      /* No code. */
    }

  if (lweight > 0.0 || lweight > logU)
    {
      /* accept the update */
      accp = 1;
      if (ci == 0)
        {
          L[li].a_rec->upinf[IM_UPDATE_ASSIGNMENT_BF].accp++;
        }
      Cupinf[ci].upinf[IM_UPDATE_ASSIGNMENT_BF].accp++;
      C[ci]->nasn[sbfold.pop + li * npops]--;
      C[ci]->nasn[sbfnew.pop + li * npops]++;

      /* Always revert to an original. */
      /* I do not release the code of HKY mutation model. I keep the code. */
      if (L[li].model == HKY)
        {
          copyfraclike (ci, li);
          storescalefactors (ci, li);
        }
    }
  else
    {
      /* restore edges changed during update */
      imaSavedRestore (ci, li); 
      imaSavedRestoreLocusInfo (ci, li);
      copy_treeinfo (&C[ci]->G[li].gweight, &saT[li].savedgweight);
      copy_treeinfo (&C[ci]->allgweight, &saC.savedallgweight);
      copy_probcalc_all (&C[ci]->allpcalc, &saC.savedallpcalc);
    }

  /* HKY may be incomplete. */
  if (L[li].model == HKY)
    {
      restorescalefactors (ci, li);
    }

  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      /* assertgenealogylocus (ci,li); */
    }
  return accp;
}
int 
bflocuslikelihood (int ci,int li)
{
  int ai;
  struct genealogy *G = NULL;
  assert (calcoptions[DONTCALCLIKELIHOODMUTATION] == 0);

  G = &(C[ci]->G[li]);
  C[ci]->allpcalc.pdg -= G->pdg;
  switch (L[li].model)
    {
    case HKY:
      if (assignmentoptions[JCMODEL] == 1)
        {
          G->pdg = likelihoodJC (ci, li, G->uvals[0]);
        }
      else
        {
          G->pdg = likelihoodHKY (ci, li, G->uvals[0], G->kappaval, -1, -1, -1, -1);
        }
      G->pdg_a[0] = G->pdg;
      break;
    case INFINITESITES:
      /* We need both pdg and pdb_a to be set. */
      G->pdg = likelihoodIS (ci, li, G->uvals[0]);
      G->pdg_a[0] = G->pdg;
      break;
    case STEPWISE:
      G->pdg = 0.0;
      for (ai = 0; ai < L[li].nlinked; ai++)
      {
        G->pdg_a[ai] = likelihoodSW (ci, li, ai, G->uvals[ai], 1.0);
        G->pdg += G->pdg_a[ai];
      }
      break;
    case JOINT_IS_SW:
      assert (0);
      ai = 0;
      G->pdg_a[ai] = likelihoodIS (ci, li, G->uvals[0]);
      for (ai = 1; ai < L[li].nlinked; ai++)
      {
        /* ui = L[li].uii[ai];       */
        /* assert (pdgnew[ui] == 0); */
        G->pdg_a[ai] = likelihoodSW (ci, li, ai, G->uvals[ai], 1.0);
        G->pdg += G->pdg_a[ai];
      }
      break;
    }

  C[ci]->allpcalc.pdg += G->pdg;

  return 0; 
}
double
bflocusmove (int ci,int li)
{
  /* iterator variables */
  int locusid;
  int foundAlleleToMove;
  int ei;
  int pj;
  int ii;
  int ij;
  int ej;
  int i;
  int mi;
  int si;
  /* numbers of counts */
  int nei;
  int ngenes;
  int nsum;
  int np;
  int ki;
  int prevki;
  int oldpop;
  int newpop;
  struct genealogy *G;
  struct edge *gtree; 
  int z1;
  int z3;
  int pc1;
  int pc3;
  char is_coalesced1;
  char is_coalesced3;
  im_edgemiginfo *m1;
  im_edgemiginfo *m3;
  double totalrate;
  double rates[2 * MAXPOPS];
  double newct;
  int down1;
  int re;
  double dnt_genealogy;
  int ri;
  int nsim;
  int ti;
  double lhweight;
  im_event *ek;
  im_ginfo *gk;
  im_ginfo *gkz1;
  double prevt;
  double dt;
  double u;
  double rt;
  int nr;
  double lqstar;
  double lq;

  lhweight = 0.0;

  /* Find all internal node sequences and mutations of a full genealogy. */
  if (L[li].model == INFINITESITES 
      && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
    {
      i = IMA_genealogy_findIntSeq (ci, li);
      assert (i == 1);
    }

  /* Pick an allele to relabel. */
  foundAlleleToMove = 0;
  while (foundAlleleToMove == 0)
    {
      ii = snind_known + randposint (snind_unknown);
      nei = sngenes_ind[ii];
      for (ij = 0; ij < nei; ij++)
        {
          locusid = saC.indlist[ii][ij].li;
          if (li == locusid)
          {
            ei = saC.indlist[ii][ij].ei;
            foundAlleleToMove = 1;
            break;
          }
        }
    }
  imaSavedSaveBranch (ci, li, ei);

  /* Prepare all migration events storage. */
  IMA_reset_edgemiginfo (&saoems[2 * li]);
  IMA_reset_edgemiginfo (&saoems[2 * li + 1]);
  IMA_reset_edgemiginfo (&saosms[2 * li]);
  IMA_reset_edgemiginfo (&sanems[2 * li]);
  IMA_reset_edgemiginfo (&sanems[2 * li + 1]);
  IMA_reset_edgemiginfo (&sansms[2 * li]);
  ej = saoems[2 * li].edgeid;
  assert (ej < 0);
  IMA_edgemiginfo_copymig (&saoems[2 * li], ci, li, ei);

  /* Label the chosen allele. */
  oldpop = C[ci]->G[li].gtree[ei].pop;
  lhweight = imaRandomPickpopnLocus (&newpop, ci, li, oldpop, C[ci]->rootpop);
  sbfold.pop = oldpop;
  sbfnew.pop = newpop;
  C[ci]->G[li].gtree[ei].pop = newpop;

  /* Simulate branches for each genealogy. */
  ngenes = L[li].numgenes;
  G = &C[ci]->G[li];
  gtree = G->gtree;

  /* If no genes in locus [[li]] belong to individual [[ii]]. */
  /* Decide old branches. -> wrong words!     */
  /* We do not have a longer branch any more. */
  assert (!(saoems[2 * li].edgeid < 0));
  /* No long branches any more. */
  /* assert (0); We can delete this line but I do not allow partial
   * genes of an individual. */
  /* The line above may not make sense. */

  assert (saoems[2 * li].upt == 0.0);
  sbfold.m1 = &saoems[2 * li];
  z1 = sbfold.m1->edgeid;

  m1 = &sanems[2 * li]; /* sbfnew.m1 could be m1. */
  m1->li = li;
  m1->edgeid = z1;
  m1->upt = 0.0;
  m1->pop = newpop;
  dnt_genealogy = imaGtreeTime (ci, li, G->root);

  /* Detach active lineages. */
  sbfold.is_finiterootbranch = 'F'; /* This is the 1st occurrence.(2 more) */
  IMA_genealogy_detach (ci, li, &sbfold, z1);

  /* Get intervals of events of a partial genealogy. */
  assert (!gsl_fcmp(dnt_genealogy, G->roottime, 1e-7));
  IMA_intervals_collect (ci, li, 0.0, G->roottime);

  /* Derive parameters from a partial genealogy. */
  IMA_genealogy_derivetheta ('o', ci, li, z1);
  gk = &saGiold[li];
  gkz1 = &saGioldz1[li];

  nsim = 0;
simulation:
  nsim++;

#ifdef DEBUG
  if (L[li].model != INFINITESITES)
    {
      assert (L[li].model == HKY || L[li].model == STEPWISE);
      assert (nsim == 1);
    }
#endif /* DEBUG */

  /* Prepare the number of lineages with which active lineages can coalesce. */
  IMA_sbf_lineages (&sbfold, ci, li, z1);

  /* Initialize active lineages. */
  m1->dnt = -1.0;
  m1->fpop = -1;
  m1->mpall = 0;
  m1->mig[0].mt = -1.0;
  m1->mig[0].mp = -1;
  sbfnew.sis1 = -1;
  sbfnew.m1 = NULL; /* sbfnew.m1 is the same as &sanems[2 * li]    */
  sbfnew.m3 = NULL; /* but we do not know this is null or not yet. */
  sbfnew.is_finiterootbranch = 'F';
  sbfnew.canz31 = 'F'; /* both canz31 must be the same? */
  sbfold.canz31 = 'F';
  sbfold.m3 = NULL; /* but we do not know this is null or not yet. */
  is_coalesced1 = 'F';

  ti = 0;
  prevt = 0.0;
  ki = 0;
  pc1 = gtree[z1].pop;
  assert (pc1 == newpop);

  /* Start simulation down towards the genealogy. */
  ek = sbfold.e1; 
  while (is_coalesced1 == 'F')
    {
      dt = ek->t - prevt;
      assert (dt > 0.0);
      while (dt > 0.0 && is_coalesced1 == 'F')
        {
          np = bfmove_ratesI (&rates, gk, gkz1,
                              &sbfold, ki,
                              pc1);
          totalrate = 0.0;
          for (pj = 0; pj < np; pj++)
            {
              assert (!(rates[pj] < 0.0));
              totalrate += rates[pj];
            }
          if (totalrate > 0.0)
            {
              u = uniform ();
              rt = -log (u) / totalrate;
              dt -= rt; 
            }
          else if (totalrate == 0.0)
            {
              /* assert (0); */
              /* assert (0); I do not know when this would happen: Answer -
               * This can happen when there is no lineage to attach an
               * active lineage in the last period. I am not sure whether
               * this could continue to happen once we have zero total rate.
               * If that is the case, we may let this happen and resimulate
               * once we get to the beyond of the bottom of a partial
               * genealogy. */
              dt = -1.0;
            }
          else
            {
              assert (0);
            }
          if (dt < 0)
            {
              /* move to next interval: we hit and exit the current while,
               * then we increase ti and ek */
              continue;
            }
          else
            {
              newct = ek->t - dt;
            }
          re = ran_discretea (np, rates);

          /* Case (A). Only one lineage z1 coalesces. */
          if ((np == 1 && is_coalesced1 == 'F')
              || (np == 2 && is_coalesced1 == 'F' && re == 0))
            {
              is_coalesced1 = 'T';
              /* We need a list of edges: n and l.                     */
              /* We computes discrete probabilities for all the edges. */
              i = bfmove_chooseAnEdge (ci, li, &sbfold, newct, pc1);
              if (L[li].model == INFINITESITES)
                {
                  sbfnew.sis1 = sbfold.lz1[pc1][i];
                }
              else if (L[li].model == HKY || L[li].model == STEPWISE)
                {
                  sbfnew.sis1 = sbfold.l[pc1][i];
                }
              else
                {
                  assert (0);
                }
              /* sbfnew.down1 = sbfold.down1; */
              sbfnew.m1 = m1;
              m1->mig[m1->mpall].mt = -1.0;
              m1->dnt = newct;
              m1->fpop = pc1;
            }
          /* Case (F). Lineage z1 migrates */
          else if (np == 2 && is_coalesced1 == 'F' && re == 1)
            {
              IMA_choose_migtopop (&ri, ki, pc1, gk);
              assert (pc1 != ri);
              pc1 = ri;
              mi = m1->mpall;
              m1->mig[mi].mt = newct;
              m1->mig[mi].mp = pc1;
              m1->mpall++;
            }
          else
            {
              assert (0);
            }
        }
      if (is_coalesced1 == 'T')
        {
          break;
        }

      /* New time is further back in time than the bottom of an event. */
      bfmove_nextI (ci, li, 0.0, ek, np, rates, 
                    NULL, &sbfold, /* &sbfnew, */
                    &ki,
                    z1,
                    &is_coalesced1,
                    &pc1);
      prevt = ek->t;
      ti++;
      if (ti < sbfold.n1)
        {
          ek++;
        }
      else
        {
          break;
        }
    }

  /* Simulation beyond the bottom of a genealogy. */
  sbfnew.m3 = NULL;
  if (is_coalesced1 == 'F') 
    {
      assert (sbfold.n1 == ti);
      /* Should we do two different actions for the last event type? */
      assert (sbfold.e1[ti - 1].type == 'c' 
              || sbfold.e1[ti - 1].type == 'e');
      assert (sbfold.e1[ti - 1].t == prevt);
      assert (findperiodoftime (ci, prevt) == ki);

      is_coalesced3 = 'F';
      newct = prevt;
      pc3 = sbfold.e1[ti - 1].pj;
      assert (!(pc1 < 0));
      assert (!(pc3 < 0));
      assert (sbfold.e1[ti - 1].ei == G->root);
      z3 = G->root;
      m3 = &sansms[2 * li];
      m3->li = li;
      m3->edgeid = z3;
      m3->upt = prevt;
      m3->pop = pc3;

      sbfold.canz31 = 'T';
      if (L[li].model == INFINITESITES
          && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
        {
          for (si = 0; si < L[li].numsites; si++)
            {
              if (G->mut[si] == z3)
                {
                  continue;
                }
              if (gtree[z3].seq[si] != sbfold.seqz1[si])
                {
                  sbfold.canz31 = 'F';
                  break;
                }
            }
        }
      else if (L[li].model == HKY || L[li].model == STEPWISE)
        {
          /* No code: we should attach an active lineage. */
        }

      nsum = IMA_sbf_nsum (&sbfold, pc1, pc3);
      assert (nsum == 2); /* Do not consider diploid data type. */
      if (is_coalesced1 == 'F' && sbfold.canz31 == 'F')
        {
          m1->dnt = -1.0;
          m1->fpop = -1;
          m1->mpall = 0;
          m1->mig[0].mt = -1.0;
          goto simulation;
        }
      assert (sbfold.canz31 == 'T');

      /* ki is set before. */
      while (nsum == 2)
        {
          assert (is_coalesced1 == 'F');
          assert (is_coalesced3 == 'F');
          assert (pc1 < 2 * npops - 1);
          assert (!(pc1 < 0));
          assert (pc3 < 2 * npops - 1);
          assert (!(pc3 < 0));
          pc1 = saC.popndown[pc1][ki];
          pc3 = saC.popndown[pc3][ki];

          bfmove_ratesIII (&rates, gk, &sbfold, ki, pc1, pc3);
          nr = 3;
          totalrate = 0.0;
          for (ri = 0; ri < nr; ri++)
            {
              totalrate += rates[ri];
            }
          assert (totalrate > 0.0);
          u = uniform ();
          rt = -log (u) / totalrate;
          prevki = ki;
          newct = newct + rt;
          ki = findperiodoftime (ci, newct);
          if (prevki == ki)
            {
              /* No Code */
            }
          else if (prevki < ki)
            {
              /* ki can be larger than prevki by more than one. */
              newct = C[ci]->tvals[prevki];
              ki = prevki + 1;
              continue;
            }
          else
            {
              assert (0);
            }
          re = ran_discretea (nr, rates);
          switch (re)
            {
            case 0:
              IMA_choose_migtopop (&ri, ki, pc1, gk);
              pc1 = ri;
              mi = m1->mpall;
              m1->mig[mi].mt = newct;
              m1->mig[mi].mp = pc1;
              m1->mpall++;
              break;
            case 1:
              IMA_choose_migtopop (&ri, ki, pc3, gk);
              pc3 = ri;
              mi = m3->mpall;
              m3->mig[mi].mt = newct;
              m3->mig[mi].mp = pc3;
              m3->mpall++;
              break;
            case 2: /* Lineages z1 coalesces with z3. */
              is_coalesced3 = 'T';
              is_coalesced1 = 'T';
              /* What are all these about? */
              sbfnew.sis1 = z3;
              /* sbfnew.down1 = sbfold.down1; */
              sbfnew.m1 = m1;
              m1->mig[m1->mpall].mt = -1.0;
              m1->dnt = newct;
              m1->fpop = pc1;
              sbfnew.m3 = m3;
              m3->mig[m3->mpall].mt = -1.0;
              m3->dnt = newct;
              m3->fpop = pc3;
              assert (pc1 == pc3);
              bfmove_chooseA1 (ci, li, &sbfnew);
              nsum = 1;
              break;
            }
        }
    }

  if (sbfold.is_finiterootbranch == 'F')
    {
      assert (sbfold.m3 == NULL);
    }
  /* Get [[sbfnew]] and [[saGinew]]. */
  IMA_intervals_collectsbfnew (ci, li, sbfnew.m3);
  IMA_genealogy_derivetheta ('n', ci, li, z1);
  if (L[li].model == INFINITESITES)
    {
      /* No code. */
    }
  else if (L[li].model == HKY || L[li].model == STEPWISE)
    {
      /* FIXME: What if there is no or single inactive lineage? */
      bfmove_selectAnEdge (ci, li, &sbfnew, &sbfold);
      /* bfmove_selectA1 (ci, li, &sbfold); */
    }

  /* Compute Hasting ratio. */
  /* G     -> Gp     -> Gstar */
  lqstar = bfq (&sbfold, &sbfnew, ci, li, gk, gkz1);
  /* Gstar -> Gstarp -> G     */
  lq = bfq (&sbfnew, &sbfold, ci, li, &saGinew[li], &saGinewz1[li]);
  lhweight += (lq - lqstar);

  /* Reattach the branch or branches. */
  down1 = gtree[sbfnew.m1->edgeid].down;
  if (sbfnew.m3 != NULL)
    {
      assert (sbfnew.sis1 == G->root);
      IMA_genealogy_appendmig (ci, li, 
                               sbfnew.sis1, sbfnew.m3, 
                               sbfnew.m1->edgeid, down1);
    }
  else
    {
      IMA_genealogy_bisectsis (ci, li, 
                               sbfnew.sis1, sbfnew.m1->edgeid, 
                               down1, 
                               sbfnew.m1, 
                               sbfnew.seqz1);
    }
  IMA_genealogy_copymig (ci, li, sbfnew.m1->edgeid, sbfnew.m1);
  gtree[G->root].time = TIMEMAX;
  gtree[G->root].down = -1;
  gtree[G->root].mig[0].mt = -1.0;

  /* Update locus information. */ 
  gtree[z1].exist = 'T';
  down1 = gtree[z1].down;
  gtree[down1].exist = 'T';
  G->roottime = imaGtreeTime (ci, li, G->root); 
  sbfold.n1 = 0;
  sbfold.n2 = 0;
  sbfold.n3 = 0;
  sbfnew.n1 = 0;
  sbfnew.n2 = 0;
  sbfnew.n3 = 0;

  if (L[li].model == INFINITESITES 
      && calcoptions[DONTCALCLIKELIHOODMUTATION] == 0)
    {
      i = IMA_genealogy_findIntSeq (ci, li);
      assert (i == 1);
    }
  else if (L[li].model == HKY) 
    {
      /* No code. */
    }
  else if (L[li].model == STEPWISE)
    {
      /* No code. */
      assert (sbfnew.A1 >= L[li].minA[0] && sbfnew.A1 <= L[li].maxA[0]);
      gtree[down1].A[0] = sbfnew.A1;
    }
  /* printf ("[%d] root time - %lf\n", step, G->roottime);  */

  return lhweight;
}



int
updateassignmenttpt (int ci)
{
  int bnocoal;
  int rperiod; 
  int accp;
  double lweight;
  double logU;
  double mweight;
  double hweight;
  struct genealogy *G;
  int li;
  int popx;
  int popa;
  int popb;

  assert (assignmentoptions[POPULATIONASSIGNMENTINFINITE] == 0);
  if (npops != 3)
    {
      IM_errloc (AT, 
                 "Three point update is implemented only for three populations");
    }

  /* Count up the number of tries of MCMC cycles. */
  if (ci == 0)
    {
      for (li = 0; li < nloci; li++)
        {
          L[li].a_rec->upinf[IM_UPDATE_ASSIGNMENT_RELABEL].tries++;
        }
    }
  Cupinf[ci].upinf[IM_UPDATE_ASSIGNMENT_RELABEL].tries++;

  /* Check if there is any coalescent event in period 1. */
  rperiod = 1; 
  bnocoal = tptExistCoalescentPeriod (ci, rperiod);
  
  /**************************************************************************/
  /* This is not good one but we exit the function when there is a coalescent
   * event in the middle period. */
  /**************************************************************************/
  if (bnocoal == 1)
    {
      return 0;
    }

  G = NULL;
  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      assertgenealogy (ci);
    }
  /* We can update the population tree of chain ci. */ 
  /* Choose one of two ingroup populations. */
  /* Relabel ... */
  tptChooseAnIngroup (ci, rperiod, &popx, &popa);
  popb = tptOutgroup (ci, rperiod);

  /* Save the current state of a chain */
  for (li = 0; li < nloci; li++)
    {
      imaSavedReset (li);
      imaSavedSaveLocusInfo (ci, li);
      copy_treeinfo (&saT[li].savedgweight, &C[ci]->G[li].gweight);
    }
  copy_treeinfo (&saC.savedallgweight, &C[ci]->allgweight);
  /* We do not call copy_probcalc_all because we do not change likelihood. */
  copy_probcalc (&saC.savedallpcalc, &C[ci]->allpcalc);

  /* Propose a new state of genealogy and assignment. */
  hweight = tptmove (ci, rperiod, popx, popa, popb, 3);

  /* Compute Metropolis-ratio. */
  setzero_genealogy_weights (&C[ci]->allgweight);
  for (li = 0; li < nloci; li++)
    {
      setzero_genealogy_weights (&C[ci]->G[li].gweight);
      treeweight (ci, li);
      sum_treeinfo (&C[ci]->allgweight, &C[ci]->G[li].gweight);
    }
  initialize_integrate_tree_prob (ci, &C[ci]->allgweight, &C[ci]->allpcalc);

  logU = log (uniform ());
  mweight = C[ci]->allpcalc.probg - saC.savedallpcalc.probg;
  lweight = beta[ci] * mweight + hweight;
  assert (beta[ci] * mweight + hweight > -1e200
          && beta[ci] * mweight + hweight < 1e200);

/* check values of acceptance probability: 
  printf ("pdg: %.3lf - %.3lf (%.3lf), pg: %.3lf - %.3lf (%.3lf), hweight: %.3lf, lweight: %.3lf vs. logU: %.3lf, [%d]\n", 
          C[ci]->allpcalc.pdg, saC.savedallpcalc.pdg, C[ci]->allpcalc.pdg - saC.savedallpcalc.pdg,
          C[ci]->allpcalc.probg, saC.savedallpcalc.probg, C[ci]->allpcalc.probg - saC.savedallpcalc.probg,
          hweight, lweight, logU, lweight > logU);
*/
 
  accp = 0;

  if (lweight > 0.0 || lweight > logU)
    {
      /* accept the update */
      accp = 1;
      if (ci == 0)
        {
          for (li = 0; li < nloci; li++)
            {
              L[li].a_rec->upinf[IM_UPDATE_ASSIGNMENT_RELABEL].accp++;
            }
        }
      Cupinf[ci].upinf[IM_UPDATE_ASSIGNMENT_RELABEL].accp++;
      SWAP (C[ci]->nasn[popa], C[ci]->nasn[popb]);

      /* Always revert to an original. */
      for (li = 0; li < nloci; li++)
        {
          if (L[li].model == HKY)
            {
              copyfraclike (ci, li);
              storescalefactors (ci, li);
            }
        }
    }
  else
    {
      for (li = 0; li < nloci; li++)
        {
          /* restore edges changed during update */
          imaSavedRestore (ci, li); 
          imaSavedRestoreLocusInfo (ci, li);
          copy_treeinfo (&C[ci]->G[li].gweight, &saT[li].savedgweight);
        }
      copy_treeinfo (&C[ci]->allgweight, &saC.savedallgweight);
      copy_probcalc (&C[ci]->allpcalc, &saC.savedallpcalc);
    }

  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == HKY)
        {
          restorescalefactors (ci, li);
        }
    }

  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      assertgenealogy (ci);
    }
  return accp;
}

int 
tptExistCoalescentPeriod (int ci, int rperiod)
{
  int v;
  double upt;
  double dnt;
  int li;
  int ei;
  double t;
 
  assert (assignmentoptions[POPULATIONASSIGNMENTINFINITE] == 0);

  if (rperiod == 0)
    {
      upt = 0.0;
    }
  else
    {
      upt = C[ci]->tvals[rperiod - 1];
    }
  dnt = C[ci]->tvals[rperiod]; 
  /* tvals is an array of size npops. */

  v = 0; /* FALSE */ 
  for (li = 0; li < nloci; li++)
    {
      for (ei = 0; ei < L[li].numlines; ei++)
        {
          t = imaGtreeTime (ci, li, ei);
          if (upt < t && t < dnt)
            {
              v = 1;
              li = nloci;
              break;
            }
        }
    }

  return v;
}

int 
tptChooseAnIngroup (int ci, int rperiod, int *popx, int *popa)
{
  int v;
  struct popedge *ptree;

  assert (rperiod == 1);

  ptree = C[ci]->poptree;
  if (bitran () /*uniform() < 0.5 */ )
    {
      *popx = ptree[3].up[1];
      *popa = ptree[3].up[0];
    }
  else
    {
      *popx = ptree[3].up[0];
      *popa = ptree[3].up[1];
    }
  v = *popa;
  return v;
}

int 
Phylogeny_tptChooseAnIngroup (int ci, int rperiod, int *popx, int *popa)
{
  assert (0 < rperiod && rperiod < npops - 1);
  if (bitran ())
    {
      *popx = C[ci]->droppops[rperiod][0];
      *popa = C[ci]->droppops[rperiod][1];
    }
  else
    {
      *popx = C[ci]->droppops[rperiod][1];
      *popa = C[ci]->droppops[rperiod][0];
    }
  return C[ci]->addpop[rperiod];
}

int 
tptOutgroup (int ci, int rperiod)
{
  int v;
  struct popedge *ptree;

  assert (rperiod == 1);

  ptree = C[ci]->poptree;
  if (ptree[4].up[0] < npops)
    {
      v = ptree[4].up[0];
    }
  else
    {
      v = ptree[4].up[1];
    }
  return v;
}

int 
Phylogeny_sister (int ci, int pi)
{
  int v;
  struct popedge *ptree;
  int down;

  ptree = C[ci]->poptree;
  down = ptree[pi].down;
  assert (ptree[pi].numup == 2);
  assert (npops <= down && down < 2 * npops - 1); /* down must be an internal edge. */
  assert (-1 < ptree[down].up[0] && ptree[down].up[0] < 2 * npops - 1);
  assert (-1 < ptree[down].up[1] && ptree[down].up[1] < 2 * npops - 1);

  if (ptree[down].up[0] == pi)
    v = ptree[down].up[1];
  else
    v = ptree[down].up[0];
  return v; 
}
double
tptmove (int ci, int rperiod, int popx, int popa, int popb, int popc)
{
  double lhweight;
  int li;
  int ei;
  int ngnodes;
  double upt;
  double dnt;

  lhweight = 0.0;

  if (rperiod == 0)
    {
      upt = 0.0;
    }
  else
    {
      upt = C[ci]->tvals[rperiod - 1];
    }
  dnt = C[ci]->tvals[rperiod]; 

  /* tvals is an array of size npops. */


  for (li = 0; li < nloci; li++)
    {
      ngnodes = L[li].numlines;
      for (ei = 0; ei < ngnodes; ei++)
        {
          imaSavedSaveBranch (ci, li, ei);
          imaGtreeSwap (ci, li, ei, popx, popa, popb, popc, upt, dnt);
        }
    }
  
  return lhweight;
}
int
imaGtreeSwap (int ci, int li, int ei, 
              int popx, int popa, int popb, int popc,
              double upt, double dnt)
{
  int n;
  int prevpop;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;
  
  assert (!(ci < 0));
  assert (!(li < 0));
  
  G = &C[ci]->G[li];
  gtree = &G->gtree[ei];

  /* Change population label at the top of an edge. */
  if (gtree->pop == popa)
    {
      gtree->pop = popb;
    }
  else if (gtree->pop == popb)
    {
      gtree->pop = popa;
    }

  n = 0;
  if (gtree->cmm > 0)
    {
      while (gtree->mig[n].mt > -0.5)
        {
          if (upt < gtree->mig[n].mt && gtree->mig[n].mt < dnt)
            {
              /* Figure out the population label at starting points of disappearing
               * ancestral population. If a lineage passes from popx, then we do
               * nothing about the labelling of migration events of the lineage in
               * the [[rperiod]]. If a lineage passes from popa or popb, then we
               * have to relabel populations of migration events of the lineages. 
               **/

              /* If population label of a just previous event is popx, then we
               * break this while loop. */
              if (n == 0)
                {
                  prevpop = gtree->pop;
                }
              else
                {
                  prevpop = gtree->mig[n-1].mp;
                }
              if (prevpop == popx)
                {
                  break;
                }

              assert (gtree->mig[n].mp != popa);
              if (gtree->mig[n].mp == popc)
                {
                  gtree->mig[n].mp = popb;
                }
              else if (gtree->mig[n].mp == popb)
                {
                  gtree->mig[n].mp = popc;
                }
            }
          else
            {
              assert (gtree->mig[n].mp != popc);
              if (gtree->mig[n].mp == popa)
                {
                  gtree->mig[n].mp = popb;
                }
              else if (gtree->mig[n].mp == popb)
                {
                  gtree->mig[n].mp = popa;
                }
            }
          n++; 
        }
      assert (!(n < 0));
    }
  return n;
}

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
imaAssignmentReadMeanAssignment (char *fname, int n, int *A)
{
  FILE *fp;
  int i, d;
  
  fp = fopen (fname, "r");
  for (i = 0; i < n; i++)
    {
      d = read_int (fp);
      A[i] = d;
    }
  fclose (fp);
  fp = NULL;
 
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

void
imaStructuramaCreateInFile (char *structurama, int byAllele)
{
  int ii;
  int li;
  int ei;
  int di;
  char *indname;
  int igenotype1;
  int igenotype2;
  FILE *inStructurama;
  inStructurama = NULL;
  int numii;
  int numloci;
  char strName[FNSIZE];

  inStructurama = fopen (structurama, "w");
  fprintf (inStructurama, "#NEXUS\n\n");
  fprintf (inStructurama, "[ Generated by IMamp ]\n\n");
  fprintf (inStructurama, "begin data;\n");

  if (byAllele == 0)
  {
    numii = snind;
    numloci = nloci;
  }
  else
  {
    numii = total_numgenes;
    numloci = 1;
  }
  fprintf (inStructurama, "\tdimensions nind=%d nloci=%d;\n", numii, numloci);
  fprintf (inStructurama, "\tinfo\n");

  if (byAllele == 0)
    {
      for (ii = 0; ii < numii; ii++)
        {
          indname = IMA_getIndividualName (ii);
          fprintf (inStructurama, "\t%s\t", indname);
          for (li = 0; li < numloci; li++)
            {
              di = IMA_getGenotype (ii, li, &igenotype1, &igenotype2);
              switch (di)
                {
                case 0:
                  fprintf (inStructurama, "( ? , ? ) ");
                  break;
                case 1: 
                  fprintf (inStructurama, "( %d , ? ) ", igenotype1);
                  break;
                case 2: 
                  fprintf (inStructurama, "( %d , %d ) ", igenotype1, igenotype2);
                  break;
                default:
                  assert (0);
                }
            }
          fprintf (inStructurama, ",\n");
        }
    }
  else
    {
      for (li = 0; li < nloci; li++)
        {
          for (ei = 0; ei < L[li].numgenes; ei++)
            {
              imaStringRemoveSpaces (strName, L[li].gNames[ei]);
              fprintf (inStructurama, "\t%s_%04d\t", strName, li);
              fprintf (inStructurama, "( 1 ) ");
              fprintf (inStructurama, ",\n");
            }
        }
    }
  fprintf (inStructurama, "\t;\n");
  fprintf (inStructurama, "end;\n\n");

  fclose (inStructurama);
  inStructurama = NULL;

  return;
}
char*
IMA_getIndividualName (int iind)
{
  int ci;
  int li;
  int ei;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL; /* genealogy of locus li */

  ci = 0;
  for (li = 0; li < nloci; li++)
    {
      G = &(C[ci]->G[li]);
      gtree = G->gtree;
      for (ei = 0; ei < L[li].numgenes; ei++)
        {
          if (gtree[ei].i == iind)
            {
              return L[li].gNames[ei];
            }
        }
    }
  return NULL; 
}
int 
IMA_getGenotype (int ii, int li, int *ig1, int *ig2)
{
  int v;
  int ei;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL; /* genealogy of locus li */

  G = &(C[sci]->G[li]);
  gtree = G->gtree;
  *ig1 = -1;
  *ig2 = -1;
  for (ei = 0; ei < L[li].numgenes; ei++)
    {
      if (gtree[ei].i == ii)
        {
          if (L[li].model == STEPWISE)
            {
              *ig1 = gtree[ei].A[0];
              if (!(L[li].pairs[ei] < 0))
                {
                  *ig2 = gtree[L[li].pairs[ei]].A[0];
                }
            }
          else
            {
              *ig1 = 0;
              if (!(L[li].pairs[ei] < 0))
                {
                  *ig2 = 1;
                }
            }
          break;
        }
    }
  if (*ig1 == -1 && *ig2 == -1)
    {
      v = 0;
    }
  else if (*ig1 != -1 && *ig2 == -1)
    {
      v = 1;
    }
  else if (*ig1 != -1 && *ig2 != -1)
    {
      v = 2;
    }
  else
    {
      assert (0);
    }
  return v;
}
void 
imaStructuramaCreateBatFile (char *fname)
{
  FILE *fp;
  char *batname;
  char *bname;
  int lenbatname;

  fp = NULL;
  batname = NULL;
  lenbatname = strlen (fname) + 4 + 1;
  batname = malloc (lenbatname * sizeof (char));
  sprintf (batname, "%s.bat", fname);

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
  bname = strrchr (fname, '\\');
#else
  bname = strrchr (fname, '/');
#endif
  if (bname == NULL)
    {
      bname = fname;
    }
  else
    {
      bname++;
    }
 
  fp = fopen (batname, "w");
  fprintf (fp, "execute %s.in\n", bname);
  fprintf (fp, "sum\n");
  fprintf (fp, "quit\n");
  fclose (fp);
  fp = NULL;

  free (batname);
  batname = NULL;
  return;
}
void
IMA_convert_IM2Mrbayes (char *mrbayes)
{
  int i;
  FILE *infile;

  infile = fopen (mrbayes, "w");
  
  fprintf (infile, "#NEXUS\n");
  fprintf (infile, "begin data;\n");
  fprintf (infile, "dimensions ntax=%d nchar=1;\n", npops);
  fprintf (infile, "format datatype=dna interleave=no gap=-;\n");
  fprintf (infile, "matrix\n");
  for (i = 0; i < npops; i++)
    {
      fprintf (infile, "%s A\n", popnames[i]);
    }
  fprintf (infile, ";\n");
  fprintf (infile, "end;\n");

  fclose (infile);
  infile = NULL;

  return;
}
void 
IMA_output_mrbayes_bat (char *fname)
{
  FILE *fp;
  char *batname;
  char *bname;
  int lenbatname;

  fp = NULL;
  batname = NULL;
  lenbatname = strlen (fname) + 4 + 1;
  batname = malloc (lenbatname * sizeof (char));
  sprintf (batname, "%s.bat", fname);

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
  bname = strrchr (fname, '\\');
#else
  bname = strrchr (fname, '/');
#endif
  if (bname == NULL)
    {
      bname = fname;
    }
  else
    {
      bname++;
    }
 
  fp = fopen (batname, "w");
  fprintf (fp, "execute %s.nex\n", bname);
  fprintf (fp, "sumt nruns=1\n");
  fprintf (fp, "quit\n");
  fclose (fp);
  fp = NULL;

  free (batname);
  batname = NULL;
  return;
}
int 
IMA_rgs_convert (int *a, int n)
{
  int max;
  int i;
  int *m;
  int e;

  m = malloc (n * sizeof (int));
  memset (m, -1, n * sizeof (int)); 

  max = 1;
  for (i = 0; i < n; i++)
    {
      e = a[i];
      assert (e < n);
      assert (!(e < 0));
      if (m[e] == -1)
        {
          m[e] = max;
          max++;
        }
      a[i] = m[e];
    }
  free (m);
  m = NULL;

  return max;

}
int 
IMA_rgf_setSnasn (int nind, int np)
{
  int v;
  int i;

  if (nind < np)
    {
      IM_err (IMERR_ASN, "There are more populations than individuals");
    }
  v = 0;
  for (i = 0 ; i < np; i++)
    {
      v += IMA_stirlings2 (nind, i + 1);
    }
  return v;
}
void 
IMA_rgf_tickSaasn (int ci)
{
  int li;
  int ii;
  int ei;
  int *a = NULL;
  int *pa;
  int r;

  assert (ci == 0);

  a = malloc (snind * sizeof (int));

  /* how do I know individuals of a locus? */
  pa = a;
  for (ii = 0; ii < snind; ii++)
    {
      li = saC.indlist[ii][0].li;
      ei = saC.indlist[ii][0].ei;
      *pa = C[ci]->G[li].gtree[ei].pop;
      pa++;
    }

  IMA_rgs_convert (a, snind);

  /* Add the number of assignments. */
  r = IMA_rankrgf2 (snind, npops, a);
  assert (r < snasn);
  saasn[r]++;

  free (a);
  a = NULL;
 
  return;
}

void 
IMA_rgf_saveSaasn ()
{
  int i;
  FILE *tfile;
  char *fname;
  int len;

  len = strlen (outfilename);
  fname = malloc ((len + 4 + 1) * sizeof (char));
  sprintf (fname, "%s.pri", outfilename);

  tfile = fopen (fname, "w");
  for (i = 0; i < snasn; i++)
    {
      fprintf (tfile, "%d\n", saasn[i]);
    }
  fclose (tfile);
  tfile = NULL;
  XFREE (fname);
  return;
}

int
IMA_fprintrgf (FILE *fp, int *f, int m)
{
  int i;
  fprintf (fp, "{");
  for (i = 0; i < m; i++)
    {
      if (i < m - 1)
        {
          fprintf (fp, "%2d ", f[i]);                
        }
      else
        {
          fprintf (fp, "%2d}", f[i]);
        }
    }
  fprintf (fp, "\n");
  return 0;
}

int
IMA_generatergf (int m, int n)
{
  int *f;
  int *fm;
  int i;
  int j;
  char done;
  int maxn;
  int rank;
  
  rank = 0;
  f = malloc (m * sizeof (int));
  fm = malloc (m * sizeof (int));
  for (i = 0; i < m; i++)
    {
      f[i] = 1;
      fm[i] = 2;
    }
  done = 'N';
  while (done == 'N')
    {
      fprintf (stdout, "[%3d] ", rank);
      rank++;
      IMA_fprintrgf (stdout, f, m);          
      j = m;
      do
        {
          j = j - 1;
        } while (f[j] == fm[j]);
      if (j > 0)
        {
          f[j] = f[j] + 1;
          for (i = j + 1; i < m; i++)
            {
              f[i] = 1;
              if (f[j] == fm[j])
                {
                  if (fm[j] < n - 1)
                    {
                      maxn = fm[j];
                    }
                  else
                    {
                      maxn = n - 1;
                    }
                  fm[i] = maxn + 1;
                }
              else
                {
                  fm[i] = fm[j];
                }
            }
        }
      else
        {
          done = 'Y';
        }
    }
  
  free (f);
  free (fm);
  f = NULL;
  fm = NULL;
  return 0;
}

int**
IMA_generalizedrgf (int m)
{
  return IMA_generalizedrgf2 (m, m);
}

int**
IMA_generalizedrgf2 (int m, int n)
{
  int j;
  int i;
  int **d;
  int M;
  int N;
  
  M = m + 1;
  N = n + 1;
  d = malloc (M * sizeof (int *));
  for (i = 0; i < M; i++)
    {
      d[i] = malloc (M * sizeof (int));
      memset (d[i], 0, M * sizeof (int));
    }
  for (j = 0; j < N; j++)
    {
      d[0][j] = 1;
    }
  for (i = 1; i < M; i++)
    {
      for (j = 0; j < M - i; j++)
        {
          d[i][j] = j * d[i-1][j] + d[i-1][j+1];
        }
    }
  return d;
}

int
IMA_freejrgf (int ***d, int m)
{
  int i;
  int M;
  
  M = m + 1;
  for (i = 0; i < M; i++)
    {
      free ((*d)[i]);
      (*d)[i] = NULL;
    }
  free (*d);
  *d = NULL;
  return 0;
}

int
IMA_fprintjrgf (FILE *fp, int **d, int m)
{
  int i;
  int j;
  int M;
  
  M = m + 1;

  fprintf (fp, "  i | ");
  for (j = 0; j < M; j++)
    {
      fprintf (fp, "%4d ", j);
    }
  fprintf (fp, "\n");
  fprintf (fp, "-----");
  for (j = 0; j < M; j++)
    {
      fprintf (fp, "-----");
    }  
  fprintf (fp, "\n");
  
  for (i = 0; i < M; i++)
    {
      fprintf (fp, "%3d | ", i);
      for (j = 0; j < M - i; j++)
        {
          fprintf (fp, "%4d ", d[i][j]);
        }
      fprintf (fp, "\n");
    } 
  return 0;
}

int
IMA_rankrgf (int m, int *f)
{
  return IMA_rankrgf2 (m, m, f);
}

int
IMA_rankrgf2 (int m, int n, int *f)
{
  int r;
  int j;
  int i;
  int **d;
  int M;
  
  for (i = 0; i < m; i++)
    {
      if (f[i] > n)
        {
          printf ("INVALID RGFs - There are elements larger than %d\n", n);
          return -1;
        }
    }
  M = m + 1;
  d = IMA_generalizedrgf2 (m, n);
  r = 0;
  j = 1;
  for (i = 2; i < M; i++)
    {
      r = r + (f[i-1] - 1) * d[m-i][j];
      if (j < f[i-1])
        {
          j = f[i-1];
        }
    }
  IMA_freejrgf (&d, m);  
  return r;
}

int
IMA_unrankrgf (int *f, int r, int m)
{  
  IMA_unrankrgf2 (f, r, m, m);
  return 0;
}

int
IMA_unrankrgf2 (int *f, int r, int m, int n)
{
  int j;
  int i;
  int **d;
  int M;
  
  M = m + 1;
  d = IMA_generalizedrgf2 (m, n);
  j = 1;
  for (i = 2; i < M; i++)
    {
      if (j * d[m-i][j] <= r)
        {
          f[i-1] = j + 1;
          r = r - j * d[m-i][j];
          j = j + 1;
        }
      else
        {
          f[i-1] = r / d[m-i][j] + 1;
          r = r % d[m-i][j];
        }
    }
  IMA_freejrgf (&d, m);
  return 0;
}

int
IMA_stirlings2 (int m, int n)
{
  int i;
  int j;
  int M;
  int N;
  int min;
  int **s;
  int v;
  
  M = m + 1;
  N = n + 1;
  
  s = malloc (M * sizeof (int *));
  for (i = 0; i < M; i++)
    {
      s[i] = malloc ((M + 1) * sizeof (int));
    }
  
  s[0][0] = 1;
  for (i = 1; i < M; i++)
    {
      s[i][0] = 0;
    }
  for (i = 0; i < M; i++)
    {
      s[i][i+1] = 0;
    }
  for (i = 1; i < M; i++)
    {
      if (i < n)
        {
          min = i;
        }
      else
        {
          min = n;
        }
      for (j = 1; j < min + 1; j++)
        {
          s[i][j] = j * s[i-1][j] + s[i-1][j-1];
        }
    }
  v = s[m][n];
  
  for (i = 0; i < M; i++)
    {
      free (s[i]);
      s[i] = NULL;
    }
  free (s);
  s = NULL;
  
  return v;
}

int
IMA_maxrgf (int *f, int m)
{
  int i;
  int maxn;
  maxn = 0;
  for (i = 0; i < m; i++)
    {
      if (maxn < f[i])
        {
          maxn = f[i];
        }
    }
  return maxn;
}
void 
IMAgenealogyAlign (char *fname, int n)
{
  int i;
  int j;
  int k;
  int l;
  int nsispop;
  int **sispop;
  int *od;
  char *pname;
  int len;
  int *tasn;
  int *easn;
  int nrow;
  int ncol;
  int **m;
  int ncombsis;
  char cb[7];
  int d;
  int d2;
  int uc; /* unsigned char */
  int v;

  /***************************************************************************/
  /* Align genealogies by swapping elements in [[gsampinf]].                 */ 
  /***************************************************************************/
  len = strlen (fname);
  pname = (char *) malloc ((len + 30) * sizeof (char)); /* .in.sum_assignments */
  sprintf (pname, "%s.in.sum_assignments", fname);

  /* Find a point estimate of assignment parameter. */
  tasn = malloc (snind * sizeof (int));
  if (assignmentoptions[PRINTSTRUCTURAMA] == 0)
    {
      imaAssignmentReadMeanAssignment (pname, snind, tasn);
    }
  else
    {
      IMA_io_readsumassignment (pname, snind, tasn);
    }

  for (i = 0; i < snind; i++)
    {
      assert (tasn[i] > 0); 
      tasn[i] = tasn[i] - 1;
    }

  /* Read all posterior samples of assignment parameter. */
  sprintf (pname, "%s.asn", fname);
  IMA_io_readp (pname, &m, &nrow, &ncol, 0);
  /* This may be different when there are multiple MCMC ouput files. */
  assert (ncol == snind);
  assert (nrow == n);  

  if (!(npops < 13))
    {
      IM_errloc (AT, 
                 "Island model: number of permutations will be orveflow: %d!",
                 npops);
    }
  if (assignmentoptions[POPULATIONASSIGNMENTINFINITE] == 0)
    {
      if (npops == 2 || npops == 3)
        {
          easn = malloc (snind * sizeof (int));
          nsispop = imaPtreeNsispop (0);
          if (nsispop < 1 || nsispop > 7)
            {
              IM_err (IMERR_ASN, 
                      " number (%d) of present-day sister populations is in between 1 and 7",
                      nsispop);
            }
          sispop = (int **) malloc (nsispop * sizeof (int *));
          for (j = 0; j < nsispop; j++)
            {
              sispop[j] = (int *) malloc (2 * sizeof (int));
            }
          imaPtreeSispop (0, sispop);
          ncombsis = 1 << nsispop;

          /* Compare two assignments: n is the number of pairs of genealogy and
           * assignment. */
          for (i = 0; i < n; i++)
            {
              /* 2^^#sisters many iterations */
              d = snind;
              for (j = 0; j < ncombsis; j++)
                {
                  /* We may not need cb.          */
                  /* It is more readable, though. */
                  uc = j;
                  for (k = 0; k < nsispop; k++)
                    {
                      if (uc & 1<<k)
                        {
                          cb[k] = '1';
                        }
                      else
                        {
                          cb[k] = '0';
                        }
                    }

                  /* Find the number of individuals to be removed. */
                  memcpy (easn, m[i], snind * sizeof (int));
                  for (k = 0; k < nsispop; k++)
                    {
                      if (cb[k] == '1')
                        {
                          for (l = 0; l < snind; l++)
                            {
                              if (easn[l] == sispop[k][0]) 
                                {
                                  easn[l] = sispop[k][1];
                                }
                              else if (easn[l] == sispop[k][1])
                                {
                                  easn[l] = sispop[k][0];
                                }
                            }
                          
                        }
                    }
                  /* Now, easn is a permutation. */

                  d2 = IMA_gdistanceSimple (snind, tasn, easn);

                  if (d2 < d)
                    {
                      d = d2; /* A smaller distance is found.  */
                      v = j;  /* The index of the permutation. */
                    }
                }
              /* Now, we know the permuted asssignment that gives a smallest
               * distance. We now swap genealogies. */

              /* v-th permutation. */
              uc = v;
              for (k = 0; k < nsispop; k++)
                {
                  if (uc & 1<<k)
                    {
                      cb[k] = '1';
                    }
                  else
                    {
                      cb[k] = '0';
                    }
                }
              for (j = 0; j < nsispop; j++)
                {
                  if (cb[j] == '1')
                    {
                      IMA_gsampinf_swap (i, sispop[j][0], sispop[j][1]);
                    }
                }
            }
          XFREE (easn);
          for (i = 0; i < nsispop; i++)
            {
              XFREE (sispop[i]);
            }
          XFREE (sispop);
        }
      else if (3 < npops && npops < 13)
        {
#ifdef COMMENT
          IM_errloc (AT, 
                     "Not yet implemented");
#endif
        }
      else
        {
          IM_errloc (AT, 
                     "Tree model: num. popn. must be less than 13 (npops = %d)",
                     npops);
        }
    }
  else
    {
      for (i = 0; i < snind; i++)
        {
          tasn[i] = tasn[i] + 1;
        }

      od = (int *) malloc (npops * sizeof (int));
      for (i = 0; i < n; i++)
        {
          for (j = 0; j < snind; j++)
            {
              m[i][j] = m[i][j] + 1;
            }
          /* What if m does not have all labels? od 
           */
          IMA_gdistanceOrderNEW (snind, tasn, m[i], npops, od);
          /* od: 1 2 0 3 */
          imaGsampinfRelabel (i, od);
        }
      XFREE (od);
    }

  XFREE (pname);
  XFREE (tasn);
  for (i = 0; i < nrow; i++)
    {
      XFREE (m[i]);
    }
  XFREE (m);

  return; 
}
void
imaGsampinfPermutedG (int gi, float *atree, int *od)
{
  int i;
  int j;
  int pi;
  int pj;

  /* pi: 0 1 2 3
   * pj: 1 2 0 3
   */
  for (pi = 0; pi < npops; pi++)
    {
      pj = od[pi];
      atree[gsamp_ccp + pi] = gsampinf[gi][gsamp_ccp + pj];
      atree[gsamp_fcp + pi] = gsampinf[gi][gsamp_fcp + pj];
      atree[gsamp_hccp + pi] = gsampinf[gi][gsamp_hccp + pj];
      atree[gsamp_qip + pi] = gsampinf[gi][gsamp_qip + pj];
    }

  for (pi = 0; pi < npops - 1; pi++)
    {
      for (pj = pi + 1; pj < npops; pj++)
        {
          i = imaGsampinfFindmig (pi, pj);
          j = imaGsampinfFindmig (od[pi], od[pj]);
          atree[gsamp_mcp + i] = gsampinf[gi][gsamp_mcp + j];
          atree[gsamp_fmp + i] = gsampinf[gi][gsamp_fmp + j];
          atree[gsamp_mip + i] = gsampinf[gi][gsamp_mip + j];

          i = imaGsampinfFindmig (pj, pi);
          j = imaGsampinfFindmig (od[pj], od[pi]);
          atree[gsamp_mcp + i] = gsampinf[gi][gsamp_mcp + j];
          atree[gsamp_fmp + i] = gsampinf[gi][gsamp_fmp + j];
          atree[gsamp_mip + i] = gsampinf[gi][gsamp_mip + j];
        }
    }

  return;
}


void
imaGsampinfRelabel (int gi, int *od)
{
  int i;
  int j;
  int pi;
  int pj;

  float *atree;

  atree = malloc (gsampinflength * sizeof (float));

  /* NOTE: The order is critical.
   * Replace
   * pj: 1 2 0 3
   * with
   * pi: 0 1 2 3
   */
  for (pi = 0; pi < npops; pi++)
    {
      pj = od[pi];
      atree[gsamp_ccp + pj] = gsampinf[gi][gsamp_ccp + pi];
      atree[gsamp_fcp + pj] = gsampinf[gi][gsamp_fcp + pi];
      atree[gsamp_hccp + pj] = gsampinf[gi][gsamp_hccp + pi];
      atree[gsamp_qip + pj] = gsampinf[gi][gsamp_qip + pi];
    }

  for (pi = 0; pi < npops - 1; pi++)
    {
      for (pj = pi + 1; pj < npops; pj++)
        {
          i = imaGsampinfFindmig (pi, pj);
          j = imaGsampinfFindmig (od[pi], od[pj]);
          atree[gsamp_mcp + j] = gsampinf[gi][gsamp_mcp + i];
          atree[gsamp_fmp + j] = gsampinf[gi][gsamp_fmp + i];
          atree[gsamp_mip + j] = gsampinf[gi][gsamp_mip + i];

          i = imaGsampinfFindmig (pj, pi);
          j = imaGsampinfFindmig (od[pj], od[pi]);
          atree[gsamp_mcp + j] = gsampinf[gi][gsamp_mcp + i];
          atree[gsamp_fmp + j] = gsampinf[gi][gsamp_fmp + i];
          atree[gsamp_mip + j] = gsampinf[gi][gsamp_mip + i];
        }
    }

  memcpy (gsampinf[gi], atree, gsampinflength * sizeof (float));
  XFREE (atree);

  return;
}

int 
imaGsampinfFindmig (int pi, int pj)
{
  int i;
  int mi;
  char migstr[10]; /* m0>1 or q0 */

  i = nummigrateparams;
  sprintf (migstr, "m%d>%d", pi, pj);
  assert (strlen (migstr) == 4);
  for (mi = 0; mi < nummigrateparams; mi++)
    {
      if (strcmp(migstr, imig[mi].str) == 0)
        {
          i = mi;
          break;
        }
    }
  assert (i < nummigrateparams);
  return i;
}

void 
IMA_gsampinf_swap (int gi, int pi, int pj)
{
  int i, j;
  int mi;
  char migstr[10]; /* m0>1 or q0 */

  /* All vlaues in gsampinf associated labels pi and pj are swapped. */
  assert (pi < npops && pj < npops);
  assert (!(pi < 0) && !(pj < 0));

  i = pi;
  j = pj;
  SWAP (gsampinf[gi][gsamp_ccp + i], gsampinf[gi][gsamp_ccp + j]); 
  SWAP (gsampinf[gi][gsamp_fcp + i], gsampinf[gi][gsamp_fcp + j]); 
  SWAP (gsampinf[gi][gsamp_hccp + i], gsampinf[gi][gsamp_hccp + j]); 
  SWAP (gsampinf[gi][gsamp_qip + i], gsampinf[gi][gsamp_qip + j]); 

  i = nummigrateparams;
  sprintf (migstr, "m%d>%d", pi, pj);
  assert (strlen (migstr) == 4);
  for (mi = 0; mi < nummigrateparams; mi++)
    {
      if (strcmp(migstr, imig[mi].str) == 0)
        {
          i = mi;
          break;
        }
    }
  assert (i < nummigrateparams);

  j = nummigrateparams;
  sprintf (migstr, "m%d>%d", pj, pi);
  assert (strlen (migstr) == 4);
  for (mi = 0; mi < nummigrateparams; mi++)
    {
      if (strcmp(migstr, imig[mi].str) == 0)
        {
          j = mi;
          break;
        }
    }
  assert (j < nummigrateparams);
  SWAP (gsampinf[gi][gsamp_mcp + i], gsampinf[gi][gsamp_mcp + j]); 
  SWAP (gsampinf[gi][gsamp_fmp + i], gsampinf[gi][gsamp_fmp + j]); 
  SWAP (gsampinf[gi][gsamp_mip + i], gsampinf[gi][gsamp_mip + j]); 

  return;
}

int 
imaPtreeNsispop (int ci)
{
  int i;
  int v;
  struct popedge *ptree;

  ptree = C[ci]->poptree;
  v = 0;
  for (i = npops; i < 2 * npops - 1; i++)
    {
      if (ptree[i].up[0] < npops && ptree[i].up[1] < npops) 
        {
          v++;
        }
    }
  return v;
}

int 
imaPtreeSispop (int ci, int **s)
{
  int i;
  int v;
  struct popedge *ptree;

  ptree = C[ci]->poptree;
  v = 0;
  for (i = npops; i < 2 * npops - 1; i++)
    {
      if (ptree[i].up[0] < npops && ptree[i].up[1] < npops) 
        {
          s[v][0] = ptree[i].up[0];
          s[v][1] = ptree[i].up[1];
          v++;
        }
    }
  return v;
}

void 
imaStructuramaLocalrun (char *fname)
{
  int lencmd;
  char *cmd;
  char *bname;
  char *defaultdir;

  /* Send input files for STRUCTURAMA. */
  /* Make a system command for sending two input files for STRUCTURAMA. */
  lencmd = 2 * strlen (fname) + 100;

  /* Find the default directory based on loadfilebase.
   * /this/directory/a.out -> defaultdir is /this/directory/
   * /a.out                -> /
   * a.out                 -> ./
   * 
   * bname -> a.out
   */
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
  bname = strrchr (fname, '\\');
#else
  bname = strrchr (fname, '/');
#endif
  if (bname == NULL)
    {
      bname = fname;
    }
  else
    {
      bname++;
    }
  imaDirBase (fname, &defaultdir); 
 
  cmd = malloc (lencmd * sizeof (char));
  /* STRUCTURAMA does not seem to handle files with relative paths. We copy
   * files to the current directory. If we run many different jobs on the same
   * directory, we have to use different base names for output files. Otherwise,
   * we overwrite other files. */

  sprintf (cmd, "%s %s.in %s.in", SYSCMDCOPY, fname, bname);
  printf ("%s\n", cmd);
  system (cmd);
  sprintf (cmd, "%s %s.in.p %s.in.p", SYSCMDCOPY, fname, bname);
  printf ("%s\n", cmd);
  system (cmd);
  sprintf (cmd, "%s %s.bat %s.bat", SYSCMDCOPY, fname, bname);
  printf ("%s\n", cmd);
  system (cmd);

  /* Run STRUCTURAMA. */
  /* Need to define DARWIN, LINUX, and 
   * STRUCTURAMA should be given when configure is call
   * For DARWIN: STRUCTURAMA is given with configure or a user must place
   * structurama_1.0 in a place where one can run with full path
   * For LINUX: STRUCTURAMA must be given and wine must be installed
   * For WIN32 or WIN64: Structurama.exe must be installed and its path must be
   * set by the user.
   */
#if defined(DARWIN)
  sprintf (cmd, "%s/structurama_1.0 < %s.bat >> log.txt", STRUCTURAMA, bname);
#elif defined(LINUX)
  sprintf (cmd, "wine %s/Structurama.exe < %s.bat >> log.txt", 
           STRUCTURAMA, bname);
#elif defined(CYGWIN)
  sprintf (cmd, "%s/Structurama.exe < %s.bat >> log.txt", STRUCTURAMA, bname);
#elif defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
  sprintf (cmd, "%s\\Structurama.exe < %s.bat >> log.txt", STRUCTURAMA, bname);
#endif 
  system (cmd);

  sprintf (cmd, 
           "%s %s.in.sum_assignments %s.in.sum_assignments", 
           SYSCMDRENAME, bname, fname);
  printf ("%s\n", cmd);
  system (cmd);
  sprintf (cmd, 
           "%s %s.in.sum_dist.nex %s.in.sum_dist.nex", 
           SYSCMDRENAME, bname, fname);
  printf ("%s\n", cmd);
  system (cmd);
  sprintf (cmd, 
           "%s %s.in.sum_pairs %s.in.sum_pairs", 
           SYSCMDRENAME, bname, fname);
  printf ("%s\n", cmd);
  system (cmd);

  sprintf (cmd, "%s %s.bat", SYSCMDDELETE, bname);
  printf ("%s\n", cmd);
  system (cmd);
  sprintf (cmd, "%s %s.in", SYSCMDDELETE, bname); 
  printf ("%s\n", cmd);
  system (cmd);
  sprintf (cmd, "%s %s.in.p", SYSCMDDELETE, bname); 
  printf ("%s\n", cmd);
  system (cmd);

  free (cmd);
  cmd = NULL;
  free (defaultdir);
  defaultdir = NULL;
  return;
}

/* Export function */
void
imaBarTick ()
{
  int i;
  int ei;
  int li;
  struct genealogy *G;
  struct edge *gtree;

  assert (assignmentoptions[POPULATIONASSIGNMENTASSIGNED] == 1);
  assert (nloci == 1);
  assert (snind == 1);
  for (li = 0; li < nloci; li++)
    {
      G = &(C[0]->G[li]);
      gtree = G->gtree;

      for (i = 0; i < L[li].numgenesunknown; i++)
        {
          ei =  L[li].numgenesknown + i;
          sassigned[i][gtree[ei].pop] += 1.0;
        }
    }
  return;
}

void
imaBarReset ()
{
  int i;
  int j;
  int li;

  assert (assignmentoptions[POPULATIONASSIGNMENTASSIGNED] == 1);
  assert (nloci == 1);
  assert (snind == 1);
  for (li = 0; li < nloci; li++)
    {
      for (i = 0; i < L[li].numgenesunknown; i++)
        {
          for (j = 0; j < npops; j++)
            {
              sassigned[i][j] = 0.0;
            }
        }
    }
  return;
}

void
imaAsnPrintNumGenesPopn_stdout ()
{
  imaAsnPrintNumGenesPopn (stdout);
}

void
imaAsnPrintNumGenesPopn (FILE *fp)
{
  int ii;
  int ei;
  int i;
  int j;
  int li;
  int ngenes[MAXPOPS];
  double sumpops;
  int ci = 0;
  struct edge *gtree = NULL;
  struct genealogy *G = NULL;

  for (i = 0; i < npops; i++)
    {
      ngenes[i] = 0;
    }

  for (ii = 0; ii < snind; ii++)
    {
      li = saC.indlist[ii][0].li;
      ei = saC.indlist[ii][0].ei;
      ngenes[C[0]->G[li].gtree[ei].pop]++;
    } 
     
  fprintf (fp, "Number of genes assigned to populations: ");
  for (i = 0; i < npops; i++)
    {
      fprintf (fp, "[%d] %d\t", i, ngenes[i]);
    }
  fprintf (fp, "\n");
  if (assignmentoptions[POPULATIONASSIGNMENTASSIGNED] == 1)
    {
      assert (nloci == 1);
      li = 0;
      G = &(C[ci]->G[li]);
      gtree = G->gtree;
      fprintf (fp, "Locus [%d]: ", li);
      for (i = L[li].numgenesknown; i < L[li].numgenes; i++)
        {
          fprintf (fp, "gene[%d] -> [%d], ", i, gtree[i].pop);
        }
      fprintf (fp, "\n\n");
      fprintf (fp, "Percentage\n");
      fprintf (fp, "----------\n");

      li = 0;
      for (i = 0; i < L[li].numgenesunknown; i++)
        {
          ei = L[li].numgenesknown + i;
          fprintf (fp, "gene[%d]: ", ei);

          sumpops = 0.0;
          for (j = 0; j < npops; j++)
            {
              sumpops += sassigned[i][j];
            }
          for (j = 0; j < npops; j++)
            {
              fprintf (fp, "[%d] %lf, ", j, sassigned[i][j] / sumpops);
            }
        }
      fprintf (fp, "\n");
    }

  return;
}

void
x1 (im_bfupdate *pbf)
/* IMA_sbf_print (im_bfupdate *pbf) */
{
  int i;
  int j;

  printf ("---------\n");
  printf ("Event e1 [%d]:\n", pbf->n1);
  for (i = 0; i < pbf->n1; i++)
    {
      printf ("[%2d] %c - [p:%2d, pi:%2d, pj:%2d] edge [%2d] at %lf\n", 
              i, 
              pbf->e1[i].type,  
              pbf->e1[i].p,
              pbf->e1[i].pi,
              pbf->e1[i].pj,
              pbf->e1[i].ei,
              pbf->e1[i].t);
    }
  printf ("\n"); 

  printf ("---------\n");
  printf ("Event e2 [%d]:\n", pbf->n2);
  for (i = 0; i < pbf->n2; i++)
    {
      printf ("[%2d] %c - [p:%2d, pi:%2d, pj:%2d] edge [%2d] at %lf\n", 
              i, 
              pbf->e2[i].type,  
              pbf->e2[i].p,
              pbf->e2[i].pi,
              pbf->e2[i].pj,
              pbf->e2[i].ei,
              pbf->e2[i].t);
    }
  printf ("\n"); 
 
  printf ("---------\n");
  printf ("Event e3 [%d]:\n", pbf->n3);
  for (i = 0; i < pbf->n3; i++)
    {
      printf ("[%2d] %c - [p:%2d, pi:%2d, pj:%2d] edge [%2d] at %lf\n", 
              i, 
              pbf->e3[i].type,  
              pbf->e3[i].p,
              pbf->e3[i].pi,
              pbf->e3[i].pj,
              pbf->e3[i].ei,
              pbf->e3[i].t);
    }
  printf ("\n"); 

  printf ("---------\n");
  if (pbf->m1 != NULL)
    {
      printf ("z1: edge [%2d], sis [%2d]\n",
              pbf->m1->edgeid, pbf->sis1);
/* 
      printf ("z1: edge [%2d], sis [%2d], down [%2d], downdown [%2d]\n",
              pbf->m1->edgeid, pbf->sis1, pbf->down1, pbf->downdown1);
      printf ("    upt [%lf], sisupt [%lf], sisdnt [%lf]\n",
              pbf->upt1, pbf->sisupt1, pbf->sisdnt1);
*/
      imaEdgemiginfoPrint (pbf->m1);
      printf ("\n");
    }
  else
    {
      printf ("z1:\n");
    }

/* DIPLOID
  printf ("---------\n");
  if (pbf->m2 != NULL)
    {
      printf ("z2: edge [%2d], sis [%2d], down [%2d], downdown [%2d]\n",
              pbf->m2->edgeid, pbf->sis2, pbf->down2, pbf->downdown2);
      printf ("    upt [%lf], sisupt [%lf], sisdnt [%lf]\n",
              pbf->upt2, pbf->sisupt2, pbf->sisdnt2);
      imaEdgemiginfoPrint (pbf->m2);
      printf ("\n");
    }
  else
    {
      printf ("z2:\n");
    }
*/

  printf ("---------\n");
  if (pbf->m3 != NULL)
    {
      printf ("z3:\n");
      imaEdgemiginfoPrint (pbf->m3);
      printf ("\n");
    }
  else
    {
      printf ("z3:\n");
    }

  printf ("---------\n");
  printf ("lz1:\n");
  for (i = 0; i < numpopsizeparams; i++)
    {
      printf ("[%d] %2d: ", i, pbf->nz1[i]);
      for (j = 0; j < pbf->nz1[i]; j++)
        {
          printf (" %2d", pbf->lz1[i][j]);
        }
      printf ("\n");
    }

/* DIPLOID
  printf ("---------\n");
  printf ("lz2:\n");
  for (i = 0; i < numpopsizeparams; i++)
    {
      printf ("[%d] %2d:", i, pbf->nz2[i]);
      for (j = 0; j < pbf->nz2[i]; j++)
        {
          printf (" %2d", pbf->lz2[i][j]);
        }
      printf ("\n");
    }
*/

  printf ("---------\n");
  printf ("l:\n");
  for (i = 0; i < numpopsizeparams; i++)
    {
      printf ("[%d]: %2d\n", i, pbf->n[i]);
    }
   
  return;
}

int
x2 (int old, int li)
{  
  im_ginfo *gk;

  if (old == 0)
    {
      printf ("saGiold\n");
      printf ("-------\n");
      gk = &saGiold[li];
      x2print (gk);
      printf ("saGioldz1\n");
      printf ("-------\n");
      gk = &saGioldz1[li];
      x2print (gk);
    }
  else
    {
      printf ("saGinew\n");
      printf ("-------\n");
      gk = &saGinew[li];
      x2print (gk);
      printf ("saGinewz1\n");
      printf ("-------\n");
      gk = &saGinewz1[li];
      x2print (gk);
    }
  return 0;
}

int
x2print (im_ginfo *gk)
{  
  
  int pi;
  int pj;
  int ti;

  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      printf ("[%d]: cc (%3d) fc (%lf) - theta (%lf)\n", 
              pi, gk->cc[pi], gk->fc[pi], gk->thetas[pi]);
    }

  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      for (pj = 0; pj < numpopsizeparams; pj++)
        {
          if (pi == numpopsizeparams - 1
              || pj == numpopsizeparams - 1)
            {
              continue;
            }
          if (pi == pj)
            {
              continue;
            }
          printf ("[%d -> %d]: cm (%3d) fm (%lf) - mms (%lf)\n", 
                  pi, pj, gk->cm[pi][pj], gk->fm[pi][pj], gk->mms[pi][pj]);
        }
    }

  /* We need number of periods where migrations are allowed. */
  /* npops - 1 for tree model and 1 for tree model.          */
  /* lastperiodnumber is for this                            */
  for (ti = 0; ti < lastperiodnumber; ti++)
    {
      for (pi = 0; pi < numpopsizeparams; pi++)
        {
          if (saC.popnmig[ti][pi] != NULL)
            {
              printf ("[%d, %d], ms (%lf)\n", ti, pi, gk->ms[ti][pi]);
            }
        }
    }
  return 0;
}
void 
BitPrintF (UByteP A, int size)
{
  int i;
  fprintf (stdout, "Set: ");
  for (i = 0; i < size*8; i++) 
    { 
      if (BitIsTrue (A, i)) 
        { 
          fprintf (stdout, "%d ", i);
        } 
    }
  fprintf (stdout, "\n");
  return;
}
void
print_popnlist ()
{
  int i;
  int j;
  int k;
  int npnodes = 2 * npops - 1;
  assert (!(npops < 0));
  assert (npops < MAXPOPS);
  for (i = 0; i < npnodes; i++)
    {
      printf ("%d:(", i);
      for (j = 0; j < npops; j++)
        {
          if (saC.popnlist[i][j] != NULL)
            {
              printf ("P%d:", j);
              for (k = 0; k < saC.npopnlist[i][j]; k++)
                {
                  printf ("%d,", saC.popnlist[i][j][k]);
                }
            }
        }
      printf (")\n");
    }
  return;
}

double 
imaAsnValue (int ci, int li)
{
  double v;  
  v = imaAsnValue_mean (ci, li);
  return v;
}
double
imaAsnValue_mean (int ci, int li)
{
  double v;
  int ei;
  int ngenes;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;

  v = 0.0;
  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  ngenes = L[li].numgenes;
  for (ei = 0; ei < ngenes; ei++)
    {
      v += (double) gtree[ei].pop;
    }
  v /= (double) ngenes; 
  G->asn = v;
  return v; 
}
void
imaAsnInitAssign (int ci)
{
  int ii;
  int ij;
  int nei;
  struct genealogy *G; 
  struct edge *gtree; 
  int ei;
  int j;
  int k;
  int li;
  int pi;
  int *m;

  G = NULL;
  gtree = NULL;
  if (assignmentoptions[POPULATIONASSIGNMENT] == 0)
    {
      for (li = 0; li < nloci; li++)
        {
          G = &(C[ci]->G[li]);
          gtree = G->gtree;
          j = 0;
          k = 0;
          for (pi = 0; pi < npops; pi++)
            {
              while (j < L[li].samppop[pi] + k)
                {
                  gtree[j].pop = pi;
                  j++;
                }
              k += L[li].samppop[pi];
            }
        }
    }
  else if (assignmentoptions[POPULATIONASSIGNMENTASN] == 1)
    {
      m = malloc (snind * sizeof (int));
      for (ii = 0; ii < snind; ii++)
        {
          pi = sindividualAssignment[ii];
          if (pi == -1)
            {
              pi = randposint (npops);
            }
          m[ii] = pi;
        }
      for (li = 0; li < nloci; li++)
        {
          gtree = C[ci]->G[li].gtree;
          for (ei = 0; ei < L[li].numgenes; ei++)
            {
              ii = gtree[ei].i;
              gtree[ei].pop = m[ii];
            }
        }
      memset (C[ci]->nasn, 0, npops * sizeof (int));
      for (ii = 0; ii < snind; ii++)
        {
          assert (0 <= m[ii] && m[ii] < npops);
          C[ci]->nasn[m[ii]]++;
        }

      /* Check if all genes of an individual are assigned to the same label. */
      assert (snind > 0);
      for (ii = 0; ii < snind; ii++)
        {
          nei = sngenes_ind[ii];
          for (ij = 0; ij < nei; ij++)
            {
              li = saC.indlist[ii][ij].li;
              ei = saC.indlist[ii][ij].ei;
              G = &C[ci]->G[li];
              gtree = G->gtree;
              if (gtree[ei].i != ii)
                {
                  IM_err (IMERR_INPUTFILEINVALID, 
                          "Individual ID (%d) and its gene's individual ID (%d) do not match at locus %s %d-th gene",
                          ii, gtree[ei].i, L[li].name, ei + 1); 
                }
              if (gtree[ei].pop != m[ii])
                {
                  IM_err (IMERR_INPUTFILEINVALID, 
                          "The %d-th Gene of Individual (%d) at locus (%s) is assigned to a different population from other genes of the individual",
                          ei + 1, ii, L[li].name); 
                }
            }
        }
      XFREE (m);
    }
  else if (assignmentoptions[POPULATIONASSIGNMENTADM] == 1)
    {
      m = malloc (total_numgenes * sizeof (int));
      for (ii = 0; ii < total_numgenes; ii++)
        {
          /* pi = sindividualAssignment[ii]; */
          pi = -1; /* No prior information */
          if (pi == -1)
            {
              pi = randposint (npops);
            }
          m[ii] = pi;
        }
      ii = 0;
      for (li = 0; li < nloci; li++)
        {
          gtree = C[ci]->G[li].gtree;
          for (ei = 0; ei < L[li].numgenes; ei++)
            {
              gtree[ei].pop = m[ii];
              ii++;
            }
        }
      memset (C[ci]->nasn, 0, npops * nloci * sizeof (int));
      ii = 0;
      for (li = 0; li < nloci; li++)
        {
          for (ei = 0; ei < L[li].numgenes; ei++)
            {
              assert (0 <= m[ii] && m[ii] < npops);
              C[ci]->nasn[li * npops + m[ii]]++;
              ii++;
            }
        }
      assert (ii == total_numgenes);
      XFREE (m);
    }
  return;
}

void
imaAsnInit (void)
{
  sbfold.e1 = NULL;
  sbfold.e2 = NULL;
  sbfold.e3 = NULL;
  sbfold.n1 = 0;
  sbfold.n2 = 0;
  sbfold.n3 = 0;
  sbfold.nmax1 = 0;
  sbfold.nmax2 = 0;
  sbfold.nmax3 = 0;
  sbfold.n = NULL;
  sbfold.l = NULL;
  sbfold.seqz1 = NULL;

  sbfnew.e1 = NULL;
  sbfnew.e2 = NULL;
  sbfnew.e3 = NULL;
  sbfnew.n1 = 0;
  sbfnew.n2 = 0;
  sbfnew.n3 = 0;
  sbfnew.nmax1 = 0;
  sbfnew.nmax2 = 0;
  sbfnew.nmax3 = 0;
  sbfnew.n = NULL;
  sbfnew.l = NULL;
  sbfnew.seqz1 = NULL;

  saGiold = NULL;
  saGioldz1 = NULL;
  saGioldz2 = NULL;
  saGinew = NULL;
  saGinewz1 = NULL;
  saGinewz2 = NULL;

  snind = -1;
  snind_known = -1;
  snind_unknown = -1;
  sngenes_ind = NULL;     
  ssizePi = -1;
  sassigned = NULL;
  sind2gi = NULL;        
  sci = -1;
  snnminus1 = NULL;

  snasn = 0;

  sindividualName = NULL;
  sindividualNameSize = 0;
  sindividualAssignment = NULL;
  return;
}
void
imaAsnFree (void)
{
  int li;
  int i;

  free_genealogy_weights (&saC.savedallgweight);
  free_probcalc (&saC.savedallpcalc);
  for (li = 0; li < nloci; li++)
    {
      free_genealogy_weights (&saT[li].savedgweight);
    }

  imaFreeEdgemiginfo ();
  imaSavedFree ();
  imaIndFree ();

  imaPtreeUnset();

  if (assignmentoptions[POPULATIONASSIGNMENTASSIGNED] == 1)
    {
      assert (nloci == 1);

      for (i = 0; i < L[0].numgenesunknown; i++)
        {
          free (sassigned[i]);
          sassigned[i] = NULL;
        }
      free (sassigned);
      sassigned = NULL;
    }

  imaSbfFree ();
  imaFreeSagi ();
  imaFreeNnminus1 ();

  assert (sindividualAssignment != NULL);
  assert (sindividualName != NULL);
  assert (sindividualNameSize > 0);
  for (i = 0; i < sindividualNameSize; i++)
  {
    XFREE (sindividualName[i]); 
  }
  XFREE (sindividualName); 
  XFREE (sindividualAssignment);

#ifdef COUNT_PRIOR_ASSIGNMENT
  free (saasn);
  saasn = NULL;
#endif /* COUNT_PRIOR_ASSIGNMENT */

  return;
}
void
imaAsnSet ()
{
  int i;

  imaInitEdgemiginfo ();
  imaSavedInit ();

  imaIndInit ();
  imaIndSet ();
  /* imaIndSetPairs (); Diploid is not supported. */

  if (assignmentoptions[POPULATIONASSIGNMENTINFINITE] == 1)
    {
      imaPtreeSetIsland ();
    }
  else
    {
      imaPtreeSet ();
    }

  if (assignmentoptions[POPULATIONASSIGNMENTASSIGNED] == 1)
    {
      if (nloci != 1)
        {
          IM_errloc (AT, "DNA Barcoding for only a single locus data set");
        }
      sassigned = malloc (L[0].numgenesunknown * sizeof (double *));
      for (i = 0; i < L[0].numgenesunknown; i++)
        {
          sassigned[i] = malloc (npops * sizeof (double));
          memset (sassigned[i], 0, npops * sizeof (double));
        }
    }
  
  imaSbfInit ();
  imaInitSagi ();
  imaInitNnminus1 ();

#ifdef COUNT_PRIOR_ASSIGNMENT
  snasn = IMA_rgf_setSnasn (snind, npops);
  assert (snasn > 0);
  saasn = malloc (snasn * sizeof (int));
  memset (saasn, 0, snasn * sizeof (int));
#endif /* COUNT_PRIOR_ASSIGNMENT */

  /* Check if the given IMa command option and data are compatible. We could
   * have done this check right for some after we read the command line option. 
   * We have * to parse data before we can decide whether the command line 
   * option and data are compatible. WE CHECK THEM HERE NOT IN THE MAIN
   * FUNCTION. 
   * We provide one of two options: assignment or DNA barcoding. This should set
   * proper options. -a1: for assignment, -a2: for DNA barcoding. This should
   * change the assignmentoptions. -a1 sets only POPULATIONASSIGNMENT while -a2
   * sets POPULATIONASSIGNMENT and POPULATIONASSIGNMENTASSIGNED. */
  if (assignmentoptions[POPULATIONASSIGNMENTASSIGNED] == 1 
      && nloci != 1)
    {
      IM_errloc (AT, 
                 "A single locus is allowed for DNA Barcoding: num. of loci (%d)",
                 nloci);
    }
  return;
}
void
imaTreeSet ()
{
  /* imaInitEdgemiginfo (); */
  imaSavedInit ();

  if (assignmentoptions[POPULATIONASSIGNMENTINFINITE] == 1)
    {
      imaPtreeSetIsland ();
    }
  else
    {
      imaPtreeSet ();
    }

  return;
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


int 
imaPhylogenySplitTiAsn (const char *fname)
{
  int i;
  int j;
  int *tasn;
  int nind;
  char *fnamein;
  char *fname_outin;
  char *fname_out;
  char *fname_asn;
  char *fname_ti;
  char *fname_sumassignment;
  char *fname_order;
  FILE *forder;
  int len;
  int nrow;
  int ncol;
  int **m;
  int *od;
  int *odp;
  int r;
  int *nod;
  int npi;
  int d;
  double *fod;
  int nperm;
  int *ranksod;
  int nburnin;

  /* Works with three populations. */
  npops = 3;
  nperm = 6;
  nburnin = 0;

  /* Example: ./iist fischer.out 39 4 */

  len = strlen (fname);
  fname_ti = malloc ((len + 3 + 1) * sizeof (char));
  sprintf (fname_ti, "%s.ti", fname);
  fname_asn = malloc ((len + 4 + 1) * sizeof (char));
  sprintf (fname_asn, "%s.asn", fname);
  IMA_io_readp (fname_asn, &m, &nrow, &ncol, nburnin);
  nind = ncol;
  for (i = 0; i < nrow; i++)
    {
      for (j = 0; j < ncol; j++)
        {
          m[i][j]++;
        }
    }
  
  tasn = malloc (nind * sizeof (int));
  fname_sumassignment = malloc ((len + 20 + 1) * sizeof (char));
  sprintf (fname_sumassignment, "%s.in.sum_assignments", fname);
  IMA_io_readsumassignment (fname_sumassignment, nind, tasn);

  fname_order = malloc ((len + 6 + 1) * sizeof (char));
  sprintf (fname_order, "%s.order", fname);
  forder = fopen (fname_order, "w");


  od = (int *) malloc (npops * sizeof (int));
  odp = (int *) malloc ((npops + 1)* sizeof (int));
  nod = (int *) malloc (nperm * sizeof (int));
  fod = (double *) malloc (nperm * sizeof (double));
  memset (fod, 0, nperm * sizeof (double));
  memset (nod, 0, nperm * sizeof (int));
  ranksod = (int *) malloc (nrow * sizeof (int));

  for (i = 0; i < nrow; i++)
    {
      /*******************************************************/
      /* CHECK: see the detail of tasn and m[i] to check od. */
      /*******************************************************/
      d = IMA_gdistanceOrderNEW (nind, tasn, m[i], npops, od); /* FIXME: npops is fixed to 2 */
      odp[0] = 0;
      for (j = 0; j < npops; j++)
        {
          odp[j + 1] = od[j] + 1; 
        }
      r = IMA_permLexRank (npops, odp);
      ranksod[i] = r;
      fprintf (forder, "%d\n", r);
      nod[r]++;
      if (nod[r] == 1)
        {
          fod[r] = d;
        }
      else
        {
          fod[r] += d;
          fod[r] /= 2.0; /* FIXME: the same bug in Thermodynamic Integration */
        }

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
  fname_out = (char *) malloc ((len + 7 + 1) * sizeof (char));
  fnamein = (char *) malloc ((len + 3 + 1) * sizeof (char));
  fname_outin = (char *) malloc ((len + 10 + 1) * sizeof (char));
  sprintf (fnamein, "%s.in", fname);
  for (i = 0; i < npi; i++)
    {
      IMA_fprintf (stdout, npops, odp);
      r = IMA_permLexRank (npops, odp);
      assert (r == i);
      fprintf (stdout, "\trank: %d - (c) %d - %lf\n", r, nod[i], fod[i]);
      IMA_permLexSuccessor (npops, odp);
      sprintf (fname_out, "%s.%02d.out", fname, i);
      imaFileAsnOpen (fname_out, fname);
      imaFileTiOpen (fname_out);
      imaFileBatCreate (fname_out);
      sprintf (fname_outin, "%s.%02d.out.in", fname, i);
      imaFileCopy (fname_outin, fnamein);
    }

  for (i = 0; i < nrow; i++)
    {
      for (j = 0; j < ncol; j++)
        {
          m[i][j]--;
        }
    }
 
  /* Ranks divides ti and asn files to multiple ones. */
  for (i = 0; i < nrow; i++)
    {
      sprintf (fname_out, "%s.%02d.out", fname, ranksod[i]);
      imaFileAsnAppend (fname_out, i, m[i], nind);
      imaFileTiAppend (fname_out, i); 
      /* atree[gsamp_ccp + pj] = gsampinf[gi][gsamp_ccp + pi]; */
/*
      r = IMA_permLexRank (npops, ranksod[i]);
      IMA_permLexUnrank(p, 3, r);
      IMA_fprintf (stdout, 3, p);
*/
    }
  
  /* Free all local variables. */
  free (ranksod);
  free (od);
  free (odp);
  free (nod);
  free (tasn);
  free (fnamein); 
  free (fname_outin); 
  free (fname_out); 
  free (fname_ti); 
  free (fname_asn); 
  free (fname_sumassignment); 
  for (i = 0; i < nrow; i++)
    {
      free (m[i]);
    }
  free (m);

  free (fname_order); 
  fclose (forder);

  return 0;
}

int 
imaFileCopy (const char *fname1, const char *fname2)
{
  char *cmd;
  int len;

  len = strlen (fname1) + strlen (fname2) + 10;
  cmd = (char *) malloc (len * sizeof (char));
  sprintf (cmd, "%s %s %s", SYSCMDCOPY, fname2, fname1);
  system (cmd);
  free (cmd);
  cmd = NULL;
  return 0;
}

int 
imaFileBatCreate (const char *fname)
{
  FILE * fp;
  char * fnamebat;
  int len;
  const char * basename;

  len = strlen (fname);
  fnamebat = (char *) malloc ((len + 4 + 1) * sizeof (char));
  sprintf (fnamebat, "%s.bat", fname);

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
  basename = strrchr (fname, '\\');
#else
  basename = strrchr (fname, '/');
#endif
  if (basename == NULL)
    {
      basename = fname;
    }
  else
    {
      basename++;
    }

  fp = fopen (fnamebat, "w");
  fprintf (fp, "execute %s.in\nsum\nquit\n", basename);
  fclose (fp);
  fp = NULL;
  free (fnamebat);

  return 0;
}

int 
imaFileAsnOpen (const char *fname, const char *fnameorg)
{
  FILE *fp;
  FILE *fporg;
  char *fnameasn;
  char *fnameinp;
  char *fnameorgasn;
  int len;
  int lenorg;
  int c;

  len = strlen (fname);
  lenorg = strlen (fnameorg);
  fnameasn = (char *) malloc ((len + 4 + 1) * sizeof (char));
  sprintf (fnameasn, "%s.asn", fname);
  fnameinp = (char *) malloc ((len + 5 + 1) * sizeof (char));
  sprintf (fnameinp, "%s.in.p", fname);
  fnameorgasn = (char *) malloc ((lenorg + 4 + 1) * sizeof (char));
  sprintf (fnameorgasn, "%s.asn", fnameorg);

  /* Copy the first line. */
  fporg = fopen (fnameorgasn, "r");
  fp = fopen (fnameasn, "w");
  c = fgetc (fporg);
  while (c != '\n')
    {
      fputc (c, fp);
      c = fgetc (fporg);
    }
  fputc (c, fp);
  fclose (fp);

  rewind (fporg);
  fp = fopen (fnameinp, "w");
  c = fgetc (fporg);
  while (c != '\n')
    {
      fputc (c, fp);
      c = fgetc (fporg);
    }
  fputc (c, fp);
  fclose (fp);
  fp = NULL;

  fclose (fporg);
  fporg = NULL;

  free (fnameasn);
  free (fnameinp);
  free (fnameorgasn);
  return 0;
}

int 
imaFileAsnAppend (const char *fname, int gi, int *m, int n)
{
  FILE *fp;
  FILE *fpinp;
  char *fnameasn;
  char *fnameinp;
  int len;
  int i;
  int *a;

  len = strlen (fname);
  fnameasn = (char *) malloc ((len + 4 + 1) * sizeof (char));
  sprintf (fnameasn, "%s.asn", fname);
  fnameinp = (char *) malloc ((len + 5 + 1) * sizeof (char));
  sprintf (fnameinp, "%s.in.p", fname);

  a = (int *) malloc (n * sizeof (int));
  memcpy (a, m, n * sizeof (int));
  IMA_rgs_convert (a, n);

  fp = fopen (fnameasn, "a");
  fpinp = fopen (fnameinp, "a");
  fprintf (fp, "%d\t0\t%d", gi, m[0]);
  fprintf (fpinp, "%d\t0\t%d", gi, a[0]);
  for (i = 1; i < n; i++)
    {
      fprintf (fp, "\t%d", m[i]);
      fprintf (fpinp, "\t%d", a[i]);
    }
  fprintf (fp, "\n");
  fprintf (fpinp, "\n");
  fclose (fp);
  fp = NULL;
  fclose (fpinp);
  fpinp = NULL;

  free (fnameasn);
  free (fnameinp);
  return 0; 
}

int 
imaFileTiOpen (const char *fname)
{
  FILE *fp;
  char *fnameti;
  int len;

  len = strlen (fname);
  fnameti = (char *) malloc ((len + 3 + 1) * sizeof (char));
  sprintf (fnameti, "%s.ti", fname);

  fp = fopen (fnameti, "w");
  fprintf (fp, "VALUESSTART\n");
  fclose (fp);
  fp = NULL;

  free (fnameti);
  return 0;
}

int 
imaFileTiAppend (const char *fname, int gi)
{
  FILE *fp;
  char *fnameti;
  int len;
  int i;

  len = strlen (fname);
  fnameti = (char *) malloc ((len + 3 + 1) * sizeof (char));
  sprintf (fnameti, "%s.ti", fname);

  fp = fopen (fnameti, "a");
  fprintf (fp, "%f", gsampinf[gi][0]);
  for (i = 1; i < gsampinflength; i++)
    {
      fprintf (fp, "\t%f", gsampinf[gi][i]);
    }
  fprintf (fp, "\n");
  fclose (fp);
  fp = NULL;

  free (fnameti);
  return 0; 

}
int 
imaAssignmentReadInFile (const char* fn, int* s, const int n, const int m)
{
  FILE* fp;
  int ii;
  int mi;
  int iv;
  double dv;

  fp = fopen (fn, "r");
  if (fp == NULL)
    {
      IM_errloc (AT, "Could not open %s", fn);
    }
  skip_a_line (fp);

  for (mi = 0; mi < m; mi++)
    {
      iv = read_int (fp);
      dv = read_double (fp);
      for (ii = 0; ii < n; ii++)
        {
          iv = read_int (fp);
          s[mi * n + ii] = iv;
        }
    }
  fclose (fp);
  fp = NULL;
  return 0;
}

int
imaAssignmentReadSizeInFile (const char* fn, int* n, int* m)
{
  FILE* fp;
  char r[FNSIZE];
  int v;
  int numberIndividual;
  int numberSample;

  fp = fopen (fn, "r");
  if (fp == NULL)
    {
      IM_errloc (AT, "Could not open %s", fn);
    }
  /* Gen lnL P(1) P(2) ... P(10) */
  v = imaParserReadString (fp, r, FNSIZE);
  if (strcmp(r, "Gen"))
    {
      IM_errloc (AT, "The first column must be Gen in file %s", fn);
    }
  if (v == 1)
    {
      IM_errloc (AT, "Invalid asn file %s", fn);
    }
  v = imaParserReadString (fp, r, FNSIZE);
  if (strcmp(r, "lnL"))
    {
      IM_errloc (AT, "The 2nd column must be lnL in file %s", fn);
    }
  if (v == 1)
    {
      IM_errloc (AT, "Invalid asn file %s", fn);
    }
  numberIndividual = 0;
  while (v == 0)
    {
      v = imaParserReadString (fp, r, FNSIZE);
      numberIndividual++; 
    }
  *n = numberIndividual;

  numberSample = 0;
  while (!feof(fp))
    {
      v = imaParserReadString (fp, r, FNSIZE);
      if (v == 1)
        {
          numberSample++; 
        }
    }
  *m = numberSample;
  fclose (fp);
  fp = NULL;
  return 0;
}

int
imaCmdFindmeanassignment (const char* fni, const char* fno)
{
  int* sampledZ;
  int* meanPart;
  int Ni;
  int Nswb;
  FILE* fp;
  int ii;
#ifdef DEBUG
  int mi;
#endif

  imaAssignmentReadSizeInFile (fni, &Ni, &Nswb);
  sampledZ = malloc (sizeof (int) * (Ni * Nswb));
  if (sampledZ == NULL)
    {
      IM_errloc (AT, "Memory allocation");
    }
  meanPart = malloc (sizeof (int) * Ni);
  if (meanPart == NULL)
    {
      IM_errloc (AT, "Memory allocation");
    }

  imaAssignmentReadInFile (fni, sampledZ, Ni, Nswb);
#ifdef DEBUG
  for (mi = 0; mi < Nswb; mi++)
    {
      printf ("[%d] ", mi); 
      for (ii = 0; ii < Ni; ii++)
        {
          printf (" [%d]%d", ii, sampledZ[mi * Ni + ii]); 
        }
      printf ("\n");
    }
#endif

  Meanpartdis (sampledZ, Ni, Nswb, meanPart);

  fp = fopen (fno, "w");
  for (ii = 0; ii < Ni; ii++)
    fprintf (fp, "%d\n", meanPart[ii]);
  fclose (fp);

  XFREE (meanPart);
  XFREE (sampledZ);
  return 0;
}

int
imaCmdFindCoassignmentProbability (const char* fni, const char* fno)
{
  int* sampledZ;
  double* Coaprob;
  int Ni;
  int Nswb;
  FILE* fp;
  int ii;
  int jj;
  int kk;

  imaAssignmentReadSizeInFile (fni, &Ni, &Nswb);
  sampledZ = malloc (sizeof (int) * (Ni * Nswb));
  if (sampledZ == NULL)
    {
      IM_errloc (AT, "Memory allocation");
    }
  Coaprob = calloc (Ni * (Ni - 1) / 2, sizeof (double));
  if (Coaprob == NULL)
    {
      IM_errloc (AT, "Memory allocation");
    }

  imaAssignmentReadInFile (fni, sampledZ, Ni, Nswb);

  Coassignprobs (sampledZ, Ni, Nswb, Coaprob);

  fp = fopen (fno, "a");

  fprintf (fp, "co-assignment probabilities\n");
  fprintf (fp, "Individual numbers and corresponding probability\n");
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
  fclose (fp);

  XFREE (sampledZ);
  XFREE (Coaprob);
  return 0;
}

/* File static function */
void
imaInitEdgemiginfo ()
{
  int i;
  assert (!(npops < 0));
  saoems = malloc (2 * nloci * sizeof (struct edgemiginfo)); 
  sanems = malloc (2 * nloci * sizeof (struct edgemiginfo)); 
  saosms = malloc (2 * nloci * sizeof (struct edgemiginfo)); 
  sansms = malloc (2 * nloci * sizeof (struct edgemiginfo)); 
  for (i = 0; i < 2 * nloci; i++)
    {
      saoems[i].mp = malloc (npops * sizeof (int));
      saoems[i].mtimeavail = malloc (npops * sizeof (double));
      sanems[i].mp = malloc (npops * sizeof (int));
      sanems[i].mtimeavail = malloc (npops * sizeof (double));
      saosms[i].mp = malloc (npops * sizeof (int));
      saosms[i].mtimeavail = malloc (npops * sizeof (double));
      sansms[i].mp = malloc (npops * sizeof (int));
      sansms[i].mtimeavail = malloc (npops * sizeof (double));
    }
  return;
}

void
imaFreeEdgemiginfo ()
{
  int i;
  for (i = 0; i < 2 * nloci; i++)
    {
      XFREE (saoems[i].mp); XFREE (saoems[i].mtimeavail);
      XFREE (sanems[i].mp); XFREE (sanems[i].mtimeavail);
      XFREE (saosms[i].mp); XFREE (saosms[i].mtimeavail);
      XFREE (sansms[i].mp); XFREE (sansms[i].mtimeavail);
    }
  XFREE (saoems);
  XFREE (sanems);
  XFREE (saosms);
  XFREE (sansms);
  return;
}

void
imaEdgemiginfoPrint (im_edgemiginfo *m)
{
  int mi;

  if (m == NULL)
    {
      return;
    }

  printf ("    miginfo: edge [%2d] from %lf to %lf\n", m->edgeid, m->upt, m->dnt);
  printf ("             popn [%2d] to [%2d]\n", m->pop, m->fpop);
  mi = 0;
  while (m->mig[mi].mt > -0.5)
    {
      printf ("    mp [%2d] at %lf\n", m->mig[mi].mp, m->mig[mi].mt);
      mi++;
    }
  printf ("    mi vs. mpall: %2d == %2d\n", mi, m->mpall);
  return;
}

int 
imaIndUnknown ()
{
  return snind_unknown;
}

/*
 * Gene names and individuals
 */
void
imaIndInit ()
{
  sci = 0;
  sngenes_ind = NULL;

  return; 
}

void
imaIndFree ()
{
  int ii;

  free (sngenes_ind);
  sngenes_ind = NULL;

  for (ii = 0; ii < snind; ii++)
    {
      free (saC.indlist[ii]);
      saC.indlist[ii] = NULL;
    }
  free (saC.indlist);
  saC.indlist = NULL;

  for (ii = 0; ii < snind; ii++)
    {
      free (sind2gi[ii]);
      sind2gi[ii] = NULL;
    } 
  free (sind2gi);
  sind2gi = NULL;

  XFREE (sinds);

  return; 
}

void
imaGtreeReseti ()
{
  int ci;
  int li;
  int ei;
  int ngnodes;
  struct genealogy *G = NULL; /* locus li */
  struct edge *gtree = NULL;  /* genealogy of locus li */

  /* initialize */
  for (ci = 0; ci < numchains; ci++)
    {
      for (li = 0; li < nloci; li++)
        {
          G = &C[ci]->G[li];
          gtree = G->gtree;
          ngnodes = L[li].numlines;
          for (ei = 0; ei < ngnodes; ei++)
            {
              gtree[ei].i = -1;
            }
        }
    }
  return;
}

void
imaGtreeCopyi ()
{
  int ci;
  int li;
  int ei;
  int ngenes;
  struct genealogy *G = NULL;     /* locus li */
  struct edge *gtree = NULL;  /* genealogy of locus li */
  struct genealogy *G2 = NULL;    /* locus li */
  struct edge *gtree2 = NULL; /* genealogy of locus li */

  /* initialize */
  for (ci = 1; ci < numchains; ci++)
    {
      for (li = 0; li < nloci; li++)
        {
          G = &(C[0]->G[li]);
          gtree = G->gtree;
          G2 = &(C[ci]->G[li]);
          gtree2 = G2->gtree;
          ngenes = L[li].numgenes;
          for (ei = 0; ei < ngenes; ei++)
            {
              assert (!(gtree[ei].i < 0));
              assert (gtree2[ei].i == -1);
              gtree2[ei].i = gtree[ei].i;
            }
        }
    }
  return;
}
void 
imaIndReadIndividualData ()
{
  int li;
  int ei;
  int ii;
  int gi;
  int hi;
  char *query;
  char **geneName;
  int* b;
  int nb;
  int found;

  /* Copy all of the gene names. */
  geneName = (char **) malloc (total_numgenes * sizeof(char *));
  ii = 0;
  for (li = 0; li < nloci; li++)
    {
      for (ei = 0; ei < L[li].numgenes; ei++)
        {
          query = L[li].gNames[ei];
          geneName[ii] = strdup (query);
          ii++;
        }
    }
  assert (total_numgenes == ii);
 
  b = (int *) malloc (total_numgenes * sizeof(int));
  nb = 0;
  for (gi = 0; gi < total_numgenes; gi++)
    {
      found = 0;
      for (hi = 0; hi < gi; hi++)
        {
          if (!strcmp (geneName[gi],geneName[hi]))
            {
              found = 1;
            }
        }
      if (found == 0)
        {
          b[nb] = gi;
          nb++;
        }
    }
  
  sindividualNameSize = nb;
  sindividualName = (char **) malloc (sindividualNameSize * sizeof(char *));
  sindividualAssignment = (int*) malloc (sindividualNameSize * sizeof(int));
  for (hi = 0; hi < nb; hi++)
    {
      gi = b[hi];
      sindividualName[hi] = strdup (geneName[gi]);
      sindividualAssignment[hi] = -1;
    }

  for (gi = 0; gi < total_numgenes; gi++)
    {
      XFREE (geneName[gi]);
    }
  XFREE (geneName); 
  XFREE (b);

  return; 
}
int 
imaUtilityExistString (char** a, int n, const char* b)
{
  int found;
  int i;
  found = -1;
  for (i = 0; i < n; i++)
    {
      if (!strcmp(a[i],b))
        {
          found = i;
        }
    }
  return found;
}

void 
imaIndReadIndividualFile (const char* fn)
{
  FILE* f;
  char s[FNSIZE];
  int numberIndividual;
  int i;
  int ii;
  int li;
  int ei;
  int gi;
  int a;
  int found;
  char** geneName;

  /* Copy all of the gene names. */
  geneName = (char **) malloc (total_numgenes * sizeof(char *));
  ii = 0;
  for (li = 0; li < nloci; li++)
    {
      for (ei = 0; ei < L[li].numgenes; ei++)
        {
          geneName[ii] = strdup (L[li].gNames[ei]);
          ii++;
        }
    }
  assert (total_numgenes == ii);

  /* Find the number of individuals. */
  sindividualNameSize = 0;
  f = NULL;
  f = fopen (fn, "r");
  if (f == NULL)
  {
    IM_errloc (AT, "Could not open %s", fn);
  }
  while (!feof(f)) 
  {
    if (fscanf(f, "%s %d", s, &numberIndividual) != 2)
    {
      if (feof(f)) 
      {
        break;
      }
      IM_errloc (AT, "Individual file %s is in wrong format", fn);
    }
    for (i = 0; i < numberIndividual; i++)
    {
      if (fscanf(f, "%s %d", s, &a) != 2)
      {
        IM_errloc (AT, "Individual file %s is in wrong format", fn);
      }
      found = imaUtilityExistString (geneName, total_numgenes, s);
      if (found >= 0)
      {
        sindividualNameSize++;
      }
    }
  }
  fclose (f);
  f = NULL;

  /* Allocate the memory for sindividualName. */
  sindividualName = (char **) malloc (sindividualNameSize * sizeof(char *));
  sindividualAssignment = (int*) malloc (sindividualNameSize * sizeof(int));

  /* Find individual names. */
  sindividualNameSize = 0;
  f = fopen (fn, "r");
  if (f == NULL)
  {
    IM_errloc (AT, "Could not open %s", fn);
  }
  while (!feof(f)) 
  {
    if (fscanf(f, "%s %d", s, &numberIndividual) != 2)
    {
      if (feof(f)) 
      {
        break;
      }
      IM_errloc (AT, "Individual file %s is in wrong format", fn);
    }
    for (i = 0; i < numberIndividual; i++)
    {
      if (fscanf(f, "%s %d", s, &a) != 2)
      {
        IM_errloc (AT, "Individual file %s is in wrong format", fn);
      }

      found = imaUtilityExistString (geneName, total_numgenes, s);
      if (found >= 0)
      {
        sindividualName[sindividualNameSize] = strdup (s);
        sindividualAssignment[sindividualNameSize] = a;
        sindividualNameSize++;
      }
    }
  }
  fclose (f);
  f = NULL;

  for (gi = 0; gi < total_numgenes; gi++)
    {
      XFREE (geneName[gi]);
    }
  XFREE (geneName); 
  return;
}
void
imaIndSet ()
{
  int ci;
  int li;
  int ei;
  int ii;
  int pi;
  int found;

  int i;
  int j;
  int n;
  int ngenes1;
  int *nis;
  int *nii;
  int ngenes;
  struct genealogy *G = NULL;     /* locus li */
  struct edge *gtree = NULL;  /* genealogy of locus li */

  if (imcmdoptions[IMCMD_INDIVIDUAL] == 1)
  {
    imaIndReadIndividualFile (individualfilename);
  }
  else
  {
    imaIndReadIndividualData ();
  }
#ifdef DEBUG
  printf ("Individuals have been read...\n");
  for (i = 0; i < sindividualNameSize; i++)
  {
    printf ("[%02d] [%s] P(%d)\n", i+1,
            sindividualName[i],
            sindividualAssignment[i]);
  }
#endif

  /* set all i's to -1 */
  imaGtreeReseti ();

  /* Known genes are first labelled. */
  for (ci = 0; ci < numchains; ci++)
    {
      for (li = 0; li < nloci; li++)
        {
          gtree = C[ci]->G[li].gtree;
          for (ei = 0; ei < L[li].numgenes; ei++)
            {
              found = imaUtilityExistString (sindividualName, sindividualNameSize, L[li].gNames[ei]);
              if (found >= 0)
                {
                  gtree[ei].i = found;
                }
              else
                {
                  IM_errloc (AT, 
                             "Individual file does not contain all of the individuals");
                }
            }
        }
    }

  /* Set numgenesknown and numgenesunknown. */
  /* Set samppop. */
  for (li = 0; li < nloci; li++)
    {
      L[li].numgenesknown = 0;
      L[li].numgenesunknown = 0;
      for (pi = 0; pi < npops; pi++)
        {
          L[li].samppop[pi] = 0;
        }
      for (ei = 0; ei < L[li].numgenes; ei++)
        {
          found = imaUtilityExistString (sindividualName, sindividualNameSize, L[li].gNames[ei]);
          assert (found >= 0);
          pi = sindividualAssignment[found];
          if (pi == -1)
            {
              L[li].numgenesunknown++;
            }
          else
            {
              L[li].numgenesknown++;
              L[li].samppop[pi] = 0;
            }
        } 
    }

  /* total number of individuals */
  snind = sindividualNameSize;
  snind_unknown = 0;
  snind_known = 0;
  for (ii = 0; ii < sindividualNameSize; ii++)
    {
      if (sindividualAssignment[ii] == -1)
        {
          snind_unknown++;
        }
      else
        {
          snind_known++;
        }
    }

  /* Check if snind is 1. */
  if (assignmentoptions[POPULATIONASSIGNMENTASSIGNED] == 1)
    {
      if (snind_unknown != 1)
        {
          IM_errloc (AT, 
                     "A single unknown gene is required: num. of unknown (%d)",
                     snind);
        }
    }

  sinds = malloc (snind_unknown * sizeof (int));

  /* list of genes for an individual */
  saC.indlist = malloc (snind * sizeof(im_node *));
  nis = malloc (snind * sizeof(int));
  nii = malloc (snind * sizeof(int));
  sngenes_ind = malloc (snind * sizeof(int)); 
  for (i = 0; i < snind; i++)
    {
      n = imaIndN (i); 
      nis[i] = n;
      nii[i] = 0;
      /* saC.indlist[i] = malloc (n * sizeof(im_node)); */
      saC.indlist[i] = malloc (nloci * 2 * sizeof(im_node));
    }
  for (li = 0; li < nloci; li++)
    {
      G = &(C[sci]->G[li]);
      gtree = G->gtree;
      ngenes1 = L[li].numgenes;
      for (ei = 0; ei < ngenes1; ei++)
        {
          i = gtree[ei].i;
          j = nii[i];
          saC.indlist[i][j].li = li;
          saC.indlist[i][j].ei = ei;
          nii[i]++;
        }
    }
  for (i = 0; i < snind; i++)
    {
      assert (nis[i] == nii[i]);
      sngenes_ind[i] = nis[i];
    }
  sind2gi = malloc (snind * sizeof (int *));
  for (ii = 0; ii < snind; ii++)
    {
      sind2gi[ii] = malloc (nloci * sizeof (int));
      for (li = 0; li < nloci; li++)
        {
          G = &(C[sci]->G[li]);
          gtree = G->gtree;
          ngenes = L[li].numgenes;
          for (ei = 0; ei < ngenes; ei++)
            {
              if (gtree[ei].i == ii)
                {
                  sind2gi[ii][li] = ei;
                  break;
                }
            }
          if (ei == ngenes)
            {
              sind2gi[ii][li] = -1;
            }
        }
    }
 
  free (nis);
  nis = NULL;
  free (nii);
  nii = NULL;

  /* just print indlist */
#ifdef DEBUG
  for (i = 0; i < snind; i++)
    {
      printf ("ind [%3d]\t", i);
      for (j = 0; j < sngenes_ind[i]; j++)
        {
          li = saC.indlist[i][j].li;
          ei = saC.indlist[i][j].ei;
          printf ("[%d](%d-%d)\t", j, li, ei);
        }
      printf ("\n");
    }
#endif

  return;
}

int
imaIndN (int k)
{
  struct genealogy *G = NULL;     /* locus li */
  struct edge *gtree = NULL;  /* genealogy of locus li */
  int li;
  int ei;
  int n;
  int i;
  int ngenes;

  n = 0;
  for (li = 0; li < nloci; li++)
    {
      G = &(C[sci]->G[li]);
      gtree = G->gtree;
      ngenes = L[li].numgenes;
      for (ei = 0; ei < ngenes; ei++)
        {
          i = gtree[ei].i;
          if (i == k)
            {
              n++;
            }
        }
    }
  return n;
}
void
imaIndSetPairs ()
{
  /* can be a function */
  /* set i, pairs */
  int ngenes;
  int li;
  int ei;
  int ei2;
  int ii;
  struct edge *gtree;

  for (li = 0; li < nloci; li++)
    {
      ngenes = L[li].numgenes;
      gtree = C[0]->G[li].gtree;
      for (ei = 0; ei < ngenes; ei++)
        {
          L[li].pairs[ei] = -1;
        }
    }

  for (li = 0; li < nloci; li++)
    {
      ngenes = L[li].numgenes;
      gtree = C[0]->G[li].gtree;
      for (ei = 0; ei < ngenes; ei++)
        {
          ii = gtree[ei].i;
          for (ei2 = 0; ei2 < ngenes; ei2++)
            {
              if (ei == ei2)
                {
                  continue;
                }
              if (gtree[ei2].i == ii)
                {
                  L[li].pairs[ei] = ei2;
                }
            }
        }
    }
  return;
}
void
imaInitNnminus1 ()
{
  int i;
  int li;
  int largestsamp;
  for (i = 0, li = 0; li < nloci; li++)
    {
      if (i < L[li].numgenes)
        {
          i = L[li].numgenes;
        }
    }
  largestsamp = i;
  snnminus1 = malloc ((largestsamp + 1) * sizeof (double));
  snnminus1[0] = 0;
  for (i = 1; i <= largestsamp; i++)
    {
      snnminus1[i] = (double) (i) * ((double) i - 1);
    }
  return;
}

void 
imaFreeNnminus1 ()
{
  free (snnminus1);
  snnminus1 = NULL;
}

int
imaSbfInit ()
{
  int n;
  int li;
  int ai;
  int pi;
  int largestnA;
  int existInfinitesitesMutation;
  int existHKYMutation;
  int existStepwiseMutation;

  n = 0;
  for (li = 0; li < nloci; li++)
    {
      if (n < L[li].numlines)
        {
          n = L[li].numlines;
        }
    }
  sbfold.nmax1 = 2 * n;
  sbfold.nmax2 = 2 * n;
  sbfold.nmax3 = 2 * n;
  sbfnew.nmax1 = 2 * n;
  sbfnew.nmax2 = 2 * n;
  sbfnew.nmax3 = 2 * n;

  sbfold.e1 = (im_event *) malloc (sbfold.nmax1 * sizeof (im_event));
  sbfold.e2 = (im_event *) malloc (sbfold.nmax2 * sizeof (im_event));
  sbfold.e3 = (im_event *) malloc (sbfold.nmax3 * sizeof (im_event));
  sbfold.nz1 = (int *) malloc (numpopsizeparams * sizeof (int));
  sbfold.n = (int *) malloc (numpopsizeparams * sizeof (int));
  sbfold.lz1 = (int **) malloc (numpopsizeparams * sizeof (int *));
  sbfold.l = (int **) malloc (numpopsizeparams * sizeof (int *));
  sbfold.a = (int *) malloc (gi_largestngenes * sizeof (int));
  sbfold.seqz1 = (int *) malloc (gi_largestnumsites * sizeof (int));
  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      assert (gi_largestngenes > 0);
      sbfold.lz1[pi] = (int *) malloc (gi_largestngenes * sizeof (int));
      sbfold.l[pi] = (int *) malloc (gi_largestngenes * sizeof (int));
    }

  sbfnew.e1 = (im_event *) malloc (sbfnew.nmax1 * sizeof (im_event));
  sbfnew.e2 = (im_event *) malloc (sbfnew.nmax2 * sizeof (im_event));
  sbfnew.e3 = (im_event *) malloc (sbfnew.nmax3 * sizeof (im_event));
  sbfnew.nz1 = (int *) malloc (numpopsizeparams * sizeof (int));
  sbfnew.n = (int *) malloc (numpopsizeparams * sizeof (int));
  sbfnew.lz1 = (int **) malloc (numpopsizeparams * sizeof (int *));
  sbfnew.l = (int **) malloc (numpopsizeparams * sizeof (int *));
  sbfnew.a = (int *) malloc (gi_largestngenes * sizeof (int));
  sbfnew.seqz1 = (int *) malloc (gi_largestnumsites * sizeof (int));
  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      assert (gi_largestngenes > 0);
      sbfnew.lz1[pi] = (int *) malloc (gi_largestngenes * sizeof (int));
      sbfnew.l[pi] = (int *) malloc (gi_largestngenes * sizeof (int));
    }

  /* likes1 and likes11 are for HKY and STEPWISE models. */
  existInfinitesitesMutation = 0;
  existStepwiseMutation = 0;
  existHKYMutation = 0;
  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == INFINITESITES)
        {
          existInfinitesitesMutation = 1;
        }
      if (L[li].model == HKY)
        {
          existHKYMutation = 1;
        }
      if (L[li].model == STEPWISE)
        {
          IM_errloc (AT, 
            "Stepwise mutation model does not work with population assignment");
          existStepwiseMutation = 1;
        }
    }
  largestnA = -1;
  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == STEPWISE)
        {
          if (largestnA < L[li].maxA[0] - L[li].minA[0] + 1)
            {
              largestnA = L[li].maxA[0] - L[li].minA[0] + 1;
            }
        }
    }
  if (existStepwiseMutation == 1 || existHKYMutation == 1)
    {
      sbfold.likes1 = (double *) malloc (gi_largestngenes * sizeof (double));
      sbfnew.likes1 = (double *) malloc (gi_largestngenes * sizeof (double));
    }
  else
    {
      sbfold.likes1 = NULL;
      sbfnew.likes1 = NULL;
    }
  if (existStepwiseMutation == 1)
    {
      sbfold.likes11 = (double **) malloc (gi_largestngenes * sizeof (double *));
      for (ai = 0; ai < gi_largestngenes; ai++)
        {
          sbfold.likes11[ai] = (double *) malloc (largestnA * sizeof (double));
        }
      sbfnew.likes11 = (double **) malloc (gi_largestngenes * sizeof (double *));
      for (ai = 0; ai < gi_largestngenes; ai++)
        {
          sbfnew.likes11[ai] = (double *) malloc (largestnA * sizeof (double));
        }
    }
  else
    {
      sbfold.likes11 = NULL;
      sbfnew.likes11 = NULL;
    }
  return 0;
}

int
imaSbfFree ()
{
  int pi, ai, li;
  int existStepwiseMutation;
  int existHKYMutation;

  XFREE (sbfold.e1); sbfold.n1 = 0; sbfold.nmax1 = 0;
  XFREE (sbfold.e2); sbfold.n2 = 0; sbfold.nmax2 = 0;
  XFREE (sbfold.e3); sbfold.n3 = 0; sbfold.nmax3 = 0;
  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      XFREE (sbfold.lz1[pi]); XFREE (sbfold.l[pi]);
    }
  XFREE (sbfold.lz1); XFREE (sbfold.l); XFREE (sbfold.nz1);
  XFREE (sbfold.n); XFREE (sbfold.a); XFREE (sbfold.seqz1);

  XFREE (sbfnew.e1); sbfnew.n1 = 0; sbfnew.nmax1 = 0;
  XFREE (sbfnew.e2); sbfnew.n2 = 0; sbfnew.nmax2 = 0;
  XFREE (sbfnew.e3); sbfnew.n3 = 0; sbfnew.nmax3 = 0;
  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      XFREE (sbfnew.lz1[pi]);
      XFREE (sbfnew.l[pi]);
    }
  XFREE (sbfnew.lz1); XFREE (sbfnew.l); XFREE (sbfnew.nz1);
  XFREE (sbfnew.n); XFREE (sbfnew.a); XFREE (sbfnew.seqz1);

  /* likes1 and likes11 are for HKY and STEPWISE models. */
  existStepwiseMutation = 0;
  existHKYMutation = 0;
  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == HKY)
        {
          existHKYMutation = 1;
        }
      if (L[li].model == STEPWISE)
        {
          IM_errloc (AT, 
            "Stepwise mutation model does not work with population assignment");
          existStepwiseMutation = 1;
        }
    }
  if (existStepwiseMutation == 1 || existHKYMutation == 1)
    {
      XFREE (sbfold.likes1);
      XFREE (sbfnew.likes1);
    }
  if (existStepwiseMutation == 1)
    {
      for (ai = 0; ai < gi_largestngenes; ai++)
        {
          XFREE (sbfold.likes11[ai]);
        }
      XFREE (sbfold.likes11);
      for (ai = 0; ai < gi_largestngenes; ai++)
        {
          XFREE (sbfnew.likes11[ai]);
        }
      XFREE (sbfnew.likes11);
    }

  return 0;
}

void 
imaInitSagi ()
{
  imaGinfoInit (&saGiold);
  imaGinfoInit (&saGioldz1);
  imaGinfoInit (&saGioldz2);
  imaGinfoInit (&saGinew);
  imaGinfoInit (&saGinewz1);
  imaGinfoInit (&saGinewz2);
  return;
}

void 
imaFreeSagi ()
{
  imaGinfoFree (&saGiold);
  imaGinfoFree (&saGioldz1);
  imaGinfoFree (&saGioldz2);
  imaGinfoFree (&saGinew);
  imaGinfoFree (&saGinewz1);
  imaGinfoFree (&saGinewz2);
  return;
}

void 
imaGinfoInit (im_ginfo **g)
{
  int li;
  int pi;

  (*g) = malloc (nloci * sizeof (im_ginfo));
  for (li = 0; li < nloci; li++)
    {
      (*g)[li].cc = malloc (numpopsizeparams * sizeof (int));
      (*g)[li].fc = malloc (numpopsizeparams * sizeof (double));
      (*g)[li].cm = malloc (numpopsizeparams * sizeof (int *));
      (*g)[li].fm = malloc (numpopsizeparams * sizeof (double *));
      (*g)[li].mms = malloc (numpopsizeparams * sizeof (double *));
      for (pi = 0; pi < numpopsizeparams; pi++)
        {
          (*g)[li].cm[pi] = malloc (numpopsizeparams * sizeof (int));
          (*g)[li].fm[pi] = malloc (numpopsizeparams * sizeof (double));
          (*g)[li].mms[pi] = malloc (numpopsizeparams * sizeof (double));
        }
      (*g)[li].ms = malloc (lastperiodnumber * sizeof (double *));
      for (pi = 0; pi < lastperiodnumber; pi++)
        {
          (*g)[li].ms[pi] = malloc (numpopsizeparams * sizeof (double));
        }
      (*g)[li].thetas = malloc (numpopsizeparams * sizeof (double));
    }
  return;
}

void
imaGinfoFree (im_ginfo **g)
{
  int li;
  int pi;

  for (li = 0; li < nloci; li++)
    {
      XFREE ((*g)[li].cc);
      XFREE ((*g)[li].fc);
      for (pi = 0; pi < numpopsizeparams ; pi++)
        {
          XFREE ((*g)[li].cm[pi]);
          XFREE ((*g)[li].fm[pi]);
          XFREE ((*g)[li].mms[pi]);
        }
      XFREE ((*g)[li].cm);
      XFREE ((*g)[li].fm);
      XFREE ((*g)[li].thetas);
      XFREE ((*g)[li].mms);
      for (pi = 0; pi < lastperiodnumber; pi++)
        {
          XFREE ((*g)[li].ms[pi]);
        }
      XFREE ((*g)[li].ms);
    }
  XFREE (*g);
  return;
}

void 
imaGinfoReset (im_ginfo *g)
{
  int ki;
  int pi;
  int pj;

  g->cc = memset (g->cc, 0, numpopsizeparams * sizeof (int));
  g->fc = memset (g->fc, 0, numpopsizeparams * sizeof (double));
  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      g->cm[pi] = memset (g->cm[pi], 0, numpopsizeparams * sizeof (int));
      g->fm[pi] = memset (g->fm[pi], 0, numpopsizeparams * sizeof (double));
      g->mms[pi] = memset (g->mms[pi], 0, numpopsizeparams * sizeof (double));
    }
  for (ki = 0; ki < lastperiodnumber; ki++)
    {
      g->ms[ki] = memset (g->ms[ki], 0, numpopsizeparams * sizeof (double));
    }
  g->thetas = memset (g->thetas, 0, numpopsizeparams * sizeof (double));

  for (pi = 0; pi < numpopsizeparams; pi++)
    {
      assert (g->cc[pi] == 0);
      assert (g->fc[pi] == 0.0);
      for (pj = 0; pj < numpopsizeparams; pj++)
        {
          assert (g->cm[pi][pj] == 0);
          assert (g->fm[pi][pj] == 0.0);
          assert (g->mms[pi][pj] == 0.0);
        }
      assert (g->thetas[pi] == 0.0);
    }
  for (ki = 0; ki < lastperiodnumber; ki++)
    {
      for (pj = 0; pj < numpopsizeparams; pj++)
        {
          assert (g->ms[ki][pj] == 0.0);
        }
    }

  return;
}

double
imaGtreeBranchLength (int ci, int li, int ei)
{
  double len;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;
  G = &(C[ci]->G[li]);
  gtree = G->gtree;
    
  assert (ei != G->root);
  len = gtree[ei].time - imaGtreeTime (ci, li, ei);
  return len;
}
int
imaGtreeSister (int ci, int li, int ei)
{
  int v;
  int gparent;

  v = -1;
  gparent = C[ci]->G[li].gtree[ei].down;
  if (!(gparent < 0))
    {
      if (C[ci]->G[li].gtree[gparent].up[0] == ei)
        {
          v = C[ci]->G[li].gtree[gparent].up[1];
        }
      else
        {
          v = C[ci]->G[li].gtree[gparent].up[0];
        }
    }
  return v;
}
int
imaGtreePop (int ci, int li, int ei)
{
  return C[ci]->G[li].gtree[ei].pop;
}
double
imaGtreeTime (int ci, int li, int ei)
{
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;
  int gchild;
  double t;

  G = &(C[ci]->G[li]);
  gtree = G->gtree;

  gchild = gtree[ei].up[0]; /* this may be also up[1]. */
  if (gchild < 0)
    {
      t = 0.0;
    }
  else
    {
      assert (gchild < L[li].numlines);
      t = gtree[gchild].time;
    }
  return t;
}
int 
imaGtreeNmig (int ci, int li, int ei)
{
  int n;
  int gparent;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;
  
  assert (!(ci < 0));
  assert (!(li < 0));
  
  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  gparent = gtree[ei].down;

  if (gparent < 0)
    {
      if (gtree[ei].cmm > 0)
        {
          assert (gtree[ei].mig[0].mt < -0.5);
        }
    }

  n = 0;

  if (gtree[ei].cmm > 0)
    {
      while (gtree[ei].mig[n].mt > -0.5)
        {
          n++; 
        }
      assert (!(n < 0));
    }
  
  gtree[ei].nmig = n;
  return n;
}
/*
 * Print genealogical trees in Newick formated strings
 */
void 
imaGnodePrint_node (FILE *fp, struct edge *gtree, int e)
{
  int mi;

  fprintf (fp, "%d[%d]", e, gtree[e].pop);

  if (modeloptions[NOMIGRATION] == 0)
    {
      for (mi = 0; mi < gtree[e].cmm; mi++)
        {
          if (gtree[e].mig[mi].mt < -0.5)
            {
              break;
            }
          fprintf (fp, "-[%d@%.1lf]", gtree[e].mig[mi].mp, gtree[e].mig[mi].mt);
        }
    }

  return;
}

void 
imaGnodePrint (FILE *fp, struct edge *gtree, int e)
{
  int left;
  int right;
  int parent;

  left = gtree[e].up[0];
  right = gtree[e].up[1];
  parent = gtree[e].down;

  assert ((left < 0 && right < 0) || !(left < 0 && right < 0));

  if (left < 0 && right < 0)
    {
      imaGnodePrint_node (fp, gtree, e);
    }
  else
    {
      fprintf (fp, "(");
      imaGnodePrint (fp, gtree, left);
      fprintf (fp, ",");
      imaGnodePrint (fp, gtree, right);
      fprintf (fp, ")");
      imaGnodePrint_node (fp, gtree, e);
    }
  if (parent < 0)
    {
      fprintf (fp, ";\n");
    }
  else
    {
      fprintf (fp, ":%lf", gtree[e].time - gtree[left].time);
    }
  return;
}

void 
imaGnodePrint_nodeseq (FILE *fp, struct edge *gtree, int e, int si)
{
  fprintf (fp, "%d[%d]", e, gtree[e].seq[si]);

  return;
}


void 
imaGnodePrintseq (FILE *fp, struct edge *gtree, int e, int si)
{
  int left;
  int right;
  int parent;

  left = gtree[e].up[0];
  right = gtree[e].up[1];
  parent = gtree[e].down;

  assert ((left < 0 && right < 0) || !(left < 0 && right < 0));

  if (left < 0 && right < 0)
    {
      imaGnodePrint_nodeseq (fp, gtree, e, si);
    }
  else
    {
      fprintf (fp, "(");
      imaGnodePrintseq (fp, gtree, left, si);
      fprintf (fp, ",");
      imaGnodePrintseq (fp, gtree, right, si);
      fprintf (fp, ")");
      imaGnodePrint_nodeseq (fp, gtree, e, si);
    }
  if (parent < 0)
    {
      fprintf (fp, ";\n");
    }
  else
    {
      fprintf (fp, ":%lf", gtree[e].time - gtree[left].time);
    }
  return;
}
void 
funcx()
{
  imaGtreePrintall (0);
  return;
}

char* 
imaGtreePrint (int ci, int li)
{
  char *ns = NULL;
  FILE *fp;
  struct genealogy *G = NULL;    /* locus li */
  struct edge *gtree = NULL; /* genealogy of locus li */
  
  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  fp = fopen ("1.tre", "a");
  imaGnodePrint (fp, gtree, G->root);
  fclose (fp);
  fp = NULL;
  
  return ns;
}

char* 
imaGtreePrintall (int ci)
{
  char *ns = NULL;
  FILE *fp;
  struct genealogy *G = NULL;    /* locus li */
  struct edge *gtree = NULL; /* genealogy of locus li */
  int li;
  
  fp = fopen ("1.tre", "w");
  for (li = 0; li < nloci; li++)
    {
      G = &(C[ci]->G[li]);
      gtree = G->gtree;
      imaGnodePrint (fp, gtree, G->root);
    }
  fclose (fp);
  fp = NULL;

  return ns;
}

char* 
imaGtreePrintsite (int ci, int li, int si)
{
  char *ns = NULL;
  FILE *fp;
  struct genealogy *G = NULL;    /* locus li */
  struct edge *gtree = NULL; /* genealogy of locus li */
  
  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  fp = fopen ("1.tre", "w");
  imaGnodePrintseq (fp, gtree, G->root, si);
  fclose (fp);
  fp = NULL;
  
  return ns;
}

char* 
imaGtreePrintseq (int ci, int li)
{
  int si;
  char *ns = NULL;
  FILE *fp;
  struct genealogy *G = NULL;    /* locus li */
  struct edge *gtree = NULL; /* genealogy of locus li */
  
  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  fp = fopen ("1.tre", "w");
  for (si = 0; si < L[li].numsites; si++)
    {
      imaGnodePrintseq (fp, gtree, G->root, si);
    }
  fclose (fp);
  fp = NULL;
  
  return ns;
}
int
findperiodoftime (int ci, double ktime)
{
  int ti; 

  ti = 0;
  while (ktime > C[ci]->tvals[ti])
    ti++;

  return ti;

}
void
imaPtreeSetIsland ()
{
  int ti;
  int ni;
  int pi;
  int i;
  int j;
  int k;
  int npnodes;
  int biti;
  int bitj;
  int n;
  int ip;
  int nperiods;
  int isExist;

  assert (!(npops < 0)); 
  assert (npops < MAXPOPS); 


  npnodes = npops + 1;
  nperiods = 2;


  ssizePi = ((2 * npops - 1) - 1) / 8 + 1;

  assert (lastperiodnumber == 1);

  /* Ps */
  BitPowerSetNew (saC.Ps, ssizePi, nperiods, biti);
  for (ti = 0; ti < lastperiodnumber; ti++)
    {
      for (ni = 0; ni < npops; ni++)
        {
          assert (!(C[sci]->plist[ti][ni] < 0));
          BitTrue (saC.Ps[ti], C[sci]->plist[ti][ni]);
        }
    }
  BitTrue (saC.Ps[1], C[sci]->plist[1][0]);

  /* popndown */
  saC.popndown = malloc ((npops + 1) * sizeof(int *));
  for (pi = 0; pi < npops; pi++)
    {
      saC.popndown[pi] = malloc (nperiods * sizeof(int));
      saC.popndown[pi][0] = pi;
      saC.popndown[pi][1] = npops;
    }
  saC.popndown[npops] = malloc (nperiods * sizeof(int));
  saC.popndown[npops][0] = -1;
  saC.popndown[npops][1] = npops;

  /**********************************************************************/
  /*                           IMA_popnlist_set                         */
  /**********************************************************************/
  assert (!(npops < 0));
  assert (npops < MAXPOPS);

  /* popnlist */
  saC.popnlist = malloc ((npops + 1) * sizeof (int **));
  saC.npopnlist = malloc ((npops + 1) * sizeof (int *));
  assert (nperiods == 2);
  saC.popnlist[npops] = malloc (nperiods * sizeof(int *)); 
  saC.popnlist[npops][1] = malloc (sizeof(int)); 
  saC.popnlist[npops][1][0] = npops;
  saC.popnlist[npops][0] = malloc (npops * sizeof(int)); 

  saC.npopnlist[npops] = malloc (nperiods * sizeof (int));
  saC.npopnlist[npops][1] = 1;
  saC.npopnlist[npops][0] = npops;
  for (i = 0; i < npops; i++)
    {
      saC.popnlist[i] = malloc (nperiods * sizeof(int *)); 
      saC.popnlist[i][1] = NULL;
      saC.popnlist[i][0] = malloc (sizeof(int));
      saC.popnlist[i][0][0] = i;
      saC.popnlist[npops][0][i] = i;

      saC.npopnlist[i] = malloc (nperiods * sizeof (int));
      saC.npopnlist[i][1] = 0;
      saC.npopnlist[i][0] = 1;
    }

/*********************************** DELELE THIS.
  saC.popnlistDELETE = malloc ((npops + 1) * sizeof(gsl_block_uint **));
  saC.popnlistDELETE[npops] = malloc (nperiods * sizeof(gsl_block_uint *)); 
  saC.popnlistDELETE[npops][1] = gsl_block_uint_alloc (1);
  saC.popnlistDELETE[npops][1]->size = 1;
  saC.popnlistDELETE[npops][1]->data[0] = npops;
  saC.popnlistDELETE[npops][0] = gsl_block_uint_alloc (npops);
  saC.popnlistDELETE[npops][0]->size = npops;
  for (i = 0; i < npops; i++)
    {
      saC.popnlistDELETE[i] = malloc (nperiods * sizeof(gsl_block_uint *)); 
      saC.popnlistDELETE[i][1] = NULL;
      saC.popnlistDELETE[i][0] = gsl_block_uint_alloc (1);
      saC.popnlistDELETE[i][0]->size = 1;
      saC.popnlistDELETE[i][0]->data[0] = i;
      saC.popnlistDELETE[npops][0]->data[i] = i;
    }
***********************************/

  /* popnlistB: bit set version of popnlist */
  BitSetPowerSetNew(saC.popnlistB, ssizePi, npnodes, nperiods, biti, bitj);
  for (i = 0; i < npnodes; i++)
    {
      for (j = 0; j < nperiods; j++)
        {
          if (saC.popnlist[i][j] != NULL)
            {
              n = saC.npopnlist[i][j];
              for (k = 0; k < n; k++)
                {
                  ip = saC.popnlist[i][j][k];
                  BitTrue (saC.popnlistB[i][j], ip);
                }
            }
        }
    }

  /**********************************************************************/
  /*                        popnmig nosplit                             */
  /**********************************************************************/
  /* numsplittimes: npops - 1 for tree model, and 0 for island model.   */
  /* We could have a variable for number of periods.                    */
  /* numpopsizeparams: 2*npops - 1 for tree model, and npops for island */
  saC.popnmig = malloc ((numsplittimes + 1) * sizeof (int **));
  for (ti = 0; ti < numsplittimes + 1; ti++)
    {
      saC.popnmig[ti] = malloc (numpopsizeparams * sizeof (int *));
      for (pi = 0; pi < numpopsizeparams; pi++)
        {
          /* #_populations to move for period ti, population pi */
          isExist = 0;
          for (ni = 0; ni < npops - ti; ni++)
            {
              if (C[sci]->plist[ti][ni] == pi)
                {
                  isExist = 1;
                }
            }
          if (isExist == 1)
            {
              n = npops - ti - 1;
            }
          else
            {
              n = 0;
            }

          if (n > 0)
            {
              saC.popnmig[ti][pi] = malloc (n * sizeof (int));
              i = 0;
              for (ni = 0; ni < npops - ti; ni++)
                {
                  if (C[sci]->plist[ti][ni] != pi)
                    {
                      saC.popnmig[ti][pi][i] = C[sci]->plist[ti][ni];
                      i++;
                    }
                }
            }
          else
            {
              saC.popnmig[ti][pi] = NULL;
            }
        }
    }

  return; 
}
void
imaPtreeSet ()
{
  int ti;
  int ni;
  int pi;
  int prevti;
  int i;
  int j;
  int k;
  int isExist;
  int ipExist;
  int sizeExist;
  int npnodes = 2 * npops - 1;
  struct popedge *pnode;
  int biti;
  int bitj;
  int n;
  int ip;

  assert (!(npops < 0)); 
  assert (npops < MAXPOPS); 

  ssizePi = ((2 * npops - 1) - 1) / 8 + 1;

  BitPowerSetNew (saC.Ps, ssizePi, npops, biti);
  for (ti = 0; ti < npops; ti++)
    {
      for (ni = 0; ni < npops - ti; ni++)
        {
          assert (!(C[sci]->plist[ti][ni] < 0));
          BitTrue (saC.Ps[ti], C[sci]->plist[ti][ni]);
        }
    }

  /* WE MAKE A FUNCTION OF THIS POPNDOWN SETUP. */
  saC.popndown = malloc ((2 * npops - 1) * sizeof(int *));
  for (pi = 0; pi < 2 * npops - 1; pi++)
    {
      saC.popndown[pi] = malloc (npops * sizeof(int));
      for (ti = 0; ti < npops; ti++)
        {
          saC.popndown[pi][ti] = -1;
        }
    }
  for (prevti = npops - 1; prevti > 0; prevti--)
    {
      ti = prevti - 1; 
      saC.popndown[C[sci]->addpop[prevti]][prevti] = C[sci]->addpop[prevti];
      saC.popndown[C[sci]->droppops[prevti][0]][ti] = C[sci]->droppops[prevti][0];
      saC.popndown[C[sci]->droppops[prevti][1]][ti] = C[sci]->droppops[prevti][1];
      for (i = prevti; i < npops; i++)
        {
          saC.popndown[C[sci]->droppops[prevti][0]][i] = 
            saC.popndown[C[sci]->addpop[prevti]][i];
          saC.popndown[C[sci]->droppops[prevti][1]][i] = 
            saC.popndown[C[sci]->addpop[prevti]][i];
        }
      for (i = 0; i < 2 * npops - 1; i++)
        {
          if (i == C[sci]->droppops[prevti][0] || i == C[sci]->droppops[prevti][1])
            {
              continue;
            }
          if (BitIsTrue (saC.Ps[ti], i))
            {
              saC.popndown[i][ti] = saC.popndown[i][prevti];
            }
        }
    }

  /**********************************************************************/
  /*                           IMA_popnlist_set                         */
  /**********************************************************************/
  assert (!(npops < 0));
  assert (npops < MAXPOPS);


  saC.popnlist = malloc (npnodes * sizeof (int **));
  saC.npopnlist = malloc (npnodes * sizeof (int *));
  for (i = 0; i < npnodes; i++)
    {
      /* Note: npops == num. of periods */
      saC.popnlist[i] = malloc (npops * sizeof(int *));  
      saC.npopnlist[i] = malloc (npops * sizeof (int));
      for (j = 0; j < npops; j++)
        {
          saC.popnlist[i][j] = NULL;
          saC.npopnlist[i][j] = 0;
        }
    }
  for (i = 0; i < npnodes; i++)
    {
      pnode = &C[sci]->poptree[i]; 
      if (pnode->e == -1)
        {
          k = pnode->b + 1;
        }
      else
        {
          k = pnode->e;
        }
      for (j = pnode->b; j < k; j++)
        {
          saC.popnlist[i][j] = malloc (sizeof (int));
          saC.popnlist[i][j][0] = i;
          saC.npopnlist[i][j] = 1; /* size */
        }
      for (j = pnode->b - 1; !(j < 0); j--)
        {
          isExist = 0; 
          ipExist = -1;
          sizeExist = saC.npopnlist[i][j + 1];
          for (k = 0; k < sizeExist; k++)
            {
              if (saC.popnlist[i][j + 1][k] == C[sci]->addpop[j + 1])
                {
                  isExist = 1;
                  ipExist = k;
                }
            }
          if (isExist == 1)
            {
              saC.popnlist[i][j] = malloc ((sizeExist + 1) * sizeof (int));
              saC.npopnlist[i][j] = sizeExist + 1; /* size */
            }
          else
            {
              saC.popnlist[i][j] = malloc (sizeExist * sizeof (int));
              saC.npopnlist[i][j] = sizeExist; /* size */
            }
          for (k = 0; k < sizeExist; k++)
            {
              saC.popnlist[i][j][k] = saC.popnlist[i][j + 1][k];
            }
          if (isExist == 1)
            {
              saC.popnlist[i][j][sizeExist] = C[sci]->droppops[j + 1][0];
              saC.popnlist[i][j][ipExist] = C[sci]->droppops[j + 1][1];
            }
        }
    }

  /* popnlistB: bit set version of popnlist */
  BitSetPowerSetNew(saC.popnlistB, ssizePi, npnodes, npops, biti, bitj);
  for (i = 0; i < npnodes; i++)
    {
      for (j = 0; j < npops; j++)
        {
          if (saC.popnlist[i][j] != NULL)
            {
              n = saC.npopnlist[i][j];
              for (k = 0; k < n; k++)
                {
                  ip = saC.popnlist[i][j][k];
                  BitTrue (saC.popnlistB[i][j], ip);
                }
            }
        }
    }

  /**********************************************************************/
  /*                       popnmig tree model                           */
  /**********************************************************************/
  /* numsplittimes: npops - 1 for tree model, and 0 for island model.   */
  /* We could have a variable for number of periods.                    */
  /* numpopsizeparams: 2*npops - 1 for tree model, and npops for island */
  saC.popnmig = malloc ((numsplittimes + 1) * sizeof (int **));
  for (ti = 0; ti < lastperiodnumber; ti++)
    {
      saC.popnmig[ti] = malloc (numpopsizeparams * sizeof (int *));
      for (pi = 0; pi < numpopsizeparams; pi++)
        {
          /* #_populations to move for period ti, population pi */
          isExist = 0;
          for (ni = 0; ni < npops - ti; ni++)
            {
              if (C[sci]->plist[ti][ni] == pi)
                {
                  isExist = 1;
                }
            }
          if (isExist == 1)
            {
              n = npops - ti - 1;
            }
          else
            {
              n = 0;
            }

          if (n > 0)
            {
              saC.popnmig[ti][pi] = malloc (n * sizeof (int));
              i = 0;
              for (ni = 0; ni < npops - ti; ni++)
                {
                  if (C[sci]->plist[ti][ni] != pi)
                    {
                      saC.popnmig[ti][pi][i] = C[sci]->plist[ti][ni];
                      i++;
                    }
                }
            }
          else
            {
              saC.popnmig[ti][pi] = NULL;
            }
        }
    }

  return; 
}

void
imaPtreeUnset ()
{
  int pi;
  int i;
  int j;
  int biti;
  int bitj;
  int npnodes; 
  int nperiods;
  int ti;

  if (assignmentoptions[POPULATIONASSIGNMENTINFINITE] == 1)
    {
      npnodes = npops + 1;
      nperiods = 2;
    }
  else
    {
      npnodes = 2 * npops - 1;
      nperiods = npops;
    }

  assert (!(npops < 0)); 
  assert (npops < MAXPOPS); 

  BitPowerSetDelete (saC.Ps, nperiods, biti);
  for (pi = 0; pi < npnodes; pi++)
    {
      free (saC.popndown[pi]);
      saC.popndown[pi] = NULL;
    }
  free (saC.popndown);
  saC.popndown = NULL;

  
  /**********************************************************************/
  /*                           IMA_popnlist_set                         */
  /**********************************************************************/
  for (i = 0; i < npnodes; i++)
    {
      for (j = 0; j < nperiods; j++)
        {
          if (saC.popnlist[i][j] != NULL)
            {
              XFREE (saC.popnlist[i][j]);
            }
        }
      XFREE (saC.popnlist[i]);
      XFREE (saC.npopnlist[i]);
    }
  XFREE (saC.popnlist);
  XFREE (saC.npopnlist);
  BitSetPowerSetDelete(saC.popnlistB, npnodes, nperiods, biti, bitj);

  /**********************************************************************/
  /*                               popnmig                              */
  /**********************************************************************/
  for (ti = 0; ti < lastperiodnumber; ti++)
    {
      for (pi = 0; pi < numpopsizeparams; pi++)
        {
          if (saC.popnmig[ti][pi] != NULL)
            {
              free (saC.popnmig[ti][pi]);
              saC.popnmig[ti][pi] = NULL;
            }
        }
      free (saC.popnmig[ti]);
      saC.popnmig[ti] = NULL;
    }
  free (saC.popnmig);
  saC.popnmig = NULL;

  return;
}

void
Phylogeny_setPopndown (int *** popndown, struct chain * t)
{
  int pi;
  int ti;

  /* WE MAKE A FUNCTION OF THIS POPNDOWN SETUP. */

  assert (*popndown == NULL);

  *popndown = malloc ((2 * npops - 1) * sizeof(int *));
  for (pi = 0; pi < 2 * npops - 1; pi++)
    {
      (*popndown)[pi] = malloc (npops * sizeof(int));
      for (ti = 0; ti < npops; ti++)
        {
          (*popndown)[pi][ti] = -1;
        }
    }

  Phylogeny_resetPopndown (*popndown, t);
}

void
Phylogeny_resetPopndown (int ** popndown, struct chain * t)
{
  int pi;
  int ti;
  int i;
  int prevti;

  /* WE MAKE A FUNCTION OF THIS POPNDOWN SETUP. */
  assert (popndown != NULL);
  for (pi = 0; pi < 2 * npops - 1; pi++)
    {
      for (ti = 0; ti < npops; ti++)
        {
          (popndown)[pi][ti] = -1;
        }
    }
  for (prevti = npops - 1; prevti > 0; prevti--)
    {
      ti = prevti - 1; 
      (popndown)[t->addpop[prevti]][prevti] = t->addpop[prevti];
      (popndown)[t->droppops[prevti][0]][ti] = t->droppops[prevti][0];
      (popndown)[t->droppops[prevti][1]][ti] = t->droppops[prevti][1];
      for (i = prevti; i < npops; i++)
        {
          (popndown)[t->droppops[prevti][0]][i] = 
            (popndown)[t->addpop[prevti]][i];
          (popndown)[t->droppops[prevti][1]][i] = 
            (popndown)[t->addpop[prevti]][i];
        }
      for (i = 0; i < 2 * npops - 1; i++)
        {
          if (i == t->droppops[prevti][0] || i == t->droppops[prevti][1])
            {
              continue;
            }
          /* if (BitIsTrue (saC.Ps[ti], i)) */
          if (ISELEMENT (i, t->periodset[ti]))
            {
              (popndown)[i][ti] = (popndown)[i][prevti];
            }
        }
    }
}

void
Phylogeny_setPopnmig (int **** popnmig, struct chain * t)
{
  int ti;
  int pi;
  int ni;
  int n;
  int i;
  int isExist;

  /**********************************************************************/
  /*                       popnmig tree model                           */
  /**********************************************************************/
  /* numsplittimes: npops - 1 for tree model, and 0 for island model.   */
  /* We could have a variable for number of periods.                    */
  /* numpopsizeparams: 2*npops - 1 for tree model, and npops for island */

  (*popnmig) = malloc ((numsplittimes + 1) * sizeof (int **));
  for (ti = 0; ti < lastperiodnumber; ti++)
    {
      (*popnmig)[ti] = malloc (numpopsizeparams * sizeof (int *));
      for (pi = 0; pi < numpopsizeparams; pi++)
        {
          /* #_populations to move for period ti, population pi */
          isExist = 0;
          for (ni = 0; ni < npops - ti; ni++)
            {
              if (t->plist[ti][ni] == pi)
                {
                  isExist = 1;
                }
            }
          if (isExist == 1)
            {
              n = npops - ti - 1;
            }
          else
            {
              n = 0;
            }

          if (n > 0)
            {
              (*popnmig)[ti][pi] = malloc (n * sizeof (int));
              i = 0;
              for (ni = 0; ni < npops - ti; ni++)
                {
                  if (t->plist[ti][ni] != pi)
                    {
                      (*popnmig)[ti][pi][i] = t->plist[ti][ni];
                      i++;
                    }
                }
            }
          else
            {
              (*popnmig)[ti][pi] = NULL;
            }
        }
    }
  return;
}

char* 
imaPtreePrint (int ci)
{
  char *ns = NULL;
  FILE *fp;
  struct popedge *ptree = NULL; /* genealogy of locus li */
  
  ptree = C[ci]->poptree;
  fp = fopen ("2.tre", "a");
  imaPnodePrint (fp, ptree, C[ci]->rootpop);
  fclose (fp);
  fp = NULL;
  
  return ns;
}

void 
imaPnodePrint_node (FILE *fp, struct popedge *ptree, int e)
{
  /* fprintf (fp, "%d[%d]", e, ptree[e].pop); */
  fprintf (fp, "%d", e);

  return;
}

void 
imaPnodePrint (FILE *fp, struct popedge *ptree, int e)
{
  int left;
  int right;
  int parent;

  left = ptree[e].up[0];
  right = ptree[e].up[1];
  parent = ptree[e].down;

  assert ((left < 0 && right < 0) || !(left < 0 && right < 0));

  if (left < 0 && right < 0)
    {
      imaPnodePrint_node (fp, ptree, e);
    }
  else
    {
      fprintf (fp, "(");
      imaPnodePrint (fp, ptree, left);
      fprintf (fp, ",");
      imaPnodePrint (fp, ptree, right);
      fprintf (fp, ")");
      imaPnodePrint_node (fp, ptree, e);
    }
  if (parent < 0)
    {
      fprintf (fp, ";\n");
    }
  else
    {
      fprintf (fp, ":%lf", ptree[e].time - ptree[left].time);
    }
  return;
}

void
imaSavedSaveLocusInfo (int ci, int li)
{
  int ai;
  struct genealogy *G = NULL;
  G = &(C[ci]->G[li]);
  saT[li].savedroot = G->root;
  saT[li].savedmignum = G->mignum;
  saT[li].savedroottime = G->roottime;
  saT[li].savedlength = G->length;
  saT[li].savedtlength = G->tlength;
  saT[li].savedpdg = G->pdg;        
  for (ai = 0; ai < L[li].nlinked; ai++)
    {
      saT[li].savedpdg_a[ai] = G->pdg_a[ai];
    }
  return;
}

void
imaSavedRestoreLocusInfo (int ci, int li)
{
  int ai;
  struct genealogy *G = NULL;
  G = &(C[ci]->G[li]);
  G->root = saT[li].savedroot;
  G->mignum = saT[li].savedmignum;
  G->roottime = saT[li].savedroottime;
  G->length = saT[li].savedlength;
  G->tlength = saT[li].savedtlength;
  G->pdg = saT[li].savedpdg;
  for (ai = 0; ai < L[li].nlinked; ai++)
    {
      G->pdg_a[ai] = saT[li].savedpdg_a[ai];
    }
  return;
}

void 
imaSavedReset (int li)
{
  saT[li].savedi = 0;
  return;
}

void 
imaSavedInit ()
{
  int li;
  int ngnodes;
  struct genealogy *G;

  saT = NULL; 

  init_genealogy_weights (&saC.savedallgweight); 
  init_probcalc (&saC.savedallpcalc);
  saT = malloc (nloci * sizeof (im_savedlocus));
  if (saT == NULL)
    {
      IM_err (IMERR_MEM, "saT");
    }
  for (li = 0; li < nloci; li++)
    {
      G = &C[0]->G[li];
      ngnodes = L[li].numlines;
      saT[li].nlinked = L[li].nlinked;
      saT[li].model = L[li].model;
      saT[li].gtree = NULL;
      saT[li].gtree = malloc (ngnodes * sizeof (im_savedlocus));
      if (saT[li].gtree == NULL)
        {
          IM_err (IMERR_MEM, "sat[%d].gtree", li);
        }
      memset (saT[li].gtree, 0, ngnodes * sizeof (im_savedlocus));
      saT[li].saved = NULL;
      saT[li].savedi = 0;
      saT[li].savedn = 0;
      init_genealogy_weights (&saT[li].savedgweight); 
      saT[li].savedpdg_a = NULL;
      saT[li].savedpdg_a = malloc (L[li].nlinked * sizeof (double));
      if (saT[li].savedpdg_a == NULL)
        {
          IM_err (IMERR_MEM, "saT[%d] savedpdg_a", li);
        }
    }

  return;
}

void 
imaSavedFree ()
{
  struct edge *saved = NULL; /* genealogy of locus li */
  int savedi;
  int savedn;
  int si;
  int li;
  int ei;
  int ngnodes;
  im_savedlocus *G;

  for (li = 0; li < nloci; li++)
    {
      G = &saT[li];
      saved = G->saved;
      savedi = G->savedi;
      savedn = G->savedn;

      for (si = 0; si < savedn; si++)
        {
          free (saved[si].mig); 
          saved[si].mig = NULL;
          free (saved[si].A);
          saved[si].A = NULL;
          free (saved[si].dlikeA);
          saved[si].dlikeA = NULL;
        }
      free (saved);
      saved = NULL;
      free (G->savedpdg_a);
      G->savedpdg_a = NULL;

      /* member gtree of im_savedlocus */
      ngnodes = L[li].numlines;
      for (ei = 0; ei < ngnodes; ei++)
        {
          if (G->gtree[ei].cmm > 0)
            {
              free (G->gtree[ei].savedmig);
              G->gtree[ei].savedmig = NULL;
              G->gtree[ei].cmm = 0;
            }
        }
      free (G->gtree);
      G->gtree = NULL;
    }
  free (saT);
  saT = NULL;

  return;
}

int
imaSavedSaveBranch (int ci, int li, int ei)
{
  int si;
  int ai;
  int mi;
  int savedi;
  int savedn;
  im_savedlocus *G = NULL;
  struct edge *gtree = NULL;
  struct edge *saved = NULL;

  G = &saT[li];
  gtree = C[ci]->G[li].gtree;
  assert (!(ei < 0));
  assert (ei < L[li].numlines);
  if (!(G->savedi < G->savedn))
    {
      /* allocate more memory of saved */
      G->savedn++;
      G->saved = realloc (G->saved, G->savedn * sizeof(struct edge));
      G->saved[G->savedi].cmm = 0;
      G->saved[G->savedi].mig = NULL;

      G->saved[G->savedi].A = malloc (L[li].nlinked * sizeof (int));
      G->saved[G->savedi].dlikeA = malloc (L[li].nlinked * sizeof (double));

    }
  gtree = C[ci]->G[li].gtree;
  saved = G->saved;
  savedi = G->savedi;
  savedn = G->savedn;

  /* Should we check this loop first?          */
  /* We may use bitset to check the existence. */
  for (si = 0; si < savedi; si++)
    {
      if (saved[si].ei == ei)
        {
          return si;
        }
    }

  saved[savedi].ei = ei;
  saved[savedi].up[0] = gtree[ei].up[0]; 
  saved[savedi].up[1] = gtree[ei].up[1]; 
  saved[savedi].down = gtree[ei].down;
  saved[savedi].time = gtree[ei].time;
  saved[savedi].pop = gtree[ei].pop;

  if (L[li].model == STEPWISE || L[li].model == JOINT_IS_SW)
    {       
      for (ai = (L[li].model == JOINT_IS_SW); 
           ai < L[li].nlinked;
           ai++)
        {
          saved[savedi].A[ai] = gtree[ei].A[ai];
          saved[savedi].dlikeA[ai] = gtree[ei].dlikeA[ai];
        }
    }

  if (modeloptions[NOMIGRATION] == 0)
    {
      if (saved[savedi].cmm < gtree[ei].cmm)
        {
          saved[savedi].cmm = gtree[ei].cmm;
          saved[savedi].mig = realloc (saved[savedi].mig, saved[savedi].cmm *
                                       sizeof (struct migstruct));
        }
      for (mi = 0; mi < gtree[ei].cmm; mi++)
        {
          saved[savedi].mig[mi].mt = gtree[ei].mig[mi].mt;
          saved[savedi].mig[mi].mp = gtree[ei].mig[mi].mp;
          if (gtree[ei].mig[mi].mt < -0.5)
            {
              break;
            }
        }
    }

  G->savedi++;
  return savedi; 
}

void 
imaSavedRestore (int ci, int li)
{
  im_savedlocus *G = NULL;
  struct edge *gtree = NULL; /* genealogy of locus li */
  struct edge *saved = NULL; /* genealogy of locus li */
  int savedi;
  int savedn;
  int mi;
  int si;
  int ei;
  int ai;

  G = &saT[li];
  gtree = C[ci]->G[li].gtree;
  saved = G->saved;
  savedi = G->savedi;
  savedn = G->savedn;

  for (si = 0; si < savedi; si++)
    {
      ei = saved[si].ei;
      assert (!(ei < 0));
      assert (ei < L[li].numlines);
      gtree[ei].up[0] = saved[si].up[0]; 
      gtree[ei].up[1] = saved[si].up[1]; 
      gtree[ei].down = saved[si].down;
      gtree[ei].time = saved[si].time;
      gtree[ei].pop = saved[si].pop;

      if (L[li].model == STEPWISE || L[li].model == JOINT_IS_SW)
        {       
          for (ai = (L[li].model == JOINT_IS_SW); 
               ai < L[li].nlinked;
               ai++)
            {
              gtree[ei].A[ai] = saved[si].A[ai];
              gtree[ei].dlikeA[ai] = saved[si].dlikeA[ai];
            }
        }

      if (modeloptions[NOMIGRATION] == 0)
        {
          for (mi = 0; mi < saved[si].cmm; mi++)
            {
              assert (mi < gtree[ei].cmm);
              gtree[ei].mig[mi].mt = saved[si].mig[mi].mt;
              gtree[ei].mig[mi].mp = saved[si].mig[mi].mp;
              if (gtree[ei].mig[mi].mt < -0.5)
                {
                  break;
                }
            }
        }
    }
  return;
}

void
imaStringRemoveSpaces (char *s1, const char *s2)
{
  int j;
  int i;
  int len;

  len = strlen (s2);
  j = 0;
  for (i = 0; i < len; i++)
    {
      if (isalnum(s2[i]) 
          || s2[i] == '_' 
          || s2[i] == '-'
          || s2[i] == '@')
        {
          s1[j] = s2[i];
          j++;
        }
    }
  s1[j] = '\0';
  return;
}
int 
Phylogeny_tptChooseAllPopulations (int ci, int rperiod, 
                                       int *popal, int *popbl, int *popcl,
                                       int *popau, int *popbu, int *popcu)
{
  *popau = C[ci]->droppops[rperiod][0];
  *popbu = C[ci]->droppops[rperiod][1];
  *popcu = C[ci]->addpop[rperiod];
  *popal = C[ci]->droppops[rperiod + 1][0];
  *popbl = C[ci]->droppops[rperiod + 1][1];
  *popcl = C[ci]->addpop[rperiod + 1];
  return 0;
}


double 
priorDivTime (int ci, double r, double x)
{
  double v;
  double loglambda;
  double logr;
  double logmu;
  double s;
  int ti;

  logr = log(r);
  loglambda = log(r/(1.0 - x));
  logmu = log(x) + loglambda;

  LogDiff(v, loglambda, logmu - r * C[ci]->tvals[npops-2]);
  /* npops! is ignored because we do not change number of populations. */
  s = (npops - 1.0) * loglambda + logr - r * C[ci]->tvals[npops-2] - v;
  for (ti = 0; ti < npops - 1; ti++)
    {
      LogDiff(v, loglambda, logmu - r * C[ci]->tvals[ti]);
      s += (2.0 * logr - r * C[ci]->tvals[ti] - v);
    }
  return s;
}

/*
 * Yang and Rannala (1997) Bayesian Phylogenetic Inference Using DNA Sequences:
 * A Markov Chain Monte Carlo Method. Mol. Biol. Evol. 14(7):717--724
 * The code is from treespace.c of PAML 4.4. We adapt the code to IMa2.
 */
int GetIofLHistory (int ci)
{
    /* Get the indexH of the labelled history (rooted tree with nodes ordered
     according to time).  
     Numbering of nodes: node # increases as the node gets older:
     node d corresponds to time 2*ns-2-d; tree.root=ns*2-2;
     t0=1 > t1 > t2 > ... > t[ns-2]
     */
    struct popedge * ptree;
    int indexH, i, j, k[MAXPOPS+1], inode, nnode, nodea[MAXPOPS * 2 - 1], s[2];
    int ancestor;
    int daughter;
    
    assert (npops < MAXPOPS); /* We cannot handle big integers. */
    ptree = C[ci]->poptree;

    /* Ordered indexH of all internal nodes. */
    /* FOR (i,npops) */
    for (i = 0; i < npops; i++)
    {
        nodea[i] = i;
    }

    for (i = 1; i < npops; i++)
    {
        nodea[C[ci]->addpop[i]] = i + npops;
    }

    /* Internal nodes should be ordered in time. */
    for (i = 1, inode = npops, nnode = npops; 
         inode < npops * 2 - 1; 
         i++, inode++, nnode--) 
    {
        ancestor = C[ci]->addpop[i];
        for (j = 0; j < 2; j++)
        {
            daughter = ptree[ancestor].up[j];
            s[j] = nodea[daughter];
        }
        k[nnode] = IMAX(s[0],s[1]); 
        s[0] = IMIN(s[0],s[1]); 
        s[1] = k[nnode];
        k[nnode] = s[1]*(s[1]-1)/2 + s[0];
    }
    indexH = 0;
    for (nnode=2; nnode <= npops; nnode++)
    {
        indexH = nnode*(nnode-1)/2*indexH + k[nnode];
    }
    return (indexH);
}


/*
 * Yang and Rannala (1997) Bayesian Phylogenetic Inference Using DNA Sequences:
 * A Markov Chain Monte Carlo Method. Mol. Biol. Evol. 14(7):717--724
 * The code is from treespace.c of PAML 4.4. We adapt the code to IMa2.
 */
int GetLHistoryI (int ci, int iLH)
{
    /* Get the ILH_th labelled history.  This function is rather similar to 
     GetTreeI which returns the I_th rooted or unrooted tree topology.
     The labeled history is recorded in the numbering of nodes: 
     node # increases as the node gets older: 
     node d corresponds to time 2*ns-2-d; tree.root=ns*2-2;
     t0=1 > t1 > t2 > ... > t[ns-2]
     k ranges from 0 to i(i-1)/2 and indexes the pair (s1 & s2, with s1<s2),
     out of i lineages, that coalesce.
     */
    int i,k, inode, nodea[MAXPOPS], s1, s2, it;
    struct popedge * ptree;
    int daughter0;
    int daughter1;
    double t;
    int rootnode;
    
    assert (npops < 10);
    assert (C[ci]->poptree == NULL);
    
    C[ci]->rootpop = npops * 2 - 2;
    C[ci]->poptree = malloc ((npops * 2 + 1) * sizeof (struct popedge));
    ptree = C[ci]->poptree;
    for (i = 0; i < npops * 2 - 1; i++)  {
        ptree->numup = 2;
        ptree->up = malloc (2 * sizeof (int));
        ptree->b = 0;
        ptree->up[0] = -1;
        ptree->up[1] = -1;
        ptree++;
    }
 
    t = 0.0;
    for (i = 0; i < npops; i++) nodea[i]=i;
    ptree = C[ci]->poptree;
    it = iLH;
    for (i = npops, inode = npops; i >= 2; i--, inode++)  
    {
        k = it%(i*(i-1)/2);  
        it /= (i*(i-1)/2); 
        s2 = (int)(sqrt(1.+8*k)-1)/2+1;  
        s1 = k-s2*(s2-1)/2; /* s1<s2, k=s2*(s2-1)/2+s1 */
        assert (s1 < s2);
        if (s1 >= s2)
        {
          IM_errloc (AT, "Tree Generation Error using index %d", iLH);
        }
        t += expo (1.0);
        daughter0 = nodea[s1];
        daughter1 = nodea[s2];
        ptree[daughter0].down = inode;
        ptree[daughter0].e = inode - npops + 1;
        ptree[daughter0].time = t; /* e.g., See Gernhard (2009) */
        ptree[daughter1].down = inode;
        ptree[daughter1].e = inode - npops + 1;
        ptree[daughter1].time = t; /* e.g., See Gernhard (2009) */
        ptree[inode].up[0] = daughter0;
        ptree[inode].up[1] = daughter1;
        ptree[inode].b = inode - npops + 1;
        nodea[s1] = inode;
        nodea[s2] = nodea[i-1]; 
    }
    rootnode = inode - 1;
    ptree[rootnode].down = -1;
    ptree[rootnode].e = -1;
    ptree[rootnode].time = TIMEMAX;
    assert (C[ci]->rootpop == inode - 1); 
    return (0);

    /* Set the following members of the chain.
     * periodset <- fillplist
     * addpop
     * droppops
     * plist
     * newickstring <- (not yet)
     * poptreestring <- (not yet)
     * popndown <- Phylogeny_setPopndown
     * popnmig <- Phylogeny_setPopnmig (not used)
     **/
    fillplist (ci);
    assert (C[ci]->popndown == NULL);
    Phylogeny_setPopndown (&C[ci]->popndown, C[ci]);
    //assert (C[ci]->popnmig == NULL);
    //Phylogeny_setPopnmig (&C[ci]->popnmig, C[ci]);

    return 0;
}



int
updateassignmentptree (int ci)
{
  int rperiod; 
  int accp;
  double lweight;
  double logU;
  double mweight;
  double hweight;
  struct genealogy *G;
  int li;
  int popx;
  int popa;
  int popb;
  int popc;
  int popal;
  int popbl;
  int popcl;
  int popau;
  int popbu;
  int popcu;
  int iTypeMove;

  assert (assignmentoptions[POPULATIONASSIGNMENTINFINITE] == 0);

  /* Count up the number of tries of MCMC cycles. */
  if (ci == 0)
    {
      L[0].t_rec->upinf[IM_UPDATE_TREE_TPT_TYPE1].tries++;
    }

  G = NULL;
  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      assertgenealogy (ci);
    }

  iTypeMove = Phylogeny_choose_a_period (ci, &rperiod);
  switch (iTypeMove)
    {
      case 0:
        /* We do not need this? rperiod = tptChooseCoalescentPeriod (ci); */
        return 0;
        break;
      case 1:
        popc = Phylogeny_tptChooseAnIngroup (ci, rperiod, &popx, &popa);
        popb = Phylogeny_sister (ci, popc);
        break;
      case 2:
        Phylogeny_tptChooseAllPopulations (ci, rperiod, 
                                           &popal, &popbl, &popcl,
                                           &popau, &popbu, &popcu);
        break;
      default:
        assert (0);
    }

  /* Save the current state of a chain */
  for (li = 0; li < nloci; li++)
    {
      imaSavedReset (li);
      imaSavedSaveLocusInfo (ci, li);
      copy_treeinfo (&saT[li].savedgweight, &C[ci]->G[li].gweight);
    }
  copy_treeinfo (&saC.savedallgweight, &C[ci]->allgweight);
  /* We do not call copy_probcalc_all because we do not change likelihood. */
  copy_probcalc (&saC.savedallpcalc, &C[ci]->allpcalc);

  /* Propose a new state of genealogy and assignment. */
  switch (iTypeMove)
    {
      case 1:
        /* FIXME: we have not computed hweight correctly? LOG2's have not been
         * subtracted yet. */
        hweight = treemoveI (ci, rperiod, 
                             popx, popa, popb, popc);
        break;
      case 2:
        hweight = treemoveII (ci, rperiod, 
                              popal, popbl, popcl, 
                              popau, popbu, popcu);
        break; 
      default:
        assert (0);
        break;
    }

  /* Compute Metropolis-ratio. */
  setzero_genealogy_weights (&C[ci]->allgweight);
  for (li = 0; li < nloci; li++)
    {
      setzero_genealogy_weights (&C[ci]->G[li].gweight);
      treeweight (ci, li);
      sum_treeinfo (&C[ci]->allgweight, &C[ci]->G[li].gweight);
    }
  initialize_integrate_tree_prob (ci, &C[ci]->allgweight, &C[ci]->allpcalc);

  logU = log (uniform ());
  mweight = C[ci]->allpcalc.probg - saC.savedallpcalc.probg;
  lweight = beta[ci] * mweight + hweight;
  assert (beta[ci] * mweight + hweight > -1e200
          && beta[ci] * mweight + hweight < 1e200);

 
  accp = 0;

  if (lweight > 0.0 || lweight > logU)
    {
      /* accept the update */
      accp = 1;
      if (ci == 0)
        {
          L[0].t_rec->upinf[IM_UPDATE_TREE_TPT_TYPE1].accp++;
        }

#if 0
      /* Always revert to an original. */
      for (li = 0; li < nloci; li++)
        {
          if (L[li].model == HKY)
            {
              copyfraclike (ci, li);
              storescalefactors (ci, li);
            }
        }
#endif
    }
  else
    {
      /* Revert it to the original state state of genealogy and assignment. */
      switch (iTypeMove)
        {
          case 1:
            hweight = treemoveI (ci, rperiod, 
                                 popx, popb, popa, popc);
            break;
          case 2:
            hweight = treemoveII (ci, rperiod, 
                                  popau, popbu, popcu,
                                  popal, popbl, popcl); 
            break; 
          default:
            break;
        }
      for (li = 0; li < nloci; li++)
        {
          /* restore edges changed during update */
          imaSavedRestore (ci, li); 
          imaSavedRestoreLocusInfo (ci, li);
          copy_treeinfo (&C[ci]->G[li].gweight, &saT[li].savedgweight);
        }
      copy_treeinfo (&C[ci]->allgweight, &saC.savedallgweight);
      copy_probcalc (&C[ci]->allpcalc, &saC.savedallpcalc);
    }

#if 0
  for (li = 0; li < nloci; li++)
    {
      if (L[li].model == HKY)
        {
          restorescalefactors (ci, li);
        }
    }
#endif

  if (assignmentoptions[POPULATIONASSIGNMENTCHECKPOINT] == 1)
    {
      assertgenealogy (ci);
    }
  return accp;
}

void
recordptree_header (char *fname, int ci)
{
  int pi;
  int lenname;
  char *treefilename;
  FILE *treefile = NULL;
  int idummy;

  idummy = ci;

  /* we find individuals for each locus. */
  lenname = 30 + strlen (fname);
  treefilename = malloc (lenname * sizeof (char));

  strcpy (treefilename, fname);
  strcat (treefilename, ".nex.t");
  treefile = fopen (treefilename, "w");

  if (treefile == NULL)
    {
      IM_err (IMERR_CREATEFILEFAIL, 
              "Tree output file cannot be created: %s", 
              treefilename);
    }

  fprintf (treefile, "#NEXUS\n");
  fprintf (treefile, "[ID: 777]\n");
  fprintf (treefile, "begin trees;\n");
  fprintf (treefile, "   translate\n");
  for (pi = 0; pi < npops; pi++)
    {
      fprintf (treefile, "      %3d %s", pi + 1, popnames[pi]);
      if (pi == npops - 1)
        {
          fprintf (treefile, ";\n");
        }
      else
        {
          fprintf (treefile, ",\n");
        }
    }
 
  fclose (treefile);
  treefile = NULL;
  free (treefilename);
  treefilename = NULL;

  return;
}

void 
recordptree (char *fname, int ci)
{
  static int gen = 0;
#ifdef DEBUG
  int i;
#endif /* DEBUG */

  int lenname;
  char *treefilename;
  FILE *treefile = NULL;

  /* we find individuals for each locus. */
  lenname = 30 + strlen (fname);
  treefilename = malloc (lenname * sizeof (char));

  strcpy (treefilename, fname);
  strcat (treefilename, ".nex.t");
  treefile = fopen (treefilename, "a");

  if (treefile == NULL)
    {
      IM_err (IMERR_CREATEFILEFAIL, 
              "Tree output file cannot be created: %s", 
              treefilename);
    }

  /* Note that newickstring and popntreestring are in different format. */
  /* Check it out in build_poptree.c. */
  fprintf (treefile, "   tree rep.%d = %s\n", step, C[ci]->newickstring);

  fclose (treefile);
  treefile = NULL;
  free (treefilename);
  treefilename = NULL;

  gen++;
  return;
}

void
recordptree_footer (char *fname, int ci)
{
  int idummy;
  int lenname;
  char *treefilename;
  FILE *treefile = NULL;

  idummy = ci;

  /* we find individuals for each locus. */
  lenname = 30 + strlen (fname);
  treefilename = malloc (lenname * sizeof (char));

  strcpy (treefilename, fname);
  strcat (treefilename, ".nex.t");
  treefile = fopen (treefilename, "a");

  if (treefile == NULL)
    {
      IM_err (IMERR_CREATEFILEFAIL, 
              "Tree output file cannot be created: %s", 
              treefilename);
    }

  fprintf (treefile, "end;\n");

  fclose (treefile);
  treefile = NULL;
  free (treefilename);
  treefilename = NULL;

  return;
}


double
treemoveI (int ci, int rperiod, int popx, int popa, int popb, int popc)
{
  double lhweight;
  lhweight = 0.0;
  int v;
  int li;
  int ei;
  double t;     /* up time of an gtree edge. */
  double tdown; /* down time of the gtree edge. */

  int mi;
  struct edge *gtree;
  double mt;
  int mp;
  double upt;
  double dnt;
  int pi;
  SET set1;
  int j;
  int k;
  struct popedge * ptree;
  int pope;

  static int statici = 0;
  statici++;

  
  upt = C[ci]->tvals[rperiod - 1];
  dnt = C[ci]->tvals[rperiod]; 

  /********************************************************************/
  /* We change struct Chain and saC.                                  */
  /* static im_popntree saC;                                          */
  /********************************************************************/
  /* Change periodset: Check function fillplist. */
  set1 = C[ci]->periodset[rperiod];
  set1 = SETREMOVE (set1, popb);
  set1 = SETADD (set1, popa);
  C[ci]->periodset[rperiod] = set1;

  /* No change to addpop. */
  /* Change droppops. */
  C[ci]->droppops[rperiod][0] = popx;
  C[ci]->droppops[rperiod][1] = popb;
  C[ci]->droppops[rperiod + 1][0] = popa;
  C[ci]->droppops[rperiod + 1][1] = popc;

  /* Change plist. */
  j = 0; 
  FORALL (k, C[ci]->periodset[rperiod])
    {
      assert (j < npops - rperiod);
      C[ci]->plist[rperiod][j] = k;
      j++;
    }

  /* Change poptree. */
  ptree = C[ci]->poptree;
  pope = ptree[popc].down;
  ptree[popa].down = pope;
  ptree[popa].e = rperiod + 1;
  ptree[popa].time = dnt;
  ptree[popb].down = popc;
  ptree[popb].e = rperiod;
  ptree[popb].time = upt;
  assert (ptree[popc].up[0] == popa || ptree[popc].up[1] == popa); 
  ptree[popc].up[0] = popx;
  ptree[popc].up[1] = popb;
  ptree[pope].up[0] = popa;
  ptree[pope].up[1] = popc;

  /* Change char poptreestring[POPTREESTRINGLENGTHMAX]; ??? */
  /* poptreewrite (ci, C[ci]->poptreestring); */
  C[ci]->newickstring[0] = '\0';
  makenewicktreestring (ci, -1, &C[ci]->newickstring, &C[ci]->nnewickstring);

  /* No change to rootpop */
  /* No change to tvals */

  /* popndown and popnmig were a part of assignment project. We place the same
   * thing under struct Chain. */
  //C[ci]->popndown[popa][rperiod] = popa;
  //C[ci]->popndown[popb][rperiod] = popc;
  Phylogeny_resetPopndown (C[ci]->popndown, C[ci]);

  /* Search this document for "popnmig tree model" as a reference. I am not sure
   * whether we need this one.  Until then, we turn it off.  
   * In "popmig tree model" we use plist to create and set popnmig.  The memory
   * of the variable changes, and therefore make sure free and malloc.
   * int ***popnmig; --- per period, per popn, list of popn's ---
   */
  /* assert (C[ci]->popnmig == NULL); */

  /********************************************************************/
  /* End of Tree update                                               */
  /********************************************************************/

  v = 0;
  for (li = 0; li < nloci; li++)
    {
      gtree = C[ci]->G[li].gtree;
      for (ei = 0; ei < L[li].numlines; ei++)
        {
          t = imaGtreeTime (ci, li, ei);
          tdown = gtree[ei].time;

          if (tdown < upt || dnt < t)
            {
              continue;
            }

          imaSavedSaveBranch (ci, li, ei);

          if (t < upt && upt < tdown)
            {
              mi = Genealogy_skipMig (ci, li, ei, upt);
              pi = Migration_above_startpopn (ci, li, ei, mi, rperiod);
              /* pi = Migration_popnAtTime (ci, li, ei, upt); */
            }
          else
            {
              mi = 0;
            }

          mt = gtree[ei].mig[mi].mt;
          if (pi == popx)
            {
              while (mt > -0.5 && mt < dnt)
                {
                  /* Swap popa and popb. */
                  mp = gtree[ei].mig[mi].mp;
                  if (mp == popa)
                    {
                      gtree[ei].mig[mi].mp = popb;
                    }
                  else if (mp == popb)
                    {
                      gtree[ei].mig[mi].mp = popa;
                    }
                  else
                    {
                      /* No code */
                    }
                  mi++;
                  mt = gtree[ei].mig[mi].mt;
                }
            }
          else
            {
              while (mt > -0.5 && mt < dnt)
                {
                  /* Replace popc with popa, popb with popc. */
                  mp = gtree[ei].mig[mi].mp;
                  if (mp == popc)
                    {
                      gtree[ei].mig[mi].mp = popa;
                    }
                  else if (mp == popb)
                    {
                      gtree[ei].mig[mi].mp = popc;
                    }
                  else 
                    {
                      /* No code */
                    }
                  mi++;
                  mt = gtree[ei].mig[mi].mt;
                }
            }
        }
    }

  return lhweight; /* Everything is deterministic. */
}

double 
treemoveII (int ci, int rperiod, 
            int popal, int popbl, int popcl,
            int popau, int popbu, int popcu)
{
  double lhweight;
  int v;
  int li;
  int ei;
  double t;     /* up time of an gtree edge. */
  double tdown; /* down time of the gtree edge. */

  int mi;
  struct edge * gtree;
  double mt;
  int mp;
  double upt;
  double dnt;
  SET set1;
  int j;
  int k;
  struct popedge * ptree;

  upt = C[ci]->tvals[rperiod - 1];
  dnt = C[ci]->tvals[rperiod]; 

  /********************************************************************/
  /* We change struct Chain and saC.                                  */
  /* static im_popntree saC;                                          */
  /********************************************************************/
  /* Change periodset: Check function fillplist. */

  set1 = C[ci]->periodset[rperiod];
  set1 = SETREMOVE (set1, popal);
  set1 = SETREMOVE (set1, popbl);
  set1 = SETREMOVE (set1, popcu);
  set1 = SETADD (set1, popau);
  set1 = SETADD (set1, popbu);
  set1 = SETADD (set1, popcl);
  C[ci]->periodset[rperiod] = set1;

  /* Change droppops and addpop. */
  C[ci]->droppops[rperiod][0] = popal;
  C[ci]->droppops[rperiod][1] = popbl;
  C[ci]->addpop[rperiod] = popcl;
  C[ci]->droppops[rperiod + 1][0] = popau;
  C[ci]->droppops[rperiod + 1][1] = popbu;
  C[ci]->addpop[rperiod + 1] = popcu;

  /* Change plist. */
  j = 0; 
  FORALL (k, C[ci]->periodset[rperiod])
    {
      C[ci]->plist[rperiod][j] = k;
      j++;
    }

  /* Change poptree. */
  ptree = C[ci]->poptree;
  ptree[popal].e = rperiod;
  ptree[popal].time = upt;
  ptree[popbl].e = rperiod;
  ptree[popbl].time = upt;
  ptree[popcl].b = rperiod;

  ptree[popau].e = rperiod + 1;
  ptree[popau].time = dnt;
  ptree[popbu].e = rperiod + 1;
  ptree[popbu].time = dnt;
  ptree[popcu].b = rperiod + 1;

  /* Change char poptreestring[POPTREESTRINGLENGTHMAX]; ??? */
  /* poptreewrite (ci, C[ci]->poptreestring); */
  C[ci]->newickstring[0] = '\0';
  makenewicktreestring (ci, -1, &C[ci]->newickstring, &C[ci]->nnewickstring);

  /* No change to rootpop */
  /* No change to tvals */

  /* popndown and popnmig were a part of assignment project. We place the same
   * thing under struct Chain. */
  //C[ci]->popndown[popal][rperiod] = popcl;
  //C[ci]->popndown[popbl][rperiod] = popcl;
  //C[ci]->popndown[popcl][rperiod] = popcl;
  //C[ci]->popndown[popau][rperiod] = popau;
  //C[ci]->popndown[popbu][rperiod] = popbu;
  //C[ci]->popndown[popcu][rperiod] = -1;
  Phylogeny_resetPopndown (C[ci]->popndown, C[ci]);

  /* Search this document for "popnmig tree model" as a reference. I am not sure
   * whether we need this one.  Until then, we turn it off.  
   * In "popmig tree model" we use plist to create and set popnmig.  The memory
   * of the variable changes, and therefore make sure free and malloc.
   * int ***popnmig; --- per period, per popn, list of popn's ---
   */
  //assert (C[ci]->popnmig == NULL);

  /********************************************************************/
  /* End of Tree update                                               */
  /********************************************************************/


  v = 0;
  lhweight = 0.0;
  for (li = 0; li < nloci; li++)
    {    
      gtree = C[ci]->G[li].gtree;
      for (ei = 0; ei < L[li].numlines; ei++)
        {
          t = imaGtreeTime (ci, li, ei);
          tdown = gtree[ei].time;

          if (tdown < upt || dnt < t)
            {
              continue;
            }

          imaSavedSaveBranch (ci, li, ei);

          if (t < upt && upt < tdown)
            {
              mi = Genealogy_skipMig (ci, li, ei, upt);
              /* mp = Migration_startpopn (ci, li, ei, mi); */
              /* pi = Migration_popnAtTime (ci, li, ei, upt); */
            }
          else
            {
              mi = 0;
            }
          
          mt = gtree[ei].mig[mi].mt;

          while (mt > -0.5 && mt < dnt)
            {
              /* Merge popal and popbl to popcl. */
              /* Divide popcu to popau and popbu. */
              mp = gtree[ei].mig[mi].mp;
              if (mp == popal || mp == popbl)
                {
                  gtree[ei].mig[mi].mp = popcl;
                  lhweight -= LOG2;
                }
              else if (mp == popcu)
                {
                  if (bitran () /*uniform() < 0.5 */ )
                    {
                      gtree[ei].mig[mi].mp = popau;
                    }
                  else
                    {
                      gtree[ei].mig[mi].mp = popbu;
                    }
                  lhweight += LOG2;
                }
              else
                {
                  /* No code. */
                }
              mi++;
              mt = gtree[ei].mig[mi].mt;
            }
        }
    }

  return lhweight; /* Everything is deterministic. */
}


int 
Phylogeny_choose_a_period (int ci, int * rperiodv)
{
  /* It returns the type of tree update using Three-point-turn updating method. */
  int v;
  double upt;
  double dnt;
  int pi;
  int a1;
  int b1;
  int b2;
  int type;
  int i;
 
  assert (assignmentoptions[POPULATIONASSIGNMENTINFINITE] == 0);

  *rperiodv = -1;
  type = 0;
  for (i = 0; i < npops; i++)
    {
      pi = randposint (npops);
      if (pi == 0 || pi == npops - 1)
        {
          continue;
        }
      upt = C[ci]->tvals[pi - 1];
      dnt = C[ci]->tvals[pi];
      if (C[ci]->addpop[pi] == C[ci]->droppops[pi + 1][0]
          || C[ci]->addpop[pi] == C[ci]->droppops[pi + 1][1])
        {
          b1 = C[ci]->droppops[pi + 1][0];
          b2 = C[ci]->droppops[pi + 1][1];
          v = Phylogeny_isCoalescent (ci, upt, dnt, b1, b2);
          if (v == 0)
            {
              type = 1;
              *rperiodv = pi;
              break;
            }
        }
      else 
        {
          a1 = C[ci]->addpop[pi]; 
          b1 = C[ci]->droppops[pi + 1][0];
          b2 = C[ci]->droppops[pi + 1][1];
          v = Phylogeny_isMigration (ci, upt, dnt, b1, b2);
          if (v > 0)
            {
              continue;
            }
          v += Phylogeny_isCoalescent2 (ci, upt, dnt, a1, b1, b2);
          if (v == 0)
            {
              type = 2;
              *rperiodv = pi;
              break;
            }
        }
    }

  if (type == 0)
    {
      assert (*rperiodv == -1);
    }
  else if (type == 1 || type == 2)
    {
      assert (0 < *rperiodv && *rperiodv < npops - 1);
    }
  else
    {
      assert (0);
    }
  return type;

}
int 
Phylogeny_isMigration (int ci, double upt, double dnt, int b1, int b2)
{
  int v;
  int li;
  int ei;
  double t;     /* up time of an gtree edge. */
  double tdown; /* down time of the gtree edge. */

  int mi;
  struct edge *gtree;
  double mt;
  int mp;
  int mp0;

  v = 0;
  for (li = 0; li < nloci; li++)
    {
      gtree = C[ci]->G[li].gtree;
      for (ei = 0; ei < L[li].numlines; ei++)
        {
          t = imaGtreeTime (ci, li, ei);
          tdown = gtree[ei].time;

          if (!(tdown < upt) && !(dnt < t))
            {
              if (upt < t && t < dnt) /* Type II and IV */
                {
                  mi = 0;
                }
              else /* Type I and III */
                {
                  assert (t < upt);
                  mi = Genealogy_skipMig (ci, li, ei, upt);
                }
              mt = gtree[ei].mig[mi].mt;

              /* Check migration events between [[upt]] and [[dnt]]. */
              /* For Type II and IV, the second condition can be deleted. */
              while (mt > -0.5 && mt < dnt) 
                {
                  /* Find the starting population. */
                  mp0 = Migration_startpopn (ci, li, ei, mi);

                  /* Find the ending population. */
                  mp = gtree[ei].mig[mi].mp;    

                  /* Check if the migration event is between b1 and b2. */
                  if ((mp0 == b1 && mp == b2) || (mp0 == b2 && mp == b1))
                    {
                      v = 1;
                      return v;
                      ei = L[li].numlines;
                      li = nloci;
                      break;
                    }
                  mi++;
                  mt = gtree[ei].mig[mi].mt;    
                }
            }
        }
    }

  return v;
}

int 
Genealogy_skipMig (int ci, int li, int ei, double t)
{
  int mi;
  struct edge *gtree;
  double mt;

  gtree = C[ci]->G[li].gtree;
  mi = 0;
  /* Skip all migration events before [[t]]. */
  mt = gtree[ei].mig[mi].mt;    
  while (mt > -0.5 && mt < t)
    {
      mi++;
      mt = gtree[ei].mig[mi].mt;
    }
  return mi;
}

int 
Migration_above_startpopn (int ci, int li, int ei, int mi, int ki)
{
  int mp;
  struct edge *gtree;

  assert (!(mi < 0)); 
  gtree = C[ci]->G[li].gtree;
  if (mi == 0)
    {
      mp = gtree[ei].pop;
    }
  else
    {
      mp = gtree[ei].mig[mi-1].mp;
    }
  mp = C[ci]->popndown[mp][ki-1];

  return mp;
}
 
int 
Migration_startpopn (int ci, int li, int ei, int mi)
{
  int mp0;
  double mt;
  int ki;
  struct edge *gtree;

  assert (!(mi < 0)); 
  gtree = C[ci]->G[li].gtree;
  if (mi == 0)
    {
      mp0 = gtree[ei].pop;
    }
  else
    {
      mp0 = gtree[ei].mig[mi-1].mp;
    }
  mt = gtree[ei].mig[mi].mt;
  ki = findperiodoftime (ci, mt);
  mp0 = C[ci]->popndown[mp0][ki];

  return mp0;
}
 
int 
Phylogeny_isCoalescent (int ci, double upt, double dnt, int b1, int b2)
{
  int v;
  int li;
  int ei;
  double t;
  int b;

  v = 0;
  for (li = 0; li < nloci; li++)
    {
      for (ei = 0; ei < L[li].numlines; ei++)
        {
          t = imaGtreeTime (ci, li, ei);
          b = imaGtreePop (ci, li, ei);
          if (upt < t && t < dnt && (b == b1 || b == b2))
            {
              v = 1;
              li = nloci;
              break;
            }
        }
    }
  return v;
}

int 
Phylogeny_isCoalescent2 (int ci, double upt, double dnt, int b1, int b2, int b3)
{
  int v;
  int li;
  int ei;
  double t;
  int b;

  v = 0;
  for (li = 0; li < nloci; li++)
    {
      for (ei = 0; ei < L[li].numlines; ei++)
        {
          t = imaGtreeTime (ci, li, ei);
          b = imaGtreePop (ci, li, ei);
          if (upt < t && t < dnt && (b == b1 || b == b2 || b == b3))
            {
              v = 1;
              li = nloci;
              break;
            }
        }
    }
  return v;
}

/* Both */
#ifndef NDEBUG
void
assertgenealogyloc (int ci, int li)
{
  li = 0; /* To avoid compiler complaints. */
  /* FIXME: why there is no use of li? */
  if (modeloptions[NOMIGRATION] == 1)
    {
      /* for not individual? 
       * if (assignmentoptions[POPULATIONASSIGNMENTINDIVIDUAL] == 0) 
          assertgenealogyloclabel (ci, li);
          assertgenealogylocdiploid (ci, li);
       */
      if (assignmentoptions[POPULATIONASSIGNMENTLOCAL] == 0)
        {
          assertgenealogyindividual (ci);
        }
    }
  else
    {
      /* if (assignmentoptions[POPULATIONASSIGNMENTINDIVIDUAL] == 0)
          assertgenealogyloclabel (ci, li);
          assertgenealogylocdiploid (ci, li);
       */ 
      if (assignmentoptions[POPULATIONASSIGNMENTLOCAL] == 0)
        {
          assertgenealogyindividual (ci);
        }
    }
  return;
}

void
assertgenealogy (int ci)
{
  int li;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;

  assertgenealogyultrametric (ci);
  assertgenealogylabel (ci);
  assertgenealogybranch (ci);

  for (li = 0; li < nloci; li++)
    {
      assertgenealogylocdiploid (ci, li);
    }

  if (assignmentoptions[POPULATIONASSIGNMENTLOCAL] == 0)
    {
      assertgenealogyindividual (ci);
    }
  assertgenealogyasn (ci);

  for (li = 0; li < nloci; li++)
    {
      G = &(C[ci]->G[li]);
      gtree = G->gtree;
      if (G->roottime > TIMEMAX / 10)
        {
          assert (0);
        }
    }

  return;
}

void
assertgenealogyasn (int ci)
{
  int i;
  int ii;
  int li;
  int ji;
  int ei;
  int pop1;
  int pop2;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;
  int ngenes[MAXPOPS];

  for (i = 0; i < npops; i++)
    {
      ngenes[i] = 0;
    }

  for (ii = 0; ii < snind; ii++)
    {
      ji = 0;
      li = saC.indlist[ii][ji].li;
      ei = saC.indlist[ii][ji].ei;
      G = &(C[ci]->G[li]);
      gtree = G->gtree;
      assert (gtree[ei].up[0] < 0 && gtree[ei].up[1] < 0);
      pop1 = gtree[ei].pop;
      for (ji = 1; ji < sngenes_ind[ii]; ji++)
        {
          li = saC.indlist[ii][ji].li;
          ei = saC.indlist[ii][ji].ei;
          G = &(C[ci]->G[li]);
          gtree = G->gtree;
          assert (gtree[ei].up[0] < 0 && gtree[ei].up[1] < 0);
          pop2 = gtree[ei].pop;
          assert (pop1 == pop2);
        }
      ngenes[pop1]++;
    }

  for (i = 0; i < npops; i++)
    {
      assert (ngenes[i] == C[ci]->nasn[i]);
    }

  return;
}

void
assertgenealogyindividual (int ci)
{
  int ii;
  int li;
  int ji;
  int ei;
  int pop1;
  int pop2;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;

  for (ii = 0; ii < snind; ii++)
    {
      ji = 0;
      li = saC.indlist[ii][ji].li;
      ei = saC.indlist[ii][ji].ei;
      G = &(C[ci]->G[li]);
      gtree = G->gtree;
      assert (gtree[ei].up[0] < 0 && gtree[ei].up[1] < 0);
      pop1 = gtree[ei].pop;
      for (ji = 1; ji < sngenes_ind[ii]; ji++)
        {
          li = saC.indlist[ii][ji].li;
          ei = saC.indlist[ii][ji].ei;
          G = &(C[ci]->G[li]);
          gtree = G->gtree;
          assert (gtree[ei].up[0] < 0 && gtree[ei].up[1] < 0);
          pop2 = gtree[ei].pop;
          assert (pop1 == pop2);
        }
    }
  return;
}

void
assertgenealogylocbranch (int ci, int li)
{
  assert (!(ci < 0));  
  assert (!(li < 0));  
  return;
}

void 
assertgenealogybranch (int ci)
{
  int li;
  for (li = 0; li < nloci; li++)
    {
      assertgenealogylocbranch (ci, li);
    }
  return;
}

void
assertgenealogyedgelabel (int ci, int li, int ei)
{
  /* Check all labels of events along a branch! */
  int left;
  int right;
  int gparent;
  int nmigs;
  int mi;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;
  double ktime;
  double kprevtime;
  int kperiod;
  int kprevperiod;
  int pop;
  int prevpop;
  int ngenes;
  int ngnodes;
 
  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  left = gtree[ei].up[0];
  right = gtree[ei].up[1];
  gparent = gtree[ei].down;
  if (modeloptions[NOMIGRATION] == 1)
    {
      nmigs = 0;
    }
  else
    {
      nmigs = imaGtreeNmig (ci, li, ei);
    }
  assert (!(nmigs < 0));

  if (gparent < 0)
    {
      assert (nmigs == 0);
      kprevperiod = -1;
      prevpop = -1;
    }
  else
    {
      ngenes = L[li].numgenes;
      ngnodes = 2 * ngenes - 1;
      assert (gparent < ngnodes);
      assert (!(gtree[gparent].pop < 0));
      kprevtime = gtree[ei].time;
      kprevperiod = findperiodoftime (ci, kprevtime);
      prevpop = gtree[gparent].pop; 
    }
  for (mi = 0; mi < nmigs; mi++)
    {
      ktime = gtree[ei].mig[nmigs - 1 - mi].mt;
      assert (!(ktime < 0.0));
      kperiod = findperiodoftime (ci, ktime);
      assert (!(prevpop < 0));
      pop = gtree[ei].mig[nmigs - 1 - mi].mp;
      IMA_check_popn_up (kprevperiod, prevpop, kperiod, pop);
      prevpop = pop;
      if (mi == nmigs - 1)
        {
          pop = gtree[ei].pop;
        }
      else
        {
          pop = gtree[ei].mig[nmigs - 2 - mi].mp;
        }
      IMA_check_popn_move (prevpop, kperiod, pop);
      assert (!(pop < 0));
      kprevperiod = kperiod;
      prevpop = pop;
    }
  ktime = imaGtreeTime (ci, li, ei);
  kperiod = findperiodoftime (ci, ktime);
  pop = gtree[ei].pop;
  IMA_check_popn_up (kprevperiod, prevpop, kperiod, pop);

  if (left < 0 && right < 0)
    {
      /* No Code */
    }
  else
    {
      assertgenealogyedgelabel (ci, li, left);
      assertgenealogyedgelabel (ci, li, right);
    }
  return;
}

void
assertgenealogylocdiploid (int ci, int li)
{
  int ei;
  int ei2;
  int ngenes;
  struct genealogy *G;
  struct edge *gtree;

  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  ngenes = L[li].numgenes;
  for (ei = 0; ei < ngenes; ei++)
    {
      ei2 = L[li].pairs[ei]; 
      if (ei2 < 0)
        {
          continue;
        }
      else
        {
          assert (gtree[ei].pop == gtree[ei2].pop);
        }
    }
  return;
}

void
assertgenealogylabel (int ci)
{
  int li;

  for (li = 0; li < nloci; li++)
    {
      assertgenealogyloclabel (ci, li);
    }
  return;
}

void
assertgenealogyloclabel (int ci, int li)
{
  struct genealogy *G = NULL;

  G = &(C[ci]->G[li]);
  assertgenealogyedgelabel (ci, li, G->root);

  return;
}

/*
 * New functions
 *
 * assertgenealogy: 
 * We check whether a genealogy is valid. With no migration
 * assumption all internal nodes with the same period are labelled the same
 * population. Tip nodes are labelled the same population as that of their
 * parent nodes if parent nodes are in period 0.
 * Diploid genes must be labelled the same population.
 */

void
assertgenealogyultrametric (int ci)
{
  int li;

  for (li = 0; li < nloci; li++)
    {
      assertgenealogylocultrametric (ci, li);
    }
  return;
}

void
assertgenealogylocultrametric (int ci, int li)
{
  int ngenes;
  int ei;
  int gparent;
  double l;
  double len;
  double lentoroot;
  struct genealogy *G = NULL;
  struct edge *gtree = NULL;
  G = &(C[ci]->G[li]);
  gtree = G->gtree;
  ngenes = L[li].numgenes;


  for (ei = 0; ei < ngenes; ei++)
    {
      gparent = gtree[ei].down;
      len = imaGtreeBranchLength (ci, li, ei);
      assert (len > 0.00000000000000001);
      while (gparent != G->root)
        {
          l = imaGtreeBranchLength (ci, li, gparent);
          assert (l > 0.00000000000000001);
          len += l;
          gparent = gtree[gparent].down;
        }
      if (ei == 0)
        {
          lentoroot = len;
        }
      assert (fabs(len - lentoroot) < 0.0001);
    }
  return;
}

/* SANGCHUL: I have no other good names for these two functions. */
void
IMA_check_popn_up (int kprevperiod, int prevpop, int kperiod, int pop)
{
  if (!(kprevperiod < 0))
    {
      assert (!(kperiod > kprevperiod));
    }
  assert (!(kperiod < 0));
  assert (kperiod < npops);
  assert (kprevperiod < npops);

  if (kprevperiod < 0) /* a root's population */
    {
      assert (BitIsTrue (saC.Ps[kperiod], pop));
    }
  else /* a non-root's population */
    {
      assert (BitIsTrue (saC.popnlistB[prevpop][kperiod], pop));
    }
  return;
}

void
IMA_check_popn_move (int notpop, int kperiod, int pop)
{
  int ti;
 
  /* [[notpop]] must exist in period [[kperiod]]. */ 
  assert (!(kperiod < 0));
  assert (kperiod < npops);
  assert (BitIsTrue (saC.Ps[kperiod], notpop));
  assert (notpop != pop);

  /* pop does not belong to notpop's descendants. */
  for (ti = kperiod; !(ti < 0); ti--)
    {
      if (BitIsTrue (saC.popnlistB[notpop][ti], pop))
        {
          assert (0);
        }
    }
  
  return;
}
#endif /* NDEBUG */

