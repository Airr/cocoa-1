//
//  alder_palign.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_palign_h
#define alder_align_alder_palign_h

#include "../union/union_array.h"
#include "gsl_vector_match.h"
#include "../fasta/alder_fasta.h"

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
int alder_palign(const char *s, interval_t I, gsl_vector_match *mem, fasta_t *fS);
__END_DECLS

#endif
