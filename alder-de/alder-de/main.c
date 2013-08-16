//
//  main.c
//  alder-de
//
//  Created by Sang Chul Choi on 8/7/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "embeddedRCall.h"
#include "renv.h"

int main(int argc, const char * argv[])
{
    renv();
    eval_R_command("print", argc, (char **)argv);
    return 0;
}

