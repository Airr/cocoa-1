/**
 * This file, alder_kmer_binary.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_binary_h
#define alder_kmer_alder_kmer_binary_h

#include <pthread.h>
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


__BEGIN_DECLS

#define ALDER_KMER_BINARY_READBLOCK_I_NP       0
#define ALDER_KMER_BINARY_READBLOCK_TYPE       4
#define ALDER_KMER_BINARY_READBLOCK_LEN        8
#define ALDER_KMER_BINARY_READBLOCK_BODY       16

int
alder_kmer_binary(void *ptr, size_t size, size_t subsize,
                  uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                  long version,
                  int K, long D,
                  int n_nt,
                  size_t totalfilesize,
                  size_t *n_byte,
                  int progress_flag,
                  int progressToError_flag,
                  struct bstrList *infile, const char *outdir,
                  const char *outfile);

int
alder_kmer_binary1(void *ptr, size_t size, size_t subsize,
                   uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                   long version,
                   int K, long D,
                   int n_nt,
                   size_t totalfilesize,
                   size_t *n_byte,
                   int progress_flag,
                   int progressToError_flag,
                   struct bstrList *infile, const char *outdir,
                   const char *outfile);

int
alder_kmer_binary2(void *ptr, size_t size, size_t subsize,
                   uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                   long version,
                   int K, long D,
                   int n_nt,
                   size_t totalfilesize,
                   size_t *n_byte,
                   int progress_flag,
                   int progressToError_flag,
                   struct bstrList *infile, const char *outdir,
                   const char *outfile);

int
alder_kmer_binary3(void *ptr, size_t size, size_t subsize,
                   uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                   long version,
                   int K, long D,
                   int n_nt,
                   size_t totalfilesize,
                   size_t *n_byte,
                   int progress_flag,
                   int progressToError_flag,
                   struct bstrList *infile, const char *outdir,
                   const char *outfile);

typedef struct alder_kmer_binary_struct alder_kmer_binary_t;

/* Readwriter thread accesses this type.
 */
struct alder_kmer_binary_struct {
    /* info */
    int k;                     /* k - kmer size                              */
    int n_binary;              /* 1                                          */
    
    /* infile */
    ssize_t len_read;
    int i_infile;
    
    /* buffer */
    uint8_t inbuf_type_infile; /* type of input file in the buffer           */
    size_t size_fixed_inbuf;   /* fixed size of a buffer                     */
    size_t size_fixed_outbuf;
    size_t size_inbuf;         /* size_inbuf - size of the inbuf             */
    size_t size_outbuf;        /* size_outbuf - size of the output buffer    */
    uint8_t *inbuf;            /* [size_inbuf] inbuf - input buffer          */
    uint8_t *outbuf;           /* [size_outbuf] outbuf - output buff         */
    
    /* file */
    struct bstrList *infile;   /* (not own) input files                      */
    void *fx;                  /* fx - input file pointer                    */
    int type_infile;           /* type of input file                         */
    FILE *fpout;               /* n_np: fpout - output file pointers         */
    bstring dout;              /* output directory                           */
    
    /* progress */
    size_t totalFilesize;      /* total file size                            */
    int progress_flag;
    int progressToError_flag;
    
    /* stat */
    uint64_t n_byte;           /* number of bytes sent                       */
    uint64_t n_kmer;           /* number of Kmers written to an out file     */
};

void alder_kmer_binary_lock_reader(alder_kmer_binary_t *a, int binary_id);
void alder_kmer_binary_unlock_reader(alder_kmer_binary_t *a, int binary_id);
void alder_kmer_binary_lock_writer(alder_kmer_binary_t *a);
void alder_kmer_binary_unlock_writer(alder_kmer_binary_t *a);

__END_DECLS


#endif /* alder_kmer_alder_kmer_binary_h */
