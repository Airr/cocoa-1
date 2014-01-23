/**
 * This file, alder_kmer_encode3_more.c, is part of alder-kmer.
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
#include "alder_kmer_encode4.h"


/**
 *  This function locks the reader so that only one thread can access to
 *  the reader's property such as file and writing something to input buffers.
 *
 *  @param a encoder4
 */
void alder_kmer_encoder4_lock_reader(alder_kmer_encoder4_t *a, int encoder4_id)
{
    /* lock */
    int oval = ALDER_KMER_ENCODER4_READER_UNLOCK;
    int cval = ALDER_KMER_ENCODER4_READER_LOCK;
    while (cval != oval) {
        oval = a->lock_reader;
        cval = oval + 1;
        if (oval == ALDER_KMER_ENCODER4_READER_UNLOCK) {
            cval = __sync_val_compare_and_swap(&a->lock_reader,
                                               (int)oval, (int)cval);
        }
    }
#if defined(ALDER_THREAD)
    alder_log("encoder4(%d): lock reader.", encoder4_id);
#endif
}

/**
 *  This function unlocks the reader (see lock_reader).
 *
 *  @param a encoder4
 */
void alder_kmer_encoder4_unlock_reader(alder_kmer_encoder4_t *a, int encoder4_id)
{
    /* lock */
    int oval = ALDER_KMER_ENCODER4_READER_LOCK;
    int cval = ALDER_KMER_ENCODER4_READER_UNLOCK;
    while (cval != oval) {
        oval = a->lock_reader;
        cval = oval - 1;
        if (oval == ALDER_KMER_ENCODER4_READER_LOCK) {
            cval = __sync_val_compare_and_swap(&a->lock_reader,
                                               (int)oval, (int)cval);
        }
    }
#if defined(ALDER_THREAD)
    alder_log("encoder4(%d): unlock reader.", encoder4_id);
#endif
}

/**
 *  This function locks the writer so that only one thread can access the
 *  property of the writer: writing something to output files.
 *
 *  @param a encoder4
 */
void alder_kmer_encoder4_lock_writer(alder_kmer_encoder4_t *a,
                                     int part_id)
{
    /* lock */
    int oval = ALDER_NO;
    int cval = ALDER_YES;
    while (cval != oval) {
        oval = a->lock_partition[part_id];
        cval = oval + 1;
        if (oval == ALDER_NO) {
            cval = __sync_val_compare_and_swap(a->lock_partition + part_id,
                                               (int)oval, (int)cval);
        }
    }
}

/**
 *  This function unlocks the writer (see lock_writer).
 *
 *  @param a encoder4
 */
void alder_kmer_encoder4_unlock_writer(alder_kmer_encoder4_t *a,
                                     int part_id)
{
    /* lock */
    int oval = ALDER_YES;
    int cval = ALDER_NO;
    while (cval != oval) {
        oval = a->lock_partition[part_id];
        cval = oval - 1;
        if (oval == ALDER_YES) {
            cval = __sync_val_compare_and_swap(a->lock_partition + part_id,
                                               (int)oval, (int)cval);
        }
    }
}


