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

/**
 *  This function determines the type of a DNA sequence file. 
 *  I assume that the sequence is one of three formats: FASTA, FASTQ, or SEQ.
 *  A file of SEQ format simply contains a DNA sequence per line.
 *
 *  @param fn file name
 *
 *  @return one of indicators of the following:
 *    ALDER_FILETYPE_FASTA
 *    ALDER_FILETYPE_FASTQ
 *    ALDER_FILETYPE_SEQ
 *    ALDER_FILETYPE_GZFASTA
 *    ALDER_FILETYPE_GZFASTQ
 *    ALDER_FILETYPE_GZSEQ
 *    ALDER_FILETYPE_BZFASTA
 *    ALDER_FILETYPE_BZFASTQ
 *    ALDER_FILETYPE_BZSEQ
 */
int alder_fileseq_type(const char *fn)
{
    int s = alder_file_isfasta(fn);
    if (s == 1) {
        s = alder_file_isgzip(fn);
        if (s == 1) {
            return ALDER_FILETYPE_GZFASTA;
        }
        s = alder_file_isbzip2(fn);
        if (s == 1) {
            return ALDER_FILETYPE_BZFASTA;
        } else {
            return ALDER_FILETYPE_FASTA;
        }
    }
    s = alder_file_isfastq(fn);
    if (s == 1) {
        s = alder_file_isgzip(fn);
        if (s == 1) {
            return ALDER_FILETYPE_GZFASTQ;
        }
        s = alder_file_isbzip2(fn);
        if (s == 1) {
            return ALDER_FILETYPE_BZFASTQ;
        } else {
            return ALDER_FILETYPE_FASTQ;
        }
    }
    
    // So, the file is neither of FASTA nor of FASTQ format.
    // I'd assume that it contains a DNA sequence per line.
    s = alder_file_isgzip(fn);
    if (s == 1) {
        return ALDER_FILETYPE_GZSEQ;
    }
    s = alder_file_isbzip2(fn);
    if (s == 1) {
        return ALDER_FILETYPE_BZSEQ;
    } else {
        return ALDER_FILETYPE_SEQ;
    }
}
