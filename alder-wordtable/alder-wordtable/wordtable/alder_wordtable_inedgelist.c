/**
 * This file, alder_wordtable_inedgelist.c, is part of alder-wordtable.
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

/* This function provides functions for add edges. Let's see how intblocklist
 * is used.
 *
 * order * tokenbits + endmarkbits + databits
 *
 * endmarkBits can be k or l+1. endmarkbits should accomodate k so we need k+1
 * to find the number of bits for endmarkbits. That is why we use l+2.
 */

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_compute.h"
#include "alder_wordtable_bits.h"
#include "alder_fileseq_token.h"
#include "alder_wordtable_tokeniterator.h"
#include "alder_wordtable_inedgelist.h"

static size_t
alder_wordtable_inedgelist_compactAndAddFinals(alder_wordtable_inedgelist_t *o,
                                               size_t start, size_t end,
                                               bool testFinal);

static bool
alder_wordtable_inedgelist_addFinal(alder_wordtable_inedgelist_t *o,
                                    size_t pos);

static void
alder_wordtable_inedgelist_addFinalRoot(alder_wordtable_inedgelist_t *o);

static bool
alder_wordtable_inedgelist_isFinal(alder_wordtable_inedgelist_t *o,
                                   size_t pos);

static bool
alder_wordtable_inedgelist_finalToDefault(alder_wordtable_inedgelist_t *o,
                                          size_t pos);

static bool
alder_wordtable_inedgelist_nodeEquals(alder_wordtable_inedgelist_t *o,
                                      size_t p, size_t q);

static int
alder_wordtable_inedgelist_indexLength(alder_wordtable_inedgelist_t *o,
                                       size_t position);

static void
alder_wordtable_inedgelist_addToReverse(alder_wordtable_inedgelist_t *o,
                                        int token);

static size_t
alder_wordtable_inedgelist_addReverseBlock(alder_wordtable_inedgelist_t *o,
                                           int token,
                                           alder_wordtable_t endmark);

static size_t
alder_wordtable_inedgelist_addBlock(alder_wordtable_inedgelist_t *o,
                                    alder_wordtable_t data);

static size_t
alder_wordtable_inedgelist_updateBlock(alder_wordtable_inedgelist_t *o,
                                       alder_wordtable_t data);

static void
alder_wordtable_inedgelist_init(alder_wordtable_inedgelist_t* o,
                                size_t order,
                                size_t maxsize,
                                size_t tokenbits,
                                size_t endmarkbits,
                                size_t databits);

static void
alder_wordtable_inedgelist_super(alder_wordtable_inedgelist_t *o,
                                 size_t blocksize);

enum alder_wordtable_inedgelist_token_state {
    alder_wordtable_inedgelist_token_state_begin,
    alder_wordtable_inedgelist_token_state_full,
    alder_wordtable_inedgelist_token_state_end
};

#pragma mark struct

struct alder_wordtable_inedgelist_struct {
    size_t ID;                           /* inedgelist ID for debug          */
    /* add */
    enum alder_wordtable_inedgelist_token_state state;
    bool isBuffering;
    size_t iOrder;
    alder_wordtable_t prefix;
    
    size_t order;                        /* size of node sequence            */
    size_t tokenBits;                    /* number of bits for a token: 2    */
    size_t dataBits;                     /* number of bits for inedge        */
    size_t endmarkBits;                  /* number of bits for order+2       */
    size_t nonindexBits;                 /* number of bits not used          */
    
    alder_wordtable_t nonindexMask;      /* mask token parts in last block   */
    alder_wordtable_t noInedgeValue;     /* 00..0000111 for DNA              */
    alder_wordtable_t endmarkMask;       /* 00..0111000 for DNA endmarkbit 3 */
    alder_wordtable_t defaultEndmark;    /* = endmarkMask                    */
    alder_wordtable_t finalEndmark;      /* 00..0011000 where 011 is order l */
    alder_wordtable_t indexMask;         /* ~nonindexMask or 1111...0000000  */
    
    bool includeReverse;
    size_t addedSequenceCount;           /* number of sequences added        */
    size_t tempReverseLength;
    // intblocklist: contains the array.
    size_t VALUEBITS;                    /* number of bits for a word        */
    size_t blocksize;                    /* number of words in a block       */
    
    size_t maxsize;                      /* maximum number of blocks         */
    alder_wordtable_t *temp;
    alder_wordtable_t *tempReverse;
    alder_wordtable_intblocklist_t *intblocklist;
};

void
alder_wordtable_inedgelist_init(alder_wordtable_inedgelist_t* o,
                                size_t order,
                                size_t maxsize,
                                size_t tokenbits,
                                size_t endmarkbits,
                                size_t databits)
{
    o->ID = 1;
    o->state = alder_wordtable_inedgelist_token_state_begin;
    o->prefix = 0;
    o->iOrder = 0;
    o->isBuffering = false;
    
    o->temp = NULL;
    o->tempReverse = NULL;
    o->intblocklist = NULL;
    
    o->order = order;
    o->tokenBits = tokenbits;
    o->dataBits = databits;
    o->endmarkBits = endmarkbits;
    o->VALUEBITS = sizeof(alder_wordtable_t) * 8;
    
    size_t intblocksize =
    alder_wordtable_compute_groupmaxsize(order * tokenbits + endmarkbits + databits,
                                      o->VALUEBITS);
    o->blocksize = intblocksize;
    
    // 1 * 32 - 3 * 2 = 26.
    o->nonindexBits = o->blocksize * o->VALUEBITS - o->order * o->tokenBits;
    if (o->nonindexBits < o->VALUEBITS) {
        o->nonindexMask = alder_wordtable_bits_intMaskLo(o->nonindexBits);
    } else {
        fprintf(stderr, "Error - impossible");
        abort();
        assert(0);
        o->nonindexMask = -1;
    }
    o->noInedgeValue = alder_wordtable_bits_intMaskLo(o->dataBits);
    o->endmarkMask = alder_wordtable_bits_intMaskLo(o->endmarkBits) << o->dataBits;
    o->defaultEndmark = o->endmarkMask;
    o->finalEndmark = ((alder_wordtable_t)o->order) << o->dataBits;
    
    if (o->nonindexBits < o->VALUEBITS) {
        o->tempReverseLength = o->blocksize;
        o->indexMask = ~(o->nonindexMask);
    } else {
        fprintf(stderr, "Error - impossible");
        abort();
        assert(0);
        o->tempReverseLength = o->blocksize - 1;
        o->indexMask = ~(alder_wordtable_bits_intMaskLo(o->nonindexBits - o->VALUEBITS));
    }
    
    o->addedSequenceCount = 0;
    o->maxsize = maxsize;
}

/* This function is just a create function.
 * I'd rename it: create1.
 * FIXME: why order+2? why not order+1?
 */
alder_wordtable_inedgelist_t*
alder_wordtable_inedgelist_getNew(size_t order, size_t maxsize)
{
    size_t tokenbits = alder_wordtable_bits_log2ceil(ALDER_WORDTABLE_TOKENSIZE);
    size_t databits = alder_wordtable_bits_log2ceil(ALDER_WORDTABLE_TOKENSIZE + 1);
    size_t endmarkbits = alder_wordtable_bits_log2ceil(order + 2);
    return alder_wordtable_inedgelist_create(order,
                                             maxsize,
                                             tokenbits,
                                             endmarkbits,
                                             databits);
}

/* This function is called by create function. It makes intblocklist with
 * the given blocksize. The blocksize is the number of words or 32-bit or 
 * 64-bit values for an incoming edge.
 */
void
alder_wordtable_inedgelist_super(alder_wordtable_inedgelist_t *o,
                                 size_t blocksize)
{
    assert(o->intblocklist == NULL);
    o->intblocklist = alder_wordtable_intblocklist_create(blocksize);
}

/* This function creates inedge list, but use getNew function.
 *
 * Assume that the order is 3.
 * Three kinds of bits: tokens, endmark (3 bits), and prefix data (3 bits).
 * VALUEBITS is the number of bits in a word.
 * bit-value = order * tokenbits (or 2) + endmark + prefix data.
 * I create an intblocklist where a block contains a bit-value one.
 *
 * order        = 3
 * tokenBits    = 2
 * endmarkBits  = 3
 * dataBits     = 3
 * nonindexBits = 26
 * deleteBits   = 26
 * 
 * A few bit-wise values are setup:
 * 0. nonindexMask   = 0 (6) 1 (26).
 * 1. noInedgeValue  = 000111
 * 2. endmarkMask    = 111000
 * 3. defaultEndmark = 111000
 * 4. finalEndmark   = 011000
 * 5. indexMask      = 1 (6) 0 (26).
 */
alder_wordtable_inedgelist_t*
alder_wordtable_inedgelist_create(size_t order,
                                  size_t maxsize,
                                  size_t tokenbits,
                                  size_t endmarkbits,
                                  size_t databits)
{
    alder_wordtable_inedgelist_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_inedgelist_init(o, order, maxsize,
                                    tokenbits, endmarkbits, databits);
    /* memory */
    alder_wordtable_inedgelist_super(o, o->blocksize);
    o->temp = malloc(sizeof(*o->temp) * o->blocksize);
    o->tempReverse = malloc(sizeof(*o->tempReverse) * o->blocksize);
    if (o->temp == NULL || o->tempReverse == NULL ||
        o->intblocklist == NULL) {
        alder_wordtable_inedgelist_destroy(o);
        return NULL;
    }
    return o;
}

/* This function destroys inedgelist.
 */
void
alder_wordtable_inedgelist_destroy(alder_wordtable_inedgelist_t *o)
{
    if (o != NULL) {
        alder_wordtable_intblocklist_destroy(o->intblocklist);
        XFREE(o->temp);
        XFREE(o->tempReverse);
        XFREE(o);
    }
}

/* This function reinitializes inedgelist. It does not free any memory.
 */
void
alder_wordtable_inedgelist_empty(alder_wordtable_inedgelist_t *o)
{
    if (o != NULL) {
        o->ID++;
//        o->state = alder_wordtable_inedgelist_token_state_begin;
//        o->prefix = 0;
//        o->iOrder = 0;
        alder_wordtable_intblocklist_empty(o->intblocklist);
    }
}

#pragma mark add

/*
 * FIXME: memory error?
 * returns
 * true if inedgelist reached its maximum capacity.
 * false otherwise.
 */
bool
alder_wordtable_inedgelist_updateFinalToken(alder_wordtable_inedgelist_t *o)
{
    size_t pos =
    alder_wordtable_inedgelist_updateBlock(o, o->finalEndmark);
    if (pos == ALDER_WORDTABLE_MAX_SIZE_T) {
        assert(0);
        fprintf(stderr, "Error - addFinalToken\n");
        abort();
    }
    o->addedSequenceCount++;
    
    o->state = alder_wordtable_inedgelist_token_state_begin;
    o->iOrder = 0;
    o->isBuffering = false;
    if (pos + 1 < o->maxsize) {
        return false;
    } else {
        return true;
    }
}

/* This function add a token to the inedge list.
 * The intblocklist does not control the number of blocks. It keeps growing
 * until the memory is full. The number of inedges is controlled by inedgelist.
 *
 * 1. Insert order-many tokens from left.
 *    If the temp block is full, then change the state.
 * 2. Add the temp block.
 * 3. Set the buffering if there are order-1 blank blocks left.
 * 4. Insert the current token to the temp block.
 * 5. Get the first letter in the just added block.
 *    Move the 2-bit value of the first letter to LSB.
 */
bool
alder_wordtable_inedgelist_addToken(alder_wordtable_inedgelist_t *o,
                                    int token)
{
    bool isFull = false;
    // 1
    if (!(token < 0)) {
        alder_wordtable_bits_shiftLeft(o->temp, o->blocksize,
                                       (int)o->tokenBits, token);
        alder_log5("Add token to temp block: %c",
                  alder_wordtable_tokenset_char_of(token));
    }
    
    if (o->state == alder_wordtable_inedgelist_token_state_begin) {
        o->iOrder++;
        if (o->iOrder == o->order) {
            o->state = alder_wordtable_inedgelist_token_state_full;
            o->prefix = o->noInedgeValue;
        }
    }
    
    if (o->state == alder_wordtable_inedgelist_token_state_full) {
        // 2
        size_t pos;
        size_t sizeInedgelist = alder_wordtable_inedgelist_size(o);
        if (sizeInedgelist + 1 < o->maxsize) {
            pos = alder_wordtable_inedgelist_addBlock(o,
                                                      o->defaultEndmark |
                                                      o->prefix);
            // 5
            o->prefix = alder_wordtable_intblocklist_getIntAt(o->intblocklist,
                                                              pos, 0);
            o->prefix = o->prefix >> (o->VALUEBITS - o->tokenBits);
        } else {
            pos = alder_wordtable_inedgelist_addBlock(o,
                                                      o->finalEndmark |
                                                      o->prefix);
            o->prefix = o->noInedgeValue;
            o->addedSequenceCount++;
            isFull = true;
        }
        if (pos == ALDER_WORDTABLE_MAX_SIZE_T) {
            assert(0);
            fprintf(stderr, "Error - addToken\n");
            abort();
        }
    }
    return isFull;
}

void
alder_wordtable_inedgelist_offBuffering(alder_wordtable_inedgelist_t *o)
{
    o->isBuffering = false;
}
                                    

/* Add a string to inedgelist.
 */
void
alder_wordtable_inedgelist_addBsequence(alder_wordtable_inedgelist_t *o,
                                        bstring bseq)
{
    alder_wordtable_tokeniterator_t *tokens =
    alder_wordtable_tokeniterator_createWithBstring(bseq);
    
    alder_wordtable_inedgelist_add(o,tokens);
    
    alder_wordtable_tokeniterator_destroy(tokens);
}

void
alder_wordtable_inedgelist_add(alder_wordtable_inedgelist_t *o,
                               alder_wordtable_tokeniterator_t *tokens)
{
    if (o->includeReverse) {
        alder_wordtable_inedgelist_addDual(o, tokens);
    } else {
        alder_wordtable_inedgelist_addSingle(o, tokens);
    }
}

/* This function add an edge list using l-string edge or k-string node.
 *
 * bbbbbb-------------------eeeppp
 *  A C G
 * 6 bits [ACG]
 * 20 bits [nothing]
 * 3 bits endmark
 * 3 bits prefix (noInedgeValue or token)
 *
 * pos : starts at 1.
 *
 * Consider a sequence: ACGTA
 * Order 3:
 * pos 1: ACG + defaultEndmark + noInedge
 * pos 2: CGT + defaultEndmark + A
 * pos 3: GTA + finalEndmark   + C
 */
void
alder_wordtable_inedgelist_addSingle(alder_wordtable_inedgelist_t *o,
                                     alder_wordtable_tokeniterator_t *tokens)
{
    // Push a string of letters (size order) into temp.
    for (size_t i = 0; i < o->order; i++) {
        if (!alder_wordtable_tokeniterator_hasNext(tokens)) {
            return;
        }
        int tokensNext = alder_wordtable_tokeniterator_next(tokens);
        // Push a letter as a 2-bit value into temp.
        alder_wordtable_bits_shiftLeft(o->temp, o->blocksize,
                                       (int)o->tokenBits, tokensNext);
    }
    // The first vertex has no inedge.
    alder_wordtable_t prefix = o->noInedgeValue;
    while (alder_wordtable_tokeniterator_hasNext(tokens)) {
        // Add a block in inedgelist.
        size_t pos = alder_wordtable_inedgelist_addBlock(o,
                                                         o->defaultEndmark |
                                                         prefix);
        int tokensNext = alder_wordtable_tokeniterator_next(tokens);
        alder_wordtable_bits_shiftLeft(o->temp, o->blocksize,
                                       (int)o->tokenBits, tokensNext);
        // Get the first letter in the just added block.
        prefix = alder_wordtable_intblocklist_getIntAt(o->intblocklist, pos, 0);
        // Move the 2-bit value of the first letter to LSB.
        prefix = prefix >> (o->VALUEBITS - o->tokenBits);
    }
    // Add the last block in inedgelist.
    alder_wordtable_inedgelist_addBlock(o, o->finalEndmark | prefix);
    o->addedSequenceCount++;
}

/* FIXME: add reverse as well.
 */
void
alder_wordtable_inedgelist_addDual(alder_wordtable_inedgelist_t *o,
                                   alder_wordtable_tokeniterator_t *tokens)
{
    // Push a string of letters (size order) into temp.
    for (size_t i = 0; i < o->order; i++) {
        if (!alder_wordtable_tokeniterator_hasNext(tokens)) {
            return;
        }
        int tokensNext = alder_wordtable_tokeniterator_next(tokens);
        // Push a letter as a 2-bit value into temp.
        alder_wordtable_bits_shiftLeft(o->temp, o->blocksize,
                                       (int)o->tokenBits, tokensNext);
        int tokenReverse = alder_wordtable_tokenset_reverse(tokensNext);
        alder_wordtable_inedgelist_addToReverse(o,tokenReverse);
    }
    // The first vertex has no inedge.
    alder_wordtable_t prefix = o->noInedgeValue;
    alder_wordtable_t reverseEndmark = o->finalEndmark;
    while (alder_wordtable_tokeniterator_hasNext(tokens)) {
        // Add a block in inedgelist.
        size_t pos = alder_wordtable_inedgelist_addBlock(o,
                                                         o->defaultEndmark |
                                                         prefix);
        int tokensNext = alder_wordtable_tokeniterator_next(tokens);
        alder_wordtable_bits_shiftLeft(o->temp, o->blocksize,
                                       (int)o->tokenBits, tokensNext);
        // Get the first letter in the just added block.
        prefix = alder_wordtable_intblocklist_getIntAt(o->intblocklist, pos, 0);
        // Move the 2-bit value of the first letter to LSB.
        prefix = prefix >> (o->VALUEBITS - o->tokenBits);
        int tokenReverse = alder_wordtable_tokenset_reverse(tokensNext);
        alder_wordtable_inedgelist_addReverseBlock(o,tokenReverse,reverseEndmark);
        alder_wordtable_inedgelist_addToReverse(o,tokenReverse);
        reverseEndmark = o->defaultEndmark;
    }
    // Add the last block in inedgelist.
    alder_wordtable_inedgelist_addBlock(o, o->finalEndmark | prefix);
    alder_wordtable_inedgelist_addReverseBlock(o,o->noInedgeValue,reverseEndmark);
    o->addedSequenceCount += 2;
}

#pragma mark print

void
alder_wordtable_inedgelist_print(alder_wordtable_inedgelist_t *o)
{
    size_t s = alder_wordtable_inedgelist_size(o);
    alder_log4("inedgelist[%zu] %d edges", o->ID, s);
    for (size_t i = 0; i < s; i++) {
        bstring v = alder_wordtable_inedgelist_posAsString(o, i);
        alder_log4("[%02zu]%s", i, bdata(v));
        bdestroy(v);
    }
}

bstring
alder_wordtable_inedgelist_tokensAsString(alder_wordtable_inedgelist_t *o,
                                          size_t pos)
{
    bstring bseq = bfromcstralloc((int)o->order + 1, "");
    alder_wordtable_intblocklist_getBlock(o->intblocklist, o->temp, pos);
    // Decode the block in the temp.
    int n_token = alder_wordtable_inedgelist_indexLength(o,pos);
    for (int i = 0; i < n_token; i++) {
        int x = alder_wordtable_bits_shiftLeft(o->temp, o->blocksize,
                                               (int)o->tokenBits, 0);
        char c = alder_wordtable_tokenset_char_of(x);
        bconchar(bseq,c);
    }
    for (int i = n_token; i < (int)o->order; i++) {
        bconchar(bseq, '$');
    }
    return bseq;
}

bstring
alder_wordtable_inedgelist_edgeAsBitString(alder_wordtable_inedgelist_t *o,
                                           size_t pos)
{
    int bitsize = (int)o->blocksize * sizeof(alder_wordtable_t) * ALDER_WORDTABLE_KFMDEFAULTS_BYTESIZE;
    bstring bseq = bfromcstralloc(bitsize + bitsize/8, "");
    alder_wordtable_intblocklist_getBlock(o->intblocklist, o->temp, pos);
    for (size_t  i = 0; i < bitsize; i++) {
        int x = alder_wordtable_bits_shiftLeft(o->temp, o->blocksize, 1, 0);
        char c = 0x30 + x;
        if (i > 0 && i % 8 == 0) {
            bconchar(bseq, ' ');
        }
        bconchar(bseq, c);
    }
    return bseq;
}

#pragma mark string

/* This function converts the temp block to a string.
 */
bstring
alder_wordtable_inedgelist_tempAsString(alder_wordtable_inedgelist_t *o)
{
    return alder_wordtable_inedgelist_asString(o, o->temp, o->blocksize);
}

bstring
alder_wordtable_inedgelist_posAsString(alder_wordtable_inedgelist_t *o,
                                       size_t pos)
{
    alder_wordtable_t *arr = malloc(sizeof(*arr)*o->blocksize);
    
    alder_wordtable_intblocklist_getBlock(o->intblocklist, arr, pos);
    
    bstring v =
    alder_wordtable_inedgelist_asString(o,arr,o->blocksize);
    
    XFREE(arr);
    
    return v;
}

bstring
alder_wordtable_inedgelist_asString(alder_wordtable_inedgelist_t *o,
                                    alder_wordtable_t *arr,
                                    size_t arrLength)
{
    // as a bit string
    bstring bitString = alder_wordtable_bits_asString(arr, arrLength);
    
    // as a string
    int lengthEdgeString = 4 + (int)o->order + 2 + 2 + 3 + 1;
    bstring edgeString = bfromcstralloc(lengthEdgeString, "");
    
    // Find the number of tokens.
    alder_wordtable_t x3 = 3;
    alder_wordtable_t x = arr[o->blocksize - 1];
    alder_wordtable_t endmark = x & o->endmarkMask;
    alder_wordtable_t databit = x & o->noInedgeValue;
    
    const size_t x30 =
    sizeof(alder_wordtable_t) * ALDER_WORDTABLE_KFMDEFAULTS_BYTESIZE - 2;
    const size_t x16 =
    sizeof(alder_wordtable_t) * ALDER_WORDTABLE_KFMDEFAULTS_BYTESIZE / 2;
    
    size_t nToken = 0;
    if (endmark == o->defaultEndmark) {
        nToken = o->order;
    } else if (endmark == o->finalEndmark) {
        nToken = o->order;
    } else {
        nToken = (size_t)(endmark >> o->dataBits);
    }
    
    if (databit == o->noInedgeValue) {
        bcatcstr(edgeString, "    ");
    } else {
        char c = alder_wordtable_tokenset_char_of((int)databit);
        for (int i = 0; i < 4; i++) {
            if (i == (int)databit) {
                bconchar(edgeString, c);
            } else {
                bconchar(edgeString, ' ');
            }
        }
    }
    bconchar(edgeString, '|');
    if (endmark == o->defaultEndmark) {
        bconchar(edgeString, 'D');
    } else if (endmark == o->finalEndmark) {
        bconchar(edgeString, 'F');
    } else {
        bconchar(edgeString, 'x');
    }
    bconchar(edgeString, ' ');
    
    for (size_t i = 0; i < nToken; i++) {
        size_t x = o->blocksize - 1 - i/x16;
        size_t y = x30 - (i % x16)*2;
        int a = (int)((arr[x] >> y) & x3);
        char c = alder_wordtable_tokenset_char_of(a);
        bconchar(edgeString, c);
    }
    for (size_t i = nToken; i < o->order; i++) {
        bconchar(edgeString, '$');
    }
    bconchar(edgeString, ' ');
    bconchar(edgeString, '-');
    bconchar(edgeString, ' ');
    bconcat(edgeString, bitString);
    bdestroy(bitString);
    return edgeString;
    
}



/* This function returns a token or 111 (or 7) in an edge.
 */
alder_wordtable_t
alder_wordtable_inedgelist_edgeToken(alder_wordtable_inedgelist_t *o,
                                     size_t position)
{
    alder_wordtable_t x = alder_wordtable_intblocklist_getIntAt(o->intblocklist,
                                                                position,
                                                                o->blocksize - 1);
    alder_wordtable_t token = x & o->noInedgeValue;
    return token;
}

/* This function returns the edge character at position.
 */
char
alder_wordtable_inedgelist_edgeTokenAsChar(alder_wordtable_inedgelist_t *o,
                                           size_t position)
{
    assert(position < alder_wordtable_inedgelist_size(o));
    alder_wordtable_t x = alder_wordtable_inedgelist_edgeToken(o, position);
    return alder_wordtable_tokenset_char_of(x);
}

#pragma mark complete

/* This function is a little more complicated than the rest of the functions
 * above. Let's see what this does.
 * 1. sort the node's sequences of length l - 1 or k, where l is the length
 *    of the edge sequence, and k is the length of node.
 * 2. After a sort of node sequences, call compact and add finals. What are
 *    they? compact? and finals?
 *    Let's see what compactAndAddFinals does!
 * 3. Final one more sort.
 *
 * So, this seems to be more complicated than I thought.
 * First, sort the list. Compact it. Add finals. Add final roots. Then, one
 * more final sort. What is it? This is something that I have to deal with.
 * FIXME: Redo the complete.
 *
 * 1. sort -> fine!
 * 2. size -> good!
 * 3. compactAndAddFinals -> fine!
 * 4. compactAndAddFinals could returns the number of edges (mid) that is 
 *    less than the number of actual edges in the list. This is because the mid
 *    is the number of edges after removing duplicates, and compactAndAddFinals
 *    would add more final completing edges.
 * 5. compactAndAddFinals disrupts the sorted list, and the final completing
 *    edges are not yet sorted. 
 * 6. This compactAndAddFinals call only add final completing edges of 
 *    previously added final completing edges.
 * 7. Add the final completing root edge.
 * 8. Sort finally the edge list.
 */
void
alder_wordtable_inedgelist_complete(alder_wordtable_inedgelist_t *o)
{
    // 1
    alder_wordtable_intblocklist_sort(o->intblocklist);
    // 2
    size_t s = alder_wordtable_intblocklist_size(o->intblocklist);
    // 3
    size_t mid = alder_wordtable_inedgelist_compactAndAddFinals(o,0,s,true);
    s = alder_wordtable_intblocklist_size(o->intblocklist);
    // 4
    while (s > mid) {
        size_t end = s;
        // 5
        alder_wordtable_intblocklist_sort2(o->intblocklist, mid, end - 1);
        // 6
        mid = alder_wordtable_inedgelist_compactAndAddFinals(o,mid,end,false);
        s = alder_wordtable_intblocklist_size(o->intblocklist);
    }
    // 7
    alder_wordtable_inedgelist_addFinalRoot(o);
    // 8
    alder_wordtable_intblocklist_sort(o->intblocklist);
}

/* This function is called by complete function. 
 * returns
 * the number of edges after removing duplicates. 
 * Note: this is not the number of edges in the list because final completing
 *       edges were added. The size of the list could be greater than nextpos.
 *
 * Let's see what it does for me.
 * The original code by Rodland says this:
 * Go through the region start..end-1, remove duplicates, and add final nodes.
 * start: 0-based position.
 * end: the number of elements or 1-off from the ending position.
 *
 * This calls finalToDefaults. What is it? finalToDefaults changes the endmark
 * to default endmark if it is not zero.
 *
 * Two nodes are equal if its values are the same upto end mark bits.
 * If they are exactly the same, nextpos is decreased by 1.
 *
 * 1. finalToDefault -> fine!
 *    If the edge is final node, then change it to a normal node.
 *    I know that the node was a final node.
 * 2. nodeEquals (curr and curr - 1) -> okay!
 *    Check if the two edges are the same upto the endmark except the databits.
 * 3. If two nodes are the same (upto endmark),
 *    I compare the databits as well. If they are the same, then two edges
 *    are in deed the same. I move up the nextpos. Note that smaller positions
 *    are upper, and larger positions are lower.
 * 4. Nodes in a group must be all final if the node is final. If there is any
 *    node in the group that is not a final node, then nodes in the group are
 *    not final. This is possible because some final nodes may not be final
 *    any more, if those nodes are added as normal nodes from other sequences.
 * 5. isFinal means that the previous node is final not the current pos one.
 * 6. isFinal is updated to nextIsFinal.
 * 7. If pos > nextpos or there were some duplicates,
 *    copy pos's block to nextpos's block.
 * 8. update nextpos.
 * 9. This is done because end-1 is not treated inside the for-loop.
 *    The just previous pos of end-1, or the position end-2, is treated inside
 *    the for-loop. The list is increased by one this one more final completing
 *    edge.
 * 10. Remove those from nextpos to end.
 *    nextpos ... end-1 are removed.
 *    Note: SORT ARE NEEDED because this function disrupts the sorts.
 * 
 * The function returns
 * the number of edges after removing duplicates. 
 * Note: this is not the number of edges in the list because final completing
 *       edges were added. The size of the list could be greater than nextpos.
 */
size_t
alder_wordtable_inedgelist_compactAndAddFinals(alder_wordtable_inedgelist_t *o,
                                               size_t start, size_t end,
                                               bool testFinal)
{
    size_t nextpos = start + 1;
    bool isFinal = true;
    bool nextIsFinal = true;
    int cnt = 0;
    // 1
    if (testFinal) {
        isFinal = alder_wordtable_inedgelist_finalToDefault(o, start);
    }
    for (size_t pos = start + 1; pos < end; pos++) {
        cnt++;
        // 1
        if (testFinal) {
            nextIsFinal = alder_wordtable_inedgelist_finalToDefault(o, pos);
        }
        // 2
        bool isNodeEqual =
        alder_wordtable_inedgelist_nodeEquals(o, pos, pos - 1);
        // 3
        if (isNodeEqual) {
            alder_wordtable_t x, y;
            x = alder_wordtable_intblocklist_getIntAt(o->intblocklist,
                                                      pos,
                                                      o->blocksize - 1);
            y = alder_wordtable_intblocklist_getIntAt(o->intblocklist,
                                                      pos - 1,
                                                      o->blocksize - 1);
            if (x == y) {
                nextpos--;
            }
            // 4
            isFinal &= nextIsFinal;
        } else {
            // 5
            if (isFinal) {
                alder_wordtable_inedgelist_addFinal(o, pos - 1);
            }
            cnt = 0;
            // 6
            isFinal = nextIsFinal;
        }
        // 7
        if (pos > nextpos) {
            alder_wordtable_intblocklist_copy(o->intblocklist, pos, nextpos);
        }
        // 8
        nextpos++;
    }
    if (isFinal) {
        // 9
        alder_wordtable_inedgelist_addFinal(o, end - 1);
    }
    // 10
    alder_wordtable_intblocklist_remove(o->intblocklist, nextpos, end);
    return nextpos;
}


/* This function converts a final node (endmark=order) to a normal node 
 * (endmark=-1), and return true if done.
 * 1. It gets the last word in the block located at pos.
 * 2. Gets the endmark by masking the rest of the it.
 * 3  If the endmark is finalEndmark,
 *    I replace the endmark with default endmark, and return true.
 *    Note that endmarkMaks is the same as the defaultEndmark.
 * 4. Otherwise, the endmark stays put, then return false.
 */
bool
alder_wordtable_inedgelist_finalToDefault(alder_wordtable_inedgelist_t *o,
                                          size_t pos)
{
    alder_wordtable_t lastWordInBlock =
    alder_wordtable_intblocklist_getIntAt(o->intblocklist,
                                          pos, o->blocksize - 1);
    if ((lastWordInBlock & o->endmarkMask) == o->finalEndmark) {
        alder_wordtable_intblocklist_orIntAt(o->intblocklist,
                                             pos, o->blocksize - 1,
                                             o->endmarkMask);
        return true;
    } else {
        return false;
    }
}

/* This function copies the edge at pos, and appends it to the list. It also
 * modifies the copy so that it follows the original as a final completing
 * node.
 * e.g.,
 * ACG is a node at pos.
 * CG$ is a node that follows the node above.
 *
 * 1. Get the length of the tokens; ACG -> 3, CG$ -> 2, G$$ -> 1, and $$$ -> 0.
 * 2. Copy the block, and append it to the list.
 * 3. Mask out the new block except the tokens in MSB using ~nonindexMask.
 * 4. Shift the block to left by 2 bits, and save the first letter in prefix;
 *    e.g., ACG -> CG. Prefix = A.
 * 5. Set the endmark to the length of the new tokens after shift, 
 *    and set the databits to the prefix.
 *
 * For a given final node, add an final completing edge to the next final node, 
 * unless already at the null node.
 * returns true if a final completing edge is added.
 */
bool
alder_wordtable_inedgelist_addFinal(alder_wordtable_inedgelist_t *o,
                                    size_t pos)
{
    // 1
    int len = alder_wordtable_inedgelist_indexLength(o, pos);
    if (len == 0) {
        return false;
    }
    // 2
    size_t newpos = alder_wordtable_intblocklist_addBlankBlock(o->intblocklist);
    if (newpos == ALDER_WORDTABLE_MAX_SIZE_T) {
        fprintf(stderr, "Error - fix this bug!\n");
        abort();
    }
    alder_wordtable_intblocklist_copy(o->intblocklist, pos, newpos);
    // 3
    alder_wordtable_intblocklist_andIntAt(o->intblocklist,
                                          newpos,
                                          o->blocksize - 1,
                                          ~o->nonindexMask);
    // 4
    int prefix = alder_wordtable_intblocklist_shiftLeft(o->intblocklist,
                                                        newpos,
                                                        (int)o->tokenBits,
                                                        0);
    // 5
    alder_wordtable_intblocklist_orIntAt(o->intblocklist, newpos,
                                         o->blocksize - 1,
                                         ((len - 1) << o->dataBits) | prefix);
    return true;
}

/* Add $$$ the final node.
 *
 * 1. Append a new block to the list.
 * 2. Reset the block to zeros.
 * 3. Set the data bits to noInedgeValue or 0..00111.
 * FIXME: Where do we set the inedgevalue for final completing root edge?
 */
void
alder_wordtable_inedgelist_addFinalRoot(alder_wordtable_inedgelist_t *o)
{
    // 1
    size_t pos = alder_wordtable_intblocklist_addBlankBlock(o->intblocklist);
    if (pos == ALDER_WORDTABLE_MAX_SIZE_T) {
        assert(0);
        fprintf(stderr, "Error - addFinalRoot");
        abort();
    }
    // 2
    for (size_t i = 0; i < o->blocksize - 1; i++) {
        alder_wordtable_intblocklist_setIntAt(o->intblocklist, pos, i, 0);
    }
    // 3
    alder_wordtable_intblocklist_setIntAt(o->intblocklist, pos,
                                          o->blocksize - 1,
                                          o->noInedgeValue);
}

/* Test whether the node is final by checking the end mark is 111000.
 * I first get the last word in the block.
 */
bool
alder_wordtable_inedgelist_isFinal(alder_wordtable_inedgelist_t *o,
                                   size_t pos)
{
    alder_wordtable_t x =
    alder_wordtable_intblocklist_getIntAt(o->intblocklist,
                                          pos, o->blocksize - 1);
    return (x & o->endmarkMask) != o->endmarkMask;
}

/* This function compares two nodes upto endmark bits except the 3 bits.
 * This compares tokens and endmark bits but not databits.
 */
bool
alder_wordtable_inedgelist_nodeEquals(alder_wordtable_inedgelist_t *o,
                                      size_t p, size_t q)
{
    size_t n = alder_wordtable_intblocklist_nBitsEqual(o->intblocklist, p, q);
    size_t diff = o->blocksize * o->VALUEBITS - n;
    return diff <= o->dataBits;
}

#pragma mark others

/* 
 * FIXME: needs more attention.
 * called by alder_wordtable_inedgedata_addEdges.
 *
 * 1. If two blocks are the same, return the number of tokens in a node seq.
 * 2. equal = either the number of tokens
 *    lenP and lenQ are the length of tokens.
 *
 */
size_t
alder_wordtable_inedgelist_nTokensEqual(alder_wordtable_inedgelist_t *o,
                                        size_t p, size_t q)
{
    int equal = alder_wordtable_intblocklist_nBitsEqual(o->intblocklist, p, q);
    if (equal >= o->blocksize * o->VALUEBITS - o->dataBits) {
        // 1
        return o->order;
    }
    // 2
    equal = ALDER_MIN(equal / o->tokenBits, (int)o->order);
    int lenP = alder_wordtable_inedgelist_indexLength(o, p);
    int lenQ = alder_wordtable_inedgelist_indexLength(o, q);
    if (lenP != lenQ) {
        int lenM = ALDER_MIN(lenP, lenQ);
        equal = ALDER_MIN(equal, lenM);
    }
    return equal;
}

/* This function frees some memory.
 */
void
alder_wordtable_inedgelist_compact(alder_wordtable_inedgelist_t *o)
{
    alder_wordtable_intblocklist_compact(o->intblocklist);
}

void
alder_wordtable_inedgelist_sort(alder_wordtable_inedgelist_t *o)
{
    alder_wordtable_intblocklist_sort(o->intblocklist);
}

/* This function returns the number of edges in the list.
 */
size_t
alder_wordtable_inedgelist_size(alder_wordtable_inedgelist_t *o)
{
    return alder_wordtable_intblocklist_size(o->intblocklist);
}

void
alder_wordtable_inedgelist_setIncludeReverse(alder_wordtable_inedgelist_t *o,
                                             bool flag)
{
    o->includeReverse = flag;
}

bool
alder_wordtable_inedgelist_isIncludeReverse(alder_wordtable_inedgelist_t *o)
                                     
{
    return o->includeReverse;
}

size_t
alder_wordtable_inedgelist_order(alder_wordtable_inedgelist_t *o)
{
    return o->order;
}

size_t
alder_wordtable_inedgelist_getAddedSequenceCount(alder_wordtable_inedgelist_t *o)
{
    return o->addedSequenceCount;
}

/* This function simply resets the sequence count.
 */
//void
//alder_wordtable_inedgelist_resetSizeCount(alder_wordtable_inedgelist_t *o)
//{
//    alder_wordtable_intblocklist_resetSizeCount(o->intblocklist);
//    o->addedSequenceCount = 0;
//}

//alder_wordtable_inedgelist_empty(alder_wordtable_inedgelist_t *o)
//{
//    alder_wordtable_intblocklist_empty(o->intblocklist);
//}

void
alder_wordtable_inedgelist_addToReverse(alder_wordtable_inedgelist_t *o,
                                        int token)
{
    alder_wordtable_bits_shiftRight2(o->tempReverse, o->tempReverseLength,
                                     (int)o->tokenBits, token);
    o->tempReverse[o->tempReverseLength - 1] &= o->indexMask;
}

/* This function ...
 * returns
 * the position for a new added block on success
 * ALDER_WORDTABLE_MAX_SIZE_T if there was some error such as not enough memory
 */
size_t
alder_wordtable_inedgelist_addReverseBlock(alder_wordtable_inedgelist_t *o,
                                           int token,
                                           alder_wordtable_t endmark)
{
    size_t pos = alder_wordtable_intblocklist_addBlankBlock(o->intblocklist);
    ALDER_RETURN_IF_EQUAL(pos, ALDER_WORDTABLE_MAX_SIZE_T);
    alder_wordtable_intblocklist_setBlock(o->intblocklist, o->tempReverse, pos);
    alder_wordtable_intblocklist_orIntAt(o->intblocklist, pos,
                                         o->blocksize - 1,
                                         endmark | token);
    return pos;
}

/* This function copies the temp block to the pos position in the list. 
 * It also appends endmark and databits after padding zeros.
 *
 * returns
 * ALDER_WORDTABLE_MAX_SIZE_T if errors occur.
 */
size_t
alder_wordtable_inedgelist_addBlock(alder_wordtable_inedgelist_t *o,
                                    alder_wordtable_t data)
{
    size_t pos = alder_wordtable_intblocklist_addBlankBlock(o->intblocklist);
    ALDER_RETURN_IF_EQUAL(pos, ALDER_WORDTABLE_MAX_SIZE_T);
    alder_wordtable_intblocklist_copyShiftLeftTo(o->intblocklist, o->temp,
                                                 pos,
                                                 (int)o->nonindexBits, data);
#if !defined(NDEBUG)
    bstring v = alder_wordtable_inedgelist_posAsString(o, pos);
    alder_log5("add[%d]: %s", pos, bdata(v));
    bdestroy(v);
#endif
    
    return pos;
}

/* This function is almost a copy of addBlock except that the position is
 * is the end position of current inedgelist not one that is added as a new 
 * one.
 */
size_t
alder_wordtable_inedgelist_updateBlock(alder_wordtable_inedgelist_t *o,
                                       alder_wordtable_t data)
{
    size_t pos = alder_wordtable_inedgelist_size(o) - 1;
    alder_wordtable_t x =
    alder_wordtable_intblocklist_getIntAt(o->intblocklist,
                                          pos,
                                          o->blocksize - 1);
    data |= (x & o->noInedgeValue);
    alder_wordtable_intblocklist_copyShiftLeftTo(o->intblocklist, o->temp,
                                                 pos,
                                                 (int)o->nonindexBits, data);
#if !defined(NDEBUG)
    bstring v = alder_wordtable_inedgelist_posAsString(o, pos);
    alder_log5("add[%d]: %s", pos, bdata(v));
    bdestroy(v);
#endif
    
    return pos;
}

/* This function returns the number of tokens.
 * Default vertex contains the order-many tokens.
 * Non-default vertex contains less than the order-many tokens.
 *   endmark seems to contain the number of tokens.
 *
 * This is correct. For default edge, this function returns the number of
 * tokens in a normal node sequence; l or k-1. For final completing edge,
 * it returns the number of normal tokens, which is equal to the position at
 * which $ starts.
 */
int
alder_wordtable_inedgelist_indexLength(alder_wordtable_inedgelist_t *o,
                                       size_t position)
{
    // Get the last word in the block at the position.
    alder_wordtable_t x =
    alder_wordtable_intblocklist_getIntAt(o->intblocklist,
                                          position,
                                          o->blocksize - 1);
    
    alder_wordtable_t endmark = x & o->endmarkMask;
    if (endmark == o->endmarkMask) {
        // default endmark.
        endmark = (alder_wordtable_t)o->order;
    } else {
        // final endmark.
        endmark = endmark >> o->dataBits;
    }
    return (int)endmark;
}
