/**
 * This file, alder_uc_backup.c, is part of alder-uc.
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

#include <stdio.h>
#include "alder_uc_backup.h"

/**
 * This file, alder_uc.c, is part of alder-uc.
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

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "alder_cmacro.h"
#include "alder_stack_void.h"
#include "alder_uc.h"
#include "alder_node_function_uc_source.h"

#pragma mark function

#include "alder_node_function_int_source.h"
#include "alder_node_function_tuple2_source.h"

#pragma mark memory

#define ALDER_BASE_TUPLE2_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_TUPLE2_NODE

/* This would create static functions that manage memory.
 * get_node_item_int
 * return_node_item_int
 * destroy_node_item_int
 * See struct alder_item_int_memory_struct.
 */
#define ALDER_BASE_INT_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_INT_NODE

/* This would create static functions that manage memory.
 * get_node_uc
 * return_node_uc
 * destroy_node_uc
 * See struct alder_uc_memory_struct.
 */
#define ALDER_BASE_UC_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_UC_NODE

#pragma mark uc

/* This function prepares for the usage of the union-copy structure.
 * TODO: alder_uc_create(compare_int, compare_tuple2)
 * returns a union-copy structure on success, and NULL otherwise.
 */
alder_uc_t * alder_uc_create()
{
    alder_uc_t *uc = malloc(sizeof(*uc));
    if (uc == NULL) {
        XFREE(uc);
        return NULL;
    }
    uc->bnode = create_node_uc();
    uc->bint = create_node_item_int();
    uc->btuple2 = create_node_item_tuple2();
    uc->set = alder_dictionary_create(compare_int, print_int);
    uc->element = alder_dictionary_create(compare_tuple2, print_tuple2);
    uc->nuf = alder_uf_create(compare_uc, print_uc);
    uc->ruf = alder_uf_create(compare_uc, print_uc);
    
    if (uc->bnode == NULL || uc->bint == NULL || uc->btuple2 == NULL ||
        uc->set == NULL || uc->element == NULL ||
        uc->nuf == NULL || uc->ruf == NULL) {
        alder_uc_destroy(uc);
        return NULL;
    }
    return uc;
}

/* This function destroys the given union-copy structure.
 * TODO: Check the memory allocation.
 */
void alder_uc_destroy(alder_uc_t *uc)
{
    if (uc != NULL) {
        alder_dictionary_destroy(uc->set);
        alder_dictionary_destroy(uc->element);
        alder_uf_destroy(uc->nuf);
        alder_uf_destroy(uc->ruf);
        destroy_node_uc(uc->bnode);
        destroy_node_item_int(uc->bint);
        destroy_node_item_tuple2(uc->btuple2);
        XFREE(uc);
    }
}

/* This function creates a new empty set.
 *
 */
int alder_uc_set_create(alder_uc_t *uc, int setid)
{
    int s;
    alder_uc_node_t *set = get_node_uc(uc->bnode);
    if (set == NULL) return ALDER_UC_ERROR_MEMORY;
    alder_item_int_node_t *setID = get_node_item_int(uc->bint);
    if (setID == NULL) return ALDER_UC_ERROR_MEMORY;
    setID->item = setid;
    set->type = ALDER_UC_NODE_SET;
    set->up = NULL;
    set->down = NULL;
    set->item = setID;
    s = alder_dictionary_insert2(uc->set, setID, set);
    
    return s;
}

/* This function inserts a 2-tuple element to a set.
 * returns
 * ALDER_UC_ERROR_MEMORY if not enough memeory.
 * ALDER_UC_ERROR_NOTEXIST unless either set or element exists.
 */
int alder_uc_set_insert(alder_uc_t *uc, int setid, int64_t e1, int64_t e2)
{
    int s;
    alder_item_int_node_t *setID = NULL;
    alder_item_tuple2_node_t *elementID = NULL;
    
    setID = get_node_item_int(uc->bint);
    elementID = get_node_item_tuple2(uc->btuple2);
    if (setID == NULL || elementID == NULL) {
        return_node_item_int(uc->bint, setID);
        return_node_item_tuple2(uc->btuple2, elementID);
        return ALDER_UC_ERROR_MEMORY;
    }
    
    setID->item = setid;
    elementID->e1 = e1;
    elementID->e2 = e2;
    alder_uc_node_t *set = alder_dictionary_find(uc->set, setID);
    alder_uc_node_t *element = alder_dictionary_find(uc->element, elementID);
    if (set == NULL || element == NULL) {
        return_node_item_int(uc->bint, setID);
        return_node_item_tuple2(uc->btuple2, elementID);
        return ALDER_UC_ERROR_NOTEXIST;
    }
    
    alder_uc_node_t *outS = set->down;
    alder_uc_node_t *inX = element->up;
    alder_uc_node_t *orgE = NULL;
    
    if (outS == NULL) {
        orgE = set;
    } else if (outS->type == ALDER_UC_NODE_NORMAL) {
        alder_uc_node_t *normal = get_node_uc(uc->bnode);
        if (normal == NULL) {
            return ALDER_UC_ERROR_MEMORY;
        }
        normal->type = ALDER_UC_NODE_NORMAL;
        normal->up = NULL;
        normal->down = NULL; /* orgE would change this down. */
        s = alder_uf_makeset(uc->nuf, normal);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        s = alder_uf_union(uc->nuf, outS, normal);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        alder_uc_node_t *root = alder_uf_root(uc->nuf, outS);
        assert(root != NULL);
        set->down = root;
        root->up = set;
        orgE = normal;
    } else if (outS->type == ALDER_UC_NODE_REVERSE ||
               outS->type == ALDER_UC_NODE_ELEMENT) {
        alder_uc_node_t *normal = get_node_uc(uc->bnode);
        if (normal == NULL) {
            return ALDER_UC_ERROR_MEMORY;
        }
        normal->type = ALDER_UC_NODE_NORMAL;
        normal->up = NULL;
        normal->down = outS;
        s = alder_uf_makeset(uc->nuf, normal);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        
        alder_uc_node_t *normal2 = get_node_uc(uc->bnode);
        if (normal2 == NULL) {
            return ALDER_UC_ERROR_MEMORY;
        }
        normal2->type = ALDER_UC_NODE_NORMAL;
        normal2->up = NULL;
        
        s = alder_uf_makeset(uc->nuf, normal2);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        
        s = alder_uf_union(uc->nuf, normal, normal2);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        alder_uc_node_t *root = alder_uf_root(uc->nuf, normal);
        assert(root != NULL);
        set->down = root;
        root->up = set;
        orgE = normal2;
    } else {
        abort();
    }
    
    if (inX == NULL) {
        element->up = orgE;
        orgE->down = element;
    } else if (inX->type == ALDER_UC_NODE_REVERSE) {
        alder_uc_node_t *reverse = get_node_uc(uc->bnode);
        if (reverse == NULL) {
            return ALDER_UC_ERROR_MEMORY;
        }
        reverse->type = ALDER_UC_NODE_REVERSE;
        reverse->up = NULL; /* orgE changes this up. */
        reverse->down = NULL;
        s = alder_uf_makeset(uc->ruf, reverse);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        
        s = alder_uf_union(uc->nuf, inX, reverse);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        alder_uc_node_t *root = alder_uf_root(uc->ruf, inX);
        element->up = root;
        root->down = element;
        reverse->up = orgE;
        orgE->down = reverse;
    } else if (inX->type == ALDER_UC_NODE_NORMAL ||
               inX->type == ALDER_UC_NODE_SET) {
        alder_uc_node_t *reverse = get_node_uc(uc->bnode);
        if (reverse == NULL) {
            return ALDER_UC_ERROR_MEMORY;
        }
        reverse->type = ALDER_UC_NODE_REVERSE;
        reverse->up = inX;
        reverse->down = NULL;
        s = alder_uf_makeset(uc->ruf, reverse);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        
        alder_uc_node_t *reverse2 = get_node_uc(uc->bnode);
        if (reverse2 == NULL) {
            return ALDER_UC_ERROR_MEMORY;
        }
        reverse2->type = ALDER_UC_NODE_REVERSE;
        reverse2->up = NULL; /* orgE changes this up. */
        reverse2->down = NULL;
        s = alder_uf_makeset(uc->ruf, reverse2);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        
        s = alder_uf_union(uc->ruf, reverse, reverse2);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        alder_uc_node_t *root = alder_uf_root(uc->ruf, reverse);
        element->up = root;
        root->down = element;
        reverse2->up = orgE;
        orgE->down = reverse2;
    } else {
        abort();
    }
    
    return_node_item_int(uc->bint, setID);
    return ALDER_UC_SUCCESS;
}

/* This function reports all elements in a set.
 * Use uc->list to find all of the elements in the set after a call to this
 * function. uc-list points to a set node, and list member of the set node
 * points to an element in the set. Follow the list till the end, or NULL.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_NOTEXIST if no such set exists,
 */
int alder_uc_set_find(alder_uc_t *uc, int setid)
{
    int s;
    alder_item_int_node_t *setID = NULL;
    
    setID = get_node_item_int(uc->bint);
    setID->item = setid;
    alder_uc_node_t *set = alder_dictionary_find(uc->set, setID);
    if (set == NULL) {
        uc->list = NULL;
        return_node_item_int(uc->bint, setID);
        return ALDER_UC_ERROR_NOTEXIST;
    }
    alder_uc_node_t *outS = set->down;
    if (outS == NULL) {
        uc->list = NULL;
        return_node_item_int(uc->bint, setID);
        return ALDER_UC_SUCCESS;
    }
    
    uc->list = outS;
    alder_uc_node_t *uclist = outS;
    alder_stack_void_t *stack = alder_stack_create_void(BLOCKSIZE);
    alder_stack_push_void(stack, outS);
    while (!alder_stack_empty_void(stack)) {
        alder_uc_node_t *dest = alder_stack_pop_void(stack);
        assert(dest != NULL);
        if (dest->type == ALDER_UC_NODE_ELEMENT) {
            // report the element as an answer.
            uclist->list = dest;
            uclist = uclist->list;
        } else if (dest->type == ALDER_UC_NODE_NORMAL) {
            // push elements in the normal node.
            s = alder_uf_enumerate(uc->nuf, dest);
            if (s != ALDER_UC_SUCCESS) {
                uc->list = NULL;
                return_node_item_int(uc->bint, setID);
                return s;
            }
            alder_uf_node_t *t = uc->nuf->uf->list;
            while (t != NULL) {
                alder_uc_node_t *eprime = t->item;
                outS = eprime->down;
                alder_stack_push_void(stack, outS);
                t = t->list;
            }
        } else if (dest->type == ALDER_UC_NODE_REVERSE) {
            // push one element in the reverse node.
            // Find the root of the reverse uf.
            alder_uc_node_t *eprime = alder_uf_root(uc->ruf, dest);
            assert(eprime != NULL);
            assert(eprime->down != NULL);
            outS = eprime->down;
            alder_stack_push_void(stack, outS);
        }
    }
    uclist->list = NULL;
    return_node_item_int(uc->bint, setID);
    
    return ALDER_UC_SUCCESS;
}

/* This function takes the union of the two sets.
 * The two sets must be disjoint.
 */
int alder_uc_set_union(alder_uc_t *uc, int setidI, int setidJ)
{
    int s;
    alder_item_int_node_t *setIDi = NULL;
    alder_item_int_node_t *setIDj = NULL;
    alder_uc_node_t *setT;
    
    setIDi = get_node_item_int(uc->bint);
    if (setIDi == NULL) {
        return ALDER_UC_ERROR_MEMORY;
    }
    setIDi->item = setidI;
    alder_uc_node_t *setI = alder_dictionary_find(uc->set, setIDi);
    if (setI == NULL) {
        return_node_item_int(uc->bint, setIDi);
        return ALDER_UC_ERROR_NOTEXIST;
    }
    
    setIDj = get_node_item_int(uc->bint);
    if (setIDj == NULL) {
        return_node_item_int(uc->bint, setIDi);
        return ALDER_UC_ERROR_MEMORY;
    }
    setIDj->item = setidJ;
    alder_uc_node_t *setJ = alder_dictionary_find(uc->set, setIDj);
    if (setJ == NULL) {
        return_node_item_int(uc->bint, setIDi);
        return_node_item_int(uc->bint, setIDj);
        return ALDER_UC_ERROR_NOTEXIST;
    }
    
    alder_uc_node_t *outI = setI->down;
    alder_uc_node_t *outJ = setJ->down;
    if (outI == NULL || outI->type != ALDER_UC_NODE_NORMAL) {
        setT = setI;
        setI = setJ;
        setJ = setT;
        setIDi->item = setidJ;
        setIDj->item = setidI;
    }
    if (outJ == NULL) {
        // Delete Sj.
        alder_dictionary_delete(uc->set, setIDj);
        // FIXME: Delete the item in the set J!
        return_node_item_int(uc->bint, setIDi);
        return_node_item_int(uc->bint, setIDj);
        return ALDER_UC_SUCCESS;
    }
    if (outI->type == ALDER_UC_NODE_NORMAL &&
        outJ->type == ALDER_UC_NODE_NORMAL) {
        s = alder_uf_union(uc->nuf, outI, outJ);
        
        alder_uc_node_t *root = alder_uf_root(uc->nuf, outI);
        assert(root != NULL);
        setI->down = root;
        root->up = setI;
        alder_dictionary_delete(uc->set, setIDj);
    } else if (outI->type == ALDER_UC_NODE_NORMAL) {
        assert(outI->type == ALDER_UC_NODE_REVERSE ||
               outI->type == ALDER_UC_NODE_ELEMENT);
        // N-ADD(child(Si),out(Sj))
        alder_uc_node_t *normal = get_node_uc(uc->bnode);
        if (normal == NULL) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return ALDER_UC_ERROR_MEMORY;
        }
        normal->type = ALDER_UC_NODE_NORMAL;
        normal->up = NULL;
        normal->down = outJ;
        s = alder_uf_union(uc->nuf, outI, normal);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        outJ->up = normal;
        alder_uc_node_t *root = alder_uf_root(uc->nuf, outI);
        setI->down = root;
        root->up = setI;
        alder_dictionary_delete(uc->set, setIDj);
    } else {
        assert(outI->type == ALDER_UC_NODE_ELEMENT ||
               outI->type == ALDER_UC_NODE_REVERSE);
        assert(outJ->type == ALDER_UC_NODE_ELEMENT ||
               outJ->type == ALDER_UC_NODE_REVERSE);
        alder_uc_node_t *normalI = get_node_uc(uc->bnode);
        if (normalI == NULL) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return ALDER_UC_ERROR_MEMORY;
        }
        normalI->type = ALDER_UC_NODE_NORMAL;
        
        alder_uc_node_t *normalJ = get_node_uc(uc->bnode);
        if (normalJ == NULL) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return ALDER_UC_ERROR_MEMORY;
        }
        normalJ->type = ALDER_UC_NODE_NORMAL;
        
        normalI->up = NULL;
        normalI->down = outI;
        outI->up = normalI;
        normalJ->up = NULL;
        normalJ->down = outJ;
        outJ->up = normalJ;
        
        s = alder_uf_makeset(uc->nuf, normalI);
        if (s != ALDER_UC_SUCCESS) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return s;
        }
        
        s = alder_uf_makeset(uc->nuf, normalJ);
        if (s != ALDER_UC_SUCCESS) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return s;
        }
        
        s = alder_uf_union(uc->nuf, normalI, normalJ);
        if (s != ALDER_UC_SUCCESS) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return s;
        }
        alder_uc_node_t *root = alder_uf_root(uc->nuf, normalI);
        setI->down = root;
        root->up = setI;
        // Delete Sj!
        alder_dictionary_delete(uc->set, setIDj);
    }
    
    return_node_item_int(uc->bint, setIDi);
    return_node_item_int(uc->bint, setIDj);
    return ALDER_UC_SUCCESS;
}

/* This function copies Si to make Sj.
 * Sj must be empty.
 * returns
 *
 * ALDER_UC_ERROR_INVALID if Sj is not empty.
 */
int alder_uc_set_copy(alder_uc_t *uc, int setidI, int setidJ)
{
    int s;
    alder_item_int_node_t *setIDi = NULL;
    alder_item_int_node_t *setIDj = NULL;
    
    setIDi = get_node_item_int(uc->bint);
    if (setIDi == NULL) {
        return ALDER_UC_ERROR_MEMORY;
    }
    setIDi->item = setidI;
    alder_uc_node_t *setI = alder_dictionary_find(uc->set, setIDi);
    if (setI == NULL) {
        return_node_item_int(uc->bint, setIDi);
        return ALDER_UC_ERROR_NOTEXIST;
    }
    
    setIDj = get_node_item_int(uc->bint);
    if (setIDj == NULL) {
        return_node_item_int(uc->bint, setIDi);
        return ALDER_UC_ERROR_MEMORY;
    }
    setIDj->item = setidJ;
    alder_uc_node_t *setJ = alder_dictionary_find(uc->set, setIDj);
    if (setJ == NULL) {
        return_node_item_int(uc->bint, setIDi);
        return_node_item_int(uc->bint, setIDj);
        return ALDER_UC_ERROR_NOTEXIST;
    }
    
    alder_uc_node_t *outI = setI->down;
    alder_uc_node_t *outJ = setJ->down;
    
    if (outJ != NULL) {
        return ALDER_UC_ERROR_INVALID;
    }
    if (outI == NULL) {
        // ready!
    } else if (outI->type == ALDER_UC_NODE_REVERSE) {
        alder_uc_node_t *root_old = alder_uf_root(uc->ruf, outI);
        
        alder_uc_node_t *reverseJ = get_node_uc(uc->bnode);
        reverseJ->type = ALDER_UC_NODE_REVERSE;
        reverseJ->up = setJ;
        reverseJ->down = NULL;
        setJ->down = reverseJ;
        
        s = alder_uf_makeset(uc->ruf, reverseJ);
        if (s != ALDER_UC_SUCCESS) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return s;
        }
        
        s = alder_uf_union(uc->ruf, outI, reverseJ);
        if (s != ALDER_UC_SUCCESS) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return s;
        }
        
        alder_uc_node_t *root = alder_uf_root(uc->ruf, outI);
        root->down = root_old->down;
        root_old->down->up = root;
    } else {
        assert(outI->type == ALDER_UC_NODE_ELEMENT ||
               outI->type == ALDER_UC_NODE_NORMAL);
        alder_uc_node_t *reverseI = get_node_uc(uc->bnode);
        if (reverseI == NULL) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return ALDER_UC_ERROR_MEMORY;
        }
        reverseI->type = ALDER_UC_NODE_REVERSE;
        reverseI->up = setI;
        reverseI->down = NULL;
        setI->up = reverseI;
        s = alder_uf_makeset(uc->ruf, reverseI);
        if (s != ALDER_UC_SUCCESS) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return s;
        }
        
        alder_uc_node_t *reverseJ = get_node_uc(uc->bnode);
        if (reverseJ == NULL) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return ALDER_UC_ERROR_MEMORY;
        }
        reverseJ->type = ALDER_UC_NODE_REVERSE;
        reverseJ->up = setJ;
        reverseJ->down = NULL;
        setJ->up = reverseJ;
        s = alder_uf_makeset(uc->ruf, reverseJ);
        if (s != ALDER_UC_SUCCESS) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return s;
        }
        
        s = alder_uf_union(uc->ruf, reverseI, reverseJ);
        if (s != ALDER_UC_SUCCESS) {
            return_node_item_int(uc->bint, setIDi);
            return_node_item_int(uc->bint, setIDj);
            return s;
        }
        
        alder_uc_node_t *root = alder_uf_root(uc->ruf, reverseI);
        root->down = outI;
        outI->up = root;
    }
    
    return_node_item_int(uc->bint, setIDi);
    return_node_item_int(uc->bint, setIDj);
    return ALDER_UC_SUCCESS;
}

/* This destroys a set with all of the elment in it.
 */
int alder_uc_set_destroy(alder_uc_t *uc, int setid)
{
    int s;
    alder_item_int_node_t *setID = NULL;
    
    setID = get_node_item_int(uc->bint);
    setID->item = setid;
    alder_uc_node_t *set = alder_dictionary_find(uc->set, setID);
    if (set == NULL) {
        uc->list = NULL;
        return_node_item_int(uc->bint, setID);
        return ALDER_UC_ERROR_NOTEXIST;
    }
    alder_uc_node_t *outS = set->down;
    if (outS == NULL) {
        // Delete Sj!
        // FIXME: delete item_int in the set!
        alder_dictionary_delete(uc->set, setID);
        return_node_item_int(uc->bint, setID);
        return ALDER_UC_SUCCESS;
    }
    
    // Collect nodes for Restore.
    alder_stack_void_t *stack = alder_stack_create_void(BLOCKSIZE);
    if (outS->type == ALDER_UC_NODE_ELEMENT) {
        outS->up = NULL;
    } else if (outS->type == ALDER_UC_NODE_REVERSE) {
        alder_stack_push_void(stack, outS);
    } else if (outS->type == ALDER_UC_NODE_NORMAL) {
        s = alder_uf_enumerate(uc->nuf, outS);
        if (s != ALDER_UC_SUCCESS) {
            return s;
        }
        alder_uf_node_t *t = uc->nuf->uf->list;
        while (t != NULL) {
            alder_uc_node_t *eprime = t->item;
            alder_uc_node_t *dest = eprime->down;
            if (dest->type == ALDER_UC_NODE_ELEMENT) {
                dest->up = NULL;
            } else {
                assert(dest->type == ALDER_UC_NODE_REVERSE);
                alder_stack_push_void(stack, dest);
            }
            t = t->list;
        }
    }
    
    // Restore
    while (!alder_stack_empty_void(stack)) {
        alder_uc_node_t *e = alder_stack_pop_void(stack);
        alder_uc_node_t *eprime = alder_uf_another(uc->ruf, e);
        assert(eprime != NULL);
        // e2 is another element in the union-find set.
        alder_uc_node_t *root_old = alder_uf_root(uc->ruf, eprime);
        alder_uf_delete(uc->ruf, e);
        alder_uc_node_t *root = alder_uf_root(uc->ruf, eprime);
        root->down = root_old->down;
        
        int nv = alder_uf_set_count(uc->ruf, eprime);
        assert(nv > 0);
        if (nv == 1) {
            assert(root == eprime);
            assert(eprime->up != NULL && eprime->down != NULL);
            if (eprime->up->type == ALDER_UC_NODE_SET ||
                eprime->down->type == ALDER_UC_NODE_ELEMENT) {
                eprime->up->down = eprime->down;
                eprime->down = eprime->up;
                // FIXME: need to delete item in UF node!
                alder_uf_delete(uc->ruf, eprime);
            } else {
                assert(eprime->up->type == ALDER_UC_NODE_NORMAL);
                assert(eprime->down->type == ALDER_UC_NODE_NORMAL);
                
                //                alder_uc_node_t *w = alder_uf_root(uc->nuf, eprime);
                
                s = alder_uf_enumerate(uc->nuf, eprime->down);
                if (s != ALDER_UC_SUCCESS) {
                    return s;
                }
                alder_uf_node_t *t = uc->nuf->uf->list;
                while (t != NULL) {
                    alder_uc_node_t *eprime2 = t->item;
                    //                    s = alder_uf_makeset(uc->nuf, eprime2);
                    alder_uf_union(uc->nuf, eprime->up, eprime2);
                    t = t->list;
                }
            }
            // remove child(v)
            // Not needed: alder_uf_delete(uc->ruf, eprime->down);
            // I just need to delete the reverse node.
        }
        // remove v.
        alder_uf_delete(uc->ruf, eprime);
    }
    
    alder_stack_destroy_void(stack);
    return ALDER_UC_SUCCESS;
}

int alder_uc_element_create(alder_uc_t *uc, int64_t e1, int64_t e2)
{
    int s;
    alder_uc_node_t *element = get_node_uc(uc->bnode);
    if (element == NULL) {
        return ALDER_UC_ERROR_MEMORY;
    }
    alder_item_tuple2_node_t *elementID = NULL;
    elementID = get_node_item_tuple2(uc->btuple2);
    if (elementID == NULL) {
        return ALDER_UC_ERROR_MEMORY;
    }
    elementID->e1 = e1;
    elementID->e2 = e2;
    
    element->type = ALDER_UC_NODE_ELEMENT;
    element->up = NULL;
    element->down = NULL;
    element->item = elementID;
    s = alder_dictionary_insert(uc->element, element);
    
    return s;
}

int alder_uc_element_insert(alder_uc_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_element_find(alder_uc_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_element_union(alder_uc_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_element_copy(alder_uc_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_element_destroy(alder_uc_t *uc)
{
    return ALDER_UC_SUCCESS;
}