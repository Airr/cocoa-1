//
//  main.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/7/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <nglogc/log.h>
#include "alder_genome.h"
#include "PackedArray.h"
#include "alder_suffix_array.h"
#include "alder_logger.h"
#include "alder_index_genome.h"
#include "alder_align_reads.h"
#include "Parameters.h"
#include "../mem/sparse_sa_test.h"

int main(int argc, const char * argv[])
{
//    alder_test_suffix_array();
//    alder_test_bit_operations();
//    alder_load_genome (NULL);
    alder_logger_initialize();
    logc_log(MAIN_LOGGER, LOG_BASIC,
             "alder-align starts.");
//    Parameters *P = alder_parameter_init();
    
//    alder_index_genome(P);
//    alder_align_reads(P);
    
//    const char *argvt[3] = {"main", "iss", "mississippi"};
//    sparse_sa_test(3, argvt);
    
//    alder_parameter_free(P);
    alder_logger_finalize();
    return 0;
}

