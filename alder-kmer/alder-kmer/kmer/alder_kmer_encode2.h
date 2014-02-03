/**
 * This file, alder_kmer_encode2.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_encode2_h
#define alder_kmer_alder_kmer_encode2_h

#include <stdint.h>
#include <unistd.h>
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

typedef struct alder_kmer_encoder2_struct alder_kmer_encoder2_t;

/* Readwriter thread accesses this type.
 */
struct alder_kmer_encoder2_struct {
    /* info */
    int k;                     /* k - kmer size                              */
    int b;                     /* bytes for a kmer                           */
    uint64_t n_ni;             /* n_ni - number of iteration                 */
    uint64_t n_np;             /* number of iteration                        */
    uint64_t i_ni;             /* iteration index                            */
    int n_encoder;             /* n_encoder - number of encoder threads      */
    
    /* reader */
    size_t *reader_lenbuf;
    size_t *reader_i_infile;
    
    /* buffer */
    size_t size_fixedoutbuffer;/* buffer size for output                     */
    size_t size_fixedinbuffer; /* input buffer size                          */
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
    void **fx;                 /* [n_encoder] fx - input file pointer        */
    FILE **fpout;              /* n_np: fpout - output file pointers         */
    bstring dout;              /* output directory                           */
    
    /* progress */
    size_t totalFilesize;      /* total file size                            */
    int progress_flag;
    int progressToError_flag;
    
    /* stat */
    uint64_t n_t_byte_not_last;/* number of bytes for encoders not last      */
    uint64_t n_t_byte_last;    /* number of bytes for the last encoders      */
    uint64_t *n_i_byte;        /* [n_encoder] number of bytes sent           */
    uint64_t *n_i_kmer;        /* [n_encoder] number of kmers                */
    uint64_t n_byte;           /* number of bytes sent                       */
    uint64_t n_kmer;           /* number of Kmers written to an out file     */
    uint64_t n_letter;         /* number of DNA letters processed            */
    uint64_t n_total_kmer;     /* total number of kmers need to processed    */
    uint64_t n_current_kmer;   /* current number of kmers processed          */
    
    /* flag */
    int status;                /* status of encoder's buffers                */
    int flag;                  /* status for readwriter                      */
};


__END_DECLS


#endif /* alder_kmer_alder_kmer_encode2_h */
