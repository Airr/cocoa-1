/**
 * This file, alder_kmer_count.c, is part of alder-kmer.
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
#include "alder_kmer_binary.h"
#include "alder_kmer_inspect.h"
#include "alder_kmer_thread.h"

/**
 *  This function is for writing a file, which would be opened by alderg-kmer.
 *
 *  @param outfile              outfile
 *  @param outdir               outdire
 *  @param K                    K
 *  @param v                    number of kmers
 *  @param N_ni                 ni
 *  @param N_np                 np
 *  @param filesizePerIteration total file size in an iteration
 */
static
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

/**
 *  This function is for writing a file, which would be opened by alderg-kmer.
 *
 *  @param outfile outfile
 *  @param outdir  out directory
 *  @param n_hash  number of hash table
 */
static
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

static
void write_config3(const char *outfile, const char *outdir, int K,
                   uint64_t nh,
                   size_t size_value, size_t size_index,
                   uint64_t filesizePerIteration)
{
    // Config file.
    bstring bfn = bformat("%s/%s.cfg", outdir, outfile);
    alder_lcfg_kmer_openForWrite(bdata(bfn));
    alder_lcfg_kmer_writeKmersize(bdata(bfn), K);
    alder_lcfg_kmer_writeNh(bdata(bfn), nh);
    alder_lcfg_kmer_writeSizeValue(bdata(bfn), size_value);
    alder_lcfg_kmer_writeSizeIndex(bdata(bfn), size_index);
    alder_lcfg_kmer_writeParfilesize(bdata(bfn), filesizePerIteration);
    bdestroy(bfn);
}

/**
 *  This is used to set up for version 5.
 *
 *  See alder_kmer_count_init for the explanation of the parameters, which
 *  are a little different.
 */
static int
alder_kmer_count_init6(int force_version, long *version,
                       uint64_t *_N_ni, uint64_t *_N_np, size_t *_N_nh,
                       size_t *_S_filesize, int *flag_nh64,
                       long *sizeInbuffer, long *sizeOutbuffer,
                       int K, uint64_t n_kmer, long D,
                       long M, long min_M_table, long max_M_table,
                       long F,
                       int n_ni, int n_np, int n_nh, int n_nt,
                       int progress_flag, int progressToErro_flag,
                       int no_partition_flag, int no_count_flag,
                       int nopack_flag,
                       struct bstrList *infile, const char *outdir,
                       const char *outfile)
{
    assert(*version == 7);
    assert(n_kmer > 0);
    /* Setup ni, np, and nh, and version. */
    uint64_t N_ni = 0;
    uint64_t N_np = 0;
    size_t N_nh = 0;
    size_t size_n_kmer = 0;
    size_t size_element_table = alder_hashtable_mcas_element_sizeof(K);
    size_t size_M_table = ((size_t)M << 20);
    
    int b = alder_encode_number2_bytesize(K);
    size_n_kmer = n_kmer * b;
    N_ni = size_n_kmer / ((size_t)D << 20) + 1;
    N_nh = ((size_t)size_M_table) / size_element_table;
    size_t N_nh_point7 = (size_t)((double)N_nh * ALDER_HASHTABLE_LOAD);
    N_np = (uint64_t)((double)n_kmer / (double)N_ni / (double)N_nh_point7);
    N_np = N_np == 0 ? 1 : N_np;
    if (N_np > (uint64_t)F) {
        N_np = (uint64_t)F;
        N_ni = (uint64_t)(((double)n_kmer / (double)N_np / (double)N_nh_point7));
        alder_loge(ALDER_ERR,
                   "maximum of number of open files is limited: "
                   "number of iterations is increased.");
    }
    
    /* Override ni, np, nh with user-provided values. */
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
    
    /* Inbuffer and outbuffer size */
    assert(*sizeInbuffer == (1<<16) && *sizeOutbuffer == (1<<20));
    
    /* Total file size per iteration. */
    uint64_t filesizePerIteration = (uint64_t)((double)size_n_kmer / (double)N_ni);
    size_t required_disk_space = (filesizePerIteration >> 20);
    
    /* PRINT SETUP */
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Kmer count setup ***");
    alder_log("%*s %d", width, "Version:", *version);
    alder_log("%*s %d", width, "Kmer size:", K);
    alder_log("%*s %ld (MB)", width, "Disk space:", D);
    alder_log("%*s %ld (MB)", width, "Memory space:", M);
    alder_log("%*s %d", width, "Number of threads:", n_nt);
    for (int i = 0; i < infile->qty; i++) {
        alder_log("%*s [%d] %s", width, "Input file:", i + 1, bdata(infile->entry[i]));
    }
    alder_log("%*s %zu (MB)", width, "binary file size:", *_S_filesize >> 20);
    alder_log("%*s %llu", width, "Number of Kmers:", n_kmer);
    alder_log("%*s %llu", width, "Number of iterations:", N_ni);
    alder_log("%*s %llu", width, "Number of partitions:", N_np);
    alder_log("%*s %zu", width, "Hash table size:", N_nh);
    alder_log("%*s %llu (MB)", width, "part file size:", filesizePerIteration >> 20);
    alder_log("%*s %s/%s.%s", width, "Count table file:",
              outdir, outfile, ALDER_KMER_TABLE_EXTENSION);
    
    unsigned long disk_space = 0;
    alder_file_availablediskspace(outdir, &disk_space);
    size_t available_diskspace = (size_t)((double)disk_space/ALDER_MB2BYTE);
    
    if (available_diskspace < required_disk_space) {
        alder_loge(1, "Available disk space: %zu (MB)", available_diskspace);
        alder_loge(1, "Required disk space: %zu (MB)", required_disk_space);
        alder_loge(1, "Need more disk space: %zu (MB)",
                   required_disk_space - available_diskspace);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu (MB)", width, "Spare space:",
              available_diskspace - required_disk_space);
    alder_log("%*s %zu (KB)", width, "Inbuffer size:", *sizeInbuffer >> 10);
    alder_log("%*s %zu (KB)", width, "Onbuffer size:", *sizeOutbuffer >> 10);
    
    *_N_ni = N_ni;
    *_N_np = N_np;
    *_N_nh = N_nh;
    write_config1(outfile, outdir, K, n_kmer, N_ni, N_np, filesizePerIteration);
    return ALDER_STATUS_SUCCESS;
}

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
alder_kmer_count_delete_partitionfile(long version,
                                      int i_ni,
                                      int n_np,
                                      int n_nt,
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

/**
 *  This function creates a binfile name in bstrList.
 *
 *  @param outfile outfile name
 *  @param outdir  output directory name
 *
 *  @return infile list
 */
static struct bstrList*
binfilename(long version, int n_nt, const char *outfile, const char *outdir)
{
    struct bstrList *binfile = NULL;
    bstring bfpar = NULL;
    bfpar = bformat("%s/%s.bin", outdir, outfile);
    if (bfpar == NULL) {
        return NULL;
    }
    binfile = bstrListCreate();
    if (binfile == NULL) {
        bdestroy(bfpar);
        return NULL;
    }
    binfile->entry[0] = bfpar;
    binfile->mlen = 1;
    binfile->qty = 1;
    return binfile;
}

/**
 *  This function estimates the buffer size.
 *
 *  @param sizeInbuffer_p  inbuf size
 *  @param sizeOutbuffer_p outbuf size
 *  @param outfile         outfile
 *  @param outdir          outdir
 */
void alder_kmer_estimate_buffer_size(long *sizeInbuffer_p, long *sizeOutbuffer_p,
                                     const char *outfile, const char *outdir)
{
    if (*sizeInbuffer_p == 0 || *sizeOutbuffer_p == 0) {
        *sizeInbuffer_p = 16;
        *sizeOutbuffer_p = 20;
    } else {
        alder_kmer_inspect_estimate_buffer_size((size_t*)&(*sizeInbuffer_p),
                                                (size_t*)&(*sizeOutbuffer_p),
                                                outdir,
                                                outfile);
    }
    *sizeInbuffer_p = 1LLU << *sizeInbuffer_p;
    *sizeOutbuffer_p = 1LLU << *sizeOutbuffer_p;
}

/**
 *  This function counts kmers.
 *
 *  @param version                  version 7
 *  @param K                        kmer size
 *  @param totalmaxkmer             total number of kmers. If this is 0, then
 *                                  the number is estimated from input files.
 *  @param D                        disk space in MB
 *  @param M                        memory size in MB
 *  @param lower_count              minimum number of occurences
 *  @param upper_count              maximum number of occurences
 *  @param F                        maximum number of open files
 *  @param sizeInbuffer             input buffer size in partitioning step
 *  @param sizeOutbuffer            output buffer size in partitioning step
 *  @param n_ni                     iterations. If -1, a value is computed
 *  @param n_np                     partitions. If -1, a value is computed
 *  @param n_nh                     hash table size per partition. If negative, 
 *                                  a value is computed.
 *  @param n_nt                     number of threads or CPU cores
 *  @param progress_flag            displays progress
 *  @param progressToErro_flag      displays progress to standard error
 *  @param only_init_flag           1 for neither encoding nor counting
 *  @param no_delete_partition_flag 1 for deleting partition files; default 0.
 *  @param no_partition_flag        1 for no partitioning step; default 0.
 *  @param no_count_flag            1 for no counting, but partitioning step.
 *                                  default 0.
 *  @param infile                   a list of input files
 *  @param outdir                   output directory
 *  @param outfile                  output file name prefix
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_count(long version,
                 int K, long totalmaxkmer, long D, long M,
                 int lower_count, int upper_count,
                 long F,
                 long sizeInbuffer, long sizeOutbuffer,
                 int n_ni, int n_np, int n_nh,
                 int n_nt,
                 int progress_flag, int progressToError_flag,
                 int only_init_flag,
                 int no_delete_partition_flag,
                 int no_partition_flag,
                 int no_count_flag,
                 struct bstrList *infile,
                 unsigned int outfile_given,
                 const char *outdir,
                 const char *outfile)
{
    assert(version == 7);
    int s = ALDER_STATUS_SUCCESS;
    /* variables for timing each step */
    time_t start;
    time_t end;
    double run_time_binary = 0;
    double run_time_encode = 0;
    double run_time_count = 0;
    /* for selecting a version of encoding or counting */
    encode_t encode;
    count_t count;
    uint64_t N_ni = 0;
    uint64_t N_np = 0;
    size_t N_nh = 0;
    size_t bin_filesize = 0;
    size_t size_data = 0;
    /* for stat */
    size_t n_byte = 0;
    uint64_t n_kmer = 0;
    size_t n_hash = 0;
    uint64_t n_dna = 0;
    uint64_t n_seq = 0;
    size_t n_kmer_encoded = 0;
    size_t n_byte_encoded = 0;
    size_t n_kmer_counted = 0;
    size_t n_total_kmer = 0;
    size_t n_current_kmer = 0;
    
    size_t S_filesize = 0;
    int flag_nh64 = 0;
    
    FILE *fpout = NULL;
    struct bstrList *cinfile = NULL;
    struct bstrList *binfile = NULL;
    uint8_t *inbuf = NULL;
    
    ///////////////////////////////////////////////////////////////////////////
    // Binary of sequence files
    ///////////////////////////////////////////////////////////////////////////
    time(&start);
    binfile = binfilename(version, n_nt, outfile, outdir);
    int force_version = version > 0 ? 1 : 0;
    alder_kmer_estimate_buffer_size(&sizeInbuffer, &sizeOutbuffer,
                                    outfile, outdir);
    
    /* Create a binary file and compute n_kmer, n_dna, n_seq, bin_filesize. */
    s = alder_kmer_binary3(NULL, 0, sizeInbuffer,
                           &n_kmer, &n_dna, &n_seq, &bin_filesize, &size_data,
                           version, K, D, M, 0, 0, n_nt,
                           progress_flag, progressToError_flag,
                           infile, outdir, outfile);
    if (s != ALDER_STATUS_SUCCESS) {
        bstrListDestroy(binfile);
        return s;
    }
    
    /* Compute N_ni, N_np, N_nh */
    s = alder_kmer_count_init6(force_version, &version,
                               &N_ni, &N_np, &N_nh,
                               &bin_filesize, &flag_nh64,
                               &sizeInbuffer, &sizeOutbuffer,
                               K, n_kmer, D,
                               M, 0, 0,
                               F,
                               n_ni, n_np, n_nh, n_nt,
                               progress_flag, progressToError_flag,
                               no_partition_flag, no_count_flag,
                               only_init_flag, infile, outdir,
                               outfile);
    if (s != ALDER_STATUS_SUCCESS) {
        bstrListDestroy(binfile);
        return s;
    }
    assert(N_ni > 0);
    time(&end);
    run_time_binary = difftime(end, start);
    
    ///////////////////////////////////////////////////////////////////////////
    // Encode and Count kmers.
    ///////////////////////////////////////////////////////////////////////////
    s = open_table_file(version, &fpout, outfile_given, outfile, outdir,
                        K, N_np, N_ni, N_nh, no_count_flag);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(ALDER_ERR_FILE, "Failed to open a table file: %s/%s.tbl",
                   outdir, outfile);
        if (version == 5) {
            bstrListDestroy(binfile);
        }
        return ALDER_STATUS_ERROR;
    }
    n_total_kmer = n_kmer * 2;
    n_current_kmer = 0;
    alder_progress_start(10);
    alder_log("*** Kmer count ***");
    for (int i = 0; i < N_ni; i++) {
        if (!no_partition_flag) {
            alder_progress_frequency(10);
            unsigned int binfile_given = 0;
            // Encode kmer: runs on nt-many threads.
            alder_log3("encoding iteration %d ...", i);
            encode = &alder_kmer_encode7;
            binfile_given = 1;
            cinfile = binfile;
            
            time(&start);
            s = (*encode)(n_nt, i, K, D, M, sizeInbuffer, sizeOutbuffer,
                          N_ni, N_np, S_filesize,
                          &n_byte_encoded, &n_kmer_encoded,
                          n_total_kmer, &n_current_kmer,
                          progress_flag, progressToError_flag,
                          binfile_given,
                          cinfile, outdir, outfile);
            if (s != ALDER_STATUS_SUCCESS) {
                alder_loge(ALDER_ERR, "Encoder (iter %d) has failed: %d", i);
                return ALDER_STATUS_ERROR;
            }
            time(&end);
            run_time_encode += difftime(end, start);
        }
        
        if (!no_count_flag) {
            // Count and save: runs on nt-many threads.
            alder_log3("counting iteration %d ...", i);
            count = &alder_kmer_thread7;
            cinfile = NULL;
            
            time(&start);
            alder_progress_frequency(1);
            s = (*count)(fpout, n_nt, i, K, M, sizeInbuffer, sizeOutbuffer,
                         N_ni, N_np, N_nh, lower_count, upper_count,
                         &n_byte, &n_kmer_counted, &n_hash,
                         n_total_kmer, &n_current_kmer,
                         progress_flag, progressToError_flag, only_init_flag,
                         inbuf, size_data, cinfile, outdir, outfile);
            if (s != ALDER_STATUS_SUCCESS) {
                alder_loge(ALDER_ERR, "Counter (iter %d) has failed: %i");
                return ALDER_STATUS_ERROR;
            }
            time(&end);
            run_time_count += difftime(end, start);
            
            if (!no_delete_partition_flag) {
                alder_log3("deleting partition files in iteration %d ...", i);
                alder_kmer_count_delete_partitionfile(version, i, (int)N_np, n_nt,
                                                      outdir, outfile);
            }
        }
    }
    assert(n_byte_encoded % N_ni == 0);
    alder_log2("Encoded Bytes: %zu", n_byte_encoded / N_ni);
    alder_log2("Encoded Kmers: %zu", n_kmer_encoded);
    alder_log2("Counted Kmers: %zu", n_kmer_counted);
    
    if (!no_count_flag) {
        s = alder_hashtable_mcas_printPackToFD_count(n_hash, fileno(fpout));
        assert(s == ALDER_STATUS_SUCCESS);
        XFCLOSE(fpout);
    }
    
    if ((!no_count_flag && !only_init_flag) || version == 6) {
        write_config2(outfile, outdir, n_hash);
    }
    
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    
    bstrListDestroy(binfile);
    if (version == 6) {
        XFREE(inbuf);
    }
    
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("%*s %.f (s) = %.f (m) = %.1f (h)", width, "Time for binary:",
              run_time_binary, run_time_binary/60, run_time_binary/3600);
    alder_log("%*s %.f (s) = %.f (m) = %.1f (h)", width, "Time for encoding:",
              run_time_encode, run_time_encode/60, run_time_encode/3600);
    alder_log("%*s %.f (s) = %.f (m) = %.1f (h)", width, "Time for counting:",
              run_time_count, run_time_count/60, run_time_count/3600);
    
    alder_log5("Counting Kmers has been finished!");
    return ALDER_STATUS_SUCCESS;
}

static int
write_table_header(long version,
                   FILE **fpout_p,
                   unsigned int outfile_given,
                   const char *outfile, const char *outdir, int K,
                   size_t N_nh, uint64_t size_value, uint64_t size_index,
                   int no_count_flag)
{
    ///////////////////////////////////////////////////////////////////////////
    // Encode and count kmers.
    int s;
    bstring bht = bformat("%s/%s.tbh", outdir, outfile);
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
        s = alder_hashtable_mcas2_printHeaderToFD(fileno(*fpout_p),
                                                  K,
                                                  (uint64_t)N_nh,
                                                  size_value, size_index);
                            
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s, ALDER_STATUS_ERROR);
    }
    bdestroy(bht);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function opens a file or uses STDOUT for a table.
 *
 *  @param fpout_p       file
 *  @param outfile_given 0 (STDOUT), 1 (file)
 *  @param outdir        output directory
 *  @param outfile       outfile
 *
 *  @return SUCCESS or FAIL
 */
static int
open_table(FILE **fpout_p,
           unsigned int outfile_given,
           const char *outdir, const char *outfile)
{
    if (outfile_given == 0) {
        *fpout_p = stdout;
    } else {
        bstring bht = bformat("%s/%s.tbl", outdir, outfile);
        if (bht == NULL) return ALDER_STATUS_ERROR;
        *fpout_p = fopen(bdata(bht), "wb+");
        if (*fpout_p == NULL) {
            bdestroy(bht);
            return ALDER_STATUS_ERROR;
        }
        bdestroy(bht);
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function determines size of value (byte), table element size (byte),
 *  the number of elements in a table, input and output buffer sizes.
 *
 *  @param _size_value   size of value in byte
 *  @param _size_index   size of index in byte
 *  @param _N_nh         number of elements in a table
 *  @param sizeInbuffer  buffer size for reading
 *  @param sizeOutbuffer buffer size for writing
 *  @param K             kmer size
 *  @param M             memory in megabyte
 *  @param lower_count   minimum value for count
 *  @param upper_count   maximum value for count
 *  @param n_nh          user specified number of elements in a table
 *  @param n_nt          number of threads
 *  @param infile        infile
 *  @param outdir        outdirectory
 *  @param outfile       outfile name
 *
 *  @return SUCCESS or FAIL
 */
static int
alder_kmer_topkmer_init(uint64_t *_size_value, uint64_t *_size_index,
                        size_t *N_nh, long *sizeInbuffer, long *sizeOutbuffer,
                        int K, long M, long lower_count, int upper_count,
                        int n_nh, int n_nt,
                        struct bstrList *infile, const char *outdir,
                        const char *outfile)
{
    /* Setup the size of buffers for reading and writing: sizeIn/Outbuffer.*/
    alder_kmer_estimate_buffer_size(sizeInbuffer, sizeOutbuffer,
                                    outfile, outdir);
    assert(*sizeInbuffer == (1<<16) && *sizeOutbuffer == (1<<20));
    
    /* Setup nh and element size in bytes. */
    size_t size_M_table = ((size_t)M << 20);
    size_t size_element_table =
    alder_hashtable_mcas2_element_sizeof_with_available_memory(K,
                                                               size_M_table);
    *N_nh = ((size_t)size_M_table) / size_element_table;
    /* Override ni, np, nh with user-provided values. */
    if (n_nh > 0) {
        *N_nh = (size_t)n_nh;
    }
    if (*N_nh > (size_t)UINT32_MAX) {
        alder_log("Hash table size is over UINT32: table index should be "
                  "of 64-bit value.");
    }
    
    /* Set index size: size_index. */
    size_t size_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE) * sizeof(uint64_t);
    size_t size_index = sizeof(uint32_t);
    if (*N_nh <=  UINT32_MAX) {
        size_index = sizeof(uint32_t);
    } else {
        size_index = sizeof(size_t);
    }
    /* Determine the byte-size of a count value. */
    size_t size_value = sizeof(uint8_t);
    if (upper_count <= UINT8_MAX) {
        size_value = sizeof(uint8_t);
    } else if (upper_count <= UINT16_MAX) {
        size_value = sizeof(uint16_t);
    } else if (upper_count <= UINT32_MAX) {
        size_value = sizeof(uint32_t);
    } else {
        assert(0);
    }
    
    /* PRINT SETUP */
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Kmer count setup ***");
    alder_log("%*s %d", width, "Kmer size:", K);
    alder_log("%*s %ld (MB)", width, "Memory space:", M);
    alder_log("%*s %d", width, "Number of threads:", n_nt);
    for (int i = 0; i < infile->qty; i++) {
        alder_log("%*s [%d] %s", width, "Input file:", i + 1, bdata(infile->entry[i]));
    }
    alder_log("%*s %zu", width, "Table size:", N_nh);
    alder_log("%*s %zu", width, "Table key size:", size_key);
    alder_log("%*s %zu", width, "Table value size:", size_value);
    alder_log("%*s %zu", width, "Table index size:", size_index);
    alder_log("%*s %s/%s.%s", width, "Count table file:",
              outdir, outfile, ALDER_KMER_TABLE_EXTENSION);
    
    unsigned long disk_space = 0;
    alder_file_availablediskspace(outdir, &disk_space);
    alder_log("%*s %zu (KB)", width, "Inbuffer size:", *sizeInbuffer >> 10);
    alder_log("%*s %zu (KB)", width, "Onbuffer size:", *sizeOutbuffer >> 10);
    
    *_size_value = size_value;
    *_size_index = size_index;
    write_config3(outfile, outdir, K, *N_nh, size_value, size_index, 0);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function executes topkmer command.
 *
 *  @param version                  version (starting at 8)
 *  @param K                        K
 *  @param totalmaxkmer             0 (no input file), + (input files)
 *  @param D                        [not used] disk space
 *  @param M                        memory (will be used for a table)
 *  @param lower_count              minimum count
 *  @param upper_count              maximum count
 *  @param F                        [not used] max number of files
 *  @param sizeInbuffer             read buffer size
 *  @param sizeOutbuffer            write buffer size
 *  @param n_ni                     [not used]
 *  @param n_np                     [not used]
 *  @param n_nh                     number of element in a table
 *  @param n_nt                     number of threads
 *  @param progress_flag            progress
 *  @param progressToError_flag     progress to stderr
 *  @param format_flag              fq, fa, or seq
 *  @param no_delete_partition_flag [not used]
 *  @param no_partition_flag        [not used]
 *  @param no_count_flag            [not used]
 *  @param infile                   input file names
 *  @param outfile_given            outfile given
 *  @param outdir                   outdir
 *  @param outfile                  outfile
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_topkmer(long version,
                   int K, long totalmaxkmer, long D, long M,
                   int lower_count, int upper_count,
                   long F,
                   long sizeInbuffer, long sizeOutbuffer,
                   int n_ni, int n_np, int n_nh,
                   int n_nt,
                   int progress_flag, int progressToError_flag,
                   int format_flag,
                   int no_delete_partition_flag,
                   int no_partition_flag,
                   int no_count_flag,
                   struct bstrList *infile,
                   unsigned int outfile_given,
                   const char *outdir,
                   const char *outfile)
{
    assert(version == 8);
    int s = ALDER_STATUS_SUCCESS;
    /* variables for timing each step */
    time_t start;
    time_t end;
    double run_time_count = 0;
    /* for selecting a version of encoding or counting */
    count_t count;
    uint64_t size_value = 0;
    uint64_t size_index = 0;
    size_t N_nh = 0;
    size_t size_data = 0;
    /* for stat */
    size_t n_byte = 0;
    size_t n_hash = 0;
    size_t n_kmer_counted = 0;
    size_t n_total_kmer = 0;
    size_t n_current_kmer = 0;
    FILE *fptbh = NULL;
    FILE *fptbl = NULL;
    
    /* Timeing: START */
    time(&start);
    /* Initialize the topkmer run. */
    s = alder_kmer_topkmer_init(&size_value, &size_index, &N_nh,
                                &sizeInbuffer, &sizeOutbuffer,
                                K, M, lower_count, upper_count, n_nh, n_nt,
                                infile, outdir, outfile);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(ALDER_ERR, "failed to initialize topkmer run.",
                   outdir, outfile);
        return s;
    }
    /* Write the table header file. */
    s = write_table_header(version, &fptbh, outfile_given, outfile, outdir,
                           K, N_nh, size_value, size_index, no_count_flag);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(ALDER_ERR_FILE, "Failed to write a table header: %s/%s.tbh",
                   outdir, outfile);
        return ALDER_STATUS_ERROR;
    }
    /* Open the table. */
    s = open_table(&fptbl, outfile_given, outdir, outfile);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(ALDER_ERR_FILE, "Failed to open a table: %s/%s.tbl",
                   outdir, outfile);
        return ALDER_STATUS_ERROR;
    }
    
    /* Run the topkmer. */
    alder_log("*** Kmer count ***");
    count = &alder_kmer_thread8;
    s = (*count)(fptbl, n_nt, 0, K, M, sizeInbuffer, sizeOutbuffer,
                 size_value, size_index, N_nh, lower_count, upper_count,
                 &n_byte, &n_kmer_counted, &n_hash,
                 n_total_kmer, &n_current_kmer,
                 progress_flag, progressToError_flag, format_flag,
                 NULL, size_data, infile, outdir, outfile);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(ALDER_ERR, "Counter (iter %d) has failed: %i");
        return ALDER_STATUS_ERROR;
    }
    alder_log2("Counted Kmers: %zu", n_kmer_counted);
    
    /* Finalize the topkmer run. */
    s = alder_hashtable_mcas2_printPackToFD_count(n_hash, fileno(fptbh));
    assert(s == ALDER_STATUS_SUCCESS);
    XFCLOSE(fptbh);
    if (outfile_given == 1) {
        XFCLOSE(fptbl);
    }
    write_config2(outfile, outdir, n_hash);
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    
    /* Timeing: END */
    time(&end);
    run_time_count += difftime(end, start);
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("%*s %.f (s) = %.f (m) = %.1f (h)", width, "Time for counting:",
              run_time_count, run_time_count/60, run_time_count/3600);
    
    alder_log5("Counting Kmers has been finished!");
    return ALDER_STATUS_SUCCESS;
}
