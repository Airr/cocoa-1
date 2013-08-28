//
//  gsl_block_match.h
//  gsl
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef gsl_gsl_block_match_h
#define gsl_gsl_block_match_h

#include <stdlib.h>
#include <stdint.h>
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

struct alder_match_struct
{
    int64_t ref;
    int64_t query;
    int64_t len;
};

typedef struct alder_match_struct alder_match_t;

struct gsl_block_match_struct
{
    size_t size;
    alder_match_t *data;
};

typedef struct gsl_block_match_struct gsl_block_match;

gsl_block_match *gsl_block_match_alloc (const size_t n);
gsl_block_match *gsl_block_match_calloc (const size_t n);
void gsl_block_match_free (gsl_block_match * b);
gsl_block_match *gsl_block_match_realloc (gsl_block_match *b, const size_t n);

int gsl_block_match_fread (FILE * stream, gsl_block_match * b);
int gsl_block_match_fwrite (FILE * stream, const gsl_block_match * b);
int gsl_block_match_fscanf (FILE * stream, gsl_block_match * b);
int gsl_block_match_fprintf (FILE * stream, const gsl_block_match * b, const char *format);

int gsl_block_match_raw_fread (FILE * stream, alder_match_t * data, const size_t n, const size_t stride);
int gsl_block_match_raw_fwrite (FILE * stream, const alder_match_t * data, const size_t n, const size_t stride);
int gsl_block_match_raw_fscanf (FILE * stream, alder_match_t * data, const size_t n, const size_t stride);
int gsl_block_match_raw_fprintf (FILE * stream, const alder_match_t * data, const size_t n, const size_t stride,const char *format);

size_t gsl_block_match_size (const gsl_block_match * b);
alder_match_t * gsl_block_match_data (const gsl_block_match * b);

//int gsl_block_match_add_match (gsl_block_match * b, const gsl_block_match a);
//gsl_block_match *gsl_block_match_realloc (gsl_block_match * b, const size_t n);

__END_DECLS

#endif
