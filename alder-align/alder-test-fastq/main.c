//
//  main.c
//  alder-test-fastq
//
//  Created by Sang Chul Choi on 8/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "../alder-align/fastq/alder_fastq.h"

int main(int argc, const char * argv[])
{
    for (size_t i = 0; i < 10000000; i++) {
        fastq_t *f = alder_fastq_alloc("data/test.fq.gz");
        while (alder_fastq_next(f)) {
//            printf("%s\n", f->seq->seq.s);
        }
    }
    return 0;
}

