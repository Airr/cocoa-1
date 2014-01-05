//
//  main.c
//  alder-file
//
//  Created by Sang Chul Choi on 9/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_file.h"
#include "alder_file_io.h"
#include "alder_file_option.h"

int main(int argc, char * argv[])
{
    ///////////////////////////////////////////////////////////////////////////
    // Check the command line arguments.
    alder_file_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    int isExitWithHelp = alder_file_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        alder_file_option_free(&option);
        my_cmdline_parser_free(&args_info);
        exit(1);
    }
    
    if (args_info.summary_flag) {
        for (size_t i = 0; i < args_info.inputs_num; i++) {
            alder_file(args_info.inputs[i]);
        }
    }
    if (args_info.numline_flag) {
        fprintf(option.fout, "--- Number of Lines in the Files ---\n");
        for (size_t i = 0; i < args_info.inputs_num; i++) {
            if (alder_file_isfile(args_info.inputs[i]) &&
                !alder_file_isbinary(args_info.inputs[i])) {
                int64_t n = alder_file_numline(args_info.inputs[i]);
                fprintf(option.fout, "%s: %lld\n", args_info.inputs[i], n);
            }
        }
    }
    if (args_info.disk_flag) {
        for (size_t i = 0; i < args_info.inputs_num; i++) {
            unsigned long space_mb = 0;
            int s = alder_file_availablediskspace(args_info.inputs[i], &space_mb);
            if (s == 0) {
                fprintf(option.fout, "Available space in %s: %lu MB\n",
                        args_info.inputs[i], space_mb);
            }
        }
    }
    if (args_info.estimate_sequence_file_flag) {
        for (size_t i = 0; i < args_info.inputs_num; i++) {
            unsigned long space_mb = 0;
            int s = alder_file_sequencefilesize(args_info.inputs[i], &space_mb);
            if (s == 0) {
                fprintf(option.fout, "Estimated sequence size in %s: %lu MB\n",
                        args_info.inputs[i], space_mb);
            } else {
                fprintf(option.fout, "File %s is not a proper file.\n",
                        args_info.inputs[i]);
            }
        }
    }
    if (args_info.mkdir_given) {
        alder_file_mkdir(args_info.mkdir_arg);
    }
    if (args_info.cpu_flag) {
        int ncpu = alder_file_sysctl();
        fprintf(stdout, "Number of CPU: %d\n", ncpu);
    }
    if (args_info.memory_flag) {
        alder_file_memory_test();
    }
    if (args_info.io_flag) {
        alder_file_io_test01();
//        alder_file_io_test();
    }
    
    alder_file_option_free(&option);
    my_cmdline_parser_free(&args_info);
    return 0;
}
