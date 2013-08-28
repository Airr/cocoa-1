//
//  fasta.h
//  deseq
//
//  Created by Sang Chul Choi on 7/24/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef deseq_fasta_h
#define deseq_fasta_h

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

////////////////////////////////////////////////////////////////////////////////
// Usage: readFasta and lengthSequenceFasta
////////////////////////////////////////////////////////////////////////////////
//    unsigned int ui1 = 0;
//    unsigned int ui2 = 0;
//    fasta_t genomeFasta;
//    lengthSequenceFasta(&ui1, &ui2);
//    genomeFasta.numberOfSequence = ui1;
//    genomeFasta.numberOfBase = ui2;
//    genomeFasta.index = (unsigned int*)malloc(ui1*sizeof(unsigned int));
//    genomeFasta.data = (char*)malloc(ui2*sizeof(char));
//    printf("Total number of sequences: %u\n", ui1);
//    printf("Total length of sequences: %u\n", ui2);
//    readFasta(&genomeFasta);
//    for (i = 0; i < genomeFasta.numberOfBase; i++)
//    {
//        printf("%c", genomeFasta.data[i]);
//    }
//    printf("\n");
//    free(genomeFasta.index);
//    free(genomeFasta.data);
//
////////////////////////////////////////////////////////////////////////////////
// Usage: randomFasta
////////////////////////////////////////////////////////////////////////////////
//    randomFasta(2, 3);
////////////////////////////////////////////////////////////////////////////////

// index: starting positions of sequences of size being numberOfSequence
//        (e.g, the first one is 0)
// data: sequence data of size being numberOfBase
typedef struct fasta_t {
    unsigned int numberOfSequence;
    unsigned int numberOfBase;
    unsigned int *index;
    char *data;
} fasta_t;

// pFastaDataStructure: allocated fasta_t.
// lengthSequenceFasta must be used to know the size of fasta_t.
int readFasta(const char* fastaFilename, fasta_t* pFastaDataStructure);

// n1: The number of sequences
// n2: The number of bases
int lengthSequenceFasta(const char* fastaFilename, unsigned int* n1, unsigned int* n2);



// n1: The number of sequences
// n2: The number of lines per sequence (60 bp per line)
int randomFasta(unsigned int n1, unsigned int n2);

__END_DECLS

#endif
