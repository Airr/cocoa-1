/**
 * This file, alder_kmer_inspect.c, is part of alder-kmer.
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
#include "bstrlib.h"
#include "alder_logger.h"
#include "alder_file_io.h"
#include "alder_file_isfastq.h"
#include "alder_file_isfasta.h"
#include "alder_file_isgzip.h"
#include "alder_file_isbzip2.h"
#include "alder_file_islz4.h"
#include "alder_fileseq.h"
#include "alder_fastq_estimate_nkmer.h"
#include "alder_fasta_estimate_nkmer.h"
#include "alder_kmer_inspect.h"

#pragma mark nkmer

/**
 *  This function estimates the total number of K-mers in a file.
 *
 *  @param v  estimated number of K-mers
 *  @param K  kmer size
 *  @param fn file name
 *
 *  @return SUCCESS with v set to the number, otehrwise FAIL with v of 0
 */
int alder_kmer_inspect_estimate_nkmer_file(uint64_t *v,
                                           int K,
                                           const char *fn)
{
    int s = ALDER_STATUS_SUCCESS;
    *v = 0;
    int isfasta = alder_file_isfasta(fn);
    int isfastq = alder_file_isfastq(fn);
    
    if (isfasta) {
        s = alder_fasta_estimate_nkmer(v, K, fn);
        if (s != ALDER_STATUS_SUCCESS) {
            *v = 0;
            return s;
        }
    } else if (isfastq) {
        s = alder_fastq_estimate_nkmer(v, K, fn);
        if (s != ALDER_STATUS_SUCCESS) {
            *v = 0;
            return s;
        }
    } else {
        // Error.
        alder_loge(ALDER_ERR_FILE,
                   "input file is not either fasta or fastq: %s", fn);
        *v = 0;
        return ALDER_STATUS_ERROR;
    }
    
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function estimates the total number of K-mers
 *
 *  @param v       estimated number of K-mers
 *  @param K       kmer size
 *  @param infile  input file names
 *
 *  @return SUCCESS with v set to the number, otehrwise FAIL with v of 0
 */
int alder_kmer_inspect_estimate_nkmer(uint64_t *v,
                                      int K,
                                      struct bstrList *infile)
{
    /* Maximum number of Kmers and total file size. */
    *v = 0;
    for (int i = 0; i < infile->qty; i++) {
        uint64_t n = 0;
        int s = alder_kmer_inspect_estimate_nkmer_file(&n, K,
                                                       bdata(infile->entry[i]));
        if (s != ALDER_STATUS_SUCCESS) {
            *v = 0;
            return s;
        }
        *v += n;
    }
    return ALDER_STATUS_SUCCESS;
    
}

/**
 *  This function estimates the best buffer size for reading from and writing 
 *  to files.
 *
 *  @param outdir  output directory
 *  @param outfile outfile name prefix
 *
 *  @return SUCCESS or FAIL
 */
int alder_kmer_inspect_estimate_buffer_size(size_t *read_log2size,
                                            size_t *write_log2size,
                                            const char *outdir,
                                            const char *outfile)
{
    int s = ALDER_STATUS_SUCCESS;
    bstring bfn = bformat("%s/%s.ins", outdir, outfile);
    
    s = alder_file_io_best_buffer_size(bdata(bfn),
                                       write_log2size,
                                       read_log2size);
    if (s != ALDER_STATUS_SUCCESS) {
        return s;
    }
    if (*write_log2size < *read_log2size) {
        *read_log2size = *write_log2size;
    }
    bdestroy(bfn);
    return ALDER_STATUS_SUCCESS;
}

#pragma mark main

/**
 *  This function inspects the computer system to optimize alder-kmer's runs
 *  on it.
 *
 *  @param infile  input files
 *  @param outdir  output directory
 *  @param outfile output file prefix
 */
void alder_kmer_inspect(struct bstrList *infile,
                        const char *outdir,
                        const char *outfile)
{
    // Read files test.
    bstring bfn = bformat("%s/%s.ins", outdir, outfile);
    
    size_t write_log2size = 0;
    size_t read_log2size = 0;
    alder_file_io_best_buffer_size(bdata(bfn),
                                   &write_log2size,
                                   &read_log2size);
    
    if (write_log2size < read_log2size) {
        read_log2size = write_log2size;
    }
    printf("Buffer size in log2: %zu (read), %zu (write)\n",
           read_log2size, write_log2size);
    
    bdestroy(bfn);
}

