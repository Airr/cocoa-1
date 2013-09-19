/**
 * This file, alder_file_isgzip.c, is part of alder-file.
 *
 * Copyright 2013 by Sang Chul Choi
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
#include "alder_file_isgzip.h"

/**
 * Return:
 * -1 could not open the file fn.
 * 1  file fn is a gzip file.
 * 0  file fn is not a gzip file.
 * 0  file fn is nil.
 */
int alder_file_isgzip(const char *fn)
{
    if (fn == NULL) {
        return 0;
    }
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        return -1;
    }
    unsigned char c1, c2;
    size_t n = fread (&c1, sizeof (char), 1, fp);
    if (n != 1) return -1;
    n = fread (&c2, sizeof (char), 1, fp);
    if (n != 1) return -1;
    fclose(fp);
    
    if (c1 == 0x1f && c2 == 0x8b) {
        return 1;
    } else {
        return 0;
    }
}
