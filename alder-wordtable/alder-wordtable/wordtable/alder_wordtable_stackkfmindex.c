/**
 * This file, alder_wordtable_stackkfmindex.c, is part of alder-wordtable.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_wordtable_kfmindex.h"
#include "alder_wordtable_stackkfmindex.h"

struct alder_wordtable_stackkfmindex_struct {
    alder_wordtable_kfmindex_pt *base;
    alder_wordtable_kfmindex_pt *top;
    size_t size;
    size_t maxEdgestack;
    struct alder_wordtable_stackkfmindex_struct *previous;
};

alder_wordtable_stackkfmindex_t*
alder_wordtable_stackkfmindex_create(size_t maxEdgestack)
{
    alder_wordtable_stackkfmindex_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->maxEdgestack = maxEdgestack;
    o->size = 10;
    o->previous = NULL;
    
    o->base = malloc (sizeof(*o->base) * o->size);
    if (o->base == NULL) {
        alder_wordtable_stackkfmindex_destroy(o);
        return NULL;
    }
    o->top = o->base;
    return o;
}

void
alder_wordtable_stackkfmindex_destroy(alder_wordtable_stackkfmindex_t *st)
{
    alder_wordtable_stackkfmindex_t *tmp;
    do {
        tmp = st->previous;
        XFREE(st->base);
        XFREE(st);
        st = tmp;
    } while (st != NULL);
}

size_t
alder_wordtable_stackkfmindex_size(alder_wordtable_stackkfmindex_t *st)
{
    size_t size = st->top - st->base;
    alder_wordtable_stackkfmindex_t *tmp = st->previous;
    while (tmp != NULL) {
        size += (tmp->top - tmp->base);
        tmp = tmp->previous;
    }
    return size;
}

int
alder_wordtable_stackkfmindex_empty (alder_wordtable_stackkfmindex_t * st)
{
    return (st->base == st->top && st->previous == NULL);
}

void
alder_wordtable_stackkfmindex_push (alder_wordtable_kfmindex_pt x,
                                    alder_wordtable_stackkfmindex_t *st)
{
    if (st->top < st->base + st->size) {
        *(st->top) = x;
        st->top += 1;
    } else {
        alder_wordtable_stackkfmindex_t *m;
        m = malloc(sizeof(*m));
        m->base = st->base;
        m->top = st->top;
        m->size = st->size;
        m->previous = st->previous;
        st->previous = m;
        st->base = malloc(sizeof(*st->base) * st->size);
        st->top = st->base + 1;
        *(st->base) = x;
    }
}

alder_wordtable_kfmindex_pt
alder_wordtable_stackkfmindex_pop(alder_wordtable_stackkfmindex_t *st)
{
    if (st->top == st->base) {
        alder_wordtable_stackkfmindex_t *old;
        old = st->previous;
        st->previous = old->previous;
        XFREE(st->base);
        st->base = old->base;
        st->top = old->top;
        st->size = old->size;
        XFREE(old);
    }
    st->top -= 1;
    return (*(st->top));
}

alder_wordtable_kfmindex_pt
alder_wordtable_stackkfmindex_top(alder_wordtable_stackkfmindex_t *st)
{
    if (st->top == st->base)
        return (*(st->previous->top - 1));
    else
        return (*(st->top - 1));
}



