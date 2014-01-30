/**
 * This file, alder_kmer_partition.c, is part of alder-kmer.
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
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_kmer_count.h"
#include "alder_kmer_encode.h"
#include "alder_kmer_partition.h"

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
    if (version == 3) {
        //
        s = alder_kmer_partition3(version,
                                  K, D, M,
                                  min_M_table, max_M_table,
                                  F,
                                  sizeInbuffer,
                                  sizeOutbuffer,
                                  n_ni, n_np,
                                  n_nh,
                                  n_nt,
                                  progress_flag,
                                  progressToError_flag,
                                  0,
                                  binfile,
                                  binfile_given,
                                  infile,
                                  outfile_given,
                                  outdir,
                                  outfile);
    } else {
        //
        s = alder_kmer_count(version, K, 0, D,
                             M, min_M_table, max_M_table,
                             F,
                             sizeInbuffer,
                             sizeOutbuffer,
                             n_ni, n_np,
                             n_nh,
                             n_nt,
                             progress_flag,
                             progressToError_flag,
                             1, // int nopack_flag,
                             1, // int no_delete_partition_flag,
                             0, // int no_partition_flag,
                             1, // int no_count_flag,
                             infile,
                             outfile_given,
                             outdir,
                             outfile);
    }
    return s;
}


int
alder_kmer_partition3(long version,
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
    size_t n_byte = 0;
    size_t S_filesize = 0;
    
    sizeInbuffer = 1 << 16;
    sizeOutbuffer = 1 << 20;
    s = alder_kmer_encode3(n_nt, 0, K, D, M, sizeInbuffer, sizeOutbuffer,
                           n_ni, n_np, S_filesize, &n_byte,
                           progress_flag, progressToError_flag,
                           binfile_given, infile, outdir, outfile);
    
    return s;
}

