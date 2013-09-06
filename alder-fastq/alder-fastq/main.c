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
    
    if (args_info.kseq_flag) {
        if (args_info.summary_flag == 1) {
            alder_fastq_kseq_summary(option.infile, option.outfile);
        }
    } else {
        if (args_info.summary_flag == 1) {
            alder_fastq_ragel_summary(option.infile, option.outfile);
        }
    }
    
    my_cmdline_parser_free(&args_info);
    alder_fastq_option_free(&option);
    return 0;
}

