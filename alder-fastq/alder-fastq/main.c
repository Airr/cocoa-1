//
//  main.c
//  alder-fastq
//
//  Created by Sang Chul Choi on 9/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_fastq.h"
#include "alder_logger.h"
#include "alder_file.h"
#include "alder_fastq_option.h"

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_fastq_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_fastq_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_fastq_option_free(&option);
        exit(1);
    }
    ///////////////////////////////////////////////////////////////////////////
    // Set up a log file if needed.
    alder_file_rm(args_info.log_arg);
    if (args_info.log_given) {
        alder_logger_initialize(args_info.log_arg, LOG_FINE);
        alder_logger_error_initialize(LOG_BASIC);
    } else {
        alder_logger_initialize(args_info.log_arg, LOG_SILENT);
        alder_logger_error_initialize(LOG_FINE);
    }
    
    if (args_info.kseq_flag) {
        if (args_info.summary_flag == 1) {
            alder_fastq_kseq_summary(option.infile, option.outfile);
        }
    } else {
        if (args_info.summary_flag == 1) {
            alder_fastq_ragel_summary(option.infile, option.outfile);
        }
    }
    
    if (args_info.concat_flag == 1) {
        for (size_t i = 0; i < option.infile->qty; i++) {
            alder_fastq_concat_t *afc = alder_fastq_concat_init(bdata(option.infile->entry[i]), NULL);
            alder_fastq_concat_read(args_info.inputs[i], afc->read, afc->qual, afc->index, afc->numberBase);
            if (option.outfile == NULL) {
                alder_fastq_concat_fprintf(NULL, afc, 0);
            } else if (option.outfile->qty == 1) {
                alder_fastq_concat_fprintf(bdata(option.outfile->entry[0]),
                                           afc, 1);
            } else if (option.outfile->qty > 1) {
                alder_fastq_concat_fprintf(bdata(option.outfile->entry[i]),
                                           afc, 0);
            }
            alder_fastq_concat_free(afc);
        }
    }
    
    if (args_info.kmer_given) {
        for (int i = 0; i < option.infile->qty; i++) {
            uint64_t v = 0;
            alder_fastq_count_kmer(bdata(option.infile->entry[i]),
                                   (int)args_info.kmer_arg, &v);
            fprintf(stdout, "File: %s\n", bdata(option.infile->entry[i]));
            fprintf(stdout, "Kmer: %llu\n", v);
        }
    }
    
    if (args_info.sequenceiterator_flag) {
        alder_fastq_sequenceiterator_test(option.infile);
    }
    if (args_info.chunk_flag) {
        alder_fastq_chunk_test(args_info.inputs, args_info.inputs_num);
    }
    
    my_cmdline_parser_free(&args_info);
    alder_fastq_option_free(&option);
    return 0;
}

