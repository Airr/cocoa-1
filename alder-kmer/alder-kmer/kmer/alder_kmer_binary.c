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
#include "alder_logger.h"
#include "alder_file.h"
#include "alder_cmacro.h"
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
 *  encoding Kmers.
 *
 *  @param ptr                  [not used] memory for the binary data
 *  @param size                 [not used] size of the memory, ptr
 *  @param subsize              block size of the binary data (inbuf size)
 *  @param n_kmer               [return] total number of kmer in the input
 *  @param n_dna                [return] total number of valid DNA characters
 *  @param n_seq                [return] total number of sequences
 *  @param totalfilesize        [return] size of a file if the file is created
 *  @param n_byte               [return] size of data if ptr is filled
 *  @param version              version 7
 *  @param K                    kmer size (not used in FASTQ but in FASTA)
 *  @param D                    [not used] disk space
 *  @param M                    [not used] memory available
 *  @param min_M_table          [not used] min memory for a table
 *  @param max_M_table          [not used] max memory for a table
 *  @param nsplit               [not used] number of splits of a binary file
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
    binary = &alder_kmer_binary3;
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
