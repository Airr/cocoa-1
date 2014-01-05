/**
 * This file, alder_file_size.c, is part of alder-file.
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

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include "bstrmore.h"
#include "alder_file_size.h"

/* This function finds the size of a file.
 * returns
 * 0 on succes.
 * 1 on failure.
 */
int alder_file_size(const char *path, size_t *size)
{
    struct stat fileStat;
    if(stat(path, &fileStat) < 0) {
        fprintf(stderr, "Error - cannot find the info of file %s: %s",
                path, strerror(errno));
        return 1;
    }
    *size = (unsigned long)(fileStat.st_size);
    return 0;
}

/* This function computes total size of files.
 */
int alder_totalfile_size(struct bstrList *infile, size_t *size)
{
    *size = 0;
    for (int i = 0; i < infile->qty; i++) {
        size_t filesize = 0;
        alder_file_size(bdata(infile->entry[i]), &filesize);
        *size += filesize;
    }
    return 0;
}

