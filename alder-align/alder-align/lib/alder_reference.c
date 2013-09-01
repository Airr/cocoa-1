//
//  alder_reference.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "gsl_vector_match.h"
#include "../union/union_array.h"
#include "alder_reference.h"

interval_t alder_reference_candidate_init()
{
    interval_t I = alder_union_interval_init();
    return I;
}

void alder_reference_candidate_free(interval_t I)
{
    alder_union_interval_free(I);
}

void alder_reference_candidate_reset(interval_t I)
{
    alder_union_interval_reset(I);
}

int alder_reference_candidate_find(interval_t I, gsl_vector_match *mem, int64_t L)
{
    for (size_t i = 0; i < mem->size; i++) {
        int64_t b = mem->data[i].ref - mem->data[i].query;
        int64_t e = mem->data[i].ref + (L - 1 - mem->data[i].query);
        alder_union_interval_add(&I, b, e);
    }
}
