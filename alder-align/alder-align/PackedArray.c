//
//  PackedArray.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/8/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_errno.h>
#include "PackedArray.h"
#include "alder_align_macro.h"

#ifndef MULTIPLICITY
#define MULTIPLICITY 1
#endif

#ifndef ATOMIC
#define ATOMIC uint8_t
#endif

#ifndef IN_FORMAT
#define IN_FORMAT "%llu"
#endif

alder_block_suffix_array *alder_block_suffix_array_alloc (const uint64_t n, const uint64_t stride)
{
    alder_block_suffix_array * b;
    if (n == 0)
    {
        GSL_ERROR_VAL ("block length n must be positive integer", GSL_EINVAL, 0);
    }
    b = (alder_block_suffix_array *) malloc (sizeof (alder_block_suffix_array));
    if (b == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for block struct", GSL_ENOMEM, 0);
    }
    b->wordLength = stride;
    b->wordCompLength = sizeof(uint64_t) * 8LLU - stride;
    b->bitRecMask = (~0LLU)>>(b->wordCompLength);
    b->length = n;
    b->lengthByte = n * stride / 8LLU + 1LLU;
    b->data = (uint8_t *) calloc (1, MULTIPLICITY * b->lengthByte * sizeof (uint8_t));
    if (b->data == 0)
    {
        free (b);        
        GSL_ERROR_VAL ("failed to allocate space for block data", GSL_ENOMEM, 0);
    }
    
    b->size = n; // should this be lengthByte?
    
    return b;
}

alder_block_suffix_array *alder_block_suffix_array_calloc (const uint64_t n, const uint64_t stride)
{
    uint64_t i;
    alder_block_suffix_array * b = alder_block_suffix_array_alloc (n, stride);
    if (b == 0)
    {
        return 0;
    }
    for (i = 0; i < b->lengthByte; i++)
    {
        b->data[i] = 0;
    }
    return b;
}

void alder_block_suffix_array_free (alder_block_suffix_array * b)
{
    RETURN_IF_NULL (b);
    free (b->data);
    free (b);
}

int alder_block_suffix_array_fread (FILE * stream, alder_block_suffix_array * b)
{
    uint64_t n = b->lengthByte;
    
    ATOMIC * data = b->data;
    
    uint64_t items = (uint64_t) fread (data, MULTIPLICITY * sizeof (ATOMIC), n, stream);
    
    if (items != n)
    {
        GSL_ERROR ("fread failed", GSL_EFAILED);
    }
    
    return 0;
}

int alder_block_suffix_array_fwrite (FILE * stream, const alder_block_suffix_array * b)
{
    size_t n = b->lengthByte ;
    
    ATOMIC * data = b->data ;
    
    size_t items = fwrite (data, MULTIPLICITY * sizeof (ATOMIC), n, stream);
    
    if (items != n)
    {
        GSL_ERROR ("fwrite failed", GSL_EFAILED);
    }
    
    return 0;
}

int alder_block_suffix_array_fscanf (FILE * stream, alder_block_suffix_array * b)
{
    size_t n = b->size;
    
    ATOMIC * data = b->data;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int k;
        for (k = 0; k < MULTIPLICITY; k++)
        {
            uint64_t tmp ;
            
            int status = fscanf (stream, IN_FORMAT, &tmp) ;
            
            uint64_t B = (i * b->wordLength)/8LLU;
            uint64_t S = (i * b->wordLength)%8LLU;
            tmp <<= S;
            uint64_t *a = (uint64_t*) (data + B);
            *a = ( (*a) & ~(b->bitRecMask<<S) ) | tmp;
            
//            data [MULTIPLICITY * i + k] = tmp;
            
            if (status != 1)
            {
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
            }
        }
    }
    
    return GSL_SUCCESS;
}

int alder_block_suffix_array_fprintf (FILE * stream, const alder_block_suffix_array * b, const char *format)
{
    size_t n = b->size ;
    
    ATOMIC * data = b->data ;
    
    size_t i;
    
    for (i = 0; i < n; i++)
    {
        int k;
        int status;
        
        for (k = 0; k < MULTIPLICITY; k++)
        {
            if (k > 0)
            {
                status = putc (' ', stream);
                
                if (status == EOF)
                {
                    GSL_ERROR ("putc failed", GSL_EFAILED);
                }
            }
            
            // Decode the suffix array.
            uint64_t B = (i * b->wordLength)/8LLU;
            uint64_t S = (i * b->wordLength)%8LLU;
            uint64_t a = *((uint64_t*) (data + B));
            a = ((a>>S) << b->wordCompLength) >> b->wordCompLength;
            status = fprintf (stream,
                              format,
                              a);
//                              data[MULTIPLICITY * i + k]);
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

int alder_block_suffix_array_raw_fscanf (FILE * stream, uint8_t * data, const uint64_t n, const uint64_t stride)
{
    size_t i;
    uint64_t wordLength = stride;
    uint64_t wordCompLength = sizeof(uint64_t)*8LLU - wordLength;
    uint64_t bitRecMask = (~0LLU)>>wordCompLength;
    
    for (i = 0; i < n; i++)
    {
        int k;
        for (k = 0; k < MULTIPLICITY; k++)
        {
            uint64_t tmp;
            
            int status = fscanf (stream, IN_FORMAT, &tmp) ;
            uint64_t B = (i * wordLength)/8LLU;
            uint64_t S = (i * wordLength)%8LLU;
            tmp <<= S;
            uint64_t *a = (uint64_t*) (data + B);
            *a = ( (*a) & ~(bitRecMask<<S) ) | tmp;
            
            //            data [MULTIPLICITY * i + k] = tmp;
           
            
//            data [MULTIPLICITY * i * stride + k] = tmp;
            
            if (status != 1)
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
        }
    }
    
    return GSL_SUCCESS;
}

int alder_block_suffix_array_raw_fprintf (FILE * stream, const uint8_t * data, const uint64_t n, const uint64_t stride, const char *format)
{
    size_t i;
    uint64_t wordLength = stride;
    uint64_t wordCompLength = sizeof(uint64_t)*8LLU - wordLength;
    
    for (i = 0; i < n; i++)
    {
        int k;
        int status;
        
        for (k = 0; k < MULTIPLICITY; k++)
        {
            if (k > 0)
            {
                status = putc (' ', stream);
                
                if (status == EOF)
                {
                    GSL_ERROR ("putc failed", GSL_EFAILED);
                }
            }
            
            // Decode the suffix array.
            uint64_t B = (i * wordLength)/8LLU;
            uint64_t S = (i * wordLength)%8LLU;
            uint64_t a = *((uint64_t*) (data + B));
            a = ((a>>S) << wordCompLength) >> wordCompLength;
            status = fprintf (stream,
                              format,
                              a);
//            data[MULTIPLICITY * i * stride + k]);
            
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

uint64_t alder_block_suffix_array_size (const alder_block_suffix_array * b)
{
    return b->size;
}

uint8_t * alder_block_suffix_array_data (const alder_block_suffix_array * b)
{
      return b->data;
}
