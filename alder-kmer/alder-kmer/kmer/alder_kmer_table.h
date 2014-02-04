/**
 * This file, alder_kmer_table.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_table_h
#define alder_kmer_alder_kmer_table_h

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

int
alder_kmer_table(long version,
                 int i_ni, int K, long M, long F,
                 long sizeInbuffer,
                 long sizeOutbuffer,
                 int n_ni, int n_np,
                 int n_nh,
                 int n_thread,
                 int progress_flag,
                 int progressToError_flag,
                 int nopack_flag,
                 const char *parfile,
                 unsigned int parfile_given,
                 struct bstrList *infile,
                 unsigned int outfile_given,
                 const char *outdir,
                 const char *outfile);


__END_DECLS


#endif /* alder_kmer_alder_kmer_table_h */
