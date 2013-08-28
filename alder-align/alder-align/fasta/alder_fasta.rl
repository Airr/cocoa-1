#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <gsl/gsl_errno.h>

#include "alder_fasta.h"
#include "alder-align/rng/rvgs.h"

#define BUFSIZE 10000
enum { GFFELEMENTSIZE = 100 };
static uint64_t sSizeCapacity = 100;

fasta_t *alder_fasta_alloc(const char* fastaFilename)
{
    int status;
    uint64_t ui1 = 0;
    uint64_t ui2 = 0;
    fasta_t *v = malloc(sizeof(fasta_t));
    if (v == NULL) return NULL;
    alder_fasta_length(fastaFilename, &ui1, &ui2);
    v->numberOfSequence = ui1;
    v->numberOfBase = ui2;
    v->sizeCapacity = ui2 + sSizeCapacity;
    v->index = malloc(ui1*sizeof(unsigned int));
    if (v->index == NULL) {
        free(v);
        return NULL;
    }
    v->data = malloc(v->sizeCapacity*sizeof(char));
    if (v->data == NULL) {
        free(v);
        free(v->index);
        return NULL;
    }
    status = alder_fasta_read(fastaFilename, v);
    if (status != GSL_SUCCESS) {
        free(v->data);
        free(v->index);
        free(v);
        return NULL;
    }
    return v;
}

void alder_fasta_free(fasta_t* pFastaDataStructure)
{
    free(pFastaDataStructure->index);
    free(pFastaDataStructure->data);
    free(pFastaDataStructure);
}

int alder_fasta_read(const char* fastaFilename, fasta_t* pFastaDataStructure)
{
    %% machine fasta;
    %% include fasta "alder_fasta.rh";
    
    size_t n1 = 0;
    uint64_t n2 = 0;
    int cs;
    int have = 0;
    char buf[BUFSIZE];

    
    FILE *fin = fopen(fastaFilename, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "Error: could not open %s\n", fastaFilename);
        GSL_ERROR ("failed to allocate space for vector struct", GSL_ENOMEM);
    }
    %% write init;
    while ( 1 ) {
        char *p, *pe, *data = buf + have;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "BUFFER OUT OF SPACE\n");
            GSL_ERROR ("failed to allocate space for vector struct", GSL_ENOMEM);
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
    
    return GSL_SUCCESS;
}

int alder_fasta_length(const char* fastaFilename, uint64_t *n1, uint64_t *n2)
{
    %% machine fastaSize;
    %% include fastaSize "alder_fasta.rh";
    
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    *n1 = 0;
    *n2 = 0;
    
    FILE *fin = fopen(fastaFilename, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "Error: could not open %s\n", fastaFilename);
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
    
    
    return 0;
}

int randomFasta(unsigned int n1, unsigned int n2)
{
    enum { LENSEQUENCEPERLINE = 60 };
    char sequencePerLine[LENSEQUENCEPERLINE + 1];
    char acgt[4] = { 'A', 'C', 'G', 'T' };
    unsigned int i, j, k;
    long l;
    
    sequencePerLine[LENSEQUENCEPERLINE] = '\0';
    for (i = 0; i < n1; i++) {
        fprintf(stdout, ">%u\n", i);
        for (j = 0; j < n2; j++)
        {
            for (k = 0; k < LENSEQUENCEPERLINE; k++) {
                l = Equilikely(0,3);
                sequencePerLine[k] = acgt[l];
            }
            fprintf(stdout, "%s\n", sequencePerLine);
        }
    }
    
    return 0;
}