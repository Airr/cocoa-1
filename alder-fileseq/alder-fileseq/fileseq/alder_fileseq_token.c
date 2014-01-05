/**
 * This file, alder_fileseq_token.c, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_fileseq_type.h"
#include "alder_fileseq_token.h"

alder_fileseq_token_t*
alder_fileseq_token_create(const char *fn)
{
    alder_fileseq_token_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    o->fasta = NULL;
    o->bseq = NULL;
    o->bseqI = 0;
    
    o->type = alder_fileseq_type(fn);
    if (o->type == ALDER_FILETYPE_FASTA) {
        o->fasta = alder_fasta_token_create(fn);
        if (o->fasta == NULL) {
            alder_fileseq_token_destroy(o);
            return NULL;
        }
    }
    
    return o;
}

alder_fileseq_token_t*
alder_fileseq_token_createWithBstring(bstring bseq)
{
    alder_fileseq_token_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    o->fasta = NULL;
    o->bseq = NULL;
    
    o->type = ALDER_FILETYPE_BSEQ;
    o->bseq = bstrcpy(bseq);
    o->bseqI = 0;
    return o;
}

void
alder_fileseq_token_setWithBstring(alder_fileseq_token_t *o,
                                   bstring bseq)
{
    assert(o->type == ALDER_FILETYPE_BSEQ);
    bdestroy(o->bseq);
    o->bseq = bstrcpy(bseq);
    o->bseqI = 0;
}

void
alder_fileseq_token_destroy(alder_fileseq_token_t *o)
{
    if (o != NULL) {
        alder_fasta_token_destroy(o->fasta);
        bdestroy(o->bseq);
        XFREE(o);
    }
}

int
alder_fileseq_token_next(alder_fileseq_token_t *o)
{
    if (o->type == ALDER_FILETYPE_FASTA) {
        return alder_fasta_token_next(o->fasta);
    } else if (o->type == ALDER_FILETYPE_BSEQ) {
        return o->bseq->data[o->bseqI++];
    }
    return 0;
}

bool
alder_fileseq_token_hasNext(alder_fileseq_token_t *o)
{
    if (o->type == ALDER_FILETYPE_FASTA) {
        return alder_fasta_token_hasNext(o->fasta);
    } else if (o->type == ALDER_FILETYPE_BSEQ) {
        return o->bseqI < blength(o->bseq) ? true : false;
    }
    return false;
}

bool
alder_fileseq_token_moreSequence(alder_fileseq_token_t *o)
{
    if (o->type == ALDER_FILETYPE_FASTA) {
        return alder_fasta_token_moreSequence(o->fasta);
    } else if (o->type == ALDER_FILETYPE_BSEQ) {
        return false;
    }
    return false;
}
