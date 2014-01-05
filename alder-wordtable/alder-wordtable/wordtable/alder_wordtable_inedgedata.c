/**
 * This file, alder_wordtable_inedgedata.c, is part of alder-wordtable.
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

/* 
 * This file contains inedgedata, which bridges KFMindex and bitpackedlist.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bstrlib.h"
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_wordtable_io.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_fileseq_token.h"
#include "alder_wordtable_tokenset.h"
#include "alder_wordtable_bitpackedlist.h"
#include "alder_wordtable_inedgelist.h"
#include "alder_wordtable_positionlist.h"
#include "alder_wordtable_inedgedata.h"

static void
alder_wordtable_inedgedata_print5bit(alder_wordtable_inedgedata_t *o,
                                     alder_dwordtable_t value);

static bool
alder_wordtable_inedgedata_isLastInGroup(alder_wordtable_inedgedata_t *o,
                                         size_t position);

static bstring
alder_wordtable_inedgedata_valueToDataString(alder_wordtable_inedgedata_t *o,
                                             alder_dwordtable_t value);

#pragma mark struct

struct alder_wordtable_inedgedata_struct {
    alder_wordtable_bitpackedlist_t *bitpackedlist;
    size_t alpha;
    size_t order;
    alder_dwordtable_t inedgebitmask;
};

void
alder_wordtable_inedgedata_destroy(alder_wordtable_inedgedata_t *o)
{
    if (o != NULL) {
        alder_wordtable_bitpackedlist_destroy(o->bitpackedlist);
        XFREE(o);
    }
}

/* This function intializes inedgedata.
 * alpha = 4.
 * order = l or k-1, where k is the order of the de Bruijn subgraph.
 * itembits = 00...00011110.
 */
static void
alder_wordtable_inedgedata_init(alder_wordtable_inedgedata_t *o,
                                size_t order)
{
    o->bitpackedlist = NULL;
    o->alpha = ALDER_WORDTABLE_TOKENSIZE;
    o->order = order;
    alder_dwordtable_t x = 1;
    alder_dwordtable_t x2 = 2;
    // 011110 when itembits is 5.
    size_t itembits = o->alpha + 1;
    o->inedgebitmask = (x << itembits) - x2;
}

static void
alder_wordtable_inedgedata_super(alder_wordtable_inedgedata_t *o,
                                 size_t bits, size_t allocate)
{
    o->bitpackedlist = alder_wordtable_bitpackedlist_create(bits, allocate);
}

/* This function creates inedgedata with order and initsize.
 * The order is l or k-1, where k is the order of a de Bruijn subgraph.
 * As I mentioned in KFMindex, initsize is not important.
 */
alder_wordtable_inedgedata_t*
alder_wordtable_inedgedata_create(size_t order, size_t initsize)
{
    alder_wordtable_inedgedata_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_inedgedata_init(o, order);
    /* memory */
    alder_wordtable_inedgedata_super(o, o->alpha + 1, initsize);
    if (o->bitpackedlist == NULL) {
        alder_wordtable_inedgedata_destroy(o);
        return NULL;
    }
    return o;
}

/* This function creates inedgedata using inedgelist.
 */
alder_wordtable_inedgedata_t*
alder_wordtable_inedgedata_createWithInEdgeList
(alder_wordtable_inedgelist_t *list)
{
    size_t order = alder_wordtable_inedgelist_order(list);
//    size_t initsize = alder_wordtable_inedgelist_estimateNodeCountBound(list);
    size_t initsize = ALDER_WORDTABLE_KFMDEFAULTS_SIZE;
    alder_wordtable_inedgedata_t *o =
    alder_wordtable_inedgedata_create(order, initsize);
    
    if (o != NULL) {
        alder_wordtable_inedgedata_addEdges(o, list);
    }
    return o;
}

/* This function adds edges using a sorted list of edges.
 *
 */
void
alder_wordtable_inedgedata_addEdges(alder_wordtable_inedgedata_t *o,
                                    alder_wordtable_inedgelist_t *list)
{
    const alder_wordtable_t x1 = 1;
    const alder_wordtable_t x2 = 2;
    alder_wordtable_t value = 0;
    size_t maxpos = alder_wordtable_inedgelist_size(list);
    int token = alder_wordtable_inedgelist_edgeToken(list, 0);
    // 00010 << {0...3}.
    // A - 00010
    // C - 00100
    // G - 01000
    // T - 10000
    if (token < (int)o->alpha) {
        value |= (x2 << token);
    }
    for (size_t pos = 1; pos < maxpos; pos++) {
        size_t equal = alder_wordtable_inedgelist_nTokensEqual(list, pos, pos-1);
        if (equal < o->order - 1) {
            value |= x1;
        }
        if (equal < o->order) {
#if !defined(NDEBUG)
            alder_wordtable_inedgedata_print5bit(o,value);
#endif
            alder_wordtable_bitpackedlist_put(o->bitpackedlist, value);
            value = 0;
        }
        token = alder_wordtable_inedgelist_edgeToken(list, pos);
        if (token < (int)o->alpha) {
            value |= (x2 << token);
        }
    }
#if !defined(NDEBUG)
            alder_wordtable_inedgedata_print5bit(o,value | x1);
#endif
    alder_wordtable_bitpackedlist_put(o->bitpackedlist, value | x1);
}

///* This function copies a value at pos in source, and appends it to the list.
// * If the ignore group flag is set or true, then the LSB is set to 0.
// */
//void
//alder_wordtable_inedgedata_addCopy(alder_wordtable_inedgedata_t *o,
//                                   alder_wordtable_inedgedata_t *source,
//                                   size_t pos,
//                                   bool ignoreGroupFlag)
//{
//    alder_dwordtable_t x =
//    alder_wordtable_bitpackedlist_get(source->bitpackedlist, pos);
//    if (ignoreGroupFlag) {
//        // Put this as a member to the inedgedata object.
//        // 1111...11111110
//        alder_dwordtable_t x2 = 1;
//        x2 = ~x2;
//        alder_wordtable_bitpackedlist_put(o->bitpackedlist, x & x2);
//    } else {
//        alder_wordtable_bitpackedlist_put(o->bitpackedlist, x);
//    }
//}

/* This function gets the 5-bit value at pos. 
 * If the ignoreGroupFlag is true, then the least bit is set to 0.
 * e.g.,
 * If 5-bit value is 11101, and ignoreGroupFlag is true, then
 * it returns 11100 not the original 5-bit value, 11101.
 */
alder_dwordtable_t
alder_wordtable_inedgedata_getValue(alder_wordtable_inedgedata_t *o,
                                    size_t pos, bool ignoreGroupFlag)
{
    alder_dwordtable_t x =
    alder_wordtable_bitpackedlist_get(o->bitpackedlist, pos);
    if (ignoreGroupFlag) {
        return x & o->inedgebitmask;
    } else {
        return x;
    }
}

/* This function tests if the value is the end of a group.
 */
bool
alder_wordtable_inedgedata_valueIsGroupEnd(alder_wordtable_inedgedata_t *o,
                                           alder_dwordtable_t value)
{
    alder_dwordtable_t x1 = 1;
    return (value & x1) == x1;
}

/* This function tests if the value at position is the end of a group.
 */
bool
alder_wordtable_inedgedata_isLastInGroup(alder_wordtable_inedgedata_t *o,
                                         size_t position)
{
    alder_dwordtable_t x =
    alder_wordtable_bitpackedlist_get(o->bitpackedlist, position);
    return alder_wordtable_inedgedata_valueIsGroupEnd(o,x);
}

/* This function tests if an edge has a given token as an inedge.
 * e.g.,
 * 00010 -> A is an in-coming edge token.
 * 00100 -> C is an in-coming edge toknn.
 */
bool
alder_wordtable_inedgedata_valueHasInEdge(alder_wordtable_inedgedata_t *o,
                                          alder_dwordtable_t value, int token)
{
    alder_dwordtable_t x2 = 2;
    return ((value >> token) & x2) == x2;
}

/* This function tests if an edge at pos has a given token as an inedge.
 */
bool
alder_wordtable_inedgedata_hasInEdge(alder_wordtable_inedgedata_t *o,
                                     size_t pos, int token)
{
    alder_dwordtable_t x =
    alder_wordtable_bitpackedlist_get(o->bitpackedlist, pos);
    return alder_wordtable_inedgedata_valueHasInEdge(o,x,token);
}

/* FIXME: This function is called when pruning. 
 * Positionlist is already sorted.
 */
void
alder_wordtable_inedgedata_remove(alder_wordtable_inedgedata_t *o,
                                  alder_wordtable_positionlist_t *list)
{
    alder_dwordtable_t x1 = 1;
    size_t x = alder_wordtable_positionlist_size(list);
    
    for (int i = (int)x - 1; i >= 0; i--) {
        size_t pos = alder_wordtable_positionlist_get(list, (size_t)i);
        bool isLastInGroup = alder_wordtable_inedgedata_isLastInGroup(o,pos--);
        if (isLastInGroup) {
            alder_dwordtable_t getpos =
            alder_wordtable_bitpackedlist_get(o->bitpackedlist, pos);
            alder_wordtable_bitpackedlist_set(o->bitpackedlist, pos,
                                              getpos | x1);
        }
    }
    alder_wordtable_bitpackedlist_remove(o->bitpackedlist, list);
}

/* This function frees partially the 5-bit array in KFMindex.
 */
void
alder_wordtable_inedgedata_forgetPriorTo(alder_wordtable_inedgedata_t *o,
                                         size_t pos)
{
    alder_wordtable_bitpackedlist_forgetPriorTo(o->bitpackedlist, pos);
}

#pragma mark print

/* This function returns a string for edge of A,C,G,T with group data at pos.
 * e.g.,
 * ACGT *
 * A  T
 *    T *
 *  C T *
 *
 * See alder_wordtable_inedgedata_valueToDataString
 */
bstring
alder_wordtable_inedgedata_nodeDataString(alder_wordtable_inedgedata_t *o,
                                          size_t pos)
{
    alder_dwordtable_t x =
    alder_wordtable_bitpackedlist_get(o->bitpackedlist, pos);
    return alder_wordtable_inedgedata_valueToDataString(o,x);
}

/* This function converts a 5-bit value to a string for edge of A,C,G,T 
 * with group data.
 * e.g.,
 * ACGT *
 * A  T
 *    T *
 *  C T *
 */
bstring
alder_wordtable_inedgedata_valueToDataString(alder_wordtable_inedgedata_t *o,
                                             alder_dwordtable_t value)
{
    char c;
    bstring bacgt = bfromcstralloc(4 + 1 + 1, "");
    alder_dwordtable_t x1 = 1;
    for (int i = 0; i < 4; i++) {
        c = (((value >> (i+1)) & x1) == x1) ?
        alder_wordtable_tokenset_char_of(i) : ' ';
        bconchar(bacgt, c);
    }
    c = ((value & x1) == x1) ? '*' : ' ';
    bconchar(bacgt, ' ');
    bconchar(bacgt, c);
    return bacgt;
}

void
alder_wordtable_inedgedata_print5bit(alder_wordtable_inedgedata_t *o,
                                     alder_dwordtable_t value)
{
    bstring v = alder_wordtable_inedgedata_valueToDataString(o,value);
    alder_log5("5bit:%s",bdata(v));
    bdestroy(v);
}

#pragma mark save and load


/* This function writes the iKFM data chunk.
 * See alder_wordtable.io.c/h for detail.
 *
 * 8B - Length
 * 4B - IKFM
 * 8B - number of 5-bit values
 * 8B - number of 64-bit values
 * DATA - number of 64-bit values x 8 bytes
 * 
 * 1. Length
 * 2. Length and type
 * 3. data
 * 4. crc
 */
int
alder_wordtable_inedgedata_save(alder_wordtable_inedgedata_t *o,
                                FILE *fp)
{
    int r = ALDER_WORDTABLE_SUCCESS;
    uint32_t crc = ALDER_WORDTABLE_IO_CRC_INIT;
    // 1
    uint64_t lengthChunk =
    alder_wordtable_bitpackedlist_save_size(o->bitpackedlist);
    // 2
    r = alder_wordtable_io_write_IKFM(fp, lengthChunk, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 3
    r = alder_wordtable_bitpackedlist_save(o->bitpackedlist, fp, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 4
    r = alder_wordtable_io_write_uint32(fp, crc, NULL);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;

    return r;
}

/* This function loads inedgedata from a file pointer; IKFM chunk.
 * The data part is in bitpackedlist. Check alder_wordtable_bitpackedlist_load
 * for how to load a bitpackedlist.
 *
 * 1. read length, and type.
 * 2. create and initialize inedgedata variable.
 * 3. create bitpackedlist variable from a file.
 * 4. check the length of data.
 * 5. read crc.
 */
alder_wordtable_inedgedata_t *
alder_wordtable_inedgedata_load(FILE *fp, size_t order)
{
    int r = ALDER_WORDTABLE_SUCCESS;
    uint32_t crc = ALDER_WORDTABLE_IO_CRC_INIT;
    uint32_t read_crc = 0;
    uint64_t lengthChunk = 0;
    // 1
    r = alder_wordtable_io_read_IKFM(fp, &lengthChunk, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    // FIXME: no check about the length of the chunk?
    // 2
    alder_wordtable_inedgedata_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_inedgedata_init(o, order);
    // 3
    o->bitpackedlist = alder_wordtable_bitpackedlist_load(fp, &crc);
    if (o->bitpackedlist == NULL) {
        alder_wordtable_inedgedata_destroy(o);
        return NULL;
    }
    // 4
    uint64_t read_lengthChunk =
    alder_wordtable_bitpackedlist_save_size(o->bitpackedlist);
    if (read_lengthChunk != lengthChunk) {
        alder_wordtable_inedgedata_destroy(o);
        return NULL;
    }
    // 5
    r = alder_wordtable_io_read_uint32(fp, &read_crc, NULL);
    if (r != ALDER_WORDTABLE_SUCCESS ||
        crc != read_crc) {
        alder_wordtable_inedgedata_destroy(o);
        return NULL;
    }
    return o;
}

#pragma mark others

/* This function returns the size of KFMindex.
 */
size_t
alder_wordtable_inedgedata_size(alder_wordtable_inedgedata_t *o)
{
    return alder_wordtable_bitpackedlist_size(o->bitpackedlist);
}

/* This function gets the 5-bit value at pos.
 */
alder_dwordtable_t
alder_wordtable_inedgedata_get(alder_wordtable_inedgedata_t *o,
                               size_t pos)
{
    return alder_wordtable_bitpackedlist_get(o->bitpackedlist, pos);
}

/* This function sets the 5-bit value to value at pos.
 */
void
alder_wordtable_inedgedata_set(alder_wordtable_inedgedata_t *o,
                               size_t pos, alder_dwordtable_t value)
{
    return alder_wordtable_bitpackedlist_set(o->bitpackedlist, pos, value);
}

/* This function returns inedge bit mask to KFMindex.
 */
alder_dwordtable_t
alder_wordtable_inedgedata_inedgebitmask(alder_wordtable_inedgedata_t *o)
{
    return o->inedgebitmask;
}

/* This function appends a 5-bit value to the KFMindex.
 */
int
alder_wordtable_inedgedata_put(alder_wordtable_inedgedata_t *o,
                               alder_dwordtable_t value)
{
    return alder_wordtable_bitpackedlist_put(o->bitpackedlist, value);
}

size_t
alder_wordtable_inedgedata_order(alder_wordtable_inedgedata_t *o)
{
    return o->order;
}
