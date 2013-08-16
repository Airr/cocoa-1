//
//  gsl_vector_string.h
//  gsl
//
//  Created by Sang Chul Choi on 8/10/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef gsl_gsl_vector_string_h
#define gsl_gsl_vector_string_h

#include <stdlib.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_inline.h>
#include <gsl/gsl_check_range.h>
#include "gsl_block_string.h"

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
    string *data;
    gsl_block_string *block;
    int owner;
}
gsl_vector_string;

typedef struct
{
    gsl_vector_string vector;
} _gsl_vector_string_view;

typedef _gsl_vector_string_view gsl_vector_string_view;

typedef struct
{
    gsl_vector_string vector;
} _gsl_vector_string_const_view;

typedef const _gsl_vector_string_const_view gsl_vector_string_const_view;


/* Allocation */

gsl_vector_string *gsl_vector_string_alloc (const size_t n);
gsl_vector_string *gsl_vector_string_calloc (const size_t n);

gsl_vector_string *gsl_vector_string_alloc_from_block (gsl_block_string * b,
                                                     const size_t offset,
                                                     const size_t n,
                                                     const size_t stride);

gsl_vector_string *gsl_vector_string_alloc_from_vector (gsl_vector_string * v,
                                                      const size_t offset,
                                                      const size_t n,
                                                      const size_t stride);

void gsl_vector_string_free (gsl_vector_string * v);

/* Views */

_gsl_vector_string_view
gsl_vector_string_view_array (string *v, size_t n);

_gsl_vector_string_view
gsl_vector_string_view_array_with_stride (string *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_string_const_view
gsl_vector_string_const_view_array (const string *v, size_t n);

_gsl_vector_string_const_view
gsl_vector_string_const_view_array_with_stride (const string *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_string_view
gsl_vector_string_subvector (gsl_vector_string *v,
                            size_t i,
                            size_t n);

_gsl_vector_string_view
gsl_vector_string_subvector_with_stride (gsl_vector_string *v,
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_string_const_view
gsl_vector_string_const_subvector (const gsl_vector_string *v,
                                  size_t i,
                                  size_t n);

_gsl_vector_string_const_view
gsl_vector_string_const_subvector_with_stride (const gsl_vector_string *v,
                                              size_t i,
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_string_set_zero (gsl_vector_string * v);
void gsl_vector_string_set_all (gsl_vector_string * v, string x);
int gsl_vector_string_set_basis (gsl_vector_string * v, size_t i);

int gsl_vector_string_fread (FILE * stream, gsl_vector_string * v);
int gsl_vector_string_fwrite (FILE * stream, const gsl_vector_string * v);
int gsl_vector_string_fscanf (FILE * stream, gsl_vector_string * v);
int gsl_vector_string_fprintf (FILE * stream, const gsl_vector_string * v,
                              const char *format);

int gsl_vector_string_memcpy (gsl_vector_string * dest, const gsl_vector_string * src);

int gsl_vector_string_reverse (gsl_vector_string * v);

int gsl_vector_string_swap (gsl_vector_string * v, gsl_vector_string * w);
int gsl_vector_string_swap_elements (gsl_vector_string * v, const size_t i, const size_t j);

string gsl_vector_string_max (const gsl_vector_string * v);
string gsl_vector_string_min (const gsl_vector_string * v);
void gsl_vector_string_minmax (const gsl_vector_string * v, string * min_out, string * max_out);

size_t gsl_vector_string_max_index (const gsl_vector_string * v);
size_t gsl_vector_string_min_index (const gsl_vector_string * v);
void gsl_vector_string_minmax_index (const gsl_vector_string * v, size_t * imin, size_t * imax);

int gsl_vector_string_add (gsl_vector_string * a, const gsl_vector_string * b);
int gsl_vector_string_sub (gsl_vector_string * a, const gsl_vector_string * b);
int gsl_vector_string_mul (gsl_vector_string * a, const gsl_vector_string * b);
int gsl_vector_string_div (gsl_vector_string * a, const gsl_vector_string * b);
int gsl_vector_string_scale (gsl_vector_string * a, const double x);
int gsl_vector_string_add_constant (gsl_vector_string * a, const double x);

int gsl_vector_string_equal (const gsl_vector_string * u,
                            const gsl_vector_string * v);

int gsl_vector_string_isnull (const gsl_vector_string * v);
int gsl_vector_string_ispos (const gsl_vector_string * v);
int gsl_vector_string_isneg (const gsl_vector_string * v);
int gsl_vector_string_isnonneg (const gsl_vector_string * v);

INLINE_DECL string gsl_vector_string_get (const gsl_vector_string * v, const size_t i);
INLINE_DECL void gsl_vector_string_set (gsl_vector_string * v, const size_t i, string x);
INLINE_DECL string * gsl_vector_string_ptr (gsl_vector_string * v, const size_t i);
INLINE_DECL const string * gsl_vector_string_const_ptr (const gsl_vector_string * v, const size_t i);

int gsl_vector_string_add_string (gsl_vector_string * v, const string b);


#ifdef HAVE_INLINE

INLINE_FUN
string
gsl_vector_string_get (const gsl_vector_string * v, const size_t i)
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
gsl_vector_string_set (gsl_vector_string * v, const size_t i, string x)
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
string *
gsl_vector_string_ptr (gsl_vector_string * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (string *) (v->data + i * v->stride);
}

INLINE_FUN
const string *
gsl_vector_string_const_ptr (const gsl_vector_string * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (const string *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS


#endif
