/**
 * This file, alder_wordtable_bitpackedlist.c, is part of alder-wordtable.
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

/* This file implements bitpackedlist for storing 5-bit arrays.
 *
 * FIXME: Use all of the bits in the allocated memory.
 *
 * This class bit-packs items of a fixes number of bits into long numbers.
 * Each long integer contains a block of items, the number of items in each 
 * block depending on the size in bits of each item. If the size of an item
 * (itembits) is 5, then a 64-bit number can contain 12 items. The remainder
 * of 4 bits does not seem to be used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_wordtable_io.h"
#include "alder_wordtable_compute.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_positionlist.h"
#include "alder_wordtable_bitpackedlist.h"

static int
alder_wordtable_bitpackedlist_shiftDownInterval
(alder_wordtable_bitpackedlist_t *o,
 size_t start, size_t end, size_t shift);

static size_t
alder_wordtable_bitpackedlist_requiredBlockarraySize
(alder_wordtable_bitpackedlist_t *o, size_t n);

static int
alder_wordtable_bitpackedlist_enlarge_subarray
(alder_wordtable_bitpackedlist_t *o, size_t newsize);

static void
alder_wordtable_bitpackedlist_init(alder_wordtable_bitpackedlist_t *o,
                                   size_t bits, size_t allocate);

#pragma mark struct

struct alder_wordtable_bitpackedlist_struct {
    size_t CALLCOUNT_GET;   /* for debug               */
    size_t DEFAULT_INITSIZE;/* Defaul #_items          */
    size_t BLOCK_BITS;      /* #_bits in a block       */
    size_t subarrayitems;   /* #_items in a subarray   */
    size_t itembits;        /* #_bits per item         */
    size_t blockitems;      /* #_items per block       */
    size_t subarraysize;    /* #_blocks in a subarray  */
    size_t itembitmask;     /* (1 << 5) - 1 = 00011111 */
    size_t blockarrayLength;/* #_subarrays in the list */
                            /* used in alloation       */
    size_t subsize;         /* #_subarrays in the list */
                            /* This is the actual.     */
    size_t maxsize;         /* subsize x subarrayitems */
    size_t size;            /* #_items in a list       */
    alder_dwordtable_t **blockarray;
};


/* This function is called by function create. 
 * It initializes variables in this module.
 */
static void
alder_wordtable_bitpackedlist_init(alder_wordtable_bitpackedlist_t *o,
                                   size_t bits, size_t allocate)
{
    size_t x1 = 1;
    o->CALLCOUNT_GET = 0;
    o->DEFAULT_INITSIZE = x1 << ALDER_WORDTABLE_BITPACKEDLIST_DEFAULT_INITSIZE;
    if (allocate == 0) {
        allocate = o->DEFAULT_INITSIZE;
    }
    o->BLOCK_BITS =
    sizeof(alder_dwordtable_t) * ALDER_WORDTABLE_KFMDEFAULTS_BYTESIZE;
    
    o->subarrayitems = x1 << ALDER_WORDTABLE_BITPACKEDLIST_SUBARRAYITEMS;
    o->itembits = bits;
    o->blockitems = o->BLOCK_BITS / bits;
    o->subarraysize = alder_wordtable_compute_groupmaxsize(o->subarrayitems,
                                                           o->blockitems);
    o->itembitmask = (x1 << o->itembits) - x1;
    o->blockarrayLength =
    alder_wordtable_compute_groupmaxsize(allocate, o->subarrayitems);
    
    assert(o->blockarrayLength > 0);
    o->size = 0;
    o->subsize = 0;
    o->maxsize = 0;
    o->blockarray = NULL;
}

void
alder_wordtable_bitpackedlist_destroy(alder_wordtable_bitpackedlist_t *o)
{
    if (o != NULL) {
        if (o->blockarray != NULL) {
            for (size_t i = 0; i < o->subsize; i++) {
                XFREE(o->blockarray[i]);
            }
            XFREE(o->blockarray);
        }
        XFREE(o);
    }
}

/* This function creates a bit list of items of size bits.
 * 
 * The first parameter bits specifies how many bits an element uses.
 * The second parameter specifies the number of items, which however appears
 * to be unimportant because items can be added whatever the initial size
 * might be. More important parameter seems to be the size of a subarray.
 *
 * This is weired, but only the 1-D is created.
 *
 * I could have an 1-D array of bytes for a bit-array.
 * There are other bit arrays that cannot use 1-D array: e.g., inedgelist.
 * The bit-array in inedgelist needs to be sorted. Values need to be 
 * aligned to byte boundary. 
 *
 * One reason why I would need this 2-D array is that I might want to
 * partially free bit-arrays. Could I use realloc? 
 *
 * FIXME: Try to use other bit-array scheme!
 */
alder_wordtable_bitpackedlist_t*
alder_wordtable_bitpackedlist_create(size_t bits, size_t allocate)
{
    alder_wordtable_bitpackedlist_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_bitpackedlist_init(o,bits,allocate);
    /* memory */
    o->blockarray = malloc(sizeof(*o->blockarray) * o->blockarrayLength);
    if (o->blockarray == NULL) {
        alder_wordtable_bitpackedlist_destroy(o);
        return NULL;
    }
    memset(o->blockarray, 0, sizeof(*o->blockarray) * o->blockarrayLength);
    return o;
}

/* This function creates one with default number of elements.
 * Use ALDER_WORDTABLE_BITPACKEDLIST_DEFAULT_INITSIZE to change 
 * the default number of items.
 */
alder_wordtable_bitpackedlist_t*
alder_wordtable_bitpackedlist_create1(size_t bits)
{
    return alder_wordtable_bitpackedlist_create(bits, 0);
}

/* This function gets the item at pos.
 *
 * pos - the position of an item
 * 
 * Consider the following list:
 *
 *  0  1  2   3  4  5   6  7  8
 *  9 10 11  12 13 14  15 16 17 <- blockpos
 * 18 19 20  21 22 23  24 25 26
 * 27 28 29  30 31 32  33 34 35
 *               ^
 *               subarraypos
 *
 * subarrayitems = 9
 * blockitems = 3
 * subsize = 4
 * subarraysize = 3
 *
 * Let's find the indices for item 13.
 * blockpos = 13 / 9 = 1
 * blocksub = 13 % 9 = 4
 * bitpos = 4 / 3 = 1
 * subarray = blockarray[blockpos]
 * subarray[bitpos] = 12 13 14
 * 
 * bitsub = blocksub % o->blockitems = 4 % 3 = 1
 * bitx = o->itembits * bitsub = 1 * 1
 *
 * Note that subarray[subpos] = [14 13 12] in bitwise.
 * subarray[subpos] >> (o->itembits * subx) = [14 13 12] >> (1 * 1)
 *                                          = [-- 14 13]
 * [-- 14 13] & [-- -- XX] = 13
 *
 * blockpos - the position of the 1st dimension
 * subarraypos - the item position in a subarray
 * subpos - which block in the subarray
 */
alder_dwordtable_t
alder_wordtable_bitpackedlist_get(alder_wordtable_bitpackedlist_t *o,
                                  size_t pos)
{
    assert(pos < o->maxsize);
    
    size_t blockpos = pos / o->subarrayitems;
    size_t blocksub = pos % o->subarrayitems;
    size_t bitpos = blocksub / o->blockitems;
    size_t bitsub = blocksub % o->blockitems;
    size_t bitx = o->itembits * bitsub;
    alder_dwordtable_t *subarray = o->blockarray[blockpos];
    // Get the value of bits.
    return (subarray[bitpos] >> bitx) & o->itembitmask;
}

/* This function sets the item at pos to value.
 */
void
alder_wordtable_bitpackedlist_set(alder_wordtable_bitpackedlist_t *o,
                                  size_t pos, alder_dwordtable_t value)
{
    assert(pos < o->maxsize);
    size_t blockpos = pos / o->subarrayitems;
    size_t blocksub = pos % o->subarrayitems;
    size_t bitpos = blocksub / o->blockitems;
    size_t bitsub = blocksub % o->blockitems;
    size_t bitx = o->itembits * bitsub;
    alder_dwordtable_t *subarray = o->blockarray[blockpos];
    // Zero the target item, and insert the value at the position.
    subarray[bitpos] &= (~(o->itembitmask << bitx));
    subarray[bitpos] |= (value << bitx);
}

/* This function appends a value to the bit array. 
 * returns
 * SUCCESS or FAIL
 *
 * maxsize is the max number of items in the list.
 * maxsize = subsize x subarrayitems
 * If size reaches maxsize, a new subarray is needed.
 * If subsize reaches blockarrayLength, a 1-D must be lengthened.
 *
 * update:
 * blockarray
 * maxsize and subsize
 */
int
alder_wordtable_bitpackedlist_put(alder_wordtable_bitpackedlist_t *o,
                                  alder_dwordtable_t value)
{
    int status = ALDER_WORDTABLE_SUCCESS;
    if (o->size == o->maxsize) {
        if (o->subsize == o->blockarrayLength) {
            status =
            alder_wordtable_bitpackedlist_enlarge_subarray(o,
                                                           o->size * 2 + 1);
        }
        assert(o->subsize < o->blockarrayLength);
        alder_dwordtable_t *a = malloc(sizeof(*a) * o->subarraysize);
        if (a == NULL) {
            return ALDER_WORDTABLE_FAIL;
        }
        memset(a, 0, sizeof(*a) * o->subarraysize);
        assert(o->blockarray[o->subsize] == NULL);
        o->blockarray[o->subsize] = a;
        o->subsize++;
        o->maxsize = o->subsize * o->subarrayitems;
    }
    alder_wordtable_bitpackedlist_set(o, o->size, value);
    o->size++;
    return ALDER_WORDTABLE_SUCCESS;
}

/* setSize is a misnomer because it fills the list with zeros upto that
 * new size.
 */
int
alder_wordtable_bitpackedlist_setSize(alder_wordtable_bitpackedlist_t *o,
                                      size_t newsize)
{
    int status = ALDER_WORDTABLE_SUCCESS;
    if (!(newsize > o->size)) {
        return ALDER_WORDTABLE_FAIL;
    }
    if (newsize > o->maxsize) {
        status = alder_wordtable_bitpackedlist_enlarge_subarray(o, newsize);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(status, ALDER_WORDTABLE_FAIL);
//        alder_wordtable_bitpackedlist_resize(o, newsize);
    }
    size_t newsubsize =
    alder_wordtable_bitpackedlist_requiredBlockarraySize(o, newsize);
    while (o->subsize < newsubsize) {
        alder_dwordtable_t *a = malloc(sizeof(*a) * o->subarraysize);
        if (a == NULL) {
            return ALDER_WORDTABLE_FAIL;
        }
        memset(a, 0, sizeof(*a) * o->subarraysize);
        o->blockarray[o->subsize++] = a;
    }
    assert(o->subsize == newsubsize);
    o->maxsize = o->subsize * o->subarrayitems;
    // Fill it with zeros.
    // FIXME: filling zero might not happen.
    o->size = newsize;
    return status;
}

/* This function removes items at positions in the list.
 */
void
alder_wordtable_bitpackedlist_remove(alder_wordtable_bitpackedlist_t *o,
                                     alder_wordtable_positionlist_t *list)
{
    size_t list_size = alder_wordtable_positionlist_size(list);
    if (list_size == 0) {
        return;
    }
    size_t pos = alder_wordtable_positionlist_get(list, 0);
    for (size_t i = 1; i < list_size; i++) {
        size_t pos_i = alder_wordtable_positionlist_get(list, i);
        alder_wordtable_bitpackedlist_shiftDownInterval(o, pos, pos_i, i);
        pos = pos_i;
    }
    alder_wordtable_bitpackedlist_shiftDownInterval(o, pos, o->size, list_size);
    o->size -= list_size;
}

/* This function frees first subarrays not including the item at pos.
 */
void
alder_wordtable_bitpackedlist_forgetPriorTo
(alder_wordtable_bitpackedlist_t *o, size_t pos)
{
    size_t subpos = pos / o->subarrayitems;
    if (subpos == 0) {
        return;
    }
    for (size_t i = 0; i < subpos; i++) {
        XFREE(o->blockarray[i]);
    }
}

#pragma mark save and load

/* This function computes the data length of IKFM chunk.
 * It is redundant to have all of the data, but it is just for the sanity
 * check. Three values of size 8 bytes include:
 * the number of bits in an item, 
 * the number of items in the KFM index,
 * and the number of blocks. 
 * A block is assumed to be of 64-bit. A block can contain upto 12 items since
 * 5 bits contitutes an item. The
 *
 */
size_t
alder_wordtable_bitpackedlist_save_size(alder_wordtable_bitpackedlist_t *o)
{
    size_t x = o->size / o->subarrayitems;
    size_t y = o->size % o->subarrayitems;
    size_t z = alder_wordtable_compute_groupmaxsize(y, o->blockitems);
    size_t nblocks = x * o->subarraysize + z;
    return 8 * (3 + nblocks);
}

/* This function writes bitpacked values.
 *
 * 1. 8 bytes - number of bits of an item in the list
 * 2. 8 bytes - number of items
 * 3. 8 bytes - number of 64-bit values.
 * 4. DATA in 64-bit values
 */
int
alder_wordtable_bitpackedlist_save(alder_wordtable_bitpackedlist_t *o,
                                   FILE *fp,
                                   uint32_t *crc)
{
    size_t x = o->size / o->subarrayitems;
    size_t y = o->size % o->subarrayitems;
    size_t z = alder_wordtable_compute_groupmaxsize(y, o->blockitems);
    size_t nblocks = x * o->subarraysize + z;
    int r = ALDER_WORDTABLE_SUCCESS;
    // 1
    r = alder_wordtable_io_write_uint64(fp, o->itembits, crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 2
    r = alder_wordtable_io_write_uint64(fp, o->size, crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 3
    r = alder_wordtable_io_write_uint64(fp, nblocks, crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 4
    for (size_t i = 0; i < x; i++) {
        for (size_t j = 0; j < o->subarraysize; j++) {
            r = alder_wordtable_io_write_uint64(fp,
                                                o->blockarray[i][j],
                                                crc);
            if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
        }
    }
    for (size_t j = 0; j < z; j++) {
        r = alder_wordtable_io_write_uint64(fp,
                                            o->blockarray[x][j],
                                            crc);
        if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    }
    return ALDER_WORDTABLE_SUCCESS;
}

/* This function is called by function create. 
 * It initializes variables in this module.
 */
static void
alder_wordtable_bitpackedlist_load_init(alder_wordtable_bitpackedlist_t *o,
                                        size_t bits, size_t allocate)
{
    alder_wordtable_bitpackedlist_init(o, bits, allocate);
    o->size = allocate;
    o->subsize = o->blockarrayLength;
    o->maxsize = o->subsize * o->subarrayitems;
}

/* This function creates bitpackedlist from a file.
 * The first three values are:
 * the number of bits in an item, 
 * the number of items in the KFM index,
 * and the number of blocks. 
 * A block is assumed to be of 64-bit. A block can contain upto 12 items since
 * 5 bits contitutes an item. The list is 2-D array, where each subarray can
 * contain subarraysize blocks.
 *
 * 1. read number of bits in an item (8B)
 * 2. read number of items in the list (8B)
 * 3. read number of blocks (8B)
 * 4. create the bitpackedlist variable, and initialize it.
 * 5. compute how many subarrays are full.
 * 6. reads blocks.
 */
alder_wordtable_bitpackedlist_t *
alder_wordtable_bitpackedlist_load(FILE *fp, uint32_t *crc)
{
    uint64_t itembits = 0;
    uint64_t size = 0;
    uint64_t nblocks = 0;
    int r = ALDER_WORDTABLE_SUCCESS;
    // 1
    r = alder_wordtable_io_read_uint64(fp, &itembits, crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    // 2
    r = alder_wordtable_io_read_uint64(fp, &size, crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    // 3
    r = alder_wordtable_io_read_uint64(fp, &nblocks, crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    // 4
    alder_wordtable_bitpackedlist_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_bitpackedlist_load_init(o, (size_t)itembits, (size_t)size);
    /* memory */
    o->blockarray = malloc(sizeof(*o->blockarray) * o->blockarrayLength);
    if (o->blockarray == NULL) {
        alder_wordtable_bitpackedlist_destroy(o);
        return NULL;
    }
    memset(o->blockarray, 0, sizeof(*o->blockarray) * o->blockarrayLength);
    for (size_t i = 0; i < o->blockarrayLength; i++) {
        o->blockarray[i] = malloc(sizeof(*o->blockarray[i]) * o->subarraysize);
        if (o->blockarray[i] == NULL) {
            alder_wordtable_bitpackedlist_destroy(o);
            return NULL;
        }
        memset(o->blockarray[i], 0, sizeof(*o->blockarray[i]) * o->subarraysize);
    }
    // 5
    size_t x = (size_t)nblocks / o->subarraysize;
    size_t z = (size_t)nblocks % o->subarraysize;
    // 6
    for (size_t i = 0; i < x; i++) {
        for (size_t j = 0; j < o->subarraysize; j++) {
            r = alder_wordtable_io_read_uint64(fp,
                                               &o->blockarray[i][j],
                                               crc);
            if (r != ALDER_WORDTABLE_SUCCESS) {
                alder_wordtable_bitpackedlist_destroy(o);
                return NULL;
            }
        }
    }
    for (size_t j = 0; j < z; j++) {
        r = alder_wordtable_io_read_uint64(fp,
                                           &o->blockarray[x][j],
                                           crc);
        if (r != ALDER_WORDTABLE_SUCCESS) {
            alder_wordtable_bitpackedlist_destroy(o);
            return NULL;
        }
    }
    return o;
}


#pragma mark others

/* Move interval between start and end, i.e. start+1,...,end-1, 
 * down by a given number of positions
 * returns 
 * SUCCESS on success,
 * otherwise fail.
 */
int
alder_wordtable_bitpackedlist_shiftDownInterval
(alder_wordtable_bitpackedlist_t *o,
 size_t start, size_t end, size_t shift)
{
    assert(shift <= start + 1);
    if (start + 1 < shift) {
        return ALDER_WORDTABLE_FAIL;
    }
    for (size_t i = start + 1; i < end; i++) {
        alder_dwordtable_t value = alder_wordtable_bitpackedlist_get(o, i);
        alder_wordtable_bitpackedlist_set(o, i - shift, value);
    }
    return ALDER_WORDTABLE_SUCCESS;
}

size_t
alder_wordtable_bitpackedlist_requiredBlockarraySize
(alder_wordtable_bitpackedlist_t *o, size_t n)
{
    return alder_wordtable_compute_groupmaxsize(n, o->subarrayitems);
}


size_t
alder_wordtable_bitpackedlist_size(alder_wordtable_bitpackedlist_t *o)
{
    return o->size;
}

/* This function prepares more space for subarrays.
 * returns SUCCESS on success.
 * otherwise FAIL
 */
static int
alder_wordtable_bitpackedlist_enlarge_subarray
(alder_wordtable_bitpackedlist_t *o, size_t newsize)
{
    assert(o->size < newsize);
    size_t newsubsize =
    alder_wordtable_bitpackedlist_requiredBlockarraySize(o, newsize);
    if (o->blockarrayLength < newsubsize) {
        alder_dwordtable_t **t = realloc(o->blockarray,
                                         sizeof(*t) * newsubsize);
        if (t == NULL) {
            return ALDER_WORDTABLE_FAIL;
        }
        for (size_t i = o->blockarrayLength; i < newsubsize; i++) {
            t[i] = NULL;
        }
        o->blockarray = t;
        o->blockarrayLength = newsubsize;
    }
    return ALDER_WORDTABLE_SUCCESS;
}

//size_t
//alder_wordtable_bitpackedlist_getItemBits(alder_wordtable_bitpackedlist_t *o)
//{
//    return o->itembits;
//}



//size_t
//alder_wordtable_bitpackedlist_getBlockBits(alder_wordtable_bitpackedlist_t *o)
//{
//    return o->BLOCK_BITS;
//}


//size_t
//alder_wordtable_bitpackedlist_getItemsPerBlock(alder_wordtable_bitpackedlist_t *o)
//{
//    return o->blockitems;
//}

//alder_dwordtable_t*
//alder_wordtable_bitpackedlist_getValueCount
//(alder_wordtable_bitpackedlist_t *o)
//{
//    alder_dwordtable_t x1 = 1;
//    size_t s = x1 << o->itembits;
//    alder_dwordtable_t *count = malloc(sizeof(*count) * s);
//    for (size_t i = o->size; i > 0; i--) {
//        size_t v = (size_t)alder_wordtable_bitpackedlist_get(o, i-1);
//        count[v]++;
//    }
//    return count;
//}


///* This function frees the memory.
// */
//void
//alder_wordtable_bitpackedlist_removeAll(alder_wordtable_bitpackedlist_t *o)
//{
//    assert(o != NULL);
//    if (o->blockarray != NULL) {
//        for (size_t i = 0; i < o->subsize; i++) {
//            XFREE(o->blockarray[i]);
//        }
//        XFREE(o->blockarray);
//    }
//    o->blockarrayLength = 0;
//    o->size = 0;
//    o->subsize = 0;
//    o->maxsize = 0;
////    alder_wordtable_bitpackedlist_compact(o);
////    o->subsize = o->blockarrayLength;
////    o->maxsize = o->subsize * o->subarrayitems;
//}




// This just changes the first dim. not second one. This is a very confusing
// function. It only changes the first dimension.
// o->blockarrayLength.
// NOTE: Do not change o->subsize.
//static void
//alder_wordtable_bitpackedlist_resize(alder_wordtable_bitpackedlist_t *o,
//                                     size_t newsize)
//{
//    if (o->size > newsize) {
//        fprintf(stderr, "newsize must be larger than o->size!\n");
//        abort();
//    }
//    
//    size_t newsubsize =
//    alder_wordtable_bitpackedlist_requiredBlockarraySize(o, newsize);
//    if (newsize == 0) {
//        assert(newsubsize == 0);
//    }
//    // Okay till now.
//    if (newsubsize > o->blockarrayLength) {
//        alder_dwordtable_t **t = realloc(o->blockarray,
//                                         sizeof(*t) * newsubsize);
//        if (t == NULL) {
//            // Memory Error!
//            fprintf(stderr, "Memory error!\n");
//            abort();
//        }
//        for (size_t i = o->blockarrayLength; i < newsubsize; i++) {
//            t[i] = NULL;
//        }
//        o->blockarray = t;
//        o->blockarrayLength = newsubsize;
//    } else if (newsubsize < o->blockarrayLength) {
//        for (size_t i = newsubsize; i < o->blockarrayLength; i++) {
//            if (o->blockarray[i] != NULL) {
//                XFREE(o->blockarray[i]);
//            }
//        }
//        if (newsubsize > 0) {
//            alder_dwordtable_t **t = realloc(o->blockarray,
//                                             sizeof(*t) * newsubsize);
//            if (t == NULL) {
//                // Memory Error!
//                fprintf(stderr, "Memory error!\n");
//                abort();
//            }
//            o->blockarray = t;
//        } else {
//            XFREE(o->blockarray);
//        }
//        o->blockarrayLength = newsubsize;
//    }
//}

//void
//alder_wordtable_bitpackedlist_compact(alder_wordtable_bitpackedlist_t *o)
//{
//    alder_wordtable_bitpackedlist_resize(o, o->size);
//}
