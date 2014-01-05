/**
 * This file, alder_type_sundell.c, is part of alder-type.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-type is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-type is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-type.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "alder_type_sundell.h"

#define REFERENCE_BIT_PATTERN   0x8000000000000000
#define ALL_ONE_MSB_ZERO        0x7fffffffffffffff
#define ALL_11_MSB_ZERO         0x001fffffffffffff

#pragma mark macro

/* The following function should be written in C macro. */

/* This function tests whether the value is a reference to a descriptor.
 */
bool alder_type_sundell_IS_REFERENCE(word_t x)
{
   	return (x & REFERENCE_BIT_PATTERN) == REFERENCE_BIT_PATTERN;
}

/* This function converts the word type to a pointer to a descriptor.
 * The first bit must be 1.
 * I select the last 63 bits from a number of type word_t.
 */
alder_type_sundell_descriptor_t *
alder_type_sundell_GET_DESCRIPTOR(word_t x)
{
    assert(((x >> (sizeof(x)*8-1)) & 1) == 1);
    assert(((1LLU << 63) - 1LLU) == ALL_ONE_MSB_ZERO);
    return (alder_type_sundell_descriptor_t*)(x & ALL_11_MSB_ZERO);
}

/* This function converts a pointer to descriptor to a word.
 *
 * ----------------------------------------------------------------
 * Make sure that index is less than 1024 or 2^10.
 *
 * ----------------------------------------------------------------
 * Make sure that the first 11 bits are zero.
 * code: assert((x & (((1LLU << 11) - 1LLU) << 53)) == 0);
 * 1. Create a number 1: 1LLU.
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 0000000000000000000000000000000000000000000000000000000000000001
 * 2. Shift it 10 bits to the left: 1LLU << 11.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 0000000000000000000000000000000000000000000000000000100000000000
 * 3. Subtract 1 from it: (1LLU << 11) - 1LLU.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 0000000000000000000000000000000000000000000000000000011111111111
 * 4. Shift it 53 bits to the left: ((1LLU << 11) - 1LLU) << 53.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 1111111111100000000000000000000000000000000000000000000000000000
 * 5. AND a number with the bit-wise number to make sure that 0 is the result.
 *
 * ----------------------------------------------------------------
 * Make sure that the first bit is 0.
 * code: assert((x & (1LLU << 63)) == 0);
 * 1. Create a number 1: 1LLU.
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 0000000000000000000000000000000000000000000000000000000000000001
 * 2. Shift it 10 bits to the left: 1LLU << 63.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 1000000000000000000000000000000000000000000000000000000000000000
 * 3. AND a number with the bit-wise number to make sure that you have 0.
 *
 * ----------------------------------------------------------------
 * Make sure that the index is less than 2^10 using bit operations.
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * The position has 1 and the rest of them are zero.
 * (1LLU << 10) makes the number:
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 0000000000000000000000000000000000000000000000000000010000000000
 *                                                      ^
 * (1LLU << 10) - 1LLU ~ Subtracting 1 from it makes the number:
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 0000000000000000000000000000000000000000000000000000001111111111
 *                                                      ^
 * ~((1LLU << 10) - 1LLU) ~ Negating it makes the number:
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 1111111111111111111111111111111111111111111111111111110000000000
 *                                                      ^
 * I can use it to check if the number can have 1's upto 10 bits.
 *
 * Now, let's convert the descriptor and the index to a word.
 * code: x = x | (1LLU << (sizeof(x)*8-1)) | (word_idx <<= 53);
 * 1. I have x which is the word value of the pointer.
 * word_t x = (word_t)descriptor;
 * 2. Create a bit-wise number with the first bit set.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 1000000000000000000000000000000000000000000000000000000000000000
 * 3. Shift the idx 53 bits to the left.
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 0000000000000000000000000000000000000000000000000000001111111111
 *                                                      ^
 * 0111111111100000000000000000000000000000000000000000000000000000
 * 4. OR x and the two bit-wise numbers.
 */
word_t alder_type_sundell_MAKE_REF(alder_type_sundell_descriptor_t *descriptor, int idx)
{
    assert(0 <= idx && idx < 1024);
    word_t x = (word_t)descriptor;
    assert((x & (((1LLU << 11) - 1LLU) << 53)) == 0);
    assert(((x >> (sizeof(x)*8-1)) & 1) == 0);
    assert((x & (1LLU << 63)) == 0);
    
    word_t word_idx = (word_t)idx;
    assert((word_idx & ~((1LLU << 10) - 1LLU)) == 0);
    x = x | (1LLU << (sizeof(x)*8-1)) | (word_idx <<= 53);
    return x;
}

/* This function converts a pointer to descriptor to a number.
 * The last 53 bits are the values to the pointer.
 *
 * Make sure that the first 11 bits are zero (See MAKE_REF).
 */
word_t alder_type_sundell_MAKE_DESCRIPTOR(alder_type_sundell_descriptor_t *descriptor)
{
    word_t x = (word_t)descriptor;
    assert((x & (((1LLU << 11) - 1LLU) << 53)) == 0);
    x &= (1LLU << 53) - 1LLU;
    return x;
}

/* This function converts a pointer to descriptor to a status.
 */
word_t alder_type_sundell_MAKE_STATUS_GIVE_DESCRIPTOR(alder_type_sundell_descriptor_t *descriptor)
{
    word_t x = (word_t)descriptor;
    return x*2+2;
}

/* This function converts a pointer to descriptor to a status.
 */
word_t alder_type_sundell_MAKE_STATUS_TRYING_DESCRIPTOR(alder_type_sundell_descriptor_t *descriptor)
{
    word_t x = (word_t)(descriptor->seq);
    return x*2+3;
}

/* This function returns the index value.
 * The index value is between 63 and 54:
 * 2^10=1024 possible values of index.
 *
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *  ^^^^^^^^^^
 * I want to take the 10-bit as a number of type word_t.
 * 1. Shift the number 53 bits to the right.
 * 2. Select the 10 bits.
 *
 * (1LLU << 10) - 1LLU -> be equal to 1023 or
 * the last 10 bits are set to 1, and the rest of bits are zero.
 *
 * 1. I have a number x, and want to extract the 10 bits of x.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *  ^^^^^^^^^^
 * 2. Shift the number 53 bits to the right.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                       ^^^^^^^^^^
 * 3. Create a bit-wise number with the last 10 bits set on.
 * (1LLU << 10) - 1LLU
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 0000000000000000000000000000000000000000000000000000001111111111
 * 4. AND the two numbers to extract the 10 bits of index.
 */
word_t alder_type_sundell_GET_INDEX(word_t x)
{
   	return (x >> 53) & ((1LLU << 10) - 1LLU);
}

/* This function gets the value of a word.
 * The MSB of the word must be 0.
 *
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 * ^
 * I want to check if the first bit is 0.
 * 1. Shift the number 63 (=8*8-1) bits to the right.
 * 2. Select the last bit.
 */
word_t alder_type_sundell_GET_VALUE(word_t value)
{
    assert(((value >> (sizeof(value)*8-1)) & 1) == 0);
    return value;
}

word_t alder_type_sundell_MAKE_VALUE(word_t value)
{
    return value;
}

// STATUS_TRYING<0> = 3 or status*2 + 3.
// STATUS_GIVE<0> = 2 or status*2 + 2.
int alder_type_sundell_STATUS_TRYING_N(int N)
{
    return N*2 + 3;
}
int alder_type_sundell_STATUS_GIVE_N(int N)
{
    return N*2 + 2;
}

int alder_type_sundell_IS_IN_AVOIDLIST(int x, word_t X)
{
    if ((1LLU << x) & X) return 1;
    else return 0;
}

word_t alder_type_sundell_ADD_AVOIDLIST(word_t y, int x)
{
    y |= (1LLU << x);
    return y;
}

word_t alder_type_sundell_IS_EMPTY_AVOIDLIST(word_t x)
{
    return x == 0;
}

int alder_type_sundell()
{
    printf("END OF WORD FUNCTION\n");
    return 0;
}

