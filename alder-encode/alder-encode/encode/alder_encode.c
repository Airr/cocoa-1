/**
 * This file, alder_encode.c, is part of alder-encode.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-encode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-encode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-encode.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "alder_encode.h"

/* This function stores a 64bit number in char array.
 */
int alder_encode_uint64_to_8char(char *dest, uint64_t source)
{
    for (int i = 0; i < 8; i++) {
        dest[7-i] = (char)(source >> 8*i);
    }

    return 0;
}

/* This function converts char array to a 64bit number.
 */
int alder_encode_8char_to_uint64(uint64_t *dest, char *source)
{
    uint64_t x = 0;
    for (int i = 0; i < 8; i++) {
        x |= (uint64_t)source[i];
        if (i < 7) {
            x <<= 8;
        }
    }
    *dest = x;

    return 0;
}