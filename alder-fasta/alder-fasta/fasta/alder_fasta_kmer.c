/**
 * This file, alder_fasta_kmer.c, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "alder_cmacro.h"
#include "alder_fasta_kmer.h"

int alder_fasta_kmer_count(const char *fn, int kmer_size, size_t *volume)
{
    size_t v = 0;
    size_t bufsize = 1 << 23;
    char *buf = malloc(sizeof(*buf) * bufsize);
    memset(buf,0,sizeof(*buf) * bufsize);
    
    int fp = open(fn, O_RDONLY);
    
    ssize_t r = read(fp, buf, bufsize);
    if (r > 0) {
        v += (size_t)r;
    } else if (r == 0) {
        fprintf(stderr, "End of File!\n");
    } else {
        fprintf(stderr, "Error - read fasta: %s\n", strerror(errno));
    }
    while (r > 0) {
        r = read(fp, buf, bufsize);
        if (r > 0) {
            v += (size_t)r;
        } else if (r == 0) {
            fprintf(stderr, "End of File!\n");
        } else {
            fprintf(stderr, "Error - read fasta: %s\n", strerror(errno));
        }
    }
    close(fp);
    
//    FILE *fp = fopen(fn, "r");
//    
//    size_t r = fread(buf, sizeof(char), bufsize, fp);
//    if (r != bufsize) {
//        if (feof(fp)) {
//            fprintf(stderr, "End of File!\n");
//        } else if (ferror(fp)) {
//            fprintf(stderr, "Error - fread fasta!\n");
//        } else {
//            
//        }
//    }
//    v += r;
//    while (!feof(fp)) {
//        r = fread(buf, sizeof(char), bufsize, fp);
//        if (r != bufsize) {
//            if (feof(fp)) {
//                fprintf(stderr, "End of File!\n");
//            } else if (ferror(fp)) {
//                fprintf(stderr, "Error - fread fasta!\n");
//            } else {
//                
//            }
//        }
//        v += r;
//    }
//    XFCLOSE(fp);
    XFREE(buf);
    *volume = v;
    return ALDER_STATUS_SUCCESS;
}