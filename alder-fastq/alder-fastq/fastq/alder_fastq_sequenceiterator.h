/**
 * This file, alder_fastq_sequenceiterator.h, is part of alder-fastq.
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

#ifndef alder_fastq_alder_fastq_sequenceiterator_h
#define alder_fastq_alder_fastq_sequenceiterator_h

#include <stdint.h>
#include "bstrmore.h"

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

void
alder_fastq_sequenceiterator_test(struct bstrList *fn);


typedef struct alder_fastq_sequenceiterator_struct 
alder_fastq_sequenceiterator_t;

alder_fastq_sequenceiterator_t*
alder_fastq_sequenceiterator_create();

void
alder_fastq_sequenceiterator_destroy(alder_fastq_sequenceiterator_t *o);

void
alder_fastq_sequenceiterator_close(alder_fastq_sequenceiterator_t *o);

int
alder_fastq_sequenceiterator_open(alder_fastq_sequenceiterator_t *o,
                                  const char *fn);

int
alder_fastq_sequenceiterator_token(alder_fastq_sequenceiterator_t *o);

__END_DECLS


#endif /* alder_fastq_alder_fastq_sequenceiterator_h */
