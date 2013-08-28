//
//  gsl_vector_match.c
//  gsl
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "gsl_block_match.h"
#include "gsl_vector_match.h"


#pragma mark init
gsl_vector_match *
gsl_vector_match_alloc (const size_t n)
{
    gsl_block_match * block;
    gsl_vector_match * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    

    v = (gsl_vector_match *) malloc (sizeof (gsl_vector_match));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    block = gsl_block_match_alloc (n);
    
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

gsl_vector_match *
gsl_vector_match_init ()
{
    gsl_block_match * block;
    gsl_vector_match * v;
    
    v = (gsl_vector_match *) malloc (sizeof (gsl_vector_match));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    block = gsl_block_match_alloc (GSLVECTORMATCHINITSIZE);
    
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

int
gsl_vector_match_add_alder_match (gsl_vector_match * a, const alder_match_t m)
{
    size_t n = (size_t)((double)a->size * GSLVECTORMATCHINCSIZE);
    if (a->block->size < n) {
        // Resize a->block
        gsl_block_match *t = gsl_block_match_realloc (a->block, n * 2);
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

gsl_vector_match *
gsl_vector_match_calloc (const size_t n)
{
    size_t i;
    
    gsl_vector_match * v = gsl_vector_match_alloc (n);
    
    if (v == 0)
        return 0;
    
    
    
    for (i = 0; i < 1 * n; i++)
    {
        v->data[i].query = 0;
        v->data[i].ref = 0;
        v->data[i].len = 0;
    }
    
    return v;
}

gsl_vector_match *
gsl_vector_match_alloc_from_block (gsl_block_match * block,
                                   const size_t offset,
                                   const size_t n,
                                   const size_t stride)
{
    gsl_vector_match * v;
    
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
    
    v = (gsl_vector_match *) malloc (sizeof (gsl_vector_match));
    
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

gsl_vector_match *
gsl_vector_match_alloc_from_vector (gsl_vector_match * w,
                                    const size_t offset,
                                    const size_t n,
                                    const size_t stride)
{
    gsl_vector_match * v;
    
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
    
    v = (gsl_vector_match *) malloc (sizeof (gsl_vector_match));
    
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
gsl_vector_match_free (gsl_vector_match * v)
{
    if (!v) { return ; };
    
    if (v->owner)
    {
        gsl_block_match_free (v->block) ;
    }
    free (v);
}


void
gsl_vector_match_set_all (gsl_vector_match * v, alder_match_t x)
{
    alder_match_t * const data = v->data;
    const size_t n = v->size;
    const size_t stride = v->stride;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        *(alder_match_t *) (data + 1 * i * stride) = x;
    }
}

void
gsl_vector_match_set_zero (gsl_vector_match * v)
{
    alder_match_t * const data = v->data;
    const size_t n = v->size;
    const size_t stride = v->stride;
    const alder_match_t zero = { .query = 0, .ref = 0, .len = 0 };
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        *(alder_match_t *) (data + 1 * i * stride) = zero;
    }
}

#pragma mark copy
int
gsl_vector_match_memcpy (gsl_vector_match * dest,
                         const gsl_vector_match * src)
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
                dest->data[1 * dest_stride * j + k] = src->data[1 * src_stride * j + k];
            }
        }
    }
    
    return GSL_SUCCESS;
}

#pragma mark file
int
gsl_vector_match_fread (FILE * stream, gsl_vector_match * v)
{
    int status = gsl_block_match_raw_fread (stream,
                                            v->data,
                                            v->size,
                                            v->stride);
    return status;
}

int
gsl_vector_match_fwrite (FILE * stream, const gsl_vector_match * v)
{
    int status = gsl_block_match_raw_fwrite (stream,
                                             v->data,
                                             v->size,
                                             v->stride);
    return status;
}


int
gsl_vector_match_fprintf (FILE * stream, const gsl_vector_match * v,
                          const char *format)
{
    int status = gsl_block_match_raw_fprintf (stream,
                                              v->data,
                                              v->size,
                                              v->stride,
                                              format);
    return status;
}

int
gsl_vector_match_fscanf (FILE * stream, gsl_vector_match * v)
{
    int status = gsl_block_match_raw_fscanf (stream,
                                             v->data,
                                             v->size,
                                             v->stride);
    return status;
}

#pragma mark minmax
alder_match_t
gsl_vector_match_max (const gsl_vector_match * v)
{
    
    
    const size_t N = v->size ;
    const size_t stride = v->stride ;
    
    alder_match_t max = v->data[0 * stride];
    size_t i;
    
    for (i = 0; i < N; i++)
    {
        alder_match_t x = v->data[i*stride];
        if (x.ref > max.ref)
            max = x;
        
        
        
        
    }
    
    return max;
}

alder_match_t
gsl_vector_match_min (const gsl_vector_match * v)
{
    
    
    const size_t N = v->size ;
    const size_t stride = v->stride ;
    
    alder_match_t min = v->data[0 * stride];
    size_t i;
    
    for (i = 0; i < N; i++)
    {
        alder_match_t x = v->data[i*stride];
        if (x.ref < min.ref)
            min = x;
        
        
        
        
    }
    
    return min;
}

void
gsl_vector_match_minmax (const gsl_vector_match * v,
                         alder_match_t * min_out,
                         alder_match_t * max_out)
{
    
    
    const size_t N = v->size ;
    const size_t stride = v->stride ;
    
    alder_match_t max = v->data[0 * stride];
    alder_match_t min = v->data[0 * stride];
    
    size_t i;
    
    for (i = 0; i < N; i++)
    {
        alder_match_t x = v->data[i*stride];
        if (x.ref < min.ref)
        {
            min = x;
        }
        if (x.ref > max.ref)
        {
            max = x;
        }
    }
    
    *min_out = min;
    *max_out = max;
}


size_t
gsl_vector_match_max_index (const gsl_vector_match * v)
{
    
    
    const size_t N = v->size ;
    const size_t stride = v->stride ;
    
    alder_match_t max = v->data[0 * stride];
    size_t imax = 0;
    size_t i;
    
    for (i = 0; i < N; i++)
    {
        alder_match_t x = v->data[i*stride];
        if (x.ref > max.ref)
        {
            max = x;
            imax = i;
        }
        
        
        
        
        
        
    }
    
    return imax;
}

size_t
gsl_vector_match_min_index (const gsl_vector_match * v)
{
    
    
    const size_t N = v->size ;
    const size_t stride = v->stride ;
    
    alder_match_t min = v->data[0 * stride];
    size_t imin = 0;
    size_t i;
    
    for (i = 0; i < N; i++)
    {
        alder_match_t x = v->data[i*stride];
        if (x.ref < min.ref)
        {
            min = x;
            imin = i;
        }
        
        
        
        
        
        
    }
    
    return imin;
}


void
gsl_vector_match_minmax_index (const gsl_vector_match * v,
                               size_t * imin_out,
                               size_t * imax_out)
{
    
    
    const size_t N = v->size ;
    const size_t stride = v->stride ;
    
    size_t imin = 0, imax = 0;
    alder_match_t max = v->data[0 * stride];
    alder_match_t min = v->data[0 * stride];
    
    size_t i;
    
    for (i = 0; i < N; i++)
    {
        alder_match_t x = v->data[i*stride];
        if (x.ref < min.ref)
        {
            min = x;
            imin = i;
        }
        if (x.ref > max.ref)
        {
            max = x;
            imax = i;
        }
    }
    *imin_out = imin;
    *imax_out = imax;
}


#pragma mark subvector
_gsl_vector_match_const_view
gsl_vector_match_const_subvector (const gsl_vector_match * v, size_t offset, size_t n)
{
    _gsl_vector_match_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, view);
    }
    
    if (offset + (n - 1) >= v->size)
    {
        GSL_ERROR_VAL ("view would extend past end of vector",
                       GSL_EINVAL, view);
    }
    
    {
        gsl_vector_match s = {0, 0, 0, 0, 0};
        
        s.data = v->data + 1 * v->stride * offset ;
        s.size = n;
        s.stride = v->stride;
        s.block = v->block;
        s.owner = 0;
        
        view.vector = s;
        return view;
    }
}

_gsl_vector_match_const_view
gsl_vector_match_const_subvector_with_stride (const gsl_vector_match * v, size_t offset, size_t stride, size_t n)
{
    _gsl_vector_match_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, view);
    }
    
    if (stride == 0)
    {
        GSL_ERROR_VAL ("stride must be positive integer",
                       GSL_EINVAL, view);
    }
    
    if (offset + (n - 1) * stride >= v->size)
    {
        GSL_ERROR_VAL ("view would extend past end of vector",
                       GSL_EINVAL, view);
    }
    
    {
        gsl_vector_match s = {0, 0, 0, 0, 0};
        
        s.data = v->data + 1 * v->stride * offset ;
        s.size = n;
        s.stride = v->stride * stride;
        s.block = v->block;
        s.owner = 0;
        
        view.vector = s;
        return view;
    }
}

#pragma mark swap
int
gsl_vector_match_swap (gsl_vector_match * v, gsl_vector_match * w)
{
    alder_match_t * d1 = v->data ;
    alder_match_t * d2 = w->data ;
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
            alder_match_t tmp = d1[i*s1 + k];
            d1[i*s1+k] = d2[i*s2 + k];
            d2[i*s2+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_match_swap_elements (gsl_vector_match * v, const size_t i, const size_t j)
{
    alder_match_t * data = v->data ;
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
            alder_match_t tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_match_reverse (gsl_vector_match * v)
{
    alder_match_t * data = v->data ;
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
            alder_match_t tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

#pragma mark view
_gsl_vector_match_const_view
gsl_vector_match_const_view_array (const gsl_vector_match * base, size_t n)
{
    _gsl_vector_match_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, view);
    }
    
    {
        gsl_vector_match v = {0, 0, 0, 0, 0};
        
        v.data = (alder_match_t *)base ;
        v.size = n;
        v.stride = 1;
        v.block = 0;
        v.owner = 0;
        
        view.vector = v;
        return view;
    }
}

_gsl_vector_match_const_view
gsl_vector_match_const_view_array_with_stride (const gsl_vector_match * base,
                                               size_t stride,
                                               size_t n)
{
    _gsl_vector_match_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, view);
    }
    
    if (stride == 0)
    {
        GSL_ERROR_VAL ("stride must be positive integer",
                       GSL_EINVAL, view);
    }
    
    {
        gsl_vector_match v = {0, 0, 0, 0, 0};
        
        v.data = (alder_match_t *)base ;
        v.size = n;
        v.stride = stride;
        v.block = 0;
        v.owner = 0;
        
        view.vector = v;
        return view;
    }
}
