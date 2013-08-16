    //
//  alder_genome.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_genome_h
#define alder_align_alder_genome_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_const.h"
#include "PackedArray.h"

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

struct alder_genome_struct
{
    uint8_t *G;
    uint8_t *sigG;
    alder_block_suffix_array *SA;
    alder_block_suffix_array *SAi;
    
    uint8_t *G1; //pointer -200 of G
};

typedef struct alder_genome_struct alder_genome;

void alder_load_genome (const char *genomeDirectory);

__END_DECLS

#endif
