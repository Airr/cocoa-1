/**
 * This file, alder_kmer_encode.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_encode_h
#define alder_kmer_alder_kmer_encode_h

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

int
alder_kmer_encode7(int n_thread,
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

__END_DECLS


#endif /* alder_kmer_alder_kmer_encode_h */
