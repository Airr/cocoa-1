//
//  File.c
//  gsl
//
//  Created by Sang Chul Choi on 8/29/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "gsl_vector_anchor.h"

void gsl_vector_anchor_test()
{
   
    alder_anchor_t m;
    m.anchor1 = 1; m.anchor2 = 2; m.anchorLength = 3; m.anchorScore = 4.0;
    
    gsl_vector_anchor *v = gsl_vector_anchor_init();
    for (size_t i = 0; i < 1000; i++) {
        gsl_vector_anchor_add(v, m);
        printf("[%4zd] v-b size: [%zd - %zd]\n", i, v->size, v->block->size);
    }
    
    gsl_vector_anchor_fprintf(stdout, v, 0);
    
    gsl_vector_anchor_free(v);
    
}