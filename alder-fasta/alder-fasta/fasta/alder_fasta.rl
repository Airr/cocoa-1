/**
 * This file, alder_fasta.c, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "bstrmore.h"
#include "alder_fasta.h"
//#include <gsl/gsl_errno.h>

#define BUFSIZE 10000
enum { GFFELEMENTSIZE = 100 };
static uint64_t sSizeCapacity = 100;

%%{
    machine fasta;
    action e_title {
        pf->index[n1++] = n2;
    }
    action t_sequence {
        pf->data[n2++] = (char)fc;
    }
    
    title = print+ >e_title;
    sequence = ([ACGTURYSWKMBDHVNacgturyswkmbdhvn]+ $t_sequence)+ cntrl*;
    main := ('>' title cntrl+ sequence+)+;
    write data;
}%%

%%{
    machine fastaSize;
    action e_title {
        (*n1)++;
    }
    action t_title {
    }
    action t_sequence {
        (*n2)++;
    }
    
    title = print+ >e_title $t_title;
    sequence = ([ACGTURYSWKMBDHVNacgturyswkmbdhvn]+ $t_sequence)+ cntrl*;
    main := ('>' title cntrl+ sequence+)+;
    write data;
}%%

%%{
    machine fastaSeqSize;
    action e_title {
        bstrVectorAdd(name, "");
        bseqname = name->entry[t1];
//        printf("e[%c]", fc);
        (n1)++;
        t1++;
        t2 = 0;
    }
    action t_title {
        bconchar(bseqname, fc);
//        printf("t[%c]", fc);
    }
    action t_sequence {
        (n2)++;
        t2++;
    }
    action f_sequence {
        length[t1-1] = t2;
//        printf("f[%c]", fc);
    }
    
    title = print+ >e_title $t_title;
    sequence = ([ACGTURYSWKMBDHVNacgturyswkmbdhvn]+ $t_sequence)+ cntrl*;
    main := ('>' title cntrl+ sequence+ %f_sequence)+;
    write data;
}%%

%%{
    machine fastaBuild;
    action e_title {
        bstrVectorAdd(af->name, "");
        bseqname = af->name->entry[t1];
//        printf("e[%c]", fc);
        (n1)++;
        t1++;
    }
    action t_title {
        bconchar(bseqname, fc);
//        printf("t[%c]", fc);
    }
    action t_sequence {
//        printf("%c", fc);
        af->data[t2] = fc;
        (n2)++;
        t2++;
    }
    action f_sequence {
        af->index[t1-1] = t2;
//        printf("f[%c]", fc);
    }
    
    title = print+ >e_title $t_title;
    sequence = ([ACGTURYSWKMBDHVNacgturyswkmbdhvn]+ $t_sequence)+ cntrl*;
    main := ('>' title cntrl+ sequence+ %f_sequence)+;
    write data;
}%%

int alder_fasta_length(const char* fn, int64_t *n1, int64_t *n2)
{
    %% machine fastaSize;
    
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    *n1 = 0;
    *n2 = 0;
    
    FILE *fin = fopen(fn, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "Error: could not open %s\n", fn);
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
        len = (int)fread( data, 1, space, fin );
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
        have = (int)(data + len - pe);
        if ( have > 0 )
            memmove( buf, pe, have );
        if ( len < space )
            break;
    }
    
    
    return 0;
}

int alder_fasta_read(const char* fn, alder_fasta_t* pf)
{
    %% machine fasta;
    
    size_t n1 = 0;
    size_t n2 = 0;
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    
    
    FILE *fin = fopen(fn, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "cannot open %s\n", fn);
        return 1;
    }
    %% write init;
    while ( 1 ) {
        char *p, *pe, *data = buf + have;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "cannot read %s\n", fn);
            return 1;
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
        %% write exec;
        /* How much is still in the buffer? */
        have = (int)(data + len - pe);
        if ( have > 0 )
            memmove( buf, pe, have );
        if ( len < space )
            break;
    }
    
    return 0;
}

alder_fasta_t *alder_fasta_alloc(const char* fn)
{
    int status;
    int64_t ui1 = 0;
    int64_t ui2 = 0;
    alder_fasta_t *v = malloc(sizeof(alder_fasta_t));
    if (v == NULL) return NULL;
    alder_fasta_length(fn, &ui1, &ui2);
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
    status = alder_fasta_read(fn, v);
    if (status != 0) {
        free(v->data);
        free(v->index);
        free(v);
        return NULL;
    }
    return v;
}

void alder_fasta_free(alder_fasta_t* pf)
{
    if (pf->name != NULL) {
        bstrVectorDelete(pf->name);
    }
    free(pf->index);
    free(pf->data);
    free(pf);
}

int alder_fasta_seqname_length(const struct bstrList* fn,
                               struct bstrList *name,
                               int64_t *length)
{
    %% machine fastaSeqSize;
    
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    bstring bseqname;
    
    int64_t t1 = 0;
    int64_t t2 = 0;
    for (size_t i = 0; i < fn->qty; i++) {
        int64_t n1 = 0;
        int64_t n2 = 0;
        FILE *fin = fopen(bdata(fn->entry[i]), "r");
        if (fin == NULL)
        {
            fprintf(stderr, "cannot open %s\n", bdata(fn->entry[i]));
            return 1;
        }
        %% write init;
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
            if ( len < space )
                break;
        }
        fclose(fin);
        printf("n1 : n2 - %lld - %lld\n", n1, n2);
    }
    
    return 0;
}

alder_fasta_t *alder_fasta_list_alloc(const struct bstrList *fn,
                                      const int64_t tSeq,
                                      const int64_t tBase,
                                      const int64_t plusK)
{
    assert(tSeq > 0);
    assert(tBase > 0);
    %% machine fastaBuild;
    
    alder_fasta_t *af = malloc(sizeof(alder_fasta_t));
    af->name = bstrVectorCreate((int)tSeq);
    af->index = malloc((size_t)tSeq * sizeof(int64_t));
    af->data = malloc((tBase+1+plusK)*sizeof(char));
    af->data[tBase] = '\0';
    af->numberOfSequence = tSeq;
    af->numberOfBase = tBase;
    af->sizeCapacity = tBase + 1 + plusK;
    af->sizeOfDataWithDollar = -1;
    
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    bstring bseqname;
    
    int64_t t1 = 0;
    int64_t t2 = 0;
    for (size_t i = 0; i < fn->qty; i++) {
        int64_t n1 = 0;
        int64_t n2 = 0;
        FILE *fin = fopen(bdata(fn->entry[i]), "r");
        if (fin == NULL)
        {
            fprintf(stderr, "cannot open %s\n", bdata(fn->entry[i]));
            alder_fasta_list_free(af);
            return NULL;
        }
        %% write init;
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
            if ( len < space )
                break;
        }
        fclose(fin);
    }
    
    return af;
    
}

void alder_fasta_list_free(alder_fasta_t *pf)
{
    if (pf->name != NULL) bstrVectorDelete(pf->name);
    if (pf->index != NULL) free(pf->index);
    if (pf->data != NULL) free(pf->data);
    if (pf != NULL) free(pf);
}

int alder_fasta_list_length(const struct bstrList *fs, int64_t *n1, int64_t *n2)
{
    int64_t tSeq = 0;
    int64_t tBase = 0;
    for (size_t i = 0; i < fs->qty; i++) {
        int64_t nSeq = 0;
        int64_t nBase = 0;
        alder_fasta_length(bdata(fs->entry[i]), &nSeq, &nBase);
        tSeq += nSeq;
        tBase += nBase;
    }
    *n1 = tSeq;
    *n2 = tBase;
    return 0;
}
//int alder_fasta_list_read(const struct bstrList *fn, alder_fasta_t* pf);


/**
 * This function returns a (not owned) bstring pointer using a position in 
 * the whole sequence of a suffix array.
 * Argument:
 * af - alder_fasta_t
 * position - position in a suffix array
 * Return:
 * a bstring pointer if successful
 * NULL otherwise.
 */
char * alder_fasta_name_by_saindex(const alder_fasta_t *af, int64_t position)
{
    int64_t i = 0;
    for (i = 0; i < af->numberOfSequence; i++) {
        if (position < af->index[i]) {
            break;
        }
    }
    return bstr2cstr(af->name->entry[i], '\0');
}

/**
 * This function returns the index of reference names using a position in
 * the whole sequence of a suffix array.
 * Argument:
 * af - alder_fasta_t
 * position - position in a suffix array
 * Return:
 * the index of the reference name if successful
 * -1 otherwise.
 */
int64_t alder_fasta_index_by_saindex(const alder_fasta_t *af, int64_t position)
{
    int64_t i = 0;
    for (i = 0; i < af->numberOfSequence; i++) {
        if (position < af->index[i]) {
            break;
        }
    }
    return i;
}

/**
 * This function returns the position in the reference name using a position
 * in the whole sequence of a suffix array.
 * Argument:
 * af - alder_fasta_t
 * position - position in a suffix array
 * Return:
 * the position in the reference name if successful
 * -1 otherwise.
 */
int64_t alder_fasta_position_by_saindex(const alder_fasta_t *af, int64_t position)
{
    int64_t i = 0;
    for (i = 0; i < af->numberOfSequence; i++) {
        if (position < af->index[i]) {
            break;
        }
    }
    if (i > 0) {
        position -= af->index[i-1];
    }
    assert(position >= 0);
    return position;
}




