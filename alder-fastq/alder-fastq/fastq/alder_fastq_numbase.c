/**
 * This file, alder_fastq_numbase.c, is part of alder-fastq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fastq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fastq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "alder_fastq_numbase.h"

/**
 * Return:
 * Total number of bases in a FASTQ file.
 * -1 if errro occur.
 * 0 if the file name is empty.
 */
int64_t alder_fastq_numbase(const char *fn)
{
    if (fn == 0) {
        return 0;
    }
    // Expect that the short read is smaller than 10kbp.
    const int bufsize = 10000;
    char buf[bufsize];
    char *b;
    int64_t numbase = 0;
    size_t numbaseSequence;
    size_t numbaseQuality;
    FILE *fp = fopen(fn, "r");
    int status = 0;
    int c;
    while (EOF != (c=getc(fp))) {
        switch (c) {
            case '@':
                // The first line start of a read sequence.
                // Eat the line.
                fscanf(fp, "%*[^\n]");
                fscanf(fp, "%*c");
                b = fgets(buf, bufsize, fp);
                if (b == NULL) {
                    numbase = -1;
                    break;
                }
                numbaseSequence = strlen(b);
                if (b[numbaseSequence-1] == '\n') {
                    b[numbaseSequence-1] = '\0';
                    numbaseSequence--;
                }
                numbase += numbaseSequence;
                status = 1;
                break;
            case '+':
                // The third line start of a read sequence.
                assert(status == 1);
                status = 0;
                fscanf(fp, "%*[^\n]");
                fscanf(fp, "%*c");
                b = fgets(buf, bufsize, fp);
                if (b == NULL) {
                    numbase = -1;
                    break;
                }
                numbaseQuality = strlen(b);
                if (b[numbaseQuality-1] == '\n') {
                    b[numbaseQuality-1] = '\0';
                    numbaseQuality--;
                }
                assert(numbaseSequence == numbaseQuality);
                break;
            default:
                assert(0);
                break;
        }
    }
    fclose(fp);
    return numbase;
}
