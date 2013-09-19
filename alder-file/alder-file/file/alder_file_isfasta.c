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
#include "alder_file_exist.h"
#include "alder_file_isgzip.h"
#include "alder_file_isfasta.h"

#define BUFLEN 16384

/**
 * Return:
 * 1 if it is a FASTA, 0 otherwise.
 * -1 if it does not exist or error.
 */
int alder_file_isfasta(const char *fn)
{
    char buf[BUFLEN];
    int status = 0;
    int s = alder_file_exist(fn);
    if (s == 0) {
        return -1;
    }
    s = alder_file_isgzip(fn);
    if (s == 1) {
        // open it with zlib.
        gzFile gz = gzopen(fn, "rb");
        if (gz == NULL) {
            return -1;
        }
        int len = gzread(gz, buf, sizeof(buf));
        if (len < 0) {
            gzclose(gz);
            return -1;
        }
        if (len > 1) {
            if (buf[0] == '>') {
                status = 1;
            } else if (buf[0] == '@') {
                status = 0;
            } else {
                status = -1;
            }
        } else {
            status = -1;
        }
        gzclose(gz);
    } else {
        // open it with a regular fopen or open.
        FILE *fp = fopen(fn, "rb");
        if (fp == NULL) return -1;
        size_t len = fread(buf, sizeof(char), BUFLEN, fp);
        if (len > 1) {
            if (buf[0] == '>') {
                status = 1;
            } else if (buf[0] == '@') {
                status = 0;
            } else {
                status = -1;
            }
        } else {
            status = -1;
        }
        fclose(fp);
    }
    
    return status;
}
