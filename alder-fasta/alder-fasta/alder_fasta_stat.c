/**
 * This file, alder_fasta_stat.c, is part of alder-fasta.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "bstrmore.h"
#include "alder_fasta.h"
#include "alder_fasta_stat.h"

int alder_fasta_stat_single(struct bstrList *fs, struct bstrList *ofs)
{
    for (size_t i = 0; i < fs->qty; i++) {
        int64_t nSeq;
        int64_t nBase;
        alder_fasta_length(bdata(fs->entry[i]), &nSeq, &nBase);
        FILE *fp = NULL;
        if (ofs == NULL) {
            fp = stdout;
        } else {
            size_t ofsIndex = 0;
            if (ofs->qty > 1) {
                assert(fs->qty == ofs->qty);
                ofsIndex = i;
            }
            if (i == 0) {
                fp = fopen(bdata(ofs->entry[ofsIndex]), "w");
            } else {
                fp = fopen(bdata(ofs->entry[ofsIndex]), "a");
            }
        }
        fprintf(fp, "File: %s\n", bdata(fs->entry[i]));
        fprintf(fp, "  Number of sequences: %lld\n", nSeq);
        fprintf(fp, "  Number of bases: %lld\n", nBase);
        if (ofs != NULL) fclose(fp);
    }
    return 0;
}


int alder_fasta_stat_seq(struct bstrList *fs, struct bstrList *ofs)
{
    int64_t tSeq = 0;
    int64_t tBase = 0;
    for (size_t i = 0; i < fs->qty; i++) {
        int64_t nSeq = 0;
        int64_t nBase = 0;
        alder_fasta_length(bdata(fs->entry[i]), &nSeq, &nBase);
        tSeq += nSeq;
        tBase += nBase;
    }
    struct bstrList *name = bstrVectorCreate((int)tSeq);
    int64_t *length = malloc((size_t)tSeq * sizeof(int64_t));
    alder_fasta_seqname_length(fs, name, length);
    
    for (size_t i = 0; i < name->qty; i++) {
        FILE *fp = NULL;
        if (ofs == NULL) {
            fp = stdout;
        } else {
            size_t ofsIndex = 0;
            if (ofs->qty > 1) {
                assert(fs->qty == ofs->qty);
                ofsIndex = i;
            }
            if (i == 0) {
                fp = fopen(bdata(ofs->entry[ofsIndex]), "w");
            } else {
                fp = fopen(bdata(ofs->entry[ofsIndex]), "a");
            }
        }
        fprintf(fp, "%10s: %lld\n", bdata(name->entry[i]), length[i]);
        if (ofs != NULL) fclose(fp);
        
    }
    free(length);
    bstrVectorDelete(name);
    
    return 0;
}
