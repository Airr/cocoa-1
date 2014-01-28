/**
 * This file, alder_kmer_thread.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_thread_h
#define alder_kmer_alder_kmer_thread_h

#include "alder_hashtable.h"
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

#pragma mark count

int alder_kmer_count_iteration(FILE *fpout,
                               int n_thread,
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
                               uint8_t *inbuf,
                               size_t size_data,
                               struct bstrList *infile,
                               const char *outdir,
                               const char *outfile);


int alder_kmer_count_iteration2(FILE *fpout,
                                int n_thread,
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
                                uint8_t *inbuf,
                                size_t size_data,
                                struct bstrList *infile,
                                const char *outdir,
                                const char *outfile);

int alder_kmer_count_iteration3(FILE *fpout,
                                int n_thread,
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
                                uint8_t *inbuf,
                                size_t size_data,
                                struct bstrList *infile,
                                const char *outdir,
                                const char *outfile);

int alder_kmer_count_iteration4(FILE *fpout,
                                int n_thread,
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
                                uint8_t *inbuf,
                                size_t size_data,
                                struct bstrList *infile,
                                const char *outdir,
                                const char *outfile);

int alder_kmer_count_iteration5(FILE *fpout,
                                int n_thread,
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
                                uint8_t *inbuf,
                                size_t size_data,
                                struct bstrList *infile,
                                const char *outdir,
                                const char *outfile);
#pragma mark common

size_t buffer_size_for_packed_kmer(int n_thread,
                                   int kmer_size,
                                   long memory_available,
                                   uint64_t np,
                                   size_t nh,
                                   size_t parfilesize);
__END_DECLS


#endif /* alder_kmer_alder_kmer_thread_h */
