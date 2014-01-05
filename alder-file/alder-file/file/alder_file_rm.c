/**
 * This file, alder_file_rm.c, is part of alder-file.
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
#include <errno.h>
#include "alder_file_exist.h"
#include "alder_file_rm.h"

/* This function removes a file.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_file_rm(const char *fn)
{
    int s = 0;
    if (alder_file_exist(fn)) {
        s = remove(fn);
        if (s != 0) {
            fprintf(stderr, "ERR : cannot delete file %s - %s\n",
                    fn, strerror(errno));
            return 1;
        }
    }
    return 0;
}
