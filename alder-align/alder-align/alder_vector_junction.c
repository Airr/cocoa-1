//
//  alder_vector_junction.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

// In the following order of the source files,
// vector/init.c
// vector/view.c
// vector/subvector.c
// vector/file.c
// vector/copy.c
// vector/swap.c
// vector/minmax.c
// vector/oper.c
// vector/prop.c

#include <stdio.h>
#include "alder_block_junction.h"
#include "alder_vector_junction.h"

alder_vector_junction *
alder_vector_junction_alloc (const size_t n)
{
    alder_block_junction * block;
    alder_vector_junction * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer", GSL_EINVAL, 0);
    }
    
    v = (alder_vector_junction *) malloc (sizeof (alder_vector_junction));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct", GSL_ENOMEM, 0);

    }
    
    block = alder_block_junction_alloc (n);
    
    if (block == 0)
    {
        free (v) ;
        GSL_ERROR_VAL ("failed to allocate space for block struct", GSL_ENOMEM, 0);        
    }
    
    v->data = block->data ;
    v->size = n;
    v->stride = 1;
    v->block = block;
    v->owner = 1;
    
    return v;
}

alder_vector_junction *
alder_vector_junction_calloc (const size_t n)
{
    size_t i;
    
    alder_vector_junction * v = alder_vector_junction_alloc (n);
    
    if (v == 0)
        return 0;
    
    
    
    for (i = 0; i < 1 * n; i++)
    {
        v->data[i] = (alder_junction){
            .start = 0LLU,
            .gap   = 0LU,
            .strand= 0,
            .motif = 0,
            .annot = 0,
            .countUnique = 0LU,
            .countMultiple = 0LLU,
            .overhangLeft = 0U,
            .overhangRight = 0U
        };
    }
    
    return v;
}

alder_vector_junction *
alder_vector_junction_alloc_from_block (alder_block_junction * block,
                                      const size_t offset,
                                      const size_t n,
                                      const size_t stride)
{
    alder_vector_junction * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer", GSL_EINVAL, 0);
    }
    
    if (stride == 0)
    {
        GSL_ERROR_VAL ("stride must be positive integer", GSL_EINVAL, 0);
    }
    
    if (block->size <= offset + (n - 1) * stride)
    {
        GSL_ERROR_VAL ("vector would extend past end of block", GSL_EINVAL, 0);
    }
    
    v = (alder_vector_junction *) malloc (sizeof (alder_vector_junction));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct", GSL_ENOMEM, 0);        
    }
    
    v->data = block->data + 1 * offset ;
    v->size = n;
    v->stride = stride;
    v->block = block;
    v->owner = 0;
    
    return v;
}

alder_vector_junction *
alder_vector_junction_alloc_from_vector (alder_vector_junction * w,
                                       const size_t offset,
                                       const size_t n,
                                       const size_t stride)
{
    alder_vector_junction * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer", GSL_EINVAL, 0);
    }
    
    if (stride == 0)
    {
        GSL_ERROR_VAL ("stride must be positive integer", GSL_EINVAL, 0);
    }
    
    if (offset + (n - 1) * stride >= w->size)
    {
        GSL_ERROR_VAL ("vector would extend past end of block", GSL_EINVAL, 0);
    }
    
    v = (alder_vector_junction *) malloc (sizeof (alder_vector_junction));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct", GSL_ENOMEM, 0);
    }
    
    v->data = w->data + 1 * w->stride * offset ;
    v->size = n;
    v->stride = stride * w->stride;
    v->block = w->block;
    v->owner = 0;
 
    return v;
}


void
alder_vector_junction_free (alder_vector_junction * v)
{
    if (!v) { return ; };
    
    if (v->owner)
    {
        alder_block_junction_free (v->block) ;
    }
    free (v);
}

void
alder_vector_junction_set_all (alder_vector_junction * v, alder_junction x)
{
    alder_junction * const data = v->data;
    const size_t n = v->size;
    const size_t stride = v->stride;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        *(alder_junction *) (data + 1 * i * stride) = x;
    }
}

void
alder_vector_junction_set_zero (alder_vector_junction * v)
{
    alder_junction * const data = v->data;
    const size_t n = v->size;
    const size_t stride = v->stride;
    const alder_junction zero = (alder_junction){
        .start = 0LLU,
        .gap   = 0LU,
        .strand= 0,
        .motif = 0,
        .annot = 0,
        .countUnique = 0LU,
        .countMultiple = 0LLU,
        .overhangLeft = 0U,
        .overhangRight = 0U
    };
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        *(alder_junction *) (data + 1 * i * stride) = zero;
    }
}


_alder_vector_junction_const_view
alder_vector_junction_const_view_array (const alder_junction * base, size_t n)
{
    _alder_vector_junction_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer", GSL_EINVAL, view);
    }
    
    {
        alder_vector_junction v = {0, 0, 0, 0, 0};
        
        v.data = (alder_junction *)base ;
        v.size = n;
        v.stride = 1;
        v.block = 0;
        v.owner = 0;
        
        view.vector = v;
        return view;
    }
}

_alder_vector_junction_const_view
alder_vector_junction_const_view_array_with_stride (const alder_junction * base,
                                                  size_t stride,
                                                  size_t n)
{
    _alder_vector_junction_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer", GSL_EINVAL, view);
    }
    
    if (stride == 0)
    {
        GSL_ERROR_VAL ("stride must be positive integer", GSL_EINVAL, view);
    }
    
    {
        alder_vector_junction v = {0, 0, 0, 0, 0};
        
        v.data = (alder_junction *)base ;
        v.size = n;
        v.stride = stride;
        v.block = 0;
        v.owner = 0;
        
        view.vector = v;
        return view;
    }
}

_alder_vector_junction_const_view
alder_vector_junction_const_subvector (const alder_vector_junction * v, size_t offset, size_t n)
{
    _alder_vector_junction_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer", GSL_EINVAL, view);
    }
    
    if (offset + (n - 1) >= v->size)
    {
        GSL_ERROR_VAL ("view would extend past end of vector", GSL_EINVAL, view);
    }
    
    {
        alder_vector_junction s = {0, 0, 0, 0, 0};
        
        s.data = v->data + 1 * v->stride * offset ;
        s.size = n;
        s.stride = v->stride;
        s.block = v->block;
        s.owner = 0;
        
        view.vector = s;
        return view;
    }
}

_alder_vector_junction_const_view
alder_vector_junction_const_subvector_with_stride (const alder_vector_junction * v, size_t offset, size_t stride, size_t n)
{
    _alder_vector_junction_const_view view = {{0, 0, 0, 0, 0}};
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer", GSL_EINVAL, view);
    }
    
    if (stride == 0)
    {
        GSL_ERROR_VAL ("stride must be positive integer", GSL_EINVAL, view);
    }
    
    if (offset + (n - 1) * stride >= v->size)
    {
        GSL_ERROR_VAL ("vector would extend past end of block", GSL_EINVAL, view);
    }
    
    {
        alder_vector_junction s = {0, 0, 0, 0, 0};
        
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
alder_vector_junction_fread (FILE * stream, alder_vector_junction * v)
{
    int status = alder_block_junction_raw_fread (stream,
                                               v->data,
                                               v->size,
                                               v->stride);
    return status;
}

int
alder_vector_junction_fwrite (FILE * stream, const alder_vector_junction * v)
{
    int status = alder_block_junction_raw_fwrite (stream,
                                                v->data,
                                                v->size,
                                                v->stride);
    return status;
}


int
alder_vector_junction_fprintf (FILE * stream, const alder_vector_junction * v,
                             const char *format)
{
    int status = alder_block_junction_raw_fprintf (stream,
                                                 v->data,
                                                 v->size,
                                                 v->stride,
                                                 format);
    return status;
}

int
alder_vector_junction_fscanf (FILE * stream, alder_vector_junction * v)
{
    int status = alder_block_junction_raw_fscanf (stream,
                                                v->data,
                                                v->size,
                                                v->stride);
    return status;
}

int
alder_vector_junction_memcpy (alder_vector_junction * dest,
                            const alder_vector_junction * src)
{
    const size_t src_size = src->size;
    const size_t dest_size = dest->size;
    
    if (src_size != dest_size)
    {
        do { gsl_error ("vector lengths are not equal", "../../vector/copy_source.c", 29, GSL_EBADLEN) ; return GSL_EBADLEN ; } while (0);
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
alder_vector_junction_swap (alder_vector_junction * v, alder_vector_junction * w)
{
    alder_junction * d1 = v->data ;
    alder_junction * d2 = w->data ;
    const size_t size = v->size ;
    const size_t s1 = 1 * v->stride ;
    const size_t s2 = 1 * w->stride ;
    size_t i, k ;
    
    if (v->size != w->size)
    {
        do { gsl_error ("vector lengths must be equal", "../../vector/swap_source.c", 32, GSL_EINVAL) ; return GSL_EINVAL ; } while (0);
    }
    
    for (i = 0; i < size; i++)
    {
        for (k = 0; k < 1; k++)
        {
            alder_junction tmp = d1[i*s1 + k];
            d1[i*s1+k] = d2[i*s2 + k];
            d2[i*s2+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
alder_vector_junction_swap_elements (alder_vector_junction * v, const size_t i, const size_t j)
{
    alder_junction * data = v->data ;
    const size_t size = v->size ;
    const size_t stride = v->stride ;
    
    if (i >= size)
    {
        do { gsl_error ("first index is out of range", "../../vector/swap_source.c", 57, GSL_EINVAL) ; return GSL_EINVAL ; } while (0);
    }
    
    if (j >= size)
    {
        do { gsl_error ("second index is out of range", "../../vector/swap_source.c", 62, GSL_EINVAL) ; return GSL_EINVAL ; } while (0);
    }
    
    if (i != j)
    {
        const size_t s = 1 * stride ;
        size_t k ;
        
        for (k = 0; k < 1; k++)
        {
            alder_junction tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

int
alder_vector_junction_reverse (alder_vector_junction * v)
{
    alder_junction * data = v->data ;
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
            alder_junction tmp = data[j*s + k];
            data[j*s+k] = data[i*s + k];
            data[i*s+k] = tmp;
        }
    }
    
    return GSL_SUCCESS;
}

//alder_junction
//alder_vector_junction_max (const alder_vector_junction * v)
//{
//    
//    
//    const size_t N = v->size ;
//    const size_t stride = v->stride ;
//    
//    alder_junction max = v->data[0 * stride];
//    size_t i;
//    
//    for (i = 0; i < N; i++)
//    {
//        alder_junction x = v->data[i*stride];
//        if (x > max)
//            max = x;
//        
//        
//        
//        
//    }
//    
//    return max;
//}

//alder_junction
//alder_vector_junction_min (const alder_vector_junction * v)
//{
//    
//    
//    const size_t N = v->size ;
//    const size_t stride = v->stride ;
//    
//    alder_junction min = v->data[0 * stride];
//    size_t i;
//    
//    for (i = 0; i < N; i++)
//    {
//        alder_junction x = v->data[i*stride];
//        if (x < min)
//            min = x;
//        
//        
//        
//        
//    }
//    
//    return min;
//}

//void
//alder_vector_junction_minmax (const alder_vector_junction * v,
//                            alder_junction * min_out,
//                            alder_junction * max_out)
//{
//    
//    
//    const size_t N = v->size ;
//    const size_t stride = v->stride ;
//    
//    alder_junction max = v->data[0 * stride];
//    alder_junction min = v->data[0 * stride];
//    
//    size_t i;
//    
//    for (i = 0; i < N; i++)
//    {
//        alder_junction x = v->data[i*stride];
//        if (x < min)
//        {
//            min = x;
//        }
//        if (x > max)
//        {
//            max = x;
//        }
//# 104 "../../vector/minmax_source.c"
//    }
//    
//    *min_out = min;
//    *max_out = max;
//}

//size_t
//alder_vector_junction_max_index (const alder_vector_junction * v)
//{
//    
//    
//    const size_t N = v->size ;
//    const size_t stride = v->stride ;
//    
//    alder_junction max = v->data[0 * stride];
//    size_t imax = 0;
//    size_t i;
//    
//    for (i = 0; i < N; i++)
//    {
//        alder_junction x = v->data[i*stride];
//        if (x > max)
//        {
//            max = x;
//            imax = i;
//        }
//        
//        
//        
//        
//        
//        
//    }
//    
//    return imax;
//}

//size_t
//alder_vector_junction_min_index (const alder_vector_junction * v)
//{
//    
//    
//    const size_t N = v->size ;
//    const size_t stride = v->stride ;
//    
//    alder_junction min = v->data[0 * stride];
//    size_t imin = 0;
//    size_t i;
//    
//    for (i = 0; i < N; i++)
//    {
//        alder_junction x = v->data[i*stride];
//        if (x < min)
//        {
//            min = x;
//            imin = i;
//        }
//        
//        
//        
//        
//        
//        
//    }
//    
//    return imin;
//}

//void
//alder_vector_junction_minmax_index (const alder_vector_junction * v,
//                                  size_t * imin_out,
//                                  size_t * imax_out)
//{
//    
//    
//    const size_t N = v->size ;
//    const size_t stride = v->stride ;
//    
//    size_t imin = 0, imax = 0;
//    alder_junction max = v->data[0 * stride];
//    alder_junction min = v->data[0 * stride];
//    
//    size_t i;
//    
//    for (i = 0; i < N; i++)
//    {
//        alder_junction x = v->data[i*stride];
//        if (x < min)
//        {
//            min = x;
//            imin = i;
//        }
//        if (x > max)
//        {
//            max = x;
//            imax = i;
//        }
//# 211 "../../vector/minmax_source.c"
//    }
//    
//    *imin_out = imin;
//    *imax_out = imax;
//}

//int
//alder_vector_junction_add (alder_vector_junction * a, const alder_vector_junction * b)
//{
//    const size_t N = a->size;
//    
//    if (b->size != N)
//    {
//        do { gsl_error ("vectors must have same length", "../../vector/oper_source.c", 27, GSL_EBADLEN) ; return GSL_EBADLEN ; } while (0);
//    }
//    else
//    {
//        const size_t stride_a = a->stride;
//        const size_t stride_b = b->stride;
//        
//        size_t i;
//        
//        for (i = 0; i < N; i++)
//        {
//            a->data[i * stride_a] += b->data[i * stride_b];
//        }
//        
//        return GSL_SUCCESS;
//    }
//}
//
//int
//alder_vector_junction_sub (alder_vector_junction * a, const alder_vector_junction * b)
//{
//    const size_t N = a->size;
//    
//    if (b->size != N)
//    {
//        do { gsl_error ("vectors must have same length", "../../vector/oper_source.c", 52, GSL_EBADLEN) ; return GSL_EBADLEN ; } while (0);
//    }
//    else
//    {
//        const size_t stride_a = a->stride;
//        const size_t stride_b = b->stride;
//        
//        size_t i;
//        
//        for (i = 0; i < N; i++)
//        {
//            a->data[i * stride_a] -= b->data[i * stride_b];
//        }
//        
//        return GSL_SUCCESS;
//    }
//}
//
//int
//alder_vector_junction_mul (alder_vector_junction * a, const alder_vector_junction * b)
//{
//    const size_t N = a->size;
//    
//    if (b->size != N)
//    {
//        do { gsl_error ("vectors must have same length", "../../vector/oper_source.c", 77, GSL_EBADLEN) ; return GSL_EBADLEN ; } while (0);
//    }
//    else
//    {
//        const size_t stride_a = a->stride;
//        const size_t stride_b = b->stride;
//        
//        size_t i;
//        
//        for (i = 0; i < N; i++)
//        {
//            a->data[i * stride_a] *= b->data[i * stride_b];
//        }
//        
//        return GSL_SUCCESS;
//    }
//}
//
//
//int
//alder_vector_junction_div (alder_vector_junction * a, const alder_vector_junction * b)
//{
//    const size_t N = a->size;
//    
//    if (b->size != N)
//    {
//        do { gsl_error ("vectors must have same length", "../../vector/oper_source.c", 102, GSL_EBADLEN) ; return GSL_EBADLEN ; } while (0);
//    }
//    else
//    {
//        const size_t stride_a = a->stride;
//        const size_t stride_b = b->stride;
//        
//        size_t i;
//        
//        for (i = 0; i < N; i++)
//        {
//            a->data[i * stride_a] /= b->data[i * stride_b];
//        }
//        
//        return GSL_SUCCESS;
//    }
//}
//
//int
//alder_vector_junction_scale (alder_vector_junction * a, const double x)
//{
//    const size_t N = a->size;
//    const size_t stride = a->stride;
//    
//    size_t i;
//    
//    for (i = 0; i < N; i++)
//    {
//        a->data[i * stride] *= x;
//    }
//    
//    return GSL_SUCCESS;
//}
//
//int
//alder_vector_junction_add_constant (alder_vector_junction * a, const double x)
//{
//    const size_t N = a->size;
//    const size_t stride = a->stride;
//    
//    size_t i;
//    
//    for (i = 0; i < N; i++)
//    {
//        a->data[i * stride] += x;
//    }
//    
//    return GSL_SUCCESS;
//}
//
//
//int
//alder_vector_junction_equal (const alder_vector_junction * u, const alder_vector_junction * v)
//{
//    const size_t n = v->size;
//    const size_t stride_u = u->stride ;
//    const size_t stride_v = v->stride ;
//    
//    size_t j;
//    
//    if (u->size != v->size)
//    {
//        do { gsl_error ("vectors must have same length", "../../vector/prop_source.c", 32, GSL_EBADLEN) ; return 0 ; } while (0);
//    }
//    
//    for (j = 0; j < n; j++)
//    {
//        size_t k;
//        
//        for (k = 0; k < 1; k++)
//        {
//            if (u->data[1 * stride_u * j + k] != v->data[1 * stride_v * j + k])
//            {
//                return 0;
//            }
//        }
//    }
//    
//    return 1;
//}
//
//int
//alder_vector_junction_isnull (const alder_vector_junction * v)
//{
//    const size_t n = v->size;
//    const size_t stride = v->stride ;
//    
//    size_t j;
//    
//    for (j = 0; j < n; j++)
//    {
//        size_t k;
//        
//        for (k = 0; k < 1; k++)
//        {
//            if (v->data[1 * stride * j + k] != 0.0)
//            {
//                return 0;
//            }
//        }
//    }
//    
//    return 1;
//}
//int
//alder_vector_junction_ispos (const alder_vector_junction * v)
//{
//    const size_t n = v->size;
//    const size_t stride = v->stride ;
//    
//    size_t j;
//    
//    for (j = 0; j < n; j++)
//    {
//        size_t k;
//        
//        for (k = 0; k < 1; k++)
//        {
//            if (v->data[1 * stride * j + k] <= 0.0)
//            {
//                return 0;
//            }
//        }
//    }
//    
//    return 1;
//}
//
//int
//alder_vector_junction_isneg (const alder_vector_junction * v)
//{
//    const size_t n = v->size;
//    const size_t stride = v->stride ;
//    
//    size_t j;
//    
//    for (j = 0; j < n; j++)
//    {
//        size_t k;
//        
//        for (k = 0; k < 1; k++)
//        {
//            if (v->data[1 * stride * j + k] >= 0.0)
//            {
//                return 0;
//            }
//        }
//    }
//    
//    return 1;
//}
//
//int
//alder_vector_junction_isnonneg (const alder_vector_junction * v)
//{
//    const size_t n = v->size;
//    const size_t stride = v->stride ;
//    
//    size_t j;
//    
//    for (j = 0; j < n; j++)
//    {
//        size_t k;
//        
//        for (k = 0; k < 1; k++)
//        {
//            if (v->data[1 * stride * j + k] < 0.0)
//            {
//                return 0;
//            }
//        }
//    }
//    
//    return 1;
//}




