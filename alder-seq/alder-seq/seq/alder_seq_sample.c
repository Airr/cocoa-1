/**
 * This file, alder_seq_sample.c, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_seq_sample.h"

//static int
//alder_tokenset_token_of(char c)
//{
//    switch (c) {
//        case 'A':
//        case 'a':
//            return 0;
//            break;
//        case 'C':
//        case 'c':
//            return 1;
//            break;
//        case 'G':
//        case 'g':
//            return 2;
//            break;
//        case 'T':
//        case 't':
//            return 3;
//            break;
//        default:
//            return -1;
//            break;
//    }
//}

static char
alder_tokenset_char_of(int token)
{
    switch (token) {
        case 0:
            return 'A';
            break;
        case 1:
            return 'C';
            break;
        case 2:
            return 'G';
            break;
        case 3:
            return 'T';
            break;
        default:
            return 'X';
            break;
    }
}



bstring alder_seq_sample_sequence(size_t n)
{
    bstring s = bfromcstralloc((int)n, "");
    for (size_t i = 0; i < n; i++) {
        int x = rand();
        bconchar(s, alder_tokenset_char_of(x % 4));
    }
    return s;
}


int alder_seq_sample(const char *fn, size_t n_seq, size_t l_seq)
{
    int status = 0;
    FILE *fp = fopen(fn, "w");
    for (size_t i = 0; i < n_seq; i++) {
        bstring bseq = alder_seq_sample_sequence(l_seq);
        fprintf(fp, "%s\n", bdata(bseq));
        bdestroy(bseq);
    }
    XFREE(fp);
    return status;
}

struct bstrList* alder_seq_sample_bstrList(size_t n_seq, size_t l_seq)
{
    struct bstrList *bseqs = bstrVectorCreate((int)n_seq);
    for (size_t i = 0; i < n_seq; i++) {
        bstring bseq = alder_seq_sample_sequence(l_seq);
        bstrVectorAddBstring(bseqs, bseq);
    }
    return bseqs;
}
