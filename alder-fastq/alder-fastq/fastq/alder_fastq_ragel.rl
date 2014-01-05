/**
 * This file, alder_fastq_ragel.c, is part of alder-fastq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fastq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fastq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alder_fastq_ragel.h"

%%{
    machine fastqBuild;
    action e_title {
//        bstrVectorAdd(af->name, "");
//        bseqname = af->name->entry[t1];
        printf("e[%c]", fc);
        (n1)++;
//        t1++;
    }
    action t_title {
//        bconchar(bseqname, fc);
        printf("t[%c]", fc);
    }
    action t_sequence {
        printf("%c", fc);
//        af->data[t2] = fc;
        (n2)++;
//        t2++;
    }
    action f_sequence {
//        af->index[t1-1] = t2;
        printf("f[%c]", fc);
        seq->f->stop = 1;
        goto _out;
    }
    
    title = print+ >e_title $t_title;
    sequence = ([ACGTURYSWKMBDHVNacgturyswkmbdhvn]+ $t_sequence)+ cntrl*;
    main := ('>' title cntrl+ sequence+ %f_sequence)+;
    write data;
}%%

rseq_t * alder_rseq_init (const char *fn, int type)
{
//    %% machine fastqBuild;
    
    FILE *fin = fopen(fn, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "cannot open %s\n", fn);
//        alder_fasta_list_free(af);
        return NULL;
    }
    
    int cs;
    %% write init;
    
    rseq_t *seq = malloc(sizeof(*seq));
    memset(seq,0,sizeof(*seq);
    seq->f = malloc(sizeof(*seq->f));
    memset(seq->f,0,sizeof(*seq->f));

    seq->f->have = 0;
    seq->f->f = fin;
    seq->f->cs = cs;
    
    return seq;
}

void alder_rseq_destroy (rseq_t * ks)
{
    fclose(ks->f->f);
    free(ks->f);
    free(ks);
}

int alder_rseq_read (rseq_t * seq)
{
//    %% machine fastqBuild;
    
    FILE *fin = (FILE*)seq->f->f;
    char *buf = seq->f->buf;
    int have = seq->f->have;
    int cs = seq->f->cs;
    seq->f->stop = 0;
    int n1 = 0;
    int n2 = 0;
    
    while ( 1 ) {
        char *eof, *p, *pe, *data = buf + have;
        eof = NULL;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "BUFFER OUT OF SPACE\n");
            exit(1);
        }
        len = (int)fread( data, 1, space, fin );
        if ( len == 0 )
            break;
        /* Find the last newline by searching backwards. */
        p = buf;
        pe = data + len - 1;
        while ( *pe != '\n' && pe >= buf )
            pe--;
        pe += 1;
        if (feof(fin)) {
            eof = pe;
        }
        %% write exec;
        
        /* How much is still in the buffer? */
        have = (int)(data + len - pe);
        if ( have > 0 )
            memmove( buf, pe, have );
        
        if (seq->f->stop == 1) {
            seq->f->cs = cs;
            seq->f->have = have;
            break;
        }
        
        if ( len < space )
            break;
    }
    printf("n1/n2: %d/%d\n", n1, n2);

    return 0;
}




