/**
 * This file, alder_wordtable.c, is part of alder-wordtable.
 *
 * Copyright 2013, 2014 by Sang Chul Choi
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
#include "bstraux.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_fileseq_token.h"
#include "alder_wordtable_sequenceiterator.h"
#include "alder_wordtable_kfmindex.h"
#include "alder_wordtable_kfmaggregator.h"
#include "alder_graph_alist.h"
#include "alder_wordtable.h"
#include "alder_wordtable_inedgedata.h"
#include "alder_seq.h"

#pragma mark test


void
alder_wordtable_tokeniterator_test()
{
    // String
    bstring bseq1 = bfromcstr("CACTAGTACAGT");
    bstring bseq2 = bfromcstr("CAGCTAGTAT");

    alder_wordtable_tokeniterator_t *t =
    alder_wordtable_tokeniterator_createWithBstring(bseq1);
    
    bool hasNext = alder_wordtable_tokeniterator_hasNext(t);
    while (hasNext) {
        int c = alder_wordtable_tokeniterator_next(t);
        printf("%d", c);
        hasNext = alder_wordtable_tokeniterator_hasNext(t);
    }
    printf("\n");

    alder_wordtable_tokeniterator_setWithBstring(t,bseq2);
    hasNext = alder_wordtable_tokeniterator_hasNext(t);
    while (hasNext) {
        int c = alder_wordtable_tokeniterator_next(t);
        printf("%d", c);
        hasNext = alder_wordtable_tokeniterator_hasNext(t);
    }
    printf("\n");
    
    alder_wordtable_tokeniterator_destroy(t);
    bdestroy(bseq2);
    bdestroy(bseq1);
    
    // File without buffer
    t = alder_wordtable_tokeniterator_create("test.fa");
    bool moreSequence = alder_wordtable_tokeniterator_moreSequence(t);
    while (moreSequence) {
        hasNext = alder_wordtable_tokeniterator_hasNext(t);
        while (hasNext) {
            int c = alder_wordtable_tokeniterator_next(t);
            printf("%d", c);
            hasNext = alder_wordtable_tokeniterator_hasNext(t);
        }
        printf("\n");
        moreSequence = alder_wordtable_tokeniterator_moreSequence(t);
    }
    alder_wordtable_tokeniterator_destroy(t);

    // File with buffer
    t = alder_wordtable_tokeniterator_create("test.fa");
    alder_wordtable_tokeniterator_set(t, 7, 3);
    moreSequence = alder_wordtable_tokeniterator_moreSequence(t);
    while (moreSequence) {
        bool hasMore = true;
        while (hasMore) {
            hasNext = true;
            while (hasNext) {
                int c = alder_wordtable_tokeniterator_next(t);
                printf("%d", c);
                hasNext = alder_wordtable_tokeniterator_hasNext(t);
            }
            printf("---\n");
            hasMore = alder_wordtable_tokeniterator_hasMore(t);
            alder_wordtable_tokeniterator_reset(t, 7);
        }
        printf("\n");
        moreSequence = alder_wordtable_tokeniterator_moreSequence(t);
    }
    alder_wordtable_tokeniterator_destroy(t);
}

void
alder_wordtable_bigbitarray_test()
{
//    const size_t s = 133;
    const size_t s = 8;
    alder_wordtable_bigbitarray_t *b = alder_wordtable_bigbitarray_create(s);

    alder_wordtable_bigbitarray_set(b, 2);
    alder_wordtable_bigbitarray_set(b, 7);
//    alder_wordtable_bigbitarray_set(b, 64);
//    alder_wordtable_bigbitarray_set(b, 65);
//    alder_wordtable_bigbitarray_set(b, 132);

    for (size_t i = 0; i < s; i++) {
        bool isSet = alder_wordtable_bigbitarray_get(b, i);
        alder_log("[%03zu] %d", i, isSet);
    }
    
    size_t c = alder_wordtable_bigbitarray_countBits(b);
    alder_log("Number of bits set = %zu", c);
    alder_wordtable_bigbitarray_print(b);
    alder_wordtable_bigbitarray_destroy(b);
}


void
alder_wordtable_positionlist_test()
{
    alder_wordtable_positionlist_t *l = alder_wordtable_positionlist_create();
    
    alder_wordtable_positionlist_add(l, 2);
    alder_wordtable_positionlist_add(l, 6);
    alder_wordtable_positionlist_add(l, 4);
    alder_wordtable_positionlist_add(l, 9);
    alder_wordtable_positionlist_add(l, 1);
    
    size_t s = alder_wordtable_positionlist_size(l);
    for (size_t i = 0; i < s; i++) {
        size_t v = alder_wordtable_positionlist_get(l, i);
        alder_log("[%02zu] %zu", i, v);
    }
    alder_log("sorting...");
    
    alder_wordtable_positionlist_sort(l);
    for (size_t i = 0; i < s; i++) {
        size_t v = alder_wordtable_positionlist_get(l, i);
        alder_log("[%02zu] %zu", i, v);
    }
    alder_wordtable_positionlist_destroy(l);
}

void
alder_wordtable_positionvaluelist_test()
{
    alder_wordtable_positionvaluelist_t *l =
    alder_wordtable_positionvaluelist_create();
    
    alder_wordtable_positionvaluelist_add(l, 2, 1);
    alder_wordtable_positionvaluelist_add(l, 6, 2);
    alder_wordtable_positionvaluelist_add(l, 4, 3);
    alder_wordtable_positionvaluelist_add(l, 9, 4);
    alder_wordtable_positionvaluelist_add(l, 1, 5);
    
    size_t s = alder_wordtable_positionvaluelist_size(l);
    for (size_t i = 0; i < s; i++) {
        size_t vp = alder_wordtable_positionvaluelist_getPosition(l, i);
        alder_dwordtable_t vv = alder_wordtable_positionvaluelist_getValue(l, i);
        alder_log("[%02zu] %zu - %lld", i, vp, vv);
    }
    alder_wordtable_positionvaluelist_destroy(l);
}

/* bitpackedlist stores values of fixed bit size.
 * The second value in the create function sets the number of values
 * in the list. This second value seems to be meaningless.
 *
 * put function adds a new value to the list.
 * get function gets a value at a position.
 * compact func deletes unused subarrays.
 * setSize increases the size and fills added elements with zeros.
 *
 */
void
alder_wordtable_bitpackedlist_test()
{
    alder_wordtable_bitpackedlist_t *b =
    alder_wordtable_bitpackedlist_create(20, 10);
    
//    alder_dwordtable_t v = 7;
    alder_wordtable_bitpackedlist_put(b,1);
    alder_wordtable_bitpackedlist_put(b,2);
    alder_wordtable_bitpackedlist_put(b,3);
//    alder_wordtable_bitpackedlist_compact(b);
    alder_wordtable_bitpackedlist_setSize(b, 5);
    alder_wordtable_bitpackedlist_put(b,4);
    alder_wordtable_bitpackedlist_put(b,5);

    alder_log("Initial");
    size_t b_size = alder_wordtable_bitpackedlist_size(b);
    for (size_t i = 0; i < b_size; i++) {
        alder_dwordtable_t rv = alder_wordtable_bitpackedlist_get(b, i);
        alder_log("[%03zu] %llu", i, rv);
    }
    
    /* position list */
    alder_wordtable_positionlist_t *l = alder_wordtable_positionlist_create();
    alder_wordtable_positionlist_add(l, 3);
    alder_wordtable_positionlist_add(l, 4);
//    alder_wordtable_positionlist_add(l, 7);
    
    alder_wordtable_bitpackedlist_remove(b,l);
    
    alder_log("Removing...");
    b_size = alder_wordtable_bitpackedlist_size(b);
    for (size_t i = 0; i < b_size; i++) {
        alder_dwordtable_t rv = alder_wordtable_bitpackedlist_get(b, i);
        alder_log("[%03zu] %llu", i, rv);
    }
    
//    alder_wordtable_bitpackedlist_removeAll(b);
//    b_size = alder_wordtable_bitpackedlist_size(b);
//    for (size_t i = 0; i < b_size; i++) {
//        alder_dwordtable_t rv = alder_wordtable_bitpackedlist_get(b, i);
//        alder_log("[%03zu] %llu", i, rv);
//    }

    // not tested functions are
    //        forgetPriorTo
    //        alder_wordtable_bitpackedlist_getValueCount
    alder_wordtable_positionlist_destroy(l);
    alder_wordtable_bitpackedlist_destroy(b);
}


void
alder_wordtable_bitpackedcumulativearray_test()
{
    // KFMsize = 3
    // stepsize = 5
    alder_wordtable_bitpackedcumulativearray_t *a =
    alder_wordtable_bitpackedcumulativearray_create(1*4+1,8);
    
    // 1 3 5 6 8
    // 1 2 2 1 2
    alder_wordtable_bitpackedcumulativearray_set(a,  0, 1);
    alder_wordtable_bitpackedcumulativearray_set(a,  1, 2);
    alder_wordtable_bitpackedcumulativearray_set(a,  2, 2);
    alder_wordtable_bitpackedcumulativearray_set(a,  3, 1);
    alder_wordtable_bitpackedcumulativearray_set(a,  4, 2);

    alder_wordtable_bitpackedcumulativearray_cumulate(a);
    
    size_t s = alder_wordtable_bitpackedcumulativearray_size(a);
    for (size_t i = 0; i < s; i++) {
        size_t av =
        alder_wordtable_bitpackedcumulativearray_get(a, i);
        alder_log("[%03zu] %zu", i, av);
    }
    alder_wordtable_bitpackedcumulativearray_destroy(a);
    
//    // KFMsize = 3
//    // stepsize = 5
//    alder_wordtable_bitpackedcumulativearray_t *a =
//    alder_wordtable_bitpackedcumulativearray_create(3*4+1,5);
//    
//    // 1 1 6 6 8 9 9 9 11 11 15 15 15
//    // 1 0 5 0 2 1 0 0  2  0  4  0  0
//    alder_wordtable_bitpackedcumulativearray_set(a,  0, 1);
//    alder_wordtable_bitpackedcumulativearray_set(a,  1, 0);
//    alder_wordtable_bitpackedcumulativearray_set(a,  2, 5);
//    alder_wordtable_bitpackedcumulativearray_set(a,  3, 0);
//    alder_wordtable_bitpackedcumulativearray_set(a,  4, 2);
//    alder_wordtable_bitpackedcumulativearray_set(a,  5, 1);
//    alder_wordtable_bitpackedcumulativearray_set(a,  6, 0);
//    alder_wordtable_bitpackedcumulativearray_set(a,  7, 0);
//    alder_wordtable_bitpackedcumulativearray_set(a,  8, 2);
//    alder_wordtable_bitpackedcumulativearray_set(a,  9, 0);
//    alder_wordtable_bitpackedcumulativearray_set(a, 10, 4);
//    alder_wordtable_bitpackedcumulativearray_set(a, 11, 0);
//    alder_wordtable_bitpackedcumulativearray_set(a, 12, 0);
//
//    alder_wordtable_bitpackedcumulativearray_cumulate(a);
//    
//    size_t s = alder_wordtable_bitpackedcumulativearray_size(a);
//    for (size_t i = 0; i < s; i++) {
//        size_t av =
//        alder_wordtable_bitpackedcumulativearray_get(a, i);
//        alder_log("[%03zu] %zu", i, av);
//    }
//    alder_wordtable_bitpackedcumulativearray_destroy(a);
}

void
alder_wordtable_bits_test()
{
    size_t s;
    alder_wordtable_t x;
    size_t y;
    
    x = 0x00000001;
    s = alder_wordtable_bits_bitSizeOfWord(x);
    assert(s == 1);
    x = 0x00000010;
    s = alder_wordtable_bits_bitSizeOfWord(x);
    assert(s == 5);
    x = 0x10000010;
    s = alder_wordtable_bits_bitSizeOfWord(x);
    assert(s == 29);
    x = 0xFFFFFFFF;
    s = alder_wordtable_bits_bitSizeOfWord(x);
    assert(s == 32);
    
    y = 0x0000000000000002;
    s = alder_wordtable_bits_log2ceil(y);
    assert(s == 1);
    y = 0x0000000000000010;
    s = alder_wordtable_bits_log2ceil(y);
    assert(s == 4);
    y = 0x0000000000010000;
    s = alder_wordtable_bits_log2ceil(y);
    assert(s == 16);
    y = 0x0000000000020000;
    s = alder_wordtable_bits_log2ceil(y);
    assert(s == 17);
    y = 0x0008000000020000;
    s = alder_wordtable_bits_log2ceil(y);
    assert(s == 52);
    
    alder_wordtable_t *arr = malloc(sizeof(*arr) * 3);
    arr[0] = 0x00000001;
    arr[1] = 0x00000001;
    arr[2] = 0x00000001;
    x = alder_wordtable_bits_shiftRight(arr, 3, 2, 1);
    assert(x == 1);
    for (int i = 0; i < 3; i++) {
        assert(arr[i] == 0x40000000);
    }
    
    arr[0] = 0xC0000001;
    arr[1] = 0x00000001;
    arr[2] = 0x00000001;
    x = alder_wordtable_bits_shiftLeft(arr, 3, 2, 2);
    assert(x == 3);
    assert(arr[0] == 0x00000004);
    assert(arr[1] == 0x00000004);
    assert(arr[2] == 0x00000006);
    
    XFREE(arr);
}

//void
//alder_wordtable_bits_test2()
//{
//    size_t z64;
//    uint64_t uv64;
//    uint32_t uv32;
//    uint8_t  uv8;
//    size_t   s;
//    int64_t v64;
//    alder_log("double size is %zu", sizeof(double));
//    alder_log("long double size is %zu", sizeof(long double));
//    
////    int64_t v64 = 0x0000000000000001;
////    s = alder_wordtable_bits_LongNumberOfLeadingZeros(v64);
////    for (size_t i = 0; i < 64; i++) {
////        alder_log("leading zeros(64)[%02zu]: %lld - %zu", i + 1, v64, s);
////        v64 <<= 1;
////        s = alder_wordtable_bits_LongNumberOfLeadingZeros(v64);
////    }
//    
//    uv64 = 0x0000000000000001;
//    s = alder_wordtable_bits_bitSize_uint64(uv64);
//    alder_log("bitsize(64): %llu - %zu", uv64, s);
//    for (size_t i = 0; i < 64; i++) {
//        uv64 <<= 1;
//        s = alder_wordtable_bits_bitSize_uint64(uv64);
//        alder_log("bitsize(64)[%02zu]: %llu - %zu", i + 2, uv64, s);
//    }
//    uv64 = 0xFFFFFFFFFFFFFFFF;
//    s = alder_wordtable_bits_bitSize_uint64(uv64);
//    alder_log("bitsize(64): %llu - %zu", uv64, s);
//    
//    v64 = 0x8000000000000000;
//    s = alder_wordtable_bits_bitSize_uint64(v64);
//    alder_log("bitsize(64): %lld - %zu", v64, s);
//
//    uv32 = 0x80000000;
//    s = alder_wordtable_bits_bitSize_uint32(uv32);
//    alder_log("bitsize(32): %u - %zu", uv32, s);
//
//    uv8 = 0x80;
//    s = alder_wordtable_bits_bitSize_uint8(uv8);
//    alder_log("bitsize(8): %u - %zu", uv8, s);
//    
//    int8_t v8 = 0x80;
//    s = alder_wordtable_bits_bitSize_uint8(v8);
//    alder_log("bitsize(8): %d - %zu", v8, s);
// 
//    alder_log("log2ceil");
//    z64 = 1;
////    s = alder_wordtable_bits_log2ceil(z64);
//    for (size_t i = 0; i < 64; i++) {
//        alder_log("log2ceil(64)[%02zu]: %zu - %zu", i, z64, s);
//        z64 <<= 1;
//        if (z64 > 1) {
//            s = alder_wordtable_bits_log2ceil(z64);
//        }
//    }
////    alder_log("log2ceil(64)[%02zu]: %zu - %zu", 64, z64, s);
//
// 
//    // alder_wordtable_bits_shiftRight
//    // alder_wordtable_bits_shiftRight2
//    // alder_wordtable_bits_shiftLeft
//    // alder_wordtable_bits_shiftLeft2
//    // alder_wordtable_bits_copyShiftLeft
//    // alder_wordtable_bits_longBitCount
//    alder_wordtable_t arr[3];
//    arr[0] = 0xFFFFFFFF;
//    arr[1] = 0x00000000;
//    arr[2] = 0xFFFFFFFD;
//    alder_wordtable_t arrinsert = 0x00000002;
//    arrinsert = alder_wordtable_bits_shiftRight(arr, 3, 2, arrinsert);
//    for (size_t i = 0; i < 3; i++) {
//        alder_log("[%zu] %u", i, arr[i]);
//    }
//    alder_log("out insert: %u", arrinsert);
//    
//    alder_log("shift left");
//    arr[0] = 0x7FFFFFFF;
//    arr[1] = 0x00000000;
//    arr[2] = 0xFFFFFFFF;
//    arrinsert = 0x00000002;
//    arrinsert = alder_wordtable_bits_shiftLeft(arr, 2, 2, arrinsert);
//    for (size_t i = 0; i < 3; i++) {
//        alder_log("[%zu] %u", i, arr[i]);
//    }
//    alder_log("out insert: %u", arrinsert);
//
//    
//    alder_log("shift left 2");
//    arr[0] = 0x7FFFFFFF;
//    arr[1] = 0x00000000;
//    arr[2] = 0xFFFFFFFF;
//    arrinsert = 0x00000002;
//    arrinsert = alder_wordtable_bits_copyShiftLeft(arr, 0,
//                                                   arr, 0,
//                                                   2, 63, arrinsert);
//    for (size_t i = 0; i < 3; i++) {
//        alder_log("[%zu] %u", i, arr[i]);
//    }
//    alder_log("out insert: %u", arrinsert);
//
//}

void
alder_wordtable_intblocklist_test()
{

    // 2 is the size of a block. Each block consists of multiple integers or
    // 4-byte variables.
    alder_wordtable_intblocklist_t *ibl = alder_wordtable_intblocklist_create(2);
    
    // 1. list, set, get, and copy.
    // 2. Sort the list.
    
    alder_wordtable_intblocklist_destroy(ibl);
}

void
alder_wordtable_inedgelist_test()
{
    alder_wordtable_inedgelist_t *e =
    alder_wordtable_inedgelist_getNew(3, 100000);
    bstring bseq1 = bfromcstr("CACTAGTACAGT");
    alder_wordtable_inedgelist_addBsequence(e, bseq1);
    bstring bseq2 = bfromcstr("CAGCTAGTAT");
    alder_wordtable_inedgelist_addBsequence(e, bseq2);
    
    size_t s = alder_wordtable_inedgelist_size(e);
    printf("Number of edges: %zu\n", s);
    
    // Check the added node and data.
    for (size_t i = 0; i < s; i++) {
        bstring bseq = alder_wordtable_inedgelist_tokensAsString(e,i);
        alder_wordtable_t edgeToken = alder_wordtable_inedgelist_edgeToken(e,i);
        char c = alder_wordtable_tokenset_char_of(edgeToken);
        printf("[%02zu] %s %c\n", i, bdata(bseq), c);
        bdestroy(bseq);
    }
    
    // Sorting...
    printf("Sorting...\n");
    alder_wordtable_inedgelist_sort(e);
    // List nodes again.
    for (size_t i = 0; i < s; i++) {
        bstring bseq = alder_wordtable_inedgelist_tokensAsString(e,i);
        alder_wordtable_t edgeToken = alder_wordtable_inedgelist_edgeToken(e,i);
        char c = alder_wordtable_tokenset_char_of(edgeToken);
        printf("[%02zu] %s %c\n", i, bdata(bseq), c);
        bdestroy(bseq);
    }

    // Complete.
    alder_wordtable_inedgelist_complete(e);
    // List nodes again.
    s = alder_wordtable_inedgelist_size(e);
    for (size_t i = 0; i < s; i++) {
        bstring bseq = alder_wordtable_inedgelist_tokensAsString(e,i);
        alder_wordtable_t edgeToken = alder_wordtable_inedgelist_edgeToken(e,i);
        char c = alder_wordtable_tokenset_char_of(edgeToken);
        printf("[%02zu] %s %c\n", i, bdata(bseq), c);
        bdestroy(bseq);
    }
    printf("In BIT-WISE FORM...\n");
    for (size_t i = 0; i < s; i++) {
        bstring bseq = alder_wordtable_inedgelist_tokensAsString(e,i);
        bstring bbit = alder_wordtable_inedgelist_edgeAsBitString(e,i);
        alder_wordtable_t edgeToken = alder_wordtable_inedgelist_edgeToken(e,i);
        char c = alder_wordtable_tokenset_char_of(edgeToken);
        printf("[%02zu] %s %c - [%s]\n", i, bdata(bseq), c, bdata(bbit));
        bdestroy(bbit);
        bdestroy(bseq);
    }

    bdestroy(bseq2);
    bdestroy(bseq1);
    alder_wordtable_inedgelist_destroy(e);
}

void
alder_wordtable_inedgedata_test()
{
    alder_wordtable_inedgelist_t *e =
    alder_wordtable_inedgelist_getNew(3, 1000);
    
    bstring bseq1 = bfromcstr("CACTAGTACAGT");
    alder_wordtable_inedgelist_addBsequence(e, bseq1);
    bstring bseq2 = bfromcstr("CAGCTAGTAT");
    alder_wordtable_inedgelist_addBsequence(e, bseq2);
    
    alder_wordtable_inedgelist_complete(e);

    alder_wordtable_inedgedata_t *ed =
    alder_wordtable_inedgedata_createWithInEdgeList(e);
    
    size_t s = alder_wordtable_inedgedata_size(ed);
    for (size_t i = 0; i < s; i++) {
        bstring bedge = alder_wordtable_inedgedata_nodeDataString(ed, i);
        printf("[%02zu] %s\n", i, bdata(bedge));
        bdestroy(bedge);
    }
    
    
    bdestroy(bseq2);
    bdestroy(bseq1);
    alder_wordtable_inedgedata_destroy(ed);
    alder_wordtable_inedgelist_destroy(e);
}

void
alder_wordtable_kfmindex_test()
{
    alder_wordtable_inedgelist_t *e =
    alder_wordtable_inedgelist_getNew(3, 1000);
    bstring bseq1 = bfromcstr("CACTAGTACAGT");
    alder_wordtable_inedgelist_addBsequence(e, bseq1);
    bstring bseq2 = bfromcstr("CAGCTAGTAT");
    alder_wordtable_inedgelist_addBsequence(e, bseq2);
    
    alder_wordtable_inedgelist_complete(e);
    
    alder_wordtable_kfmindex_t *kfm =
    alder_wordtable_kfmindex_create2(e, 5); // 5
    
//    alder_wordtable_kfmindex_computeKFMarray0(kfm);
    
    // Print them out.
    size_t s = alder_wordtable_kfmindex_size(kfm);
    for (size_t i = 0; i < s; i++) {
        bstring bedge = alder_wordtable_kfmindex_toString(kfm, i);
        printf("[%02zu] %s\n", i, bdata(bedge));
        bdestroy(bedge);
    }

    
    // Navigate KFMindex using prevPos and prevPos2.
    
    // We may need these in KFMindex merge.
    // prune?
    // checkIfUnused?
    
    
    bdestroy(bseq2);
    bdestroy(bseq1);
    alder_wordtable_inedgelist_destroy(e);
    alder_wordtable_kfmindex_destroy(kfm);
}

void
alder_wordtable_kfminterval_test()
{
    
}

void
alder_wordtable_kfmmerge_test()
{
    // Create two KFMindex objects.
    // See the content of each of the two KFMindex's.
    // Merge them.
    // See the content of the merged one to check the merge.
    
    alder_wordtable_inedgelist_t *e1 =
    alder_wordtable_inedgelist_getNew(3,1000);
    
    alder_wordtable_inedgelist_t *e2 =
    alder_wordtable_inedgelist_getNew(3,1000);
    
    bstring bseq1 = bfromcstr("CACTAGTACAGT");
    alder_wordtable_inedgelist_addBsequence(e1, bseq1);
    bstring bseq2 = bfromcstr("CAGCTAGTAT");
    alder_wordtable_inedgelist_addBsequence(e2, bseq2);
    
    alder_wordtable_inedgelist_complete(e1);
    alder_wordtable_inedgelist_complete(e2);
    
    alder_wordtable_kfmindex_t *kfm1 =
    alder_wordtable_kfmindex_create2(e1, 5); // 5
    alder_wordtable_kfmindex_t *kfm2 =
    alder_wordtable_kfmindex_create2(e2, 5); // 5

    alder_wordtable_kfmindex_t *kfm3 =
    alder_wordtable_kfmmerger_mergeTwoKFMindex(kfm1,kfm2,false,5);

    // Print kfm3.
    // merge code looks okay to me.
    // memory management in KFMindex needs attention.
    FILE *fp = fopen("kfmmerge.txt", "w");
    size_t s = alder_wordtable_kfmindex_size(kfm3);
    for (size_t i = 0; i < s; i++) {
        bstring bedge = alder_wordtable_kfmindex_toString(kfm3, i);
        fprintf(fp, "[%010zu]%s\n", i, bdata(bedge));
        bdestroy(bedge);
    }
    XFCLOSE(fp);
    
    
    bdestroy(bseq2);
    bdestroy(bseq1);
    alder_wordtable_inedgelist_destroy(e1);
    alder_wordtable_inedgelist_destroy(e2);
    alder_wordtable_kfmindex_destroy(kfm3);
    alder_wordtable_kfmindex_destroy(kfm2);
    alder_wordtable_kfmindex_destroy(kfm1);
}


void
alder_wordtable_kfmprune_test()
{
    // FIXME: Case 1.
    // Use the following function to prune edges.
    // alder_wordtable_kfmindex_pruneFinalCompletions
    // Create KFMindex and add unnecessary edges.
    // Prune it.
    alder_wordtable_inedgelist_t *e =
    alder_wordtable_inedgelist_getNew(3, 1000);
    
    bstring bseq1 = bfromcstr("CACTAGTACAGT");
    alder_wordtable_inedgelist_addBsequence(e, bseq1);
    bstring bseq2 = bfromcstr("CAGCTAGTAT");
    alder_wordtable_inedgelist_addBsequence(e, bseq2);
    
    alder_wordtable_inedgelist_complete(e);
    
    alder_wordtable_kfmindex_t *kfm =
    alder_wordtable_kfmindex_create2(e, 5); // 5
    
//    alder_wordtable_kfmindex_computeKFMarray0(kfm);
    
    // Print them out.
    size_t s = alder_wordtable_kfmindex_size(kfm);
    for (size_t i = 0; i < s; i++) {
        bstring bedge = alder_wordtable_kfmindex_toString(kfm, i);
        printf("[%02zu] %s\n", i, bdata(bedge));
        bdestroy(bedge);
    }
    
    alder_wordtable_kfmindex_pruneFinalCompletions(kfm);
    
    bdestroy(bseq2);
    bdestroy(bseq1);
    alder_wordtable_inedgelist_destroy(e);
    alder_wordtable_kfmindex_destroy(kfm);
    
    
    // FIXME: Case 2.
    // Create two KFMindex that merge to one with superfluous final completing
    // vertices.
    // Merge them.
    // Prune it.
    
}


/* Let's create a simple graph.
 * Let's create the graph in the Figure 1 at Rodland13.
 * I need a hash table for checking whether a l-string is already used.
 * I won't use a hash table, but a simple list of l-string. 
 * Use bstrlib's functions for a bag of l-string.
 */
void
alder_wordtable_kfmsim_test()
{
    // Start with a string.
    bstring bFinal = bfromcstr("$$$");
    bstring bseq1 = bfromcstr("CACTAGTACAGT");
    bstring bseq2 = bfromcstr("CAGCTAGTAT");
    struct bstrList *bseqs= bstrVectorCreate(2);
    bseqs->entry[bseqs->qty++] = bseq1;
    bseqs->entry[bseqs->qty++] = bseq2;
    
    // Size of K and L.
    int lLen = blength(bFinal);
//    int kLen = lLen + 1;
    
    // Add l-string's except the last one.
    struct bstrList *lstring = bstrVectorCreate(1);
    // Add the final string, $$$, first.
    bstrVectorAddBstring(lstring, bFinal);
    for (int i = 0; i < bseqs->qty; i++) {
        bstring bseq = bseqs->entry[i];
        for (int j = 0; j < blength(bseq) - lLen; j++) {
            bstrVectorAddBstring(lstring, bmidstr (bseq, j, lLen));
        }
    }
    int nLstring = blength(bseq1) + blength(bseq2) - lLen * 2;
    assert(lstring->qty == nLstring + 1);
    
    // Check if the last l-string is final.
    for (int i = 0; i < bseqs->qty; i++) {
        bstring bFinalCandidate = bTail(bseqs->entry[i], lLen);
        bool isFinal = true;
        for (int j = 0; j < lstring->qty; j++) {
            if (!bstrcmp(bFinalCandidate, lstring->entry[j])) {
                isFinal = false;
            }
        }
        // Add the string and its final completing
        if (isFinal) {
            bconcat(bseqs->entry[i], bFinal);
            bconcat(bFinalCandidate, bFinal);
            for (int j = 0; j < lLen; j++) {
                bstrVectorAddBstring(lstring,
                                     bmidstr(bFinalCandidate, j, lLen));
                
            }
        }

    }
    
    fprintf(stdout, "L-strings are:\n");
    bstrVectorPrint(stdout, lstring);
    fprintf(stdout, "Sorting...\n");
    bstrVectorSort(lstring);
    bstrVectorPrint(stdout, lstring);
    fprintf(stdout, "Unique...\n");
    bstrVectorUnique(lstring);
    bstrVectorPrint(stdout, lstring);
    
    // Check if each edge k-string exists in the input string set.
    // We already appended the final vertex string to the relevant seqeunces.
    // Create an array of char.
    uint8_t *eta = malloc(sizeof(*eta) * 4 * lstring->qty);
    memset(eta, 0, sizeof(*eta) * 4 * lstring->qty);
    for (int i = 0; i < lstring->qty; i++) {
        bstring bav = bfromcstr("A");
        bconcat(bav, lstring->entry[i]);
        for (int j = 0; j < 4; j++) {
            bav->data[0] = (unsigned char)alder_wordtable_tokenset_char_of(j);
            if (bstrVectorExistSubstring(bseqs, bav) != BSTR_ERR) {
                eta[i + j * lstring->qty] = 1;
            }
        }
        bdestroy(bav);
    }
    for (int i = 0; i < lstring->qty; i++) {
        fprintf(stdout, "[%03d]", i);
        for (int j = 0; j < 4; j++) {
            if (eta[i + j * lstring->qty] == 1) {
                fprintf(stdout, "%c", alder_wordtable_tokenset_char_of(j));
            } else {
                fprintf(stdout, " ");
            }
        }
        fprintf(stdout, "-%s", bdata(lstring->entry[i]));
        fprintf(stdout, "\n");
    }
    
    // Use lstring to construct the group end array.
    uint8_t *fi = malloc(sizeof(*fi) * lstring->qty);
    memset(fi, 0, sizeof(*fi) * lstring->qty);
    fi[lstring->qty - 1] = 1;
    for (int i = 0; i < lstring->qty - 1; i++) {
        bstring bseqm1 = bHead(lstring->entry[i], lLen - 1);
        bstring bseqm2 = NULL;
        if (i == lstring->qty - 1) {
            bseqm2 = bfromcstr("XX");
        } else {
            bseqm2 = bHead(lstring->entry[i+1], lLen - 1);
        }
        if (bstrcmp(bseqm1, bseqm2)) {
            fi[i] = 1;
        }
        bdestroy(bseqm2);
        bdestroy(bseqm1);
    }
    
    // Use eta to contruct the rho table.
    size_t *rho = malloc(sizeof(*rho) * 4 * lstring->qty);
    memset(rho, 0, sizeof(*rho) * 4 * lstring->qty);
    size_t rho_value = 1;
    for (int i = 0; i < 4; i++) {
        bool isPrevInEdge = false;
        bool isInEdge = false;
        for (int j = 0; j < lstring->qty; j++) {
            if (eta[j + i * lstring->qty] == 1) {
                isInEdge = true;
            }
            if (fi[j] == 1) {
                isPrevInEdge = isInEdge;
                isInEdge = false;
            }
            
            rho[j + i * lstring->qty] = rho_value;
            if (fi[j] == 1 && isPrevInEdge == true) {
                rho_value++;
            }
        }
    }
    
    // Print the table.
    for (int i = 0; i < lstring->qty; i++) {
        fprintf(stdout, "[%03d]", i);
        for (int j = 0; j < 4; j++) {
            if (eta[i + j * lstring->qty] == 1) {
                fprintf(stdout, "%c", alder_wordtable_tokenset_char_of(j));
            } else {
                fprintf(stdout, " ");
            }
        }
        fprintf(stdout, " %s", bdata(lstring->entry[i]));
        if (fi[i] == 1) {
            fprintf(stdout, " * ");
        } else {
            fprintf(stdout, "   ");
        }
        for (int j = 0; j < 4; j++) {
            fprintf(stdout, "%3zu", rho[i + j * lstring->qty]);
        }
        fprintf(stdout, "\n");
    }
    
    XFREE(rho);
    XFREE(fi);
    XFREE(eta);
    bstrVectorDelete(lstring);
    bstrVectorDelete(bseqs);
}

/* This function consructs KFMindex of a set of sequences using Rodland13's
 * method.
 */
static void
alder_wordtable_kfmsim_construct_fkmindex_using_rodland(const char *fn,
                                                        struct bstrList *bseqs,
                                                        int kLen)
{
    alder_wordtable_inedgelist_t *e =
    alder_wordtable_inedgelist_getNew(kLen - 1, 1000);
    
    for (int i = 0; i < bseqs->qty; i++) {
        alder_wordtable_inedgelist_addBsequence(e, bseqs->entry[i]);
    }
    
    alder_wordtable_inedgelist_complete(e);
    
    alder_wordtable_kfmindex_t *kfm =
    alder_wordtable_kfmindex_create2(e, 5); // 5
    
//    alder_wordtable_kfmindex_computeKFMarray0(kfm);
    
    // Print them out.
    FILE *fp = fopen(fn, "w");
    size_t s = alder_wordtable_kfmindex_size(kfm);
    for (size_t i = 0; i < s; i++) {
        bstring bedge = alder_wordtable_kfmindex_toString(kfm, i);
        fprintf(fp, "[%010zu]%s\n", i, bdata(bedge));
        bdestroy(bedge);
    }
    XFCLOSE(fp);

    // Navigate KFMindex using prevPos and prevPos2.
    
    // We may need these in KFMindex merge.
    // prune?
    // checkIfUnused?
    
    alder_wordtable_inedgelist_destroy(e);
    alder_wordtable_kfmindex_destroy(kfm);
}

/* This function constructs KFMindex of a set of sequences using a naive 
 * method.
 * Note that the length is the edge length. The length of string in vertices
 * is one subtracted by 1.
 */
static void
alder_wordtable_kfmsim_construct_fkmindex_using_naive(const char *fn,
                                                      struct bstrList *bseqsI,
                                                      int kLen)
{
    struct bstrList *bseqs = bstrVectorCopy(bseqsI);
    // Size of K and L.
    int lLen = kLen - 1;
    // Start with a string.
    bstring bFinal = brepeat(kLen - 1, '$');
    
    
    // Add l-string's except the last one.
    struct bstrList *lstring = bstrVectorCreate(1);
    // Add the final string, $$$, first.
    bstrVectorAddBstring(lstring, bFinal);
    for (int i = 0; i < bseqs->qty; i++) {
        bstring bseq = bseqs->entry[i];
        for (int j = 0; j < blength(bseq) - lLen; j++) {
            bstrVectorAddBstring(lstring, bmidstr (bseq, j, lLen));
        }
    }
    
    // Check if the last l-string is final.
    for (int i = 0; i < bseqs->qty; i++) {
        bstring bFinalCandidate = bTail(bseqs->entry[i], lLen);
        bool isFinal = true;
        for (int j = 0; j < lstring->qty; j++) {
            if (!bstrcmp(bFinalCandidate, lstring->entry[j])) {
                isFinal = false;
            }
        }
        // Add the string and its final completing
        if (isFinal) {
            bconcat(bseqs->entry[i], bFinal);
            bconcat(bFinalCandidate, bFinal);
            for (int j = 0; j < lLen; j++) {
                bstrVectorAddBstring(lstring,
                                     bmidstr(bFinalCandidate, j, lLen));
                
            }
        }

    }
    
    bstrVectorSort(lstring);
    bstrVectorUnique(lstring);
    
    // Check if each edge k-string exists in the input string set.
    // We already appended the final vertex string to the relevant seqeunces.
    // Create an array of char.
    uint8_t *eta = malloc(sizeof(*eta) * 4 * lstring->qty);
    memset(eta, 0, sizeof(*eta) * 4 * lstring->qty);
    for (int i = 0; i < lstring->qty; i++) {
        bstring bav = bfromcstr("A");
        bconcat(bav, lstring->entry[i]);
        for (int j = 0; j < 4; j++) {
            bav->data[0] = (unsigned char)alder_wordtable_tokenset_char_of(j);
            if (bstrVectorExistSubstring(bseqs, bav) != BSTR_ERR) {
                eta[i + j * lstring->qty] = 1;
            }
        }
        bdestroy(bav);
    }
    
    // Use lstring to construct the group end array.
    uint8_t *fi = malloc(sizeof(*fi) * lstring->qty);
    memset(fi, 0, sizeof(*fi) * lstring->qty);
    fi[lstring->qty - 1] = 1;
    for (int i = 0; i < lstring->qty - 1; i++) {
        bstring bseqm1 = bHead(lstring->entry[i], lLen - 1);
        bstring bseqm2 = NULL;
        if (i == lstring->qty - 1) {
            bseqm2 = brepeat(kLen - 2, 'X');
        } else {
            bseqm2 = bHead(lstring->entry[i+1], lLen - 1);
        }
        if (bstrcmp(bseqm1, bseqm2)) {
            fi[i] = 1;
        }
        bdestroy(bseqm2);
        bdestroy(bseqm1);
    }
    
    // Use eta to contruct the rho table.
    size_t *rho = malloc(sizeof(*rho) * 4 * lstring->qty);
    memset(rho, 0, sizeof(*rho) * 4 * lstring->qty);
    size_t rho_value = 1;
    for (int i = 0; i < 4; i++) {
        bool isPrevInEdge = false;
        bool isInEdge = false;
        for (int j = 0; j < lstring->qty; j++) {
            if (eta[j + i * lstring->qty] == 1) {
                isInEdge = true;
            }
            if (fi[j] == 1) {
                isPrevInEdge = isInEdge;
                isInEdge = false;
            }
            
            rho[j + i * lstring->qty] = rho_value;
            if (fi[j] == 1 && isPrevInEdge == true) {
                rho_value++;
            }
        }
    }
    
    // Print the table.
    FILE *fp = fopen(fn, "w");
    for (int i = 0; i < lstring->qty; i++) {
        fprintf(fp, "[%010d]", i);
        for (int j = 0; j < 4; j++) {
            if (eta[i + j * lstring->qty] == 1) {
                fprintf(fp, "%c", alder_wordtable_tokenset_char_of(j));
            } else {
                fprintf(fp, " ");
            }
        }
        if (fi[i] == 1) {
            fprintf(fp, " * ");
        } else {
            fprintf(fp, "   ");
        }
        fprintf(fp, "%s ", bdata(lstring->entry[i]));
        for (int j = 0; j < 4; j++) {
            fprintf(fp, "%zu", rho[i + j * lstring->qty]);
            if (j < 3) {
                fprintf(fp, "\t");
            }
        }
        fprintf(fp, "\n");
    }
    XFCLOSE(fp);
    
    XFREE(rho);
    XFREE(fi);
    XFREE(eta);
    bstrVectorDelete(lstring);
    bstrVectorDelete(bseqs);
}

/* This would create an array of simulated data to check if the KFMindex 
 * works on those data.  I use the following steps:
 *
 * N = #_simulation
 * K = edge length
 * NS = number of sequences
 * LS = sequence length
 *
 * 1. Sample a set of seqeunces.
 * 2. Construct the Rodland table using a naive method.
 * 3. Construct it using a more optimized method.
 * 4. Compare two files.
 */
void
alder_wordtable_kfmcheck_test()
{
    size_t N = 1;
//    size_t n_seq = 10;
//    size_t l_seq = 10;
    int kLen = 4;
//    int lLen = kLen - 1;
    
    for (size_t n = 0; n < N; n++) {
        // 1. Create a set of sequences (NS, LS).
//        struct bstrList* bseqs = alder_seq_sample_bstrList(n_seq, l_seq);
        bstring bseq1 = bfromcstr("CACTAGTACAGT");
        bstring bseq2 = bfromcstr("CAGCTAGTAT");
        struct bstrList *bseqs= bstrVectorCreate(2);
        bseqs->entry[bseqs->qty++] = bseq1;
        bseqs->entry[bseqs->qty++] = bseq2;
        
        // 2. Construct the Rodland table using a naive method.
        bstring bfn1 = bfromcstr("kfmcheck1.txt");
        alder_wordtable_kfmsim_construct_fkmindex_using_naive(bdata(bfn1),
                                                              bseqs,
                                                              kLen);
        
        // 3. Construct the Rodland table using KFMindex.
        bstring bfn2 = bfromcstr("kfmcheck2.txt");
        alder_wordtable_kfmsim_construct_fkmindex_using_rodland(bdata(bfn2),
                                                                bseqs,
                                                                kLen);
        
        // 4. Compare the two tables (as files).
        
        
        bdestroy(bfn1);
        bdestroy(bfn2);
        bstrVectorDelete(bseqs);
    }
}

/* This function reads sequences in a file.
 */
void
alder_wordtable_sequenceiterator_test(struct bstrList *fn)
{
    // order 3 or l = 3, and k = 4.
    // buffersize must be 2 = l - 1.
    alder_wordtable_sequenceiterator_t *seqs =
    alder_wordtable_sequenceiterator_create(fn, 2 - 1);
    
    size_t count = 0;
    int token = alder_wordtable_sequenceiterator_token(seqs);
    while (token < ALDER_WORDTABLE_TOKENSIZE + 2) {
        if (token < ALDER_WORDTABLE_TOKENSIZE) {
            char c = alder_wordtable_tokenset_char_of(token);
            printf("%c", c);
            count++;
            if (count == 1) {
                alder_wordtable_sequenceiterator_buffer(seqs);
            }
        } else {
            // End of a sequence
            count = 0;
            printf("\n");
        }
        if (token == ALDER_WORDTABLE_TOKENSIZE + 1) {
            printf("New file:\n");
        }
        token = alder_wordtable_sequenceiterator_token(seqs);
    }
    alder_wordtable_sequenceiterator_destroy(seqs);
    
    // 1. character
    // 2. quality score
    // 3. end or not.
}

/* Use a list of sequence files to build a kfm file.
 */
void
alder_wordtable_kfmaggregator_test(struct bstrList *infile,
                                   const char *fn)
{
    size_t order = 3;
    size_t maxEdgelist = 5;
    size_t maxEdgestack = 32;
    alder_wordtable_kfmaggregator_t *aggr =
    alder_wordtable_kfmaggregator_create(order, maxEdgelist, maxEdgestack);
    
    // I need to save the final node for later use.
    // 
    alder_wordtable_sequenceiterator_t *seqs =
    alder_wordtable_sequenceiterator_create(infile, order);
    
    alder_wordtable_kfmaggregator_addAll(aggr, seqs);
    
    alder_wordtable_kfmindex_t *kfm =
    alder_wordtable_kfmaggregator_getKFMindex(aggr);
    
    // Print KFMindex.
    alder_wordtable_kfmindex_save(kfm , fn);
    //
    alder_wordtable_kfmindex_destroy(kfm);
    
    alder_wordtable_sequenceiterator_destroy(seqs);
    alder_wordtable_kfmaggregator_destroy(aggr);
}

void
alder_wordtable_kfmio_test(const char *fn)
{
    alder_wordtable_kfmindex_t *kfm = alder_wordtable_kfmindex_load(fn);

    alder_wordtable_kfmindex_print(kfm);
    
    alder_wordtable_kfmindex_destroy(kfm);
}


void
alder_wordtable_kfmfind_test()
{
    alder_wordtable_kfmindex_t *kfm = alder_wordtable_kfmindex_load("aggr.kfm");

    alder_wordtable_kfmindex_print(kfm);

    size_t a = alder_wordtable_kfmindex_findLstring(kfm, "AGC");
    alder_log("AGC: %zu", a);
    a = alder_wordtable_kfmindex_findLstring(kfm, "GGG");
    alder_log("GGG: %zu", a);
    a = alder_wordtable_kfmindex_findLstring(kfm, "CTA");
    alder_log("CTA: %zu", a);
    
    
    
    alder_wordtable_kfmindex_destroy(kfm);
    
    
}