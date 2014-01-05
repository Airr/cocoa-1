/**
 * This file, alder_node_memory_template_on.h, is part of alder-uc.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-uc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-uc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-uc.  If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(ALDER_BASE_UC_NODE)
#define BASE uc
#elif defined(ALDER_BASE_UC_EDGE_NODE)
#define BASE uc_edge
#elif defined(ALDER_BASE_UF_NODE)
#define BASE uf
#elif defined(ALDER_BASE_TREE_NODE)
#define BASE tree
#elif defined(ALDER_BASE_RBTREE_NODE)
#define BASE rbtree
#elif defined(ALDER_BASE_SEGTREE_NODE)
#define BASE segtree
#elif defined(ALDER_BASE_ITEM_INT_NODE)
#define BASE item_int
#elif defined(ALDER_BASE_ITEM_TUPLE2_NODE)
#define BASE item_tuple2
#elif defined(ALDER_BASE_VOID_NODE)
#define BASE void
#endif

#define CONCAT2x(a,b) a ## _ ## b
#define CONCAT2(a,b) CONCAT2x(a,b)
#define CONCAT3x(a,b,c) a ## _ ## b ## _ ## c
#define CONCAT3(a,b,c) CONCAT3x(a,b,c)
#define CONCAT4x(a,b,c,d) a ## _ ## b ## _ ## c ## _ ## d
#define CONCAT4(a,b,c,d) CONCAT4x(a,b,c,d)

#define STRING(x) #x
#define EXPAND(x) STRING(x)
#define NAME(x) EXPAND(TYPE(x))

#define TYPE CONCAT3(alder,BASE,node_t)
#define TYPE_STRUCT CONCAT3(alder,BASE,node_struct)
#define STACK_TYPE CONCAT3(alder_stack,BASE,t)
#define STACK_TYPE_STRUCT CONCAT3(alder_stack,BASE,struct)
#define FUNCTION(a) CONCAT2(a,BASE)
#define MEMORY_TYPE CONCAT3(alder,BASE,memory_struct)
#define BLOCKSIZE 256
