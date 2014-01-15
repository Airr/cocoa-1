/**
 * This file, alder_kmer_decode.c, is part of alder-kmer.
 *
 * Copyright 2013 by Sang Chul Choi
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_dna.h"
#include "alder_file_size.h"
#include "alder_logger.h"
#include "alder_encode_number.h"
#include "alder_progress.h"
#include "alder_kmer_decode.h"

/**
 *  This function is a command function, or is called by the main function.
 *  It reads partition files to decode the encoded kmers.
 *
 *  @param K             kmer size
 *  @param progress_flag progress
 *  @param infile        input partition files
 *  @param outdir        output directory
 *  @param outfile       output file name prefix
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_decode2(int K,
                   int progress_flag,
                   struct bstrList *infile,
                   const char *outdir,
                   const char *outfile)
{
    size_t totalFilesize;
    size_t lenBuf = 0;
    size_t sizeBuf = 1 << 23; /* 8 MB */
    
    sizeBuf = (int)alder_encode_number2_adjustBufferSizeForKmer(K, sizeBuf);
    alder_totalfile_size(infile, &totalFilesize);
    
    bstring bfn = bformat("%s/%s.dec", outdir, outfile);
    FILE *fpout = fopen(bdata(bfn), "w");
    ALDER_RETURN_ERROR_IF_NULL(fpout, ALDER_STATUS_ERROR);
    
    uint8_t *buf = malloc(sizeof(*buf) * sizeBuf);
    alder_encode_number2_t *m2 = alder_encode_number2_createWithKmer(K);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(K);
    if (m == NULL || m2 == NULL || buf == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "Fatal - counter(): not enough memory");
        bdestroy(bfn);
        alder_encode_number_destroy(m);
        alder_encode_number2_destroy(m2);
        XFREE(buf);
        return ALDER_STATUS_ERROR;
    }
    
    memset(buf, 0, sizeof(*buf) * sizeBuf);
    size_t ib = m2->b / 8;
    size_t jb = m2->b % 8;

    size_t i_totalFilesize = 0;
    for (int i_file = 0; i_file < infile->qty; i_file++) {
        FILE *fp = fopen(bdata(infile->entry[i_file]), "rb");
        if (fp == NULL) {
            alder_loge(ALDER_ERR_FILE, "failed to open file: %s",
                       bdata(infile->entry[i_file]));
            break;
        }
        
        lenBuf = 1;
        while (lenBuf > 0) {
            lenBuf = fread(buf, sizeof(*buf), sizeBuf, fp);
            i_totalFilesize += lenBuf;
            if (progress_flag) {
                alder_progress_step(i_totalFilesize, totalFilesize, 0);
            }
            
            assert(lenBuf % m2->b == 0);
            uint64_t n_kmer = lenBuf / m2->b;
            size_t x = 0;
            for (uint64_t i_kmer = 0; i_kmer < n_kmer; i_kmer++) {
                
                /* Decode a kmer. */
                for (size_t i = 0; i < ib; i++) {
                    for (size_t j = 0; j < 8; j++) {
                        m2->n[i].key8[j] = buf[x++];
                    }
                }
                for (size_t j = 0; j < jb; j++) {
                    m2->n[ib].key8[j] = buf[x++];
                }
                for (int i = 0; i < m2->s; i++) {
                    m->n[i] = m2->n[i].key64;
                }
                
                alder_encode_number_print_DNA_and_hash(m, fpout);
            }
            
        }
        XFCLOSE(fp);
    }
    if (progress_flag) {
        alder_progress_end(0);
    }
    
    bdestroy(bfn);
    alder_encode_number_destroy(m);
    alder_encode_number2_destroy(m2);
    XFREE(buf);
    XFCLOSE(fpout);
    return ALDER_STATUS_SUCCESS;
}

/* This function decodes a partition file to create its original sequence
 * file. 
 *
 * 1. I'd simply convert the partition file to a file with Kmer sequences.
 * 2. Or, I could recover the original sequence file.
 */
int
alder_kmer_decode(int K,
                  int progress_flag,
                  struct bstrList *infile,
                  const char *outdir,
                  const char *outfile)
{
    size_t lenBuf = 0;
    size_t sizeBuf = 1 << 23;
    
    sizeBuf = (int)alder_encode_number_adjust_buffer_size_for_packed_kmer
    (K, sizeBuf);
    
    /* Compute total file sizes. */
    size_t totalFilesize = 0;
    for (int i = 0; i < infile->qty; i++) {
        size_t filesize = 0;
        alder_file_size(bdata(infile->entry[i]), &filesize);
        totalFilesize += filesize;
    }
    
    bstring bfn = bformat("%s/%s.dec", outdir, outfile);
    FILE *fpout = fopen(bdata(bfn), "w");
    if (fpout == NULL) {
        bdestroy(bfn);
        return ALDER_STATUS_ERROR;
    }
    char *buf = malloc(sizeof(*buf) * sizeBuf);
    memset(buf, 0, sizeof(*buf) * sizeBuf);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(K);
    
    size_t i_totalFilesize = 0;
    for (int i = 0; i < infile->qty; i++) {
        FILE *fp = fopen(bdata(infile->entry[i]), "rb");
        lenBuf = fread(buf, sizeof(*buf), sizeBuf, fp);
        i_totalFilesize += lenBuf;
        while (lenBuf > 0) {
            uint8_t *inbuf = (uint8_t*)buf;
            uint8_t *cbuf = inbuf;
            int i_8bit = 7;
            uint64_t n_kmer = 4 * lenBuf / K;
            for (uint64_t i = 0; i < n_kmer; i++) {
                cbuf = alder_encode_number_kmer_with_packed(m, cbuf, &i_8bit);
                if (K < 4 && m->n[0] == 0) {
                    break;
                }
                alder_encode_number_print_DNA_and_hash(m, fpout);
            }
            lenBuf = fread(buf, sizeof(*buf), sizeBuf, fp);
            i_totalFilesize += lenBuf;
            if (progress_flag) {
                alder_progress_step(i_totalFilesize, totalFilesize, 0);
            }
        }
        XFCLOSE(fp);
    }
    if (progress_flag) {
        alder_progress_end(0);
    }
    
    
    bdestroy(bfn);
    alder_encode_number_destroy(m);
    XFREE(buf);
    XFCLOSE(fpout);
    return ALDER_STATUS_SUCCESS;
}
