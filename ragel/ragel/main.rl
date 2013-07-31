//
//  main.c
//  ragel
//
//  Created by Sang Chul Choi on 7/23/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char * argv[])
{
    %% machine gtf;
    %% include gtf "gtf.rh";

    int i;
    char *eof;
    int cs, res = 0;
    if (argc > 1)
    {
        char *p = argv[1];
        char *pe = p + strlen( p );
        printf("args: %s\n", p);
        // char* eof = is_eof ? pe : ((char*) 0);
        %% write init;
        %% write exec;
    }
    return 0;
}

