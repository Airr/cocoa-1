/**
 * This file, alder_imaa_option.c, is part of alder-imaa.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-imaa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-imaa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-imaa.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include "cmdline.h"
#include "bstrmore.h"
#include "alder_file_isstdin.h"
#include "alder_imaa_cite.h" 
#include "alder_imaa_option.h" 

int alder_imaa_option_init(alder_imaa_option_t *o, struct gengetopt_args_info *a)
{
    int status = 0;
    if (a->cite_given) {
        alder_imaa_cite();
        status = 1;
        return status;
    }
    o->is_stdin = alder_file_isstdin();
    // logc_logWarning(ERROR_LOGGER, "no input file.");
    if (o->is_stdin == 0 && a->inputs_num == 0) {
        status = 1;
    }
    // logc_logWarning(ERROR_LOGGER, "standard input and input files.");
    if (o->is_stdin == 1 && a->inputs_num > 0) {
        status = 1;
    }
    if (status == 1) {
        return status;
    }

    o->infile = NULL;
    o->outfile = NULL;
    o->logfilename = NULL;
    if (a->inputs_num > 0) o->infile = bstrVectorCreate((int)a->inputs_num);
    for (size_t i = 0; i < a->inputs_num; i++) {
        bstrVectorAdd(o->infile, a->inputs[i]);
    }
    if (a->outfile_given > 0) o->outfile = bstrVectorCreate((int)a->outfile_given);
    for (size_t i = 0; i < a->outfile_given; i++) {
        bstrVectorAdd(o->outfile, a->outfile_arg[i]);
    }
    if (a->log_given) {
        o->logfilename = bfromcstr(a->log_arg);
    }
    return status;
}

void alder_imaa_option_free(alder_imaa_option_t *o)
{
    if (o->infile != NULL) bstrListDestroy(o->infile);
    if (o->outfile != NULL) bstrListDestroy(o->outfile);
    if (o->logfilename != NULL) bdestroy(o->logfilename);
}

