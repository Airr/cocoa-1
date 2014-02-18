/* simdiv  2009  Jody Hey, Sang Chul Choi and Yong Wang*/


#undef GLOBVARS
#include "simdiv.h"


/* LOCAL PROTOTYPES */
void fillinfolist (void);
//void parenth0(int current);
void parenth0 (void);
void parenth (int mode, int current);
void poptreeread (int mode);
void presim (void);
void readlocusfile (void);
void readinputfile (void);

char *t;
int nextnode, lastpopnum;
int ancestralpopnums[2 * MAXPOPS - 1];
FILE *infile, *locusvaluefile;
char infilename[FNSIZE];
char locusvaluefilename[FNSIZE];
double splittimes[MAXPOPS - 1];
char poptreestring[3][201];
/* LOCAL FUNCTIONS */

void
presim (void)
{
  /* split up real start from loop start */
  int i;

  for (i = 0 /*1 */ ; i <= MAXSEQS; i++)        // finger prints for each of the MAXSEQS sequence
    {
      singlesetb (sampstandard[i], i);
    }
  structchromesize = sizeof (struct chrome);    // size of one record of 'chrome'
  chromesize = seqlength * sizeof (struct BSET);        // size of one sequence
  intsize = sizeof (int);
  i = sizeof (struct edge);
  anynonISmodel = _false;
  if (loadlocusvaluefile)
    for (i = 0; i < nloci; i++)
      {
        if (locusmutationmodel[i] != IS)
          anynonISmodel = _true;
      }
}                               /*presim */


void
fillinfolist (void)
/* infolist[i] contains information on the internal nodes, ordered in time from recent to distant past 
the information in infolist[i] is needed by clsc for the time between infolist[i].timeofnode and infolist[i+1].timeofnode.
    int pnodenum;  the node number
    double timeofnode;  the time at which that node occured  - gotten from .dn of one of the descendants
    int numup;   number of descendants
    int up[MAXPOPS];  identities of the descendants
    int numothers;    # of others nodes existing at that time.
    int others[2*MAXPOPS];   others[j]  means that node j is existing at that time.
     */
{
  int i, j, k;
  i = numpops;
  infolist[0].pnodenum = -1;    // info of first inteval
  infolist[0].timeofnode = 0;
  infolist[0].numup = 0;
  j = 1;
  do
    {
      infolist[j].pnodenum = i; // id of the node create at the start of an inteval
      infolist[j].timeofnode = pnodes[pnodes[i].up[0]].time;    // start time of an inteval
      infolist[j].numup = pnodes[i].numup;
      for (k = 0; k < infolist[j].numup; k++)
        infolist[j].up[k] = pnodes[i].up[k];
      for (k = 0; k < 2 * MAXPOPS; k++)
        infolist[j].others[k] = 0;      // populations exist in the inteval
      i++;                      // NOTE!!! time inteval here is not sorted by time!!! instead by the id of the pop create at the start of that inteval
      j++;
    }
  while (i <= lastpopnum);
  numt = j;                     // number of time intevals
  shelltimeinfo (&infolist[0], numt);   // NOTE!!! here we sort the time intevals by time
  for (i = 0; i < numpops; i++)
    infolist[0].others[i] = 1;  // mark the exist populations at each inteval
  for (i = 1; i < numt; i++)
    {
      for (j = 0; j < 2 * MAXPOPS; j++)
        infolist[i].others[j] = infolist[i - 1].others[j];
      infolist[i].others[infolist[i].pnodenum] = 1;
    }
  for (i = 0; i < numt; i++)
    {
      for (j = 0; j <= i; j++)
        for (k = 0; k < infolist[j].numup; k++)
          {
            infolist[i].others[infolist[j].up[k]] = 0;
          }
    }
  for (i = 0; i < numt; i++)    // count the number of exist populations at an inteval
    {
      k = 0;
      for (j = 0; j < 2 * MAXPOPS; j++)
        k += infolist[i].others[j];
      infolist[i].numothers = k;
    }
}                               /* fillinfolist */

/* parenth0()
come in on first opening parenthesis
the ith opening parenthesis is associated with a number after its corresponding closing parenthesis

parenth0() will go through parentheses from left to right,
when it comes to a close parenthesis it records the ancestral node number for that pairing

ancestral node numbers range from numpops to 2*numpops-2 and proceed from lowest
to highest in order of what time they occured

the ancestral node number is recorded in the array ancestralpopnums[]

the position in the array is the count of which parenthesis pair has just closed, plus numpops

in other words if it is the 0'th parenthesis pair (i.e. the first one that opened, meaning it is the
outermost pair),  then the ancestral node number is recorded in ancestralpopnums[numpops]

If it is the ith pair that has closed, it is recorded in ancestralpopnums[numpops + i]

This seemed to be the only way to get the correct labeling of internal nodes. 

Then when the function parenth() is called,  the correct times and populatino sizes can be associated 
with these ancestral populations. 
*/
//void parenth0(int current)
void
parenth0 (void)
{
  int itemp;
  char *ne;
  int psetlist[MAXPOPS], nextlistspot, popennum;
  nextlistspot = 0;
  popennum = 0;
  ne = t;
  while (*ne != '\0')
    {
      if (*ne == '(')
        {
          psetlist[nextlistspot] = popennum;
          nextlistspot++;
          popennum++;
          ne++;
        }
      else
        {
          if (*ne == ')')
            {
              ne += 2;
              itemp = strtol (ne, &ne, 10);
              ancestralpopnums[numpops + psetlist[nextlistspot - 1]] = itemp;
              nextlistspot--;
            }
          else
            ne++;
        }
    }
}                               /* parenth0 */


void
parenth (int mode, int current)
/* recursive - reenters for every open parentheses */
/* mode 1  means that times are read in */
/* mode 2  means that thetas are read in */
{
  int itemp, skiptemp;
  double ttemp;
  char *ne;
  int apopnum;

  apopnum = ancestralpopnums[current];
  t++;
  /* next could be:
     - a number  (a simple unode) read it and get ':' and double number after it
     - an open parenthesis (a complex unode - call parenth)
     - a comma  skip it
     - a close parenthesis - close the pnode and get ':' and double number after it
   */
  do
    {
      if (isdigit (*t))         // at a sampled population
        {
          itemp = strtol (t, &ne, 10);  // read the id of population
          t = ne;
          t++;                  /* skip colon */
          ttemp = strtod (t, &ne);      // read the ancestral node #/time/size of population
          t = ne;
          if (mode == 1)
            {
              pnodes[itemp].time = ttemp;
              pnodes[apopnum].up[pnodes[apopnum].numup] = itemp;
              pnodes[apopnum].numup++;
              pnodes[itemp].dn = apopnum;
            }
          if (mode == 2)
            {
              pnodes[itemp].popsize = ttemp;
              thetas[itemp] = ttemp;
            }
        }
      if (*t == ',')
        {
          t++;
        }
      if (*t == '(')            // read a '(' the part quote by '()' is population with id 'current+1', which is a up branch of 'current'
        {
          if (nextnode == -1)
            nextnode = numpops + 1;
          else
            nextnode++;
          if (mode == 1)
            {
              pnodes[ancestralpopnums[nextnode]].dn = apopnum;
              pnodes[apopnum].up[pnodes[apopnum].numup] =
                ancestralpopnums[nextnode];
              pnodes[apopnum].numup++;
            }
          parenth (mode, nextnode);
          if (*t == '\0')
            return;
        }
      if (*t == '\0')
        return;
    }
  while (*t != ')');
  t += 2;                       // skip /* parentheses, and colon */
  skiptemp = strtol (t, &t, 10);        // read the ancestral pop number,  though don't use it
  t++;                          // skip the colon
  ttemp = strtod (t, &ne);
  t = ne;
  if (mode == 2)
    {
      pnodes[apopnum].popsize = ttemp;
      thetas[apopnum] = ttemp;
    }
  if (mode == 1)
    {
      if (pnodes[apopnum].dn != -1)
        {
          pnodes[apopnum].time = ttemp;
          myassert (pnodes[apopnum].time >
                    pnodes[pnodes[apopnum].up[0]].time);
        }
      else
        pnodes[apopnum].time = 1e10;
    }

}                               /* parenth */

/* mode 0 to get the basic tree and identify the population identity numbers for the ancestral populations */
/* mode 1 to get the population splitting times */
/* mode 2 to get the population sizes */
void
poptreeread (int mode)
{
  int i, j, tti;
  char *tt;
  int openpcheck, closepcheck;  // count number of open brackets and close brackets

  if (mode == 0)                // initialize the tree
    {
      for (i = 0; i < 2 * numpops - 1; i++)
        {
          if (i < numpops)
            pnodes[i].pnodenum = i;
          else
            pnodes[i].pnodenum = -1;
          pnodes[i].numup = 0;
          for (j = 0; j < MAXPOPS; j++)
            pnodes[i].up[j] = -1;
          pnodes[i].dn = -1;
        }

    }
  //check the tree
  tt = poptreemin;
  for (tti = 0, i = 0; (unsigned) i < strlen (t); i++)  // use tt/poptreemin to keep the string of tree barring ":number" 
    {
      if (t[i] == '(' || t[i] == ')')
        {
          tt[tti] = t[i];
          tti++;
        }
      else
        {
          if (t[i] == ':')
            {
              do
                {
                  i++;
                }
              while (isdigit (t[i]) || t[i] == '.');
              i--;
            }
          else
            {
              if (isdigit (t[i]) || t[i] == ',')
                {
                  tt[tti] = t[i];
                  tti++;
                }
            }
        }
    }
  tt[tti] = '\0';

  for (closepcheck = 0, openpcheck = 0, i = 0; (unsigned) i < strlen (t); i++)  // check if the number of open parenthese match the close parentheses
    {
      closepcheck += t[i] == ')';
      openpcheck += t[i] == '(';
      if ((t[i] == ' ') || (t[i] == '\t') || (t[i] == '\n') || (t[i] == '\r'))  // remove spaces
        {
          t[i] = '\0';
          strcat (t, &(t[i + 1]));
          i--;
        }
    }
  if (closepcheck != openpcheck)
    {
      printf
        (" tree error   # open parentheses: %d   # close parentheses: %d \n",
         openpcheck, closepcheck);
      myassert (0);
      exit (-4);
    }
  nextnode = -1;
  if (mode == 0)
//              parenth0(numpops);
    parenth0 ();
  else
    {
      if (numpops > 1)          // 2 or more than 2 populations 
        parenth (mode, numpops);
      else                      // 1 or less populations  ?? when does this get used?? does it work ?? 
        parenth (mode, 0);
    }
}                               /* end poptreeread */

#define MAXLOCUSFILELINELENGTH 200
/* lines have information in the same order as locus lines in the IM programs:
   1. name
   2. sequence of numpops sample sizes
   3. mutation model:  I - IS (default)  S - Stepwise  J - joint H - HKY   A - Infinite Alleles
   4. inheritance scalar for that locus
   5. then if model J,  there is a mutation rate for an STR locus, followed by a mutation rate for the sequence locus
   any other mutation model just has one mutation rate on the line. 
   there must be nloci lines   */

void
readlocusfile (void)
{
  char *cc, modelc, textline[MAXLOCUSFILELINELENGTH + 1];
  int i, j;
  int lociread = 0;

  nlociallocate = nloci;

  locusnames = malloc (nlociallocate * sizeof (char *));
  for (i = 0; i < nlociallocate; i++)
    locusnames[i] = malloc (MAXLOCUSNAMELENGTH * sizeof (char));
  locussamplesizes = malloc (nlociallocate * sizeof (int *));
  for (i = 0; i < nlociallocate; i++)
    locussamplesizes[i] = malloc (numpops * sizeof (int));
  locusmutationrate = malloc (nlociallocate * sizeof (double *));
  for (i = 0; i < nlociallocate; i++)
    locusmutationrate[i] = malloc (MAXLINKED * sizeof (double));
  locusinheritance = malloc (sizeof (double) * nlociallocate);
  locusmutationmodel = malloc (sizeof (int) * nlociallocate);
  locuslength = malloc (sizeof (int) * nlociallocate);
  locusttratio = malloc (sizeof (double) * nlociallocate);
  if ((locusvaluefile = fopen (locusvaluefilename, "r")) == NULL)
    {
      printf ("Error opening file of locus specific values\n");
      myassert (0);
      exit (0);
    }
  for (i = 0; i < nloci; i++)
    {
      if (fgets (textline, MAXLOCUSFILELINELENGTH, locusvaluefile) != NULL
          && strlen (textline) > 10)
        {
          lociread++;

          while ((textline[strlen (textline) - 1] == '\n')
                 || (textline[strlen (textline) - 1] == ' '))
            textline[strlen (textline) - 1] = '\0';
          cc = textline;
          sscanf (cc, "%s", locusnames[i]);
          cc = nextnonspace (cc);
          for (j = 0; j < numpops; j++)
            {
              sscanf (cc, "%d", &locussamplesizes[i][j]);
              cc = nextnonspace (cc);
            }
          sscanf (cc, "%d", &locuslength[i]);
          cc = nextnonspace (cc);
          sscanf (cc, "%c", &modelc);
          //enum model {IS,INFINITEALLELES,SW,JOINT,HKY};
          switch (modelc)
            {
            case 'I':
              locusmutationmodel[i] = IS;
              modelcount[IS]++;
              break;
            case 'A':
              locusmutationmodel[i] = INFINITEALLELES;
              modelcount[INFINITEALLELES]++;
              break;
            case 'S':
              locusmutationmodel[i] = SW;
              modelcount[SW]++;
              break;
            case 'J':
              locusmutationmodel[i] = JOINT;
              modelcount[JOINT]++;
              break;
            case 'H':
              locusmutationmodel[i] = HKY;
              modelcount[HKY]++;
              break;
            default:
              printf ("no mutation model in locus value file \n");
              myassert (0);
              exit (0);
            }
          cc = nextnonspace (cc);
          sscanf (cc, "%lf", &locusinheritance[i]);
          if (locusmutationmodel[i] != SW)
            {
              cc = nextnonspace (cc);
              sscanf (cc, "%lf", &locusmutationrate[i][0]);
            }
          if (locusmutationmodel[i] == JOINT)
            {
              cc = nextnonspace (cc);
              sscanf (cc, "%lf", &locusmutationrate[i][1]);
            }
          if (locusmutationmodel[i] == SW)
            {
              for (j = 0; j < locuslength[i]; j++)
                {
                  cc = nextnonspace (cc);
                  sscanf (cc, "%lf", &locusmutationrate[i][j]);
                }
            }

          if (locusmutationmodel[i] == HKY)
            {
              cc = nextnonspace (cc);
              sscanf (cc, "%lf", &locusttratio[i]);
            }
        }
      else
        {
          nloci = lociread;
          break;
        }
    }
}                               /* readlocusfile */

  // format of input file:
  // any lines at the top of the file beginning with '#' are ignored
  // 1st line: integer, the number of populations (npop)
  // 2nd line: population string,  identical format as IMa2
  // 3rd line: population string with sample pop //'s replaced by ':t' where t is
  //           the splittime for that population, ancestral population numbers 
  //           are replaced by 't', the splittime for that ancestor
  //           splittime for last ancestor should be zero
  // 4th line: population string with sample pop //'s replaced by ':q' where q is 
  //           the population size parameter, ancestral population numbers are 
  //           replaced by 'q', the population size parameter for that ancestor
  // npop x npop matrix: there are npop rows each with npop values, separated by 
  //           a space. The value in row i column j is the migration parameter 
  //           for migration from population i to population j (backwards in 
  //           time).  Be careful, many populations cannot have migration because 
  //           of the phylogeny. All diagonal elements must be zero. 
  // last line:  npop integers, the sample sizes of sampled populations in order
void
readinputfile (void)
{
  int i, j;
  char tline[201];
  char *chpt;
  if ((infile = fopen (infilename, "r")) == NULL)
    {
      printf ("Error opening text file for reading\n");
      myassert (0);
      exit (0);
    }
  while (fgets (tline, 200, infile) != NULL && tline[0] == '#');
  chpt = &(tline[0]);
  sscanf (chpt, "%d", &numpops);
  lastpopnum = 2 * numpops - 2;

  //mode 0 for tree with #'s for ancestral pops, mode 1 for tree with times, mode 2 for tree with pop sizes

  for (i = 0; i < 3; i++)
    {
      if (fgets (tline, 200, infile) != NULL)
        {
          strcpy (poptreestring[i], tline);
          t = &(tline[0]);
          poptreeread (i);
        }
      else
        {
          printf (" tree string missing \n");
          myassert (0);
          exit (0);
        }
    }
  if (numpops > 1)
    {
      if (splitrate > 0)        // simulate split times
        {
          for (j = numpops; j < 2 * numpops - 1; j++)
            {
              if (j - numpops > 0)      // exponential parameter is number of populations in period divided by rate
                splittimes[j - numpops] = splittimes[j - numpops - 1] + expo ((numpops - (j - numpops)) / splitrate);   // number of populations in period divided by rate
              else
                splittimes[j - numpops] =
                  expo ((numpops - (j - numpops)) / splitrate);
              for (i = 0; i < 2 * numpops - 1; i++)
                {
                  if (pnodes[i].dn == j)
                    pnodes[i].time = splittimes[j - numpops];
                }
            }
        }
      // read in migration rates
      for (i = 0; i <= lastpopnum; i++)
        {
          fgets (tline, 200, infile);
          chpt = &tline[0];
          for (j = 0; j <= lastpopnum; j++)
            while (M[i][j] = strtod (chpt, &chpt))
              j++;
        }
    }
  // get sample sizes unless a locusvaluefile is being used
  if (!loadlocusvaluefile)
    {
      fgets (tline, 200, infile);
      chpt = &(tline[0]);
      if (isdigit (tline[0]) == _false)
        {
          printf (" input file does not have sample sizes where expected \n");
          myassert (0);
          exit (0);
        }
      for (i = 0; i < numpops; i++)
        {
          sscanf (chpt, "%d", &num[i]);
          chpt = nextnonspace (chpt);
        }
    }
}                               //readinputfile

/********************/
/* GLOBAL FUNCTIONS */
/********************/

void
start (int argc, char *argv[])
{
  int i, j;
  char ch, chp;
  char pstr[256];
  long seed_for_ran1;
  char command_line[201];
  double beta;

  boolean Dp, Op, Xp, Sp, Ip, Lp, Ep, Fap, Fcp, Fgp, Ftp, Rp, Cp, Up, Vp, Bp,
    Hp, Q0p, Q1p, Q2p, M0p, M1p, Tp, N0p, N1p, Yp, Wp, Zp;
  boolean Ap, Gp;
  /* Number of Population and population tree are different. */
  Ap = _false;                  /* population model file */
  Gp = _false;                  /* number of populations */
  Op = _false;                  /* output file */
  Ip = _false;                  /* input file name */
  Xp = _false;                  /* comment */
  Sp = _false;                  /* random number seed */
  Lp = _false;                  /* # pops and # loci */
  Up = _false;                  /* mutation model */
  Vp = _false;                  /* relative rate for SW portion of joint */
  Cp = _false;                  /* recombinatino rate */
  Rp = _false;                  /* transition transversion ratio */
  Bp = _false;                  /* scale parameter for the gamma distribution */
  Hp = _false;                  /* hey lab options */
  Ep = _false;                  /*  splitting rate parameter */
  Yp = _false;                  /* outgroup splitting time */
  Zp = _false;                  /* mean of sequence length */
  Wp = _false;                  /* optional filename containing locus names, sample sizes, mutation model, inheritance scalars and mutation rates */
  Dp = _false;                  /* inheritance scalar */
  Q0p = Q1p = Q2p = M0p = M1p = Tp = N0p = N1p = _false;
  Fap = Fcp = Fgp = Ftp = _false;       /* frequencies for HKY model */


/*    Theta for pop0  4Nr  for pop 0
#populations  # loci
numpop integers,  giving the sample sizes, for each population in order from pop 0 to pop numpops

A tree string that provides the structure of the tree and the time depths of internal nodes
The identical tree string, except it gives the population sizes relative to pop 0 

A matrix of migration rates.

command line should have:
filenames
recombination rate
# loci
mutation model and rate
base frequencies if HKY
random number seed
comment

input file should have, inorder
lines of explanatory text, beginning with '#'
row of number of sample sizes - the number of values is the number of populations
newick format tree, with times
newick format tree, with population sizes (trees should match)
matrix of coalescent migration rates
  values in migration rate matrix are the rate from row pop to column pop, in the coalescent
    
*/
  printf ("SIMDIV,  Hey lab coalescent simulator, June 16, 2009\n");
  strcpy (command_line, "");
  if (argc == 1 || (argc == 2 && toupper (argv[1][1]) == 'H'))
    {
      printf
        ("Command line usage for simdiv - no spaces between flag and variable \n");
      printf
        (" Maximum # individuals per locus: %d.  Default sequence length: %d. Maximum # populations: %d \n",
         MAXSAMPSIZE, DEFAULTLOCILENGTH, MAXPOPS);
      printf (" If no input file is given, 2 population model is assumed \n");
/* hidden stuff for hey lab use 
        printf("-b  used with -h1, scale parameter of gamma distribution\n"); */
      printf ("-a  population tree\n");
      printf ("-g  number of populations\n");
      printf ("-c  the ratio of recombination rate to mutation rate, default = 0 \n");
      printf ("-d  inheritance scalar, default = 1.0\n");
      printf
        ("-e  splitting rate parameter - overrides all splitting times\n");
      printf ("-fa frequency of A,  default = 0.25\n");
      printf ("-fc frequency of C,  default = 0.25\n");
      printf ("-fg frequency of G,  default = 0.25\n");
      printf ("-ft frequency of T,  default = 0.25\n");
      printf
        ("-i  input filename (no spaces)- required for >2 populations. Contains: sample sizes,population tree, split times, thetas, migration matrix\n");
/* hidden stuff for hey lab use 
        printf("-h  hey lab options \n");
        printf("     0:do four-gamete test on each locus \n"); 
        printf("     1:gamma distributed mutation rates for different loci\n");
        printf("     2: poisson distributed sequence length\n"); 
        printf("     3: replace random exponential and poisson variables with expected values \n"); */
      printf ("-j  running options\n");
      printf ("     0:do not print simulated data, only summaries of variation \n");
      printf ("     1:print data for all loci, even if there is no variation (default is to repeat loci with zero variation)\n");
      printf ("     2:set gene naming for diploid samples (i.e. pairs of sequences get the same name\n");
      printf ("     3:print summary of variation at end of output file\n");
      printf ("     4:assignment data\n");
      printf ("     5:assignment data with different population tree (-a and -g must be given)\n");
      printf ("       Note -j45 not just -j5\n");
 
      printf ("-L  the number of loci \n");
      printf
        ("-m0 (2 pop model only) migration rate per mutation for population 0 to population 1 \n");
      printf
        ("-m1 (2 pop model only) migration rate per mutation for population 1 to population 0 \n");
      printf ("-n0 (2 pop model only) the sample size for population 0 \n");
      printf ("-n1 (2 pop model only) the sample size for population 1 \n");
      printf ("-o  output file name (no spaces) \n");
      printf ("-q0 (2 pop model only) theta for population 0\n");
      printf ("-q1 (2 pop model only) theta for population 1\n");
      printf ("-q2 (2 pop model only) theta for ancestral population\n");
      printf
        ("-r  transition/transversion ratio for HKY model, default = 2 \n");
      printf ("-s  random number seed \n");
      printf
        ("-t (2 pop model only) time of population splitting on scale of mutations \n");
      printf
        ("-u  mutation model I - IS (default)  S - Stepwise  J - joint H - HKY   A - Infinite Alleles\n");
      printf
        ("-v  relative mutation rate for SW locus  - only relevant under JOINT model \n");
      printf ("-x  comment for first line of output file \n");
/* hidden stuff for hey lab use 
        printf("-y  used with heylaboptions[GAMMADISTRIBUTEDMUTATIONERATES], outgroup splitting time\n"); */
      printf
        ("-w  optional filename containing relative locus mutation model, sample sizes, inheritancescalar, mutation rates\n");
      printf
        ("-z  sequence length (default for IS model: print only variable sites, default for HKY model: use default sequence length)\n");
      exit (0);
    }
  else
    {
      bf[0] = bf[1] = bf[2] = bf[3] = 0.25;
      for (j = 1, i = 1; i < argc; i++) // detect if there is a -I tag
        {
          strcpy (pstr, argv[i]);
          ch = pstr[0];
          if (ch == '-' || ch == '/' || ch == '\\')
            {
              ch = pstr[1];
            }
          if (toupper (ch) == 'I')
            break;
          else
            j++;
        }
      if (j == argc)            // no -I tag detected, two population model assumed, create the pop tree
        {
          noinputfile = _true;
          numpops = 2;
          pnodes[0].pnodenum = 0;
          pnodes[0].numup = 0;
          pnodes[0].dn = 2;
          pnodes[1].pnodenum = 1;
          pnodes[1].numup = 0;
          pnodes[1].dn = 2;
          pnodes[2].pnodenum = 2;
          pnodes[2].numup = 2;
          pnodes[2].up[0] = 0;
          pnodes[2].up[1] = 1;
          pnodes[0].dn = -1;
        }
      for (i = 1; i < argc; i++)        // read in command line parameters
        {
          strcpy (pstr, argv[i]);
          strcat (command_line, " ");
          strcat (command_line, pstr);
          ch = pstr[0];
          if (ch == '-' || ch == '/' || ch == '\\')
            {
              ch = pstr[1];
              strdelete (pstr, 1, 2);
            }
          else
            {
              ch = pstr[0];
              strdelete (pstr, 1, 1);
            }
          switch (toupper (ch))
            {
            case 'A':
              strcpy (populationtree, pstr);
              Ap = _true;
              break;
            case 'G':
              npopulation = atoi (&pstr[0]);
              Gp = _true;
              break;
            case 'X':
              sprintf (textline, "%s", pstr);
              Xp = _true;
              break;
            case 'O':
              strcpy (outfilename, pstr);
              Op = _true;
              break;
            case 'W':
              strcpy (locusvaluefilename, pstr);
              loadlocusvaluefile = _true;
              Wp = _true;
              break;
            case 'S':
              seed_for_ran1 = atoi (&pstr[0]);
              if (!seed_for_ran1)
                seed_for_ran1 = 1;
              Sp = _true;
              break;
            case 'I':
              strcpy (infilename, pstr);
              Ip = _true;
              noinputfile = _false;
              break;
            case 'F':
              ch = toupper (pstr[0]);
              strdelete (pstr, 1, 1);
              switch (ch)
                {
                case 'A':
                  bf[0] = atof (&pstr[0]);
                  Fap = 1;
                  break;
                case 'C':
                  bf[1] = atof (&pstr[0]);
                  Fcp = 1;
                  break;
                case 'G':
                  bf[2] = atof (&pstr[0]);
                  Fgp = 1;
                  break;
                case 'T':
                  bf[3] = atof (&pstr[0]);
                  Ftp = 1;
                  break;
                }
              break;
            case 'R':
              ttratio = atof (&pstr[0]);
              Rp = _true;
              break;
            case 'U':
              chp = toupper (pstr[0]);
              switch (chp)
                {
                case 'I':
                  umodel = IS;
                  break;
                case 'S':
                  umodel = SW;
                  break;
                case 'J':
                  umodel = JOINT;
                  break;
                case 'H':
                  umodel = HKY;
                  break;
                case 'A':
                  umodel = INFINITEALLELES;
                  break;
                default:
                  umodel = 0;
                }

              Up = _true;
              break;
            case 'V':
              stepwiseu = atof (&pstr[0]);
              if (stepwiseu > 0)
                Vp = _true;
              break;
            case 'C':
              recrate = (double) atof (&pstr[0]);
              if (recrate >= 0)
                Cp = _true;
              break;
            case 'L':
              nloci = atoi (&pstr[0]);
              Lp = _true;
              break;
            case 'T':
              pnodes[0].time = pnodes[1].time = atof (&pstr[0]);
              Tp = _true;
              break;
            case 'M':
              if (pstr[0] == '1')
                j = 1;
              else
                j = 0;
              strdelete (pstr, 1, 1);
              if (j == 1)
                {
                  M[0][1] = (double) atof (&pstr[0]);
                  M0p = _true;
                }
              else
                {
                  M[1][0] = (double) atof (&pstr[0]);
                  M1p = _true;
                }
              break;
            case 'J':
              j = strlen (pstr) - 1;
              while (j >= 0)
                {
                  ropt[atoi (&pstr[j])] = 1;
                  pstr[j] = '\0';
                  j--;
                }

              break;
            case 'D':
              hs = atof (&pstr[0]);
              if (hs > 0.0 && hs <= 1.0)
                Dp = _true;
              else
                {
                  printf
                    ("input inheritance scalar is out of range, value 1 is used instead\n");
                  hs = 1;
                }
            case 'B':
              beta = atof (&pstr[0]);
              if (beta >= 0.0)
                Bp = _true;
              break;
            case 'Q':
              ch = toupper (pstr[0]);
              strdelete (pstr, 1, 1);
              switch (ch)
                {
                case '1':
                  pnodes[0].popsize = thetas[0] = atof (&pstr[0]);
                  Q0p = 1;
                  break;
                case '2':
                  pnodes[1].popsize = thetas[1] = atof (&pstr[0]);
                  Q1p = 1;
                  break;
                case 'A':
                  pnodes[2].popsize = thetas[2] = atof (&pstr[0]);
                  Q2p = 1;
                  break;
                }
              break;
            case 'N':
              if (pstr[0] == '1')
                j = 1;
              else
                j = 0;
              strdelete (pstr, 1, 1);
              if (j == 1)
                {
                  num[0] = atoi (&pstr[0]);
                  N0p = _true;
                }
              else
                {
                  num[1] = atoi (&pstr[0]);
                  N1p = _true;
                }
              break;
            case 'E':
              splitrate = atof (&pstr[0]);
              Ep = _true;
              break;
            case 'Y':
              tog = atof (&pstr[0]);
              if (tog > 0)
                Yp = _true;
              break;
            case 'Z':
              Zp = _true;
              printseqlength = atoi (&pstr[0]);
              break;
            }
        }
    }
/*handlying variables designating the length of sequences and gene copies 
IS model
  1 dont care about length,  only print variable sites
  2 user specifies length  (< default),  print all sites

HKY model
  1 user specifies length  (< default),  print all sites

JOINT
  always 1 SW locus,  this gets the first mutation rate given in a loadfile. 
  if a length is given, it refers to the IS portion  
  if no length is given,  the IS portion is printed at only variable sites

SW
  1 if no length is given then there is only 1 locus
  2 user specifies length (<= 15),  this is the # of loci 

need 2 global variables to manage length
  seqlength - the length of the list of places where mutations can happen, used for simulating locations of mutations
  printseqlength - the user specified value
  if printseqlength is zero,  then printing depends on the mutation model:
      IS - seqlength is set to default max length,  but printseqlength ends up being the number of mutations, i.e. only print variable sites
      HKY - set seqlength and printseqlength to be the default max length
      JOINT - for IS portion seqlength is default max, but like for IS model, i.e. printseqlength is # mutations and only print variable sites 
      SW - default number of SW portions (i.e. seqlength is 1) 
  if printseqlength > 0,  then seqlength = printseqlength 
*/

  if (ropt[ASSIGNMENTDIFFERNTTREE] == 1)
    {
      if (ropt[ASSIGNMENT] == 0)
        {
          printf ("-j5 must go with -j4 together: run with -j45 not just -j5\n");
          myassert (0);
          exit (-1);
        }
      if (Ap == _false || Gp == _false)
        {
          printf ("If -j45, then we must be given -a and -g options\n");
          myassert (0);
          exit (-1);
        }
    }
  if (!Ep)
    splitrate = 0;              // do not use splitting rate to pick times 
  if (!Cp)
    recrate = DEFAULTRECRATE;
  if (!Vp)
    stepwiseu = DEFAULTSTEPWISEU;
  if (!Wp)
    loadlocusvaluefile = _false;
  if (!Zp)
    {
      printseqlength = 0;
      if (umodel == SW)
        seqlength = 1;
      else
        seqlength = DEFAULTLOCILENGTH;
    }
  else
    {
      seqlength = printseqlength;
    }
  if ((umodel != IS && umodel != JOINT) && recrate != 0)
    {
      printf ("recombination rate cannot be non-zero, when the mutation model is not infinite-sites \n");
      myassert (0);
      exit (-1);
    }
  if (!Op)
    {
      strcpy (outfilename, "outfile.txt");
    }
  if (!Dp)
    hs = 1;
  else
    {
      if (Wp)
        {
          printf
            ("inheritance scalar given on command line _and_ in loadvaluefile  \n");
          myassert (0);
          exit (-1);
        }
    }
  if (!Ip && !noinputfile)
    {
      printf ("NAME OF INPUT FILE : \n");
      gets (infilename);
    }
  if (!Rp && umodel == HKY)
    {
      ttratio = 2;
    }
  if (heylaboptions[GAMMADISTRIBUTEDMUTATIONERATES] && !Bp)
    {
      printf
        ("No scale parameter for gamma distribution entered on command line\n");
      myassert (0);
      exit (-1);
    }
  if (heylaboptions[GAMMADISTRIBUTEDMUTATIONERATES] && !Yp)
    {
      printf ("No valid out group splitting time provided\n");
      myassert (0);
      exit (-1);
    }
  if (!Sp)
    seed_for_ran1 = (int) time (NULL);
  idumval = -seed_for_ran1;
  idum = &idumval;
  ran1 (idum);

#ifdef EXTRADEBUG
  count_malloc = 0;
  count_free = 0;
  count_others = 0;
#endif
  urela = malloc (sizeof (double) * nloci);
  count_others++;
  oglen = malloc (sizeof (int) * nloci);
  count_others++;
  ogmut = malloc (sizeof (int) * nloci);
  count_others++;

  for (i = 0; i < nloci; i++)
    {
      if (heylaboptions[POISSONDISTSEQLENGTHS])
        {
          oglen[i] = poidev ((double) seqlength - 1.0) + 1;
        }
      else
        {
          oglen[i] = seqlength;
        }
      if (!heylaboptions[GAMMADISTRIBUTEDMUTATIONERATES])
        urela[i] = 1;
      else
        {
          urela[i] = pickgamma (beta) / beta;
          ogmut[i] = poidev (2.0 * urela[i] * tog * oglen[i]);
        }
    }

  if ((outfile = fopen (outfilename, "w")) == NULL)
    {
      printf ("Error opening text file for writing\n");
      myassert (0);
      exit (0);
    }
  if (Ip)                       // read in pop tree from input file
    readinputfile ();

  if (noinputfile && (!N0p || !N1p))
    {
      printf ("no sample sizes entered on command line \n");
      myassert (0);
      exit (0);
    }
  for (i = 0, numsum = 0; i < numpops; i++)
    numsum += num[i];
  printf (" total sample size %d \n", numsum);
  fillinfolist ();              // fill in the timeinfo varaibles which keeps the information about the time intevals
  if (loadlocusvaluefile)
    {
      readlocusfile ();
      if (heylaboptions[GAMMADISTRIBUTEDMUTATIONERATES] == _true)
        {
          printf
            ("cannot choose mutation rates from gamma distribution _and_ specify relative rates in loadfile \n");
          myassert (0);
          exit (-1);
        }
    }
  else
    {
      modelcount[umodel] = nloci;
    }
  // setup header of output file 
  FP "**SIMDIV output @copyright Jody Hey ** - coalescent simulations with multiple populations and gene flow   \n");   // outputing input info
  FP "#Command line string: %s\n", command_line);
  if (ropt[DIPLOIDGENENAMES])
    FP "#gene names set for appearance of diploid samples\n");
  FP "# #loci:%d seed:%d  #pops:%d", nloci, seed_for_ran1, numpops);
  if (Vp)
    FP "# stepwise rate: %.1f", stepwiseu);
  FP "# sample sizes : ");
  for (i = 0; i < numpops; i++)
    FP "%d ", num[i]);
  FP "\n");
  if (!noinputfile)
    {
      FP "#   tree with ancestral population numbers   :  %s",
        poptreestring[0]);
      FP "#   tree with time depths    : %s", poptreestring[1]);
      FP "#   relative population sizes: %s", poptreestring[2]);
    }
  FP "#   migration rate matrix (from row pop to column pop, in the coalescent) \n");
  for (i = 0; i <= lastpopnum; i++)
    {
      FP "# ");
      for (j = 0; j <= lastpopnum; j++)
        FP "\t%.3f", M[i][j]);
      FP "\n");
    }
  if (splitrate > 0)
    {
      FP "# Simulated population split times: Split rate parameter: %.3lf  Split Times:", splitrate);
      for (i = 0; i < numpops - 1; i++)
        FP "  %.4lf", splittimes[i]);
      FP "\n");
    }
  if (ropt[NODATAONLYSUMMARY])
    FP "\nPRINT ONLY SUMMARIES, NO DATA \n");

  if (Gp == _false)
    {
      FP "%d \n", numpops);
      for (i = 0; i < numpops; i++)
        FP "pop%d  ", i);
    }
  else
    {
      FP "# Note the different population tree from the simulation one\n");
      FP "%d \n", npopulation);
      for (i = 0; i < npopulation; i++)
        FP "pop%d  ", i);
    }
  FP "\n");

  if (Ap == _false)
    {
      if (!noinputfile)
        FP "%s", poptreestring[0]);
      else
        FP "(0,1):2\n");
    }
  else
    {
      FP "%s\n", populationtree);
    }
  FP "%d\n", nloci);
  numutstrans = 0;              // transition and transversion mutation count
  numutstranv = 0;
  meannumuts = 0;
  meannumrec = 0;
  presim ();
}                               /* start */
