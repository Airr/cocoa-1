/**
 * This file, alder_file_option.c, is part of alder-file.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-file.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "cmdline.h"
#include "alder_file_isstdin.h"
#include "alder_file_option.h"

int alder_file_option_init(alder_file_option_t *o,
                           struct gengetopt_args_info *a)
{
    o->isstdin = alder_file_isstdin();
    if (a->output_given) {
        o->fout = fopen(a->output_arg, "w");
    } else {
        o->fout = stdout;
    }
    return 0;
}

void alder_file_option_free(alder_file_option_t *o)
{
    if (o->fout != stdout) {
        fclose(o->fout);
    }
}
