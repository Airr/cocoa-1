/**
 * This file, alder_fastq_concat.h, is part of alder-fastq.
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

#ifndef alder_fastq_alder_fastq_concat_h
#define alder_fastq_alder_fastq_concat_h

#include <stdint.h>

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
 * type - 0: single-end, 1: pair-end, for pair-end data.
 *   I thought that I could use the following scheme: even-index is for the
 *   forward read, and odd-index is for the reverse read.
 *   But, reading function could be complicated. So, I have the first part
 *   for forward, and the second part for reverse.
 * fixedLength - 0: not a fixed length, >0: fixed length. For fast access to
 *   sequence or quality score data, we could fix the length of each read even
 *   if lengths of read sequences vary. The remaining part of a read is filled
 *   with zeros.
 * numberRead - total number of read sequences.
 * numberBase - total number of bases.
 * sizeWithDollar - larger than numberBase because of additional dollar signs.
 *   This is for creating a suffix array.
 * sizeCapacity - the size of memory allocated for data and qual.
 * index - 1-based last positions of read sequences.
 * data - if fixedLength is 0, data contains a concatenated sequence. 
 *   Otherwise, read sequences with filled with zeros are concatenated.
 * qual - this contains the corresponding quality scores.
 */
typedef struct {
    int     type;
    int     fixedLength;
    int64_t numberRead;
    int64_t numberBase;
    int64_t sizeWithDollar;
    int64_t sizeCapacity;
    int64_t *index;
    char    *read;
    char    *qual;
} alder_fastq_concat_t;

/**
 * The first line of a read consists of a name and a comment.
 * numberRead - a convenient variable
 * name - read names
 * comment - read comments
 */
typedef struct {
    int64_t numberRead;
    struct bstrList *name;
    struct bstrList *comment;
} alder_fastq_concat_name_t;


alder_fastq_concat_t * alder_fastq_concat_init(const char *fn1,
                                               const char *fn2);
void alder_fastq_concat_fprintf(const char *fn, alder_fastq_concat_t *afc,
                                  int append);
void alder_fastq_concat_free(alder_fastq_concat_t *afc);
int alder_fastq_concat_read(const char *fn,
                            char *read,
                            char *qual,
                            int64_t *index,
                            const int64_t s);

__END_DECLS


#endif /* alder_fastq_alder_fastq_concat_h */
