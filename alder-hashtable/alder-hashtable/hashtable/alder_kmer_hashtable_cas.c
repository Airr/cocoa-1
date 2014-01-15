/**
 * This file, alder_kmer_hashtable_cas.c, is part of alder-kmer.
 *
 * Copyright 2013 by Sang Chul Choi
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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "alder_cmacro.h"
#include "alder_bit.h"
#include "alder_encode.h"
#include "alder_kmer_hashtable_cas.h"

/* This function creates a hash table for kmer counting with a feature of
 * taking advantage of CAS (compare and swap) instruction.
 */
alder_kmer_hashtable_cas_t * alder_kmer_hashtable_cas_create (int k,
                                                              uint64_t m)
{
    alder_kmer_hashtable_cas_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    
    o->size = m;
    o->stride = k%32 ? k/32 + 1 : k/32;
    o->key = NULL;
    o->value = NULL;
    
    o->key = malloc(sizeof(*o->key) * o->size * o->stride);
    o->value = malloc(sizeof(*o->value) * o->size);
    if (o->key == NULL || o->value == NULL) {
        alder_kmer_hashtable_cas_destroy(o);
        return NULL;
    }
    memset(o->key,0,sizeof(*o->key) * o->size * o->stride);
    memset(o->value,0,sizeof(*o->value) * o->size);
    return o;
}

/* This function destroys a hash table for kmer counting.
 */
void alder_kmer_hashtable_cas_destroy (alder_kmer_hashtable_cas_t *o)
{
    if (o != NULL) {
        XFREE(o->key);
        XFREE(o->value);
        XFREE(o);
    }
}


/* This function increases the value by one if there exists a key or m.
 * If not found, the key m is added to the hash table with the value of 1.
 *
 * The procedure in Marcais and Kingsford.
 * i <- 0
 * x <- pos(key,i)
 * current_key <- CAS(K[x], EMPTY, key)
 * while (current_key == EMTPY or current_key == key) {
 *     i++;
 *     x <- pos(key,i)
 *     FAIL if x is out of the bound in K - impossible in DSK method
 *     current_key <- CAS(K[x], EMPTY, key)
 * }
 * cval <- V[x]
 * oval <- cval
 * while (cval == oval) {
 *     oval <- cval
 *     cval <- CAS(V[x], oval, oval+1)
 * }
 
                uint64_t key = hash_m % HASH_SIZE;
                uint64_t current_key = EMPTY_KEY + 1; // just another key to enter the while loop.
                assert(current_key != key);       // I must enter while loop.
                while (current_key != EMPTY_KEY && current_key != key) {
 current_key = __sync_val_compare_and_swap(, EMPTY_KEY, key);
 */
void alder_kmer_hashtable_cas_increase(alder_kmer_hashtable_cas_t *o,
                                       alder_encode_number_t *m)
{
    // K[x]  = o->key + x * stride
    // EMPTY = o->key + x * (stride + 1) - 1 ...
    // key   = m->n[0], m->n[1], ..., m->n[m->s - 1].
    //
    uint64_t tries = 1;
    uint64_t i = 0;
    uint64_t HASH_SIZE = o->size;
    uint64_t EMPTY_KEY = 0;
    uint64_t key = m->n[0];
    uint64_t x = alder_encode_number_hash(m) % o->size;
    uint64_t current_key = __sync_val_compare_and_swap(&o->key[x], EMPTY_KEY, key);
    while (current_key != EMPTY_KEY && current_key != key && tries < HASH_SIZE) {
        i++;
        x = (x + 1) % o->size;
        current_key = __sync_val_compare_and_swap(&o->key[x], EMPTY_KEY, key);
        tries++;
    }
    
    if (!(tries < o->size)) {
        fprintf(stderr, "Fatal Error - hash table is full!\n");
        assert(0);
        abort();
    }
    
    uint64_t cval = o->value[x];
    uint64_t oval = cval + 1;
    while (cval != oval) {
        oval = cval;
        cval++;
        cval = __sync_val_compare_and_swap(&o->value[x], (uint32_t)oval, (uint32_t)cval);
    }
}

void alder_kmer_hashtable_cas_report(alder_kmer_hashtable_cas_t *o)
{
    for (uint64_t i = 0; i < o->size; i++) {
        if (o->key[i] < o->size) {
//            fprintf(stdout, "[%07llu] key-%07llu %llu\n", i, o->data[i].key, o->data[i].value);
        }
    }
}
