/**
 * This file, gsl_vector_seqpair.h, is part of alder-gsl.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-gsl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-gsl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-gsl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_gsl_gsl_vector_seqpair_h
#define alder_gsl_gsl_vector_seqpair_h

#include <stdlib.h>
#include <gsl/gsl_errno.h>
#include "gsl_vector_anchor.h"

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

struct alder_seqpair_struct {
    char *seq1;
    char *seq2;
    gsl_vector_anchor *anchor;
};
typedef struct alder_seqpair_struct alder_seqpair_t;

struct gsl_block_seqpair_struct
{
    size_t size;
    alder_seqpair_t *data;
};

typedef struct gsl_block_seqpair_struct gsl_block_seqpair;

gsl_block_seqpair *gsl_block_seqpair_alloc (const size_t n);
gsl_block_seqpair *gsl_block_seqpair_calloc (const size_t n);
void gsl_block_seqpair_free (gsl_block_seqpair * b);

int gsl_block_seqpair_fread (FILE * stream, gsl_block_seqpair * b);
int gsl_block_seqpair_fwrite (FILE * stream, const gsl_block_seqpair * b);
int gsl_block_seqpair_fscanf (FILE * stream, gsl_block_seqpair * b);
int gsl_block_seqpair_fprintf (FILE * stream, const gsl_block_seqpair * b, const char *format);

int gsl_block_seqpair_raw_fread (FILE * stream, alder_seqpair_t * b, const size_t n, const size_t stride);
int gsl_block_seqpair_raw_fwrite (FILE * stream, const alder_seqpair_t * b, const size_t n, const size_t stride);
int gsl_block_seqpair_raw_fscanf (FILE * stream, alder_seqpair_t * b, const size_t n, const size_t stride);
int gsl_block_seqpair_raw_fprintf (FILE * stream, const alder_seqpair_t * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_seqpair_size (const gsl_block_seqpair * b);
alder_seqpair_t * gsl_block_seqpair_data (const gsl_block_seqpair * b);
gsl_block_seqpair * gsl_block_seqpair_realloc (gsl_block_seqpair *b, const size_t n);


__END_DECLS

#include <stdlib.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_inline.h>
#include <gsl/gsl_check_range.h>

__BEGIN_DECLS

/* C99 not for GNU */
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#ifndef INLINE_FUN
#define INLINE_FUN inline
#endif

#ifndef INLINE_DECL
#define INLINE_DECL inline
#endif

typedef struct
{
    size_t size;
    size_t stride;
    alder_seqpair_t *data;
    gsl_block_seqpair *block;
    int owner;
}
gsl_vector_seqpair;


/* Allocation */
gsl_vector_seqpair *gsl_vector_seqpair_init ();
int gsl_vector_seqpair_add (gsl_vector_seqpair * a, const alder_seqpair_t m);


gsl_vector_seqpair *gsl_vector_seqpair_alloc (const size_t n);
gsl_vector_seqpair *gsl_vector_seqpair_calloc (const size_t n);

gsl_vector_seqpair *gsl_vector_seqpair_alloc_from_block (gsl_block_seqpair * b,
                                                         const size_t offset,
                                                         const size_t n,
                                                         const size_t stride);

gsl_vector_seqpair *gsl_vector_seqpair_alloc_from_vector (gsl_vector_seqpair * v,
                                                          const size_t offset,
                                                          const size_t n,
                                                          const size_t stride);

void gsl_vector_seqpair_free (gsl_vector_seqpair * v);

int gsl_vector_seqpair_fprintf (FILE * stream, const gsl_vector_seqpair * v,
                                const char *format);


int gsl_vector_seqpair_reverse (gsl_vector_seqpair * v);

int gsl_vector_seqpair_swap (gsl_vector_seqpair * v, gsl_vector_seqpair * w);
int gsl_vector_seqpair_swap_elements (gsl_vector_seqpair * v, const size_t i, const size_t j);

INLINE_DECL alder_seqpair_t gsl_vector_seqpair_get (const gsl_vector_seqpair * v, const size_t i);
INLINE_DECL void gsl_vector_seqpair_set (gsl_vector_seqpair * v, const size_t i, alder_seqpair_t x);
INLINE_DECL alder_seqpair_t * gsl_vector_seqpair_ptr (gsl_vector_seqpair * v, const size_t i);
INLINE_DECL const alder_seqpair_t * gsl_vector_seqpair_const_ptr (const gsl_vector_seqpair * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
alder_seqpair_t
gsl_vector_seqpair_get (const gsl_vector_seqpair * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        abort();
        alder_seqpair_t m;
        GSL_ERROR_VAL ("index out of range", GSL_EINVAL, m);
    }
#endif
    return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_seqpair_set (gsl_vector_seqpair * v, const size_t i, alder_seqpair_t x)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
    v->data[i * v->stride] = x;
}

INLINE_FUN
alder_seqpair_t *
gsl_vector_seqpair_ptr (gsl_vector_seqpair * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (alder_seqpair_t *) (v->data + i * v->stride);
}

INLINE_FUN
const alder_seqpair_t *
gsl_vector_seqpair_const_ptr (const gsl_vector_seqpair * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (const alder_seqpair_t *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS


#endif /* alder_gsl_gsl_vector_seqpair_h */
