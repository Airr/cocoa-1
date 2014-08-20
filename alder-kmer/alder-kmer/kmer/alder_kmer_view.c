/**
 * This file, alder_kmer_view.c, is part of alder-kmer.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-kmer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kmer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_file_open.h"
#include "alder_encode_number.h"
#include "alder_kmer_uncompress.h"
#include "alder_kmer_decode.h"
#include "alder_kmer_view.h"

#pragma mark kmc

/* A copy from cmdline.h: update it as you change the --format option. */
enum enum_format {
    format__NULL = -1,
    format_arg_fasta = 0,
    format_arg_fastq,
    format_arg_fa,
    format_arg_fq,
    format_arg_seq,
    format_arg_kmc,
    format_arg_tbl,
    format_arg_tbh,
    format_arg_par,
    format_arg_bin
};

/**
 *  This function reads kmc binary files and prints them out in human readable
 *  text.
 *
 *  @param infile        infile
 *  @param outfile_given outfile given or not
 *  @param outdir        outdir
 *  @param outfile       outfile name
 *
 *  @return success or fail
 */
static int
alder_kmer_view_kmc(struct bstrList *infile,
                    unsigned int outfile_given,
                    const char *outdir,
                    const char *outfile,
                    int K)
{
    ///////////////////////////////////////////////////////////////////////////
    // Template: multiple infile and single otufile.
    ///////////////////////////////////////////////////////////////////////////
    FILE *fpout;
    fpout = alder_file_open_with_outdirfilegiven(outfile_given,
                                                 outdir,
                                                 outfile);
    if (fpout == NULL) {
        alder_loge(ALDER_ERR_FILE, "failed to open file %s/%s",
                   outdir, outfile);
        return ALDER_STATUS_ERROR;
    }
    
    int i_start = 0;
    if (infile->qty == 0) {
        i_start = -1;
    }
    for (int i_infile = i_start; i_infile < infile->qty; i_infile++) {
        FILE *fpin = NULL;
        if (i_infile == -1) {
            fpin = stdin;
        } else {
            fpin = fopen(bdata(infile->entry[i_infile]), "rb");
            if (fpin == NULL) {
                alder_loge(ALDER_ERR_FILE, "failed to open %s",
                           bdata(infile->entry[i_infile]));
                if (outfile_given) {
                    XFCLOSE(fpout);
                }
                return ALDER_STATUS_ERROR;
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // BEGIN
        ///////////////////////////////////////////////////////////////////////
        alder_encode_number_t *kmer = alder_encode_number_create_for_kmer(K);
        while (1) {
            size_t n1 = fread(kmer->n, sizeof(*kmer->n), kmer->s, fpin);
            uint32_t kmer_count = 0;
            size_t n2 = fread(&kmer_count, sizeof(kmer_count), 1, fpin);
            if (n1 != kmer->s && n2 != 1) {
                if (feof(fpin)) {
                    break;
                }
                alder_loge(ALDER_ERR, "failed to read a kmer from file %s",
                           bdata(infile->entry[i_infile]));
                XFCLOSE(fpin);
                alder_encode_number_destroy(kmer);
                return ALDER_STATUS_ERROR;
            }
            alder_encode_number_print_in_DNA(fpout, kmer);
            fprintf(fpout, "\t%u\n", kmer_count);
        }
        alder_encode_number_destroy(kmer);
        ///////////////////////////////////////////////////////////////////////
        // END
        ///////////////////////////////////////////////////////////////////////
        
        if (i_infile >= 0) {
            XFCLOSE(fpin);
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Template: multiple infile and single otufile.
    ///////////////////////////////////////////////////////////////////////////
    if (outfile_given) {
        XFCLOSE(fpout);
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function reads binary files and prints them out in human readable 
 *  text.
 *
 *  @param format        format
 *  @param infile        infile
 *  @param outfile_given outfile given or not
 *  @param outdir        outdir
 *  @param outfile       outfile name
 *
 *  @return success or fail
 */
int
alder_kmer_view(int format,
                struct bstrList *infile,
                unsigned int outfile_given,
                const char *outdir,
                const char *outfile,
                int K)
{
    int s = ALDER_STATUS_SUCCESS;
    if (format == format_arg_kmc) {
        s = alder_kmer_view_kmc(infile, outfile_given, outdir, outfile, K);
    } else if (format == format_arg_bin) {
        alder_kmer_uncompress(0,
                              infile,
                              outfile_given,
                              outdir,
                              outfile);
    } else if (format == format_arg_par) {
        s = alder_kmer_decode(K, 0,
                              infile,
                              outfile_given,
                              outdir,
                              outfile);
    }
    
    return s;
}



