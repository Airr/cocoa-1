//
//  fastq.rl
//  deseq
//
//  Created by Sang Chul Choi on 7/24/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fasta.h"
#include "gff.h"
#include "fastaStream.h"
#include "rvgs.h"

enum { SIZEOFREADFASTQ = 100 };

int randomFastqUsingGFF(const char* fastaFilename, const char* gffFilename)
{
    // Create mRNAs to create short reads.
    int seqid;
    unsigned int chromosomeOffset;
    char buf[100000];
    unsigned int i, j, k, jj, kk;
    unsigned int start;
    unsigned int end;
    unsigned int numberOfGene;
    unsigned int numberOfmRNA;
    unsigned int numberOfExon;
    gff_t genomeGFF;
    
    // Load the genome sequence in FASTA.
    unsigned int ui1 = 0;
    unsigned int ui2 = 0;
    fasta_t genomeFasta;
    lengthSequenceFasta(fastaFilename, &ui1, &ui2);
    genomeFasta.numberOfSequence = ui1;
    genomeFasta.numberOfBase = ui2;
    genomeFasta.index = (unsigned int*)malloc(ui1*sizeof(unsigned int));
    genomeFasta.data = (char*)malloc(ui2*sizeof(char));
    printf("Total number of sequences: %u\n", ui1);
    printf("Total length of sequences: %u\n", ui2);
    readFasta(fastaFilename, &genomeFasta);
    
    // Use the annotation in GFF.
    parseGFFLength(gffFilename, &numberOfGene, &numberOfmRNA, &numberOfExon);
    genomeGFF.numberOfGene = numberOfGene;
    genomeGFF.numberOfmRNA = numberOfmRNA;
    genomeGFF.numberOfExon = numberOfExon;
    genomeGFF.gene = (gff_gene_t*)malloc(numberOfGene*sizeof(gff_gene_t));
    genomeGFF.mRNA = (gff_mRNA_t*)malloc(numberOfmRNA*sizeof(gff_mRNA_t));
    genomeGFF.exon = (gff_exon_t*)malloc(numberOfExon*sizeof(gff_exon_t));
    for (i = 0; i < numberOfGene; i++)
    {
        genomeGFF.gene[i].seqid = NULL;
        genomeGFF.gene[i].id = NULL;
        genomeGFF.gene[i].mRNA = NULL;
    }
    for (i = 0; i < numberOfmRNA; i++)
    {
        genomeGFF.mRNA[i].id = NULL;
        genomeGFF.mRNA[i].exon = NULL;
    }
    for (i = 0; i < numberOfExon; i++)
    {
        genomeGFF.exon[i].id = NULL;
    }
    parseGFF(gffFilename, &genomeGFF);
//    for (i = 0; i < genomeFasta.numberOfBase; i++)
//    {
//        printf("%c", genomeFasta.data[i]);
//    }
//    printf("\n");
//    for (i = 0; i < genomeFasta.numberOfSequence; i++) {
//        printf("%u ", genomeFasta.index[i]);
//    }
//    printf("\n");
    for (i = 0; i < numberOfGene; i++)
    {
//        printf("%s: %u\n", genomeGFF.gene[i].id, genomeGFF.gene[i].numberOfmRNA);
        seqid = atoi(genomeGFF.gene[i].seqid);
        chromosomeOffset = genomeFasta.index[seqid];
        for (j = 0; j < genomeGFF.gene[i].numberOfmRNA; j++)
        {
            jj = genomeGFF.gene[i].mRNA[j];
//            printf("\t%s: %u\n", genomeGFF.mRNA[jj].id, genomeGFF.mRNA[jj].numberOfExon);
            printf(">%s\n", genomeGFF.mRNA[jj].id);
            
            kk = genomeGFF.mRNA[jj].exon[0];
            start = genomeGFF.exon[kk].start - 1;
            end = genomeGFF.exon[kk].end;
            strncpy(buf, genomeFasta.data + chromosomeOffset + start, end - start);
            buf[end - start] = '\0';
//            printf("\t\t%s %u - %u\n", genomeGFF.exon[kk].id, start, end);
            for (k = 1; k < genomeGFF.mRNA[jj].numberOfExon; k++)
            {
                kk = genomeGFF.mRNA[jj].exon[k];
                start = genomeGFF.exon[kk].start - 1;
                end = genomeGFF.exon[kk].end;
                strncat(buf, genomeFasta.data + chromosomeOffset + start, end - start);
//                printf("\t\t%s %u - %u\n", genomeGFF.exon[kk].id, start, end);
            }
            printf("%s\n", buf);
            
            // Now, create FASTQ files.
        }
    }
    
    
    // Deallocate the memory of genomeGFF and genomeFasta.
    for (i = 0; i < numberOfGene; i++)
    {
        if (genomeGFF.gene[i].seqid != NULL)
            free(genomeGFF.gene[i].seqid);
        if (genomeGFF.gene[i].id != NULL)
            free(genomeGFF.gene[i].id);
        if (genomeGFF.gene[i].mRNA != NULL)
            free(genomeGFF.gene[i].mRNA);
    }
    for (i = 0; i < numberOfmRNA; i++)
    {
        if (genomeGFF.mRNA[i].id != NULL)
            free(genomeGFF.mRNA[i].id);
        if (genomeGFF.mRNA[i].exon != NULL)
            free(genomeGFF.mRNA[i].exon);
    }
    for (i = 0; i < numberOfExon; i++)
    {
        if (genomeGFF.exon[i].id != NULL)
            free(genomeGFF.exon[i].id);
    }
    free(genomeGFF.gene);
    free(genomeGFF.mRNA);
    free(genomeGFF.exon);
    free(genomeFasta.index);
    free(genomeFasta.data);
    
    return 0;
}

int randomFastqUsingFasta(const char* fastaFilename,
                          unsigned int numberOfRead)
{
    long l;
    unsigned int i;
    unsigned int indexRead;
    size_t lengthOfmRNA;
    int isNext = 1;
    struct fastaFormat fsm;
    char readOfFastq[SIZEOFREADFASTQ + 1];
    char qualityOfFastq[SIZEOFREADFASTQ + 1];
    FILE *fin;
    
    fin = fopen(fastaFilename, "r");
    fastaStreamInitialize( &fsm, fin );
    indexRead = 0;
    for (i = 0; i < SIZEOFREADFASTQ; i++)
    {
        qualityOfFastq[i] = 'I';
    }
    qualityOfFastq[SIZEOFREADFASTQ] = '\0';
    while ( isNext == 1 ) {
        isNext = fastaStreamNext( &fsm, fin );
        lengthOfmRNA = strlen(fsm.buf);
        for (i = 0; i < numberOfRead; i++)
        {
            l = Equilikely(0, lengthOfmRNA - SIZEOFREADFASTQ);
            strncpy(readOfFastq, fsm.buf + l, SIZEOFREADFASTQ);
            readOfFastq[SIZEOFREADFASTQ] = '\0';
            indexRead++;
            fprintf(stdout, "@%u\n%s\n+\n%s\n", indexRead, readOfFastq, qualityOfFastq);
        }
    }
    fastaStreamFinalize( &fsm );
    fclose(fin);
    return 0;
}










