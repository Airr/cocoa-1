//
//  alder_reference.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_reference_h
#define alder_align_alder_reference_h

#include "gsl_vector_match.h"
#include "alder_interval.h"

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

interval_t alder_reference_candidate_init();
void alder_reference_candidate_free(interval_t I);
void alder_reference_candidate_reset(interval_t I);
int alder_reference_candidate_find(interval_t I, gsl_vector_match *mem, int64_t L);

__END_DECLS

#endif
