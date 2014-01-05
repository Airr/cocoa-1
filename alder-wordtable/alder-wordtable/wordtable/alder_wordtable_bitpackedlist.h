/**
 * This file, alder_wordtable_bitpackedlist.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_bitpackedlist_h
#define alder_wordtable_alder_wordtable_bitpackedlist_h

#include <stdbool.h>
#include <stdint.h>
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_positionlist.h"

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

typedef struct alder_wordtable_bitpackedlist_struct
alder_wordtable_bitpackedlist_t;

void
alder_wordtable_bitpackedlist_destroy(alder_wordtable_bitpackedlist_t *o);

alder_wordtable_bitpackedlist_t*
alder_wordtable_bitpackedlist_create(size_t bits, size_t allocate);

alder_wordtable_bitpackedlist_t*
alder_wordtable_bitpackedlist_create1(size_t bits);

size_t
alder_wordtable_bitpackedlist_size(alder_wordtable_bitpackedlist_t *o);


alder_dwordtable_t
alder_wordtable_bitpackedlist_get(alder_wordtable_bitpackedlist_t *o,
                                  size_t pos);

void
alder_wordtable_bitpackedlist_set(alder_wordtable_bitpackedlist_t *o,
                                  size_t pos, alder_dwordtable_t value);

int
alder_wordtable_bitpackedlist_put(alder_wordtable_bitpackedlist_t *o,
                                  alder_dwordtable_t value);

int
alder_wordtable_bitpackedlist_setSize(alder_wordtable_bitpackedlist_t *o,
                                      size_t newsize);

void
alder_wordtable_bitpackedlist_remove(alder_wordtable_bitpackedlist_t *o,
                                     alder_wordtable_positionlist_t *list);

void
alder_wordtable_bitpackedlist_forgetPriorTo
(alder_wordtable_bitpackedlist_t *o, size_t pos);

size_t
alder_wordtable_bitpackedlist_save_size(alder_wordtable_bitpackedlist_t *o);

int
alder_wordtable_bitpackedlist_save(alder_wordtable_bitpackedlist_t *o,
                                   FILE *fp, uint32_t *crc);

alder_wordtable_bitpackedlist_t *
alder_wordtable_bitpackedlist_load(FILE *fp, uint32_t *crc);

__END_DECLS


#endif /* alder_wordtable_alder_wordtable_bitpackedlist_h */
