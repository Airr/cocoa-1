/**
 * This file, alder_fastq_write.c, is part of alder-fastq.
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
#include <stdlib.h>
#include <assert.h>
#include "bstrlib.h"
#include "alder_cmacro.h"
#include "alder_fastq_write.h"

/* This function writes sequences in the string list sl to a FASTQ file.
 * The sequence identifier is just a number, and quality score is '?'.
 * returns
 * SUCCESS
 * given
 * fp - file pointer
 * sl - string list
 */
int
alder_fastq_write_bstrListToFILE(FILE *fp, struct bstrList *sl)
{
    for (int i = 0; i < sl->qty; i++) {
        fprintf(fp, "@%d\n%s\n+\n", i, bdata(sl->entry[i]));
        for (int j = 0; j < blength(sl->entry[i]); j++) {
            fputc('?', fp);
        }
        fputc('\n', fp);
    }
    return ALDER_STATUS_SUCCESS;
}

/* This function writes sequences in the string list sl to a FASTQ file.
 * The sequence identifier is just a number, and quality score is '?'.
 * returns
 * SUCCESS
 * given
 * fn - filename
 * sl - string list
 */
int
alder_fastq_write_bstrListToFilename(const char *fn, struct bstrList *sl)
{
    FILE *fp = fopen(fn, "w");
    if (fp == NULL) {
        return 1;
    }
    alder_fastq_write_bstrListToFILE(fp, sl);
    fclose(fp);
    return ALDER_STATUS_SUCCESS;
}