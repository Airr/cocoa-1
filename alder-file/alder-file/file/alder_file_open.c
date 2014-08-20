/**
 * This file, alder_file_open.c, is part of alder-file.
 *
 * Copyright 2014 by Sang Chul Choi
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
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "alder_file_open.h"

/**
 *  This function attempts to open a file and return FILE pointer.
 *  if outfile_given is 0, it returns stdout.
 *
 *  @param outfile_given outfile given or not
 *  @param outdir        outdirectory
 *  @param outfile       outfile name
 *
 *  @return FILE pointer
 */
FILE *
alder_file_open_with_outdirfilegiven(unsigned int outfile_given,
                                     const char *outdir,
                                     const char *outfile)
{
    FILE *fpout = stdout;
    if (outfile_given) {
        bstring bfn = bformat("%s/%s", outdir, outfile);
        if (bfn == NULL) {
            return NULL;
        }
        fpout = fopen(bdata(bfn), "w");
        if (fpout == NULL) {
            bdestroy(bfn);
            return NULL;
        }
        bdestroy(bfn);
    }
    return fpout;
}