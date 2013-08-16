//
//  main.c
//  memc
//
//  Created by Sang Chul Choi on 8/12/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include "sparse_sa.h"

int main(int argc, const char * argv[])
{

    // insert code here...
    if (argc != 3)
    {
        printf("./memc [query-string] [ref-string]\n");
        printf("e.g., ./memc iss mississippi\n");
        exit(0);
    }
    
    uint64_t K = 2;
    alder_sparse_sa_t *sparseSA = alder_sparse_sa_alloc (argv[2], K);
    query_thread(sparseSA, argv[1]);
    alder_sparse_sa_free (sparseSA);

    return 0;
}

