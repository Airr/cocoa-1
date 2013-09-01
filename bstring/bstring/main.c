//
//  main.c
//  bstring
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "bstrlib.h"
#include "bstraux.h"

int main(int argc, const char * argv[])
{
/*
    bstring b = blk2bstr (bsStaticBlkParms ("Fast init. "));
    bReverse(b);
    char *tailS = bdata(b);
    // insert code here...
    int lenB = blength(b);
    char c = bchare(b, 3, 'x');
    char *s = bdataofs(b, 3);
    printf("Hello, World! %s\n", bdata(b));
    bdestroy(b);
*/
    bstring b = bfromcstr ("1:N:0:ABCD");
    struct bstrList * lines;
    lines = bsplit (b, ':');
    
    for (int i=0; i < lines->qty; i++) {
        printf ("%04d: %s\n", i, bdatae (lines->entry[i], "NULL"));
    }
    bstrListDestroy (lines);
    bdestroy (b);
    
    
    return 0;
}

