/**
 * This file, alder_wordtable_kfmaggregator.h, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_wordtable_alder_wordtable_kfmaggregator_h
#define alder_wordtable_alder_wordtable_kfmaggregator_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_wordtable_sequenceiterator.h"
#include "alder_wordtable_inedgelist.h"
#include "alder_wordtable_tokenset.h"
#include "alder_wordtable_stackkfmindex.h"

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

typedef struct alder_wordtable_kfmaggregator_struct
alder_wordtable_kfmaggregator_t;
    
void
alder_wordtable_kfmaggregator_destroy(alder_wordtable_kfmaggregator_t *o);

alder_wordtable_kfmaggregator_t*
alder_wordtable_kfmaggregator_create(size_t order,
                                     size_t maxEdgelist,
                                     size_t maxEdgestack);

alder_wordtable_kfmaggregator_t*
alder_wordtable_kfmaggregator_create1(size_t order);

size_t
alder_wordtable_kfmaggregator_addAll(alder_wordtable_kfmaggregator_t *o,
                                     alder_wordtable_sequenceiterator_t *seqs);


alder_wordtable_kfmindex_t*
alder_wordtable_kfmaggregator_getKFMindex
(alder_wordtable_kfmaggregator_t *o);

void
alder_wordtable_kfmaggregator_setIncludeReverse(alder_wordtable_kfmaggregator_t *o,
                                                bool flag);

void
alder_wordtable_kfmaggregator_setStepsize(alder_wordtable_kfmaggregator_t *o,
                                          size_t stepsize);


size_t
alder_wordtable_kfmaggregator_getStepsize(alder_wordtable_kfmaggregator_t *o);


size_t
alder_wordtable_kfmaggregator_getBlocksize(alder_wordtable_kfmaggregator_t *o);




__END_DECLS

#endif /* alder_wordtable_alder_wordtable_kfmaggregator_h */
