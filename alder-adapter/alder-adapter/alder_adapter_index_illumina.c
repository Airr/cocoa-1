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
#include "alder_file_isgzip.h"
#include "alder_adapter_index_illumina.h"

//KSEQ_INIT(int, read)

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

