/**
 * This file, alder_kseq_sequenceiterator.h, is part of alder-kseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-kseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kseq.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_kseq_alder_kseq_sequenceiterator_h
#define alder_kseq_alder_kseq_sequenceiterator_h

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

void
alder_kseq_sequenceiterator_test(struct bstrList *fn);


typedef struct alder_kseq_sequenceiterator_struct 
alder_kseq_sequenceiterator_t;

alder_kseq_sequenceiterator_t*
alder_kseq_sequenceiterator_create();

void
alder_kseq_sequenceiterator_destroy(alder_kseq_sequenceiterator_t *o);

void
alder_kseq_sequenceiterator_close(alder_kseq_sequenceiterator_t *o);

int
alder_kseq_sequenceiterator_open(alder_kseq_sequenceiterator_t *o,
                                  const char *fn);

int
alder_kseq_sequenceiterator_token(alder_kseq_sequenceiterator_t *o);

__END_DECLS


#endif /* alder_kseq_alder_kseq_sequenceiterator_h */
