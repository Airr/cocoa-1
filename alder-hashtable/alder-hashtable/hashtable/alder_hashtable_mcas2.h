/**
 * This file, alder_hashtable_mcas2.h, is part of alder-kmer.
 *
 * Copyright 2014 by Sang Chul Choi
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

#ifndef alder_kmer_alder_hashtable_mcas2_h
#define alder_kmer_alder_hashtable_mcas2_h

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
 *  WARN: THIS DOES NOT FEATURE MCAS.
 *
 *  This is a 2nd version of a hash table for MCAS. An element in the table 
 *  consists of 6 elements;
 *    key, value, offset, left, right, timestamp. This hash table
 *  features deletions of elements. Initially, the hash table is filled upto
 *  70% as the 1st version. Once it is filled upto that point, I need to 
 *  delete some elements whenever I add a new one. If a key is found, I
 *  increment the value and change the timestamp. If the key is not found, I
 *  insert the key to an empty slot in the hash table. In this case, in order
 *  to keep the 70% rule of the hash table, some other element in the table
 *  must be deleted. The question is which one should deleted. 
 *
 *  To delete an element, I use offset, left, right, and timestamp.
 *
 *  I use a frequency table of counts to help determine the minimum value and 
 *  the minimum timestamp. Consider a histogram of the frequency of counts; 
 *  the histogram would be skewed to the right because most of elements would
 *  have smaller values. The minimum value is chosen as a count where the 
 *  number of elements with counts larger than the chosen count is 70% of the 
 *  70% or about 50% of the total size of the hash table. If more than 50% of 
 *  the elements have count value 1, then I choose 1. The minimum value could
 *  be greater than 1 if the hash table is small and larger data. 
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
    FILE *fpd;                  /* File for deleted keys: -deleted.kmc       */
    FILE *fps;                  /* File for saved keys: -saved.kmc           */
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
                             int min_value, int max_value,
                             int save_all_flag,
                             const char *outdir,
                             const char *outfile);

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
alder_hashtable_mcas2_t*
alder_hashtable_mcas2_read_table(const char *fn);

int
alder_hashtable_mcas2_search(alder_hashtable_mcas2_t *o,
                             uint64_t *key,
                             uint64_t *pos_key);
__END_DECLS


#endif /* alder_kmer_alder_hashtable_mcas2_h */
