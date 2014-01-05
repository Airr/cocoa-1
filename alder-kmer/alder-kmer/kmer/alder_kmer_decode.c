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
#include "alder_encode_number.h"
#include "alder_progress.h"
#include "alder_kmer_decode.h"

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
    
    
    alder_encode_number_destroy(m);
    XFREE(buf);
    XFCLOSE(fpout);
    
    
    
    return ALDER_STATUS_SUCCESS;
}
