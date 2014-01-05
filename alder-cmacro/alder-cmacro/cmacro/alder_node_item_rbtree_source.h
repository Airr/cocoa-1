/**
 * This file, alder_node_item_rbtree_source.h, is part of alder-rbtree.
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

#ifndef alder_rbtree_alder_node_item_rbtree_source_h
#define alder_rbtree_alder_node_item_rbtree_source_h
#define ALDER_BASE_RBTREE_NODE
#include "alder_node_memory_template_on.h"

#include <stdint.h>

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

typedef struct TYPE_STRUCT
{
    int use;
    void *key;
    union {
        struct TYPE_STRUCT *left;
        struct TYPE_STRUCT *next;
    };
    struct TYPE_STRUCT *right;
    enum { red, black } color;
} TYPE;

#include "alder_node_item_common_source.h"

__END_DECLS

#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_RBTREE_NODE
#endif /* alder_rbtree_alder_node_item_rbtree_source_h */
