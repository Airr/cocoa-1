//
//  main.c
//  alder-fastq-random
//
//  Created by Sang Chul Choi on 8/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "../deseq/fastq.h"

int main(int argc, const char * argv[])
{
    // Run the first function to create data/test.rna.fa
    // Then, run the second function create a FASTQ file.
//    randomFastqUsingGFF("data/test.fa", "data/test.gff");
    randomFastqUsingFasta("data/test.rna.fa", 10);
    return 0;
}

