/* simdiv  2009  Jody Hey, Sang Chul Choi and Yong Wang*/
/* routines for bigsets - draws on stuff for 32 bit sets*/

#define SETUNIT 11
typedef unsigned long SET;
/*typedef */ struct BSET
{
  SET p[SETUNIT];
};
#define SET_BITS 32
#define emptyset   ((SET) 0)    /* produces an unsigned long of value 0 */
#define singleset(i)  (((SET) 1) << (i))        /* makes a set of the integer i */
#define add(set,i)  ((set) | singleset(i))      /* set with i added to it */
#define element(i,set)   (singleset((i))  & (set) )     /*true if i in the set */
#define intersect(set1,set2)   ((set1) & (set2))
#define union(set1,set2)   ((set1) | (set2))
#define setdiff(set1,set2)   ((set1) ^ (set2))
#define firstset(n) ( (SET) (1 << (n))-1))

#define BSET_BITS 352
#define singlesetb(bset1,i) \
   bset1 = emptysetb;\
  if ( (i) <= 31) (bset1.p[0]) = ((SET) 1 << (i));\
  else if ( (i) <= 63) (bset1.p[1]) = ((SET) 1 << ((i) - 32));\
  else if ( (i) <= 95) (bset1.p[2]) = ((SET) 1 << ((i) - 64)); \
  else if ( (i) <= 127) (bset1.p[3]) = ((SET) 1 << ((i) - 96));\
  else if ( (i) <= 159) (bset1.p[4]) = ((SET) 1 << ((i) - 128));\
  else if ( (i) <= 191) (bset1.p[5]) = ((SET) 1 << ((i) - 160));\
  else if ( (i) <= 223) (bset1.p[6]) = ((SET) 1 << ((i) - 192));\
  else if ( (i) <= 255) (bset1.p[7]) = ((SET) 1 << ((i) - 224));\
  else if ( (i) <= 287) (bset1.p[8]) = ((SET) 1 << ((i) - 256));\
  else if ( (i) <= 319) (bset1.p[9]) = ((SET) 1 << ((i) - 288));\
                     else (bset1.p[10]) = ((SET)1 << ((i) - 320))

#define unionb(bset1,bset2,bset3) \
     (bset3.p[0]) = (bset1.p[0]) | (bset2.p[0]);\
     (bset3.p[1]) = (bset1.p[1]) | (bset2.p[1]);\
     (bset3.p[2]) = (bset1.p[2]) | (bset2.p[2]);\
     (bset3.p[3]) = (bset1.p[3]) | (bset2.p[3]);\
     (bset3.p[4]) = (bset1.p[4]) | (bset2.p[4]);\
     (bset3.p[5]) = (bset1.p[5]) | (bset2.p[5]);\
     (bset3.p[6]) = (bset1.p[6]) | (bset2.p[6]);\
     (bset3.p[7]) = (bset1.p[7]) | (bset2.p[7]);\
     (bset3.p[8]) = (bset1.p[8]) | (bset2.p[8]);\
     (bset3.p[9]) = (bset1.p[9]) | (bset2.p[9]);\
     (bset3.p[10])= (bset1.p[10]) | (bset2.p[10])


#define intersectb(bset1,bset2,bset3) \
     (bset3.p[0]) = (bset1.p[0]) & (bset2.p[0]);\
     (bset3.p[1]) = (bset1.p[1]) & (bset2.p[1]);\
     (bset3.p[2]) = (bset1.p[2]) & (bset2.p[2]);\
     (bset3.p[3]) = (bset1.p[3]) & (bset2.p[3]);\
     (bset3.p[4]) = (bset1.p[4]) & (bset2.p[4]);\
     (bset3.p[5])  = (bset1.p[5]) & (bset2.p[5]);\
     (bset3.p[6])  = (bset1.p[6]) & (bset2.p[6]);\
     (bset3.p[7])  = (bset1.p[7]) & (bset2.p[7]);\
     (bset3.p[8])  = (bset1.p[8]) & (bset2.p[8]);\
     (bset3.p[9])  = (bset1.p[9]) & (bset2.p[9]);\
     (bset3.p[10]) = (bset1.p[10]) & (bset2.p[10])
     /* puts the intersection of bset1 and bset2 into bset3 */

/* return _true if any overlap,  _false otherwise */
#define  checkintersect(bset1,bset2)\
	((bset1.p[0] & bset2.p[0]) || \
	 (bset1.p[1] & bset2.p[1]) || \
	 (bset1.p[2] & bset2.p[2]) || \
	 (bset1.p[3] & bset2.p[3]) || \
	 (bset1.p[4] & bset2.p[4]) || \
	 (bset1.p[5] & bset2.p[5]) || \
	 (bset1.p[6] & bset2.p[6]) || \
	 (bset1.p[7] & bset2.p[7]) || \
	 (bset1.p[8] & bset2.p[8]) || \
	 (bset1.p[9] & bset2.p[9]) || \
	 (bset1.p[10] & bset2.p[10]))
/* return _true if bset1 is less than bset2 in at least one unit */

#define bsetless(bset1,bset2)\
	((bset1.p[0] < bset2.p[0]) || \
	 (bset1.p[1] < bset2.p[1]) || \
	 (bset1.p[2] < bset2.p[2]) || \
	 (bset1.p[3] < bset2.p[3]) || \
	 (bset1.p[4] < bset2.p[4]) || \
	 (bset1.p[5] < bset2.p[5]) || \
	 (bset1.p[6] < bset2.p[6]) || \
	 (bset1.p[7] < bset2.p[7]) || \
	 (bset1.p[8] < bset2.p[8]) || \
	 (bset1.p[9] < bset2.p[9]) || \
	 (bset1.p[10] < bset2.p[10]))

#define setdiffb(bset1,bset2,bset3) \
     (bset3.p[0]) = (bset1.p[0]) ^ (bset2.p[0]);\
     (bset3.p[1]) = (bset1.p[1]) ^ (bset2.p[1]);\
     (bset3.p[2]) = (bset1.p[2]) ^ (bset2.p[2]);\
     (bset3.p[3]) = (bset1.p[3]) ^ (bset2.p[3]);\
     (bset3.p[4]) = (bset1.p[4]) ^ (bset2.p[4]);\
     (bset3.p[5]) = (bset1.p[5]) ^ (bset2.p[5]);\
     (bset3.p[6]) = (bset1.p[6]) ^ (bset2.p[6]);\
     (bset3.p[7]) = (bset1.p[7]) ^ (bset2.p[7]);\
     (bset3.p[8]) = (bset1.p[8]) ^ (bset2.p[8]);\
     (bset3.p[9]) = (bset1.p[9]) ^ (bset2.p[9]);\
     (bset3.p[10])= (bset1.p[10])^ (bset2.p[10])

#define getlowb(bset,bset1) \
  if ( (bset.p[0]) & -(bset.p[0]) ) {\
      (bset1.p[0]) = (bset.p[0]) & -(bset.p[0]);\
      (bset1.p[1]) = (SET) 0;\
      (bset1.p[2]) = (SET) 0;\
      (bset1.p[3]) = (SET) 0;\
      (bset1.p[4]) = (SET) 0;\
      (bset1.p[5]) = (SET) 0;\
      (bset1.p[6]) = (SET) 0;\
      (bset1.p[7]) = (SET) 0;\
      (bset1.p[8]) = (SET) 0;\
      (bset1.p[9]) = (SET) 0;\
      (bset1.p[10])= (SET) 0;}\
   else if ( (bset.p[1]) & -(bset.p[1]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (bset.p[1]) & -(bset.p[1]);\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (SET) 0;\
    (bset1.p[5]) = (SET) 0;\
	(bset1.p[6]) = (SET) 0;\
	(bset1.p[7]) = (SET) 0;\
	(bset1.p[8]) = (SET) 0;\
	(bset1.p[9]) = (SET) 0;\
	(bset1.p[10]) = (SET) 0;}\
   else if ( (bset.p[2]) & -(bset.p[2]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (bset.p[2]) & -(bset.p[2]);\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (SET) 0;\
    (bset1.p[5]) = (SET) 0;    (bset1.p[6]) = (SET) 0;\
	(bset1.p[7]) = (SET) 0;\
	(bset1.p[8]) = (SET) 0;\
	(bset1.p[9]) = (SET) 0;\
	(bset1.p[10]) = (SET) 0;}\
  else if ( (bset.p[3]) & -(bset.p[3]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (bset.p[3]) & -(bset.p[3]);  \
    (bset1.p[4]) = (SET) 0; \
    (bset1.p[5]) = (SET) 0; \
	(bset1.p[6]) = (SET) 0; \
	(bset1.p[7]) = (SET) 0; \
	(bset1.p[8]) = (SET) 0; \
	(bset1.p[9]) = (SET) 0; \
	(bset1.p[10])= (SET) 0;} \
  else if ( (bset.p[4]) & -(bset.p[4]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (bset.p[4]) & -(bset.p[4]);\
    (bset1.p[5]) = (SET) 0;\
    (bset1.p[6]) = (SET) 0;\
    (bset1.p[7]) = (SET) 0;\
    (bset1.p[8]) = (SET) 0;\
    (bset1.p[9]) = (SET) 0;\
    (bset1.p[10])= (SET) 0;}\
  else if ( (bset.p[5]) & -(bset.p[5]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (SET) 0;\
    (bset1.p[5]) = (bset.p[5]) & -(bset.p[5]);\
    (bset1.p[6]) = (SET) 0;\
    (bset1.p[7]) = (SET) 0;\
    (bset1.p[8]) = (SET) 0;\
    (bset1.p[9]) = (SET) 0;\
    (bset1.p[10])= (SET) 0;}\
  else if ( (bset.p[6]) & -(bset.p[6]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (SET) 0;\
    (bset1.p[5]) = (SET) 0;\
    (bset1.p[6]) = (bset.p[6]) & -(bset.p[6]);\
    (bset1.p[7]) = (SET) 0;\
    (bset1.p[8]) = (SET) 0;\
    (bset1.p[9]) = (SET) 0;\
    (bset1.p[10])= (SET) 0;}\
  else if ( (bset.p[7]) & -(bset.p[7]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (SET) 0;\
    (bset1.p[5]) = (SET) 0;\
    (bset1.p[6]) = (SET) 0;\
    (bset1.p[7]) = (bset.p[7]) & -(bset.p[7]);\
    (bset1.p[8]) = (SET) 0;\
    (bset1.p[9]) = (SET) 0;\
    (bset1.p[10])= (SET) 0;}\
  else if ( (bset.p[8]) & -(bset.p[8]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (SET) 0;\
    (bset1.p[5]) = (SET) 0;\
    (bset1.p[6]) = (SET) 0;\
    (bset1.p[7]) = (SET) 0;\
    (bset1.p[8]) = (bset.p[8]) & -(bset.p[8]);\
    (bset1.p[9]) = (SET) 0;\
    (bset1.p[10])= (SET) 0;}\
  else if ( (bset.p[9]) & -(bset.p[9]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (SET) 0;\
    (bset1.p[5]) = (SET) 0;\
    (bset1.p[6]) = (SET) 0;\
    (bset1.p[7]) = (SET) 0;\
    (bset1.p[8]) = (SET) 0;\
    (bset1.p[9]) = (bset.p[9]) & -(bset.p[9]);\
    (bset1.p[10])= (SET) 0;}\
  else if ( (bset.p[10]) & -(bset.p[10]) ){\
    (bset1.p[0]) = (SET) 0;\
    (bset1.p[1]) = (SET) 0;\
    (bset1.p[2]) = (SET) 0;\
    (bset1.p[3]) = (SET) 0;\
    (bset1.p[4]) = (SET) 0;\
    (bset1.p[5]) = (SET) 0;\
    (bset1.p[6]) = (SET) 0;\
    (bset1.p[7]) = (SET) 0;\
    (bset1.p[8]) = (SET) 0;\
    (bset1.p[9]) = (SET) 0;\
    (bset1.p[10]) = (bset.p[10]) & -(bset.p[10]);}


/* gets the lowest value in bset and creates bset1 with it */
#define elementb(bset,i) \
   (((i) <= 31) && ((bset.p[0]) & ((SET) 1 << (i)))) || \
   (((i) <= 63) &&  ((i) >=32) && ((bset.p[1]) & ((SET) 1 << ((i) - 32))))||\
   (((i) <= 95) &&  ((i) >=64) && ((bset.p[2]) & ((SET) 1 << ((i) - 64))))||\
   (((i) <= 127) && ((i) >=96) && ((bset.p[3]) & ((SET) 1 << ((i) - 96))))||\
   (((i) <= 159) && ((i) >=128) && ((bset.p[4]) & ((SET) 1 << ((i) - 128))))||\
   (((i) <= 191) && ((i) >=160) && ((bset.p[5]) & ((SET) 1 << ((i) - 160))))||\
   (((i) <= 223) && ((i) >=192) && ((bset.p[6]) & ((SET) 1 << ((i) - 192))))||\
   (((i) <= 255) && ((i) >=224) && ((bset.p[7]) & ((SET) 1 << ((i) - 224))))||\
   (((i) <= 287) && ((i) >=256) && ((bset.p[8]) & ((SET) 1 << ((i) - 256))))||\
   (((i) <= 319) && ((i) >=288) && ((bset.p[9]) & ((SET) 1 << ((i) - 288))))||\
   (((i) >= 320) && ( (bset.p[10]) & ((SET)1 << ((i) - 320))))
     /*returns true if i in bset */

#define forallb(i,bset) \
	for ( (i) = 0; (i) < BSET_BITS; ++(i)) if( elementb( (bset),(i) ) )
  /* forall : permits an action to be applied to all members of a set
     for example
     int k;
     forall(k,z) printf("%d ",k);   */

#define emptyb(bset) ( (bset.p[0]) == emptyset) &&\
      ( (bset.p[1]) == emptyset) &&\
      ( (bset.p[2]) == emptyset) &&\
      ( (bset.p[3]) == emptyset) &&\
      ( (bset.p[4]) == emptyset) &&\
      ( (bset.p[5]) == emptyset) &&\
      ( (bset.p[6]) == emptyset) &&\
      ( (bset.p[7]) == emptyset) &&\
      ( (bset.p[8]) == emptyset) &&\
      ( (bset.p[9]) == emptyset) && ( (bset.p[10]) == emptyset)
  /* true if bset is empty */
#define notemptyb(bset) ( (bset.p[0]) != emptyset) ||\
       ( (bset.p[1]) != emptyset) ||\
       ( (bset.p[2]) != emptyset) ||\
       ( (bset.p[3]) != emptyset) ||\
       ( (bset.p[4]) != emptyset) ||\
       ( (bset.p[5]) != emptyset) ||\
       ( (bset.p[6]) != emptyset) ||\
       ( (bset.p[7]) != emptyset) ||\
       ( (bset.p[8]) != emptyset) ||\
      ( (bset.p[9]) != emptyset) || ( (bset.p[10]) != emptyset)

  /* true if bset is not empty */
/*
int cardinality(struct BSET x){
	int count = 0,j;
  for (j=0;j<=SETUNIT;j++){
	while (x.p[j] != emptyset) {
		x.p[j] ^= (x.p[j] & -x.p[j]); ++count;
		}
	return(count);
	}
  */
