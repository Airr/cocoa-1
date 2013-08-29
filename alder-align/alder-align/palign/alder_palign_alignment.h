//
//  alder_align_alignment.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_palign_alignment_h
#define alder_align_alder_palign_alignment_h

#include <stdint.h>
#include "../gsl/gsl_vector.anchor.h"

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

struct alder_alignment_struct {
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
typedef struct alder_alignment_struct alder_sam_t;

struct alder_sequencepair_struct {
    char *seq1;
    char *seq2;
    gsl_vector_anchor *anchor;
//    int *anchor1;
//    int *anchor2;
//    int *anchorLength;
//    double *anchorScore;
//    size_t numberOfAnchor;
    // We need this to be a gsl_vector or gsl_vector_anchor.
    // We also need gsl_vector_sequencepair.
    // This means that we need some kind of gsl_vector for C struct.
    // We first define a C struct.
    // Given a C struct, we want to create two files.
    // gsl_vector_{structrue name}.[ch].
};

typedef struct alder_sequencepair_struct alder_sequencepair_t;

__END_DECLS

#endif
