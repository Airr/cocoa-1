//
//  main.rl
//  gff
//
//  Created by Sang Chul Choi on 7/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 10000
enum { GFFELEMENTSIZE = 100 };

int main(int argc, const char * argv[])
{
    typedef struct mRNA_t {
        unsigned int i;
        char seqid[GFFELEMENTSIZE];
        char feature[GFFELEMENTSIZE];
        int start;
        int end;
        char attribute_tag[GFFELEMENTSIZE];
        char attribute_value[GFFELEMENTSIZE];
        char geneid[GFFELEMENTSIZE];
        char name[GFFELEMENTSIZE];
    } mRNA_t;
    mRNA_t mRNAInfo;
    
%% machine gff;
%% include gff "gff.rh";
    
    int i;
    int cs = 0;
    int have = 0;
    char buf[BUFSIZE];
    char stringBuf[BUFSIZE];
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
        len = fread( data, 1, space, stdin );
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
            printf("I: %s\t%s\t%d\t%d\t%s\t%s\n",
                   mRNAInfo.seqid, mRNAInfo.feature,
                   mRNAInfo.start, mRNAInfo.end, mRNAInfo.name, mRNAInfo.geneid);
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
