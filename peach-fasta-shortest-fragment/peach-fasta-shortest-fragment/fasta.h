//
//  fasta.h
//  peach-fasta-shortest-fragment
//
//  Created by Sang Chul Choi on 8/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef peach_fasta_shortest_fragment_fasta_h
#define peach_fasta_shortest_fragment_fasta_h

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

typedef struct fasta_struct_t {
    unsigned int numberOfSequence;
    unsigned int numberOfBase;
    unsigned int *index;
    char *data;
} fasta_t;

int readFasta(const char* fastaFilename, fasta_t* pFastaDataStructure);
int lengthSequenceFasta(const char* fastaFilename, unsigned int* n1, unsigned int* n2);

__END_DECLS

#endif
