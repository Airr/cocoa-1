/**
 * This file, alder_seq_streamer.c, is part of alder-seq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-seq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-seq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-seq.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "alder_seq_streamer.h"

/* This function reads sequences from a seq file.
 * The seq file contains sequences delimited by a new line.
 *
 * 1. Inspect last k characters.
 * 2. If there is no new line, push back k-1 character to the input file.
 * 3. Otherwise, push pack characters upto the just before the new line.
 */
int alder_seq_streamer(int *len_content, char *buf, char *buf2,
                       size_t size_buf, FILE *fp,
                       int kmer_size, int max_name)
{
    
    int end_buf = (int)fread(buf2, 1, size_buf, fp);
    
    char *newline = memchr(buf2 + (end_buf - kmer_size), '\n', (size_t)kmer_size);
    if (newline == NULL) {
        for (size_t i = 0; i < kmer_size - 1; i++) {
            ungetc((int)buf2[end_buf - 1 - i], fp);
        }
    } else {
        size_t length_to_newline = ((buf2 + end_buf) - newline) / sizeof(*newline);
        for (size_t i = 0; i < length_to_newline - 1; i++) {
            assert(buf2[end_buf - 1 - i] != '\n');
            ungetc((int)buf2[end_buf - 1 - i], fp);
        }
        end_buf = (int)((newline - buf2) / sizeof(*newline));
    }
    if (end_buf < kmer_size) {
        end_buf = 0;
    } else {
        // Copy buf2 to buf1.
        for (size_t i = 0; i < end_buf; i++) {
//            buf[i] = (char)((((int)buf2[i])>>1)&3);
            if (buf2[i] == 'A' || buf2[i] == 'a') {
                buf[i] = 0x00;
            } else if (buf2[i] == 'C' || buf2[i] == 'c') {
                buf[i] = 0x01;
            } else if (buf2[i] == 'T' || buf2[i] == 't') {
                buf[i] = 0x02;
            } else if (buf2[i] == 'G' || buf2[i] == 'g') {
                buf[i] = 0x03;
            } else {
                buf[i] = 0x04;
            }
        }
    }
    *len_content = end_buf;
    
    return 0;
}