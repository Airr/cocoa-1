/**
 * This file, main.c, is part of alder-adapter.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-adapter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-adapter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-adapter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <assert.h>
#include "alder_adapter_option.h"
#include "alder_logger.h"
#include "cmdline.h"
#include "alder_adapter_cmd_check.h"
#include "bstrmore.h"
#include "alder_fastq_stat.h"
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
    alder_adapter_option_t main_option;
    
    ///////////////////////////////////////////////////////////////////////////
    // Initialize the error logger.
    ///////////////////////////////////////////////////////////////////////////
    alder_logger_error_initialize(LOG_WARNING);
    
    ///////////////////////////////////////////////////////////////////////////
    // Check the command line.
    ///////////////////////////////////////////////////////////////////////////
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_adatper_cmd_check(&main_option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        exit(1);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Setup the regular logger.
    ///////////////////////////////////////////////////////////////////////////
    if (args_info.log_flag == 1 || args_info.logfile_given) {
        assert(main_option.logfilename != NULL);
        alder_logger_initialize(bdata(main_option.logfilename), LOG_INFO);
    }
    
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
    
    ///////////////////////////////////////////////////////////////////////////
    // Free everything.
    ///////////////////////////////////////////////////////////////////////////
    alder_adapter_option_free(&main_option);
    my_cmdline_parser_free(&args_info);
    alder_logger_finalize();
    alder_logger_error_finalize();
    return 0;
}

