//
//  gsl_vector_match_test.c
//  gsl
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "gsl_vector_match.h"
#include "gsl_vector_match_test.h"

void gsl_vector_match_test ()
{
    int status;
    size_t n;
    char *testFile = "gsl_block_match_test.out";
    char *testBinaryFile = "gsl_block_match_test.bin";
    
    // We need to create a vector with 0 elements.
    // We need to add an element to the vector.
    // We can iterate the vector.
    for (size_t i = 0; i < 100000; i++) {
        gsl_vector_match *v = gsl_vector_match_init();
        
        for (size_t j = 0; j < 1000; j++) {
            alder_match_t m = { .query = 1, .ref = 2, .len = 3 };
            gsl_vector_match_add_alder_match(v, m);
            m.query = 2, m.ref = 3, m.len = 4;
            gsl_vector_match_add_alder_match(v, m);
        }
//            gsl_vector_match_fprintf(stdout, v, NULL);
        
        gsl_vector_match_free(v);
    }
}
