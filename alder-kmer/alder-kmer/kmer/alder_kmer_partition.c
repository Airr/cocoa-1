/**
 * This file, alder_kmer_partition.c, is part of alder-kmer.
 *
 * Copyright 2013,2014 by Sang Chul Choi
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_kmer_count.h"
#include "alder_kmer_encode.h"
#include "alder_kmer_common.h"
#include "alder_kmer_partition.h"

/**
 *  This function converts a binary file to partition files.
 *
 *  @param version              version
 *  @param K                    K
 *  @param D                    disk space in megabyte
 *  @param M                    memory in megabyte
 *  @param min_M_table          min memory for a table
 *  @param max_M_table          max memory for a table
 *  @param F                    max number of open fi
 *  @param sizeInbuffer         size input buffer
 *  @param sizeOutbuffer        size output buffer
 *  @param n_ni                 ni
 *  @param n_np                 np
 *  @param n_nh                 nh
 *  @param n_nt                 nt
 *  @param progress_flag        progress
 *  @param progressToError_flag progrsss stderr
 *  @param use_seqfile          use seq file = no (default)
 *  @param binfile              binfile
 *  @param binfile_given        binfile given
 *  @param infile               infile
 *  @param outfile_given        outfile given
 *  @param outdir               outdir
 *  @param outfile              outfile
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_partition(long version,
                     int K, long D, long M,
                     long min_M_table, long max_M_table,
                     long F,
                     long sizeInbuffer,
                     long sizeOutbuffer,
                     int n_ni, int n_np,
                     int n_nh,
                     int n_nt,
                     int progress_flag,
                     int progressToError_flag,
                     int use_seqfile,
                     const char *binfile,
                     unsigned int binfile_given,
                     struct bstrList *infile,
                     unsigned int outfile_given,
                     const char *outdir,
                     const char *outfile)
{
    int s = ALDER_STATUS_SUCCESS;
    encode_t encode;
    
    size_t n_byte = 0;
    size_t n_kmer = 0;
    size_t S_filesize = 0;
    sizeInbuffer = ALDER_KMER_BUFSIZE;
    sizeOutbuffer = 1 << 20;
    size_t n_total_kmer = 100; // FIXME: total kmer should be known.
    size_t n_current_kmer = 0;
    assert(version == 7);
    for (int i_ni = 0; i_ni < n_ni; i_ni++) {
        encode = &alder_kmer_encode7;
        s = (*encode)(n_nt, i_ni, K, D, M, sizeInbuffer, sizeOutbuffer,
                               n_ni, n_np, S_filesize, &n_byte, &n_kmer,
                               n_total_kmer, &n_current_kmer,
                               progress_flag, progressToError_flag,
                               binfile_given, infile, outdir, outfile);
    }
    
    return s;
}

