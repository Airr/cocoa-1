/**
 * This file, alder_fileseq_chunk.c, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_fileseq_type.h"
#include "alder_fasta_chunk.h"
#include "alder_fastq_chunk.h"
#include "alder_seq_chunk.h"
#include "alder_fileseq_chunk.h"

/* This function returns a chunk of data from a file. 
 * It fills two buffers: buf and buf2. The capacity of the buffers is specified
 * by sizeBuf and sizeBuf2. The length of the content is marked by lenBuf and
 * lenBuf2. The buf contains the main content, and buf2 contains those that
 * are not part of the main, but would be later appended to the main content.
 * For example, let's say that I'd read a chunk from a file, but I want to 
 * send those complete reads in a FASTQ file. The ending part of a chunk 
 * might contain a partial input. Then, I'd save the partial in buf2, and 
 * in another call to this function, I'd copy buf2 to buf first, and read
 * a chunk to append it to the buf. 
 */
int
alder_fileseq_chunk(size_t *lenBuf, char *buf, size_t sizeBuf,
                    size_t *lenBuf2, char *buf2, size_t sizeBuf2,
                    int kmer_size, FILE *fp, void *fx, int type)
{
    int s = ALDER_STATUS_SUCCESS;
    assert(fp == NULL);
    if (fp != NULL) {
        if (type == ALDER_FILETYPE_FASTA) {
            s = alder_fasta_chunk2(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, kmer_size - 1, fp);
        } else if (type == ALDER_FILETYPE_FASTQ) {
            s = alder_fastq_chunk2(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, fp);
        } else if (type == ALDER_FILETYPE_SEQ) {
            s = alder_fasta_chunk2(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, kmer_size - 1, fp);
        } else {
            assert(0);
        }
    } else if (fx != NULL) {
        if (type == ALDER_FILETYPE_FASTA) {
            s = alder_fasta_chunk(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, kmer_size - 1, fx, 0);
        } else if (type == ALDER_FILETYPE_FASTQ) {
            s = alder_fastq_chunk(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, fx, 0);
        } else if (type == ALDER_FILETYPE_SEQ) {
            s = alder_fasta_chunk(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, kmer_size - 1, fx, 0);
        } else if (type == ALDER_FILETYPE_GZFASTA) {
            s = alder_fasta_chunk(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, kmer_size - 1, fx, 1);
        } else if (type == ALDER_FILETYPE_GZFASTQ) {
            s = alder_fastq_chunk(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, fx, 1);
        } else if (type == ALDER_FILETYPE_GZSEQ) {
            s = alder_fasta_chunk(lenBuf, buf, sizeBuf,
                                  lenBuf2, buf2, sizeBuf2, kmer_size - 1, fx, 1);
        } else {
            assert(0);
        }
        
    } else {
        return ALDER_STATUS_ERROR;
    }
    return s;
}

