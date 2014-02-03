/**
 * This file, alder_kmer_table.c, is part of alder-kmer.
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
#include "alder_logger.h"
#include "alder_file_size.h"
#include "alder_cmacro.h"
#include "alder_kmer_count.h"
#include "alder_hashtable_mcas.h"
#include "alder_kmer_thread.h"
#include "alder_progress.h"
#include "alder_kmer_table.h"

static int
open_table_file(long version,
                FILE **fpout_p,
                unsigned int outfile_given,
                const char *outfile, const char *outdir, int K,
                uint64_t N_np, uint64_t N_ni, size_t N_nh, int no_count_flag)
{
    ///////////////////////////////////////////////////////////////////////////
    // Encode and count kmers.
    int s;
    bstring bht = bformat("%s/%s.tbl", outdir, outfile);
    if (bht == NULL) return ALDER_STATUS_ERROR;
    alder_log5("creating a hash count table (%s)", bdata(bht));
    
    *fpout_p = NULL;
    if (!no_count_flag) {
        *fpout_p = fopen(bdata(bht), "wb+");
        if (*fpout_p == NULL) {
            bdestroy(bht);
            return ALDER_STATUS_ERROR;
        }
        // This is the place to write the header part of the hash count table.
        alder_log5("writing the header of the hash count table ...");
        s = alder_hashtable_mcas_printHeaderToFD(fileno(*fpout_p),
                                                 K,
                                                 (uint64_t)N_nh,
                                                 N_ni, N_np);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s, ALDER_STATUS_ERROR);
    }
    bdestroy(bht);
    return ALDER_STATUS_SUCCESS;
}

static int
alder_kmer_count_withPartition_init(size_t *_S_filesize,
                                    int K,
                                    size_t n_nh,
                                    int n_thread,
                                    struct bstrList *infile,
                                    const char *outdir,
                                    const char *outfile)
{
    uint64_t v = 0;
    alder_log3("Start counting Kmers using partition files ...");
    /* Compute total file sizes. */
    size_t totalFilesize = 0;
    for (int i = 0; i < infile->qty; i++) {
        size_t filesize = 0;
        alder_file_size(bdata(infile->entry[i]), &filesize);
        totalFilesize += filesize;
    }
    *_S_filesize = totalFilesize;
    
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Kmer count setup ***");
    alder_log("%*s %d", width, "Kmer size:", K);
    alder_log("%*s N/A", width, "Disk space (MB):");
    alder_log("%*s N/A", width, "Memory space (MB):");
    for (int i = 0; i < infile->qty; i++) {
        alder_log2("%*s [%d] %s", width, "Input file:", i + 1, bdata(infile->entry[i]));
    }
    alder_log("%*s %zu", width, "Total file size (B):", totalFilesize);
    alder_log("%*s %llu", width, "Number of Kmers:", v);
    alder_log("%*s N/A", width, "Number of iterations:");
    alder_log("%*s N/A", width, "Number of partitions:");
    alder_log("%*s %zu", width, "Hash table size:", n_nh);
    alder_log("%*s %d", width, "Number of threads:", n_thread);
    alder_log("%*s %s/%s.%s", width, "Count table file:",
              outdir, outfile, ALDER_KMER_TABLE_EXTENSION);
    
    return ALDER_STATUS_SUCCESS;
}

static bstring
filename_partition_with_thread(const char *parfile, int i_ni, int i_np, int counter_id)
{
    /* File name setup */
    bstring bfpar = bformat("%s-%llu-%llu-%d.par", parfile, i_ni, i_np, counter_id);
    if (bfpar == NULL) return NULL;
    return bfpar;
}

static bstring
filename_partition(const char *parfile, int i_ni, int i_np)
{
    /* File name setup */
    bstring bfpar = bformat("%s-%llu-%llu.par", parfile, i_ni, i_np);
    if (bfpar == NULL) return NULL;
    return bfpar;
}

/**
 *  This function computes the number of elements in a hash table using a
 *  partition or set of partition files.
 *
 *  @param parfile  parfile prefix
 *  @param n_thread number of thread
 *  @param K        kmer size
 *
 *  @return table size if successful, otherwise 0
 */
size_t compute_nh_firstparfile(long version, const char *parfile, int n_thread, int K)
{
    /* */
    size_t N_nh = 0;
    size_t size_parfile = 0;
    if (version == 1) {
        alder_file_size(parfile, &size_parfile);
    } else if (version == 2) {
        bstring bpar = filename_partition(parfile, 0, 0);
        alder_file_size(bdata(bpar), &size_parfile);
        bdestroy(bpar);
    } else {
        for (int i = 0; i < n_thread; i++) {
            bstring bpar = filename_partition_with_thread(parfile, 0, 0, i);
            if (bpar == NULL) {
                return 0;
            }
            size_t size_one_parfile = 0;
            alder_file_size(bdata(bpar), &size_one_parfile);
            bdestroy(bpar);
            size_parfile += size_one_parfile;
        }
    }
    int b = alder_encode_number2_bytesize(K);
    N_nh = (size_t)((double)size_parfile / (double)b / ALDER_HASHTABLE_LOAD);
    return N_nh;
}

/* This function uses partition files to create a hash table of counts.
 *
 * What should be fixed?
 * 1. n_nh
 */
int
alder_kmer_table(long version,
                 int i_ni, int K, long M, long F,
                 long sizeInbuffer,
                 long sizeOutbuffer,
                 int n_ni, int n_np,
                 int n_nh,
                 int n_thread,
                 int progress_flag,
                 int progressToError_flag,
                 int nopack_flag,
                 const char *parfile,
                 unsigned int parfile_given,
                 struct bstrList *infile,
                 unsigned int outfile_given,
                 const char *outdir,
                 const char *outfile)
{
    int s = ALDER_STATUS_SUCCESS;
    count_t count;
    
    uint64_t N_ni = 1;
    uint64_t N_np = infile->qty;
    size_t N_nh = n_nh;
    size_t S_filesize;
    size_t n_hash = 0;
    size_t n_byte = 0;
    size_t n_total_kmer = 100;
    size_t n_current_kmer = 1;
    size_t n_kmer_counted = 0;
    struct bstrList *cinfile = NULL;

    alder_kmer_estimate_buffer_size(&sizeInbuffer, &sizeOutbuffer,
                                    outfile, outdir);
    
    alder_kmer_count_withPartition_init(&S_filesize, K, N_nh, n_thread,
                                        infile, outdir, outfile);
    
    
    if (parfile_given == 1) {
        N_ni = n_ni;
        N_np = n_np;
        N_nh = compute_nh_firstparfile(2, parfile, n_thread, K);
        if (N_nh == 0) {
            return ALDER_STATUS_ERROR;
        }
    }
    if (infile->qty > 0) {
        N_ni = 1;
        N_np = infile->qty;
        N_nh = 0;
        for (int i = 0; i < infile->qty; i++) {
            N_nh += compute_nh_firstparfile(1, bdata(infile->entry[i]), n_thread, K);
        }
    }
    
    FILE *fpout = NULL;
    s = open_table_file(version, &fpout, outfile_given, outfile, outdir,
                        K, N_np, N_ni, N_nh, 0);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(1, "Failed to open a table file.");
        return ALDER_STATUS_ERROR;
    }
    alder_log("*** Kmer count using partition files ***");
    // Count and save: runs on nt-many threads.
    assert(version == 2 || version == 7);
    if (version == 2) {
        count = &alder_kmer_count_iteration2;
    } else {
        count = &alder_kmer_thread7;
    }
    for (int i_ni = 0; i_ni < n_ni; i_ni++) {
        (*count)(fpout,
                 n_thread,
                 i_ni,
                 K,
                 M,
                 sizeInbuffer,
                 sizeOutbuffer,
                 N_ni,
                 N_np,
                 N_nh,
                 S_filesize,
                 &n_byte,
                 &n_kmer_counted,
                 &n_hash,
                 n_total_kmer,
                 &n_current_kmer,
                 progress_flag,
                 progressToError_flag,
                 nopack_flag,
                 NULL,
                 0,
                 cinfile,
                 outdir,
                 outfile);
    }
    
    s = alder_hashtable_mcas_printPackToFILE_count(n_hash, fpout);
    
    XFCLOSE(fpout);
    
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    
    alder_log5("Counting Kmers has been finished!");
    return s;
}
