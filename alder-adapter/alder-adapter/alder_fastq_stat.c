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
    stat->adapter_seq = NULL;
}

void alder_fastq_stat_free(alder_fastq_stat_t *stat)
{
    bdestroy(stat->adapter_seq);
}

int alder_fastq_stat_log(alder_fastq_stat_t *stat)
{
    logc_log(MAIN_LOGGER, LOG_INFO, "Adapter [%d]: %s",
             stat->adapter_seq_id,
             bdata(stat->adapter_seq));
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
    
    return 0;
}
