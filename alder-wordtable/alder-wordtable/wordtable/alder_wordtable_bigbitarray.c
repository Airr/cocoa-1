/**
 * This file, alder_wordtable_bigbitarray.c, is part of alder-wordtable.
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

/* This creates an array of bits.  The size of bits is given on creating
 * the object. Use set function to set a bit at a position, and call get 
 * function to check a bit value at a position.
 *
    alder_wordtable_bigbitarray_t *b = alder_wordtable_bigbitarray_create(133);
    alder_wordtable_bigbitarray_set(b, 2);
    alder_wordtable_bigbitarray_set(b, 132);
    bool isSet = alder_wordtable_bigbitarray_get(b, 2);
    size_t c = alder_wordtable_bigbitarray_countBits(b);
    alder_wordtable_bigbitarray_destroy(b);
 *
 * Note.
 * 1. These functions are used in kfmmerger.
 *
 * TODO.
 * 1. Allow to change the number of blocks when creating a bit-array.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_bits.h"
#include "alder_wordtable_compute.h"
#include "alder_wordtable_bigbitarray.h"

struct alder_wordtable_bigbitarray_struct {
    size_t maxsize;         /* GIVEN: total number of bits  */
    size_t subarraysize;    /* GIVEN: #_block in a subarray */
    size_t blocksize;       /* FIXED: #_bits in a block     */
    size_t subarraybits;    /* subarraysize x blocksize     */
    size_t blockarraysize;  /* #_subarrays                  */
    alder_dwordtable_t **blockarray;
};

void
alder_wordtable_bigbitarray_destroy(alder_wordtable_bigbitarray_t *o)
{
    if (o != NULL) {
        if (o->blockarray != NULL) {
            for (size_t i = 0; i < o->blockarraysize; i++) {
                XFREE(o->blockarray[i]);
            }
            XFREE(o->blockarray);
        }
        XFREE(o);
    }
}

alder_wordtable_bigbitarray_t*
alder_wordtable_bigbitarray_create(size_t size)
{
    alder_wordtable_bigbitarray_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    /* init */
    o->maxsize = size;
    o->blocksize = sizeof(alder_dwordtable_t) * ALDER_WORDTABLE_KFMDEFAULTS_BYTESIZE;
    size_t x1 = 1;
    o->subarraysize = x1 << ALDER_WORDTABLE_BIGBITARRAY_SUBARRAYSIZE;
    o->subarraybits = o->subarraysize * o->blocksize;
    
    o->blockarraysize =
    alder_wordtable_compute_groupmaxsize(size, o->subarraybits);
    
    /* memory */
    o->blockarray = malloc(sizeof(*o->blockarray) * o->blockarraysize);
    if (o->blockarray == NULL) {
        alder_wordtable_bigbitarray_destroy(o);
        return NULL;
    }
    memset(o->blockarray, 0, sizeof(*o->blockarray) * o->blockarraysize);
    for (size_t i = 0; i < o->blockarraysize; i++) {
        o->blockarray[i] = malloc(sizeof(*o->blockarray[i]) * o->subarraysize);
        if (o->blockarray[i] == NULL) {
            alder_wordtable_bigbitarray_destroy(o);
            return NULL;
        }
        memset(o->blockarray[i], 0, sizeof(*o->blockarray[i]) * o->subarraysize);
    }
    return o;
}

void
alder_wordtable_bigbitarray_set(alder_wordtable_bigbitarray_t *o, size_t pos)
{
    size_t blockpos = pos / o->subarraybits;
    size_t blocksub = pos % o->subarraybits;
    size_t bitpos = blocksub / o->blocksize;
    size_t bitsub = blocksub % o->blocksize;
    alder_dwordtable_t x1 = 1;
    o->blockarray[blockpos][bitpos] |= (x1 << bitsub);
}

bool
alder_wordtable_bigbitarray_get(alder_wordtable_bigbitarray_t *o, size_t pos)
{
    size_t blockpos = pos / o->subarraybits;
    size_t blocksub = pos % o->subarraybits;
    size_t bitpos = blocksub / o->blocksize;
    size_t bitsub = blocksub % o->blocksize;
    alder_dwordtable_t x1 = 1;
    if (o->blockarray[blockpos][bitpos] & (x1 << bitsub)) {
        return true;
    } else {
        return false;
    }
}

size_t
alder_wordtable_bigbitarray_countBits(alder_wordtable_bigbitarray_t *o)
{
    size_t count = 0;
    for (size_t i = 0; i < o->blockarraysize; i++) {
        for (size_t j = 0; j < o->subarraysize; j++) {
            uint64_t x = o->blockarray[i][j];
            count += alder_wordtable_bits_longBitCount(x);
        }
    }
    return count;
}

void
alder_wordtable_bigbitarray_print(alder_wordtable_bigbitarray_t *o)
{
    bool isExit = false;
    bstring bs = bfromcstralloc((int)o->maxsize * 10, " ");
    alder_dwordtable_t x1 = 1;
    for (size_t i = 0; i < o->blockarraysize; i++) {
        for (size_t j = 0; j < o->subarraybits; j++) {
            size_t pos = j + i * o->subarraybits;
            if (j + i * o->subarraybits == o->maxsize) {
                isExit = true;
                break;
            }
            size_t k = j / o->blocksize;
            size_t kbit = j % o->blocksize;
            alder_dwordtable_t x = o->blockarray[i][k];
            
            bformata(bs, "[%zu] ", pos);
            if (x & (x1 << kbit)) {
                bconchar(bs, '1');
            } else {
                bconchar(bs, '0');
            }
            bconchar(bs, ' ');
        }
        if (isExit == true) {
            break;
        }
    }
    alder_log5("bit: %s", bdata(bs));
    bdestroy(bs);
}









