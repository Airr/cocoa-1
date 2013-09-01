//
//  alder_adapter_index_illumina.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/31/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "bstrlib.h"
#include "kseq.h"
#include "alder_file_isgzip.h"
#include "alder_adapter_index_illumina.h"
#include "alder_adapter_index_illumina_gzip.h"

KSEQ_INIT(int, read)

extern char *adapter_sequence[19];

int alder_adapter_index_illumina_seq_summary(const char *s)
{
    int truseqIndex = -1;
    
    bstring b = bfromcstr (s);
    struct bstrList * lines;
    lines = bsplit (b, ':');
    if (lines->qty == 4) {
        bstring bAdapterIndexLetter = bmidstr(lines->entry[3], 1, 5);
        if (bAdapterIndexLetter) {
            // 33 (6)
            bstring bAdapter = bfromcstr(adapter_sequence[0]);
            for (int i = 0; i < 12; i++) {
                bstring bAdapter2 = bfromcstr(adapter_sequence[i]);
//                bassignmidstr(bAdapter, bAdapter2, 33, 6);
                bassignmidstr(bAdapter, bAdapter2, 34, 5);
                if (!bstrcmp(bAdapter, bAdapterIndexLetter)) {
                    truseqIndex = i;
                    bdestroy(bAdapter2);
                    break;
                }
                bdestroy(bAdapter2);
            }
            bdestroy(bAdapter);
            bdestroy(bAdapterIndexLetter);
        } 
    } else {
        truseqIndex = -1;
    }
    
    bstrListDestroy (lines);
    bdestroy (b);

    
    return truseqIndex;
}

///////////////////////////////////////////////////////////////////////////////
// OBSOLETE
///////////////////////////////////////////////////////////////////////////////
int alder_adapter_index_illumina(const char *fnin)
{
    assert(fnin != NULL);
    
    int truseqIndex = -1;
    if (fnin != NULL) {
        int status = alder_file_isgzip(fnin);
        if (status == 1) {
            return alder_adapter_index_illumina_gzip(fnin);
        }
    }
    int fdin = fnin == NULL ? dup(STDIN_FILENO) : open(fnin, O_RDONLY);
    kseq_t *seq;
    seq = kseq_init(fdin);
    kseq_read(seq);
    if (seq->comment.s == NULL) {
        truseqIndex = -1;
    } else {
        truseqIndex = alder_adapter_index_illumina_seq_summary(seq->comment.s);
    }
    kseq_destroy(seq);
    
    if (fnin == NULL) {
        dup2(fdin, STDIN_FILENO);
    } else {
        close(fdin);
    }
    
    
    return truseqIndex;
}
