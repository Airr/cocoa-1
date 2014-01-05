/**
 * This file, alder_wordtable_sequenceiterator.c, is part of alder-wordtable.
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
#include "alder_logger.h"
#include "bstrmore.h"
#include "alder_fileseq.h"
#include "alder_wordtable_tokenset.h"
#include "alder_wordtable_tokeniterator.h"
#include "alder_wordtable_sequenceiterator.h"

enum alder_wordtable_sequenceiterator_state {
    alder_wordtable_sequenceiterator_state_open,
    alder_wordtable_sequenceiterator_state_buffering,
    alder_wordtable_sequenceiterator_state_buffered,
    alder_wordtable_sequenceiterator_state_begin
};

struct alder_wordtable_sequenceiterator_struct {
    enum alder_wordtable_sequenceiterator_state state;
    size_t iFile;
    size_t buffersize;
    size_t iBuffer;
    int *buffer;
    struct bstrList *file;
    alder_fileseq_sequenceiterator_t *fileseq;
};

static void
alder_wordtable_sequenceiterator_init(alder_wordtable_sequenceiterator_t *o,
                                      size_t buffersize)
{
    o->state = alder_wordtable_sequenceiterator_state_begin;
    o->iFile = 0;
    o->iBuffer = 0;
    o->buffersize = buffersize;
    o->buffer = NULL;
    o->file = NULL;
    o->fileseq = NULL;
}

alder_wordtable_sequenceiterator_t*
alder_wordtable_sequenceiterator_create(struct bstrList *fn,
                                        size_t buffersize)
{
    alder_wordtable_sequenceiterator_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_sequenceiterator_init(o,buffersize);
    /* memory */
    o->file = bstrVectorCopy(fn);
    o->buffer = malloc(sizeof(*o->buffer) * buffersize);
    o->fileseq = alder_fileseq_sequenceiterator_create();
    if (o->file == NULL ||
        o->buffer == NULL ||
        o->fileseq == NULL) {
        alder_wordtable_sequenceiterator_destroy(o);
        return NULL;
    }
    return o;
}

void
alder_wordtable_sequenceiterator_destroy(alder_wordtable_sequenceiterator_t *o)
{
    if (o != NULL) {
        assert(o->iBuffer == 0);
        XFREE(o->buffer);
        bstrVectorDelete(o->file); o->file = NULL;
        alder_fileseq_sequenceiterator_destroy(o->fileseq);
        XFREE(o);
    }
}

/* Each call of next would return 0,1,2,3, or 4. The numbers less than 4 
 * denote A, C, G, or T. The number 4 implies that a sequence ends there.
 * Nondisplay characters are ignored. Ambiguous characters and the bracket
 * is converted to the number 4. These rules apply to FASTA format files.
 * In FASTQ files, characters must be contiguous. The end of a line character
 * ends a sequence. A number for each site in a read sequence of a FASTQ file
 * accompanies the corresponding character; the quality score for the 
 * character. A file of much simpler format, called seq, would contain 
 * one sequence a line. The end of a line ends the sequence in the line.
 * I support these three types of sequence formats.
 *
 * The client code could call this function. If you got 4 twice in a row,
 * there is no left token in the iterator, and you should close it.
 *
 * Consider the following cases:
 * 1. A file could be already open. Then, just return a token.
 * 2. No file is open. Open a new file and return a token.
 * 3. Buffing state: save order - 1.
 */
int
alder_wordtable_sequenceiterator_token(alder_wordtable_sequenceiterator_t *o)
{
    int token = ALDER_WORDTABLE_TOKENSIZE;
    if (o->state == alder_wordtable_sequenceiterator_state_open) {
        // use the open file to get a next token.
        token = alder_fileseq_sequenceiterator_token(o->fileseq);
        assert(token <= ALDER_WORDTABLE_TOKENSIZE + 1);
        if (token == ALDER_WORDTABLE_TOKENSIZE + 1) {
            // End of File: reopen another file in the next call.
            o->state = alder_wordtable_sequenceiterator_state_begin;
//            token = ALDER_WORDTABLE_TOKENSIZE;
            
        }
    } else if (o->state == alder_wordtable_sequenceiterator_state_begin) {
        // open a file.
        if (o->iFile < o->file->qty) {
            size_t i = o->iFile;
            o->iFile++;
            if (i > 0) {
                alder_fileseq_sequenceiterator_close(o->fileseq);
            }
            alder_fileseq_sequenceiterator_open(o->fileseq,
                                                bdata(o->file->entry[i]));
            // read a token.
            token = alder_fileseq_sequenceiterator_token(o->fileseq);
            assert(token <= ALDER_WORDTABLE_TOKENSIZE + 1);
            if (token == ALDER_WORDTABLE_TOKENSIZE + 1) {
                // End of File: reopen another file in the next call.
                token = ALDER_WORDTABLE_TOKENSIZE;
            } else {
                o->state = alder_wordtable_sequenceiterator_state_open;
            }
        } else {
            token = ALDER_WORDTABLE_TOKENSIZE + 2;
        }
    } else if (o->state == alder_wordtable_sequenceiterator_state_buffering) {
        // read a token.
        token = alder_fileseq_sequenceiterator_token(o->fileseq);
        assert(token <= ALDER_WORDTABLE_TOKENSIZE + 1);
        if (token == ALDER_WORDTABLE_TOKENSIZE + 1) {
            // End of File: revoke the buffering.
            o->state = alder_wordtable_sequenceiterator_state_begin;
            o->iBuffer = 0;
//            token = ALDER_WORDTABLE_TOKENSIZE;
        } else if (token == ALDER_WORDTABLE_TOKENSIZE) {
            // End of Seq: revoke the buffering.
            o->state = alder_wordtable_sequenceiterator_state_open;
            o->iBuffer = 0;
        } else {
            // add it to the buffer
            o->buffer[o->iBuffer] = token;
            o->iBuffer++;
#if !defined(NDEBUG)
            assert(o->iBuffer <= o->buffersize);
            for (size_t i = 0; i < o->iBuffer; i++) {
                alder_log5("buffer[%zu]: %c", i,
                           alder_wordtable_tokenset_char_of(o->buffer[i]));
            }
#endif
        }
        if (o->iBuffer == o->buffersize) {
            o->state = alder_wordtable_sequenceiterator_state_buffered;
        }
    } else if (o->state == alder_wordtable_sequenceiterator_state_buffered) {
        // use the buffered token
        size_t i = o->buffersize - o->iBuffer;
        if (i == 0) {
            token = ALDER_WORDTABLE_TOKENSIZE;
            o->iBuffer--;
        } else {
            token = o->buffer[i-1];
            if (o->iBuffer == 0) {
                o->state = alder_wordtable_sequenceiterator_state_open;
            } else {
                o->iBuffer--;
            }
        }
    }
    return token;
}

/* Set the buffer true. 
 * The next o->buffersize tokens would be saved.
 * When the buffer is full, the final token or 4 will be returned by token
 * function above.
 */
void
alder_wordtable_sequenceiterator_buffer(alder_wordtable_sequenceiterator_t *o)
{
    o->state = alder_wordtable_sequenceiterator_state_buffering;
}

bool
alder_wordtable_sequenceiterator_isBuffering
(alder_wordtable_sequenceiterator_t *o)
{
    return
    o->state == alder_wordtable_sequenceiterator_state_buffering ||
    o->state == alder_wordtable_sequenceiterator_state_buffered;
}
