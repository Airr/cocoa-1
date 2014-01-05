/**
 * This file, alder_wordtable_tokeniterator.c, is part of alder-wordtable.
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
#include "alder_wordtable_tokenset.h"
#include "alder_fileseq_token.h"
#include "alder_wordtable_tokeniterator.h"

alder_wordtable_tokeniterator_t*
alder_wordtable_tokeniterator_create(const char *fn)
{
    alder_wordtable_tokeniterator_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->isBuffered = false;
    o->buffer = NULL;
    o->bufferLength = 0;
    o->remaining = 0;
    o->pos = 0;
    o->fileseq = NULL;

    o->fileseq = alder_fileseq_token_create(fn);
    if (o->fileseq == NULL) {
        alder_wordtable_tokeniterator_destroy(o);
        return NULL;
    }
    
    /* memory */
    return o;
}

alder_wordtable_tokeniterator_t*
alder_wordtable_tokeniterator_createWithBstring(bstring bseq)
{
    alder_wordtable_tokeniterator_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->isBuffered = false;
    o->fileseq = NULL;
    o->buffer = NULL;
    
    o->fileseq = alder_fileseq_token_createWithBstring(bseq);
    if (o->fileseq == NULL) {
        alder_wordtable_tokeniterator_destroy(o);
        return NULL;
    }
    return o;
}

void
alder_wordtable_tokeniterator_setWithBstring(alder_wordtable_tokeniterator_t *o,
                                             bstring bseq)
{
    alder_fileseq_token_setWithBstring(o->fileseq, bseq);
}


void
alder_wordtable_tokeniterator_destroy(alder_wordtable_tokeniterator_t *o)
{
    if (o != NULL) {
        alder_fileseq_token_destroy(o->fileseq);
        XFREE(o->buffer);
        XFREE(o);
    }
}

bool
alder_wordtable_tokeniterator_hasNext(alder_wordtable_tokeniterator_t *o)
{
    if (o->isBuffered) {
        if (o->remaining == 0) {
            o->pos = o->bufferLength;
            return false;
        } else if (o->pos > 0) {
            return true;
        }
    }
    return alder_fileseq_token_hasNext(o->fileseq);
}

int
alder_wordtable_tokeniterator_next(alder_wordtable_tokeniterator_t *o)
{
    int c;
    if (o->isBuffered) {
        o->remaining--;
        if (o->remaining < o->bufferLength) {
            c = alder_fileseq_token_next(o->fileseq);
            o->buffer[o->remaining] = c;
        } else if (o->pos > 0) {
            o->pos--;
            c = o->buffer[o->pos];
        } else {
            c = alder_fileseq_token_next(o->fileseq);
        }
    } else {
        c = alder_fileseq_token_next(o->fileseq);
    }
    return alder_wordtable_tokenset_token_of(c);
}

bool
alder_wordtable_tokeniterator_moreSequence(alder_wordtable_tokeniterator_t *o)
{
    return alder_fileseq_token_moreSequence(o->fileseq);
}


void
alder_wordtable_tokeniterator_set(alder_wordtable_tokeniterator_t *o,
                                  size_t maxlength,
                                  size_t bufferLength)
{
    assert(o->buffer == NULL);
    assert(o->isBuffered == false);
    o->buffer = malloc(sizeof(*o->buffer) * bufferLength);
    o->bufferLength = bufferLength;
    o->isBuffered = true;
    o->remaining = maxlength;
}

bool
alder_wordtable_tokeniterator_hasMore(alder_wordtable_tokeniterator_t *o)
{
    // Only for files not for bseq.
    if (o->isBuffered) {
        return o->remaining == 0 && o->pos > 0;
    } else {
        return true;
//        return alder_wordtable_tokeniterator_hasNext(o);
    }
}

void
alder_wordtable_tokeniterator_reset(alder_wordtable_tokeniterator_t *o,
                                    size_t maxlength)
{
    assert(o->bufferLength > 0);
    assert(o->isBuffered == true);
    o->remaining = maxlength;
}

