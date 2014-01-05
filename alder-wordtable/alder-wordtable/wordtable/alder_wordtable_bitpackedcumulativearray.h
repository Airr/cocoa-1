/**
 * This file, alder_wordtable_bitpackedcumulativearray.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_bitpackedcumulativearray_h
#define alder_wordtable_alder_wordtable_bitpackedcumulativearray_h

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
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

typedef struct alder_wordtable_bitpackedcumulativearray_struct
alder_wordtable_bitpackedcumulativearray_t;


alder_wordtable_bitpackedcumulativearray_t*
alder_wordtable_bitpackedcumulativearray_create(size_t size,
                                                size_t maxstep);

void
alder_wordtable_bitpackedcumulativearray_destroy
(alder_wordtable_bitpackedcumulativearray_t* o);

void
alder_wordtable_bitpackedcumulativearray_set
(alder_wordtable_bitpackedcumulativearray_t* o,
 size_t pos, alder_dwordtable_t value);

size_t
alder_wordtable_bitpackedcumulativearray_get
(alder_wordtable_bitpackedcumulativearray_t* o, size_t pos);

void
alder_wordtable_bitpackedcumulativearray_cumulate
(alder_wordtable_bitpackedcumulativearray_t* o);

size_t
alder_wordtable_bitpackedcumulativearray_size
(alder_wordtable_bitpackedcumulativearray_t* o);

int
alder_wordtable_bitpackedcumulativearray_save
(alder_wordtable_bitpackedcumulativearray_t* o, FILE *fp);

alder_wordtable_bitpackedcumulativearray_t*
alder_wordtable_bitpackedcumulativearray_load(FILE *fp);

__END_DECLS


#endif /* alder_wordtable_alder_wordtable_bitpackedcumulativearray_h */
