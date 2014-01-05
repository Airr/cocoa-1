/**
 * This file, alder_rbtree_int.h, is part of alder-rbtree.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-rbtree is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-rbtree is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-rbtree.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_rbtree_alder_rbtree_int_h
#define alder_rbtree_alder_rbtree_int_h

#include <stdint.h>
#include "alder_rbtree.h"
#include "alder_cmacro.h"
#include "alder_node_item_int_source.h"

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

typedef struct alder_rbtree_int_struct {
    alder_rbtree_t *tree;
    struct alder_int_memory_struct *block;
} alder_rbtree_int_t;


__END_DECLS


#endif /* alder_rbtree_alder_rbtree_int_h */
