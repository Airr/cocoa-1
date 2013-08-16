//
//  gsl_block_junction.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_gsl_block_junction_h
#define alder_align_gsl_block_junction_h

#include <stdlib.h>
#include <gsl/gsl_errno.h>
#include "alder_junction.h"

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

struct gsl_block_junction_struct
{
    size_t size;
    alder_junction *data;
};

typedef struct gsl_block_junction_struct gsl_block_junction;

gsl_block_junction *gsl_block_junction_alloc (const size_t n);
gsl_block_junction *gsl_block_junction_calloc (const size_t n);
void gsl_block_junction_free (gsl_block_junction * b);

int gsl_block_junction_fread (FILE * stream, gsl_block_junction * b);
int gsl_block_junction_fwrite (FILE * stream, const gsl_block_junction * b);
int gsl_block_junction_fscanf (FILE * stream, gsl_block_junction * b);
int gsl_block_junction_fprintf (FILE * stream, const gsl_block_junction * b, const char *format);

int gsl_block_junction_raw_fread (FILE * stream, alder_junction * b, const size_t n, const size_t stride);
int gsl_block_junction_raw_fwrite (FILE * stream, const alder_junction * b, const size_t n, const size_t stride);
int gsl_block_junction_raw_fscanf (FILE * stream, alder_junction * b, const size_t n, const size_t stride);
int gsl_block_junction_raw_fprintf (FILE * stream, const alder_junction * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_junction_size (const gsl_block_junction * b);
unsigned long * gsl_block_junction_data (const gsl_block_junction * b);

__END_DECLS

#endif
