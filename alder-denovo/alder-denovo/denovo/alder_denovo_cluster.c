/**
 * This file, alder_denovo_cluster.c, is part of alder-denovo.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-denovo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-denovo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-denovo.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_denovo_cluster.h"

/* This function takes a list of pairs of read matches to build a union tree.
 * fn - files
 * returns 0 on success, or -1 otherwise.
 */
int alder_denovo_cluster(struct bstrList *fn)
{
    for (int i = 0; i < fn->qty; i++) {
        fprintf(stdout, "%s\n", bdata(fn->entry[i]));
    }
    
    // See cs_ds-brass 6.2
    
    return 0;
}
