/**
 * This file, alder_fastq.h, is part of alder-fastq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fastq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fastq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_fastq_alder_fastq_h
#define alder_fastq_alder_fastq_h

#include "alder_fastq_kseq.h"
#include "alder_fastq_kseq_summary.h"
#include "alder_fastq_ragel_summary.h"

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

typedef struct {
    int type; // 0: regular file, 1: gzipped file
    void *f;
    kseq_t *seq;
} alder_fastq_t;

kseq_t * alder_fastq_init (const char *fn);

alder_fastq_t * alder_fastq_open(const char *fn);
kseq_t * alder_fastq_read(alder_fastq_t *f);
void alder_fastq_close(alder_fastq_t *f);

__END_DECLS


#endif /* alder_fastq_alder_fastq_h */
