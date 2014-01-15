/**
 * This file, alder_file_creatable.c, is part of alder-file.
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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "alder_file_exist.h"
#include "alder_file_creatable.h"

/* Argument: 
 * A string for a file path.
 * Return:
 * A string for the directory of the file path.
 */
static char * alder_path_basedir(const char *fn)
{
    size_t len_fn = strlen(fn);
    char *dn = calloc(len_fn+1, sizeof(char));
    memcpy(dn, fn, len_fn);
    int k = (int)len_fn;
    for (; k >= 0; k--) {
        if (dn[k] == '/') {
            break;
        }
    }
    if (k == -1) {
        dn[0] = '.';
        dn[1] = '\0';
    } else {
        dn[k] = '\0';
    }
    return dn;
}

// Descripion:
// A file or directory is creatable if that file or directory does not exist,
// and the user has the write permission to the base directory.
// Argument:
// fn - a file path
// Return:
// 0 - if writable
// <0 - if not writable
int alder_file_creatable(const char *fn)
{
    char *dn = alder_path_basedir(fn);
    
    int status = alder_file_exist(fn);
    if (status == 1) {
        free(dn);
        return -1;
    }
    
    struct stat fileStat;
    status = stat(dn, &fileStat);
    if (!(status < 0)) {
        if (S_ISDIR(fileStat.st_mode)) {
            if ((fileStat.st_mode & S_IWUSR && fileStat.st_mode & S_IXUSR) ||
                (fileStat.st_mode & S_IWGRP && fileStat.st_mode & S_IXGRP) ||
                (fileStat.st_mode & S_IWOTH && fileStat.st_mode & S_IXOTH)) {
                status = 0;
            } else {
                status = -1;
            }
        } else {
            status = -1;
        }
    }
    
    free(dn);
    return status;
}
