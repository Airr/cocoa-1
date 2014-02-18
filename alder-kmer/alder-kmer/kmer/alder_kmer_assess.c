/**
 * This file, alder_kmer_assess.c, is part of alder-kmer.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-kmer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kmer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_kmer_assess.h"
#include "alder_hashtable_mcas.h"

/**
 *  This function compares the approximately counted table with an exactly
 *  counted one.
 *
 *  @param tabfile exactly counted table
 *  @param infile  approximately counted table file
 *  @param outdir  output directory
 *  @param outfile outfile name
 *
 *  @return SUCCESS or FAIL
 */
int alder_kmer_assess(long upper_count,
                      const char *tabfile,
                      struct bstrList *infile,
                      const char *outdir,
                      const char *outfile)
{
    double *error_kmer = malloc(sizeof(*error_kmer) * upper_count);
    if (error_kmer == NULL) {
        return ALDER_STATUS_ERROR;
    }
    memset(error_kmer, 0, sizeof(*error_kmer) * upper_count);
    uint64_t *n_kmer = malloc(sizeof(*n_kmer) * upper_count);
    if (n_kmer == NULL) {
        XFREE(error_kmer);
        return ALDER_STATUS_ERROR;
    }
    
    alder_hashtable_mcas12_assess(error_kmer, n_kmer, upper_count,
                                  tabfile, bdata(infile->entry[0]), 0);
    
    fprintf(stdout, "*** Assessment of Approximation ***\n");
    for (long i = 0; i < upper_count; i++) {
        fprintf(stdout, "[%ld] %.f (%llu)\n", i+1, error_kmer[i], n_kmer[i]);
    }
    
    XFREE(error_kmer);
    XFREE(n_kmer);
    return ALDER_STATUS_SUCCESS;
}


