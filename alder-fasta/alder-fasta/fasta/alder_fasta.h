/**
 * This file, alder_fasta.h, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_fasta_alder_fasta_h
#define alder_fasta_alder_fasta_h

#include <stdio.h>
#include <stdint.h>
#include "bstrlib.h"
#include "alder_fasta_kmer.h"
#include "alder_fasta_streamer.h"
#include "alder_fasta_sequenceiterator.h"
#include "alder_fasta_chunk.h"

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
 * This structure is the same as that of alder_fastq_concat_t except that
 * it does not have qual. See alder_fastq_concat_t for the description of 
 * variables.
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
} alder_fasta_concat_t;

typedef struct {
    int     type;
    int     fixedLength;
    int64_t numberOfSequence;
    int64_t numberOfBase;
    int64_t sizeOfDataWithDollar;
    int64_t sizeCapacity;
    int64_t *index;
    struct  bstrList *name;
    char    *data;
} alder_fasta_t;

int alder_fasta_length(const char* fn, int64_t *n1, int64_t *n2);
int alder_fasta_read(const char* fn, alder_fasta_t *pf);
alder_fasta_t *alder_fasta_alloc(const char *fn);
void alder_fasta_free(alder_fasta_t *pf);
int alder_fasta_seqname_length(const struct bstrList* fn, struct bstrList *name, int64_t *length);
bstring alder_fasta_name_by_position(const alder_fasta_t *af, int64_t position);

alder_fasta_t *alder_fasta_list_alloc(const struct bstrList *fn,
                                      const int64_t tSeq,
                                      const int64_t tBase,
                                      const int64_t plusK);
void alder_fasta_list_free(alder_fasta_t *pf);

int alder_fasta_list_length(const struct bstrList *fs, int64_t *n1, int64_t *n2);
//int alder_fasta_list_read(const struct bstrList *fs, alder_fasta_t* pf);


char * alder_fasta_name_by_saindex(const alder_fasta_t *af, int64_t position);
int64_t alder_fasta_index_by_saindex(const alder_fasta_t *af, int64_t position);
int64_t alder_fasta_position_by_saindex(const alder_fasta_t *af, int64_t position);

// Kmer
int alder_fasta_count_kmer(const char *fn, int kmer_size,
                           size_t *curBufsize, size_t totalBufsize,
                           uint64_t *volume,
                           int progress_flag,
                           int progressToError_flag);

__END_DECLS


#endif /* alder_fasta_alder_fasta_h */
