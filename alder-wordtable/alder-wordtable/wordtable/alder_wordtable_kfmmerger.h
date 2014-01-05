/**
 * This file, alder_wordtable_kfmmerger.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_kfmmerger_h
#define alder_wordtable_alder_wordtable_kfmmerger_h

#include <stdint.h>
#include "alder_wordtable_kfmindex.h"

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

typedef struct alder_wordtable_kfmmerger_struct
alder_wordtable_kfmmerger_t;


void
alder_wordtable_kfmmerger_destroy(alder_wordtable_kfmmerger_t *o);

alder_wordtable_kfmindex_t*
alder_wordtable_kfmmerger_mergeTwoKFMindex(alder_wordtable_kfmindex_t *a,
                                           alder_wordtable_kfmindex_t *b,
                                           bool deleteSources,
                                           size_t stepsize);

alder_wordtable_kfmmerger_t*
alder_wordtable_kfmmerger_create(alder_wordtable_kfmindex_t *a,
                                 alder_wordtable_kfmindex_t *b,
                                 bool deleteSources);

alder_wordtable_kfmindex_t*
alder_wordtable_kfmmerger_merge(alder_wordtable_kfmmerger_t* o,
                                size_t stepsize);

void
alder_wordtable_kfmmerger_prepareMerge(alder_wordtable_kfmmerger_t* o,
                                       size_t level,
                                       size_t aStart, size_t aEnd,
                                       size_t bStart, size_t bEnd);

void
alder_wordtable_kfmmerger_prepareInsertA(alder_wordtable_kfmmerger_t* o,
                                         size_t level,
                                         size_t aPos, size_t bPos);

void
alder_wordtable_kfmmerger_prepareMergeSingles(alder_wordtable_kfmmerger_t* o,
                                              size_t level,
                                              size_t aPos, size_t bPos);

void
alder_wordtable_kfmmerger_performMerge(alder_wordtable_kfmmerger_t* o);


__END_DECLS


#endif /* alder_wordtable_alder_wordtable_kfmmerger_h */
