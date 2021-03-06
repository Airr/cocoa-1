//
//  gsl_vector_anchor.c
//  gsl
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "gsl_vector_anchor.h"


gsl_vector_anchor *
gsl_vector_anchor_alloc (const size_t n)
{
    gsl_block_anchor * block;
    gsl_vector_anchor * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    v = (gsl_vector_anchor *) malloc (sizeof (gsl_vector_anchor));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    block = gsl_block_anchor_alloc (n);
    
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

gsl_vector_anchor *
gsl_vector_anchor_init ()
{
    gsl_block_anchor * block;
    gsl_vector_anchor * v;
    
    v = (gsl_vector_anchor *) malloc (sizeof (gsl_vector_anchor));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    block = gsl_block_anchor_alloc (GSLVECTORSIZE);
    
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

gsl_vector_anchor *
gsl_vector_anchor_calloc (const size_t n)
{
    gsl_vector_anchor * v = gsl_vector_anchor_alloc (n);
    
    if (v == 0)
        return 0;
    
    memset(v->data, 0, n*sizeof(alder_anchor_t));

    return v;
}

gsl_vector_anchor *
gsl_vector_anchor_alloc_from_block (gsl_block_anchor * block,
                                    const size_t offset,
                                    const size_t n,
                                    const size_t stride)
{
    gsl_vector_anchor * v;
    
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
    
    v = (gsl_vector_anchor *) malloc (sizeof (gsl_vector_anchor));
    
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

gsl_vector_anchor *
gsl_vector_anchor_alloc_from_vector (gsl_vector_anchor * w,
                                     const size_t offset,
                                     const size_t n,
                                     const size_t stride)
{
    gsl_vector_anchor * v;
    
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
    
    v = (gsl_vector_anchor *) malloc (sizeof (gsl_vector_anchor));
    
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
gsl_vector_anchor_free (gsl_vector_anchor * v)
{
//    RETURN_IF_NULL (v);
    if (!v) {
        return;
    }
    
    if (v->owner)
    {
        gsl_block_anchor_free (v->block) ;
    }
    free (v);
}

int
gsl_vector_anchor_fread (FILE * stream, gsl_vector_anchor * v)
{
    int status = gsl_block_anchor_raw_fread (stream,
                                             v->data,
                                             v->size,
                                             v->stride);
    return status;
}

int
gsl_vector_anchor_fwrite (FILE * stream, const gsl_vector_anchor * v)
{
    int status = gsl_block_anchor_raw_fwrite (stream,
                                              v->data,
                                              v->size,
                                              v->stride);
    return status;
}


int
gsl_vector_anchor_fprintf (FILE * stream, const gsl_vector_anchor * v,
                           const char *format)
{
    int status = gsl_block_anchor_raw_fprintf (stream,
                                               v->data,
                                               v->size,
                                               v->stride,
                                               format);
    return status;
}

int
gsl_vector_anchor_fscanf (FILE * stream, gsl_vector_anchor * v)
{
    int status = gsl_block_anchor_raw_fscanf (stream,
                                              v->data,
                                              v->size,
                                              v->stride);
    return status;
}

int
gsl_vector_anchor_memcpy (gsl_vector_anchor * dest,
                          const gsl_vector_anchor * src)
{
    const size_t src_size = src->size;
    const size_t dest_size = dest->size;
    
    if (src_size != dest_size)
    {
        GSL_ERROR ("vector lengths are not equal", GSL_EBADLEN);
    }
    
    {
        const size_t src_stride = src->stride ;
        const size_t dest_stride = dest->stride ;
        size_t j;
        
        for (j = 0; j < src_size; j++)
        {
            size_t k;
            
            for (k = 0; k < 1; k++)
            {
                dest->data[1 * dest_stride * j + k]
                = src->data[1 * src_stride * j + k];
            }
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_anchor_swap (gsl_vector_anchor * v, gsl_vector_anchor * w)
{
    alder_anchor_t * d1 = v->data ;
    alder_anchor_t * d2 = w->data ;
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
            alder_anchor_t tmp = d1[i*s1 + k];
            d1[i*s1+k] = d2[i*s2 + k];
            d2[i*s2+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_anchor_swap_elements (gsl_vector_anchor * v, const size_t i, const size_t j)
{
    alder_anchor_t * data = v->data ;
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
            alder_anchor_t tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_anchor_reverse (gsl_vector_anchor * v)
{
    alder_anchor_t * data = v->data ;
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
            alder_anchor_t tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}


int
gsl_vector_anchor_add (gsl_vector_anchor * a, const alder_anchor_t m)
{
    if (a->block->size == a->size) {
        // Resize a->block
        gsl_block_anchor *t = gsl_block_anchor_realloc (a->block, 2 * a->block->size);
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

