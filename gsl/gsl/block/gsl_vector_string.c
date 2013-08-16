//
//  gsl_vector_string.c
//  gsl
//
//  Created by Sang Chul Choi on 8/10/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "gsl_vector_string.h"

gsl_vector_string *
gsl_vector_string_alloc (const size_t n)
{
    gsl_block_string * block;
    gsl_vector_string * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    v = (gsl_vector_string *) malloc (sizeof (gsl_vector_string));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    block = gsl_block_string_alloc (n);
    
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

gsl_vector_string *
gsl_vector_string_calloc (const size_t n)
{
    size_t i;
    
    gsl_vector_string * v = gsl_vector_string_alloc (n);
    
    if (v == 0)
        return 0;
    
    
    
    for (i = 0; i < 1 * n; i++)
    {
        v->data[i] = 0;
    }
    
    return v;
}

gsl_vector_string *
gsl_vector_string_alloc_from_block (gsl_block_string * block,
                                    const size_t offset,
                                    const size_t n,
                                    const size_t stride)
{
    gsl_vector_string * v;
    
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
    
    v = (gsl_vector_string *) malloc (sizeof (gsl_vector_string));
    
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

gsl_vector_string *
gsl_vector_string_alloc_from_vector (gsl_vector_string * w,
                                     const size_t offset,
                                     const size_t n,
                                     const size_t stride)
{
    gsl_vector_string * v;
    
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
    
    v = (gsl_vector_string *) malloc (sizeof (gsl_vector_string));
    
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
gsl_vector_string_free (gsl_vector_string * v)
{
    if (!v) { return ; };
    
    if (v->owner)
    {
        gsl_block_string_free (v->block) ;
    }
    free (v);
}


void
gsl_vector_string_set_all (gsl_vector_string * v, string x)
{
    string * const data = v->data;
    const size_t n = v->size;
    const size_t stride = v->stride;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        if (data[i * stride] != NULL) {
            free(data[i * stride]);
        }
        data[i * stride] = strdup(x);
    }
}

void
gsl_vector_string_set_zero (gsl_vector_string * v)
{
    string * const data = v->data;
    const size_t n = v->size;
    const size_t stride = v->stride;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        if (data[i * stride] != NULL) {
            free(data[i * stride]);
        }
        data[i * stride] = NULL;
    }
}

_gsl_vector_string_const_view
gsl_vector_string_const_view_array (const string * base, size_t n)
{
    _gsl_vector_string_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, view);
    }
    
    {
        gsl_vector_string v = {0, 0, 0, 0, 0};
        
        v.data = (string *)base ;
        v.size = n;
        v.stride = 1;
        v.block = 0;
        v.owner = 0;
        
        view.vector = v;
        return view;
    }
}

_gsl_vector_string_const_view
gsl_vector_string_const_view_array_with_stride (const string * base,
                                                size_t stride,
                                                size_t n)
{
    _gsl_vector_string_const_view view = {{0, 0, 0, 0, 0}};
    
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
        gsl_vector_string v = {0, 0, 0, 0, 0};
        
        v.data = (string *)base ;
        v.size = n;
        v.stride = stride;
        v.block = 0;
        v.owner = 0;
        
        view.vector = v;
        return view;
    }
}


_gsl_vector_string_const_view
gsl_vector_string_const_subvector (const gsl_vector_string * v, size_t offset, size_t n)
{
    _gsl_vector_string_const_view view = {{0, 0, 0, 0, 0}};
    
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
        gsl_vector_string s = {0, 0, 0, 0, 0};
        
        s.data = v->data + 1 * v->stride * offset ;
        s.size = n;
        s.stride = v->stride;
        s.block = v->block;
        s.owner = 0;
        
        view.vector = s;
        return view;
    }
}

_gsl_vector_string_const_view
gsl_vector_string_const_subvector_with_stride (const gsl_vector_string * v, size_t offset, size_t stride, size_t n)
{
    _gsl_vector_string_const_view view = {{0, 0, 0, 0, 0}};
    
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
        gsl_vector_string s = {0, 0, 0, 0, 0};
        
        s.data = v->data + 1 * v->stride * offset ;
        s.size = n;
        s.stride = v->stride * stride;
        s.block = v->block;
        s.owner = 0;
        
        view.vector = s;
        return view;
    }
}



int
gsl_vector_string_fread (FILE * stream, gsl_vector_string * v)
{
    int status = gsl_block_string_raw_fread (stream,
                                             v->data,
                                             v->size,
                                             v->stride);
    return status;
}

int
gsl_vector_string_fwrite (FILE * stream, const gsl_vector_string * v)
{
    int status = gsl_block_string_raw_fwrite (stream,
                                              v->data,
                                              v->size,
                                              v->stride);
    return status;
}


int
gsl_vector_string_fprintf (FILE * stream, const gsl_vector_string * v,
                           const char *format)
{
    int status = gsl_block_string_raw_fprintf (stream,
                                               v->data,
                                               v->size,
                                               v->stride,
                                               format);
    return status;
}

int
gsl_vector_string_fscanf (FILE * stream, gsl_vector_string * v)
{
    int status = gsl_block_string_raw_fscanf (stream,
                                              v->data,
                                              v->size,
                                              v->stride);
    return status;
}

int
gsl_vector_string_memcpy (gsl_vector_string * dest,
                          const gsl_vector_string * src)
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
            if (dest->data[dest_stride * j] != NULL)
            {
                free(dest->data[dest_stride * j]);
                dest->data[dest_stride * j] = NULL;
            }
            if (src->data[src_stride * j] != NULL)
            {
                dest->data[dest_stride * j] = strdup(src->data[dest_stride * j]);
            }
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_string_swap (gsl_vector_string * v, gsl_vector_string * w)
{
    string * d1 = v->data ;
    string * d2 = w->data ;
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
            string tmp = d1[i*s1 + k];
            d1[i*s1+k] = d2[i*s2 + k];
            d2[i*s2+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_string_swap_elements (gsl_vector_string * v, const size_t i, const size_t j)
{
    string * data = v->data ;
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
            string tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_string_reverse (gsl_vector_string * v)
{
    string * data = v->data ;
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
            string tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_vector_string_add_string (gsl_vector_string * v, const string b)
{
    int status;
    status = gsl_block_string_add_string(v->block, b);
    if (status == GSL_SUCCESS)
    {
        v->size = v->block->size;
    }
    else
    {
        GSL_ERROR("adding string failed in vector", GSL_ENOMEM);
    }
    return status;
}

int
gsl_vector_string_equal (const gsl_vector_string * u, const gsl_vector_string * v)
{
    const size_t n = v->size;
    const size_t stride_u = u->stride ;
    const size_t stride_v = v->stride ;
    
    size_t j;
    
    if (u->size != v->size)
    {
        GSL_ERROR_VAL ("vectors must have same length", GSL_EBADLEN, 0);
    }
    
    for (j = 0; j < n; j++)
    {
        size_t k;
        
        for (k = 0; k < 1; k++)
        {
            if (strcmp(u->data[1 * stride_u * j + k], v->data[1 * stride_v * j + k]))
            {
                return 0;
            }
        }
    }
    
    return 1;
}

int
gsl_vector_string_isnull (const gsl_vector_string * v)
{
    const size_t n = v->size;
    const size_t stride = v->stride ;
    
    size_t j;
    
    for (j = 0; j < n; j++)
    {
        size_t k;
        
        for (k = 0; k < 1; k++)
        {
            if (v->data[1 * stride * j + k] != NULL)
            {
                return 0;
            }
        }
    }
    
    return 1;
}

