//
//  gsl_vector_seqpair.c
//  gsl
//
//  Created by Sang Chul Choi on 8/29/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "gsl_vector_seqpair.h"

gsl_block_seqpair *
gsl_block_seqpair_realloc (gsl_block_seqpair *b, const size_t n)
{
    if (b == NULL) {
        GSL_ERROR_VAL ("b must have been allocated",
                       GSL_EINVAL, 0);
    }
    if (n < b->size) {
        GSL_ERROR_VAL ("b can only increase in size",
                       GSL_EINVAL, 0);
    }
    alder_seqpair_t *t = realloc(b->data, n * sizeof(alder_seqpair_t));
    if (t == NULL) {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    b->data = t;
    b->size = n;
    
    return b;
}

gsl_block_seqpair *
gsl_block_seqpair_alloc (const size_t n)
{
    gsl_block_seqpair * b;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("block length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    b = (gsl_block_seqpair *) malloc (sizeof (gsl_block_seqpair));
    
    if (b == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    
    b->data = (alder_seqpair_t *) calloc (1, 1 * n * sizeof (alder_seqpair_t));
    
    if (b->data == 0)
    {
        free (b);
        
        GSL_ERROR_VAL ("failed to allocate space for block data",
                       GSL_ENOMEM, 0);
    }
    
    b->size = n;
    
    return b;
}

gsl_block_seqpair *
gsl_block_seqpair_calloc (const size_t n)
{
    gsl_block_seqpair * b = gsl_block_seqpair_alloc (n);
    
    if (b == 0)
        return 0;
    
    memset(b->data, 0, n*sizeof(alder_seqpair_t));
    
    return b;
}

void
gsl_block_seqpair_free (gsl_block_seqpair * b)
{
    if (!b) {
        return;
    }
    for (size_t i = 0; i < b->size; i++) {
        if (b->data[i].seq1 != NULL) free(b->data[i].seq1);
        if (b->data[i].seq2 != NULL) free(b->data[i].seq2);
        gsl_vector_anchor_free(b->data[i].anchor);
    }
    free (b->data);
    free (b);
}

size_t
gsl_block_seqpair_size (const gsl_block_seqpair * b)
{
    return b->size ;
}

alder_seqpair_t *
gsl_block_seqpair_data (const gsl_block_seqpair * b)
{
    return b->data ;
}

int
gsl_block_seqpair_fprintf (FILE * stream, const gsl_block_seqpair * b, const char *format)
{
    size_t n = b->size ;
    
    alder_seqpair_t * data = b->data ;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int status;
        const alder_seqpair_t *t = data + i;
        
        status = fprintf(stream, "\n{\n");
        if (status < 0) GSL_ERROR ("fprintf failed", GSL_EFAILED);
        status = fprintf(stream, "%s\n%s\n", t->seq1, t->seq2);
        if (status < 0) GSL_ERROR ("fprintf failed", GSL_EFAILED);
        status = gsl_vector_anchor_fprintf(stream, t->anchor, 0);
        if (status < 0) GSL_ERROR ("fprintf failed", GSL_EFAILED);
        status = fprintf(stream, "}\n");
        if (status < 0) GSL_ERROR ("fprintf failed", GSL_EFAILED);
    }
    
    return 0;
}

int
gsl_block_seqpair_raw_fprintf (FILE * stream,
                               const alder_seqpair_t * data,
                               const size_t n,
                               const size_t stride,
                               const char *format)
{
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int status;
        const alder_seqpair_t *t = data + i * stride;
        
        status = fprintf(stream, "\n{\n");
        if (status < 0) GSL_ERROR ("fprintf failed", GSL_EFAILED);
        status = fprintf(stream, "%s\n%s\n", t->seq1, t->seq2);
        if (status < 0) GSL_ERROR ("fprintf failed", GSL_EFAILED);
        status = gsl_vector_anchor_fprintf(stream, t->anchor, 0);
        if (status < 0) GSL_ERROR ("fprintf failed", GSL_EFAILED);
        status = fprintf(stream, "}\n");
        if (status < 0) GSL_ERROR ("fprintf failed", GSL_EFAILED);
    }
    
    return 0;
}

#pragma mark vector

int
gsl_vector_seqpair_add (gsl_vector_seqpair * a, const alder_seqpair_t m)
{
    if (a->block->size == a->size) {
        // Resize a->block
        gsl_block_seqpair *t = gsl_block_seqpair_realloc (a->block, 2 * a->block->size);
        if (t == NULL) {
            GSL_ERROR ("failed to allocate space for vector struct",
                       GSL_ENOMEM);
        }
        a->data = a->block->data;
    }
    a->data[a->size] = m;
    a->size++;
    
    return GSL_SUCCESS;
}

gsl_vector_seqpair *
gsl_vector_seqpair_init ()
{
    gsl_block_seqpair * block;
    gsl_vector_seqpair * v;
    size_t n = 10;
    
    v = (gsl_vector_seqpair *) malloc (sizeof (gsl_vector_seqpair));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    block = gsl_block_seqpair_alloc (n);
    
    if (block == 0)
    {
        free (v) ;
        
        GSL_ERROR_VAL ("failed to allocate space for block",
                       GSL_ENOMEM, 0);
    }
    
    v->data = block->data ;
    v->size = 0;
    v->stride = 1;
    v->block = block;
    v->owner = 1;
    
    return v;
}

gsl_vector_seqpair *
gsl_vector_seqpair_alloc (const size_t n)
{
    gsl_block_seqpair * block;
    gsl_vector_seqpair * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    v = (gsl_vector_seqpair *) malloc (sizeof (gsl_vector_seqpair));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    block = gsl_block_seqpair_alloc (n);
    
    if (block == 0)
    {
        free (v) ;
        
        GSL_ERROR_VAL ("failed to allocate space for block",
                       GSL_ENOMEM, 0);
    }
    
    v->data = block->data ;
    v->size = n;
    v->stride = 1;
    v->block = block;
    v->owner = 1;
    
    return v;
}

gsl_vector_seqpair *
gsl_vector_seqpair_calloc (const size_t n)
{
    gsl_vector_seqpair * v = gsl_vector_seqpair_alloc (n);
    
    if (v == 0)
        return 0;
    
    memset(v->data, 0, n*sizeof(alder_seqpair_t));
    
    return v;
}

gsl_vector_seqpair *
gsl_vector_seqpair_alloc_from_block (gsl_block_seqpair * block,
                                     const size_t offset,
                                     const size_t n,
                                     const size_t stride)
{
    gsl_vector_seqpair * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    if (stride == 0)
    {
        GSL_ERROR_VAL ("stride must be positive integer", GSL_EINVAL, 0);
    }
    
    if (block->size <= offset + (n - 1) * stride)
    {
        GSL_ERROR_VAL ("vector would extend past end of block", GSL_EINVAL, 0);
    }
    
    v = (gsl_vector_seqpair *) malloc (sizeof (gsl_vector_seqpair));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    v->data = block->data + 1 * offset ;
    v->size = n;
    v->stride = stride;
    v->block = block;
    v->owner = 0;
    
    return v;
}

gsl_vector_seqpair *
gsl_vector_seqpair_alloc_from_vector (gsl_vector_seqpair * w,
                                      const size_t offset,
                                      const size_t n,
                                      const size_t stride)
{
    gsl_vector_seqpair * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    if (stride == 0)
    {
        GSL_ERROR_VAL ("stride must be positive integer", GSL_EINVAL, 0);
    }
    
    if (offset + (n - 1) * stride >= w->size)
    {
        GSL_ERROR_VAL ("vector would extend past end of block", GSL_EINVAL, 0);
    }
    
    v = (gsl_vector_seqpair *) malloc (sizeof (gsl_vector_seqpair));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    v->data = w->data + 1 * w->stride * offset ;
    v->size = n;
    v->stride = stride * w->stride;
    v->block = w->block;
    v->owner = 0;
    
    return v;
}


void
gsl_vector_seqpair_free (gsl_vector_seqpair * v)
{
    if (!v) {
        return;
    }
    
    if (v->owner)
    {
        gsl_block_seqpair_free (v->block) ;
    }
    free (v);
}


int
gsl_vector_seqpair_fprintf (FILE * stream, const gsl_vector_seqpair * v,
                            const char *format)
{
    int status = gsl_block_seqpair_raw_fprintf (stream,
                                                v->data,
                                                v->size,
                                                v->stride,
                                                format);
    return status;
}

int
gsl_vector_seqpair_swap (gsl_vector_seqpair * v, gsl_vector_seqpair * w)
{
    alder_seqpair_t * d1 = v->data ;
    alder_seqpair_t * d2 = w->data ;
    const size_t size = v->size ;
    const size_t s1 = 1 * v->stride ;
    const size_t s2 = 1 * w->stride ;
    size_t i, k ;
    
    if (v->size != w->size)
    {
        GSL_ERROR("vector lengths must be equal", GSL_EINVAL);
    }
    
    for (i = 0; i < size; i++)
    {
        for (k = 0; k < 1; k++)
        {
            alder_seqpair_t tmp = d1[i*s1 + k];
            d1[i*s1+k] = d2[i*s2 + k];
            d2[i*s2+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_seqpair_swap_elements (gsl_vector_seqpair * v, const size_t i, const size_t j)
{
    alder_seqpair_t * data = v->data ;
    const size_t size = v->size ;
    const size_t stride = v->stride ;
    
    if (i >= size)
    {
        GSL_ERROR("first index is out of range", GSL_EINVAL);
    }
    
    if (j >= size)
    {
        GSL_ERROR("second index is out of range", GSL_EINVAL);
    }
    
    if (i != j)
    {
        const size_t s = 1 * stride ;
        size_t k ;
        
        for (k = 0; k < 1; k++)
        {
            alder_seqpair_t tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_seqpair_reverse (gsl_vector_seqpair * v)
{
    alder_seqpair_t * data = v->data ;
    const size_t size = v->size ;
    const size_t stride = v->stride ;
    
    const size_t s = 1 * stride ;
    
    size_t i ;
    
    for (i = 0 ; i < (size / 2) ; i++)
    {
        size_t j = size - i - 1 ;
        size_t k;
        
        for (k = 0; k < 1; k++)
        {
            alder_seqpair_t tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}
