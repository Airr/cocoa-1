//
//  samtools.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_samtools_h
#define alder_align_samtools_h

#include <stdio.h>
#include "../fasta/alder_fasta.h"
#include "../palign/alder_align_alignment.h"

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


FILE *alder_sam_open(const char *fn, const fasta_t *fh);
int alder_sam_print(FILE *fp, const alder_alignment_t *a);
void alder_sam_close(FILE *fp);
int alder_sam_check(const char *fn);

__END_DECLS

#endif
