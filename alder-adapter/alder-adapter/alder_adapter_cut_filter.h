//
//  alder_adapter_cut_filter.h
//  alder-adapter
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_adapter_alder_adapter_cut_filter_h
#define alder_adapter_alder_adapter_cut_filter_h

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

int alder_adapter_cut_filter(const char *adapter_seq, const int m,
                             bstring bSequence, bstring bQuality,
                             int *isFiltered,
                             int *start2_,
                             const alder_adapter_option_t *option);

__END_DECLS


#endif
