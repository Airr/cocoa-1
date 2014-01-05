/**
 * This file, alder_wordtable_kfmindex.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_kfmindex_h
#define alder_wordtable_alder_wordtable_kfmindex_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_wordtable_inedgelist.h"
#include "alder_wordtable_positionlist.h"
#include "alder_wordtable_positionvaluelist.h"

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

typedef struct alder_wordtable_kfmindex_struct
alder_wordtable_kfmindex_t;

typedef alder_wordtable_kfmindex_t* alder_wordtable_kfmindex_pt;

alder_wordtable_kfmindex_t*
alder_wordtable_kfmindex_create(size_t order, size_t initsize);


alder_wordtable_kfmindex_t*
alder_wordtable_kfmindex_create2(alder_wordtable_inedgelist_t *list,
                                 size_t stepsize);

alder_wordtable_kfmindex_t*
alder_wordtable_kfmindex_create1(alder_wordtable_inedgelist_t *list);


void
alder_wordtable_kfmindex_destroy(alder_wordtable_kfmindex_t *o);


void
alder_wordtable_kfmindex_setKFMstepsize(alder_wordtable_kfmindex_t *o,
                                        size_t stepsize);

bool
alder_wordtable_kfmindex_isPruned(alder_wordtable_kfmindex_t *o);

int
alder_wordtable_kfmindex_computeKFMarray0(alder_wordtable_kfmindex_t *o);

int
alder_wordtable_kfmindex_computeKFMarray(alder_wordtable_kfmindex_t *o,
                                         size_t indexsize);






size_t
alder_wordtable_kfmindex_prevPos(alder_wordtable_kfmindex_t *o,
                                 size_t tokenpos);

size_t
alder_wordtable_kfmindex_prevPos2(alder_wordtable_kfmindex_t *o,
                                 size_t tokenpos, int token);

size_t
alder_wordtable_kfmindex_kfmSolve(alder_wordtable_kfmindex_t *o,
                                  size_t posvalue);

#pragma mark find

size_t
alder_wordtable_kfmindex_findLstring(alder_wordtable_kfmindex_t *o,
                                     const char *s);

size_t
alder_wordtable_kfmindex_find_alpha(alder_wordtable_kfmindex_t *o,
                                    const char *s);
size_t
alder_wordtable_kfmindex_find_beta(alder_wordtable_kfmindex_t *o,
                                   const char *s);

#pragma mark print

bstring
alder_wordtable_kfmindex_toString(alder_wordtable_kfmindex_t *o,
                                  size_t pos);

bstring
alder_wordtable_kfmindex_nodeString(alder_wordtable_kfmindex_t *o,
                                    size_t pos);

void
alder_wordtable_kfmindex_print(alder_wordtable_kfmindex_t *o);

void
alder_wordtable_kfmindex_print5bit(alder_wordtable_kfmindex_t *o);

#pragma mark save and load

int
alder_wordtable_kfmindex_save(alder_wordtable_kfmindex_t *o,
                              const char *fn);

alder_wordtable_kfmindex_t *
alder_wordtable_kfmindex_load(const char *fn);

size_t
alder_wordtable_kfmindex_pruneFinalCompletions(alder_wordtable_kfmindex_t *o);

#pragma mark UnusedFinals

typedef struct alder_wordtable_unusedfinals_struct {
    size_t tokensize;
    alder_wordtable_positionlist_t *unusedNodes;
    alder_wordtable_positionvaluelist_t *changedValues;
} alder_wordtable_unusedfinals_t;

alder_wordtable_unusedfinals_t*
alder_wordtable_unusedfinals_create(size_t tokensize);
void
alder_wordtable_unusedfinals_destroy(alder_wordtable_unusedfinals_t *o);

void
alder_wordtable_unusedfinals_removeUnneededEdges
(alder_wordtable_kfmindex_t *o,
 alder_wordtable_unusedfinals_t *o1);

bool
alder_wordtable_unusedfinals_checkIfUnused
(alder_wordtable_kfmindex_t *o,
 alder_wordtable_unusedfinals_t *o1, size_t level, size_t pos);

alder_wordtable_positionlist_t*
alder_wordtable_unusedfinals_getUnusedNodes(alder_wordtable_unusedfinals_t *o);

#pragma mark Others

void
alder_wordtable_kfmindex_remove(alder_wordtable_kfmindex_t *o,
                                alder_wordtable_positionlist_t *list);

size_t
alder_wordtable_kfmindex_size(alder_wordtable_kfmindex_t *o);

size_t
alder_wordtable_kfmindex_order(alder_wordtable_kfmindex_t *o);

size_t
alder_wordtable_kfmindex_tokenset_size(alder_wordtable_kfmindex_t *o);

alder_dwordtable_t
alder_wordtable_kfmindex_get(alder_wordtable_kfmindex_t *o, size_t pos);

bool
alder_wordtable_kfmindex_valueHasInEdge(alder_wordtable_kfmindex_t *o,
                                        alder_dwordtable_t value, int token);

alder_dwordtable_t
alder_wordtable_kfmindex_getValue(alder_wordtable_kfmindex_t *o,
                                  size_t pos, bool ignoreGroupFlag);
void
alder_wordtable_kfmindex_put(alder_wordtable_kfmindex_t *o,
                             alder_dwordtable_t value);

void
alder_wordtable_kfmindex_forgetPriorTo(alder_wordtable_kfmindex_t *o,
                                       size_t pos);


bool
alder_wordtable_kfmindex_hasInEdge(alder_wordtable_kfmindex_t *o,
                                   size_t pos, int token);


__END_DECLS


#endif /* alder_wordtable_alder_wordtable_kfmindex_h */
