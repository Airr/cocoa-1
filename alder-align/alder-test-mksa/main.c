//
//  main.c
//  alder-test-mksa
//
//  Created by Sang Chul Choi on 8/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <nglogc/log.h>
#include "alder_logger.h"
#include "../alder-align/mem/sparse_sa.h"
#include "../alder-align/fasta/alder_fasta.h"

int main(int argc, const char * argv[])
{
    alder_logger_initialize();
    logc_log(MAIN_LOGGER, LOG_BASIC, "alder-align starts.");
    
    alder_sparse_sa_t *sparseSA = alder_sparse_sa_alloc_file("data/test.fa", 2);
    
    alder_sparse_sa_free(sparseSA);
    alder_logger_finalize();
    return 0;
}

