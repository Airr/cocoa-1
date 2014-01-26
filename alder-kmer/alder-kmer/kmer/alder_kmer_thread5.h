/**
 * This file, alder_kmer_thread5.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_thread5_h
#define alder_kmer_alder_kmer_thread5_h

#include <stdint.h>
#include "bstrlib.h"
#include "alder_hashtable.h"

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
    ALDER_KMER_COUNTER5_READER_UNLOCK      = 9,
    ALDER_KMER_COUNTER5_READER_LOCK        = 10,
    ALDER_KMER_COUNTER5_READER_RELEASE     = 11,
    ALDER_KMER_COUNTER5_WRITER_UNLOCK      = 12,
    ALDER_KMER_COUNTER5_WRITER_LOCK        = 13,
    ALDER_KMER_COUNTER5_WRITER_RELEASE     = 14
};

#pragma mark struct

typedef struct alder_kmer_thread5_struct
alder_kmer_thread5_t;

/* Readwriter thread accesses this type.
 *
 *  cur_inbuf        - position of the sub buffer for a counter
 *  cur_i_np         - i_np for the current round
 *  n_blockByReader  - the number of blocks to read (set initially)
 *  n_blockByCounter - the number that counter increments
 */
struct alder_kmer_thread5_struct {
    /* info */
    int k;                       /* k - kmer size                            */
    int b;                       /* b - bytes for the kmer size              */
    uint64_t i_ni;               /* iteration index - always 0               */
    uint64_t n_ni;               /* n_ni - always 1                          */
    uint64_t n_np;               /* n_np - number of partition               */
    uint64_t n_nh;               /* n_np - number of partition               */
    int n_counter;               /* n_counter - number of counter threads    */
    
    size_t *cur_inbuf;           /* [n_counter] inbuf pos for each counter   */
    int64_t *cur_i_np;          /* [n_counter] i_np for the current counter */
    
    /* buffer */
    size_t size_subinbuf;        /* size of each divided buffer              */
    size_t size_inbuf;           /* size_inbuf - size of the input buffer    */
    uint8_t *inbuf;              /* inbuf - input buffer                     */
    int64_t *n_blockByReader;    /* [n_ht] input buffer blocks               */
    int64_t *n_blockByCounter;   /* [n_ht] input buffer blocks               */
    size_t size_data;            /* size of input data in inbuf              */
                                 /* use the memory from here for tables      */
    
    size_t size_writebuffer;     /* buffer size for writing table            */
    
    /* outbuf: hash table */
    size_t n_ht;                 /* number of hash tables                    */
    size_t main_i_np;            /* main partition index                     */
    size_t main_table;           /* main table                               */
    alder_hashtable_mcas_t **ht; /* (per partition) hash table               */
    
    /* file */
    struct bstrList *infile;     /* (not own) input partition file names     */
    FILE *fpout;                 /* (not own) count table output file pointer*/
    bstring boutfile;            /* outfile name                             */
    bstring boutdir;             /* out directory                            */
    
    /* bin file stat */
    uint64_t n_kmer;
    uint64_t n_dna;
    uint64_t n_seq;
    
    /* stat */
    size_t n_byte;
    size_t n_hash;
    size_t totalFilesize;        /* total file size                          */
    int progress_flag;           /* flag for progress bar                    */
    int progressToError_flag;    /* flag for progress bar                    */
    
    /* flag */
    bool isMultithreaded;        /* are there other counters?                */
};

int alder_kmer_counter5_lock_reader(alder_kmer_thread5_t *a, int counter_id);
void alder_kmer_counter5_unlock_reader(alder_kmer_thread5_t *a, int counter_id);
int alder_kmer_counter5_increment_n_block(alder_kmer_thread5_t *a,
                                          uint64_t i_np,
                                          size_t c_table,
                                          int counter_id);

__END_DECLS


#endif /* alder_kmer_alder_kmer_thread5_h */
