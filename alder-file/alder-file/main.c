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

int main(int argc, char * argv[])
{
    struct gengetopt_args_info a;
    if (my_cmdline_parser (argc, argv, &a) != 0) exit(1) ;

    for (size_t i = 0; i < a.inputs_num; i++) {
        alder_file(a.inputs[i]);
    }
    
    my_cmdline_parser_free(&a);
    return 0;
}
