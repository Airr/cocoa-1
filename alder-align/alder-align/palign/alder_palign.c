//
//  alder_palign.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../union/union_array.h"
#include "gsl_vector_match.h"
#include "../fasta/alder_fasta.h"
#include "../gsl/gsl_vector_seqpair.h"
#include "../gsl/gsl_vector_anchor.h"
#include "gsl_vector_anchor.h"
#include "gsl_vector_seqpair.h"
#include "alder_align_pairwise.h"
#include "gsl_vector_sam.h"

int alder_palign(const char *s, interval_t I, gsl_vector_match *mem, fasta_t *fS)
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
    gsl_vector_seqpair *v = gsl_vector_seqpair_alloc((size_t)numberOfI);
    for (size_t i = 0; i < (size_t)numberOfI; i++) {
        int64_t startI = I[(i+1)*2];
        int64_t endI   = I[(i+1)*2 + 1];
        int64_t sizeI  = endI - startI + 1;
        alder_seqpair_t m;
        m.anchor = gsl_vector_anchor_init();
        m.seq1 = strdup(s);
        m.seq2 = malloc((sizeI+1)*sizeof(char));
        strncpy(m.seq2, fS->data + startI, sizeI); m.seq2[sizeI] = '\0';
        gsl_vector_seqpair_add(v, m);
    }
    
    for (size_t i = 0; i < mem->size; i++) {
        alder_match_t m = gsl_vector_match_get(mem, i);
        
        int64_t indexI;
        bool isWithinInterval;
        alder_union_interval_find_interval_point(I, m.ref, &indexI, &isWithinInterval);
        assert(isWithinInterval == true);
        int64_t startI = I[(indexI+1)*2];
        alder_anchor_t a;
        a.anchor1 = m.query + 1;     // anchor1 - 1-based position
        a.anchor2 = m.ref - startI;  // anchor2 - 1-based position
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
    alder_align_read_initialize();
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
        alder_align_read_execute_with_anchor(sam, sp->seq1, sp->seq2,
                                             anchor1, anchor2,
                                             anchorLength, anchorScore,
                                             numberOfAnchor);
        // Get the alignment!
        
        
        free(anchorScore);
        free(anchorLength);
        free(anchor2);
        free(anchor1);
        
        
    }
    alder_align_read_finalize();
    
    // Print alder_sam_t.
    gsl_vector_sam_fprintf(stdout, samV, 0);
    gsl_vector_sam_free(samV);
    
    gsl_vector_seqpair_free(v);
}