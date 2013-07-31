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


int randomFastqUsingGFF(const char* fastaFilename, const char* gffFilename);

int randomFastqUsingFasta(const char* fastaFilename,
                          unsigned int numberOfRead);

#endif
