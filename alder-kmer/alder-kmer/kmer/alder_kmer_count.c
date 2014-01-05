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
#include "alder_file_availablediskspace.h"
#include "alder_logger.h"
#include "alder_progress.h"
#include "alder_fileseq.h"
#include "alder_file_size.h"
#include "alder_file_rm.h"
#include "alder_hashtable_mcas.h"
#include "alder_kmer_encode.h"
#include "alder_kmer_count.h"
#include "alder_kmer_pack.h"
#include "alder_lcfg_kmer.h"
#include "alder_kmer_thread.h"

static int
alder_kmer_count_init(uint64_t *_N_ni, uint64_t *_N_np, size_t *_N_nh,
                      size_t *_S_filesize,
                      int K, long totalmaxkmer, long D, long M, long F,
                      int n_ni, int n_np, int n_nh, int n_nt,
                      int progress_flag, int progressToErro_flag,
                      int no_partition_flag, int do_partition_flag,
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
alder_kmer_count_compute_partitionfilesize(size_t *_S_filesize,
                                           int i_ni,
                                           int n_np,
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
    bstring bfntable = bformat("%s/%s.tb2", outdir, outfile);
    alder_lcfg_kmer_writeTablefilename(bdata(bfn), bdata(bfntable));
    size_t tabfilesize;
    alder_file_size(bdata(bfntable), &tabfilesize);
    alder_lcfg_kmer_writeTablefilesize(bdata(bfn), tabfilesize);
    bdestroy(bfntable);
    bdestroy(bfn);
}

/* This function creates a kmer_t for counting kmers.
 * returns
 * kmer type on success,
 * nil if an error occurs.
 * given
 * k - kmer size
 * D - disk space in MB
 * M - memory size in MB
 * F - maximum number of open files
 * n_ni - number of iteration
 * n_np - number of partition
 * n_hashtable_size - size of the hash table
 * n_thread - number of threads
 * infile - input files
 * outdir - outfile is created in this outdir directory
 * outfile - outfile
 *
 * 1. find the value of v or the length of each sequence in the input data.
 * 2. compute ni.
 * 3. compute np.
 * 4. compute has table size.
 */
/* This function counts k-mers.
 * returns
 * ALDER_KMER_SUCCESS on success,
 * ALDER_KMER_ERROR_FILE if an output file cannot be created.
 * ALDER_KMER_ERROR_FILENOTEXIST if no such file exists.
 * procedure
 * . Initialize a set of empty lists on disk.
 * . Open D files for writing.
 * . Multiple threads reads different parts of N files
 * . Converts * and writes kmers to D files.
 * . A reader/writer thread model is used.
 * . Encode and write kmers to those files.
 */
int
alder_kmer_count(int K, long totalmaxkmer, long D, long M, long F,
                 long sizeInbuffer,
                 long sizeOutbuffer,
                 int n_ni, int n_np,
                 int n_nh,
                 int n_nt,
                 int progress_flag,
                 int progressToErro_flag,
                 int nopack_flag,
                 int no_delete_partition_flag,
                 int no_partition_flag,
                 int do_partition_flag,
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
    s = alder_kmer_count_init(&N_ni, &N_np, &N_nh, &S_filesize,
                              K, totalmaxkmer, D, M, F,
                              n_ni, n_np,
                              n_nh,
                              n_nt,
                              progress_flag, progressToErro_flag,
                              no_partition_flag, do_partition_flag,
                              nopack_flag,
                              infile, outdir,
                              outfile);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(1, "Failed to start counting.");
        return ALDER_STATUS_ERROR;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Encode and count kmers.
    bstring bht = bformat("%s/%s.tbl", outdir, outfile);
    if (bht == NULL) return ALDER_STATUS_ERROR;
    alder_log5("creating a hash count table (%s)", bdata(bht));
    FILE *fpout = NULL;
    if (!do_partition_flag) {
        fpout = fopen(bdata(bht), "wb+");
        if (fpout == NULL) {
            bdestroy(bht);
            return ALDER_STATUS_ERROR;
        }
        // This is the place to write the header part of the hash count table.
        alder_log5("writing the header of the hash count table ...");
        s = alder_hashtable_mcas_print_header_with_FILE(fpout,
                                                        K, (int)N_nh,
                                                        N_ni, N_np);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s, ALDER_STATUS_ERROR);
        alder_log("*** Kmer count ***");
    }
    for (int i = 0; i < N_ni; i++) {
        if (!no_partition_flag) {
            // Encode kmer: runs on nt-many threads.
            alder_log3("encoding iteration %d ...", i);
            alder_kmer_encode(n_nt, i, K, D, M,
                              sizeInbuffer, sizeOutbuffer,
                              N_ni, N_np,
                              S_filesize,
                              &n_byte,
                              progress_flag,
                              progressToErro_flag,
                              infile, outdir,
                              outfile);
            
        }
        
        if (!do_partition_flag) {
            // Count and save: runs on nt-many threads.
//            size_t S_partitionfilesize = 0;
//            alder_kmer_count_compute_partitionfilesize(&S_partitionfilesize,
//                                                       i, (int)N_np,
//                                                       outdir, outfile);
            
            alder_log3("counting iteration %d ...", i);
            size_t i_n_hash = 0;
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
                                       progressToErro_flag,
                                       nopack_flag,
                                       NULL,
                                       outdir,
                                       outfile);
            n_hash += i_n_hash;
            
            if (!no_delete_partition_flag) {
                alder_log3("deleting partition files in iteration %d ...", i);
                alder_kmer_count_delete_partitionfile(i, (int)N_np,
                                                      outdir, outfile);
            }
        }
    }
    if (!do_partition_flag) {
        s = alder_hashtable_mcas_printPackToFILE_count(n_hash, fpout);
        XFCLOSE(fpout);
    }
    
    if (!do_partition_flag && nopack_flag) {
        // no code.
    } else {
        // Rewrite the hash table file.
        int n_uint64 = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
        size_t hash_element_size = (n_uint64 * sizeof(uint64_t) +
                                    (sizeof(uint16_t)));
        n_byte += S_filesize/11 - n_hash * hash_element_size;
        if (progress_flag) {
            alder_progress_step(n_byte, S_filesize, progressToErro_flag);
        }
        alder_kmer_pack_kmercount(bdata(bht), outdir, outfile);
    }
    
    if (progress_flag) {
        alder_progress_end(progressToErro_flag);
    }
    
    write_config2(outfile, outdir, n_hash);
    
    bdestroy(bht);
    alder_log5("Counting Kmers has been finished!");
    return ALDER_STATUS_SUCCESS;
}

/* This function uses partition files to create a hash table of counts.
 *
 * What should be fixed?
 * 1. n_nh
 */
int
alder_kmer_count_withPartition(int i_ni, int K, long D, long M, long F,
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
    uint64_t N_np = infile->qty;
    size_t N_nh = n_nh;
    size_t S_filesize;
    size_t n_hash = 0;
    size_t n_byte = 0;
    alder_kmer_count_withPartition_init(&S_filesize, K, N_nh, n_thread,
                                        infile, outdir, outfile);
    
    ///////////////////////////////////////////////////////////////////////////
    // Encode and count kmers.
    bstring bht = bformat("%s/%s.tbl", outdir, outfile);
    if (bht == NULL) return ALDER_STATUS_ERROR;
    alder_log5("creating a hash count table (%s)", bdata(bht));
    FILE *fpout = fopen(bdata(bht), "wb");
    if (fpout == NULL) {
        bdestroy(bht);
        return ALDER_STATUS_ERROR;
    }
    // This is the place to write the header part of the hash count table.
    alder_log5("writing the header of the hash count table ...");
    s = alder_hashtable_mcas_print_header_with_FILE(fpout,
                                                    K, (int)N_nh,
                                                    1, N_np);
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s, ALDER_STATUS_ERROR);
    alder_log("*** Kmer count using partition files ***");
    // Count and save: runs on nt-many threads.
    alder_kmer_count_iteration(fpout,
                               n_thread,
                               i_ni,
                               K,
                               M,
                               sizeInbuffer,
                               0,
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
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    XFCLOSE(fpout);
    bdestroy(bht);
    alder_log5("Counting Kmers has been finished!");
    return ALDER_STATUS_SUCCESS;
}

int
alder_kmer_partition(int K, long D, long M, long F,
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
    s = alder_kmer_count(K, 0, D, M, F,
                         sizeInbuffer,
                         sizeOutbuffer,
                         n_ni, n_np,
                         n_nh,
                         n_nt,
                         progress_flag,
                         progressToError_flag,
                         0, // int nopack_flag,
                         1, // int no_delete_partition_flag,
                         0, // int no_partition_flag,
                         1, // int do_partition_flag,
                         infile, outdir,
                         outfile);
    return ALDER_STATUS_SUCCESS;
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
alder_kmer_count_init(uint64_t *_N_ni, uint64_t *_N_np, size_t *_N_nh,
                      size_t *_S_filesize,
                      int K, long totalmaxkmer, long D, long M, long F,
                      int n_ni, int n_np, int n_nh, int n_nt,
                      int progress_flag, int progressToErro_flag,
                      int no_partition_flag, int do_partition_flag,
                      int nopack_flag,
                      struct bstrList *infile, const char *outdir,
                      const char *outfile)
{
    uint64_t v = 0;
    uint64_t N_ni;
    uint64_t N_np;
    size_t N_nh;
    int s = ALDER_STATUS_SUCCESS;
    alder_log3("Start counting Kmers ...");
    
    /* Maximum number of Kmers. */
    if (totalmaxkmer == 0) {
        alder_log("Counting maximum number of kmers ...");
        s = alder_fileseq_count_kmer_files(&v, infile, K, 0, 0);
//                                           progress_flag, progressToErro_flag);
        alder_log("Maximum number of kmers is %llu.", v);
    } else {
        v = (uint64_t)totalmaxkmer;
    }
    
    /* Compute ni: 2vK = total number of bits for all of the Kmers. */
    if (n_ni < 0) {
        N_ni = v * 2 * K / (D * ALDER_MB2BYTE * 8)+ 1;
    } else {
        N_ni = n_ni;
    }

    /* Compute np = v*(2k+16)/(0.7*ni*M). */
    int n_uint64 = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    size_t hash_element_size = (n_uint64 * sizeof(uint64_t) * ALDER_BYTE2BIT +
                                (sizeof(uint16_t) * ALDER_BYTE2BIT));
    if (n_np < 0) {
        N_np = ((v * hash_element_size) /
                (ALDER_MB2BITTIMESPOINT7 * N_ni * M) + 1);
    } else {
        N_np = n_np;
    }
    if (N_np > (uint64_t)F) {
        N_np = (uint64_t)F;
    }
    
    /* Compute hash table size. */
    if (n_nh < 0) {
        N_nh = (M << 23) / hash_element_size;
    } else {
        N_nh = (size_t)n_nh;
    }
    
    /* Compute total file sizes. */
    alder_totalfile_size(infile, _S_filesize);
    
    /* Compute total file sizes for processing. */
    size_t totalByteKmer = (v * 2 * K) >> 3;
    if (!no_partition_flag && !do_partition_flag) {
        *_S_filesize = (*_S_filesize) * N_ni + totalByteKmer;
    } else if (!no_partition_flag && do_partition_flag) {
        *_S_filesize = (*_S_filesize) * N_ni;
    } else if (no_partition_flag && !do_partition_flag) {
        *_S_filesize = totalByteKmer;
    }
    if (!nopack_flag) {
        *_S_filesize += (*_S_filesize / 10);
    }
    
    /* Total file size per iteration. */
    uint64_t filesizePerIteration = v * K / 4 / N_ni;
    size_t total_hashfile_size = M * N_np * N_ni;
    size_t required_disk_space = (filesizePerIteration >> 20) + total_hashfile_size;
    
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Kmer counting setup ***");
    alder_log("%*s %d", width, "Kmer size:", K);
    alder_log("%*s %ld (MB)", width, "Disk space:", D);
    alder_log("%*s %ld (MB)", width, "Memory space:", M);
    for (int i = 0; i < infile->qty; i++) {
        alder_log("%*s [%d] %s", width, "Input file:", i + 1, bdata(infile->entry[i]));
    }
    alder_log("%*s %zu (B)", width, "Total file size:", *_S_filesize);
    alder_log("%*s %llu", width, "Number of Kmers:", v);
    alder_log("%*s %llu", width, "Number of iterations:", N_ni);
    alder_log("%*s %llu", width, "Number of partitions:", N_np);
    alder_log("%*s %llu (MB)", width, "part file size:", filesizePerIteration >> 20);
    alder_log("%*s %llu (MB)", width, "hash file size:", total_hashfile_size);
    alder_log("%*s %zu", width, "Hash table size:", N_nh);
    alder_log("%*s %d", width, "Number of threads:", n_nt);
    alder_log("%*s %s/%s.%s", width, "Count table file:",
              outdir, outfile, ALDER_KMER_TABLE_EXTENSION);
    alder_log("%*s %ld (MB)", width, "Need disk space:", required_disk_space);
    
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

