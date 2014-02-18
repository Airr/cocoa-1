/* simdiv  2009  Jody Hey, Sang Chul Choi and Yong Wang*/

/* renamed simdiv, broken up into multiple files, added some stuff by J Hey  3/17/2008 */
/* based on SIMDIVA,  by J Hey and Yong Wang */

/* older notes: 
/* SIMDIVA  - by Jody Hey  - based on SIMDIV*/
/* coalescent simulations under isolation model w/ migration  and recombination 
  generates mulitlocus data sets in the format to be read by multilocus version of dpgma program 
11/30/06  - added  Infinite Alleles model 
 modified 8/2006  
this version can handle multiple populations, and requires and input file, containing tree and parameter info
as well as command line input

Can also handle recombination with the IS model

Can handle the IS, SW, JOINT and HKY mutation models

HKY model not thoroughly checked  - check simdivhu.c  as programmed by Yong Wang 

gnodes are nodes on the genealogy
pnodes are on the population tree

requires command line input
-o  output file name (no spaces) 
-i  input file name (no spaces) 
-x  comment for first line of output file 
-s random number seed 
-u  mutation model I - IS (default)  S - Stepwise  J - joint H - HKY 
-v  relative mutation rate for SW locus  - only relevant under JOINT model 
-fa frequency of A -  for HKY model,  default = 0.25
-fc frequency of C -  for HKY model,  default = 0.25
-fg frequency of G -  for HKY model,  default = 0.25
-ft frequency of T -  for HKY model,  default = 0.25
-c  the ratio of recombination rate to mutation rate, default = 0
-L  the number of loci 
-r transition/transversion ratio, default = 2

input file format
First 0 or more lines can begin w/ '#'  and are ignored
input line 1,  an array of integers giving the sample sizes, for each population in order from pop 0 to pop numpops
input line 2, a tree string that provides the structure of the tree and the time depths (in coalescent units) of internal nodes
    - note that times must be given for each branch, and they must be completely additive and clocklike
	- the down time for the base of the tree (number of last parentheses) does not matter
input line 3, the identical tree string, except it gives the thetas for the corresponding population after the colon 
input line 4 :  1st line of square matrix of migration rates. 
	the rate is the row pop to the column pop, in the coalescent
    below the diagonal the rate is the column pop to the row pop.
    should put zeros on the diagonal.
    
Migration rates among internal nodes are calculated as the average of that among descendant populations

The number of populations is determined from the number of sample sizes given on the first input line
example:
#title - example input file
10  10  10
((0:0.2,1:0.2):0.4,2:0.4):0
((0:0.20,1:0.10):15,2:30):10
0 0.1 0.4
0.2 0  1.0
0   0   0 
if only one population then the tree format should be
():0   for the times
():X   for the pop size,  where X is the pop size
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#ifdef  _MSC_VER
#include <process.h>
#endif /*  _MSC_VER */
#include <assert.h>

#ifdef  __MWERKS__
#include <console.h>
#include "sioux.h"
#endif

#ifdef  _MSC_VER
// JH added 12/22/08 ,  this is a leak detection library,  only invoked in MVC++ debug mode and if vld is installed
#include <vld.h>
#endif /*  _MSC_VER */

/* microsft visual studio compiler stuff */
/* disable deprecate warning  C4996 */
#pragma warning( disable : 4996)


#ifdef _DEBUG
#ifdef _MSC_VER
#define USE_MYASSERT
#define MYDEBUG
# endif
#endif /*  */

//#define USE_MYASSERT 
#ifdef USE_MYASSERT
#  define assert(a) myassert((a))
#else
#  define myassert(a) assert((a))
#endif /*  */

#undef EXTRADEBUG

#ifdef EXTRADEBUG
#define malloc(size) watch_clsc_malloc(size)
#define free watch_clsc_free
#endif


#undef HPDBG
//#define HPDBG

#ifdef HPDBG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "sets352.h"

/***************/
/*   CONSTANTS */
/***************/

#define MAXSEQS (long) SET_BITS * (long) SETUNIT
#define CHROMELENGTH 5000       //5000  /*10001 *//*51*/ /*251 */
#define MAXCHROMESIZE  (long) CHROMELENGTH * (long)SET_BITS * (long) SETUNIT
#define MAXLINKED 15            // maximum number of linked regions on a genealogy, used for JOINT and SW if there are multiple linked SW loci
#define FNSIZE 4096
#define MAXPOPS 10
#define MAXSAMPSIZE 350
#define SWSTART 150
#define SWLIMIT 140
#define MIGMAX    10000
#define TIMEMAX 10000.0
#define IASTART  0              // starting allele for root under infinite sites model
#define DEFAULTRECRATE  0       // default recombination rate
#define DEFAULTSTEPWISEU  1.0   // default stepwise mutation rate
#define DEFAULTLOCILENGTH CHROMELENGTH  // 1000  // default # of base pairs in a locus upon output
#define MAXLOCUSNAMELENGTH  20


static int imaxarg1, imaxarg2;
#define IMAX(a,b) (imaxarg1=(a),imaxarg2=(b),(imaxarg1) > (imaxarg2) ? (imaxarg1) : (imaxarg2))
static int iminarg1, iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ? (iminarg1) : (iminarg2))

/*********************/
/* STRUCTURES, ENUMs */
/********************/

/* types of events during coalescent */
typedef enum
{ _false, _true } boolean;

/* running options */

// BRANCHANDMUTATIONEXPECTIATONS  apply expected branch lengths and mutation counts 
// DIPLOIDGENENAMES - give pairs of gene copies the same name so that output can be run in IMa using options that assume diploid samples
// PRINTDATASUMMARY - prints some basic summaries of the data 

enum
{ NODATAONLYSUMMARY, KEEPZEROVARIATION, DIPLOIDGENENAMES, PRINTDATASUMMARY, ASSIGNMENT, ASSIGNMENTDIFFERNTTREE,
    RUNOPTIONNUMBER };

/* heylab options */
enum
{ FOURGAMETETEST, GAMMADISTRIBUTEDMUTATIONERATES, POISSONDISTSEQLENGTHS,
    BRANCHANDMUTATIONEXPECTIATONS, HEYLABOPTIONNUMBER };


struct chrome
{
  int ci;
  struct BSET b[CHROMELENGTH];
  struct chrome *l, *r;
};

struct mutation
{
  struct BSET d;
  int spot;
  int from;
  int to;
  struct mutation *next;
};

// contains the pointers to the gnodes in the linked list for each population at the current time
struct linpop
{
  struct chrome *l, *r;
};

struct pnode
{
  int pnodenum;                 /* the node number, 0 thru (numpops-1) are the populations, after that they are internal nodes */
  int numup;                    /* number of descendant nodes */
  int up[MAXPOPS];              /* the identities of the descendant nodes */
  int dn;                       /* the down node */
  double time;                  /* the time on the branch to the down node */
  double popsize;               /*population size parameter */
};

struct timeinfo
{
  int pnodenum;
  double timeofnode;
  int numup;
  int up[MAXPOPS];
  int numothers;
  int others[2 * MAXPOPS];
};

struct edge
{
  int ci;
  int up[2];
  int down;
  double time;
  double timei;
  double mig[MIGMAX + 1];
  int A[MAXLINKED];
  int pop;
  int mpop;
};

enum model
{ IS, INFINITEALLELES, SW, JOINT, HKY };

/***************/
/*    MACROS   */
/***************/
#define FP  fprintf(outfile,
#define FPL fprintf(locusoutfile,
#define absv(x)  (((x) < 0.0) ? -(x) : (x))
/*random integer between 1 and x  inclusive,  x should be an integer*/
#define randint(x)  (int)( (ran1(idum) * (x)) + 1)
static double sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)
#define INTEGERROUND(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

/**************************/
/*  FUNCTION PROTOTYPES   */
/**************************/

/*  util.c */
// miscellaneous
void shelltimeinfo (struct timeinfo *lptr, int length);
int cardinality (struct BSET x);
//debugging 
#ifdef USE_MYASSERT
void myassert (unsigned short isit);
#endif
void *watch_clsc_malloc (int size);
void watch_clsc_free (void *p);

//string utilities 
void strdelete (char *s, int pos, int len);
char *nextnonspace (char *textline);
char *nextwhite (char *c);
// random number functions
double expo (double c);
long double ldouble_exp (long double value);
int pickpoisson (double param);
double gasdev ();
double pickgamma (double beta);
double ran1 (long *idum);
double gammln (double xx);
int poidev (double xm);

/*  clsc.c  */
void removegnode (int whichpop);
void loopinit (int locus);
void clsc (int nowt, int locus);
void pool (int nowt);
void SWmutations (int node, int locus, int locuspart);
void IAmutations (int node, int locus);

/* init.c */
void start (int argc, char *argv[]);

/* output.c */
void printsimdata (int locus);
void fourgametetest (int locus);
//void calcstats(int locus);
void calcstats (void);


/******************************************/
/***** GLOBAL VARIABLES ************/
/******************************************/

/* if GLOBVARS is defined then gextern is ignored. This
causes the corresponding variable declarations to be definitions.
GLOBVARS is only defined in front.c.  If GLOBVARS is not defined,
as is the case at the beginning of the other files, then gextern
gets replaced by extern and the variable declarations are simply
declarations */

#ifdef GLOBVARS
#define gextern
#else
#define gextern extern
#endif

gextern double M[2 * MAXPOPS][2 * MAXPOPS];     // migration rates
gextern double hs;              // inheritance scalar
gextern int ropt[RUNOPTIONNUMBER];      // run option
gextern int heylaboptions[HEYLABOPTIONNUMBER];  // hey lab options
gextern double *urela;
gextern int *oglen;
gextern int *ogmut;
gextern double recrate;         // recombination rates
gextern int numt;
gextern double thetas[2 * MAXPOPS];     // thetas
gextern double stepwiseu;
gextern int numpops;
gextern FILE *outfile, *treeprintfile;
gextern FILE *outfile, *locusoutfile;
gextern char outfilename[FNSIZE];
gextern char populationtree[FNSIZE];
gextern int npopulation;
gextern char textline[401];
gextern int nloci;
gextern int nlociallocate;
gextern int seqlength;
gextern int printseqlength;
gextern double tog;
gextern char poptreemin[201];   // population tree
gextern double thetaest[MAXPOPS];       // estimators by waterson method(????) 
gextern double thetaestsw[MAXPOPS];
gextern double meanpi[MAXPOPS]; // pi
gextern double meantajd[MAXPOPS];
gextern int nlociseqpoly[MAXPOPS], nlocistrpoly[MAXPOPS];
gextern int nlocitajd[MAXPOPS];
gextern int nlociseqdiv[MAXPOPS][MAXPOPS], nlocistrdiv[MAXPOPS][MAXPOPS];
gextern double meandiv[MAXPOPS][MAXPOPS];       // division
gextern double meandivsw[MAXPOPS][MAXPOPS];
gextern long *idum, idumval;    /* used by ran1 */
gextern double splitrate;

/* pointer to current gnode, left and right gnodes for selected and neutral*/
gextern struct chrome *nowgnode, *left2, *right2, *left1, *right1;
/*pointer to current mutation and first mutation */
gextern struct mutation *nowmut, *firstmut, *othermut;
/* size of struct chrome, and size of just chromosome data part of structure */
gextern unsigned int structchromesize, chromesize;
/* size of integer */
gextern int intsize;
gextern int numpops, num[2 * MAXPOPS], nownum[2 * MAXPOPS], numsum, nowsum;
gextern boolean noinputfile;
/* array of segments representing each possible single sample item*/
gextern struct BSET sampstandard[MAXSEQS + 1];
/* segments representing full samples */
gextern struct BSET fullsamp[MAXPOPS], fullsampa;
/* base frequencies */
gextern double bf[4];
/* transition transversion ratio */
gextern double ttratio;
gextern double mutmatrix[4][4];
/* count of transition/transversion */
gextern int numutstrans, numutstranv;
/* the next gnode to get added to a gene tree - used for stepwise  model */
gextern int nextgnodenum;
gextern int rootgnode;
/* current time during coalcescence */
gextern double nowtime;
/* array of chromosome positions for checking for complete clsc */
gextern int clscchecklist[CHROMELENGTH], clistnum;
/* array of used mutation positions */
gextern int usedmut[CHROMELENGTH];
/* number of mutations */
gextern int numuts, numutssw, numutsIA;
gextern int numrec;
gextern double meannumuts, meannumutssw, meannumutsIA, meannumrec;
/* count of migrations */
gextern int nummig;
/* the allelic state of the last added inifinite alleles mutation */
gextern int lastIAmutation;
gextern int numloci_nomuts;     // number of loci without mutations 
gextern int count_malloc, count_free, count_others;
gextern struct edge *tree;
gextern struct linpop linpops[2 * MAXPOPS];
gextern struct pnode pnodes[MAXPOPS * 2 - 1];
gextern struct timeinfo infolist[MAXPOPS];
gextern char **locusnames;
gextern double **locusmutationrate;     // two rates allowed in case there is a locus with the JOINT model, in which case two rates needed
gextern double *locusinheritance;
gextern int *locusmutationmodel;
gextern int *locuslength;
gextern int **locussamplesizes;
gextern double *locusttratio;
gextern boolean loadlocusvaluefile;
gextern enum model umodel;
gextern int modelcount[HKY + 1];
gextern int anynonISmodel;

#ifdef HPDBG
gextern int tmpFlag;
#endif

extern struct BSET emptysetb;
