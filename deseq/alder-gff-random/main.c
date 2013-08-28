//
//  main.c
//  alder-gff-random
//
//  Created by Sang Chul Choi on 8/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "../deseq/gff.h"

int main(int argc, const char * argv[])
{
    randomGFF(2, 6000, 1000, 800, 300, 100, 500, 200);
    return 0;
}

