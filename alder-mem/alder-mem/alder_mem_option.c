/**
 * This file, alder_mem_option.c, is part of alder-mem.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-mem is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-mem is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-mem.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include "cmdline.h"
#include "bstrmore.h"
#include "alder_file.h"
#include "alder_mem_cite.h"
#include "alder_mem_option.h"


int alder_mem_option_init(alder_mem_option_t *o, struct gengetopt_args_info *a)
{
    int status = 0;
    if (a->cite_given) {
        alder_mem_cite();
        status = 1;
    }
    o->sparse_suffix_array_k = a->sparse_suffix_array_k_arg;

    o->is_stdin = alder_file_isstdin();
    
    // logc_logWarning(ERROR_LOGGER, "no input file.");
    if (o->is_stdin == 0 && a->inputs_num == 0) {
//        status = 1;
    }
    // logc_logWarning(ERROR_LOGGER, "standard input and input files.");
    if (o->is_stdin == 1 && a->inputs_num > 0) {
        status = 1;
    }

    if (a->inputs_num > 0) {
        o->infile = bstrVectorCreate((int)a->inputs_num);
    } else {
        o->infile = NULL;
    }
    for (size_t i = 0; i < a->inputs_num; i++) {
        bstrVectorAdd(o->infile, a->inputs[i]);
    }
    if (a->outfile_given > 0) {
        o->outfile = bstrVectorCreate((int)a->outfile_given);
    } else {
        o->outfile = NULL;
    }
    for (size_t i = 0; i < a->outfile_given; i++) {
        bstrVectorAdd(o->outfile, a->outfile_arg[i]);
        int r = alder_file_creatable(a->outfile_arg[i]);
        if (r != 0) status = 1;
    }
    o->logfilename = bfromcstr(a->log_arg);
    if (a->reference_given > 0) {
        o->refile = bstrVectorCreate((int)a->reference_given);
    } else {
        o->refile = NULL;
    }
    for (size_t i = 0; i < a->reference_given; i++) {
        bstrVectorAdd(o->refile, a->reference_arg[i]);
        int r = alder_file_exist(a->reference_arg[i]);
        if (r != 0) {
            status = 1;
        }
    }
    if (a->query_given) {
        o->query = bfromcstr(a->query_arg);
    } else {
        o->query = NULL;
    }
    return status;
}

void alder_mem_option_free(alder_mem_option_t *o)
{
    if (o->refile != NULL) bstrListDestroy(o->refile);
    if (o->infile != NULL) bstrListDestroy(o->infile);
    if (o->outfile != NULL) bstrListDestroy(o->outfile);
    if (o->logfilename != NULL) bdestroy(o->logfilename);
    if (o->query != NULL) bdestroy(o->query);
}

