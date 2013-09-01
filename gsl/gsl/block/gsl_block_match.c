//
//  gsl_block_match.c
//  gsl
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "gsl_block_match.h"

#pragma mark init
gsl_block_match *
gsl_block_match_alloc (const size_t n)
{
    gsl_block_match * b;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("block length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    b = (gsl_block_match *) malloc (sizeof (gsl_block_match));
    
    if (b == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    
    b->data = (alder_match_t *) calloc (1, 1 * n * sizeof (alder_match_t));
    
    if (b->data == 0)
    {
        free (b);
        
        GSL_ERROR_VAL ("failed to allocate space for block data",
                       GSL_ENOMEM, 0);
    }
    
    b->size = n;
    
    return b;
}

gsl_block_match *
gsl_block_match_calloc (const size_t n)
{
    size_t i;
    
    gsl_block_match * b = gsl_block_match_alloc (n);
    
    if (b == 0)
        return 0;
    
    
    
    for (i = 0; i < 1 * n; i++)
    {
        b->data[i].query = 0;
        b->data[i].ref = 0;
        b->data[i].len = 0;
    }
    
    return b;
}

void
gsl_block_match_free (gsl_block_match * b)
{
    if (!b) { return ; };
    free (b->data);
    free (b);
}

gsl_block_match *
gsl_block_match_realloc (gsl_block_match *b, const size_t n)
{
    if (b == NULL) {
        GSL_ERROR_VAL ("b must have been allocated",
                       GSL_EINVAL, 0);
    }
    if (n < b->size) {
        GSL_ERROR_VAL ("b can only increase in size",
                       GSL_EINVAL, 0);
    }
    alder_match_t *t = realloc(b->data, n * sizeof(alder_match_t));
    if (t == NULL) {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    b->data = t;
    b->size = n;
    
    return b;
}


#pragma mark block
size_t
gsl_block_match_size (const gsl_block_match * b)
{
    return b->size ;
}

alder_match_t *
gsl_block_match_data (const gsl_block_match * b)
{
    return b->data ;
}


#pragma mark file

int
gsl_block_match_fread (FILE * stream, gsl_block_match * b)
{
    size_t n = b->size ;
    
    alder_match_t * data = b->data ;
    
    size_t items = fread (data, 1 * sizeof (alder_match_t), n, stream);
    
    if (items != n)
    {
        GSL_ERROR ("fread failed", GSL_EFAILED);
    }
    
    return 0;
}

int
gsl_block_match_fwrite (FILE * stream, const gsl_block_match * b)
{
    size_t n = b->size ;
    
    alder_match_t * data = b->data ;
    
    size_t items = fwrite (data, 1 * sizeof (alder_match_t), n, stream);
    
    if (items != n)
    {
        GSL_ERROR ("fwrite failed", GSL_EFAILED);
    }
    
    return 0;
}

int
gsl_block_match_raw_fread (FILE * stream, alder_match_t * data,
                           const size_t n, const size_t stride)
{
    if (stride == 1)
    {
        size_t items = fread (data, 1 * sizeof (alder_match_t), n, stream);
        
        if (items != n)
        {
            GSL_ERROR ("fread failed", GSL_EFAILED);
        }
    }
    else
    {
        size_t i;
        
        for (i = 0; i < n; i++)
        {
            size_t item = fread (data + 1 * i * stride,
                                 1 * sizeof (alder_match_t), 1, stream);
            if (item != 1)
            {
                GSL_ERROR ("fread failed", GSL_EFAILED);
            }
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_block_match_raw_fwrite (FILE * stream, const alder_match_t * data,
                            const size_t n, const size_t stride)
{
    
    if (stride == 1)
    {
        size_t items = fwrite (data, 1 * sizeof (alder_match_t), n, stream);
        
        if (items != n)
        {
            GSL_ERROR ("fwrite failed", GSL_EFAILED);
        }
    }
    else
    {
        size_t i;
        
        for (i = 0; i < n; i++)
        {
            size_t item = fwrite (data + 1 * i * stride,
                                  1 * sizeof (alder_match_t),
                                  1, stream);
            if (item != 1)
            {
                GSL_ERROR ("fwrite failed", GSL_EFAILED);
            }
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_block_match_fprintf (FILE * stream, const gsl_block_match * b, const char *format)
{
    size_t n = b->size ;
    
    alder_match_t * data = b->data ;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int k;
        int status;
        
        for (k = 0; k < 1; k++)
        {
            if (k > 0)
            {
                status = putc (' ', stream);
                
                if (status == (-1))
                {
                    GSL_ERROR ("putc failed", GSL_EFAILED);
                }
            }
            status = fprintf (stream,
                              "%lld\t%lld\t%lld",
                              data[1 * i + k].query,
                              data[1 * i + k].ref,
                              data[1 * i + k].len);
            if (status < 0)
            {
                GSL_ERROR ("fprintf failed", GSL_EFAILED);
            }
        }
        
        status = putc ('\n', stream);
        
        if (status == (-1))
        {
            GSL_ERROR ("putc failed", GSL_EFAILED);
        }
    }
    
    return 0;
}

int
gsl_block_match_fscanf (FILE * stream, gsl_block_match * b)
{
    size_t n = b->size ;
    
    alder_match_t * data = b->data ;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int k;
        for (k = 0; k < 1; k++)
        {
            alder_match_t tmp ;
            
            int status = fscanf (stream,
                                 "%lld\t%lld\t%lld\n",
                                 &tmp.query,
                                 &tmp.ref,
                                 &tmp.len);
            
            data [1 * i + k] = tmp;
            
            
            if (status != 3)
            {
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
            }
        }
    }
    
    return GSL_SUCCESS;
}


int
gsl_block_match_raw_fprintf (FILE * stream,
                             const alder_match_t * data,
                             const size_t n,
                             const size_t stride,
                             const char *format)
{
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int k;
        int status;
        
        for (k = 0; k < 1; k++)
        {
            if (k > 0)
            {
                status = putc (' ', stream);
                
                if (status == (-1))
                {
                    GSL_ERROR ("putc failed", GSL_EFAILED);
                }
            }
            status = fprintf (stream,
                              "%lld\t%lld\t%lld",
                              data[1 * i * stride + k].query,
                              data[1 * i * stride + k].ref,
                              data[1 * i * stride + k].len);
            if (status < 0)
            {
                GSL_ERROR ("fprintf failed", GSL_EFAILED);
            }
        }
        
        status = putc ('\n', stream);
        
        if (status == (-1))
        {
            GSL_ERROR ("putc failed", GSL_EFAILED);
        }
    }
    
    return 0;
}

int
gsl_block_match_raw_fscanf (FILE * stream,
                            alder_match_t * data,
                            const size_t n,
                            const size_t stride)
{
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int k;
        for (k = 0; k < 1; k++)
        {
            alder_match_t tmp;
            
            int status = fscanf (stream,
                                 "%lld\t%lld\t%lld\n",
                                 &tmp.query,
                                 &tmp.ref,
                                 &tmp.len);
            
            data [1 * i * stride + k] = tmp;
            
            if (status != 3)
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
        }
    }
    
    return GSL_SUCCESS;
}


