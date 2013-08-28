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
//    bstring b = bfromcstr ("Hello");
    bstring b = blk2bstr (bsStaticBlkParms ("Fast init. "));
    bReverse(b);
    char *tailS = bdata(b);
    // insert code here...
    int lenB = blength(b);
    char c = bchare(b, 3, 'x');
    char *s = bdataofs(b, 3);
    printf("Hello, World! %s\n", bdata(b));
    bdestroy(b);
    return 0;
}

