/**
 * This file, alder_hashtable_mcas1.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_hashtable_mcas1_h
#define alder_kmer_alder_hashtable_mcas1_h

#include <stdint.h>
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
 *  The first version of the hashtable mcas uses different structures for
 *  counting and saving kmers. When counting kmers, it has two separate 
 *  variables of key and value. When saving kmers, it has only one entry, which
 *  consists of key and value.  When saving kmers, this seems to be more 
 *  natural. There must be reasons why I chose to use separate variables of key
 *  and value, which I do not remember.  I have sticked to this way of separate
 *  key and value too long. Because hashtable_mcas module is too large, I 
 *  would make smaller codes for them. This structure is an attempt to move 
 *  the separation of key and value scheme to the merge of them.
 *
 *  When counting I need two fields: key and value. When saving and searching
 *  tables, I need three fields: key, value, and index. When saving tables,
 *  index seems to be useful because many of slots are empty, which would be 
 *  not saved at all. Index values would tell me which slots are occupied.
 *  When search a table, I could use index or could build up the same hash 
 *  table for searching. So, this causes inconsistency between counting and
 *  searching.
 *
 *  I used the separate key and value variables because I wished to search
 *  for keys by 64-bit values. Key's value can be smaller than 64-bit; I used
 *  16-bit value for counts. MCAS works with 64-bit values of key; this may be
 *  why I chose to use separate key and value. 
 *
 *  I need to update count using CAS. I think that I need to fix the 
 *  implementation of version 1 hash table unless I really need something
 *  different. I would load the saved table using the same structure. 
 *
 *  FINAL THOUGHTS
 *  1. Use index only when saving a table.
 *  2. Use the same hash table for counting and searching.
 */
struct alder_hashtable_mcas1_struct {
    int k;                      /* k-mer size                                */
    int size_key;               /* size of key                               */
    int pos_value;              /* position of value in a key                */
    int size_auxiliary;         /* stride - size_key                         */
    int stride;                 /* stride in key, offset, value, timestamp   */
                                /* stride is in unit of uint64_t             */
    int size_frequency;         /* size of frequency                         */
    size_t size;                /* number of elements in the hash table      */
    size_t size7;               /* number of elements in the hash table      */
    uint64_t size5;             /* number of elements for min_freq_value     */
    
    uint64_t min_value;         /* min occurence                             */
    uint64_t max_value;         /* max occurence                             */
    uint64_t *key;              /* [C] n: size x stride                      */
                                /* key, value                                */
    struct libdivide_u64_t fast_size;
};

typedef struct alder_hashtable_mcas1_struct alder_hashtable_mcas1_t;



__END_DECLS


#endif /* alder_kmer_alder_hashtable_mcas1_h */
