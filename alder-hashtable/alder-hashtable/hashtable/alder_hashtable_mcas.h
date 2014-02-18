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

#define ALDER_HASHTABLE_MCAS_OCCUPIED 0x4000000000000000
#define ALDER_HASHTABLE_MCAS_STATE_UNFINISHED 1
#define ALDER_HASHTABLE_MCAS_STATE_FINISHED   2
#define ALDER_HASHTABLE_MCAS_STATE_LOCKED     3

/**
 *  n is the size of the array on the current line.
 */
struct alder_hashtable_mcas_struct {
    int k;                      /* k-mer size                           */
    int stride;                 /* stride in key                        */
    int state;                  /* 1 unfinished, 2 finished, 3 locked   */
    int min;                    /* min occurence                        */
    int max;                    /* max occurence                        */
    size_t size;                /* number of elements in the hash table */
    size_t size7;               /* number of elements in the hash table */
    size_t index;               /* current index for iteration          */
    uint64_t i_np;              /* partition index                      */
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

uint64_t
alder_hashtable_mcas_i_np(alder_hashtable_mcas_t *o);

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

#pragma mark version2

/**
 *  This is a 2nd version of a hash table for MCAS. An element in the table 
 *  consists of three elements; key, value, and timestamp. This hash table
 *  features deletions of elements. Initially, the hash table is filled upto
 *  70% as the 1st version. Once it is filled upto that point, I need to 
 *  delete some elements whenever I add a new one. If a key is found, I
 *  increment the value and change the timestamp. If the key is not found, I
 *  insert the key to an empty slot in the hash table. In this case, in order
 *  to keep the 70% rule of the hash table, some other element in the table
 *  must be deleted. The question is which one should deleted. 
 *
 *  To delete an element, I use two factors; value and timestamp. I randomly
 *  choose an element as a starting point, and test whether the element can be
 *  deleted. If the element's value is smaller than a given minimum value and
 *  the element's timestamp is less than another given minimum time stamp, then
 *  it can be deleted. The choice of min value and min time stamp would affect
 *  the accuracy and the speed of counting. These two, especially, min time
 *  stamp, would be a factor that needs attention. I will explain how the two
 *  values are chosen in the next paragraph. Keep testing and trying to delete
 *  until one element is successfully deleted.
 *
 *  I use a frequency table of counts to help determine the minimum value and 
 *  the minimum timestamp. Consider a histogram of the frequency of counts; 
 *  the histogram would be skewed to the right because most of elements would
 *  have smaller values. The minimum value is chosen as a count where the 
 *  number of elements with counts larger than the chosen count is 70% of the 
 *  70% or about 50% of the total size of the hash table. If more than 50% of 
 *  the elements have count value 1, then I choose 1. The minimum value could
 *  be greater than 1 if the hash table is small and larger data. 
 *
 *  The minimum time stamp could have something to do with the frequency of 
 *  update of count. Whenever the same key is updated with its count, I'd 
 *  compute the average of update frequency; 
 *  A[n] = A[n-1] x (n-1) / n + V[n] / n
 *  This may not be precise, but it would serve its role.
 *
 *  hash element(bits) = key(2k) + value(16) + timestamp([kmers]) + offset
 *    e.g. k=27, key(54) + value(16) + timestamp(37) + offset(37)
 *               key(8byte) + offset(30) + value(32) + timestamp(62)
 *               key must fit to 8-byte values unless MCAS code is changed.
 */
struct alder_hashtable_mcas2_struct {
    int k;                      /* k-mer size                                */
    int size_key;               /* size of key                               */
    int pos_value;              /* position of value in a key                */
    int pos_pointerL;           /* position of pointer in a key              */
    int pos_pointerR;           /* position of pointer in a key              */
    int pos_timestamp;          /* position of timestamp in a key            */
    int size_auxiliary;         /* stride - size_key                         */
    int stride;                 /* stride in key, offset, value, timestamp   */
                                /* stride is in unit of uint64_t             */
    int size_frequency;         /* size of frequency                         */
    size_t size;                /* number of elements in the hash table      */
    size_t size7;               /* number of elements in the hash table      */
    uint64_t size5;             /* number of elements for min_freq_value     */
    
    size_t n_key;               /* [C] number of keys in the table           */
    uint64_t min_value;         /* min occurence                             */
    uint64_t max_value;         /* max occurence                             */
    uint64_t min_freq_value;    /* [C] min occurence by freq table           */
    uint64_t timestamp;         /* [C] time stamp                            */
    uint64_t pointer;           /* [C] pointer to the current slot           */
    uint64_t pointer_head;      /* [C] pointer to head slot                  */
    uint64_t pointer_tail;      /* [C] pointer to tail slot                  */
    uint64_t *empty_key;        /* n: stride - EMPTY key                     */
    uint64_t *frequency;        /* [C] 2^16: frequency of count              */
    uint64_t *frequencyL;       /* [C] 2^16: frequency of count on the left  */
    uint64_t *key;              /* [C] n: size x stride                      */
                                /* key, value, offset, left/right, timestamp */
    struct libdivide_u64_t fast_size;
};

typedef struct alder_hashtable_mcas2_struct alder_hashtable_mcas2_t;
typedef struct alder_hashtable_mcas2_query_struct alder_hashtable_mcas2_query_t;

int
alder_hashtable_mcas2_load(const char *fn, int isSummary);

int
alder_hashtable_mcas2_query(const char *fn,
                            const char *query);

int
alder_hashtable_mcas2_printHeaderToFD(int fd,
                                      int kmer_size,
                                      uint64_t n_nh,
                                      uint64_t size_value,
                                      uint64_t size_index);

int64_t
alder_hashtable_mcas2_printToFILE(alder_hashtable_mcas2_t *o, FILE *fp);

int
alder_hashtable_mcas2_printPackToFD_count(size_t value, int fd);

alder_hashtable_mcas2_t*
alder_hashtable_mcas2_create(int k, size_t size, int stride,
                             int min_value, int max_value);

void
alder_hashtable_mcas2_destroy(alder_hashtable_mcas2_t *o);

int
alder_hashtable_mcas2_reset(alder_hashtable_mcas2_t *o);

int64_t
alder_hashtable_mcas2_printPackToFD(alder_hashtable_mcas2_t *o,
                                    int fd,
                                    size_t bufsize);

int
alder_hashtable_mcas2_increment_hashcode(alder_hashtable_mcas2_t *o,
                                         uint64_t *key,
                                         uint64_t x,
                                         uint64_t *res_key,
                                         uint64_t *added_x,
                                         bool isMultithreaded);

int
alder_hashtable_mcas2_increment_spacesaving(alder_hashtable_mcas2_t *o,
                                            uint64_t *key,
                                            uint64_t x,
                                            uint64_t *res_key,
                                            uint64_t *res_key2,
                                            uint64_t *added_x,
                                            uint64_t *removed_x,
                                            bool isMultithreaded);

int
alder_hashtable_mcas12_assess(double *error_kmer,
                              uint64_t *n_kmer,
                              size_t s_kmer,
                              const char *fn,
                              const char *fn2,
                              int isSummary);

int
alder_hashtable_mcas2_size_with_available_memory(size_t *size_table,
                                                 int *size_stride,
                                                 int K, size_t mem_size);

size_t
alder_hashtable_mcas2_element_sizeof_with_available_memory(int K,
                                                           size_t size_mem);

size_t
alder_hashtable_mcas2_element_sizeof_with_size_table(int K,
                                                     size_t size_table);

void
alder_hashtable_mcas2_set_pointer_to_tail(alder_hashtable_mcas2_t *o);

int
alder_hashtable_mcas2_debug_print(alder_hashtable_mcas2_t *o,
                                  uint64_t *c_key,
                                  uint64_t x,
                                  uint64_t added_x,
                                  uint64_t removed_x,
                                  const char *outdir,
                                  const char *outfile);

__END_DECLS

#endif /* alder_hashtable_alder_hashtable_mcas_h */
