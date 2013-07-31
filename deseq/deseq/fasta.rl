#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fasta.h"
#include "rvgs.h"

#define BUFSIZE 10000
enum { GFFELEMENTSIZE = 100 };

int readFasta(const char* fastaFilename, fasta_t* pFastaDataStructure)
{
    %% machine fasta;
    %% include fasta "fasta.rh";
    
    unsigned int n1 = 0;
    unsigned int n2 = 0;
    int cs;
    int have = 0;
    char buf[BUFSIZE];

    
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

int lengthSequenceFasta(const char* fastaFilename, unsigned int* n1, unsigned int* n2)
{
    %% machine fastaSize;
    %% include fastaSize "fasta.rh";
    
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