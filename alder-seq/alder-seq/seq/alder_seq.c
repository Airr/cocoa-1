/**
 * This file, alder_seq.c, is part of alder-seq.
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
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include "alder_cmacro.h"

char *slurp(const char *file, size_t *len) {
    struct stat s;
    char *buf;
    int fd;
    if (stat(file, &s) == -1) {
        fprintf(stderr,"can't stat %s: %s\n", file, strerror(errno));
        return NULL;
    }
    *len = s.st_size;
    if ( (fd = open(file, O_RDONLY)) == -1) {
        fprintf(stderr,"can't open %s: %s\n", file, strerror(errno));
        return NULL;
    }
    buf = malloc(*len);
    if (buf) {
        if (read(fd, buf,*len) != *len) {
            fprintf(stderr,"incomplete read\n");
            return NULL;
        }
    }
    close(fd);
    return buf;
}

/* This function counts kmers in a sequence file.
 * A sequence file contains lines of sequences. Each line represents a
 * sequence. A new line delimits sequences. This is one of simplest format
 * of sequences files.
 *
 * returns
 * SUCCESS or FAIL
 */
int alder_seq_count_kmer(const char *fn, int kmer_size, uint64_t *volume)
{
    uint64_t n = 0;
    char *buf;
    size_t len;;
    
    buf = slurp(fn, &len);
    
    size_t c = 0;
    size_t r = len;
    char *next_c = (char*)1;
    while (next_c != NULL) {
        next_c = memchr(buf + c, '\n', r);
        if (next_c != NULL) {
            size_t l = (next_c - (buf + c))/sizeof(*next_c);
            if (!(l < (size_t)kmer_size)) {
                n += (l - (size_t)kmer_size + 1);
            }
            r -= (l + 1);
            c += (l + 1);
        }
    }

    XFREE(buf);
    *volume = n;
    return ALDER_STATUS_SUCCESS;
}