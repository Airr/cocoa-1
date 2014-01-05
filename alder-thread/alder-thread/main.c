/**
 * This file, main.c, is part of alder-thread.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-thread is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-thread is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-thread.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cmdline.h"
#include "alder_thread.h"
#include "alder_thread_option.h" 

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_thread_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_thread_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_thread_option_free(&option);
        exit(1);
    }
    
    if (args_info.worker_flag) {
        alder_thread_worker();
    }
    if (args_info.argument_flag) {
        alder_thread_argument();
    }
    if (args_info.join_flag) {
        alder_thread_join();
    }
    if (args_info.stack_flag) {
        alder_thread_stack();
    }
    if (args_info.readwriter_flag) {
        alder_thread_readwriter((int)args_info.nthread_arg);
    }
    if (args_info.casn_flag) {
        alder_thread_casn();
    }
    if (args_info.mcas_flag) {
//        GC_INIT();
        alder_thread_mcas();
    }
    if (args_info.sundell_flag) {
        //        GC_INIT();
        alder_thread_sundell();
    }
    
    my_cmdline_parser_free(&args_info);
    alder_thread_option_free(&option);
    
    /* Last thing that main() should do */
    pthread_exit(NULL);
    return 0;
}

