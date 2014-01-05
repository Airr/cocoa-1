/**
 * This file, alder_uc.h, is part of alder-uc.
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
#ifndef alder_uc_alder_uc_h
#define alder_uc_alder_uc_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_uf_void.h"
#include "alder_dictionary.h"

#pragma mark uc node

/* I need two types for items. One is for elements in the union-copy
 * structure, and the other for elements in the union-find structure.
 * I could have a generic union-copy structure with void* element.
 * This would allow me to use different types of elements in a union-copy
 * structure.
 */

enum {
    ALDER_UC_SUCCESS = 0,
    ALDER_UC_ERROR = 1,
    ALDER_UC_ERROR_MEMORY = 2,
    ALDER_UC_ERROR_DUPLICATE = 3,
    ALDER_UC_ERROR_NOTEXIST = 4,
    ALDER_UC_ERROR_INVALID = 5
};

typedef enum {
    ALDER_UC_NODE_SET,
    ALDER_UC_NODE_ELEMENT,
    ALDER_UC_NODE_NORMAL,
    ALDER_UC_NODE_REVERSE
} alder_uc_node_type_t;


#include "alder_node_item_uc_source.h"
#include "alder_node_item_uc_edge_source.h"
#include "alder_node_item_int_source.h"
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

#pragma mark uf

/* I need data type and comparison function, and need to change
 * the union-find structure.
 * How can I generalize the union-find structure?
 * 1. I want to keep the tree structure.
 * 2. I want to keep the union-find's main structure.
 * 3. I need tree_node_t to have a void pointer for key.
 * 4. I need uf_node_t to have a void pointer for item node.
 * 5. I need to decouple the data and the operations.
 * 6. One more thing is how to have memory allocation separately for each
 *    union-find structure. I cannot create multiple union-find structures
 *    because every structure would access the same static memory.
 * 7. Let me create uf/alder_uf_void.[ch] files for all of these above.
 */

#pragma mark uc

/* Dictionaries contains sets and elements.
 */

/* is the entry point of the union-copy structure.
 */
typedef struct alder_uc {
    alder_dictionary_t *set;     // node - set name is a number.
    alder_dictionary_t *element; // node - element name is a 2-tuple.
    alder_uf_t *nuf;             // edge
    alder_uf_t *ruf;             // edge
    struct alder_uc_memory_struct *bnode;
    struct alder_item_int_memory_struct *bint;
    struct alder_item_tuple2_memory_struct *btuple2;
    alder_uc_node_t *list;       // node for enumeration
} alder_uc_t;

alder_uc_t * alder_uc_create();
void alder_uc_destroy(alder_uc_t *uc);
int alder_uc_set_create(alder_uc_t *uc, int setid);
int alder_uc_set_insert(alder_uc_t *uc, int setid, int64_t e1, int64_t e2);
int alder_uc_set_find(alder_uc_t *uc, int setid);
int alder_uc_set_union(alder_uc_t *uc, int setidI, int setidJ);
int alder_uc_set_copy(alder_uc_t *uc, int setidI, int setidJ);
int alder_uc_set_destroy(alder_uc_t *uc, int setid);
int alder_uc_element_create(alder_uc_t *uc, int64_t e1, int64_t e2);
int alder_uc_element_insert(alder_uc_t *uc);
int alder_uc_element_find(alder_uc_t *uc);
int alder_uc_element_union(alder_uc_t *uc);
int alder_uc_element_copy(alder_uc_t *uc);
int alder_uc_element_destroy(alder_uc_t *uc);

__END_DECLS

#endif /* alder_uc_alder_uc_h */
