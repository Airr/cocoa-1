/**
 * This file, alder_wordtable_bits.c, is part of alder-wordtable.
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

/* This file provides functions for managing bits.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "bstrlib.h"
#include "alder_cmacro.h"
#include "alder_wordtable_bits.h"
#include "alder_wordtable_kfmdefaults.h"

#if defined(ALDER_WORDTABLE_WORDSIZE_32)
#ifndef INT_LOG2BITS
#define INT_LOG2BITS 5
#endif
#ifndef INT_BITS
#define INT_BITS 32
#endif
#elif defined(ALDER_WORDTABLE_WORDSIZE_64)
#ifndef INT_LOG2BITS
#define INT_LOG2BITS 6
#endif
#ifndef INT_BITS
#define INT_BITS 64
#endif
#endif

/* This function returns the number of bits needed for a number x.
 * e.g., 8 (binary 1000) -> 4
 *
 * Note that log2 might not work as you wish if the parameter x is too large.
 * This is why this function takes a different form from the others.
 * I chose 33 with no reason. 
 * Note also that left shift on signed integer might be undefined in some
 * compiler.
 */
size_t alder_wordtable_bits_bitSize_uint64(uint64_t x)
{
    if (x > 4294967296) {
        x >>= 33;
        return (size_t)ceil(log2(x + 1)) + 33;
    } else if (x + 1 != 0) {
        return (size_t)ceil(log2(x + 1));
    } else {
        return 64;
    }
}
/* This function returns the number of bits needed for a number x.
 * e.g., 8 (binary 1000) -> 4
 *
 * Note that this works for 32-bit number but not 64-bit number.
 */
size_t alder_wordtable_bits_bitSize_uint32(uint32_t x)
{
    return (size_t)ceil(log2((double)x + 1));
}
/* A number is represented as a binary. The largest place in the number is
 * used to find the number of zeros in a variable. Say, a binary number of
 * type byte is given: 8. Its binary form is 0001 1000. The number of leading
 * zeros is 3. This function returns the number of bits needed for a number x,
 * which is 5.
 */
size_t alder_wordtable_bits_bitSizeOfWord(alder_wordtable_t x)
{
#if defined(ALDER_WORDTABLE_WORDSIZE_32)
    return alder_wordtable_bits_bitSize_uint32(x);
#elif defined(ALDER_WORDTABLE_WORDSIZE_64)
    return alder_wordtable_bits_bitSize_uint64(x);
#endif
}

/* This function returns the number of bits needed for storing numbers
 * ranging from 0 to x-1.
 * returns 0..64.
 */
size_t alder_wordtable_bits_log2ceil(size_t x)
{
    assert(x > 1);
    if (x == 0 || x == 1) {
        return 0;
    } else {
        return (size_t)ceil(log2((double)x));
    }
}

/* This function returns a value with lower 'bits' significant bits set.
 * e.g., bits 3 -> 00...000111
 * called in inedgelist.
 */
alder_wordtable_t alder_wordtable_bits_intMaskLo(size_t bits)
{
    alder_wordtable_t x = 1;
    return (x << bits) - x;
}

/*
 */
int alder_wordtable_bits_shiftRight(alder_wordtable_t *arr,
                                    size_t arrLength,
                                    int rightshift,
                                    int insert)
{
    return alder_wordtable_bits_shiftRight2(arr, arrLength, rightshift, insert);
    
}

/* This function push a value of 'insert' into the array arr from left to
 * right. Note that the array looks like this:
 *
 * insert    [0]     [1]
 * 0000xx -> 100110  0111yy
 *
 * [0]     [1]
 * xx1001  100111
 *
 * This looks like shift-right operation in BIG-endian machine.
 *
 * --- FROM RODLAND13 ---
 * Shift the values of an integer array, starting at pos and continuing for
 * length positions, rightshift bits to the right, inserting the insert value 
 * at the left and returning the value that was shifted out on the right.
 * --- FROM RODLAND13 ---
 *
 * See this line-by-line to see what this function does.
 * It inserts 2-bit value from the left, and returns the last 2-bit value
 * of the last element of the array arr.
 * arr = [0] 100110 [1] 0111yy
 * INT_BITS = 6
 * insert = 0000xx
 * WE WANT a new arr = [0] xx1001 [1] 100111
 * MSB of size 2 at arr[0], or xx, is inserted.
 * LSB of size 2 at arr[1], or yy, is gone.
 *
 * Let's see what each line of the codes does.
 * rightshift = 2
 * x = 000001
 * 000100 <- x << rightshift
 * rightmask = 000011
 * leftshift = 6 - 2 = 4
 * arr = [0] 100110 [1] 0111yy
 * i = 0
 *   right = arr[i] & rightmask = 100110 & 000011 = 000010
 *   x = arr[i] >> rightshift = 100110 >> 2 = 001001
 *   y = insert << leftshift = 0000xx << 4 = xx0000
 *   arr[i] = x | y = xx1001
 *   insert = right = 000010
 * i = 1
 * ...
 */
int
alder_wordtable_bits_shiftRight2(alder_wordtable_t *arr,
                                 size_t length,
                                 int rightshift,
                                 int insert)
{
    alder_wordtable_t x = 1;
    alder_wordtable_t rightmask = (x << rightshift) - x;
    int leftshift = INT_BITS - rightshift;
    for (size_t i = 0; i < length; i++) {
        int right = arr[i] & rightmask;
        alder_wordtable_t x = arr[i] >> rightshift;
        alder_wordtable_t y = ((alder_wordtable_t)insert) << leftshift;
        arr[i] = x | y;
        insert = right;
    }
    return insert;
}

/*
 */
int alder_wordtable_bits_shiftLeft(alder_wordtable_t *arr,
                                   size_t arrLength,
                                   int leftshift,
                                   int insert)
{
    return alder_wordtable_bits_copyShiftLeft(arr, 0, arr, 0,
                                              arrLength,
                                              leftshift, insert);
}

/*
 */
int alder_wordtable_bits_shiftLeft2(alder_wordtable_t *arr,
                                    size_t pos,
                                    size_t arrLength,
                                    int leftshift,
                                    int insert)
{
    return alder_wordtable_bits_copyShiftLeft(arr, pos, arr, pos,
                                              arrLength, leftshift, insert);
}

/*
 *
 * --- FROM RODLAND13 ---
 * Copy bits from array into given position in other array, shifting it 
 * by the given number of bits. Will perform the copying from right to left.
 * --- FROM RODLAND13 ---
 *
 */
//
// This function ... let's see what it does.
//
// from = [0] 11101010 [1] yy011010 [2] 01110111 [3] 10001001
// fromPos = 1
// to   = [0] 11101010 [1] 01101001 [2] 11011110 [3] 001001xx
// toPos = 1
// len = 3
// leftshift = 2
// insert = 000000xx
// INT_LOG2BITS = 3 (because 2^3 = 8, which is the number of bits in a word.
// INT_BITS = 8
//
// Consider two cases: 1) leftshift is less than 8, and 2) larger than 7.
// Case 1.
// leftshift = 2 = 00000010
// shiftpos = leftshift >> INT_LOG2BITS = 00000010 >> 3 = 0
// adjpos = toPos - shiftpos = toPos - 0 = 0
// leftshift = leftshift & (INT_BITS - 1)
//           = 2 & (8 - 1) = 00000010 & (00001000 - 1)
//           = 00000010 & 00000111 = 00000010
//           = 2
// rightshift = 8 - leftshift = 8 - 2 = 6
// FOR-LOOP 1 does not take effect, because shiftpos = 0.
// FOR-LOOP 2.
// i = len or 3.
//   tmp = from[fromPos + len] >> rightshift
//       = from[0 + 3] >> 6 = 10001001 >> 6 = ------10
//   x = from[fromPos + len] << leftshift
//     = from[0 + 3] << 2 = 10001001 << 2 = 001001--
//   insert = 000000xx
//   to[adjpos + i] = to[0 + len] = to[len]
//                  = x | insert = 001001-- | 000000xx
//                  = 001001xx
//   insert = tmp = ------10
// i = 2
//   tmp = from[fromPos + 2] >> rightshift
//       = from[2] >> rightshift = 01110111 >> 6 = ------01
//   x = from[2] << leftshift = 01110111 << 2 = 110111--
//   insert = ------10
//   to[adjpos + i] = to[2]
//                  = x | insert = 110111-- | ------10
//                  = 11011110
//   insert = tmp = ------01
// i = 1
//   tmp = from[1] >> rightshift
//       = yy011010 >> 6 = ------yy
//   x = from[1] << leftshift
//     = yy011010 << 2 = 011010--
//   insert = ------10
//   to[1] = x | insert
//         = 011010-- | ------10 = 01101010
//   insert = tmp = ------yy
//
// Case 2.
// Consider two cases: 1) leftshift is less than 8, and 2) larger than 7.
// from = [0] 11101010 [1] -------- [2] yy110111 [3] 10001001
// fromPos = 0
// to   = [0] 11101010 [1] 11011110 [2] 001001-- [3] ------xx
// toPos = 0
// len = 3
// leftshift = 10
// insert = ------xx
// INT_LOG2BITS = 3 (because 2^3 = 8, which is the number of bits in a word.
// INT_BITS = 8
//
// leftshift = 10 = 00001010
// shiftpos = leftshift >> 3 = ---00001 = 1
// adjpos = toPos - 1 = -1
// leftshift = leftshift & (8 - 1) = 00001010 & 00000111 = -----010 = 2
// rightshift = 6
// FOR-LOOP 1.
// i = 1
//   to[adjpos + len + i] = to[-1 + 3 + 1]
//                        = to[3] = ------xx
//   insert = --------
// FOR-LOOP 2.
// i = len = 3
//   tmp = from[fromPos + 3] >> rightshift = from[3] >> 6
//       = 10001001 >> 6 = ------10
//   x = from[3] << leftshift = 10001001 << 2 = 001001--
//   insert = --------
//   to[adjpos + 3] = to[-1 + 3] = to[2]
//                  = 001001-- | -------- = 001001--
//   insert = tmp = ------10
// i = 2
//   tmp = from[2] >> rightshift
//       = yy110111 >> 6 = ------yy
//   x = from[2] << leftshift = yy110111 << 2 = 110111--
//   insert = ------10
//   to[-1 + 2] = to[1] = 110111-- | ------10 = 11011110
//   insert = tmp = ------yy
//
// Let me consider more cases:
// Case 3.
// from = [0] 11101010 [1] -------- [2] -------- [3] 10001001
// fromPos = 0
// len = 3
// leftshift = 16
// insert = ------xx
// to   = [0] 11101010 [1] 10001001 [2] -------- [3] ------xx
// toPos = 0
// return insert = 0
//
// shiftpos = 10000 = ---10 = 2
// adjpos = -2
// leftshift = 10000 & 00111 = 0
// rightshift = 8
// FOR-LOOP 1.
// to[-2 + 3 + 2] = to[3] = insert
// to[-2 + 3 + 1] = to[2] = 0
// FOR-LOOP 2.
// to[-2 + 3] = to[1] = (from[len] << 0) | insert = 10001001
// insert = tmp = from[len] >> 8 = 0
//
// Case 4.
// leftshift = 18
// from = [0] 11101010 [1] -------- [2] -------- [3] yy001001
// fromPos = 0
// len = 3
// leftshift = 16
// insert = ------xx
// to   = [0] 11101010 [1] 001001-- [2] -------- [3] ------xx
// toPos = 0
// return insert = ------yy
//
int alder_wordtable_bits_copyShiftLeft(alder_wordtable_t *from, size_t fromPos,
                                       alder_wordtable_t *to, size_t toPos,
                                       size_t len,
                                       int leftshift,
                                       int insert)
{
    assert(len > 0);
    assert(leftshift > 0);
    assert(leftshift <= INT_BITS * len);
    size_t shiftpos = leftshift >> INT_LOG2BITS;
    // toPos is less than shiftpos.
    
    leftshift = leftshift & (INT_BITS - 1);
    int rightshift = INT_BITS - leftshift;
    
    int insert_t = insert;
    /* FOR-LOOP 1 */
    if (shiftpos > 0) {
        insert_t = 0;
    }
    
    /* FOR-LOOP 2 */
    int tmp;
    for (int i = (int)len - 1; i >= (int)shiftpos; i--) {
        tmp = from[fromPos + i] >> rightshift;
        alder_wordtable_t x = from[fromPos + i] << leftshift;
        to[i + toPos - shiftpos] = x | insert_t;
        insert_t = tmp;
    }
    insert = insert_t;
    for (int i = (int)shiftpos - 1; i >= 0; i--) {
        to[i + len + toPos - shiftpos] = insert;
        insert = 0;
    }
    return insert;
}

alder_wordtable_t alder_wordtable_bits_MAX_SAFE_INT()
{
    return UINT32_MAX - 100;
}

#pragma mark print

/* This function returns 1 if a bit at pos in arr is set.
 * Otherwise, it returns 0.
 */
int alder_wordtable_bits_bit(alder_wordtable_t *arr,
                             size_t arrLength,
                             size_t pos)
{
    // mbits: number of bits per element in arr.
    // nbits: total number of bits in arr
    size_t mbits = sizeof(*arr) * ALDER_WORDTABLE_KFMDEFAULTS_BYTESIZE;
    size_t nbits = mbits * arrLength;
    assert(pos < nbits);
    size_t x = pos / mbits;
    size_t y = pos % mbits;
    alder_wordtable_t x1 = 1;
    return (int)((arr[x] >> y) & x1);
}

/* This function returns a bstring of the arr in bitwise format.
 */
bstring
alder_wordtable_bits_asString(alder_wordtable_t *arr, size_t arrLength)
{
    char c;
    const alder_wordtable_t x1 = 1;
    size_t mbits = sizeof(*arr) * ALDER_WORDTABLE_KFMDEFAULTS_BYTESIZE;
    size_t nbits = mbits * arrLength;
    bstring bs = bfromcstralloc((int)nbits + 1, "");
    for (size_t i = 0; i < arrLength; i++) {
        for (size_t j = mbits - 1; j != ALDER_WORDTABLE_MAX_SIZE_T; j--) {
            if ((arr[i] >> j) & x1) {
                c = '1';
            } else {
                c = '0';
            }
            bconchar(bs, c);
            if (j % 8 == 0) {
                bconchar(bs, ' ');
            }
        }
    }
    return bs;
}



size_t alder_wordtable_bits_arrayLength(size_t x)
{
    return x;
}

/* This function 
 */
const static uint64_t width_1  = 0x5555555555555555;
const static uint64_t width_2  = 0x3333333333333333;
const static uint64_t width_4  = 0x0f0f0f0f0f0f0f0f;
const static uint64_t width_8  = 0x00ff00ff00ff00ff;
const static uint64_t width_16 = 0x0000ffff0000ffff;
const static uint64_t width_32 = 0x00000000ffffffff;
size_t alder_wordtable_bits_longBitCount(uint64_t x)
{
    x = (x & width_1 ) + ((x >>  1) & width_1 );
    x = (x & width_2 ) + ((x >>  2) & width_2 );
    x = (x & width_4 ) + ((x >>  4) & width_4 );
    x = (x & width_8 ) + ((x >>  8) & width_8 );
    x = (x & width_16) + ((x >> 16) & width_16);
    x = (x & width_32) + ((x >> 32) & width_32);
    return (size_t)x;
}

/* See
 * http://stackoverflow.com/questions/6234533/how-to-find-the-leading-number-of-zeros-in-a-number-using-c
 */
/* The following functions seems to be called by 
 * alder_wordtable_intblocklist_nBitsEqual
 */
/* This was used with bitSizeOfLong.
 */
size_t alder_wordtable_bits_LongNumberOfLeadingZeros(int64_t x)
{
    return 64 - alder_wordtable_bits_bitSize_uint64(x);
}
/* This seems to be a Java-provided function.
 * This was used with bitSizeOfInt.
 */
size_t alder_wordtable_bits_IntNumberOfLeadingZeros(int32_t x)
{
    return 32 - alder_wordtable_bits_bitSize_uint32(x);
}
/* could be called by alder_wordtable_intblocklist_nBitsEqual
 */
size_t alder_wordtable_bits_bitSizeOfLong(int64_t x)
{
    return alder_wordtable_bits_bitSize_uint64(x);
}
/* called by alder_wordtable_intblocklist_nBitsEqual
 */
size_t alder_wordtable_bits_bitSizeOfInt(int32_t x)
{
    return alder_wordtable_bits_bitSize_uint32(x);
}

#undef INT_LOG2BITS
#undef INT_BITS
