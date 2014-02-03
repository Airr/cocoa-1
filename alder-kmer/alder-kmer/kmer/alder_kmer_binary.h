/**
 * This file, alder_kmer_binary.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_binary_h
#define alder_kmer_alder_kmer_binary_h

#include <pthread.h>
#include <stdint.h>

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

#define ALDER_KMER_BINARY_READBLOCK_I_NP       0
#define ALDER_KMER_BINARY_READBLOCK_TYPE       4
#define ALDER_KMER_BINARY_READBLOCK_LEN        8
#define ALDER_KMER_BINARY_READBLOCK_BODY       16

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
                  const char *outfile);

int
alder_kmer_binary3(void *ptr, size_t size, size_t subsize,
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


__END_DECLS


#endif /* alder_kmer_alder_kmer_binary_h */
