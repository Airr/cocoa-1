//
//  fasta.c
//  peach-fasta-shortest-fragment
//
//  Created by Sang Chul Choi on 8/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fasta.h"

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
