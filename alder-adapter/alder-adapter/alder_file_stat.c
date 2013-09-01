//
//  alder_file_stat.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 9/1/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <sys/stat.h>
#include "alder_fastq_stat.h"

int alder_file_stat()
{
    
}

//  0 : exist
// -1 : not exist
int alder_file_exist(const char *fn)
{
    struct stat buf;
    int status = stat(fn, &buf);
    return status;
}
