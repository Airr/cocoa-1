/**
 * This file, alder_file_islz4.c, is part of alder-file.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_file_islz4.h"

/**
 *  This function tests if a file is of the lz4 format.
 *  4 bytes Little endian - 0x184D2204
 *
 *  @param fn file name
 *
 *  @return YES or NO
 */
int
alder_file_islz4(const char *fn)
{
    if (fn == NULL) {
        return ALDER_FILE_NO;
    }
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        return ALDER_FILE_NO;
    }
    unsigned char c1, c2, c3, c4;
    size_t n = fread (&c1, sizeof(c1), 1, fp);
    if (n != 1) return ALDER_FILE_NO;
    n = fread (&c2, sizeof (c2), 1, fp);
    if (n != 1) return ALDER_FILE_NO;
    n = fread (&c3, sizeof (c3), 1, fp);
    if (n != 1) return ALDER_FILE_NO;
    n = fread (&c4, sizeof (c4), 1, fp);
    if (n != 1) return ALDER_FILE_NO;
    fclose(fp);
    
    if (c1 == 0x04 && c2 == 0x22 && c3 == 0x4d && c4 == 0x18) {
        return ALDER_FILE_YES;
    } else {
        return ALDER_FILE_NO;
    }
}
