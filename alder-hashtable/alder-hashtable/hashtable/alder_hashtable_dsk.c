/**
 * This file, alder_hashtable_dsk.c, is part of alder-hashtable.
 *
 * Copyright 2013 by Sang Chul Choi
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
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_encode.h"
#include "alder_hash.h"
#include "alder_hashtable_dsk.h"

/*
 * index?
 */
struct alder_hashtable_dsk_struct {
    int k;                      /* k-mer size                           */
    int stride;                 /* stride in key                        */
    size_t size;                /* number of elements in the hash table */
    uint64_t index;             /* Just added index                     */
    uint64_t *empty_key;        /* n: stride  - EMPTY key               */
    uint64_t *key;              /* n: size x stride                     */
    uint16_t *value;            /* n: size                              */
};

static void
alder_hashtable_dsk_init(alder_hashtable_dsk_t *o,
                         int k,
                         size_t size)
{
    o->k = k;
    o->stride = ALDER_BYTESIZE_KMER(k,ALDER_NUMKMER_8BYTE);
    o->size = size;
    o->empty_key = NULL;
    o->key = NULL;
    o->value = NULL;
}

/* This function creates a hash table (size) for counting kmer of size k.
 * returns
 * alder_hashtable_dsk_t on success
 * nil if an error occurs.
 * given
 * k - kmer size
 * size - hash table size
 * empty_key - a key used as an empty key (the size must be o->stride).
 *
 * 1. If empty_key is NULL, keys are set to 0x3F. The MSB in a byte or any 
 * numbers is used for lock-free programming. The second to the MSB bit is 0
 * if the slot of a hash table is empty. This is very handy because I need MSB
 * for lock-free programming, and I can use one more bit to tell whether a
 * slot is empty or not.
 * ALDER_NUMKMER_8BYTE must be 31; I use 62 bits for encoding a DNA sequence.
 * The remaining two bits are used for other purposes such as lock-free 
 * multi-threaded programming, and for checking whether a slot in the hash
 * table is empty or not. The second to the last bit or the bit next to MSB
 * is set to 0 if the slot with the value is empty. When the slot is occupied,
 * I set it to 1.
 */
alder_hashtable_dsk_t * alder_hashtable_dsk_create(int k,
                                                   size_t size,
                                                   uint64_t *empty_key)
{
    assert(ALDER_NUMKMER_8BYTE == 31);
    alder_hashtable_dsk_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    alder_hashtable_dsk_init(o, k, size);
    /* memory */
    o->empty_key = malloc(sizeof(*o->empty_key) * o->stride);
    o->key = malloc(sizeof(*o->key) * o->size * o->stride);
    o->value = malloc(sizeof(*o->value) * o->size);
    if (o->empty_key == NULL || o->key == NULL || o->value == NULL) {
        alder_hashtable_dsk_destroy(o);
        return NULL;
    }
    // 1
    if (empty_key == NULL) {
        memset(o->empty_key, 0x3F, sizeof(*o->empty_key) * o->stride);
    } else {
        memcpy(o->empty_key, empty_key, sizeof(*o->empty_key) * o->stride);
    }
    memset(o->key,0,sizeof(*o->key) * o->size * o->stride);
    memset(o->value,0,sizeof(*o->value) * o->size);
    return o;
}

/* This function frees the memory for hashtable_dsk_t.
 */
void alder_hashtable_dsk_destroy(alder_hashtable_dsk_t *o)
{
    if (o != NULL) {
        XFREE(o->empty_key);
        XFREE(o->key);
        XFREE(o->value);
        XFREE(o);
    }
}

/* This function resets the key and value.
 * returns
 * SUCCESS or ERROR
 *
 * 1. check if the empty key consists of the same bytes.
 * 2. reset the key.
 * 3. reset the value; the number of elements is size.
 */
int alder_hashtable_dsk_reset(alder_hashtable_dsk_t *o)
{
    // 1
    alder_encode_number_t *s = alder_encode_number_create_for_kmer_view(o->k);
    if (s == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot reset the hash table");
        return ALDER_STATUS_ERROR;
    }
    s->n = o->empty_key;
    int is_same_each_byte = alder_encode_number_is_same_byte(s);
    alder_encode_number_destroy_view(s);
    // 2
    if (is_same_each_byte == 1) {
        // Use memset!
        // First, find the byte.
        int x = 0;
        for (int i = 0; i < 8; i++) {
            if (o->empty_key[0] & (1 << i)) {
                x |= 1 << i;
            }
        }
        assert(x == 0x3F);
        memset(o->key, x, sizeof(*o->key) * o->size * o->stride);
    } else {
        // Use loop!
        uint64_t *ck = o->key;
        for (int i = 0; i < o->size; i++) {
            for (int j = 0; j < o->stride; j++) {
                *(ck + j) = o->empty_key[j];
            }
            ck += o->stride;
        }
    }
    // 3
    memset(o->value, 0, sizeof(*o->value) * o->size);
    return ALDER_STATUS_SUCCESS;
}

/* This function sets the empty key.
 */
void alder_hashtable_dsk_empty_key(alder_hashtable_dsk_t *o,
                                   uint64_t *empty_key)
{
    assert(o != NULL);
    assert(empty_key != NULL);
    memcpy(o->empty_key, empty_key, sizeof(*empty_key)*o->stride);
}

/* This function finds the key in the hash table and increases the value by 1.
 * returns
 * SUCCESS if the value of the slot at the key is incremented,
 * ERROR if the hash table is full.
 *
 * 1.
 */
int alder_hashtable_dsk_increment(alder_hashtable_dsk_t *o, uint64_t *key)
{
    int s = ALDER_STATUS_SUCCESS;
    int stride = o->stride;
    uint64_t *empty_key = o->empty_key;
    uint64_t *okey = o->key;
    uint64_t index = alder_hash_code01(key, stride) % o->size;
    
    /* Search the hash table while the key is different. */
    size_t n_try = 1;
    int is_occupied = memcmp(okey + index * stride, empty_key, 
                             sizeof(*empty_key) * stride);
    int is_not_found = memcmp(okey + index * stride, key,
                              sizeof(*key) * stride);
    while (is_occupied && is_not_found && n_try < o->size) {
        index = (index + 1) % o->size;
        is_occupied = memcmp(okey + index * stride, empty_key,
                             sizeof(*empty_key) * stride);
        is_not_found = memcmp(okey + index * stride, key,
                              sizeof(*key) * stride);
        n_try++;
    }
    if (is_occupied == 0) {
        assert(o->value[index] == 0);
        o->value[index] = 1;
        memcpy(okey + index * stride, key, sizeof(*key)*stride);
        o->index = index;
    } else if (is_not_found == 0) {
        assert(o->value[index] != 0);
        o->value[index]++;
        o->index = index;
    } else {
        // The hash table is full.
        s = ALDER_STATUS_ERROR;
        o->index = -1;
    }
    
    return s;
}

/* This function creates the content of the hash table in a file.
 * returns
 * 0 on success
 * 1 if an error occurs.
 * procedure
 * . print the header of a hash table: 
 *   kmer_size, table_size, #_iteration, and #_partition.
 * See also
 * alder_hashtable_dsk_print_header_with_descriptor
 * alder_hashtable_dsk_print_header_with_FILE
 */
int alder_hashtable_dsk_print_header_with_filename(alder_hashtable_dsk_t *o,
                                                   const char *fn,
                                                   int kmer_size,
                                                   int hashtable_size,
                                                   uint64_t n_ni,
                                                   uint64_t n_np)
{
    int fildes = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_HASHTABLE_ERROR;
    }
    alder_hashtable_dsk_print_header_with_descriptor(o, fildes, kmer_size,
                                                     hashtable_size,
                                                     n_ni, n_np);
    
    close(fildes);
    return ALDER_HASHTABLE_SUCCESS;
}

int alder_hashtable_dsk_print_header_with_descriptor(alder_hashtable_dsk_t *o,
                                                     int fildes,
                                                     int kmer_size,
                                                     int hashtable_size,
                                                     uint64_t n_ni,
                                                     uint64_t n_np)
{
    write(fildes, &kmer_size, sizeof(kmer_size));
    write(fildes, &hashtable_size, sizeof(hashtable_size));
    write(fildes, &n_ni, sizeof(n_ni));
    write(fildes, &n_np, sizeof(n_np));
    return ALDER_HASHTABLE_SUCCESS;
}

int alder_hashtable_dsk_print_header_with_FILE(alder_hashtable_dsk_t *o,
                                               FILE *fp,
                                               int kmer_size,
                                               int hashtable_size,
                                               uint64_t n_ni,
                                               uint64_t n_np)
{
    fwrite(&kmer_size, sizeof(kmer_size), 1, fp);
    fwrite(&hashtable_size, sizeof(hashtable_size), 1, fp);
    fwrite(&n_ni, sizeof(n_ni), 1, fp);
    fwrite(&n_np, sizeof(n_np), 1, fp);
    return ALDER_HASHTABLE_SUCCESS;
}

int alder_hashtable_dsk_print_with_filename(alder_hashtable_dsk_t *o, const char *fn)
{
    int fildes = open(fn, O_WRONLY | O_APPEND, 0644);
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_HASHTABLE_ERROR;
    }
    alder_hashtable_dsk_print_with_descriptor(o, fildes);
    close(fildes);
    return ALDER_HASHTABLE_SUCCESS;
}

/* This function prints the content of the hash table in a file.
 */
int alder_hashtable_dsk_print_with_descriptor(alder_hashtable_dsk_t *o, int fildes)
{
    write(fildes, o->empty_key, sizeof(*o->empty_key) * o->stride);
    write(fildes, o->key, sizeof(*o->key) * o->size * o->stride);
    write(fildes, o->value, sizeof(*o->value) * o->size);
    return 0;
}

/* This function prints the content of the hash table in a file.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_hashtable_dsk_print_with_FILE(alder_hashtable_dsk_t *o, FILE *fp)
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


/* This function prints the content of a hash table.
 */
int alder_hashtable_dsk_print(alder_hashtable_dsk_t *o, FILE *fp)
{
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
        }
    }
    alder_encode_number_destroy_view(m);
    return 0;
}

/* This function loads a hash table from a file.
 * returns
 * alder_hash_table_dsk_t on success
 * nil if an error occurs.
 * 
 * The hash table header consists of 
 * kmer size, hash table size, #_iteration, #_partition.
 */
alder_hashtable_dsk_t * alder_hashtable_dsk_load(const char *fn)
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
    memset(empty_key, 0, sizeof(*empty_key) * stride);
    
    alder_hashtable_dsk_t *o = alder_hashtable_dsk_create(kmer_size,
                                                          hashtable_size,
                                                          NULL);
    for (uint64_t i_ni = 0; i_ni < n_ni; i_ni++) {
        for (uint64_t i_np = 0; i_np < n_np; i_np++) {
            // Read a hash table.
            read(fildes, empty_key, sizeof(*empty_key) * stride);
            alder_hashtable_dsk_empty_key(o, empty_key);
            read(fildes, o->key, sizeof(*o->key) * o->size * o->stride);
            read(fildes, o->value, sizeof(*o->value) * o->size);
            // Print or return it.
            alder_hashtable_dsk_print(o, stdout);
        }
    }
    XFREE(empty_key);
    alder_hashtable_dsk_destroy(o);
    close(fildes);
    return o;
}







