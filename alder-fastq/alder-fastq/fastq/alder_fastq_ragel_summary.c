/**
 * This file, alder_fastq_ragel_summary.c, is part of alder-fastq.
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
#include <zlib.h>
#include <fcntl.h>
#include "alder_logger.h"
#include "bstrlib.h"
#include "alder_file_isgzip.h"
//#include "alder_kseq.h"
#include "alder_fastq_ragel.h"
#include "alder_fastq_ragel_summary.h"

int alder_fastq_ragel_summary(struct bstrList *fs, struct bstrList *ofs)
{
    for (size_t i = 0; i < fs->qty; i++) {
        int fp = -1;
        int isGzip = 0;
        gzFile fpgz;
        isGzip = alder_file_isgzip(bdata(fs->entry[i]));
        if (isGzip == 1) {
            fpgz = gzopen(bdata(fs->entry[i]), "r");
        } else {
            fp = open(bdata(fs->entry[i]), O_RDONLY);
        }
        if (fp < 0 && fpgz == Z_NULL) {
            logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", bdata(fs->entry[i]));
            return -1;
        }
        
        rseq_t *seq;
        if (isGzip == 0) {
//            seq = alder_rseq_init((void *)(intptr_t)fp, isGzip);
            seq = alder_rseq_init(bdata(fs->entry[i]), isGzip);
        } else {
//            seq = alder_rseq_init(fpgz, isGzip);
            seq = alder_rseq_init(bdata(fs->entry[i]), isGzip);
        }
        alder_rseq_read(seq);
        
        int64_t n = 0;
        do {
            n++;
        } while (alder_rseq_read(seq) >= 0);
        alder_rseq_destroy(seq);
        
        if (fs != NULL) {
            if (isGzip == 0) {
                close(fp);
            } else {
                gzclose(fpgz);
            }
        }
        
        FILE *ofp = NULL;
        if (ofs == NULL) {
            ofp = stdout;
        } else {
            size_t ofsIndex = 0;
            if (ofs->qty > 1) {
                assert(fs->qty == ofs->qty);
                ofsIndex = i;
            }
            if (i == 0) {
                ofp = fopen(bdata(ofs->entry[ofsIndex]), "w");
            } else {
                ofp = fopen(bdata(ofs->entry[ofsIndex]), "a");
            }
        }
        fprintf(ofp, "%10s: %lld\n", bdata(fs->entry[i]), n);
        if (ofs != NULL) fclose(ofp);
    }
    return 0;
}
