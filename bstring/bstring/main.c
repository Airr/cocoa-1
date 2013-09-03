//
//  main.c
//  bstring
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include "bstrlib.h"
#include "bstraux.h"
#include "bstrmore.h"

void test_trim_quality2()
{
    // *: 33+9
    // ?: 33+30
    bstring b2 = bfromcstr("***????**");
    bstring b1 = bfromcstr("NNNACGTNN");
    int s = bltrimq2(b2, b1, 33+10);
    printf("b1=ACGTNN: %s\n", bdata(b1));
    printf("b2=xxxxxx: %s\n", bdata(b2));
    s = brtrimq2(b2, b1, 33+10);
    printf("b1=ACGT: %s\n", bdata(b1));
    printf("b2=xxxx: %s\n", bdata(b2));
    bdestroy(b2);
    bdestroy(b1);
    
}

void test_trim2()
{
    bstring b1 = bfromcstr("NNNACGTNN");
    bstring b2 = bfromcstr("!\"#$%&'()");
    int s = bltrimc2(b1, b2, 'N');
    printf("b1=ACGTNN: %s\n", bdata(b1));
    printf("b2=xxxxxx: %s\n", bdata(b2));
    s = brtrimc2(b1, b2, 'N');
    printf("b1=ACGT: %s\n", bdata(b1));
    printf("b2=xxxx: %s\n", bdata(b2));
    bdestroy(b2);
    bdestroy(b1);
    
    b1 = bfromcstr("NNNACGTNN");
    b2 = bfromcstr("!\"#$%&'()");
    s = btrimc2(b1, b2, 'N');
    printf("b1=ACGT: %s\n", bdata(b1));
    printf("b2=xxxx: %s\n", bdata(b2));
    bdestroy(b2);
    bdestroy(b1);
}

void test_trim_quality()
{
    bstring b = bfromcstr("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
    int s = bltrimq(b, 33+30);
    printf("b[>=30 ?]: %s\n", bdata(b));
    s = brtrimq(b, 126);
    printf("b[>=126 ?]: %s\n", bdata(b));
    s = brtrimq(b, 127);
    printf("b[>=127 ?]: %s\n", bdata(b));
    bdestroy(b);
}

void test_trim()
{
    bstring b = bfromcstr("0123456789");
    printf("b: %s\n", bdata(b));
    int s = btrunc(b, 9);
    assert(s == BSTR_OK);
    printf("bleft[0-8]: %s\n", bdata(b));
    s = bdelete(b, 0, 1);
    assert(s == BSTR_OK);
    printf("bleft[1-8]: %s\n", bdata(b));
    s = bltrim(b, 1);
    assert(s == BSTR_OK);
    printf("bleft[2-8]: %s\n", bdata(b));
    s = brtrim(b, 1);
    printf("bleft[2-7]: %s\n", bdata(b));
    s = bltrim(b, 0);
    printf("bleft[2-7]: %s\n", bdata(b));
    s = brtrim(b, 0);
    printf("bleft[2-7]: %s\n", bdata(b));
    s = brtrimc(b, '7');
    printf("bleft[2-6]: %s\n", bdata(b));
    s = bltrimc(b, '2');
    printf("bleft[3-6]: %s\n", bdata(b));
    s = btrimc(b, '3');
    printf("bleft[3-5]: %s\n", bdata(b));
    
    bdestroy(b);
}



void test_tokenize()
{
    bstring b = bfromcstr ("1:N:0:ABCD");
    struct bstrList * lines;
    lines = bsplit (b, ':');
    
    for (int i=0; i < lines->qty; i++) {
        printf ("%04d: %s\n", i, bdatae (lines->entry[i], "NULL"));
    }
    bstrListDestroy (lines);
    bdestroy (b);
}

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
    
//    test_trim();
//    test_trim_quality();
//    test_trim2();
    test_trim_quality2();
    
    return 0;
}

