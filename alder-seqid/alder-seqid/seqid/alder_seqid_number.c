/**
 * This file, alder_seqid_number.c, is part of alder-seqid.
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
#include "alder_file.h"
#include "alder_fasta.h"
#include "alder_fastq.h"
#include "alder_seqid_number.h"

int alder_seqid_number(const char *fn1, const char *fn2,
                       int64_t *nSeq, int64_t *nBase)
{
    
    int64_t nSeq_;
    int64_t nBase_;
    int s = alder_file_whatformat(fn1);
    if (s == 1) {
        alder_fasta_length(fn1, &nSeq_, &nBase_);
    } else if (s == 2) {
        alder_fastq_length(fn1, &nSeq_, &nBase_);
    } else {
        return -1;
    }
    *nSeq = nSeq_;
    *nBase = nBase_;
    
    if (fn2 != NULL) {
        int s = alder_file_whatformat(fn2);
        if (s == 1) {
            alder_fasta_length(fn2, &nSeq_, &nBase_);
        } else if (s == 2) {
            alder_fastq_length(fn2, &nSeq_, &nBase_);
        } else {
            return -1;
        }
        *nSeq += nSeq_;
        *nBase += nBase_;
    }
    return 0;
}
