/**
 * This file, alder_file_isbinary.c, is part of alder-file.
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
#include <string.h>
#include "alder_file_isbinary.h"

static const int bufsize = 10000;

/**
 * Return:
 * 0 if the file is possibly a text file.
 * 1 if the file is possibly a binary file.
 */
int alder_file_isbinary(const char *fn)
{
    int isbinary = 1;
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        return 0;
    }
    unsigned char buf[bufsize];
    size_t n = fread(buf, sizeof(unsigned char), bufsize, fp);
    if (n == 0) {
        return 0;
    }
//    fprintf(stdout, "n:%zd\n", n);
    void *b = memchr(buf, '\0', n);
    if (b == NULL) {
        isbinary = 0;
    }
    fclose(fp);
    return isbinary;
}
