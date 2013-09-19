/**
 * This file, alder_seqid.h, is part of alder-seqid.
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
#ifndef alder_seqid_alder_seqid_h
#define alder_seqid_alder_seqid_h

#include <stdint.h>
#include "bstrlib.h"
#include "alder_seqid_file.h"
#include "alder_seqid_set.h"
#include "alder_seqid_number.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

/**
 * This contains data from sequence files whether they are FASTA, FASTQ, or
 * something similar.
 *
 * numberOfSequence - total number of sequences.
 * numberOfBase - total number of bases.
 * sizeOfDataWithDollar - larger than numberOfBase because of dollar signs.
 * sizeCapacity - size of data.
 * index - the last position of each sequence (1-based). The last index is
 *   equal to the number of bases.
 * filename - file names. The first part of filename array is for pair-end,
 *   and the second part is for single-end.
 * fileIndex - the last position of each file (1-based). The last index is
 *   equal to the number of bases.
 * singleIndex - the start position of single-end part in data.
 * currentIndex - the index of a current sequence for reading
 * data - consists of sequences concatenated. Largely, the data array is
 *   dividied into pair-end and single-end parts. In the first pair-end
 *   part, even-index sequence is for forward read, and odd-index sequence
 *   is for reverse read. singleIndex is the start position of single-end
 *   part.
 */
typedef struct {
    int64_t numberOfSequence;
    int64_t numberOfBase;
    int64_t sizeOfDataWithDollar;
    int64_t sizeCapacity;
    int64_t *index;
    struct bstrList *filename;
    int64_t *fileIndex;
    int64_t singleIndex;
    int64_t currentIndex;
    char    *data;
} alder_seqid_t;

/**
 * I construct alder_seqid_name_t using alder_seqid_t.
 *
 * numberRead - a convenient variable
 * name - sequence names
 * comment - sequence comment
 */
typedef struct {
    int n;
    struct bstrList *name;
    struct bstrList *comment;
} alder_seqid_name_t;

alder_seqid_t * alder_seqid_init(struct bstrList *fn);
void alder_seqid_free(alder_seqid_t *ast);
void alder_seqid_fprintf(FILE * stream, const alder_seqid_t *ast);
int alder_seqid_open(alder_seqid_t *ast);
bstring alder_seqid_read(alder_seqid_t *ast);
void alder_seqid_close(alder_seqid_t *ast);

__END_DECLS

#endif /* alder_seqid_alder_seqid_h */
