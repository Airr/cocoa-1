/**
 * This file, alder_hash.c, is part of alder-hash.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-hash is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-hash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-hash.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdint.h>
#include "alder_hash.h"

/**
 *  alder_hash_code01 is the function to be used in alder-kmer.
 */

/* This function returns a hash code of a 64-bit value.
 */
uint64_t alder_hash_code00(uint64_t key)
{
    uint64_t code = key;
    code = code ^ (code >> 14);
    code = (~code) + (code << 18);
    code = code ^ (code >> 31);
    code = code * 21;
    code = code ^ (code >> 11);
    code = code + (code << 6);
    code = code ^ (code >> 22);
    return code;
}

/* This function returns a hash code of an array of 64-bit values.
 */
uint64_t alder_hash_code01(uint64_t *n, int s)
{
//    uint64_t x = alder_hash_code00(n[0]);
//    for (int i = 1; i < s; i++) {
//        x += (i * alder_hash_code00(n[i]));
//    }
//    return x;
    return alder_hash_code00(n[0]) * s * (s+1) / 2;
}



//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby
// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment
// and endian-ness issues if used across multiple platforms.
// 64-bit hash for 64-bit platforms

uint64_t MurmurHash64A ( const void * key, int len, unsigned int seed )
{
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;
    
    uint64_t h = seed ^ (len * m);
    
    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len/8);
    
    while(data != end)
    {
        uint64_t k = *data++;
        
        k *= m;
        k ^= k >> r;
        k *= m;
        h ^= k;
        h *= m;
    }
    
    const unsigned char * data2 = (const unsigned char*)data;
    
    switch(len & 7)
    {
        case 7: h ^= ((uint64_t)(data2[6]) << 48);
        case 6: h ^= ((uint64_t)(data2[5]) << 40);
        case 5: h ^= ((uint64_t)(data2[4]) << 32);
        case 4: h ^= ((uint64_t)(data2[3]) << 24);
        case 3: h ^= ((uint64_t)(data2[2]) << 16);
        case 2: h ^= ((uint64_t)(data2[1]) << 8);
        case 1: h ^= (uint64_t)(data2[0]);
            h *= m;
    };
    
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    
    return h;
}

uint64_t alder_hash_code01_appleby(uint64_t *n, int s)
{
    return MurmurHash64A(n, s, 1);
}








