/**
 * This file, alder_kmer_thread3.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_thread3_h
#define alder_kmer_alder_kmer_thread3_h

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
    ALDER_KMER_COUNTER3_READER_UNLOCK      = 9,
    ALDER_KMER_COUNTER3_READER_LOCK        = 10,
    ALDER_KMER_COUNTER3_READER_RELEASE     = 11,
    ALDER_KMER_COUNTER3_WRITER_UNLOCK      = 12,
    ALDER_KMER_COUNTER3_WRITER_LOCK        = 13,
    ALDER_KMER_COUNTER3_WRITER_RELEASE     = 14
};

#pragma mark struct

typedef struct alder_kmer_thread3_struct
alder_kmer_thread3_t;

/* Readwriter thread accesses this type.
 */
struct alder_kmer_thread3_struct {
    /* info */
    int k;                       /* k - kmer size                            */
    int b;                       /* b - bytes for the kmer size              */
    uint64_t i_ni;               /* i-th iteration                           */
    uint64_t n_ni;               /* n_ni - number of iteration               */
    uint64_t n_np;               /* n_np - number of partition               */
    int n_counter;               /* n_counter - number of counter threads    */
    
    /* lock */
    int lock_reader;
    int lock_writer;
    /* reader */
    size_t next_lenbuf;
    uint8_t *next_inbuf;         /* inbuf - input buffer                     */
    
    size_t reader_lenbuf;
    int reader_i_parfile;
    
    /* buffer */
    size_t size_subinbuf;        /* size of each divided buffer              */
    size_t size_inbuf;           /* size_inbuf - size of the input buffer    */
    size_t n_subbuf;             /* number of divided buffers in inbuf       */
    uint8_t *inbuf;              /* inbuf - input buffer                     */
    int *n_blockByReader;        /* [n_np] input buffer blocks               */
    int *n_blockByCounter;       /* [n_np] input buffer blocks               */
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
    FILE *fpin;                  /* partition input file pointer             */
    
    /* stat */
    size_t n_byte;
    size_t n_kmer;
    size_t n_hash;
    size_t totalFilesize;      /* total file size                            */
    int progress_flag;         /* flag for progress bar                      */
    int progressToError_flag;  /* flag for progress bar                      */
    int nopack_flag;           /* flag for no pack operation                 */
    
    /* flag */
    bool isMultithreaded;      /* are there other counters?                  */
    int status;                /* status of counter's buffers                */
};

void alder_kmer_counter_lock_reader(alder_kmer_thread3_t *a);
void alder_kmer_counter_unlock_reader(alder_kmer_thread3_t *a);
void alder_kmer_counter_lock_writer(alder_kmer_thread3_t *a, uint64_t i_np);
void alder_kmer_counter_unlock_writer(alder_kmer_thread3_t *a);
void alder_kmer_counter_increment_n_block(alder_kmer_thread3_t *a, uint64_t i_np);

__END_DECLS


#endif /* alder_kmer_alder_kmer_thread3_h */