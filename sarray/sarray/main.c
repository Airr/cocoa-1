//
//  main.c
//  sarray
//
//  Created by Sang Chul Choi on 7/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sarray.h"

int main(int argc, const char * argv[])
{
    const char *s = "ABRACADABRA";
    int *l;
    int n;
    int *a;
    int i;
    
    n = strlen(s) + 1;
    a = scode(s);
    sarray(a, n);
    l = lcp(a, s, n);
    
    for (i = 0; i < n; i++)
    {
        printf("[%04d] %5d %5d\n", i, a[i], l[i]);
    }
    
    free(l);
    free(a);
    return 0;
}

