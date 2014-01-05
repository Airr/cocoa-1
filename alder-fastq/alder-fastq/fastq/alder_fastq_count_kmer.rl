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
#include "alder_file.h"
#include "alder_progress.h"
#include "alder_fastq_count_kmer.h"

/* 8MB */
#define BUFSIZE 8388608

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

static int alder_fastq_count_kmer_file(const char *fn, int kmer_size,
                                       size_t *curBufsize, size_t totalBufsize,
                                       uint64_t *volume,
                                       int progress_flag,
                                       int progressToError_flag);
static int alder_fastq_count_kmer_gzip(const char *fn, int kmer_size,
                                       size_t *curBufsize, size_t totalBufsize,
                                       uint64_t *volume,
                                       int progress_flag,
                                       int progressToError_flag);

/* This function counts kmers (size of kmer_size) in a fastq file.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_fastq_count_kmer(const char *fn, int kmer_size,
                           size_t *curBufsize, size_t totalBufsize,
                           uint64_t *volume,
                           int progress_flag, int progressToError_flag)
{
    int s = alder_file_isgzip(fn);
    if (s) {
        alder_fastq_count_kmer_gzip(fn, kmer_size, curBufsize, totalBufsize,
                                    volume, progress_flag, progressToError_flag);
    } else {
        alder_fastq_count_kmer_file(fn, kmer_size, curBufsize, totalBufsize,
                                    volume, progress_flag, progressToError_flag);
    }
    return s;
}

static int alder_fastq_count_kmer_file(const char *fn, int kmer_size,
                                       size_t *curBufsize, size_t totalBufsize,
                                       uint64_t *volume,
                                       int progress_flag,
                                       int progressToError_flag)
{
    %% machine fastqKmer;
    int cs;
    char *buf = malloc(sizeof(*buf) * BUFSIZE);
    memset(buf,0,sizeof(*buf) * BUFSIZE);
    *volume = 0;
    uint64_t t1 = 0;
    
    FILE *fp = fopen(fn, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 1;
    }
    %% write init;
    while ( 1 ) {
        char *p;
        char *pe;
        char *eof;
        size_t len = fread(buf, sizeof(char), BUFSIZE, fp);
        *curBufsize += len;
        if (progress_flag) {
            alder_progress_step(*curBufsize, totalBufsize, progressToError_flag);
        }
        if (len == 0) {
            break;
        }
        p = buf;
        pe = buf + len;
        if (len < BUFSIZE) {
            eof = pe;
        }
        %% write exec;
    }
    fclose(fp);
    free(buf);
    return 0;
}

static int alder_fastq_count_kmer_gzip(const char *fn, int kmer_size,
                                       size_t *curBufsize, size_t totalBufsize,
                                       uint64_t *volume,
                                       int progress_flag,
                                       int progressToError_flag)
{
    %% machine fastqKmer;
    int cs;
    char *buf = malloc(sizeof(*buf) * BUFSIZE);
    memset(buf,0,sizeof(*buf) * BUFSIZE);
    *volume = 0;
    uint64_t t1 = 0;
    
    gzFile fp = gzopen(fn, "r");
//    FILE *fp = fopen(fn, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 1;
    }
    %% write init;
    while ( 1 ) {
        char *p;
        char *pe;
        char *eof;
        int len = gzread ((gzFile)fp, buf, BUFSIZE);
//        size_t len = fread(buf, sizeof(char), BUFSIZE, fp);
        *curBufsize += len;
        if (progress_flag) {
            alder_progress_step(*curBufsize, totalBufsize, progressToError_flag);
        }
        if (len < 0) {
            break;
        } else if (len == 0) {
            break;
        }
        p = buf;
        pe = buf + len;
        if (len < BUFSIZE) {
            eof = pe;
        }
        %% write exec;
    }
    gzclose(fp);
//    fclose(fp);
    free(buf);
    return 0;
}