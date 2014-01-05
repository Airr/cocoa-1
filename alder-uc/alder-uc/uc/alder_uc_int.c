/**
 * This file, alder_uc_int.c, is part of alder-uc.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-uc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-uc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-uc.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include "alder_uc_int.h"
#include "alder_node_function_item_int_source.h"

/* This would create static functions that manage memory.
 * get_node_item_int
 * return_node_item_int
 * destroy_node_item_int
 * See struct alder_item_int_memory_struct.
 */
#define ALDER_BASE_ITEM_INT_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_ITEM_INT_NODE

/* This function creates a union-copy structure with integer elements.
 */
alder_uc_int_t * alder_uc_int_create()
{
    alder_uc_int_t *uc = malloc(sizeof*uc);
    if (uc != NULL) {
        uc->uc = alder_uc_create(compare_item_int, compare_item_int);
        uc->bint = create_node_item_int();
        if (uc->uc == NULL || uc->bint == NULL) {
            alder_uc_int_destroy(uc);
            uc = NULL;
        }
    }
    return uc;
}

/* This function destroys the given union-copy structure.
 */
void alder_uc_int_destroy(alder_uc_int_t *uc)
{
    if (uc != NULL) {
        alder_uc_destroy(uc->uc);
        destroy_node_item_int(uc->bint);
        XFREE(uc);
    }
}

/* This function creates a new empty set of integer elements.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory,
 * ALDER_ERROR_DUPULICATE if the set already exists.
 */
int alder_uc_int_set_create(alder_uc_int_t *uc, int setid)
{
    int s;
    alder_item_int_node_t *setID = get_node_item_int(uc->bint);
    if (setID == NULL) return ALDER_UC_ERROR_MEMORY;
    setID->item = setid;
    s = alder_uc_set_create(uc->uc, setID);
    return s;
}

/* This function inserts an element into a set.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory,
 * ALDER_UC_ERROR_NOTEXIST unless either set or element exists.
 */
int alder_uc_int_set_insert(alder_uc_int_t *uc, int setid, int elementid)
{
    int s = ALDER_UC_SUCCESS;
    alder_item_int_node_t *setID = NULL;
    alder_item_int_node_t *elementID = NULL;
    
    setID = get_node_item_int(uc->bint);
    elementID = get_node_item_int(uc->bint);
    if (setID == NULL || elementID == NULL) {
        s = ALDER_UC_ERROR_MEMORY;
    } else {
        setID->item = setid;
        elementID->item = elementid;
        s = alder_uc_set_insert(uc->uc, setID, elementID);
    }
    return_node_item_int(uc->bint, setID);
    return_node_item_int(uc->bint, elementID);
    return s;
}

/* This function collects all of the elements in a set.
 * returns
 * ALDER_UC_SUCCESS on success, 
 * ALDER_UC_ERROR_MEMORY if not enough memeory,
 * ALDER_UC_ERROR_NOTEXIST if no such set exists.
 *
 * The first element in the set is at uc->uc->list->list.
 * The set is at uc->uc->list. Follow the linked list to access all of the
 * elements in the given set till list being NULL.
 */
int alder_uc_int_set_find(alder_uc_int_t *uc, int setid)
{
    int s = ALDER_UC_SUCCESS;
    alder_item_int_node_t *setID = get_node_item_int(uc->bint);
    ALDER_RETURN_ERROR_IF_NULL(setID, ALDER_UC_ERROR_MEMORY);
    setID->item = setid;
    s = alder_uc_set_find(uc->uc, setID);
    return_node_item_int(uc->bint, setID);
    return s;
}

/* This function takes the union of two sets.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory,
 * ALDER_UC_ERROR_INVALID if Sj is not empty.
 * ALDER_UC_ERROR_NOTEXIST if no such set exists,
 */
int alder_uc_int_set_union(alder_uc_int_t *uc, int seti, int setj)
{
    int s = ALDER_UC_SUCCESS;
    alder_item_int_node_t *setIDi = get_node_item_int(uc->bint);
    ALDER_RETURN_ERROR_IF_NULL(setIDi, ALDER_UC_ERROR_MEMORY);
    alder_item_int_node_t *setIDj = get_node_item_int(uc->bint);
    if (setIDj == NULL) {
        return_node_item_int(uc->bint, setIDi);
        return ALDER_UC_ERROR_MEMORY;
    }
    setIDi->item = seti;
    setIDj->item = setj;
    s = alder_uc_set_union(uc->uc, setIDi, setIDj);
    return_node_item_int(uc->bint, setIDi);
    return_node_item_int(uc->bint, setIDj);
    return s;
}

/* This function copies
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory,
 * ALDER_UC_ERROR_INVALID if Sj is not empty.
 * ALDER_UC_ERROR_NOTEXIST if no such set exists,
 */
int alder_uc_int_set_copy(alder_uc_int_t *uc, int seti, int setj)
{
    int s = ALDER_UC_SUCCESS;
    alder_item_int_node_t *setIDi = get_node_item_int(uc->bint);
    ALDER_RETURN_ERROR_IF_NULL(setIDi, ALDER_UC_ERROR_MEMORY);
    alder_item_int_node_t *setIDj = get_node_item_int(uc->bint);
    if (setIDj == NULL) {
        return_node_item_int(uc->bint, setIDi);
        return ALDER_UC_ERROR_MEMORY;
    }
    setIDi->item = seti;
    setIDj->item = setj;
    s = alder_uc_set_copy(uc->uc, setIDi, setIDj);
    return_node_item_int(uc->bint, setIDi);
    return_node_item_int(uc->bint, setIDj);
    return s;
}

/* This function destroys a set.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory, 
 * ALDER_UC_ERROR_NOTEXIST if no such set exists,
 */
int alder_uc_int_set_destroy(alder_uc_int_t *uc, int setid)
{
    int s = ALDER_UC_SUCCESS;
    alder_item_int_node_t *setID = get_node_item_int(uc->bint);
    ALDER_RETURN_ERROR_IF_NULL(setID, ALDER_UC_ERROR_MEMORY);
    setID->item = setid;
    
    alder_item_int_node_t *setID2 = NULL;
    s = alder_uc_set_destroy(uc->uc, setID, (void**)&setID2);
    if (s == ALDER_UC_SUCCESS) {
        assert(setID2 != NULL);
        return_node_item_int(uc->bint, setID2);
    }
    return_node_item_int(uc->bint, setID);
    return s;
}

/* This function creates an element that does not belong to any set.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory,
 * ALDER_ERROR_DUPULICATE if the set already exists.
 */
int alder_uc_int_element_create(alder_uc_int_t *uc, int elementid)
{
    int s;
    alder_item_int_node_t *elementID = get_node_item_int(uc->bint);
    if (elementID == NULL) return ALDER_UC_ERROR_MEMORY;
    elementID->item = elementid;
    s = alder_uc_element_create(uc->uc, elementID);
    return ALDER_UC_SUCCESS;
}

int alder_uc_int_element_insert(alder_uc_int_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_int_element_find(alder_uc_int_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_int_element_union(alder_uc_int_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_int_element_copy(alder_uc_int_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

void alder_uc_int_test()
{
    
    printf("Hello, World!\n");
}
