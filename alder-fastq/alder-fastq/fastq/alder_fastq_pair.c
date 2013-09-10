/**
 * This file, alder_fastq_pair.c, is part of alder-fastq.
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
#include <zlib.h>
#include <fcntl.h>
#include <assert.h>
#include "bstrlib.h"
#include "alder_fastq_kseq.h"
#include "alder_logger.h"
#include "alder_file_isgzip.h"
#include "alder_vector_pair.h"
#include "alder_fastq_pair.h"

int alder_fastq_nopair(alder_vector_pair_t *v)
{
    for (int i = 0; i < v->size; i++) {
        v->data[i].first = i;
        v->data[i].second = -1;
    }
    return 0;
}


int alder_fastq_pair(struct bstrList *sl, alder_vector_pair_t *v)
{
    assert(sl->qty == (int)v->size);
    
    size_t pairIndex = 0;
    for (int i = 0; i < sl->qty; i++) {
        int isPaired = 0;
        for (int j = 0; j < i; j++) {
            if (v->data[j].first == i || v->data[j].second == i) {
                isPaired = 1;
                break;
            }
        }
        if (isPaired == 1) continue;
        
        bstring b = sl->entry[i];
        bstring readname = alder_fastq_readname(bdata(b), 1);
        isPaired = 0;
        for (int j = 0; j < sl->qty; j++) {
            if (i == j) continue;
            bstring b2 = sl->entry[j];
            bstring readname2 = alder_fastq_readname(bdata(b2), 1);
            if (!bstrcmp(readname, readname2)) {
                v->data[pairIndex].first = i;
                v->data[pairIndex].second = j;
                pairIndex++;
                isPaired = 1;
                bdestroy(readname2);
                break;
            }
            bdestroy(readname2);
        }
        if (isPaired == 0) {
            v->data[pairIndex].first = i;
            v->data[pairIndex].second = -1;
            pairIndex++;
        }
        bdestroy(readname);
    }
    for (int i = (int)pairIndex; i < sl->qty; i++) {
        v->data[i].first = -1;
        v->data[i].second = -1;
    }
    assert(pairIndex <= sl->qty);
    
    return 0;
}

// Get k-th read name in the FASTQ file.
// Return:
//   bstring - name of the read.
//   NULL - if k is larger than the number of reads in the FASTQ.
bstring alder_fastq_readname(const char *fn, size_t k)
{
    int fp = -1;
    int isGzip = 0;
    gzFile fpgz;
    if (fn == NULL) {
        fp = STDIN_FILENO;
    } else {
        isGzip = alder_file_isgzip(fn);
        if (isGzip == 1) {
            fpgz = gzopen(fn, "r");
        } else {
            fp = open(fn, O_RDONLY);
        }
    }
    if (fp < 0 && fpgz == Z_NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", fn);
        return NULL;
    }
    
    kseq_t *seq;
    if (isGzip == 0) {
        seq = alder_kseq_init((void *)(intptr_t)fp, isGzip);
    } else {
        seq = alder_kseq_init(fpgz, isGzip);
    }
    size_t i = 0;
    for (i = 0; i < k; i++) {
        if (0 < alder_kseq_read(seq)) break;
    }
    bstring b = i+1 == k ? bfromcstr(seq->name.s) : NULL;
    alder_kseq_destroy(seq);
    if (fn != NULL) {
        if (isGzip == 0) {
            close(fp);
        } else {
            gzclose(fpgz);
        }
    }
    return b;
}

