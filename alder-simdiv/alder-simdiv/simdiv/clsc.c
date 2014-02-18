/* simdiv  2009  Jody Hey, Sang Chul Choi and Yong Wang*/


#undef GLOBVARS
#include "simdiv.h"


/* LOCAL PROTOTYPES */
void addgnode (int whichpop);
boolean checknode (struct chrome *nodetocheck);
void addmut (struct BSET mutseg, int segspot);
void dorec (int whichpop);
void domig (int frompop, int topop, double time);
boolean checkclsc (void);
boolean doco (int whichpop, double time);
void clsc (int nowt, int locus);

/* FUNCTIONS */

void
addgnode (int whichpop)
/* assume nowgnode as been malloced and has data in the chromosome */
{
  nownum[whichpop]++;
  nowsum++;
  myassert (nowgnode->ci >= 0 || umodel == IS);
  if (linpops[whichpop].l == NULL)
    {
      linpops[whichpop].l = nowgnode;
      linpops[whichpop].r = nowgnode;
      linpops[whichpop].l->l = NULL;
      linpops[whichpop].r->r = NULL;
    }
  else
    {
      linpops[whichpop].r->r = nowgnode;
      nowgnode->l = linpops[whichpop].r;
      nowgnode->r = NULL;
      linpops[whichpop].r = nowgnode;
    }
}                               /* addgnode */

void
addmut (struct BSET mutseg, int segspot)
{
  struct mutation *newmut;
  int j = 0;
  boolean checkfull = _true;
  boolean checknotempty = _false;

  while (j < SETUNIT)
    {
      checkfull = checkfull && mutseg.p[j] == fullsampa.p[j];
      checknotempty = checknotempty || mutseg.p[j] > 0;
      j++;
    }
  if (!checkfull && checknotempty)
    {
      newmut = malloc (sizeof (struct mutation));
      newmut->d = mutseg;
      newmut->from = -1;
      newmut->to = -1;
      newmut->spot = segspot;   /* spot does not matter for non-recombining simulations */
      if (umodel == HKY)
        usedmut[segspot]++;
      else
        usedmut[segspot] = 1;
      if (firstmut == NULL)
        {
          firstmut = newmut;
          newmut->next = NULL;
        }
      else
        {
          newmut->next = firstmut;
          firstmut = newmut;
        }
      numuts++;
    }
  else
    {
      if (recrate == 0)
        printf ("error\n");
    }
}


/* old code used for debugging
boolean checknode(struct chrome *nodetocheck)
    {
    int i=0,j = 0;
    boolean nodeok = _false;
    while ( !nodeok && (i < seqlength ) )
        {
		j  = 0;
		while (!nodeok && j < SETUNIT)
			{
			if ( nodetocheck->b[i].p[j] > 0 ) nodeok = _true;
			j++;
			}
        i++;
        }
    return(nodeok);
    } */


void
dorec (int whichpop)
{
  int i, j;
  int pick, count, recspot, *rnum;
  unsigned short n1 = 0, n2 = 0;
  struct chrome tempc1, tempc2;

  rnum = &nownum[whichpop];
  nowgnode = linpops[whichpop].l;
  pick = randint (*rnum);       // pick a lineage in the population
  count = 1;
  while (count < pick)
    {
      nowgnode = nowgnode->r;
      count++;
    }
  myassert (nowgnode != NULL);
  recspot = randint (seqlength) - 1;    // pick a spot for recombination
  /* n1 and n2 are the number of units that have between 1 and fullsampa
     items.  If n1 (or n2) is 0 then the chromosome is ignored and is not split up */
  for (i = 0; i < seqlength; i++)
    {
      if (i <= recspot)
        {
          tempc1.b[i] = nowgnode->b[i];
          for (j = 0; j < SETUNIT; j++)
            n1 += (unsigned short) ((tempc1.b[i].p[j] >= 1)
                                    && (tempc1.b[i].p[j] < fullsampa.p[j]));
          tempc2.b[i] = emptysetb;
        }
      else
        {
          tempc2.b[i] = nowgnode->b[i];
          for (j = 0; j < SETUNIT; j++)
            n2 += (unsigned short) ((tempc2.b[i].p[j] >= 1)
                                    && (tempc2.b[i].p[j] < fullsampa.p[j]));
          tempc1.b[i] = emptysetb;
        }
    }
  if (n1 && n2)
    {
      removegnode (whichpop);   // remove the lineage
      nowgnode = malloc (structchromesize);
      memcpy (&(nowgnode->b[0]), &(tempc1.b[0]), chromesize);   // add two new lineages
      addgnode (whichpop);
      nowgnode = malloc (structchromesize);
      memcpy (&(nowgnode->b[0]), &(tempc2.b[0]), chromesize);
      addgnode (whichpop);
      numrec++;
    }
//      myassert(count_free + nowsum +numuts + count_others== count_malloc); 
}                               /* dorec */

void
domig (int frompop, int topop, double time)
{
  struct chrome tempc, *l, *la;
  int pick, count, j;

  l = linpops[frompop].l;
  la = linpops[topop].l;
  nowgnode = l;
  pick = randint (nownum[frompop]);     // pick a lineage in the source population, 1,2,...,n
  count = 1;
  while (count < pick)
    {
      nowgnode = nowgnode->r;
      myassert (nowgnode != la);
      count++;
    }
  j = 0;
  if (umodel != IS)
    {
      while (tree[nowgnode->ci].mig[j] > -0.5)  // find the last migration
        {
          j++;
          if (j >= MIGMAX)
            {
              printf
                ("error - too much migration : frompop %d topop %d ci %d j %d\n",
                 frompop, topop, nowgnode->ci, j);
              myassert (0);
              exit (-3);
            }
        }
      tree[nowgnode->ci].mig[j] = time; // add the migration
      tree[nowgnode->ci].mig[j + 1] = -1;
      tree[nowgnode->ci].mpop = topop;
    }
  memcpy (&(tempc.b[0]), &(nowgnode->b[0]), chromesize);        // adjust the gnode list
  tempc.ci = nowgnode->ci;
  removegnode (frompop);        // remove the lineage from the source population                        
  nowgnode = malloc (structchromesize);
  memcpy (&(nowgnode->b[0]), &(tempc.b[0]), chromesize);
  nowgnode->ci = tempc.ci;
  addgnode (topop);             // add the lineage to the recipient population
  //  myassert(count_free + nowsum + numuts +count_others== count_malloc);

  nummig++;
}                               /* domig */

boolean
checkclsc (void)                // check whether coalescent if finished for all sites
{
  int i, j, jj;
  boolean fullcheck;
  i = 0;
  while (i < clistnum)
    {
      fullcheck = _true;
      jj = 0;
      while (fullcheck && jj < SETUNIT)
        {
          fullcheck = nowgnode->b[clscchecklist[i]].p[jj] == fullsampa.p[jj];
          jj++;
        }
      if (fullcheck)
        {
          for (j = i; j < clistnum; j++)
            clscchecklist[j] = clscchecklist[j + 1];
          clistnum--;
        }
      else
        i++;
    }
  if (clistnum == 0)
    return (_true);
  else
    return (_false);
}

boolean
doco (int whichpop, double time)
{
  struct chrome tempc;
  int pick, i, j, count;
  boolean is_clsc;

  myassert (nownum[whichpop] > 1);
  nowgnode = linpops[whichpop].l;
  pick = randint (nownum[whichpop]);    // pick the first lineage to coalesce
  count = 1;
  while (count < pick)
    {
      nowgnode = nowgnode->r;
      count++;
    }
  if (umodel != IS)
    {
      //  myassert(tree[nowgnode->ci].pop == whichpop);
      tree[nowgnode->ci].down = nextgnodenum;   // creat one branch in the tree
      tree[nowgnode->ci].time = time;
      tree[nextgnodenum].up[0] = nowgnode->ci;
      tree[nextgnodenum].pop = tree[nextgnodenum].mpop =
        tree[nowgnode->ci].mpop;
      if (tree[nowgnode->ci].up[0] == -1)
        tree[nowgnode->ci].timei = time;        // timei is length of the branch
      else
        tree[nowgnode->ci].timei =
          tree[nowgnode->ci].time - tree[tree[nowgnode->ci].up[0]].time;
    }
  memcpy (&(tempc.b[0]), &(nowgnode->b[0]), chromesize);
  removegnode (whichpop);       // remove the first lineage
  nowgnode = linpops[whichpop].l;
  pick = randint (nownum[whichpop]);    // pick the second lineage to coalesce
  count = 1;
  while (count < pick)
    {
      nowgnode = nowgnode->r;
      count++;
    }
  if (umodel != IS)
    {

      tree[nowgnode->ci].down = nextgnodenum;
      tree[nowgnode->ci].time = time;
      if (tree[nowgnode->ci].up[0] == -1)
        tree[nowgnode->ci].timei = time;
      else
        tree[nowgnode->ci].timei =
          tree[nowgnode->ci].time - tree[tree[nowgnode->ci].up[0]].time;
      tree[nextgnodenum].ci = nextgnodenum;
      tree[nextgnodenum].up[1] = nowgnode->ci;
//       myassert(tree[nowgnode->ci].mpop == tree[nextgnodenum].pop);
      tree[nextgnodenum].mig[0] = -1;
      if (umodel == SW)
        for (i = 0; i < seqlength; i++)
          tree[nextgnodenum].A[i] = -1;
    }
  for (i = 0; i < seqlength; i++)       // calculate the seg code of each site for the new lineage
    for (j = 0; j < SETUNIT; j++)
      {
        tempc.b[i].p[j] |= nowgnode->b[i].p[j];
        myassert (tempc.b[i].p[j] <= fullsampa.p[j]);
      }
  removegnode (whichpop);       // remove the second lineage
  nowgnode = malloc (structchromesize);
  memcpy (&(nowgnode->b[0]), &(tempc.b[0]), chromesize);        // create the new lineage
  if (umodel != IS)
    {
      nowgnode->ci = nextgnodenum;
      if (nextgnodenum == 2 * numsum - 2)       // last lineage???? not true when there are recombinations
        {
          tree[nextgnodenum].down = -1;
          tree[nextgnodenum].time = TIMEMAX;
          tree[nextgnodenum].timei = 0;
          rootgnode = nextgnodenum;
          if (umodel == SW)
            for (i = 0; i < seqlength; i++)
              tree[rootgnode].A[i] = SWSTART;
          if (umodel == JOINT)
            tree[rootgnode].A[0] = SWSTART;
          if (umodel == INFINITEALLELES)
            {
              lastIAmutation = IASTART;
              tree[rootgnode].A[0] = lastIAmutation;
            }
        }
      nextgnodenum++;
    }
  addgnode (whichpop);          // add the new lineage to the population
  is_clsc = checkclsc ();
//    myassert(count_free + nowsum +numuts + count_others== count_malloc); 
  return (is_clsc);
}

/********************/
/* GLOBAL FUNCTIONS */
/********************/

void
removegnode (int whichpop)
{
  /* free the space where nowgnode is pointing */
  /* nowgnode must be pointing to a gnode in whichpop */

  struct chrome *tnode;
#ifdef _DEBUG
  struct chrome *checktnode;
#endif

  nownum[whichpop]--;
  nowsum--;
  if ((nowgnode == linpops[whichpop].l) && (nowgnode == linpops[whichpop].r))
    {
      linpops[whichpop].l = NULL;
      linpops[whichpop].r = NULL;
      free (nowgnode);
    }
  else
    {
      if (nowgnode == linpops[whichpop].l)
        {
          tnode = linpops[whichpop].l->r;
          linpops[whichpop].l = tnode;
          linpops[whichpop].l->l = NULL;
          free (nowgnode);
        }
      else
        {
          if (nowgnode == linpops[whichpop].r)
            {
              tnode = linpops[whichpop].r->l;
              linpops[whichpop].r = tnode;
              linpops[whichpop].r->r = NULL;
              free (nowgnode);
            }
          else
            {
#ifdef _DEBUG
              checktnode = nowgnode->l;
              while (checktnode != linpops[whichpop].l && checktnode != NULL)
                checktnode = checktnode->l;
              myassert (checktnode == linpops[whichpop].l);
              checktnode = nowgnode->r;
              while (checktnode != linpops[whichpop].r && checktnode != NULL)
                checktnode = checktnode->r;
              myassert (checktnode == linpops[whichpop].r);
#endif
              tnode = nowgnode->l;
              tnode->r = nowgnode->r;
              tnode = nowgnode->r;
              tnode->l = nowgnode->l;
              free (nowgnode);
            }
        }
    }
}                               /*removegnode */

void
loopinit (int locus)
{
  int i, ii, j, k, jj;
  for (i = 0; i < numpops; i++)
    {
      nownum[i] = 0;
      linpops[i].l = NULL;
      linpops[i].r = NULL;
    }
  firstmut = NULL;
  nowmut = NULL;

  /* create sample chromosomes and add to heap from left to right */
  fullsampa = emptysetb;
  nextgnodenum = 0;
  if (loadlocusvaluefile)
    numsum = 0;
  for (i = 0, j = 0 /* 1 */ ; i < numpops; i++)
    {
      if (loadlocusvaluefile)
        {
          num[i] = locussamplesizes[locus][i];
          numsum += num[i];
        }
      fullsamp[i] = emptysetb;
      for (ii = 1; ii <= num[i]; ii++, j++)
        {
          nowgnode = malloc (structchromesize);
          for (k = 0; k <= seqlength - 1; k++)
            {
              nowgnode->b[k] = sampstandard[j];
            }
          nowgnode->ci = nextgnodenum;
          nextgnodenum++;
          addgnode (i);
          for (jj = 0; jj < SETUNIT; jj++)
            fullsamp[i].p[jj] |= sampstandard[j].p[jj];
        }
      for (jj = 0; jj < SETUNIT; jj++)
        fullsampa.p[jj] |= fullsamp[i].p[jj];
    }
  if (numsum <= 0)
    {
      printf ("total sample size less than or equal to 0 \n");
      exit (-1);
    }
  if (numsum > MAXSAMPSIZE)
    {
      printf ("total sample size exceeds %d \n", MAXSAMPSIZE);
      exit (-1);
    }
  for (i = 0; i < seqlength; i++)
    {
      clscchecklist[i] = i;
      usedmut[i] = 0;
    }
  clistnum = seqlength;
  nowtime = 0.0;
  numuts = 0;
  numrec = 0;
  numutssw = 0;
  numutsIA = 0;
  nowtime = 0;
  printf ("Locus %d ", locus);
  if (umodel != IS)
    {
      tree = malloc ((2 * (numsum) - 1) * sizeof (struct edge));        // tree records the coalescent tree
      count_others++;
      for (i = 0, j = 0; i <= numpops; i++)
        {
          for (ii = 0; ii < num[i]; ii++, j++)
            {
              tree[j].ci = j;
              tree[j].down = -1;
              tree[j].up[0] = -1;
              tree[j].up[1] = -1;
              tree[j].time = -1;
              tree[j].pop = i;
              tree[j].mpop = i;
              tree[j].mig[0] = -1;
            }
        }
    }
}                               /* loopinit */

void
SWmutations (int node, int locus, int locuspart)
{
  int i, d, newA;
  double t, u;
  int lnode, rnode;

  if (loadlocusvaluefile)
    u = locusmutationrate[locus][locuspart];
  else
    u = stepwiseu;
  lnode = tree[node].up[0];
  rnode = tree[node].up[1];
  if (lnode != -1)
    {
      t = tree[lnode].timei;
      d = pickpoisson (u * t);
      numutssw += d;
      newA = tree[node].A[locuspart];
      for (i = 0; i < d; i++)
        {
          if (ran1 (idum) < 0.5)
            newA = IMAX (SWSTART - SWLIMIT, newA - 1);
          else
            newA = IMIN (SWSTART + SWLIMIT, newA + 1);
        }
      tree[lnode].A[locuspart] = newA;
      SWmutations (lnode, locus, locuspart);
    }
  if (rnode != -1)
    {
      t = tree[rnode].timei;
      d = pickpoisson (u * t);
      numutssw += d;
      newA = tree[node].A[locuspart];
      for (i = 0; i < d; i++)
        {
          if (ran1 (idum) < 0.5)
            newA = IMAX (SWSTART - SWLIMIT, newA - 1);
          else
            newA = IMIN (SWSTART + SWLIMIT, newA + 1);
        }
      tree[rnode].A[locuspart] = newA;
      SWmutations (rnode, locus, locuspart);
    }
  return;
}

void
IAmutations (int node, int locus)
{
  int d;
  double t, u;
  int lnode, rnode;

  // not sure if this is correct for this mutation model
  if (loadlocusvaluefile)
    u = locusmutationrate[locus][0];
  else
    u = 1;

  lnode = tree[node].up[0];
  rnode = tree[node].up[1];
  if (lnode != -1)
    {
      t = tree[lnode].timei;
      d = pickpoisson (t * u);
      if (d > 0)
        {
          lastIAmutation++;
          tree[lnode].A[0] = lastIAmutation;
        }
      else
        tree[lnode].A[0] = tree[node].A[0];
      numutsIA += d;
      IAmutations (lnode, locus);
    }
  if (rnode != -1)
    {
      t = tree[rnode].timei;
      d = pickpoisson (t * u);
      if (d > 0)
        {
          lastIAmutation++;
          tree[rnode].A[0] = lastIAmutation;
        }
      else
        tree[rnode].A[0] = tree[node].A[0];
      numutsIA += d;
      IAmutations (rnode, locus);
    }
  return;
}

void
clsc (int nowt, int locus)
{
  int muts, pick, i, j, k, jj;
  double endtime, time_to_event;
  double _uni, erate;
  boolean done, notemptypick;
  struct BSET segpick;
  int e, numevents;
  struct event
  {
    int eventtype;              /* 0 - coalesce; 1 - recombine; 2- migrate */
    int pop1;
    int pop2;
    double rate;
    double cumrate;
  } eventlist[2 * MAXPOPS + MAXPOPS * MAXPOPS];
  int np, poplist[MAXPOPS];

  done = _false;
  np = infolist[nowt].numothers;        // number of exist populations

  for (i = 0, j = 0; i < 2 * MAXPOPS; i++)      // list of exist populations
    if (infolist[nowt].others[i] == 1)
      {
        poplist[j] = i;
        j++;
      }
  nowtime = infolist[nowt].timeofnode;  // start time of current interval
  if (nowt < numt - 1)
    endtime = infolist[nowt + 1].timeofnode;    // end time of current interval, for internal population
  else
    endtime = 1e10;             // for most ancestral population (infinite)
  while (!done)
    {
      /* set up rates */
      numevents = 0;
      erate = 0;
      eventlist[0].cumrate = 0;
      for (i = 0; i < np; i++)  // colescent rates
        {
          if (nownum[poplist[i]] > 1)
            {
              eventlist[numevents].eventtype = 0;
              eventlist[numevents].pop1 = poplist[i];
              eventlist[numevents].rate = nownum[poplist[i]] * (nownum[poplist[i]] - 1) / thetas[poplist[i]];   // coalescent rate for each population
              /*if (loadlocusvaluefile)
                 eventlist[numevents].rate /= locusinheritance[locus]; */
              eventlist[numevents].rate /= hs;
              erate += eventlist[numevents].rate;       // total rate
              if (numevents > 0)
                eventlist[numevents].cumrate = eventlist[numevents - 1].cumrate + eventlist[numevents].rate;    // cumulative rate for each type of event
              else
                eventlist[numevents].cumrate = eventlist[numevents].rate;
              numevents++;
            }

        }
      if (recrate > 0 && umodel == IS)  // can only do recombination in Infinite Sites model                                                                                                                                    // recombination rates // sdfsdf
        for (i = 0; i < np; i++)
          {
            eventlist[numevents].eventtype = 1;
            eventlist[numevents].pop1 = poplist[i];
            eventlist[numevents].rate = nownum[poplist[i]] * recrate;   // recombination rates for each population
            erate += eventlist[numevents].rate; // total rate
            if (numevents > 0)
              eventlist[numevents].cumrate = eventlist[numevents - 1].cumrate + eventlist[numevents].rate;      // cumulative rate for each type of event
            else
              eventlist[numevents].cumrate = eventlist[numevents].rate;
            numevents++;
          }
      for (i = 0; i < np; i++)  // migration rates
        for (j = 0; j < np; j++)
          if (j != i)
            {
              if (M[poplist[i]][poplist[j]] > 0)
                {
                  eventlist[numevents].eventtype = 2;
                  eventlist[numevents].pop1 = poplist[i];
                  eventlist[numevents].pop2 = poplist[j];
                  eventlist[numevents].rate = nownum[poplist[i]] * M[poplist[i]][poplist[j]];   // migration rates for i->j (backwards)
                  erate += eventlist[numevents].rate;   // total rate
                  if (numevents > 0)
                    {
                      eventlist[numevents].cumrate = eventlist[numevents - 1].cumrate + eventlist[numevents].rate;      // cumulative rate for each type of event
                    }
                  else
                    eventlist[numevents].cumrate = eventlist[numevents].rate;
                  numevents++;
                }
            }
      for (i = 0; i < numevents; i++)   // normalize the cumulative rates
        {
          eventlist[i].cumrate /= eventlist[numevents - 1].cumrate;
        }
      _uni = ran1 (idum);
      // time to next event
      if (erate > 0)
        {
          if (heylaboptions[BRANCHANDMUTATIONEXPECTIATONS])
            time_to_event = 1.0 / erate;
          else
            time_to_event = log (_uni) / (-erate);      // time to next event
          e = 0;
          _uni = ran1 (idum);
          while (_uni > eventlist[e].cumrate)
            e++;                // category of next event
          done = ((time_to_event + nowtime >= endtime) || (nowsum < 2));
        }
      else
        done = 1;
      if (done)
        {
          time_to_event = endtime - nowtime;
          nowtime = endtime;
        }
      else
        nowtime += time_to_event;
      if (loadlocusvaluefile)
        umodel = locusmutationmodel[locus];
      if (umodel != SW && umodel != INFINITEALLELES && nowsum > 1)      // get mutations for sequences
        for (i = 0; i < np; i++)
          {
            nowgnode = linpops[poplist[i]].l;
            // for each lineage exist at that inteval
            while (nowgnode != NULL)
              {
                //muts = pickpoisson(time_to_event*urela[locus]);                                                             // generate number of mutations
                if (loadlocusvaluefile)
                  {
                    if (locusmutationmodel[locus] == JOINT)
                      muts =
                        poidev (time_to_event * locusmutationrate[locus][1]);
                    else
                      muts =
                        poidev (time_to_event * locusmutationrate[locus][0]);
                  }
                else
                  muts = poidev (time_to_event * urela[locus]);

                for (k = 0; k <= muts - 1; k++) // generate position of mutations
                  {
                    if (umodel == HKY)
                      pick = randint (seqlength) - 1;
                    else
                      {
                        do
                          {
                            pick = randint (seqlength) - 1;
                          }
                        while (usedmut[pick] == 1);
                      }
                    segpick = nowgnode->b[pick];        //judge how many existant lineages affect by this mutation
                    notemptypick = _false;
                    jj = 0;
                    while (!notemptypick && jj < SETUNIT)
                      {
                        notemptypick = segpick.p[jj] > 0;
                        jj++;
                      }
                    if (notemptypick)   // create mutation
                      {
                        addmut (segpick, pick);
                      }
                    if (numuts >= seqlength - 1
                        && (umodel == IS || umodel == JOINT))
                      {
                        printf
                          ("too many mutations - set -z value (sequence length) and/or CHROMELENGTH higher \n");
                        FP "too many mutations -  set -z value (sequence length) and/or  CHROMELENGTH higher \n");
                        fclose (outfile);
                        myassert (0);
                        exit (0);
                      }
                  }
                myassert (nowgnode != NULL);
                nowgnode = nowgnode->r;
              }
          }
      if (!done)
        {
          if (eventlist[e].eventtype == 0)
            doco (eventlist[e].pop1, nowtime);
          if (eventlist[e].eventtype == 1)
            dorec (eventlist[e].pop1);
          if (eventlist[e].eventtype == 2)
            domig (eventlist[e].pop1, eventlist[e].pop2, nowtime);
        }
    }
}                               /* end clsc */


void pool (int nowt)
{
  int i, pop;
  struct chrome tempc;

  for (i = 0; i < infolist[nowt + 1].numup; i++)
    {
      pop = infolist[nowt + 1].up[i];
      while (nownum[pop])
        {
          nowgnode = linpops[pop].r;
          tempc.ci = nowgnode->ci;
          memcpy (&(tempc.b[0]), &(nowgnode->b[0]), chromesize);
          removegnode (pop);
          nowgnode = malloc (structchromesize);
          memcpy (&(nowgnode->b[0]), &(tempc.b[0]), chromesize);
          nowgnode->ci = tempc.ci;
          addgnode (infolist[nowt + 1].pnodenum);
        }
    }
}                               /* pool */
