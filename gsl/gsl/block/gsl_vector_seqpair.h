//
//  gsl_vector_seqpair.h
//  gsl
//
//  Created by Sang Chul Choi on 8/29/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef gsl_gsl_vector_seqpair_h
#define gsl_gsl_vector_seqpair_h

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

//typedef struct
//{
//  gsl_vector_seqpair vector;
//} _gsl_vector_seqpair_view;
//
//typedef _gsl_vector_seqpair_view gsl_vector_seqpair_view;
//
//typedef struct
//{
//  gsl_vector_seqpair vector;
//} _gsl_vector_seqpair_const_view;
//
//typedef const _gsl_vector_seqpair_const_view gsl_vector_seqpair_const_view;

/* Addition */
//#ifndef GSLVECTORSIZE
//#define GSLVECTORSIZE 10
//#endif
//
//#ifndef GSLVECTORINCSIZE
//#define GSLVECTORINCSIZE 1.2
//#endif

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

/* Views */

//_gsl_vector_seqpair_view
//gsl_vector_seqpair_view_array (alder_seqpair_t *v, size_t n);
//
//_gsl_vector_seqpair_view
//gsl_vector_seqpair_view_array_with_stride (alder_seqpair_t *base,
//                                         size_t stride,
//                                         size_t n);
//
//_gsl_vector_seqpair_const_view
//gsl_vector_seqpair_const_view_array (const alder_seqpair_t *v, size_t n);
//
//_gsl_vector_seqpair_const_view
//gsl_vector_seqpair_const_view_array_with_stride (const alder_seqpair_t *base,
//                                               size_t stride,
//                                               size_t n);
//
//_gsl_vector_seqpair_view
//gsl_vector_seqpair_subvector (gsl_vector_seqpair *v,
//                            size_t i,
//                            size_t n);
//
//_gsl_vector_seqpair_view
//gsl_vector_seqpair_subvector_with_stride (gsl_vector_seqpair *v,
//                                        size_t i,
//                                        size_t stride,
//                                        size_t n);
//
//_gsl_vector_seqpair_const_view
//gsl_vector_seqpair_const_subvector (const gsl_vector_seqpair *v,
//                                  size_t i,
//                                  size_t n);
//
//_gsl_vector_seqpair_const_view
//gsl_vector_seqpair_const_subvector_with_stride (const gsl_vector_seqpair *v,
//                                              size_t i,
//                                              size_t stride,
//                                              size_t n);

/* Operations */
// void gsl_vector_seqpair_set_zero (gsl_vector_seqpair * v);
// void gsl_vector_seqpair_set_all (gsl_vector_seqpair * v, alder_seqpair_t x);

//int gsl_vector_seqpair_fread (FILE * stream, gsl_vector_seqpair * v);
//int gsl_vector_seqpair_fwrite (FILE * stream, const gsl_vector_seqpair * v);
//int gsl_vector_seqpair_fscanf (FILE * stream, gsl_vector_seqpair * v);
int gsl_vector_seqpair_fprintf (FILE * stream, const gsl_vector_seqpair * v,
                                const char *format);

//int gsl_vector_seqpair_memcpy (gsl_vector_seqpair * dest, const gsl_vector_seqpair * src);

int gsl_vector_seqpair_reverse (gsl_vector_seqpair * v);

int gsl_vector_seqpair_swap (gsl_vector_seqpair * v, gsl_vector_seqpair * w);
int gsl_vector_seqpair_swap_elements (gsl_vector_seqpair * v, const size_t i, const size_t j);

// alder_seqpair_t gsl_vector_seqpair_max (const gsl_vector_seqpair * v);
// alder_seqpair_t gsl_vector_seqpair_min (const gsl_vector_seqpair * v);
// void gsl_vector_seqpair_minmax (const gsl_vector_seqpair * v, alder_seqpair_t * min_out, alder_seqpair_t * max_out);
//
// size_t gsl_vector_seqpair_max_index (const gsl_vector_seqpair * v);
// size_t gsl_vector_seqpair_min_index (const gsl_vector_seqpair * v);
// void gsl_vector_seqpair_minmax_index (const gsl_vector_seqpair * v, size_t * imin, size_t * imax);

// int gsl_vector_seqpair_add (gsl_vector_seqpair * a, const gsl_vector_seqpair * b);
// int gsl_vector_seqpair_sub (gsl_vector_seqpair * a, const gsl_vector_seqpair * b);
// int gsl_vector_seqpair_mul (gsl_vector_seqpair * a, const gsl_vector_seqpair * b);
// int gsl_vector_seqpair_div (gsl_vector_seqpair * a, const gsl_vector_seqpair * b);
// int gsl_vector_seqpair_scale (gsl_vector_seqpair * a, const double x);
// int gsl_vector_seqpair_add_constant (gsl_vector_seqpair * a, const double x);
//
// int gsl_vector_seqpair_equal (const gsl_vector_seqpair * u,
//                             const gsl_vector_seqpair * v);
//
// int gsl_vector_seqpair_isnull (const gsl_vector_seqpair * v);
// int gsl_vector_seqpair_ispos (const gsl_vector_seqpair * v);
// int gsl_vector_seqpair_isneg (const gsl_vector_seqpair * v);
// int gsl_vector_seqpair_isnonneg (const gsl_vector_seqpair * v);

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

#endif
