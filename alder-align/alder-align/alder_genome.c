//
//  alder_genome.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "alder_genome.h"

#define LENGTH_GENOME_FILE 100

void alder_load_genome (const char *genomeDirectory)
{
    FILE *fin;
    time_t timing;
    time (&timing);
    
    fprintf(stdout, "Started loading the genome: %s\n", asctime (localtime (&timing)));
    
    size_t lengthOfGenomeFiles = strlen(genomeDirectory) + LENGTH_GENOME_FILE;
    char *pathToGenome = malloc(lengthOfGenomeFiles * sizeof(char));
    char *pathToSA = malloc(lengthOfGenomeFiles * sizeof(char));
    char *pathToSAIndex = malloc(lengthOfGenomeFiles * sizeof(char));
    strcpy(pathToGenome, genomeDirectory);
    strcpy(pathToSA, genomeDirectory);
    strcpy(pathToSAIndex, genomeDirectory);
    
    // Read SA and SAr.
    // We could create a binary file with all necessary files.
    // Size of SA - SA
    // Size of SAr - SAr
    // Size of genome
    // Size of SAIndex
    
    uint64_t nSAbyte = 1000000LLU;
    uint64_t nGenome = 1000000LLU;
    uint8_t GstrandBit = (uint8_t) floor(log(nGenome)/log(2))+1;
    if (GstrandBit < 32) GstrandBit = 32U;
    
    uint64_t GstrandMask = ~(1LLU<<GstrandBit);
    uint64_t nSA = (nSAbyte*8)/(GstrandBit+1LLU);
    alder_block_suffix_array * sa = alder_block_suffix_array_alloc(nSA, GstrandBit+1LLU);

    uint8_t SAiMarkNbit = GstrandBit+1;
    uint8_t SAiMarkAbsentBit = GstrandBit+2;
    
    uint64_t SAiMarkNmaskC      = 1LLU << SAiMarkNbit;
    uint64_t SAiMarkNmask       = ~SAiMarkNmaskC;
    uint64_t SAiMarkAbsentMaskC = 1LLU << SAiMarkAbsentBit;
    uint64_t SAiMarkAbsentMask = ~SAiMarkAbsentMaskC;
    
    uint64_t nSAi; // dfferent nSA, i means index.
    alder_block_suffix_array * saIndex = alder_block_suffix_array_alloc(nSAi, GstrandBit+3LLU);
    
//    *fin = fopen(pathToGenome, "rb");
//    alder_block_suffix_array_fread(fin, sa);
//    fclose(fin);
    fin = fopen(pathToSA, "rb");
    alder_block_suffix_array_fread(fin, sa);
    fclose(fin);
    fin = fopen(pathToSAIndex, "rb");
    alder_block_suffix_array_fread(fin, saIndex);
    fclose(fin);
    
    alder_block_suffix_array_free(sa);
    alder_block_suffix_array_free(saIndex);

    // splicing junction information
    free(pathToSA);
    free(pathToGenome);
    free(pathToSAIndex);
    time (&timing);
}
