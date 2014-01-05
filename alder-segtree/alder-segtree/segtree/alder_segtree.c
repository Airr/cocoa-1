/**
 * This file, alder_segtree.c, is part of alder-segtree.
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
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "alder_cmacro.h"
#include "alder_dictionary.h"
#include "alder_segtree.h"
#include "alder_node_function_void_source.h"
#include "alder_node_function_tuple2_source.h"

#define ALDER_BASE_SEGTREE_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_SEGTREE_NODE

#define ALDER_BASE_ITEM_TUPLE2_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_ITEM_TUPLE2_NODE

#pragma mark compare

/* 
 * Compare (x,?) vs interval.
 */
static int
compare_contain_begin_openpoint (const int64_t x, const void *c)
{
    const alder_item_tuple2_node_t *a = c;
    if (a->e1 == a->e2) {
        if (x < a->e1) {
            return -1;
        } else if (x > a->e1) {
            return +1;
        } else {
            assert(0);
            return 0;
        }
    } else {
        assert(a->e1 < a->e2);
        if (x < a->e1) {
            return -1;
        } else if (x > a->e1) {
            return +1;
        } else {
            return 0;
        }
    }
}
/* 
 * Compare (?,x) vs interval.
 */
static int
compare_contain_end_openpoint (const int64_t x, const void *c)
{
    const alder_item_tuple2_node_t *a = c;
    if (a->e1 == a->e2) {
        if (x < a->e1) {
            return -1;
        } else if (x > a->e1) {
            return +1;
        } else {
            assert(0);
            return 0;
        }
    } else {
        assert(a->e1 < a->e2);
        if (x < a->e2) {
            return -1;
        } else if (x > a->e2) {
            return +1;
        } else {
            return 0;
        }
    }
}
    
static int
compare_contain_point_in_tuple2(const int64_t x, const void *c)
{
    const alder_item_tuple2_node_t *a = c;
    if (a->e1 == a->e2) {
        if (x < a->e1) {
            return -1;
        } else if (x > a->e1) {
            return +1;
        } else {
            return 0;
        }
    } else {
        assert(a->e1 < a->e2);
        if (x <= a->e1) {
            return -1;
        } else if (x >= a->e2) {
            return +1;
        } else {
            return 0;
        }
    }
}

static void
rbtree_down(alder_segtree_t *o, alder_segtree_node_t * n)
{
    alder_segtree_node_t * nprime2 = NULL;
    alder_segtree_node_t * nprime = get_node_segtree(o->block);
    alder_segtree_node_t * lchild = n->left;
    alder_segtree_node_t * rchild = n->right;
    
    alder_uc_set_create(o->uc, nprime);
    alder_uc_set_copy(o->uc, n, nprime);
    alder_uc_set_union(o->uc, lchild, nprime);
    alder_uc_set_union(o->uc, rchild, n);
    alder_uc_set_destroy(o->uc, nprime, (void**)&nprime2);
    
    assert(nprime == nprime2);
    return_node_segtree(o->block, nprime);
}

static void
left_rotation (alder_segtree_t *o, alder_segtree_node_t * n)
{
    // DOWN(n)
    // DOWN(rchild(n))
    rbtree_down(o, n);
    rbtree_down(o, n->right);
    alder_segtree_node_t *tmp_node;
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
right_rotation (alder_segtree_t *o, alder_segtree_node_t * n)
{
    // DOWN(n)
    // DOWN(lchild(n))
    rbtree_down(o, n);
    rbtree_down(o, n->left);
    alder_segtree_node_t *tmp_node;
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

/* NOTE: The order of c1 and c2 does matter.
 * c1: input key
 * c2: node key
 */
static int
compare_tuple2_for_delta(const void *c1, const void *c2)
{
    const alder_item_tuple2_node_t *a1 = c1;
    const alder_item_tuple2_node_t *a2 = c2;
    if (a2->e1 == a2->e2) {
        if (a1->e2 < a2->e1) {
            return -1;
        } else if (a1->e1 > a2->e2) {
            return +1;
        } else {
            return 0;
        }
    } else {
        if (a1->e2 <= a2->e1) {
            return -1;
        } else if (a1->e1 >= a2->e2) {
            return +1;
        } else {
            return 0;
        }
    }
}

#pragma mark rbtree

/* This function finds the leaf whose associated interval contains a point x.
 * returns
 * a leaf in the tree on success.
 * NULL if it failed to find such a leaf.
 */
static
void *rbtree_find_leaf_containing_point (alder_segtree_t *o, int64_t x)
{
    alder_segtree_node_t *tree = o->tree;
    alder_segtree_node_t *tmp_node = NULL;
    if (tree->left == NULL) {
        return NULL;
    }
    
    tmp_node = tree;
    while (tmp_node->right != NULL)
    {
        if (compare_contain_point_in_tuple2(x,tmp_node->key) < 0) {
            tmp_node = tmp_node->left;
        } else {
            tmp_node = tmp_node->right;
        }
    }
    if (compare_contain_point_in_tuple2(x,tmp_node->key) == 0) {
        return tmp_node->left;
    } else {
        return NULL;
    }
}

/* This function finds the node with the key.
 * returns
 * a leaf with the key in the tree on success,
 * NULL if it failed to find such a leaf.
 */
static
void *rbtree_find (alder_segtree_t *o, void *key)
{
    alder_segtree_node_t *tree = o->tree;
    alder_segtree_node_t *tmp_node = NULL;
    if (tree->left == NULL) {
        return NULL;
    }
    
    tmp_node = tree;
    while (tmp_node->right != NULL)
    {
        if (compare_tuple2(key,tmp_node->key) < 0) {
            tmp_node = tmp_node->left;
        } else {
            tmp_node = tmp_node->right;
        }
    }
    if (compare_tuple2(key,tmp_node->key) == 0) {
        return tmp_node->left;
    } else {
        return NULL;
    }
}

/* This function finds an internal node that is the least common ancestor.
 */
static
int rbtree_find_delta (alder_segtree_t *o, void *key)
{
    int s = ALDER_SEGTREE_SUCCESS;
    alder_segtree_node_t *tree = o->tree;
    alder_segtree_node_t *tmp_node = NULL;
    if (tree->left == NULL) {
        return s;
    }
    
    tmp_node = tree;
    while (tmp_node->right != NULL)
    {
        int c = compare_tuple2_for_delta(key,tmp_node->key);
        if (c < 0) {
            tmp_node = tmp_node->left;
        } else if (c > 0) {
            tmp_node = tmp_node->right;
        } else {
            break;
        }
    }
    
    alder_item_tuple2_node_t *intv = key;
    int64_t x1 = intv->e1;
    int64_t x2 = intv->e2;
    // Follow the left child.
    alder_segtree_node_t *child = tmp_node->left;
    do {
        int c = compare_contain_point_in_tuple2(x1, child);
        if (c < 0) {
            s = alder_uc_set_insert(o->uc, child->right, key);
            ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
            child = child->left;
        } else {
            child = child->right;
        }
    } while (child->right != NULL);
    
    // Follow the right child.
    child = tmp_node->right;
    do {
        int c = compare_contain_point_in_tuple2(x2, child);
        if (c < 0) {
            child = child->left;
        } else {
            s = alder_uc_set_insert(o->uc, child->left, key);
            ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
            child = child->right;
        }
    } while (child->right != NULL);
    
    return s;
}

/* This function inserts a key-value to the red-black tree.
 * returns
 * ALDER_SEGTREE_SUCCESS on success,
 * ALDER_SEGTREE_ERROR_DUPLICATE if the key already exists.
 */
static
int rbtree_insert(alder_segtree_t *o, void *key)
{
    alder_segtree_node_t *tree = o->tree;
    
    alder_segtree_node_t *current_node = NULL;
    int finished = 0;
    if (tree->left == NULL)
    {
        tree->key = key;
        tree->color = black;      /* root is always black */
        tree->right = NULL;
    }
    else
    {
        alder_segtree_node_t *path_stack[100];
        int path_st_p = 0;
        current_node = tree;
        while (current_node->right != NULL)
        {
            path_stack[path_st_p++] = current_node;
            if (compare_tuple2(key,current_node->key) < 0)
                current_node = current_node->left;
            else
                current_node = current_node->right;
        }
        /* found the candidate leaf. Test whether key distinct */
        if (compare_tuple2(current_node->key,key) == 0)
            return ALDER_SEGTREE_ERROR_DUPLICATE;
        /* key is distinct, now perform the insert */
        {
            alder_segtree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node_segtree(o->block);
            old_leaf->left = current_node->left;
            old_leaf->key = current_node->key;
            old_leaf->right = NULL;
            old_leaf->color = red;
            new_leaf = get_node_segtree(o->block);
            new_leaf->key = key;
            new_leaf->right = NULL;
            new_leaf->color = red;
            if (compare_tuple2(current_node->key,key) < 0)
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
            alder_segtree_node_t *upper_node, *other_node;
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
                        right_rotation (o, upper_node);
                        upper_node->right->color = red;
                        upper_node->color = black;
                        finished = 1;
                    }
                    else          /* current_node->right->color == red */
                    {
                        left_rotation (o, current_node);
                        right_rotation (o, upper_node);
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
                        left_rotation (o, upper_node);
                        upper_node->left->color = red;
                        upper_node->color = black;
                        finished = 1;
                    }
                    else          /* current_node->left->color == red */
                    {
                        right_rotation (o, current_node);
                        left_rotation (o, upper_node);
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
    return ALDER_SEGTREE_SUCCESS;
}

/* This function add a new endpoint in a segment tree.
 * returns
 * ALDER_SEGTREE_SUCCESS on success,
 * ALDER_SEGTREE_ERROR_MEMORY if not enough memory,
 */
static
int new_endpoint(alder_segtree_t *o, int64_t x, alder_item_tuple2_node_t *xx)
{
    int s = ALDER_SEGTREE_SUCCESS;
    // gamma <- the leaf containing x in the tree.
    alder_segtree_node_t *S_gamma = rbtree_find_leaf_containing_point(o, x);
    assert(S_gamma != NULL);
    alder_item_tuple2_node_t *intv_gamma = S_gamma->key;
    assert(S_gamma->key != NULL);
    if (intv_gamma->e1 != intv_gamma->e2) {
        assert(intv_gamma->e1 < intv_gamma->e2);
        int64_t a = intv_gamma->e1;
        int64_t b = intv_gamma->e2;
        // int(gamma) <- [x:x]
        intv_gamma->e1 = x;
        intv_gamma->e2 = x;
        // add (a:x) as a leaf gamma' to the tree.
        alder_item_tuple2_node_t *intv_ax = get_node_item_tuple2(o->bintv);
        ALDER_RETURN_ERROR_IF_NULL(intv_ax, ALDER_SEGTREE_ERROR_MEMORY);
        intv_ax->e1 = a; intv_ax->e2 = x;
        rbtree_insert(o, (void*)intv_ax);
        // add (x:b) as a leaf gamma'' to the tree.
        alder_item_tuple2_node_t *intv_xb = get_node_item_tuple2(o->bintv);
        ALDER_RETURN_ERROR_IF_NULL(intv_xb, ALDER_SEGTREE_ERROR_MEMORY);
        intv_xb->e1 = x; intv_xb->e2 = b;
        rbtree_insert(o, (void*)intv_xb);
        
        // SET-CREATE(S_gamma')
        // SET-CREATE(S_gamma'')
        alder_segtree_node_t *S_gamma1 = rbtree_find(o, intv_ax);
        alder_segtree_node_t *S_gamma2 = rbtree_find(o, intv_xb);
        alder_uc_set_create(o->uc, S_gamma1);
        alder_uc_set_create(o->uc, S_gamma2);
        // SET-COPY(S_gamma,S_gamma')
        // SET-COPY(S_gamma,S_gamma'')
        alder_uc_set_copy(o->uc, S_gamma, S_gamma1);
        alder_uc_set_copy(o->uc, S_gamma, S_gamma2);
        // m(gamma) <- 0
        S_gamma->m = 0;
    }
    // SET-INSERT(S_gamma,[x1:x2])
    // m(gamma) <- m(gamma) + 1
    alder_uc_set_insert(o->uc, S_gamma, xx);
    return s;
}


/* This function deletes a node with the given key.
 * returns
 * value on success
 */
static
void *rbtree_delete(alder_segtree_t *o, void *delete_key)
{
    alder_segtree_node_t *tree = o->tree;
    alder_segtree_node_t *tmp_node, *upper, *other;
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
        alder_segtree_node_t *path_stack[100];
        int path_st_p = 0;
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            path_stack[path_st_p++] = tmp_node;
            upper = tmp_node;
            if (compare_tuple2(delete_key,tmp_node->key) < 0)
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
        if (compare_tuple2(tmp_node->key,delete_key) != 0)
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
            return_node_segtree(o->block, tmp_node);
            return_node_segtree(o->block, other);
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
                                left_rotation (o, upper);
                                upper->left->color = red;
                                upper->color = black;
                            }
                            else
                            {
                                right_rotation (o, other);
                                left_rotation (o, upper);
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
                                left_rotation (o, upper);
                                left_rotation (o, upper->left);
                                upper->left->left->color = red;
                                upper->left->color = black;
                                upper->color = black;
                                finished = 1;
                            }
                            else
                            {
                                left_rotation (o, upper);
                                right_rotation (o, upper->left->right);
                                left_rotation (o, upper->left);
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
                            left_rotation (o, upper);
                            upper->left->color = red;
                            upper->color = black;
                            finished = 1;
                        }
                        else
                        {
                            right_rotation (o, other);
                            left_rotation (o, upper);
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
                                right_rotation (o, upper);
                                upper->right->color = red;
                                upper->color = black;
                            }
                            else
                            {
                                left_rotation (o, other);
                                right_rotation (o, upper);
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
                                right_rotation (o, upper);
                                right_rotation (o, upper->right);
                                upper->right->right->color = red;
                                upper->right->color = black;
                                upper->color = black;
                                finished = 1;
                            }
                            else
                            {
                                right_rotation (o, upper);
                                left_rotation (o, upper->right->left);
                                right_rotation (o, upper->right);
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
                            right_rotation (o, upper);
                            upper->right->color = red;
                            upper->color = black;
                            finished = 1;
                        }
                        else
                        {
                            left_rotation (o, other);
                            right_rotation (o, upper);
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

/* This function finds a leaf [x:x] with interval (x,?).
 */
static
void *rbtree_find_leaf_to_right(alder_segtree_t *o, int64_t x)
{
    alder_segtree_node_t *tree = o->tree;
    alder_segtree_node_t *tmp_node = NULL;
    if (tree->left == NULL) {
        return NULL;
    }
    
    tmp_node = tree;
    while (tmp_node->right != NULL)
    {
        if (compare_contain_begin_openpoint(x,tmp_node->key) < 0) {
            tmp_node = tmp_node->left;
        } else {
            tmp_node = tmp_node->right;
        }
    }
    if (compare_contain_begin_openpoint(x,tmp_node->key) == 0) {
        return tmp_node;
    } else {
        return NULL;
    }
}

/* This function finds a leaf [x:x] with interval (?,x).
 */
static
void *rbtree_find_leaf_to_left(alder_segtree_t *o, int64_t x)
{
    alder_segtree_node_t *tree = o->tree;
    alder_segtree_node_t *tmp_node = NULL;
    if (tree->left == NULL) {
        return NULL;
    }
    
    tmp_node = tree;
    while (tmp_node->right != NULL)
    {
        if (compare_contain_end_openpoint(x,tmp_node->key) < 0) {
            tmp_node = tmp_node->left;
        } else {
            tmp_node = tmp_node->right;
        }
    }
    if (compare_contain_end_openpoint(x,tmp_node->key) == 0) {
        return tmp_node;
    } else {
        return NULL;
    }
}

static
int remove_endpoint(alder_segtree_t *o, int64_t x)
{
    int s = ALDER_SEGTREE_SUCCESS;
    alder_segtree_node_t *S_gamma = rbtree_find_leaf_containing_point(o, x);
    S_gamma->m--;
    if (S_gamma->m == 0) {
        // Find the left and right leaves of the node
        alder_segtree_node_t *S_left = rbtree_find_leaf_to_left(o, x);
        assert(S_left != NULL);
        alder_segtree_node_t *S_right = rbtree_find_leaf_to_right(o, x);
        assert(S_right != NULL);
        // left: (a,x)
        // right: (x,b)
        alder_item_tuple2_node_t *intv_left = S_left->key;
        alder_item_tuple2_node_t *intv_right = S_right->key;
        // int(gamma) <- (a,b)
        alder_item_tuple2_node_t *intv = S_gamma->key;
        intv->e1 = intv_left->e1;
        intv->e2 = intv_right->e2;
        // Delete the left leaf.
        rbtree_delete(o, intv_left);
        // Delete the right leaf.
        rbtree_delete(o, intv_right);
    }
    return s;
}


#pragma mark segtree

/* This function creates a segment tree.
 * returns
 * alder_segtree_t on success,
 * NULL if it failed to create a alder_segtree_t.
 */
alder_segtree_t *alder_segtree_create()
{
    alder_segtree_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->block = create_node_segtree();
    o->bintv = create_node_item_tuple2();
    o->uc = alder_uc_create(compare_void, compare_tuple2);
    o->dictionary = alder_dictionary_create(compare_tuple2, print_tuple2);
    if (o->block == NULL || o->bintv == NULL ||
        o->uc == NULL || o->dictionary == NULL) {
        alder_segtree_destroy(o);
        return NULL;
    }
    
    alder_segtree_node_t *node = get_node_segtree(o->block);
    if (node == NULL) {
        alder_segtree_destroy(o);
        return NULL;
    }
    node->left = NULL;
    node->right = NULL;
    node->color = black;
    o->tree = node;
    return o;
}

/* This function destroys an alder_segtree_t.
 */
void alder_segtree_destroy(alder_segtree_t *o)
{
    if (o != NULL) {
        destroy_node_segtree(o->block);
        destroy_node_item_tuple2(o->bintv);
        alder_uc_destroy(o->uc);
        alder_dictionary_destroy(o->dictionary);
        XFREE(o);
    }
}

/* This function inserts a new interval into a segment tree.
 * returns
 * ALDER_SEGTREE_SUCCESS on success,
 *
 */
int alder_segtree_insert(alder_segtree_t *o, int64_t x1, int64_t x2)
{
    int s = ALDER_SEGTREE_SUCCESS;
    alder_item_tuple2_node_t *xx = get_node_item_tuple2(o->bintv);
    ALDER_RETURN_ERROR_IF_NULL(xx, ALDER_SEGTREE_ERROR_MEMORY);
    xx->e1 = x1; xx->e2 = x2;
    s = alder_dictionary_insert(o->dictionary, xx);
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
    s = new_endpoint(o, x1, xx);
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
    s = new_endpoint(o, x2, xx);
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
    // delta <- the least common ancestor of the two added leaves.
    s = rbtree_find_delta (o, xx);
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
    return s;
}

/* This function deletes an interval in a segment tree.
 * returns
 * ALDER_SEGTREE_SUCCESS on success,
 */
int alder_segtree_delete(alder_segtree_t *o, int64_t x1, int64_t x2)
{
    int s = ALDER_SEGTREE_SUCCESS;
    alder_item_tuple2_node_t *xx = get_node_item_tuple2(o->bintv);
    ALDER_RETURN_ERROR_IF_NULL(xx, ALDER_SEGTREE_ERROR_MEMORY);
    xx->e1 = x1; xx->e2 = x2;
    
    // gamma <- the leaf in the dictionary that contains [x1:x2]
    alder_item_tuple2_node_t *gamma = alder_dictionary_find(o->dictionary, xx);
    ALDER_RETURN_ERROR_IF_NULL(gamma, ALDER_SEGTREE_ERROR_MEMORY);
    alder_item_tuple2_node_t *gamma2 = NULL;
    // ELEMENT-DESTROY(gamma)
    s = alder_uc_element_destroy(o->uc, gamma, (void**)&gamma2);
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
    // delete [x1:x2] from the dictionary.
    alder_item_tuple2_node_t *xx2 = alder_dictionary_delete(o->dictionary, xx);
    assert(xx2 != NULL);
    // REMOVE-ENDPOINT(x1)
    s = remove_endpoint(o, x1);
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
    // REMOVE-ENDPOINT(x2)
    s = remove_endpoint(o, x2);
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
    
    // FIXME: I do not know which one must be deleted?
    return_node_item_tuple2(o->bintv, xx);
    return_node_item_tuple2(o->bintv, xx2);
    return_node_item_tuple2(o->bintv, gamma);
    return_node_item_tuple2(o->bintv, gamma2);
    return s;
}








