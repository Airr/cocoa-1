//
//  PackedArray.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/8/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_PackedArray_h
#define alder_align_PackedArray_h

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

#include <stdint.h>

__BEGIN_DECLS

struct alder_block_suffix_array_struct
{
    uint64_t size;
    uint8_t *data;
    
    uint64_t bitRecMask;
    uint64_t wordCompLength;
    uint64_t wordLength;
    uint64_t length;
    uint64_t lengthByte;
//    char* charArray;
};

typedef struct alder_block_suffix_array_struct alder_block_suffix_array;

// We can save a number of elements between 0 and 2^stride.
// n: number of elements
// m: size of each element
alder_block_suffix_array *alder_block_suffix_array_alloc (const uint64_t n, const uint64_t stride);
alder_block_suffix_array *alder_block_suffix_array_calloc (const uint64_t n, const uint64_t stride);
void alder_block_suffix_array_free (alder_block_suffix_array * b);

int alder_block_suffix_array_fread (FILE * stream, alder_block_suffix_array * b);
int alder_block_suffix_array_fwrite (FILE * stream, const alder_block_suffix_array * b);
int alder_block_suffix_array_fscanf (FILE * stream, alder_block_suffix_array * b);
int alder_block_suffix_array_fprintf (FILE * stream, const alder_block_suffix_array * b, const char *format);

//int alder_block_suffix_array_raw_fread (FILE * stream, uint8_t * data, const uint64_t n, const uint64_t stride);
//int alder_block_suffix_array_raw_fwrite (FILE * stream, const uint8_t * data, const uint64_t n, const uint64_t stride);
int alder_block_suffix_array_raw_fscanf (FILE * stream, uint8_t * data, const uint64_t n, const uint64_t stride);
int alder_block_suffix_array_raw_fprintf (FILE * stream, const uint8_t * data, const uint64_t n, const uint64_t stride, const char *format);

uint64_t alder_block_suffix_array_size (const alder_block_suffix_array * b);
uint8_t * alder_block_suffix_array_data (const alder_block_suffix_array * b);


__END_DECLS

#endif
