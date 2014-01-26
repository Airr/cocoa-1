/**
 * This file, alder_fastq_estimate_nkmer.c, is part of alder-fastq.
 *
 * Copyright 2014 by Sang Chul Choi
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
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "alder_cmacro.h"
#include "bzlib.h"
#include "alder_file.h"
#include "alder_fastq_estimate_nkmer.h"

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
            *v += (t1 - kmer_size + 1);
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
int alder_fastq_estimate_nkmer(uint64_t *v,
                               int kmer_size,
                               const char *fn)
{
    int s = ALDER_STATUS_SUCCESS;
    %% machine fastqKmer;
    int cs;
    
    size_t curBufsize = 0;
    size_t totalBufsize = ALDER_BUFSIZE_8MB;
    unsigned int used_byte_bzip2 = 0;
    
    int isGzip = alder_file_isgzip(fn);
    int isBzip = alder_file_isbzip2(fn);
    *v = 0;
    uint64_t t1 = 0;
    
    gzFile fpgz = Z_NULL;
    BZFILE *fpbz = NULL;
    bz_stream strm;
    FILE *fp = NULL;
    
    if (isGzip == ALDER_YES) {
        fpgz = gzopen(fn, "r");
    } else if (isBzip == ALDER_YES) {
        fp = fopen(fn, "r");
        int verbosity = 0;
        int small = 0;
        strm.bzalloc = NULL;
        strm.bzfree = NULL;
        strm.opaque = NULL;
        s = BZ2_bzDecompressInit(&strm, verbosity, small);
        if (s != BZ_OK) {
            fprintf(stderr, "Error: failed to init bz2 %s\n", fn);
            return 1;
        }
    } else {
        fp = fopen(fn, "r");
    }
    if (fpgz == Z_NULL && fpbz == NULL && fp == NULL) {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 1;
    }
    char *buf = malloc(sizeof(*buf) * ALDER_BUFSIZE_8MB);
    memset(buf,0,sizeof(*buf) * ALDER_BUFSIZE_8MB);
    char *ibuf = malloc(sizeof(*ibuf) * ALDER_BUFSIZE_8MB);
    memset(ibuf,0,sizeof(*ibuf) * ALDER_BUFSIZE_8MB);
    
    %% write init;
    while ( 1 ) {
        char *p;
        char *pe;
        char *eof;
        int len;
        
        if (totalBufsize <= curBufsize) {
            break;
        }
        if (isGzip == ALDER_YES) {
            len = gzread(fpgz, buf, ALDER_BUFSIZE_8MB);
        } else if (isBzip == ALDER_YES) {
            len = (int)fread(ibuf, sizeof(char), ALDER_BUFSIZE_8MB, fp);
            
            strm.next_in = (char *)ibuf;
            strm.avail_in = (unsigned int)len;
            
            
            strm.next_out = (char*)buf;
            strm.avail_out = (unsigned int)ALDER_BUFSIZE_8MB;
            s = BZ2_bzDecompress(&strm);
            if (BZ_OK == s || BZ_STREAM_END == s) {
                //
                used_byte_bzip2 = (unsigned int)len - strm.avail_in;
                len = (int)(strm.next_out - (char*)buf);
            } else {
                // Error!
                len = 0;
            }
        } else {
            len = (int)fread(buf, sizeof(char), ALDER_BUFSIZE_8MB, fp);
        }
        if (len <= 0) {
            break;
        }
        curBufsize += len;
        
        p = buf;
        pe = buf + len;
        if (len < ALDER_BUFSIZE_8MB) {
            eof = pe;
        }
        %% write exec;
    }
    
    /* Estimate the number of K-mers using the count. */
    size_t filesize = 0;
    alder_file_size(fn, &filesize);
    if (isGzip == ALDER_YES) {
        if (!gzeof(fpgz)) {
            z_off_t cur = gzoffset(fpgz);
            if (cur != -1) {
                *v = (uint64_t)(*v * ((double)filesize / (double)cur));
            } else {
                *v = 0;
                s = ALDER_STATUS_ERROR;
            }
        }
    } else if (isBzip == ALDER_YES) {
        if (ALDER_BUFSIZE_8MB < filesize || strm.avail_in > 0) {
            *v = (uint64_t)(*v * ((double)filesize / (double)(used_byte_bzip2)));
            
        }
    } else {
        if (ALDER_BUFSIZE_8MB < filesize) {
            *v = (uint64_t)(*v * ((double)filesize / (double)curBufsize));
        }
    }
    
    if (isGzip == ALDER_YES) {
        gzclose(fpgz);
        fpgz = Z_NULL;
    } else if (isBzip == ALDER_YES) {
        XFCLOSE(fp);
    } else {
        XFCLOSE(fp);
    }
    free(buf);
    free(ibuf);
    return s;
}
