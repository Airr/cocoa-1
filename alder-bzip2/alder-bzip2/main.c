//
//  main.c
//  alder-bzip2
//
//  Created by Sang Chul Choi on 1/5/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "alder_bzip2.h"

int main(int argc, const char * argv[])
{
    
    alder_bzip2_decompress(argv[1], argv[2]);

    return 0;
}

