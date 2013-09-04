/**
 * This file, alder_adapter_pair.c, is part of alder-adapter.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-adapter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-adapter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-adapter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <fcntl.h>
#include <zlib.h>
#include <assert.h>
#include "bstrmore.h"
#include "alder_logger.h"
#include "alder_file_isgzip.h"
#include "alder_kseq_base.h"
#include "alder_adapter_pair.h"
#include "alder_adapter_index_illumina.h"
#include "alder_adapter_index_illumina_gzip.h"

// We are given a number of FASTQ files.
// We find pairs, if any, of them. By seeing the first read sequences.
// We may need to check adapter sequences as well.

extern char *adapter_sequence[19];

int alder_adapter_nopair(alder_adapter_option_t *o)
{
    struct bstrList *sl = o->infile;
    size_t pairIndex = 0;
    for (int i = 0; i < sl->qty; i++) {
        o->pair[pairIndex++] = i;
        o->pair[pairIndex++] = -1;
    }
    o->pair[pairIndex++] = -1;
    o->pair[pairIndex++] = -1;
    return 0;
}

int alder_adapter_set_adapter(alder_adapter_option_t *o)
{
    // Check the pairs.
    struct bstrList *sl = o->infile;
    
    // Check the adapter sequences.
    // We get the adapter sequences from the command line.
    // o->adapter->qty == 0 -> no adapter from command line option.
    // o->adapter->qty >  0 -> adapter from command line option.
    // We check the adapters only if o->adapter->qty == 0
    if (o->adapter->qty == 0) {
        for (int i = 0; i < sl->qty; i++) {
            bstring b = sl->entry[i];
            bstring bAdapter = alder_adapter_adapter(bdata(b), NULL);
            o->adapter->entry[i] = bAdapter;
        }
    }
    
    return 0;
}

// We may need a better data structure.
// A set structure for testing existence.
// o->paired must be 2 * (number of input files).
// positive-positive for pairs.
// positive-negative for singles.
int alder_adapter_pair(alder_adapter_option_t *o)
{
    // Check the pairs.
    struct bstrList *sl = o->infile;
    size_t pairIndex = 0;
    for (int i = 0; i < sl->qty; i++) {
        // FIXME: allocate pair?
        int isPaired = 0;
        for (int j = 0; j < pairIndex; j++) {
            if (o->pair[j] == i) {
                isPaired = 1;
                break;
            }
        }
        if (isPaired == 1) {
            continue;
        }
        
        bstring b = sl->entry[i];
        bstring readname = alder_adapter_read_name(bdata(b));
        isPaired = 0;
        for (int j = 0; j < sl->qty; j++) {
            if (i == j) continue;
            bstring b2 = sl->entry[j];
            bstring readname2 = alder_adapter_read_name(bdata(b2));
            if (!bstrcmp(readname, readname2)) {
                o->pair[pairIndex++] = i;
                o->pair[pairIndex++] = j;
                isPaired = 1;
                bdestroy(readname2);
                break;
            }
            bdestroy(readname2);
        }
        if (isPaired == 0) {
            o->pair[pairIndex++] = i;
            o->pair[pairIndex++] = -1;
        }
        bdestroy(readname);
    }
    o->pair[pairIndex++] = -1;
    o->pair[pairIndex++] = -1;
    assert(pairIndex <= (sl->qty + 1)* 2);
    
    return 0;
}

bstring alder_adapter_read_name(const char *fnin)
{
    int fp = -1;
    int isGzip = 0;
    gzFile fpgz;
    if (fnin == NULL) {
        fp = STDIN_FILENO;
    } else {
        isGzip = alder_file_isgzip(fnin);
        if (isGzip == 1) {
            fpgz = gzopen(fnin, "r");
        } else {
            fp = open(fnin, O_RDONLY);
        }
    }
    if (fp < 0 && fpgz == Z_NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", fnin);
        return NULL;
    }

    kseq_t *seq;
    if (isGzip == 0) {
        seq = kseq_init((void *)(intptr_t)fp, isGzip);
    } else {
        seq = kseq_init(fpgz, isGzip);
    }
    kseq_read(seq);
    bstring b = bfromcstr(seq->name.s);
    kseq_destroy(seq);
    
    if (fnin != NULL) {
        if (isGzip == 0) {
            close(fp);
        } else {
            gzclose(fpgz);
        }
    }
    return b;
}

bstring alder_adapter_adapter(const char *fnin, char *adapter_seq)
{
    int fp = -1;
    int isGzip = 0;
    gzFile fpgz;
    if (fnin == NULL) {
        fp = STDIN_FILENO;
    } else {
        isGzip = alder_file_isgzip(fnin);
        if (isGzip == 1) {
            fpgz = gzopen(fnin, "r");
        } else {
            fp = open(fnin, O_RDONLY);
        }
    }
    if (fp < 0 && fpgz == Z_NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", fnin);
        return NULL;
    }

    int isAdapterFound = 0;
    kseq_t *seq;
    if (isGzip == 0) {
        seq = kseq_init((void *)(intptr_t)fp, isGzip);
    } else {
        seq = kseq_init(fpgz, isGzip);
    }
    kseq_read(seq);
    bstring b;
    if (adapter_seq == NULL && seq->comment.s != NULL) {
//    if (seq->comment.s != NULL) {
        int index = alder_adapter_index_illumina_seq_summary(seq->comment.s);
        if (index >= 0) {
            isAdapterFound = 1;
            adapter_seq = adapter_sequence[index];
            
        }
    } else if (adapter_seq != NULL) {
        isAdapterFound = 1;
    }
    kseq_destroy(seq);
    
    assert(adapter_seq != NULL);
    b = bfromcstr(adapter_seq);
    if (adapter_seq == NULL) {
        logc_logErrorBasic(ERROR_LOGGER, 0, "no adapter sequence is found.");
    }
    
    
    if (fnin != NULL) {
        if (isGzip == 0) {
            close(fp);
        } else {
            gzclose(fpgz);
        }
    }
    return b;
}
