/**
 * This file, alder_fasta_cat.c, is part of alder-fasta.
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
#include "alder_fasta_cat.h"

int alder_fasta_cat(struct bstrList *fs, struct bstrList *ofs)
{
    int64_t tSeq = 0;
    int64_t tBase = 0;
//    for (size_t i = 0; i < fs->qty; i++) {
//        int64_t nSeq = 0;
//        int64_t nBase = 0;
//        alder_fasta_length(bdata(fs->entry[i]), &nSeq, &nBase);
//        tSeq += nSeq;
//        tBase += nBase;
//    }
    
    alder_fasta_list_length(fs, &tSeq, &tBase);

    
    alder_fasta_t *catFasta = alder_fasta_list_alloc(fs, tSeq, tBase, 3);
    
    for (size_t i = 0; i < catFasta->numberOfSequence; i++) {
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
        fprintf(fp, "%10s: %lld\n",
                bdata(catFasta->name->entry[i]),
                catFasta->index[i]);
        if (i == catFasta->numberOfSequence - 1) {
            fprintf(fp, "Length: %zd == %lld\n", strlen(catFasta->data), tBase);
            fprintf(fp, "%s\n", catFasta->data);
        }
        if (ofs != NULL) fclose(fp);
    }
    alder_fasta_list_free(catFasta);

    return 0;
 
}
