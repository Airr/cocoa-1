/**
 * This file, alder_file_exist.c, is part of alder-file.
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
#include <sys/stat.h>
#include "alder_file_exist.h"

/**
 * This function returns 0 or SUCCESS if the file exists.
 * Otherwise, it returns -1.
 *
 * Argument:
 * fn - a file name
 * Return:
 * 1 exist, 0 not exist
 */
int alder_file_exist(const char *fn)
{
    struct stat buf;
    int status = stat(fn, &buf);
    if (status == 0) {
        status = 1;
    } else {
        status = 0;
    }
    return status;
}