//
//  gsl_vector_seqpair_test.c
//  gsl
//
//  Created by Sang Chul Choi on 8/29/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "gsl_vector_seqpair.h"

void gsl_vector_seqpair_test()
{
    alder_anchor_t ma;
    ma.anchor1 = 1; ma.anchor2 = 2; ma.anchorLength = 3; ma.anchorScore = 4.0;
    gsl_vector_anchor *a;
    
    for (size_t j = 0; j < 10000; j++) {
        
        gsl_vector_seqpair *v = gsl_vector_seqpair_init();
        
        alder_seqpair_t m;
        
        for (size_t i = 0; i < 10000; i++) {
            a = gsl_vector_anchor_init(); gsl_vector_anchor_add(a, ma); gsl_vector_anchor_add(a, ma);
            m.anchor = a; m.seq1 = strdup("AAA"); m.seq2 = strdup("AAA");
            gsl_vector_seqpair_add(v, m);
        }
    //    gsl_vector_seqpair_fprintf(stdout, v, 0);
        
        gsl_vector_seqpair_free(v);
        
    }
}