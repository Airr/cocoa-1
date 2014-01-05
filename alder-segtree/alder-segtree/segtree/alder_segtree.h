/**
 * This file, alder_segtree.h, is part of alder-segtree.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-segtree is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-segtree is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-segtree.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_segtree_alder_segtree_h
#define alder_segtree_alder_segtree_h

#include <stdint.h>
#include "alder_dictionary.h"
#include "alder_uc.h"
#include "alder_cmacro.h"
#include "alder_node_item_segtree_source.h"
#include "alder_node_item_tuple2_source.h"

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

enum {
    ALDER_SEGTREE_SUCCESS = 0,
    ALDER_SEGTREE_ERROR = 1,
    ALDER_SEGTREE_ERROR_MEMORY = 2,
    ALDER_SEGTREE_ERROR_DUPLICATE = 3,
    ALDER_SEGTREE_ERROR_NOTEXIST = 4
};

/* This C struct is for a segment tree.
 * uc - elements are 2-tuples, and sets are segtree_node.
 */
typedef struct alder_segtree_struct {
    alder_segtree_node_t *tree;
    alder_uc_t *uc;
    alder_dictionary_t *dictionary;
    struct alder_segtree_memory_struct *block;
    struct alder_item_tuple2_memory_struct *bintv;
} alder_segtree_t;

alder_segtree_t *alder_segtree_create();
void alder_segtree_destroy(alder_segtree_t *o);
void *alder_segtree_find (alder_segtree_t *o, int64_t x1, int64_t x2);
int alder_segtree_insert(alder_segtree_t *o, int64_t x1, int64_t x2);
int alder_segtree_delete(alder_segtree_t *o, int64_t x1, int64_t x2);

__END_DECLS

#endif /* alder_segtree_alder_segtree_h */
