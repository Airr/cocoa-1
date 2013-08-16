//
//  main.c
//  palign
//
//  Created by Sang Chul Choi on 8/14/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "alder_assemble.h"


int main(int argc, const char * argv[])
{
    srand(1);
    char codes[4] = {'A','C','G','T'};
    alder_align_read_initialize();
    for (size_t i = 0; i < 10000; i++)
    {
        size_t sLen = rand() % 100;
        if (sLen < 30) sLen = 30;
        char *s = malloc((sLen + 1)*sizeof(char));
        for (size_t j = 0; j < sLen; j++) {
            size_t sCode = rand() % 4;
            s[j] = codes[sCode];
        }
        s[sLen] = '\0';
        
        alder_align_read_execute(s, s);
        free(s);
    }
    alder_align_read_finalize();
    return 0;
}

