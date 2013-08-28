//
//  alder_align_alignment.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_align_alignment_h
#define alder_align_alder_align_alignment_h

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

typedef struct alder_alignment_struct {
    char     *qname;
    uint16_t flag;
    char     *rname;
    uint32_t pos;
    uint8_t  mapq;
    char     *cigar;
    char     *rnext;
    uint32_t pnext;
    int32_t  tlen;
    char     *seq;
    char     *qual;
};

typedef struct alder_alignment_struct alder_alignment_t;

__END_DECLS

#endif
