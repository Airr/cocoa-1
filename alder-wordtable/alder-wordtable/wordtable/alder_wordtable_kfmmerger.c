/**
 * This file, alder_wordtable_kfmmerger.c, is part of alder-wordtable.
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

/* This file features functions for merging two KFMindex variable.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_bigbitarray.h"
#include "alder_wordtable_kfmindex.h"
#include "alder_wordtable_kfmmerger.h"

struct alder_wordtable_kfmmerger_struct {
    size_t alpha;
    size_t order;
    alder_wordtable_kfmindex_t *a;              /* not own */
    alder_wordtable_kfmindex_t *b;              /* not own */
    alder_wordtable_bigbitarray_t *isA;
    alder_wordtable_bigbitarray_t *isSameNode;
    alder_wordtable_bigbitarray_t *isSameGroup;
    bool deleteSource;
    alder_wordtable_kfmindex_t *m;              /* create and return */
};

/* This would be the main merge function.
 */
alder_wordtable_kfmindex_t*
alder_wordtable_kfmmerger_mergeTwoKFMindex(alder_wordtable_kfmindex_t *a,
                                           alder_wordtable_kfmindex_t *b,
                                           bool deleteSources,
                                           size_t stepsize)
{
    alder_wordtable_kfmmerger_t *o =
    alder_wordtable_kfmmerger_create(a,b,deleteSources);
    
    alder_wordtable_kfmmerger_merge(o, stepsize);
    alder_wordtable_kfmindex_t *m = o->m;
    alder_wordtable_kfmmerger_destroy(o);
    return m;
}

/* This function frees the merger object with bit arrays.
 */
void
alder_wordtable_kfmmerger_destroy(alder_wordtable_kfmmerger_t *o)
{
    if (o != NULL) {
        alder_wordtable_bigbitarray_destroy(o->isA);
        alder_wordtable_bigbitarray_destroy(o->isSameGroup);
        alder_wordtable_bigbitarray_destroy(o->isSameNode);
        XFREE(o);
    }
}

/* This function is called by the merge function to merge two KFMindex objects.
 * 1. The first KFMindex must be smaller in size.
 * 2. The order must be the same.
 * 3. The token set size must be the same.
 * 4.
 */
alder_wordtable_kfmmerger_t*
alder_wordtable_kfmmerger_create(alder_wordtable_kfmindex_t *a,
                                 alder_wordtable_kfmindex_t *b,
                                 bool deleteSources)
{
    alder_wordtable_kfmmerger_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    /* init */
    o->isA = NULL;
    o->isSameGroup = NULL;
    o->isSameNode = NULL;
    o->order = 0;
    o->alpha = 0;
    o->a = NULL;
    o->b = NULL;
    o->m = NULL;
    o->deleteSource = false;
    
    /* set */
    o->deleteSource = deleteSources;
    size_t a_size = alder_wordtable_kfmindex_size(a);
    size_t b_size = alder_wordtable_kfmindex_size(b);
    /* a is always smaller in size. */
    o->a = a_size > b_size ? b : a;
    o->b = a_size > b_size ? a : b;
    size_t a_order = alder_wordtable_kfmindex_order(a);
    size_t b_order = alder_wordtable_kfmindex_order(b);
    if (a_order != b_order) {
        alder_wordtable_kfmmerger_destroy(o);
        return NULL;
    }
    o->order = ALDER_MAX(a_order, b_order);
    size_t a_tokenset_size = alder_wordtable_kfmindex_tokenset_size(a);
    size_t b_tokenset_size = alder_wordtable_kfmindex_tokenset_size(b);
    assert(a_tokenset_size == b_tokenset_size);
    if (a_tokenset_size != b_tokenset_size) {
        alder_wordtable_kfmmerger_destroy(o);
        return NULL;
    }
    o->alpha = a_tokenset_size;
    
    /* memory */
    o->isA = alder_wordtable_bigbitarray_create(a_size + b_size);
    o->isSameNode = alder_wordtable_bigbitarray_create(a_size);
    o->isSameGroup = alder_wordtable_bigbitarray_create(a_size + b_size);
    if (o->isA == NULL ||
        o->isSameNode == NULL ||
        o->isSameGroup == NULL ) {
        alder_wordtable_kfmmerger_destroy(o);
        return NULL;
    }
    return o;
}

/* Algorithm 6. Merge two kFMindex objects.
 *
 * This does not seem to be intuitive unless I thoroughly see what this is 
 * about. In the for-loop, do the merge prep for final edge and final
 * completing edges. The start position is at the final node or 0th.
 * For example, 
 * l = 0. $$$
 * l = 1. ?$$.
 * l = 2. ??$
 * Note that group checking is done by the l + 1 level.
 * e.g.,
 * l = 0 $$$ group checking is done by l = 1 when l = 1.
 * l = 1 group checking is done by l = 2 when l becomes 1.
 * After prep for final completing edges, I do the prep for normal edges.
 * 
 * Consider two lists:
 * A: 1' 2' 3'
 * B: 1  3  5  7
 * sameA: 0 0 0
 * isA  : 0 0 0 0 0 0 0
 *
 * In the level 0, A's 1' is at [0,1), B's 1 is at [0,1).
 * sameA: 1 0 0
 * isA  : 1 0 0 0 0 0 0
 * 
 * Now, 2' in A is at [1,2). It is not found in B or [1,1).
 * 2' < 3, 3 is the smallest one that is greater than 2'.
 * sameA: 1 0 0
 * isA  : 1 0 1 0 0 0 0
 *
 * Now, 3' is found in A and B: or A [2,3) and B [1,2).
 * sameA: 1 0 1
 * isA  : 1 0 1 1 0 0 0
 *
 * In the level 1, group is set when it is found in B.
 * Their group starts at a+a' and ends in b+b'-1.
 * The last one is already set, so I need only unset the first
 * b+b'-2.
 *
 * I basically check whether each node sequence exists in A or B in
 * the prepMerge function.
 */
alder_wordtable_kfmindex_t*
alder_wordtable_kfmmerger_merge(alder_wordtable_kfmmerger_t* o,
                                size_t stepsize)
{
    const size_t a_size = alder_wordtable_kfmindex_size(o->a);
    const size_t b_size = alder_wordtable_kfmindex_size(o->b);
    
    // I could do this after the merge preparation.
    // This may not be efficient because I would double the memory here.
    o->m = alder_wordtable_kfmindex_create(o->order,
                                           a_size + b_size);
    ALDER_RETURN_NULL_IF_NULL(o->m);
    alder_wordtable_kfmindex_setKFMstepsize(o->m, stepsize);
    
    // Prep. for final completing edges.
    for (size_t l = 0; l < o->order; l++) {
        alder_wordtable_kfmmerger_prepareMerge(o, l, 0, 1, 0, 1);
    }
    // Prep. for normal edges.
    alder_wordtable_kfmmerger_prepareMerge(o, o->order, 0, a_size, 0, b_size);
    
    // Do merge operation.
    // Here I need a KFMindex.
    // While I do the merge, I could partially free memory
    // for the two KFMindex objects.
    alder_wordtable_kfmmerger_performMerge(o);
    
#if !defined(NDEBUG)
    alder_log4("Premerged Index (before computeKFMarray):");
    alder_wordtable_kfmindex_print5bit(o->m);
#endif

    
    // Recompute KFMindex.
    alder_wordtable_kfmindex_computeKFMarray0(o->m);
    return o->m;
}

/* Algorithm 7. Prepare a merge.
 * level: 0 ... order or (k-1 or l)
 * aStart/aEnd: range for A
 * bStart/bEnd: range for B
 *
 * The function is called differently for normal vertex sequence and 
 * those final completing vertex seqeunces. 
 *
 * For final completing vertex,
 * alder_wordtable_kfmmerger_prepareMerge(o, l, 0, 1, 0, 1);
 * Here l = 0, 1, ..., order - 1. The order is k-1 or l. l is the order.
 * Because The prevous letter is $, start and end are 0 and 1.
 * Not clear yet why this is the case.
 *
 * For normal vertex,
 * alder_wordtable_kfmmerger_prepareMerge(o, o->order, 0, a_size, 0, b_size);
 * A vertex sequene is constructed backward from the the end by prefixing 
 * one letter at a time. I search each KFMindex from 0 to the end.
 * 
 * This function is called level + 1 time recursively. When level is 1, 
 * I reach the group or k-2 length of sequence, and when level is 0, 
 * I reach the whole word of k-1 length of sequence.
 *
 * As the level goes down to near 0, the range gets shorter. 
 * When two values in a range are the same, I do not have any sequences 
 * with the built sequence upto that point as their suffix.
 * At level 0, difference between two values in a range must be 1.
 */
void
alder_wordtable_kfmmerger_prepareMerge(alder_wordtable_kfmmerger_t* o,
                                       size_t level,
                                       size_t aStart, size_t aEnd,
                                       size_t bStart, size_t bEnd)
{
    // No edges in A are found for merge, so exit.
    if (aStart == aEnd) {
        return;
    }
    // No edges in B are found for merge. Insert edges in A.
    if (bStart == bEnd) {
        // Insert A vertices for which there are no corresponding B vertices
        // aPos = aEnd - 1 ... aStart
        // or aStart ... aEnd - 1
        for (size_t aPos = aStart; aPos < aEnd; aPos++) {
            alder_wordtable_kfmmerger_prepareInsertA(o, level, aPos, bStart);
        }
        return;
    }
    // One edge in A and one edge in B are found.
    if ((aStart + 1 == aEnd) && (bStart + 1 == bEnd)) {
        // Both intervals are single vertices: can simplify computations
        alder_wordtable_kfmmerger_prepareMergeSingles(o,
                                                      level,
                                                      aStart,
                                                      bStart);
        return;
    }
    // Check the group end if level is 1, and go to the next level recursively
    if (level == 1) {
        // Merge vertex groups: both are non-empty
        // for loop over [aEnd + bEnd - 2 ... aStart + bStart]
        for (size_t i = aStart + bStart; i < aEnd + bEnd - 1; i++) {
            // Mark nodes as group interior to avoid copying their group end marks
            alder_wordtable_bigbitarray_set(o->isSameGroup, i);
        }
    }
    // Now, multiple edges in B are found to be merged.
    level--;
    for (int token = 0; token < (int)o->alpha; token++) {
        size_t a_prevPos_start =
        alder_wordtable_kfmindex_prevPos2(o->a, aStart, token);
        
        size_t a_prevPos_end =
        alder_wordtable_kfmindex_prevPos2(o->a, aEnd, token);
        
        size_t b_prevPos_start =
        alder_wordtable_kfmindex_prevPos2(o->b, bStart, token);
        
        size_t b_prevPos_end =
        alder_wordtable_kfmindex_prevPos2(o->b, bEnd, token);
        
        alder_wordtable_kfmmerger_prepareMerge(o, level,
                                               a_prevPos_start, a_prevPos_end,
                                               b_prevPos_start, b_prevPos_end);
    }
}

/* This function is called when no edge in B is found to be merged. 
 * This would make sense because when no edge in B is found start and end 
 * values are the same, so I do not need both of them. I need just one of
 * the two values, which is bPos. 
 * When no edge in B is found to be merge, there may be multiple values in
 * A that are possible to be merged. I iterate over the multiple values,
 * and aPos is one of them.
 *
 * Because no edge in B is found, I need one of the three bit arrays to
 * be updated: isA.
 * sameA is NOT updated because the edge in A is not a duplicate.
 * isSameGroup is NOT updated because the edge group in A will be used.
 *
 * When prefixing a letter, I must use those letters that are in the incoming
 * edge set. Because I use only one value, this is a requirement. If I 
 * have a range, then I could have be able to check whether the backtracked
 * vertex exist or not by checking the start and end values of the range.
 * Since I have only one position in KFMindex, I have to follow the incoming
 * edge only. This is also true for bPos or position in B. Why?
 *
 * NOT CLEAR YET: but this is my best guess. Say, AA < AC. AA is the target,
 * AC is in KFMindex B. The current position is for AA not AAA. Because
 * AA < AC, it must be true that AAA < AAC. So, the position AAC is the 
 * position AAA can be inserted into. This has something to do with the 
 * reason why prePos2 for B must be also called. After all, the KFMindex
 * provoides a ordered vertex set of sequences of length k-1.
 *
 * This function do the merge prep for the case where edges in only A are
 * needed to be inserted. No edges in B are found to be merged.
 * I follow only the inedge with the corresponding token. No token of 
 * inedge means that the node in B is absent. I recursively call 
 * prepareInsertA function for the inedge token.
 */
void
alder_wordtable_kfmmerger_prepareInsertA(alder_wordtable_kfmmerger_t* o,
                                         size_t level,
                                         size_t aPos, size_t bPos)
{
    if (level == 0) {
        // isA[alpha_A + alpha_B] <- true - Algorithm 7
        alder_wordtable_bigbitarray_set(o->isA, aPos + bPos);
        return;
    }
    level--;
    alder_dwordtable_t value = alder_wordtable_kfmindex_get(o->a, aPos);
    for (int token = 0; token < (int)o->alpha; token++) {
        bool a_valueHasInEdge =
        alder_wordtable_kfmindex_valueHasInEdge(o->a, value, token);
        
        if (a_valueHasInEdge) {
            size_t a_prevPos =
            alder_wordtable_kfmindex_prevPos2(o->a, aPos, token);
            
            size_t b_prevPos =
            alder_wordtable_kfmindex_prevPos2(o->b, bPos, token);
            
            alder_wordtable_kfmmerger_prepareInsertA(o, level,
                                                     a_prevPos, b_prevPos);
        }
    }
}

/* This function is called when only one in A and another one in B are found.
 *
 * I could update all of the three bit-arrays when level reaches 0 or 1.
 * isA, and isSameNode when level is 0.
 * isSameGroup when level is 1.
 *
 * I check the 5-bit value to figure out whether the inedge exists. First,
 * I check this for KFMindex A. No such inedge is found: so exit.
 * If there is one, I check it for KFMindex B. 
 * If B does have one, then I call the function again.
 * Otherwise, I call InsertA function because from this point no edge in B
 * is found to be merged.
 *
 * This function do the merge prep for the case where one inedge in A and
 * another in B are found to be merged.
 * I follow the inedge with the corresponding token for each of A and B.
 * If no inedge in B is found, I call perpareInsertA instead.
 */
void
alder_wordtable_kfmmerger_prepareMergeSingles(alder_wordtable_kfmmerger_t* o,
                                              size_t level,
                                              size_t aPos, size_t bPos)
{
    // Set isA and isSameNode when level 0.
    // Set isSameGroup when level 1.
    if (level == 0) {
        alder_wordtable_bigbitarray_set(o->isA, aPos + bPos);
        alder_wordtable_bigbitarray_set(o->isSameNode, aPos);
        return;
    } else if (level == 1) {
        alder_wordtable_bigbitarray_set(o->isSameGroup, aPos + bPos);
    }
    level--;
    alder_dwordtable_t aValue = alder_wordtable_kfmindex_get(o->a, aPos);
    alder_dwordtable_t bValue = alder_wordtable_kfmindex_get(o->b, bPos);
    for (int token = 0; token < (int)o->alpha; token++) {
        bool a_valueHasInEdge =
        alder_wordtable_kfmindex_valueHasInEdge(o->a, aValue, token);
        
        if (a_valueHasInEdge) {
            bool b_valueHasInEdge =
            alder_wordtable_kfmindex_valueHasInEdge(o->b, bValue, token);
            
            size_t a_prevPos =
            alder_wordtable_kfmindex_prevPos2(o->a, aPos, token);
            
            size_t b_prevPos =
            alder_wordtable_kfmindex_prevPos2(o->b, bPos, token);
            
            if (b_valueHasInEdge) {
                alder_wordtable_kfmmerger_prepareMergeSingles(o, level,
                                                              a_prevPos,
                                                              b_prevPos);
            } else {
                alder_wordtable_kfmmerger_prepareInsertA(o, level,
                                                         a_prevPos,
                                                         b_prevPos);
            }
        }
    }
}

/* Algorithm 8 in Rodland13.
 *
 * Merge the nodes from index a and index b using the prepared information
 *
 * isSameGroup_get is true: group end is reset to 0.
 * isSameGroup looks like this when merge is necessary:
 * 1 1 1 1 0
 * Only the last one is set to 0, so that one in the last one keeps its 
 * group end mark.
 *
 * First, if the edge is found in only B, then I copy the 5-bit value to 
 * append it to the new KFMindex.
 * Second, if the edge is found both in A and B, then I union 5-bit values and
 * append it to the new KFMindex.
 * Third, if the edge is found only in A, then I copy the 5-bit value and
 * append it to the new KFMindex.
 *
 * Note that the order of if-else statement is necessary.
 *
 * 
 */
void
alder_wordtable_kfmmerger_performMerge(alder_wordtable_kfmmerger_t* o)
{
    const size_t a_size = alder_wordtable_kfmindex_size(o->a);
    const size_t b_size = alder_wordtable_kfmindex_size(o->b);
    const size_t abSize = a_size + b_size;
    size_t aPos = 0;
    size_t bPos = 0;
#if !defined(NDEBUG)
    alder_log5("isA");
    alder_wordtable_bigbitarray_print(o->isA);
    alder_log5("isSameNode");
    alder_wordtable_bigbitarray_print(o->isSameNode);
    alder_log5("isSameGroup");
    alder_wordtable_bigbitarray_print(o->isSameGroup);
#endif
    while (aPos + bPos < abSize) {
        bool isA_get = alder_wordtable_bigbitarray_get(o->isA, aPos + bPos);
        
        if (!isA_get) {
            // Node is in B, not in A
            bool isSameGroup_get =
            alder_wordtable_bigbitarray_get(o->isSameGroup, aPos + bPos);
            
            alder_dwordtable_t b_getValue =
            alder_wordtable_kfmindex_getValue(o->b, bPos, isSameGroup_get);
            
            alder_wordtable_kfmindex_put(o->m, b_getValue);
            bPos++;
        } else {
            bool isSameNode_get =
            alder_wordtable_bigbitarray_get(o->isSameNode, aPos);
            
            if (isSameNode_get) {
                // Node is in A and B, so the two must be merged
                alder_dwordtable_t value =
                alder_wordtable_kfmindex_getValue(o->a, aPos, true);
                
                aPos++;
                bool isSameGroup_get =
                alder_wordtable_bigbitarray_get(o->isSameGroup, aPos + bPos);
                
                alder_dwordtable_t b_getValue =
                alder_wordtable_kfmindex_getValue(o->b, bPos, isSameGroup_get);
                
                value |= b_getValue;
                bPos++;
                alder_wordtable_kfmindex_put(o->m, value);
            } else {
                // Node is in only A
                assert(aPos < a_size);
                bool isSameGroup_get =
                alder_wordtable_bigbitarray_get(o->isSameGroup, aPos + bPos);
                
                alder_dwordtable_t a_getValue =
                alder_wordtable_kfmindex_getValue(o->a, aPos, isSameGroup_get);
                
                alder_wordtable_kfmindex_put(o->m, a_getValue);
                aPos++;
            }
        }
        // 5-bit array is deleted partially.
        /* To reduce memory overhead in a merge. */
        if (o->deleteSource &&
            (aPos + bPos) % ALDER_WORDTABLE_KFMMERGER_FREESIZE == 0) {
            alder_wordtable_kfmindex_forgetPriorTo(o->a, aPos);
            alder_wordtable_kfmindex_forgetPriorTo(o->b, bPos);
        }
    }
    // FIXME: memory
    // kfmStore is deleted, and 5-bit array is completely deleted as well.
    if (o->deleteSource) {
        alder_wordtable_kfmindex_destroy(o->a);
        alder_wordtable_kfmindex_destroy(o->b);
//        alder_wordtable_kfmindex_reset(o->a);
//        alder_wordtable_kfmindex_reset(o->b);
    }
}

