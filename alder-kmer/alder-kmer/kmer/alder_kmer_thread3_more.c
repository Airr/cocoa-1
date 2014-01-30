/**
 * This file, alder_kmer_thread3_more.c, is part of alder-kmer.
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
#include "alder_kmer_thread3.h"



/**
 *  This function increases the count of blocks by 1 atomically.
 *
 *  @param a    counter
 *  @param i_np partition index
 *
 *  @return void
 */
void alder_kmer_counter3_increment_n_block(alder_kmer_thread3_t *a, uint64_t i_np)
{
    int oval = a->n_blockByCounter[i_np];
    int cval = oval + 1;
    while (oval != cval) {
        oval = a->n_blockByCounter[i_np];
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(a->n_blockByCounter + i_np,
                                           (int)oval,
                                           (int)cval);
    }
}
