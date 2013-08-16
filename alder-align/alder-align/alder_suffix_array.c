//
//  alder_suffix_array.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/8/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <gsl/gsl_rng.h>
#include "PackedArray.h"

const char *byte_to_binary(uint8_t x)
{
    static char b[9];
    b[8] = '\0';
    char *p = b;
    
    uint8_t z = 0x80;
//    printf("x = %x\n", x);
    for (uint64_t i = 0; i < 8; i++)
    {
//        printf("[%llu] %x\t", i, z);
        *p++ = (x & z) ? '1' : '0';
//        if (x & z)
//        {
//            printf("1\n");
//            *p = '1';
//        }
//        else
//        {
//            printf("0\n");
//            *p = '0';
//        }
//        p++;
        z >>= 1;
    }
    
    return b;
}

void alder_test_bit_operations ()
{
    char *tmp;
    char *a = "00100100";
    uint8_t b = (uint8_t)strtol(a, &tmp, 2);
    printf("%x\t%u\t%s\n", b, b, byte_to_binary(b));
    b = ~b;
    printf("%x\t%u\t%s\n", b, b, byte_to_binary(b));
    b = (0xf1)>>3;
    printf("%x\t%u\t%s\n", b, b, byte_to_binary(b));
    uint64_t c = 35LLU;
    uint64_t d = c/8LLU;
    uint64_t e = c%8LLU;
    printf("%llu %llu\n", d, e);
    
    uint8_t aa[32];
    for (uint8_t i = 0; i < 32; i++)
    {
        aa[i] = 0;
    }
    uint64_t *bb = (uint64_t*)(aa + 0);
    *bb = 255LLU;
    for (uint8_t i = 0; i < 32; i++)
    {
        if (i % 8 == 0)
            printf("\n");
        printf("%x ", aa[i]);
    }
    printf("\n");
}

void alder_test_suffix_array ()
{
    const uint64_t numberOfElement = 1000000000;
    const uint64_t stride = 35;
    uint64_t maximumOfElement = 1LLU<<stride;
    
    putenv("GSL_RNG_SEED=123");
    putenv("GSL_RNG_TYPE=taus");
    printf("Number of element  : %llu\n", numberOfElement);
    printf("Size of each elment: %llu\n", stride);
    printf("Maximum value      : %llu\n", maximumOfElement);
    assert(maximumOfElement == 34359738368LLU);
    
    const gsl_rng_type * T;
    gsl_rng * r;
    gsl_rng_env_setup();
    T = gsl_rng_default;
    r = gsl_rng_alloc (T);
    printf ("generator type: %s\n", gsl_rng_name (r));
    printf ("seed = %lu\n", gsl_rng_default_seed);
    printf ("first value = %lu\n", gsl_rng_get (r));
    unsigned long int randomV;
    FILE *fout = fopen("data/sa.0", "wb");
    for (uint64_t i = 0; i < numberOfElement; i++) {
        randomV = gsl_rng_get (r); // 0 ~ 2^35 - 1
        randomV %= maximumOfElement;
        fprintf(fout, "%lu", randomV);
        fprintf(fout, "\n");
//        if (i < numberOfElement - 1)
//            fprintf(fout, " ");
//        else
//            fprintf(fout, "\n");
    }
    fclose(fout);
    gsl_rng_free (r);
    
    alder_block_suffix_array * b = alder_block_suffix_array_alloc(numberOfElement, stride);
    printf ("length of block = %llu\n", b->size);
//    printf ("block data address = %#x\n", b->data);
    FILE *fin = fopen("data/sa.0", "r");
    alder_block_suffix_array_fscanf(fin, b);
    fclose(fin);

    fout = fopen("data/sa.1", "w");
    alder_block_suffix_array_fprintf(fout, b, "%llu");
    fclose(fout);
    
    fout = fopen("data/sa.bin", "wb");
    alder_block_suffix_array_fwrite(fout, b);
    fclose(fout);
    
    fin = fopen("data/sa.bin", "rb");
    alder_block_suffix_array_fread(fin, b);
    fclose(fin);
    
    fout = fopen("data/sa.2", "w");
    alder_block_suffix_array_fprintf(fout, b, "%llu");
    fclose(fout);
    alder_block_suffix_array_free(b);
}

