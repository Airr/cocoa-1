/**
 * This file, alder_hashtable_mcas2.c, is part of alder-kmer.
 *
 * Copyright 2013, 2014 by Sang Chul Choi
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

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "libdivide.h"
#include "alder_hash.h"
#include "alder_rng.h"
#include "alder_mcas_wf.h"
#include "alder_encode.h"
#include "libdivide.h"
#include "alder_hashtable_mcas2.h"

#define ALDER_HASHTABLE_FULL 2
#define V64_NULL             0x7FFFFFFFFFFFFFFF
#define V32_NULL             0x000000007FFFFFFF
#define V32_L_NULL           0x000000007FFFFFFF
#define V32_U_NULL           0x7FFFFFFF00000000
#define V32_POINTER_L        0x7FFFFFFF00000000
#define V32_POINTER_R        0x000000007FFFFFFF
#define V64_U32_MASK0        0x00000000FFFFFFFF
#define V64_L32_MASK0        0xFFFFFFFF00000000
#define V32_OFFSET_MASK      0xFFFFFFFF00000000
#define V32_VALUE_MASK       0x00000000FFFFFFFF
#define V16_VALUE_MASK       0x000000000000FFFF


#pragma mark version2

/**
 *  The version 2 of the hash table features deletion of old key with small
 *  values. This would allow me to count most frequent kmers. Two questions
 *  arise: 1. how to delete a key in a open-addressing hash table, and 2. how
 *  to choose a key to delete. I would like to delete a key that has a small
 *  value, and one that had been added long before. I devised an
 *  open-addressing hash table by having the following fields in a table slot:
 *  Field: [key, value, offset, left, right, and timestamp].
 *  I admit that this would increase the space requirement of a slot. The 
 *  algorithm starts by 
 *  1. keeping adding keys till the table is 70% full, (stage 1)
 *  2. adding a new key or update the existent key, and delete an "old" key 
 *     with "small" value. (stage 2)
 *  This algorithm keeps the usage of a table contant or 70% full because
 *  I add one and delete other. Then, a natural question is which key should
 *  be deleted; 1. a key with a small value, and 2. the key must be old.
 *  I keep a frequency table to decide the minimum value. To do this, I choose
 *  a value where the cumulative count reaches 21%. First, I thought that I
 *  could only use timestamp to choose a key to delete. I find it easier to
 *  construct double-linked list that connects slots in a hash table. The
 *  left/right values store the slot positions to the left and right. 
 *
 *  It is a little more difficult to understand the structure at the moment.
 *  I have three pointers; head, tail, and middle. The middle one is used to
 *  choose a slot to delete. Because the minimum value could change, I need
 *  to decide to move the middle pointer to the left or right by using
 *  a second frequency table. This second frequency table stores the count
 *  value frequency to the left from the middle pointer inclusively.
 *
 *  After stage 1, and before stage 2:
 *  1. The frequencyL must be all zero.
 *  2. pointer and pointer_tail are the same.
 *
 *  Expand this explanation.
 */

/**
 *  This function returns the size of a stride of uint64_t. 
 *  The size of a table can be
 *  ALDER_HASHTABLE_SIZE31 0x7FFFFFFF 2147483647
 *  If the size is larger than this number, referencing a slot is possible
 *  by increasing the size of reference; I increment something or stride. 
 *  The stride is the number of uint64_t; this is the size of a slot in unit
 *  of uint64. 
 *
 *  @param K    kmer size
 *  @param size table size
 *
 *  @return stride
 */
static int
alder_hashtable_mcas2_stride(int K, size_t size)
{
    int size_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    int stride32 = (size_key + 3) * 8;
    int stride64 = (size_key + 4) * 8;
    
    int stride = 0;
    if (size <= ALDER_HASHTABLE_SIZE31) {
        // The size of a hash table can be addressed using 31-bit value.
        stride = stride32;
    } else {
        // If v64 is larger than v32, I use v64 to use a larger memory.
        stride = stride64;
    }
    return stride;
}


/**
 *  This function initializes the hash table.
 *
 *  @param o    table
 *  @param k    kmer size
 *  @param size number of elements in the table
 *
 */
static void
alder_hashtable_mcas2_init(alder_hashtable_mcas2_t *o, int k, size_t size,
                           int stride,
                           int min_value, int max_value)
{
    o->k = k;
    o->size_key = ALDER_BYTESIZE_KMER(k,ALDER_NUMKMER_8BYTE);
    o->pos_value = o->size_key;
    if (size <= ALDER_HASHTABLE_SIZE31) {
        o->pos_pointerL = o->pos_value + 1;
        o->pos_pointerR = o->pos_value + 1;
    } else {
        o->pos_pointerL = o->pos_value + 1;
        o->pos_pointerR = o->pos_value + 2;
    }
    o->pos_timestamp = stride - 1;
    o->size_auxiliary = stride - o->size_key;
    o->stride = stride;
    o->size_frequency = ALDER_HASHTABLE_SIZE16;
    o->size = size;
    o->size7 = (size_t)((double)o->size * ALDER_HASHTABLE_LOAD);
    o->size5 = (uint64_t)((double)o->size7 * ALDER_HASHTABLE_LOAD);
    o->n_key = 0;
    o->min_value = (uint64_t)min_value;
    o->max_value = (uint64_t)max_value;
    o->min_freq_value = 1;
    o->timestamp = 0;
    o->pointer = UINT64_MAX;
    if (size <= ALDER_HASHTABLE_SIZE31) {
        o->pointer_head = V32_NULL;
        o->pointer_tail = V32_NULL;
    } else {
        o->pointer_head = V64_NULL;
        o->pointer_tail = V64_NULL;
    }
    o->empty_key = NULL;
    o->frequency = NULL;
    o->frequencyL = NULL;
    o->key = NULL;
    o->fpd = NULL;
    o->fps = NULL;
    o->fast_size = libdivide_u64_gen((uint64_t)size);
}

/**
 *  This function computes the memmory size of a table (version2) 
 *  given its size.
 *
 *  @param K    kmer size
 *  @param size number of elements
 *
 *  @return memmory size for a table
 */
size_t
alder_hashtable_mcas2_sizeof(int K, size_t size)
{
    size_t v = 0;
    int stride = alder_hashtable_mcas2_stride(K, size);
    alder_hashtable_mcas2_t ov;
    alder_hashtable_mcas2_init(&ov, K, size, stride, 0, 0);
    v = (sizeof(ov) +
         sizeof(*ov.empty_key) * ov.stride +
         sizeof(*ov.frequency) * ov.size_frequency +
         sizeof(*ov.frequencyL) * ov.size_frequency +
         sizeof(*ov.key) * ov.stride * size);
    return v;
}

/**
 *  This function computes the number of elements in a table that would fit to
 *  the given memory size. It also determines the stride size. The stride is in
 *  unit of uint64_t or 8-byte.
 *
 *  @param K        kmer size
 *  @param mem_size memory size in byte
 *
 *  @return SUCCESS or FAIL
 *
 *  Element in the key:
 *  [n-uint64] + [uint64:offset,value] + [uint64:left,right] + [uint64:timestamp]
 *  If the number of size is less than uint32, then uint32 can be used to
 *  locate left and right slot. The size should be decided based on available
 *  memory. I am not sure whether offset can be larger than 30-bit value.
 *  I hope that that is not the case with 70%-usage of the table.
 *  size_key is the number of uint64 using kmer size.
 *  offset is 30-bit size value.
 *  value is 32-bit value.
 *  left,right can be upto 31-bit because I may need 1-bit for MCAS.
 *  timestamp is 63-bit value.
 *  So, by assuming that the size of a table is less than or equal to 
 *  31-bit value, I can compute the size. If the size is larger than 31-bit
 *  value, then I need to use larger left and right pointers.
 *  [n-uint64] + [uint64:offset,value] + [2-uint64:left,right] + [uint64:timestamp]
 *  This would increase 8-byte more for each element. So, small increase
 *  in memory actually decreases the size of a hash table.
 */
int
alder_hashtable_mcas2_size_with_available_memory(size_t *size_table,
                                                 int *size_stride,
                                                 int K, size_t mem_size)
{
    int size_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    int stride32 = size_key + 3;
    int stride64 = size_key + 4;
    size_t v32 = mem_size / (stride32 * sizeof(uint64_t));
    size_t v64 = mem_size / (stride64 * sizeof(uint64_t));
    int stride = 0;
    size_t v = 0;
    if (v32 <= ALDER_HASHTABLE_SIZE31) {
        // The size of a hash table can be addressed using 31-bit value.
        stride = stride32;
        v = v32;
    } else {
        if (v32 < v64) {
            // If v64 is larger than v32, I use v64 to use a larger memory.
            stride = stride64;
            v = v64;
        } else {
            // Otherwise, I do not need to use v64, and revert back to v32.
            stride = stride32;
            v = ALDER_HASHTABLE_SIZE31;
        }
    }
    *size_table = v;
    *size_stride = stride;
    return ALDER_STATUS_SUCCESS;
}


/**
 *  This function computes the number of bytes for storing an element in a
 *  table given the size of memory in byte.
 *
 *  @param K        kmer size
 *  @param size_mem memmory in byte
 *
 *  @return element size in byte
 */
size_t
alder_hashtable_mcas2_element_sizeof_with_available_memory(int K,
                                                           size_t size_mem)
{
    size_t size_table = 0;
    int size_stride = 0;
    alder_hashtable_mcas2_size_with_available_memory(&size_table,
                                                     &size_stride,
                                                     K, size_mem);
    alder_hashtable_mcas2_t ov;
    size_t v = (sizeof(*ov.key) * size_stride);
    return v;
}

/**
 *  This function computes the number of bytes for stroing an element in a 
 *  table given the size of a table.
 *
 *  @param K          kemr size
 *  @param size_table size of a table
 *
 *  @return element size in byte
 */
size_t
alder_hashtable_mcas2_element_sizeof_with_size_table(int K,
                                                     size_t size_table)
{

    int size_stride =  alder_hashtable_mcas2_stride(K, size_table);
    alder_hashtable_mcas2_t ov;
    size_t v = (sizeof(*ov.key) * size_stride);
    return v;
}

/**
 *  This function resets the hash table. All of the keys are empty, and values
 *  are zero.
 *
 *  @param o hash table
 *
 *  @return SUCESS
 */
int
alder_hashtable_mcas2_reset(alder_hashtable_mcas2_t *o)
{
    size_t size_auxiliary = o->stride - o->size_key;
    
    memset(o->empty_key, 0x44, sizeof(*o->empty_key) * o->stride);
    for (size_t i = 0; i < o->size; i++) {
        size_t pos = i * o->stride;
        memset(o->key + pos, 0x44, sizeof(*o->key) * o->size_key);
        memset(o->key + pos + o->size_key, 0, sizeof(*o->key) * size_auxiliary);
    }
    memset(o->frequency, 0, sizeof(*o->frequency) * o->size_frequency);
    memset(o->frequencyL, 0, sizeof(*o->frequencyL) * o->size_frequency);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function frees the memory allocated to the table.
 *
 *  @param o table
 */
void
alder_hashtable_mcas2_destroy(alder_hashtable_mcas2_t *o)
{
    if (o != NULL) {
        XFCLOSE(o->fpd);
        XFCLOSE(o->fps);
        XFREE(o->empty_key);
        XFREE(o->frequency);
        XFREE(o->frequencyL);
        XFREE(o->key);
        XFREE(o);
    }
}

/**
 *  This function creates a table of version 2.
 *
 *  @param k    kmer size
 *  @param size number of elements in a table
 *
 *  @return table
 */
alder_hashtable_mcas2_t*
alder_hashtable_mcas2_create(int k, size_t size, int stride,
                             int min_value, int max_value,
                             int save_all_flag,
                             const char *outdir,
                             const char *outfile)
{
    alder_hashtable_mcas2_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    alder_hashtable_mcas2_init(o, k, size, stride, min_value, max_value);
    
    /* memory */
    o->empty_key = malloc(sizeof(*o->empty_key) * o->stride);
    o->frequency = malloc(sizeof(*o->frequency) * o->size_frequency);
    o->frequencyL = malloc(sizeof(*o->frequencyL) * o->size_frequency);
    o->key = malloc(sizeof(*o->key) * o->size * o->stride);   /* bigmem */
    if (o->empty_key == NULL ||
        o->frequency == NULL ||
        o->frequencyL == NULL ||
        o->key == NULL) {
        alder_hashtable_mcas2_destroy(o);
        return NULL;
    }
    alder_hashtable_mcas2_reset(o);
    
    /* file */
    if (save_all_flag) {
        bstring bfnd = bformat("%s/%s-deleted.kmc", outdir, outfile);
        o->fpd = fopen(bdata(bfnd), "wb");
        bdestroy(bfnd);
        
        bstring bfns = bformat("%s/%s-saved.kmc", outdir, outfile);
        o->fps = fopen(bdata(bfns), "wb");
        bdestroy(bfns);
    }
    
    return o;
}

/**
 *  This function
 *
 *  @param o              table
 *  @param x_p            position of a key in the table
 *  @param tries_p        number of tries or offset
 *  @param isKeyFound_p   YES if the key is found
 *  @param isEmptyFound_p YES if an empty key is found
 *  @param key            query key
 *
 *  @return pos of the key (in byte)
 */
static uint64_t
alder_hashtable_mcas2_find_key(alder_hashtable_mcas2_t *o,
                               uint64_t *x_p, uint64_t *tries_p,
                               int *isKeyFound_p, int *isEmptyFound_p,
                               uint64_t *key)
{
    /* 1. Find the key or an empty key: pos is the position in the key. */
    uint64_t pos=0;
    while (*isEmptyFound_p == ALDER_NO && *isKeyFound_p == ALDER_NO) {
        pos = *x_p * o->stride;
        *isEmptyFound_p = ALDER_YES;
        for (int i = 0; i < o->size_key; i++) {
            if (o->key[pos + i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                *isEmptyFound_p = ALDER_NO;
                break;
            }
        }
        if (*isEmptyFound_p == ALDER_NO) {
            *isKeyFound_p = ALDER_YES;
            for (int i = 0; i < o->size_key; i++) {
                if (o->key[pos + i] != key[i]) {
                    *isKeyFound_p = ALDER_NO;
                    break;
                }
            }
        }
        if (*isEmptyFound_p == ALDER_NO && *isKeyFound_p == ALDER_NO) {
            (*tries_p)++;
            (*x_p)++;
            if (*x_p == o->size) {
                *x_p = 0;
            }
        }
    }
    return pos;
}

/**
 *  This function insert a key to an empty slot.
 *
 *  @param o       table
 *  @param pointer position of an empty slot
 *  @param key     key
 *  @param tries   offset
 */
static void
alder_hashtable_mcas2_insert_to_empty(alder_hashtable_mcas2_t *o,
                                      uint64_t pointer,
                                      uint64_t *key,
                                      uint64_t tries)
{
    size_t pos = pointer * o->stride;
    /* Set the key and reset the auxiliary part of the key. */
    memcpy(o->key + pointer * o->stride, key, sizeof(*key) * o->size_key);
    o->n_key++;
    /* Place an empty slot to the head as in function with_hashcode. */
    uint64_t v = (tries << 32);             // Set the new offset.
    v++;                                    // Increment the value
    o->key[pos + o->pos_value] = v;         // Finish offset and value.
    /* Set left/right: place an empty slot to the head. */
    uint64_t pos_head = o->pointer_head * o->stride;
    if (o->size <= ALDER_HASHTABLE_SIZE31) {
        /* Left to head, and right to NULL. */
        o->key[pos + o->pos_pointerL] = ((o->pointer_head << 32) |
                                             V32_NULL);
        /* Let the head point to the new empty. */
        o->key[pos_head + o->pos_pointerR] &= V64_L32_MASK0;
        o->key[pos_head + o->pos_pointerR] |= pointer;
    } else {
        /* Left to head, and right to NULL. */
        o->key[pos + o->pos_pointerL] = o->pointer_head;
        o->key[pos + o->pos_pointerR] = V64_NULL;
        /* Let the head point to the new empty. */
        o->key[pos_head + o->pos_pointerR] = pointer;
    }
    o->key[pos + o->pos_timestamp] = o->timestamp; // Finish timestamp.
    /* Move the head pointer to the new empty slot. */
    o->pointer_head = pointer;
    /* Increment the frequency table. */
    o->frequency[1]++;
}

/**
 *  This function updates the found key's value.
 *
 *  @param o       table
 *  @param pointer position at which the key is found
 *  @param tries   offset (tries + 1)
 */
static void
alder_hashtable_mcas2_update_found_key(alder_hashtable_mcas2_t *o,
                                       uint64_t pointer, uint64_t tries)
{
    size_t pos;
    pos = pointer * o->stride;
    /* Set offset and value. */
    uint64_t v = o->key[pos + o->pos_value];// Get offset+value.
    v &= V32_VALUE_MASK;                    // Reset offset.
    uint64_t c = v;
    
    ///////////////////////////////////////////////////////////////////
    // Frequency can be updated with functions.
    ///////////////////////////////////////////////////////////////////
    /* Decrement the frequency. */
    if (ALDER_HASHTABLE_SIZE16 <= c) {
        c = ALDER_HASHTABLE_SIZE16 - 1;
    }
    o->frequency[c]--;
    /* Change the frequencyL. */
    uint64_t timestamp = o->key[pos + o->pos_timestamp];
    uint64_t pos_P = o->pointer * o->stride;
    uint64_t timestamp_pointer = o->key[pos_P + o->pos_timestamp];
    if (timestamp < timestamp_pointer) {
        o->frequencyL[c]--;
    }
    
    c = v;
    v |= (tries << 32);                     // Set the new offset.
    if (c < o->max_value) {                 // Increment value
        c++;                                // if less than max.
        v++;
    }
    o->key[pos + o->pos_value] = v;         // Finish offset and value.
    /* Set left/right: place the slot to the head. */
    uint64_t pos_H = o->pointer_head * o->stride;
    uint64_t pointer_L, pointer_R, pos_L, pos_R;
    if (o->size <= ALDER_HASHTABLE_SIZE31) {
        /* Take the current and move it to the head. */
        pointer_L = (o->key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32;
        pointer_R = o->key[pos + o->pos_pointerL] & V32_POINTER_R;
        pos_L = pointer_L * o->stride;
        pos_R = pointer_R * o->stride;
        /* Case 1. the pointer is not at the head. */
        if (pointer_R != V32_NULL) {
            assert(o->pointer_head != pointer);
            /* The pointer is not at the tail. */
            if (pointer_L != V32_NULL) {
                /* Left points to right. */
                o->key[pos_L + o->pos_pointerR] &= V64_L32_MASK0;
                o->key[pos_L + o->pos_pointerR] |= pointer_R;
                /* Right points to left. */
                o->key[pos_R + o->pos_pointerL] &= V64_U32_MASK0;
                o->key[pos_R + o->pos_pointerL] |= (pointer_L << 32);
            }
            /* The pointer is at the tail. */
            else {
                /* Right's left points to NULL. */
                o->key[pos_R + o->pos_pointerL] &= V64_U32_MASK0;
                o->key[pos_R + o->pos_pointerL] |= V32_U_NULL;
                o->pointer_tail = pointer_R;
            }
            /* Pointer's left to head. */
            o->key[pos + o->pos_pointerL] &= V64_U32_MASK0;
            o->key[pos + o->pos_pointerL] |= (o->pointer_head << 32);
            /* Pointer's right to NULL. */
            o->key[pos + o->pos_pointerR] &= V64_L32_MASK0;
            o->key[pos + o->pos_pointerR] |= V32_L_NULL;
            /* head to the pointer. */
            o->key[pos_H + o->pos_pointerR] &= V64_L32_MASK0;
            o->key[pos_H + o->pos_pointerR] |= pointer;
            o->pointer_head = pointer;
        }
        /* Case 2. the pointer is at the head. */
        else {
            assert(o->pointer_head == pointer);
        }
        
        /* Move the middle pointer to the right. */
        if (o->pointer == pointer) {
            if (pointer_R != V32_L_NULL) {
                o->pointer = pointer_R;
            }
        }
    } else {
        /* Take the current and move it to the head. */
        pointer_L = o->key[pos + o->pos_pointerL];
        pointer_R = o->key[pos + o->pos_pointerR];
        pos_L = pointer_L * o->stride;
        pos_R = pointer_R * o->stride;
        /* Case 1. the pointer is not at the head. */
        if (pointer_R != V64_NULL) {
            assert(o->pointer_head != pointer);
            /* The pointer is not at the tail. */
            if (pointer_L != V64_NULL) {
                /* Left points to right. */
                o->key[pos_L + o->pos_pointerR] = pointer_R;
                /* Right points to left. */
                o->key[pos_R + o->pos_pointerL] = pointer_L;
            }
            /* The pointer is at the tail. */
            else {
                /* Right points to NULL. */
                o->key[pos_R + o->pos_pointerL] = V64_NULL;
            }
            /* Pointer's left to head. */
            o->key[pos + o->pos_pointerL] = o->pointer_head;
            /* Pointer's right to NULL. */
            o->key[pos + o->pos_pointerR] = V64_NULL;
            /* head to the pointer. */
            o->key[pos_H + o->pos_pointerR] = pointer;
            o->pointer_head = pointer;
        }
        /* Case 2. the pointer is at the head. */
        else {
            assert(o->pointer_head == pointer);
        }
        
        /* Move the middle pointer to the right. */
        if (o->pointer == pointer) {
            if (pointer_R != V64_NULL) {
                o->pointer = pointer_R;
            }
        }
        
    }
    /* Set timestamp. */
    o->key[pos + o->pos_timestamp] = o->timestamp; // Finish timestamp.
    /* Change the frequency. */
    if (ALDER_HASHTABLE_SIZE16 <= c) {
        c = ALDER_HASHTABLE_SIZE16 - 1;
    }
    o->frequency[c]++;
}

/**
 *  This function add a key to the 2nd version of a hash table. It fills upto
 *  70% of the table. 
 *
 *  @param o               table
 *  @param key             key
 *  @param x               hash value
 *  @param res_key         extra key
 *  @param added_x         position at which the key is added or updated
 *  @param isMultithreaded multi-thread flag
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas2_increment_hashcode(alder_hashtable_mcas2_t *o,
                                         uint64_t *key,
                                         uint64_t x,
                                         uint64_t *res_key,
                                         uint64_t *added_x,
                                         bool isMultithreaded)
{
    assert(isMultithreaded == false);
#if !defined(NDEBUG)
    uint64_t y = x % o->size;
#endif
    uint64_t x_over_size = libdivide_u64_do(x, &o->fast_size);
    x = x - x_over_size * o->size;
#if !defined(NDEBUG)
    assert(x == y);
#endif
    
    uint64_t pos = 0;
    uint64_t tries = 0;
    int isKeyFound = ALDER_NO;
    int isEmptyFound = ALDER_NO;
    /* 1. Search for the key or an empty key. */
    pos = alder_hashtable_mcas2_find_key(o, &x, &tries,
                                         &isKeyFound, &isEmptyFound, key);
    if (o->size < tries) {
        alder_loge(ALDER_ERR_HASH, "initial table is full!");
        assert(0);
        abort();
        return ALDER_STATUS_ERROR;
    }
    
    /* 2. Add a key to the empty slot or update the key. */
    *added_x = x;
    uint64_t pointer = x;
    tries++;                                  // Adjust tries to be offset.
    o->timestamp++;                           // Increment timestamp by 1.
    /* Case 1. Empty key. */
    if (isEmptyFound == ALDER_YES && o->pointer != UINT64_MAX) {
        alder_hashtable_mcas2_insert_to_empty(o, pointer, key, tries);
    }
    /* Case 2. Found key. */
    else if (isKeyFound == ALDER_YES) {
        alder_hashtable_mcas2_update_found_key(o, pointer, tries);
    }
    /* Case 3. The first key ever. */
    else {
        /* The first slot ever. */
        assert(isEmptyFound == ALDER_YES);
        assert(o->pointer == UINT64_MAX);
        assert(o->timestamp == 1);
        memcpy(o->key + pos, key, sizeof(*key) * o->size_key);
        o->n_key = 1;
        o->key[pos + o->pos_value] = 0x0000000100000001;
        if (o->size <= ALDER_HASHTABLE_SIZE31) {
            o->key[pos + o->pos_pointerL] = V32_U_NULL | V32_L_NULL;
        } else {
            o->key[pos + o->pos_pointerL] = V64_NULL;
            o->key[pos + o->pos_pointerR] = V64_NULL;
        }
        o->key[pos + o->pos_timestamp] = o->timestamp;
        o->pointer = pointer;
        o->pointer_head = pointer;
        o->pointer_tail = pointer;
        o->frequency[1] = 1;
    }
    
    if (o->size7 <= o->n_key) {
        return ALDER_HASHTABLE_FULL;
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function returns the oldest key with value less than or equal to
 *  the min_freq_value. I start at a middle pointer. I need a min_freq_value.
 *
 *  1. Determine which direction I move to find an old key.
 *
 *  Note that the frequencyL does not include the middle pointer slot.
 *
 *  @param o     table
 *  @param not_x key that must not be deleted
 *
 *  @return position of the oldest key
 */
static uint64_t
alder_hashtable_mcas2_search_old_key(alder_hashtable_mcas2_t *o,
                                     uint64_t not_x)
{
    uint64_t pointer_delete = o->pointer;
    
    /* 1. Determine the minimum count value using frequency table. */
    o->min_freq_value = 1;
    uint64_t n_kmer = 0;
    for (uint64_t i = 1; i < ALDER_HASHTABLE_SIZE16; i++) {
        n_kmer += o->frequency[i];
        if (o->size5 <= n_kmer) {
            o->min_freq_value = i;
            break;
        }
    }
    
    /* 2. Detemine whether I move toward the tail (left) or head. */
    uint64_t n_left_slot = 0;
    for (uint64_t i = 1; i <= o->min_freq_value; i++) {
        n_left_slot += o->frequencyL[i];
    }
    bool moveRight = true;
    if (n_left_slot > 0) {
        moveRight = false;
    }
    
    /* 3. Probe the linked-list to find one with a small value. */
    uint64_t pos, value;
    if (moveRight == true) {
        pos = o->pointer * o->stride;
        value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
        while (o->min_freq_value < value) {
            /* Increment frequencyL as the pointer moves to the right. */
            o->frequencyL[value]++;
            /* Move to right. */
            if (o->size <= ALDER_HASHTABLE_SIZE31) {
                pos = (o->key[pos + o->pos_pointerR] & V32_POINTER_R) * o->stride;
            } else {
                pos = o->key[pos + o->pos_pointerR] * o->stride;
            }
            value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
        }
        pointer_delete = pos / o->stride;
    } else {
        assert(n_left_slot > 0);
        /* Move to the left because the current one is not selected. */
        pos = o->pointer * o->stride;
        assert(o->pointer < o->size);
        
//        if (o->size <= ALDER_HASHTABLE_SIZE31) {
//            pos = ((o->key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32) * o->stride;
//        } else {
//            pos = o->key[pos + o->pos_pointerL] * o->stride;
//        }
        
        while (n_left_slot > 0) {
            /* Move to the left. */
            if (o->size <= ALDER_HASHTABLE_SIZE31) {
                pos = ((o->key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32) * o->stride;
            } else {
                pos = o->key[pos + o->pos_pointerL] * o->stride;
            }
            value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
            /* Decrement frequencyL as the pointer moves to the left. */
            o->frequencyL[value]--;
            if (value <= o->min_freq_value) {
                n_left_slot--;
            }
        }
        pointer_delete = pos / o->stride;
    }
    return pointer_delete;
}

/**
 *  This function deletes a key by searching for one from x and avoiding not_x.
 *
 *  @param o               table
 *  @param x               starting position
 *  @param not_x           not delete the key at this position
 *  @param isMultithreaded multi-thread flag
 *
 *  @return SUCCESS or FAIL
 *
    void compress(int free) {
        int i = (free + 1) % SIZE, off = 1;
        for (; ex[i] != 0; i = (i + 1) % SIZE, off++)
            if (ex[i] > off) {
                // move current element
                a[free] = a[i];
                ex[free] = ex[i] - off;
                // mark current slot as free
                ex[i] = 0;
                off = 0;
                free = i;
            }
 *
 */
static uint64_t
alder_hashtable_mcas2_spacesaving_delete(alder_hashtable_mcas2_t *o,
                                         uint64_t not_x,
                                         uint64_t x_free,
                                         bool isMultithreaded)
{
    assert(isMultithreaded == false);
    assert(x_free != not_x);
    int isEmptyFound = ALDER_NO;
    uint64_t x = x_free;
    uint64_t offset = 0;
    
    ///////////////////////////////////////////////////////////////////////////
    // Rearrange left and right pointers before deleting the key.
    uint64_t pos = x_free * o->stride;
    uint64_t pointer_L;
    uint64_t pointer_R;
    uint64_t pos_L;
    uint64_t pos_R;
    if (o->size <= ALDER_HASHTABLE_SIZE31) {
        /* Take the current and move it the head. */
        pointer_L = (o->key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32;
        pointer_R = o->key[pos + o->pos_pointerL] & V32_POINTER_R;
        pos_L = pointer_L * o->stride;
        pos_R = pointer_R * o->stride;
        assert(pointer_R != V32_NULL);
        if (pointer_L != V32_NULL) {
            /* Left points to right. */
            o->key[pos_L + o->pos_pointerR] &= V64_L32_MASK0;
            o->key[pos_L + o->pos_pointerR] |= pointer_R;
            /* Right points to left. */
            o->key[pos_R + o->pos_pointerL] &= V64_U32_MASK0;
            o->key[pos_R + o->pos_pointerL] |= (pointer_L << 32);
        } else {
            /* Right's left points to NULL. */
            o->key[pos_R + o->pos_pointerL] &= V64_U32_MASK0;
            o->key[pos_R + o->pos_pointerL] |= V32_U_NULL;
            o->pointer_tail = pointer_R;
        }
    } else {
        pointer_L = o->key[pos + o->pos_pointerL];
        pointer_R = o->key[pos + o->pos_pointerR];
        pos_L = pointer_L * o->stride;
        pos_R = pointer_R * o->stride;
        assert(pointer_R != V64_NULL);
        if (pointer_L != V64_NULL) {
            /* Left points to right. */
            o->key[pos_L + o->pos_pointerR] = pointer_R;
            /* Right points to left. */
            o->key[pos_R + o->pos_pointerL] = pointer_L;
        } else {
            /* Right points to NULL. */
            o->key[pos_R + o->pos_pointerL] = V64_NULL;
            o->pointer_tail = pointer_R;
        }
    }
    o->pointer = pointer_R;
    /* Decrement frequencyL. */
    uint64_t value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
    o->frequency[value]--;
    //
    ///////////////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////////////
    // First, delete the key!
    size_t pos_x = x * o->stride;
    memset(o->key + pos_x, 0x44, sizeof(*o->key) * o->size_key);
    for (int i = 0; i < o->size_auxiliary; i++) {
        o->key[pos_x + o->size_key + i] = 0;
    }
    //
    ///////////////////////////////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////////////////////////////
    // Move up some keys below.
    // This does not affect the double-linked list.
    while (isEmptyFound == ALDER_NO) {
        x++;
        if (x == o->size) {
            x = 0;
        }
        offset++;
        
        // Get offset.
        size_t pos_x = x * o->stride;
        size_t pos_free = x_free * o->stride;
        uint64_t v_x = o->key[pos_x + o->pos_value];
        uint64_t ex_x = (v_x & V32_POINTER_L) >> 32;
        
        if (ex_x == 0) {
            isEmptyFound = ALDER_YES;
        } else if (ex_x > offset) {
            ///////////////////////////////////////////////////////////////////
            // Rearrage the double-linked list: x -> x_free
            // 0 -> 4 rearrange 0's neighbor node left/right
            // 3-0-1 -> 3-4-1.
            // 0 -> 4
            // 3-0 -> 3-4
            // 0-1 -> 4-1
            // pointer
            // pointer_head
            // pointer_tail
            //
            // x = 0
            // x_free = 4
            if (o->pointer == x) {
                o->pointer = x_free;
            }
            if (o->pointer_head == x) {
                o->pointer_head = x_free;
            }
            if (o->pointer_tail == x) {
                o->pointer_tail = x_free;
            }
            if (o->size <= ALDER_HASHTABLE_SIZE31) {
                pos = pos_x;
                pointer_L = (o->key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32;
                pointer_R = o->key[pos + o->pos_pointerL] & V32_POINTER_R;
                pos_L = pointer_L * o->stride;
                pos_R = pointer_R * o->stride;
                /* Left points to the new slot. */
                if (pointer_L != V32_NULL) {
                    o->key[pos_L + o->pos_pointerR] &= V64_L32_MASK0;
                    o->key[pos_L + o->pos_pointerR] |= x_free;
                }
                /* Right points to the new slot. */
                if (pointer_R != V32_NULL) {
                    o->key[pos_R + o->pos_pointerL] &= V64_U32_MASK0;
                    o->key[pos_R + o->pos_pointerL] |= (x_free << 32);
                }
            } else {
                pos = pos_x;
                pointer_L = o->key[pos + o->pos_pointerL];
                pointer_R = o->key[pos + o->pos_pointerR];
                pos_L = pointer_L * o->stride;
                pos_R = pointer_R * o->stride;
                /* Left points to the new slot. */
                if (pointer_L != V64_NULL) {
                    o->key[pos_L + o->pos_pointerR] = x_free;
                }
                /* Right points to the new slot. */
                if (pointer_R != V64_NULL) {
                    o->key[pos_R + o->pos_pointerL] = x_free;
                }
            }
            //
            ///////////////////////////////////////////////////////////////////
            
            // Move the current element.
            uint64_t offset_free = ex_x - offset;
            v_x &= V32_VALUE_MASK;
            v_x |= (offset_free << 32);
            o->key[pos_x + o->size_key] = v_x;
            for (int i = 0; i < o->stride; i++) {
                o->key[pos_free + i] = o->key[pos_x + i];
            }
            
            // Mark the current slot as free.
            memset(o->key + pos_x, 0x44, sizeof(*o->key) * o->size_key);
            for (int i = 0; i < o->size_auxiliary; i++) {
                o->key[pos_x + o->pos_value + i] = 0;
            }
            
            offset = 0;
            x_free = x;
        }
    }
    //
    ///////////////////////////////////////////////////////////////////////////
    
    return ALDER_STATUS_SUCCESS;
}

void
alder_hashtable_mcas2_set_pointer_to_tail(alder_hashtable_mcas2_t *o)
{
    assert(o->pointer == o->pointer_tail);
    o->pointer = o->pointer_tail;
    for (int i = 0; i < ALDER_HASHTABLE_SIZE16; i++) {
        assert(o->frequencyL[i] == 0);
    }
}

/**
 *  This function writes a deleted key and its value to a file: o->fpd.
 *
 *  @param o         table
 *  @param removed_x removed key's positions
 */
static void
alder_hashtable_mcas2_write_deleted_key(alder_hashtable_mcas2_t *o,
                                        uint64_t x)
{
    uint64_t pos_removed = x * o->stride;
    size_t s = fwrite(o->key + pos_removed, sizeof(*o->key) * o->size_key, 1, o->fpd);
    if (s != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to write deleted keys");
    }
    uint32_t value = (o->key[pos_removed + o->pos_value] & V32_VALUE_MASK);
    s = fwrite(&value, sizeof(value), 1, o->fpd);
    if (s != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to write deleted key's value");
    }
}

/**
 *  This function writes a saved key and its value to a file: o->fps.
 *
 *  @param o table
 *  @param x key's positions
 */
static void
alder_hashtable_mcas2_write_saved_key(alder_hashtable_mcas2_t *o,
                                      uint64_t x)
{
    uint64_t pos_removed = x * o->stride;
    size_t s = fwrite(o->key + pos_removed, sizeof(*o->key) * o->size_key, 1, o->fps);
    if (s != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to write saved keys");
    }
    uint32_t value = (o->key[pos_removed + o->pos_value] & V32_VALUE_MASK);
    s = fwrite(&value, sizeof(value), 1, o->fps);
    if (s != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to write saved key's value");
    }
}

/**
 *  This function find a key in the given hash table. If the key is found,
 *  it increments the value of the key. If not, it finds an empty key to remove
 *  it from the table, and randomly choose a key with the smallest value to
 *  delete it. The pre-condition of this function is that the table is 70% 
 *  full.
 *
 *  @param o               table
 *  @param key             key
 *  @param x               hash
 *  @param res_key         key + value for compare-and-swap operation
 *  @param res_key2        key + value for compare-and-swap operation
 *  @param min_value       minimum value
 *  @param isMultithreaded multithread flag
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas2_increment_spacesaving(alder_hashtable_mcas2_t *o,
                                            uint64_t *key,
                                            uint64_t x,
                                            uint64_t *res_key,
                                            uint64_t *res_key2,
                                            uint64_t *added_x,
                                            uint64_t *removed_x,
                                            bool isMultithreaded)
{
    assert(isMultithreaded == false);
#if !defined(NDEBUG)
    uint64_t y = x % o->size;
#endif
    uint64_t x_over_size = libdivide_u64_do(x, &o->fast_size);
    x = x - x_over_size * o->size;
#if !defined(NDEBUG)
    assert(x == y);
#endif
    
    size_t pos = 0;
    uint64_t tries = 0;
    int isKeyFound = ALDER_NO;
    int isEmptyFound = ALDER_NO;
    /* 1. Find the key or an empty key: pos is the position in the key. */
    pos = alder_hashtable_mcas2_find_key(o, &x, &tries,
                                         &isKeyFound, &isEmptyFound, key);
    assert(isEmptyFound == ALDER_YES || isKeyFound == ALDER_YES);
    
    *added_x = x;
    uint64_t pointer = x;
    tries++;                                  // Adjust tries to be offset.
    o->timestamp++;                           // Increment timestamp by 1.
    assert(tries <= V32_NULL);
    /* If an empty key is found, replace it with the key. */
    if (isEmptyFound == ALDER_YES) {
        alder_hashtable_mcas2_insert_to_empty(o, pointer, key, tries);
        /* Delete an old key with a small value.       */
        /* This could change tail and middle pointers. */
        *removed_x = alder_hashtable_mcas2_search_old_key(o, pointer);
        /* Write the deleted key and value to a file: .kmc */
        if (o->fpd != NULL) {
            alder_hashtable_mcas2_write_deleted_key(o, *removed_x);
        }
        /* Delete the key. */
        alder_hashtable_mcas2_spacesaving_delete(o, pointer, *removed_x,
                                                 isMultithreaded);
        o->n_key--;
    }
    /* Case 2. Found key. */
    else {
        assert(isKeyFound == ALDER_YES);
        alder_hashtable_mcas2_update_found_key(o, pointer, tries);
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function writes the header of a version-2 hash table.
 *
 *  @param fd        file
 *  @param kmer_size K
 *  @param n_nh      size of the table
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas2_printHeaderToFD(int fd,
                                      int kmer_size,
                                      uint64_t n_nh,
                                      uint64_t size_value,
                                      uint64_t size_index)
{
    ssize_t s = 0;
    s = write(fd, &kmer_size, sizeof(kmer_size));
    if (s != sizeof(kmer_size)) return ALDER_STATUS_ERROR;
    s = write(fd, &n_nh, sizeof(n_nh));
    if (s != sizeof(n_nh)) return ALDER_STATUS_ERROR;
    s = write(fd, &size_value, sizeof(size_value));
    if (s != sizeof(size_value)) return ALDER_STATUS_ERROR;
    s = write(fd, &size_index, sizeof(size_index));
    if (s != sizeof(size_index)) return ALDER_STATUS_ERROR;
   
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function appends the number of elements to the table.
 *
 *  @param value number of elements in the table
 *  @param fd    file
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas2_printPackToFD_count(size_t value, int fd)
{
    ssize_t s = write(fd, &value, sizeof(value));
    if (s != sizeof(value)) {
        assert(0);
        return ALDER_STATUS_ERROR;
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function prints the contents of a table after counting is finished.
 *
 *  @param o       table
 *  @param fd      file
 *  @param bufsize buffer size
 *
 *  @return SUCCESS or FAIL
 */
int64_t
alder_hashtable_mcas2_printPackToFD(alder_hashtable_mcas2_t *o,
                                    int fd,
                                    size_t bufsize)
{
    int64_t n = 0;
    size_t stride = o->stride;
    size_t size_key = o->size_key * sizeof(uint64_t);
    size_t size_value = sizeof(uint32_t);
    size_t size = o->size;
    uint64_t *key = o->key;
    size_t i_buf = 0;
    
    /* Determine the byte-size of an index value. */
    size_t size_index = sizeof(uint32_t);
    if (size <=  UINT32_MAX) {
        size_index = sizeof(uint32_t);
    } else {
        size_index = sizeof(size_t);
    }
    /* Determine the byte-size of a count value. */
    if (o->max_value <= UINT8_MAX) {
        size_value = sizeof(uint8_t);
    } else if (o->max_value <= UINT16_MAX) {
        size_value = sizeof(uint16_t);
    } else if (o->max_value <= UINT32_MAX) {
        size_value = sizeof(uint32_t);
    } else {
        assert(0);
    }
    
    uint8_t *buf = malloc(bufsize + o->stride);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer_view(o->k);
    
    for (uint64_t i = 0; i < size; i++) {
        if (i_buf > bufsize) {
            write(fd, buf, i_buf);
            i_buf = 0;
        }
        size_t keypos = i * stride;
        uint64_t v = key[keypos + o->pos_value];     // Get offset+value.
        uint64_t c = v & V32_VALUE_MASK;             // Get value.
        
        if (o->fps != NULL && c > 0) {
            alder_hashtable_mcas2_write_saved_key(o, i);
        }

        if (o->min_value <= c) {
            
            memcpy(buf + i_buf, key + keypos, size_key);
            i_buf += size_key;
            
            switch (size_value) {
                case 1:
                    to_uint8(buf + i_buf,0) = (uint8_t)c;
                    break;
                case 2:
                    to_uint16(buf + i_buf,0) = (uint16_t)c;
                    break;
                case 4:
                    to_uint32(buf + i_buf,0) = (uint32_t)c;
                    break;
                default:
                    assert(0);
                    break;
            }
            i_buf += size_value;
            
            switch (size_index) {
                case 4:
                    to_uint32(buf + i_buf,0) = (uint32_t)i;
                    break;
                case 8:
                    to_uint64(buf + i_buf,0) = (uint64_t)i;
                    break;
                default:
                    assert(0);
                    break;
            }
            i_buf += size_index;
            
            n++;
        }
    }
    
    if (i_buf > 0) {
        write(fd, buf, i_buf);
    }
    
    XFREE(buf);
    alder_encode_number_destroy_view(m);
    
    return n;
}

/**
 *  This function prints the version-2 table to a file.
 *
 *  @param o  table
 *  @param fp file
 *
 *  @return number of elements in the table
 */
int64_t
alder_hashtable_mcas2_printToFILE(alder_hashtable_mcas2_t *o, FILE *fp)
{
    int64_t v = 0;
    size_t stride = o->stride;
    size_t size = o->size;
    uint64_t *key = o->key;
    size_t size_index = sizeof(uint32_t);
    if (size <=  UINT32_MAX) {
        size_index = sizeof(uint32_t);
    } else {
        size_index = sizeof(size_t);
    }
    alder_encode_number_t *m = alder_encode_number_create_for_kmer_view(o->k);
    
    assert(size <= UINT32_MAX);
    if (size <= UINT32_MAX) {
        for (uint32_t i = 0; i < size; i++) {
            uint64_t v = key[i * stride + o->size_key]; // Get offset+value.
            uint64_t c = v & 0x00000000ffffffff;  // Reset offset.
            uint64_t offset = (v & 0x3fffffff00000000) >> 32;
            uint64_t timestamp = o->key[i * stride + o->pos_timestamp];
            assert(c < UINT16_MAX);
            fprintf(fp, "%u\t%llu\t%llu\t%llu\n", i, offset, c, timestamp);
        }
    }
    
    alder_encode_number_destroy_view(m);
    return v;
}

/**
 *  This function prints the table for debug.
 *
 *  @param o table
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas2_debug_print(alder_hashtable_mcas2_t *o,
                                  uint64_t *c_key,
                                  uint64_t x,
                                  uint64_t added_x,
                                  uint64_t removed_x,
                                  const char *outdir,
                                  const char *outfile)
{
    alder_log5("Timestamp: %llu", o->timestamp);
    FILE *fp = NULL;
    bstring bfn = bformat("%s/%s-dbg.%llu", outdir, outfile, o->timestamp);
    fp = fopen(bdata(bfn), "w");
    
    
    
    int v = 0;
    size_t stride = o->stride;
    size_t size = o->size;
    uint64_t *key = o->key;
    alder_encode_number_t *m = alder_encode_number_create_for_kmer_view(o->k);
    
    fprintf(fp, "*** Key ***\n");
    m->n = c_key;
    fprintf(fp, "key    : ");
    alder_encode_number_print_in_DNA(fp, m);
    fprintf(fp, "\n");
    fprintf(fp, "hash   : %llu\n", x);
    fprintf(fp, "pos    : %llu\n", x % o->size);
    if (added_x != o->size) {
        fprintf(fp, "added  : %llu\n", added_x);
    }
    if (removed_x != o->size) {
        fprintf(fp, "removed: %llu\n", removed_x);
    }
    
    fprintf(fp, "\n*** Hash Table ***\n");
    int width[7] = {4, 1, 9, 9, 9, 9, 11};
    width[1] = o->k + 1;
    uint64_t value;
    uint64_t offset;
    uint64_t left;
    uint64_t right;
    uint64_t timestamp;
    fprintf(fp, "%*s ", width[0], "idx");
    for (int i = 0; i < o->k; i++) {
        fprintf(fp, " ");
    }
//    alder_encode_number_print_in_DNA(fp, m);
    fprintf(fp, " %*s%*s%*s%*s%*s\n",
            width[2], "value", width[3], "offset",
            width[4], "left", width[5], "right", width[6], "timestamp");
    for (size_t i = 0; i < size; i++) {
        uint64_t pos = i * stride;
        m->n = key + pos;
        value = key[pos + o->pos_value] & V32_VALUE_MASK;
        offset = (key[pos + o->pos_value] & V32_OFFSET_MASK) >> 32;
        if (o->size <= ALDER_HASHTABLE_SIZE31) {
            left = (key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32;
            right = key[pos + o->pos_pointerR] & V32_POINTER_R;
            if (left == V32_NULL) {
                left = 444;
            }
            if (right == V32_NULL) {
                right = 444;
            }
        } else {
            left = key[pos + o->pos_pointerL];
            right = key[pos + o->pos_pointerR];
            if (left == V64_NULL) {
                left = 444;
            }
            if (right == V64_NULL) {
                right = 444;
            }
        }
        timestamp = key[pos + o->pos_timestamp];
        
        // Print them in a fixed spaced.
        fprintf(fp, "%*zu ", width[0], i);
        if (memcmp(m->n, o->empty_key, sizeof(*m->n) * o->size_key)) {
            alder_encode_number_print_in_DNA(fp, m);
        } else {
            for (int j = 0; j < o->k; j++) {
                fprintf(fp, "-");
            }
        }
        fprintf(fp, " %*llu%*llu%*llu%*llu%*llu\n",
                width[2], value, width[3], offset,
                width[4], left, width[5], right, width[6], timestamp);
    }
    
    fprintf(fp, "\n*** Linked-List ***\n");
    
    uint64_t pointer = o->pointer_head;
    while (1) {
        uint64_t pos = pointer * stride;
        m->n = key + pos;
        value = key[pos + o->pos_value] & V32_VALUE_MASK;
        offset = (key[pos + o->pos_value] & V32_OFFSET_MASK) >> 32;
        if (o->size <= ALDER_HASHTABLE_SIZE31) {
            left = (key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32;
            right = key[pos + o->pos_pointerR] & V32_POINTER_R;
            if (left == V32_NULL) {
                left = 444;
            }
            if (right == V32_NULL) {
                right = 444;
            }
        } else {
            left = key[pos + o->pos_pointerL];
            right = key[pos + o->pos_pointerR];
            if (left == V64_NULL) {
                left = 444;
            }
            if (right == V64_NULL) {
                right = 444;
            }
        }
        timestamp = key[pos + o->pos_timestamp];
        
        // Print them in a fixed spaced.
        fprintf(fp, "%*llu ", width[0], pointer);
        alder_encode_number_print_in_DNA(fp, m);
        fprintf(fp, " %*llu%*llu%*llu%*llu%*llu",
                width[2], value, width[3], offset,
                width[4], left, width[5], right, width[6], timestamp);
        if (pointer == o->pointer) {
            fprintf(fp, " <--- P");
        }
        if (pointer == o->pointer_head) {
            fprintf(fp, " <--- H");
        }
        if (pointer == o->pointer_tail) {
            fprintf(fp, " <--- T");
        }
        fprintf(fp, "\n");
        if (o->size <= ALDER_HASHTABLE_SIZE31) {
            if (left == 444) {
                left = V32_NULL;
            }
            if (right == 444) {
                right = V32_NULL;
            }
        } else {
            if (left == 444) {
                left = V64_NULL;
            }
            if (right == 444) {
                right = V64_NULL;
            }
        }
        
        pointer = left;
        if (o->size <= ALDER_HASHTABLE_SIZE31) {
            if (left == V32_NULL) {
                break;
            }
        } else {
            if (left == V64_NULL) {
                break;
            }
        }
    }
    
    fprintf(fp, "\n*** Frequency ***\n");
    for (int i = 0; i < 10; i++) {
        fprintf(fp, "[%d] %llu ", i, o->frequency[i]);
    }
    fprintf(fp, "\n*** FrequencyL ***\n");
    for (int i = 0; i < 10; i++) {
        fprintf(fp, "[%d] %llu ", i, o->frequencyL[i]);
    }
    fprintf(fp, "\n");
    
    alder_encode_number_destroy_view(m);
    bdestroy(bfn);
    XFCLOSE(fp);
    return v;
}

/**
 *  This function reads the header file of a table.
 *
 *  @param fn       header file name
 *  @param K_p      K
 *  @param nh_p     nh - size of the table
 *  @param sv_p     sv - byte-size of value
 *  @param si_p     si - byte-size of index
 *  @param th_p     th - total number of elements with values
 *
 *  @return SUCCESS or FAIL
 */
static int
read_table2_header(const char *fn, int *K_p,
                   uint64_t *nh_p, uint64_t *sv_p, uint64_t *si_p,
                   uint64_t *th_p)
                   
{
    ssize_t readLen = 0;
    int fildes = open(fn, O_RDWR); // O_RDONLY
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    readLen = read(fildes, &(*K_p), sizeof(*K_p));
    if (readLen != sizeof(*K_p) || *K_p <= 0) {
        fprintf(stderr, "Error - failed to read K in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Table of alder-kmer ***");
    alder_log("%*s %d", width, "K:", *K_p);
    
    readLen = read(fildes, &(*nh_p), sizeof(*nh_p));
    if (readLen != sizeof(*nh_p) || *nh_p == 0) {
        fprintf(stderr, "Error - failed to read Nh in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "nh:", *nh_p);
    
    readLen = read(fildes, &(*sv_p), sizeof(*sv_p));
    if (readLen != sizeof(*sv_p) || *sv_p == 0) {
        fprintf(stderr, "Error - failed to read Ni in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "sv:", *sv_p);
    
    readLen = read(fildes, &(*si_p), sizeof(*si_p));
    if (readLen != sizeof(*si_p) || *si_p == 0) {
        fprintf(stderr, "Error - failed to read Np in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "si:", *si_p);
    
    readLen = read(fildes, &(*th_p), sizeof(*th_p));
    if (readLen != sizeof(*th_p) || *th_p == 0) {
        fprintf(stderr, "Error - failed to read Np in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "th:", *th_p);
    
    close(fildes);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function searches the version-2 table for a query.
 *
 *  @param fn    table name prefix (.tbl/.tbh)
 *  @param query query
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas2_query(const char *fn,
                            const char *query)
{
    int K;
    uint64_t nh;
    uint64_t si;
    uint64_t sv;
    uint64_t th;
    // K - kmer size
    // nh - table size
    // sv - number of bytes in a value
    // si - number of bytes in an index
    // th - actual number of elements in a table
    int s = read_table2_header("outfile.tbh", &K, &nh, &sv, &si, &th);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
                fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    
    int size_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE) * sizeof(uint64_t);
    size_t size_element = size_key + sv + si;
    uint8_t *table = NULL;
    size_t size_item = sizeof(*table) * size_element * th;
    table = malloc(size_item + size_element);
    memset(table, 0, size_item + size_element);
    FILE *fp = fopen(fn, "rb");
    size_t nitems = fread(table, size_item, 1, fp);
    if (nitems != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to read a table %s", fn);
    }
    XFCLOSE(fp);
    
    /* Choose a Kmer sequence. */
    bstring bq = bfromcstr(query);
    if (bq->slen != K) {
        fprintf(stderr, "Kmer length is not %d.\n", K);
        return ALDER_STATUS_ERROR;
    }
    alder_encode_number_t *s1 = alder_encode_number_create_for_kmer(K);
    alder_encode_number_kmer_with_char(s1, (char*)bq->data);
    bdestroy(bq);
    alder_encode_number_t *s2 = alder_encode_number_create_rev(s1);
    alder_encode_number_t *ss = NULL;
    uint64_t hash_s1 = alder_encode_number_hash(s1);
    uint64_t hash_s2 = alder_encode_number_hash(s2);
    uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
    ss = (hash_s1 < hash_s2) ? s1 : s2;
    
    size_t x = hash_ss % nh;
//    x = 104851588;
    alder_log5("kmer: %s", query);
    alder_log5("pos: %zu", x);
    
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    
    /* Find the start and end of i_ni/i_np table. */
    size_t h_size = size_key + sv + si;
    size_t bit_value = 8 * (8 - sv);
    size_t bit_index = 8 * (8 - si);
    
    size_t start = 0;
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint64_t table_size = nh;
    int64_t start_index = 0;
    int64_t end_index = table_size;
    
    alder_log5("start: %lld", start);
    alder_log5("end_index: %lld", end_index);
    alder_log5("start_index: %lld", start_index);
    
    size_t pos = (size_t)-1;
    uint64_t value = 0;
    
    int64_t middle_index = start_index - 1;
    off_t middle = start;
    alder_log5("middle index: %lld", middle_index);
    alder_log5("middle: %lld", middle);
    while (start_index <= end_index) {
        int64_t new_middle_index = start_index + (end_index - start_index)/2;
        if (new_middle_index == table_size) {
            break;
        }
        middle = new_middle_index * h_size;
        middle_index = new_middle_index;
        alder_log5("middle index: %lld", middle_index);
        alder_log5("middle: %lld", middle);
        
        // Pos
        pos = to_size(table, middle + size_key + sv);
        pos <<= bit_index;
        pos >>= bit_index;
        for (int i = 0; i < si; i++) {
            alder_log5("[%d] %d", i, table[middle + size_key + sv + i] << (8 * i));
        }
        
        alder_log5("pos: %lld", pos);
        if (pos == x) {
            // Found!
            alder_log5("pos: %lld == %lld", pos, x);
            break;
        } else if (pos < x) {
            // middle replaces start.
            start_index = middle_index + 1;
            alder_log5("pos: %lld < %lld", pos, x);
            alder_log5("start_index: %lld (%lld[middle_index] + 1)", start_index, middle_index);
            alder_log5("end_index  : %lld", end_index);
        } else {
            // middle replaces end.
            alder_log5("pos: %lld >= %lld", pos, x);
            end_index = middle_index - 1;
            alder_log5("start_index: %lld", start_index);
            alder_log5("end_index  : %lld (%lld[middle_index] - 1)", end_index, middle_index);
        }
        
    }
    assert(pos < ((size_t)-1));
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    if (x == pos) {
        // key
        for (int i = 0; i < key->s; i++) {
            key->n[i] = to_uint64(table, middle + i * 8);
        }
        value = to_uint64(table, middle + size_key);
        value <<= bit_value;
        value >>= bit_value;
        
        // value and pos
//        alder_encode_number_print_in_DNA(stderr, ss);
//        fprintf(stderr, "\n");
//        alder_encode_number_print_in_DNA(stderr, key);
//        fprintf(stderr, "\t%llu\t", value);
//        fprintf(stderr, "%lld\n", middle_index);
        
        // Now, check the key itself because pos can be the same for different
        // keys.
        size_t tries = 1;
        while (memcmp(key->n, ss->n, key->s * sizeof(*key->n)) &&
               tries <= table_size) {
            tries++;
            middle_index++;
            if (middle_index == table_size) {
                middle_index = 0;
            }
            middle = middle_index * h_size;
            pos = to_size(table, middle + size_key + sv);
            pos <<= bit_index;
            pos >>= bit_index;
            value = to_uint64(table, middle + size_key);
            value <<= bit_value;
            value >>= bit_value;
            
            assert(s == ALDER_STATUS_SUCCESS);
            
//            alder_encode_number_print_in_DNA(stderr, key);
//            fprintf(stderr, "\t%zu\n", pos);
        }
        if (table_size < tries) {
            assert(0);
        }
        // Found at middle_index!
        alder_encode_number_print_in_DNA(stderr, key);
        fprintf(stderr, "\t%llu\t", value);
        fprintf(stderr, "%lld\n", middle_index);
        fprintf(stderr, "Found!\n");
        
    } else {
        // Not found.
        fprintf(stderr, "Not Found!\n");
    }
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    
    XFREE(table);
    alder_encode_number_destroy(key);
    alder_encode_number_destroy(s2);
    alder_encode_number_destroy(s1);
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas2_load(const char *fn, int isSummary)
{
    int K;
    uint64_t nh;
    uint64_t si;
    uint64_t sv;
    uint64_t th;
    // K - kmer size
    // nh - table size
    // sv - number of bytes in a value
    // si - number of bytes in an index
    // th - actual number of elements in a table
    int s = read_table2_header("outfile.tbh", &K, &nh, &sv, &si, &th);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
                fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint64_t value;
    size_t pos;
    int size_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE) * sizeof(uint64_t);
    size_t h_size = size_key + sv + si;
    size_t bit_value = 8 * (8 - sv);
    size_t bit_index = 8 * (8 - si);
    
    size_t size_element = size_key + sv + si;
    uint8_t *table = NULL;
    size_t size_item = sizeof(*table) * size_element * 2;
    table = malloc(size_item);
    memset(table, 0, size_item);
    
    FILE *fp = fopen(fn, "rb");
    
    for (uint64_t i_s = 0; i_s < th; i_s++) {
        // Read each element: key and value.
//        alder_hashtable_mcas_nextFromFile(fildes, key, &value, &pos, flag_nh64);
        
        size_t nitems = fread(table, size_element, 1, fp);
        if (nitems != 1) {
            alder_loge(ALDER_ERR_FILE, "failed to read a table %s", fn);
            abort();
        }
        size_t middle = 0 * h_size;
        for (int i = 0; i < key->s; i++) {
            key->n[i] = to_uint64(table, middle + i * 8);
        }
        pos = to_size(table, middle + size_key + sv);
        pos <<= bit_index;
        pos >>= bit_index;
        value = to_uint64(table, middle + size_key);
        value <<= bit_value;
        value >>= bit_value;
        
        // Print key and value.
        alder_encode_number_print_in_DNA(stdout, key);
        fprintf(stdout, "\t");
        alder_encode_number_print_in_revDNA(stdout, key);
        fprintf(stdout, "\t%llu", value);
        fprintf(stdout, "\t%zu\n", pos);
    }
    alder_encode_number_destroy(key);
    XFCLOSE(fp);
    
//    s = alder_hashtable_mcas_large_close(fildes);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to read the end of a table file %s\n",
                fn);
        return ALDER_STATUS_ERROR;
    }
    
    if (isSummary) {
        fprintf(stdout, "Kmer size          : %d\n", K);
        fprintf(stdout, "Hash table size    : %llu\n", nh);
        fprintf(stdout, "Size of value      : %llu\n", sv);
        fprintf(stdout, "Size of index      : %llu\n", si);
        fprintf(stdout, "Number of elements : %llu\n", th);
    }
    
    XFREE(table);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function reads the hash table of version 2.
 *
 *  @param fn filename
 *
 *  @return table
 */
alder_hashtable_mcas2_t*
alder_hashtable_mcas2_read_table(const char *fn)
{
    int K;
    uint64_t nh;
    uint64_t si;
    uint64_t sv;
    uint64_t th;
    // K - kmer size
    // nh - table size
    // sv - number of bytes in a value
    // si - number of bytes in an index
    // th - actual number of elements in a table
    int s = read_table2_header("outfile.tbh", &K, &nh, &sv, &si, &th);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
                fn, strerror(errno));
        return NULL;
    }
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint64_t value;
    size_t pos;
    int size_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE) * sizeof(uint64_t);
    size_t h_size = size_key + sv + si;
    size_t bit_value = 8 * (8 - sv);
    size_t bit_index = 8 * (8 - si);
    
    /* Create the hash table. */
    int stride = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE) + 1;
    alder_hashtable_mcas2_t *o = alder_hashtable_mcas2_create(K, nh, stride,
                                                              0, 0, 0,
                                                              NULL, NULL);
    
    size_t size_element = size_key + sv + si;
    uint8_t *table = NULL;
    size_t size_item = sizeof(*table) * size_element * 2;
    table = malloc(size_item);
    memset(table, 0, size_item);
    
    FILE *fp = fopen(fn, "rb");
    
    for (uint64_t i_s = 0; i_s < th; i_s++) {
        // Read each element: key and value.
        size_t nitems = fread(table, size_element, 1, fp);
        if (nitems != 1) {
            alder_loge(ALDER_ERR_FILE, "failed to read a table %s", fn);
            abort();
        }
        size_t middle = 0 * h_size;
        for (int i = 0; i < key->s; i++) {
            key->n[i] = to_uint64(table, middle + i * 8);
        }
        pos = to_size(table, middle + size_key + sv);
        pos <<= bit_index;
        pos >>= bit_index;
        value = to_uint64(table, middle + size_key);
        value <<= bit_value;
        value >>= bit_value;
        
        memcpy(o->key + pos * stride, table, size_key);
        o->key[pos * o->stride + 1] = value;
        
//        // Print key and value.
//        alder_encode_number_print_in_DNA(stdout, key);
//        fprintf(stdout, "\t");
//        alder_encode_number_print_in_revDNA(stdout, key);
//        fprintf(stdout, "\t%llu", value);
//        fprintf(stdout, "\t%zu\n", pos);
    }
    alder_encode_number_destroy(key);
    XFCLOSE(fp);
    
    XFREE(table);
    return o;
}

/**
 *  This funciton search the table of version 2 for a key.
 *
 *  @param o       table
 *  @param key     key
 *  @param pos_key position of the key
 *
 *  @return success or fail
 */
int
alder_hashtable_mcas2_search(alder_hashtable_mcas2_t *o,
                             uint64_t *key,
                             uint64_t *pos_key)
{
    
    uint64_t x = alder_hash_code01(key, o->size_key) % o->size;
    uint64_t x_over_size = libdivide_u64_do(x, &o->fast_size);
    x = x - x_over_size * o->size;
    
    uint64_t pos = 0;
    uint64_t tries = 0;
    int isKeyFound = ALDER_NO;
    int isEmptyFound = ALDER_NO;
    /* 1. Search for the key or an empty key. */
    pos = alder_hashtable_mcas2_find_key(o, &x, &tries,
                                         &isKeyFound, &isEmptyFound, key);
    if (o->size < tries) {
        return ALDER_STATUS_ERROR;
    }
    if (isEmptyFound == ALDER_YES) {
        return ALDER_STATUS_ERROR;
    }
    *pos_key = pos;
    return ALDER_STATUS_SUCCESS;
}


#pragma mark version12

struct alder_hashtable_mcas2_query_struct {
    int k;                      /* k-mer size                                */
    uint64_t nh;
    uint64_t si;
    uint64_t sv;
    uint64_t th;
    int size_key;               /* size of key                               */
    size_t size_element;
    size_t size_item;
    size_t h_size;
    size_t bit_value;
    size_t bit_index;
    uint8_t *table;
    
    int pos_timestamp;          /* pos_timestamp = size_key + 1              */
    int stride;                 /* stride in key, offset, value, timestamp   */
    int size_frequency;         /* size of frequency                         */
    size_t size;                /* number of elements in the hash table      */
    size_t size7;               /* number of elements in the hash table      */
    size_t n_key;               /* number of keys in the table               */
    uint64_t min_value;         /* min occurence                             */
    uint64_t max_value;         /* max occurence                             */
    uint64_t min_freq_value;    /* min occurence determined by freq. table   */
    uint64_t span_timestamp;    /* min time stamp                            */
    uint64_t n_timestamp;       /* time stamp                                */
    uint64_t timestamp;         /* time stamp                                */
    uint64_t *empty_key;        /* n: stride - EMPTY key                     */
    uint64_t *frequency;        /* 2^16: frequency of count                  */
    uint64_t *key;              /* n: size x stride (key+value+timestamp)    */
    struct libdivide_u64_t fast_size;
};

void
alder_hashtable_mcas2_query_destroy(alder_hashtable_mcas2_query_t *o)
{
    if (o != NULL) {
        XFREE(o->table);
        XFREE(o);
    }
}

alder_hashtable_mcas2_query_t *
alder_hashtable_mcas2_query_create(const char *fn)
{
    alder_hashtable_mcas2_query_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    
    int s = read_table2_header("outfile.tbh",
                               &o->k, &o->nh, &o->sv, &o->si, &o->th);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_hashtable_mcas2_query_destroy(o);
        return NULL;
    }
    
    o->size_key = ALDER_BYTESIZE_KMER(o->k,ALDER_NUMKMER_8BYTE) * sizeof(uint64_t);
    o->size_element = o->size_key + o->sv + o->si;
    o->size_item = sizeof(*o->table) * o->size_element * o->th;
    o->table = malloc(o->size_item + o->size_element);
    if (o->table == NULL) {
        alder_hashtable_mcas2_query_destroy(o);
        return NULL;
    }
    memset(o->table, 0, o->size_item + o->size_element);
    
    FILE *fp = fopen(fn, "rb");
    size_t nitems = fread(o->table, o->size_item, 1, fp);
    if (nitems != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to read a table %s", fn);
        alder_hashtable_mcas2_query_destroy(o);
        return NULL;
    }
    XFCLOSE(fp);
    
    o->h_size = o->size_key + o->sv + o->si;
    o->bit_value = 8 * (8 - o->sv);
    o->bit_index = 8 * (8 - o->si);
    return o;
}

int
alder_hashtable_mcas2_query_search(alder_hashtable_mcas2_query_t *o,
                                   alder_encode_number_t *ss,
                                   uint64_t *c_value)
{
    *c_value = 0;
    uint64_t hash_ss = alder_encode_number_hash(ss);
    
    size_t x = hash_ss % o->nh;
    
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    size_t start = 0;
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(o->k);
    uint64_t table_size = o->nh;
    int64_t start_index = 0;
    int64_t end_index = table_size;
    
    alder_log5("start: %lld", start);
    alder_log5("end_index: %lld", end_index);
    alder_log5("start_index: %lld", start_index);
    
    size_t pos = (size_t)-1;
    uint64_t value = 0;
    
    int64_t middle_index = start_index - 1;
    off_t middle = start;
    alder_log5("middle index: %lld", middle_index);
    alder_log5("middle: %lld", middle);
    while (start_index <= end_index) {
        int64_t new_middle_index = start_index + (end_index - start_index)/2;
        if (new_middle_index == table_size) {
            break;
        }
        middle = new_middle_index * o->h_size;
        middle_index = new_middle_index;
        alder_log5("middle index: %lld", middle_index);
        alder_log5("middle: %lld", middle);
        
        // Pos
        pos = to_size(o->table, middle + o->size_key + o->sv);
        pos <<= o->bit_index;
        pos >>= o->bit_index;
        for (int i = 0; i < o->si; i++) {
            alder_log5("[%d] %d", i, o->table[middle + o->size_key + o->sv + i] << (8 * i));
        }
        
        alder_log5("pos: %lld", pos);
        if (pos == x) {
            // Found!
            alder_log5("pos: %lld == %lld", pos, x);
            break;
        } else if (pos < x) {
            // middle replaces start.
            start_index = middle_index + 1;
            alder_log5("pos: %lld < %lld", pos, x);
            alder_log5("start_index: %lld (%lld[middle_index] + 1)", start_index, middle_index);
            alder_log5("end_index  : %lld", end_index);
        } else {
            // middle replaces end.
            alder_log5("pos: %lld >= %lld", pos, x);
            end_index = middle_index - 1;
            alder_log5("start_index: %lld", start_index);
            alder_log5("end_index  : %lld (%lld[middle_index] - 1)", end_index, middle_index);
        }
        
    }
    assert(pos < ((size_t)-1));
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    if (x == pos) {
        // key
        for (int i = 0; i < key->s; i++) {
            key->n[i] = to_uint64(o->table, middle + i * 8);
        }
        value = to_uint64(o->table, middle + o->size_key);
        value <<= o->bit_value;
        value >>= o->bit_value;
        
        // Now, check the key itself because pos can be the same for different
        // keys.
        size_t tries = 1;
        while (memcmp(key->n, ss->n, key->s * sizeof(*key->n)) &&
               tries <= table_size) {
            tries++;
            middle_index++;
            if (middle_index == table_size) {
                middle_index = 0;
            }
            middle = middle_index * o->h_size;
            pos = to_size(o->table, middle + o->size_key + o->sv);
            pos <<= o->bit_index;
            pos >>= o->bit_index;
            value = to_uint64(o->table, middle + o->size_key);
            value <<= o->bit_value;
            value >>= o->bit_value;
        }
        if (table_size < tries) {
            assert(0);
        }
        // Found at middle_index!
        alder_encode_number_print_in_DNA(stderr, key);
        *c_value = value;
//        fprintf(stderr, "\t%llu\t", value);
//        fprintf(stderr, "%lld\n", middle_index);
//        fprintf(stderr, "Found!\n");
        
    } else {
        // Not found.
//        fprintf(stderr, "Not Found!\n");
    }
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    
    alder_encode_number_destroy(key);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function compares an exactly counted table with an approximately 
 *  counted one.
 *
 *  @param error_kmer [return] error rate
 *  @param n_kmer     [return] total number of unique kmers
 *  @param s_kmer     size of error rate and total number of unique kmers
 *  @param fn         an exactly counted table file
 *  @param fn2        an approximately counted table file
 *  @param isSummary  [not used]
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas12_assess(double *error_kmer,
                              uint64_t *n_kmer,
                              size_t s_kmer,
                              const char *fn,
                              const char *fn2,
                              int isSummary)
{
    int fildes;
    int K;
    uint64_t nh;
    uint64_t ni;
    uint64_t np;
    uint64_t tnh;
    uint64_t *n_nhs;
    int s = open_table_header(fn, &fildes, &K, &nh, &ni, &np, &tnh, &n_nhs);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
                fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    
    uint64_t c_value = 0;
    alder_hashtable_mcas2_query_t *query =
    alder_hashtable_mcas2_query_create(fn2);
    
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint16_t value;
    size_t pos;
    int flag_nh64 = 0;
    if (nh <= UINT32_MAX) {
        flag_nh64 = 0;
    } else {
        flag_nh64 = 1;
    }
    for (uint64_t i_s = 0; i_s < tnh; i_s++) {
        // Read each element: key and value.
        alder_hashtable_mcas_nextFromFile(fildes, key, &value, &pos, flag_nh64);
        assert(0 < value);
        
        /* Search the approximately counted table for the key. */
        alder_hashtable_mcas2_query_search(query, key, &c_value);
        for (size_t i_kmer = 0; i_kmer < s_kmer; i_kmer++) {
            if (i_kmer < (size_t)value) {
                n_kmer[i_kmer]++;
                assert(c_value <= value);
                error_kmer[i_kmer] += ((double)value - (double)c_value)/(double)value;
            }
        }
        
        // Print key and value.
        alder_encode_number_print_in_DNA(stdout, key);
        fprintf(stdout, "\t");
        alder_encode_number_print_in_revDNA(stdout, key);
        fprintf(stdout, "\t%d", value);
        fprintf(stdout, "\t%zu\n", pos);
    }
    alder_encode_number_destroy(key);
    
    s = alder_hashtable_mcas_large_close(fildes);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to read the end of a table file %s\n",
                fn);
        XFREE(n_nhs);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    
    for (size_t i_kmer = 0; i_kmer < s_kmer; i_kmer++) {
        if (n_kmer[i_kmer] > 0) {
            error_kmer[i_kmer] /= ((double)n_kmer[i_kmer]);
        } else {
            assert(error_kmer[i_kmer] == 0);
        }
    }
    
    if (isSummary) {
        fprintf(stdout, "Kmer size           : %d\n", K);
        fprintf(stdout, "Hash table size     : %llu\n", nh);
        fprintf(stdout, "Number of iteration : %llu\n", ni);
        fprintf(stdout, "Number of partition : %llu\n", np);
        fprintf(stdout, "Number of elements  : %llu\n", tnh);
    }
    
    alder_hashtable_mcas2_query_destroy(query);
    XFREE(n_nhs);
    close(fildes);
    return ALDER_STATUS_SUCCESS;
}
