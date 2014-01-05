/**
 * This file, alder_wordtable_kfminterval.c, is part of alder-wordtable.
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
#include "alder_wordtable_kfminterval.h"

struct alder_wordtable_kfminterval_struct {
    alder_wordtable_kfmindex_t *index;
    size_t start;
    size_t end;
    size_t length;
    size_t order;
};

static void
alder_wordtable_kfminterval_reset(alder_wordtable_kfminterval_t *o);
static bool
alder_wordtable_kfminterval_backtrackInterval(alder_wordtable_kfminterval_t *o,
                                              int token);
static bool
alder_wordtable_kfminterval_backtrackPosition(alder_wordtable_kfminterval_t *o,
                                              int token);



alder_wordtable_kfminterval_t*
alder_wordtable_kfminterval_create(alder_wordtable_kfmindex_t *index)
{
    ALDER_RETURN_NULL_IF_NULL(index);
    alder_wordtable_kfminterval_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->index = index;
    o->order = alder_wordtable_kfmindex_order(index);
    alder_wordtable_kfminterval_reset(o);
    
    return o;
}

void
alder_wordtable_kfminterval_destroy(alder_wordtable_kfminterval_t *o)
{
    if (o != NULL) {
        XFREE(o);
    }
}

void
alder_wordtable_kfminterval_reset(alder_wordtable_kfminterval_t *o)
{
    o->start = 0;
    o->end = alder_wordtable_kfmindex_size(o->index);
    o->length = 0;
}

/* This function finds a string in KFMindex.
 */
bool
alder_wordtable_kfminterval_find(alder_wordtable_kfminterval_t *o,
                                 int *tokens, size_t tokensLength)
{
    alder_wordtable_kfminterval_reset(o);
    bool result = true;
    for (int i = (int)tokensLength - 1; i >= 0; i--) {
        result = alder_wordtable_kfminterval_backtrack(o, tokens[i]);
        if (result == false) {
            break;
        }
    }
    return result;
}

/* Algorithms 2, 3, and 4 in Rodland13.
 *
 */
bool
alder_wordtable_kfminterval_backtrack(alder_wordtable_kfminterval_t *o,
                                      int token)
{
    if (token < 0) {
        o->length++;
        if (o->start==0) {
            if (o->end>0) {
                o->end=1; return true;
            } else {
                o->end=0; return false;
            }
        } else {
            o->start=1; o->end=1; return false;
        }
    } else if (o->length < o->order) {
        return alder_wordtable_kfminterval_backtrackInterval(o,token);
    } else {
        return alder_wordtable_kfminterval_backtrackPosition(o,token);
    }
}

/* Algorithm 3 in Rodland13.
 */
bool
alder_wordtable_kfminterval_backtrackInterval(alder_wordtable_kfminterval_t *o,
                                              int token)
{
    o->length++;
    if (o->start == o->end) {
        o->start = alder_wordtable_kfmindex_prevPos2(o->index, o->start, token);
        o->end = o->start;
        return false;
    }
    o->start = alder_wordtable_kfmindex_prevPos2(o->index, o->start, token);
    o->end = alder_wordtable_kfmindex_prevPos2(o->index, o->end, token);
    return o->start < o->end;
}

/* Algorithm 4 in Rodland13.
 */
bool
alder_wordtable_kfminterval_backtrackPosition(alder_wordtable_kfminterval_t *o,
                                              int token)
{
    o->length++;
    size_t next = alder_wordtable_kfmindex_prevPos2(o->index, o->start, token);
    if (o->start == o->end) {
        o->start = next;
        o->end = o->start;
        return false;
    }
    bool hasInEdge = alder_wordtable_kfmindex_hasInEdge(o->index,
                                                        o->start, token);
    if (hasInEdge) {
        o->start = next;
        o->end = next + 1;
        return true;
    } else {
        o->start = next;
        o->end = next;
        return false;
    }
}


