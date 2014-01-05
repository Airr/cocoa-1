/**
 * This file, main.c, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_cmacro.h"
#include "alder_fasta.h"
#include "alder_logger.h"
#include "alder_file.h"
#include "alder_fasta_token.h"
#include "alder_fasta_stat.h"
#include "alder_fasta_cat.h"
#include "alder_fasta_option.h"

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_fasta_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_fasta_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_fasta_option_free(&option);
        exit(1);
    }
    ///////////////////////////////////////////////////////////////////////////
    // Set up a log file if needed.
    alder_file_rm(args_info.log_arg);
    if (args_info.log_given) {
        alder_logger_initialize(args_info.log_arg, LOG_FINE);
        alder_logger_error_initialize(LOG_BASIC);
    } else {
        alder_logger_initialize(args_info.log_arg, LOG_SILENT);
        alder_logger_error_initialize(LOG_FINE);
    }

    if (args_info.stat_flag == 1) {
        alder_fasta_stat_single(option.infile, option.outfile);
    }
    if (args_info.seq_flag == 1) {
        alder_fasta_stat_seq(option.infile, option.outfile);
    }
    if (args_info.cat_flag == 1) {
        alder_fasta_cat(option.infile, option.outfile);
    }
    if (args_info.kmer_given) {
        for (int i = 0; i < option.infile->qty; i++) {
            uint64_t v = 0;
            if (args_info.use_ragel_flag) {
                alder_fasta_count_kmer(bdata(option.infile->entry[i]),
                                       (int)args_info.kmer_arg, &v);
            } else {
                size_t vz = 0;
                alder_fasta_kmer_count(bdata(option.infile->entry[i]),
                                       (int)args_info.kmer_arg, &vz);
                v = (uint64_t)vz;
            }
            fprintf(stdout, "File: %s\n", bdata(option.infile->entry[i]));
            fprintf(stdout, "Kmer: %llu\n", v);
        }
    }
    if (args_info.stream_flag) {
        int len_content;
        size_t size_buf = (size_t)args_info.bufsize_arg;
        char *buf = malloc(sizeof(*buf));
        FILE *fp = fopen(bdata(option.infile->entry[0]), "r");
        alder_fasta_streamer(&len_content, buf, size_buf, fp, 3, 100);
        while (len_content > 0) {
            
            for (int i = 0; i < len_content; i++) {
                if (buf[i] == '\n') {
                    fprintf(stdout, "#");
                } else {
                    fprintf(stdout, "%c", buf[i]);
                }
//                fputc(buf[i], stdout);
            }
            fprintf(stdout, "\n");
            alder_fasta_streamer(&len_content, buf, size_buf, fp, 3, 100);
        }
        XFCLOSE(fp);
    }
    if (args_info.token_flag) {
        for (int i = 0; i < option.infile->qty; i++) {
            fprintf(stdout, "File: %s\n", bdata(option.infile->entry[i]));
            alder_fasta_token_t *t = alder_fasta_token_create(bdata(option.infile->entry[i]));
            while (alder_fasta_token_moreSequence(t)) {
                while (alder_fasta_token_hasNext(t)) {
                    char c = (char)alder_fasta_token_next(t);
                    fprintf(stdout, "%c", c);
                }
                fprintf(stdout, "\n");
            }
            alder_fasta_token_destroy(t);
        }
    }
    if (args_info.sequenceiterator_flag) {
        alder_fasta_sequenceiterator_test(option.infile);
    }
    if (args_info.chunk_flag) {
        alder_fasta_chunk_test(args_info.inputs, args_info.inputs_num);
    }

    my_cmdline_parser_free(&args_info);
    alder_fasta_option_free(&option);
    return 0;
}

