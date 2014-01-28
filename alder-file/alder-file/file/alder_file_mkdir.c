/**
 * This file, alder_file_mkdir.c, is part of alder-file.
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
#include <errno.h>
#include <unistd.h>
#include "bstrlib.h"
#include "alder_cmacro.h"
#include "alder_file_exist.h"
#include "alder_file_isfile.h"
#include "alder_file_getcwd.h"
#include "alder_file_mkdir.h"

/* This function creates directories.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_file_mkdir(const char *path)
{
    int s = 0;
    mode_t mode = ALDER_FILE_DEFAULT_MODE;

    bstring bpath = bfromcstr(path);
    if (bpath == NULL) {
        return ALDER_STATUS_ERROR;
    }
    if (bpath->data[0] != '/') {
        // Get the current directory.
        // The following four lines can tolerate any errors.
        bstring bbuf = alder_file_getcwd();
        bconchar(bbuf, '/');
        bconcat(bbuf, bpath);
        s = bassign(bpath, bbuf);
        if (s == BSTR_ERR) {
            bdestroy(bpath);
            return ALDER_STATUS_ERROR;
        }
        bdestroy(bbuf);
    }
    
    bstring bslash = bfromcstr("/");
    if (bslash == NULL) {
        bdestroy(bpath);
        return ALDER_STATUS_ERROR;
    }
    int pslash = binstr(bpath, 1, bslash);
    while (pslash != BSTR_ERR) {
        // Check if the directory exists.
        bpath->data[pslash] = '\0';
        int s = alder_file_exist((const char*)bpath->data);
        if (s == 0) {
            if(mkdir((const char*)bpath->data, mode) == -1 && errno != EEXIST) {
                bdestroy(bslash);
                bdestroy(bpath);
                return ALDER_STATUS_ERROR;
            }
        }
        bpath->data[pslash] = '/';
        pslash = binstr(bpath, pslash + 1, bslash);
        
    }
    bdestroy(bslash);
    
    s = alder_file_exist(bdata(bpath));
    if (s == 0) {
        if(mkdir(bdata(bpath), mode) == -1 && errno != EEXIST) {
            bdestroy(bpath);
            return ALDER_STATUS_ERROR;
        }
    } else {
        s = alder_file_isfile(bdata(bpath));
        if (s == 1) {
            fprintf(stderr, "Not a directory: %s\n", bdata(bpath));
            bdestroy(bpath);
            return ALDER_STATUS_ERROR;
        }
    }
    bdestroy(bpath);
    return ALDER_STATUS_SUCCESS;
}
