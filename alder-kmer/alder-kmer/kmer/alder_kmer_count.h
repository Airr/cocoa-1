/**
 * This file, alder_kmer_count.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_count_h
#define alder_kmer_alder_kmer_count_h

#include <stdint.h>
#include "bstrlib.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

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
size_t *n_kmer,
size_t *n_hash,
size_t n_total_kmer,
size_t *n_current_kmer,
int progress_flag,
int progressToError_flag,
int nopack_flag,
uint8_t *inbuf,
size_t size_data,
struct bstrList *infile,
const char *outdir,
const char *outfile);

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
size_t *n_kmer,
size_t n_total_kmer,
size_t *n_current_kmer,
int progress_flag,
int progressToError_flag,
unsigned int binfile_given,
struct bstrList *infile,
const char *outdir,
const char *outfile);

int
alder_kmer_count(long version,
                 int K, long totalmaxkmer, long D, long M,
                 long min_M_table, long max_M_table,
                 long F,
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
                 struct bstrList *infile,
                 unsigned int outfile_given,
                 const char *outdir,
                 const char *outfile);

void alder_kmer_estimate_buffer_size(long *sizeInbuffer_p, long *sizeOutbuffer_p,
                                     const char *outfile, const char *outdir);

__END_DECLS


#endif /* alder_kmer_alder_kmer_count_h */
