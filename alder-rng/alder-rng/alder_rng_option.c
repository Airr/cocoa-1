/**
 * This file, alder_rng_option.c, is part of alder-rng.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-rng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-rng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-rng.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cmdline.h"
#include "bstrmore.h"
#include "alder_rng_option.h" 

int alder_rng_option_init(alder_rng_option_t *o, struct gengetopt_args_info *a)
{
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
    return 0;
}

void alder_rng_option_free(alder_rng_option_t *o)
{
    if (o->infile != NULL) bstrListDestroy(o->infile);
    if (o->outfile != NULL) bstrListDestroy(o->outfile);
    if (o->logfilename != NULL) bdestroy(o->logfilename);
}

