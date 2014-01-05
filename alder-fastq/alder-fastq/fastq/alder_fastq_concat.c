/**
 * This file, alder_fastq_concat.c, is part of alder-fastq.
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
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "alder_file.h"
#include "alder_fastq_concat.h"
#include "alder_fastq_numbase.h"

/**
 * Return:
 * a concatenated FASTQ if successful, NULL otherwise.
 */
alder_fastq_concat_t * alder_fastq_concat_init(const char *fn1,
                                               const char *fn2)
{
    const int64_t moreSpace = 100;
    int64_t numLine = alder_file_numline(fn1);
    alder_fastq_concat_t *afc = malloc(sizeof(*afc));
    if (afc == NULL) {
        return NULL;
    }
    memset(afc, 0, sizeof(*afc));
    if (numLine%4 != 0) {
        // The number of lines can be divided by 4.
        free(afc);
        return NULL;
    }
    afc->numberRead = numLine/4;
    afc->numberBase = alder_fastq_numbase(fn1);
    afc->sizeCapacity = afc->numberBase + moreSpace;
    afc->sizeWithDollar = -1;
    afc->type = 0; // single-end data
    afc->fixedLength = 0; // no fixed length
    afc->read = malloc(afc->sizeCapacity * sizeof(*afc->read));
    if (afc->read == NULL) {
        free(afc);
        return NULL;
    }
    memset(afc->read,0,afc->sizeCapacity * sizeof(*afc->read));
    afc->qual = malloc(afc->sizeCapacity * sizeof(*afc->qual));
    if (afc->qual == NULL) {
        free(afc->read);
        free(afc);
        return NULL;
    }
    memset(afc->qual,0,afc->sizeCapacity * sizeof(*afc->qual));
    afc->index = malloc(afc->numberRead * sizeof(*afc->index));
    if (afc->index == NULL) {
        free(afc->qual);
        free(afc->read);
        free(afc);
        return NULL;
    }
    memset(afc->index,0,afc->numberRead * sizeof(*afc->index));
    int status = (int)alder_fastq_concat_read(fn1, afc->read, afc->qual, afc->index,
                                              afc->numberBase);
    if (status != 0) {
        free(afc->index);
        free(afc->qual);
        free(afc->read);
        free(afc);
        return NULL;
    }
    
    return afc;
}

/**
 * Argument:
 * fn - output file name
 * afc - a concatenated FASTQ
 * append - 0 for no, 1 for yes.
 */
void alder_fastq_concat_fprintf(const char *fn, alder_fastq_concat_t *afc,
                                  int append)
{
    FILE *fp;
    if (fn == NULL) {
        fp = stdout;
    } else {
        if (append == 0) {
            fp = fopen(fn, "w");
        } else {
            fp = fopen(fn, "a");
        }
    }
//    
//typedef struct {
//    int     type;
//    int     fixedLength;
//    int64_t numberRead;
//    int64_t numberBase;
//    int64_t sizeWithDollar;
//    int64_t sizeCapacity;
//    int64_t *index;
//    char    *read;
//    char    *qual;
//} alder_fastq_concat_t;
    
    fprintf(fp,
            "# type: %d\n"
            "# fixedLength: %d\n"
            "# numberRead: %lld\n"
            "# numberBase: %lld\n"
            "# sizeWithDollar: %lld\n"
            "# sizeCapacity: %lld\n",
            afc->type, afc->fixedLength, afc->numberRead,
            afc->numberBase, afc->sizeWithDollar, afc->sizeCapacity);
    fprintf(fp, "%s\n", afc->read);
    if (afc->qual != NULL) {
        fprintf(fp, "%s\n", afc->qual);
    }
    
    fprintf(fp, "0");
    for (size_t i = 0; i < afc->numberRead; i++) {
        fprintf(fp, "-%lld", afc->index[i]);
    }
    fprintf(fp, "\n");
    
    if (fn != NULL) {
        fclose(fp);
    }
}

/**
 *
 */
void alder_fastq_concat_free(alder_fastq_concat_t *afc)
{
    if (afc != NULL) {
        if (afc->read != NULL) free(afc->read);
        if (afc->qual != NULL) free(afc->qual);
        if (afc->index != NULL) free(afc->index);
        free(afc);
    }
}

/**
 * Argument: 
 * fn - a FASTQ file name
 * read - memory allocated for read sequences
 * qual - memory for quality scores (can be NULL)
 * s - toal size of the bases
 * Return:
 * 0 if successful, -1 otherwise.
 */
int alder_fastq_concat_read(const char *fn,
                            char *read,
                            char *qual,
                            int64_t *index,
                            const int64_t s)
{
    // Expect that the short read is smaller than 10kbp.
    const int bufsize = 10000;
    char buf[bufsize];
    char *b;
    int64_t numbase = 0;
    size_t numbaseSequence;
    size_t numbaseQuality;
    int64_t numRead = 0;
    
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
                    status = -1;
                    break;
                }
                numbaseSequence = strlen(b);
                if (b[numbaseSequence-1] == '\n') {
                    numbaseSequence--;
                } else {
                    status = -1;
                    break;
                }
                memcpy(read + numbase, b, numbaseSequence);
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
                    status = -1;
                    break;
                }
                numbaseQuality = strlen(b);
                if (b[numbaseQuality-1] == '\n') {
                    numbaseQuality--;
                } else {
                    status = -1;
                    break;
                }
                if (qual != NULL) {
                    memcpy(qual + numbase, b, numbaseQuality);
                }
                if (numbaseSequence != numbaseQuality)
                {
                    status = -1;
                    break;
                }
                numbase += numbaseSequence;
                index[numRead++] = numbase;
                break;
            default:
                assert(0);
                break;
        }
    }
    fclose(fp);
    if (status == 0) {
        read[numbase] = '\0';
        if (qual != NULL) {
            qual[numbase] = '\0';
        }
    }
    return status;
}

