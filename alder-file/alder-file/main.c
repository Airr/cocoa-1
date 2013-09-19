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
    
    alder_file_option_free(&option);
    my_cmdline_parser_free(&args_info);
    return 0;
}
