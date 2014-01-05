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

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "alder_cmacro.h"
#include "alder_hash.h"
#include "alder_mcas_wf.h"
#include "alder_encode.h"
#include "alder_hashtable_mcas.h"

static int
alder_hashtable_mcas_large_open(const char *fn, int *fildes,
                                int *K, int *S, uint64_t *Ni, uint64_t *Np);

static int
alder_hashtable_mcas_large_next(alder_hashtable_mcas_t *o, int fildes);

static int
alder_hashtable_mcas_next(alder_hashtable_mcas_t *o,
                          alder_encode_number_t *key,
                          uint16_t *value);
static int
alder_hashtable_mcas_push(alder_hashtable_mcas_t *o,
                          alder_encode_number_t *key,
                          uint16_t value);

void
alder_hashtable_mcas_large_close(int fildes);
/* n is the size of the array on the current line.
 */
struct alder_hashtable_mcas_struct {
    int k;                      /* k-mer size                           */
    int stride;                 /* stride in key                        */
    size_t size;                /* number of elements in the hash table */
    size_t index;               /* current index for iteration          */
    uint64_t *empty_key;        /* n: stride - EMPTY key                */
    uint64_t *key;              /* n: size x stride                     */
    uint16_t *value;            /* n: size                              */
};

static BOOL pseudo_mcas3(int count, uint64_t *addressA,
                         uint64_t *oldA, uint64_t *newA, uint64_t *addressB);

/* This function initializes variables of hashtable_mcas.
 */
static void
alder_hashtable_mcas_init(alder_hashtable_mcas_t *o, int k, size_t size)
{
    o->k = k;
    o->stride = ALDER_BYTESIZE_KMER(k,ALDER_NUMKMER_8BYTE);
    o->size = size;
    o->empty_key = NULL;
//    o->res_key = NULL;
    o->key = NULL;
    o->value = NULL;
}

/* This function creates an open-addressing hash table.
 *
 * 0x44 is used for an empty element in a hash table.
 * This way, I do not need to set a bit 63rd bit to 1 to denote that the slot
 * is occupied.
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
    o->key = malloc(sizeof(*o->key) * o->size * o->stride);
    o->value = malloc(sizeof(*o->value) * o->size);
    if (o->empty_key == NULL || o->key == NULL || o->value == NULL) {
        alder_hashtable_mcas_destroy(o);
        return NULL;
    }
    memset(o->empty_key, 0x44, sizeof(*o->empty_key) * o->stride);
//    memset(o->res_key, 0, sizeof(*o->res_key) * o->stride);
    memset(o->key, 0x44, sizeof(*o->key) * o->size * o->stride);
    memset(o->value, 0, sizeof(*o->value) * o->size);
    return o;
}

void
alder_hashtable_mcas_destroy(alder_hashtable_mcas_t *o)
{
    if (o != NULL) {
        XFREE(o->empty_key);
//        XFREE(o->res_key);
        XFREE(o->key);
        XFREE(o->value);
        XFREE(o);
    }
}

/* This function resets the hash table.
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

uint64_t
alder_hashtable_mcas_key1(alder_hashtable_mcas_t *o, size_t p)
{
    return o->key[p * o->stride + 1];
}

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

/* This function increments the hash.
 * returns
 * SUCCESS or FAIL
 */
int
alder_hashtable_mcas_increment(alder_hashtable_mcas_t *o,
                               uint64_t *key,
                               uint64_t *res_key,
                               bool isMultithreaded)
{
    uint64_t x = alder_hash_code01(key, o->stride) % o->size;
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
            x = (x + 1) % o->size;
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
            // assert(0);
            // abort();
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
//        if (!(oval == 0 && cval == 0)) {
//            fprintf(stderr, "oval: %d, cval: %d\n", oval, cval);
//        }
//        assert(oval == 0 && cval == 0 && o->value[x] == 1);
    } else {
        // single threaded open addressing hash table operation
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
        o->value[x]++;
    }
    return ALDER_STATUS_SUCCESS;
}

/* This function increments the hash. 
 * returns
 * Number of occurences for the key.
 *
 * WARN: This is not a thread-safe function.
 */
int
alder_hashtable_mcaspseudo_increment(alder_hashtable_mcas_t *o,
                                     uint64_t *key,
                                     uint64_t *res_key)
{
    int s = 0;
    
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


/* This function checks whether it can increments the hash table.
 * returns
 * 1 if it can, otherwise 0.
 *
 * WARN: This is not thread-safe.
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
        return 0;
    } else {
        // only if the original key was empty.
        assert(!memcmp(o->key + x * o->stride, key, sizeof(*key)*o->stride));
        if (succeeded == 1) {
            succeeded = pseudo_mcas3(o->stride, o->key + x * o->stride,
                                     key, o->empty_key, res_key);
            assert(succeeded == 1);
        }
        return 1;
    }
}

/* This function mimics call_mcas3. This is not a thread-safe function.
 * Compare addressA and oldA.
 * If they are the same, addressA is replaced by newA, and addressB is by oldA.
 * Otherwise, addressA stays put, and addressB is replaced by addressA.
 *
 * WARN: This is not thread-safe.
 */
BOOL pseudo_mcas3(int count, uint64_t *addressA, uint64_t *oldA, uint64_t *newA,
                  uint64_t *addressB)
{
    BOOL isSuccess = 1;
    if (memcmp(addressA, oldA, sizeof(*addressA) * count)) {
        isSuccess = 0;
    }
    memcpy(addressB, addressA, sizeof(*addressB) * count);
    if (isSuccess) {
        memcpy(addressA, newA, sizeof(*addressA) * count);
    }
    return isSuccess;
}

/* This funciton searthes the hash table for the key, and returns its value.
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

/* This function returns the number of open slots in the hash table.
 *
 * WARN: This is not thread-safe.
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

/* This function returns the maximum values in the value.
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

/* This function prints the header of a hash table.
 * returns
 * SUCCESS or FAIL
 * The header consists of 
 * 1. 4B: K-mer size
 * 2. 4B: hash table size
 * 3. 8B: number of iterations
 * 4. 8B: number of partitions
 */
int
alder_hashtable_mcas_print_header_with_FILE(FILE *fp,
                                            int kmer_size,
                                            int hashtable_size,
                                            uint64_t n_ni,
                                            uint64_t n_np)
{
    size_t s = 0;
    s = fwrite(&kmer_size, sizeof(kmer_size), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    fwrite(&hashtable_size, sizeof(hashtable_size), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    fwrite(&n_ni, sizeof(n_ni), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    fwrite(&n_np, sizeof(n_np), 1, fp);
    if (s != 1) return ALDER_STATUS_ERROR;
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas_print_with_FILE(alder_hashtable_mcas_t *o, FILE *fp)
{
    size_t s;
    s = fwrite(o->empty_key, sizeof(*o->empty_key),  o->stride, fp);
    if (s != o->stride) {
        return 1;
    }
    s = fwrite(o->key, sizeof(*o->key), o->size * o->stride, fp);
    if (s != o->size * o->stride) {
        return 1;
    }
    s = fwrite(o->value, sizeof(*o->value), o->size, fp);
    if (s != o->size) {
        return 1;
    }
    return 0;
}

/* This function prints the hash table.
 * returns
 * the number of kmers (with count).
 */
int64_t
alder_hashtable_mcas_printPackToFILE(alder_hashtable_mcas_t *o, FILE *fp)
{
    int64_t v = 0;
//    int k = o->k;
    size_t stride = o->stride;
    size_t size = o->size;
    uint64_t *empty_key = o->empty_key;
    uint64_t *key = o->key;
    uint16_t *value = o->value;
    alder_encode_number_t *m = alder_encode_number_create_for_kmer_view(o->k);
    for (int i = 0; i < size; i++) {
        // Print only key that is not empty.
        size_t keypos = i * stride;
        if (memcmp(key + keypos, empty_key, sizeof(*key)*stride)) {
            size_t s = fwrite(key + keypos, sizeof(*key), stride, fp);
            if (s != stride) {
                return -1;
            }
            s = fwrite(value + i, sizeof(*value), 1, fp);
            if (s != 1) {
                return -1;
            }
            v++;
        }
    }
    alder_encode_number_destroy_view(m);
    return v;
}

int
alder_hashtable_mcas_printPackToFILE_count(size_t value, FILE *fp)
{
    fseek(fp, 4, SEEK_SET);
    int size = (int)value;
    size_t s = fwrite(&value, sizeof(size), 1, fp);
    if (s != 1) {
        return -1;
    } else {
        return 0;
    }
}


/* This function prints the content of a hash table.
 * returns
 * number of empty slots in the hash table
 */
size_t alder_hashtable_mcas_print(alder_hashtable_mcas_t *o, FILE *fp)
{
    size_t v = 0;
    alder_encode_number_t *m = alder_encode_number_create_for_kmer_view(o->k);
    for (int i = 0; i < o->size; i++) {
        // Print only key that is not empty.
        if (memcmp(o->key + i * o->stride, o->empty_key, sizeof(*o->key)*o->stride)) {
            // Print the key and its value.
            m->n = o->key + i * o->stride;
            alder_encode_number_print_in_DNA(fp, m);
            fprintf(fp, "\t");
            alder_encode_number_print_in_revDNA(fp, m);
            fprintf(fp, "\t");
            fprintf(fp, "%u\n", o->value[i]);
        } else {
            v++;
        }
    }
    alder_encode_number_destroy_view(m);
    return v;
}

/* 
 */
static ssize_t
read_large_bytes_kmer_file(alder_hashtable_mcas_t *o, size_t bufsize, int fildes)
{
    // Kmer sequences
    size_t keysize = (sizeof(*o->key) * o->stride);
    bufsize = bufsize / keysize * keysize;
    ssize_t totalRead = 0;
    ssize_t r;
    size_t nbyte = keysize * o->size;
    
    size_t cbyte = 0;
    size_t rbyte = bufsize;
    size_t sbyte = 0;
    sbyte = rbyte < nbyte ? bufsize : nbyte;
    
    r = read(fildes, o->key, sbyte);
    
    if (r != sbyte && r == -1) {
        fprintf(stderr, "Error - %s\n", strerror(errno));
    }
    totalRead += r;
    while (rbyte < nbyte) {
        cbyte = rbyte;
        rbyte += bufsize;
        sbyte = rbyte < nbyte ? bufsize : nbyte - cbyte;
        
        r = read(fildes, o->key + cbyte/sizeof(*o->key), sbyte);
        
        if (r != sbyte && r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
        }
        totalRead += r;
    }
    assert(totalRead == (ssize_t)nbyte);
    return r;
}

static ssize_t
read_large_bytes_count_file(alder_hashtable_mcas_t *o, size_t bufsize, int fildes)
{
    // Count sequences
    size_t keysize = sizeof(*o->value);
    bufsize = bufsize / keysize * keysize;
    ssize_t totalRead = 0;
    ssize_t r;
    size_t nbyte = sizeof(*o->value) * o->size;
    
    size_t cbyte = 0;
    size_t rbyte = bufsize;
    size_t sbyte = 0;
    sbyte = rbyte < nbyte ? bufsize : nbyte;
    
    r = read(fildes, o->value, sbyte);
    if (r != sbyte && r == -1) {
        fprintf(stderr, "Error - %s\n", strerror(errno));
    }
    totalRead += r;
    while (rbyte < nbyte) {
        cbyte = rbyte;
        rbyte += bufsize;
        sbyte = rbyte < nbyte ? bufsize : nbyte - cbyte;
        
        r = read(fildes, o->value + cbyte/sizeof(*o->value), sbyte);
        if (r != sbyte && r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
        }
        totalRead += r;
    }
    assert(totalRead == (ssize_t)nbyte);
    return r;
}


/* This function loads a hash table from a file.
 * returns
 * alder_hash_table_mcas_t on success
 * nil if an error occurs.
 * 
 * The hash table header consists of 
 * kmer size, hash table size, #_iteration, #_partition.
 */
alder_hashtable_mcas_t * alder_hashtable_mcas_load(const char *fn,
                                                   int isSummary)
{
    int fildes = open(fn, O_RDONLY);
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return NULL;
    }
    int kmer_size;
    int hashtable_size;
    uint64_t n_ni;
    uint64_t n_np;
    read(fildes, &kmer_size, sizeof(kmer_size));
    read(fildes, &hashtable_size, sizeof(hashtable_size));
    read(fildes, &n_ni, sizeof(n_ni));
    read(fildes, &n_np, sizeof(n_np));
    int stride = ALDER_BYTESIZE_KMER(kmer_size,ALDER_NUMKMER_8BYTE);
    uint64_t *empty_key = malloc(sizeof(*empty_key) * stride);
    memset(empty_key,0,sizeof(*empty_key) * stride);
    size_t bufsize = 1 << 23;
    
    alder_hashtable_mcas_t *o = alder_hashtable_mcas_create(kmer_size,
                                                            hashtable_size);
    size_t n_empty_slot = 0;
    for (uint64_t i_ni = 0; i_ni < n_ni; i_ni++) {
        for (uint64_t i_np = 0; i_np < n_np; i_np++) {
            // Read a hash table.
            ssize_t r = read(fildes, empty_key, sizeof(*empty_key) * stride);
            if (r != sizeof(*empty_key) * stride) {
                if (r == -1) {
                    fprintf(stderr, "Error - %s\n", strerror(errno));
                }
            }
            
            r = read_large_bytes_kmer_file(o, bufsize, fildes);
            r = read_large_bytes_count_file(o, bufsize, fildes);
            
            // Print or return it.
            n_empty_slot += alder_hashtable_mcas_print(o, stdout);
        }
    }
//    ssize_t r = read(fildes, empty_key, 1);
//    assert(r == 0);
    
    XFREE(empty_key);
    alder_hashtable_mcas_destroy(o);
    close(fildes);
    
    if (isSummary) {
        size_t n_total_slot = (size_t)hashtable_size * n_ni * n_np;
        fprintf(stdout, "Kmer size           : %d\n", kmer_size);
        fprintf(stdout, "Hash table size     : %d\n", hashtable_size);
        fprintf(stdout, "Number of iteration : %llu\n", n_ni);
        fprintf(stdout, "Number of partition : %llu\n", n_np);
        fprintf(stdout, "Empty/Total slots = %zu/%zu (%0.1f%%)\n",
                n_empty_slot, n_total_slot, (double)n_empty_slot * 100/(double)n_total_slot);
    }
    return NULL;
}

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

/* This funciton searches the hash table in the file for the query.
 * returns
 * 0 on success
 * 1 otherwise
 */
int
alder_hashtable_mcas_query(const char *fn,
                           const char *query)
{
    int fildes = open(fn, O_RDONLY);
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    int kmer_size;
    int hashtable_size;
    uint64_t n_ni;
    uint64_t n_np;
    read(fildes, &kmer_size, sizeof(kmer_size));
    read(fildes, &hashtable_size, sizeof(hashtable_size));
    read(fildes, &n_ni, sizeof(n_ni));
    read(fildes, &n_np, sizeof(n_np));
    
    bstring bq = bfromcstr(query);
    if (bq->slen != kmer_size) {
        return ALDER_STATUS_ERROR;
    }
    
    alder_encode_number_t *sq = alder_encode_number_create_for_kmer(kmer_size);
    alder_encode_number_kmer_with_char(sq, (char*)bq->data);
    alder_encode_number_t *s2 = alder_encode_number_create_rev(sq);
    alder_encode_number_t *ss = NULL;
    uint64_t i_ni = 0;
    uint64_t i_np = 0;
    alder_hashtable_mcas_select(&ss, &i_ni, &i_np, sq, s2,
                                n_ni, n_np);
    
    // skip
    int stride = ALDER_BYTESIZE_KMER(kmer_size,ALDER_NUMKMER_8BYTE);
    uint64_t *empty_key = malloc(sizeof(*empty_key) * stride);
    memset(empty_key,0,sizeof(*empty_key) * stride);
    size_t bufsize = 1 << 23;
    
    alder_hashtable_mcas_t *o = alder_hashtable_mcas_create(kmer_size,
                                                            hashtable_size);
    //
    size_t hash_element_size = (stride * sizeof(uint64_t) +
                                (sizeof(uint16_t)));
    size_t offset = (sizeof(kmer_size) + sizeof(hashtable_size) +
                     sizeof(n_ni) + sizeof(n_np) +
                     hash_element_size * hashtable_size * (i_ni * n_np + i_np));
    //
    off_t s_f = lseek(fildes, offset, SEEK_SET);
    
    // Read a hash table.
    ssize_t r = read(fildes, empty_key, sizeof(*empty_key) * stride);
    if (r != sizeof(*empty_key) * stride) {
        if (r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
        }
    }
    r = read_large_bytes_kmer_file(o, bufsize, fildes);
    r = read_large_bytes_count_file(o, bufsize, fildes);
    // Find the key and return the value.
    
    int value = -1;
    alder_hashtable_mcaspseudo_find(o, ss->n, &value);
    fprintf(stderr, "%d\n", value);
    
    
    XFREE(empty_key);
    alder_hashtable_mcas_destroy(o);
    close(fildes);
    alder_encode_number_destroy(s2);
    alder_encode_number_destroy(sq);
    bdestroy(bq);
    return ALDER_STATUS_SUCCESS;
}

/* This function reads the kmer part in a hash table.
 */
static int
count_read_kmer(size_t *v, int K, size_t hashtablesize, int fildes)
{
    *v = 0;
    size_t v_c = 0;
    
    // Kmer sequences
    int stride = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    
    // Empty key
    uint64_t *empty_key = malloc(sizeof(empty_key) * stride);
    if (empty_key == NULL) {
        return ALDER_STATUS_ERROR;
    }
    memset(empty_key, 0x44, sizeof(*empty_key) * stride);
    
    // Prepare a buffer.
    uint64_t *key = NULL;
    size_t bufsize = 1 << 23;
    size_t keysize = sizeof(*key) * stride;
    bufsize = bufsize / keysize * keysize;
    key = malloc(bufsize);
    memset(key, 0, bufsize);
    assert(bufsize % keysize == 0);
    
    // Start reading.
    ssize_t totalRead = 0;
    size_t nbyte = hashtablesize * keysize;
    size_t rbyte = 0;
    
//    size_t cbyte = 0;
//    size_t rbyte = bufsize;
//    size_t sbyte = 0;
//    sbyte = rbyte < nbyte ? bufsize : nbyte;
//    r = read(fildes, key, sbyte);
//    if (r != sbyte && r == -1) {
//        fprintf(stderr, "Error - %s\n", strerror(errno));
//    }
//    assert(r % keysize == 0);
//    size_t count_key = r / keysize;
//    for (int i = 0; i < count_key; i++) {
//        // Print only key that is not empty.
//        if (memcmp(key + i * stride, empty_key, sizeof(*key)*stride)) {
//            // Print the key and its value.
//            v_c++;
//        }
//    }
//    totalRead += r;
    
    while (rbyte < nbyte) {
        size_t cbyte = rbyte;
        rbyte += bufsize;
        size_t sbyte = rbyte < nbyte ? bufsize : nbyte - cbyte;
        
        ssize_t r = read(fildes, key, sbyte);
        if (r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
            break;
        }
        assert(r >= 0);
        assert(r % keysize == 0);
        size_t count_key = r / keysize;
        for (int i = 0; i < count_key; i++) {
            // Print only key that is not empty.
            if (memcmp(key + i * stride, empty_key, sizeof(*key)*stride)) {
                // Print the key and its value.
                v_c++;
            }
        }
        
        // See the key.
        totalRead += r;
    }
    assert(totalRead == (ssize_t)nbyte);
   
    XFREE(empty_key);
    XFREE(key);
    *v = v_c;
    return ALDER_STATUS_SUCCESS;
}

/* This function reads the value (count) parts in a hash table.
 */
static int
count_read_count(size_t *v, int K, size_t hashtablesize, int fildes)
{
    // Kmer sequences
    int stride = 1;
    
    // Prepare a buffer.
    uint16_t *value = NULL;
    size_t bufsize = 1 << 23;
    size_t valuesize = sizeof(*value) * stride;
    bufsize = bufsize / valuesize * valuesize;
    value = malloc(bufsize);
    memset(value, 0, bufsize);
    assert(bufsize % valuesize == 0);
    
    // Start reading.
    ssize_t totalRead = 0;
    ssize_t r;
    size_t nbyte = hashtablesize * valuesize;
    
    size_t cbyte = 0;
    size_t rbyte = bufsize;
    size_t sbyte = 0;
    sbyte = rbyte < nbyte ? bufsize : nbyte;
    r = read(fildes, value, sbyte);
    if (r != sbyte && r == -1) {
        fprintf(stderr, "Error - %s\n", strerror(errno));
    }
    totalRead += r;
    
    while (rbyte < nbyte) {
        cbyte = rbyte;
        rbyte += bufsize;
        sbyte = rbyte < nbyte ? bufsize : nbyte - cbyte;
        
        r = read(fildes, value, sbyte);
        if (r != sbyte && r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
            break;
        }
        assert(r % valuesize == 0);
//        size_t count_value = r / valuesize;
        
//        for (int i = 0; i < count_value; i++) {
//            // Print value that is not empty.
//        }
        
        // See the key.
        totalRead += r;
    }
    assert(totalRead == (ssize_t)nbyte);
   
    XFREE(value);
//    *v = v_c;
    return ALDER_STATUS_SUCCESS;
}

/* This function counts non-empty slots in a hash table.
 *
 */
int
alder_hashtable_mcas_count(const char *fn,
                           size_t *volumn)
{
    int fildes = open(fn, O_RDONLY);
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    int kmer_size;
    int hashtable_size;
    uint64_t n_ni;
    uint64_t n_np;
    read(fildes, &kmer_size, sizeof(kmer_size));
    read(fildes, &hashtable_size, sizeof(hashtable_size));
    read(fildes, &n_ni, sizeof(n_ni));
    read(fildes, &n_np, sizeof(n_np));
    int stride = ALDER_BYTESIZE_KMER(kmer_size,ALDER_NUMKMER_8BYTE);
    uint64_t *empty_key = malloc(sizeof(*empty_key) * stride);
    memset(empty_key,0,sizeof(*empty_key) * stride);
//    size_t bufsize = 1 << 23;
    
    // Count non-empty slots.
    size_t n_slot = 0;
    for (uint64_t i_ni = 0; i_ni < n_ni; i_ni++) {
        for (uint64_t i_np = 0; i_np < n_np; i_np++) {
            // Read a hash table.
            ssize_t r = read(fildes, empty_key, sizeof(*empty_key) * stride);
            if (r != sizeof(*empty_key) * stride) {
                if (r == -1) {
                    fprintf(stderr, "Error - %s\n", strerror(errno));
                }
            }
            size_t count_key = 0;
            r = count_read_kmer(&count_key, kmer_size, hashtable_size, fildes);
            r = count_read_count(NULL, kmer_size, hashtable_size, fildes);
            n_slot += count_key;
        }
    }
    
    XFREE(empty_key);
    close(fildes);
    
    size_t n_total_slot = (size_t)hashtable_size * n_ni * n_np;
    fprintf(stdout, "Kmer size           : %d\n", kmer_size);
    fprintf(stdout, "Hash table size     : %d\n", hashtable_size);
    fprintf(stdout, "Number of iteration : %llu\n", n_ni);
    fprintf(stdout, "Number of partition : %llu\n", n_np);
    fprintf(stdout, "Slots/Total slots = %zu/%zu (%0.1f%%)\n",
            n_slot, n_total_slot, (double)n_slot * 100/(double)n_total_slot);
    
    *volumn = n_slot;
    return ALDER_STATUS_SUCCESS;
}

/* This function opens a hash table file and reads the header.
 * It closes the file descriptor.
 */
int
alder_hashtable_mcas_header(const char *fn,
                            int *K, int *S, uint64_t *Ni, uint64_t *Np)
{
    int fildes = open(fn, O_RDONLY);
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    read(fildes, K, sizeof(*K));
    read(fildes, S, sizeof(*S));
    read(fildes, Ni, sizeof(*Ni));
    read(fildes, Np, sizeof(*Np));
    close(fildes);
    return ALDER_STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Functions for command pack in alder-kmer
///////////////////////////////////////////////////////////////////////////////

int
alder_hashtable_mcas_large_convert(alder_hashtable_mcas_t *dst,
                                   alder_hashtable_mcas_t *src,
                                   const char *fn,
                                   uint64_t xi_np,
                                   uint64_t xn_np)
{
    
    // Open the input hash table.
    int K = 0;
    int S = 0;
    uint64_t ni = 0;
    uint64_t np = 0;
    int fildes;
    int s = alder_hashtable_mcas_large_open(fn, &fildes, &K, &S, &ni, &np);
    if (s != ALDER_STATUS_SUCCESS) {
        return s;
    }
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer_view(K);
    
    // Reset the x hash table.
    alder_hashtable_mcas_reset(dst);
    
    for (size_t i_ni = 0; i_ni < ni; i_ni++) {
        for (size_t i_np = 0; i_np < np; i_np++) {
            // Reset the main hash table.
            alder_hashtable_mcas_reset(src);
            // Extract each hash table over ni x np.
            alder_hashtable_mcas_large_next(src, fildes);
            
            // Read each key and find its place in the current table,
            // if the key belongs to this partition.
            // Find each element and put it if it is in the current partition.
            uint16_t value;
            alder_hashtable_mcas_next(src, key, &value);
            while (value != 0) {
                uint64_t w_np = alder_encode_number_hash(key) % xn_np;
                if (w_np == xi_np) {
                    alder_hashtable_mcas_push(dst, key, value);
                }
                alder_hashtable_mcas_next(src, key, &value);
            }
        }
    }
    // Close the input hash table.
    alder_hashtable_mcas_large_close(fildes);
    alder_encode_number_destroy_view(key);

    return s;
}

/* This function opens a hash table file and reads the header.
 * It does NOT close the file pointer.
 */
int
alder_hashtable_mcas_large_open(const char *fn, int *fildes,
                                int *K, int *S, uint64_t *Ni, uint64_t *Np)
{
    *fildes = open(fn, O_RDONLY);
    if (*fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    read(*fildes, K, sizeof(*K));
    read(*fildes, S, sizeof(*S));
    read(*fildes, Ni, sizeof(*Ni));
    read(*fildes, Np, sizeof(*Np));
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas_large_next(alder_hashtable_mcas_t *o, int fildes)
{
    size_t bufsize = 1 << 23;
    // Read a hash table.
    ssize_t r = read(fildes, o->empty_key, sizeof(*o->empty_key) * o->stride);
    if (r != sizeof(*o->empty_key) * o->stride) {
        if (r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
        }
        return ALDER_STATUS_ERROR;
    }
    
    r = read_large_bytes_kmer_file(o, bufsize, fildes);
    r = read_large_bytes_count_file(o, bufsize, fildes);
    
    o->index = 0;
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas_next(alder_hashtable_mcas_t *o,
                          alder_encode_number_t *key,
                          uint16_t *value)
{
    // Start at the current index to check if the key is non-empty, and to
    // return it.
    uint64_t x1 = 1;
    *value = 0;
    while (o->index < o->size) {
        key->n = o->key + o->index * o->stride;
        if((key->n[0] >> 62) & x1) {
            // no code: this key is empty.
        } else {
            *value = o->value[o->index];
            o->index++;
            break;
        }
        o->index++;
    }
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas_push(alder_hashtable_mcas_t *o,
                          alder_encode_number_t *key,
                          uint16_t value)
{
    // Insert the key and value in the hash table.
    size_t tries = 1;
    uint64_t x = alder_hash_code01(key->n, o->stride) % o->size;
    // single threaded open addressing hash table operation
    int isKeyFound = memcmp(o->key + x * o->stride, key->n,
                            sizeof(*key->n) * o->stride);
    int isEmptyFound = memcmp(o->key + x * o->stride, o->empty_key,
                              sizeof(*o->empty_key) * o->stride);
    while (isEmptyFound != 0 && isKeyFound != 0 && tries <= o->size) {
        tries++;
        x = (x + 1) % o->size;
        isKeyFound = memcmp(o->key + x * o->stride, key->n,
                            sizeof(*key->n) * o->stride);
        isEmptyFound = memcmp(o->key + x * o->stride, o->empty_key,
                              sizeof(*o->empty_key) * o->stride);
    }
    if (o->size < tries) {
        return ALDER_STATUS_ERROR;
    }
    if (isEmptyFound == 0) {
        memcpy(o->key + x * o->stride, key->n, sizeof(*key->n) * o->stride);
    }
    o->value[x] = value;
    return ALDER_STATUS_SUCCESS;
}

void
alder_hashtable_mcas_large_close(int fildes)
{
    char c;
    ssize_t r = read(fildes, &c, sizeof(c));
    assert(r == 0);
    
    close(fildes);
}

int
alder_hashtable_mcas_nextFromFile(int fildes,
                                  alder_encode_number_t *key,
                                  uint16_t *value)
{
    size_t keysize = sizeof(*key->n) * key->s;
    ssize_t r = read(fildes, key->n, keysize);
    if (r != keysize) {
        return ALDER_STATUS_ERROR;
    }
    
    r = read(fildes, value, sizeof(*value));
    if (r != sizeof(*value)) {
        return ALDER_STATUS_ERROR;
    }
    
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas_convertCountToHashTable(alder_hashtable_mcas_t *ht,
                                             const char *fn,
                                             uint64_t xi_np,
                                             uint64_t xn_np)
{
    // Read the header.
    int K = 0;
    int S = 0;
    uint64_t ni = 0;
    uint64_t np = 0;
    int fildes;
    int s = alder_hashtable_mcas_large_open(fn, &fildes, &K, &S, &ni, &np);
    
    if (s != ALDER_STATUS_SUCCESS) {
        return s;
    }
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    
    uint16_t value;
    //
    for (size_t i_s = 0; i_s < (size_t)S; i_s++) {
        // Read each element: key and value.
//            alder_hashtable_mcas_large_next(src, fildes);
        alder_hashtable_mcas_nextFromFile(fildes, key, &value);
        
        alder_hashtable_mcas_push(ht, key, value);
        
    }
    alder_hashtable_mcas_large_close(fildes);
    
    alder_encode_number_destroy(key);
    return 0;
}


