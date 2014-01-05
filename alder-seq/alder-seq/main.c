/**
 * This file, main.c, is part of alder-seq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-seq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-seq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-seq.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_logger.h"
#include "alder_file_rm.h"
#include "alder_seq.h"
#include "alder_seq_option.h" 

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_seq_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_seq_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_seq_option_free(&option);
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
    
    if (args_info.chunk_flag) {
        alder_seq_chunk_test(args_info.inputs, args_info.inputs_num);
    }
    
    my_cmdline_parser_free(&args_info);
    alder_seq_option_free(&option);
    return 0;
}

