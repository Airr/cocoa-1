//
//  fastq.h
//  deseq
//
//  Created by Sang Chul Choi on 7/24/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef deseq_fastq_h
#define deseq_fastq_h

////////////////////////////////////////////////////////////////////////////////
// Usage: randomFastqUsingGFF
// Description: Prepare test fasta and gff files.
//              This would create a FASTA file not FASTQ.
//    randomFastqUsingGFF("test.fa", "test.gff");
//
// Usage: creates a random FASTQ file from a FASTA file with mRNA sequences.
//    randomFastqUsingFasta("test.mRNA.fa", 10);

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

int randomFastqUsingGFF(const char* fastaFilename, const char* gffFilename);

int randomFastqUsingFasta(const char* fastaFilename,
                          unsigned int numberOfRead);

__END_DECLS

#endif
