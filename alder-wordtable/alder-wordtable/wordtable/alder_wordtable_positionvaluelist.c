/**
 * This file, alder_wordtable_positionvaluelist.c, is part of alder-wordtable.
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

/* This file contains functions to use a list of pairs of non-negative numbers.
 * Each pair has a position and a value. I can add pairs, and get pairs 
 * using index that must be less than the size of the list.  To iterate over 
 * the list, use the size of the list.
 *
 * e.g.,
 * alder_wordtable_positionvaluelist_t *l =
 * alder_wordtable_positionvaluelist_create();
 *
 * alder_wordtable_positionvaluelist_add(l, 2, 1);
 * size_t s = alder_wordtable_positionvaluelist_size(l);
 * size_t vp = alder_wordtable_positionvaluelist_getPosition(l, 0);
 * alder_dwordtable_t vv = alder_wordtable_positionvaluelist_getValue(l, 0);
 * alder_wordtable_positionvaluelist_destroy(l);
 *
 * Note.
 * 1. These functions are used in pruning KFMindex.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_positionvaluelist.h"

struct alder_wordtable_positionvaluelist_struct {
    size_t *position;
    alder_dwordtable_t *value;
    size_t size;               /* number of elements   */
    size_t length;             /* capacity of the list */
};

static int
alder_wordtable_positionvaluelist_resize
(alder_wordtable_positionvaluelist_t *o, size_t newsize);

static int
alder_wordtable_positionvaluelist_ensureSpace
(alder_wordtable_positionvaluelist_t *o);

alder_wordtable_positionvaluelist_t*
alder_wordtable_positionvaluelist_create()
{
    alder_wordtable_positionvaluelist_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    /* init */
    o->size = 0;
    o->length = ALDER_WORDTABLE_GROWABLEARRAY_INIT_SIZE;
    o->position = NULL;
    o->value = NULL;
    
    /* memory */
    o->position = malloc(sizeof(*o->position) * o->length);
    o->value = malloc(sizeof(*o->value) * o->length);
    if (o->position == NULL || o->value == NULL) {
        alder_wordtable_positionvaluelist_destroy(o);
        return NULL;
    }
    return o;
}

void
alder_wordtable_positionvaluelist_destroy(alder_wordtable_positionvaluelist_t *o)
{
    if (o != NULL) {
        XFREE(o->position);
        XFREE(o->value);
        XFREE(o);
    }
}

void
alder_wordtable_positionvaluelist_add
(alder_wordtable_positionvaluelist_t *o,
 size_t pos, alder_dwordtable_t val)
{
    alder_wordtable_positionvaluelist_ensureSpace(o);
    o->position[o->size] = pos;
    o->value[o->size] = val;
    o->size++;
}

size_t
alder_wordtable_positionvaluelist_getPosition
(alder_wordtable_positionvaluelist_t *o, size_t index)
{
    return o->position[index];
}

alder_dwordtable_t
alder_wordtable_positionvaluelist_getValue
(alder_wordtable_positionvaluelist_t *o, size_t index)
{
    return o->value[index];
}

size_t
alder_wordtable_positionvaluelist_size(alder_wordtable_positionvaluelist_t *o)
{
    return o->size;
}

static int
alder_wordtable_positionvaluelist_resize
(alder_wordtable_positionvaluelist_t *o, size_t newsize)
{
    int status = ALDER_WORDTABLE_SUCCESS;
    if (newsize < o->length) {
        return ALDER_WORDTABLE_FAIL;
    }
    size_t *tp = realloc(o->position, sizeof(*tp) * newsize);
    if (tp == NULL) {
        return ALDER_WORDTABLE_FAIL;
    }
    o->position = tp;
    alder_dwordtable_t *tv = realloc(o->value, sizeof(*tv) * newsize);
    if (tv == NULL) {
        return ALDER_WORDTABLE_FAIL;
    }
    o->value = tv;
    o->length = newsize;
    return status;
}

static int
alder_wordtable_positionvaluelist_ensureSpace
(alder_wordtable_positionvaluelist_t *o)
{
    int status = ALDER_WORDTABLE_SUCCESS;
    if (o->size == o->length) {
        size_t l = o->length * 2;
        if (l < o->length) {
            return ALDER_WORDTABLE_FAIL;
        }
        alder_wordtable_positionvaluelist_resize(o,l);
    }
    return status;
}

