/**
 * This file, alder_fastq_count_kmer.c, is part of alder-fastq.
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

#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "alder_cmacro.h"
#include "bzlib.h"
#include "alder_file.h"
#include "alder_progress.h"
#include "alder_fastq_count_kmer.h"

/* See https://github.com/lomereiter/bioragel/blob/master/examples/d/fastq.rl
 */
%%{
    machine fastqKmer;
    action e_title {
        t1 = 0;
    }
    action t_sequence {
        t1++;
    }
    action f_sequence {
        if (t1 >= kmer_size) {
            *volume += (t1 - kmer_size + 1);
        }
    }
    
    newline = ('\n' | '\r''\n');
    qual = ([!-~])+;
    seq = [^\n]+ $t_sequence %f_sequence;
    seqname = [^\n]+ >e_title;
    required_seqname = seqname;
    optional_seqname = seqname?;
    block = '@' required_seqname newline
            seq newline
            '+' optional_seqname newline
            qual newline;
    main := block+;

    write data;
}%%


/**
 *  This funciton counts the number of DNA letters to compute the maximum
 *  number of Kmers in the file.
 *
 *  @param fn                   file name
 *  @param kmer_size            kmer size
 *  @param curBufsize           for displaying progress
 *  @param totalBufsize         for displaying progress
 *  @param volume               number of kmer
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to std err
 *
 *  @return SUCCESS or FAIL
 */
int alder_fastq_count_kmer(const char *fn, int kmer_size,
                           size_t *curBufsize, size_t totalBufsize,
                           uint64_t *volume,
                           int progress_flag, int progressToError_flag)
{
    %% machine fastqKmer;
    int cs;
    
    int isGzip = alder_file_isgzip(fn);
    int isBzip = alder_file_isbzip2(fn);
    *volume = 0;
    uint64_t t1 = 0;
    
    gzFile fpgz = Z_NULL;
    BZFILE *fpbz = NULL;
    FILE *fp = NULL;
    
    if (isGzip == ALDER_YES) {
        fpgz = gzopen(fn, "r");
    } else if (isBzip == ALDER_YES) {
        fpbz = BZ2_bzopen(fn, "r");
    } else {
        fp = fopen(fn, "r");
    }
    if (fpgz == Z_NULL && fpbz == NULL && fp == NULL) {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 1;
    }
    char *buf = malloc(sizeof(*buf) * ALDER_BUFSIZE_8MB);
    memset(buf,0,sizeof(*buf) * ALDER_BUFSIZE_8MB);
    
    %% write init;
    while ( 1 ) {
        char *p;
        char *pe;
        char *eof;
        int len;
        
        if (totalBufsize <= *curBufsize) {
            break;
        }
        if (isGzip == ALDER_YES) {
            len = gzread(fpgz, buf, ALDER_BUFSIZE_8MB);
        } else if (isBzip == ALDER_YES) {
            len = BZ2_bzread(fpbz, buf, ALDER_BUFSIZE_8MB);
        } else {
            len = (int)fread(buf, sizeof(char), ALDER_BUFSIZE_8MB, fp);
        }
        if (len <= 0) {
            break;
        }
        *curBufsize += len;
        if (progress_flag) {
            alder_progress_step(*curBufsize, totalBufsize, progressToError_flag);
        }
        
        p = buf;
        pe = buf + len;
        if (len < ALDER_BUFSIZE_8MB) {
            eof = pe;
        }
        %% write exec;
    }
    
    if (isGzip == ALDER_YES) {
        gzclose(fpgz);
        fpgz = Z_NULL;
    } else if (isBzip == ALDER_YES) {
        BZ2_bzclose(fpbz);
        fpbz = NULL;
    } else {
        XFCLOSE(fp);
    }
    free(buf);
    return 0;
}
