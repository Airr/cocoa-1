/**
 * This file, alder_kmer_binary.c, is part of alder-kmer.
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
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include "bzlib.h"
#include "alder_logger.h"
#include "alder_file.h"
#include "alder_cmacro.h"
#include "libdivide.h"
#include "bstrmore.h"
#include "alder_fasta.h"
#include "alder_fastq.h"
#include "alder_fileseq_type.h"
#include "alder_fileseq_chunk.h"
#include "bstrmore.h"
#include "alder_encode.h"
#include "alder_encode_number.h"
#include "alder_fileseq_streamer.h"
#include "alder_progress.h"
#include "alder_dna.h"

#include "alder_kmer_binary.h"

typedef int (*binary_t)(
void *ptr, size_t size, size_t subsize,
uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
size_t *totalfilesize,
size_t *n_byte,
long version,
int K, long D, long M, long min_M_table, long max_M_table,
long nsplit,
int progress_flag,
int progressToError_flag,
struct bstrList *infile, const char *outdir,
const char *outfile);

/**
 *  This function converts a set of sequence data to a binary file before
 *  starting the procedure of counting Kmers. Data can be loaded on the memory
 *  or can be created in a file.
 *
 *  @param ptr                  memory for the binary data
 *  @param size                 size of the memory, ptr
 *  @param subsize              block size of the binary data
 *  @param n_kmer               [return] total number of kmer in the input
 *  @param n_dna                [return] total number of valid DNA characters
 *  @param n_seq                [return] total number of sequences
 *  @param totalfilesize        [return] size of a file if the file is created
 *  @param n_byte               [return] size of data if ptr is filled
 *  @param version              ?
 *  @param K                    kmer size
 *  @param D                    disk space
 *  @param M                    memory available
 *  @param min_M_table          min memory for a table
 *  @param max_M_table          max memory for a table
 *  @param nsplit               number of splits of a binary file
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to stderr
 *  @param infile               infile
 *  @param outdir               outdir
 *  @param outfile              outfile name
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_binary(void *ptr, size_t size, size_t subsize,
                  uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                  size_t *totalfilesize,
                  size_t *n_byte,
                  long version,
                  int K, long D, long M, long min_M_table, long max_M_table,
                  long nsplit,
                  int progress_flag,
                  int progressToError_flag,
                  struct bstrList *infile, const char *outdir,
                  const char *outfile)
{
    binary_t binary;
    
    if (version == 2) {
        binary = &alder_kmer_binary3;
    } else if (version == 3) {
        binary = &alder_kmer_binary3;
    } else {
        binary = &alder_kmer_binary5;
    }
    int s = (*binary)(ptr, size, subsize,
                      n_kmer, n_dna, n_seq,
                      totalfilesize,
                      n_byte,
                      version,
                      K, D, M, min_M_table, max_M_table,
                      nsplit,
                      progress_flag,
                      progressToError_flag,
                      infile, outdir, outfile);
    return s;
}
