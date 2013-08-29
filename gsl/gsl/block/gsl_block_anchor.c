//
//  gsl_block_anchor.c
//  gsl
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "gsl_block_anchor.h"

gsl_block_anchor *
gsl_block_anchor_alloc (const size_t n)
{
    gsl_block_anchor * b;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("block length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    b = (gsl_block_anchor *) malloc (sizeof (gsl_block_anchor));
    
    if (b == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    
    b->data = (alder_anchor_t *) calloc (1, 1 * n * sizeof (alder_anchor_t));
    
    if (b->data == 0)
    {
        free (b);
        
        GSL_ERROR_VAL ("failed to allocate space for block data",
                       GSL_ENOMEM, 0);
    }
    
    b->size = n;
    
    return b;
}

gsl_block_anchor *
gsl_block_anchor_calloc (const size_t n)
{
    gsl_block_anchor * b = gsl_block_anchor_alloc (n);
    
    if (b == 0)
        return 0;
    
    
    
    memset(b->data, 0, n*sizeof(alder_anchor_t));
    
    return b;
}

void
gsl_block_anchor_free (gsl_block_anchor * b)
{
    if (!b) { return ; };
    free (b->data);
    free (b);
}

gsl_block_anchor *
gsl_block_anchor_realloc (gsl_block_anchor *b, const size_t n)
{
    if (b == NULL) {
        GSL_ERROR_VAL ("b must have been allocated",
                       GSL_EINVAL, 0);
    }
    if (n < b->size) {
        GSL_ERROR_VAL ("b can only increase in size",
                       GSL_EINVAL, 0);
    }
    alder_anchor_t *t = realloc(b->data, n * sizeof(alder_anchor_t));
    if (t == NULL) {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    b->data = t;
    b->size = n;
    
    return b;
}


size_t
gsl_block_anchor_size (const gsl_block_anchor * b)
{
    return b->size ;
}

alder_anchor_t *
gsl_block_anchor_data (const gsl_block_anchor * b)
{
    return b->data ;
}

int
gsl_block_anchor_fread (FILE * stream, gsl_block_anchor * b)
{
    size_t n = b->size ;
    
    alder_anchor_t * data = b->data ;
    
    size_t items = fread (data, 1 * sizeof (alder_anchor_t), n, stream);
    
    if (items != n)
    {
        GSL_ERROR ("fread failed", GSL_EFAILED);
    }
    
    return 0;
}

int
gsl_block_anchor_fwrite (FILE * stream, const gsl_block_anchor * b)
{
    size_t n = b->size ;
    
    alder_anchor_t * data = b->data ;
    
    size_t items = fwrite (data, 1 * sizeof (alder_anchor_t), n, stream);
    
    if (items != n)
    {
        GSL_ERROR ("fwrite failed", GSL_EFAILED);
    }
    
    return 0;
}

int
gsl_block_anchor_raw_fread (FILE * stream, alder_anchor_t * data,
                            const size_t n, const size_t stride)
{
    if (stride == 1)
    {
        size_t items = fread (data, 1 * sizeof (alder_anchor_t), n, stream);
        
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
                                 1 * sizeof (alder_anchor_t), 1, stream);
            if (item != 1)
            {
                GSL_ERROR ("fread failed", GSL_EFAILED);
            }
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_block_anchor_raw_fwrite (FILE * stream, const alder_anchor_t * data,
                             const size_t n, const size_t stride)
{
    
    if (stride == 1)
    {
        size_t items = fwrite (data, 1 * sizeof (alder_anchor_t), n, stream);
        
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
                                  1 * sizeof (alder_anchor_t),
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
gsl_block_anchor_fprintf (FILE * stream, const gsl_block_anchor * b, const char *format)
{
    size_t n = b->size ;
    
    alder_anchor_t * data = b->data ;
    
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
                
                if (status == EOF)
                {
                    GSL_ERROR ("putc failed", GSL_EFAILED);
                }
            }
            const alder_anchor_t * t = data + 1*i + k;
            status = fprintf (stream,
                              "%d\t%d\t%d\t%lf",
                              t->anchor1, t->anchor2, t->anchorLength,
                              t->anchorScore);
            if (status < 0)
            {
                GSL_ERROR ("fprintf failed", GSL_EFAILED);
            }
        }
        
        status = putc ('\n', stream);
        
        if (status == EOF)
        {
            GSL_ERROR ("putc failed", GSL_EFAILED);
        }
    }
    
    return 0;
}

int
gsl_block_anchor_fscanf (FILE * stream, gsl_block_anchor * b)
{
    size_t n = b->size ;
    
    alder_anchor_t * data = b->data ;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int k;
        for (k = 0; k < 1; k++)
        {
            alder_anchor_t tmp ;
            
            int status = fscanf (stream, "%d\t%d\t%d\t%lf\n",
                                 &tmp.anchor1,
                                 &tmp.anchor2,
                                 &tmp.anchorLength,
                                 &tmp.anchorScore) ;
            
            data [1 * i + k] = tmp;
            
            
            if (status != 4)
            {
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
            }
        }
    }
    
    return GSL_SUCCESS;
}


int
gsl_block_anchor_raw_fprintf (FILE * stream,
                              const alder_anchor_t * data,
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
                
                if (status == EOF)
                {
                    GSL_ERROR ("putc failed", GSL_EFAILED);
                }
            }
            const alder_anchor_t * t = data + 1*i*stride + k;
            status = fprintf (stream,
                              "%d\t%d\t%d\t%lf",
                              t->anchor1, t->anchor2, t->anchorLength,
                              t->anchorScore);
            if (status < 0)
            {
                GSL_ERROR ("fprintf failed", GSL_EFAILED);
            }
        }
        
        status = putc ('\n', stream);
        
        if (status == EOF)
        {
            GSL_ERROR ("putc failed", GSL_EFAILED);
        }
    }
    
    return 0;
}

int
gsl_block_anchor_raw_fscanf (FILE * stream,
                             alder_anchor_t * data,
                             const size_t n,
                             const size_t stride)
{
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int k;
        for (k = 0; k < 1; k++)
        {
            alder_anchor_t tmp;
            
            int status = fscanf (stream, "%d\t%d\t%d\t%lf\n",
                                 &tmp.anchor1,
                                 &tmp.anchor2,
                                 &tmp.anchorLength,
                                 &tmp.anchorScore) ;
            
            data [1 * i * stride + k] = tmp;
            
            if (status != 4)
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
        }
    }
    
    return GSL_SUCCESS;
}
