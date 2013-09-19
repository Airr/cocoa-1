/**
 * This file, alder_file_numline.c, is part of alder-file.
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
#include "alder_file_numline.h"

/**
 * Return:
 * >0 if successful, -1 otherwise.
 */
int64_t alder_file_numline(const char *fn)
{
    FILE *fp = fopen(fn, "r");
    if (fp == NULL) {
        return -1;
    }
    int64_t lines = 0;
    while (EOF != (fscanf(fp,"%*[^\n]"), fscanf(fp,"%*c"))) ++lines;
    fclose(fp);
    return lines;
}
