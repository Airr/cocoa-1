/**
 * This file, alder_fastq_sequenceiterator.c, is part of alder-fastq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fastq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fastq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <zlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_file_isgzip.h"
#include "alder_kseq.h"
#include "alder_token_dna.h"
#include "alder_fastq_sequenceiterator.h"

enum alder_fastq_sequenceiterator_state {
    alder_fastq_sequenceiterator_state_begin,
    alder_fastq_sequenceiterator_state_open,
    alder_fastq_sequenceiterator_state_end
};

struct alder_fastq_sequenceiterator_struct {
    enum alder_fastq_sequenceiterator_state state;
    int type; // 0: regular file, 1: gzipped file
    int iSeq;
    void *f;
    kseq_t *seq;
};

static void
alder_fastq_sequenceiterator_init(alder_fastq_sequenceiterator_t *o);


/* This function tests the call of the functions in this file.
 */
void
alder_fastq_sequenceiterator_test(struct bstrList *fn)
{
    alder_fastq_sequenceiterator_t *fiter =
    alder_fastq_sequenceiterator_create();
    
    for (int i = 0; i < fn->qty; i++) {
        alder_fastq_sequenceiterator_open(fiter,
                                          bdata(fn->entry[i]));
        int token = alder_fastq_sequenceiterator_token(fiter);
        while (token <= ALDER_TOKEN_DNA_SIZE) {
            if (token < ALDER_TOKEN_DNA_SIZE) {
                char c = alder_token_dna_char_of(token);
                printf("%c", c);
            } else {
                printf("\n");
            }
            token = alder_fastq_sequenceiterator_token(fiter);
        }
        alder_fastq_sequenceiterator_close(fiter);
    }
    alder_fastq_sequenceiterator_destroy(fiter);
}


alder_fastq_sequenceiterator_t*
alder_fastq_sequenceiterator_create()
{
    alder_fastq_sequenceiterator_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    alder_fastq_sequenceiterator_init(o);
    return o;
}

void
alder_fastq_sequenceiterator_destroy(alder_fastq_sequenceiterator_t *o)
{
    if (o != NULL) {
        alder_fastq_sequenceiterator_close(o);
        XFREE(o);
    }
}

void
alder_fastq_sequenceiterator_init(alder_fastq_sequenceiterator_t *o)
{
    o->state = alder_fastq_sequenceiterator_state_begin;
    o->type = 0;
    o->f = NULL;
    o->seq = NULL;
}

/* This function close the current file.
 */
void
alder_fastq_sequenceiterator_close(alder_fastq_sequenceiterator_t *o)
{
    if (o != NULL) {
        alder_kseq_destroy(o->seq);
        o->seq = NULL;
        if (o->type == 0) {
            close((int)((intptr_t)o->f));
        } else {
            gzclose(o->f);
        }
        alder_fastq_sequenceiterator_init(o);
    }
}

/* This function opens a FASTQ file.
 */
int
alder_fastq_sequenceiterator_open(alder_fastq_sequenceiterator_t *o,
                                  const char *fn)
{
    int fp = -1;
    int isGzip = 0;
    gzFile fpgz = Z_NULL;
    if (fn == NULL) {
        fp = STDIN_FILENO;
    } else {
        isGzip = alder_file_isgzip(fn);
        if (isGzip == 1) {
            fpgz = gzopen(fn, "r");
        } else {
            fp = open(fn, O_RDONLY);
        }
    }
    if (fp < 0 && fpgz == Z_NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", fn);
        return ALDER_STATUS_ERROR;
    }
    
    kseq_t *seq = NULL;
    assert(isGzip == 0 || isGzip == 1);
    if (isGzip == 0) {
        seq = alder_kseq_init((void *)(intptr_t)fp, isGzip);
    } else {
        seq = alder_kseq_init(fpgz, isGzip);
    }
    if (seq == NULL) {
        if (isGzip == 0) { close(fp); } else { gzclose(fpgz); }
        logc_logErrorBasic(ERROR_LOGGER, 0, "cannot open %s.\n", fn);
        return ALDER_STATUS_ERROR;
    }
    
    o->type = isGzip;
    o->seq = seq;
    if (isGzip == 0) {
        o->f = (void *) (intptr_t) fp;
    } else {
        o->f = fpgz;
    }
    
    return ALDER_STATUS_SUCCESS;
}

/* This function returns a token from a FASTQ file.
 * 0,1,2,3 for A,C,G,T.
 * 4 for any other; means the end of a sequence
 * 5 for End of File.
 */
int
alder_fastq_sequenceiterator_token(alder_fastq_sequenceiterator_t *o)
{
    int token = ALDER_TOKEN_DNA_SIZE + 1;
    
    if (o->state == alder_fastq_sequenceiterator_state_begin) {
        // read a sequence.
        int status = alder_kseq_read(o->seq);
        if (status > 0) {
            o->state = alder_fastq_sequenceiterator_state_open;
            o->iSeq = 0;
            assert(o->iSeq < o->seq->seq.l);
        } else {
            // End of File.
            o->state = alder_fastq_sequenceiterator_state_end;
            token = ALDER_TOKEN_DNA_SIZE + 1;
        }
    }
    
    if (o->state == alder_fastq_sequenceiterator_state_open) {
        if (o->iSeq < o->seq->seq.l) {
            token = alder_token_dna_token_of(o->seq->seq.s[o->iSeq]);
            o->iSeq++;
            if (token == ALDER_TOKEN_DNA_SIZE) {
                while (token == ALDER_TOKEN_DNA_SIZE &&
                       o->iSeq < o->seq->seq.l) {
                    token = alder_token_dna_token_of(o->seq->seq.s[o->iSeq]);
                    o->iSeq++;
                }
                // case: ...ACNNN
                if (token == ALDER_TOKEN_DNA_SIZE) {
                    assert(o->iSeq == o->seq->seq.l);
                    o->state = alder_fastq_sequenceiterator_state_begin;
                }
                // case: ...ANNNA
                // case: ...ANNAC
                else {
                    o->iSeq--;
                }
                token = ALDER_TOKEN_DNA_SIZE;
            }
        } else {
            token = ALDER_TOKEN_DNA_SIZE;
            o->state = alder_fastq_sequenceiterator_state_begin;
        }
    }
    
    return token;
}











 