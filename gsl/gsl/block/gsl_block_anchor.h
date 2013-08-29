//
//  gsl_block_anchor.h
//  gsl
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef gsl_gsl_block_anchor_h
#define gsl_gsl_block_anchor_h

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

struct alder_anchor_struct {
    int anchor1;
    int anchor2;
    int anchorLength;
    double anchorScore;
};
typedef struct alder_anchor_struct alder_anchor_t;

struct gsl_block_anchor_struct
{
    size_t size;
    alder_anchor_t *data;
};

typedef struct gsl_block_anchor_struct gsl_block_anchor;

gsl_block_anchor *gsl_block_anchor_alloc (const size_t n);
gsl_block_anchor *gsl_block_anchor_calloc (const size_t n);
void gsl_block_anchor_free (gsl_block_anchor * b);

int gsl_block_anchor_fread (FILE * stream, gsl_block_anchor * b);
int gsl_block_anchor_fwrite (FILE * stream, const gsl_block_anchor * b);
int gsl_block_anchor_fscanf (FILE * stream, gsl_block_anchor * b);
int gsl_block_anchor_fprintf (FILE * stream, const gsl_block_anchor * b, const char *format);

int gsl_block_anchor_raw_fread (FILE * stream, alder_anchor_t * b, const size_t n, const size_t stride);
int gsl_block_anchor_raw_fwrite (FILE * stream, const alder_anchor_t * b, const size_t n, const size_t stride);
int gsl_block_anchor_raw_fscanf (FILE * stream, alder_anchor_t * b, const size_t n, const size_t stride);
int gsl_block_anchor_raw_fprintf (FILE * stream, const alder_anchor_t * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_anchor_size (const gsl_block_anchor * b);
alder_anchor_t * gsl_block_anchor_data (const gsl_block_anchor * b);
gsl_block_anchor * gsl_block_anchor_realloc (gsl_block_anchor *b, const size_t n);

__END_DECLS

#endif