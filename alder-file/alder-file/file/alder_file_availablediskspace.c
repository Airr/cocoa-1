/**
 * This file, alder_file_availablediskspace.c, is part of alder-file.
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

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include "alder_file_availablediskspace.h"

/* This function returns available disk space in bytes.
 * returns
 * 0 on success,
 * 1 on some error.
 */
int alder_file_availablediskspace(const char *path, unsigned long *space)
{
    struct statvfs buf;
    int s = statvfs(path, &buf);
    if (s != 0) {
        fprintf(stderr, "Error - cannot compute the available space. %s at line %d: %s\n",
                __FILE__, __LINE__, strerror(errno));
        return 1;
    }
    *space = buf.f_bavail * buf.f_frsize;
    return 0;
}

