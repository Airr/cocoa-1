//
//  fastaStream.c
//  deseq
//
//  Created by Sang Chul Choi on 7/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

enum { BUFLEN = 3 };

struct format
{
	char *buf;
	unsigned int buflen;    
};

int fastaStreamInitialize( struct format *fsm, FILE *fin )
{
	fsm->buflen = BUFLEN;
	fsm->buf = (char*)malloc(fsm->buflen*sizeof(char));
    return 0;
}

int fastaStreamNext( struct format *fsm, FILE *fin )
{
    char c;
    unsigned int n = 0;
    
    c = fgetc(fin);
    assert(c == '>');
    while (c != '\n') {
        c = fgetc(fin);
    }
    while (c != '>' && c != EOF) {
        c = fgetc(fin);
        if (isalnum(c) == 0) {
            continue;
        }
        fsm->buf[n] = c;
        n++;
        if (fsm->buflen <= n) {
            fsm->buflen += BUFLEN;
            fsm->buf = (char*)realloc(fsm->buf, fsm->buflen*sizeof(char));
        }
    }
    fsm->buf[n] = '\0';
    if (c == EOF) {
        return 0;
    } else {
        ungetc(c, fin);
        return 1;
    }
}

int fastaStreamFinalize( struct format *fsm )
{
    if (fsm->buf != NULL && fsm->buflen > 0)
    {
        free(fsm->buf);
        fsm->buflen = 0;
    }
	return 0;
}


struct format fsm;

int main(int argc, const char * argv[])
{
    int isNext = 1;
    FILE *fin = fopen("/Users/goshng/Library/Developer/Xcode/DerivedData/deseq-hcobcucskulqnzgasrjxhitcryvo/Build/Products/Debug/test.mRNA.fa", "r");
	fastaStreamInitialize( &fsm, fin );
	while ( isNext == 1 ) {
		isNext = fastaStreamNext( &fsm, fin );
        printf("%s\n", fsm.buf);
	}
    fastaStreamFinalize( &fsm );
	return 0;
}

