/**
 * This file, alder_file_isfile.c, is part of alder-file.
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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "alder_file_isfile.h"

/**
 * Return:
 * 1 for a file, 0 otherwise.
 */
int alder_file_isfile(const char *fn)
{
    struct stat fileStat;
    if(stat(fn, &fileStat) < 0) {
        return 0;
    }
    
    int isfile = (S_ISDIR(fileStat.st_mode)) ? 0 : 1;
    return isfile;
}
