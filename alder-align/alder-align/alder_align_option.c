/**
 * This file, alder_align_option.c, is part of alder-align.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-align is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-align is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-align.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "bstring/bstrlib.h"
#include "alder_align_option.h"

void alder_align_option_init(alder_align_option_t *option)
{
    option->infile = NULL;
    option->logfilename = NULL;
    option->outfile = NULL;
    option->refile = NULL;
}

void alder_align_option_free(alder_align_option_t *option)
{
    if (option->refile != NULL) bstrListDestroy(option->refile);
    if (option->infile != NULL) bstrListDestroy(option->infile);
    if (option->outfile != NULL) bstrListDestroy(option->outfile);
    if (option->logfilename != NULL) bdestroy(option->logfilename);
}