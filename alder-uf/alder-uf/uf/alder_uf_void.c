/**
 * This file, alder_uf_void.c, is part of alder-uf.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-uf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-uf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-uf.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_uf_void.h"

#pragma mark debug

static void
alder_uf_assert()
{
    assert(0);
    abort();
}

#pragma mark memory

#define BLOCKSIZE 256

#pragma mark memory uf

#define ALDER_BASE_UF_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_UF_NODE


#define ALDER_BASE_TREE_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_TREE_NODE

#if 0
static alder_uf_node_t *
get_node_uf(struct alder_uf_memory_struct *block_uf)
{
    alder_uf_node_t *t = NULL;
    if (block_uf->free != NULL) {
        t = block_uf->free;
        block_uf->free = block_uf->free->next;
    } else {
        if (block_uf->current  == NULL || block_uf->size == 0) {
            block_uf->nhead++;
            alder_uf_node_t* *th = realloc(block_uf->head, block_uf->nhead * sizeof(*th));
            if (th == NULL) {
                return NULL;
            }
            block_uf->head = th;
            block_uf->head[block_uf->nhead-1] = malloc(BLOCKSIZE*sizeof(*block_uf->head[0]));
            assert(sizeof(alder_uf_node_t)==sizeof(*block_uf->head[0]));
            if (block_uf->head[block_uf->nhead-1] == NULL) {
                return NULL;
            }
            block_uf->current = block_uf->head[block_uf->nhead-1];
            block_uf->size = BLOCKSIZE;
        }
        t = block_uf->current++;
        block_uf->size--;
    }
    /* Initialize the allocated one. */
    return t;
}

static void
return_node_uf(struct alder_uf_memory_struct *block_uf, alder_uf_node_t *node)
{
    if (node != NULL) {
        node->next = block_uf->free;
        block_uf->free = node;
    }
}

static void destroy_node_uf(struct alder_uf_memory_struct *block_uf)
{
    if (block_uf->head != NULL) {
        for (size_t i = 0; i < block_uf->nhead; i++) {
            free(block_uf->head[i]);
        }
        free(block_uf->head);
        block_uf->head    = NULL;
        block_uf->current = NULL;
        block_uf->free    = NULL;
        block_uf->size    = 0;
        block_uf->nhead   = 0;
    }
}
#endif

#pragma mark memory tree

#if 0
static alder_tree_node_t*
get_node_tree(struct alder_tree_memory_struct *block_tree)
{
    alder_tree_node_t *t = NULL;
    if (block_tree->free != NULL) {
        t = block_tree->free;
        block_tree->free = block_tree->free->next;
    } else {
        if (block_tree->current  == NULL || block_tree->size == 0) {
            block_tree->nhead++;
            alder_tree_node_t* *th = realloc(block_tree->head, block_tree->nhead * sizeof(*th));
            if (th == NULL) {
                return NULL;
            }
            block_tree->head = th;
            block_tree->head[block_tree->nhead-1] = malloc(BLOCKSIZE*sizeof(*block_tree->head[0]));
            assert(sizeof(alder_tree_node_t)==sizeof(*block_tree->head[0]));
            if (block_tree->head[block_tree->nhead-1] == NULL) {
                return NULL;
            }
            block_tree->current = block_tree->head[block_tree->nhead-1];
            block_tree->size = BLOCKSIZE;
        }
        t = block_tree->current++;
        block_tree->size--;
    }
    /* Initialize the allocated one. */
    return t;
}

static void
return_node_tree(struct alder_tree_memory_struct *block_tree, alder_tree_node_t *node)
{
    if (node != NULL) {
        node->next = block_tree->free;
        block_tree->free = node;
    }
}


static void
destroy_node_tree(struct alder_tree_memory_struct *block_tree)
{
    if (block_tree->head != NULL) {
        for (size_t i = 0; i < block_tree->nhead; i++) {
            free(block_tree->head[i]);
        }
        free(block_tree->head);
        block_tree->head    = NULL;
        block_tree->current = NULL;
        block_tree->free    = NULL;
        block_tree->size    = 0;
        block_tree->nhead   = 0;
    }
}
#endif

/* This used to be in the just above section. */
static void
return_node_treeWithUF(struct alder_tree_memory_struct *block_tree,
                       struct alder_uf_memory_struct *block_uf,
                       alder_tree_node_t *node)
{
    assert(node->right==NULL);
    return_node_uf(block_uf, (alder_uf_node_t*)node->next);
    node->next = block_tree->free;
    block_tree->free = node;
}


#pragma mark memory stack

typedef struct alder_uf_node_stack {
    alder_uf_node_t **base;
    alder_uf_node_t  **top;
    int size;
    struct alder_uf_node_stack *prev;
} alder_uf_node_stack_t;


static alder_uf_node_stack_t *
uf_stack_create(int size)
{
    alder_uf_node_stack_t *s;
    s = malloc(sizeof(*s));
    s->base = malloc( size * sizeof(*s->base) );
    s->size = size;
    s->top = s->base;
    s->prev = NULL;
    return s;
}

static int uf_stack_empty(alder_uf_node_stack_t *s)
{
    return s->base == s->top && s->prev == NULL;
}

static void uf_stack_push(alder_uf_node_t *x, alder_uf_node_stack_t *s)
{
    if ( s->top < s->base + s->size )
    {
        *(s->top) = x;
        s->top++;
    }
    else
    {
        alder_uf_node_stack_t *new;
        new = malloc(sizeof(*new));
        new->base = s->base;
        new->top = s->top;
        new->size = s->size;
        new->prev = s->prev;
        s->prev = new;
        s->base = malloc(s->size * sizeof(*s->base));
        s->top = s->base+1;
        *(s->base) = x;
    }
}

static alder_uf_node_t* uf_stack_pop(alder_uf_node_stack_t *s)
{
    if( s->top == s->base )
    {
        alder_uf_node_stack_t *old;
        old = s->prev;
        s->prev = old->prev;
        free(s->base);
        s->base = old->base;
        s->top = old->top;
        s->size = old->size;
        free(old);
    }
    s->top -= 1;
    return *(s->top);
}

#if 0
static alder_uf_node_t* uf_stack_top(alder_uf_node_stack_t *s)
{
    if (s->top == s->base)
        return *(s->prev->top - 1);
    else
        return *(s->top - 1);
}
#endif

static void uf_stack_destroy(alder_uf_node_stack_t *s)
{
    alder_uf_node_stack_t *tmp;
    do
    {
        tmp = s->prev;
        free(s->base);
        free(s);
        s = tmp;
    }
    while (s != NULL);
}

#pragma mark uf

static alder_uf_node_t *
uf_insert (struct alder_uf_memory_struct *block_uf, void *new_item)
{
    alder_uf_node_t *new_node;
    new_node = get_node_uf(block_uf);
    if (new_node == NULL) return NULL;
    new_node->item = new_item;
    new_node->rank = 1;
    new_node->degree = 0;
    new_node->up = NULL;
    new_node->down = NULL;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

static alder_uf_node_t *
uf_create (struct alder_uf_memory_struct *block_uf, void *new_item)
{
    alder_uf_node_t *new_node = uf_insert(block_uf, new_item);
    new_node->rank = 0;
    return new_node;
}

/* This function finds the roots of two nodes in the union-find structure,
 * and tests whether they points to the same root ine union-find structure.
 * returns
 * 1 or true if the two nodes are led to the same root.
 * 0 or false if the two nodes are led to different roots.
 */
static int
uf_same (alder_uf_node_t * node1, alder_uf_node_t * node2)
{
    alder_uf_node_t *root1;
    alder_uf_node_t *root2;
    /* find both roots */
    for (root1 = node1; root1->up->rank > 0; root1 = root1->up);
    for (root2 = node2; root2->up->rank > 0; root2 = root2->up);
    
    /* return result */
    return (root1 == root2);
}

/* This function deletes a node in the union-find structure.
 */
static void
uf_delete (alder_uf_node_t * node1)
{
    alder_uf_node_t *rootUF;
    alder_uf_node_t *root1;
    alder_uf_node_t *t1;
    alder_uf_node_t *pnode1;
    alder_uf_node_t *down1;
    
    /* Two cases: 1. no down, and 2. down. */
    for (root1 = node1; root1->up->rank > 0; root1 = root1->up);
    rootUF = root1->up;
    pnode1 = node1->up;
    down1 = node1->down;
    if (down1 == NULL) {
        /* Detach the node. */
        t1 = node1;
        //        if (t1->left != NULL && pnode1->down == t1) pnode1->down = t1->left;
        if (pnode1->down == t1) pnode1->down = t1->left;
        if (t1->left != NULL) t1->left->right = t1->right;
        if (t1->right != NULL) t1->right->left = t1->left;
        pnode1->degree--;
    } else if (pnode1 == rootUF) {
        t1 = down1->left;
        down1->left = node1->left;
        down1->right = node1->right;
        if (node1->left != NULL) node1->left->right = down1;
        if (node1->right != NULL) node1->right->left = down1;
        down1->up = rootUF;
        if (rootUF->down == node1) rootUF->down = down1;
        alder_uf_node_t *t11 = down1->down;
        if (t11 == NULL) {
            down1->down = t1;
            if (t1 != NULL) {
                t1->right = NULL;
            }
        } else {
            while (t11->left != NULL) {
                t11 = t11->left;
            }
            t11->left = t1;
            if (t1 != NULL) {
                t1->right = t11;
            }
        }
        while (t1 != NULL) {
            t1->up = down1;
            t1 = t1->left;
        }
        down1->degree += node1->degree - 1;
        pnode1 = down1;
    } else {
        t1 = down1;
        t1->right = node1->right;
        if (node1->right != NULL) node1->right->left = t1;
        if (pnode1->down == node1) pnode1->down = t1;
        while (t1->left != NULL) {
            t1->up = pnode1;
            t1 = t1->left;
        }
        t1->up = pnode1;
        t1->left = node1->left;
        if (node1->left != NULL) node1->left->right = t1;
        pnode1->degree += node1->degree - 1;
    }
    
    while (pnode1 != rootUF) {
        /* Check the rank. */
        if (pnode1->degree == 0) {
            assert(pnode1->down == NULL);
        } else {
            assert(pnode1->down != NULL);
            t1 = pnode1->down;
            int new_rank = 0;
            while (t1 != NULL) {
                if (new_rank < t1->rank) new_rank = t1->rank;
                t1 = t1->left;
            }
            if (pnode1->rank != new_rank + 1) {
                pnode1->rank = new_rank + 1;
            } else {
                break;
            }
        }
        pnode1 = pnode1->up;
    }
}

/* This function takes the union of two nodes in the union-find structure.
 * returns
 * ALDER_ERROR_DUPLICATE if the two nodes are the same or are led to the same
 *   root in the union-find structure.
 */
static int
uf_join (alder_uf_node_t * node1, alder_uf_node_t * node2)
{
    alder_uf_node_t *rootUF;
    alder_uf_node_t *root1;
    alder_uf_node_t *root2;
    alder_uf_node_t *t1;
    alder_uf_node_t *t2;
    if (node1 == node2) return ALDER_ERROR_DUPLICATE;
    
    /* find both roots */
    for (root1 = node1; root1->up->rank > 0; root1 = root1->up);
    for (root2 = node2; root2->up->rank > 0; root2 = root2->up);
    if (root1 == root2) return ALDER_ERROR_DUPLICATE;
    rootUF = root1->up;
    assert(root2->up == rootUF);
    
    /* now root1 is the larger subtree */
    if (root1->rank < root2->rank) {
        t1 = root1;
        root1 = root2;
        root2 = t1;
    }
    if (root1->rank == 1) {
        assert(root1->degree == 0);
        assert(root1->rank == 1);
        assert(root1->down == NULL);
        assert(root2->degree == 0);
        assert(root2->rank == 1);
        assert(root2->down == NULL);
        /* Detach root2. */
        t2 = root2;
        if (rootUF->down == t2) assert(t2->left != NULL);
        if (rootUF->down == t2) rootUF->down = t2->left;
        if (t2->left != NULL) t2->left->right = t2->right;
        if (t2->right != NULL) t2->right->left = t2->left;
        root1->down = root2;
        root2->up = root1;
        root2->right = NULL;
        root2->left = NULL;
        assert(root2->down == NULL);
        root1->rank = 2;
        root1->degree = 1;
    } else if (root1->rank == 2) {
        assert(root1->degree > 0);
        assert(root1->down != NULL);
        if (root2->rank == 1) {
            assert(root2->degree == 0);
            assert(root2->down == NULL);
            /* Detach root1. */
            t1 = root1;
            if (rootUF->down == t1) assert(t1->left != NULL);
            if (rootUF->down == t1) rootUF->down = t1->left;
            if (t1->left != NULL) t1->left->right = t1->right;
            if (t1->right != NULL) t1->right->left = t1->left;
            root2->down = root1;
            root1->up = root2;
            root1->right = NULL;
            root1->left = NULL;
            assert(root1->down != NULL);
        } else {
            assert(root2->rank == 2);
            assert(root2->degree > 0);
            assert(root2->down != NULL);
            /* Detach root1. */
            t1 = root1;
            if (rootUF->down == t1) assert(t1->left != NULL);
            if (rootUF->down == t1) rootUF->down = t1->left;
            if (t1->left != NULL) t1->left->right = t1->right;
            if (t1->right != NULL) t1->right->left = t1->left;
            /* Append root1 to root2's children. */
            t2 = root2->down;     /* (1) */
            root1->up = root2;    /* (2) */
            root2->down = root1;  /* (3) */
            t2->right = root1;    /* (4) */
            root1->left = t2;     /* (5) */
            root1->right = NULL;  /* (6) */
        }
        root2->rank = 3;
        root2->degree++;
        /* root1's rank/degree no change. */
    } else if (root1->rank >= 3) {
        if (root2->rank < root1->rank)
        {
            if (root2->rank == 1) {
                assert(root2->degree == 0);
                assert(root2->down == NULL);
                /* Detach root2. */
                t2 = root2;
                if (rootUF->down == t2) assert(t2->left != NULL);
                if (rootUF->down == t2) rootUF->down = t2->left;
                if (t2->left != NULL) t2->left->right = t2->right;
                if (t2->right != NULL) t2->right->left = t2->left;
                assert(root1->down != NULL);
                t1 = root1->down;           /* (1) */
                alder_uf_node_t *t11 = t1->down;  /* (2) */
                root2->up = t1;             /* (3) */
                t1->down = root2;           /* (4) */
                root2->left = t11;          /* (5) */
                root2->right = NULL;        /* (6) */
                if (t11 != NULL) {
                    t11->right = root2;     /* (7) */
                } else {
                    assert(t1->rank == 1);
                    t1->rank = 2;
                }
                t1->degree++;
            } else {
                assert(root2->rank >= 2);
                /* Detach root2. */
                t2 = root2;
                if (rootUF->down == t2) assert(t2->left != NULL);
                if (rootUF->down == t2) rootUF->down = t2->left;
                if (t2->left != NULL) t2->left->right = t2->right;
                if (t2->right != NULL) t2->right->left = t2->left;
                assert(root1->down != NULL);
                t1 = root1->down;           /* (1) */
                alder_uf_node_t *t11 = t1->down;  /* (2) */
                root2->up = t1;             /* (3) */
                t2 = root2->down;           /* (4) */
                t2->right = root2;          /* (5) */
                root2->left = t2;           /* (6) */
                root2->right = NULL;        /* (7) */
                root2->down = NULL;         /* (8) */
                while (t2->left != NULL) {
                    t2->up = t1;            /* (9) */
                    t2 = t2->left;          /* (10)*/
                }
                t2->up = t1;                /* (11)*/
                t1->down = root2;           /* (12)*/
                t2->left = t11;             /* (13)*/
                if (t11 != NULL) {
                    t11->right = t2;        /* (14)*/
                } else {
                    assert(t1->rank == 1);
                    t1->rank = 2;
                }
                t1->degree += root2->degree + 1;
                root2->rank = 1;
                root2->degree = 0;
            }
        }
        else
        {
            assert(root1->rank == root2->rank);
            /* Detach root2. */
            t2 = root2;
            if (rootUF->down == t2) assert(t2->left != NULL);
            if (rootUF->down == t2) rootUF->down = t2->left;
            if (t2->left != NULL) t2->left->right = t2->right;
            if (t2->right != NULL) t2->right->left = t2->left;
            
            /* join root2 list to root1 list, pointing to root1 */
            t2 = root2->down;
            while (t2->left != NULL)
            {
                t2->up = root1;
                t2 = t2->left;  /* move to end of root2 list */
            }
            t2->up = root1;
            t1 = root1->down;
            t1->right = t2;
            t2->left = t1;
            root1->down = root2->down;
            
            /* now lists joined together below root 1 */
            if (root1->degree < root1->rank) {
                /* Make it wider. */
                root2->up = root1->down;     /* (1) */
                t1 = root1->down->down;      /* (2) */
                root2->left = t1;            /* (3) */
                root2->right = NULL;         /* (4) */
                if (t1 != NULL) {
                    t1->right = root2;       /* (5) */
                } else {
                    root1->down->rank = 2;
                }
                root2->up->down = root2;     /* (6) */
                root2->down = NULL;          /* (7) */
                
                root2->rank = 1;
                root1->degree += root2->degree;
                root2->degree = 0;
                root1->down->degree++;
            }
            else                  /* root2 becomes new root, root1 goes below */
            {
                /* Make it taller. */
                /* Replace root1 with root2. */
                t1 = root1;
                root2->left = root1->left;
                root2->right = root1->right;
                root2->up = rootUF;
                if (rootUF->down == root1) rootUF->down = root2;
                if (root1->right != NULL) root1->right->left = root2;
                if (root1->left != NULL) root1->left->right = root2;
                
                root1->up = root2;           /* (1) */
                root1->right = NULL;
                root1->left = NULL;
                root2->down = root1;         /* (2) */
                
                root2->rank = root1->rank + 1;
                root1->degree += root2->degree;
                root2->degree = 1;
            }
        }
    }
    rootUF->degree--;
    return ALDER_SUCCESS;
}

#pragma mark tree

static alder_tree_node_t *
tree_create (struct alder_tree_memory_struct *block_tree)
{
    alder_tree_node_t *t = get_node_tree(block_tree);
    if (t == NULL) return NULL;
    t->left = NULL;
    return t;
}

#if 0
static void
tree_destroy (alder_tree_node_t * tree)
{
    alder_tree_node_t *current_node, *tmp;
    if (tree->left == NULL) {
        return_node_tree(tree);
    }
    else
    {
        current_node = tree;
        while (current_node->right != NULL)
        {
            if (current_node->left->right == NULL)
            {
                return_node_tree(current_node->left);
                tmp = current_node->right;
                return_node_tree(current_node);
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
        return_node_tree(current_node);
    }
}
#endif

static void
tree_destroyWithUF(struct alder_tree_memory_struct *block_tree,
                   struct alder_uf_memory_struct *block_uf,
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


/* This functions search the binary tree for a value node with the query key.
 * returns the value leaf node if a node is found, and nil otherwise.
 */
static void*
tree_find (alder_tree_node_t * tree, void *query_key, alder_comparison_func *rb_compare)
{
    alder_tree_node_t *tmp_node;
    if (tree->left == NULL) {
        return NULL;
    } else {
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            //            if (query_key < tmp_node->key)
            if ((*rb_compare)(query_key, tmp_node->key) < 0)
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
        }
        //        if (tmp_node->key == query_key)
        if ((*rb_compare)(tmp_node->key, query_key) == 0)
            return tmp_node->left;
        else
            return NULL;
    }
}


/* This function inserts a node to a binary search tree.
 * returns
 * ALDER_SUCCESS on success.
 * ALDER_ERROR_DUPULICATE if the item already exists in the tree.
 * ALDER_ERROR_MEMORY if not enough memory available.
 */
static int
tree_insert (alder_tree_node_t * tree, void *new_key,
             alder_uf_node_t *new_object,
             alder_comparison_func *rb_compare,
             struct alder_tree_memory_struct *block_tree)
{
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
            if ((*rb_compare)(new_key, tmp_node->key) < 0)
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
        }
        /* found the candidate leaf. Test whether key distinct */
        //        if (tmp_node->key == new_key)
        if ((*rb_compare)(tmp_node->key, new_key) == 0)
            return ALDER_ERROR_DUPLICATE;
        /* key is distinct, now perform the insert */
        {
            alder_tree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node_tree(block_tree);
            if (old_leaf == NULL) {
                return ALDER_ERROR_MEMORY;
            }
            old_leaf->left = tmp_node->left;
            old_leaf->key = tmp_node->key;
            old_leaf->right = NULL;
            old_leaf->height = 0;
            new_leaf = get_node_tree(block_tree);
            if (new_leaf == NULL) {
                return ALDER_ERROR_MEMORY;
            }
            new_leaf->left = (alder_tree_node_t *) new_object;
            new_leaf->key = new_key;
            new_leaf->right = NULL;
            new_leaf->height = 0;
            //            if (tmp_node->key < new_key) {
            if ((*rb_compare)(tmp_node->key, new_key) < 0) {
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
    return ALDER_SUCCESS;
}



static alder_uf_node_t *
tree_delete (alder_tree_node_t * tree, void *delete_key,
             alder_comparison_func *rb_compare,
             struct alder_tree_memory_struct *block_tree)
{
    alder_tree_node_t *tmp_node, *upper_node, *other_node;
    alder_uf_node_t *deleted_object;
    int finished;
    if (tree->left == NULL)
        return (NULL);
    else if (tree->right == NULL)
    {
        //        if (tree->key == delete_key)
        if ((*rb_compare)(tree->key, delete_key) == 0)
        {
            deleted_object = (alder_uf_node_t*) tree->left;
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
            if ((*rb_compare)(delete_key, tmp_node->key) < 0)
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
        //        if (tmp_node->key != delete_key)
        if ((*rb_compare)(tmp_node->key, delete_key) != 0)
            deleted_object = NULL;
        else
        {
            upper_node->key = other_node->key;
            upper_node->left = other_node->left;
            upper_node->right = other_node->right;
            upper_node->height = other_node->height;
            deleted_object = (alder_uf_node_t*) tmp_node->left;
            return_node_tree(block_tree, tmp_node);
            return_node_tree(block_tree, other_node);
            
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

#pragma mark alder

/* This function creates a union-find structure.
 * returns a union-find structure, and NULL otherwise.
 * always call alder_uf_destroy after alder_uf_create
 * even if alder_uf_create failed. This would free any memory
 * allocated by alder_uf_create.
 */
alder_uf_t * alder_uf_create(alder_comparison_func *alder_compare,
                             alder_print_func *alder_print)
{
    alder_uf_t *uf;
    uf = malloc(sizeof(*uf));
    if (uf == NULL) {
        return NULL;
    }
    uf->alder_compare = alder_compare;
    uf->alder_print = alder_print;
    uf->block_size = BLOCKSIZE;
    
    uf->block_tree.current = NULL;
    uf->block_tree.free    = NULL;
    uf->block_tree.head    = NULL;
    uf->block_tree.nhead   = 0;
    uf->block_tree.size    = 0;
    
    uf->block_uf.current = NULL;
    uf->block_uf.free    = NULL;
    uf->block_uf.head    = NULL;
    uf->block_uf.nhead   = 0;
    uf->block_uf.size    = 0;
    
    uf->tree = tree_create(&uf->block_tree);
//    item_node_t *new_item = get_node_item();
//    new_item->item.a = 111111;
//    new_item->item.b = 111111;
    uf->uf = uf_create(&uf->block_uf, NULL);
    if (uf->tree == NULL || uf->uf == NULL) {
        if (uf->tree != NULL) return_node_tree(&uf->block_tree, uf->tree);
        if (uf->uf != NULL) return_node_uf(&uf->block_uf, uf->uf);
        free(uf);
        return NULL;
    }
    return uf;
}

/* This function destroys the union-find structure.
 */
void alder_uf_destroy(alder_uf_t *uf)
{
    /* Return memory for the union-find structure. */
    if (uf != NULL) {
        tree_destroyWithUF(&uf->block_tree, &uf->block_uf, uf->tree);
        return_node_uf(&uf->block_uf, uf->uf);
        destroy_node_tree(&uf->block_tree);
        destroy_node_uf(&uf->block_uf);
        free(uf);
    }
}

/* This function creates a new set with a single element.
 * returns ALDER_SUCCESS if a new set is created with the element.
 * ALDER_ERROR otherwise. If the element already exists in the tree, this
 * function returns ALDER_ERROR_DUPLICATE.
 * always call alder_uf_destroy if this function returns ALDER_ERROR_MEMORY.
 */
int alder_uf_makeset(alder_uf_t *uf, void *e)
{
    int s;
//    item_node_t *new_item;
    alder_uf_node_t *new_uf_node;
    
    alder_uf_node_t *n = tree_find (uf->tree, e, uf->alder_compare);
    if (n != NULL) {
        return ALDER_ERROR_DUPLICATE;
    }
    
//    new_item = get_node_item();
//    if (new_item == NULL) return ALDER_ERROR_MEMORY;
//    new_item->item = e;
    new_uf_node = uf_insert(&uf->block_uf, e);
    if (new_uf_node == NULL) {
//        return_node_item(new_item);
        return ALDER_ERROR_MEMORY;
    }
    
    alder_uf_node_t *rootUF = uf->uf;
    alder_uf_node_t *t;
    t = rootUF->down;
    if (t != NULL) {
        assert(t->right==NULL);
        t->right = new_uf_node;
        new_uf_node->left = t;
    }
    new_uf_node->up = rootUF;
    rootUF->down = new_uf_node;
    rootUF->degree++;
    
    s = tree_insert(uf->tree, e, new_uf_node, uf->alder_compare, &uf->block_tree);
    if (s != ALDER_SUCCESS) {
        alder_uf_assert();
    }
    
    return s;
}

/* This function takes the union of two sets. Two sets are identified by
 * two different elements in them.
 * returns
 * ALDER_ERROR_DUPLICATE - two items are the same or belongs to the same set.
 * ALDER_ERROR_NOTEXIST - not both of the items exist in the univeral set.
 */
int alder_uf_union(alder_uf_t *uf, void *e1, void *e2)
{
    //    if (e1 == e2) {
    if ((*uf->alder_compare)(e1, e2) == 0) {
        return ALDER_ERROR_DUPLICATE;
    }
    alder_uf_node_t *n1, *n2;
    n1 = tree_find(uf->tree, e1, uf->alder_compare);
    n2 = tree_find(uf->tree, e2, uf->alder_compare);
    if (n1 == NULL || n2 == NULL) {
        return ALDER_ERROR_NOTEXIST;
    }
    int s = uf_join (n1, n2);
    return s;
}


/* This function finds the item of a set.
 * returns
 * ALDER_SUCCESS on success.
 * ALDER_ERROR_NOTEXIST if the item does not exist.
 */
int alder_uf_find(alder_uf_t *uf, void *e, void **r)
{
    alder_uf_node_t *n;
    alder_uf_node_t *root;
    n = tree_find (uf->tree, e, uf->alder_compare);
    if (n == NULL) {
        *r = NULL;
        return ALDER_ERROR_NOTEXIST;
    }
    for (root = n; root->up->rank > 0; root = root->up);
    *r = root->item;
    return ALDER_SUCCESS;
}

/* This function finds the item in the search tree.
 * returns
 * ALDER_SUCCESS on success.
 * ALDER_ERROR_NOTEXIST if the item does not exist.
 */
int alder_uf_item(alder_uf_t *uf, void *e, void **r)
{
    alder_uf_node_t *n = tree_find (uf->tree, e, uf->alder_compare);
    if (n == NULL) {
        *r = NULL;
        return ALDER_ERROR_NOTEXIST;
    }
    *r = n->item;
    return ALDER_SUCCESS;
}


/* This function tests whether two elements are in the same set.
 * returns
 * ALDER_ERROR_NOTEXIST if not both of the elements do not exist in the set.
 * ALDER_SUCCESS on success with s set to
 * 1 or true if the two elements are in the same set.
 * 0 or false if they are in different sets.
 */
int alder_uf_same(alder_uf_t *uf, void *e1, void *e2, int *s)
{
    alder_uf_node_t *n1, *n2;
    n1 = tree_find(uf->tree, e1, uf->alder_compare);
    if (n1 == NULL) return ALDER_ERROR_NOTEXIST;
    n2 = tree_find(uf->tree, e2, uf->alder_compare);
    if (n2 == NULL) return ALDER_ERROR_NOTEXIST;
    *s = uf_same(n1, n2);
    return ALDER_SUCCESS;
}

/* This function deletes an element in the union-find structure.
 * returns
 * ALDER_ERROR_NOTEXIST if the element does not exist in the set.
 * ALDER_SUCCESS on successful deletion.
 */
int alder_uf_delete(alder_uf_t *uf, void *e)
{
    alder_uf_node_t *n;
    alder_uf_node_t *delete_node;
    n = tree_find(uf->tree, e, uf->alder_compare);
    if (n == NULL) return ALDER_ERROR_NOTEXIST;
    delete_node = tree_delete(uf->tree, e, uf->alder_compare, &uf->block_tree);
    if (delete_node == NULL) return ALDER_ERROR;
    if (delete_node != n) return ALDER_ERROR;
    uf_delete(n);
//    return_node_item(n->item);
    return_node_uf(&uf->block_uf, n);
    return ALDER_SUCCESS;
}

/* This function constructs a linked list of items for a set.
 * returns
 * ALDER_ERROR_NOTEXIST if the element does not exist in the set.
 * ALDER_SUCCESS on successful deletion.
 * Use uf->uf->list to access all of the enumerated values.
 */
int alder_uf_enumerate(alder_uf_t *uf, void *e)
{
    alder_uf_node_t *rootUF;
    alder_uf_node_t *root;
    alder_uf_node_t *n;
    alder_uf_node_t *c;
    n = tree_find(uf->tree, e, uf->alder_compare);
    if (n == NULL) return ALDER_ERROR_NOTEXIST;
    
    for (root = n; root->up->rank > 0; root = root->up);
    rootUF = root->up;
    
    alder_uf_node_stack_t *stack = uf_stack_create(100);
    uf_stack_push(root, stack);
    c = rootUF;
    while (!uf_stack_empty(stack)) {
        alder_uf_node_t *t = uf_stack_pop(stack);
        alder_uf_node_t *t2 = t->down;
        while (t2 != NULL) {
            uf_stack_push(t2, stack);
            t2 = t2->left;
        }
        c->list = t;
        c = t;
    }
    c->list = NULL;
    uf_stack_destroy(stack);
    return ALDER_SUCCESS;
}

/* This function counts the number of element in a union-find set.
 */
int alder_uf_set_count(alder_uf_t *uf, void *e)
{
    int s = alder_uf_enumerate(uf, e);
    if (s != ALDER_SUCCESS) {
        return s;
    }
    int v = 0;
    alder_uf_node_t *t = uf->uf->list;
    while (t != NULL) {
        v++;
    }
    return v;
}

/* This function returns another element, if any, in a union-find set.
 * returns
 * NULL if there is only a single element in the set.
 */
void *alder_uf_another(alder_uf_t *uf, void *e)
{
    int s = alder_uf_enumerate(uf, e);
    if (s != ALDER_SUCCESS) {
        return NULL;
    }
    alder_uf_node_t *v = NULL;
    alder_uf_node_t *t = uf->uf->list;
    while (t != NULL) {
        if (t != e) {
            v = t;
            break;
        }
    }
    if (v == NULL) {
        return NULL;
    } else {
        return v->item;
    }
}

/* This function is an example of using uf->uf->list to access enumerated
 * elements.
 */
void alder_uf_printSet(alder_uf_t *uf, FILE *fp)
{
    alder_uf_node_t *t = uf->uf->list;
    while (t != NULL) {
        (*uf->alder_print)(fp, t->item);
        fputc(' ', fp);
        t = t->list;
    }
    fputc('\n', fp);
}

/* This function counts the sets.
 */
int alder_uf_count(alder_uf_t *uf)
{
    alder_uf_node_t *rootUF;
    rootUF = uf->uf;
    
    int i = 0;
    alder_uf_node_t *t = rootUF->down;
    while (t != NULL) {
        i++;
        t = t->left;
    }
    return i;
}

/* This function returns the name of k-th set.
 */
void * alder_uf_set_k(alder_uf_t *uf, int k)
{
    alder_uf_node_t *rootUF;
    rootUF = uf->uf;
    
    alder_uf_node_t *t = rootUF->down;
    for (int i = 0; i < k; i++) {
        t = t->left;
    }
    return t->item;
}

void *alder_uf_root(alder_uf_t *uf, void *e)
{
    alder_uf_node_t *root;
    alder_uf_node_t *n;
    n = tree_find(uf->tree, e, uf->alder_compare);
    if (n == NULL) return NULL;
    for (root = n; root->up->rank > 0; root = root->up);
    return root;
}

/* This function tests the union-find algorithm.
 * creates 10 sets, of which index starts at 0 and ends in 9.
 * Set 0 contains elements from 0 to 99.
 * Set 1 contains those from 100 to 199.
 * Set 9 contains those from 900 to 999.
 * So, Set i contains those from (i x 100) to (i x 100 + 99).
 * I randomly join two elements in each set until all of the elements in each
 * set are joined together.
 * tests the following:
 * 1. Each of two elements in a set must be in the same set.
 * 2. Each set must contains 100 elements.
 * 3. I repeatedly remove an element in the set until the set is empty.
 * 4. Repeat the check for each set after the removal of an element.
 */
int alder_uf_test0()
{
#if 0
    srand(1);
    alder_uf_t *uf = alder_uf_create();
    const int size_superset = 10;
    const int size_set = 10000;
    
    for (int k = 0; k < size_superset; k++) {
        for (item_t i = k*size_set; i < (k+1)*size_set; i++) {
            alder_uf_makeset(uf, i);
        }
        int c = alder_uf_count(uf);
        while (c > 1 + k) {
            int rn1 = rand()%c;
            int rn2 = rand()%c;
            while (rn1 == rn2) {
                rn2 = rand()%c;
            }
            item_t set1 = alder_uf_set_k(uf, rn1);
            item_t set2 = alder_uf_set_k(uf, rn2);
            if (set1 >= k*size_set && set1 < (k+1)*size_set
                && set2 >= k*size_set && set2 < (k+1)*size_set) {
                alder_uf_union(uf, set1, set2);
                c = alder_uf_count(uf);
            }
        }
    }
    //    alder_uf_print(uf, stdout, 1);
    /* Test of find. */
    for (int i = 0; i < size_superset*size_set; i++) {
        int k = i / size_set;
        item_t e = i;
        int s = alder_uf_find(uf, &e);
        assert(s == ALDER_SUCCESS);
        assert(e >= k*size_set && e < (k+1)*size_set
               && e >= k*size_set && e < (k+1)*size_set);
    }
    
    for (int i = 0; i < size_superset*size_set; i++) {
        int rn1 = rand()%(size_superset*size_set);
        int rn2 = rand()%(size_superset*size_set);
        int k1 = rn1 / size_set;
        int k2 = rn2 / size_set;
        int s;
        alder_uf_same(uf, rn1, rn2, &s);
        if (k1 == k2) {
            assert(s == 1);
        } else {
            if (s != 0) {
                printf("[i] %d [s] %d [k] %d vs. %d [rn] %d vs. %d\n", i, s, k1, k2, rn1, rn2);
            }
            assert(s == 0);
            
        }
    }
    
    for (int i = 0; i < size_superset*size_set; i++) {
        //        alder_uf_print(uf, stdout, i);
        alder_uf_delete(uf, i);
    }
    alder_uf_print(uf, stdout, 1);
    
    alder_uf_destroy(uf);
#endif
    
    return ALDER_SUCCESS;
}
