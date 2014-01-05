/**
 * This file, alder_file_getcwd.c, is part of alder-file.
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
#include <errno.h>
#include <string.h>
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "alder_file_getcwd.h"

/* This function returns the current working directory.
 * return
 * a bstring with a directory on success
 * nil if an error occurs
 */
bstring alder_file_getcwd()
{
    // Get the current directory.
    int bbuf_size = 1024;
    bstring bbuf = bfromcstralloc(bbuf_size, "");
    ALDER_RETURN_NULL_IF_NULL(bbuf);
    char* r_buf;
    do {
        int s = balloc (bbuf, bbuf_size);
        if (s != BSTR_OK) {
            bdestroy(bbuf);
            return NULL;
        }
        r_buf = getcwd((char*)bbuf->data, (size_t)bbuf->mlen);
        if (!r_buf) {
            if (errno == ERANGE) {
                bbuf_size *= 2;
            } else {
                bdestroy(bbuf);
                return NULL;
            }
        }
    } while (!r_buf);
    bbuf->slen = (int)strlen((const char*)bbuf->data);
    return bbuf;
}