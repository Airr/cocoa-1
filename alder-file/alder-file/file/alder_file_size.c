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
#include "alder_cmacro.h"
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

/**
 *  This function returns the uncompressed file size of a gzipped file.
 *  WARN: This is not a reliable way of computing the uncompressed file size.
 *  http://stackoverflow.com/questions/9209138/uncompressed-file-size-using-zlibs-gzip-file-access-function
 *
 *  @param path file name
 *  @param size return value
 *
 *  @return SUCCESS or FAIL
 */
int alder_file_size_gz(const char *path, size_t *size)
{
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    unsigned char b4;
    
    FILE *fp = fopen(path, "rb");
    if(fp == NULL) {
        fprintf(stderr, "Error - failed to open file %s: %s",
                path, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    fseek(fp, SEEK_END, 4);
    fread(&b4, sizeof(b4), 1, fp);
    fread(&b3, sizeof(b3), 1, fp);
    fread(&b2, sizeof(b2), 1, fp);
    fread(&b1, sizeof(b1), 1, fp);
    XFCLOSE(fp);
    
    int val = (b1 << 24) | (b2 << 16) + (b3 << 8) + b4;
    *size = (unsigned long)(val);
    return 0;
}


