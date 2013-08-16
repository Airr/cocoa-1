//
//  alder_align_reads.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/11/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_align_reads_h
#define alder_align_alder_align_reads_h

#include "Parameters.h"

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

int alder_align_reads (Parameters *p);

__END_DECLS

#endif
