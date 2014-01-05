/**
 * This file, alder_fileseq.c, is part of alder-fileseq.
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
#include "alder_cmacro.h"
#include "alder_file.h"
#include "alder_fasta.h"
#include "alder_fastq.h"
#include "alder_seq.h"
#include "alder_progress.h"
#include "alder_fileseq.h"

/* This function attempts to fill the structure pointed to by buf with sequence
 * statistics in file fn. Pass a pathname fn which refers to a file on the file 
 * system.
 * returns
 * 0 on success,
 * 1 if an error occurs.
 */
int alder_fileseq_stat(const char *fn, alder_fileseq_stat_t *buf, int k)
{
    int s = 0;
    buf->v = 0;
    size_t curBufsize = 0;
    size_t totalBufsize = 0;
    s = alder_file_isfasta(fn);
    if (s) {
        uint64_t v = 0;
        s = alder_fasta_count_kmer(fn, k, &curBufsize, totalBufsize, &v, 0, 0);
        buf->v = v;
    }
    s = alder_file_isfastq(fn);
    if (s) {
        if (buf->v != 0) {
            return 1;
        }
        uint64_t v = 0;
//        s = alder_fastq_count_kmer(fn, k, &v);
        buf->v = v;
    }
    return 0;
}

/* This function counts total number of kmers in files using sequence lengths.
 * returns
 * SUCESS or FAIL
 * given
 * infile - input files
 * k - kmer size
 */
int alder_fileseq_count_kmer_files(uint64_t *v, struct bstrList *infile, int k,
                                   int progress_flag, int progressToError_flag)
{
    int s = ALDER_STATUS_SUCCESS;
    
    /* Compute total file sizes. */
    size_t totalFilesize = 0;
    alder_totalfile_size(infile, &totalFilesize);
    size_t curFilesize = 0;
    
    uint64_t n = 0;
    for (int i = 0; i < infile->qty; i++) {
        uint64_t m = 0;
        s = alder_fileseq_count_kmer_file(&m, bdata(infile->entry[i]), k,
                                          &curFilesize, totalFilesize,
                                          progress_flag, progressToError_flag);
        if (s == 0) {
            n += m;
        } else {
            n = 0;
            s = ALDER_STATUS_ERROR;
            break;
        }
    }
    *v = n;
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    return s;
}

/* This function counts kmers in a file.
 * returns
 * SUCCESS if the file contains any sequnce of letters.
 * FAIL otherwise
 *
 * given
 * infile - input files
 * k - kmer size
 */
int alder_fileseq_count_kmer_file(uint64_t *v, const char *fn, int k,
                                  size_t *curBufsize, size_t totalBufsize,
                                  int progress_flag,
                                  int progressToError_flag)
{
    int s = ALDER_STATUS_SUCCESS;
    uint64_t n = 0;
    
    s = alder_file_exist(fn);
    if (s == 0) {
        return ALDER_STATUS_ERROR;
    }
    
    if (alder_file_isfasta(fn)) {
        s = alder_fasta_count_kmer(fn, k, curBufsize, totalBufsize,
                                   &n, progress_flag, progressToError_flag);
    } else if (alder_file_isfastq(fn)) {
        s = alder_fastq_count_kmer(fn, k, curBufsize, totalBufsize,
                                   &n, progress_flag, progressToError_flag);
    } else {
        s = alder_seq_count_kmer(fn, k, &n);
    }
    if (s == 0) {
        *v = n;
    } else {
        *v = 0;
    }
    
    return s;
}
