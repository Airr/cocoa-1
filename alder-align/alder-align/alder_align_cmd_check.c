/**
 * This file, alder_align_cmd_check.c, is part of alder-align.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-align is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-align is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-align.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <assert.h>
#include "bstrmore.h"
#include "alder_logger.h"
#include "alder_file_isstdin.h"
#include "alder_file_exist.h"
#include "alder_align_cite.h"
#include "alder_fastq_pair.h"
#include "alder_align_cmd_check.h"

// 0: success
// 1: failure
int alder_align_cmd_check(alder_align_option_t *option, struct gengetopt_args_info *args_info)
{
    int status = 0;
    if (args_info->cite_given) {
        alder_align_cite();
        status = 1;
        return status;
    }
    
    option->sparse_suffix_array_k = (int)args_info->sparse_suffix_array_k_arg;
    option->is_stdin = alder_file_isstdin();
    
    if (option->is_stdin == 0 && args_info->inputs_num == 0) {
        logc_logWarning(ERROR_LOGGER, "no input file.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->help_help);
        status = 1;
    }
    if (option->is_stdin == 1 && args_info->inputs_num > 0) {
        logc_logWarning(ERROR_LOGGER, "standard input and input files.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->help_help);
        status = 1;
    }
    
    if (option->is_stdin == 1 && args_info->log_given && !args_info->logfile_given) {
        logc_logWarning(ERROR_LOGGER, "no log file name is possible with standard input.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->logfile_help);
        status = 1;
    }
    
    if (status == 1) {
        return status;
    }
    
    
    // Files: stdin, log, input, reference, and output file names.
    if (args_info->log_flag == 1 && !args_info->logfile_given) {
        if (option->is_stdin == 0) {
            option->logfilename = bfromcstr(args_info->inputs[0]);
            bcatcstr(option->logfilename, ".log");
        } else {
            option->logfilename = NULL;
            status = 1;
        }
    } else if (args_info->logfile_given) {
        args_info->log_flag = 1;
        option->logfilename = bfromcstr(args_info->logfile_arg);
    } else {
        option->logfilename = NULL;
    }
    if (args_info->inputs_num == 0 || option->is_stdin == 1) {
        option->infile = NULL;
    } else {
        option->infile = bstrVectorCreate((int)args_info->inputs_num);
    }
    for (size_t i = 0; i < args_info->inputs_num; i++) {
        bstrVectorAdd(option->infile, args_info->inputs[i]);
        int r = alder_file_exist(args_info->inputs[i]);
        if (r == 0) {
            logc_logWarning(ERROR_LOGGER, "no such input file %s.", args_info->inputs[i]);
            status = 1;
        }
    }
    assert(args_info->reference_given > 0);
    option->refile = bstrVectorCreate((int)args_info->reference_given);
    for (size_t i = 0; i < args_info->reference_given; i++) {
        bstrVectorAdd(option->refile, args_info->reference_arg[i]);
        int r = alder_file_exist(args_info->reference_arg[i]);
        if (r == 0) {
            logc_logWarning(ERROR_LOGGER, "no such reference file %s.", args_info->reference_arg[i]);
            status = 1;
        }
    }
    if (args_info->output_given > 0) {
        option->outfile = bstrVectorCreate((int)args_info->output_given);
        for (size_t i = 0; i < args_info->output_given; i++) {
            bstrVectorAdd(option->outfile, args_info->output_arg[i]);
            int r = alder_file_exist(args_info->output_arg[i]);
            if (r == 0) {
                logc_logWarning(ERROR_LOGGER, "no such output file %s.", args_info->output_arg[i]);
                status = 1;
            }
        }
    } else {
        option->outfile = NULL;
    }
    
    option->pair = alder_vector_pair_t_alloc(args_info->inputs_num);
    if (args_info->no_pair_flag == 0) {
        alder_fastq_pair(option->infile, option->pair);
    } else {
        alder_fastq_nopair(option->pair);
    }


    return status;
}
