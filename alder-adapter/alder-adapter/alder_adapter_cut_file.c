//
//  alder_adapter_cut.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>

#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "alder_logger.h"
#include "kseq.h"
#include "bstrlib.h"
#include "bstrmore.h"
#include "alder_adapter_index_illumina.h"
#include "alder_adapter_cut.h"
#include "alder_fastq_stat.h"
#include "alder_adapter_cut_file.h"
#include "alder_adapter_cut_core.h"

KSEQ_INIT(int, read)

extern char *adapter_sequence[19];

int alder_adapter_cut_file(const char *fnin, const char *fnout,
                           const char *adapter_seq, const double error_rate,
                           const alder_adapter_option_t *option,
                           alder_fastq_stat_t *stat)
{
    int fp = fnin == NULL ? STDIN_FILENO : open(fnin, O_RDONLY);
    if (fp < 0) {
        fprintf(stderr, "error: cannot open %s\n", fnin);
        return -1;
    }
    FILE *fpout = fnout == NULL ? stdout : fopen(fnout, "w");
    if (fpout == NULL) {
        fprintf(stderr, "error: cannot make a file %s\n", fnout);
        if (fnin!= NULL) close(fp);
        return -1;
    }
    
#include "alder_adapter_cut_core.c"
    
	kseq_destroy(seq);
    if (fnin != NULL) close(fp);
    if (fnout != NULL) fclose(fpout);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// OBSOLETE: ..._stdin
///////////////////////////////////////////////////////////////////////////////

// Two things: error_rate and min_overlap
// Defaults are error_rate (0.1), and min_overlap (3).
// If we have all of the following conditions, we trim the 3'-end.
// 1) best_matches > min_overlap.
// 2) best_j == n
// 3) we cut at start2 of the read sequence.
int alder_adapter_cut_file_stdin(const char *fnin, const char *fnout,
                                 char *adapter_seq, const double error_rate,
                                 const int log_flag,
                                 alder_fastq_stat_t *stat)
{
    assert(fnin == NULL);
    int fdin = STDIN_FILENO;
    
    logc_log(MAIN_LOGGER, LOG_FINEST, "  ..._stdin: 1");
    FILE *fpout = fnout == NULL ? stdout : fopen(fnout, "w");
    if (fpout == NULL) {
        fprintf(stderr, "error: cannot make a file %s\n", fnout);
        return 1;
    }
    logc_log(MAIN_LOGGER, LOG_FINEST, "  ..._stdin: 2");
    
	kseq_t *seq;
    
    ///////////////////////////////////////////////////////////////////////////
    // COMPLICATED!
    ///////////////////////////////////////////////////////////////////////////
    logc_log(MAIN_LOGGER, LOG_FINEST, "  ..._stdin: bgn: kseq_init");
	seq = kseq_init(fdin);
    // if adapter_seq is not given.
    // Use the first seq to figure this out.
    // seq->comment.s
    kseq_read(seq);
    logc_log(MAIN_LOGGER, LOG_FINEST, "  ..._stdin: bgn: adapter_seq");
    if (adapter_seq == NULL) {
        if (seq->comment.s == NULL) {
            kseq_destroy(seq);
            if (fnout != NULL) fclose(fpout);
            return -1;
        }
        int index = alder_adapter_index_illumina_seq_summary(seq->comment.s);
        if (index < 0) {
            kseq_destroy(seq);
            if (fnout != NULL) fclose(fpout);
            return -1;
        }
        adapter_seq = adapter_sequence[index];
    }
    logc_log(MAIN_LOGGER, LOG_FINEST, "  ..._stdin: %s", adapter_seq);
    ///////////////////////////////////////////////////////////////////////////
    // COMPLICATED!
    ///////////////////////////////////////////////////////////////////////////
    
    assert(adapter_seq != NULL);
    int m = (int)strlen(adapter_seq);
    const int flags = 14;
    const int degenerate = 0;
    int start1, best_i, start2, best_j, best_matches, best_cost;
    
    do {
        stat->n_total++;
//        ++n, slen += seq->seq.l, qlen += seq->qual.l;
        alder_adapter_cut(adapter_seq, m, seq->seq.s, (int)seq->seq.l,
                          error_rate, flags, degenerate,
                          &start1, &best_i, &start2, &best_j,
                          &best_matches, &best_cost);
        seq->seq.s[start2] = '\0';
        seq->qual.s[start2] = '\0';
        if (seq->comment.s == NULL) {
            fprintf(fpout, "@%s\n%s\n+\n%s\n",
                    seq->name.s, seq->seq.s, seq->qual.s);
        } else {
            fprintf(fpout, "@%s %s\n%s\n+\n%s\n",
                    seq->name.s, seq->comment.s, seq->seq.s, seq->qual.s);
        }
    } while (kseq_read(seq) >= 0);
        
    kseq_destroy(seq);
    
    if (fnin != NULL) close(fdin);
    if (fnout != NULL) fclose(fpout);

    return 0;
}



