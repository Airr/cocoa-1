//
//  alder_fastq_stat.h
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/31/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_adapter_alder_fastq_stat_h
#define alder_adapter_alder_fastq_stat_h

#include <stdint.h>
#include "bstrlib.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

typedef struct {
    int64_t n_total;   // Number of reads.
    int64_t n_trimmed; // count - reads trimmed.
    int64_t n_base;
    int64_t n_base_remained;
    bstring adapter_seq;
    int adapter_seq_id;
} alder_fastq_stat_t;

void alder_fastq_stat_init(alder_fastq_stat_t *stat);
void alder_fastq_stat_free(alder_fastq_stat_t *stat);
int alder_fastq_stat_log(alder_fastq_stat_t *stat);

__END_DECLS

#endif
