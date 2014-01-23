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

// Some useful information
// http://naml.us/blog/tag/thomas-wang

/**
 *  alder_hash_code01 is the function to be used in alder-kmer.
 */

/* This function returns a hash code of a 64-bit value.
 */
/**
 *  This is the code in dsk by Rizk et al. (2013).
 *  Thomas Wang 64 bit mix hash function?
 *
 *  @param key key
 *
 *  @return hash value
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

/**
 *  This code comes from
 *  https://raw.github.com/facebook/folly/master/folly/Hash.h
 *
 *  I kept the following copy-right notice for any further information.
 */
/*
 * Copyright 2013 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Thomas Wang 64 bit mix hash function
 */

static inline uint64_t twang_mix64(uint64_t key) {
    key = (~key) + (key << 21);  // key *= (1 << 21) - 1; key -= 1;
    key = key ^ (key >> 24);
    key = key + (key << 3) + (key << 8);  // key *= 1 + (1 << 3) + (1 << 8)
    key = key ^ (key >> 14);
    key = key + (key << 2) + (key << 4);  // key *= 1 + (1 << 2) + (1 << 4)
    key = key ^ (key >> 28);
    key = key + (key << 31);  // key *= 1 + (1 << 31)
    return key;
}

/*
 * Inverse of twang_mix64
 *
 * Note that twang_unmix64 is significantly slower than twang_mix64.
 */

static inline uint64_t twang_unmix64(uint64_t key) {
    // See the comments in jenkins_rev_unmix32 for an explanation as to how this
    // was generated
    key *= 4611686016279904257U;
    key ^= (key >> 28) ^ (key >> 56);
    key *= 14933078535860113213U;
    key ^= (key >> 14) ^ (key >> 28) ^ (key >> 42) ^ (key >> 56);
    key *= 15244667743933553977U;
    key ^= (key >> 24) ^ (key >> 48);
    key = (key + 1) * 9223367638806167551U;
    return key;
}

/*
 * Thomas Wang downscaling hash function
 */

inline uint32_t twang_32from64(uint64_t key) {
    key = (~key) + (key << 18);
    key = key ^ (key >> 31);
    key = key * 21;
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);
    return (uint32_t) key;
}

/*
 * Robert Jenkins' reversible 32 bit mix hash function
 */

inline uint32_t jenkins_rev_mix32(uint32_t key) {
    key += (key << 12);  // key *= (1 + (1 << 12))
    key ^= (key >> 22);
    key += (key << 4);   // key *= (1 + (1 << 4))
    key ^= (key >> 9);
    key += (key << 10);  // key *= (1 + (1 << 10))
    key ^= (key >> 2);
    // key *= (1 + (1 << 7)) * (1 + (1 << 12))
    key += (key << 7);
    key += (key << 12);
    return key;
}

/*
 * Inverse of jenkins_rev_mix32
 *
 * Note that jenkinks_rev_unmix32 is significantly slower than
 * jenkins_rev_mix32.
 */

inline uint32_t jenkins_rev_unmix32(uint32_t key) {
    // These are the modular multiplicative inverses (in Z_2^32) of the
    // multiplication factors in jenkins_rev_mix32, in reverse order.  They were
    // computed using the Extended Euclidean algorithm, see
    // http://en.wikipedia.org/wiki/Modular_multiplicative_inverse
    key *= 2364026753U;
    
    // The inverse of a ^= (a >> n) is
    // b = a
    // for (int i = n; i < 32; i += n) {
    //   b ^= (a >> i);
    // }
    key ^=
    (key >> 2) ^ (key >> 4) ^ (key >> 6) ^ (key >> 8) ^
    (key >> 10) ^ (key >> 12) ^ (key >> 14) ^ (key >> 16) ^
    (key >> 18) ^ (key >> 20) ^ (key >> 22) ^ (key >> 24) ^
    (key >> 26) ^ (key >> 28) ^ (key >> 30);
    key *= 3222273025U;
    key ^= (key >> 9) ^ (key >> 18) ^ (key >> 27);
    key *= 4042322161U;
    key ^= (key >> 22);
    key *= 16773121U;
    return key;
}

/*
 * Fowler / Noll / Vo (FNV) Hash
 *     http://www.isthe.com/chongo/tech/comp/fnv/
 */

const uint32_t FNV_32_HASH_START = 2166136261UL;
const uint64_t FNV_64_HASH_START = 14695981039346656037ULL;

//hash = FNV_32_HASH_START
inline uint32_t fnv32(const char* s,
                      uint32_t hash) {
    for (; *s; ++s) {
        hash += (hash << 1) + (hash << 4) + (hash << 7) +
        (hash << 8) + (hash << 24);
        hash ^= *s;
    }
    return hash;
}

//uint32_t hash = FNV_32_HASH_START) {
inline uint32_t fnv32_buf(const void* buf,
                          int n,
                          uint32_t hash) {
    const char* char_buf = (const char*)(buf);
    
    for (int i = 0; i < n; ++i) {
        hash += (hash << 1) + (hash << 4) + (hash << 7) +
        (hash << 8) + (hash << 24);
        hash ^= char_buf[i];
    }
    
    return hash;
}

// uint64_t hash = FNV_64_HASH_START) {
inline uint64_t fnv64(const char* s,
                      uint64_t hash) {
    for (; *s; ++s) {
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) +
        (hash << 8) + (hash << 40);
        hash ^= *s;
    }
    return hash;
}

//uint64_t hash = FNV_64_HASH_START) {
inline uint64_t fnv64_buf(const void* buf,
                          int n,
                          uint64_t hash) {
    const char* char_buf = (const char*)(buf);
    
    for (int i = 0; i < n; ++i) {
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) +
        (hash << 8) + (hash << 40);
        hash ^= char_buf[i];
    }
    return hash;
}

/*
 * Paul Hsieh: http://www.azillionmonkeys.com/qed/hash.html
 */

#define get16bits(d) (*((const uint16_t*) (d)))

inline uint32_t hsieh_hash32_buf(const void* buf, int len) {
    const char* s = (const char*)(buf);
    uint32_t hash = len;
    uint32_t tmp;
    int rem;
    
    if (len <= 0 || buf == 0) {
        return 0;
    }
    
    rem = len & 3;
    len >>= 2;
    
    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (s);
        tmp    = (get16bits (s+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        s  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }
    
    /* Handle end cases */
    switch (rem) {
        case 3:
            hash += get16bits(s);
            hash ^= hash << 16;
            hash ^= s[sizeof (uint16_t)] << 18;
            hash += hash >> 11;
            break;
        case 2:
            hash += get16bits(s);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1:
            hash += *s;
            hash ^= hash << 10;
            hash += hash >> 1;
    }
    
    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
    
    return hash;
};

#undef get16bits


uint64_t alder_hash_code03(uint64_t n)
{
    return twang_mix64(n);
}

uint64_t alder_hash_code04(uint64_t n)
{
    return twang_unmix64(n);
}