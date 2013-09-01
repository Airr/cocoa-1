//
//  alder_adapter_index_illumina_gzip.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/31/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <zlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "kseq.h"
#include "bstrlib.h"
#include "alder_adapter_index_illumina.h"
#include "alder_adapter_index_illumina_gzip.h"

KSEQ_INIT(gzFile, gzread)

extern char *adapter_sequence[19];

int alder_adapter_index_illumina_gzip(const char *fnin)
{
    assert(fnin != NULL);
    
    int truseqIndex = -1;
    gzFile fp;
    int fdin = open(fnin, O_RDONLY);
    // Error???
    fp = gzdopen(fdin, "r");
    // Error???
    
    kseq_t *seq;
    seq = kseq_init(fp);
    kseq_read(seq);
    if (seq->comment.s == NULL) {
        truseqIndex = -1;
    } else {
        truseqIndex = alder_adapter_index_illumina_seq_summary(seq->comment.s);
/*
        bstring b = bfromcstr (seq->comment.s);
        struct bstrList * lines;
        lines = bsplit (b, ':');
        if (lines->qty == 4) {
            bstring bAdapterIndexLetter = bfromcstr(bdatae (lines->entry[3], "NULL"));
            if (bAdapterIndexLetter) {
                // 33 (6)
                bstring bAdapter = bfromcstr(adapter_sequence[0]);
                for (int i = 0; i < 12; i++) {
                    bstring bAdapter2 = bfromcstr(adapter_sequence[i]);
                    bassignmidstr(bAdapter, bAdapter2, 33, 6);
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
*/
    }
    kseq_destroy(seq);
    
    gzclose(fp);
    
    return truseqIndex;
 
}
