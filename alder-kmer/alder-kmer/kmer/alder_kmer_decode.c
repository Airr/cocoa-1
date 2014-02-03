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
alder_kmer_decode(int K,
                  int progress_flag,
                  struct bstrList *infile,
                  unsigned int outfile_given,
                  const char *outdir,
                  const char *outfile)
{
    size_t lenBuf = 0;
    size_t sizeBuf = 1 << 23; /* 8 MB */
    
    sizeBuf = (int)alder_encode_number2_adjustBufferSizeForKmer(K, sizeBuf);
    
    
    bstring bfn = bformat("%s/%s.dec", outdir, outfile);
    if (bfn == NULL) {
        alder_loge(ALDER_ERR_FILE, "failed to decode kmers");
        return ALDER_STATUS_ERROR;
    }
    FILE *fpout = stdout;
    int progressToError_flag = 1;
    if (outfile_given) {
        fpout = fopen(bdata(bfn), "w");
        progressToError_flag  = 0;
    } else {
        progress_flag = 0;
    }
    
    uint8_t *buf = malloc(sizeof(*buf) * sizeBuf);
    alder_encode_number2_t *m2 = alder_encode_number2_createWithKmer(K);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(K);
    if (m == NULL || m2 == NULL || buf == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "Fatal - counter(): not enough memory");
        if (outfile_given) {
            XFCLOSE(fpout);
        }
        bdestroy(bfn);
        alder_encode_number_destroy(m);
        alder_encode_number2_destroy(m2);
        XFREE(buf);
        return ALDER_STATUS_ERROR;
    }
    
    memset(buf, 0, sizeof(*buf) * sizeBuf);
    size_t ib = m2->b / 8;
    size_t jb = m2->b % 8;

    size_t total_filesize = 0;
    int i_start = 0;
    if (infile->qty == 0) {
        assert(progress_flag == 0);
        i_start = -1;
    } else {
        alder_totalfile_size(infile, &total_filesize);
    }
    size_t i_totalFilesize = 0;
    if (progress_flag == 1) {
        alder_progress_start(1);
    }
    for (int i_file = i_start; i_file < infile->qty; i_file++) {
        
        FILE *fp = NULL;
        if (i_file == -1) {
            fp = stdin;
        } else {
            fp = fopen(bdata(infile->entry[i_file]), "rb");
            if (fp == NULL) {
                alder_loge(ALDER_ERR_FILE, "failed to open %s",
                           bdata(infile->entry[i_file]));
                if (outfile_given) {
                    XFCLOSE(fpout);
                }
                bdestroy(bfn);
                alder_encode_number_destroy(m);
                alder_encode_number2_destroy(m2);
                XFREE(buf);
                return ALDER_STATUS_ERROR;
            }
        }
        
        lenBuf = 1;
        while (lenBuf > 0) {
            lenBuf = fread(buf, sizeof(*buf), sizeBuf, fp);
            i_totalFilesize += lenBuf;
            if (progress_flag) {
                alder_progress_step(i_totalFilesize, total_filesize,
                                    progressToError_flag);
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
        if (i_file >= 0) {
            XFCLOSE(fp);
        }
    }
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    
    bdestroy(bfn);
    alder_encode_number_destroy(m);
    alder_encode_number2_destroy(m2);
    XFREE(buf);
    if (outfile_given) {
        XFCLOSE(fpout);
    }
    return ALDER_STATUS_SUCCESS;
}

