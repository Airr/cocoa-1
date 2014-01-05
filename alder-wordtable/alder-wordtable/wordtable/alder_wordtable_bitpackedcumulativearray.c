/**
 * This file, alder_wordtable_bitpackedcumulativearray.c, is part of alder-wordtable.
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

/* This file implements the rho in Rodland13. 
 * size - number of elements in rho (KFMsize x 4 + 1)
 * maxstep - ceil(2^(log_2 maxstep)) e.g., 5 -> 8
 * 
 * set function - starts with (0,1), and (pos,diff).
 * call cumulate
 * 
 * Use get function to get a stored rho.
 */

/**
 * Implementation of CumulativeArray where cumulative, cum[i], with increments
 * in the range 0..maxstep are cumulated and decomposed into cum[i]%factor, which
 * is stored, and cum[i]/factor for which the increments 0 or 1 are stored in a bit list.
 * In addition, cum[64*r]/factor are stored for fast aggregation of cum[i]/factor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_wordtable_io.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_bits.h"
#include "alder_wordtable_bitpackedlist.h"
#include "alder_wordtable_bitpackedcumulativearray.h"

static void
alder_wordtable_bitpackedcumulativearray_init
(alder_wordtable_bitpackedcumulativearray_t* o,
 size_t size,
 size_t maxstep);

#pragma mark struct

struct alder_wordtable_bitpackedcumulativearray_struct {
    size_t size;                              /* number of elements */
    size_t maxstep;                           /* */
    size_t BLOCKBITS;                         /* 64 */
    alder_dwordtable_t factor;
    
    alder_dwordtable_t *increments;           /* num bits >= size            */
    alder_dwordtable_t *stored;               /* the same size as increments */
    alder_wordtable_bitpackedlist_t *bitpack;
};

void
alder_wordtable_bitpackedcumulativearray_init
(alder_wordtable_bitpackedcumulativearray_t* o,
 size_t size,
 size_t maxstep)
{
    o->size = size;
    o->maxstep = maxstep;
    size_t bits = alder_wordtable_bits_log2ceil(maxstep);
    alder_dwordtable_t x1 = 1;
    o->factor = x1 << bits;
    o->BLOCKBITS = sizeof(alder_dwordtable_t) * 8;
    
    /* init */
    o->bitpack = NULL;
    o->stored = NULL;
    o->increments = NULL;
}

alder_wordtable_bitpackedcumulativearray_t*
alder_wordtable_bitpackedcumulativearray_create(size_t size,
                                                size_t maxstep)
{
    alder_wordtable_bitpackedcumulativearray_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_bitpackedcumulativearray_init(o,size,maxstep);
   
    size_t bits = alder_wordtable_bits_log2ceil(maxstep);
    size_t longsize = 1 + (size - 1)/o->BLOCKBITS;
    /* memory */
    o->bitpack = alder_wordtable_bitpackedlist_create1(bits);
    o->increments = malloc(sizeof(*o->increments) * longsize);
    o->stored = malloc(sizeof(*o->stored) * longsize);
    if (o->bitpack == NULL || o->increments == NULL || o->stored == NULL) {
        alder_wordtable_bitpackedcumulativearray_destroy(o);
        return NULL;
    }
    memset(o->increments, 0, sizeof(*o->increments) * longsize);
    memset(o->stored, 0, sizeof(*o->stored) * longsize);
    alder_wordtable_bitpackedlist_setSize(o->bitpack, size);
    
    return o;
}

void
alder_wordtable_bitpackedcumulativearray_destroy
(alder_wordtable_bitpackedcumulativearray_t* o)
{
    if (o != NULL) {
        alder_wordtable_bitpackedlist_destroy(o->bitpack);
        XFREE(o->increments);
        XFREE(o->stored);
        XFREE(o);
    }
}

/* This function sets the value at pos.
 * The value cannot be larger than factor or maxstep.
 * If the value is equal to factor, then I'd mark it in increment in bitwise 
 * manner.
 */
void
alder_wordtable_bitpackedcumulativearray_set
(alder_wordtable_bitpackedcumulativearray_t* o,
 size_t pos, alder_dwordtable_t value)
{
    assert(value <= o->factor);
    alder_dwordtable_t x = value % o->factor;
    alder_wordtable_bitpackedlist_set(o->bitpack, pos, x);
    assert(value <= o->maxstep);
    assert(value <= o->factor);
    if (value == o->factor) {
        size_t blockpos = pos / o->BLOCKBITS;
        size_t blocksub = pos % o->BLOCKBITS;
        alder_dwordtable_t x1 = 1;
        alder_dwordtable_t bitpos = x1 << blocksub;
        o->increments[blockpos] |= bitpos;
    } else if (value > o->factor) {
        abort();
    }
}

/* This function computes the cumulative array from increment values.
 *
 * 1. Get uj from the bitpackedlist at pos.
 * 2. Add the value at pos to the value or the acumulated value.
 *    If the value is equal to the factor, then the value from the bitpack
 *    would be zero.
 * 3.
 * 4.
 *
 * 0 ... 63 64 ... 127 128
 * b[0]     b[1]       b[2]  <-  blockvalue
 * stored = b.
 * stored[0] = 0
 *
 * increments is used in two ways.
 */
void
alder_wordtable_bitpackedcumulativearray_cumulate
(alder_wordtable_bitpackedcumulativearray_t* o)
{
    alder_dwordtable_t value = 0;
    alder_dwordtable_t blockvalue = 0;
    for (size_t pos = 0; pos < o->size; pos++) {
        size_t blockpos = pos / o->BLOCKBITS;
        size_t blocksub = pos % o->BLOCKBITS;
        alder_dwordtable_t x1 = 1;
        alder_dwordtable_t bitpos = x1 << blocksub;
        // 1
        alder_dwordtable_t uj =
        alder_wordtable_bitpackedlist_get(o->bitpack, pos);
        // 2
        value += uj;
        if ((o->increments[blockpos] & bitpos) != 0) {
            assert(uj == 0);
            value += o->factor;
            o->increments[blockpos] ^= bitpos;
        }
        // 3
        if (pos % o->BLOCKBITS == 0) {
            o->stored[blockpos] = blockvalue;
        }
        // 4
        if (value / o->factor > blockvalue) {
            blockvalue++;
            o->increments[blockpos] |= bitpos;
        }
        alder_dwordtable_t x = value % o->factor;
        alder_wordtable_bitpackedlist_set(o->bitpack, pos, x);
    }
}

/* This function returns the cumulative values.
 *
 * 1. Select bits of increments at blockpos upto the blocksub + 1 bits.
 * 2. Count set bits upto the blocksub + 1 bits.
 * 3. stored[blockpos] is the number of factors at the start of the block.
 *    long_bitcount is the number of factors upto the position.
 *    This sum is the number of factors.
 *    I'd add x to the sum to find the cumulative value at pos.
 */
size_t
alder_wordtable_bitpackedcumulativearray_get
(alder_wordtable_bitpackedcumulativearray_t* o, size_t pos)
{
    // 1
    size_t blockpos = pos / o->BLOCKBITS;
    size_t blocksub = pos % o->BLOCKBITS;
    alder_dwordtable_t x2 = 2;
    alder_dwordtable_t x1 = 1;
    alder_dwordtable_t incbits =
    o->increments[blockpos] & ((x2 << blocksub) - x1);
    // 2
    size_t Long_bitCount = alder_wordtable_bits_longBitCount((uint64_t)incbits);
    // 3
    alder_dwordtable_t x = alder_wordtable_bitpackedlist_get(o->bitpack, pos);
    size_t value = (o->stored[blockpos] + Long_bitCount) * o->factor + x;
    return value;
}


size_t
alder_wordtable_bitpackedcumulativearray_size
(alder_wordtable_bitpackedcumulativearray_t* o)
{
    return o->size;
}

#pragma mark save and load

//struct alder_wordtable_bitpackedcumulativearray_struct {
//    size_t size;                              /* number of elements */
//    size_t maxstep;                           /* */
//    size_t BLOCKBITS;                         /* 64 */
//    alder_dwordtable_t factor;
//    
//    alder_dwordtable_t *increments;           /* num bits >= size            */
//    alder_dwordtable_t *stored;               /* the same size as increments */
//    alder_wordtable_bitpackedlist_t *bitpack;
//};

/* This function writes the iRHO data chunk.
 * See alder_wordtable.io.c/h for detail.
 *
 * 8B - Length
 * 4B - iRHO
 * 8B - size
 * 8B - maxstep
 * DATA of increment : longsize x 8 - longsize = 1 + (size - 1)/64;
 * DATA of stored : longsize x 8 - longsize = 1 + (size - 1)/64;
 * DATA of bitpack : alder_wordtable_bitpackedlist_save_size
 * The following 4 items in bitpackedlist.
 * 1. 8 bytes - number of bits of an item in the list
 * 2. 8 bytes - number of items (size is the same as the one above.)
 * 3. 8 bytes - number of 64-bit values.
 * 4. DATA in 64-bit values
 */
int
alder_wordtable_bitpackedcumulativearray_save
(alder_wordtable_bitpackedcumulativearray_t* o, FILE *fp)
{
    int r = ALDER_WORDTABLE_SUCCESS;
    uint32_t crc = ALDER_WORDTABLE_IO_CRC_INIT;
    // 1
    size_t longsize = 1 + (o->size - 1)/o->BLOCKBITS;
    uint64_t lengthChunk = 2 * 8 + 2 * longsize * 8;
    lengthChunk += alder_wordtable_bitpackedlist_save_size(o->bitpack);
    // 2
    r = alder_wordtable_io_write_IRHO(fp, lengthChunk, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 3
    uint64_t size = (uint64_t)o->size;
    uint64_t maxstep = (uint64_t)o->maxstep;
    r = alder_wordtable_io_write_uint64(fp, size, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    r = alder_wordtable_io_write_uint64(fp, maxstep, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    for (size_t i = 0; i < longsize; i++) {
        r = alder_wordtable_io_write_uint64(fp, o->increments[i], &crc);
        if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    }
    for (size_t i = 0; i < longsize; i++) {
        r = alder_wordtable_io_write_uint64(fp, o->stored[i], &crc);
        if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    }
    // 4
    r = alder_wordtable_bitpackedlist_save(o->bitpack, fp, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;
    // 5
    r = alder_wordtable_io_write_uint32(fp, crc, NULL);
    if (r != ALDER_WORDTABLE_SUCCESS) return ALDER_WORDTABLE_FAIL;

    return r;
}

/* This function loads bitpackedcumulativearray from a file.
 *
 * 1. Length and IRHO type
 * 2. size and maxstep
 * 3. memory for bitpacked cumulative array
 * 4. memory for increments and stored.
 * 5. increments and stored
 * 6. load bitpack
 * 7. crc
 */
alder_wordtable_bitpackedcumulativearray_t*
alder_wordtable_bitpackedcumulativearray_load(FILE *fp)
{
    int r = ALDER_WORDTABLE_SUCCESS;
    uint32_t crc = ALDER_WORDTABLE_IO_CRC_INIT;
    uint32_t read_crc = 0;
    uint64_t read_lengthChunk = 0;
    // 1
    r = alder_wordtable_io_read_IRHO(fp, &read_lengthChunk, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    // 2
    uint64_t size = 0;
    uint64_t maxstep = 0;
    r = alder_wordtable_io_read_uint64(fp, &size, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    r = alder_wordtable_io_read_uint64(fp, &maxstep, &crc);
    if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    // 3
    alder_wordtable_bitpackedcumulativearray_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_bitpackedcumulativearray_init(o, size, maxstep);
    // 4
    size_t longsize = 1 + (o->size - 1)/o->BLOCKBITS;
    o->increments = malloc(sizeof(*o->increments) * longsize);
    o->stored = malloc(sizeof(*o->stored) * longsize);
    if (o->increments == NULL || o->stored == NULL) {
        alder_wordtable_bitpackedcumulativearray_destroy(o);
        return NULL;
    }
    memset(o->increments, 0, sizeof(*o->increments) * longsize);
    memset(o->stored, 0, sizeof(*o->stored) * longsize);
    // 5
    for (size_t i = 0; i < longsize; i++) {
        r = alder_wordtable_io_read_uint64(fp, &o->increments[i], &crc);
        if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    }
    for (size_t i = 0; i < longsize; i++) {
        r = alder_wordtable_io_read_uint64(fp, &o->stored[i], &crc);
        if (r != ALDER_WORDTABLE_SUCCESS) return NULL;
    }
    // 6
    o->bitpack = alder_wordtable_bitpackedlist_load(fp, &crc);
    if (o->bitpack == NULL) {
        alder_wordtable_bitpackedlist_destroy(o->bitpack);
        return NULL;
    }
    uint64_t lengthChunk = 2 * 8 + 2 * longsize * 8;;
    lengthChunk += (uint64_t)alder_wordtable_bitpackedlist_save_size(o->bitpack);
    if (read_lengthChunk != lengthChunk) {
        alder_wordtable_bitpackedlist_destroy(o->bitpack);
        return NULL;
    }
    // 7
    r = alder_wordtable_io_read_uint32(fp, &read_crc, NULL);
    if (r != ALDER_WORDTABLE_SUCCESS ||
        crc != read_crc) {
        alder_wordtable_bitpackedcumulativearray_destroy(o);
        return NULL;
    }
    return o;
}


