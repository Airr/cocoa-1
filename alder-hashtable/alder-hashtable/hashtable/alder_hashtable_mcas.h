/**
 * This file, alder_hashtable_mcas.h, is part of alder-hashtable.
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

#ifndef alder_hashtable_alder_hashtable_mcas_h
#define alder_hashtable_alder_hashtable_mcas_h

#include <stdint.h>
#include <stdbool.h>
#include "bstrlib.h"
#include "alder_encode_number.h"

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

#define ALDER_HASHTABLE_MCAS_OCCUPIED 0x4000000000000000

typedef struct alder_hashtable_mcas_struct
alder_hashtable_mcas_t;

alder_hashtable_mcas_t*
alder_hashtable_mcas_create(int k, size_t size);

void
alder_hashtable_mcas_destroy(alder_hashtable_mcas_t *o);

int
alder_hashtable_mcas_reset(alder_hashtable_mcas_t *o);

int
alder_hashtable_mcas_increment(alder_hashtable_mcas_t *o,
                               uint64_t *key,
                               uint64_t *res_key,
                               bool isMultithreaded);

size_t
alder_hashtable_mcas_size(alder_hashtable_mcas_t *o);

uint64_t
alder_hashtable_mcas_key0(alder_hashtable_mcas_t *o, size_t p);

uint64_t
alder_hashtable_mcas_key1(alder_hashtable_mcas_t *o, size_t p);

bstring
alder_hashtable_mcas_key(alder_hashtable_mcas_t *o, size_t p);

int
alder_hashtable_mcas_value(alder_hashtable_mcas_t *o, size_t p);

int
alder_hashtable_mcaspseudo_increment(alder_hashtable_mcas_t *o,
                                     uint64_t *key,
                                     uint64_t *res_key);

int
alder_hashtable_mcaspseudo_can_increment(alder_hashtable_mcas_t *o,
                                         uint64_t *key,
                                         uint64_t *res_key);

int
alder_hashtable_mcaspseudo_number_open_slot(alder_hashtable_mcas_t *o);

int
alder_hashtable_mcaspseudo_maximum_count(alder_hashtable_mcas_t *o);

int
alder_hashtable_mcas_print_header_with_FILE(FILE *fp,
                                            int kmer_size,
                                            int hashtable_size,
                                            uint64_t n_ni,
                                            uint64_t n_np);

int
alder_hashtable_mcas_print_with_FILE(alder_hashtable_mcas_t *o, FILE *fp);

int64_t
alder_hashtable_mcas_printPackToFILE(alder_hashtable_mcas_t *o, FILE *fp);
int
alder_hashtable_mcas_printPackToFILE_count(size_t value, FILE *fp);

alder_hashtable_mcas_t * alder_hashtable_mcas_load(const char *fn,
                                                   int isSummary);

int
alder_hashtable_mcas_query(const char *fn,
                           const char *query);

int
alder_hashtable_mcas_count(const char *fn,
                           size_t *volumn);

int
alder_hashtable_mcas_header(const char *fn,
                            int *K, int *S, uint64_t *Ni, uint64_t *Np);

int
alder_hashtable_mcas_large_convert(alder_hashtable_mcas_t *dst,
                                   alder_hashtable_mcas_t *src,
                                   const char *fn,
                                   uint64_t xi_np,
                                   uint64_t xn_np);

int
alder_hashtable_mcas_convertCountToHashTable(alder_hashtable_mcas_t *ht,
                                             const char *fn,
                                             uint64_t xi_np,
                                             uint64_t xn_np);

void
alder_hashtable_mcas_select(alder_encode_number_t **ss,
                            uint64_t *i_ni,
                            uint64_t *i_np,
                            alder_encode_number_t *s1,
                            alder_encode_number_t *s2,
                            uint64_t number_iteration,
                            uint64_t number_partition);

int
alder_hashtable_mcaspseudo_find(alder_hashtable_mcas_t *o,
                                uint64_t *key,
                                int *value);
__END_DECLS


#endif /* alder_hashtable_alder_hashtable_mcas_h */
