/**
 * This file, alder_wordtable_positionvaluelist.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_positionvaluelist_h
#define alder_wordtable_alder_wordtable_positionvaluelist_h

#include <stdint.h>
#include "alder_wordtable_kfmdefaults.h"

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

typedef struct alder_wordtable_positionvaluelist_struct
alder_wordtable_positionvaluelist_t;


alder_wordtable_positionvaluelist_t*
alder_wordtable_positionvaluelist_create();

void
alder_wordtable_positionvaluelist_destroy
(alder_wordtable_positionvaluelist_t *o);

void
alder_wordtable_positionvaluelist_add
(alder_wordtable_positionvaluelist_t *o, size_t pos, alder_dwordtable_t value);

size_t
alder_wordtable_positionvaluelist_getPosition
(alder_wordtable_positionvaluelist_t *o, size_t index);

alder_dwordtable_t
alder_wordtable_positionvaluelist_getValue
(alder_wordtable_positionvaluelist_t *o, size_t index);

size_t
alder_wordtable_positionvaluelist_size(alder_wordtable_positionvaluelist_t *o);


__END_DECLS


#endif /* alder_wordtable_alder_wordtable_positionvaluelist_h */
