/* IMa2  2007-2009 Jody Hey, Rasmus Nielsen and Sang Chul Choi */

#ifndef _STRUCTURAMA_H_
#define _STRUCTURAMA_H_
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

void IMA_structurama_summary (char *fname);
void IMA_io_readsumassignment (char *fname, int ncol, int *A);
void IMA_structurama_uncertainty (char *fname);
__END_DECLS

#endif /* _STRUCTURAMA_H_ */

