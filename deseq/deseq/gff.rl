//
//  gff.rl
//  deseq
//
//  Created by Sang Chul Choi on 7/24/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "gff.h"
#include "rvgs.h"

int randomGFF(unsigned int numberOfChromosome,
              unsigned int numberOfBase,
              unsigned int sizeOfGene,
              unsigned int sizeOfExon,
              unsigned int minDistanceBetweenGene,
              unsigned int minDistanceBetweenExon,
              unsigned int maxDistanceBetweenGene,
              unsigned int maxDistanceBetweenExon)
{
    long l;
    unsigned int i, j, k;
    unsigned int seqid;
    const char* source = "randomGFF";
    const char* type[5] = {
        "region", "gene", "mRNA", "exon"
    };
    unsigned int start;
    unsigned int end;
    char score = '.';
    char strand[4] = { '+', '-', '.', '?' };
    char phase[4] = { '.', '1', '2', '3' };
    char *attributes;
    unsigned int strandOfGene;
    unsigned int geneID = 0;
    unsigned int exonID = 0;
    unsigned int exonStart, exonEnd, geneStart, geneEnd;
    
    
    fprintf(stdout,
            "##gff-version 3\n"
            "#!gff-spec-version 1.20\n"
            "#!processor randomGFF by Sang Chul Choi\n");
    for (i = 0; i < numberOfChromosome; i++) {
        start = 1;
        end = numberOfBase;
        fprintf(stdout, "##sequence-region %u %u %u\n",
                i, start, numberOfBase);
        fprintf(stdout,
                "%u\t%s\t%s\t%u\t%u\t%c\t%c\t%c\tID=id%u\n",
                i, source, type[0], start, end, score, strand[0], phase[0], i);
        geneStart = start;
        geneEnd = start;
        while (geneEnd < numberOfBase) {
            l = Equilikely(minDistanceBetweenGene, maxDistanceBetweenGene);
            geneStart = geneEnd + l;
            l = Equilikely(sizeOfGene, 2*sizeOfGene);
            geneEnd = geneStart + l;
            if (geneEnd >= numberOfBase)
            {
                break;
            }
            strandOfGene = Equilikely(0,1);
            start = geneStart;
            end = geneEnd;
            fprintf(stdout,
                    "%u\t%s\t%s\t%u\t%u\t%c\t%c\t%c\tID=gene%u\n",
                    i, source, type[1], start, end, score,
                    strand[strandOfGene], phase[0], geneID);
            fprintf(stdout,
                    "%u\t%s\t%s\t%u\t%u\t%c\t%c\t%c\tID=rna%u;Parent=gene%u\n",
                    i, source, type[2], start, end, score,
                    strand[strandOfGene], phase[0], geneID, geneID);
            exonStart = geneStart;
            exonEnd = geneStart; // Just to enter the following while loop.
            while (exonEnd < geneEnd) {
                l = Equilikely(minDistanceBetweenExon, maxDistanceBetweenExon);
                exonStart = exonEnd + l;
                l = Equilikely(sizeOfExon, 2*sizeOfExon);
                exonEnd = exonStart + l;
                if (exonEnd < geneEnd)
                {
                    start = exonStart;
                    end = exonEnd;
                    fprintf(stdout,
                            "%u\t%s\t%s\t%u\t%u\t%c\t%c\t%c\tID=exon%u;Parent=rna%u\n",
                            i, source, type[3], start, end, score,
                            strand[strandOfGene], phase[0], exonID, geneID);
                    exonID++;
                }
            }
            geneID++;
        }
    }
}

int parseGFFLength(const char* gffFilename, unsigned int *n1, unsigned int *n2, unsigned int *n3)
{
    mRNA_t mRNAInfo;
    int i;
    int cs = 0;
    int have = 0;
    char buf[BUFSIZE];
    char stringBuf[BUFSIZE];
    unsigned int numberOfGene = 0;
    unsigned int numberOfmRNA = 0;
    unsigned int numberOfExon = 0;
    
    // Find numbers of genes, mRNAs, and exons.
    %% machine gffSize;
    %% include gffSize "gff.rh";
    FILE *fin = fopen(gffFilename, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "Error: could not open %s\n", gffFilename);
        return 0;
    }
    %% write init;
    while ( 1 ) {
        char *p, *pe, *data = buf + have;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "BUFFER OUT OF SPACE\n");
            exit(1);
        }
        len = fread( data, 1, space, fin );
        if ( len == 0 )
            break;
        /* Find the last newline by searching backwards. */
        p = buf;
        pe = data + len - 1;
        while ( *pe != '\n' && pe >= buf )
            pe--;
        pe += 1;
        %% write exec;
        /* How much is still in the buffer? */
        have = data + len - pe;
        if ( have > 0 )
            memmove( buf, pe, have );
        if ( len < space )
            break;
    }
    printf("Number of gene: %u\n", numberOfGene);
    printf("Number of mRNA: %u\n", numberOfmRNA);
    printf("Number of exon: %u\n", numberOfExon);

    *n1 = numberOfGene;
    *n2 = numberOfmRNA;
    *n3 = numberOfExon;
    
    return 0;
}


int parseGFF(const char* gffFilename, gff_t* pGFFDataStructure)
{
    mRNA_t mRNAInfo;
    unsigned int currentGeneID;
    unsigned int currentmRNAID;
    unsigned int geneID = 0;
    unsigned int mRNAID = 0;
    unsigned int exonID = 0;
    unsigned int valueUi, valueUj, valueUk;
    unsigned int foundUi;
    int i;
    int cs = 0;
    int have = 0;
    char buf[BUFSIZE];
    char stringBuf[BUFSIZE];
    char *pWord;

    
    %% machine gff;
    %% include gff "gff.rh";
    
    FILE *fin = fopen(gffFilename, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "Error: could not open %s\n", gffFilename);
        return 0;
    }
    while ( 1 )
    {
        char *eof;
        char *p, *pe, *data = buf + have;
        char *headOfLine;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "BUFFER OUT OF SPACE\n");
            exit(1);
        }
        len = fread( data, 1, space, fin );
        if (feof(stdin))
        {
            printf("END OF FILE\n");
        }
        printf("Buffer Len: %d\n", len);
        if ( len == 0 )
            break;
        
        /* Find the last newline by searching backwards. */
        p = buf;
        pe = buf;
        while ( *pe != '\n' && pe < data + len )
        {
            pe++;
        }
        if (pe == data + len)
        {
            fprintf(stderr,"Error: no end-of-line in the buffer\n");
            break;
        }
        else
        {
            pe += 1;
        }
        while (pe != NULL)
        {
            headOfLine = pe;
            strncpy(stringBuf, p, pe - p);
            stringBuf[pe-p] = '\0';
            printf("X: %s", stringBuf);
            for (i = 0; i < pe - p; i++)
            {
                printf("%X ", *(p + i));
            }
            printf("[%x]", *pe);
            printf("\n");
            %% write init;
            %% write exec;
            p = headOfLine;
            pe = headOfLine;
            while ( *pe != '\n' && pe < data + len )
            {
                pe++;
            }
            if (pe == data + len)
            {
                pe = NULL;
            }
            else
            {
                pe += 1;
            }
            printf("I: %s\t%s\t%d\t%d\t%s\t%s\t%s\n",
                   mRNAInfo.seqid, mRNAInfo.feature,
                   mRNAInfo.start, mRNAInfo.end, mRNAInfo.name, mRNAInfo.geneid,
                   mRNAInfo.parent);
            if (strcmp(mRNAInfo.feature, "gene") == 0)
            {
                pGFFDataStructure->gene[geneID].start = mRNAInfo.start;
                pGFFDataStructure->gene[geneID].end = mRNAInfo.end;
                pGFFDataStructure->gene[geneID].seqid = strdup(mRNAInfo.seqid);
                pGFFDataStructure->gene[geneID].id = strdup(mRNAInfo.id);
                pGFFDataStructure->gene[geneID].numberOfmRNA = 0;
                pGFFDataStructure->gene[geneID].mRNA =
                    (unsigned int*)malloc(100*sizeof(unsigned int));
                currentGeneID = geneID;
                geneID++;
            }
            else if (strcmp(mRNAInfo.feature, "mRNA") == 0)
            {
                pGFFDataStructure->mRNA[mRNAID].start = mRNAInfo.start;
                pGFFDataStructure->mRNA[mRNAID].end = mRNAInfo.end;
                pGFFDataStructure->mRNA[mRNAID].id = strdup(mRNAInfo.id);
                pGFFDataStructure->mRNA[mRNAID].numberOfExon = 0;
                pGFFDataStructure->mRNA[mRNAID].exon =
                    (unsigned int*)malloc(100*sizeof(unsigned int));
                // Complicated!!! Is there an easy way?
                valueUj = geneID - 1;
                valueUi = pGFFDataStructure->gene[valueUj].numberOfmRNA;
                pGFFDataStructure->gene[valueUj].mRNA[valueUi] = mRNAID;
                pGFFDataStructure->gene[valueUj].numberOfmRNA++;
                currentmRNAID = mRNAID;
                mRNAID++;
            }
            else if (strcmp(mRNAInfo.feature, "exon") == 0)
            {
                pGFFDataStructure->exon[exonID].start = mRNAInfo.start;
                pGFFDataStructure->exon[exonID].end = mRNAInfo.end;
                pGFFDataStructure->exon[exonID].id = strdup(mRNAInfo.id);
                // Complicated!!! Is there an easy way?
                // We could have multiple mRNAIDs.
                pWord = strtok(mRNAInfo.parent, ",");
                while (pWord != NULL)
                {
                    valueUk = pGFFDataStructure->gene[currentGeneID].numberOfmRNA;
                    foundUi = 0;
                    for (valueUj = currentmRNAID - valueUk + 1;
                         valueUj <= currentmRNAID;
                         valueUj++)
                    {
                        if (strcmp(pGFFDataStructure->mRNA[valueUj].id,pWord) == 0)
                        {
                            foundUi = 1;
                            break;
                        }
                    }
                    assert(foundUi == 1);
                    valueUi = pGFFDataStructure->mRNA[valueUj].numberOfExon;
                    pGFFDataStructure->mRNA[valueUj].exon[valueUi] = exonID;
                    pGFFDataStructure->mRNA[valueUj].numberOfExon++;
                    pWord = strtok(NULL, ",");
                }
                exonID++;
            }

        }
        printf("END OF WHILE\n");
        
        /* How much is still in the buffer? */
        have = data + len - headOfLine;
        if ( have > 0 )
            memmove( buf, headOfLine, have );
        if ( len < space )
            break;
    }
    return 0;
}
