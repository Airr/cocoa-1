//
//  gsl_block_string_test.c
//  gsl
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include "gsl_block_string.h"

void gsl_block_string_test ()
{
    int status;
    size_t n;
    char *testFile = "gsl_block_string_test.out";
    char *testBinaryFile = "gsl_block_string_test.bin";
    
    // Create a string block of size 0.
    gsl_error_handler_t *old_handler = gsl_set_error_handler_off();
    gsl_block_string *b2 = gsl_block_string_alloc(0);
    assert(b2 != NULL);
    gsl_set_error_handler(old_handler);
    
    // Create a string block of size 1, and add a null string.
    gsl_block_string *b = gsl_block_string_alloc(1);
    old_handler = gsl_set_error_handler_off();
    status = gsl_block_string_add_string(b, NULL);
    assert(status == GSL_EINVAL);
    gsl_set_error_handler(old_handler);
    
    // Add a string "first" to the vector.
    gsl_block_string_add_string(b, "first");
    gsl_block_string_fprintf(stdout, b, "%s");
    
    // Add one more item.
    gsl_block_string_add_string(b, "second");
    n = gsl_block_string_size(b);
    assert(n == 2);
    
    // Add one more and save the items to a file.
    gsl_block_string_add_string(b, "third");
    FILE *fout = fopen(testFile, "w");
    gsl_block_string_fprintf(fout, b, "[%s]");
    fclose(fout);
    
    // Create a second vector of size 3, and read the content of the file.
    gsl_block_string_realloc(b2, 3);
    FILE *fin = fopen(testFile, "r");
    gsl_block_string_fscanf(fin, b2);
    fclose(fin);
    gsl_block_string_fprintf(stdout, b2, "%s");
    
    // Save the second vector in a binary file.
    fout = fopen(testBinaryFile, "wb");
    gsl_block_string_fwrite(fout, b2);
    fclose(fout);
    
    // Read the binary file in the first vector.
    fin = fopen(testBinaryFile, "rb");
    gsl_block_string_fread(fin, b);
    fclose(fout);
    gsl_block_string_fprintf(stdout, b, "%s");
    
    // Free the used string vectors.
    gsl_block_string_free(b);
    gsl_block_string_free(b2);

}