/**
 * This file, alder_wordtable_inedgedata.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_inedgedata_h
#define alder_wordtable_alder_wordtable_inedgedata_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_wordtable_bitpackedlist.h"
#include "alder_wordtable_inedgelist.h"

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

typedef struct alder_wordtable_inedgedata_struct
alder_wordtable_inedgedata_t;


void
alder_wordtable_inedgedata_destroy(alder_wordtable_inedgedata_t *o);

alder_wordtable_inedgedata_t*
alder_wordtable_inedgedata_create(size_t order, size_t initsize);

alder_wordtable_inedgedata_t*
alder_wordtable_inedgedata_createWithInEdgeList
(alder_wordtable_inedgelist_t *list);

size_t
alder_wordtable_inedgedata_order(alder_wordtable_inedgedata_t *o);
void
alder_wordtable_inedgedata_addEdges(alder_wordtable_inedgedata_t *o,
                                    alder_wordtable_inedgelist_t *list);

alder_dwordtable_t
alder_wordtable_inedgedata_getValue(alder_wordtable_inedgedata_t *o,
                                    size_t pos, bool ignoreGroupFlag);
bool
alder_wordtable_inedgedata_valueIsGroupEnd(alder_wordtable_inedgedata_t *o,
                                           alder_dwordtable_t value);

bool
alder_wordtable_inedgedata_valueHasInEdge(alder_wordtable_inedgedata_t *o,
                                          alder_dwordtable_t value, int token);

bool
alder_wordtable_inedgedata_hasInEdge(alder_wordtable_inedgedata_t *o,
                                     size_t pos, int token);
void
alder_wordtable_inedgedata_remove(alder_wordtable_inedgedata_t *o,
                                  alder_wordtable_positionlist_t *list);
void
alder_wordtable_inedgedata_forgetPriorTo(alder_wordtable_inedgedata_t *o,
                                         size_t pos);

bstring
alder_wordtable_inedgedata_nodeDataString(alder_wordtable_inedgedata_t *o,
                                          size_t pos);

size_t
alder_wordtable_inedgedata_size(alder_wordtable_inedgedata_t *o);

alder_dwordtable_t
alder_wordtable_inedgedata_get(alder_wordtable_inedgedata_t *o,
                               size_t pos);

int
alder_wordtable_inedgedata_put(alder_wordtable_inedgedata_t *o,
                               alder_dwordtable_t value);

void
alder_wordtable_inedgedata_set(alder_wordtable_inedgedata_t *o,
                               size_t pos, alder_dwordtable_t value);

alder_dwordtable_t
alder_wordtable_inedgedata_inedgebitmask(alder_wordtable_inedgedata_t *o);


int
alder_wordtable_inedgedata_save(alder_wordtable_inedgedata_t *o,
                                FILE *fp);

alder_wordtable_inedgedata_t *
alder_wordtable_inedgedata_load(FILE *fp, size_t order);

__END_DECLS


#endif /* alder_wordtable_alder_wordtable_inedgedata_h */
