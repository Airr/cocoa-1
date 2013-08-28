//
//  gsl_block_match_test.c
//  gsl
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include "gsl_block_match.h"
#include "gsl_block_match_test.h"

void gsl_block_match_test ()
{
    char *testFile = "gsl_block_match_test.out";
    char *testBinaryFile = "gsl_block_match_test.bin";
    
    // Create a match block of size 0.
//    gsl_error_handler_t *old_handler = gsl_set_error_handler_off();
//    gsl_block_match *b2 = gsl_block_match_alloc(0);
//    assert(b2 != NULL);
//    gsl_set_error_handler(old_handler);
    
    // Create a match block of size 1, and add a null match.
    gsl_block_match *b = gsl_block_match_alloc(3);
    alder_match_t *d = gsl_block_match_data(b);
    d[0].query = 1;
    d[0].ref = 2;
    d[0].len = 3;
    d[1].query = 1;
    d[1].ref = 2;
    d[1].len = 3;
    d[2].query = 1;
    d[2].ref = 2;
    d[2].len = 3;
    
    FILE *fout = fopen(testFile, "w");
    gsl_block_match_fprintf(fout, b, NULL);
    fclose(fout);
    
    gsl_block_match *b2 = gsl_block_match_alloc(3);
    FILE *fin = fopen(testFile, "r");
    gsl_block_match_fscanf(fin, b2);
    gsl_block_match_fprintf(stdout, b2, NULL);
    fclose(fin);
    
    fout = fopen(testBinaryFile, "wb");
    gsl_block_match_fwrite(fout, b);
    fclose(fout);
    
    gsl_block_match *b3 = gsl_block_match_alloc(3);
    fin = fopen(testBinaryFile, "rb");
    gsl_block_match_fread(fin, b3);
    fclose(fin);
    gsl_block_match_fprintf(stdout, b3, NULL);
    
    gsl_block_match_free(b);
    gsl_block_match_free(b2);
    gsl_block_match_free(b3);
}