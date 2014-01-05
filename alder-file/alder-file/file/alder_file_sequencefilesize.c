/**
 * This file, alder_file_sequencefilesize.c, is part of alder-file.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-file.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_file_isgzip.h"
#include "alder_file_isfasta.h"
#include "alder_file_isfastq.h"
#include "alder_file_size.h"
#include "alder_file_sequencefilesize.h"

/* This function estimates the size of sequences in a file.
 * A file can contain just sequences, FASTA format, or FASTQ format.
 * The file can be gzipped.
 * returns
 * 0 on success,
 * 1 if an error occurs.
 */
int alder_file_sequencefilesize(const char *path, unsigned long *size)
{
    int filetype = 0;
    int ALDER_FILE_FASTA = 0x0001;
    int ALDER_FILE_FASTQ = 0x0002;
    int ALDER_FILE_SEQUE = 0x0004;
    int isgzip = alder_file_isgzip(path);
    int s = alder_file_isfasta(path);
    
    if (s == 1) {
        filetype |= ALDER_FILE_FASTA;
    }
    s = alder_file_isfastq(path);
    if (s == 1) {
        filetype |= ALDER_FILE_FASTQ;
    } else if (s == -1) {
        filetype = 0;
    } else if ((filetype & ALDER_FILE_FASTA) == 0x0000) {
        filetype |= ALDER_FILE_SEQUE;
    }
    
    if (filetype & ALDER_FILE_FASTA) {
        //
        fprintf(stdout, "File %s is FASTA.\n", path);
        // Estimate nucleotides.
        //
    } else if (filetype & ALDER_FILE_FASTQ) {
        //
        fprintf(stdout, "File %s is FASTQ.\n", path);
        // Estimate nucleotides.
        //
        
    } else if (filetype & ALDER_FILE_SEQUE) {
        //
        fprintf(stdout, "File %s is just a sequence file.\n", path);
        // Estimate nucleotides.
        if (isgzip) {
            // Find the uncompressed file size.
            fprintf(stderr, "YET TO BE IMPLEMENTED!\n");
            return 1;
        } else {
            // Find the file size.
            s = alder_file_size(path, size);
            return s;
        }
    } else {
        return 1;
    }
    
    return 0;
}