/**
 * This file, alder_file_isfasta.c, is part of alder-seqid.
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
#include "alder_cmacro.h"
#include "bzlib.h"
#include "alder_file_exist.h"
#include "alder_file_isgzip.h"
#include "alder_file_isbzip2.h"
#include "alder_file_isfasta.h"

/**
 *  This function tests whether a file is a FASTA format file.
 *
 *  @param fn file name
 *
 *  @return ALDER_YES if so, ALDER_NO otherwise.
 */
int alder_file_isfasta(const char *fn)
{
    char buf[ALDER_BUFSIZE_100];
    int status = 0;
    size_t readLen = 0;
    
    int s = alder_file_exist(fn);
    if (s == 0) {
        return ALDER_NO;
    }
    s = alder_file_isgzip(fn);
    int s2 = alder_file_isbzip2(fn);
    if (s == 1) {
        // open it with zlib.
        gzFile gz = gzopen(fn, "rb");
        if (gz == NULL) {
            return ALDER_NO;
        }
        int len = gzread(gz, buf, sizeof(buf));
        if (len < 0) {
            gzclose(gz);
            return ALDER_NO;
        }
        readLen = len;
        gzclose(gz);
    } else if (s2 == 1) {
        // open it with bzlib.
        BZFILE *bz = BZ2_bzopen(fn, "rb");
        if (bz == NULL) {
            return ALDER_NO;
        }
        int len = BZ2_bzread(bz, buf, sizeof(buf));
        if (len < 0) {
            BZ2_bzclose(bz);
            return ALDER_NO;
        }
        readLen = len;
        BZ2_bzclose(bz);
    } else {
        // open it with a regular fopen or open.
        FILE *fp = fopen(fn, "rb");
        if (fp == NULL) return ALDER_NO;
        readLen = fread(buf, sizeof(char), sizeof(buf), fp);
        fclose(fp);
    }
    
    /* I just check the first character. */
    if (readLen > 1) {
        if (buf[0] == '>') {
            status = ALDER_YES;
        } else {
            status = ALDER_NO;
        }
    } else {
        status = ALDER_NO;
    }
    
    return status;
}
