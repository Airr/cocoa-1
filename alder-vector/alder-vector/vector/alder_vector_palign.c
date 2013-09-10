/**
 * This file, alder_vector_palign.c, is part of alder-vector.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-vector is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-vector is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-vector.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_permutation.h>
#include "alder_vector_palign.h"

#pragma mark test

/** Edit this function.
 *
 */
void cmd_alder_vector_palign(int * a, size_t n)
{
    alder_vector_palign_t *v = alder_vector_palign_init(1);
    for (size_t i = 0; i < n/2; i++) {
        //        printf("[%zd] %d - %d\n", i, a[2*i], a[2*i+1]);
        alder_vector_palign_data_t m = { .s = a[2*i], .e = a[2*i+1] };
        alder_vector_palign_add(v, m);
    }
    alder_vector_palign_fprintf(stdout, v);
    alder_vector_palign_sort_vector_interval(v);
    printf("After sort\n");
    alder_vector_palign_fprintf(stdout, v);
    
    alder_vector_palign_free(v);
}

#pragma mark need-to-align

/**
 * This function tests whether the i-th palign needs any alignment.
 * Argument:
 * v - a palign vector
 * i - i-th element
 * Return:
 * 0 if there is no need to align.
 * 1 otherwise.
 */
int
alder_vector_palign_needToAlign (alder_vector_palign_t * palign, const size_t palignIndex)
{
    int status = 0;
    const size_t numberOfAnchor = palign->data[palignIndex].anchor->size;
    assert(numberOfAnchor != 0);
    
    if (numberOfAnchor == 1) {
        int anchor1 = (int)palign->data[palignIndex].qanchor->data[0].s;
        int anchor2 = (int)(palign->data[palignIndex].anchor->data[0].s - palign->data[palignIndex].s);
        int anchorLength = (int)(palign->data[palignIndex].qanchor->data[0].e - palign->data[palignIndex].qanchor->data[0].s);
        assert(palign->data[palignIndex].anchor->data[0].e - palign->data[palignIndex].anchor->data[0].s == anchorLength);
        if (anchor1 > 0 || anchor2 > 0 || anchorLength != blength(palign->data[palignIndex].query)) {
            status = 1;
        }
    } else if (numberOfAnchor > 1) {
        status = 1;
    }
    
    return status;
}

#pragma mark data

alder_vector_palign_data_t *
alder_vector_palign_data_t_alloc (const size_t n)
{
    alder_vector_palign_data_t *d =
      (alder_vector_palign_data_t *) calloc (1, 1 * n * sizeof (alder_vector_palign_data_t));
    for (size_t i = 0; i < n; i++) {
        // d[i].anchor = alder_vector_interval_init(1);
        d[i].qanchor = NULL;
        d[i].anchor = NULL;
        d[i].query = NULL;
        d[i].subject = NULL;
        d[i].s = 0;
        d[i].e = 0;
    }
    
    return d;
}

void
alder_vector_palign_data_t_free (alder_vector_palign_data_t * d)
{
    if (d->query != NULL) bdestroy(d->query);
    if (d->subject != NULL) bdestroy(d->subject);
    if (d->qanchor != NULL) alder_vector_interval_free(d->qanchor);
    if (d->anchor != NULL) alder_vector_interval_free(d->anchor);
}

#pragma mark block

/* This function allocates memory for a block of n elements of
 * alder_vector_palign_data_t, returning a pointer to the block struct.
 * The block is not initialized and so the values of its elements are
 * undefined. Use the function
 * alder_vector_palign_data_t_calloc if you want to ensure that all the
 * elements are initialized to zero.
 * Argument:
 * n - size of the elements
 * Return:
 * A point to the block struct if successful.
 * A null pointer is returned if insufficient memory is available to
 * create the block.
 */
alder_vector_palign_block_t *
alder_vector_palign_block_t_alloc (const size_t n)
{
    alder_vector_palign_block_t * b;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("block length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    b = (alder_vector_palign_block_t *) malloc (sizeof (alder_vector_palign_block_t));
    
    if (b == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    
    // b->data = (alder_vector_palign_data_t *) calloc (1, 1 * n * sizeof (alder_vector_palign_data_t));
    b->data = alder_vector_palign_data_t_alloc (n);
    
    if (b->data == 0)
    {
        free (b);
        GSL_ERROR_VAL ("failed to allocate space for block data",
                       GSL_ENOMEM, 0);
    }
    
    b->size = n;
    
    return b;
}

/*
 * Argument:
 * n - size of the elements
 * Return:
 * A point to the block struct if successful.
 * A null pointer is returned if insufficient memory is available to
 * create the block.
 */
alder_vector_palign_block_t *
alder_vector_palign_block_t_calloc (const size_t n)
{
    alder_vector_palign_block_t * b = alder_vector_palign_block_t_alloc (n);
    if (b == 0) return 0;
    memset(b->data, 0, n * sizeof(alder_vector_palign_data_t));
    return b;
}

/* This function frees the memory used by a block b previously allocated with
 * alder_vector_palign_block_t_alloc or
 * alder_vector_palign_block_t_calloc.
 * Argument:
 * b - a block
 */
void
alder_vector_palign_block_t_free (alder_vector_palign_block_t * b)
{
    if (!b) return;
    for (size_t i = 0; i < b->size; i++) {
        alder_vector_palign_data_t_free(b->data + i);
    }
    free (b->data);
    free (b);
}

/* This function returns the size of the block pointed by b.
 * Argument:
 * b - a block
 * Return:
 * the size of the block.
 */
size_t
alder_vector_palign_block_t_size (const alder_vector_palign_block_t * b)
{
    if (!b) return 0;
    return b->size;
}

/* This function returns the pointer to the data in the block.
 * Argument:
 * b - a block
 * Return:
 * the data of the block.
 */
alder_vector_palign_data_t *
alder_vector_palign_block_t_data (const alder_vector_palign_block_t * b)
{
    if (!b) return 0;
    return b->data;
}

/* The function increases the size of a block.
 * Argument:
 * b - a block
 * n - a new size (must be greater than or equal to the oringal size.
 * Return:
 * A point to the block struct if successful.
 * A null pointer is returned if insufficient memory is available to
 * increase the block.
 */
alder_vector_palign_block_t *
alder_vector_palign_block_t_realloc (alder_vector_palign_block_t *b,
                                       const size_t n)
{
    if (b == NULL) {
        GSL_ERROR_VAL ("b must have been allocated",
                       GSL_EINVAL, 0);
    }
    if (n < b->size) {
        GSL_ERROR_VAL ("b can only increase in size",
                       GSL_EINVAL, 0);
    }
    alder_vector_palign_data_t *t = realloc(b->data, n * sizeof (alder_vector_palign_data_t));
    if (t == NULL) {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    b->data = t;
    b->size = n;
    
    return b;
}

/** Edit this function.
 *
 */
/* This function prints the data out to stream.
 * Argument:
 * stream - output FILE stream
 * data - data
 * n - size
 * Return:
 * 0 if successful
 */
int
alder_vector_palign_raw_fprintf (FILE * stream,
                                   const alder_vector_palign_data_t * data,
                                   const size_t n)
{
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int status;
        
        const alder_vector_palign_data_t * t = data + i;
        
        fprintf(stream, "Palign - [%zd]\n", i);
        status = fprintf (stream,
                          "L-window: %lld - %lld\n",
                          t->s, t->e);
        fprintf(stream, "Query anchor\n");
        alder_vector_interval_fprintf(stream, t->qanchor);
        fprintf(stream, "Sbjct anchor\n");
        alder_vector_interval_fprintf(stream, t->anchor);
        
        status = fprintf (stream,
                          "Query  : %s\n"
                          "Subject: %s\n",
                          bdata(t->query), bdata(t->subject));
        
        if (status < 0)
        {
            GSL_ERROR ("fprintf failed", GSL_EFAILED);
        }
        
    }
    
    return 0;
}


#pragma mark vector

/* This function creates a vector of length n, returning a pointer to a newly
 * initialized vector struct. A new block is allocated for the elements of the
 * vector, and stored in the block component of the vector struct.
 * Argument:
 * n - size of the elements
 * Return:
 * A point to the vector struct if successful.
 * A null pointer is returned if insufficient memory is available to
 * create the vector.
 */
alder_vector_palign_t *
alder_vector_palign_alloc (const size_t n)
{
    alder_vector_palign_block_t * b;
    alder_vector_palign_t * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    v = (alder_vector_palign_t *) malloc (sizeof (alder_vector_palign_t));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    b = alder_vector_palign_block_t_alloc (n);;
    
    if (b == 0)
    {
        free (v) ;
        GSL_ERROR_VAL ("failed to allocate space for block",
                       GSL_ENOMEM, 0);
    }
    
    v->data = b->data ;
    v->size = n;
    v->block = b;
    
    return v;
    
}

/* This function frees a previously allocated vector v.
 * Argument:
 * v - a vector
 */
void
alder_vector_palign_free (alder_vector_palign_t * v)
{
    if (!v) return;
    alder_vector_palign_block_t_free(v->block);
    free (v);
}

/* This function initializes a vector of length n, returning a pointer to a
 * newly initialized vector struct. A new block is allocated for the elements
 * of the vector, and stored in the block component of the vector struct.
 * The size of the vector is set to zero while the size of the block is set
 * to the argument n. The size of block indicates the capacity of the vector.
 * Argument:
 * n - size of the elements
 * Return:
 * A point to the vector struct if successful.
 * A null pointer is returned if insufficient memory is available to
 * create the vector.
 */
alder_vector_palign_t *
alder_vector_palign_init (size_t n)
{
    alder_vector_palign_block_t * b;
    alder_vector_palign_t * v;
    
    v = (alder_vector_palign_t *) malloc (sizeof (alder_vector_palign_t));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    b = alder_vector_palign_block_t_alloc (n);;
    
    if (b == 0)
    {
        free (v) ;
        GSL_ERROR_VAL ("failed to allocate space for block",
                       GSL_ENOMEM, 0);
    }
    
    v->data = b->data;
    v->size = 0;
    v->block = b;
    
    return v;
}

/* This function inserts a new data to the vector. It automatically increases
 * the size of the vector if
 * Argument:
 * v - a vector
 * m - a data
 * Return:
 * 0 if succesfful.
 * not zero otherwise.
 */
int
alder_vector_palign_add (alder_vector_palign_t * v,
                           const alder_vector_palign_data_t m)
{
    if (v->block->size == v->size) {
        // Resize a->block
        alder_vector_palign_block_t *t =
        alder_vector_palign_block_t_realloc(v->block, 2 * v->block->size);
        if (t == NULL) {
            GSL_ERROR ("failed to allocate space for vector struct",
                       GSL_ENOMEM);
        }
        v->data = v->block->data;
    }
    v->data[v->size] = m;
    v->size++;
    
    return GSL_SUCCESS;
}

alder_vector_palign_data_t
alder_vector_palign_get (const alder_vector_palign_t * v, const size_t i)
{
    return v->data[i];
}

void
alder_vector_palign_set (alder_vector_palign_t * v,
                           const size_t i, alder_vector_palign_data_t x)
{
    v->data[i] = x;
}

alder_vector_palign_data_t *
alder_vector_palign_ptr (alder_vector_palign_t * v, const size_t i)
{
    return (alder_vector_palign_data_t *) (v->data + i);
}

const alder_vector_palign_data_t *
alder_vector_palign_const_ptr (const alder_vector_palign_t * v,
                                 const size_t i)
{
    return (const alder_vector_palign_data_t *) (v->data + i);
}

/* This function prints the data out to stream.
 * Argument:
 * stream - output FILE stream
 * v - a vector
 * Return:
 * 0 if successful
 */
int
alder_vector_palign_fprintf (FILE * stream,
                               const alder_vector_palign_t * v)
{
    int status = alder_vector_palign_raw_fprintf (stream,
                                                    v->data,
                                                    v->size);
    return status;
}

#pragma mark sort


static inline void my_interval_downheap (alder_vector_palign_data_t * data, const size_t stride, const size_t N, size_t k);

static inline void
my_interval_downheap (alder_vector_palign_data_t * data, const size_t stride, const size_t N, size_t k)
{
    alder_vector_palign_data_t v = data[k * stride];
    
    while (k <= N / 2)
    {
        size_t j = 2 * k;
        
        /* Edit this line. */
        if (j < N && data[j * stride].s < data[(j + 1) * stride].s)
        {
            j++;
        }
        
        if (!(v.s < data[j * stride].s))
        {
            break;
        }
        
        data[k * stride] = data[j * stride];
        
        k = j;
    }
    
    data[k * stride] = v;
}

void
alder_vector_palign_sort_interval (alder_vector_palign_data_t * data, const size_t stride, const size_t n)
{
    size_t N;
    size_t k;
    
    if (n == 0)
    {
        return;
    }
    
    
    
    
    N = n - 1;
    
    k = N / 2;
    k++;
    do
    {
        k--;
        my_interval_downheap (data, stride, N, k);
    }
    while (k > 0);
    
    while (N > 0)
    {
        
        alder_vector_palign_data_t tmp = data[0 * stride];
        data[0 * stride] = data[N * stride];
        data[N * stride] = tmp;
        
        
        N--;
        
        my_interval_downheap (data, stride, N, 0);
    }
}

void
alder_vector_palign_sort_vector_interval (alder_vector_palign_t * v)
{
    alder_vector_palign_sort_interval (v->data, 1, v->size) ;
}

static inline void index_interval_downheap (size_t * p, const alder_vector_palign_data_t * data, const size_t stride, const size_t N, size_t k);

static inline void
index_interval_downheap (size_t * p, const alder_vector_palign_data_t * data, const size_t stride, const size_t N, size_t k)
{
    const size_t pki = p[k];
    
    while (k <= N / 2)
    {
        size_t j = 2 * k;
        
        /* Edit this line. */
        if (j < N && data[p[j] * stride].s < data[p[j + 1] * stride].s)
        {
            j++;
        }
        
        /* Edit this line. */
        if (!(data[pki * stride].s < data[p[j] * stride].s))
        {
            break;
        }
        
        p[k] = p[j];
        
        k = j;
    }
    
    p[k] = pki;
}

void
alder_vector_palign_sort_interval_index (size_t * p, const alder_vector_palign_data_t * data, const size_t stride, const size_t n)
{
    size_t N;
    size_t i, k;
    
    if (n == 0)
    {
        return;
    }
    
    
    
    for (i = 0 ; i < n ; i++)
    {
        p[i] = i ;
    }
    
    
    
    
    N = n - 1;
    
    k = N / 2;
    k++;
    do
    {
        k--;
        index_interval_downheap (p, data, stride, N, k);
    }
    while (k > 0);
    
    while (N > 0)
    {
        
        size_t tmp = p[0];
        p[0] = p[N];
        p[N] = tmp;
        
        
        N--;
        
        index_interval_downheap (p, data, stride, N, 0);
    }
}

int
alder_vector_palign_sort_vector_interval_index (gsl_permutation * permutation, const alder_vector_palign_t * v)
{
    if (permutation->size != v->size)
    {
        GSL_ERROR ("permutation and vector lengths are not equal", GSL_EBADLEN);
    }
    
    alder_vector_palign_sort_interval_index (permutation->data, v->data, 1, v->size) ;
    
    return GSL_SUCCESS ;
}

