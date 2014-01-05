/**
 * This file, alder_kmer_hashtable.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_hashtable_h
#define alder_kmer_alder_kmer_hashtable_h

#include <stdint.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

typedef uint64_t alder_hash_key_t;

__BEGIN_DECLS

enum {
    ALDER_KMER_HASHTABLE_SUCCESS = 0,
    ALDER_KMER_HASHTABLE_ERROR = 1,
    ALDER_KMER_HASHTABLE_ERROR_MEMORY = 2
};

typedef struct alder_kmer_hashtable_keyvalue_struct {
    alder_hash_key_t key;
    uint64_t value;
} alder_kmer_hashtable_keyvalue_t;

typedef struct alder_kmer_hashtable_struct {
    uint64_t key_lower_only;
    uint64_t size;
    alder_kmer_hashtable_keyvalue_t *data;
} alder_kmer_hashtable_t;

alder_kmer_hashtable_t * alder_kmer_hashtable_create (uint64_t m);
void alder_kmer_hashtable_destroy (alder_kmer_hashtable_t *o);

//void alder_kmer_hashtable_insert(alder_kmer_hashtable_t *o,
//                                 alder_hash_key_t key,
//                                 int32_t value);
void alder_kmer_hashtable_increase(alder_kmer_hashtable_t *o, alder_hash_key_t key);
alder_kmer_hashtable_keyvalue_t*
alder_kmer_hashtable_find(alder_kmer_hashtable_t *o, alder_hash_key_t key);
uint64_t alder_kmer_hashtable_hash(alder_hash_key_t key);
void alder_kmer_hashtable_report(alder_kmer_hashtable_t *o);

__END_DECLS


#endif /* alder_kmer_alder_kmer_hashtable_h */
