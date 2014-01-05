/**
 * This file, alder_fileseq_sequenceiterator.c, is part of alder-fileseq.
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
#include "bstrlib.h"
#include "alder_kseq_sequenceiterator.h"
#include "alder_fileseq_type.h"
#include "alder_fasta_token.h"
#include "alder_fileseq_sequenceiterator.h"

enum alder_fileseq_type {
    alder_fileseq_type_kseq,
    alder_fileseq_type_seq
};

struct alder_fileseq_sequenceiterator_struct {
    enum alder_fileseq_type type;
    /* FASTQ */
    /* FASTA */
    alder_kseq_sequenceiterator_t *kseq;
    /* BSEQ */
    bstring bseq;
    int bseqI;
};

static void
alder_fileseq_sequenceiterator_init(alder_fileseq_sequenceiterator_t *o);

alder_fileseq_sequenceiterator_t*
alder_fileseq_sequenceiterator_create()
{
    alder_fileseq_sequenceiterator_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    alder_fileseq_sequenceiterator_init(o);
    
    o->kseq = alder_kseq_sequenceiterator_create();
    return o;
}

void
alder_fileseq_sequenceiterator_destroy(alder_fileseq_sequenceiterator_t *o)
{
    if (o != NULL) {
        alder_kseq_sequenceiterator_destroy(o->kseq);
        XFREE(o);
    }
}

void
alder_fileseq_sequenceiterator_init(alder_fileseq_sequenceiterator_t *o)
{
    o->type = alder_fileseq_type_kseq;
    o->kseq = NULL;
    o->bseq = NULL;
}

int
alder_fileseq_sequenceiterator_open(alder_fileseq_sequenceiterator_t *o,
                                    const char *fn)
{
    int type = alder_fileseq_type(fn);
    if (type == ALDER_FILETYPE_FASTQ ||
        type == ALDER_FILETYPE_FASTA) {
        o->type = alder_fileseq_type_kseq;
        alder_kseq_sequenceiterator_open(o->kseq, fn);
    } else if (type == ALDER_FILETYPE_SEQ) {
        o->type = alder_fileseq_type_seq;
    } else {
        assert(0);
    }
    
    return 0;
}

int
alder_fileseq_sequenceiterator_token(alder_fileseq_sequenceiterator_t *o)
{
    switch (o->type) {
        case alder_fileseq_type_kseq:
            return alder_kseq_sequenceiterator_token(o->kseq);
            break;
        default:
            return 0;
            break;
    }
}

void
alder_fileseq_sequenceiterator_close(alder_fileseq_sequenceiterator_t *o)
{
    switch (o->type) {
        case alder_fileseq_type_kseq:
            alder_kseq_sequenceiterator_close(o->kseq);
            break;
        default:
            break;
    }
}