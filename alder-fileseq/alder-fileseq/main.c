/**
 * This file, main.c, is part of alder-fileseq.
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
#include "cmdline.h"
#include "alder_fileseq.h"
#include "alder_fileseq_option.h" 

int main(int argc, char * argv[])
{
    int s = 0;
    int isExitWithHelp = 0;
    alder_fileseq_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_fileseq_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_fileseq_option_free(&option);
        exit(1);
    }
    
    if (args_info.stat_flag) {
        for (int i = 0; i < option.infile->qty; i++) {
            alder_fileseq_stat_t buf;
            s = alder_fileseq_stat(bdata(option.infile->entry[i]), &buf,
                                   (int) args_info.kmer_arg);
            if (s == 0) {
                fprintf(stdout, "Number of kmers: %llu\n", buf.v);
            }
        }
    }
    
    if (args_info.encode_flag) {
        // --encode -o directory 1.fa
//        int alder_fileseq_kmer_encode(int n_thread,
//                                      int i_iteration,
//                                      int kmer_size,
//                                      long disk_space,
//                                      long memory_available,
//                                      uint64_t n_iteration,
//                                      uint64_t n_partition,
//                                      struct bstrList *infile,
//                                      bstring dout)
        alder_fileseq_kmer_encode(2, 0, 5,
                                  10, 1,
                                  2, 3,
                                  option.infile,
                                  option.outfile->entry[0]);
    }
    
    
    if (args_info.token_flag) {
        for (int i = 0; i < option.infile->qty; i++) {
            fprintf(stdout, "File: %s\n", bdata(option.infile->entry[i]));
            alder_fileseq_token_t *t = alder_fileseq_token_create(bdata(option.infile->entry[i]));
            while (alder_fileseq_token_moreSequence(t)) {
                while (alder_fileseq_token_hasNext(t)) {
                    char c = (char)alder_fileseq_token_next(t);
                    fprintf(stdout, "%c", c);
                }
                fprintf(stdout, "\n");
            }
            alder_fileseq_token_destroy(t);
        }
    }
    
    my_cmdline_parser_free(&args_info);
    alder_fileseq_option_free(&option);
    return 0;
}

