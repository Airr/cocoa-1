//
//  main.c
//  alder-ssa
//
//  Created by Sang Chul Choi on 9/6/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_ssa_option.h"

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_ssa_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_ssa_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_ssa_option_free(&option);
        exit(1);
    }
    
    my_cmdline_parser_free(&args_info);
    alder_ssa_option_free(&option);
    return 0;
}

