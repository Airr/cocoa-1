//
//  gsl_block_anchor_test.c
//  gsl
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "gsl_block_anchor.h"

int gsl_block_anchor_test()
{
    gsl_block_anchor *b = gsl_block_anchor_alloc(2);
    b->data[0].anchor1 = 1;
    b->data[0].anchor2 = 2;
    b->data[0].anchorLength = 3;
    b->data[0].anchorScore = 4.0;
    b->data[1].anchor1 = 10;
    b->data[1].anchor2 = 20;
    b->data[1].anchorLength = 30;
    b->data[1].anchorScore = 40.0;
    FILE *fp = fopen("anchor.txt","w");
    gsl_block_anchor_fprintf(fp, b, 0);
    fclose(fp);
    gsl_block_anchor_free(b);
    
    b = gsl_block_anchor_alloc(2);
    fp = fopen("anchor.txt", "r");
    gsl_block_anchor_fscanf(fp, b);
    fclose(fp);
    gsl_block_anchor_fprintf(stdout, b, 0);
    gsl_block_anchor_free(b);
    return 0;
}
