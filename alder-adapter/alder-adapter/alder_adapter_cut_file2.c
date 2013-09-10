/**
 * This file, alder_adapter_cut_file2.c, is part of alder-adapter.
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
#include <zlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include "alder_logger.h"
#include "bstrlib.h"
#include "bstrmore.h"
#include "alder_adapter_index_illumina.h"
#include "alder_adapter_cut.h"
#include "alder_fastq_stat.h"
#include "alder_adapter_cut_filter.h"
#include "alder_file_isgzip.h"
#include "alder_fastq_kseq.h"

#include "alder_adapter_cut_file2.h"


extern char *adapter_sequence[19];

// fnin, fnout
// fnin2, fnout2
int alder_adapter_cut_file2(const char *fnin, const char *fnout,
                            const char *fnin2, const char *fnout2,
                            const char *adapter_seq, const char *adapter_seq2,
                            const alder_adapter_option_t *option,
                            alder_fastq_stat_t *stat)
{
    assert((fnin2 == NULL && fnout2 == NULL) ||
           (fnin2 != NULL && fnout2 != NULL));
    
    // The first file.
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
        return -1;
    }
    FILE *fpout = fnout == NULL ? stdout : fopen(fnout, "w");
    if (fpout == NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot create file %s.\n", fnout);
        if (fnin!= NULL) close(fp);
        return -1;
    }
    
    // The second file.
    int fp2 = -1;
    int isGzip2 = 0;
    gzFile fpgz2 = NULL;
    if (fnin2 != NULL) {
        isGzip2 = alder_file_isgzip(fnin2);
        if (isGzip2 == 1) {
            fpgz2 = gzopen(fnin2, "r");
        } else {
            fp2 = open(fnin2, O_RDONLY);
        }
    }
    if (fnin2 != NULL && fp2 < 0 && fpgz2 == Z_NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot open %s.\n", fnin2);
        if (fnin != NULL) close(fp);
        fclose(fpout);
        return -1;
    }
    FILE *fpout2 = fnout2 == NULL ? stdout : fopen(fnout2, "w");
    if (fpout2 == NULL) {
        logc_logErrorBasic(ERROR_LOGGER, 0, "cannot create file %s.\n", fnout2);
        if (fnin2 != NULL) close(fp2);
        if (fnin != NULL) close(fp);
        fclose(fpout);
        return -1;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Two functions in the following two source files
    // alder_adapter_cut_file.c
    // alder_adapter_cut_gzip.c
    // are almost identical. Espeically, they share the following core part.
    ///////////////////////////////////////////////////////////////////////////////
    
    // First adapter
    int isAdapterFound = 0;
    kseq_t *seq;
    if (isGzip == 0) {
        seq = alder_kseq_init((void *)(intptr_t)fp, isGzip);
    } else {
        seq = alder_kseq_init(fpgz, isGzip);
    }
    alder_kseq_read(seq);
    
    if (fnin == NULL) {
        if (adapter_seq == NULL && seq->comment.s != NULL) {
            int index = alder_adapter_index_illumina_seq_summary(seq->comment.s);
            stat->adapter_seq_id = index;
            if (index >= 0) {
                isAdapterFound = 1;
                adapter_seq = adapter_sequence[index];
            }
        } else if (adapter_seq != NULL) {
            isAdapterFound = 1;
        }
        assert(adapter_seq != NULL);
        if (adapter_seq == NULL) {
            logc_logErrorBasic(ERROR_LOGGER, 0, "no adapter sequence is found.");
            if (fnout2 != NULL) fclose(fpout2);
            if (fnin2 != NULL) {
                if (isGzip2 == 0) {
                    close(fp2);
                } else {
                    gzclose(fpgz2);
                }
            }
            if (fnout != NULL) fclose(fpout);
            if (fnin != NULL) {
                if (isGzip == 0) {
                    close(fp);
                } else {
                    gzclose(fpgz);
                }
            }
            return -1;
        }
    } else {
        isAdapterFound = 1;
        assert(adapter_seq != NULL);
    }
    
    // First adapter 2
//    int isAdapterFound2 = 1;
    kseq_t *seq2 = NULL;
    if (fnin2 != NULL) {
        if (isGzip2 == 0) {
            seq2 = alder_kseq_init((void *)(intptr_t)fp2, isGzip2);
        } else {
            seq2 = alder_kseq_init(fpgz2, isGzip2);
        }
        alder_kseq_read(seq2);
    }
    
    assert(isAdapterFound == 1);
    
    // Start processing.
//    stat->adapter_seq = bfromcstr(adapter_seq); // FIXME? only one adapter is added to stat.
    int m = (int)strlen(adapter_seq);
    int m2;
    if (seq2 != NULL) m2 = (int)strlen(adapter_seq2);
    do {
        int status;
        int start21, start22;
        
        bstring bSequence = bfromcstr(seq->seq.s);
        bstring bQuality = bfromcstr(seq->qual.s);
        int isFiltered = 0;
        status = alder_adapter_cut_filter(adapter_seq, m, bSequence, bQuality,
                                          &isFiltered, &start21, option);
        if (status != 0) {
            logc_logBasic(ERROR_LOGGER,
                          "Fatal: negative start2 or large start2\n"
                          "File: %s\n"
                          "%s\n%s\n+\n%s\n",
                          fnin, seq->name.s, seq->seq.s, seq->qual.s);
            abort();
        }
        bstring bSequence2;
        bstring bQuality2;
        int isFiltered2 = 0;
        if (seq2 != NULL) {
            bSequence2 = bfromcstr(seq2->seq.s);
            bQuality2 = bfromcstr(seq2->qual.s);
            status = alder_adapter_cut_filter(adapter_seq2, m2, bSequence2, bQuality2,
                                              &isFiltered2, &start22, option);
            if (status != 0) {
                logc_logBasic(ERROR_LOGGER,
                              "Fatal: negative start2 or large start2\n"
                              "File: %s\n"
                              "%s\n%s\n+\n%s\n",
                              fnin2, seq2->name.s, seq2->seq.s, seq2->qual.s);
                abort();
            }
        }
        
        // NEED 2
        stat->n_total++;
        stat->n_base += seq->seq.l;
        if (seq2 != NULL) stat->n_base2 += seq2->seq.l;
        if (option->keep == 0) {
            if (isFiltered == 0 && isFiltered2 == 0) {
                if (seq->comment.s == NULL) {
                    fprintf(fpout, "@%s\n%s\n+\n%s\n",
                            seq->name.s, bdata(bSequence), bdata(bQuality));
                    if (seq2 != NULL) {
                        fprintf(fpout2, "@%s\n%s\n+\n%s\n",
                                seq2->name.s, bdata(bSequence2), bdata(bQuality2));
                    }
                } else {
                    fprintf(fpout, "@%s %s\n%s\n+\n%s\n",
                            seq->name.s, seq->comment.s, bdata(bSequence), bdata(bQuality));
                    if (seq2 != NULL) {
                        fprintf(fpout2, "@%s %s\n%s\n+\n%s\n",
                                seq2->name.s, seq2->comment.s, bdata(bSequence2), bdata(bQuality2));
                    }
                }
                stat->n_base_remained += start21;
                if (start21 < seq->seq.l) stat->n_trimmed++;
                if (seq2 != NULL) {
                    stat->n_base_remained2 += start22;
                    if (start22 < seq2->seq.l) stat->n_trimmed2++;
                }
            } else {
                stat->n_trimmed++;
                if (seq2 != NULL) stat->n_trimmed2++;
            }
        } else {
            if (isFiltered == 1 && isFiltered2 == 1) {
                if (seq->comment.s == NULL) {
                    fprintf(fpout, "@%s\n%s\n+\n%s\n",
                            seq->name.s,
                            bdata(bSequence),
                            bdata(bQuality));
                    if (seq2 != NULL) {
                        fprintf(fpout2, "@%s\n%s\n+\n%s\n",
                                seq2->name.s,
                                bdata(bSequence2),
                                bdata(bQuality2));
                    }
                } else {
                    fprintf(fpout, "@%s %s\n%s\n+\n%s\n",
                            seq->name.s, seq->comment.s,
                            bdata(bSequence),
                            bdata(bQuality));
                    if (seq2 != NULL) {
                        fprintf(fpout2, "@%s %s\n%s\n+\n%s\n",
                                seq2->name.s, seq2->comment.s,
                                bdata(bSequence2),
                                bdata(bQuality2));
                    }
                }
            }
        }
        // NEED 2
        bdestroy(bQuality);  bdestroy(bSequence);
        if (seq2 != NULL) {
            bdestroy(bQuality2); bdestroy(bSequence2);
        }
        
        if (seq2 != NULL) alder_kseq_read(seq2);
    } while (alder_kseq_read(seq) >= 0);
    
    
    alder_kseq_destroy(seq);
    if (seq2 != NULL) {
        alder_kseq_destroy(seq2);
    }
    if (fnout2 != NULL) fclose(fpout2);
    if (fnin2 != NULL) {
        if (isGzip2 == 0) {
            close(fp2);
        } else {
            gzclose(fpgz2);
        }
    }
    if (fnout != NULL) fclose(fpout);
    if (fnin != NULL) {
        if (isGzip == 0) {
            close(fp);
        } else {
            gzclose(fpgz);
        }
    }
    
    return 0;
}
