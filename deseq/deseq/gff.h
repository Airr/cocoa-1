//
//  gff.h
//  deseq
//
//  Created by Sang Chul Choi on 7/24/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////
// Usage: randomGFF
////////////////////////////////////////////////////////////////////////////////
//        randomGFF(2, 200000, 1000, 100, 1000, 100, 2000, 200);
//
////////////////////////////////////////////////////////////////////////////////
// Usage: parseGFFLength and parseGFF.
////////////////////////////////////////////////////////////////////////////////
//    parseGFFLength(gffFilename, &numberOfGene, &numberOfmRNA, &numberOfExon);
//    genomeGFF.numberOfGene = numberOfGene;
//    genomeGFF.numberOfmRNA = numberOfmRNA;
//    genomeGFF.numberOfExon = numberOfExon;
//    genomeGFF.gene = (gff_gene_t*)malloc(numberOfGene*sizeof(gff_gene_t));
//    genomeGFF.mRNA = (gff_mRNA_t*)malloc(numberOfmRNA*sizeof(gff_mRNA_t));
//    genomeGFF.exon = (gff_exon_t*)malloc(numberOfExon*sizeof(gff_exon_t));
//    for (i = 0; i < numberOfGene; i++)
//    {
//        genomeGFF.gene[i].seqid = NULL;
//        genomeGFF.gene[i].id = NULL;
//        genomeGFF.gene[i].mRNA = NULL;
//    }
//    for (i = 0; i < numberOfmRNA; i++)
//    {
//        genomeGFF.mRNA[i].id = NULL;
//        genomeGFF.mRNA[i].exon = NULL;
//    }
//    for (i = 0; i < numberOfExon; i++)
//    {
//        genomeGFF.exon[i].id = NULL;
//    }
//    parseGFF(gffFilename, &genomeGFF);
//
//    for (i = 0; i < numberOfGene; i++)
//    {
//        if (genomeGFF.gene[i].seqid != NULL)
//            free(genomeGFF.gene[i].seqid);
//            if (genomeGFF.gene[i].id != NULL)
//                free(genomeGFF.gene[i].id);
//                if (genomeGFF.gene[i].mRNA != NULL)
//                    free(genomeGFF.gene[i].mRNA);
//                    }
//    for (i = 0; i < numberOfmRNA; i++)
//    {
//        if (genomeGFF.mRNA[i].id != NULL)
//            free(genomeGFF.mRNA[i].id);
//            if (genomeGFF.mRNA[i].exon != NULL)
//                free(genomeGFF.mRNA[i].exon);
//                }
//    for (i = 0; i < numberOfExon; i++)
//    {
//        if (genomeGFF.exon[i].id != NULL)
//            free(genomeGFF.exon[i].id);
//            }
//    free(genomeGFF.gene);
//    free(genomeGFF.mRNA);
//    free(genomeGFF.exon);
//
////////////////////////////////////////////////////////////////////////////////


#ifndef deseq_gff_h
#define deseq_gff_h

#define BUFSIZE 10000
enum { GFFELEMENTSIZE = 100 };

// A single line in a GFF file.
typedef struct mRNA_t {
    unsigned int i;
    char seqid[GFFELEMENTSIZE];
    char id[GFFELEMENTSIZE];
    char feature[GFFELEMENTSIZE];
    int start;
    int end;
    char strand;
    char attribute_tag[GFFELEMENTSIZE];
    char attribute_value[GFFELEMENTSIZE];
    char geneid[GFFELEMENTSIZE];
    char name[GFFELEMENTSIZE];
    char parent[GFFELEMENTSIZE];
} mRNA_t;

// An exon in a GFF file.
typedef struct gff_exon_t {
    unsigned int start;
    unsigned int end;
    char *id;
} gff_exon_t;

// A mRNA in a GFF file.
typedef struct gff_mRNA_t {
    unsigned int start;
    unsigned int end;
    char *id;
    unsigned int numberOfExon;
    unsigned int *exon;
} gff_mRNA_t;

// A gene in a GFF file.
typedef struct gff_gene_t {
    char *seqid;
    unsigned int start;
    unsigned int end;
    char strand; // +, -, .
    char *id;
    unsigned int numberOfmRNA;
    unsigned int *mRNA;
} gff_gene_t;


// A whole gff for genes.
typedef struct gff_t {
    unsigned int numberOfGene;
    unsigned int numberOfmRNA;
    unsigned int numberOfExon;
    gff_gene_t *gene;
    gff_mRNA_t *mRNA;
    gff_exon_t *exon;
} gff_t;


// numberOfChromosome: number of chromosome
// numberOfBase: number of bases per chromosome
// sizeOfGene: size of genes
// sizeOfExon: size of exons
// minDistanceBetweenGene: minimum distance between genes
// minDistanceBetweenExon: minimum distance between exons
// maxDistanceBetweenGene: maximum distance between genes
// maxDistanceBetweenExon: maximum distance between exons
int randomGFF(unsigned int numberOfChromosome,
              unsigned int numberOfBase,
              unsigned int sizeOfGene,
              unsigned int sizeOfExon,
              unsigned int minDistanceBetweenGene,
              unsigned int minDistanceBetweenExon,
              unsigned int maxDistanceBetweenGene,
              unsigned int maxDistanceBetweenExon);

int parseGFF(const char* gffFilename, gff_t* pGFFDataStructure);
int parseGFFLength(const char* gffFilename, unsigned int *n1, unsigned int *n2, unsigned int *n3);

#endif
