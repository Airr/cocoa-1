/**
 * This file, alder_cigar_queue.h, is part of alder-cigar.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-cigar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-cigar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-cigar.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_cigar_alder_cigar_queue_h
#define alder_cigar_alder_cigar_queue_h

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

typedef struct alder_cigar_item_struct {
    int n;
    int t;
    // 0: match (M)
    // 1: insertion (I)
    // 2: deletion(D)
    // 3: N
    // 4: S
    // 5: H
    // 6: P
    // 7: =
    // 8: X
} alder_cigar_item_t;

typedef struct alder_cigar_qu_t
{
    alder_cigar_item_t item;
    struct alder_cigar_qu_t *next;
} alder_cigar_queue_t;

void free_node();

alder_cigar_queue_t * alder_cigar_queue_create ();
int alder_cigar_queue_empty (alder_cigar_queue_t * qu);
void alder_cigar_enqueue (alder_cigar_item_t x, alder_cigar_queue_t * qu);
alder_cigar_item_t alder_cigar_dequeue (alder_cigar_queue_t * qu);
alder_cigar_item_t alder_cigar_front_element (alder_cigar_queue_t * qu);
void alder_cigar_remove_queue (alder_cigar_queue_t * qu);
size_t alder_cigar_size (alder_cigar_queue_t * qu);
size_t alder_cigar_strsize (alder_cigar_queue_t * qu);

__END_DECLS


#endif /* alder_cigar_alder_cigar_queue_h */
