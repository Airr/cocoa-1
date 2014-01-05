/**
 * This file, alder_fasta_token.c, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_fasta_token.h"

/* Assumptions:
 * 1. There is at least one sequence in a FASTA file.
 * 2. End-of-line character in each line.
 */

static void
alder_fasta_token_first(alder_fasta_token_t *o)
{
    int c = 0;
    o->bufi = 0;
    o->bufend = 0;
    
    size_t i = 0;
    while (c != '\n') {
        c = fgetc(o->fp);
        if (c == EOF) {
            if (feof(o->fp)) {
                // End of file.
            } else {
                // Error!
            }
            o->endOfFile = true;
            break;
        }
        if (c == '\n') {
            c = fgetc(o->fp);
            if (c == EOF) {
                if (feof(o->fp)) {
                    // End of file.
                } else {
                    // Error!
                }
                o->endOfFile = true;
            } else {
                ungetc(c, o->fp);
            }
            break;
        }
        o->buf[i++] = c;
        if (i == o->bufsize) {
            break;
        }
    }
    if (i > 0) {
        o->moreSequence = true;
    }
    o->bufend = i;
}

bool
alder_fasta_token_hasNext(alder_fasta_token_t *o)
{
    if (o->bufi < o->bufend) {
        return true;
    } else {
        int c = fgetc(o->fp);
        if (c == EOF) {
            if (feof(o->fp)) {
                // End of file.
            } else {
                // Error!
            }
            o->moreSequence = false;
            return false;
        }
        if (c == '\n') {
            // Check if the following line is a title of a sequence.
            c = fgetc(o->fp);
            if (c == EOF) {
                if (feof(o->fp)) {
                    // End of file.
                } else {
                    // Error!
                }
                o->moreSequence = false;
                return false;
            }
        }
        if (c == '>') {
            // Skip the title line.
            while (c != '\n') {
                c = fgetc(o->fp);
            }
            alder_fasta_token_first(o);
            return false;
        }
        
        assert(c == 'A' || c == 'C' || c == 'G' || c == 'T' ||
               c == 'a' || c == 'c' || c == 'g' || c == 't');
        assert(c != '\n');
        ungetc(c, o->fp);
        alder_fasta_token_first(o);
        return true;
    }
}


alder_fasta_token_t*
alder_fasta_token_create(const char *fn)
{
    alder_fasta_token_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    
    /* init */
    o->bfn = NULL;
    o->fp = NULL;
    o->bufsize = 100000;
    o->buf = NULL;
    o->bufi = 0;
    o->bufend = 0;
    o->moreSequence = true;
    o->endOfFile = false;
    
    /* memory */
    o->bfn = bfromcstr(fn);
    o->buf = malloc(sizeof(*o->buf)*o->bufsize);
    if (o->bfn == NULL || o->buf == NULL) {
        alder_fasta_token_destroy(o);
        return NULL;
    }
    memset(o->buf,0,sizeof(*o->buf)*o->bufsize);
    o->fp = fopen(fn, "r");
    if (o->fp == NULL) {
        alder_fasta_token_destroy(o);
        return NULL;
    }
    // Check if the first character is '>'.
    int c = fgetc(o->fp);
    if (c != '>') {
        alder_fasta_token_destroy(o);
        return NULL;
    }
    ungetc(c, o->fp);
    
    alder_fasta_token_hasNext(o);
    if (o->bufend > 0) {
        o->moreSequence = true;
        o->endOfFile = false;
    }
    
    return o;
}

void
alder_fasta_token_destroy(alder_fasta_token_t *o)
{
    if (o != NULL) {
        bdestroy(o->bfn);
        XFCLOSE(o->fp);
        XFREE(o->buf);
        XFREE(o);
    }
}

int
alder_fasta_token_next(alder_fasta_token_t *o)
{
    return o->buf[o->bufi++];
}

bool
alder_fasta_token_moreSequence(alder_fasta_token_t *o)
{
    return o->moreSequence;
}

