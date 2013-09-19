/**
 * This file, alder_seqid_file.c, is part of alder-seqid.
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "bstraux.h"
#include "bstrmore.h"
#include "alder_vector_pair.h"
#include "alder_file.h"
#include "alder_seqid_file.h"

/**
 * Argument:
 * fn - file names
 * Return:
 * An ordered file names.
 */
alder_seqid_file_t * alder_seqid_file_init(struct bstrList *fn)
{
    // Test if all of the files exist.
    // Determine the file type (text or gzip).
    // Use the first line to determine file formats (FASTA or FASTQ).
    // Read the first sequence name of each file.
    // Pair files using sequence names.
    alder_seqid_file_t *v = malloc(sizeof(alder_seqid_file_t));
    int numberOfFile = fn->qty;
    v->filename = bstrVectorCreate(numberOfFile);
    v->type = malloc(numberOfFile * sizeof(int));
    v->format = malloc(numberOfFile * sizeof(int));
    v->pair = malloc(numberOfFile * sizeof(int));
    
    for (int i = 0; i < fn->qty; i++) {
        int s = alder_file_exist(bdata(fn->entry[i]));
        if (s == 0) {
            alder_seqid_file_free(v);
            return NULL;
        }
    }
    for (int i = 0; i < fn->qty; i++) {
        int s = alder_file_isgzip(bdata(fn->entry[i]));
        if (s == 1) {
            v->type[i] = 2;
        } else {
            v->type[i] = 1;
        }
    }
    for (int i = 0; i < fn->qty; i++) {
        int s = alder_file_whatformat(bdata(fn->entry[i]));
        if (s == 1) {
            v->format[i] = 1;
        } else if (s == 2) {
            v->format[i] = 2;
        } else {
            alder_seqid_file_free(v);
            return NULL;
        }
    }
    
    struct bstrList *bnames = bstrVectorCreate(fn->qty);
    for (int i = 0; i < fn->qty; i++) {
        bstring s = alder_file_firstline(bdata(fn->entry[i]));
        int posFirstSpace = bstrchrp(s, ' ', 0);
        bstring bword = NULL;
        if (posFirstSpace != BSTR_ERR) {
            bword = bHead(s, posFirstSpace);
        } else {
            bword = bstrcpy(s);
        }
        bdestroy(s);
        bstring bname = bTail(bword, bword->slen - 1);
        bdestroy(bword);
        bstrVectorAdd(bnames, bdata(bname));
        bdestroy(bname);
    }
    
    for (int i = 0; i < fn->qty; i++) {
        v->pair[i] = -1;
        for (int j = 0; j < fn->qty; j++) {
            if (i == j) continue;
            if (!bstrcmp(bnames->entry[i], bnames->entry[j])) {
                v->pair[i] = j;
            }
        }
    }
    
    int *pairCount = malloc(numberOfFile * sizeof(int));
    memset(pairCount, 0, numberOfFile*sizeof(int));
    for (int i = 0; i < fn->qty; i++) {
        int j = v->pair[i];
        pairCount[j]++;
    }
    
    for (int i = 0; i < fn->qty; i++) {
        if (pairCount[i] > 1) {
            free(pairCount);
            alder_seqid_file_free(v);
            return NULL;
        }
    }
    free(pairCount);
    
    ///////////////////////////////////////////////////////////////////////////
    // Order pairs.
    int pairI = 0;
    alder_vector_pair_t * pair = alder_vector_pair_t_alloc(fn->qty);
    for (int i = 0; i < fn->qty; i++) {
        if (v->pair[i] >= 0) {
            int alreadPaired = 0;
            for (int j = 0; j < pairI; j++) {
                if (pair->data[j].first == i ||
                    pair->data[j].second== i) {
                    alreadPaired = 1;
                    break;
                }
            }
            if (alreadPaired == 0) {
                if (i < v->pair[i]) {
                    pair->data[pairI].first = i;
                    pair->data[pairI].second= v->pair[i];
                } else {
                    pair->data[pairI].first = v->pair[i];
                    pair->data[pairI].second= i;
                }
                pairI++;
            }
        }
    }
    for (int i = 0; i < fn->qty; i++) {
        if (v->pair[i] < 0) {
            pair->data[pairI].first = i;
            pair->data[pairI].second= -1;
            pairI++;
        }
    }
    alder_seqid_file_t *v2 = malloc(sizeof(alder_seqid_file_t));
    v2->filename = bstrVectorCreate(numberOfFile);
    v2->type = malloc(numberOfFile * sizeof(int));
    v2->format = malloc(numberOfFile * sizeof(int));
    v2->pair = malloc(numberOfFile * sizeof(int));
    int seqidIndex = 0;
    for (int i = 0; i < pairI; i++) {
        int first = pair->data[i].first;
        int second = pair->data[i].second;
        assert(first >= 0);
        bstrVectorAdd(v2->filename, bdata(fn->entry[first]));
        v2->type[seqidIndex] = v->type[first];
        v2->format[seqidIndex] = v->format[first];
        
        if (second >= 0) {
            v2->pair[seqidIndex] = seqidIndex + 1;
        } else {
            v2->pair[seqidIndex] = -1;
        }
        seqidIndex++;
        if (second >= 0) {
            bstrVectorAdd(v2->filename, bdata(fn->entry[second]));
            v2->type[seqidIndex] = v->type[second];
            v2->format[seqidIndex] = v->format[second];
            v2->pair[seqidIndex] = seqidIndex - 1;
            seqidIndex++;
        }
    }
    assert(seqidIndex == numberOfFile);
    
#if 0
    for (int i = 0; i < fn->qty; i++) {
        int j = v->pair[i];
        if (j < 0) {
            fprintf(stdout, "%2d %10s: %10s (%d) - no pair (%d)\n", i,
                    bdata(fn->entry[i]),
                    bdata(bnames->entry[i]), pairCount[i], j);
        } else {
            fprintf(stdout, "%2d %10s: %10s (%d) - pair %10s (%d)\n", i,
                    bdata(fn->entry[i]),
                    bdata(bnames->entry[i]), pairCount[i], bdata(fn->entry[j]), j);
        }
    }
    
    fprintf(stdout, "\n");
    for (int i = 0; i < fn->qty; i++) {
        fprintf(stdout, "%2d %10s: format (%d), type (%d), pair (%d)\n", i,
                bdata(v2->filename->entry[i]),
                v2->format[i], v2->type[i], v2->pair[i]);
    }
#endif
    
    alder_seqid_file_free(v);
    alder_vector_pair_t_free(pair);
    bstrVectorDelete(bnames);
    return v2;
}

void alder_seqid_file_fprintf(FILE * stream, alder_seqid_file_t *asf)
{
    if (asf == NULL) {
        return;
    }
    for (int i = 0; i < asf->filename->qty; i++) {
        fprintf(stdout, "%2d %10s: format (%d), type (%d), pair (%d)\n", i,
                bdata(asf->filename->entry[i]),
                asf->format[i], asf->type[i], asf->pair[i]);
    }
}

void alder_seqid_file_free(alder_seqid_file_t *asf)
{
    if (asf != NULL) {
        if (asf->filename != NULL) bstrListDestroy(asf->filename);
        if (asf->type != NULL) free(asf->type);
        if (asf->format != NULL) free(asf->format);
        if (asf->pair != NULL) free(asf->pair);
        free(asf);
    } else {
        assert(0);
    }
}
