/**
 * This file, main.c, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_file.h"
#include "alder_logger.h"
#include "alder_wordtable.h"
#include "alder_wordtable_option.h"

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_wordtable_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_wordtable_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        fprintf(stderr, "%s\n", args_info.help_help);
        my_cmdline_parser_free(&args_info);
        alder_wordtable_option_free(&option);
        exit(1);
    }
    alder_file_rm(args_info.log_arg);
    if (args_info.log_given) {
        alder_logger_initialize(args_info.log_arg, LOG_FINEST);
        alder_logger_error_initialize(LOG_BASIC);
    } else {
        alder_logger_initialize(args_info.log_arg, LOG_SILENT);
        alder_logger_error_initialize(LOG_SILENT);
    }
//    alder_log0("log0");
//    alder_log("log");
//    alder_loge0(1, "loge0"); always out to the screen
//    alder_loge(1, "loge");   only to the screen with --log option on.
    
    if (args_info.sequenceiterator_flag) {
        alder_wordtable_sequenceiterator_test(option.infile);
    }
    if (args_info.bigbitarray_flag) {
        alder_wordtable_bigbitarray_test();
    }
    if (args_info.bitpackedlist_flag) {
        alder_wordtable_bitpackedlist_test();
    }
    if (args_info.positionlist_flag) {
        alder_wordtable_positionlist_test();
    }
    if (args_info.positionvaluelist_flag) {
        alder_wordtable_positionvaluelist_test();
    }
    if (args_info.bitpackedcumulativearray_flag) {
        alder_wordtable_bitpackedcumulativearray_test();
    }
    if (args_info.bits_flag) {
        alder_wordtable_bits_test();
    }
    if (args_info.tokeniterator_flag) {
        alder_wordtable_tokeniterator_test();
    }
    if (args_info.inedgelist_flag) {
        alder_wordtable_inedgelist_test();
    }
    if (args_info.intblocklist_flag) {
        alder_wordtable_intblocklist_test();
    }
    if (args_info.inedgedata_flag) {
        alder_wordtable_inedgedata_test();
    }
    if (args_info.kfmindex_flag) {
        alder_wordtable_kfmindex_test();
    }
    if (args_info.kfminterval_flag) {
        alder_wordtable_kfminterval_test();
    }
    if (args_info.kfmprune_flag) {
        alder_wordtable_kfmprune_test();
    }
    if (args_info.kfmmerge_flag) {
        alder_wordtable_kfmmerge_test();
    }
    if (args_info.kfmaggregator_flag) {
        alder_wordtable_kfmaggregator_test(option.infile,
                                           args_info.outfile_arg[0]);
    }
    if (args_info.kfmio_flag) {
        alder_wordtable_kfmio_test(args_info.inputs[0]);
    }
    if (args_info.kfmsim_flag) {
        alder_wordtable_kfmsim_test();
    }
    if (args_info.kfmcheck_flag) {
        alder_wordtable_kfmcheck_test();
    }
    
    if (args_info.build_flag) {
//        alder_wordtable_kFMindexFromFastaFileUsingAggregator
//        (args_info.inputs[0],
//         (size_t)args_info.kmersize_arg,
//         (size_t)args_info.blocksize_arg);
    }
    if (args_info.kfmfind_flag) {
        alder_wordtable_kfmfind_test();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    // Cleanup!
    if (args_info.log_given) {
        alder_logger_finalize();
        alder_logger_error_finalize();
    } else {
        alder_logger_finalize();
        alder_logger_error_finalize();
        alder_file_rm(args_info.log_arg);
    }
    my_cmdline_parser_free(&args_info);
    alder_wordtable_option_free(&option);
    return 0;
}

