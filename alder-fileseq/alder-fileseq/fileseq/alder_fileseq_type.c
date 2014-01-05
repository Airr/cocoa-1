/**
 * This file, alder_fileseq_type.c, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_file.h"
#include "alder_fileseq_type.h"

int alder_fileseq_type(const char *fn)
{
    int s = alder_file_isfasta(fn);
    if (s == 1) {
        s = alder_file_isgzip(fn);
        if (s == 1) {
            return ALDER_FILETYPE_GZFASTA;
        } else {
            return ALDER_FILETYPE_FASTA;
        }
    }
    s = alder_file_isfastq(fn);
    if (s == 1) {
        s = alder_file_isgzip(fn);
        if (s == 1) {
            return ALDER_FILETYPE_GZFASTQ;
        } else {
            return ALDER_FILETYPE_FASTQ;
        }
    }
    s = alder_file_isgzip(fn);
    if (s == 1) {
        return ALDER_FILETYPE_GZSEQ;
    } else {
        return ALDER_FILETYPE_SEQ;
    }
}