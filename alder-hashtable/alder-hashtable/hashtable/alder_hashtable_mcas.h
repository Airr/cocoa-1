/**
 * This file, alder_hashtable_mcas.h, is part of alder-hashtable.
 *
 * Copyright 2013, 2014 by Sang Chul Choi
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

#include "alder_hashtable_mcas1.h"
#include "alder_hashtable_mcas2.h"
#include "alder_hashtable_mcas3.h"

#include <stdint.h>
#include <stdbool.h>
#include "bstrlib.h"
#include "alder_encode_number.h"
#include "libdivide.h"

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

/**
 *  This structure is used to count kmers and search for them.
 */
struct alder_hashtable_mcas_struct {
    int k;                      /* k-mer size                           */
    int stride;                 /* stride in key                        */
    int min;                    /* min occurence                        */
    int max;                    /* max occurence                        */
    size_t size;                /* number of elements in the hash table */
    uint64_t *empty_key;        /* n: stride - EMPTY key                */
    uint64_t *key;              /* n: size x stride                     */
    uint16_t *value;            /* n: size                              */
    struct libdivide_u64_t fast_size;
};

typedef struct alder_hashtable_mcas_struct alder_hashtable_mcas_t;

size_t
alder_hashtable_mcas_element_sizeof(int K);

size_t
alder_hashtable_mcas_sizeof(int k, size_t size);

size_t
alder_hashtable_mcas_size_with_available_memory(int K, size_t mem_size);

alder_hashtable_mcas_t*
alder_hashtable_mcas_create(int k, size_t size);

alder_hashtable_mcas_t*
alder_hashtable_mcas_createWithMemory(int k, size_t size,
                                      void *mem, size_t mem_size);

void
alder_hashtable_mcas_destroy(alder_hashtable_mcas_t *o);

int
alder_hashtable_mcas_reset(alder_hashtable_mcas_t *o);

int
alder_hashtable_mcas_increment_with_hashcode(alder_hashtable_mcas_t *o,
                                             uint64_t *key,
                                             uint64_t x,
                                             uint64_t *res_key,
                                             bool isMultithreaded);

int
alder_hashtable_mcas_increment(alder_hashtable_mcas_t *o,
                               uint64_t *key,
                               uint64_t *res_key,
                               bool isMultithreaded);

int
alder_hashtable_mcas_state(alder_hashtable_mcas_t *o);

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
alder_hashtable_mcas_printHeaderToFILE(FILE *fp,
                                       int kmer_size,
                                       uint64_t hashtable_size,
                                       uint64_t n_ni,
                                       uint64_t n_np);
int
alder_hashtable_mcas_printHeaderToFD(int fd,
                                     int kmer_size,
                                     uint64_t n_nh,
                                     uint64_t n_ni,
                                     uint64_t n_np);

int64_t
alder_hashtable_mcas_printPackToFILE(alder_hashtable_mcas_t *o, FILE *fp);
int64_t
alder_hashtable_mcas_printPackToFD(alder_hashtable_mcas_t *o, int fd, size_t bufsize);

int
alder_hashtable_mcas_printCountPerPartition(FILE *fp, size_t value,
                                            uint64_t i_ni, uint64_t i_np,
                                            uint64_t n_np);
int
alder_hashtable_mcas_printCountPerPartitionFD(int fd, size_t value,
                                              uint64_t i_ni, uint64_t i_np,
                                              uint64_t n_np);

int
alder_hashtable_mcas_printPackToFILE_count(size_t value, FILE *fp);
int
alder_hashtable_mcas_printPackToFD_count(size_t value, int fd);

int
alder_hashtable_mcas_printPackToFD_np(uint64_t value, int fd);

int
alder_hashtable_mcas_load(const char *fn, int isSummary);

int
alder_hashtable_mcas_query(const char *fn,
                           const char *query);

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

void
alder_hashtable_mcas_select2(alder_encode_number2_t **ss,
                             uint64_t *i_ni,
                             uint64_t *i_np,
                             alder_encode_number2_t *s1,
                             alder_encode_number2_t *s2,
                             uint64_t number_iteration,
                             uint64_t number_partition);

int
alder_hashtable_mcaspseudo_find(alder_hashtable_mcas_t *o,
                                uint64_t *key,
                                int *value);

int
alder_hashtable_mcas_nextFromFile(int fildes,
                                  alder_encode_number_t *key,
                                  uint16_t *value,
                                  size_t *pos,
                                  int flag_nh64);
int
alder_hashtable_mcas_updateFile(int fildes,
                                alder_encode_number_t *key,
                                uint16_t *value,
                                size_t *pos,
                                int flag_nh64);
int
alder_hashtable_mcas_large_close(int fildes);


int
alder_hashtable_mcas_find_open(const char *fn, int *fildes_p, int *K_p,
                               uint64_t *nh_p, uint64_t *ni_p, uint64_t *np_p,
                               uint64_t *tnh_p, uint64_t **n_nhs_p);
int
alder_hashtable_mcas_find(int fildes,
                          const char *query,
                          int K,
                          uint64_t nh,
                          uint64_t ni,
                          uint64_t np,
                          uint64_t tnh,
                          uint64_t *n_nhs);

int
alder_hashtable_mcas_read_headerFILE(const char *fn, long *fpos,
                                     int *K_p, uint64_t *nh_p,
                                     uint64_t *ni_p, uint64_t *np_p,
                                     uint64_t *tnh_p, uint64_t **n_nhs_p);

size_t
alder_hashtable_mcas_table_entry_sizeof(int K, uint64_t nc, uint64_t nh);

alder_hashtable_mcas_t*
alder_hashtable_mcas_read_table(const char *fn, uint64_t i_ni, uint64_t i_np);

int
alder_hashtable_mcas_search(alder_hashtable_mcas_t *o,
                            uint64_t *key,
                            uint64_t *pos_key);
int
alder_hashtable_mcas_print(FILE *fp, alder_hashtable_mcas_t *o,
                           uint16_t min_count);

int
open_table_header(const char *fn, int *fildes_p, int *K_p,
                  uint64_t *nh_p, uint64_t *ni_p, uint64_t *np_p,
                  uint64_t *tnh_p, uint64_t **n_nhs_p);
__END_DECLS

#endif /* alder_hashtable_alder_hashtable_mcas_h */
