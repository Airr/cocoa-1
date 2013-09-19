/**
 * This file, alder_file_firstline.c, is part of alder-seqid.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-seqid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-seqid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-seqid.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <zlib.h>
#include "bstraux.h"
#include "alder_file_exist.h"
#include "alder_file_isgzip.h"
#include "alder_file_firstline.h"

#define BUFLEN 16384

/**
 * Argument:
 * fn - a file name
 * Return:
 * a bstring with a line if successful, NULL otherwise.
 */
bstring alder_file_firstline(const char *fn)
{
    char buf[BUFLEN];
    bstring bline = NULL;
    int len = 0;
    int status = 0;
    int s = alder_file_exist(fn);
    if (s == 0) {
        return 0;
    }
    s = alder_file_isgzip(fn);
    if (s == 1) {
        // open it with zlib.
        gzFile gz = gzopen(fn, "rb");
        if (gz == NULL) return NULL;
        len = gzread(gz, buf, sizeof(buf));
        if (len <= 0) {
            gzclose(gz);
            return NULL;
        }
        if (len <= 0) {
            status = -1;
        }
        gzclose(gz);
    } else {
        // open it with a regular fopen or open.
        FILE *fp = fopen(fn, "rb");
        if (fp == NULL) return NULL;
        len = (int)fread(buf, sizeof(char), BUFLEN, fp);
        if (len <= 0) {
            status = -1;
        }
        fclose(fp);
    }
    if (status == 0) {
        bstring bbuffer = blk2bstr (buf, len);
        int pos = bstrchrp (bbuffer, '\n', 0);
        if (pos == BSTR_ERR) {
            bline = bstrcpy(bbuffer);
        } else {
            bline = bHead(bbuffer, pos);
        }
        bdestroy(bbuffer);
    }
    
    return bline;
}
