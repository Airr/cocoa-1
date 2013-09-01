//
//  gsl_vector_sam.h
//  gsl
//
//  Created by Sang Chul Choi on 8/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef gsl_gsl_vector_sam_h
#define gsl_gsl_vector_sam_h

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

struct alder_sam_struct {
    char     *qname;
    uint16_t flag;
    char     *rname;
    uint32_t pos;
    uint8_t  mapq;
    char     *cigar;
    char     *rnext;
    uint32_t pnext;
    int32_t  tlen;
    char     *seq;
    char     *qual;
    char     *optional;
};

typedef struct alder_sam_struct alder_sam_t;

struct gsl_block_sam_struct
{
    size_t size;
    alder_sam_t *data;
};

typedef struct gsl_block_sam_struct gsl_block_sam;

gsl_block_sam *gsl_block_sam_alloc (const size_t n);
void gsl_block_sam_free (gsl_block_sam * b);

int gsl_block_sam_fprintf (FILE * stream, const gsl_block_sam * b, const char *format);
int gsl_block_sam_raw_fprintf (FILE * stream, const alder_sam_t * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_sam_size (const gsl_block_sam * b);
alder_sam_t * gsl_block_sam_data (const gsl_block_sam * b);


__END_DECLS


#include <stdlib.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_inline.h>
#include <gsl/gsl_check_range.h>

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

//#ifndef HAVE_INLINE
//#define HAVE_INLINE
//#endif
//
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

typedef struct
{
    size_t size;
    size_t stride;
    alder_sam_t *data;
    gsl_block_sam *block;
    int owner;
}
gsl_vector_sam;

/* Allocation */
gsl_vector_sam *gsl_vector_sam_alloc (const size_t n);
void gsl_vector_sam_free (gsl_vector_sam * v);


/* Operations */
int gsl_vector_sam_fprintf (FILE * stream, const gsl_vector_sam * v,
                            const char *format);

INLINE_DECL void gsl_vector_sam_set (gsl_vector_sam * v, const size_t i, alder_sam_t x);
INLINE_DECL alder_sam_t * gsl_vector_sam_ptr (gsl_vector_sam * v, const size_t i);
INLINE_DECL const alder_sam_t * gsl_vector_sam_const_ptr (const gsl_vector_sam * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
void
gsl_vector_sam_set (gsl_vector_sam * v, const size_t i, alder_sam_t x)
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
alder_sam_t *
gsl_vector_sam_ptr (gsl_vector_sam * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (alder_sam_t *) (v->data + i * v->stride);
}

INLINE_FUN
const alder_sam_t *
gsl_vector_sam_const_ptr (const gsl_vector_sam * v, const size_t i)
{
#if GSL_RANGE_CHECK
    if (GSL_RANGE_COND(i >= v->size))
    {
        GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
    return (const alder_sam_t *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif
