/**
 * This file, main.c, is part of alder-encode.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-encode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-encode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-encode.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "bstrlib.h"
#include "alder_encode.h"
#include "alder_encode_kmer.h"
#include "alder_encode_option.h" 

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_encode_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_encode_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_encode_option_free(&option);
        exit(1);
    }
    
    if (args_info.number_flag) {
        alder_encode_number_test();
    }
    
    if (args_info.number2_flag) {
        alder_encode_number2_test();
    }
    
//    if (args_info.kmer_given) {
//        alder_encode_kmer_t *kmerEncoder =
//        alder_encode_kmer_create((int)args_info.kmer_arg,
//                                 strlen(args_info.seq_arg));
//        
//        alder_encode_kmer_in(kmerEncoder, args_info.seq_arg, strlen(args_info.seq_arg));
//        alder_encode_kmer_convert(kmerEncoder);
//        alder_encode_kmer_decode(kmerEncoder);
//        alder_encode_kmer_decode_as_number(kmerEncoder);
//        alder_encode_kmer_destroy(kmerEncoder);
//    }
    
    my_cmdline_parser_free(&args_info);
    alder_encode_option_free(&option);
    return 0;
}

