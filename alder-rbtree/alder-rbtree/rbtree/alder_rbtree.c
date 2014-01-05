/**
 * This file, alder_rbtree.c, is part of alder-rbtree.
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

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "alder_cmacro.h"
#include "alder_rbtree.h"

#define ALDER_BASE_RBTREE_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_RBTREE_NODE

alder_rbtree_t *alder_rbtree_create(alder_comparison_func *compare)
{
    alder_rbtree_t *tree = malloc(sizeof(*tree));
    ALDER_RETURN_NULL_IF_NULL(tree);
    tree->block = create_node_rbtree();
    if (tree->block == NULL) {
        XFREE(tree);
        return NULL;
    }
    tree->compare = compare;
    
    alder_rbtree_node_t *node = get_node_rbtree(tree->block);
    if (node == NULL) {
        destroy_node_rbtree(tree->block);
        XFREE(tree);
        return NULL;
    }
    node->left = NULL;
    node->right = NULL;
    node->color = black;
    tree->tree = node;
    return tree;
}

void alder_rbtree_destroy(alder_rbtree_t *o)
{
    if (o != NULL) {
        destroy_node_rbtree(o->block);
        XFREE(o);
    }
}

static void
left_rotation (alder_rbtree_node_t * n)
{
    alder_rbtree_node_t *tmp_node;
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
right_rotation (alder_rbtree_node_t * n)
{
    alder_rbtree_node_t *tmp_node;
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

void *alder_rbtree_find (alder_rbtree_t *o, void *key)
{
    alder_rbtree_node_t *tree = o->tree;
    alder_rbtree_node_t *tmp_node = NULL;
    if (tree->left == NULL) {
        return NULL;
    }
    
    tmp_node = tree;
    while (tmp_node->right != NULL)
    {
        if ((*o->compare)(key,tmp_node->key) < 0) {
            tmp_node = tmp_node->left;
        } else {
            tmp_node = tmp_node->right;
        }
    }
    if ((*o->compare)(key,tmp_node->key) == 0) {
        return tmp_node->left;
    } else {
        return NULL;
    }
}

/* This function inserts a key-value to the red-black tree.
 * returns
 * ALDER_RBTREE_SUCCESS on success,
 * ALDER_RBTREE_ERROR_DUPLICATE if the key already exists.
 */
int alder_rbtree_insert(alder_rbtree_t *o, void *key, void *value)
{
    alder_rbtree_node_t *tree = o->tree;
    
    alder_rbtree_node_t *current_node = NULL;
    int finished = 0;
    if (tree->left == NULL)
    {
        tree->left = value;
        tree->key = key;
        tree->color = black;      /* root is always black */
        tree->right = NULL;
    }
    else
    {
        alder_rbtree_node_t *path_stack[100];
        int path_st_p = 0;
        current_node = tree;
        while (current_node->right != NULL)
        {
            path_stack[path_st_p++] = current_node;
            if ((*o->compare)(key,current_node->key) < 0)
                current_node = current_node->left;
            else
                current_node = current_node->right;
        }
        /* found the candidate leaf. Test whether key distinct */
        if ((*o->compare)(current_node->key,key) == 0)
            return ALDER_RBTREE_ERROR_DUPLICATE;
        /* key is distinct, now perform the insert */
        {
            alder_rbtree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node_rbtree(o->block);
            old_leaf->left = current_node->left;
            old_leaf->key = current_node->key;
            old_leaf->right = NULL;
            old_leaf->color = red;
            new_leaf = get_node_rbtree(o->block);
            new_leaf->left = value;
            new_leaf->key = key;
            new_leaf->right = NULL;
            new_leaf->color = red;
            if ((*o->compare)(current_node->key,key) < 0)
            {
                current_node->left = old_leaf;
                current_node->right = new_leaf;
                current_node->key = key;
            }
            else
            {
                current_node->left = new_leaf;
                current_node->right = old_leaf;
            }
        }
        /* rebalance */
        if (current_node->color == black || current_node == tree)
            finished = 1;
        /* else: current_node is upper node of red-red conflict */
        while (path_st_p > 0 && !finished)
        {
            alder_rbtree_node_t *upper_node, *other_node;
            upper_node = path_stack[--path_st_p];
            if (upper_node->left->color == upper_node->right->color)
            {   /* both red, and upper_node black */
                upper_node->left->color = black;
                upper_node->right->color = black;
                upper_node->color = red;
            }
            else /* current_node red, other_node black */
            {
                if (current_node == upper_node->left)
                {
                    other_node = upper_node->right;
                    /* other_node->color == black */
                    if (current_node->right->color == black)
                    {
                        right_rotation (upper_node);
                        upper_node->right->color = red;
                        upper_node->color = black;
                        finished = 1;
                    }
                    else          /* current_node->right->color == red */
                    {
                        left_rotation (current_node);
                        right_rotation (upper_node);
                        upper_node->right->color = black;
                        upper_node->left->color = black;
                        upper_node->color = red;
                    }
                }
                else              /* current_node == upper_node->right */
                {
                    other_node = upper_node->left;
                    /* other_node->color == black */
                    if (current_node->left->color == black)
                    {
                        left_rotation (upper_node);
                        upper_node->left->color = red;
                        upper_node->color = black;
                        finished = 1;
                    }
                    else          /* current_node->left->color == red */
                    {
                        right_rotation (current_node);
                        left_rotation (upper_node);
                        upper_node->right->color = black;
                        upper_node->left->color = black;
                        upper_node->color = red;
                    }
                }               /* end current_node left/right of upper */
                current_node = upper_node; // FIXME: maybe not needed?
            }                   /*end other_node red/black */
            if (!finished && path_st_p > 0)
            /* upper is red, conflict possibly propagates upward */
            {
                current_node = path_stack[--path_st_p];
                if (current_node->color == black)
                    finished = 1;   /* no conflict above */
                /* else current upper node of red-red conflict */
            }
        }                       /* end while loop moving back to root */
        tree->color = black;      /* root is always black */
    }
    return ALDER_RBTREE_SUCCESS;
}

/* This function deletes a node with the given key.
 * returns
 * value on success
 */
void *alder_rbtree_delete(alder_rbtree_t *o, void *delete_key)
{
    alder_rbtree_node_t *tree = o->tree;
    alder_rbtree_node_t *tmp_node, *upper, *other;
    void *deleted_object;
    int finished = 0;
    if (tree->left == NULL)
        return (NULL);
    else if (tree->right == NULL)
    {
        if (tree->key == delete_key)
        {
            deleted_object = tree->left;
            tree->left = NULL;
            return (deleted_object);
        }
        else
            return (NULL);
    }
    else
    {
        alder_rbtree_node_t *path_stack[100];
        int path_st_p = 0;
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            path_stack[path_st_p++] = tmp_node;
            upper = tmp_node;
            if ((*o->compare)(delete_key,tmp_node->key) < 0)
            {
                tmp_node = upper->left;
                other = upper->right;
            }
            else
            {
                tmp_node = upper->right;
                other = upper->left;
            }
        }
        if ((*o->compare)(tmp_node->key,delete_key) != 0)
        {
            deleted_object = NULL;
            finished = 1;
        }
        else
        {
            upper->key = other->key;
            upper->left = other->left;
            upper->right = other->right;
            if ((upper->color == black && other->color == red) ||
                (upper->color == red && other->color == black))
                finished = 1;
            /* else: both black, so we have a missing black */
            upper->color = black;
            deleted_object = tmp_node->left;
            return_node_rbtree(o->block, tmp_node);
            return_node_rbtree(o->block, other);
        }
        /*start rebalance */
        while (path_st_p > 0 && !finished)
        {
            tmp_node = path_stack[--path_st_p];
            /* tmp_node is black-deficient */
            if (tmp_node->color == red || tmp_node == tree)
            {
                tmp_node->color = black;
                finished = 1;
            }
            else                  /* black node, black-deficient, and not root */
            {
                upper = path_stack[--path_st_p];
                if (tmp_node == upper->left)
                {
                    other = upper->right;
                    if (upper->color == black)
                    {
                        if (other->color == black)
                        {
                            if (other->left->color == black)
                            {
                                left_rotation (upper);
                                upper->left->color = red;
                                upper->color = black;
                            }
                            else
                            {
                                right_rotation (other);
                                left_rotation (upper);
                                upper->left->color = black;
                                upper->right->color = black;
                                upper->color = black;
                                finished = 1;
                            }
                        }
                        else      /* other->color == red */
                        {
                            if (other->left->left->color == black)
                            {
                                left_rotation (upper);
                                left_rotation (upper->left);
                                upper->left->left->color = red;
                                upper->left->color = black;
                                upper->color = black;
                                finished = 1;
                            }
                            else
                            {
                                left_rotation (upper);
                                right_rotation (upper->left->right);
                                left_rotation (upper->left);
                                upper->left->left->color = black;
                                upper->left->right->color = black;
                                upper->left->color = red;
                                upper->color = black;
                                finished = 1;
                            }
                        }
                    }
                    else          /* upper->color == red */
                    {
                        if (other->left->color == black)
                        {
                            left_rotation (upper);
                            upper->left->color = red;
                            upper->color = black;
                            finished = 1;
                        }
                        else
                        {
                            right_rotation (other);
                            left_rotation (upper);
                            upper->left->color = black;
                            upper->right->color = black;
                            upper->color = red;
                            finished = 1;
                        }
                    }
                }
                else              /* tmp_node = upper->right */
                {
                    other = upper->left;
                    if (upper->color == black)
                    {
                        if (other->color == black)
                        {
                            if (other->right->color == black)
                            {
                                right_rotation (upper);
                                upper->right->color = red;
                                upper->color = black;
                            }
                            else
                            {
                                left_rotation (other);
                                right_rotation (upper);
                                upper->left->color = black;
                                upper->right->color = black;
                                upper->color = black;
                                finished = 1;
                            }
                        }
                        else      /* other->color == red */
                        {
                            if (other->right->right->color == black)
                            {
                                right_rotation (upper);
                                right_rotation (upper->right);
                                upper->right->right->color = red;
                                upper->right->color = black;
                                upper->color = black;
                                finished = 1;
                            }
                            else
                            {
                                right_rotation (upper);
                                left_rotation (upper->right->left);
                                right_rotation (upper->right);
                                upper->right->right->color = black;
                                upper->right->left->color = black;
                                upper->right->color = red;
                                upper->color = black;
                                finished = 1;
                            }
                        }
                    }
                    else          /* upper->color == red */
                    {
                        if (other->right->color == black)
                        {
                            right_rotation (upper);
                            upper->right->color = red;
                            upper->color = black;
                            finished = 1;
                        }
                        else
                        {
                            left_rotation (other);
                            right_rotation (upper);
                            upper->left->color = black;
                            upper->right->color = black;
                            upper->color = red;
                            finished = 1;
                        }
                    }
                }
            }
        }
        /*end rebalance */
        return (deleted_object);
    }
}
