//
//  alder_vector_junction.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_vector_junction_h
#define alder_align_alder_vector_junction_h

#include <stdlib.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_inline.h>
#include <gsl/gsl_check_range.h>
#include "alder_junction.h"
#include "alder_block_junction.h"

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

typedef struct
{
    size_t size;
    size_t stride;
    alder_junction *data;
    alder_block_junction *block;
    int owner;
}
alder_vector_junction;

typedef struct
{
    alder_vector_junction vector;
} _alder_vector_junction_view;

typedef _alder_vector_junction_view alder_vector_junction_view;

typedef struct
{
    alder_vector_junction vector;
} _alder_vector_junction_const_view;

typedef const _alder_vector_junction_const_view alder_vector_junction_const_view;


/* Allocation */

alder_vector_junction *alder_vector_junction_alloc (const size_t n);
alder_vector_junction *alder_vector_junction_calloc (const size_t n);

alder_vector_junction *alder_vector_junction_alloc_from_block (alder_block_junction * b,
                                                     const size_t offset,
                                                     const size_t n,
                                                     const size_t stride);

alder_vector_junction *alder_vector_junction_alloc_from_vector (alder_vector_junction * v,
                                                      const size_t offset,
                                                      const size_t n,
                                                      const size_t stride);

void alder_vector_junction_free (alder_vector_junction * v);

/* Views */

_alder_vector_junction_view
alder_vector_junction_view_array (alder_junction *v, size_t n);

_alder_vector_junction_view
alder_vector_junction_view_array_with_stride (alder_junction *base,
                                         size_t stride,
                                         size_t n);

_alder_vector_junction_const_view
alder_vector_junction_const_view_array (const alder_junction *v, size_t n);

_alder_vector_junction_const_view
alder_vector_junction_const_view_array_with_stride (const alder_junction *base,
                                               size_t stride,
                                               size_t n);

_alder_vector_junction_view
alder_vector_junction_subvector (alder_vector_junction *v,
                            size_t i,
                            size_t n);

_alder_vector_junction_view
alder_vector_junction_subvector_with_stride (alder_vector_junction *v,
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_alder_vector_junction_const_view
alder_vector_junction_const_subvector (const alder_vector_junction *v,
                                  size_t i,
                                  size_t n);

_alder_vector_junction_const_view
alder_vector_junction_const_subvector_with_stride (const alder_vector_junction *v,
                                              size_t i,
                                              size_t stride,
                                              size_t n);

/* Operations */

void alder_vector_junction_set_zero (alder_vector_junction * v);
void alder_vector_junction_set_all (alder_vector_junction * v, alder_junction x);
int alder_vector_junction_set_basis (alder_vector_junction * v, size_t i);

int alder_vector_junction_fread (FILE * stream, alder_vector_junction * v);
int alder_vector_junction_fwrite (FILE * stream, const alder_vector_junction * v);
int alder_vector_junction_fscanf (FILE * stream, alder_vector_junction * v);
int alder_vector_junction_fprintf (FILE * stream, const alder_vector_junction * v,
                              const char *format);

int alder_vector_junction_memcpy (alder_vector_junction * dest, const alder_vector_junction * src);

int alder_vector_junction_reverse (alder_vector_junction * v);

int alder_vector_junction_swap (alder_vector_junction * v, alder_vector_junction * w);
int alder_vector_junction_swap_elements (alder_vector_junction * v, const size_t i, const size_t j);

alder_junction alder_vector_junction_max (const alder_vector_junction * v);
alder_junction alder_vector_junction_min (const alder_vector_junction * v);
void alder_vector_junction_minmax (const alder_vector_junction * v, alder_junction * min_out, alder_junction * max_out);

size_t alder_vector_junction_max_index (const alder_vector_junction * v);
size_t alder_vector_junction_min_index (const alder_vector_junction * v);
void alder_vector_junction_minmax_index (const alder_vector_junction * v, size_t * imin, size_t * imax);

int alder_vector_junction_add (alder_vector_junction * a, const alder_vector_junction * b);
int alder_vector_junction_sub (alder_vector_junction * a, const alder_vector_junction * b);
int alder_vector_junction_mul (alder_vector_junction * a, const alder_vector_junction * b);
int alder_vector_junction_div (alder_vector_junction * a, const alder_vector_junction * b);
int alder_vector_junction_scale (alder_vector_junction * a, const double x);
int alder_vector_junction_add_constant (alder_vector_junction * a, const double x);

int alder_vector_junction_equal (const alder_vector_junction * u,
                            const alder_vector_junction * v);

int alder_vector_junction_isnull (const alder_vector_junction * v);
int alder_vector_junction_ispos (const alder_vector_junction * v);
int alder_vector_junction_isneg (const alder_vector_junction * v);
int alder_vector_junction_isnonneg (const alder_vector_junction * v);

INLINE_DECL alder_junction alder_vector_junction_get (const alder_vector_junction * v, const size_t i);
INLINE_DECL void alder_vector_junction_set (alder_vector_junction * v, const size_t i, alder_junction x);
INLINE_DECL alder_junction * alder_vector_junction_ptr (alder_vector_junction * v, const size_t i);
INLINE_DECL const alder_junction * alder_vector_junction_const_ptr (const alder_vector_junction * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
alder_junction
alder_vector_junction_get (const alder_vector_junction * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
    return v->data[i * v->stride];
}

INLINE_FUN
void
alder_vector_junction_set (alder_vector_junction * v, const size_t i, alder_junction x)
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
alder_junction *
alder_vector_junction_ptr (alder_vector_junction * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (alder_junction *) (v->data + i * v->stride);
}

INLINE_FUN
const alder_junction *
alder_vector_junction_const_ptr (const alder_vector_junction * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (const alder_junction *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif
