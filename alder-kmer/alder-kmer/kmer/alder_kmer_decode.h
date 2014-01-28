/**
 * This file, alder_kmer_decode.h, is part of alder-kmer.
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

#ifndef alder_kmer_alder_kmer_decode_h
#define alder_kmer_alder_kmer_decode_h

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
alder_kmer_decode2(int K,
                   int progress_flag,
                   struct bstrList *infile,
                   unsigned int outfile_given,
                   const char *outdir,
                   const char *outfile);

int
alder_kmer_decode(int K,
                  int progress_flag,
                  struct bstrList *infile,
                  const char *outdir,
                  const char *outfile);

__END_DECLS


#endif /* alder_kmer_alder_kmer_decode_h */
