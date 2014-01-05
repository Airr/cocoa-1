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

#pragma mark memory

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

static int alder_uc_nuf_delete(alder_uc_t *uc, alder_uc_node_t *v)
{
    int s = ALDER_UC_SUCCESS;
    alder_uc_node_t *n = NULL;
    s = alder_uf_item(uc->nuf, v, (void**)&n);
    assert(s == ALDER_UF_SUCCESS);
    return_node_uc(uc->bnode, n);
    alder_uf_delete(uc->nuf, v);
    return s;
}

static int alder_uc_ruf_delete(alder_uc_t *uc, alder_uc_node_t *v)
{
    int s = ALDER_UC_SUCCESS;
    alder_uc_node_t *n = NULL;
    s = alder_uf_item(uc->ruf, v, (void**)&n);
    assert(s == ALDER_UF_SUCCESS);
    return_node_uc(uc->bnode, n);
    alder_uf_delete(uc->ruf, v);
    return s;
}

#pragma mark uc



/* This function prepares for the usage of the union-copy structure.
 * TODO: alder_uc_create(compare_int, compare_tuple2)
 * returns a union-copy structure on success, and NULL otherwise.
 */
alder_uc_t * alder_uc_create(alder_comparison_func *compare_set,
                             alder_comparison_func *compare_element)
{
    alder_uc_t *uc = malloc(sizeof(*uc));
    if (uc == NULL) {
        uc->bnode = create_node_uc();
        uc->set = alder_dictionary_create(compare_set, NULL);
        uc->element = alder_dictionary_create(compare_element, NULL);
        uc->nuf = alder_uf_create(compare_uc, print_uc);
        uc->ruf = alder_uf_create(compare_uc, print_uc);
        
        if (uc->bnode == NULL ||
            uc->set == NULL || uc->element == NULL ||
            uc->nuf == NULL || uc->ruf == NULL) {
            alder_uc_destroy(uc);
            uc = NULL;
        }
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
        XFREE(uc);
    }
}

/* This function creates a new empty set.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory,
 * ALDER_ERROR_DUPULICATE if the set already exists.
 */
int alder_uc_set_create(alder_uc_t *uc, void *setID)
{
    int s;
    alder_uc_node_t *set = get_node_uc(uc->bnode);
    if (set == NULL) return ALDER_UC_ERROR_MEMORY;
    set->type = ALDER_UC_NODE_SET;
    set->up = NULL;
    set->down = NULL;
    set->item = setID;
    s = alder_dictionary_insert2(uc->set, setID, set);
    return s;
}

/* This function inserts a 2-tuple element to a set.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memeory,
 * ALDER_UC_ERROR_NOTEXIST unless either set or element exists,
 * ALDER_UC_ERROR_UFMAKESET if uf_makeset fails.
 */
int alder_uc_set_insert(alder_uc_t *uc, void *setID, void *elementID)
{
    int s = ALDER_UC_SUCCESS;
    alder_uc_node_t *set = alder_dictionary_find(uc->set, setID);
    alder_uc_node_t *element = alder_dictionary_find(uc->element, elementID);
    if (set == NULL || element == NULL) return ALDER_UC_ERROR_NOTEXIST;
    
    alder_uc_node_t *outS = set->down;
    alder_uc_node_t *inX = element->up;
    alder_uc_node_t *orgE = NULL;
    assert(set->up == NULL && element->down == NULL);
    assert(outS == NULL || outS->type != ALDER_UC_NODE_SET);
    if (outS == NULL) {
        orgE = set;
    } else if (outS->type == ALDER_UC_NODE_NORMAL) {
        alder_uc_node_t *normal = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(normal,ALDER_UC_ERROR_MEMORY);
        normal->type = ALDER_UC_NODE_NORMAL;
        normal->up = NULL;
        normal->down = NULL; /* orgE would change this down. */
        s = alder_uf_makeset(uc->nuf, normal);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,ALDER_UC_ERROR_UFMAKESET);
        s = alder_uf_union(uc->nuf, outS, normal);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        alder_uc_node_t *root = alder_uf_root(uc->nuf, outS);
        assert(root != NULL);
        set->down = root;
        root->up = set;
        orgE = normal;
    } else if (outS->type == ALDER_UC_NODE_REVERSE ||
               outS->type == ALDER_UC_NODE_ELEMENT) {
        alder_uc_node_t *normal = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(normal,ALDER_UC_ERROR_MEMORY);
        normal->type = ALDER_UC_NODE_NORMAL;
        normal->up = NULL;
        normal->down = outS;
        outS->up = normal;
        s = alder_uf_makeset(uc->nuf, normal);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,ALDER_UC_ERROR_UFMAKESET);
        
        alder_uc_node_t *normal2 = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(normal2,ALDER_UC_ERROR_MEMORY);
        normal2->type = ALDER_UC_NODE_NORMAL;
        normal2->up = NULL;
        normal2->down = NULL;
        s = alder_uf_makeset(uc->nuf, normal2);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,ALDER_UC_ERROR_UFMAKESET);
        s = alder_uf_union(uc->nuf, normal, normal2);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        alder_uc_node_t *root = alder_uf_root(uc->nuf, normal);
        assert(root != NULL);
        set->down = root;
        root->up = set;
        orgE = normal2;
    }
    assert(orgE != NULL);
    assert(orgE->down == NULL);

    if (inX == NULL) {
        element->up = orgE;
        orgE->down = element;
    } else if (inX->type == ALDER_UC_NODE_REVERSE) {
        alder_uc_node_t *reverse = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(reverse,ALDER_UC_ERROR_MEMORY);
        reverse->type = ALDER_UC_NODE_REVERSE;
        reverse->up = NULL; /* orgE changes this up. */
        reverse->down = NULL;
        s = alder_uf_makeset(uc->ruf, reverse);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,ALDER_UC_ERROR_UFMAKESET);
        s = alder_uf_union(uc->nuf, inX, reverse);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        alder_uc_node_t *root = alder_uf_root(uc->ruf, inX);
        assert(root != NULL);
        element->up = root;
        root->down = element;
        reverse->up = orgE;
        orgE->down = reverse;
    } else if (inX->type == ALDER_UC_NODE_NORMAL ||
               inX->type == ALDER_UC_NODE_SET) {
        alder_uc_node_t *reverse = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(reverse,ALDER_UC_ERROR_MEMORY);
        reverse->type = ALDER_UC_NODE_REVERSE;
        reverse->up = inX;
        reverse->down = NULL;
        inX->down = reverse;
        s = alder_uf_makeset(uc->ruf, reverse);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,ALDER_UC_ERROR_UFMAKESET);
        
        alder_uc_node_t *reverse2 = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(reverse2,ALDER_UC_ERROR_MEMORY);
        reverse2->type = ALDER_UC_NODE_REVERSE;
        reverse2->up = NULL; /* orgE changes this up. */
        reverse2->down = NULL;
        s = alder_uf_makeset(uc->ruf, reverse2);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,ALDER_UC_ERROR_UFMAKESET);
        s = alder_uf_union(uc->ruf, reverse, reverse2);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        alder_uc_node_t *root = alder_uf_root(uc->ruf, reverse);
        assert(root != NULL);
        element->up = root;
        root->down = element;
        reverse2->up = orgE;
        orgE->down = reverse2;
    }
    assert(orgE->down != NULL);

    return ALDER_UC_SUCCESS;
}

/* This function reports all elements in a set.
 * Use uc->list to find all of the elements in the set after a call to this
 * function. uc-list points to a set node, and list member of the set node
 * points to an element in the set. Follow the list till the end, or NULL.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memeory,
 * ALDER_UC_ERROR_NOTEXIST if no such set exists.
 */
int alder_uc_set_find(alder_uc_t *uc, void *setID)
{
    int s;
    alder_uc_node_t *set = alder_dictionary_find(uc->set, setID);
    uc->list = set;
    alder_uc_node_t *uclist = set;
    if (set == NULL) {
        return ALDER_UC_ERROR_NOTEXIST;
    }
    alder_uc_node_t *outS = set->down;
    if (outS == NULL) {
        uclist->list = NULL;
        return ALDER_UC_SUCCESS;
    }
    
    alder_stack_void_t *stack = alder_stack_create_void(BLOCKSIZE);
    alder_stack_push_void(stack, outS);
    while (!alder_stack_empty_void(stack)) {
        alder_uc_node_t *dest = alder_stack_pop_void(stack);
        assert(dest != NULL);
        assert(dest->type != ALDER_UC_NODE_SET);
        if (dest->type == ALDER_UC_NODE_ELEMENT) {
            // report the element as an answer.
            uclist->list = dest;
            uclist = uclist->list;
        } else if (dest->type == ALDER_UC_NODE_NORMAL) {
            // push elements in the normal node.
            s = alder_uf_enumerate(uc->nuf, dest);
            ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,ALDER_UC_ERROR_UFENUMERATE);
            alder_uf_node_t *t = uc->nuf->uf->list;
            while (t != NULL) {
                alder_uc_node_t *vprime = t->item;
                assert(vprime != NULL);
                assert(vprime->type == ALDER_UC_NODE_NORMAL);
                assert(vprime->down != NULL);
                alder_stack_push_void(stack, vprime->down);
                t = t->list;
            }
        } else {
            assert(dest->type == ALDER_UC_NODE_REVERSE);
            // push one element in the reverse node.
            // Find the root of the reverse uf.
            alder_uc_node_t *vprime = alder_uf_root(uc->ruf, dest);
            assert(vprime != NULL);
            assert(vprime->down != NULL);
            alder_stack_push_void(stack, vprime->down);
        }
    }
    uclist->list = NULL;

    return ALDER_UC_SUCCESS;
}

/* This function takes the union of the two sets.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory, 
 * ALDER_UC_ERROR_NOTEXIST if no such set exists,
 *
 * The two sets must be disjoint. The first set pointed by setIDi is the union
 * of the two sets. The second set pointed by setIDj will be empty.
 */
int alder_uc_set_union(alder_uc_t *uc, void *setIDi, void *setIDj)
{
    int s = ALDER_UC_SUCCESS;
    alder_uc_node_t *setI = alder_dictionary_find(uc->set, setIDi);
    alder_uc_node_t *setJ = alder_dictionary_find(uc->set, setIDj);
    ALDER_RETURN_ERROR_IF_NULL(setI, ALDER_UC_ERROR_NOTEXIST);
    ALDER_RETURN_ERROR_IF_NULL(setJ, ALDER_UC_ERROR_NOTEXIST);
    alder_uc_node_t *outI = setI->down;
    alder_uc_node_t *outJ = setJ->down;
    if (outJ == NULL) {
        // No code.
    } else if (outI == NULL) {
        // WARN: Do not switch the following two lines.
        ALDER_UC_DISCONNECT_NODE_DOWN(setJ);
        ALDER_UC_CONNECT_NODE_UP_DOWN(setI,outJ);
    } else if (outI->type == ALDER_UC_NODE_NORMAL &&
               outJ->type == ALDER_UC_NODE_NORMAL) {
        // WARN: setJ must be disconnected first.
        ALDER_UC_DISCONNECT_NODE_DOWN(setJ);
        s = alder_uf_union(uc->nuf, outI, outJ);
        alder_uc_node_t *root = alder_uf_root(uc->nuf, outI);
        assert(root != NULL);
        ALDER_UC_CONNECT_NODE_UP_DOWN(setI,root);
    } else if (outI->type == ALDER_UC_NODE_NORMAL &&
               outJ->type != ALDER_UC_NODE_NORMAL) {
        assert(outJ->type == ALDER_UC_NODE_ELEMENT ||
               outJ->type == ALDER_UC_NODE_REVERSE);
        // WARN: setJ must be disconnected first.
        ALDER_UC_DISCONNECT_NODE_DOWN(setJ);
        alder_uc_node_t *normal = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(normal, ALDER_UC_ERROR_MEMORY);
        normal->type = ALDER_UC_NODE_NORMAL;
        normal->up = NULL;
        ALDER_UC_CONNECT_NODE_UP_DOWN(normal, outJ);
        s = alder_uf_union(uc->nuf, outI, normal);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        alder_uc_node_t *root = alder_uf_root(uc->nuf, outI);
        ALDER_UC_CONNECT_NODE_UP_DOWN(setI, root);
    } else if (outJ->type == ALDER_UC_NODE_NORMAL &&
               outI->type != ALDER_UC_NODE_NORMAL) {
        assert(outI->type == ALDER_UC_NODE_ELEMENT ||
               outI->type == ALDER_UC_NODE_REVERSE);
        // WARN: setJ must be disconnected first.
        ALDER_UC_DISCONNECT_NODE_DOWN(setJ);
        alder_uc_node_t *normal = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(normal, ALDER_UC_ERROR_MEMORY);
        normal->type = ALDER_UC_NODE_NORMAL;
        normal->up = NULL;
        ALDER_UC_CONNECT_NODE_UP_DOWN(normal, outI);
        s = alder_uf_union(uc->nuf, outJ, normal);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        alder_uc_node_t *root = alder_uf_root(uc->nuf, outJ);
        ALDER_UC_CONNECT_NODE_UP_DOWN(setI, root);
    } else {
        assert(outI->type == ALDER_UC_NODE_ELEMENT ||
               outI->type == ALDER_UC_NODE_REVERSE);
        assert(outJ->type == ALDER_UC_NODE_ELEMENT ||
               outJ->type == ALDER_UC_NODE_REVERSE);
        // WARN: setJ must be disconnected first.
        ALDER_UC_DISCONNECT_NODE_DOWN(setJ);
        alder_uc_node_t *normalI = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(normalI, ALDER_UC_ERROR_MEMORY);
        normalI->type = ALDER_UC_NODE_NORMAL;
        
        alder_uc_node_t *normalJ = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(normalJ, ALDER_UC_ERROR_MEMORY);
        normalJ->type = ALDER_UC_NODE_NORMAL;
        
        normalI->up = NULL;
        ALDER_UC_CONNECT_NODE_UP_DOWN(normalI, outI);
        normalJ->up = NULL;
        ALDER_UC_CONNECT_NODE_UP_DOWN(normalJ, outJ);
        
        s = alder_uf_makeset(uc->nuf, normalI);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        s = alder_uf_makeset(uc->nuf, normalJ);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        s = alder_uf_union(uc->nuf, normalI, normalJ);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        
        alder_uc_node_t *root = alder_uf_root(uc->nuf, normalI);
        ALDER_UC_CONNECT_NODE_UP_DOWN(setI, root);
    }
    
    return ALDER_UC_SUCCESS;
}

/* This function copies Si to make Sj.
 * Sj must be empty.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory,
 * ALDER_UC_ERROR_INVALID if Sj is not empty.
 * ALDER_UC_ERROR_NOTEXIST if no such set exists,
 */
int alder_uc_set_copy(alder_uc_t *uc, void *setIDi, void *setIDj)
{
    int s = ALDER_UC_SUCCESS;
    alder_uc_node_t *setI = alder_dictionary_find(uc->set, setIDi);
    alder_uc_node_t *setJ = alder_dictionary_find(uc->set, setIDj);
    ALDER_RETURN_ERROR_IF_NULL(setI, ALDER_UC_ERROR_NOTEXIST);
    ALDER_RETURN_ERROR_IF_NULL(setJ, ALDER_UC_ERROR_NOTEXIST);
    alder_uc_node_t *outI = setI->down;
    alder_uc_node_t *outJ = setJ->down;
    
    if (outJ != NULL) {
        s = ALDER_UC_ERROR_INVALID;
    } else if (outI == NULL) {
        // ready!
    } else if (outI->type == ALDER_UC_NODE_REVERSE) {
        alder_uc_node_t *root = alder_uf_root(uc->ruf, outI);
        assert(root != NULL);
        alder_uc_node_t *root_down = root->down;
        
        alder_uc_node_t *reverseJ = get_node_uc(uc->bnode);
        reverseJ->type = ALDER_UC_NODE_REVERSE;
        reverseJ->down = NULL;
        ALDER_UC_CONNECT_NODE_UP_DOWN(setJ, reverseJ);
        s = alder_uf_makeset(uc->ruf, reverseJ);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        s = alder_uf_union(uc->ruf, outI, reverseJ);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        
        root = alder_uf_root(uc->ruf, outI);
        assert(root != NULL);
        ALDER_UC_CONNECT_NODE_UP_DOWN(root, root_down);
    } else {
        assert(outI->type == ALDER_UC_NODE_ELEMENT ||
               outI->type == ALDER_UC_NODE_NORMAL);
        alder_uc_node_t *reverseI = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(reverseI, ALDER_UC_ERROR_MEMORY);
        reverseI->type = ALDER_UC_NODE_REVERSE;
        reverseI->down = NULL;
        ALDER_UC_CONNECT_NODE_UP_DOWN(setI, reverseI);
        s = alder_uf_makeset(uc->ruf, reverseI);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        
        alder_uc_node_t *reverseJ = get_node_uc(uc->bnode);
        ALDER_RETURN_ERROR_IF_NULL(reverseJ, ALDER_UC_ERROR_MEMORY);
        reverseJ->type = ALDER_UC_NODE_REVERSE;
        reverseJ->down = NULL;
        ALDER_UC_CONNECT_NODE_UP_DOWN(setJ, reverseJ);
        s = alder_uf_makeset(uc->ruf, reverseJ);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        
        s = alder_uf_union(uc->ruf, reverseI, reverseJ);
        ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
        
        alder_uc_node_t *root = alder_uf_root(uc->ruf, reverseI);
        ALDER_UC_CONNECT_NODE_UP_DOWN(root, outI);
    }
    
    return s;
}

/* This function destroys a set with all of the elements in it.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory, 
 * ALDER_UC_ERROR_NOTEXIST if no such set exists,
 *
 * FIXME: This may be incomplete.
 */
int alder_uc_set_destroy(alder_uc_t *uc, void *setID, void **psetID)
{
    int s = ALDER_UC_SUCCESS;
    *psetID = NULL;
    alder_uc_node_t *set = alder_dictionary_find(uc->set, setID);
    ALDER_RETURN_ERROR_IF_NULL(set, ALDER_UC_ERROR_NOTEXIST);
    alder_uc_node_t *outS = set->down;
    
    // Collect nodes for Restore.
    alder_stack_void_t *stack = alder_stack_create_void(BLOCKSIZE);
    if (outS == NULL) {
        // No code.
    } else if (outS->type == ALDER_UC_NODE_ELEMENT) {
        ALDER_UC_DISCONNECT_NODE_DOWN(set);
    } else if (outS->type == ALDER_UC_NODE_REVERSE) {
        alder_stack_push_void(stack, outS);
    } else if (outS->type == ALDER_UC_NODE_NORMAL) {
        s = alder_uf_enumerate(uc->nuf, outS);
        assert(s == ALDER_UC_SUCCESS);
        alder_uf_node_t *t = uc->nuf->uf->list;
        while (t != NULL) {
            alder_uc_node_t *e= t->item;
            assert(e != NULL);
            assert(e->down != NULL);
            alder_uc_node_t *dest = e->down;
            if (dest->type == ALDER_UC_NODE_ELEMENT) {
                ALDER_UC_DISCONNECT_NODE_DOWN(e);
            } else {
                assert(dest->type == ALDER_UC_NODE_REVERSE);
                alder_stack_push_void(stack, dest);
            }
            alder_uf_node_t *t2 = t->list;
            s = alder_uc_nuf_delete(uc, (alder_uc_node_t*)t2->item);
            t = t2;
            // FIXME: how can I delete e?
            // FIXME: I cannot use this uf_delete function
            //        because I need them while enumerating elements in
            //        a uf set.
            //        Either I need to have a function to delete a uf-set,
            //        or I need an array of uf-elements for a uf-set.
            //
            // alder_uf_delete(uc->nuf, e);
        }
    }
    
    // Restore
    while (!alder_stack_empty_void(stack)) {
        alder_uc_node_t *e = alder_stack_pop_void(stack);
        assert(e != NULL);
        alder_uc_node_t *eprime = alder_uf_another(uc->ruf, e);
        assert(eprime != NULL);
        // e2 is another element in the union-find set.
        
        alder_uc_node_t *root = alder_uf_root(uc->ruf, e);
        assert(root != NULL);
        alder_uc_node_t *root_down = root->down;
        assert(root_down != NULL);
        s = alder_uc_ruf_delete(uc, e);
        assert(s == ALDER_UC_SUCCESS);
        root = alder_uf_root(uc->ruf, eprime);
        ALDER_UC_CONNECT_NODE_UP_DOWN(root, root_down);
        
        int nv = alder_uf_set_count(uc->ruf, eprime);
        assert(nv > 0);
        if (nv == 1) {
            // NOTE: eprime is the root of the reverse node.
            assert(root == eprime);
            assert(eprime->up != NULL && eprime->down != NULL);
            
            if (eprime->up->type == ALDER_UC_NODE_SET ||
                eprime->down->type == ALDER_UC_NODE_ELEMENT) {
                ALDER_UC_CONNECT_NODE_UP_DOWN(eprime->up, eprime->down);
                s = alder_uc_ruf_delete(uc, eprime);
                assert(s == ALDER_UC_SUCCESS);
            } else {
                assert(eprime->up->type == ALDER_UC_NODE_NORMAL);
                assert(eprime->down->type == ALDER_UC_NODE_NORMAL);
                root = alder_uf_root(uc->nuf, eprime->up);
                alder_uc_node_t *root_up = root->up;
                alder_uc_node_t *eprimeup2 = alder_uf_another(uc->nuf, eprime->up);
                assert(eprimeup2 != NULL);
                s = alder_uf_enumerate(uc->nuf, eprime->down);
                ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
                alder_uf_node_t *t = uc->nuf->uf->list;
                while (t != NULL) {
                    alder_uc_node_t *eprime2 = t->item;
                    alder_uf_union(uc->nuf, eprime->up, eprime2);
                    t = t->list;
                }
                // NOTE: I need to remove eprime and eprime->up.
                s = alder_uc_ruf_delete(uc, eprime->up);
                assert(s == ALDER_UC_SUCCESS);
                s = alder_uc_ruf_delete(uc, eprime);
                assert(s == ALDER_UC_SUCCESS);
                root = alder_uf_root(uc->ruf, eprimeup2);
                ALDER_UC_CONNECT_NODE_UP_DOWN(root_up, root);
            }
            // remove child(v)
            // Not needed: alder_uf_delete(uc->ruf, eprime->down);
            // I just need to delete the reverse node.
        }
    }
    alder_stack_destroy_void(stack);
    *psetID = alder_dictionary_delete(uc->set, setID);
    assert(*psetID != NULL);
    return s;
}

int alder_uc_element_create(alder_uc_t *uc, void *elementID)
{
    int s = ALDER_UC_SUCCESS;
    alder_uc_node_t *element = get_node_uc(uc->bnode);
    if (element == NULL) return ALDER_UC_ERROR_MEMORY;
    element->type = ALDER_UC_NODE_ELEMENT;
    element->up = NULL;
    element->down = NULL;
    element->item = elementID;
    s = alder_dictionary_insert2(uc->element, elementID, element);
    return s;
}

int alder_uc_element_insert(alder_uc_t *uc, void *elementID, void *setID)
{
    return alder_uc_set_insert(uc, setID, elementID);
}

int alder_uc_element_find(alder_uc_t *uc, void *elementID)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_element_union(alder_uc_t *uc, void *elementIDi, void *elementIDj)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_element_copy(alder_uc_t *uc, void *elementIDi, void *elementIDj)
{
    
    return ALDER_UC_SUCCESS;
}

/* This function removes an element from all the sets that contain it.
 * returns
 * ALDER_UC_SUCCESS on success,
 * ALDER_UC_ERROR_MEMORY if not enough memory, 
 * ALDER_UC_ERROR_NOTEXIST if no such element exists,
 */
int alder_uc_element_destroy(alder_uc_t *uc, void *elementID, void **pelementID)
{
    int s = ALDER_UC_SUCCESS;
    *pelementID = NULL;
    alder_uc_node_t *element = alder_dictionary_find(uc->element, elementID);
    ALDER_RETURN_ERROR_IF_NULL(element, ALDER_UC_ERROR_NOTEXIST);
    alder_uc_node_t *outE = element->up;
    
    // Collect nodes for Restore.
    alder_stack_void_t *stack = alder_stack_create_void(BLOCKSIZE);
    if (outE == NULL) {
        // No code.
    } else if (outE->type == ALDER_UC_NODE_SET) {
        ALDER_UC_DISCONNECT_NODE_UP(element);
    } else if (outE->type == ALDER_UC_NODE_NORMAL) {
        alder_stack_push_void(stack, outE);
    } else if (outE->type == ALDER_UC_NODE_REVERSE) {
        s = alder_uf_enumerate(uc->ruf, outE);
        assert(s == ALDER_UC_SUCCESS);
        alder_uf_node_t *t = uc->ruf->uf->list;
        while (t != NULL) {
            alder_uc_node_t *e= t->item;
            assert(e != NULL);
            assert(e->up != NULL);
            alder_uc_node_t *org = e->up;
            if (org->type == ALDER_UC_NODE_SET) {
                ALDER_UC_DISCONNECT_NODE_UP(e);
            } else {
                assert(org->type == ALDER_UC_NODE_NORMAL);
                alder_stack_push_void(stack, org);
            }
            alder_uf_node_t *t2 = t->list;
            s = alder_uc_ruf_delete(uc, (alder_uc_node_t*)t2->item);
            t = t2;
            // FIXME: how can I delete e?
            // FIXME: I cannot use this uf_delete function
            //        because I need them while enumerating elements in
            //        a uf set.
            //        Either I need to have a function to delete a uf-set,
            //        or I need an array of uf-elements for a uf-set.
            //
            // alder_uf_delete(uc->nuf, e);
        }
    }
    
    // Restore
    while (!alder_stack_empty_void(stack)) {
        alder_uc_node_t *e = alder_stack_pop_void(stack);
        assert(e != NULL);
        alder_uc_node_t *eprime = alder_uf_another(uc->nuf, e);
        assert(eprime != NULL);
        // e2 is another element in the union-find set.
        
        alder_uc_node_t *root = alder_uf_root(uc->nuf, e);
        assert(root != NULL);
        alder_uc_node_t *root_up = root->up;
        assert(root_up != NULL);
        s = alder_uc_nuf_delete(uc, e);
        assert(s == ALDER_UC_SUCCESS);
        root = alder_uf_root(uc->nuf, eprime);
        ALDER_UC_CONNECT_NODE_UP_DOWN(root_up, root);
        
        int nv = alder_uf_set_count(uc->nuf, eprime);
        assert(nv > 0);
        if (nv == 1) {
            // NOTE: eprime is the root of the reverse node.
            assert(root == eprime);
            assert(eprime->up != NULL && eprime->down != NULL);
            
            if (eprime->up->type == ALDER_UC_NODE_SET ||
                eprime->down->type == ALDER_UC_NODE_ELEMENT) {
                ALDER_UC_CONNECT_NODE_UP_DOWN(eprime->up, eprime->down);
                s = alder_uc_nuf_delete(uc, eprime);
                assert(s == ALDER_UC_SUCCESS);
            } else {
                assert(eprime->up->type == ALDER_UC_NODE_REVERSE);
                assert(eprime->down->type == ALDER_UC_NODE_REVERSE);
                root = alder_uf_root(uc->ruf, eprime->down);
                alder_uc_node_t *root_down = root->down;
                alder_uc_node_t *eprimedown2 = alder_uf_another(uc->ruf, eprime->down);
                assert(eprimedown2 != NULL);
                s = alder_uf_enumerate(uc->ruf, eprime->up);
                ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
                alder_uf_node_t *t = uc->ruf->uf->list;
                while (t != NULL) {
                    alder_uc_node_t *eprime2 = t->item;
                    alder_uf_union(uc->ruf, eprime->down, eprime2);
                    t = t->list;
                }
                // NOTE: I need to remove eprime and eprime->up.
                s = alder_uc_nuf_delete(uc, eprime->down);
                assert(s == ALDER_UC_SUCCESS);
                s = alder_uc_nuf_delete(uc, eprime);
                assert(s == ALDER_UC_SUCCESS);
                root = alder_uf_root(uc->nuf, eprimedown2);
                ALDER_UC_CONNECT_NODE_UP_DOWN(root, root_down);
            }
            // remove child(v)
            // Not needed: alder_uf_delete(uc->ruf, eprime->down);
            // I just need to delete the reverse node.
        }
    }
    alder_stack_destroy_void(stack);
    *pelementID = alder_dictionary_delete(uc->element, elementID);
    assert(*pelementID != NULL);
    return s;
}





