//
//  gsl_vector_string_test.c
//  gsl
//
//  Created by Sang Chul Choi on 8/10/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include "gsl_vector_string.h"

void gsl_vector_string_test ()
{
    int status;
//    size_t n;
    FILE *fout;
    FILE *fin;
    char *testFile = "gsl_vector_string_test.out";
    char *testBinaryFile = "gsl_vector_string_test.bin";
    gsl_vector_string *v = NULL;
    gsl_vector_string *v2 = NULL;
    gsl_vector_string *v3 = NULL;
    
    // Create a vector of size 3.
    v = gsl_vector_string_alloc(3);
    gsl_vector_string_add_string(v,"1");
    gsl_vector_string_add_string(v,"22");
    gsl_vector_string_add_string(v,"333");
    fout = fopen(testFile, "w");
    printf("Writing the first in %s.\n", testFile);
    gsl_vector_string_fprintf(fout, v, "[%s]");
    fclose(fout);

    // Create a second vector, and read the text file into the second.
    v2 = gsl_vector_string_alloc(3);
    fin = fopen(testFile, "r");
    gsl_vector_string_fscanf(fin, v2);
    fclose(fin);
    
    // Wrtie the second vector to a binary file.
    fout = fopen(testBinaryFile, "wb");
    gsl_vector_string_fwrite(fout, v2);
    fclose(fout);
    
    // Create one more vector, and read the binary file into it.
    v3 = gsl_vector_string_alloc(3);
    fin = fopen(testBinaryFile, "rb");
    gsl_vector_string_fread(fin, v3);
    fclose(fin);
    
    // Let's see the content of them.
    printf("The first vector is\n");
    gsl_vector_string_fprintf(stdout, v, "v :%s");
    printf("The second vector is\n");
    gsl_vector_string_fprintf(stdout, v2, "v2:%s");
    printf("The third vector is\n");
    gsl_vector_string_fprintf(stdout, v3, "v3:%s");
    
    // Copy the second to the first.
    gsl_vector_string_memcpy(v3, v);
    printf("Copying v to v3, and the v3 is now\n");
    gsl_vector_string_fprintf(stdout, v3, "v3:%s");
    
    // Swap v and v2.
    gsl_vector_string_swap(v, v2);
    printf("Swaping v to v2...\n");
    gsl_vector_string_fprintf(stdout, v, "v :%s");
    gsl_vector_string_fprintf(stdout, v2, "v2:%s");
    
    // Swap 1st and 2nd of v.
    gsl_vector_string_swap_elements(v, 0, 1);
    printf("Swaping 1st and 2nd element of v...\n");
    gsl_vector_string_fprintf(stdout, v, "v :%s");
    
    // Reverse the elements of v.
    gsl_vector_string_reverse(v);
    printf("Reversing v...\n");
    gsl_vector_string_fprintf(stdout, v, "v :%s");
    
    // Check if v and v2 are different:
    status = gsl_vector_string_equal(v, v2);
    assert(status == 0);
    gsl_vector_string_memcpy(v, v2);
    status = gsl_vector_string_equal(v, v2);
    assert(status == 1);
    printf("Copying v2 to v, and the v is now\n");    
    gsl_vector_string_fprintf(stdout, v, "v :%s");
    
    gsl_vector_string_free(v);
    gsl_vector_string_free(v2);
    gsl_vector_string_free(v3);
}
