/**
 * This file, alder_fileseq_sample.c, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_fileseq_type.h"
#include "alder_seq_sample.h"
#include "alder_fileseq_sample.h"

/* This function creates a file with sequences.
 */
int
alder_fileseq_sample(const char *fn, int type,
                     size_t n_seq, size_t l_seq)
{
    int status = 0;
    if (type == ALDER_FILETYPE_FASTA) {
        assert(0);
    } else if (type == ALDER_FILETYPE_FASTQ) {
        assert(0);
    } else if (type == ALDER_FILETYPE_SEQ) {
        status = alder_seq_sample(fn, n_seq, l_seq);
    } else {
        // No code.
        assert(0);
    }
    return status;
}

