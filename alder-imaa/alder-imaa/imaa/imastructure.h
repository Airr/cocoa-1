/* IMa2 2007-2011 Jody Hey, Rasmus Nielsen and Sang Chul Choi */
#ifndef _IMASTRUCTURE_H_
#define _IMASTRUCTURE_H_
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

int Partdis (const int Ne, const int *A, const int *B);
void Meanpartdis (int *sampledz, int ne, int ns, int *meanpartition);
void Coassignprobs (int *sampledz, int ni, int ns, double *coaprob);

__END_DECLS
#endif /* _IMASTRUCTURE_H_ */

