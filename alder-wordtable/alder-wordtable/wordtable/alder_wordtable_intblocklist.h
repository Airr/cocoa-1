/**
 * This file, alder_wordtable_intblocklist.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_intblocklist_h
#define alder_wordtable_alder_wordtable_intblocklist_h

#include <stdint.h>
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_bits.h"

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

typedef struct alder_wordtable_intblocklist_struct
alder_wordtable_intblocklist_t;
    
alder_wordtable_intblocklist_t*
alder_wordtable_intblocklist_create(size_t blocksize);
void alder_wordtable_intblocklist_destroy(alder_wordtable_intblocklist_t *o);

size_t alder_wordtable_intblocklist_size(alder_wordtable_intblocklist_t *o);

void
alder_wordtable_intblocklist_getBlock(alder_wordtable_intblocklist_t *o,
                                      alder_wordtable_t *dest,
                                      size_t pos);

void
alder_wordtable_intblocklist_setBlock(alder_wordtable_intblocklist_t *o,
                                      alder_wordtable_t *src,
                                      size_t pos);



size_t
alder_wordtable_intblocklist_addBlankBlock(alder_wordtable_intblocklist_t *o);

void
alder_wordtable_intblocklist_copy(alder_wordtable_intblocklist_t *o,
                                  size_t p,
                                  size_t q);

alder_wordtable_t
alder_wordtable_intblocklist_getIntAt(alder_wordtable_intblocklist_t *o,
                                      size_t pos, size_t subpos);
void
alder_wordtable_intblocklist_setIntAt(alder_wordtable_intblocklist_t *o,
                                      size_t pos, size_t subpos,
                                      alder_wordtable_t value);

void
alder_wordtable_intblocklist_orIntAt(alder_wordtable_intblocklist_t *o,
                                     size_t pos, size_t subpos,
                                     alder_wordtable_t value);
void
alder_wordtable_intblocklist_andIntAt(alder_wordtable_intblocklist_t *o,
                                      size_t pos, size_t subpos,
                                      alder_wordtable_t value);

void
alder_wordtable_intblocklist_compact(alder_wordtable_intblocklist_t *o);

int
alder_wordtable_intblocklist_nBitsEqual(alder_wordtable_intblocklist_t *o,
                                        size_t p,
                                        size_t q);


int
alder_wordtable_intblocklist_shiftLeft(alder_wordtable_intblocklist_t *o,
                                       size_t pos,
                                       int leftshift,int insert);
int
alder_wordtable_intblocklist_copyShiftLeftTo(alder_wordtable_intblocklist_t *o,
                                             alder_wordtable_t *src,
                                             size_t pos,
                                             int leftshift,int insert);

void
alder_wordtable_intblocklist_remove(alder_wordtable_intblocklist_t *o,
                                    size_t start, size_t end);

void
alder_wordtable_intblocklist_empty(alder_wordtable_intblocklist_t *o);

void
alder_wordtable_intblocklist_sort(alder_wordtable_intblocklist_t *o);

void
alder_wordtable_intblocklist_sort2(alder_wordtable_intblocklist_t *o,
                                   size_t start, size_t end);

size_t
alder_wordtable_intblocklist_blocksize(alder_wordtable_intblocklist_t *o);

__END_DECLS


#endif /* alder_wordtable_alder_wordtable_intblocklist_h */
