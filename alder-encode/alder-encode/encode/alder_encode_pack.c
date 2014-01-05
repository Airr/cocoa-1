/**
 * This file, alder_encode_pack.c, is part of alder-encode.
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

#include <stdio.h>
#include "alder_encode_pack.h"

int alder_encode_pack_midbuf_size(int k)
{
    int s = 0;
    int k_class = k % 4;
    if (k_class == 0) {
        s = 0;
    } else if (k_class == 1) {
        s = k * 3 / 4;
    } else if (k_class == 2) {
        s = k * 1 / 4;
    } else if (k_class == 3) {
        s = k * 3 / 4;
    }
    return s;
}

int alder_encode_pack_midbuf_save(int k, int j)
{
    int s = 0;
    int k_class = k % 4;
    if (k_class == 0) {
        s = 0;
    } else if (k_class == 1) {
        s = j;
    } else if (k_class == 2) {
        s = j/2;
    } else if (k_class == 3) {
        s = j * 3 % 4;
    }
    return s;
}