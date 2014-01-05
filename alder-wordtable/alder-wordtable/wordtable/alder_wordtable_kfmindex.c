/**
 * This file, alder_wordtable_kfmindex.c, is part of alder-wordtable.
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

/* Features of KFMindex.
 *
 * KFMindex variable has two essential variables: kfmStore and inedgedata.
 * inedgedata stores the 5-bit array for the main data, and kfmStore is
 * for the store-rho table. These variables are of the following types:
 * kfmStore -> bitpackedcumulativearray
 * 5-bit array -> bitpackedlist (via inedgedata)
 *
 * NOTE: a merge function is privided in the kfmmerge file.
 *
 * Binary file forma:
 * File header
 * . (2) Two bytes of magic number (0xF6 0xED)
 * . (8) KFMindex
 * . (1) File version
 * . () Checksum - md5sum value
 * . () Software name
 * . () Version of the software
 * . Which chunks are optional
 * . http://www.alder.org/TR/KFM/  location of the spec
 *
 * tIME image last-modification time
 *
 * Record header
 * TLV (tag length value) - 12 bytes of record header.
 * 1 byte = record type
 * 3 byte = reserve
 * 8 byte = record length
 * followed by record conent.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_bitpackedlist.h"
#include "alder_wordtable_inedgedata.h"
#include "alder_wordtable_inedgelist.h"
#include "alder_wordtable_bitpackedcumulativearray.h"
#include "alder_wordtable_io.h"
#include "alder_wordtable_kfmindex.h"

#pragma mark static

static size_t ID = 1;

static size_t
alder_wordtable_kfmindex_maxStepsize(alder_wordtable_kfmindex_t *o);

static size_t
alder_wordtable_kfmindex_kfmIndexPos(alder_wordtable_kfmindex_t *o,
                                     size_t index);

static void
alder_wordtable_kfmindex_init(alder_wordtable_kfmindex_t *o,
                              size_t order);

static void
alder_wordtable_kfmindex_setTokenFlags(alder_wordtable_kfmindex_t *o,
                                       alder_dwordtable_t value,
                                       alder_dwordtable_t *flag);

static void
alder_wordtable_kfmindex_storeIndexInKFMarray(alder_wordtable_kfmindex_t *o,
                                              alder_dwordtable_t *cnt,
                                              size_t index);

static void
alder_wordtable_kfmindex_addFlaggedTokens(alder_wordtable_kfmindex_t *o,
                                          alder_dwordtable_t *cnt,
                                          alder_dwordtable_t *flag);

static size_t
alder_wordtable_kfmindex_prevPosDown(alder_wordtable_kfmindex_t *o,
                                     size_t pos, int token,
                                     size_t initpos, size_t prev);

static size_t
alder_wordtable_kfmindex_prevPosUp(alder_wordtable_kfmindex_t *o,
                                   size_t pos, int token,
                                   size_t initpos, size_t prev);

static size_t
alder_wordtable_kfmindex_nodeGroupStart(alder_wordtable_kfmindex_t *o,
                                        size_t pos);

static size_t
alder_wordtable_kfmindex_kfmNearbyIndex(alder_wordtable_kfmindex_t *o,
                                        size_t pos);

static
size_t
alder_wordtable_kfmindex_kfmStore_get(alder_wordtable_kfmindex_t *o,
                                      size_t pos);

static size_t
alder_wordtable_kfmindex_kfmStore_size(alder_wordtable_kfmindex_t *o);

static size_t size(alder_wordtable_kfmindex_t *o);
static alder_dwordtable_t get(alder_wordtable_kfmindex_t *o, size_t pos);
static void set(alder_wordtable_kfmindex_t *o, size_t pos, alder_dwordtable_t value);

#pragma mark struct

struct alder_wordtable_kfmindex_struct {
    size_t ID;                     /* ID for debug             */
    size_t CALLCOUNT_PREVPOS;      /* for debug                */
    size_t kfmSize;                /* store-rho's size         */
    size_t stepsize;               /* store-rho's step         */
    bool isPruned;
    size_t alpha;                  /* num of letters           */
    size_t order;                  /* l=k-1 in de Bruijn graph */
    alder_dwordtable_t inedgebitmask;
    alder_wordtable_bitpackedcumulativearray_t *kfmStore;
    alder_wordtable_inedgedata_t *inedgedata;
};

/* This function initializes variables in kfmindex.
 */
static void
alder_wordtable_kfmindex_init(alder_wordtable_kfmindex_t *o,
                              size_t order)
{
    o->ID = ID++;
    o->CALLCOUNT_PREVPOS = 0;
    o->inedgedata = NULL;
    o->kfmStore = NULL;
    o->kfmSize = 0;
    o->stepsize = 0;
    o->isPruned = false;
    o->alpha = ALDER_WORDTABLE_TOKENSIZE;
    o->order = order;
}

/* This function is called by the create function.
 * KFMindex's main data or the list of 5-bit values is constructed.
 * The order is the size of vertex sequence. The de Bruijn subgraph's order
 * is order + 1. The edge sequence length is order + 1, or k, and vertex
 * sequence length is order or l (=k-1). The initsize is the size of KFMindex.
 * This size can be lengthened. It does not matter much how large the initsize
 * parameter is. 
 *
 * FIXME: more seemingly important parameter is the size of subarrays in the
 * 5-bit list data. How do I optimize the length? Or, I could use something
 * else.
 */
static void
alder_wordtable_kfmindex_super(alder_wordtable_kfmindex_t *o,
                               size_t order, size_t initsize)
{
    o->inedgedata = alder_wordtable_inedgedata_create(order, initsize);
}

/* This function is called by create2 function.
 * list contains all of the necessary data for creating a KFMindex. 
 * The order and initial size of a KFMindex can be found in the list.
 */
static void
alder_wordtable_kfmindex_super2(alder_wordtable_kfmindex_t *o,
                                alder_wordtable_inedgelist_t *list)
{
    o->inedgedata = alder_wordtable_inedgedata_createWithInEdgeList(list);
}

/* This function creates a blank KFMindex for merge.
 * order: the l value or k-1, where k is the order of a de Bruijn subgraph.
 * The initial size can be any value. Note that the value is used to 
 * finish the memory allocation yet.
 * The memory of 5-bit array is stored in a two-dim array. The initsize is
 * the size of the 1st dimension. The actual subarray is allocated on demand.
 * The initsize can be large, but the actual memory would be tiny.
 *
 * The initsize would be the sum of two size values of KFMindex objects.
 * This function does not call computeKFMarray because I have no KFMindex 
 * to compute yet even at the end of the create function.
 * 
 * Call computeKFMarray afte a call of this function and adding some edges.
 * Before calling computeKFMarray, set the stepsize in store-rho table.
 * This function is called in merging two KFMindex variables.
 * Only after a call of computeKFMarray, can I use the created KFMindex 
 * variable. Do not forget to pruning the created KFMindex.
 */
alder_wordtable_kfmindex_t*
alder_wordtable_kfmindex_create(size_t order, size_t initsize)
{
    alder_wordtable_kfmindex_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_kfmindex_init(o, order);
    /* memory */
    alder_wordtable_kfmindex_super(o, order, initsize);
    if (o->inedgedata == NULL) {
        alder_wordtable_kfmindex_destroy(o);
        return NULL;
    }
    /* set */
    o->inedgebitmask = alder_wordtable_inedgedata_inedgebitmask(o->inedgedata);
    return o;
}

/* This function creates KFMindex using inedgelist.
 * This function calls computeKFMarray to setup KFMindex using the edge list
 * given in the function.
 */
alder_wordtable_kfmindex_t*
alder_wordtable_kfmindex_create2(alder_wordtable_inedgelist_t *list,
                                 size_t stepsize)
{
    alder_wordtable_kfmindex_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    size_t order = alder_wordtable_inedgelist_order(list);
    alder_wordtable_kfmindex_init(o, order);
    /* memory and KFMindex */
    alder_wordtable_kfmindex_super2(o, list);
    if (o->inedgedata == NULL) {
        alder_wordtable_kfmindex_destroy(o);
        return NULL;
    }
    /* set */
    o->inedgebitmask = alder_wordtable_inedgedata_inedgebitmask(o->inedgedata);
    o->stepsize = stepsize;
    alder_wordtable_kfmindex_computeKFMarray0(o);
    return o;
}

/* This function creates KFMindex using inedgelist with a default stepsize.
 *
 */
alder_wordtable_kfmindex_t*
alder_wordtable_kfmindex_create1(alder_wordtable_inedgelist_t *list)
{
    alder_wordtable_kfmindex_t *o =
    alder_wordtable_kfmindex_create2(list,
                                     ALDER_WORDTABLE_KFMDEFAULTS_STEPSIZE);
    return o;
}

/* This function frees the inedge index of 5-bit array, 
 * and the store-rho table.
 */
void
alder_wordtable_kfmindex_destroy(alder_wordtable_kfmindex_t *o)
{
    if (o != NULL) {
        alder_wordtable_bitpackedcumulativearray_destroy(o->kfmStore);
        alder_wordtable_inedgedata_destroy(o->inedgedata);
        XFREE(o);
    }
}

void
alder_wordtable_kfmindex_setKFMstepsize(alder_wordtable_kfmindex_t *o,
                                        size_t stepsize)
{
    o->stepsize = stepsize;
}

//size_t
//alder_wordtable_kfmindex_getKFMsize(alder_wordtable_kfmindex_t *o)
//{
//    return alder_wordtable_bitpackedcumulativearray_size(o->kfmStore);
//}

bool
alder_wordtable_kfmindex_isPruned(alder_wordtable_kfmindex_t *o)
{
    return o->isPruned;
}

///* This function frees memory allocated for the FKMindex when it is 
// * not needed anymore during the merge. This deletes the 5-bit array of
// * the inedge index, and the bit array of the rho table.
// */
//void
//alder_wordtable_kfmindex_reset(alder_wordtable_kfmindex_t *o)
//{
//    alder_wordtable_bitpackedlist_removeAll(o->inedgedata->bitpackedlist);
//    alder_wordtable_bitpackedcumulativearray_destroy(o->kfmStore);
//    o->kfmStore = NULL;
//    o->isPruned = false;
//}

#pragma mark computeKFMarray

/* This function computes the rho table with a size of a store-rho. 
 * If the stepsize is 1, then I store all of the rho table.
 * I could try this of stepsize of 1 for testing purpose.
 */
int
alder_wordtable_kfmindex_computeKFMarray0(alder_wordtable_kfmindex_t *o)
{
    size_t kfmsize = 1 + (size(o) - 1) / o->stepsize;
    int s = alder_wordtable_kfmindex_computeKFMarray(o, kfmsize);
    return s;
}

/* This function constructs the table of rho in Rodland13.
 *
 * indexsize is the size of store-rho table. Here, the size is the number of
 * rows, not the total size of the cumulative array.
 *
 * Consider the examplary figure 1 in Rodland13.
 * 1. Set the first value to 1 at the index 0.
 * 2. Setup two arrays: cnt[4] for counts of A,C,G,T, and
 *                      flag[4] for counts (but 0 or 1) of A,C,G,T.
 *                      Initially, these are set to 0.
 *    I start at position 0.
 * 3. Get a 5-bit value at a position in KFMindex using get function.
 * 4. Decode the value to set the flag of 4 or A,C,G,T.
 *    Set flag array using the edge list. For example, the first edge
 *    in the example is xxxT or 0001 for flag.
 * 5. Sum the token count when I am at a group end position.
 *    Or, check if the edge is the group end. If so, I add flag to cnt.
 * 6. Then, check if I am at the store-rho's position. If so, save cnt in
 *    kfmStore. Note that the cumulative bit array size is 4*kfmSize + 1.
 *    The 1 is for the first element at position 0. The value is also 1.
 *    When saving cnt in kfmStore, I save four values periodically by the
 *    value of kfmSize. index increases 0, 1, 2, 3. The last one is the value
 *    of kfmSize.
 *    Store the counts at the store-rho table.
 *    Note that I store the increment only, and later call function
 *    alder_wordtable_bitpackedcumulativearray_cumulate
 *    to complete the rho table.
 *
 *    For example, consider a case where:
 *    kfmSize = 3
 *    index = 1
 *    Then, I save each of the four values in cnt at
 *    1, 4, 7, 10.
 *    Next time, I save cnt at
 *    2, 5, 8, 11.
 *    In the last time, I save cnt at
 *    3, 6, 9, 12.
 *    This would complete the construction of the store-rho table.
 * 7. So, ouf of the for-loop, index must be equal to kfmSize + 1.
 *    index == kfmSize + 1.
 * 8. I am ready to complete the store-rho table.
 *    Call alder_wordtable_bitpackedcumulativearray_cumulate
 *
 * returns 0 on success, 1 otherwise.
 */
int
alder_wordtable_kfmindex_computeKFMarray(alder_wordtable_kfmindex_t *o,
                                         size_t indexsize)
{
    int status = ALDER_WORDTABLE_SUCCESS;
    o->kfmSize = indexsize > size(o) ? size(o) : indexsize;
    if (o->kfmStore != NULL) {
        alder_wordtable_bitpackedcumulativearray_destroy(o->kfmStore);
        o->kfmStore = NULL;
    }
    size_t store_rho_size = o->kfmSize * o->alpha + 1;
    size_t stepsize = alder_wordtable_kfmindex_maxStepsize(o);
    o->kfmStore =
    alder_wordtable_bitpackedcumulativearray_create(store_rho_size,stepsize);
    if (o->kfmStore == NULL) {
        return ALDER_WORDTABLE_FAIL;
    }
    
    size_t alpha = o->alpha;
    alder_dwordtable_t *cnt = malloc(sizeof(*cnt) * alpha);
    alder_dwordtable_t *flag = malloc(sizeof(*flag) * alpha);
    memset(cnt, 0, sizeof(*cnt) * alpha);
    memset(flag, 0, sizeof(*flag) * alpha);
    
    size_t index = 0;
    alder_wordtable_bitpackedcumulativearray_set(o->kfmStore, index, 1);
    index++;
    size_t nextStorePos = alder_wordtable_kfmindex_kfmIndexPos(o, index);
    for (size_t i = 0; i < size(o); i++) {
        alder_dwordtable_t value = get(o, i);
        alder_wordtable_kfmindex_setTokenFlags(o, value, flag);
        if (alder_wordtable_inedgedata_valueIsGroupEnd(o->inedgedata, value)) {
            alder_wordtable_kfmindex_addFlaggedTokens(o, cnt, flag);
        }
#if !defined(NDEBUG)
        alder_log5("cnt[%d] %llu %llu %llu %llu", i,
                   cnt[0], cnt[1], cnt[2], cnt[3]);
#endif
        if (i + 1 == nextStorePos) {
#if !defined(NDEBUG)
        alder_log5("added index[%zu] cnt[%d] %llu %llu %llu %llu", index, i,
                   cnt[0], cnt[1], cnt[2], cnt[3]);
#endif
            alder_wordtable_kfmindex_storeIndexInKFMarray(o, cnt, index);
            index++;
            nextStorePos = alder_wordtable_kfmindex_kfmIndexPos(o, index);
        }
    }
    assert(index == o->kfmSize + 1);
#if !defined(NDEBUG)
    alder_log5("before cumulate");
    for (size_t i = 0; i < o->kfmSize * alpha + 1; i++) {
        size_t av =
        alder_wordtable_bitpackedcumulativearray_get(o->kfmStore, i);
        alder_log5("[%03zu] %zu", i, av);
    }
#endif
    alder_wordtable_bitpackedcumulativearray_cumulate(o->kfmStore);
    
    size_t lastPrev =
    alder_wordtable_bitpackedcumulativearray_get(o->kfmStore,
                                                 o->kfmSize * alpha);
#if !defined(NDEBUG)
    alder_log5("after cumulate");
    for (size_t i = 0; i < o->kfmSize * alpha + 1; i++) {
        size_t av =
        alder_wordtable_bitpackedcumulativearray_get(o->kfmStore, i);
        alder_log5("[%03zu] %zu", i, av);
    }
    alder_log5("size(o) = %zu", size(o));
    alder_log5("lastPrev= %zu", lastPrev);
#endif
    
    assert(size(o) == lastPrev);
    if (size(o) != lastPrev) {
        status = ALDER_WORDTABLE_FAIL;
    }
    XFREE(cnt);
    XFREE(flag);
    return status;
}

/* This function converts a 5-bit value to an array of 0 or 1.
 * e.g.,
 * 00110 -> flag[] = {1,1,0,0}.
 * Note that I do not reset values to 0 when bit is 0.
 * This reset is done in 
 * addFlaggedTokens function or
 * when I hit a group end.
 *
 * See alder_wordtable_kfmindex_computeKFMarray, which calls this function.
 */
void
alder_wordtable_kfmindex_setTokenFlags(alder_wordtable_kfmindex_t *o,
                                       alder_dwordtable_t value,
                                       alder_dwordtable_t *flag)
{
    size_t alpha = o->alpha;
    alder_dwordtable_t x2 = 2;
    for (size_t i = 0; i < alpha; i++) {
        if ((value & (x2 << i)) != 0) {
            flag[i] = 1;
        }
    }
}

/* This function stores the counts at the rho_store.
 * I reset cnt array for a next store-rho.
 *
 * See alder_wordtable_kfmindex_computeKFMarray, which calls this function.
 */
void
alder_wordtable_kfmindex_storeIndexInKFMarray(alder_wordtable_kfmindex_t *o,
                                              alder_dwordtable_t *cnt,
                                              size_t index)
{
    size_t alpha = o->alpha;
    for (size_t i = 0; i < alpha; i++) {
        size_t x = index + i * o->kfmSize;
        alder_dwordtable_t y = cnt[i];
        alder_wordtable_bitpackedcumulativearray_set(o->kfmStore, x, y);
    }
    memset(cnt, 0, sizeof(*cnt) * alpha);
}

/* Add flags to count, and reset flag array.
 *
 * See alder_wordtable_kfmindex_computeKFMarray, which calls this function.
 */
void
alder_wordtable_kfmindex_addFlaggedTokens(alder_wordtable_kfmindex_t *o,
                                          alder_dwordtable_t *cnt,
                                          alder_dwordtable_t *flag)
{
    size_t alpha = o->alpha;
    for (size_t i = 0; i < alpha; i++) {
        cnt[i] += flag[i];
    }
    memset(flag, 0, sizeof(*flag) * alpha);
}

#pragma mark prevPos

/* Algorithm 1 in Rodland13.
 *
 * This function finds the previous position of a position in the FKMindex.
 * There are n x 4 elements in the rho table. The tokenpos value is a value
 * less than n x 4. This value represents position in the KFMindex and token.
 * tokenpos % n tells us the position in the KFMindex.
 * tokenpos / n tells us which letter or token we are looking for.
 *
 * This function rho provides a way of backtracking the KFMindex. 
 * Consider the following table given by Figure 1 in Rodland13.
 * There is an edge:
 * AGC (3) -> GCT (9)
 * To backtrack from the sink vertex to the source vertex, I can use the 
 * KFMindex. GCT has an edge with A, which means there is a node sequence of
 * AGC. I can find the location of AGC in the KFMindex using the rho-table.
 * The position is engraved on the rho-table; the first value of the 9th
 * row in the rho table is 3, which is the row for AGC.
 * But, the rho-table has a row of [3,8,10,15] at the position 9. Why do I 
 * need these other values? The first value 3 has some meaning in the 
 * de Bruijn subgraph, but the other values do not. 
 * The meaning of the other values is that if there was a node sequence by
 * backtracking from the node 9, the node is less than the node at the pos
 * in the rho-table. Say, from GCT, backtracking C, the backtracked node 
 * sequence is CGC, which does not exist in the de Bruijn subgraph, but
 * CGC < CTA (8). Not just CGC is less than CTA (8), CTA is the least one
 * among the node sequences that is greater than CGC. 
 *
 * These two usages of the rho or prevPos are both important although the 
 * latter one does not mean much in the de Bruijn subgraph. The latter usage
 * will be important when searching a node sequence and/or merging two 
 * KFMindex variables.
 *
 *     Edge   V   Rho-Table
 * [00]   T * $$$ 1	6	9	11
 * [01]   T   ACA 1	6	9	12
 * [02] C   * ACT 1	6	9	12
 * [03] C     AGC 1	7	9	13
 * [04] C T * AGT 1	7	9	13
 * [05]   T * AT$ 1	8	9	14
 * [06]       CAC 1	8	9	15
 * [07]A    * CAG 1	8	9	15
 * [08]A G  * CTA 2	8	9	15
 * [09]A    * GCT 3	8	10	15
 * [10]A    * GTA 4	8	10	15
 * [11]A    * T$$ 5	8	10	15
 * [12]  G    TAC 6	8	10	15
 * [13] C     TAG 6	8	10	15
 * [14]  G  * TAT 6	8	10	15
 *
 */
size_t
alder_wordtable_kfmindex_prevPos(alder_wordtable_kfmindex_t *o,
                                 size_t tokenpos)
{
    size_t pos = tokenpos % size(o);
    size_t token = tokenpos / size(o);
    return alder_wordtable_kfmindex_prevPos2(o, pos, (int)token);
}

/* This function finds the previous position at pos for token.
 * pos is the position in KFMindex: 0 .. n-1.
 * token is a letter: 0,1,2,3.
 *
 * See alder_wordtable_kfmindex_prevPos
 */
size_t
alder_wordtable_kfmindex_prevPos2(alder_wordtable_kfmindex_t *o,
                                  size_t pos, int token)
{
    // Find the index of the nearest KFMindex storage.
    size_t index = alder_wordtable_kfmindex_kfmNearbyIndex(o, pos);
    // Find the position of the stored KFMindex sotrage.
    size_t initpos = alder_wordtable_kfmindex_kfmIndexPos(o, index);
    // Get the stored rho.
    size_t kappaPos = index + token * o->kfmSize; // o->kfmSize is zeta.
    size_t prestored = alder_wordtable_kfmindex_kfmStore_get(o,kappaPos);
    if (pos < initpos) {
        size_t x = alder_wordtable_kfmindex_prevPosDown(o, pos, token,
                                                        initpos, prestored);
        return x;
    } else if (pos > initpos) {
        size_t x = alder_wordtable_kfmindex_prevPosUp(o, pos, token,
                                                      initpos, prestored);
        return x;
    } else {
        return prestored;
    }
}

/* This function finds the rho at pos
 * when initpos is further down below the pos. 
 * The stored value at initpos is prev.
 * I move up from initpos.
 */
size_t
alder_wordtable_kfmindex_prevPosDown(alder_wordtable_kfmindex_t *o,
                                     size_t pos, int token,
                                     size_t initpos, size_t prev)
{
    // Find the first node in the current group.
    initpos = alder_wordtable_kfmindex_nodeGroupStart(o, initpos);
    if (initpos <= pos) {
        return prev;
    }
    alder_dwordtable_t flags = 0;
    assert(initpos != 0);
    initpos--;
    while (true) {
        // Get the edge and group end at a position.
        alder_dwordtable_t value = get(o, initpos);
        bool y = alder_wordtable_inedgedata_valueIsGroupEnd(o->inedgedata,
                                                            value);
        if (y) {
            bool z = alder_wordtable_inedgedata_valueHasInEdge(o->inedgedata,
                                                               flags, token);
            if (z) {
                prev--;
            }
            if (initpos < pos) {
                return prev;
            }
            flags = 0;
        }
        flags |= value;
        
        if (initpos == 0) {
            break;
        }
        initpos--;
    }
    return prev;
}

/* This function finds the rho at pos
 * when initpos is further up above the pos.
 * The stored value at initpos is prev.
 * I move down from at initpos.
 */
size_t
alder_wordtable_kfmindex_prevPosUp(alder_wordtable_kfmindex_t *o,
                                     size_t pos, int token,
                                     size_t initpos, size_t prev)
{
    initpos = alder_wordtable_kfmindex_nodeGroupStart(o, initpos);
    alder_dwordtable_t flags = 0;
    while (initpos < pos) {
        alder_dwordtable_t value = get(o,initpos);
        initpos++;
        flags |= value;
        bool y = alder_wordtable_inedgedata_valueIsGroupEnd(o->inedgedata,
                                                            flags);
        if (y) {
            bool z = alder_wordtable_inedgedata_valueHasInEdge(o->inedgedata,
                                                               flags, token);
            if (z) {
                prev++;
            }
            flags = 0;
        }
    }
    return prev;
}

/* This function finds the start node in the group that the node at pos
 * belongs to.
 */
size_t
alder_wordtable_kfmindex_nodeGroupStart(alder_wordtable_kfmindex_t *o,
                                        size_t pos)
{
    while (pos > 0) {
        pos--;
        size_t x = get(o,pos);
        // alder_wordtable_bitpackedlist_get(o->inedgedata->bitpackedlist, pos);
        bool y = alder_wordtable_inedgedata_valueIsGroupEnd(o->inedgedata, x);
        if (y) {
            return pos + 1;
        }
    }
    return pos;
}

/* This function returns the KFMindex position in the KFMindex size.
 * The index is the position at the stored-rho table.
 * WARN: This function must go along with kfmIndexPos.
 */
size_t
alder_wordtable_kfmindex_kfmIndexPos(alder_wordtable_kfmindex_t *o,
                                     size_t index)
{
    size_t pos = index;
    pos *= size(o);
    pos /= o->kfmSize;
    return pos;
}

/* This function computes the maximum step size given the total size
 * of KFMindex and the stored KFMsize.
 */
size_t
alder_wordtable_kfmindex_maxStepsize(alder_wordtable_kfmindex_t *o)
{
    
    return 1 + (size(o) - 1) / o->kfmSize;
}

/* WARN: This function must go along with kfmIndexPos.
 *       Ignore the following info.
 *
 * This tells us the storage index of the rho.
 * An = n x k (n = {0..})
 * Find a non-negative value of n, whose An is nearest to a number.
 * See a series of numbers:
 * 0 5 10 15 20 25 30 ...
 * where k = 5.
 * Given a number 16, find n where An is nearest to 16.
 * 15 is the nearest one, and its n is 3.
 * 
 * Given number v, (v + k/2)/k is the n.
 * e.g.,
 * 16 + 5/2 -> 18/5 = 3.
 *
 */
size_t
alder_wordtable_kfmindex_kfmNearbyIndex(alder_wordtable_kfmindex_t *o,
                                        size_t pos)
{
    size_t index = pos;
    index *= o->kfmSize;
    index += size(o) / 2;
    index /= size(o);
    return index;
}

#pragma mark STOPPED HERE


#pragma mark VERTEX

/* 
 * This function computes the rho^{inv}(i). Let's call this rhoinv(i).
 * The argument i in rhoinv(i) is posvalue.
 *
 * rhoinv(i) finds another value v = j + token x KFMindex size, which 
 *           essentially finds j and token, such that
 *           rho(a,j) = i, and rho(a,j+1) = i + 1.
 *           I need to find another rho value v (or essentially j), at which
 *           the rho value changes from posvalue to posvalue + 1. So, the
 *           position j must be at the group end position.
 *
 * From Rodland13 (upper right-hand side at page 8), ---
 * The pair (a,j) can be found through a binary search. However, since the 
 * computation of ρ is done in a stepwise manner starting at one of the stored
 * values, a binary search should be performed on the stored values to identify 
 * the interval that contains the solution, and the stepwise procedure then
 * followed until the solution is found.
 *
 * rhoinv(i) = (a,j) is that a is the first letter of vi, while j is the last 
 * vertex in the vertex group with (k−2)-prefix equal to the (k−2)-suffix of
 * v_i: i.e. there is an edge from vi to a vertex in the same vertex group 
 * as v_j. However, since the vertices in the same vertex group only differ by
 * the last letter, and we don’t have to determine this letter, we do not have
 * to determine which vertex (or ver- tices) in this vertex group has an edge 
 * from v_i. By iterating this procedure k − 1 times as in Algorithm 5, we can
 * find the string v_i.
 * ---
 *
 * 1. Find min index in kfmStore.
 * 2. posvalue is below midx.
 * 3. Move min down upto posvalue.
 *
 * FIXME: not understood yet.
 *
 * min, max, mid - index value in store-rho.
 *
 * Let's use this function to find a vertex at pos 12.
 * pos 12 -> pos 2 (because 12 at pos 2 and 13 at pos 3 (or 2+1).
 * pos 2 -> pos 8 (because 2 at pos 8 and 3 at pos 8 + 1 (or 9).
 * pos 8 -> pos 14 (because 8 at pos 14, and 9 at pos 14 + 1.
 * Each trace leaves a letter: T (12), A (2), C (8).
 * TAC at pos 12.
 */
size_t
alder_wordtable_kfmindex_kfmSolve(alder_wordtable_kfmindex_t *o,
                                  size_t posvalue)
{
    size_t x = alder_wordtable_kfmindex_kfmStore_get(o,0);
    if (posvalue < x) {
        assert(posvalue == 0);
        return 0;
//        return -size(o);
    }
    // Find position with kfm[pos]=kfm[pos+1]-1=value
    size_t min = 0;
    size_t max = alder_wordtable_kfmindex_kfmStore_size(o) - 1;
    //size_t max = o->kfmStore->size - 1;
//    while (max - min > 1) {
    while (max > min + 1) {
        size_t mid = (min + max) / 2;
        size_t x = alder_wordtable_kfmindex_kfmStore_get(o, mid);
        if (x > posvalue) {
            max = mid;
        } else {
            min = mid;
        }
    }
    // Search from min to find where value is first attained
    size_t minposvalue = alder_wordtable_kfmindex_kfmStore_get(o, min);
    assert(posvalue >= minposvalue);
    
    size_t pos = alder_wordtable_kfmindex_kfmIndexPos(o, min);
    int token = (int) (pos / size(o));
    pos = alder_wordtable_kfmindex_nodeGroupStart(o, pos % size(o));
    
    alder_dwordtable_t bits = 0;
    while (true) {
        alder_dwordtable_t x = alder_wordtable_kfmindex_get(o, pos);
        bits |= x;
        bool isGroupEnd =
        alder_wordtable_inedgedata_valueIsGroupEnd(o->inedgedata, bits);
       
        if (isGroupEnd) {
            bool hasInEdge =
            alder_wordtable_inedgedata_valueHasInEdge(o->inedgedata,
                                                      bits, token);
            if (hasInEdge) {
                if (posvalue == minposvalue) {
                    return pos + size(o) * token;
                }
                minposvalue++;
            }
            bits = 0;
        }
        pos++;
    }
}

#pragma mark find

/* This function find the location or index of string pointed by s. The string
 * must be of size being at least l or k-1.
 * 
 * Algorithm 2
 */
size_t
alder_wordtable_kfmindex_findLstring(alder_wordtable_kfmindex_t *o,
                                     const char *s)
{
    size_t alpha = alder_wordtable_kfmindex_find_alpha(o,s);
    size_t beta = alder_wordtable_kfmindex_find_beta(o,s);
    if (alpha == beta - 1) {
        return alpha;
    } else {
        return size(o);
    }
}

size_t
alder_wordtable_kfmindex_find_alpha(alder_wordtable_kfmindex_t *o,
                                    const char *s)
{
    size_t pos = 0;
    for (size_t i = 0; i < o->order; i++) {
        size_t j = o->order - 1 - i;
        int token = alder_wordtable_tokenset_token_of(*(s+j));
        pos = alder_wordtable_kfmindex_prevPos2(o, pos, token);
    }
    return pos;
}

size_t
alder_wordtable_kfmindex_find_beta(alder_wordtable_kfmindex_t *o,
                                   const char *s)
{
    size_t pos = size(o);
    for (size_t i = 0; i < o->order; i++) {
        size_t j = o->order - 1 - i;
        int token = alder_wordtable_tokenset_token_of(*(s+j));
        pos = alder_wordtable_kfmindex_prevPos2(o, pos, token);
    }
    return pos;
}


#pragma mark print

/* This function prints the content of the KFMindex in the variable.
 */
void
alder_wordtable_kfmindex_print(alder_wordtable_kfmindex_t *o)
{
    alder_log4("KFMindex [%zu]", o->ID);
    size_t s = alder_wordtable_kfmindex_size(o);
    for (size_t i = 0; i < s; i++) {
        bstring bedge = alder_wordtable_kfmindex_toString(o, i);
        alder_log4("[%02zu]%s", i, bdata(bedge));
        bdestroy(bedge);
    }
}
void
alder_wordtable_kfmindex_print5bit(alder_wordtable_kfmindex_t *o)
{
    alder_log4("KFMindex [%zu]", o->ID);
    size_t s = alder_wordtable_kfmindex_size(o);
    for (size_t i = 0; i < s; i++) {
        bstring bedge = alder_wordtable_inedgedata_nodeDataString(o->inedgedata, i);
        alder_log4("[%02zu]%s", i, bdata(bedge));
        bdestroy(bedge);
    }
}


bstring
alder_wordtable_kfmindex_toString(alder_wordtable_kfmindex_t *o,
                                  size_t pos)
{
    bstring bindexString =
    alder_wordtable_inedgedata_nodeDataString(o->inedgedata, pos);
    
    bstring bnodeString = alder_wordtable_kfmindex_nodeString(o,pos);
    
    bconcat(bindexString, bnodeString);
    
    size_t rho[4];
    for (int token = 0; token < 4; token++) {
        rho[token] = alder_wordtable_kfmindex_prevPos2(o,pos,token);
    }
    bstring brhoString = bformat(" %zu\t%zu\t%zu\t%zu", rho[0], rho[1], rho[2], rho[3]);
    bconcat(bindexString, brhoString);
    
    bdestroy(bnodeString);
    bdestroy(brhoString);
    
    return bindexString;
}

/* This function implements Algorithm 5 in Rodland13.
 */
bstring
alder_wordtable_kfmindex_nodeString(alder_wordtable_kfmindex_t *o,
                                    size_t pos)
{
    size_t s = size(o);
    size_t p = pos;
    int k = (int)o->order;
    bstring bnodeString = bfromcstralloc(k + 3, " ");
    for (int i = 0; i < k; i++) {
        size_t x = alder_wordtable_kfmindex_kfmSolve(o, p);
        int token = x == 0 ? 4 : (int)(x / s);
        p = x == 0 ? 0 : x % s;
        char c = alder_wordtable_tokenset_char_of(token);
        bconchar(bnodeString, c);
    }
    return bnodeString;
}

#pragma mark save and load

// header
// Borrowed from PNG header
// Version number - single byte
// Use fopen, fread, fwrite, getc, putc
// Don't forget to check feof() and ferror() (or equivalents)
//   after doing a series of getc() or putc() calls.
// If you're writing a file that will be used predominantly
//   on 80x86 machines, use little-endian ordering for all values.
// Checksums on File Data
//   The best place for a CRC is actually at the end of a chunk of data.
// Having a full file length in the header.
//   Add an explicit end marker.  Seek to the end of the file and read it.
// Document what every byte means:
//   e.g.,
//   All values little-endian
//   +00 2B Magic number (0xd5aa)
//   +02 1B Version number (currently 1)
//   +03 1B (pad byte)
//   +04 4B CRC-32
//   +08 4B Length of data
//   +0c 2B Offset to start of data
//   +xx [data]
// Example
//   The identification string is followed by a series of "chunks", each of which is headed by the chunk type and chunk length (so unrecognized chunks can be skipped), and followed by a 32-bit CRC.
// Instead of changing a version when data formats change, the chunk type is altered.  This allows adding chunks that older applications can ignore.  This is similar in concept to the "Apple Preferred Format", used on the Apple IIgs (and elsewhere?) for image formats.

/* This function saves the KFMindex variable in a file.
 * See alder_wordtable_io.c for detail.
 * 
 * The order of chunks in the KFMindex binary file:
 * header, IHDR, IKFM, IRHO, IEND.
 * IKFM is the chunk for inedgedata, and IRHO is for bitpackedcumulativearray.
 *
 * 1. open a file for reading
 * 2. write the header
 * 3. write IHDR chunk
 * 4. write IKFM chunk
 * 5. write IRHO chunk
 * 6. write IEND chunk
 */
int
alder_wordtable_kfmindex_save(alder_wordtable_kfmindex_t *o,
                              const char *fn)
{
    int r = ALDER_WORDTABLE_SUCCESS;
    // 1
    FILE *fp = fopen(fn, "wb");
    if (fp == NULL) return ALDER_WORDTABLE_FAIL;
    // 2
    r = alder_wordtable_io_write_header(fp);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 3
    r = alder_wordtable_io_write_IHDR(fp,
                                      o->order,
                                      size(o),
                                      o->kfmSize,
                                      o->stepsize);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 4
    r = alder_wordtable_inedgedata_save(o->inedgedata, fp);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 5
    r = alder_wordtable_bitpackedcumulativearray_save(o->kfmStore, fp);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 6
    r = alder_wordtable_io_write_IEND(fp);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    
    XFCLOSE(fp);
    return ALDER_STATUS_SUCCESS;
}

/* This function initializes variables in kfmindex.
 */
static void
alder_wordtable_kfmindex_load_init(alder_wordtable_kfmindex_t *o,
                                   size_t order,
                                   size_t kfmSize,
                                   size_t stepsize)
{
    alder_wordtable_kfmindex_init(o, order);
    o->kfmSize = kfmSize;
    o->stepsize = stepsize;
}

/* This function loads a KFMindex from a file.
 *
 * 1. open a file.
 * 2. read the header.
 * 3. read IHDR chunk, which would include essential variables for a KFMindex.
 * 4. allocate the memory for a KFMindex, and initialize it.
 * 5. read the main index table, which is in inedgedata; IKFM chunk.
 * 6. read the rho table; IRHO chunk.
 * 7. read the end of the file.
 */
alder_wordtable_kfmindex_t *
alder_wordtable_kfmindex_load(const char *fn)
{
    int r = ALDER_WORDTABLE_SUCCESS;
    // 1
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        return NULL;
    }
    // 2
    r = alder_wordtable_io_read_header(fp);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    // 3
    size_t kfm_order = 0;
    size_t kfm_size = 0;
    size_t kfm_rhosize = 0;
    size_t kfm_stepsize = 0;
    r = alder_wordtable_io_read_IHDR(fp,
                                     &kfm_order,
                                     &kfm_size,
                                     &kfm_rhosize,
                                     &kfm_stepsize);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    // 4
    alder_wordtable_kfmindex_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        XFCLOSE(fp);
        return NULL;
    }
    alder_wordtable_kfmindex_load_init(o, kfm_order, kfm_rhosize, kfm_stepsize);
    // 5
    o->inedgedata = alder_wordtable_inedgedata_load(fp, o->order);
    if (o->inedgedata == NULL) {
        alder_wordtable_kfmindex_destroy(o);
        XFCLOSE(fp);
        return NULL;
    }
    // 6
    o->kfmStore = alder_wordtable_bitpackedcumulativearray_load(fp);
    if (o->kfmStore == NULL) {
        alder_wordtable_kfmindex_destroy(o);
        XFCLOSE(fp);
        return NULL;
    }
    // 7
    r = alder_wordtable_io_read_IEND(fp);
    if (r != ALDER_WORDTABLE_SUCCESS) {
        alder_wordtable_kfmindex_destroy(o);
        XFCLOSE(fp);
        return NULL;
    }
        
    XFCLOSE(fp);
    return o;
}

#pragma mark prune

/* Prune the index of unnecessary final completing vertices.
 *
 */
size_t
alder_wordtable_kfmindex_pruneFinalCompletions(alder_wordtable_kfmindex_t *o)
{
    size_t tokensize = alder_wordtable_tokenset_size();
    alder_wordtable_unusedfinals_t *pruner =
    alder_wordtable_unusedfinals_create(tokensize);
    
    // Algorithm 9
    alder_wordtable_unusedfinals_removeUnneededEdges(o,pruner);
    
    alder_wordtable_positionlist_t *unusedNodes =
    alder_wordtable_unusedfinals_getUnusedNodes(pruner);
    
    // Sort the positions of nodes and remove them.
    alder_wordtable_positionlist_sort(unusedNodes);
    alder_wordtable_kfmindex_remove(o, unusedNodes);
    
    // Recompute KFMarray.
    // FIXME: Free KFMstore first.
    alder_wordtable_kfmindex_computeKFMarray0(o);
    o->isPruned = true;
    
    // Free the pruner.
    alder_wordtable_unusedfinals_destroy(pruner);
    return alder_wordtable_positionlist_size(unusedNodes);
}

#pragma mark kfminterval


bool
alder_wordtable_kfmindex_contains(alder_wordtable_kfmindex_t *o,
                                  alder_wordtable_kfmindex_t *oi)
{

    return true;
}

#pragma mark prune

alder_wordtable_unusedfinals_t*
alder_wordtable_unusedfinals_create(size_t tokensize)
{
    alder_wordtable_unusedfinals_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->tokensize = tokensize;
    o->unusedNodes = NULL;
    o->changedValues = NULL;
    
    o->unusedNodes = alder_wordtable_positionlist_create();
    o->changedValues = alder_wordtable_positionvaluelist_create();
    if (o->unusedNodes == NULL || o->changedValues == NULL) {
        alder_wordtable_unusedfinals_destroy(o);
        return NULL;
    }
    return o;
}

void
alder_wordtable_unusedfinals_destroy(alder_wordtable_unusedfinals_t *o)
{
    if (o != NULL) {
        alder_wordtable_positionlist_destroy(o->unusedNodes);
        alder_wordtable_positionvaluelist_destroy(o->changedValues);
        XFREE(o);
    }
}

void
alder_wordtable_unusedfinals_removeUnneededEdges
(alder_wordtable_kfmindex_t *o,
 alder_wordtable_unusedfinals_t *o1)
{
    alder_wordtable_unusedfinals_checkIfUnused(o,o1,
                                               o->order,
                                               0);
    
    size_t changedValues_size =
    alder_wordtable_positionvaluelist_size(o1->changedValues);
    for (size_t k = 0; k < changedValues_size; k++) {  // for (i,E_new)
        size_t j = changedValues_size - k - 1;
        
        // i in (i,E_new)
        size_t i =
        alder_wordtable_positionvaluelist_getPosition(o1->changedValues, j);
        // E_new in (i,E_new)
        size_t E_new =
        alder_wordtable_positionvaluelist_getValue(o1->changedValues, j);
        // E_i <- E_new
        alder_wordtable_inedgedata_set(o->inedgedata, i, E_new);
    }
}

/* Algorithm 10. Prune ...
 * CheckUnused(l,i).
 * l = level
 * i = pos or node_i or node_pos.
 *
 */
bool
alder_wordtable_unusedfinals_checkIfUnused
(alder_wordtable_kfmindex_t *o,
 alder_wordtable_unusedfinals_t *o1, size_t level, size_t pos)
{
    const alder_dwordtable_t x1 = 1;
    // value => E_i
    alder_dwordtable_t value = alder_wordtable_kfmindex_get(o, pos);
    if (level > 1) {
        alder_dwordtable_t unsetValue = 0;
        alder_dwordtable_t edgeflag = 1;
        for (int token = 0; token < o1->tokensize; token++) {
            edgeflag <<= 1;
            if ((value & edgeflag) != 0) {
                size_t j = alder_wordtable_kfmindex_prevPos2(o,
                                                             pos,
                                                             token);
                bool unused =
                alder_wordtable_unusedfinals_checkIfUnused(o,o1,level-1,j);
                
                if (unused) {
                    alder_wordtable_positionlist_add(o1->unusedNodes, j);
                    unsetValue |= edgeflag;
                }
            }
        }
        // unsetValue => E_del
        if (unsetValue != 0) {        // E_del is not empty.
            value -= unsetValue;      // E_i \ E_del
            // E_prune = (i,E_i\E_del).
            alder_wordtable_positionvaluelist_add(o1->changedValues,
                                                  pos, value);
        }
    } else if ((value & x1) == 0) {   // if not f_i
        alder_dwordtable_t used = 0;  // used => E_del
        alder_dwordtable_t j = pos;
        while ((used & x1) == 0) {    // while not f_j do
            j++;
            used |= get(o, j);        // E_del <- E_del \cup E_j
        }
        value &= ~used;               // E_i \ E_del
        set(o, pos, value);           // E_i <- E_i \ E_del
    }
    // Test of E_i \ E_del is empty
    return (value & o->inedgebitmask) == 0;
}

alder_wordtable_positionlist_t*
alder_wordtable_unusedfinals_getUnusedNodes(alder_wordtable_unusedfinals_t *o)
{
    return o->unusedNodes;
}



#pragma mark others

void
alder_wordtable_kfmindex_remove(alder_wordtable_kfmindex_t *o,
                                alder_wordtable_positionlist_t *list)
{
    alder_wordtable_inedgedata_remove(o->inedgedata, list);
}

size_t
alder_wordtable_kfmindex_size(alder_wordtable_kfmindex_t *o)
{
    return alder_wordtable_inedgedata_size(o->inedgedata);
}

size_t
alder_wordtable_kfmindex_order(alder_wordtable_kfmindex_t *o)
{
    return o->order;
}

size_t
alder_wordtable_kfmindex_tokenset_size(alder_wordtable_kfmindex_t *o)
{
    return ALDER_WORDTABLE_TOKENSIZE;
}

/* This function returns the 5-bit value at pos in KFMindex.
 */
alder_dwordtable_t
alder_wordtable_kfmindex_get(alder_wordtable_kfmindex_t *o, size_t pos)
{
    return alder_wordtable_inedgedata_get(o->inedgedata, pos);
}

bool
alder_wordtable_kfmindex_valueHasInEdge(alder_wordtable_kfmindex_t *o,
                                        alder_dwordtable_t value, int token)
{
    return alder_wordtable_inedgedata_valueHasInEdge(o->inedgedata, value, token);
}
                                        
alder_dwordtable_t
alder_wordtable_kfmindex_getValue(alder_wordtable_kfmindex_t *o,
                                  size_t pos, bool ignoreGroupFlag)
{
    return alder_wordtable_inedgedata_getValue(o->inedgedata, pos, ignoreGroupFlag);
}


void
alder_wordtable_kfmindex_put(alder_wordtable_kfmindex_t *o,
                             alder_dwordtable_t value)
{
    alder_wordtable_inedgedata_put(o->inedgedata, value);
}

void
alder_wordtable_kfmindex_forgetPriorTo(alder_wordtable_kfmindex_t *o,
                                       size_t pos)
{
    alder_wordtable_inedgedata_forgetPriorTo(o->inedgedata, pos);
}


static size_t
size(alder_wordtable_kfmindex_t *o)
{
    return alder_wordtable_inedgedata_size(o->inedgedata);
}

/* This function returns the 5-bit array at pos.
 */
static alder_dwordtable_t
get(alder_wordtable_kfmindex_t *o, size_t pos)
{
    return alder_wordtable_inedgedata_get(o->inedgedata, pos);
}

/* This function sets the 5-bit array to value at pos.
 */
static void
set(alder_wordtable_kfmindex_t *o, size_t pos, alder_dwordtable_t value)
{
    return alder_wordtable_inedgedata_set(o->inedgedata, pos, value);
}

/* This function returns the rho at pos.
 */
static size_t
alder_wordtable_kfmindex_kfmStore_get(alder_wordtable_kfmindex_t *o,
                                      size_t pos)
{
    return alder_wordtable_bitpackedcumulativearray_get(o->kfmStore, pos);
}

/* This function returns the size of the store-rho table. 
 * This size can be smaller than KFMindex size.
 */
static size_t
alder_wordtable_kfmindex_kfmStore_size(alder_wordtable_kfmindex_t *o)
{
    return alder_wordtable_bitpackedcumulativearray_size(o->kfmStore);
}

bool
alder_wordtable_kfmindex_hasInEdge(alder_wordtable_kfmindex_t *o,
                                   size_t pos, int token)
{
    return alder_wordtable_inedgedata_hasInEdge(o->inedgedata, pos, token);
}

