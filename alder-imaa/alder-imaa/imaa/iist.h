#ifndef _IIST_H_
#define _IIST_H_
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

int IMA_gdistanceRemoved (int n, int *Ai, int *Bi, char *Ci);
int IMA_gdistanceOrder (int n, int *Ai, int *Bi, int m, int *od);
int IMA_gdistance (int n, int *Ai, int *Bi);
int IMA_gdistanceSimple (int n, int *Ai, int *Bi);
int IMA_max_intarray (int n, int *A);
void IMA_io_readsumassignment (char *fname, int n, int *A);

int IMA_permLexSuccessor (int n, int *p);
int IMA_permLexRank (int n, int *p);
int IMA_permLexUnrank (int *p, int n, int r);
int IMA_permFprintf (FILE *fp, int n, int *p);
int IMA_factorial (int n);
int skip_a_line (FILE *fp);
int read_int (FILE *fp);
double read_double (FILE *fp);
int IMA_io_readp (char *fname, int ***m, int *mr, int *mc);
int bmlReadpp (char *fname, double ***m, int *mc, int nind, int npops);
int IMA_fprintf (FILE *fp, int n, int *p);

__END_DECLS

#endif /* _IIST_H_ */

