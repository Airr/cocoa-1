/**
 * This file, alder_bit.c, is part of alder-bit.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-bit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-bit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-bit.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdint.h>
#include <assert.h>
#include "bstrlib.h"
#include "alder_bit.h"


/* This function converts the four characters of nucleotides to their
 * corresponding numbers.
 * returns
 * 0 if c is A or a,
 * 1 if c is C or c,
 * 2 if c is T or t, or
 * 3 if c is G or g.
 */
int alder_bit_acgt2int(char c)
{
    assert(c == 'A' || c == 'C' || c == 'T' || c == 'G');
    assert(c == 'a' || c == 'c' || c == 't' || c == 'g');
    return (((int)c)>>1)&3;
}

char alder_bit_int2acgt(int i)
{
    assert(i == 0 || i == 1 || i == 2 || i == 3);
    char c;
    if (i == 0) {
        c = 'A';
    } else if (i == 1) {
        c = 'C';
    } else if (i == 2) {
        c = 'T';
    } else {
        c = 'G';
    }
    return c;
}

/* Use alder-kmer-hex256 to create this array. */
static uint8_t alder_bit_four2uc_matrix[4][4] = {
    {0x00,0x00,0x00,0x00},
    {0x40,0x10,0x04,0x01},
    {0x80,0x20,0x08,0x02},
    {0xC0,0x30,0x0C,0x03}
};

/* This function converts four numbers to a unsiged int8 or char or byte.
 * e.g., 1 2 3 0 -> 01 10 11 00 = 0x6C
 */
uint8_t alder_bit_four2uc(int i1, int i2, int i3, int i4)
{
    assert(0 <= i1 && i1 < 4);
    assert(0 <= i2 && i2 < 4);
    assert(0 <= i3 && i3 < 4);
    assert(0 <= i4 && i4 < 4);
    uint8_t x = 0;
    x |= alder_bit_four2uc_matrix[i1][0];
    x |= alder_bit_four2uc_matrix[i2][1];
    x |= alder_bit_four2uc_matrix[i3][2];
    x |= alder_bit_four2uc_matrix[i4][3];
    return x;
}

/* This function converts a byte to four numbers.
 * e.g., 01 10 11 00 -> 1 2 3 0
 */
void alder_bit_uc2four(uint8_t uc, uint8_t *i1, uint8_t *i2, uint8_t *i3, uint8_t *i4)
{
    *i1 = uc & 0xC0;
    *i2 = uc & 0x30;
    *i3 = uc & 0x0C;
    *i4 = uc & 0x03;
}

/* Use alder-kmer-hex256 to create this array. */
static uint64_t alder_bit_kmer2number_matrix[32][4] = {
    {0x0000000000000000, 0x0000000000000001, 0x0000000000000002, 0x0000000000000003},
    {0x0000000000000000, 0x0000000000000004, 0x0000000000000008, 0x000000000000000C},
    {0x0000000000000000, 0x0000000000000010, 0x0000000000000020, 0x0000000000000030},
    {0x0000000000000000, 0x0000000000000040, 0x0000000000000080, 0x00000000000000C0},
    {0x0000000000000000, 0x0000000000000100, 0x0000000000000200, 0x0000000000000300},
    {0x0000000000000000, 0x0000000000000400, 0x0000000000000800, 0x0000000000000C00},
    {0x0000000000000000, 0x0000000000001000, 0x0000000000002000, 0x0000000000003000},
    {0x0000000000000000, 0x0000000000004000, 0x0000000000008000, 0x000000000000C000},
    {0x0000000000000000, 0x0000000000010000, 0x0000000000020000, 0x0000000000030000},
    {0x0000000000000000, 0x0000000000040000, 0x0000000000080000, 0x00000000000C0000},
    {0x0000000000000000, 0x0000000000100000, 0x0000000000200000, 0x0000000000300000},
    {0x0000000000000000, 0x0000000000400000, 0x0000000000800000, 0x0000000000C00000},
    {0x0000000000000000, 0x0000000001000000, 0x0000000002000000, 0x0000000003000000},
    {0x0000000000000000, 0x0000000004000000, 0x0000000008000000, 0x000000000C000000},
    {0x0000000000000000, 0x0000000010000000, 0x0000000020000000, 0x0000000030000000},
    {0x0000000000000000, 0x0000000040000000, 0x0000000080000000, 0x00000000C0000000},
    {0x0000000000000000, 0x0000000100000000, 0x0000000200000000, 0x0000000300000000},
    {0x0000000000000000, 0x0000000400000000, 0x0000000800000000, 0x0000000C00000000},
    {0x0000000000000000, 0x0000001000000000, 0x0000002000000000, 0x0000003000000000},
    {0x0000000000000000, 0x0000004000000000, 0x0000008000000000, 0x000000C000000000},
    {0x0000000000000000, 0x0000010000000000, 0x0000020000000000, 0x0000030000000000},
    {0x0000000000000000, 0x0000040000000000, 0x0000080000000000, 0x00000C0000000000},
    {0x0000000000000000, 0x0000100000000000, 0x0000200000000000, 0x0000300000000000},
    {0x0000000000000000, 0x0000400000000000, 0x0000800000000000, 0x0000C00000000000},
    {0x0000000000000000, 0x0001000000000000, 0x0002000000000000, 0x0003000000000000},
    {0x0000000000000000, 0x0004000000000000, 0x0008000000000000, 0x000C000000000000},
    {0x0000000000000000, 0x0010000000000000, 0x0020000000000000, 0x0030000000000000},
    {0x0000000000000000, 0x0040000000000000, 0x0080000000000000, 0x00C0000000000000},
    {0x0000000000000000, 0x0100000000000000, 0x0200000000000000, 0x0300000000000000},
    {0x0000000000000000, 0x0400000000000000, 0x0800000000000000, 0x0C00000000000000},
    {0x0000000000000000, 0x1000000000000000, 0x2000000000000000, 0x3000000000000000},
    {0x0000000000000000, 0x4000000000000000, 0x8000000000000000, 0xC000000000000000}
};

/* This function converts an array of numbers (0,1,2, or 3) to a single number.
 * e.g., kmer[] = 1 2 1 3 0 -> 1*(4^4) + 2*(4^3) + 1*(4^2) + 3*(4^1) + 0*(4^0)
 */
kmer_t alder_bit_kmer2number(uint8_t *kmer, size_t size)
{
    assert(size <= 32);
    kmer_t x = 0;
    for (size_t i = 0; i < size; i++) {
        assert(0 <= kmer[i] < 4);
        x |= alder_bit_kmer2number_matrix[size - i - 1][kmer[i]];
    }
    return x;
}

kmer_t alder_bit_kmerpos2number(char c, int pos)
{
    int i = (((int)c)>>1)&3;
    kmer_t x = alder_bit_kmer2number_matrix[pos][i];
    return x;
}



void alder_bit_number2kmer(bstring bkmer, uint64_t n, int k)
{
    //    bstring bkmer = bfromcstralloc (k + 1, "");
    btrunc(bkmer, 0);
    for (int i = 0; i < k; i++) {
        uint64_t x = (n & alder_bit_kmer2number_matrix[k-i-1][3]) >> (2*(k-i-1));
        bconchar(bkmer, alder_bit_int2acgt((int)x));
    }
}

static uint64_t alder_bit_1_array[64] = {
    0x0000000000000001,
    0x0000000000000002,
    0x0000000000000004,
    0x0000000000000008,
    0x0000000000000010,
    0x0000000000000020,
    0x0000000000000040,
    0x0000000000000080,
    0x0000000000000100,
    0x0000000000000200,
    0x0000000000000400,
    0x0000000000000800,
    0x0000000000001000,
    0x0000000000002000,
    0x0000000000004000,
    0x0000000000008000,
    0x0000000000010000,
    0x0000000000020000,
    0x0000000000040000,
    0x0000000000080000,
    0x0000000000100000,
    0x0000000000200000,
    0x0000000000400000,
    0x0000000000800000,
    0x0000000001000000,
    0x0000000002000000,
    0x0000000004000000,
    0x0000000008000000,
    0x0000000010000000,
    0x0000000020000000,
    0x0000000040000000,
    0x0000000080000000,
    0x0000000100000000,
    0x0000000200000000,
    0x0000000400000000,
    0x0000000800000000,
    0x0000001000000000,
    0x0000002000000000,
    0x0000004000000000,
    0x0000008000000000,
    0x0000010000000000,
    0x0000020000000000,
    0x0000040000000000,
    0x0000080000000000,
    0x0000100000000000,
    0x0000200000000000,
    0x0000400000000000,
    0x0000800000000000,
    0x0001000000000000,
    0x0002000000000000,
    0x0004000000000000,
    0x0008000000000000,
    0x0010000000000000,
    0x0020000000000000,
    0x0040000000000000,
    0x0080000000000000,
    0x0100000000000000,
    0x0200000000000000,
    0x0400000000000000,
    0x0800000000000000,
    0x1000000000000000,
    0x2000000000000000,
    0x4000000000000000,
    0x8000000000000000
};

/* This function returns the number of 0's in the highest bit order.
 */
int alder_bit_highest_bit_1(uint64_t x)
{
    int number_high_zero = 0;
    while ((x & alder_bit_1_array[63]) !=  alder_bit_1_array[63]) {
        number_high_zero++;
        x <<= 1;
    }
    return number_high_zero;
}

static uint64_t alder_bit_right_1s_array[65] = {
    0x0000000000000000,
    0x0000000000000001,
    0x0000000000000003,
    0x0000000000000007,
    0x000000000000000F,
    0x000000000000001F,
    0x000000000000003F,
    0x000000000000007F,
    0x00000000000000FF,
    0x00000000000001FF,
    0x00000000000003FF,
    0x00000000000007FF,
    0x0000000000000FFF,
    0x0000000000001FFF,
    0x0000000000003FFF,
    0x0000000000007FFF,
    0x000000000000FFFF,
    0x000000000001FFFF,
    0x000000000003FFFF,
    0x000000000007FFFF,
    0x00000000000FFFFF,
    0x00000000001FFFFF,
    0x00000000003FFFFF,
    0x00000000007FFFFF,
    0x0000000000FFFFFF,
    0x0000000001FFFFFF,
    0x0000000003FFFFFF,
    0x0000000007FFFFFF,
    0x000000000FFFFFFF,
    0x000000001FFFFFFF,
    0x000000003FFFFFFF,
    0x000000007FFFFFFF,
    0x00000000FFFFFFFF,
    0x00000001FFFFFFFF,
    0x00000003FFFFFFFF,
    0x00000007FFFFFFFF,
    0x0000000FFFFFFFFF,
    0x0000001FFFFFFFFF,
    0x0000003FFFFFFFFF,
    0x0000007FFFFFFFFF,
    0x000000FFFFFFFFFF,
    0x000001FFFFFFFFFF,
    0x000003FFFFFFFFFF,
    0x000007FFFFFFFFFF,
    0x00000FFFFFFFFFFF,
    0x00001FFFFFFFFFFF,
    0x00003FFFFFFFFFFF,
    0x00007FFFFFFFFFFF,
    0x0000FFFFFFFFFFFF,
    0x0001FFFFFFFFFFFF,
    0x0003FFFFFFFFFFFF,
    0x0007FFFFFFFFFFFF,
    0x000FFFFFFFFFFFFF,
    0x001FFFFFFFFFFFFF,
    0x003FFFFFFFFFFFFF,
    0x007FFFFFFFFFFFFF,
    0x00FFFFFFFFFFFFFF,
    0x01FFFFFFFFFFFFFF,
    0x03FFFFFFFFFFFFFF,
    0x07FFFFFFFFFFFFFF,
    0x0FFFFFFFFFFFFFFF,
    0x1FFFFFFFFFFFFFFF,
    0x3FFFFFFFFFFFFFFF,
    0x7FFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF
};

/* This function returns a bit-wise value with the first n zeros.
 */
uint64_t alder_bit_all_1_with_left_zero_of(int n)
{
    return alder_bit_right_1s_array[64 - n];
}

/**
 *  This is the function in the dsk (Rizk et al. 2013).
 *
 *  @param lkmer kmer type
 *
 *  @return kmer
 */
kmer_t alder_bit_hash(kmer_t lkmer)
{
    // some hashing to uniformize repartition
    kmer_t kmer_hash = lkmer ^ (lkmer >> 14);
    kmer_hash = (~kmer_hash) + (kmer_hash << 18);
    kmer_hash = kmer_hash ^ (kmer_hash >> 31);
    kmer_hash = kmer_hash * 21;
    kmer_hash = kmer_hash ^ (kmer_hash >> 11);
    kmer_hash = kmer_hash + (kmer_hash << 6);
    kmer_hash = kmer_hash ^ (kmer_hash >> 22);
    return kmer_hash;
}
