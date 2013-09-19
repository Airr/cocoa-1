/**
 * This file, alder_seqid.c, is part of alder-seqid.
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

#include <stdio.h>
#include <stdlib.h>
#include "bstrmore.h"
#include "alder_seqid_set.h"
#include "alder_seqid_file.h"
#include "alder_seqid_number.h"
#include "alder_seqid.h"

#define ALDER_SA_PLUSK 100

alder_seqid_t * alder_seqid_init(struct bstrList *fn)
{
    alder_seqid_file_t *asf = NULL;
    asf = alder_seqid_file_init(fn);
    if (asf == NULL) {
        return NULL;
    }
//    alder_seqid_file_fprintf(stdout, asf);
    
    // Use asf to read files.
    int64_t numberOfSequence = 0;
    int64_t numberOfBase = 0;
    for (int i = 0; i < asf->filename->qty; i++) {
        int64_t nSeq = 0;
        int64_t nBase = 0;
        if (asf->pair[i] < 0) {
            alder_seqid_number(bdata(asf->filename->entry[i]), NULL,
                               &nSeq, &nBase);
        } else {
            alder_seqid_number(bdata(asf->filename->entry[i]),
                               bdata(asf->filename->entry[i+1]),
                               &nSeq, &nBase);
            i++;
        }
        numberOfSequence += nSeq;
        numberOfBase += nBase;
    }
    
    alder_seqid_t *ast = malloc(sizeof(alder_seqid_t));
    if (ast == NULL) {
        alder_seqid_file_free(asf);
        return NULL;
    }
    ast->fileIndex = NULL;
    ast->index = NULL;
    ast->data = NULL;
    ast->filename = bstrVectorCreate(fn->qty);
    if (ast->filename == NULL) {
        alder_seqid_free(ast);
        alder_seqid_file_free(asf);
        return NULL;
    }
    for (int i = 0; i < asf->filename->qty; i++) {
        bstrVectorAdd(ast->filename, bdata(asf->filename->entry[i]));
    }
    ast->numberOfBase = numberOfBase;
    ast->numberOfSequence = numberOfSequence;
    ast->sizeOfDataWithDollar = -1;
    ast->sizeCapacity = numberOfBase + ALDER_SA_PLUSK;
    ast->data = malloc(ast->sizeCapacity * sizeof(char));
    ast->index = malloc((numberOfSequence + 1)* sizeof(int64_t));
    ast->fileIndex = malloc(asf->filename->qty * sizeof(int64_t));
    if (ast->data == NULL ||
        ast->index == NULL ||
        ast->fileIndex == NULL) {
        alder_seqid_free(ast);
        alder_seqid_file_free(asf);
        return NULL;
    }
    
    // Read them all.
    int64_t tSeq = 0;
    int64_t tBase = 0;
    ast->index[0] = 0;
    for (int i = 0; i < asf->filename->qty; i++) {
        int64_t nSeq = 0;
        int64_t nBase = 0;
        if (asf->pair[i] < 0) {
            alder_seqid_set(bdata(asf->filename->entry[i]), NULL,
                            ast->data + tBase,
                            ast->index + tSeq + 1,
                            tBase,
                            &nSeq, &nBase);
        } else {
            alder_seqid_set(bdata(asf->filename->entry[i]),
                            bdata(asf->filename->entry[i+1]),
                            ast->data + tBase,
                            ast->index + tSeq + 1,
                            tBase,
                            &nSeq, &nBase);
        }
        tSeq += nSeq;
        tBase += nBase;
        if (asf->pair[i] >= 0) {
            i++;
            ast->fileIndex[i] = tBase;
        }
        ast->fileIndex[i] = tBase;
    }
    ast->data[tBase] = '\0';
    
    alder_seqid_file_free(asf);
    return ast;
}

void alder_seqid_free(alder_seqid_t *ast)
{
    if (ast->data != NULL) free(ast->data);
    if (ast->fileIndex != NULL) free(ast->fileIndex);
    if (ast->index != NULL) free(ast->index);
    if (ast->filename != NULL) bstrVectorDelete(ast->filename);
}

void alder_seqid_fprintf(FILE * stream, const alder_seqid_t *ast)
{
    fprintf(stream, "%s\n", ast->data);
}

int alder_seqid_open(alder_seqid_t *ast)
{
    ast->currentIndex = 0;
    return 0;
}

bstring alder_seqid_read(alder_seqid_t *ast)
{
    bstring bseq = NULL;
    int64_t s = ast->index[ast->currentIndex];
    int64_t e = ast->index[ast->currentIndex+1];
    bseq = blk2bstr (ast->data + s, (int)(e - s));
    ast->currentIndex++;
    return bseq;
}
void alder_seqid_close(alder_seqid_t *ast)
{
    ast->currentIndex = 0;
}
