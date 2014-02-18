/**
 * This file, alder_kmer_thread8.h, is part of alder-kmer.
 *
 * Copyright 2013,2014 by Sang Chul Choi
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

#ifndef alder_kmer_alder_kmer_thread8_h
#define alder_kmer_alder_kmer_thread8_h

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

typedef struct alder_kmer_thread8_struct alder_kmer_thread8_t;

struct alder_kmer_thread8_struct {
    /* info */
    int k;                       /* k - kmer size                            */
    int b;                       /* b - bytes for the kmer size              */
    int n_counter;               /* n_counter - number of counter threads    */
    
    /* encoder part */
    int type_infile;
    void *fx;                    /* fx - input file pointer                  */
    int reader_i_infile;
    int reader_type_infile;
    size_t reader_lenbuf2;
    
    /* reader */
    size_t next_lenbuf;
    uint8_t *next_inbuf;         /* inbuf - input buffer                     */
    size_t reader_lenbuf;
    size_t size_inbuf2;          /* size_inbuf - size of the input buffer    */
    uint8_t *inbuf2;             /* inbuf2 - input buffer                    */
    
    /* buffer */
    size_t size_readbuffer;      /* buffer size for writing table            */
    size_t size_writebuffer;     /* buffer size for writing table            */
    
    size_t size_subinbuf;        /* size of each divided buffer              */
    size_t size_inbuf;           /* size_inbuf - size of the input buffer    */
    uint8_t *inbuf;              /* inbuf - input buffer                     */
    
    /* outbuf: hash table */
    alder_hashtable_mcas2_t *ht; /* (per partition) hash table               */
    
    /* file */
    struct bstrList *infile;     /* (not own) input partition file names     */
    FILE *fpout;                 /* (not own) count table file pointer       */
    bstring boutfile;            /* outfile name                             */
    bstring boutdir;             /* out directory                            */
    FILE *fpin;                  /* partition input file pointer             */
    
    /* stat */
    size_t *n_t_byte;          /* number of bytes for each turn of partition */
    size_t *n_i_byte;          /* number of bytes in each iteration          */
    size_t *n_i_kmer;          /* number of kmer in each iteration           */
    size_t n_byte;
    size_t n_kmer;
    size_t n_hash;
    size_t n_total_byte;       /* total number of bytes to be processed      */
    size_t n_total_kmer;       /* total number of kmers to be processed      */
    size_t n_current_kmer;     /* number of kmers processed so far           */
    
    int progress_flag;         /* flag for progress bar                      */
    int progressToError_flag;  /* flag for progress bar                      */
    
    /* flag */
    bool isMultithreaded;      /* are there other counters?                  */
};

void alder_kmer_thread8_increment_n_block(alder_kmer_thread8_t *a, uint64_t i_np);

__END_DECLS


#endif /* alder_kmer_alder_kmer_thread8_h */
