/**
 * This file, alder_wordtable_inedgelist.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_inedgelist_h
#define alder_wordtable_alder_wordtable_inedgelist_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_wordtable_bits.h"
#include "alder_wordtable_intblocklist.h"
#include "alder_wordtable_tokenset.h"
#include "alder_wordtable_tokeniterator.h"


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

typedef struct alder_wordtable_inedgelist_struct
alder_wordtable_inedgelist_t;


alder_wordtable_inedgelist_t*
alder_wordtable_inedgelist_getNew(size_t order,
                                  size_t maxsize);

alder_wordtable_inedgelist_t*
alder_wordtable_inedgelist_create(size_t order,
                                  size_t maxsize,
                                  size_t tokenbits,
                                  size_t endmarkbits,
                                  size_t databits);

void
alder_wordtable_inedgelist_destroy(alder_wordtable_inedgelist_t *o);

size_t
alder_wordtable_inedgelist_size(alder_wordtable_inedgelist_t *o);

void
alder_wordtable_inedgelist_setIncludeReverse(alder_wordtable_inedgelist_t *o,
                                             bool flag);

bool
alder_wordtable_inedgelist_isIncludeReverse(alder_wordtable_inedgelist_t *o);

size_t
alder_wordtable_inedgelist_order(alder_wordtable_inedgelist_t *o);

size_t
alder_wordtable_inedgelist_getAddedSequenceCount(alder_wordtable_inedgelist_t *o);

bool
alder_wordtable_inedgelist_updateFinalToken(alder_wordtable_inedgelist_t *o);

bool
alder_wordtable_inedgelist_addToken(alder_wordtable_inedgelist_t *o,
                                    int token);

void
alder_wordtable_inedgelist_addBsequence(alder_wordtable_inedgelist_t *o,
                                        bstring bseq);

void
alder_wordtable_inedgelist_add(alder_wordtable_inedgelist_t *o,
                               alder_wordtable_tokeniterator_t *tokens);
void
alder_wordtable_inedgelist_addSingle(alder_wordtable_inedgelist_t *o,
                                     alder_wordtable_tokeniterator_t *tokens);


void
alder_wordtable_inedgelist_addDual(alder_wordtable_inedgelist_t *o,
                                   alder_wordtable_tokeniterator_t *tokens);


bstring
alder_wordtable_inedgelist_tokensAsString(alder_wordtable_inedgelist_t *o,
                                          size_t pos);

bstring
alder_wordtable_inedgelist_edgeAsBitString(alder_wordtable_inedgelist_t *o,
                                           size_t pos);

alder_wordtable_t
alder_wordtable_inedgelist_edgeToken(alder_wordtable_inedgelist_t *o,
                                     size_t position);
char
alder_wordtable_inedgelist_edgeTokenAsChar(alder_wordtable_inedgelist_t *o,
                                           size_t position);


void
alder_wordtable_inedgelist_complete(alder_wordtable_inedgelist_t *o);

size_t
alder_wordtable_inedgelist_nTokensEqual(alder_wordtable_inedgelist_t *o,
                                        size_t p, size_t q);

void
alder_wordtable_inedgelist_empty(alder_wordtable_inedgelist_t *o);

void
alder_wordtable_inedgelist_compact(alder_wordtable_inedgelist_t *o);

void
alder_wordtable_inedgelist_sort(alder_wordtable_inedgelist_t *o);

bstring
alder_wordtable_inedgelist_asString(alder_wordtable_inedgelist_t *o,
                                    alder_wordtable_t *arr,
                                    size_t arrLength);
bstring
alder_wordtable_inedgelist_tempAsString(alder_wordtable_inedgelist_t *o);

bstring
alder_wordtable_inedgelist_posAsString(alder_wordtable_inedgelist_t *o,
                                       size_t pos);

void
alder_wordtable_inedgelist_print(alder_wordtable_inedgelist_t *o);

__END_DECLS


#endif /* alder_wordtable_alder_wordtable_inedgelist_h */
