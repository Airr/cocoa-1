//
//  main.c
//  ctest
//
//  Created by Sang Chul Choi on 8/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "main.h"

int main(int argc, const char * argv[])
{
    test_division_by_integer();
    return 0;
}

void test_division_by_integer()
{
    int64_t i = -1/2;
    assert(i == 0);
    i = (int64_t)(-3.0/2.0);
    assert(i == -1);
}