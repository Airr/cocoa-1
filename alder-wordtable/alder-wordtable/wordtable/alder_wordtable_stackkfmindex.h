/**
 * This file, alder_wordtable_stackkfmindex.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_stackkfmindex_h
#define alder_wordtable_alder_wordtable_stackkfmindex_h

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

typedef struct alder_wordtable_stackkfmindex_struct
alder_wordtable_stackkfmindex_t;
    
alder_wordtable_stackkfmindex_t*
alder_wordtable_stackkfmindex_create(size_t size);

void
alder_wordtable_stackkfmindex_destroy(alder_wordtable_stackkfmindex_t *st);

size_t
alder_wordtable_stackkfmindex_size(alder_wordtable_stackkfmindex_t *st);

int
alder_wordtable_stackkfmindex_empty (alder_wordtable_stackkfmindex_t * st);

void
alder_wordtable_stackkfmindex_push (alder_wordtable_kfmindex_pt x,
                                    alder_wordtable_stackkfmindex_t *st);

alder_wordtable_kfmindex_pt
alder_wordtable_stackkfmindex_pop(alder_wordtable_stackkfmindex_t *st);

alder_wordtable_kfmindex_pt
alder_wordtable_stackkfmindex_top(alder_wordtable_stackkfmindex_t *st);


__END_DECLS


#endif /* alder_wordtable_alder_wordtable_stackkfmindex_h */
