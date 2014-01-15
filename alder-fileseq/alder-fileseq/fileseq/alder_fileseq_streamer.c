/**
 * This file, alder_fileseq_streamer.c, is part of alder-kmer.
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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "alder_encode.h"
#include "alder_fileseq_type.h"
#include "alder_fasta_streamer.h"
#include "alder_fastq_streamer.h"
#include "alder_seq_streamer.h"
#include "alder_fileseq_streamer.h"

/* This function prepares for input buffer for encoding.
 * This converts sequences in FASTA to sequences in a simpler format.
 * Note that the encoder and readwriter might have to be changed if this
 * input format changes. The buf2 is an extra for storing read bytes from an
 * input file. The buf should be used as an input buffer. len_content is the
 * size of bytes to be used in buf. Both buf and buf2 are of size_buf size.
 *
 * 1. I convert A, C, G, T to 0, 1, 3, 2.
 * 2. Sequences are separated '>' or ones that are not A, C, G, T.
 * 3. Other characters are removed.
 * 4. I consider kmer size to remove characters.
 *    Neighboring characters are also removed.
 * 5. Sequences are separated by 0xFF.
 * 6. Sequences shorter than kmer_size are removed.
 *
 * returns
 * 0 on success
 * 1 if length of the content is 0 and it does reach the end of file.
 * in this case of 1, the caller should call this function until the return
 * is 0.
 */
int alder_fileseq_streamer(int *len_content, char *buf, char *buf2,
                           size_t size_buf, FILE *fp,
                           int kmer_size, int type,
                           int max_name)
{
    int s = 0;
    if (type == ALDER_FILETYPE_FASTA) {
        s = alder_fasta_streamer2(len_content, buf,
                                  buf2,
                                  size_buf, fp,
                                  kmer_size, max_name);
        while (s == 1) {
            s = alder_fasta_streamer2(len_content, buf,
                                      buf2,
                                      size_buf, fp,
                                      kmer_size, max_name);
        }
    } else if (type == ALDER_FILETYPE_FASTQ) {
        //
        s = alder_fastq_streamer(len_content, buf,
                                 buf2,
                                 size_buf, fp,
                                 kmer_size, max_name);
    } else if (type == ALDER_FILETYPE_SEQ) {
        //
        s = alder_seq_streamer(len_content, buf,
                               buf2,
                               size_buf, fp,
                               kmer_size, max_name);
    } else {
        // No code.
        assert(0);
    }
    
    // Check if I reached end-of-file.
    // Then, let the caller know that so that the file pointer is closed,
    // and set to NULL.
    // FIXME: close file if nothing to read.
    
    return s;
}

int alder_fileseq_streamer_print(char *buf, size_t buf_size)
{
    return 0;
}

