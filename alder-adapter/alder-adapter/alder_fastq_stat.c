//
//  alder_fastq_stat.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/31/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "alder_logger.h"
#include "alder_fastq_stat.h"

void alder_fastq_stat_init(alder_fastq_stat_t *stat)
{
    stat->n_total = 0;
    stat->n_trimmed = 0;
    stat->n_base = 0;
    stat->n_base_remained = 0;
    stat->n_trimmed2 = 0;
    stat->n_base2 = 0;
    stat->n_base_remained2 = 0;
    stat->adapter_seq = NULL;
    stat->adapter_seq2 = NULL;
    stat->adapter_seq_id = -1;
    stat->adapter_seq_id2 = -1;
}

int alder_fastq_stat_log (alder_fastq_stat_t *stat,
                          const char *fnin, const char *fnout,
                          const char *fnin2, const char *fnout2,
                          const char *adapter_seq, const char *adapter_seq2)
                          
{
    if (fnin == NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "file: standard input");
    } else if (fnin != NULL && fnin2 == NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "file: %s", fnin);
    } else if (fnin != NULL && fnin2 != NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "file1: %s", fnin);
        logc_log(MAIN_LOGGER, LOG_INFO, "file2: %s", fnin2);
    }
    
    if (fnin == NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "file: standard input");
        logc_log(MAIN_LOGGER, LOG_INFO, "Adapter [%d]: %s",
                 stat->adapter_seq_id, stat->adapter_seq);
    } else if (fnin != NULL && fnin2 == NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "Adapter [%d]: %s",
                 stat->adapter_seq_id, stat->adapter_seq);
    } else if (fnin != NULL && fnin2 != NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "Adapter1 [%d]: %s",
                 stat->adapter_seq_id, stat->adapter_seq);
        logc_log(MAIN_LOGGER, LOG_INFO, "Adapter2 [%d]: %s",
                 stat->adapter_seq_id2, stat->adapter_seq2);
    }
    
    if (fnin == NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "Summary of file");
    } else if (fnin != NULL && fnin2 == NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "Summary of file");
    } else if (fnin != NULL && fnin2 != NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "Summary of file 1");
    }
    
    logc_log(MAIN_LOGGER, LOG_INFO, "Number of reads: %llu", stat->n_total);
    logc_log(MAIN_LOGGER, LOG_INFO, "Number of bases: %llu", stat->n_base);
    double d = (double) stat->n_trimmed / stat->n_total;
    logc_log(MAIN_LOGGER, LOG_INFO,
             "Number of trimmed reads: %llu (%lf)",
             stat->n_trimmed, d);
    d = (double) (stat->n_base - stat->n_base_remained) / stat->n_base;
    logc_log(MAIN_LOGGER, LOG_INFO,
             "Number of trimmed bases: %llu (%lf)",
             (stat->n_base - stat->n_base_remained), d);
    
    if (fnin2 != NULL) {
        logc_log(MAIN_LOGGER, LOG_INFO, "Summary of file 2");
        logc_log(MAIN_LOGGER, LOG_INFO, "Number of reads: %llu", stat->n_total);
        logc_log(MAIN_LOGGER, LOG_INFO, "Number of bases: %llu", stat->n_base);
        double d = (double) stat->n_trimmed / stat->n_total;
        logc_log(MAIN_LOGGER, LOG_INFO,
                 "Number of trimmed reads: %llu (%lf)",
                 stat->n_trimmed, d);
        d = (double) (stat->n_base - stat->n_base_remained) / stat->n_base;
        logc_log(MAIN_LOGGER, LOG_INFO,
                 "Number of trimmed bases: %llu (%lf)",
                 (stat->n_base - stat->n_base_remained), d);
    }
    
    return 0;
}
