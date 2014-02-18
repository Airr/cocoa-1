/**
 * This file, main.c, is part of alder-simdiv.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-simdiv is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-simdiv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-simdiv.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_simdiv_option.h" 

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_simdiv_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_simdiv_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_simdiv_option_free(&option);
        exit(1);
    }
    
    my_cmdline_parser_free(&args_info);
    alder_simdiv_option_free(&option);
    return 0;
}

