/**
 * This file, alder_fastq.c, is part of alder-fastq.
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
#include <stdint.h>
#include <zlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "alder_logger.h"
#include "alder_file_isgzip.h"
#include "alder_kseq.h"
#include "alder_fastq.h"

/*
 * Argument:
 * fn - a FASTQ file name
 * Return:
 * NULL if I could not open the FASTQ file.
 * alder_fastq_t if I successfully open the FASTQ file.
 */
alder_fastq_t * alder_fastq_open(const char *fn)
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
    
    kseq_t *seq = NULL;
    assert(isGzip == 0 || isGzip == 1);
    if (isGzip == 0) {
        seq = alder_kseq_init((void *)(intptr_t)fp, isGzip);
    } else {
        seq = alder_kseq_init(fpgz, isGzip);
    }
    if (seq == NULL) {
        if (isGzip == 0) { close(fp); } else { gzclose(fpgz); }
        logc_logErrorBasic(ERROR_LOGGER, 0, "cannot open %s.\n", fn);
        return NULL;
    }
    
    alder_fastq_t * afd = malloc(sizeof(*afd));
    if (afd == NULL) {
        alder_kseq_destroy(seq);
        if (isGzip == 0) { close(fp); } else { gzclose(fpgz); }
        logc_logErrorBasic(ERROR_LOGGER, 0, "cannot open %s.\n", fn);
        return NULL;
    }
    memset(afd, 0, sizeof(*afd));
    afd->type = isGzip;
    afd->seq = seq;
    if (isGzip == 0) {
        afd->f = (void *) (intptr_t) fp;
    } else {
        afd->f = fpgz;
    }
 
    return afd;
}

/*
 * Return:
 *   NULL if nothing to read.
 *   kseq_t* if there is something to read.
 */
kseq_t * alder_fastq_read(alder_fastq_t *f)
{
    if (f == NULL) return NULL;
    int status = alder_kseq_read(f->seq);
    if (status < 0) {
        return NULL;
    } else {
        return f->seq;
    }
}

/*
 * Close alder_fastq_t by deleting kseq and close the file associated with it.
 * Free alder_fastq_t as well.
 */
void alder_fastq_close(alder_fastq_t *f)
{
    if (!f) return;
    alder_kseq_destroy(f->seq);
    if (f->type == 0) {
        close((int)((intptr_t)f->f));
    } else {
        gzclose(f->f);
    }
    free(f);
}

kseq_t * alder_fastq_init (const char *fn)
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
    assert(isGzip == 0 || isGzip == 1);
    if (isGzip == 0) {
        seq = alder_kseq_init((void *)(intptr_t)fp, isGzip);
    } else {
        seq = alder_kseq_init(fpgz, isGzip);
    }

    return seq;
}
