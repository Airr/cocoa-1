/**
 * This file, alder_seqid_read.c, is part of alder-seqid.
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

#include <zlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include "alder_fastq.h"
#include "alder_logger.h"
#include "alder_file.h"
#include "alder_seqid_set.h"

int alder_seqid_set(const char *fn1, const char *fn2,
                    char *data, int64_t *index, const int64_t tBase,
                    int64_t *nSeq, int64_t *nBase)
{
    int fp1 = -1;
    int isGzip1 = 0;
    gzFile fpgz1 = NULL;
    int fp2 = -1;
    int isGzip2 = 0;
    gzFile fpgz2 = NULL;
    
    isGzip1 = alder_file_isgzip(fn1);
    if (isGzip1 == 1) {
        fpgz1 = gzopen(fn1, "r");
    } else {
        fp1 = open(fn1, O_RDONLY);
    }
    if (fp1 < 0 && fpgz1 == Z_NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", fn1);
        return -1;
    }
    if (fn2 != NULL) {
        isGzip2 = alder_file_isgzip(fn2);
        if (isGzip2 == 1) {
            fpgz2 = gzopen(fn2, "r");
        } else {
            fp2 = open(fn2, O_RDONLY);
        }
        if (fp2 < 0 && fpgz2 == Z_NULL) {
            logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", fn2);
            return -1;
        }
    }
    
    kseq_t *seq1 = NULL;
    kseq_t *seq2 = NULL;
    if (isGzip1 == 0) {
        seq1 = alder_kseq_init((void *)(intptr_t)fp1, isGzip1);
    } else {
        seq1 = alder_kseq_init(fpgz1, isGzip1);
    }
    if (fn2 != NULL) {
        if (isGzip2 == 0) {
            seq2 = alder_kseq_init((void *)(intptr_t)fp2, isGzip2);
        } else {
            seq2 = alder_kseq_init(fpgz2, isGzip2);
        }
    }
    
    int64_t n = 0;
    int64_t nbase = 0;
    while (alder_kseq_read(seq1) >= 0)
    {
        alder_kseq_read(seq2);
        memcpy(data+nbase, seq1->seq.s, seq1->seq.l);
        nbase += (int64_t)seq1->seq.l;
        index[n++] = tBase + nbase;
        if (fn2 != NULL) {
            memcpy(data+nbase, seq2->seq.s, seq2->seq.l);
            nbase += (int64_t)seq2->seq.l;
            index[n++] = tBase + nbase;
        }
    }
    alder_kseq_destroy(seq1);
    if (fn2 != NULL) {
        alder_kseq_destroy(seq2);
    }
    
    if (isGzip1 == 0) {
        close(fp1);
    } else {
        gzclose(fpgz1);
    }
    if (fn2 != NULL) {
        if (isGzip2 == 0) {
            close(fp2);
        } else {
            gzclose(fpgz2);
        }
    }
    
    *nSeq = n;
    *nBase = nbase;

    return 0;
}
