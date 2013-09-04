//
//  main.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <assert.h>
#include "bstrmore.h"
#include "cmdline.h"
#include "alder_file_isgzip.h"
#include "alder_adapter_cut.h"
#include "alder_adapter_cut_file.h"
#include "alder_adapter_cut_gzip.h"
#include "alder_adapter_index_illumina.h"
#include "alder_file_isstdin.h"
#include "alder_logger.h"
#include "alder_fastq_stat.h"
#include "alder_file_stat.h"
#include "alder_adapter_cut_core.h"
#include "alder_adapter_pair.h"
#include "alder_adapter_cmd_check.h"
#include "alder_adapter_cut_file2.h"

// TruSeq Adapters - Index 1-16,18,19 (Note: Illumina Index 17 is missing)
char *adapter_sequence[19] = {
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACATCACGATCTCGTATGCCGTCTTCTGCTTG",   // 1
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACCGATGTATCTCGTATGCCGTCTTCTGCTTG",   // 2
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACTTAGGCATCTCGTATGCCGTCTTCTGCTTG",   // 3
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACTGACCAATCTCGTATGCCGTCTTCTGCTTG",   // 4
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACACAGTGATCTCGTATGCCGTCTTCTGCTTG",   // 5
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACGCCAATATCTCGTATGCCGTCTTCTGCTTG",   // 6
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACCAGATCATCTCGTATGCCGTCTTCTGCTTG",   // 7
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACACTTGAATCTCGTATGCCGTCTTCTGCTTG",   // 8
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACGATCAGATCTCGTATGCCGTCTTCTGCTTG",   // 9
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACTAGCTTATCTCGTATGCCGTCTTCTGCTTG",   // 10
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACGGCTACATCTCGTATGCCGTCTTCTGCTTG",   // 11
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACCTTGTAATCTCGTATGCCGTCTTCTGCTTG",   // 12
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACAGTCAACAATCTCGTATGCCGTCTTCTGCTTG", // 13
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACAGTTCCGTATCTCGTATGCCGTCTTCTGCTTG", // 14
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACATGTCAGAATCTCGTATGCCGTCTTCTGCTTG", // 15
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACCCGTCCCGATCTCGTATGCCGTCTTCTGCTTG", // 16
    "XXX", // (17)
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACGTCCGCACATCTCGTATGCCGTCTTCTGCTTG", // 18
    "GATCGGAAGAGCACACGTCTGAACTCCAGTCACGTGAAACGATCTCGTATGCCGTCTTCTGCTTG", // 19
};

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;

    alder_error_logger_initialize(LOG_WARNING);
    
    ///////////////////////////////////////////////////////////////////////////
    // Check the command line.
    ///////////////////////////////////////////////////////////////////////////
    alder_adapter_option_t main_option;
    main_option.is_stdin = alder_file_isstdin();
    int isstdin = main_option.is_stdin;
    isExitWithHelp = alder_adatper_cmd_check(&main_option, &args_info);
    
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        exit(1);
    }
    alder_adapter_pair(&main_option);
    
    ///////////////////////////////////////////////////////////////////////////
    // Setup logfilename
    ///////////////////////////////////////////////////////////////////////////
    char *mainLogfilename = NULL;
    if (args_info.log_given && !args_info.logfile_given) {
        // logfilename starts with the basename
        assert(isstdin == 0);
        bstring blogext = bfromcstr(".log");
        bstring bfilename = bfromcstr(args_info.inputs[0]);
        bstring blogfilename = bfromcstralloc (blength(bfilename)+5, args_info.inputs[0]);
        bconcat(blogfilename, blogext);
        mainLogfilename = bstr2cstr(blogfilename, '\0');
        bdestroy(blogfilename);
        bdestroy(bfilename);
        bdestroy(blogext);
    } else if (args_info.logfile_given) {
        args_info.log_flag = 1;
        bstring blogfilename = bfromcstr(args_info.logfile_arg);
        mainLogfilename = bstr2cstr(blogfilename, '\0');
    }
    if (args_info.log_flag == 1 || args_info.logfile_given) {
        alder_logger_initialize(mainLogfilename, LOG_INFO);
    }
    if (mainLogfilename != NULL) {
        bcstrfree(mainLogfilename);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Adapter sequence argument
    ///////////////////////////////////////////////////////////////////////////
//    char *the_adapter_sequence = NULL;
//    if (args_info.adapter_given) {
//        the_adapter_sequence = args_info.adapter_arg;
//    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    // Cuts adapter parts in read sequences.
    ///////////////////////////////////////////////////////////////////////////
    alder_fastq_stat_t stat;
    int pairIndex = 0;
    int firstIndex = main_option.pair[pairIndex*2];
    int secondIndex = main_option.pair[pairIndex*2+1];
    while (!(firstIndex < 0 && secondIndex < 0)) {
        alder_fastq_stat_init(&stat);
        char *fnin = bstr2cstr(main_option.infile->entry[firstIndex], '\0');
        char *fnout = bstr2cstr(main_option.outfile->entry[firstIndex], '\0');
        char *adapter_seq = bstr2cstr(main_option.adapter->entry[firstIndex], '\0');
        
        char *fnin2 = NULL;
        char *fnout2 = NULL;
        char *adapter_seq2 = NULL;
        if (!(secondIndex < 0)) {
            fnin2 = bstr2cstr(main_option.infile->entry[secondIndex], '\0');
            fnout2 = bstr2cstr(main_option.outfile->entry[secondIndex], '\0');
            adapter_seq2 = bstr2cstr(main_option.adapter->entry[secondIndex], '\0');
        }
        
        alder_adapter_cut_file2(fnin, fnout, fnin2, fnout2,
                                adapter_seq, adapter_seq2,
                                &main_option, &stat);
        
    
        if (args_info.log_flag == 1 || args_info.logfile_given) {
            alder_fastq_stat_log(&stat, fnin, fnout, fnin2, fnout2, adapter_seq, adapter_seq2);
        }
        bcstrfree(fnin);
        bcstrfree(fnout);
        bcstrfree(adapter_seq);
        if (!(secondIndex < 0)) {
            bcstrfree(fnin2);
            bcstrfree(fnout2);
            bcstrfree(adapter_seq2);
        }
        pairIndex++;
        firstIndex = main_option.pair[pairIndex*2];
        secondIndex = main_option.pair[pairIndex*2+1];
    }
    
/*
    char *inputFilename = NULL;
    if (isstdin == 0) {
        bstring bInputFilename = bfromcstr(args_info.inputs[0]);
        inputFilename = bstr2cstr(bInputFilename, '\0');
        bdestroy(bInputFilename);
    }
    int status = alder_file_isgzip(inputFilename);
    assert(status == 0 || status == 1);
    if (status == 0) {
        status = alder_adapter_cut_file(inputFilename,
                                        args_info.output_arg,
                                        NULL, NULL,
                                        the_adapter_sequence,
                                        NULL,
                                        &main_option,
                                        &stat);
    } else if (status == 1) {
        alder_adapter_cut_gzip(inputFilename,
                               args_info.output_arg,
                               NULL, NULL,
                               the_adapter_sequence,
                               NULL,
                               &main_option,
                               &stat);
    } else {
        fprintf(stderr, "error: input file is not good.\n");
        my_cmdline_parser_print_help();
    }
    if (inputFilename != NULL) bcstrfree(inputFilename);
*/
    
//    alder_fastq_stat_free(&stat);
    
    alder_adapter_option_free(&main_option);
//    bstrVectorDelete(inputfiles);
    my_cmdline_parser_free(&args_info);
    alder_logger_finalize();
    alder_error_logger_finalize();
    return 0;
}

