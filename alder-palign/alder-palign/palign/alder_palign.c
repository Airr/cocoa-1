/**
 * This file, alder_palign.c, is part of alder-palign.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-palign is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-palign is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-palign.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bstrlib.h"
#include "alder_interval.h"
#include "gsl_vector_match.h"
#include "alder_fasta.h"
#include "gsl_vector_seqpair.h"
#include "gsl_vector_anchor.h"
#include "gsl_vector_seqpair.h"
#include "gsl_vector_sam.h"
#include "alder_vector.h"
#include "alder_palign_exact.h"
#include "alder_palign_dialign.h"

gsl_vector_sam * alder_palign_align_in_sam(alder_vector_palign_t *palign,
                                           const alder_fasta_t *fS,
                                           const char *queryName)
{
    // Alignment with dialign.
    alder_palign_dialign_initialize();
    gsl_vector_sam *samV = gsl_vector_sam_alloc(palign->size);
    for (size_t i = 0; i < palign->size; i++) {
        int needToAlign = alder_vector_palign_needToAlign (palign, i);
        
        // if there is anything to align.
        // otherwise, just finish the alignment.
        alder_sam_t *sam = gsl_vector_sam_ptr(samV, i);
        if (needToAlign == 1) {
            alder_palign_dialign_execute_with_anchor(sam, palign, i, fS, queryName);
        } else {
            alder_palign_exact_execute_with_anchor(sam, palign, i, fS, queryName);
        }
        
    }
    alder_palign_dialign_finalize();
    
    // Print alder_sam_t.
    gsl_vector_sam_fprintf(stdout, samV, 0);
    ///////////////////////////////////////////////////////////////////////////
    // Filter SAM.
    ///////////////////////////////////////////////////////////////////////////
    //
    
    
//    gsl_vector_sam_free(samV);
    
    return samV;
}

#if 0

int alder_palign(const char *s, interval_t I, gsl_vector_match *mem, alder_fasta_t *fS)
{
    // Use s as the first sequence for the pairwise sequence alignment.
    // Prepare the second sequence from each of I using fS.
    // Use mem to find anchors in the pairwise alignment.
    // Perform a pairwise alignment.
    // Write CAM, SAM, and BAM files.
    
    // Use I[1] to find the number of regions.
    // Assign each mem to one of the alignment or interval as anchors.
    
    int64_t numberOfI = I[1];
    assert(numberOfI > 0);
    //    gsl_vector_seqpair *v = gsl_vector_seqpair_alloc((size_t)numberOfI);
    gsl_vector_seqpair *v = gsl_vector_seqpair_init();
    for (size_t i = 0; i < (size_t)numberOfI; i++) {
        int64_t startI = I[(i+1)*2];
        int64_t endI   = I[(i+1)*2 + 1];
        int64_t sizeI  = endI - startI + 1;
        alder_seqpair_t m;
        m.anchor = gsl_vector_anchor_init();
        bstring bS = bfromcstr(s);
        m.seq1 = bstr2cstr(bS, '\0');
        bdestroy(bS);
        m.seq2 = malloc((sizeI+1)*sizeof(char));
        strncpy(m.seq2, fS->data + startI, sizeI); m.seq2[sizeI] = '\0';
        gsl_vector_seqpair_add(v, m);
    }
    
    for (size_t i = 0; i < mem->size; i++) {
        alder_match_t m = gsl_vector_match_get(mem, i);
        
        int64_t indexI;
        bool isWithinInterval;
        alder_interval_union_find_interval_point(I, m.ref, &indexI, &isWithinInterval);
        assert(isWithinInterval == true);
        int64_t startI = I[(indexI+1)*2];
        alder_anchor_t a;
        a.anchor1 = m.query + 1;     // anchor1 - 1-based position
        a.anchor2 = m.ref - startI + 1;  // anchor2 - 1-based position
        assert(a.anchor1 == 1);
        assert(a.anchor2 == 1);
        a.anchorLength = m.len;
        a.anchorScore = 10.0;
        
        alder_seqpair_t *sp = gsl_vector_seqpair_ptr(v, (size_t)indexI);
        gsl_vector_anchor_add(sp->anchor, a);
    }
    
    //
    // alder_sam_t...
    gsl_vector_sam *samV = gsl_vector_sam_alloc((size_t)numberOfI);
    alder_palign_dialign_read_initialize();
    for (size_t i = 0; i < v->size; i++) {
        
        alder_seqpair_t *sp = gsl_vector_seqpair_ptr(v, i);
        size_t numberOfAnchor = sp->anchor->size;
        int *anchor1 = malloc(numberOfAnchor*sizeof(int));
        int *anchor2 = malloc(numberOfAnchor*sizeof(int));
        int *anchorLength = malloc(numberOfAnchor*sizeof(int));
        double *anchorScore = malloc(numberOfAnchor*sizeof(double));
        for (size_t j = 0; j < sp->anchor->size; j++) {
            anchor1[j] = sp->anchor->data[j].anchor1;
            anchor2[j] = sp->anchor->data[j].anchor2;
            anchorLength[j] = sp->anchor->data[j].anchorLength;
            anchorScore[j] = sp->anchor->data[j].anchorScore;
        }
        alder_sam_t *sam = gsl_vector_sam_ptr(samV, i);
        alder_palign_dialign_read_execute_with_anchor(sam,
                                                      fS,
                                                      0,
                                                      sp->seq1, sp->seq2,
                                             anchor1, anchor2,
                                             anchorLength, anchorScore,
                                             numberOfAnchor);
        // Get the alignment!
        
        
        free(anchorScore);
        free(anchorLength);
        free(anchor2);
        free(anchor1);
        
        
    }
    alder_palign_dialign_read_finalize();
    
    // Print alder_sam_t.
    gsl_vector_sam_fprintf(stdout, samV, 0);
    gsl_vector_sam_free(samV);
    
    gsl_vector_seqpair_free(v);
    return 0;
}

#endif
