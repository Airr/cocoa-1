/**
 * This file, alder_wordtable_positionlist.c, is part of alder-wordtable.
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

/* This file contains functions to use a list of non-negative numbers.
 * I can add non-negative numbers, and get numbers using index that must be
 * less than the size of the list.  To iterate over the list, use the size
 * of the list. Numbers can be sorted in increasing order.
 *
 * e.g.,
 * alder_wordtable_positionlist_t *l = alder_wordtable_positionlist_create();
 * alder_wordtable_positionlist_add(l, 2);
 * size_t s = alder_wordtable_positionlist_size(l);
 * size_t v = alder_wordtable_positionlist_get(l, 0);
 * alder_wordtable_positionlist_sort(l);
 * alder_wordtable_positionlist_destroy(l);
 *
 * Note.
 * 1. These functions are used in pruning KFMindex.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_positionlist.h"

struct alder_wordtable_positionlist_struct {
    size_t *list;
    size_t size;   /* number of elements   */
    size_t length; /* capacity of the list */
};

static void
alder_wordtable_positionlist_resize(alder_wordtable_positionlist_t *o,
                                    size_t newsize);
static void
alder_wordtable_positionlist_ensureSpace(alder_wordtable_positionlist_t *o);

alder_wordtable_positionlist_t*
alder_wordtable_positionlist_create()
{
    alder_wordtable_positionlist_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->size = 0;
    o->length = ALDER_WORDTABLE_GROWABLEARRAY_INIT_SIZE;
    o->list = malloc(sizeof(*o->list) * o->length);
    if (o->list == NULL) {
        alder_wordtable_positionlist_destroy(o);
        return NULL;
    }
    return o;
}

void
alder_wordtable_positionlist_destroy(alder_wordtable_positionlist_t *o)
{
    if (o != NULL) {
        XFREE(o->list);
        XFREE(o);
    }
}

void
alder_wordtable_positionlist_add(alder_wordtable_positionlist_t *o,
                                 size_t value)
{
    alder_wordtable_positionlist_ensureSpace(o);
    o->list[o->size++] = value;
}

size_t
alder_wordtable_positionlist_get(alder_wordtable_positionlist_t *o,
                                 size_t position)
{
    if (position < o->length) {
        return o->list[position];
    } else {
        fprintf(stderr, "Error! - positionlist_get\n");
        abort();
    }
}

size_t
alder_wordtable_positionlist_size(alder_wordtable_positionlist_t *o)
{
    return o->size;
}

/* This function enlarges the size of the list.
 */
static void
alder_wordtable_positionlist_resize(alder_wordtable_positionlist_t *o,
                                    size_t newsize)
{
    if (newsize < o->length) {
        fprintf(stderr, "Invalid positionlist_resize!");
        abort();
    }
    size_t *t = realloc(o->list, sizeof(*t) * newsize);
    if (t == NULL) {
        fprintf(stderr, "Error - memory\n");
        abort();
    }
    o->list = t;
    o->length = newsize;
}

static int compar(const void *a, const void *b)
{
    const size_t av = *(size_t *)a;
    const size_t bv = *(size_t *)b;
    if (av > bv) {
        return 1;
    } else if (av < bv) {
        return -1;
    } else {
        return 0;
    }
}

void
alder_wordtable_positionlist_sort(alder_wordtable_positionlist_t *o)
{
    qsort(o->list, o->size, sizeof(*o->list), compar);
}

static void
alder_wordtable_positionlist_ensureSpace(alder_wordtable_positionlist_t *o)
{
    if (o->size == o->length) {
        size_t l = o->length * 2;
        if (l < o->length) {
            fprintf(stderr, "Error - Overflow!\n");
            abort();
        }
        alder_wordtable_positionlist_resize(o,l);
    }
}

