//
//  alder_adapter_cut_gzip.h
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/31/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_adapter_alder_adapter_cut_gzip_h
#define alder_adapter_alder_adapter_cut_gzip_h

#include "alder_fastq_stat.h"
#include "alder_adapter_cut_core.h"

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

int alder_adapter_cut_gzip(const char *fnin, const char *fnout,
                           const char *adapter_seq, const double error_rate,
                           const alder_adapter_option_t *option,
                           alder_fastq_stat_t *stat);
__END_DECLS

#endif
