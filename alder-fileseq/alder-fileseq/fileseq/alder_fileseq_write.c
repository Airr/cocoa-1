/**
 * This file, alder_fileseq_write.c, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "bstrmore.h"
#include "alder_file_format.h"
#include "alder_fasta_write.h"
#include "alder_fastq_write.h"
#include "alder_seq_write.h"
#include "alder_fileseq_write.h"


/* This function writes the sequences in a file.
 * returns 
 * 1 on success, 0 otherwise.
 */
int
alder_fileseq_write_bstrListToFilename(const char *fn,
                                       struct bstrList *sl,
                                       alder_format_type_t type,
                                       int n_file)
{
    int m_mod_n = sl->qty % n_file;
    int m_div_n = sl->qty / n_file;
    int n_first = m_div_n + 1;
    int n_second = m_div_n;
    
    bstring bseq_extension;
    if (type == ALDER_FORMAT_FASTA) {
        bseq_extension = bfromcstr("fa");
    } else if (type == ALDER_FORMAT_FASTQ) {
        bseq_extension = bfromcstr("fq");
    } else {
        bseq_extension = bfromcstr("seq");
    }
    
    int start_seq_n = 0;
    int end_seq_n = 0;
    struct bstrList *sublist = bstrViewCreate();
    for (int i = 0; i < n_file; i++) {
        start_seq_n = end_seq_n;
        if (i < m_mod_n) {
            // use n_first
            end_seq_n = start_seq_n + n_first;
        } else {
            // use n_second
            end_seq_n = start_seq_n + n_second;
        }
        sublist->qty = 0;
        for (int j = start_seq_n; j < end_seq_n; j++) {
            bstrViewAdd(sublist, sl->entry[j]);
        }
        
        bstring bfilename = bformat("%s-%02d.%s", fn, i, bdata(bseq_extension));
        switch (type) {
            case ALDER_FORMAT_FASTA:
                alder_fasta_write_bstrListToFilename(bdata(bfilename), sublist);
                break;
            case ALDER_FORMAT_FASTQ:
                alder_fastq_write_bstrListToFilename(bdata(bfilename), sublist);
                break;
            default:
                alder_seq_write_bstrListToFilename(bdata(bfilename), sublist);
                break;
        }
        bdestroy(bfilename);
    }
    bstrViewDestroy(sublist);
    return ALDER_STATUS_SUCCESS;
}

int alder_fileseq_append_bstrList(const char *fn,
                                  struct bstrList *sl,
                                  alder_format_type_t type)
{
    FILE *fp = fopen(fn, "a");
    if (fp == NULL) {
        return 1;
    }
    for (int i = 0; i < sl->qty; i++) {
        fprintf(fp, "%s\n", bdata(sl->entry[i]));
    }
    fclose(fp);
    return 0;
}
