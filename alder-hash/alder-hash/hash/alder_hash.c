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
    uint64_t x = alder_hash_code00(n[0]);
    for (int i = 1; i < s; i++) {
        x += (i * alder_hash_code00(n[i]));
    }
    return x;
}