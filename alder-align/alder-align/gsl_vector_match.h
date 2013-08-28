//
//  gsl_vector_match.h
//  gsl
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef gsl_gsl_vector_match_h
#define gsl_gsl_vector_match_h

#include <stdlib.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_inline.h>
#include <gsl/gsl_check_range.h>
#include "gsl_block_match.h"

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

// size is the size of valid elements.
// block->size is larger than size so that we can add an element.
typedef struct
{
    size_t size;
    size_t stride;
    alder_match_t *data;
    gsl_block_match *block;
    int owner;
}
gsl_vector_match;

typedef struct
{
    gsl_vector_match vector;
} _gsl_vector_match_view;

typedef _gsl_vector_match_view gsl_vector_match_view;

typedef struct
{
    gsl_vector_match vector;
} _gsl_vector_match_const_view;

typedef const _gsl_vector_match_const_view gsl_vector_match_const_view;

/* Addition */
#ifndef GSLVECTORMATCHINITSIZE
#define GSLVECTORMATCHINITSIZE 10
#endif
#ifndef GSLVECTORMATCHINCSIZE
#define GSLVECTORMATCHINCSIZE 1.2
#endif
gsl_vector_match * gsl_vector_match_init ();
int gsl_vector_match_add_alder_match (gsl_vector_match * a, const alder_match_t m);

/* Allocation */

gsl_vector_match *gsl_vector_match_alloc (const size_t n);
gsl_vector_match *gsl_vector_match_calloc (const size_t n);

gsl_vector_match *gsl_vector_match_alloc_from_block (gsl_block_match * b,
                                                       const size_t offset,
                                                       const size_t n,
                                                       const size_t stride);

gsl_vector_match *gsl_vector_match_alloc_from_vector (gsl_vector_match * v,
                                                        const size_t offset,
                                                        const size_t n,
                                                        const size_t stride);

void gsl_vector_match_free (gsl_vector_match * v);

/* Views */

_gsl_vector_match_view
gsl_vector_match_view_array (gsl_vector_match *v, size_t n);

_gsl_vector_match_view
gsl_vector_match_view_array_with_stride (gsl_vector_match *base,
                                          size_t stride,
                                          size_t n);

_gsl_vector_match_const_view
gsl_vector_match_const_view_array (const gsl_vector_match *v, size_t n);

_gsl_vector_match_const_view
gsl_vector_match_const_view_array_with_stride (const gsl_vector_match *base,
                                                size_t stride,
                                                size_t n);

_gsl_vector_match_view
gsl_vector_match_subvector (gsl_vector_match *v,
                             size_t i,
                             size_t n);

_gsl_vector_match_view
gsl_vector_match_subvector_with_stride (gsl_vector_match *v,
                                         size_t i,
                                         size_t stride,
                                         size_t n);

_gsl_vector_match_const_view
gsl_vector_match_const_subvector (const gsl_vector_match *v,
                                   size_t i,
                                   size_t n);

_gsl_vector_match_const_view
gsl_vector_match_const_subvector_with_stride (const gsl_vector_match *v,
                                               size_t i,
                                               size_t stride,
                                               size_t n);

/* Operations */

void gsl_vector_match_set_zero (gsl_vector_match * v);
void gsl_vector_match_set_all (gsl_vector_match * v, alder_match_t x);

int gsl_vector_match_fread (FILE * stream, gsl_vector_match * v);
int gsl_vector_match_fwrite (FILE * stream, const gsl_vector_match * v);
int gsl_vector_match_fscanf (FILE * stream, gsl_vector_match * v);
int gsl_vector_match_fprintf (FILE * stream, const gsl_vector_match * v,
                               const char *format);

int gsl_vector_match_memcpy (gsl_vector_match * dest, const gsl_vector_match * src);

int gsl_vector_match_reverse (gsl_vector_match * v);

int gsl_vector_match_swap (gsl_vector_match * v, gsl_vector_match * w);
int gsl_vector_match_swap_elements (gsl_vector_match * v, const size_t i, const size_t j);

alder_match_t gsl_vector_match_max (const gsl_vector_match * v);
alder_match_t gsl_vector_match_min (const gsl_vector_match * v);
void gsl_vector_match_minmax (const gsl_vector_match * v, alder_match_t * min_out, alder_match_t * max_out);

size_t gsl_vector_match_max_index (const gsl_vector_match * v);
size_t gsl_vector_match_min_index (const gsl_vector_match * v);
void gsl_vector_match_minmax_index (const gsl_vector_match * v, size_t * imin, size_t * imax);

INLINE_DECL alder_match_t gsl_vector_match_get (const gsl_vector_match * v, const size_t i);
INLINE_DECL void gsl_vector_match_set (gsl_vector_match * v, const size_t i, alder_match_t x);
INLINE_DECL alder_match_t * gsl_vector_match_ptr (gsl_vector_match * v, const size_t i);
INLINE_DECL const alder_match_t * gsl_vector_match_const_ptr (const gsl_vector_match * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
alder_match_t
gsl_vector_match_get (const gsl_vector_match * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        alder_match_t m = { 0, 0, 0 };
        GSL_ERROR_VAL ("index out of range", GSL_EINVAL, m);
    }
#endif
    return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_match_set (gsl_vector_match * v, const size_t i, alder_match_t x)
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
alder_match_t *
gsl_vector_match_ptr (gsl_vector_match * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (alder_match_t *) (v->data + i * v->stride);
}

INLINE_FUN
const alder_match_t *
gsl_vector_match_const_ptr (const gsl_vector_match * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (const alder_match_t *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS



#endif
