/**
 * This file, alder_hashtable_mcas3.h, is part of alder-kmer.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-kmer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kmer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_kmer_alder_hashtable_mcas3_h
#define alder_kmer_alder_hashtable_mcas3_h

#include <stdint.h>
#include "libdivide.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

/**
 *  This structure implements a chain-based hashtable.  It has two arrays; 
 *  main array is used as roots of chains, and aux array is used for non-root
 *  items. Sometimes, an element in the main array can be used for a non-root
 *  element. 
 *  An element contains the following items:
 *  1. key
 *  2. value/timestamp
 *  3. prev/next
 *  4. left/right
 * 
 *  The main memory is allocated by uint64. I am not sure whether this is
 *  necessary.
 *  
 *  Operations:
 *  1. Add a key to the table.
 *  2. Increment the count of an existing key.
 *  3. Delete an old key.
 *  4. Update frequency tables (frequency and frequencyL).
 *  5. Update pointer, pointer_head, pointer_tail.
 */
struct alder_hashtable_mcas3_struct {
    /* fixed */
    int k;                      /* k-mer size                                */
    int size_key;               /* size of key                               */
    int pos_value;              /* position of value in a key                */
    int pos_timestamp;          /* position of timestamp in a key            */
    int pos_pointerN;           /* position of prev pointer in a key         */
    int pos_pointerP;           /* position of next pointer in a key         */
    int pos_pointerL;           /* position of left pointer in a key         */
    int pos_pointerR;           /* position of right pointer in a key        */
    int size_auxiliary;         /* stride - size_key                         */
    int stride;                 /* stride of an element (uint64)             */
    uint64_t size_frequency;    /* size of frequency                         */
    uint64_t size;              /* number of elements in the hash table      */
    uint64_t size5;             /* number of elements for min_freq_value     */
    
    
    uint64_t min_value;         /* min occurence                             */
    uint64_t max_value;         /* max occurence                             */
    uint64_t min_freq_value;    /* [C] min occurence by freq table           */
    
    uint64_t timestamp;         /* [C] time stamp                            */
    
    /* data */
    uint64_t pointer;           /* [C] pointer to the current slot           */
    uint64_t pointer_head;      /* [C] pointer to head slot                  */
    uint64_t pointer_tail;      /* [C] pointer to tail slot                  */
    uint64_t *empty_key;        /* n: stride - EMPTY key                     */
    uint64_t *frequency;        /* [C] 2^16: frequency of count              */
    uint64_t *frequencyL;       /* [C] 2^16: frequency of count on the left  */
    uint64_t *key;              /* [C] n: size x stride                      */
                                /* key, value, offset, left/right, timestamp */
    
    FILE *fpd;                  /* File for deleted keys: -deleted.kmc       */
    FILE *fps;                  /* File for saved keys: -saved.kmc           */
    struct libdivide_u64_t fast_size;
};

typedef struct alder_hashtable_mcas3_struct alder_hashtable_mcas3_t;



__END_DECLS


#endif /* alder_kmer_alder_hashtable_mcas3_h */
