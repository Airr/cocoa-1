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

//#define _LARGEFILE64_SOURCE /* See feature_test_macros(7) */
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

static BOOL pseudo_mcas3(int count, uint64_t *addressA,
                         uint64_t *oldA, uint64_t *newA, uint64_t *addressB);


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

#pragma mark load

static uint64_t
alder_hashtable_mcas_find_key(alder_hashtable_mcas_t *o,
                              uint64_t *x_p, uint64_t *tries_p,
                              int *isKeyFound_p, int *isEmptyFound_p,
                              uint64_t *key)
{
    /* 1. Find the key or an empty key: pos is the position in the key. */
    uint64_t pos=0;
    while (*isEmptyFound_p == ALDER_NO && *isKeyFound_p == ALDER_NO) {
        pos = *x_p * o->stride;
        *isEmptyFound_p = ALDER_YES;
        for (int i = 0; i < o->stride; i++) {
            if (o->key[pos + i] != ALDER_HASHTABLE_MCAS_EMPTYKEY) {
                *isEmptyFound_p = ALDER_NO;
                break;
            }
        }
        if (*isEmptyFound_p == ALDER_NO) {
            *isKeyFound_p = ALDER_YES;
            for (int i = 0; i < o->stride; i++) {
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
 *  This function searches the table for a key.
 *
 *  @param o       table
 *  @param key     key
 *  @param pos_key position of the key
 *
 *  @return success (if the key is found) or fail
 */
int
alder_hashtable_mcas_search(alder_hashtable_mcas_t *o,
                            uint64_t *key,
                            uint64_t *pos_key)
{
    
    uint64_t x = alder_hash_code01(key, o->stride) % o->size;
    uint64_t x_over_size = libdivide_u64_do(x, &o->fast_size);
    x = x - x_over_size * o->size;
    
    uint64_t pos = 0;
    uint64_t tries = 0;
    int isKeyFound = ALDER_NO;
    int isEmptyFound = ALDER_NO;
    /* 1. Search for the key or an empty key. */
    pos = alder_hashtable_mcas_find_key(o, &x, &tries,
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

/**
 *  This function prints kmers with count larger than the given min_count.
 *
 *  @param o         table
 *  @param min_count minimum count
 *
 *  @return success or fail
 */
int
alder_hashtable_mcas_print(FILE *fp, alder_hashtable_mcas_t *o,
                           uint16_t min_count)
{
    alder_encode_number_t *kmer = alder_encode_number_create_for_kmer(o->k);
    for (uint64_t i = 0; i < o->size; i++) {
        if (o->value[i] > min_count) {
            memcpy(kmer->n, o->key + i * o->stride,
                   sizeof(*o->key) * o->stride);
            alder_encode_number_print_in_DNA(fp, kmer);
            fprintf(fp, "\t%d\n", o->value[i]);
        }
    }
    alder_encode_number_destroy(kmer);
    return ALDER_STATUS_SUCCESS;
}


/**
 *  This function reads a partition table in a file.
 *
 *  @param fn table file name
 *  @param ni index of ni
 *  @param np index of np
 *
 *  @return table
 */
alder_hashtable_mcas_t*
alder_hashtable_mcas_read_table(const char *fn, uint64_t i_ni, uint64_t i_np)
{
    int s;
    /* Read the header and find the position in the file to read the table. */
    long fpos;
    int K;
    uint64_t nh;
    uint64_t ni;
    uint64_t np;
    uint64_t tnh;
    uint64_t *n_nhs = NULL;
    s = alder_hashtable_mcas_read_headerFILE(fn, &fpos, &K, &nh,
                                             &ni, &np, &tnh, &n_nhs);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(ALDER_ERR_FILE, "failed to open a table file %s: %s\n",
                   fn, strerror(errno));
        return NULL;
    }
    assert(n_nhs != NULL);
    
    alder_hashtable_mcas_t *o = alder_hashtable_mcas_create(K, nh);
    if (o == NULL) {
        alder_loge(ALDER_ERR, "failed to create a table using file %s", fn);
        return NULL;
    }
    
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        alder_loge(ALDER_ERR_FILE, "failed to open file %s: %s",
                   fn, strerror(errno));
        alder_hashtable_mcas_destroy(o);
        return NULL;
    }
    
    /* Position the current to the start of the partition. */
    uint64_t nc = UINT16_MAX;
    size_t n_byte_entry = alder_hashtable_mcas_table_entry_sizeof(K, nc, nh);
    uint64_t i_nx = i_np + i_ni * np;
    uint64_t n_entry = 0;
    for (uint64_t i = 0; i < i_nx; i++) {
        n_entry += n_nhs[i];
    }
    long pos_nx = fpos + n_entry * n_byte_entry;
    
    s = fseek(fp, pos_nx, SEEK_SET);
    if (s != 0) {
        alder_loge(ALDER_ERR_FILE, "failed to read file %s: %s",
                   fn, strerror(errno));
        alder_hashtable_mcas_destroy(o);
        return NULL;
    }
    
    /* Read keys. */
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint16_t value;
    uint32_t pos32;
    uint64_t pos;
    long size_key = sizeof(*o->key) * o->stride;
    for (uint64_t i = 0; i < n_nhs[i_nx]; i++) {
        size_t nitem = fread(key->n, size_key, 1, fp);
        nitem += fread(&value, sizeof(value), 1, fp);
        if (nh <= UINT32_MAX) {
            nitem += fread(&pos32, sizeof(pos32), 1, fp);
            pos = (uint64_t)pos32;
        } else {
            nitem += fread(&pos, sizeof(pos), 1, fp);
        }
        if (nitem != 3) {
            alder_loge(ALDER_ERR, "failed to read a key in file %s", fn);
            alder_hashtable_mcas_destroy(o);
            return NULL;
        }
        memcpy(o->key + pos * o->stride, key->n, sizeof(*o->key) * o->stride);
        o->value[pos] = value;
    }
    
    XFREE(n_nhs);
    return o;
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
 *  This function computes the memory size for an entry in a table.
 *
 *  @param K  kmer size
 *  @param nc maximum count
 *  @param nh table size
 *
 *  @return size of an entry in a table in byte
 */
size_t
alder_hashtable_mcas_table_entry_sizeof(int K, uint64_t nc, uint64_t nh)
{
    int n_uint64_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    size_t size_value = 0;
    if (nc <= UINT8_MAX) {
        size_value = sizeof(uint8_t);
    } else if (nc <= UINT16_MAX) {
        size_value = sizeof(uint16_t);
    } else {
        size_value = sizeof(uint32_t);
    }
    size_t size_index = 0;
    if (nh <= UINT32_MAX) {
        size_index = sizeof(uint32_t);
    } else {
        size_index = sizeof(uint64_t);
    }
    size_t s = n_uint64_key * sizeof(uint64_t) + size_value + size_index;
    return s;
}


/**
 *  This function reads the header of a dsk table.
 *
 *  @param fn      table file name
 *  @param K_p     kmer size
 *  @param nh_p    nh    - the original table size
 *  @param ni_p    ni    - number of iteration
 *  @param np_p    np    - number of partitition
 *  @param tnh_p   tnh   - total number of entries
 *  @param n_nhs_p n_nhs - number of entries each partition table
 *
 *  @return SUCCESS or FAIL
 */
int
alder_hashtable_mcas_read_headerFILE(const char *fn, long *fpos,
                                     int *K_p, uint64_t *nh_p,
                                     uint64_t *ni_p, uint64_t *np_p,
                                     uint64_t *tnh_p, uint64_t **n_nhs_p)
{
    size_t n = 0;
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        alder_loge(ALDER_ERR_FILE, "failed to open file %s: %s",
                   fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    n = fread(K_p, sizeof(*K_p), 1, fp);
    if (n != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to read K in file %s", fn);
        return ALDER_STATUS_ERROR;
    }
    n = fread(nh_p, sizeof(*nh_p), 1, fp);
    if (n != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to read nh in file %s", fn);
        return ALDER_STATUS_ERROR;
    }
    n = fread(ni_p, sizeof(*ni_p), 1, fp);
    if (n != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to read ni in file %s", fn);
        return ALDER_STATUS_ERROR;
    }
    n = fread(np_p, sizeof(*np_p), 1, fp);
    if (n != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to read np in file %s", fn);
        return ALDER_STATUS_ERROR;
    }
    n = fread(tnh_p, sizeof(*tnh_p), 1, fp);
    if (n != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to read tnh in file %s", fn);
        return ALDER_STATUS_ERROR;
    }
    
    *n_nhs_p = malloc(sizeof(**n_nhs_p) * *ni_p * *np_p);
    if (*n_nhs_p == NULL) {
        alder_loge(ALDER_ERR_FILE, "failed to read n_nhs in file %s", fn);
        return ALDER_STATUS_ERROR;
    }
    n = fread(*n_nhs_p, sizeof(**n_nhs_p) * (*ni_p * *np_p), 1, fp);
    if (n != 1) {
        alder_loge(ALDER_ERR_FILE, "failed to read n_nhs in file %s", fn);
        return ALDER_STATUS_ERROR;
    }
    
    uint64_t t_nh = 0;
    for (uint64_t i = 0; i < (*ni_p) * (*np_p); i++) {
//        alder_log("%*s %llu", width, "nh(%llu):", i, (*n_nhs_p)[i]);
        t_nh += (*n_nhs_p)[i];
    }
    if (t_nh != *tnh_p) {
        alder_loge(ALDER_ERR_FILE,
                   "tnh and sum of (ni x np)-many n_nhs values are different.");
        return ALDER_STATUS_ERROR;
    }
    
    *fpos = ftell(fp);
    XFCLOSE(fp);
    return ALDER_STATUS_SUCCESS;
}

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

