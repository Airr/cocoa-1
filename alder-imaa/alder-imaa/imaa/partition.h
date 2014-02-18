/* IMa2  2007-2009 Jody Hey, Rasmus Nielsen and Sang Chul Choi */

#ifndef _PARTITION_H_
#define _PARTITION_H_
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


void IMA_inp2partitionview (char *fname);

struct im_x_struct {
  int i; /* the number of the observation */
  double logL; /* the log likelihood of the partition */
  int k; /* the number of source populations, k, represented in the sample */
  int *ks; /* the number of individuals assigned to first source population */
  int **is; /* the list of individuals assigned to this population */
};
typedef struct im_x_struct im_x;

struct im_inp_struct {
  int nind;    /* total number of individuals */
  int nmaxpop; /* maximum number of possible source populations */
  int nobs;    /* number of observations */
  int nperiod; /* period */
  double uprior; /* prior parameter u */
  double tprior; /* prior parameter theta */
  im_x *x;   /* observations */
};
typedef struct im_inp_struct im_inp;

__END_DECLS

#endif /* _PARTITION_H_ */

