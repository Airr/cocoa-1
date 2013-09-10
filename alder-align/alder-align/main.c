//
//  main.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/7/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include "alder_align_option.h"
#include "alder_logger.h"
#include "cmdline.h"
#include "alder_align_cmd_check.h"
#include "alder_fastq.h"
#include "gsl_vector_match.h"
#include "alder_ssa.h"
#include "alder_fastq_kseq.h"
#include "alder_file_isgzip.h"
#include "alder_vector.h"
#include "alder_mem.h"
#include "alder_palign.h"

int main(int argc, char * argv[])
{
    ///////////////////////////////////////////////////////////////////////////
    // Check the command line.
    ///////////////////////////////////////////////////////////////////////////
    int isExitWithHelp = 0;
    alder_align_option_t main_option;
    alder_align_option_init(&main_option);
    alder_logger_error_initialize(LOG_FINEST);
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_align_cmd_check(&main_option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_align_option_free(&main_option);
        alder_logger_error_finalize();
        exit(1);
    }
    if (main_option.logfilename != NULL) {
        alder_logger_initialize(bdata(main_option.logfilename), LOG_FINE);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Build SA.
    ///////////////////////////////////////////////////////////////////////////
    alder_sparse_sa_t *sparseSA = alder_sparse_sa_alloc_file(main_option.refile,
                                                             (int64_t)main_option.sparse_suffix_array_k);
    if (sparseSA == NULL) {
        if (main_option.logfilename != NULL) alder_logger_finalize();
        alder_align_option_free(&main_option);
        alder_logger_error_finalize();
        return 1;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Align read sequences.
    ///////////////////////////////////////////////////////////////////////////    
    // Read FASTQ files and pair them just like alder-pre.          
    // Find all MEMs and their positions in the query.
    // Find regions from the reference genome that include MEMs. 
    // Perform pairwise sequence alignment between ref and the query.
    // Print out the alignment to cam/sam file.
    //    fastq_t *f = alder_fastq_alloc("data/test.fq.gz");
    uint64_t min_len = args_info.min_len_mem_arg;
    int status = 0;
    int pairIndex = 0;
    int firstIndex = main_option.pair->data[pairIndex].first;
    int secondIndex = main_option.pair->data[pairIndex].second;
//    interval_t I = alder_reference_candidate_init();
    while (!(firstIndex < 0 && secondIndex < 0)) {
        assert(firstIndex >= 0 && firstIndex < args_info.inputs_num);
        
        kseq_t *seq1 = NULL;
        kseq_t *seq2 = NULL;
        alder_fastq_t * fastq1 = alder_fastq_open(bdata(main_option.infile->entry[firstIndex]));
        if (fastq1 == NULL) {
            logc_logWarning(ERROR_LOGGER, "cannot process %s.\n",
                            bdata(main_option.infile->entry[firstIndex]));
            status = -1;
            break;
        }
        alder_fastq_t * fastq2 = NULL;
        if (secondIndex >= 0) {
            fastq2 = alder_fastq_open(bdata(main_option.infile->entry[secondIndex]));
            if (fastq2 == NULL) {
                logc_logWarning(ERROR_LOGGER, "cannot process %s.\n",
                                bdata(main_option.infile->entry[secondIndex]));
                status = -1;
                break;
            }
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Use each sequence to align it to the reference genome or SA.
        // Get a palign vector to use it with palign module.
        ///////////////////////////////////////////////////////////////////////
       
        while ((seq1 = alder_fastq_read(fastq1)) != NULL) {
            seq2 = alder_fastq_read(fastq2);
            if (seq2 == NULL && 0 <= secondIndex && secondIndex < main_option.infile->qty) {
                status = -1;
                break;
            }
            assert(seq2 == NULL);
            
            ///////////////////////////////////////////////////////////////////
            // Find the palign vector.
            ///////////////////////////////////////////////////////////////////
            alder_vector_palign_t *palign = alder_mem_palign(sparseSA, seq1->seq.s, min_len);
            
            ///////////////////////////////////////////////////////////////////
            // Use v_palign with alder_palign module.
            ///////////////////////////////////////////////////////////////////
            gsl_vector_sam *sam = alder_palign_align_in_sam(palign, sparseSA->fS, seq1->name.s);
            
            ///////////////////////////////////////////////////////////////////
            // Print or filter sam.
            ///////////////////////////////////////////////////////////////////
            
            // Remove palign and sam.
            gsl_vector_sam_free(sam);
            alder_vector_palign_free(palign);
        }
        alder_fastq_close(fastq1);
        alder_fastq_close(fastq2);
        if (status < 0) {
            break;
        }
        
        pairIndex++;
        if (main_option.pair->size == pairIndex) break;
        firstIndex = main_option.pair->data[pairIndex].first;
        secondIndex = main_option.pair->data[pairIndex].second;
    }
//    alder_reference_candidate_free(I);
    if (status < 0) {
        logc_logWarning(ERROR_LOGGER, "problem in processing FASTQ files.\n");
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Finalize alder-align.
    ///////////////////////////////////////////////////////////////////////////
    alder_sparse_sa_free(sparseSA);
    if (main_option.logfilename != NULL) alder_logger_finalize();
    alder_align_option_free(&main_option);
    alder_logger_error_finalize();
    return 0;
}



