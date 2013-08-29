//
//  alder_palign.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "../union/union_array.h"
#include "../gsl_vector_match.h"
#include "../fasta/alder_fasta.h"

int alder_palign(const char *s, interval_t I, gsl_vector_match *mem, fasta_t *fS)
{
    // Use s as the first sequence for the pairwise sequence alignment.
    // Prepare the second sequence from each of I using fS.
    // Use mem to find anchors in the pairwise alignment.
    // Perform a pairwise alignment.
    // Write CAM, SAM, and BAM files.
    
    // Use I[1] to find the number of regions.
    // Assign each mem to one of the alignment or interval as anchors.
    
    
}