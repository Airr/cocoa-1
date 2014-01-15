/**
 * This file, alder_kmer_count.c, is part of alder-kmer.
 *
 * Copyright 2013 by Sang Chul Choi
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
#include "alder_file.h"
#include "alder_file_availablediskspace.h"
#include "alder_logger.h"
#include "alder_progress.h"
#include "alder_fileseq.h"
#include "alder_file_size.h"
#include "alder_file_rm.h"
#include "alder_hashtable_mcas.h"
#include "alder_kmer_encode.h"
#include "alder_kmer_count.h"
#include "alder_lcfg_kmer.h"
#include "alder_kmer_thread.h"

static int
alder_kmer_count_init(long version,
                      uint64_t *_N_ni, uint64_t *_N_np, size_t *_N_nh,
                      size_t *_S_filesize, int *flag_nh64,
                      int K, long totalmaxkmer, long D, long M, long F,
                      int n_ni, int n_np, int n_nh, int n_nt,
                      int progress_flag, int progressToErro_flag,
                      int no_partition_flag, int no_count_flag,
                      int nopack_flag,
                      struct bstrList *infile, const char *outdir,
                      const char *outfile);

static int
alder_kmer_count_withPartition_init(size_t *_S_filesize,
                                    int K,
                                    size_t n_nh,
                                    int n_thread,
                                    struct bstrList *infile,
                                    const char *outdir,
                                    const char *outfile);

static int
alder_kmer_count_delete_partitionfile(int i_ni,
                                      int n_np,
                                      const char *outdir,
                                      const char *outfile);

void write_config2(const char *outfile, const char *outdir, size_t n_hash)
{
    // Config file.
    bstring bfn = bformat("%s/%s.cfg", outdir, outfile);
    alder_lcfg_kmer_writeNh(bdata(bfn), n_hash);
    bstring bfntable = bformat("%s/%s.tbl", outdir, outfile);
    alder_lcfg_kmer_writeTablefilename(bdata(bfn), bdata(bfntable));
    size_t tabfilesize;
    alder_file_size(bdata(bfntable), &tabfilesize);
    alder_lcfg_kmer_writeTablefilesize(bdata(bfn), tabfilesize);
    bdestroy(bfntable);
    bdestroy(bfn);
}

int
open_table_file(FILE **fpout_p, const char *outfile, const char *outdir, int K,
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
        s = alder_hashtable_mcas_printHeaderToFILE(*fpout_p,
                                                   K,
                                                   (uint64_t)N_nh,
                                                   N_ni, N_np);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s, ALDER_STATUS_ERROR);
    }
    bdestroy(bht);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function counts kmers.
 *
 *  @param K                        kmer size
 *  @param totalmaxkmer             total number of kmers. If this is 0, then
 *                                  the number is estimated from input files.
 *  @param D                        disk space in MB
 *  @param M                        memory size in MB
 *  @param F                        maximum number of open files
 *  @param sizeInbuffer             input buffer size in partitioning step
 *  @param sizeOutbuffer            output buffer size in partitioning step
 *  @param n_ni                     iterations. If negative, a value is
 *                                  computed from input files, and other info.
 *  @param n_np                     partitions. If negative, a value is
 *                                  computed from input files, and other info.
 *  @param n_nh                     hash table size per partition. If negative, 
 *                                  a value is computed.
 *  @param n_nt                     number of threads or CPU cores
 *  @param progress_flag            displays progress
 *  @param progressToErro_flag      displays progress to standard error
 *  @param nopack_flag              1 for not packing the resulting hash table;
 *                                  default 0.
 *  @param no_delete_partition_flag 1 for deleting partition files; default 0.
 *  @param no_partition_flag        1 for no partitioning step; default 0.
 *  @param no_count_flag        1 for no counting, but partitioning step.
 *                                  default 0.
 *  @param infile                   a list of input files
 *  @param outdir                   output directory
 *  @param outfile                  output file name prefix
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_count(long version,
                 int K, long totalmaxkmer, long D, long M, long F,
                 long sizeInbuffer,
                 long sizeOutbuffer,
                 int n_ni, int n_np,
                 int n_nh,
                 int n_nt,
                 int progress_flag,
                 int progressToError_flag,
                 int nopack_flag,
                 int no_delete_partition_flag,
                 int no_partition_flag,
                 int no_count_flag,
                 struct bstrList *infile, const char *outdir,
                 const char *outfile)
{
    int s = ALDER_STATUS_SUCCESS;
    uint64_t N_ni;
    uint64_t N_np;
    size_t N_nh;
    size_t S_filesize;
    size_t n_hash = 0;
    size_t n_byte = 0;
    int flag_nh64 = 0;
    s = alder_kmer_count_init(version, &N_ni, &N_np, &N_nh, &S_filesize,
                              &flag_nh64,
                              K, totalmaxkmer, D, M, F,
                              n_ni, n_np,
                              n_nh,
                              n_nt,
                              progress_flag, progressToError_flag,
                              no_partition_flag, no_count_flag,
                              nopack_flag,
                              infile, outdir,
                              outfile);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(1, "Failed to start counting.");
        return ALDER_STATUS_ERROR;
    }
    
    FILE *fpout = NULL;
    s = open_table_file(&fpout, outfile, outdir,
                        K, N_np, N_ni, N_nh, no_count_flag);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(1, "Failed to open a table file.");
        return ALDER_STATUS_ERROR;
    }
    alder_log("*** Kmer count ***");
    for (int i = 0; i < N_ni; i++) {
        if (!no_partition_flag) {
            // Encode kmer: runs on nt-many threads.
            alder_log3("encoding iteration %d ...", i);
            if (version == 1) {
                alder_kmer_encode(n_nt, i, K, D, M,
                                  sizeInbuffer, sizeOutbuffer,
                                  N_ni, N_np,
                                  S_filesize,
                                  &n_byte,
                                  progress_flag,
                                  progressToError_flag,
                                  infile, outdir,
                                  outfile);
            } else if (version == 2) {
                alder_kmer_encode2(n_nt, i, K, D, M,
                                   sizeInbuffer, sizeOutbuffer,
                                   N_ni, N_np,
                                   S_filesize,
                                   &n_byte,
                                   progress_flag,
                                   progressToError_flag,
                                   infile, outdir,
                                   outfile);
            } else if (version == 3) {
                alder_kmer_encode3(n_nt, i, K, D, M,
                                   sizeInbuffer, sizeOutbuffer,
                                   N_ni, N_np,
                                   S_filesize,
                                   &n_byte,
                                   progress_flag,
                                   progressToError_flag,
                                   infile, outdir,
                                   outfile);
            } else {
                assert(0);
            }
        }
        
        if (!no_count_flag) {
            // Count and save: runs on nt-many threads.
            alder_log3("counting iteration %d ...", i);
            size_t i_n_hash = 0;
            
            if (version == 1) {
                alder_kmer_count_iteration(fpout,
                                           n_nt,
                                           i,
                                           K,
                                           M,
                                           sizeInbuffer,
                                           N_ni,
                                           N_np,
                                           N_nh,
                                           S_filesize,
                                           &n_byte,
                                           &i_n_hash,
                                           progress_flag,
                                           progressToError_flag,
                                           nopack_flag,
                                           NULL,
                                           outdir,
                                           outfile);
            } else if (version == 2) {
                alder_kmer_count_iteration2(fpout,
                                            n_nt,
                                            i,
                                            K,
                                            M,
                                            sizeInbuffer,
                                            N_ni,
                                            N_np,
                                            N_nh,
                                            S_filesize,
                                            &n_byte,
                                            &i_n_hash,
                                            progress_flag,
                                            progressToError_flag,
                                            nopack_flag,
                                            NULL,
                                            outdir,
                                            outfile);
            } else if (version == 3) {
                alder_kmer_count_iteration3(fpout,
                                            n_nt,
                                            i,
                                            K,
                                            M,
                                            sizeInbuffer,
                                            N_ni,
                                            N_np,
                                            N_nh,
                                            S_filesize,
                                            &n_byte,
                                            &i_n_hash,
                                            progress_flag,
                                            progressToError_flag,
                                            nopack_flag,
                                            NULL,
                                            outdir,
                                            outfile);
            }
            n_hash += i_n_hash;
            
            if (!no_delete_partition_flag) {
                alder_log3("deleting partition files in iteration %d ...", i);
                alder_kmer_count_delete_partitionfile(i, (int)N_np,
                                                      outdir, outfile);
            }
        }
    }
    if (!no_count_flag) {
        s = alder_hashtable_mcas_printPackToFILE_count(n_hash, fpout);
        assert(s == ALDER_STATUS_SUCCESS);
        XFCLOSE(fpout);
    }
    
    if (!no_count_flag && !nopack_flag) {
        write_config2(outfile, outdir, n_hash);
    }
    
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    
    alder_log5("Counting Kmers has been finished!");
    return ALDER_STATUS_SUCCESS;
}

int
alder_kmer_partition(long version,
                     int K, long D, long M, long F,
                     long sizeInbuffer,
                     long sizeOutbuffer,
                     int n_ni, int n_np,
                     int n_nh,
                     int n_nt,
                     int progress_flag,
                     int progressToError_flag,
                     struct bstrList *infile, const char *outdir,
                     const char *outfile)
{
    
    int s = ALDER_STATUS_SUCCESS;
    s = alder_kmer_count(version, K, 0, D, M, F,
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
                         infile, outdir,
                         outfile);
    return s;
}

/* This function uses partition files to create a hash table of counts.
 *
 * What should be fixed?
 * 1. n_nh
 */
int
alder_kmer_count_withPartition(long version,
                               int i_ni, int K, long M, long F,
                               long sizeInbuffer,
                               long sizeOutbuffer,
                               int n_ni, int n_np,
                               int n_nh,
                               int n_thread,
                               int progress_flag,
                               int progressToError_flag,
                               int nopack_flag,
                               struct bstrList *infile, const char *outdir,
                               const char *outfile)
{
    int s = ALDER_STATUS_SUCCESS;
    uint64_t N_ni = 1;
    uint64_t N_np = infile->qty;
    size_t N_nh = n_nh;
    size_t S_filesize;
    size_t n_hash = 0;
    size_t n_byte = 0;
    alder_kmer_count_withPartition_init(&S_filesize, K, N_nh, n_thread,
                                        infile, outdir, outfile);
    
    FILE *fpout = NULL;
    s = open_table_file(&fpout, outfile, outdir,
                        K, N_np, N_ni, N_nh, 0);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(1, "Failed to open a table file.");
        return ALDER_STATUS_ERROR;
    }
    alder_log("*** Kmer count using partition files ***");
    // Count and save: runs on nt-many threads.
    if (version == 1) {
        alder_kmer_count_iteration(fpout,
                                   n_thread,
                                   i_ni,
                                   K,
                                   M,
                                   sizeInbuffer,
                                   N_ni,
                                   N_np,
                                   N_nh,
                                   S_filesize,
                                   &n_byte,
                                   &n_hash,
                                   progress_flag,
                                   progressToError_flag,
                                   nopack_flag,
                                   infile,
                                   outdir,
                                   outfile);
    } else if (version == 2) {
        alder_kmer_count_iteration2(fpout,
                                    n_thread,
                                    i_ni,
                                    K,
                                    M,
                                    sizeInbuffer,
                                    N_ni,
                                    N_np,
                                    N_nh,
                                    S_filesize,
                                    &n_byte,
                                    &n_hash,
                                    progress_flag,
                                    progressToError_flag,
                                    nopack_flag,
                                    infile,
                                    outdir,
                                    outfile);
    } else if (version == 3) {
        alder_kmer_count_iteration3(fpout,
                                    n_thread,
                                    i_ni,
                                    K,
                                    M,
                                    sizeInbuffer,
                                    N_ni,
                                    N_np,
                                    N_nh,
                                    S_filesize,
                                    &n_byte,
                                    &n_hash,
                                    progress_flag,
                                    progressToError_flag,
                                    nopack_flag,
                                    infile,
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

void write_config1(const char *outfile, const char *outdir, int K,
                   uint64_t v, uint64_t N_ni, uint64_t N_np,
                   uint64_t filesizePerIteration)
{
    // Config file.
    bstring bfn = bformat("%s/%s.cfg", outdir, outfile);
    alder_lcfg_kmer_openForWrite(bdata(bfn));
    alder_lcfg_kmer_writeKmersize(bdata(bfn), K);
    alder_lcfg_kmer_writeTotalkmer(bdata(bfn), v);
    alder_lcfg_kmer_writeNi(bdata(bfn), N_ni);
    alder_lcfg_kmer_writeNp(bdata(bfn), N_np);
    alder_lcfg_kmer_writeParfilesize(bdata(bfn), filesizePerIteration);
    bdestroy(bfn);
}

/* This computes ni, np, nh, and file size using
 * K: kmer size
 * D: disk space (MB)
 * M: memory (MB)
 * F: max number of files
 * n_nt: number of threads
 * infile: input file names
 *
 * If ni, np, nh are given, I'd use them instead.
 *
 * ni: All of the Kmers are needed to be stored in files before counting them.
 *     Total bits for storing them are 2vK because each letter needs two bits 
 *     and vK is the total number of letters.
 * np: Total bits for all of Kmers
 *     ------------------------
 *     Available memory in bits
 */
static int
alder_kmer_count_init(long version,
                      uint64_t *_N_ni, uint64_t *_N_np, size_t *_N_nh,
                      size_t *_S_filesize, int *flag_nh64,
                      int K, long totalmaxkmer, long D, long M, long F,
                      int n_ni, int n_np, int n_nh, int n_nt,
                      int progress_flag, int progressToErro_flag,
                      int no_partition_flag, int no_count_flag,
                      int nopack_flag,
                      struct bstrList *infile, const char *outdir,
                      const char *outfile)
{
    size_t vFilesize = 0;
    uint64_t v = 0;
    uint64_t N_ni = 0;
    uint64_t N_np = 0;
    size_t N_nh = 0;
    int s = ALDER_STATUS_SUCCESS;
    alder_log3("Start counting Kmers ...");
    
    /* Maximum number of Kmers and total file size. */
    if (totalmaxkmer == 0) {
        int isZipped = 0;
        for (int i = 0; i < infile->qty; i++) {
            isZipped += alder_file_isgzip(bdata(infile->entry[i]));
            isZipped += alder_file_isbzip2(bdata(infile->entry[i]));
        }
        if (isZipped == 0) {
            alder_log("Using total file size to estimate maximum number of kmers ...");
            alder_totalfile_size(infile, &vFilesize);
            s = alder_fileseq_estimate_kmer_files(&v, infile, K, 0, 0);
            assert(s == ALDER_STATUS_SUCCESS);
        } else {
            alder_log("Counting maximum number of kmers ...");
            s = alder_fileseq_count_kmer_files(&v, &vFilesize, infile, K, 0, 0);
            assert(s == ALDER_STATUS_SUCCESS);
        }
        alder_log("Maximum number of kmers is %llu.", v);
        alder_log("Size of all of the files is %zu.", vFilesize);
    } else {
        v = (uint64_t)totalmaxkmer;
    }
    
    /* Setup of ni, np, and nh. */
    if (version == 1) {
        /* Compute ni: 2vK = total number of bits for all of the Kmers. */
        N_ni = v * 2 * K / (D * ALDER_MB2BYTE * 8)+ 1;
        
        /* Compute np = v*(2k+16)/(0.7*ni*M). */
        size_t size_element = alder_hashtable_mcas_element_sizeof(K);
        size_t hash_element_size = size_element * ALDER_BYTE2BIT;
        
        N_np = ((v * hash_element_size) / (ALDER_MB2BITTIMESPOINT7 * N_ni * M) + 1);
        
        if (N_np > (uint64_t)F) {
//            N_np = (uint64_t)F;
            assert(0);
            abort();
            // Increase ni.
        }
        
        /* Compute hash table size. */
        N_nh = (((size_t)M) << 23) / hash_element_size;
        
    } else if (version > 1) {
        int b = alder_encode_number2_bytesize(K);
        size_t totalByteKmer = v * b;
        // ni: number of iterations
        N_ni = totalByteKmer / (D << 20) + 1;
        // table size:
        
        size_t size_element = alder_hashtable_mcas_element_sizeof(K);
        if (n_nt > 1) {
            N_nh = (((size_t)M/2) << 20) / size_element;
        } else if (n_nt == 1) {
            N_nh = (((size_t)M) << 20) / size_element;
        }
        size_t N_nh_point7 = N_nh * 7 / 10 + 1;
        N_np = ALDER_BYTESIZE_KMER(v,N_ni) / N_nh_point7 + 1;
        if (N_np > (uint64_t)F) {
            N_ni = ALDER_BYTESIZE_KMER(N_ni * N_np, F);
            N_np = (uint64_t)F;
            alder_loge(ALDER_ERR,
                       "maximum of number of open files is limited: "
                       "number of iterations is increased.");
        }
    }
    if (n_ni > 0) {
        N_ni = n_ni;
    }
    if (n_np > 0) {
        N_np = n_np;
    }
    if (n_nh > 0) {
        N_nh = (size_t)n_nh;
    }
    if (N_nh > (size_t)UINT32_MAX) {
        *flag_nh64 = 1;
        alder_log("Hash table size is over UINT32: table index should be "
                  "of 64-bit value.");
    } else {
        *flag_nh64 = 0;
    }
    
    assert(N_ni > 0 && N_np > 0 && N_nh > 0);
    
    /* Compute total file sizes. */
//    alder_totalfile_size(infile, _S_filesize);
    *_S_filesize = vFilesize;
    
    /* Compute total file sizes for processing. */
    size_t totalByteKmer;
    if (version == 1) {
        totalByteKmer = (v * 2 * K) >> 3;
    } else if (version >= 2) {
        int b = alder_encode_number2_bytesize(K);
        totalByteKmer = v * b; // FIXME: what is for version 2?
    } else {
        assert(0);
        totalByteKmer = 0;
    }
    
    if (!no_partition_flag && !no_count_flag) {
        *_S_filesize = (*_S_filesize) * N_ni + totalByteKmer;
    } else if (!no_partition_flag && no_count_flag) {
        *_S_filesize = (*_S_filesize) * N_ni;
    } else if (no_partition_flag && !no_count_flag) {
        *_S_filesize = totalByteKmer;
    }
//    if (!nopack_flag) {
//        *_S_filesize += (*_S_filesize / 10);
//    }
    
    /* Total file size per iteration. */
    uint64_t filesizePerIteration = totalByteKmer / N_ni;
//    size_t total_hashfile_size = M * N_np * N_ni;
    size_t required_disk_space = (filesizePerIteration >> 20); // + total_hashfile_size;
    
    /* PRINT SETUP */
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Kmer counting setup ***");
    alder_log("%*s %d", width, "Kmer size:", K);
    alder_log("%*s %ld (MB)", width, "Disk space:", D);
    alder_log("%*s %ld (MB)", width, "Memory space:", M);
    alder_log("%*s %d", width, "Number of threads:", n_nt);
    for (int i = 0; i < infile->qty; i++) {
        alder_log("%*s [%d] %s", width, "Input file:", i + 1, bdata(infile->entry[i]));
    }
    alder_log("%*s %zu (B)", width, "Total file size:", *_S_filesize);
    alder_log("%*s %llu", width, "Estimated num. Kmers:", v);
    alder_log("%*s %llu", width, "Number of iterations:", N_ni);
    alder_log("%*s %llu", width, "Number of partitions:", N_np);
    alder_log("%*s %zu", width, "Hash table size:", N_nh);
    alder_log("%*s %llu (MB)", width, "part file size:", filesizePerIteration >> 20);
    alder_log("%*s %s/%s.%s", width, "Count table file:",
              outdir, outfile, ALDER_KMER_TABLE_EXTENSION);
    
    alder_log("%*s %ld (MB)", width, "Need disk space:", required_disk_space);
//    alder_log("%*s %llu (MB)", width, "hash file size:", total_hashfile_size);
    
    unsigned long disk_space = 0;
    alder_file_availablediskspace(outdir, &disk_space);
    size_t available_diskspace = (size_t)(disk_space/ALDER_MB2BYTE);
    if (available_diskspace < required_disk_space) {
        alder_loge(1, "Available disk space: %zu (MB)", available_diskspace);
        alder_loge(1, "Required disk space: %zu (MB)", required_disk_space);
        alder_loge(1, "Need more disk space: %zu (MB)",
                   required_disk_space - available_diskspace);
        return ALDER_STATUS_ERROR;
    }

    *_N_ni = N_ni;
    *_N_np = N_np;
    *_N_nh = N_nh;
    
    write_config1(outfile, outdir, K, v, N_ni, N_np, filesizePerIteration);
    
    return ALDER_STATUS_SUCCESS;
}

int
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
    alder_log("*** Kmer counting setup ***");
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


//int
//alder_kmer_count_compute_partitionfilesize(size_t *_S_filesize,
//                                           int i_ni,
//                                           int n_np,
//                                           const char *outdir,
//                                           const char *outfile)
//{
//    size_t totalFilesize = 0;
//    for (int i = 0; i < n_np; i++) {
//        bstring bfpar = bformat("%s/%s-%llu-%llu.par", outdir, outfile,
//                                i_ni, i);
//        size_t filesize = 0;
//        alder_file_size(bdata(bfpar), &filesize);
//        totalFilesize += filesize;
//        bdestroy(bfpar);
//    }
//    *_S_filesize = totalFilesize;
//    return ALDER_STATUS_SUCCESS;
//}

int
alder_kmer_count_delete_partitionfile(int i_ni,
                                      int n_np,
                                      const char *outdir,
                                      const char *outfile)
{
    for (int i = 0; i < n_np; i++) {
        bstring bfpar = bformat("%s/%s-%llu-%llu.par", outdir, outfile,
                                i_ni, i);
        alder_file_rm(bdata(bfpar));
        bdestroy(bfpar);
    }
    return ALDER_STATUS_SUCCESS;
}

