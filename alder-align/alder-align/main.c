//
//  main.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/7/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <nglogc/log.h>
#include "alder_genome.h"
#include "PackedArray.h"
#include "alder_suffix_array.h"
#include "alder_logger.h"
#include "alder_index_genome.h"
#include "alder_align_reads.h"
#include "Parameters.h"
#include "mem/sparse_sa_test.h"
#include "fastq/alder_fastq.h"
#include "mem/sparse_sa.h"
#include "gsl_vector_match.h"
#include "lib/alder_reference.h"
#include "palign/alder_palign.h"

int main(int argc, const char * argv[])
{
    alder_logger_initialize();
    logc_log(MAIN_LOGGER, LOG_BASIC, "alder-align starts.");
    alder_sparse_sa_t *sparseSA = alder_sparse_sa_alloc_file("data/test.fa", 2);
    
    
    
    fastq_t *f = alder_fastq_alloc("data/test.fq.gz");
    interval_t I = alder_reference_candidate_init();
    while (alder_fastq_next(f)) {
//        printf("%s\n", f->seq->seq.s);
        
        uint64_t min_len = 15;
        bool print = true;
        int num_threads = 1;
        
        gsl_vector_match *mem = gsl_vector_match_init();
        
        // Find all MEMs and their positions in the query.
        alder_MEM(sparseSA, f->seq->seq.s, mem, min_len, print, num_threads);
//        gsl_vector_match_fprintf(stdout, mem, NULL);
        // Find regions from the reference genome that include MEMs.
        int64_t L = (int64_t)strlen(f->seq->seq.s);
        alder_reference_candidate_find(I, mem, L);
        
        // Perform pairwise sequence alignment between ref and the query.
        // Print out the alignment to cam/sam file.
        alder_palign(f->seq->seq.s, I, mem, sparseSA->fS);
        
        
        gsl_vector_match_free(mem);
        
        alder_union_interval_reset(I);
    }
    alder_reference_candidate_free(I);
    
    alder_sparse_sa_free(sparseSA);
    alder_logger_finalize();
    return 0;
}

