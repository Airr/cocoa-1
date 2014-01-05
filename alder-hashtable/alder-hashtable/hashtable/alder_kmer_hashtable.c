/**
 * This file, alder_kmer_hashtable.c, is part of alder-kmer.
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
#include <stdio.h>
#include <string.h>
#include "alder_bit.h"
#include "alder_cmacro.h"
#include "alder_kmer_hashtable.h"

/* This function creates a hash table for kmer counting.
 */
alder_kmer_hashtable_t * alder_kmer_hashtable_create (uint64_t m)
{
    alder_kmer_hashtable_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    
    o->size = sizeof(alder_kmer_hashtable_keyvalue_t);
    o->size = m/sizeof(alder_kmer_hashtable_keyvalue_t) + 1;
    o->data = malloc(sizeof(*o->data) * o->size);
    if (o->data == NULL) {
        XFREE(o);
        return NULL;
    }
    memset(o->data, 0, sizeof(*o->data) * o->size);
    
    int number_high_zero = alder_bit_highest_bit_1(m);
    o->key_lower_only = alder_bit_all_1_with_left_zero_of(number_high_zero);
    
    return o;
}

/* This function destroys a hash table for kmer counting.
 */
void alder_kmer_hashtable_destroy (alder_kmer_hashtable_t *o)
{
    if (o != NULL) {
        XFREE(o->data);
        XFREE(o);
    }
}

/* This function zeros values.
 */
void alder_kmer_hashtable_reset (alder_kmer_hashtable_t *o)
{
    if (o != NULL) {
        memset(o->data, 0, sizeof(*o->data) * o->size);
    }
}

/* This function find an element in a hash table.
 */
alder_kmer_hashtable_keyvalue_t*
alder_kmer_hashtable_find(alder_kmer_hashtable_t *o, alder_hash_key_t key)
{
    uint64_t p = alder_kmer_hashtable_hash(key) % o->size;
    alder_kmer_hashtable_keyvalue_t *e = o->data + p;
    uint64_t tries = 0;

    // search until we either find the key, or find an empty slot.
    while (e->value != 0 && e->key != key && tries < o->size) {
        p = (p + 1) % o->size;
        e = o->data + p;
        tries++;
    }
    if (tries == o->size) {
        fprintf(stderr, "Fatal Error - hash table is full!\n");
        exit(1);
    }

    return e;
}

/* This function increase the value of an element by one.
 */
void alder_kmer_hashtable_increase(alder_kmer_hashtable_t *o, alder_hash_key_t key)
{
//    alder_hash_key_t key_pos = alder_kmer_hashtable_hash(key) & o->key_lower_only;
    alder_kmer_hashtable_keyvalue_t *e = alder_kmer_hashtable_find(o, key);
    if (e->value == 0) {
        e->key = key;
    }
    e->value++;
}

///* This function inserts a new value.
// */
//void alder_kmer_hashtable_insert(alder_kmer_hashtable_t *o,
//                                 alder_hash_key_t key,
//                                 int32_t value)
//{
////    alder_hash_key_t key_pos = alder_kmer_hashtable_hash(key) & o->key_lower_only;
//    alder_kmer_hashtable_keyvalue_t *e = alder_kmer_hashtable_find(o, key);
//    if (e->value == 0) {
//        e->key = key;
//    }
//    e->value = value;
//}

/* This function converts a key to an almost uniform hash value.
 */
uint64_t alder_kmer_hashtable_hash(alder_hash_key_t key)
{
    uint64_t code = key;
    code = code ^ (code >> 14);
    code = (~code) + (code << 18);
    code = code ^ (code >> 31);
    code = code * 21;
    code = code ^ (code >> 11);
    code = code + (code << 6);
    code = code ^ (code >> 22);
    return code;
}

void alder_kmer_hashtable_report(alder_kmer_hashtable_t *o)
{
    for (uint64_t i = 0; i < o->size; i++) {
        if (o->data[i].value > 0) {
            fprintf(stdout, "[%07llu] key-%07llu %llu\n", i, o->data[i].key, o->data[i].value);
        }
    }
}
