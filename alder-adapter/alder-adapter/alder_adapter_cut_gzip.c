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

KSEQ_INIT(gzFile, gzread)

extern char *adapter_sequence[19];

int alder_adapter_cut_gzip(const char *fnin, const char *fnout,
                           const char *adapter_seq, const double error_rate,
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
    
#include "alder_adapter_cut_core.c"
    
	kseq_destroy(seq);
    if (fnin != NULL) gzclose(fp);
    if (fnout != NULL) fclose(fpout);
    
    return 0;
}





