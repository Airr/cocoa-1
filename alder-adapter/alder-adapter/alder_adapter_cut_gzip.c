//
//  alder_adapter_cut_gzip.c
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
#include "alder_logger.h"
#include "kseq.h"
#include "bstrlib.h"
#include "bstrmore.h"
#include "alder_adapter_index_illumina.h"
#include "alder_adapter_cut.h"
#include "alder_fastq_stat.h"
#include "alder_adapter_cut_file.h"
#include "alder_adapter_cut_core.h"
#include "alder_adapter_cut_filter.h"

KSEQ_INIT(gzFile, gzread)

extern char *adapter_sequence[19];

int alder_adapter_cut_gzip(const char *fnin, const char *fnout,
                           const char *fnin2, const char *fnout2,
                           const char *adapter_seq, const char *adapter_seq2,
                           const alder_adapter_option_t *option,
                           alder_fastq_stat_t *stat)
{
    gzFile fp;
    int fdin = open(fnin, O_RDONLY);
    if (fdin < 0) {
        fprintf(stderr, "error: cannot open %s\n", fnin);
        return -1;
    }
    fp = gzdopen(fdin, "r");
    if (fp == Z_NULL) {
        fprintf(stderr, "error: cannot gzopen %s\n", fnin);
        close(fdin);
        return -1;
    }
    FILE *fpout = fnout == NULL ? stdout : fopen(fnout, "w");
    if (fpout == NULL) {
        fprintf(stderr, "error: cannot make a file %s\n", fnout);
        gzclose(fp);
        return -1;
    }
    // The second FASTQ file.
    gzFile fp2;
    int fdin2 = open(fnin2, O_RDONLY);
    if (fdin2 < 0) {
        fprintf(stderr, "error: cannot open %s\n", fnin2);
        gzclose(fp);
        return -1;
    }
    fp2 = gzdopen(fdin2, "r");
    if (fp2 == Z_NULL) {
        fprintf(stderr, "error: cannot gzopen %s\n", fnin2);
        fclose(fpout);
        gzclose(fp);
        close(fdin2);
        return -1;
    }
    FILE *fpout2 = fnout2 == NULL ? stdout : fopen(fnout2, "w");
    if (fpout2 == NULL) {
        fprintf(stderr, "error: cannot make a file %s\n", fnout2);
        fclose(fpout);
        gzclose(fp);
        gzclose(fp2);
        return -1;
    }
    
#include "alder_adapter_cut_core.c"
    
    if (fnin != NULL) gzclose(fp);
    if (fnout != NULL) fclose(fpout);
    
    return 0;
}





