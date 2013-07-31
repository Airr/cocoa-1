//
//  main.c
//  deseq
//
//  Created by Sang Chul Choi on 7/17/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>
#include "cmdline.h"
#include "ftpget.h"
#include "httpget.h"
// Lehmer random number generator
#include "rngs.h"
#include "rvgs.h"
// Temporary directories
#include "mktemp.h"
// FASTQ file reader
#include "readfq.h"
// FASTA file reader
#include "fasta.h"
#include "gff.h"
#include "fastq.h"
#include "deseq.h"

static struct gengetopt_args_info args_info;

int main(int argc, const char * argv[])
{
    int i = 0;
    long l = 0;
    
    if (my_cmdline_parser(argc, (char **)argv, &args_info) != 0)
        exit(1) ;
    
    for (i = 0; i < args_info.first_given; ++i)
        printf("first string: %s\n", args_info.first_arg[i]);
    
    for (i = 0; i < args_info.second_given; ++i)
        printf("second string: %s\n", args_info.second_arg[i]);
    
    if (args_info.genome_given)
        printf("genome: %s\n", args_info.genome_arg);
    
    if (args_info.out_given)
        printf("output prefix: %s\n", args_info.out_arg);

    PlantSeeds(1); // Use -1 for using a seed from the clock.
    SelectStream(1);
//    l = Equilikely(100000,1000000);
//    printf("Equilikely: %ld\n", l);
    
//    peach_mktemp();
//    peach_remove();
//    peach_readfq(NULL);
//    peachFastqPreprocess(NULL);
//    readFasta();
    //randomFasta(2, 3333);
    //parseGFF();
//    randomFastqUsingGFF("test.fa", "test.gff");
    

    return 0;
}

