/**
 * This file, alder_fileseq.h, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_fileseq_alder_fileseq_h
#define alder_fileseq_alder_fileseq_h

#include <stdint.h>
#include "bstrlib.h"
#include "alder_fileseq_token.h"
#include "alder_fileseq_sample.h"
#include "alder_fileseq_sequenceiterator.h"
#include "alder_fileseq_write.h"

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

typedef struct alder_fileseq_stat_struct {
    uint64_t n; /* Number of sequences */
    uint64_t l; /* Number of bases */
    uint64_t v; /* Number of kmers */
} alder_fileseq_stat_t;

int alder_fileseq_stat(const char *fn, alder_fileseq_stat_t *buf, int k);
int alder_fileseq_count_kmer_files(uint64_t *v, size_t *vFilesize,
                                   struct bstrList *infile, int k,
                                   int progress_flag, int progressToError_flag);
int alder_fileseq_count_kmer_file(uint64_t *v,
                                  const char *fn, int k,
                                  size_t *curBufsize, size_t totalBufsize,
                                  int progress_flag,
                                  int progressToError_flag);

int
alder_fileseq_estimate_kmer_files(uint64_t *v, 
                                  struct bstrList *infile, int k,
                                  int progress_flag, int progressToError_flag);

int
alder_fileseq_estimate_kmer_file(uint64_t *v,
                                 const char *fn, int k,
                                 size_t *curBufsize, size_t totalBufsize,
                                 int progress_flag,
                                 int progressToError_flag);

__END_DECLS

#endif /* alder_fileseq_alder_fileseq_h */
