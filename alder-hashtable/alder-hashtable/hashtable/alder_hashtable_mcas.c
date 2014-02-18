/**
 * This file, alder_hashtable_mcas.c, is part of alder-hashtable.
 *
 * Copyright 2013,2014 by Sang Chul Choi
 *
 * alder-hashtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-hashtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-hashtable.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _LARGEFILE64_SOURCE /* See feature_test_macros(7) */
#include <sys/types.h>
#include <unistd.h>

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
#include "alder_hashtable_mcas.h"

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

static BOOL pseudo_mcas3(int count, uint64_t *addressA,
                         uint64_t *oldA, uint64_t *newA, uint64_t *addressB);


int
alder_hashtable_mcas_state(alder_hashtable_mcas_t *o)
{
    return o->state;
}

uint64_t
alder_hashtable_mcas_i_np(alder_hashtable_mcas_t *o)
{
    return o->i_np;
}

/**
 *  This function initializes variables of hashtable_mcas.
 *
 *  @param o    hash table
 *  @param k    kmer size
 *  @param size hash table size
 *
 *  @return void
 */
static void
alder_hashtable_mcas_init(alder_hashtable_mcas_t *o, int k, size_t size)
{
    o->k = k;
    o->stride = ALDER_BYTESIZE_KMER(k,ALDER_NUMKMER_8BYTE);
    o->size = size;
    o->empty_key = NULL;
    o->key = NULL;
    o->value = NULL;
    o->fast_size = libdivide_u64_gen((uint64_t)size);
}

/**
 *  This function creates an open-addressing hash table.
 *
 *  @param k    kmer size
 *  @param size hash table size
 *
 *  @return hash table
 *
 *  0x44 is used for an empty element in a hash table.
 */
alder_hashtable_mcas_t*
alder_hashtable_mcas_create(int k, size_t size)
{
    alder_hashtable_mcas_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    alder_hashtable_mcas_init(o, k, size);
    /* memory */
    o->empty_key = malloc(sizeof(*o->empty_key) * o->stride);
    o->key = malloc(sizeof(*o->key) * o->size * o->stride);   /* bigmem */
    o->value = malloc(sizeof(*o->value) * o->size);           /* bigmem */
    if (o->empty_key == NULL || o->key == NULL || o->value == NULL) {
        alder_hashtable_mcas_destroy(o);
        return NULL;
    }
    memset(o->empty_key, 0x44, sizeof(*o->empty_key) * o->stride);
    alder_hashtable_mcas_reset(o);
    return o;
}

/**
 *  This function uses already allocated memory to assign (not allocate) 
 *  a table to that memory. This function does not accompany a destroy 
 *  function because of no memory allocation.
 *
 *  @param K        kmer size
 *  @param size     table size
 *  @param mem      memory location
 *  @param mem_size available memory size
 *
 *  @return table
 */
alder_hashtable_mcas_t*
alder_hashtable_mcas_createWithMemory(int K, size_t size,
                                      void *mem, size_t mem_size)
{
    size_t required_size = alder_hashtable_mcas_sizeof(K, size);
    if (mem_size != required_size) {
        alder_loge(ALDER_ERR_MEMORY, "Bug: table memory requirement is not met.");
        return NULL;
    }
    void *d = mem;
    alder_log("BEGIN: %p", d);
    
    /* memory */
    alder_hashtable_mcas_t *o = (alder_hashtable_mcas_t*)d;
    alder_log("mem table pointer: %zu", sizeof(*o));
    d += sizeof(*o);
    memset(o, 0, sizeof(*o));
    alder_hashtable_mcas_init(o, K, size);
    o->empty_key = d;
    alder_log("empty_key: %p", d);
    d += sizeof(*o->empty_key) * o->stride;
    o->key = d;
    alder_log("key: %p", d);
    d += sizeof(*o->key) * o->size * o->stride;
    o->value = d;
    alder_log("value: %p", d);
    d += sizeof(*o->value) * o->size;
    alder_log("END: %p", d);
    if (o->empty_key == NULL || o->key == NULL || o->value == NULL) {
        assert(0);
        alder_loge(ALDER_ERR_MEMORY, "Bug: table memory allocation failed.");
        return NULL;
    }
    memset(o->empty_key, 0x44, sizeof(*o->empty_key) * o->stride);
    alder_hashtable_mcas_reset(o);
    return o;
}

/**
 *  This function computes the memmory size of a table.
 *
 *  @param K    kmer size
 *  @param size number of elements
 *
 *  @return memmory size for a table
 */
size_t
alder_hashtable_mcas_sizeof(int K, size_t size)
{
    size_t v = 0;
    alder_hashtable_mcas_t ov;
    int stride = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    // FIXME: check this if there is a problem.
//    v = (sizeof(&ov) + sizeof(ov) +
    v = (sizeof(ov) +
         sizeof(*ov.empty_key) * stride +
         sizeof(*ov.key) * stride * size +
         sizeof(*ov.value) * size);
    
//    uint16_t *value; /* n: size                              */
    
    return v;
}

/**
 *  This function computes the number of elements in a table that would fit to
 *  the given memory size.
 *
 *  @param K        kmer size
 *  @param mem_size memory size
 *
 *  @return number of elements in a table
 */
size_t
alder_hashtable_mcas_size_with_available_memory(int K, size_t mem_size)
{
    size_t v = mem_size;
    alder_hashtable_mcas_t ov;
    int stride = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
//    size_t x = sizeof(ov); 64
    v -= (sizeof(ov) + sizeof(*ov.empty_key) * stride);
    v /= (sizeof(*ov.key) * stride + sizeof(*ov.value));
    return v;
}


/**
 *  This function computes the number of bytes for storing an element in a
 *  table.
 *
 *  @param K    kmer size
 *
 *  @return element size in byte
 */
size_t
alder_hashtable_mcas_element_sizeof(int K)
{
    alder_hashtable_mcas_t ov;
    int stride = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    size_t v = (sizeof(*ov.key) * stride + sizeof(*ov.value));
    return v;
}

/**
 *  This function frees the hash table.
 *
 *  @param o hash table
 */
void
alder_hashtable_mcas_destroy(alder_hashtable_mcas_t *o)
{
    if (o != NULL) {
        XFREE(o->empty_key);
        XFREE(o->key);
        XFREE(o->value);
        XFREE(o);
    }
}

/*
 */
/**
 *  This function resets the hash table. All of the keys are empty, and values
 *  are zero.
 *
 *  @param o hash table
 *
 *  @return SUCESS
 */
int
alder_hashtable_mcas_reset(alder_hashtable_mcas_t *o)
{
    memset(o->key, 0x44, sizeof(*o->key) * o->size * o->stride);
    memset(o->value, 0, sizeof(*o->value) * o->size);
    return ALDER_STATUS_SUCCESS;
}

size_t
alder_hashtable_mcas_size(alder_hashtable_mcas_t *o)
{
    return o->size;
}

int
alder_hashtable_mcas_value(alder_hashtable_mcas_t *o, size_t p)
{
    return o->value[p];
}

uint64_t
alder_hashtable_mcas_key0(alder_hashtable_mcas_t *o, size_t p)
{
    return o->key[p * o->stride];
}

/**
 *  This function returns the key at p as a bstring.
 *
 *  @param o hash table
 *  @param p position
 *
 *  @return bstring of the key
 */
bstring
alder_hashtable_mcas_key(alder_hashtable_mcas_t *o, size_t p)
{
    alder_encode_number_t *m =
    alder_encode_number_create_for_kmer_view(o->k);
    m->n = o->key + p * o->stride;
    bstring bseq = bfromcstr("");
    alder_encode_number_to_bstring(m, bseq);
    alder_encode_number_destroy_view(m);
    return bseq;
}

#pragma mark increment

int
alder_hashtable_mcas_increment_with_hashcode(alder_hashtable_mcas_t *o,
                                             uint64_t *key,
                                             uint64_t x,
                                             uint64_t *res_key,
                                             bool isMultithreaded)
{
//    uint64_t x = alder_hash_code01(key, o->stride) % o->size;
    
//    x = x % o->size;
    
#if !defined(NDEBUG)
    uint64_t y = x % o->size;
#endif
    uint64_t x_over_size = libdivide_u64_do(x, &o->fast_size);
    x = x - x_over_size * o->size;
#if !defined(NDEBUG)
    assert(x == y);
#endif
    
    size_t tries = 1;
    BOOL succeeded = 0;
    if (isMultithreaded) {
        // Find the value at the memory if the mcas call failed
        // or empty_key is not found.
        succeeded = call_mcas3(o->stride, o->key + x * o->stride,
                               o->empty_key, key, res_key);
        if (o->stride > 1 && succeeded == 0) {
            for (size_t i = 0; i < o->stride; i++) {
                res_key[i] = (uint64_t)
                mcas_read_strong((void*)(o->key + x * o->stride + i));
            }
        }
        while (memcmp(res_key, o->empty_key, sizeof(*res_key)*o->stride) &&
               memcmp(res_key, key, sizeof(*res_key)*o->stride) &&
               tries <= o->size) {
            tries++;
//            x = (x + 1) % o->size;
            x++;
            if (x == o->size) {
                x = 0;
            }
            // Find the value at the memory if the mcas call failed
            // or empty_key is not found.
            succeeded = call_mcas3(o->stride, o->key + x * o->stride,
                                   o->empty_key, key, res_key);
            if (o->stride > 1 && succeeded == 0) {
                for (size_t i = 0; i < o->stride; i++) {
                    res_key[i] = (uint64_t)
                    mcas_read_strong((void*)(o->key + x * o->stride + i));
                }
            }
        }
        if (o->size < tries) {
            assert(memcmp(res_key, o->empty_key, sizeof(*res_key)*o->stride));
            assert(memcmp(res_key, key, sizeof(*res_key)*o->stride));
            fprintf(stderr, "Fatal Error - hash table is full!\n");
            return ALDER_STATUS_ERROR;
        }
        
        uint16_t cval = o->value[x];
        uint16_t oval = cval + 1;
        while (cval != oval) {
            oval = cval;
            cval++;
            cval = __sync_val_compare_and_swap(&o->value[x],
                                               (uint16_t)oval, (uint16_t)cval);
        }
    } else {
        // single threaded open addressing hash table operation
#if defined(USEMEMCPY)
        int isKeyFound = memcmp(o->key + x * o->stride, key,
                                sizeof(*key) * o->stride);
        int isEmptyFound = memcmp(o->key + x * o->stride, o->empty_key,
                                  sizeof(*o->empty_key) * o->stride);
        
        while (isEmptyFound != 0 && isKeyFound != 0 && tries <= o->size) {
            tries++;
            x = (x + 1) % o->size;
            isKeyFound = memcmp(o->key + x * o->stride, key,
                                sizeof(*key) * o->stride);
            isEmptyFound = memcmp(o->key + x * o->stride, o->empty_key,
                                  sizeof(*o->empty_key) * o->stride);
        }
        if (o->size < tries) {
            return ALDER_STATUS_ERROR;
        }
        if (isEmptyFound == 0) {
            memcpy(o->key + x * o->stride, key, sizeof(*key) * o->stride);
        }
#else
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
        int isKeyFound = ALDER_YES;
        int i;
        size_t pos = x * o->stride;
        for (i = 0; i < o->stride; i++) {
            if (o->key[pos + i] != key[i]) {
                isKeyFound = ALDER_NO;
                break;
            }
        }
        int isEmptyFound = ALDER_YES;
        for (i = 0; i < o->stride; i++) {
            if (o->key[pos + i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                isEmptyFound = ALDER_NO;
                break;
            }
        }
        
        while (isEmptyFound == ALDER_NO && isKeyFound == ALDER_NO && tries <= o->size) {
            tries++;
//            x = (x + 1) % o->size;
            x++;
            if (x == o->size) {
                x = 0;
            }
            size_t pos = x * o->stride;
            isKeyFound = ALDER_YES;
            for (i = 0; i < o->stride; i++) {
                if (o->key[pos + i] != key[i]) {
                    isKeyFound = ALDER_NO;
                    break;
                }
            }
            isEmptyFound = ALDER_YES;
            for (i = 0; i < o->stride; i++) {
                if (o->key[pos + i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                    isEmptyFound = ALDER_NO;
                    break;
                }
            }
        }
        if (o->size < tries) {
            return ALDER_STATUS_ERROR;
        }
        if (isEmptyFound == ALDER_YES) {
            memcpy(o->key + x * o->stride, key, sizeof(*key) * o->stride);
        }
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
#endif
        o->value[x]++;
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function increments the hash. This is the main function in the hash
 *  table.
 *
 *  @param o               hash table
 *  @param key             key
 *  @param res_key         auxilary key
 *  @param isMultithreaded multi-threaded flag
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas_increment(alder_hashtable_mcas_t *o,
                               uint64_t *key,
                               uint64_t *res_key,
                               bool isMultithreaded)
{
    uint64_t x = alder_hash_code01(key, o->stride) % o->size;
    return alder_hashtable_mcas_increment_with_hashcode(o, key, x, res_key,
                                                        isMultithreaded);
}

/**
 *  This function increments the hash.
 *
 *  WARN: This is not a thread-safe function.
 *
 *  @param o       hash table
 *  @param key     key
 *  @param res_key auxilary key
 *
 *  @return Number of occurences for the key.
 */
int
alder_hashtable_mcaspseudo_increment(alder_hashtable_mcas_t *o,
                                     uint64_t *key,
                                     uint64_t *res_key)
{
    int s = 0;
    
    uint64_t x = alder_hash_code01(key, o->stride) % o->size;
    size_t tries = 1;
//    BOOL succeeded = 0;
    
    tries = 1;
    pseudo_mcas3(o->stride, o->key + x * o->stride,
                 o->empty_key, key, res_key);
    while (memcmp(res_key, o->empty_key, sizeof(*res_key)*o->stride) &&
           memcmp(res_key, key, sizeof(*res_key)*o->stride) &&
           tries <= o->size) {
        tries++;
        x = (x + 1) % o->size;
        pseudo_mcas3(o->stride, o->key + x * o->stride,
                     o->empty_key, key, res_key);
    }
    if (o->size < tries) {
        assert(memcmp(res_key, o->empty_key, sizeof(*res_key)*o->stride));
        assert(memcmp(res_key, key, sizeof(*res_key)*o->stride));
        fprintf(stderr, "Fatal Error - hash table is full!\n");
        assert(0);
        abort();
    }
    
    uint16_t cval = o->value[x];
    uint16_t oval = cval + 1;
    while (cval != oval) {
        oval = cval;
        cval++;
        cval = __sync_val_compare_and_swap(&o->value[x], (uint16_t)oval, (uint16_t)cval);
    }
    s = o->value[x];
    return s;
}

/**
 *  This function checks whether it can increments the hash table.
 *
 *  WARN: This is not thread-safe.
 *
 *  @param o       hash table
 *  @param key     key
 *  @param res_key auxiliary key
 *
 *  @return YES or NO
 */
int
alder_hashtable_mcaspseudo_can_increment(alder_hashtable_mcas_t *o,
                                         uint64_t *key,
                                         uint64_t *res_key)
{
    uint64_t x = alder_hash_code01(key, o->stride) % o->size;
    size_t tries = 1;
    BOOL succeeded = 0;
    
    tries = 1;
    succeeded = pseudo_mcas3(o->stride, o->key + x * o->stride,
                             o->empty_key, key, res_key);
    while (memcmp(res_key, o->empty_key, sizeof(*res_key)*o->stride) &&
           memcmp(res_key, key, sizeof(*res_key)*o->stride) &&
           tries <= o->size) {
        tries++;
        x = (x + 1) % o->size;
        succeeded = pseudo_mcas3(o->stride, o->key + x * o->stride,
                                 o->empty_key, key, res_key);
    }
    if (o->size < tries) {
        assert(memcmp(res_key, o->empty_key, sizeof(*res_key)*o->stride));
        assert(memcmp(res_key, key, sizeof(*res_key)*o->stride));
        return ALDER_NO;
    } else {
        // only if the original key was empty.
        assert(!memcmp(o->key + x * o->stride, key, sizeof(*key)*o->stride));
        if (succeeded == 1) {
            succeeded = pseudo_mcas3(o->stride, o->key + x * o->stride,
                                     key, o->empty_key, res_key);
            assert(succeeded == 1);
        }
        return ALDER_YES;
    }
}

/**
 *  This function mimics call_mcas3. This is not a thread-safe function.
 *  Compare addressA and oldA. If they are the same, addressA is replaced 
 *  by newA, and addressB is by oldA. Otherwise, addressA stays put, and 
 *  addressB is replaced by addressA.
 *
 *  WARN: This is not thread-safe.
 *
 *  @param count    number of elements
 *  @param addressA memory to be updated
 *  @param oldA     old value
 *  @param newA     new value
 *  @param addressB auxiliary value
 *
 *  @return YES or NO
 */
BOOL
pseudo_mcas3(int count, uint64_t *addressA, uint64_t *oldA, uint64_t *newA,
             uint64_t *addressB)
{
    BOOL isSuccess = ALDER_YES;
    if (memcmp(addressA, oldA, sizeof(*addressA) * count)) {
        isSuccess = ALDER_NO;
    }
    memcpy(addressB, addressA, sizeof(*addressB) * count);
    if (isSuccess) {
        memcpy(addressA, newA, sizeof(*addressA) * count);
    }
    return isSuccess;
}

#pragma mark query

/**
 *  This funciton searthes the hash table for the key, and returns its value.
 *
 *  WARN: This is not thread-safe.
 *
 *  @param o     hash table
 *  @param key   key
 *  @param value return value
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcaspseudo_find(alder_hashtable_mcas_t *o,
                                uint64_t *key,
                                int *value)
{
    uint64_t x = alder_hash_code01(key, o->stride) % o->size;
    size_t tries = 1;
    
    // single threaded open addressing hash table operation
    int isKeyFound = memcmp(o->key + x * o->stride, key,
                            sizeof(*key) * o->stride);
    while (isKeyFound != 0 && tries <= o->size) {
        tries++;
        x = (x + 1) % o->size;
        isKeyFound = memcmp(o->key + x * o->stride, key,
                            sizeof(*key) * o->stride);
    }
    if (o->size < tries) {
        return ALDER_STATUS_ERROR;
    }
    *value = o->value[x];
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function returns the number of open slots in the hash table.
 *
 *  WARN: This is not thread-safe.
 *
 *  @param o hash table
 *
 *  @return number of empty slots
 */
int
alder_hashtable_mcaspseudo_number_open_slot(alder_hashtable_mcas_t *o)
{
    int v = 0;
    for (size_t i = 0; i < o->size; i++) {
        if (!memcmp(o->key + i, o->empty_key, sizeof(*o->key) * o->stride)) {
            v++;
        }
    }
    return v;
}

/**
 *  This function returns the maximum values in the value.
 *
 *  WARN: This is not thread-safe.
 *
 *  @param o hash table
 *
 *  @return maximum value
 */
int
alder_hashtable_mcaspseudo_maximum_count(alder_hashtable_mcas_t *o)
{
    uint16_t v = 0;
    for (size_t i = 0; i < o->size; i++) {
        if (v < o->value[i]) {
            v = o->value[i];
        }
    }
    return (int)v;
}

#pragma mark print

/**
 *  This function prints the header of a hash table.
 *
 *  @param fp             file pointer
 *  @param kmer_size      kmer size
 *  @param hashtable_size total number of elements in the hash table
 *  @param n_ni           iterations
 *  @param n_np           partitions
 *
 *  @return SUCCESS or FAIL
 *
 * The header consists of 
 * 1. 4B: K-mer size
 * 2. 8B: size of a hash table that are used for counting
 * 3. 8B: number of iterations (ni)
 * 4. 8B: number of partitions (np)
 * 5. 8B: total number of elements
 * 6. (8 x ni x np B): number of elements in each partition
 */
#define ALDER_HASHTABLE_MCAS_OFFSET_NNH  4
#define ALDER_HASHTABLE_MCAS_OFFSET_NP  20
#define ALDER_HASHTABLE_MCAS_OFFSET_TNH 28
#define ALDER_HASHTABLE_MCAS_OFFSET_ENH 36
int
alder_hashtable_mcas_printHeaderToFILE(FILE *fp,
                                       int kmer_size,
                                       uint64_t n_nh,
                                       uint64_t n_ni,
                                       uint64_t n_np)
{
    size_t s = 0;
    s = fwrite(&kmer_size, sizeof(kmer_size), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    fwrite(&n_nh, sizeof(n_nh), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    fwrite(&n_ni, sizeof(n_ni), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    fwrite(&n_np, sizeof(n_np), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    
    uint64_t t_nh = 0;
    fwrite(&t_nh, sizeof(t_nh), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    
    uint64_t v = 0;
    for (uint64_t i = 0; i < n_ni; i++) {
        for (uint64_t j = 0; j < n_np; j++) {
            fwrite(&v, sizeof(v), 1, fp);
            if (s != 1) return ALDER_STATUS_ERROR;
        }
    }
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas_printHeaderToFD(int fd,
                                     int kmer_size,
                                     uint64_t n_nh,
                                     uint64_t n_ni,
                                     uint64_t n_np)
{
    uint64_t t_nh = 0;
    uint64_t v = 0;
    ssize_t s = 0;
    s = write(fd, &kmer_size, sizeof(kmer_size));
    if (s != sizeof(kmer_size)) return ALDER_STATUS_ERROR;
    s = write(fd, &n_nh, sizeof(n_nh));
    if (s != sizeof(n_nh)) return ALDER_STATUS_ERROR;
    s = write(fd, &n_ni, sizeof(n_ni));
    if (s != sizeof(n_ni)) return ALDER_STATUS_ERROR;
    s = write(fd, &n_np, sizeof(n_np));
    if (s != sizeof(n_np)) return ALDER_STATUS_ERROR;
    s = write(fd, &t_nh, sizeof(t_nh));
    if (s != sizeof(t_nh)) return ALDER_STATUS_ERROR;
    for (uint64_t i = 0; i < n_ni; i++) {
        for (uint64_t j = 0; j < n_np; j++) {
            s = write(fd, &v, sizeof(v));
            if (s != sizeof(v)) return ALDER_STATUS_ERROR;
        }
    }
   
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function prints the body of a hash table.
 *
 *  @param o  hash table
 *  @param fp file pointer
 *
 *  @return number of elements written to the file
 */
int64_t
alder_hashtable_mcas_printPackToFILE(alder_hashtable_mcas_t *o, FILE *fp)
{
    int64_t v = 0;
    size_t stride = o->stride;
    size_t size = o->size;
//    uint64_t *empty_key = o->empty_key;
    uint64_t *key = o->key;
    
    uint16_t *value = o->value;
    alder_encode_number_t *m = alder_encode_number_create_for_kmer_view(o->k);
    
    /* NOTE: I am not sure whether this would be faster.      */
    /*       Two for-loops are the same except the type of i. */
    if (size <= UINT32_MAX) {
        for (uint32_t i = 0; i < size; i++) {
            // Print only key that is not empty.
            size_t keypos = i * stride;
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            int isSomeKey = ALDER_NO;
            
//            int i_stride;
//            for (i_stride = 0; i_stride < o->stride; i_stride++) {
//                
//                if (key[keypos + i_stride] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
//                    isSomeKey = ALDER_YES;
//                    break;
//                }
//            }
            
            if (value[i] > 0) {
                isSomeKey = ALDER_YES;
            }
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
//            if (memcmp(key + keypos, empty_key, sizeof(*key)*stride)) {
            if (isSomeKey == ALDER_YES) {
                size_t s = fwrite(key + keypos, sizeof(*key), stride, fp);
                if (s != stride) {
                    return -1;
                }
                s = fwrite(value + i, sizeof(*value), 1, fp);
                if (s != 1) {
                    return -1;
                }
                s = fwrite(&i, sizeof(i), 1, fp);
                if (s != 1) {
                    return -1;
                }
                v++;
            }
        }
    } else {
        for (size_t i = 0; i < size; i++) {
            // Print only key that is not empty.
            size_t keypos = i * stride;
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            int isSomeKey = ALDER_NO;
//            int i_stride;
//            for (i_stride = 0; i_stride < o->stride; i_stride++) {
//                if (key[keypos + i_stride] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
//                    isSomeKey = ALDER_YES;
//                    break;
//                }
//            }
            if (value[i] > 0) {
                isSomeKey = ALDER_YES;
            }
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
//            if (memcmp(key + keypos, empty_key, sizeof(*key)*stride)) {
            if (isSomeKey == ALDER_YES) {
                size_t s = fwrite(key + keypos, sizeof(*key), stride, fp);
                if (s != stride) {
                    return -1;
                }
                s = fwrite(value + i, sizeof(*value), 1, fp);
                if (s != 1) {
                    return -1;
                }
                s = fwrite(&i, sizeof(i), 1, fp);
                if (s != 1) {
                    return -1;
                }
                v++;
            }
        }
    }
    
    alder_encode_number_destroy_view(m);
    return v;
}

int64_t
alder_hashtable_mcas_printPackToFD(alder_hashtable_mcas_t *o,
                                   int fd,
                                   size_t bufsize)
{
    int64_t v = 0;
    size_t stride = o->stride;
    size_t size = o->size;
    uint64_t *key = o->key;
    uint16_t *value = o->value;
    size_t i_buf = 0;
    size_t size_key = sizeof(*key) * stride;
    size_t size_value = sizeof(*value);
    size_t size_index = sizeof(uint32_t);
    if (size <=  UINT32_MAX) {
        size_index = sizeof(uint32_t);
    } else {
        size_index = sizeof(size_t);
    }
    size_t size_element = size_key + size_value + size_index;
    uint8_t *buf = malloc(bufsize + size_element);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer_view(o->k);
    
    /* NOTE: I am not sure whether this would be faster.      */
    /*       Two for-loops are the same except the type of i. */
    if (size <= UINT32_MAX) {
        for (uint32_t i = 0; i < size; i++) {
            if (i_buf > bufsize) {
                write(fd, buf, i_buf);
                i_buf = 0;
            }
            if (value[i] >= o->min) {
//                assert(value[i] == 1000);
                size_t keypos = i * stride;
                memcpy(buf + i_buf, key + keypos, size_key);
                i_buf += size_key;
                to_uint16(buf + i_buf, 0) = value[i];
                i_buf += size_value;
                to_uint32(buf + i_buf, 0) = i;
                i_buf += size_index;
                v++;
            }
        }
    } else {
        for (size_t i = 0; i < size; i++) {
            if (i_buf > bufsize) {
                write(fd, buf, i_buf);
                i_buf = 0;
            }
            if (value[i] >= o->min) {
                size_t keypos = i * stride;
                memcpy(buf + i_buf, key + keypos, size_key);
                i_buf += size_key;
                to_uint16(buf + i_buf, 0) = value[i];
                i_buf += size_value;
                to_size(buf + i_buf, 0) = i;
                i_buf += size_index;
                v++;
            }
        }
    }
    if (i_buf > 0) {
        write(fd, buf, i_buf);
    }
    
    XFREE(buf);
    alder_encode_number_destroy_view(m);
    return v;
}

/**
 *  This function writes the size of total elements in the table.
 *
 *  @param value 64-bit value
 *  @param fp    file pointer
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas_printPackToFILE_count(size_t value, FILE *fp)
{
    
    fseek(fp, ALDER_HASHTABLE_MCAS_OFFSET_TNH, SEEK_SET);
    size_t s = fwrite(&value, sizeof(value), 1, fp);
    if (s != 1) {
        return ALDER_STATUS_ERROR;
    } else {
        return ALDER_STATUS_SUCCESS;
    }
}

int
alder_hashtable_mcas_printPackToFD_count(size_t value, int fd)
{
    off_t tnh_r = lseek(fd, ALDER_HASHTABLE_MCAS_OFFSET_TNH, SEEK_SET);
    if (tnh_r != ALDER_HASHTABLE_MCAS_OFFSET_TNH) {
        assert(0);
        return ALDER_STATUS_ERROR;
    }
    ssize_t s = write(fd, &value, sizeof(value));
    if (s != sizeof(value)) {
        assert(0);
        return ALDER_STATUS_ERROR;
    }
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas_printPackToFD_np(uint64_t value, int fd)
{
    off_t pos = (ALDER_HASHTABLE_MCAS_OFFSET_NP);
    off_t last = lseek(fd, 0, SEEK_CUR);
    if (last == -1) {
        // Error.
        return ALDER_STATUS_ERROR;
    }
    off_t pos_r = lseek(fd, pos, SEEK_SET);
    assert(pos_r == pos);
    if (pos_r != pos) {
        // Error.
        return ALDER_STATUS_ERROR;
    }
    ssize_t s = write(fd, &value, sizeof(value));
    if (s != sizeof(value)) {
        // Error.
        return ALDER_STATUS_ERROR;
    }
    off_t last_r = lseek(fd, last, SEEK_SET);
    if (last_r != last) {
        // Error.
        return ALDER_STATUS_ERROR;
    }
    return ALDER_STATUS_SUCCESS;
}


/**
 *  This function writes the number of elements in the partition index by
 *  iteration and partition.
 *
 *  @param fp    file pointer
 *  @param value number of elements
 *  @param i_ni  iteration index
 *  @param i_np  partition index
 *  @param n_np  number of partitions in an iteration
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas_printCountPerPartition(FILE *fp, size_t value,
                                            uint64_t i_ni, uint64_t i_np,
                                            uint64_t n_np)
{
    long pos = (ALDER_HASHTABLE_MCAS_OFFSET_ENH +
                (i_ni * n_np + i_np) * sizeof(n_np));
    long last = ftell(fp);
    fseek(fp, pos, SEEK_SET);
    size_t s = fwrite(&value, sizeof(value), 1, fp);
    fseek(fp, last, SEEK_SET);
    if (s != 1) {
        return ALDER_STATUS_ERROR;
    } else {
        return ALDER_STATUS_SUCCESS;
    }
}
int
alder_hashtable_mcas_printCountPerPartitionFD(int fd, size_t value,
                                              uint64_t i_ni, uint64_t i_np,
                                              uint64_t n_np)
{
    off_t pos = (ALDER_HASHTABLE_MCAS_OFFSET_ENH +
                (i_ni * n_np + i_np) * sizeof(n_np));
    off_t last = lseek(fd, 0, SEEK_CUR);
    if (last == -1) {
        // Error.
        return ALDER_STATUS_ERROR;
    }
    off_t pos_r = lseek(fd, pos, SEEK_SET);
    assert(pos_r == pos);
    if (pos_r != pos) {
        // Error.
        return ALDER_STATUS_ERROR;
    }
    ssize_t s = write(fd, &value, sizeof(value));
    if (s != sizeof(value)) {
        // Error.
        return ALDER_STATUS_ERROR;
    }
    off_t last_r = lseek(fd, last, SEEK_SET);
    if (last_r != last) {
        // Error.
        return ALDER_STATUS_ERROR;
    }
    return ALDER_STATUS_SUCCESS;
}


#pragma mark read


/**
 *  This function reads the header of a table file.
 *
 *  @param fn       file name
 *  @param fildes_p file descriptor
 *  @param K_p      kmer
 *  @param nh_p     table size
 *  @param ni_p     iterations
 *  @param np_p     partitions
 *  @param n_nhs_p  sizes for all partitions
 *
 *  @return SUCCESS or FAIL
 */
int
open_table_header(const char *fn, int *fildes_p, int *K_p,
                  uint64_t *nh_p, uint64_t *ni_p, uint64_t *np_p,
                  uint64_t *tnh_p, uint64_t **n_nhs_p)
{
    ssize_t readLen = 0;
    *fildes_p = open(fn, O_RDWR); // O_RDONLY
    if (*fildes_p == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    readLen = read(*fildes_p, &(*K_p), sizeof(*K_p));
    if (readLen != sizeof(*K_p) || *K_p <= 0) {
        fprintf(stderr, "Error - failed to read K in file %s\n", fn);
        close(*fildes_p);
        return ALDER_STATUS_ERROR;
    }
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Table of alder-kmer ***");
    alder_log("%*s %d", width, "K:", *K_p);
    
    readLen = read(*fildes_p, &(*nh_p), sizeof(*nh_p));
    if (readLen != sizeof(*nh_p) || *nh_p == 0) {
        fprintf(stderr, "Error - failed to read Nh in file %s\n", fn);
        close(*fildes_p);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "nh:", *nh_p);
    
    readLen = read(*fildes_p, &(*ni_p), sizeof(*ni_p));
    if (readLen != sizeof(*ni_p) || *ni_p == 0) {
        fprintf(stderr, "Error - failed to read Ni in file %s\n", fn);
        close(*fildes_p);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "ni:", *ni_p);
    
    readLen = read(*fildes_p, &(*np_p), sizeof(*np_p));
    if (readLen != sizeof(*np_p) || *np_p == 0) {
        fprintf(stderr, "Error - failed to read Np in file %s\n", fn);
        close(*fildes_p);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "np:", *np_p);
    
    readLen = read(*fildes_p, &(*tnh_p), sizeof(*tnh_p));
//    if (readLen != sizeof(*tnh_p) || *tnh_p == 0) {
    if (readLen != sizeof(*tnh_p)) {
        fprintf(stderr, "Error - failed to read TNh in file %s\n", fn);
        close(*fildes_p);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "tnh:", *tnh_p);
    
    *n_nhs_p = malloc(sizeof(**n_nhs_p) * *ni_p * *np_p);
    if (*n_nhs_p == NULL) {
        fprintf(stderr,
                "Error - not enough memory in reading a table file %s\n", fn);
        close(*fildes_p);
        return ALDER_STATUS_ERROR;
    }
    ssize_t len = read(*fildes_p, *n_nhs_p, sizeof(*n_nhs_p) * *ni_p * *np_p);
    if (len != sizeof(*n_nhs_p) * *ni_p * *np_p) {
        fprintf(stderr, "Error - no proper header of table file %s: %s\n",
                fn, strerror(errno));
        XFREE(*n_nhs_p);
        close(*fildes_p);
        return ALDER_STATUS_ERROR;
    }
    
    uint64_t t_nh = 0;
    for (uint64_t i = 0; i < (*ni_p) * (*np_p); i++) {
        alder_log("%*s %llu", width, "nh(%llu):", i, (*n_nhs_p)[i]);
        t_nh += (*n_nhs_p)[i];
    }
    if (t_nh != *tnh_p) {
        fprintf(stderr,
                "Error - Nh and sum of (ni x np)-many values are different.\n");
        assert(0);
        XFREE(*n_nhs_p);
        close(*fildes_p);
        return ALDER_STATUS_ERROR;
    }
    
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function loads a hash table from a file.
 *
 *  @param fn        table file name
 *  @param isSummary summary flag; default 0.
 *
 *  @return SUCCESS or FAIL
 *
 *  The hash table header consists of
 *  kmer size (4B), hash table size (8B), #_iteration (8B), #_partition (8B),
 *  #_element in the table for each partition, and data.
 */
int
alder_hashtable_mcas_load(const char *fn, int isSummary)
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
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint16_t value;
    size_t pos;
    int flag_nh64 = 0;
    if (nh <= UINT32_MAX) {
        flag_nh64 = 0;
    } else {
        flag_nh64 = 1;
    }
#if defined(PRINT_TABLE_ID)
    size_t prev_pos = INT32_MAX;
    int i_table = 0;
#endif
    for (uint64_t i_s = 0; i_s < tnh; i_s++) {
        // Read each element: key and value.
        alder_hashtable_mcas_nextFromFile(fildes, key, &value, &pos, flag_nh64);
#if defined(PRINT_TABLE_ID)
        if (prev_pos > pos) {
            fprintf(stdout, "Table: %d\n", i_table++);
        }
        prev_pos = pos;
#endif
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
    
    if (isSummary) {
        fprintf(stdout, "Kmer size           : %d\n", K);
        fprintf(stdout, "Hash table size     : %llu\n", nh);
        fprintf(stdout, "Number of iteration : %llu\n", ni);
        fprintf(stdout, "Number of partition : %llu\n", np);
        fprintf(stdout, "Number of elements  : %llu\n", tnh);
    }
    
    XFREE(n_nhs);
    close(fildes);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function selects one of two strands of a Kmer sequence.
 *
 *  @param ss               Kmer
 *  @param i_ni             iteration
 *  @param i_np             partition
 *  @param s1               Kmer (forward)
 *  @param s2               Kmer (reverse)
 *  @param number_iteration number of iterations
 *  @param number_partition number of partitions
 *
 *  This function is called when encoding Kmers.
 */
void
alder_hashtable_mcas_select(alder_encode_number_t **ss,
                            uint64_t *i_ni,
                            uint64_t *i_np,
                            alder_encode_number_t *s1,
                            alder_encode_number_t *s2,
                            uint64_t number_iteration,
                            uint64_t number_partition)
{
    uint64_t hash_s1 = alder_encode_number_hash(s1);
    uint64_t hash_s2 = alder_encode_number_hash(s2);
    uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
    *ss = (hash_s1 < hash_s2) ? s1 : s2;
    
    *i_ni = hash_ss % number_iteration;
    *i_np = hash_ss / number_iteration % number_partition;
}

void
alder_hashtable_mcas_select2(alder_encode_number2_t **ss,
                             uint64_t *i_ni,
                             uint64_t *i_np,
                             alder_encode_number2_t *s1,
                             alder_encode_number2_t *s2,
                             uint64_t number_iteration,
                             uint64_t number_partition)
{
    uint64_t hash_s1 = alder_encode_number2_hash(s1);
    uint64_t hash_s2 = alder_encode_number2_hash(s2);
    uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
    *ss = (hash_s1 < hash_s2) ? s1 : s2;
    
    *i_ni = hash_ss % number_iteration;
    *i_np = hash_ss / number_iteration % number_partition;
}

/**
 *  This function returns a position in a file.
 *
 *  @param table_size table size
 *  @param h_size     hash table size
 *  @param start      start
 *  @param fildes     file descriptor
 *  @param flag_nh64  flag for position
 *  @param pos        position
 *  @param key        key
 *  @param ni         ni
 *  @param np         np
 *  @param i_ni       i_ni
 *  @param i_np       i_np
 *  @param nh         nh
 *
 *  @return position
 */
size_t
next_position(int fildes,
              uint64_t middle_index,
              uint64_t table_size,
              size_t h_size, off_t start,
              int flag_nh64,
              alder_encode_number_t *key, uint16_t *value,
              uint64_t ni, uint64_t np,
              uint64_t i_ni, uint64_t i_np, uint64_t nh)
{
    /* Search the section by a sort method. */
    int s;
//    off_t middle = start + middle_index * h_size;
//    lseek(fildes, middle, SEEK_SET);
    
    size_t pos;
    s = alder_hashtable_mcas_nextFromFile(fildes, key, value, &pos, flag_nh64);
    assert(s == ALDER_STATUS_SUCCESS);
#if !defined(NDEBUG)
    uint64_t hash_key = alder_encode_number_hash(key);
    uint64_t i_ni_key = hash_key % ni;
    uint64_t i_np_key = hash_key / ni % np;
    assert(i_ni == i_ni_key);
    assert(i_np == i_np_key);
#endif
//    x = hash_key % nh;
    return pos;
}

/**
 *  This funciton searches the hash table in the file for the query.
 *
 *  @param fn    table file name
 *  @param query query kmer sequence
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas_query(const char *fn,
                           const char *query)
{
    int fildes;
    int K;
    uint64_t nh;
    uint64_t ni;
    uint64_t np;
    uint64_t tnh;
    uint64_t *n_nhs = NULL;
    int s = open_table_header(fn, &fildes, &K, &nh, &ni, &np, &tnh, &n_nhs);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
                fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    assert(n_nhs != NULL);
    int flag_nh64 = 0;
    if (nh <= UINT32_MAX) {
        flag_nh64 = 0;
    } else {
        flag_nh64 = 1;
    }
    
    /* Choose a Kmer sequence. */
    bstring bq = bfromcstr(query);
    if (bq->slen != K) {
        fprintf(stderr, "Kmer length is not %d.\n", K);
        XFREE(n_nhs);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_encode_number_t *sq = alder_encode_number_create_for_kmer(K);
    alder_encode_number_kmer_with_char(sq, (char*)bq->data);
    bdestroy(bq);
    alder_encode_number_t *s2 = alder_encode_number_create_rev(sq);
    alder_encode_number_t *ss = NULL;
    uint64_t i_ni = 0;
    uint64_t i_np = 0;
    alder_hashtable_mcas_select(&ss, &i_ni, &i_np, sq, s2, ni, np);
    
    size_t x = alder_encode_number_hash(ss) % nh;
//    x = 104851588;
    alder_log5("kmer: %s", query);
    alder_log5("pos: %zu", x);
    
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    
    /* Find the start and end of i_ni/i_np table. */
    int stride = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    size_t h_size;
    if (nh <= UINT32_MAX) {
        h_size = (stride * sizeof(uint64_t) + (sizeof(uint16_t)) + sizeof(uint32_t));
    } else {
        h_size = (stride * sizeof(uint64_t) + (sizeof(uint16_t)) + sizeof(uint64_t));
    }
    off_t start = ALDER_HASHTABLE_MCAS_OFFSET_ENH + ni * np * 8;
    for (uint64_t i = 0; i < i_ni * np + i_np; i++) {
        start += (n_nhs[i] * h_size);
    }
//    off_t end = start + n_nhs[i_ni * np + i_np] * h_size;
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint64_t table_size = n_nhs[i_ni * np + i_np];
    int64_t start_index = 0;
    int64_t end_index = table_size;
    
    alder_log5("start: %lld", start);
    alder_log5("end_index: %lld", end_index);
    alder_log5("start_index: %lld", start_index);
    
    
    size_t pos = (size_t)-1;
    uint16_t value = 0;
//    if (s != ALDER_STATUS_SUCCESS) {
//        fprintf(stderr, "Error - failed to read an element in query.\n");
//        close(fildes);
//        alder_encode_number_destroy(s2);
//        alder_encode_number_destroy(sq);
//        bdestroy(bq);
//        return ALDER_STATUS_ERROR;
//    }
    
    int64_t middle_index = start_index - 1;
    off_t middle = start;
    lseek(fildes, middle, SEEK_SET);
    alder_log5("middle index: %lld", middle_index);
    alder_log5("middle: %lld", middle);
    while (start_index <= end_index) {
        int64_t new_middle_index = start_index + (end_index - start_index)/2;
        if (new_middle_index == table_size) {
            break;
        }
        middle = (new_middle_index - 1 - middle_index) * h_size;
        lseek(fildes, middle, SEEK_CUR);
        middle_index = new_middle_index;
        alder_log5("middle index: %lld", middle_index);
        alder_log5("middle: %lld", middle);
        pos = next_position(fildes, middle_index,
                            table_size, h_size, start, flag_nh64,
                            key, &value,
                            ni, np, i_ni, i_np, nh);
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
        } else {
            // middle replaces end.
            alder_log5("pos: %lld >= %lld", pos, x);
            end_index = middle_index - 1;
            alder_log5("start_index: %lld (%lld[middle_index] - 1)", start_index, middle_index);
        }
        
    }
    assert(pos < ((size_t)-1));
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    if (x == pos) {
        // key
        // value and pos
        alder_encode_number_print_in_DNA(stderr, ss);
        fprintf(stderr, "\n");
        alder_encode_number_print_in_DNA(stderr, key);
        fprintf(stderr, "\t%d\t", value);
        fprintf(stderr, "%lld\n", middle_index);
        // Now, check the key itself because pos can be the same for different
        // keys.
        size_t tries = 1;
        while (memcmp(key->n, ss->n, key->s * sizeof(*key->n)) &&
               tries <= table_size) {
            tries++;
            middle_index++;
            if (middle_index == table_size) {
                middle_index = 0;
                lseek(fildes, start, SEEK_SET);
            }
            s = alder_hashtable_mcas_nextFromFile(fildes, key, &value, &pos, flag_nh64);
            assert(s == ALDER_STATUS_SUCCESS);
            
            alder_encode_number_print_in_DNA(stderr, key);
            fprintf(stderr, "\t%zu\n", pos);
        }
        if (table_size < tries) {
            assert(0);
        }
//        // Found at middle_index!
//        //
//        lseek(fildes, -h_size, SEEK_CUR);
//        s = alder_hashtable_mcas_updateFile(fildes, key, &value, &pos, flag_nh64);
//        alder_encode_number_print_in_DNA(stderr, key);
//        fprintf(stderr, "\t%d\t", value);
//        fprintf(stderr, "%lld\n", middle_index);
//        fprintf(stderr, "Found!\n");
        
    } else {
        // Not found.
        fprintf(stderr, "Not Found!\n");
    }
    assert(n_nhs != NULL);
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    
    close(fildes);
    XFREE(n_nhs);
    alder_encode_number_destroy(s2);
    alder_encode_number_destroy(sq);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function just closes the input file.
 *
 *  @param fildes file
 *
 *  @return SUCCESS on no data left, FAIL otherwise.
 */
int
alder_hashtable_mcas_large_close(int fildes)
{
    char c;
    ssize_t r = read(fildes, &c, sizeof(c));
    close(fildes);
    if (r == 0) {
        return ALDER_STATUS_SUCCESS;
    } else {
        return ALDER_STATUS_ERROR;
    }
}

/**
 *  This function reads three values from data part in the hash table file.
 *
 *  @param fildes    file
 *  @param key       key
 *  @param value     value
 *  @param pos       position
 *  @param flag_nh64 position (32bit) if 0, position (64bit) otherwise.
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas_nextFromFile(int fildes,
                                  alder_encode_number_t *key,
                                  uint16_t *value,
                                  size_t *pos,
                                  int flag_nh64)
{
    size_t keysize = sizeof(*key->n) * key->s;
    ssize_t r = read(fildes, key->n, keysize);
    if (r != keysize) {
        *value = 0;
        *pos = 0;
        return ALDER_STATUS_ERROR;
    }
    
    r = read(fildes, value, sizeof(*value));
    if (r != sizeof(*value)) {
        *value = 0;
        *pos = 0;
        return ALDER_STATUS_ERROR;
    }
    
    if (flag_nh64 == 0) {
        uint32_t v = 0;
        r = read(fildes, &v, sizeof(v));
        if (r != sizeof(v)) {
            *value = 0;
            *pos = 0;
        }
        *pos = v;
    } else {
        r = read(fildes, pos, sizeof(*pos));
        if (r != sizeof(*pos)) {
            *value = 0;
            *pos = 0;
        }
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function decreases the value at a position in the table.
 *
 *  @param fildes    fildes
 *  @param key       key
 *  @param value     value
 *  @param pos       pos
 *  @param flag_nh64 flag for 64bit table
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas_updateFile(int fildes,
                                alder_encode_number_t *key,
                                uint16_t *value,
                                size_t *pos,
                                int flag_nh64)
{
    size_t keysize = sizeof(*key->n) * key->s;
    ssize_t r = read(fildes, key->n, keysize);
    if (r != keysize) {
        *value = 0;
        *pos = 0;
        return ALDER_STATUS_ERROR;
    }
    
    r = read(fildes, value, sizeof(*value));
    if (r != sizeof(*value)) {
        *value = 0;
        *pos = 0;
        return ALDER_STATUS_ERROR;
    }
    /* Decrease the value by 1. */
    lseek(fildes, -2, SEEK_CUR);
    *value = *value - 1;
    r = write(fildes, value, sizeof(*value));
    if (r != sizeof(*value)) {
        *value = 0;
        *pos = 0;
        return ALDER_STATUS_ERROR;
    }
    
    if (flag_nh64 == 0) {
        uint32_t v = 0;
        r = read(fildes, &v, sizeof(v));
        if (r != sizeof(v)) {
            *value = 0;
            *pos = 0;
        }
        *pos = v;
    } else {
        r = read(fildes, pos, sizeof(*pos));
        if (r != sizeof(*pos)) {
            *value = 0;
            *pos = 0;
        }
    }
    return ALDER_STATUS_SUCCESS;
}


#pragma mark match command

/**
 *  I need a function to search a table for a kmer. I need to update the table.
 *  1. Open a table file.
 *  2. Search the table for a kmer, and update the count or something in the
 *     table.
 *  3. Keep searching until the table is closed.
 *  4. Close it.
 */
int
alder_hashtable_mcas_find_open(const char *fn, int *fildes_p, int *K_p,
                               uint64_t *nh_p, uint64_t *ni_p, uint64_t *np_p,
                               uint64_t *tnh_p, uint64_t **n_nhs_p)
{
    int s = open_table_header(fn, fildes_p, K_p, nh_p, ni_p, np_p, tnh_p, n_nhs_p);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
                fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    return s;
}

/**
 *  This function finds a kmer and update the count of the kmer in the table.
 *
 *  @param fildes file descriptor
 *  @param query  query kmer
 *
 *  @return YES or NO
 */
int
alder_hashtable_mcas_find(int fildes,
                          const char *query,
                          int K,
                          uint64_t nh,
                          uint64_t ni,
                          uint64_t np,
                          uint64_t tnh,
                          uint64_t *n_nhs)
{
    int s = ALDER_STATUS_SUCCESS;
    
//    int s = open_table_header(fn, &fildes, &K, &nh, &ni, &np, &tnh, &n_nhs);
//    if (s != ALDER_STATUS_SUCCESS) {
//        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
//                fn, strerror(errno));
//        return ALDER_STATUS_ERROR;
//    }
    
    assert(n_nhs != NULL);
    int flag_nh64 = 0;
    if (nh <= UINT32_MAX) {
        flag_nh64 = 0;
    } else {
        flag_nh64 = 1;
    }
    
    /* Choose a Kmer sequence. */
    bstring bq = bfromcstr(query);
    if (bq->slen != K) {
        fprintf(stderr, "Kmer length is not %d.\n", K);
        return ALDER_STATUS_ERROR;
    }
    alder_encode_number_t *sq = alder_encode_number_create_for_kmer(K);
    alder_encode_number_kmer_with_char(sq, (char*)bq->data);
    bdestroy(bq);
    alder_encode_number_t *s2 = alder_encode_number_create_rev(sq);
    alder_encode_number_t *ss = NULL;
    uint64_t i_ni = 0;
    uint64_t i_np = 0;
    alder_hashtable_mcas_select(&ss, &i_ni, &i_np, sq, s2, ni, np);
    
    size_t x = alder_encode_number_hash(ss) % nh;
//    x = 104851588;
    
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    
    /* Find the start and end of i_ni/i_np table. */
    int stride = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    size_t h_size;
    if (nh <= UINT32_MAX) {
        h_size = (stride * sizeof(uint64_t) + (sizeof(uint16_t)) + sizeof(uint32_t));
    } else {
        h_size = (stride * sizeof(uint64_t) + (sizeof(uint16_t)) + sizeof(uint64_t));
    }
    off_t start = ALDER_HASHTABLE_MCAS_OFFSET_ENH + ni * np * 8;
    for (uint64_t i = 0; i < i_ni * np + i_np; i++) {
        start += (n_nhs[i] * h_size);
    }
//    off_t end = start + n_nhs[i_ni * np + i_np] * h_size;
    
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint64_t table_size = n_nhs[i_ni * np + i_np];
    int64_t start_index = 0;
    int64_t end_index = table_size;
    
    size_t pos = (size_t)-1;
    uint16_t value = 0;
//    if (s != ALDER_STATUS_SUCCESS) {
//        fprintf(stderr, "Error - failed to read an element in query.\n");
//        close(fildes);
//        alder_encode_number_destroy(s2);
//        alder_encode_number_destroy(sq);
//        bdestroy(bq);
//        return ALDER_STATUS_ERROR;
//    }
    
    int64_t middle_index = start_index - 1;
    off_t middle = start;
    lseek(fildes, middle, SEEK_SET);
    while (start_index <= end_index) {
        int64_t new_middle_index = start_index + (end_index - start_index)/2;
        if (new_middle_index == table_size) {
            break;
        }
        middle = (new_middle_index - 1 - middle_index) * h_size;
        lseek(fildes, middle, SEEK_CUR);
        middle_index = new_middle_index;
        pos = next_position(fildes, middle_index,
                            table_size, h_size, start, flag_nh64,
                            key, &value,
                            ni, np, i_ni, i_np, nh);
        if (pos == x) {
            // Found!
            break;
        } else if (pos < x) {
            // middle replaces start.
            start_index = middle_index + 1;
        } else {
            // middle replaces end.
            end_index = middle_index - 1;
        }
        
    }
    assert(pos < ((size_t)-1));
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    if (x == pos) {
        // key
        // value and pos
//        alder_encode_number_print_in_DNA(stderr, ss);
//        fprintf(stderr, "\n");
//        alder_encode_number_print_in_DNA(stderr, key);
//        fprintf(stderr, "\t%d\t", value);
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
                lseek(fildes, start, SEEK_SET);
            }
            s = alder_hashtable_mcas_nextFromFile(fildes, key, &value, &pos, flag_nh64);
            assert(s == ALDER_STATUS_SUCCESS);
            
//            alder_encode_number_print_in_DNA(stderr, key);
//            fprintf(stderr, "\t%zu\n", pos);
        }
        if (table_size < tries) {
            assert(0);
        }
        // Found at middle_index!
        //
        lseek(fildes, -h_size, SEEK_CUR);
        s = alder_hashtable_mcas_updateFile(fildes, key, &value, &pos, flag_nh64);
//        alder_encode_number_print_in_DNA(stderr, key);
//        fprintf(stderr, "\t%d\t", value);
//        fprintf(stderr, "%lld\n", middle_index);
//        fprintf(stderr, "Found!\n");
        s = ALDER_YES;
        
    } else {
        // Not found.
//        fprintf(stderr, "Not Found!\n");
        s = ALDER_NO;
    }
    assert(n_nhs != NULL);
    /*************************************************************************/
    /*                               SEARCH                                  */
    /*************************************************************************/
    
//    close(fildes);
//    XFREE(n_nhs);
    alder_encode_number_destroy(s2);
    alder_encode_number_destroy(sq);
    return s;
}

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
 *  1. keep adding keys till the table is 70% full,
 *  2. add a new key or update the existent key, and delete an "old" key with
 *     "small" value. 
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
                             int min_value, int max_value)
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
    return o;
}

/**
 *  This function add a key to the 2nd version of a hash table. It fills upto
 *  70% of the table. 
 *
 *  @param o               table
 *  @param key             key
 *  @param x               hash value
 *  @param res_key         extra key
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
    BOOL succeeded = 0;
    int isKeyFound = ALDER_NO;
    int isEmptyFound = ALDER_NO;
    if (isMultithreaded) {
        alder_loge(ALDER_ERR, "No multithreaded implementation");
        assert(0);
        abort();
        
        while (isEmptyFound == ALDER_NO &&
               isKeyFound == ALDER_NO &&
               tries <= o->size) {
            // Find the value at the memory if the mcas call failed
            // or empty_key is not found.
            succeeded = call_mcas3(o->size_key, o->key + x * o->stride,
                                   o->empty_key, key, res_key);
            if (o->size_key > 1 && succeeded == 0) {
                for (size_t i = 0; i < o->size_key; i++) {
                    res_key[i] = (uint64_t)
                    mcas_read_strong((void*)(o->key + x * o->stride + i));
                }
            }
            
            isKeyFound = ALDER_YES;
            for (int i = 0; i < o->size_key; i++) {
                if (res_key[i] != key[i]) {
                    isKeyFound = ALDER_NO;
                    break;
                }
            }
            isEmptyFound = ALDER_YES;
            for (int i = 0; i < o->size_key; i++) {
                if (res_key[i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                    isEmptyFound = ALDER_NO;
                    break;
                }
            }
            tries++;
            x++;
            if (x == o->size) {
                x = 0;
            }
        }
        if (o->size < tries) {
            assert(memcmp(res_key, o->empty_key, sizeof(*res_key)*o->stride));
            assert(memcmp(res_key, key, sizeof(*res_key)*o->stride));
            fprintf(stderr, "Fatal Error - hash table is full!\n");
            return ALDER_STATUS_ERROR;
        }
        
        // Increment the time stamp first.
        // NOTE: timestamp is rough time not synchronized precisely.
        uint64_t cval = o->timestamp;
        uint64_t oval = cval + 0x10000;
        while (cval != oval) {
            oval = cval;
            cval += 0x10000;
            cval = __sync_val_compare_and_swap(&o->timestamp,
                                               (uint64_t)oval,
                                               (uint64_t)cval);
        }
        
        // Update the timestamp and value of the key at x.
        size_t pos = x * o->stride + o->size_key;
        cval = o->key[pos];
        oval = cval + 1;
        while (cval != oval) {
            oval = cval;
            // Change the old value.
            cval &= 0xc00000000000ffff;               // Reset timestampe
            cval |= o->timestamp;                     // Set the new timestamp
            cval++;                                   // Increment the value
            cval = __sync_val_compare_and_swap(o->key + pos,
                                               (uint64_t)oval,
                                               (uint64_t)cval);
        }
    } else {
        /* single-threaded */
        
        /* 1. Find the key or an empty key: pos is the position in the key. */
        while (isEmptyFound == ALDER_NO &&
               isKeyFound == ALDER_NO &&
               tries <= o->size) {
            pos = x * o->stride;
            isEmptyFound = ALDER_YES;
            for (int i = 0; i < o->size_key; i++) {
                if (o->key[pos + i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                    isEmptyFound = ALDER_NO;
                    break;
                }
            }
            if (isEmptyFound == ALDER_NO) {
                isKeyFound = ALDER_YES;
                for (int i = 0; i < o->size_key; i++) {
                    if (o->key[pos + i] != key[i]) {
                        isKeyFound = ALDER_NO;
                        break;
                    }
                }
            }
            if (isEmptyFound == ALDER_NO && isKeyFound == ALDER_NO) {
                tries++;
                x++;
                if (x == o->size) {
                    x = 0;
                }
            }
        }
        if (o->size < tries) {
            alder_loge(ALDER_ERR_HASH, "initial table is full!");
            assert(0);
            abort();
            return ALDER_STATUS_ERROR;
        }
        
        ///////////////////////////////////////////////////////////////////////
        // BEGIN: Set the value.
        ///////////////////////////////////////////////////////////////////////
        *added_x = x;
        uint64_t pointer = x;
        tries++;                                  // Adjust tries to be offset.
        o->timestamp++;                           // Increment timestamp by 1.
        /* Case 1. Empty key. */
        if (isEmptyFound == ALDER_YES && o->pointer != UINT64_MAX) {
            /* Copy the key. */
            memcpy(o->key + pos, key, sizeof(*key) * o->size_key);
            o->n_key++;
            /* Set offset and value. */
            uint64_t v = (tries << 32);
            v++;
            o->key[pos + o->pos_value] = v;
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
        /* Case 2. Found key. */
        else if (isKeyFound == ALDER_YES) {
            /* Set offset and value. */
            uint64_t v = o->key[pos + o->pos_value];  // Get offset+value.
            v &= V32_VALUE_MASK;                      // Reset offset.
            uint64_t c = v;
            v |= (tries << 32);                       // Set the new offset.
            if (c < o->max_value) {                   // Increment value
                c++;                                  // if less than max.
                v++;
            }
            o->key[pos + o->pos_value] = v;           // Finish offset and value.
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
            }
            /* Set timestamp. */
            o->key[pos + o->pos_timestamp] = o->timestamp; // Finish timestamp.
            /* Increment the frequency. */
            if (ALDER_HASHTABLE_SIZE16 <= c) {
                c = ALDER_HASHTABLE_SIZE16 - 1;
            }
            o->frequency[c]++;
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
        ///////////////////////////////////////////////////////////////////////
        // END: Set the value.
        ///////////////////////////////////////////////////////////////////////
        if (o->size7 <= o->n_key) {
            return ALDER_HASHTABLE_FULL;
        }
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
search_old_key(alder_hashtable_mcas2_t *o, uint64_t not_x)
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
        if (o->size <= ALDER_HASHTABLE_SIZE31) {
            pos = o->pointer * o->stride;
            value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
            while (o->min_freq_value < value) {
                /* Increment frequencyL as the pointer moves to the right. */
                o->frequencyL[value]++;
                /* Move to right. */
                pos = (o->key[pos + o->pos_pointerR] & V32_POINTER_R) * o->stride;
                value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
            }
            pointer_delete = pos / o->stride;
        } else {
            pos = o->pointer * o->stride;
            value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
            while (o->min_freq_value < value) {
                /* Increment frequencyL as the pointer moves to the right. */
                o->frequencyL[value]++;
                /* Move to the right. */
                pos = o->key[pos + o->pos_pointerR] * o->stride;
                value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
            }
            pointer_delete = pos / o->stride;
        }
    } else {
        if (o->size <= ALDER_HASHTABLE_SIZE31) {
            //
            assert(n_left_slot > 0);
            /* Move to the left because the current one is not selected. */
            pos = o->pointer * o->stride;
            assert(o->pointer < o->size);
            pos = (o->key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32;
            assert(pos < o->size);
            pos *= o->stride;
            while (n_left_slot > 0) {
                /* Move to the left. */
                pos = (o->key[pos + o->pos_pointerL] & V32_POINTER_L) >> 32;
                assert(pos < o->size);
                pos *= o->stride;
                value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
                /* Decrement frequencyL as the pointer moves to the left. */
                o->frequencyL[value]--;
                if (value <= o->min_freq_value) {
                    n_left_slot--;
                }
            }
            pointer_delete = pos / o->stride;
        } else {
            assert(n_left_slot > 0);
            /* Move to the left because the current one is not selected. */
            pos = o->pointer * o->stride;
            assert(pos < o->size);
            pos = o->key[pos + o->pos_pointerL];
            assert(pos < o->size);
            pos *= o->stride;
            while (n_left_slot > 0) {
                /* Move to the left. */
                pos = o->key[pos + o->pos_pointerL];
                assert(pos < o->size);
                pos *= o->stride;
                value = o->key[pos + o->pos_value] & V16_VALUE_MASK;
                /* Decrement frequencyL as the pointer moves to the left. */
                o->frequencyL[value]--;
                if (value <= o->min_freq_value) {
                    n_left_slot--;
                }
            }
            pointer_delete = pos / o->stride;
        }
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
                                         uint64_t *removed_x,
                                         bool isMultithreaded)
{
    assert(isMultithreaded == false);
    /* Find a key to delete. */
    uint64_t x;
    uint64_t x_free = search_old_key(o, not_x);
    assert(x_free != not_x);
    *removed_x = x_free;
    
    
    int isEmptyFound = ALDER_NO;
    x = x_free;
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
            // FIXME: 0 -> 4 rearrange 0's neighbor node left/right
            //        3-0-1 -> 3-4-1.
            //        0 -> 4
            //        3-0 -> 3-4
            //        0-1 -> 4-1
            //        pointer
            //        pointer_head
            //        pointer_tail
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
    o->pointer = o->pointer_tail;
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
    BOOL succeeded = 0;
    int isKeyFound = ALDER_NO;
    int isEmptyFound = ALDER_NO;
    if (isMultithreaded) {
        assert(0);
        
        while (isEmptyFound == ALDER_NO &&
               isKeyFound == ALDER_NO) {
            // Find the value at the memory if the mcas call failed
            // or empty_key is not found.
            succeeded = call_mcas3(o->size_key, o->key + x * o->stride,
                                   o->empty_key, key, res_key);
            if (o->size_key > 1 && succeeded == 0) {
                for (size_t i = 0; i < o->size_key; i++) {
                    res_key[i] = (uint64_t)
                    mcas_read_strong((void*)(o->key + x * o->stride + i));
                }
            }
            
            isKeyFound = ALDER_YES;
            for (int i = 0; i < o->size_key; i++) {
                if (res_key[i] != key[i]) {
                    isKeyFound = ALDER_NO;
                    break;
                }
            }
            isEmptyFound = ALDER_YES;
            for (int i = 0; i < o->size_key; i++) {
                if (res_key[i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                    isEmptyFound = ALDER_NO;
                    break;
                }
            }
            tries++;
            x++;
            if (x == o->size) {
                x = 0;
            }
        }
        assert(!(o->size < tries));
        
        // Increment the time stamp first.
        // NOTE: timestamp is rough time not synchronized precisely.
        uint64_t cval = o->timestamp;
        uint64_t oval = cval + 0x10000;
        while (cval != oval) {
            oval = cval;
            cval += 0x10000;
            cval = __sync_val_compare_and_swap(&o->timestamp,
                                               (uint64_t)oval,
                                               (uint64_t)cval);
        }
        // Update the timestamp and value of the key at x.
        size_t pos = x * o->stride + o->size_key;
        cval = o->key[pos];
        oval = cval + 1;
        while (cval != oval) {
            oval = cval;
            // Change the old value.
            cval &= 0xc00000000000ffff;               // Reset timestampe
            cval |= o->timestamp;                     // Set the new timestamp
            cval++;                                   // Increment the value
            cval = __sync_val_compare_and_swap(o->key + pos,
                                               (uint64_t)oval,
                                               (uint64_t)cval);
        }
        
        if (isEmptyFound == ALDER_YES) {
            assert(cval == 0);
            // Choose an old key with min_value, and delete it.
            /* NOTE: delete an element from an open-addressing hash table! */
            /*       SINCE THIS IS NOT AN EASY TASK, I NEED TO PUT IT OFF. */
            uint64_t y = alder_rng_uint64(o->size);
            while (1) {
                size_t pos = y * o->stride;
                
                for (size_t i = 0; i < o->stride; i++) {
                    res_key[i] = (uint64_t)
                    mcas_read_strong((void*)(o->key + x * o->stride + i));
                }
                isEmptyFound = ALDER_YES;
                for (int i = 0; i < o->size_key; i++) {
                    if (res_key[pos + i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                        isEmptyFound = ALDER_NO;
                        break;
                    }
                }
                if (isEmptyFound == ALDER_YES) {
                    assert(res_key[pos + o->size_key] == 0);
                    continue;
                }
                if (x == y) {
                    continue;
                } else {
                    // Another key is found
                    // Get the value and timestamp.
                    cval = o->key[pos + o->size_key];
//                    uint64_t timestamp = (cval & 0x3fffffffffff0000) >> 16;
                    uint64_t value = cval & 0x000000000000ffff;
                    
                    if (value < o->min_freq_value) {
//                        timestamp < o->timestamp - o->span_timestamp) {
                        // Delete this key or replace it with the empty key.
                        succeeded = call_mcas3(o->stride,
                                               o->key + y * o->stride,
                                               res_key, o->empty_key,
                                               res_key2);
                        if (o->size_key > 1) {
                            for (size_t i = 0; i < o->stride; i++) {
                                res_key[i] = (uint64_t)
                                mcas_read_strong((void*)(o->key + x * o->stride + i));
                            }
                        }
                        if (succeeded == 0) {
                            isEmptyFound = ALDER_YES;
                            for (int i = 0; i < o->size_key; i++) {
                                if (res_key2[pos + i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                                    isEmptyFound = ALDER_NO;
                                    break;
                                }
                            }
                            if (isEmptyFound == ALDER_YES) {
                                // Successfully replaced with an empty key.
                                break;
                            }
                        }
                    }
                }
                y++;
                if (y == o->size) {
                    y = 0;
                }
            }
        } else {
            assert(cval > o->min_freq_value);
            assert(isKeyFound == ALDER_YES);
        }
    } else {
        /* single-threaded */
        
        /* 1. Find the key or an empty key: pos is the position in the key. */
        while (isEmptyFound == ALDER_NO && isKeyFound == ALDER_NO) {
            pos = x * o->stride;
            isEmptyFound = ALDER_YES;
            for (int i = 0; i < o->size_key; i++) {
                if (o->key[pos + i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                    isEmptyFound = ALDER_NO;
                    break;
                }
            }
            if (isEmptyFound == ALDER_NO) {
                isKeyFound = ALDER_YES;
                for (int i = 0; i < o->size_key; i++) {
                    if (o->key[pos + i] != key[i]) {
                        isKeyFound = ALDER_NO;
                        break;
                    }
                }
            }
            if (isEmptyFound == ALDER_NO && isKeyFound == ALDER_NO) {
                tries++;
                x++;
                if (x == o->size) {
                    x = 0;
                }
            }
        }
        assert(isEmptyFound == ALDER_YES || isKeyFound == ALDER_YES);
        
        ///////////////////////////////////////////////////////////////////////
        // BEGIN: Set the value.
        ///////////////////////////////////////////////////////////////////////
        *added_x = x;
        uint64_t pointer = x;
        tries++;                                  // Adjust tries to be offset.
        o->timestamp++;                           // Increment timestamp by 1.
        assert(tries <= V32_NULL);
        /* If an empty key is found, replace it with the key. */
        if (isEmptyFound == ALDER_YES) {
            /* Set the key and reset the auxiliary part of the key. */
            memcpy(o->key + x * o->stride, key, sizeof(*key) * o->size_key);
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
            
            /* Delete an old key with a small value.       */
            /* This could change tail and middle pointers. */
            alder_hashtable_mcas2_spacesaving_delete(o, x, removed_x, isMultithreaded);
            o->n_key--;
        }
        /* Case 2. Found key. */
        else {
            assert(isKeyFound == ALDER_YES);
            /* Set offset and value. */
            uint64_t v = o->key[pos + o->pos_value];// Get offset+value.
            v &= V32_VALUE_MASK;                    // Reset offset.
            uint64_t c = v;
            
            ///////////////////////////////////////////////////////////////////
            // Frequency can be updated with functions.
            ///////////////////////////////////////////////////////////////////
            /* Change the frequency. */
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


