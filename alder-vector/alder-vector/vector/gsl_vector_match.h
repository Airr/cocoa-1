/**
 * This file, gsl_vector_match.h, is part of alder-gsl.
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

#ifndef alder_gsl_gsl_vector_match_h
#define alder_gsl_gsl_vector_match_h

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

///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_inline.h>
#include <gsl/gsl_check_range.h>

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
int gsl_vector_match_reset (gsl_vector_match * a);

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

#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#ifdef INLINE_DECL
#undef INLINE_DECL
#endif

#ifndef INLINE_DECL
#define INLINE_DECL static inline
#endif

#ifdef INLINE_FUN
#undef INLINE_FUN
#endif

#ifndef INLINE_FUN
#define INLINE_FUN static inline
#endif

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


#endif /* alder_gsl_gsl_vector_match_h */
