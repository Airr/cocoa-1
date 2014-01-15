/**
 * This file, alder_fastq_streamer.c, is part of alder-fastq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fastq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fastq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "alder_fastq_streamer.h"

/* This function reads a FASTQ file.
 *
 * returns
 * 0 on success
 * 1 if an error occurs.
 *
 * 1. Find the sequence line.
 * 2. Convert the sequence to an array of numbers.
 * 3. If a character is not one of 4 bases, truncate it.
 */
int alder_fastq_streamer(int *len_content, char *buf, char *buf2,
                         size_t size_buf, FILE *fp,
                         int kmer_size, int max_name)
{
    int end_buf = (int)fread(buf2, 1, size_buf, fp);
    
    if (buf2[0] != '@') {
        return 1;
    }
    
    size_t i_header = 0;
    size_t x = 1000; // I need to find the last read.
    size_t c = 0;
    size_t l = end_buf;
    while (x < l) {
        // A new line at the first line.
        char *newline1 = memchr(buf + c, '\n', l);
        size_t len1 = (newline1 - (buf + c))/sizeof(*newline1);
        l = l - len1 - 1;
        char *newline2 = memchr(newline1 + 1, '\n', l);
        size_t len2 = (newline2 - newline1)/sizeof(*newline1) - 1;
        l = l - len2 - 1;
        char *newline3 = memchr(newline2 + 1, '\n', l);
        size_t len3 = (newline3 - newline2)/sizeof(*newline1) - 1;
        l = l - len3 - 1;
        char *newline4 = memchr(newline3 + 1, '\n', l);
        size_t len4 = (newline4 - newline3)/sizeof(*newline1) - 1;
        l = l - len4 - 1;
        
        size_t i_buf1 = 0;
        for (size_t i_buf2 = 0; i_buf2 < len2; i_buf2++) {
            char c2 = *(newline1 + 1 + i_buf2);
            if (c2 == 'A' || c2 == 'a') {
                buf[i_header+i_buf1] = 0x00;
                i_buf1++;
            } else if (c2 == 'C' || c2 == 'c') {
                buf[i_header+i_buf1] = 0x01;
                i_buf1++;
            } else if (c2 == 'T' || c2 == 't') {
                buf[i_header+i_buf1] = 0x02;
                i_buf1++;
            } else if (c2 == 'G' || c2 == 'g') {
                buf[i_header+i_buf1] = 0x03;
                i_buf1++;
            } else {
                // Ignore the neighboring k-1 characters.
                if (i_buf1 < kmer_size) {
                    // i_header stays put.
                } else {
                    buf[i_header+i_buf1] = 0xFF;
                    i_buf1++;
                    i_header += i_buf1;
                }
                i_buf1 = 0;
            }
            if (i_buf2 - 1 == len2 && i_buf1 > 0) {
                buf[i_header+i_buf1] = 0xFF;
                i_buf1++;
                i_header += i_buf1;
            }
        }
    }
    
    // Push back remaining characters.
    assert(buf2[end_buf - l] == '@');
    for (int i = 0; i < l; i++) {
        ungetc((int)buf2[end_buf - 1 - i], fp);
    }
    char at_character = fgetc(fp);
    assert(at_character == '@');
    ungetc(at_character, fp);
    
//    if (i_header < kmer_size) {
//        end_buf = 0;
//    } else {
//        end_buf = (int)i_header;
//    }
    
    return 0;
}