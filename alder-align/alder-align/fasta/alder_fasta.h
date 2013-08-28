//
//  fasta.h
//  deseq
//
//  Created by Sang Chul Choi on 7/24/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef deseq_fasta_h
#define deseq_fasta_h

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

////////////////////////////////////////////////////////////////////////////////
// Usage: readFasta and lengthSequenceFasta
////////////////////////////////////////////////////////////////////////////////
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
    uint64_t numberOfSequence;
    uint64_t numberOfBase;
    uint64_t sizeCapacity;
    uint64_t sizeOfDataWithDollar;
    uint64_t *index;
    char *data;
} fasta_t;

// pFastaDataStructure: allocated fasta_t.
// lengthSequenceFasta must be used to know the size of fasta_t.

/** Reads sequences in a fasta format file.
 *  \param fastaFilename File path of a fasta format file.
 *  \param pFastaDataStructure Fasta file storage variable. This variable was
 *         already allocated. See alder_fasta_length.
 *
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
const char *fasta = "a.fa";
unsigned int ui1 = 0;
unsigned int ui2 = 0;
fasta_t genomeFasta;
alder_fasta_length(fasta, &ui1, &ui2);
genomeFasta.numberOfSequence = ui1;
genomeFasta.numberOfBase = ui2;
genomeFasta.index = (unsigned int*)malloc(ui1*sizeof(unsigned int));
genomeFasta.data = (char*)malloc(ui2*sizeof(char));
printf("Total number of sequences: %u\n", ui1);
printf("Total length of sequences: %u\n", ui2);
alder_fasta_read(fasta, &genomeFasta);
for (i = 0; i < genomeFasta.numberOfBase; i++) {
    printf("%c", genomeFasta.data[i]);
}
printf("\n");
free(genomeFasta.index);
free(genomeFasta.data);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
int alder_fasta_read(const char* fastaFilename, fasta_t* pFastaDataStructure);

/** Determines the number of sequences and the number of bases in a fasta-format
 *  file.
 *  \param n1 Number of sequences.
 *  \param n2 Number of bases.
 */
int alder_fasta_length(const char* fastaFilename, uint64_t *n1, uint64_t *n2);

/** Creates fasta_t using a fasta-format file.
 *  \param fastaFilename File path of a fasta format file.
 *  \return Fasta sequences data.
 */
fasta_t *alder_fasta_alloc(const char* fastaFilename);

/** Frees fasta_t.
 *  \param pFastaDataStructure Fasta sequence data.
 */
void alder_fasta_free(fasta_t* pFastaDataStructure);


/** Prints out the content of a fasta format file by sampling bases.
 *  \param n1 The number of sequences.
 *  \param n2 The number of lines per sequence (60 bp per line).
 */
int randomFasta(unsigned int n1, unsigned int n2);

__END_DECLS

#endif
