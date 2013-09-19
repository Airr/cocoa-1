/**
 * This file, alder_denovo_concat.c, is part of alder-denovo.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-denovo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-denovo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-denovo.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "alder_fasta.h"
#include "alder_file.h"
#include "alder_fastq.h"
#include "alder_denovo_concat.h"

/**
 * This function uses one or two FASTQ files to create a concatenated 
 * sequence. I use alder_fastq_concat module to concatenate read sequences. 
 * Then I create alder_fasta_t not alder_fastq_concat_t. I need index and read,
 * which are the same as index and data in alder_fasta_t.
 */
alder_fasta_t * alder_denovo_concat(const char *fn1, const char *fn2)
{
    // NOTE: I have to merge alder_fasta_t and alder_fasta_concat_t.
    //
    // We make a bstrList with the two file names, and call
    // alder_sparse_sa_alloc_file.
    // But, we create a alder_sparse_sa_t.
    //
    // Here is the structure of the code flow:
    // We need a structure that is common to FASTA and the fS variable in
    // alder_sparse_sa_t type. The name alder_fasta_t is not a good one
    // because the data of a suffix array could be decoupled from the original
    // data file format. In other words, files could be FASTA or FASTQ. This
    // is where I have trouble. So, I need a one more layer between fS and
    // file formats. I also need a converter from file format to fS.
    // FASTQ and FASTA have two common variables: sequence name, and sequence.
    // FASTQ has base quality scores, and the number of reads tend to be large.
    // FASTA has no base quality scores, and tends to have smaller number of
    // sequences although it is not always the case. Sequence names are not
    // useful till we reference them. As long as we have an organized
    // reference point to which file we should look at, it is okay.
    // The reference point consists of input files and the order of them;
    // just an array of file names and the ordered index array should serve
    // the duty of a reference point.
    // Let's create a module called ... seqid.
}

/**
 * This function is similar to that of alder_fastq_concat_init. The difference
 * is the type of the return.
 */
alder_fasta_concat_t * alder_denovo_concat_init(const char *fn1, const char *fn2)
{
    const int64_t moreSpace = 100;
    
    if (fn1 == NULL) return NULL;
    int64_t numLine1 = alder_file_numline(fn1);
    if (numLine1%4 != 0) {
        // The number of lines can be divided by 4.
        return NULL;
    }
    int64_t numLine2 = 0;
    if (fn2 != NULL) {
        numLine2 = alder_file_numline(fn2);
        if (numLine1 != numLine2) {
            return NULL;
        }
    }
    
    alder_fasta_concat_t *af = malloc(sizeof(alder_fasta_concat_t));
    if (af == NULL) return NULL;
    
    af->numberRead = (numLine1 + numLine2)/4;
    int64_t numberBase1 = alder_fastq_numbase(fn1);
    int64_t numberBase2 = alder_fastq_numbase(fn2);
    af->numberBase = numberBase1 + numberBase2;
    af->sizeCapacity = af->numberBase + moreSpace;
    af->sizeWithDollar = -1;
   
    if (fn2 == NULL) {
        af->type = 0;
    } else {
        af->type = 1;
    }
    af->fixedLength = 0; // no fixed length
    af->read = malloc(af->sizeCapacity * sizeof(char));
    if (af->read == NULL) {
        free(af);
        return NULL;
    }
    af->index = malloc(af->numberRead * sizeof(int64_t));
    if (af->index == NULL) {
        free(af->read);
        free(af);
        return NULL;
    }
    int status = (int)alder_fastq_concat_read(fn1, af->read, NULL, af->index,
                                              numberBase1);
    if (status != 0) {
        free(af->index);
        free(af->read);
        free(af);
        return NULL;
    }
    if (fn2 != NULL) {
        status = (int)alder_fastq_concat_read(fn2, af->read + numberBase1,
                                              NULL, af->index + numberBase1,
                                              numberBase2);
    }
    if (status != 0) {
        free(af->index);
        free(af->read);
        free(af);
        return NULL;
    }
    
    return af;
}
