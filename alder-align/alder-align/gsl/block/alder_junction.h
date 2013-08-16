//
//  alder_junction.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_junction_h
#define alder_align_alder_junction_h

#include <stdint.h>

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

struct alder_junction_struct
{
    uint64_t start;
    uint32_t gap;
    char strand;
    char motif;
    char annot;
    uint32_t countUnique;
    uint64_t countMultiple;
    uint16_t overhangLeft;
    uint16_t overhangRight;
};

typedef struct alder_junction_struct alder_junction;

__END_DECLS

#endif
