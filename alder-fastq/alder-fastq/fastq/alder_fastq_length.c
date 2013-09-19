/**
 * This file, alder_fastq_length.c, is part of alder-seqid.
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
#include "bstrlib.h"
#include "alder_file.h"
#include "alder_fastq.h"
#include "alder_logger.h"
#include "alder_fastq_length.h"

/**
 * Return:
 * n1 - number of sequences
 * n2 - number of bases
 */
int alder_fastq_length(const char* fn, int64_t *n1, int64_t *n2)
{
    int status = alder_file_isgzip(fn);
    if (status == 1) {
        status = alder_fastq_length_gzip(fn, n1, n2);
    } else {
        status = alder_fastq_length_file(fn, n1, n2);
    }
    return 0;
}

int alder_fastq_length_gzip(const char* fn, int64_t *n1, int64_t *n2)
{
    int fp = -1;
    int isGzip = 0;
    gzFile fpgz;
    isGzip = alder_file_isgzip(fn);
    if (isGzip == 1) {
        fpgz = gzopen(fn, "r");
    } else {
        fp = open(fn, O_RDONLY);
    }
    if (fp < 0 && fpgz == Z_NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", fn);
        return -1;
    }
    
    kseq_t *seq;
    if (isGzip == 0) {
        seq = alder_kseq_init((void *)(intptr_t)fp, isGzip);
    } else {
        seq = alder_kseq_init(fpgz, isGzip);
    }
    
    int64_t n = 0;
    int64_t nbase = 0;
    while (alder_kseq_read(seq) >= 0)
    {
        n++;
        nbase += (int64_t)seq->seq.l;
    }
    alder_kseq_destroy(seq);
    
    if (isGzip == 0) {
        close(fp);
    } else {
        gzclose(fpgz);
    }
    
    *n1 = n;
    *n2 = nbase;
    return 0;
}


/**
 * Return:
 * 0 on success, -1 otherwise.
 */
int alder_fastq_length_file(const char* fn, int64_t *n1, int64_t *n2)
{
    int64_t n = alder_file_numline(fn);
    if (n%4 == 0) {
        *n1 = n/4;
    } else {
        return -1;
    }
    *n2 = alder_fastq_numbase(fn);

    return 0;
}


