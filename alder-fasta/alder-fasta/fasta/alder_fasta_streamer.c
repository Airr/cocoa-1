/**
 * This file, alder_fasta_streamer.c, is part of alder-fileseq.
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

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "alder_encode.h"
#include "alder_fasta_streamer.h"

/* This function prepares for input buffer for encoding.
 * This converts a sequences in FASTA to sequences in a simpler format.
 * Note that the encoder and readwriter might have to be changed if this
 * input format changes. The buf2 is an extra for storing read bytes from an
 * input file. The buf should be used as an input buffer. len_content is the 
 * size of bytes to be used in buf. Bothe buf and buf2 are of size_buf size.
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
int alder_fasta_streamer2(int *len_content, char *buf, char *buf2,
                         size_t size_buf, FILE *fp,
                         int kmer_size, int max_name)
{
    char c1, c2;
    size_t i1, i2;
    size_t i_header;
    uint64_t nb;
    int status;
    
    int end_buf = (int)fread(buf2, 1, size_buf, fp);
    
    // Assume that the first character is '>' or bases.
    // i2 will point to the first base of a sequence.
    i2 = 0;
    c2 = buf2[i2];
    if (c2 == '>') {
        while (c2 != '\n') {
            c2 = buf2[i2++];
        }
    }
    status = ALDER_FASTA_STREAMER_STATUS_SEQ;
    
    // i1 and i_header point to the first position in the first buffer.
    // I copy necessary parts in the second buffer to those in the first.
    // nb is the length of the current sequence.
    i1 = 0;
    i_header = 0;
    nb = 0;
    while (i2 < end_buf) {
        c2 = buf2[i2++];
        c1 = 0xFF;
        if (status == ALDER_FASTA_STREAMER_STATUS_SEQ) {
            if (c2 == 'A' || c2 == 'a') {
                c1 = 0x00;
            } else if (c2 == 'C' || c2 == 'c') {
                c1 = 0x01;
            } else if (c2 == 'T' || c2 == 't') {
                c1 = 0x02;
            } else if (c2 == 'G' || c2 == 'g') {
                c1 = 0x03;
            } else if (c2 == '>') {
                // A start of a sequence
                status = ALDER_FASTA_STREAMER_STATUS_TITLE;
                // If length of sequence is less than kmer size,
                // push i1 back to the start, and chop off sequence title.
                if (nb < kmer_size) {
                    i1 = i_header;
                    nb = 0;
                    continue;
                }
                // A start of a sequence (effectively)
                c1 = 0xFF;
                buf[i1++] = c1;
                i_header = i1;
                nb = 0;
                continue;
            } else if (isspace(c2) != 0) {
                // Any spaces are removed in sequences.
                continue;
            } else if (isalpha(c2) != 0) {
                // All other characters separate the sequence.
                // A shorter sequence is removed.
                if (nb < kmer_size) {
                    i1 = i_header;
                    nb = 0;
                    continue;
                }
                // A start of a sequence (effectively)
                c1 = 0xFF;
                buf[i1++] = c1;
                i_header = i1;
                nb = 0;
                continue;
            }
            buf[i1++] = c1;
            nb++;
        } else {
            // Chop off sequence title.
            while (i2 < end_buf && c2 != '\n') {
                c2 = buf2[i2++];
            }
            if (c2 == '\n') {
                status = ALDER_FASTA_STREAMER_STATUS_SEQ;
            }
        }
    }
    
    if (status == ALDER_FASTA_STREAMER_STATUS_TITLE) {
        c2 = fgetc(fp);
        while (c2 != '\n') {
            c2 = fgetc(fp);
        }
        ungetc(c2, fp);
    } else {
        if (nb < kmer_size) {
            for (int i = 0; i < nb; i++) {
                ungetc((int)buf2[end_buf - 1 - i], fp);
            }
            i1 -= nb;
        } else {
            for (int i = 0; i < kmer_size - 1; i++) {
                ungetc((int)buf2[end_buf - 1 - i], fp);
            }
        }
    }
    *len_content = (int)i1;
    
    if (end_buf == size_buf && i1 == 0) {
        return 1;
    } else {
        return 0;
    }
}

int alder_fasta_streamer2_print(char *buf, size_t buf_size)
{
    size_t i1 = 0;
    while (i1 < buf_size) {
        uint64_t nb;
        char *sep = memchr(buf, 0xFF, buf_size);
        if (sep == NULL) {
            nb = buf_size - i1;
        } else {
            nb = (sep - buf)/sizeof(*sep) - i1;
        }
        for (uint64_t i = 0; i < nb; i++) {
            if (i != 0 && i % 10 == 0) {
                fprintf(stdout, "\n");
            }
            fprintf(stdout, "%d", (int)buf[i1]);
            i1++;
        }
        fprintf(stdout, "\n");
    }
    return 0;
}

/* This function fills buf with sequences only.
 * returns 0 on success
 *
 * len_content - a returned buffer content size
 * buf - a returned buffer
 * size_buf - the maximum size of the buffer
 * fp - FILE pointer
 *
 * 1. Search for an angle bracket in the tail of the buffer.
 *    If an angle bracket is not found, push k-1 bases to file.
 * 2. If there is one bracket in the tail, search for a new line from the '>'
 * 3. If there is a new line in the tail, count nucleotide bases after new line
 * 4. It there are tailing less than k bases, push all of them to input file.
 *    Adjust the length of the content.
 * 5. If there are tailing no less than k bases, push back k-1 bases to file.
 */
int alder_fasta_streamer(int *len_content, char *buf,
                         size_t size_buf, FILE *fp,
                         int kmer_size, int max_name)
{
    int end_buf = (int)fread(buf, 1, size_buf, fp);
    
    // 1. Search for chevron or angle bracket or > or <.
    int pos_chevron = end_buf;
    int len_check = (max_name < *len_content) ? max_name : end_buf;
    for (int i = 0; i < len_check; i++) {
        char c = ((char*)buf)[end_buf - 1 - i];
        if (c == '>') {
            pos_chevron = end_buf - 1 - i;
            break;
        }
    }
    // 2. Search for a new line from an angle brachet if there is an angle bracket.
    if (pos_chevron < end_buf) {
        int pos_newline = end_buf;
        for (int i = pos_chevron + 1; i < end_buf; i++) {
            char c = ((char*)buf)[i];
            if (c == '\n') {
                pos_newline = i;
                break;
            }
        }
        // 3. If there is a new line in the tail, count nucleotide bases after new line
        // Chop off the remaining sequence name if a new line is not found.
        if (pos_newline < end_buf) {
            int n_tailing_base = end_buf - pos_newline - 1;
            // 4. It there are tailing less than k bases, push all of them to input file.
            if (n_tailing_base < kmer_size) {
                for (int i = 0; i < n_tailing_base; i++) {
                    ungetc((int)buf[end_buf - 1 - i], fp);
                }
                end_buf = pos_chevron;
            } else {
            // 5. If there are tailing no less than k bases, push back k-1 bases to file.
                for (int i = 0; i < kmer_size - 1; i++) {
                    ungetc((int)buf[end_buf - 1 - i], fp);
                }
                // end_buf = end_buf;
            }
        } else {
            end_buf = pos_chevron;
        }
    } else {
        // If an angle bracket is not found, push k-1 bases to file.
        // Save k - 1 to append to the next stream.
        for (int i = 0; i < kmer_size - 1; i++) {
            ungetc((int)buf[end_buf - 1 - i], fp);
        }
//        end_buf = end_buf;
    }
    
    *len_content = end_buf;
    return 0;
}