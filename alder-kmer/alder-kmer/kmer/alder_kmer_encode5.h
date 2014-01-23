/**
 * This file, alder_kmer_encode5.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_encode5_h
#define alder_kmer_alder_kmer_encode5_h

#include <stdint.h>
#include "bstrlib.h"

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
    ALDER_KMER_ENCODER5_READER_UNLOCK      = 9,
    ALDER_KMER_ENCODER5_READER_LOCK        = 10,
    ALDER_KMER_ENCODER5_READER_RELEASE     = 11,
    ALDER_KMER_ENCODER5_WRITER_UNLOCK      = 12,
    ALDER_KMER_ENCODER5_WRITER_LOCK        = 13,
    ALDER_KMER_ENCODER5_WRITER_RELEASE     = 14
};

typedef struct alder_kmer_encoder5_struct alder_kmer_encoder5_t;

/* Readwriter thread accesses this type.
 */
struct alder_kmer_encoder5_struct {
    /* info */
    int k;                     /* k - kmer size                              */
    int b;                     /* bytes for a kmer                           */
    uint64_t n_ni;             /* n_ni - number of iteration                 */
    uint64_t n_np;             /* number of iteration                        */
    uint64_t i_ni;             /* iteration index                            */
    int n_encoder;             /* n_encoder - number of encoder threads      */
    
    /* lock */
    int lock_reader;
    int lock_writer;
    int *lock_partition;
    
    /* reader */
    size_t reader_lenbuf;
    size_t reader_lenbuf2;
    uint8_t reader_type_infile;
    int reader_i_infile;
    
    /* buffer */
    size_t size_fixedoutbuffer;/* buffer size for output                     */
    //    size_t n_subbuf;           /* number of divided buffers in inbuf         */
    size_t size_subinbuf;      /* size of each divided buffer                */
    size_t size_inbuf;         /* size_inbuf - size of the inbuf             */
    size_t size_inbuf2;        /* size_inbuf2 - size of the inbuf2           */
    size_t size_outbuf;        /* size_outbuf - size of the output buffer    */
    size_t size_suboutbuf;     /* per thread: size of each sub out buffer    */
    size_t size_suboutbuf2;    /* per partition: size of each sub out buffer */
    uint8_t *inbuf;            /* [size_inbuf] inbuf - input buffer          */
    uint8_t *inbuf2;           /* [size_inbuf2] inbuf2 - input buffer        */
    uint8_t *outbuf;           /* [size_outbuf] outbuf - output buff         */
    int *encoder_remaining_outbuf;
    /* body size */
    size_t size_suboutbuf2_body;
    
    /* file */
    struct bstrList *infile;   /* (not own) input files                      */
    void *fx;                  /* fx - input file pointer                    */
    int type_infile;           /* type of input file                         */
    FILE **fpout;              /* n_np: fpout - output file pointers         */
    bstring dout;              /* output directory                           */
    
    /* progress */
    size_t totalFilesize;      /* total file size                            */
    int progress_flag;
    int progressToError_flag;
    
    /* stat */
    uint64_t n_byte;           /* number of bytes sent                       */
    uint64_t n_kmer;           /* number of Kmers written to an out file     */
    uint64_t n_letter;         /* number of DNA letters processed            */
    
    /* flag */
    int status;                /* status of encoder's buffers                */
    int flag;                  /* status for readwriter                      */
};

//void alder_kmer_encoder5_lock_reader(alder_kmer_encoder5_t *a, int encoder_id);
//void alder_kmer_encoder5_unlock_reader(alder_kmer_encoder5_t *a, int encoder_id);
//void alder_kmer_encoder5_lock_writer(alder_kmer_encoder5_t *a, int part_id);
//void alder_kmer_encoder5_unlock_writer(alder_kmer_encoder5_t *a, int part_id);

__END_DECLS


#endif /* alder_kmer_alder_kmer_encode5_h */
