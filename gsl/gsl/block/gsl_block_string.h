#ifndef alder_align_alder_block_string_h
#define alder_align_alder_block_string_h

#include <stdlib.h>
#include <gsl/gsl_errno.h>

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

typedef char* string;

struct gsl_block_string_struct
{
    size_t size;
    string *data;
};

typedef struct gsl_block_string_struct gsl_block_string;

gsl_block_string *gsl_block_string_alloc (const size_t n);
gsl_block_string *gsl_block_string_calloc (const size_t n);
void gsl_block_string_free (gsl_block_string * b);

int gsl_block_string_fread (FILE * stream, gsl_block_string * b);
int gsl_block_string_fwrite (FILE * stream, const gsl_block_string * b);
int gsl_block_string_fscanf (FILE * stream, gsl_block_string * b);
int gsl_block_string_fprintf (FILE * stream, const gsl_block_string * b, const char *format);

int gsl_block_string_raw_fread (FILE * stream, string * data, const size_t n, const size_t stride);
int gsl_block_string_raw_fwrite (FILE * stream, const string * data, const size_t n, const size_t stride);
int gsl_block_string_raw_fscanf (FILE * stream, string * data, const size_t n, const size_t stride);
int gsl_block_string_raw_fprintf (FILE * stream, const string * data, const size_t n, const size_t stride,const char *format);


size_t gsl_block_string_size (const gsl_block_string * b);
string * gsl_block_string_data (const gsl_block_string * b);

int gsl_block_string_add_string (gsl_block_string * b, const string a);
gsl_block_string *gsl_block_string_realloc (gsl_block_string * b, const size_t n);


__END_DECLS

#endif /* alder_align_alder_block_string_h */
