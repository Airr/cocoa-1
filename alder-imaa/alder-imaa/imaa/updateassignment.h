/* IMa2 2007-2011 Jody Hey, Rasmus Nielsen and Sang Chul Choi */
#ifndef _UPDATEASSIGNMENT_H_
#define _UPDATEASSIGNMENT_H_
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif
__BEGIN_DECLS
#define SWAP(a,b) do {\
  char c[sizeof((a))]; \
  memcpy((void *)&(c), (void *)&(a), sizeof((c))); \
  memcpy((void *)&(a), (void *)&(b), sizeof((a))); \
  memcpy((void *)&(b), (void *)&(c), sizeof((b))); \
} while (0)
struct im_node_struct
{
  int li;               /* locus id               */
  int ei;               /* node and its branch id */
  double t;             /* time at the node       */
}; 

typedef struct im_node_struct im_node;

struct im_savededge_struct
{
  int savedpop;           /* old population the edge is in at its top */
  im_migstruct *savedmig; /* migration events                         */
  int cmm;                /* current size of mig array                */
};

typedef struct im_savededge_struct im_savededge;

struct im_savedlocus_struct
{
  /* copy members of locus structure */
  int nlinked;
  int model; 
  /* for saving population labels    */
  im_savededge *gtree;
  /* for saving some branches        */
  struct edge *saved;
  int savedi;
  int savedn;
  /* for saving locus members        */
  int savedroot;
  int savedmignum;
  double savedroottime;
  double savedlength;
  double savedtlength;
  struct genealogy_weights savedgweight;
  double savedpdg;
  double savedplg;
  double *savedpdg_a;

  /* rootmove global variable would be removed from update_gtree.c */
  int copyedge[3];
  int holddownA[MAXLINKED];
  int medgedrop;
  int mrootdrop;
  int rootmove;
  double lmedgedrop;
  double lmrootdrop;
  double holdsisdlikeA[MAXLINKED];
};

typedef struct im_savedlocus_struct im_savedlocus;

struct im_popntree_struct
{
  struct genealogy_weights savedallgweight;
  struct probcalc savedallpcalc;
  double savedt[MAXPERIODS];

  im_node **indlist;
  /* gsl_block_uint ***popnlistDELETE; FIXME: change it to int ** */

  /* Do we need popnlist, npopnlist, popnlistB, and Ps? */
  /* We do not seem to use these. popndown and popnmig are used. */
  int ***popnlist;
  int **npopnlist;

  UByteP **popnlistB;
  UByteP *Ps;     /* list of populations during a period  */
  int **popndown;

  int ***popnmig; /* per period, per popn, list of popn's */
};

typedef struct im_popntree_struct im_popntree;

struct im_ginfo_struct
{
  int *cc;        /* coalescent counts for populations    */
  double *fc;     /* coalescent rates  for populations    */
  int **cm;       /* migration counts between populations */
  double **fm;    /* migration rates between populations  */
  double *thetas; /* theta estimates for populations      */
  double **mms;   /* migration rates between populations  */
  double **ms;    /* migration rate away for populations  */
}; 

typedef struct im_ginfo_struct im_ginfo;

struct im_event_struct
{
  double t;  /* time                                    */
  char type; /* 'c' or 'm' or 's' or 'e'                */
  int p;     /* population label or p-th split time     */
  int pi;    /* only for migration events: starting pop */
  int pj;    /* only for migration events: ending pop   */
  int ei;    /* edge ID for coalescent event            */
}; 

typedef struct im_event_struct im_event;

struct im_bfupdate_struct
{
  im_event *e1;  /* events                      */
  int n1;        /* number of total events e1   */
  int nmax1;     /* capacity of the array e1    */
  im_event *e2;  /* events plus active          */
  int n2;
  int nmax2;
  im_event *e3;  /* events beyond a partial     */
  int n3;
  int nmax3;
  int *nz1;
  int **lz1;
  int *n;
  int **l;
  int *a;        /* array of gi_largestsample   */
  double *likes1;/* array of gi_largestsample   */
  int likei1;    /* chosen one                  */
  double **likes11; /* maxA - minA + 1          */
  int likei11;   /* chosen one                  */
  int *seqz1;    /* seq at the bottom of z1     */
  char canz31;   /* 1 if z3 and z1 can coalesce */
  char is_finiterootbranch;
  int A1;

  int sis1;
  im_edgemiginfo *m1;
  im_edgemiginfo *m3;
  int pop;
};

typedef struct im_bfupdate_struct im_bfupdate;
void recordassignment_header (char *fname, int ci, int bIsPerChain);
void imaAdmixturePrintHeader (char *fname, int ci, int bIsPerChain);
int IMA_ninds ();
int IMA_nindsunknown ();
void recordassignment (char *fname, int ci, int gen, int bIsPerChain);
void recordassignmentloci (char *fname, int ci);
void imaAdmixturePrint (char *fname, int ci);
int skip_a_line (FILE *fp);
int read_int (FILE *fp);
double read_double (FILE *fp);
int imaParserReadString (FILE *fp, char* s, int sn);

void imaBarTick ();
void imaBarReset ();
void imaAsnPrintNumGenesPopn (FILE *fp);
void imaAsnPrintNumGenesPopn_stdout ();
char* imaGtreePrint (int ci, int li);
char* imaGtreePrintall (int ci);
char* imaGtreePrintsite (int ci, int li, int si);
char* imaGtreePrintseq (int ci, int li);
void funcx();
void Phylogeny_setPopndown (int *** popndown, struct chain * t);
void Phylogeny_resetPopndown (int ** popndown, struct chain * t);
void Phylogeny_setPopnmig (int **** popnmig, struct chain * t);
char* imaPtreePrint (int ci);
void x1 (im_bfupdate *pbf);
int x2 (int old, int li);
int x2print (im_ginfo *gk);
/* void IMA_sbf_print (im_bfupdate *pbf); */
void  BitPrintF (UByteP A, int size);
void print_popnlist ();
double imaAsnValue (int ci, int li);
void imaAsnInitAssign (int ci);
void imaAsnInit (void);
void imaAsnFree (void);
void imaAsnSet ();
void imaTreeSet ();
#ifdef NDEBUG
# define assertgenealogy(A)
# define assertgenealogyloc(A,B)
#else
void assertgenealogyloc (int ci, int li);
void assertgenealogy (int ci);
#endif /* NDEBUG */
int imaPhylogenySplitTiAsn (const char *fname);
int IMA_fprintf (FILE *fp, int n, int *p);
int imaFileAsnOpen (const char *fnameasn, const char *fnameorg);
int imaFileAsnAppend (const char *fnameasn, int gi, int *m, int n);
int imaFileTiOpen (const char *fnameti);
int imaFileTiAppend (const char *fnameti, int gi);
int imaFileBatCreate (const char *fname);
int imaFileCopy (const char *fname1, const char *fname2);
void imaStructuramaLocalrun (char *fname);
int imaCmdFindmeanassignment (const char* infilename, const char* outfilename);
int imaAssignmentReadSizeInFile (const char* infilename, int* n, int* m);
int imaAssignmentReadInFile (const char* fn, int* s, const int n, const int m);
int imaCmdFindCoassignmentProbability (const char* fni, const char* fno);

void imaEdgemiginfoPrint (im_edgemiginfo *m);
int imaIndUnknown ();
int updateassignmentrelabel (int ci);
int updateassignmentbf (int ci);
int bflikelihood (int ci);
int IMA_genealogy_findIntSeq (int ci, int li);
void IMA_genealogy_join (im_edge *gtree, int up, int down, int downdown);
void IMA_genealogy_absorbdown (im_edge *gtree, int up, int down);
void IMA_genealogy_bisectsis (int ci, int li, 
                              int sis, int edge, int down, 
                              im_edgemiginfo *em, 
                              int *seqz);
double likelihoodJC (int ci, int li, double u);
int computeLk (double (*l_k)[4], double u, int li, im_edge *t, int k, int si);
double PijJC (int i, int j, double t);
double IMA_edge_length (im_edge *t, int ei);
int updateassignmentrelabellocus (int ci,int li);
int updateassignmentbflocus (int ci,int li);
int bflocuslikelihood (int ci,int li);
int updateassignmenttpt (int ci);
double priorDivTime (int ci, double r, double x);
int updateassignmentptree (int ci);
void recordptree_header (char *fname, int ci);
void recordptree (char *fname, int ci);
void recordptree_footer (char *fname, int ci);

void imaStructuramaCreateInFile (char *structrurama, int byAllele);
void imaStructuramaCreateBatFile (char *fname);
void IMA_convert_IM2Mrbayes (char *mrbayes);
void IMA_output_mrbayes_bat (char *fname);
void imaStructuramaUncertainty (char *fname, int nburnin);
void imaIMTreeUncertainty (char *fname, const char *ext, int nburnin);
void IMAgenealogyAlign (char *fname, int n);
void IMA_gsampinf_swap (int gi, int pi, int pj);
void imaGsampinfPermutedG (int gi, float *atree, int *od);

void imaGsampinfRelabel (int gi, int *od);
int imaGsampinfFindmig (int pi, int pj);
extern int gsampinflength;
int IMA_rgs_convert (int *a, int n);
void IMA_rgf_tickSaasn (int ci);
void IMA_rgf_saveSaasn ();
int IMA_fprintrgf (FILE *fp, int *f, int m);
int IMA_generatergf (int m, int n);
int** IMA_generalizedrgf (int m);
int IMA_freejrgf (int ***d, int m);
int IMA_fprintjrgf (FILE *fp, int **d, int m);
int IMA_rankrgf (int m, int *f);
int IMA_unrankrgf (int *f, int r, int m);
int IMA_stirlings2 (int m, int n);
int IMA_maxrgf (int *f, int m);

int** IMA_generalizedrgf2 (int m, int n);
int IMA_rankrgf2 (int m, int n, int *f);
int IMA_unrankrgf2 (int *f, int r, int m, int n);

void imaStructuramaSummary (char *fname, int nburnin);
void IMA_io_readsumassignment (char *fname, int ncol, int *A);
__END_DECLS
#endif /* _UPDATEASSIGNMENT_H_ */

