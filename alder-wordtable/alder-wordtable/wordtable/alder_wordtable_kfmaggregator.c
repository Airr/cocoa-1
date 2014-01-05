/**
 * This file, alder_wordtable_kfmaggregator.c, is part of alder-wordtable.
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

/* Rodland13 describes this KFMaggreator as one that splits input sequences
 * into blocks of a manageable size so that a KFMindex is built from a block
 * of inedgelist. KFMindex variables are then merged.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_wordtable_kfmindex.h"
#include "alder_wordtable_sequenceiterator.h"
#include "alder_wordtable_tokeniterator.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_kfmmerger.h"
#include "alder_wordtable_kfmaggregator.h"

static int
alder_wordtable_kfmaggregator_inedgelistToKFMindex
(alder_wordtable_kfmaggregator_t *o);

static void
alder_wordtable_kfmaggregator_addKFMindex
(alder_wordtable_kfmaggregator_t *o, alder_wordtable_kfmindex_t *kFM);

static void
alder_wordtable_kfmaggregator_mergeKFMindex
(alder_wordtable_kfmaggregator_t *o);

static void
alder_wordtable_kfmaggregator_init(alder_wordtable_kfmaggregator_t *o,
                                   size_t order,
                                   size_t maxEdgelist,
                                   size_t maxEdgestack);

static void
alder_wordtable_kfmaggregator_complete(alder_wordtable_kfmaggregator_t *o);

//static bool
//alder_wordtable_kfmaggregator_addBlockToKFMsubsets
//(alder_wordtable_kfmaggregator_t *o);
//
//static void
//alder_wordtable_kfmaggregator_addKFMsubset
//(alder_wordtable_kfmaggregator_t *o, alder_wordtable_kfmindex_t *kFM);
//
//static void
//alder_wordtable_kfmaggregator_mergeStackElements
//(alder_wordtable_kfmaggregator_t *o, size_t level);
//
//static bool
//alder_wordtable_kfmaggregator_shouldPrune
//(alder_wordtable_kfmaggregator_t *o,
// size_t level,
// alder_wordtable_kfmindex_t *index);

#pragma mark struct

struct alder_wordtable_kfmaggregator_struct {
    bool pruneAfterFinalMerge;
    size_t alpha;                               /* 4 for DNA                 */
    size_t order;                               /* order or l=k-1            */
    size_t maxEdgelist;                         /* max edges in a inedgelist */
    size_t maxEdgestack;                        /* max edges in the stack    */
    size_t nEdgestack;                          /* num edges in the stack    */
    size_t stepsize;                            /* stepsize for a store-rho  */
    alder_wordtable_stackkfmindex_t *kFMstack;
    alder_wordtable_inedgelist_t *inEdgeList;
    
    /* not useful at the moment */
    /* could dispense with these */
    size_t mergenode;
    size_t pruneIntermediateMergesIntervals;
};

void
alder_wordtable_kfmaggregator_init(alder_wordtable_kfmaggregator_t *o,
                                   size_t order,
                                   size_t maxEdgelist,
                                   size_t maxEdgestack)
{
    o->pruneAfterFinalMerge = true;
    o->alpha = alder_wordtable_tokenset_size();
    o->order = order;
    o->maxEdgelist = maxEdgelist;
    o->maxEdgestack = maxEdgestack;
    o->nEdgestack = 0;
    o->stepsize = ALDER_WORDTABLE_KFMDEFAULTS_STEPSIZE;
    o->inEdgeList = NULL;
    o->kFMstack = NULL;
    
    /* not useful at the moment */
    /* could dispense with these */
    o->mergenode = 0;
    o->pruneIntermediateMergesIntervals =
    ALDER_WORDTABLE_KFMDEFAULTS_PRUNE_INTERMEDIATE_LEVELS;
}

/* The stack must be empty before destroying the KFMindex aggregator.
 */
void
alder_wordtable_kfmaggregator_destroy(alder_wordtable_kfmaggregator_t *o)
{
    if (o != NULL) {
        size_t n = alder_wordtable_stackkfmindex_size(o->kFMstack);
        if (n != 0) {
            assert(0);
        }
        alder_wordtable_stackkfmindex_destroy(o->kFMstack);
        alder_wordtable_inedgelist_destroy(o->inEdgeList);
        XFREE(o);
    }
}

/* This function creates an aggregator for KFMindex of l being order.
 *
 * I'd consider two sections of a memory space. One is for inedgelist, and 
 * the other is for KFMindex stack. Inedges are first read into the inedgelist,
 * and when the inedgelist is full because the number of edges reaches
 * maxEdgelist, the inedgelist is converted to a KFMindex and is pushed into
 * the stack. As the number of edges in the KFMindex variables in the stack
 * reaches the number of maxEdgestack, I'd merge all of the KFMindex variables.
 * Even after the merge of all of the FKMindex variable, if the number of edges
 * in the merged KFMindex is greater than some given fraction of the number,
 * then I'd have to save it in a FKMindex file. Later, after processing all of
 * the sequence files, I'd reload KFMindex variables from files, and try to
 * merge them if the memory is available. 
 */
alder_wordtable_kfmaggregator_t*
alder_wordtable_kfmaggregator_create(size_t order,
                                     size_t maxEdgelist,
                                     size_t maxEdgestack)
{
    alder_wordtable_kfmaggregator_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_kfmaggregator_init(o,order,maxEdgelist,maxEdgestack);
    /* memory */
    o->inEdgeList = alder_wordtable_inedgelist_getNew(order,
                                                      maxEdgelist);
    o->kFMstack = alder_wordtable_stackkfmindex_create(10);
    if (o->inEdgeList == NULL || o->kFMstack == NULL) {
        alder_wordtable_kfmaggregator_destroy(o);
        return NULL;
    }
    return o;
}

/* This function creates an aggregator with a default max block size for the
 * inedgelist.
 */
alder_wordtable_kfmaggregator_t*
alder_wordtable_kfmaggregator_create1(size_t order)
{
    alder_wordtable_kfmaggregator_t *o =
    alder_wordtable_kfmaggregator_create(order,
                                         ALDER_WORDTABLE_KFMDEFAULTS_BLOCKSIZE,
                                         10);
    return o;
}

#pragma mark add

/* This function adds all of the words, and creates a KFMindex variable.
 * sequenceiterator would be crated with an array of sequence files.
 */
size_t
alder_wordtable_kfmaggregator_addAll(alder_wordtable_kfmaggregator_t *o,
                                     alder_wordtable_sequenceiterator_t *seqs)
{
    int token = -1;
    while (token < ALDER_WORDTABLE_TOKENSIZE + 2) {
        token = alder_wordtable_sequenceiterator_token(seqs);
        if (token < ALDER_WORDTABLE_TOKENSIZE) {
            // Add a token to inedgelist.
            bool isFull = alder_wordtable_inedgelist_addToken(o->inEdgeList, token);
            
            if (isFull) {
                alder_wordtable_kfmaggregator_inedgelistToKFMindex(o);
                alder_wordtable_inedgelist_addToken(o->inEdgeList, -1);
            }
            
        } else if (token == ALDER_WORDTABLE_TOKENSIZE) {
            // End of a sequence
            bool isFull = alder_wordtable_inedgelist_updateFinalToken(o->inEdgeList);
            if (isFull) {
                alder_wordtable_kfmaggregator_inedgelistToKFMindex(o);
            }
        } else if (token == ALDER_WORDTABLE_TOKENSIZE + 2) {
            // End of all of the files
            alder_wordtable_kfmaggregator_inedgelistToKFMindex(o);
        } else {
            // End of a file
            assert(token == ALDER_WORDTABLE_TOKENSIZE + 1);
        }
    }
    // Convert any remaining blocks to a KFMindex.
    alder_wordtable_kfmaggregator_inedgelistToKFMindex(o);
    // inedgelist must be zero in size.
    size_t inEdgeList_size = alder_wordtable_inedgelist_size(o->inEdgeList);
    assert(inEdgeList_size == 0);
    return 0;
}

#pragma mark replace

int
alder_wordtable_kfmaggregator_inedgelistToKFMindex
(alder_wordtable_kfmaggregator_t *o)
{
    size_t inEdgeList_size = alder_wordtable_inedgelist_size(o->inEdgeList);
    if (inEdgeList_size == 0) {
        return 0;
    }
    
    // 1
    alder_wordtable_inedgelist_complete(o->inEdgeList);
#if !defined(NDEBUG)
    alder_wordtable_inedgelist_print(o->inEdgeList);
#endif
    alder_wordtable_kfmindex_t *index =
    alder_wordtable_kfmindex_create2(o->inEdgeList, o->stepsize);
#if !defined(NDEBUG)
    alder_wordtable_kfmindex_print(index);
#endif
    
    // 2
    alder_wordtable_kfmaggregator_addKFMindex(o, index);
    // 3
    alder_wordtable_inedgelist_empty(o->inEdgeList);
    return true;
}

void
alder_wordtable_kfmaggregator_addKFMindex
(alder_wordtable_kfmaggregator_t *o, alder_wordtable_kfmindex_t *kFM)
{
    o->nEdgestack += alder_wordtable_kfmindex_size(kFM);
    alder_wordtable_stackkfmindex_push(kFM, o->kFMstack);
    if (o->maxEdgestack < o->nEdgestack) {
        // Merge KFMindex.
        size_t nStack = alder_wordtable_stackkfmindex_size(o->kFMstack);
        while (nStack > 1) {
            alder_wordtable_kfmaggregator_mergeKFMindex(o);
            nStack = alder_wordtable_stackkfmindex_size(o->kFMstack);
        }
    }
}

/* This merges top two kFM-index stack elements.
 * Level determines how often we prune merged KFMindex.
 * Add the merged one back to the stack.
 */
void
alder_wordtable_kfmaggregator_mergeKFMindex
(alder_wordtable_kfmaggregator_t *o)
{
    alder_wordtable_kfmindex_t *aIndex =
    alder_wordtable_stackkfmindex_pop(o->kFMstack);
    
    alder_wordtable_kfmindex_t *bIndex =
    alder_wordtable_stackkfmindex_pop(o->kFMstack);
    
#if !defined(NDEBUG)
    alder_log4("aIndex:");
    alder_wordtable_kfmindex_print(aIndex);
    alder_log4("bIndex:");
    alder_wordtable_kfmindex_print(bIndex);
#endif
    
    size_t aSize = alder_wordtable_kfmindex_size(aIndex);
    size_t bSize = alder_wordtable_kfmindex_size(bIndex);
    o->nEdgestack -= (aSize + bSize);
    
    alder_wordtable_kfmindex_t *kFM = 
    alder_wordtable_kfmmerger_mergeTwoKFMindex(aIndex, bIndex, true, o->stepsize);
    
#if !defined(NDEBUG)
    alder_log4("merged Index:");
    alder_wordtable_kfmindex_print(kFM);
#endif
    
    alder_wordtable_kfmindex_pruneFinalCompletions(kFM);
    
#if !defined(NDEBUG)
    alder_log4("Pruned merged Index:");
    alder_wordtable_kfmindex_print(kFM);
#endif
    
    alder_wordtable_stackkfmindex_push(kFM, o->kFMstack);
    size_t mSize = alder_wordtable_kfmindex_size(kFM);
    o->nEdgestack += mSize;
}


#pragma mark kfm

/* This function returns the resulting KFMindex.
 */
alder_wordtable_kfmindex_t*
alder_wordtable_kfmaggregator_getKFMindex
(alder_wordtable_kfmaggregator_t *o)
{
    alder_wordtable_kfmaggregator_inedgelistToKFMindex(o);
    size_t s = alder_wordtable_stackkfmindex_size(o->kFMstack);
    if (s == 0) {
        return NULL;
    }
    
    alder_wordtable_kfmaggregator_complete(o);
    s = alder_wordtable_inedgelist_size(o->inEdgeList);
//    alder_wordtable_inedgelist_empty(o->inEdgeList);
//    alder_wordtable_inedgelist_compact(o->inEdgeList);
    // inedgelist must be empty.
    assert(s == 0);
    
    alder_wordtable_kfmindex_t *index =
    alder_wordtable_stackkfmindex_pop(o->kFMstack);
    
    size_t sizeOfStack = alder_wordtable_stackkfmindex_size(o->kFMstack);
    assert(sizeOfStack == 0);
    
    bool x = alder_wordtable_kfmindex_isPruned(index);
    if (o->pruneAfterFinalMerge && !x) {
        alder_wordtable_kfmindex_pruneFinalCompletions(index);
    }
    return index;
}

/* 
 * This completes data, merging all kFM-indices.
 */
void
alder_wordtable_kfmaggregator_complete(alder_wordtable_kfmaggregator_t *o)
{
    size_t x = alder_wordtable_stackkfmindex_size(o->kFMstack);
    while (x > 1) {
        alder_wordtable_kfmaggregator_mergeKFMindex(o);
        x = alder_wordtable_stackkfmindex_size(o->kFMstack);
    }
}


#pragma mark others

void
alder_wordtable_kfmaggregator_setIncludeReverse(alder_wordtable_kfmaggregator_t *o,
                                          bool flag)
{
    alder_wordtable_inedgelist_setIncludeReverse(o->inEdgeList, flag);
}

void
alder_wordtable_kfmaggregator_setStepsize(alder_wordtable_kfmaggregator_t *o,
                                          size_t stepsize)
{
    o->stepsize = stepsize;
}

size_t
alder_wordtable_kfmaggregator_getStepsize(alder_wordtable_kfmaggregator_t *o)
{
    return o->stepsize;
}

size_t
alder_wordtable_kfmaggregator_getBlocksize(alder_wordtable_kfmaggregator_t *o)
{
    return o->maxEdgelist;
}

#pragma mark stack

/* This function adds a new block of KFMindex.
 * 
 * 1. Complete inedgelist and create a KFMindex from it.
 * 2. Merge the KFMindex.
 * 3. Free the inedgelist.
 */
//bool
//alder_wordtable_kfmaggregator_addBlockToKFMsubsets
//(alder_wordtable_kfmaggregator_t *o)
//{
//    size_t inEdgeList_size = alder_wordtable_inedgelist_size(o->inEdgeList);
//    assert(inEdgeList_size > 0);
//    
//    
//    // 1
//    alder_wordtable_inedgelist_complete(o->inEdgeList);
//#if !defined(NDEBUG)
//    alder_wordtable_inedgelist_print(o->inEdgeList);
//#endif
//    alder_wordtable_kfmindex_t *index =
//    alder_wordtable_kfmindex_create2(o->inEdgeList, o->stepsize);
//    
//    // 2
//    alder_wordtable_kfmaggregator_addKFMsubset(o, index);
//    // 3
//    alder_wordtable_inedgelist_empty(o->inEdgeList);
//    return true;
//}

/* This function
 * Add kFM-index to stack, merging indices as we go 
 *
 *    c size in stack
 *      1 1
 *     10 2 -> 1
 *     11 2
 *    100 3 -> 1
 *    101 2
 *    110 3 -> 2
 *    111 3
 *   1000 4 -> 1
 *   1001 2
 *   1010 3 -> 2
 * ...
 *  10000 5 -> 1
 * ...
 * 100000 6 -> 1
 * You can see that the number of zeros is equal to the number of merge
 * operations. This would increase the number of KFMindex as I add more 
 * KFMindex. 
 * 
 * I need to do something else. Consider the memory space of the following: 
 * A is for inedgelist, and B is for KFMindex stack. As the space A is full
 * because of the full inedgelist, I convert the inedgelst to a KFMindex, and
 * save it in the memory space B. I expect that the converted KFMindex would 
 * take much less memory space so the space B could contain multiple KFMindex.
 * The space B could be considered a stack of KFMindex variables. When the 
 * space B is full because of KFMindex variables, I'd merge all of the KFMindex
 * variables into one. The merged one could take smaller space than the sapce
 * B. So, I'd continue to add more KFMindex from more inedge list by filling
 * inedges in the space A. When the space B is full even after merge, I'd have
 * to save the KFMindex in the space B in a file.
 *
 * -------
 * |     |
 * |  A  |
 * |     |
 * -------
 * |     |
 * |  B  |
 * |     |
 * -------
 */
//void
//alder_wordtable_kfmaggregator_addKFMsubset
//(alder_wordtable_kfmaggregator_t *o, alder_wordtable_kfmindex_t *kFM)
//{
//    o->nEdgestack += alder_wordtable_kfmindex_size(kFM);
//    
//    alder_wordtable_stackkfmindex_push(kFM, o->kFMstack);
//    o->mergenode++;
//    size_t c = o->mergenode;
//    size_t level = 0;
//    while ((c & 1) == 0) {
//        c >>= 1;
//        alder_wordtable_kfmaggregator_mergeStackElements(o, ++level);
//    }
//}

/* This merges top two kFM-index stack elements.
 * Level determines how often we prune merged KFMindex.
 * Add the merged one back to the stack.
 */
//void
//alder_wordtable_kfmaggregator_mergeStackElements
//(alder_wordtable_kfmaggregator_t *o, size_t level)
//{
//    alder_wordtable_kfmindex_t *aIndex =
//    alder_wordtable_stackkfmindex_pop(o->kFMstack);
//    
//    alder_wordtable_kfmindex_t *bIndex =
//    alder_wordtable_stackkfmindex_pop(o->kFMstack);
//    
//    alder_wordtable_kfmindex_t *kFM = 
//    alder_wordtable_kfmmerger_mergeTwoKFMindex(aIndex, bIndex, true, o->stepsize);
//    
//    bool shouldPrune = alder_wordtable_kfmaggregator_shouldPrune(o, level, kFM);
//    if (shouldPrune) {
//        alder_wordtable_kfmindex_pruneFinalCompletions(kFM);
//    }
//    alder_wordtable_stackkfmindex_push(kFM, o->kFMstack);
//}

/* This checks if just merged kFM-index should be pruned.
 */
//bool
//alder_wordtable_kfmaggregator_shouldPrune
//(alder_wordtable_kfmaggregator_t *o,
// size_t level,
// alder_wordtable_kfmindex_t *index)
//{
//    if (alder_wordtable_kfmindex_isPruned(index)) {
//        return false;
//    }
//    if (o->pruneIntermediateMergesIntervals <= 0) {
//        return false;
//    }
//    return (level % o->pruneIntermediateMergesIntervals) == 0;
//}
