/**
 * This file, alder_kmer_thread5_more.c, is part of alder-kmer.
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
#include "alder_logger.h"
#include "alder_kmer_thread5.h"


/**
 *  This function locks the reader so that only one thread can access to
 *  the reader's property such as file and writing something to input buffers.
 *
 *  @param a encoder
 *  @param counter_id counter id
 *
 *  @return YES (locked), NO otherwise.
 */
int alder_kmer_counter5_lock_reader(alder_kmer_thread5_t *a, int counter_id)
{
    /* lock */
    size_t p = a->cur_inbuf[counter_id];
    uint32_t oval = ALDER_KMER_COUNTER5_READER_UNLOCK;
    uint32_t cval = ALDER_KMER_COUNTER5_READER_LOCK;
    while (cval != oval) {
        p += a->size_subinbuf;
        if (p == a->size_data) {
            p = 0;
            a->cur_i_np[counter_id]++;
            assert(a->cur_i_np[counter_id] <= a->n_np);
            if (a->cur_i_np[counter_id] == a->n_np) {
                // exit.
                break;
            }
        }
        oval = to_uint32(a->inbuf, p);
        cval = oval + 1;
        if (oval % 2 == 0 && oval / 2 == (uint32_t)a->cur_i_np[counter_id]) {
            cval = __sync_val_compare_and_swap(a->inbuf + p,
                                               (uint32_t)oval, (uint32_t)cval);
        }
    }
    if (cval == oval) {
        a->cur_inbuf[counter_id] = p;
        return ALDER_YES;
    } else {
        // cur_inbuf is undefined.
        return ALDER_NO;
    }
}

/**
 *  This function unlocks the input buffer section.
 *
 *  @param a encoder
 */
void alder_kmer_counter5_unlock_reader(alder_kmer_thread5_t *a, int counter_id)
{
    /* unlock */
    size_t p = a->cur_inbuf[counter_id];
    uint32_t oval = to_uint32(a->inbuf, p);
    uint32_t cval = oval + 1;
    assert(oval % 2);
    cval = __sync_val_compare_and_swap(a->inbuf + p, (uint32_t)oval, (uint32_t)cval);
    assert(cval == oval);
}

/**
 *  This function increases the count of blocks by 1 atomically.
 *  n_blockByCounter is an array of size 2. Partitions of even-numbered index
 *  would use 0-th table, and those of odd-numbered index  would use the other
 *  1-st table. 
 *
 *  @param a    counter
 *  @param i_np partition index
 *
 *  @return YES or NO (YES for the end of the blocks)
 */
int alder_kmer_counter5_increment_n_block(alder_kmer_thread5_t *a,
                                          uint64_t i_np,
                                          size_t i_table,
                                          int counter_id)
{
//    uint64_t i_table = i_np % 2;
    int64_t oval = a->n_blockByCounter[i_table];
    int64_t cval = oval + 1;
    while (oval != cval) {
        oval = a->n_blockByCounter[i_table];
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(a->n_blockByCounter + i_table,
                                           (int64_t)oval,
                                           (int64_t)cval);
    }
    alder_log3("counter(%d): Part(%llu/%llu) Block(%lld/%lld)", counter_id, i_np, a->n_np, cval + 1, a->n_blockByReader[i_table]);
    if (cval + 1 == a->n_blockByReader[i_table]) {
        //
        return ALDER_YES;
    } else {
        return ALDER_NO;
    }
    

    
    /* Progress */
//    a->n_byte += a->size_subinbuf;
//    if (a->progress_flag) {
//        alder_progress_step(a->n_byte, a->totalFilesize,
//                            a->progressToError_flag);
//    }

}





