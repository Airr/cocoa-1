//
//  main.c
//  brass-ds-union-tree
//
//  Created by Sang Chul Choi on 8/18/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "union_array.h"

#define OBJECTSIZE 11

int main(int argc, const char * argv[])
{
    int status;
    interval_t a[OBJECTSIZE] = {
        {.b = 1, .e = 3},
        {.b = 2, .e = 5},
        {.b = 4, .e = 6},
        {.b = 7, .e = 8}
    };
    
    endpoint_t *t;
    for (size_t k = 0; k < 1; k++)
    {
        t = create_union_array(a, OBJECTSIZE);
    }
    free(t);
    return 0;
}

