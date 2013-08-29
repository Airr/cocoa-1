//
//  gsl_vector_anchor.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/29/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_gsl_vector_anchor_h
#define alder_align_gsl_vector_anchor_h

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

#include <stdlib.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_inline.h>
#include <gsl/gsl_check_range.h>
#include "gsl_block_anchor.h"

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
    alder_anchor_t *data;
    gsl_block_anchor *block;
    int owner;
}
gsl_vector_anchor;

typedef struct
{
    gsl_vector_anchor vector;
} _gsl_vector_anchor_view;

typedef _gsl_vector_anchor_view gsl_vector_anchor_view;

typedef struct
{
    gsl_vector_anchor vector;
} _gsl_vector_anchor_const_view;

typedef const _gsl_vector_anchor_const_view gsl_vector_anchor_const_view;

/* Addition */
#ifndef GSLVECTORSIZE
#define GSLVECTORSIZE 10
#endif

#ifndef GSLVECTORINCSIZE
#define GSLVECTORINCSIZE 1.2
#endif

/* Allocation */
gsl_vector_anchor *gsl_vector_anchor_init ();
int gsl_vector_anchor_add (gsl_vector_anchor * a, const alder_anchor_t m);

gsl_vector_anchor *gsl_vector_anchor_alloc (const size_t n);
gsl_vector_anchor *gsl_vector_anchor_calloc (const size_t n);

gsl_vector_anchor *gsl_vector_anchor_alloc_from_block (gsl_block_anchor * b,
                                                       const size_t offset,
                                                       const size_t n,
                                                       const size_t stride);

gsl_vector_anchor *gsl_vector_anchor_alloc_from_vector (gsl_vector_anchor * v,
                                                        const size_t offset,
                                                        const size_t n,
                                                        const size_t stride);

void gsl_vector_anchor_free (gsl_vector_anchor * v);

/* Views */

_gsl_vector_anchor_view
gsl_vector_anchor_view_array (alder_anchor_t *v, size_t n);

_gsl_vector_anchor_view
gsl_vector_anchor_view_array_with_stride (alder_anchor_t *base,
                                          size_t stride,
                                          size_t n);

_gsl_vector_anchor_const_view
gsl_vector_anchor_const_view_array (const alder_anchor_t *v, size_t n);

_gsl_vector_anchor_const_view
gsl_vector_anchor_const_view_array_with_stride (const alder_anchor_t *base,
                                                size_t stride,
                                                size_t n);

_gsl_vector_anchor_view
gsl_vector_anchor_subvector (gsl_vector_anchor *v,
                             size_t i,
                             size_t n);

_gsl_vector_anchor_view
gsl_vector_anchor_subvector_with_stride (gsl_vector_anchor *v,
                                         size_t i,
                                         size_t stride,
                                         size_t n);

_gsl_vector_anchor_const_view
gsl_vector_anchor_const_subvector (const gsl_vector_anchor *v,
                                   size_t i,
                                   size_t n);

_gsl_vector_anchor_const_view
gsl_vector_anchor_const_subvector_with_stride (const gsl_vector_anchor *v,
                                               size_t i,
                                               size_t stride,
                                               size_t n);

/* Operations */

void gsl_vector_anchor_set_zero (gsl_vector_anchor * v);
void gsl_vector_anchor_set_all (gsl_vector_anchor * v, alder_anchor_t x);

int gsl_vector_anchor_fread (FILE * stream, gsl_vector_anchor * v);
int gsl_vector_anchor_fwrite (FILE * stream, const gsl_vector_anchor * v);
int gsl_vector_anchor_fscanf (FILE * stream, gsl_vector_anchor * v);
int gsl_vector_anchor_fprintf (FILE * stream, const gsl_vector_anchor * v,
                               const char *format);

int gsl_vector_anchor_memcpy (gsl_vector_anchor * dest, const gsl_vector_anchor * src);

int gsl_vector_anchor_reverse (gsl_vector_anchor * v);

int gsl_vector_anchor_swap (gsl_vector_anchor * v, gsl_vector_anchor * w);
int gsl_vector_anchor_swap_elements (gsl_vector_anchor * v, const size_t i, const size_t j);

alder_anchor_t gsl_vector_anchor_max (const gsl_vector_anchor * v);
alder_anchor_t gsl_vector_anchor_min (const gsl_vector_anchor * v);
void gsl_vector_anchor_minmax (const gsl_vector_anchor * v, alder_anchor_t * min_out, alder_anchor_t * max_out);

size_t gsl_vector_anchor_max_index (const gsl_vector_anchor * v);
size_t gsl_vector_anchor_min_index (const gsl_vector_anchor * v);
void gsl_vector_anchor_minmax_index (const gsl_vector_anchor * v, size_t * imin, size_t * imax);

//int gsl_vector_anchor_add (gsl_vector_anchor * a, const gsl_vector_anchor * b);
//int gsl_vector_anchor_sub (gsl_vector_anchor * a, const gsl_vector_anchor * b);
//int gsl_vector_anchor_mul (gsl_vector_anchor * a, const gsl_vector_anchor * b);
//int gsl_vector_anchor_div (gsl_vector_anchor * a, const gsl_vector_anchor * b);
//int gsl_vector_anchor_scale (gsl_vector_anchor * a, const double x);
//int gsl_vector_anchor_add_constant (gsl_vector_anchor * a, const double x);
//
//int gsl_vector_anchor_equal (const gsl_vector_anchor * u,
//                             const gsl_vector_anchor * v);
//
//int gsl_vector_anchor_isnull (const gsl_vector_anchor * v);
//int gsl_vector_anchor_ispos (const gsl_vector_anchor * v);
//int gsl_vector_anchor_isneg (const gsl_vector_anchor * v);
//int gsl_vector_anchor_isnonneg (const gsl_vector_anchor * v);

INLINE_DECL alder_anchor_t gsl_vector_anchor_get (const gsl_vector_anchor * v, const size_t i);
INLINE_DECL void gsl_vector_anchor_set (gsl_vector_anchor * v, const size_t i, alder_anchor_t x);
INLINE_DECL alder_anchor_t * gsl_vector_anchor_ptr (gsl_vector_anchor * v, const size_t i);
INLINE_DECL const alder_anchor_t * gsl_vector_anchor_const_ptr (const gsl_vector_anchor * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
alder_anchor_t
gsl_vector_anchor_get (const gsl_vector_anchor * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        alder_anchor_t m;
        GSL_ERROR_VAL ("index out of range", GSL_EINVAL, m);
    }
#endif
    return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_anchor_set (gsl_vector_anchor * v, const size_t i, alder_anchor_t x)
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
alder_anchor_t *
gsl_vector_anchor_ptr (gsl_vector_anchor * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (alder_anchor_t *) (v->data + i * v->stride);
}

INLINE_FUN
const alder_anchor_t *
gsl_vector_anchor_const_ptr (const gsl_vector_anchor * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (const alder_anchor_t *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif
