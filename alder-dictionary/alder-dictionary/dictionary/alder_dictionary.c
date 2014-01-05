/**
 * This file, alder_dictionary.c, is part of alder-dictionary.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-dictionary is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-dictionary is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-dictionary.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include "alder_cmacro.h"
#include "alder_dictionary.h"

#define ALDER_BASE_TREE_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_TREE_NODE

#pragma mark tree

/* This function creates a dictionary.
 * returns a pointer to alder_tree_node_t. If an error occurs, it returns NULL.
 */
alder_dictionary_t *
alder_dictionary_create (alder_comparison_func *compare,
                         alder_print_func *print)
{
    alder_dictionary_t *o = malloc(sizeof(*o)); XRETURN(o);
    o->block = malloc(sizeof(*o->block));
    if (o->block == NULL) {
        free(o);
        return NULL;
    }
    o->compare = compare;
    o->print = print;
    
    o->tree = get_node_tree(o->block);
    if (o->tree == NULL) {
        destroy_node_tree(o->block);
        free(o->block);
        free(o);
        return NULL;
    }
    
    o->tree->left = NULL;
    return o;
}

/* This function destroys a dictionary.
 */
void
alder_dictionary_destroy (alder_dictionary_t *o)
{
    if (o == NULL) return;
    
    alder_tree_node_t *tree = o->tree;
    alder_tree_node_t *current_node, *tmp;
    if (tree->left == NULL) {
        return_node_tree(o->block, tree);
    }
    else
    {
        current_node = tree;
        while (current_node->right != NULL)
        {
            if (current_node->left->right == NULL)
            {
                return_node_tree(o->block, current_node->left);
                tmp = current_node->right;
                return_node_tree(o->block, current_node);
                current_node = tmp;
            }
            else
            {
                tmp = current_node->left;
                current_node->left = tmp->right;
                tmp->right = current_node;
                current_node = tmp;
            }
        }
        return_node_tree(o->block, current_node);
    }
    destroy_node_tree(o->block);
    XFREE(o->block);
    XFREE(o);
}

#if 0
static void
tree_destroyWithUF(struct alder_tree_memory *block_tree,
                   struct alder_uf_memory *block_uf,
                   alder_tree_node_t * tree)
{
    alder_tree_node_t *current_node, *tmp;
    if (tree->left == NULL) {
        return_node_tree(block_tree, tree);
    }
    else
    {
        current_node = tree;
        while (current_node->right != NULL)
        {
            if (current_node->left->right == NULL)
            {
                return_node_treeWithUF(block_tree, block_uf, current_node->left);
                tmp = current_node->right;
                return_node_tree(block_tree, current_node);
                current_node = tmp;
            }
            else
            {
                tmp = current_node->left;
                current_node->left = tmp->right;
                tmp->right = current_node;
                current_node = tmp;
            }
        }
        return_node_treeWithUF(block_tree, block_uf, current_node);
    }
}
#endif

static void
tree_left_rotation (alder_tree_node_t * n)
{
    alder_tree_node_t *tmp_node;
    void *tmp_key;
    tmp_node = n->left;
    tmp_key = n->key;
    n->left = n->right;
    n->key = n->right->key;
    n->right = n->left->right;
    n->left->right = n->left->left;
    n->left->left = tmp_node;
    n->left->key = tmp_key;
}

static void
tree_right_rotation (alder_tree_node_t * n)
{
    alder_tree_node_t *tmp_node;
    void *tmp_key;
    tmp_node = n->right;
    tmp_key = n->key;
    n->right = n->left;
    n->key = n->left->key;
    n->left = n->right->left;
    n->right->left = n->right->right;
    n->right->right = tmp_node;
    n->right->key = tmp_key;
}


/* This function finds the value associated the query key.
 * returns a pointer to void. If an error occurs, it returns NULL.
 */
void *
alder_dictionary_find (alder_dictionary_t *o, void *query_key)
{
    alder_tree_node_t * tree = o->tree;
    alder_tree_node_t *tmp_node;
    if (tree->left == NULL) {
        return NULL;
    } else {
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            if ((*o->compare)(query_key, tmp_node->key) < 0)
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
        }
        if ((*o->compare)(tmp_node->key, query_key) == 0)
            return tmp_node->left;
        else
            return NULL;
    }
}

int
alder_dictionary_insert (alder_dictionary_t *o,
                         void *new_object)
{
    return alder_dictionary_insert2 (o, new_object, new_object);
}

/* This function inserts a node to a binary search tree.
 * returns
 * ALDER_SUCCESS on success.
 * ALDER_ERROR_DUPULICATE if the item already exists in the tree.
 * ALDER_ERROR_MEMORY if not enough memory available.
 */
int
alder_dictionary_insert2 (alder_dictionary_t *o,
                          void *new_key,
                          void *new_object)
{
    alder_tree_node_t * tree = o->tree;
    alder_tree_node_t *tmp_node;
    int finished;
    if (tree->left == NULL) {
        tree->left = (alder_tree_node_t *) new_object;
        tree->key = new_key;
        tree->height = 0;
        tree->right = NULL;
    } else {
        alder_tree_node_t *path_stack[100];
        int path_st_p = 0;
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            path_stack[path_st_p++] = tmp_node;
            //            if (new_key < tmp_node->key)
            if ((*o->compare)(new_key, tmp_node->key) < 0)
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
        }
        /* found the candidate leaf. Test whether key distinct */
        //        if (tmp_node->key == new_key)
        if ((*o->compare)(tmp_node->key, new_key) == 0)
            return ALDER_DICTIONARY_ERROR_DUPLICATE;
        /* key is distinct, now perform the insert */
        {
            alder_tree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node_tree(o->block);
            if (old_leaf == NULL) {
                return ALDER_DICTIONARY_ERROR_MEMORY;
            }
            old_leaf->left = tmp_node->left;
            old_leaf->key = tmp_node->key;
            old_leaf->right = NULL;
            old_leaf->height = 0;
            new_leaf = get_node_tree(o->block);
            if (new_leaf == NULL) {
                return ALDER_DICTIONARY_ERROR_MEMORY;
            }
            new_leaf->left = (alder_tree_node_t *) new_object;
            new_leaf->key = new_key;
            new_leaf->right = NULL;
            new_leaf->height = 0;
            if ((*o->compare)(tmp_node->key, new_key) < 0) {
                tmp_node->left = old_leaf;
                tmp_node->right = new_leaf;
                tmp_node->key = new_key;
            } else {
                tmp_node->left = new_leaf;
                tmp_node->right = old_leaf;
            }
            tmp_node->height = 1;
        }
        /* rebalance */
        finished = 0;
        while (path_st_p > 0 && !finished) {
            int tmp_height, old_height;
            tmp_node = path_stack[--path_st_p];
            old_height = tmp_node->height;
            if (tmp_node->left->height - tmp_node->right->height == 2) {
                if (tmp_node->left->left->height - tmp_node->right->height == 1) {
                    tree_right_rotation (tmp_node);
                    tmp_node->right->height = tmp_node->right->left->height + 1;
                    tmp_node->height = tmp_node->right->height + 1;
                } else {
                    tree_left_rotation (tmp_node->left);
                    tree_right_rotation (tmp_node);
                    tmp_height = tmp_node->left->left->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            } else if (tmp_node->left->height - tmp_node->right->height == -2) {
                if (tmp_node->right->right->height - tmp_node->left->height == 1) {
                    tree_left_rotation (tmp_node);
                    tmp_node->left->height = tmp_node->left->right->height + 1;
                    tmp_node->height = tmp_node->left->height + 1;
                } else {
                    tree_right_rotation (tmp_node->right);
                    tree_left_rotation (tmp_node);
                    tmp_height = tmp_node->right->right->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            } else {  /* update height even if there was no rotation */
                if (tmp_node->left->height > tmp_node->right->height)
                    tmp_node->height = tmp_node->left->height + 1;
                else
                    tmp_node->height = tmp_node->right->height + 1;
            }
            if (tmp_node->height == old_height) {
                finished = 1;
            }
        }
        
    }
    return ALDER_DICTIONARY_SUCCESS;
}


void *
alder_dictionary_delete (alder_dictionary_t *o, void *delete_key)
{
    alder_tree_node_t * tree = o->tree;
    alder_tree_node_t *tmp_node, *upper_node, *other_node;
    void *deleted_object;
    int finished;
    if (tree->left == NULL)
        return (NULL);
    else if (tree->right == NULL)
    {
        //        if (tree->key == delete_key)
        if ((*o->compare)(tree->key, delete_key) == 0)
        {
            deleted_object = (void*)tree->left;
            tree->left = NULL;
            return (deleted_object);
        }
        else
            return (NULL);
    }
    else
    {
        alder_tree_node_t *path_stack[100];
        int path_st_p = 0;
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            path_stack[path_st_p++] = tmp_node;
            upper_node = tmp_node;
            //            if (delete_key < tmp_node->key)
            if ((*o->compare)(delete_key, tmp_node->key) < 0)
            {
                tmp_node = upper_node->left;
                other_node = upper_node->right;
            }
            else
            {
                tmp_node = upper_node->right;
                other_node = upper_node->left;
            }
        }
        if ((*o->compare)(tmp_node->key, delete_key) != 0)
            deleted_object = NULL;
        else
        {
            upper_node->key = other_node->key;
            upper_node->left = other_node->left;
            upper_node->right = other_node->right;
            upper_node->height = other_node->height;
            deleted_object = (void*)tmp_node->left;
            return_node_tree(o->block, tmp_node);
            return_node_tree(o->block, other_node);
            
        }
        /*start rebalance */
        finished = 0;
        path_st_p -= 1;
        while (path_st_p > 0 && !finished)
        {
            int tmp_height, old_height;
            tmp_node = path_stack[--path_st_p];
            old_height = tmp_node->height;
            if (tmp_node->left->height - tmp_node->right->height == 2)
            {
                if (tmp_node->left->left->height - tmp_node->right->height == 1)
                {
                    tree_right_rotation (tmp_node);
                    tmp_node->right->height = tmp_node->right->left->height + 1;
                    tmp_node->height = tmp_node->right->height + 1;
                }
                else
                {
                    tree_left_rotation (tmp_node->left);
                    tree_right_rotation (tmp_node);
                    tmp_height = tmp_node->left->left->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            }
            else if (tmp_node->left->height - tmp_node->right->height == -2)
            {
                if (tmp_node->right->right->height -
                    tmp_node->left->height == 1)
                {
                    tree_left_rotation (tmp_node);
                    tmp_node->left->height = tmp_node->left->right->height + 1;
                    tmp_node->height = tmp_node->left->height + 1;
                }
                else
                {
                    tree_right_rotation (tmp_node->right);
                    tree_left_rotation (tmp_node);
                    tmp_height = tmp_node->right->right->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            }
            else                  /* update height even if there was no rotation */
            {
                if (tmp_node->left->height > tmp_node->right->height)
                    tmp_node->height = tmp_node->left->height + 1;
                else
                    tmp_node->height = tmp_node->right->height + 1;
            }
            if (tmp_node->height == old_height)
                finished = 1;
        }
        /*end rebalance */
        return (deleted_object);
    }
}


void
alder_dictionary_print (alder_dictionary_t *o, FILE *fp)
{
    alder_tree_node_t *path_stack[100];
    alder_tree_node_t *tree = o->tree;
    alder_tree_node_t *node = tree;
    
    int path_st_p = 0;
    path_stack[path_st_p++] = tree;
    while (path_st_p > 0) {
        node = path_stack[--path_st_p];
        if (node->right != NULL) {
            path_stack[path_st_p++] = node->right;
            path_stack[path_st_p++] = node->left;
        } else {
            (*o->print)(fp, node->left);
            fprintf(fp, " ");
        }
        
    }
    fprintf(fp, "\n");
}
