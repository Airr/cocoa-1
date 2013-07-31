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
#include "fastaStream.h"

enum { BUFLEN = 3 };

int fastaStreamInitialize( struct fastaFormat *fsm, FILE *fin )
{
	fsm->buflen = BUFLEN;
	fsm->buf = (char*)malloc(fsm->buflen*sizeof(char));
    return 0;
}

int fastaStreamNext( struct fastaFormat *fsm, FILE *fin )
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

int fastaStreamFinalize( struct fastaFormat *fsm )
{
    if (fsm->buf != NULL && fsm->buflen > 0)
    {
        free(fsm->buf);
        fsm->buflen = 0;
    }
	return 0;
}




