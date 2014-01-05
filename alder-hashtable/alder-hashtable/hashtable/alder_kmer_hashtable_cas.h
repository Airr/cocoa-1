/**
 * This file, alder_kmer_hashtable_cas.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_hashtable_cas_h
#define alder_kmer_alder_kmer_hashtable_cas_h

#include <stdint.h>
#include "alder_encode.h"

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

enum {
    ALDER_KMER_HASHTABLE_CAS_SUCCESS = 0,
    ALDER_KMER_HASHTABLE_CAS_ERROR = 1,
    ALDER_KMER_HASHTABLE_CAS_ERROR_MEMORY = 2
};

typedef struct alder_kmer_hashtable_cas_struct {
    int k;                      /* k-mer size                           */
    uint64_t size;              /* number of elements in the hash table */
    uint64_t stride;            /* stride in key                        */
    uint64_t *key;              /* size x stride many uint64_t          */
    uint32_t *value;            /* size many unit32_t                   */
} alder_kmer_hashtable_cas_t;

alder_kmer_hashtable_cas_t * alder_kmer_hashtable_cas_create (int k,
                                                              uint64_t m);
void alder_kmer_hashtable_cas_destroy (alder_kmer_hashtable_cas_t *o);

void alder_kmer_hashtable_cas_increase(alder_kmer_hashtable_cas_t *o,
                                       alder_encode_number_t *m);

//alder_kmer_hashtable_cas_keyvalue_t*
//alder_kmer_hashtable_cas_find(alder_kmer_hashtable_cas_t *o, alder_hash_key_t key);
//uint64_t alder_kmer_hashtable_cas_hash(alder_hash_key_t key);
void alder_kmer_hashtable_cas_report(alder_kmer_hashtable_cas_t *o);

__END_DECLS


#endif /* alder_kmer_alder_kmer_hashtable_cas_h */
