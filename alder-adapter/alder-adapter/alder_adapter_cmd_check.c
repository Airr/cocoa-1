//
//  alder_adapter_cmd_check.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 9/1/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstrmore.h"
#include "alder_logger.h"
#include "alder_file_stat.h"
#include "alder_adapter_pair.h"
#include "alder_adapter_cmd_check.h"

// 0: success
// 1: failure
int alder_adatper_cmd_check(alder_adapter_option_t *option, struct gengetopt_args_info *args_info)
{
    int status = 0;
    
    if (args_info->help_given) {
        my_cmdline_parser_print_help();
        status = 1;
        return status;
    }
    option->keep = args_info->keep_flag;
    if (args_info->phread_given) {
        option->phred = 64;
    } else {
        option->phred = 33;
    }
    
    // cutadapt options
    option->cutadapt_flag = 14;
    option->cutadapt_degenerate = 0;
    
    // FIXME: could make these option initializatoin.
    // pairs, input files, output files, and adapters
    // pair (0,1 -> -1, -1 if stdin)
    option->pair = malloc((args_info->inputs_num + 1)* 2 * sizeof(int));
    if (args_info->inputs_num == 0 || option->is_stdin == 1) {
        option->infile = NULL;
    } else {
        option->infile = bstrVectorCreate((int)args_info->inputs_num);
    }
    for (size_t i = 0; i < args_info->inputs_num; i++) {
        bstrVectorAdd(option->infile, args_info->inputs[i]);
    }
    if (option->is_stdin == 0) {
        option->outfile = bstrVectorCreate((int)args_info->inputs_num);
        if (args_info->output_given > 0) {
            if (args_info->output_given == args_info->inputs_num) {
                for (size_t i = 0; i < args_info->output_given; i++) {
                    bstrVectorAdd(option->outfile, args_info->output_arg[i]);
                }
            } else {
                logc_logWarning(ERROR_LOGGER,
                                "option --adapter needs as many values as input files.");
                logc_logWarning(ERROR_LOGGER, "See the following option\n%s.",
                                args_info->adapter_help);
                status = 1;
            }
        } else {
            for (size_t i = 0; i < args_info->inputs_num; i++) {
                bstrVectorAdd(option->outfile, args_info->inputs[i]);
                bcatcstr(option->outfile->entry[i], ".alder.fq");
            }
        }
        if (!bstrVectorCompare(option->infile, option->outfile)) {
            logc_logWarning(ERROR_LOGGER,
                            "input and output file names are the same.");
            status = 1;
        }
    } else {
        if (args_info->output_given == 1) {
            option->outfile = bstrVectorCreate(1);
            bstrVectorAdd(option->outfile, args_info->output_arg[0]);
            bcatcstr(option->outfile->entry[0], ".alder.fq");
        } else {
            option->outfile = NULL;
        }
    }
    if (option->is_stdin == 0) {
        option->adapter = bstrVectorCreate((int)args_info->inputs_num);
        if (args_info->adapter_given > 0) {
            if (args_info->adapter_given == args_info->inputs_num) {
                for (size_t i = 0; i < args_info->adapter_given; i++) {
                    bstrVectorAdd(option->adapter, args_info->adapter_arg[i]);
                }
            } else {
                logc_logWarning(ERROR_LOGGER,
                                "option --adapter needs as many values as input files.");
                logc_logWarning(ERROR_LOGGER, "See the following option\n%s.",
                                args_info->adapter_help);
                status = 1;
            }
        }
    } else {
        if (args_info->adapter_given == 1) {
            option->adapter = bstrVectorCreate(1);
            bstrVectorAdd(option->adapter, args_info->adapter_arg[0]);
        } else {
            logc_logWarning(ERROR_LOGGER,
                            "option --adapter needs one value for standard input.");
            logc_logWarning(ERROR_LOGGER, "See the following option\n%s.",
                            args_info->adapter_help);
            status = 1;
        }
    }
    if (args_info->no_pair_flag == 0) {
        alder_adapter_pair(option);
    } else {
        alder_adapter_nopair(option);
    }
    alder_adapter_set_adapter(option);
    
    
    option->error_rate= args_info->error_rate_arg;
    option->trim_left = args_info->trim_left_arg;
    option->trim_right= args_info->trim_right_arg;
    option->trim_ambiguous_left = args_info->no_trim_ambiguous_left_flag;
    option->trim_ambiguous_right= args_info->no_trim_ambiguous_right_flag;
    option->trim_quality_left = args_info->trim_quality_left_arg;
    option->trim_quality_right= args_info->trim_quality_right_arg;
    option->filter_length = args_info->filter_length_arg;
    option->filter_quality = args_info->filter_quality_arg;
    option->filter_ambiguous = args_info->filter_ambiguous_arg;
    option->filter_uniq = args_info->filter_uniq_flag;
    
    if (option->trim_left < 0) {
        logc_logWarning(ERROR_LOGGER, "option --trim-left cannot be negative.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->trim_left_help);
        status = 1;
    }
    if (option->trim_right < 0) {
        logc_logWarning(ERROR_LOGGER, "option --trim-right cannot be negative.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->trim_left_help);
        status = 1;
    }
    if (option->trim_quality_left < 0) {
        logc_logWarning(ERROR_LOGGER, "option --trim-quality-left cannot be negative.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->trim_quality_left_help);
        status = 1;
    }
    if (option->trim_quality_right < 0) {
        logc_logWarning(ERROR_LOGGER, "option --trim-quality-right cannot be negative.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->trim_quality_right_help);
        status = 1;
    }
    if (option->filter_length < 0) {
        logc_logWarning(ERROR_LOGGER, "option --filter-length cannot be negative.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->filter_length_help);
        status = 1;
    }
    if (option->filter_ambiguous < 0) {
        logc_logWarning(ERROR_LOGGER, "option --filter-ambiguous cannot be negative.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->filter_ambiguous_help);
        status = 1;
    }
    if (option->filter_quality < 0) {
        logc_logWarning(ERROR_LOGGER, "option --filter-quality cannot be negative.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->filter_quality_help);
        status = 1;
    }
    
    if (option->is_stdin == 0 && args_info->inputs_num == 0) {
        logc_logWarning(ERROR_LOGGER, "no input file.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->fastq_help);
        status = 1;
    }
    
    if (option->is_stdin == 1 && args_info->inputs_num > 0) {
        logc_logWarning(ERROR_LOGGER, "standard input and input files.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->fastq_help);
        status = 1;
    }
    
    if (option->is_stdin == 1 && args_info->log_given && !args_info->logfile_given) {
        logc_logWarning(ERROR_LOGGER, "no log file name is possible with standard input.");
        logc_logWarning(ERROR_LOGGER, "See the following option\n%s.", args_info->logfile_help);
        status = 1;
    }
    
    if (args_info->inputs_num > 0) {
        for (unsigned int i = 0; i < args_info->inputs_num; i++) {
            int r = alder_file_exist(args_info->inputs[i]);
            if (r != 0) {
                logc_logWarning(ERROR_LOGGER, "no such file %s.", args_info->inputs[i]);
                status = 1;
            }
        }
    }
    return status;
}
