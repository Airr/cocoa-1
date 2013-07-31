//
//  deseq.c
//  deseq
//
//  Created by Sang Chul Choi on 7/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <gsl/gsl_sf_bessel.h>

int testfunction (void)
{
    double x = 5.0;
    double y = gsl_sf_bessel_J0 (x);
    printf ("J0(%g) = %.18e\n", x, y);
    return 0;
}

int simulateDESeqCountData()
{
    return 0;
}