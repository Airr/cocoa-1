//
//  gsl_vector_sam_test.c
//  gsl
//
//  Created by Sang Chul Choi on 8/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gsl_vector_sam.h"

void gsl_vector_sam_test()
{
    gsl_vector_sam *v = gsl_vector_sam_alloc(3);
    alder_sam_t *s = gsl_vector_sam_ptr(v, 0);
    s->qname = strdup("qname");
    gsl_vector_sam_free(v);
}