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
#include "alder_kmer_binary.h"
#include "alder_kmer_inspect.h"
#include "alder_kmer_thread.h"

typedef int (*encode_t)(int n_encoder,
int i_iteration,
int kmer_size,
long disk_space,
long memory_available,
long sizeInbuffer,
long sizeOutbuffer,
uint64_t n_iteration,
uint64_t n_partition,
size_t totalfilesize,
size_t *n_byte,
int progress_flag,
int progressToError_flag,
struct bstrList *infile,
const char *outdir,
const char *outfile);

typedef int (*count_t)(FILE *fpout,
int n_counter,
int i_ni,
int kmer_size,
long memory_available,
long sizeInbuffer,
long sizeOutbuffer,
uint64_t n_ni,
uint64_t n_np,
size_t n_nh,
size_t totalfilesize,
size_t *n_byte,
size_t *n_hash,
int progress_flag,
int progressToError_flag,
int nopack_flag,
struct bstrList *infile,
const char *outdir,
const char *outfile);

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

/**
 *  This computes ni, np, nh, and file size using
 *  K: kmer size
 *  D: disk space (MB)
 *  M: memory (MB)
 *  F: max number of files
 *  n_nt: number of threads
 *  infile: input file names
 *
 *  If ni, np, nh are given, I'd use them instead.
 *
 *  ni: All of the Kmers are needed to be stored in files before counting them.
 *      Total bits for storing them are 2vK because each letter needs two bits
 *      and vK is the total number of letters.
 *  np: Total bits for all of Kmers
 *      ------------------------
 *      Available memory in bits
 *
 *  It also determines which version of counting kmer is used.
 *  1. Estimate memory requirement first.
 *
 *  [return parameters]
 *  @param version             version of counting is determined
 *  @param _N_ni               iteration
 *  @param _N_np               partition
 *  @param _N_nh               hash table size
 *  @param _S_filesize         input file size
 *  @param flag_nh64           hash table index bit (32-bit or 64-bit)
 *  @param sizeInbuffer        reading buffer size
 *  @param sizeOutbuffer       writing buffer size
 *
 *  [input parameters]
 *  @param K                   kmer size
 *  @param totalmaxkmer        total number of kmers in the input data
 *  @param D                   disk space (MB)
 *  @param M                   memory (MB)
 *  @param F                   max number of open files
 *  @param n_ni                user-provided ni
 *  @param n_np                user-provided np
 *  @param n_nh                user-provided nh
 *  @param n_nt                number of threads
 *  @param progress_flag       progress
 *  @param progressToErro_flag progress to stderr
 *  @param no_partition_flag   no partition flag
 *  @param no_count_flag       no count flag
 *  @param nopack_flag         no pack flag [OBSOLETE]
 *  @param infile              infile
 *  @param outdir              outdir
 *  @param outfile             outfile name
 *
 *  @return SUCCESS or FAIL
 */
static int
alder_kmer_count_init(long *version,
                      uint64_t *_N_ni, uint64_t *_N_np, size_t *_N_nh,
                      size_t *_S_filesize, int *flag_nh64,
                      long *sizeInbuffer, long *sizeOutbuffer,
                      int K, long totalmaxkmer, long D, long M, long F,
                      int n_ni, int n_np, int n_nh, int n_nt,
                      int progress_flag, int progressToErro_flag,
                      int no_partition_flag, int no_count_flag,
                      int nopack_flag,
                      struct bstrList *infile, const char *outdir,
                      const char *outfile)
{
    long version_arg = *version;
    if (*version > 0) {
        version_arg = 0;
    }
    
    uint64_t n_v = 0;
    uint64_t v = 0;
    uint64_t N_ni = 0;
    uint64_t N_np = 0;
    size_t N_nh = 0;
    int s = ALDER_STATUS_SUCCESS;
    alder_log3("Start counting Kmers ...");
    
    /* Estimate the total number of K-mers. */
    if (totalmaxkmer == 0) {
        s = alder_kmer_inspect_estimate_nkmer(&v, K, infile);
        alder_log("Maximum number of kmers is %llu.", v);
    } else {
        v = (uint64_t)totalmaxkmer;
    }
    
    /* Setup ni, np, and nh, and version. */
    size_t size_total_kmer = 0;
    if (*version == 1) {
        size_total_kmer = (v * 2 * K) >> 3;
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
        
    } else if (version_arg == 0) {
        /* Test of version 6 */
        s = alder_kmer_inspect_estimate_nkmer(&n_v, 1, infile);
        size_t size_binary_data = n_v / 4;
        size_t size_table = (1LLU << 32);
        size_t size_total_data = size_binary_data + size_table;
        size_t size_M = M << 20;
        /* Test of version 4 or 5 */
        int b = alder_encode_number2_bytesize(K);
        size_total_kmer = v * b;
        N_ni = size_total_kmer / (D << 20) + 1;
        size_t size_element_table = alder_hashtable_mcas_element_sizeof(K);
        if (n_nt > 1) {
            N_nh = (((size_t)M/2) << 20) / size_element_table;
        } else if (n_nt == 1) {
            N_nh = (((size_t)M) << 20) / size_element_table;
        }
        size_t N_nh_point7 = (size_t)((double)N_nh * 7.0 / 10.0);
        N_np = (uint64_t)((double)v / (double)N_ni / (double)N_nh_point7);
        N_np = N_np == 0 ? 1 : N_np;
        if (N_np > (uint64_t)F) {
            N_np = (uint64_t)F;
            N_ni = (uint64_t)(((double)v / (double)N_np / (double)N_nh_point7));
            alder_loge(ALDER_ERR,
                       "maximum of number of open files is limited: "
                       "number of iterations is increased.");
        }
        
        if (*version == 0) {
            if (size_total_data < size_M) {
                *version = 6;
            } else if (N_ni > 1) {
                *version = 5;
            } else {
                *version = 4;
            }
        }
        if (*version == 6) {
            if (size_total_data >= size_M) {
                alder_loge(ALDER_ERR_MEMORY, "Not enough memory for version 6");
//                return ALDER_STATUS_ERROR;
            }
            N_ni = 1;
        }
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
    if (*sizeInbuffer == 0 || *sizeOutbuffer == 0) {
        //
        *sizeInbuffer = 16;
        *sizeOutbuffer = 20;
    } else {
        alder_kmer_inspect_estimate_buffer_size((size_t*)sizeInbuffer,
                                                (size_t*)sizeOutbuffer,
                                                outdir,
                                                outfile);
    }
    *sizeInbuffer = 1LLU << *sizeInbuffer;
    *sizeOutbuffer = 1LLU << *sizeOutbuffer;
    
    ///////////////////////////////////////////////////////////////////////////
    // To display the progress
    // Version 6. reading input files, and processing the binary file of them.
    //            They could be done in two ways.
    //            1. total input file size
    //            2. total data buffer x np
    // Version 5. reading input files to create the binary file of them, and
    //            processing the binary file x ni times
    //            1. total input file size
    //            2. binary file x ni
    // Version 4. reading input files, and processing partition files x np times
    //            1. total input file size
    //            2. [np] partition files
    // NOTE. I would use the input file size only at this stage.
    ///////////////////////////////////////////////////////////////////////////
    
    /* Compute total file sizes for processing. */
    alder_totalfile_size(infile, _S_filesize);
    
    //    if (!no_partition_flag && !no_count_flag) {
    //        *_S_filesize = (*_S_filesize) * N_ni + size_total_kmer;
    //    } else if (!no_partition_flag && no_count_flag) {
    //        *_S_filesize = (*_S_filesize) * N_ni;
    //    } else if (no_partition_flag && !no_count_flag) {
    //        *_S_filesize = size_total_kmer;
    //    }
    
    /* Total file size per iteration. */
    uint64_t filesizePerIteration = (uint64_t)((double)size_total_kmer / (double)N_ni);
    //    size_t total_hashfile_size = M * N_np * N_ni;
    size_t required_disk_space = (filesizePerIteration >> 20);
    
    /* PRINT SETUP */
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Kmer counting setup ***");
    alder_log("%*s %d", width, "Version:", *version);
    alder_log("%*s %d", width, "Kmer size:", K);
    alder_log("%*s %ld (MB)", width, "Disk space:", D);
    alder_log("%*s %ld (MB)", width, "Memory space:", M);
    alder_log("%*s %d", width, "Number of threads:", n_nt);
    for (int i = 0; i < infile->qty; i++) {
        alder_log("%*s [%d] %s", width, "Input file:", i + 1, bdata(infile->entry[i]));
    }
    alder_log("%*s %zu (B)", width, "Total file size:", *_S_filesize);
    alder_log("%*s %zu (KB)", width, "Inbuffer size:", *sizeInbuffer << 10);
    alder_log("%*s %zu (KB)", width, "Onbuffer size:", *sizeOutbuffer << 10);
    alder_log("%*s %llu", width, "Estimated num. Kmers:", v);
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
    
    *_N_ni = N_ni;
    *_N_np = N_np;
    *_N_nh = N_nh;
    write_config1(outfile, outdir, K, v, N_ni, N_np, filesizePerIteration);
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

static int
open_table_file(long version,
                FILE **fpout_p, const char *outfile, const char *outdir, int K,
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
        if (version < 4) {
            s = alder_hashtable_mcas_printHeaderToFILE(*fpout_p,
                                                       K,
                                                       (uint64_t)N_nh,
                                                       N_ni, N_np);
        } else {
            s = alder_hashtable_mcas_printHeaderToFD(fileno(*fpout_p),
                                                     K,
                                                     (uint64_t)N_nh,
                                                     N_ni, N_np);
        }
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s, ALDER_STATUS_ERROR);
    }
    bdestroy(bht);
    return ALDER_STATUS_SUCCESS;
}

static int
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

/**
 *  This function creates a binfile name in bstrList.
 *
 *  @param outfile outfile name
 *  @param outdir  output directory name
 *
 *  @return infile list
 */
static struct bstrList*
binfilename(const char *outfile, const char *outdir)
{
    struct bstrList *binfile = NULL;
    bstring bfpar = bformat("%s/%s.bin", outdir, outfile);
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
    encode_t encode;
    count_t count;
    
    int s = ALDER_STATUS_SUCCESS;
    uint64_t N_ni = 0;
    uint64_t N_np = 0;
    size_t N_nh = 0;
    size_t S_filesize = 0;
    int flag_nh64 = 0;
    FILE *fpout = NULL;
    size_t n_hash = 0;
    size_t n_byte = 0;
    s = alder_kmer_count_init(&version, &N_ni, &N_np, &N_nh, &S_filesize,
                              &flag_nh64, &sizeInbuffer, &sizeOutbuffer,
                              K, totalmaxkmer, D, M, F,
                              n_ni, n_np,
                              n_nh,
                              n_nt,
                              progress_flag, progressToError_flag,
                              no_partition_flag, no_count_flag,
                              nopack_flag,
                              infile, outdir,
                              outfile);
    assert(version == 4 || version == 5 || version == 6);
    struct bstrList *binfile = NULL;
    if (version == 5) {
        uint64_t n_kmer = 0;
        uint64_t n_dna = 0;
        uint64_t n_seq = 0;
        s = alder_kmer_binary(NULL, 0, 0,
                              &n_kmer, &n_dna, &n_seq,
                              version,
                              K,
                              D,
                              n_nt,
                              S_filesize,
                              &n_byte,
                              progress_flag,
                              progressToError_flag,
                              infile,
                              outdir,
                              outfile);
        if (s != ALDER_STATUS_SUCCESS) {
            alder_loge(ALDER_ERR, "Failed to start counting.");
            return ALDER_STATUS_ERROR;
        }
        binfile = binfilename(outfile, outdir);
        alder_loge(ALDER_ERR_FILE, "Decide ni, and np.");
        assert(0);
    } else if (version == 6) {
        assert(N_ni == 1);
        no_partition_flag = 1;
        no_count_flag = 0;
        no_delete_partition_flag = 1;
    }
    
    s = open_table_file(version, &fpout, outfile, outdir,
                        K, N_np, N_ni, N_nh, no_count_flag);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(ALDER_ERR_FILE, "Failed to open a table file: %s/%s.tbl",
                   outdir, outfile);
        if (version == 5) {
            bstrListDestroy(binfile);
        }
        return ALDER_STATUS_ERROR;
    }
    alder_log("*** Kmer count ***");
    
    // DSK approach.
    for (int i = 0; i < N_ni; i++) {
        if (!no_partition_flag) {
            assert(version != 6);
            // Encode kmer: runs on nt-many threads.
            alder_log3("encoding iteration %d ...", i);
            if (version == 1) {
                encode = &alder_kmer_encode;
            } else if (version == 2) {
                encode = &alder_kmer_encode2;
            } else if (version == 3) {
                encode = &alder_kmer_encode3;
            } else if (version == 4) {
                encode = &alder_kmer_encode4;
            } else if (version == 5) {
                encode = &alder_kmer_encode5;
                infile = binfile;
            } else {
                alder_loge(ALDER_ERR, "Bug: report it!");
                return ALDER_STATUS_ERROR;
            }
            (*encode)(n_nt, i, K, D, M,
                      sizeInbuffer, sizeOutbuffer,
                      N_ni, N_np,
                      S_filesize,
                      &n_byte,
                      progress_flag,
                      progressToError_flag,
                      infile, outdir,
                      outfile);
        }
        
        if (!no_count_flag) {
            // Count and save: runs on nt-many threads.
            alder_log3("counting iteration %d ...", i);
            size_t i_n_hash = 0;
            
            if (version == 1) {
                count = &alder_kmer_count_iteration;
            } else if (version == 2) {
                count = &alder_kmer_count_iteration2;
            } else if (version == 3) {
                count = &alder_kmer_count_iteration3;
            } else if (version == 4 || version == 5) {
                count = &alder_kmer_count_iteration4;
            } else {
                assert(version == 6);
                count = &alder_kmer_count_iteration5;
            }
            (*count)(fpout,
                     n_nt,
                     i,
                     K,
                     M,
                     sizeInbuffer,
                     sizeOutbuffer,
                     N_ni,
                     N_np,
                     N_nh,
                     S_filesize,
                     &n_byte,
                     &i_n_hash,
                     progress_flag,
                     progressToError_flag,
                     nopack_flag,
                     infile,
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
    if (!no_count_flag) {
        s = alder_hashtable_mcas_printPackToFD_count(n_hash, fileno(fpout));
        assert(s == ALDER_STATUS_SUCCESS);
        XFCLOSE(fpout);
    }
    
    if ((!no_count_flag && !nopack_flag) || version == 6) {
        write_config2(outfile, outdir, n_hash);
    }
    
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    
    if (version == 5) {
        bstrListDestroy(binfile);
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
    count_t count;
    uint64_t N_ni = 1;
    uint64_t N_np = infile->qty;
    size_t N_nh = n_nh;
    size_t S_filesize;
    size_t n_hash = 0;
    size_t n_byte = 0;
    alder_kmer_count_withPartition_init(&S_filesize, K, N_nh, n_thread,
                                        infile, outdir, outfile);
    
    FILE *fpout = NULL;
    s = open_table_file(version, &fpout, outfile, outdir,
                        K, N_np, N_ni, N_nh, 0);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(1, "Failed to open a table file.");
        return ALDER_STATUS_ERROR;
    }
    alder_log("*** Kmer count using partition files ***");
    // Count and save: runs on nt-many threads.
    if (version == 1) {
        count = &alder_kmer_count_iteration;
    } else if (version == 2) {
        count = &alder_kmer_count_iteration2;
    } else if (version == 3) {
        count = &alder_kmer_count_iteration3;
    } else if (version == 4 || version == 5) {
        count = &alder_kmer_count_iteration4;
    } else {
        fprintf(stderr, "No alder_kmer_count_iteration4 for version > 5\n");
        count = NULL;
        assert(0);
        abort();
    }
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
             &n_hash,
             progress_flag,
             progressToError_flag,
             nopack_flag,
             infile,
             outdir,
             outfile);
    
    
    s = alder_hashtable_mcas_printPackToFILE_count(n_hash, fpout);
    
    XFCLOSE(fpout);
    
    if (progress_flag) {
        alder_progress_end(progressToError_flag);
    }
    
    alder_log5("Counting Kmers has been finished!");
    return s;
}

