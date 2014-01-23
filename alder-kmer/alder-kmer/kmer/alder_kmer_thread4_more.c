/**
 * This file, alder_kmer_thread4_more.c, is part of alder-kmer.
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
#include "alder_kmer_thread4.h"


/**
 *  This function locks the reader so that only one thread can access to
 *  the reader's property such as file and writing something to input buffers.
 *
 *  @param a encoder
 */
void alder_kmer_counter4_lock_reader(alder_kmer_thread4_t *a)
{
    /* lock */
    int oval = ALDER_KMER_COUNTER4_READER_UNLOCK;
    int cval = ALDER_KMER_COUNTER4_READER_LOCK;
    while (cval != oval) {
        oval = a->lock_reader;
#if defined(ALDER_THREAD)
        alder_log("counter(): trying locking reader (oval:%d).", oval);
#endif
        cval = oval + 1;
        if (oval == ALDER_KMER_COUNTER4_READER_UNLOCK) {
            cval = __sync_val_compare_and_swap(&a->lock_reader,
                                               (int)oval, (int)cval);
        }
    }
}

/**
 *  This function unlocks the reader (see lock_reader).
 *
 *  @param a encoder
 */
void alder_kmer_counter4_unlock_reader(alder_kmer_thread4_t *a)
{
    /* lock */
    int oval = ALDER_KMER_COUNTER4_READER_LOCK;
    int cval = ALDER_KMER_COUNTER4_READER_UNLOCK;
    while (cval != oval) {
        oval = a->lock_reader;
        cval = oval - 1;
        if (oval == ALDER_KMER_COUNTER4_READER_LOCK) {
            cval = __sync_val_compare_and_swap(&a->lock_reader,
                                               (int)oval, (int)cval);
        }
    }
}

/**
 *  This function locks the writer so that only one thread can access the
 *  property of the writer: writing something to output files.
 *
 *  @param a encoder
 */
void alder_kmer_counter4_lock_writer(alder_kmer_thread4_t *a, uint64_t i_np)
{
    /* lock */
    int oval = ALDER_KMER_COUNTER4_WRITER_UNLOCK;
    int cval = ALDER_KMER_COUNTER4_WRITER_LOCK;
    while (cval != oval) {
        oval = a->lock_writer;
#if defined(ALDER_THREAD)
        alder_log("counter(): trying locking writer (oval:%d).", oval);
#endif
        cval = oval + 1;
        if (oval == ALDER_KMER_COUNTER4_WRITER_UNLOCK &&
            a->main_i_np == i_np) {
            cval = __sync_val_compare_and_swap(&a->lock_writer,
                                               (int)oval, (int)cval);
        }
    }
}

/**
 *  This function unlocks the writer (see lock_writer).
 *
 *  @param a encoder
 */
void alder_kmer_counter4_unlock_writer(alder_kmer_thread4_t *a)
{
    /* lock */
    int oval = ALDER_KMER_COUNTER4_WRITER_LOCK;
    int cval = ALDER_KMER_COUNTER4_WRITER_UNLOCK;
    while (cval != oval) {
        oval = a->lock_writer;
        cval = oval - 1;
        if (oval == ALDER_KMER_COUNTER4_WRITER_LOCK) {
            cval = __sync_val_compare_and_swap(&a->lock_writer,
                                               (int)oval, (int)cval);
        }
    }
}


/**
 *  This function increases the count of blocks by 1 atomically.
 *
 *  @param a    counter
 *  @param i_np partition index
 *
 *  @return void
 */
void alder_kmer_counter4_increment_n_block(alder_kmer_thread4_t *a, uint64_t i_np)
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
