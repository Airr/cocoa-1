/**
 * This file, gsl_vector_sam.c, is part of alder-gsl.
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

#include <stdio.h>
#include "bstrlib.h"
#include "gsl_vector_sam.h"

gsl_block_sam *
gsl_block_sam_alloc (const size_t n)
{
    gsl_block_sam * b;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("block length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    b = (gsl_block_sam *) malloc (sizeof (gsl_block_sam));
    
    if (b == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    
    b->data = (alder_sam_t *) calloc (1, 1 * n * sizeof (alder_sam_t));
    
    if (b->data == 0)
    {
        free (b);
        
        GSL_ERROR_VAL ("failed to allocate space for block data",
                       GSL_ENOMEM, 0);
    }
    
    b->size = n;
    
    return b;
}

void
gsl_block_sam_free (gsl_block_sam * b)
{
    if (!b) {
        return;
    }
    for (size_t i = 0; i < b->size; i++) {
        alder_sam_t *t = b->data + i;
        if (t->cigar != NULL) bcstrfree(t->cigar);
        if (t->optional != NULL) bcstrfree(t->optional);
        if (t->qname != NULL) bcstrfree(t->qname);
        if (t->qual != NULL) bcstrfree(t->qual);
        if (t->rname != NULL) bcstrfree(t->rname);
        if (t->rnext != NULL) bcstrfree(t->rnext);
        if (t->seq != NULL) bcstrfree(t->seq);
    }
    free (b->data);
    free (b);
}

size_t
gsl_block_sam_size (const gsl_block_sam * b)
{
    return b->size ;
}

alder_sam_t *
gsl_block_sam_data (const gsl_block_sam * b)
{
    return b->data ;
}

int
gsl_block_sam_fprintf (FILE * stream, const gsl_block_sam * b, const char *format)
{
    size_t n = b->size ;
    
    alder_sam_t * data = b->data ;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int status;
        alder_sam_t *t = data + i;
        status = fprintf (stream,
                          "%s\t%d\t%s\t%d\t%d\t%s\t%s\t%d\t%d\t%s\t%s\t%s",
                          t->qname,
                          t->flag,
                          t->rname,
                          t->pos,
                          t->mapq,
                          t->cigar,
                          t->rnext,
                          t->pnext,
                          t->tlen,
                          t->seq,
                          t->qual,
                          t->optional);
        if (status < 0)
        {
            GSL_ERROR ("fprintf failed", GSL_EFAILED);
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
gsl_block_sam_raw_fprintf (FILE * stream,
                           const alder_sam_t * data,
                           const size_t n,
                           const size_t stride,
                           const char *format)
{
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int status;
        
        const alder_sam_t *t = data + i;
        status = fprintf (stream,
                          "%s\t%d\t%s\t%d\t%d\t%s\t%s\t%d\t%d\t%s\t%s\t%s",
                          t->qname,
                          t->flag,
                          t->rname,
                          t->pos,
                          t->mapq,
                          t->cigar,
                          t->rnext,
                          t->pnext,
                          t->tlen,
                          t->seq,
                          t->qual,
                          t->optional);
        if (status < 0)
        {
            GSL_ERROR ("fprintf failed", GSL_EFAILED);
        }
        
        status = putc ('\n', stream);
        
        if (status == EOF)
        {
            GSL_ERROR ("putc failed", GSL_EFAILED);
        }
    }
    
    return 0;
}


#pragma mark vector


gsl_vector_sam *
gsl_vector_sam_alloc (const size_t n)
{
    gsl_block_sam * block;
    gsl_vector_sam * v;
    
    if (n == 0)
    {
        GSL_ERROR_VAL ("vector length n must be positive integer",
                       GSL_EINVAL, 0);
    }
    
    v = (gsl_vector_sam *) malloc (sizeof (gsl_vector_sam));
    
    if (v == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for vector struct",
                       GSL_ENOMEM, 0);
    }
    
    block = gsl_block_sam_alloc (n);
    
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


void
gsl_vector_sam_free (gsl_vector_sam * v)
{
    if (!v) return;
    
    if (v->owner)
    {
        gsl_block_sam_free (v->block) ;
    }
    free (v);
}

int
gsl_vector_sam_fprintf (FILE * stream, const gsl_vector_sam * v,
                        const char *format)
{
    int status = gsl_block_sam_raw_fprintf (stream,
                                            v->data,
                                            v->size,
                                            v->stride,
                                            format);
    return status;
}

