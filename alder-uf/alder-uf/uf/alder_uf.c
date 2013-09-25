/**
 * This file, alder_uf.c, is part of alder-uf.
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
#include "alder_uf.h"

#pragma mark memory

#define BLOCKSIZE 256

#pragma mark memory item

struct alder_item_memory {
    item_node_t **head;
    item_node_t *current;
    item_node_t *free;
    int size;
    size_t nhead;
};

static struct alder_item_memory block_item = { NULL, NULL, NULL, 0 };

static item_node_t *get_node_item()
{
    item_node_t *t = NULL;
    if (block_item.free != NULL) {
        t = block_item.free;
        block_item.free = block_item.free->next;
    } else {
        if (block_item.current  == NULL || block_item.size == 0) {
            block_item.nhead++;
            item_node_t* *th = realloc(block_item.head, block_item.nhead * sizeof(*th));
            if (th == NULL) {
                return NULL;
            }
            block_item.head = th;
            block_item.head[block_item.nhead-1] = malloc(BLOCKSIZE*sizeof(*block_item.head[0]));
            assert(sizeof(item_node_t)==sizeof(*block_item.head[0]));
            if (block_item.head[block_item.nhead-1] == NULL) {
                return NULL;
            }
            block_item.current = block_item.head[block_item.nhead-1];
            block_item.size = BLOCKSIZE;
        }
        t = block_item.current++;
        block_item.size--;
    }
    /* Initialize the allocated one. */
    return t;
}

static void return_node_item(item_node_t *node)
{
    node->next = block_item.free;
    block_item.free = node;
}

static void destroy_node_item()
{
    if (block_item.head != NULL) {
        for (size_t i = 0; i < block_item.nhead; i++) {
            free(block_item.head[i]);
        }
        free(block_item.head);
    }
}

#pragma mark memory uf

struct alder_uf_memory {
    uf_node_t **head;
    uf_node_t *current;
    uf_node_t *free;
    int size;
    size_t nhead;
};

static struct alder_uf_memory block_uf = { NULL, NULL, NULL, 0 };

static uf_node_t *get_node_uf()
{
    uf_node_t *t = NULL;
    if (block_uf.free != NULL) {
        t = block_uf.free;
        block_uf.free = block_uf.free->next;
    } else {
        if (block_uf.current  == NULL || block_uf.size == 0) {
            block_uf.nhead++;
            uf_node_t* *th = realloc(block_uf.head, block_uf.nhead * sizeof(*th));
            if (th == NULL) {
                return NULL;
            }
            block_uf.head = th;
            block_uf.head[block_uf.nhead-1] = malloc(BLOCKSIZE*sizeof(*block_uf.head[0]));
            assert(sizeof(uf_node_t)==sizeof(*block_uf.head[0]));
            if (block_uf.head[block_uf.nhead-1] == NULL) {
                return NULL;
            }
            block_uf.current = block_uf.head[block_uf.nhead-1];
            block_uf.size = BLOCKSIZE;
        }
        t = block_uf.current++;
        block_uf.size--;
    }
    /* Initialize the allocated one. */
    return t;
}

static void return_node_uf(uf_node_t *node)
{
    node->next = block_uf.free;
    block_uf.free = node;
}

static void destroy_node_uf()
{
    if (block_uf.head != NULL) {
        for (size_t i = 0; i < block_uf.nhead; i++) {
            free(block_uf.head[i]);
        }
        free(block_uf.head);
    }
}

#pragma mark memory tree

struct alder_tree_memory {
    tree_node_t **head;
    tree_node_t *current;
    tree_node_t *free;
    int size;
    size_t nhead;
};

static struct alder_tree_memory block_tree = { NULL, NULL, NULL, 0 };

static tree_node_t *get_node_tree()
{
    tree_node_t *t = NULL;
    if (block_tree.free != NULL) {
        t = block_tree.free;
        block_tree.free = block_tree.free->next;
    } else {
        if (block_tree.current  == NULL || block_tree.size == 0) {
            block_tree.nhead++;
            tree_node_t* *th = realloc(block_tree.head, block_tree.nhead * sizeof(*th));
            if (th == NULL) {
                return NULL;
            }
            block_tree.head = th;
            block_tree.head[block_tree.nhead-1] = malloc(BLOCKSIZE*sizeof(*block_tree.head[0]));
            assert(sizeof(tree_node_t)==sizeof(*block_tree.head[0]));
            if (block_tree.head[block_tree.nhead-1] == NULL) {
                return NULL;
            }
            block_tree.current = block_tree.head[block_tree.nhead-1];
            block_tree.size = BLOCKSIZE;
        }
        t = block_tree.current++;
        block_tree.size--;
    }
    /* Initialize the allocated one. */
    return t;
}

static void return_node_tree(tree_node_t *node)
{
    node->next = block_tree.free;
    block_tree.free = node;
}

static void return_node_treeWithUFAndItem(tree_node_t *node)
{
    assert(node->right==NULL);
    return_node_item(((uf_node_t*)node->next)->item);
    return_node_uf((uf_node_t*)node->next);
    
    node->next = block_tree.free;
    block_tree.free = node;
}

static void destroy_node_tree()
{
    if (block_tree.head != NULL) {
        for (size_t i = 0; i < block_tree.nhead; i++) {
            free(block_tree.head[i]);
        }
        free(block_tree.head);
    }
}


#pragma mark memory stack

typedef struct alder_uf_node_stack {
    uf_node_t **base;
    uf_node_t  **top;
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

static void uf_stack_push(uf_node_t *x, alder_uf_node_stack_t *s)
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

static uf_node_t* uf_stack_pop(alder_uf_node_stack_t *s)
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
static uf_node_t* uf_stack_top(alder_uf_node_stack_t *s)
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

#pragma mark item

static void
item_print(FILE *fp, item_node_t *i)
{
    fprintf(fp, "%d", i->item);
}

#pragma mark uf

static uf_node_t *
uf_create (item_node_t *new_item)
{
    uf_node_t *new_node;
    new_node = get_node_uf();
    if (new_node == NULL) return NULL;
    new_node->item = new_item;
    new_node->rank = 0;
    new_node->degree = 0;
    new_node->up = NULL;
    new_node->down = NULL;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}


static uf_node_t *
uf_insert (item_node_t *new_item)
{
    uf_node_t *new_node;
    new_node = get_node_uf();
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

static int
same_class (uf_node_t * node1, uf_node_t * node2)
{
    uf_node_t *root1;
    uf_node_t *root2;
    /* find both roots */
    for (root1 = node1; root1->up->rank > 0; root1 = root1->up);
    for (root2 = node1; root2->up->rank > 0; root2 = root2->up);
    
    /* return result */
    return (root1 == root2);
}

static void
uf_delete (uf_node_t * node1)
{
    uf_node_t *rootUF;
    uf_node_t *root1;
    uf_node_t *t1;
    uf_node_t *pnode1;
    uf_node_t *down1;
    
    /* Two cases: 1. no down, and 2. down. */
    for (root1 = node1; root1->up->rank > 0; root1 = root1->up);
    rootUF = root1->up;
    pnode1 = node1->up;
    down1 = node1->down;
    if (down1 == NULL) {
        /* Detach the node. */
        t1 = node1;
        if (t1->left != NULL && pnode1->down == t1) pnode1->down = t1->left;
        if (t1->left != NULL) t1->left->right = t1->right;
        if (t1->right != NULL) t1->right->left = t1->left;
        pnode1->degree--;
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
            assert(pnode1->rank <= new_rank + 1);
            if (pnode1->rank < new_rank + 1) {
                pnode1->rank = new_rank + 1;
            } else {
                break;
            }
        }
        pnode1 = pnode1->up;
    }
}

static void
join (uf_node_t * node1, uf_node_t * node2)
{
    uf_node_t *rootUF;
    uf_node_t *root1;
    uf_node_t *root2;
    uf_node_t *t1;
    uf_node_t *t2;
    /* find both roots */
    for (root1 = node1; root1->up->rank > 0; root1 = root1->up);
    for (root2 = node2; root2->up->rank > 0; root2 = root2->up);
    assert(root1 != root2);
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
        if (t2->left != NULL && rootUF->down == t2) rootUF->down = t2->left;
        if (t2->left != NULL) t2->left->right = t2->right;
        if (t2->right != NULL) t2->right->left = t2->left;
        root2->up = root1;
        root1->down = root2;
        root2->right = NULL;
        root2->left = NULL;
        rootUF->degree--;
        root1->rank = 2;
        root1->degree = 1;
    } else if (root1->rank == 2) {
        if (root2->rank == 1) {
            assert(root2->degree == 0);
            assert(root2->down == NULL);
            /* Detach root1. */
            t1 = root1;
            if (t1->left != NULL && rootUF->down == t1) rootUF->down = t1->left;
            if (t1->left != NULL) t1->left->right = t1->right;
            if (t1->right != NULL) t1->right->left = t1->left;
            root1->up = root2;
            root2->down = root1;
            root1->right = NULL;
            root1->left = NULL;
            rootUF->degree--;
            root2->rank = 3;
            root2->degree = 1;
        } else {
            assert(root2->rank == 2);
            /* Detach root1. */
            t1 = root1;
            if (t1->left != NULL && rootUF->down == t1) rootUF->down = t1->left;
            if (t1->left != NULL) t1->left->right = t1->right;
            if (t1->right != NULL) t1->right->left = t1->left;
            /* Append root1 to root2's children. */
            t2 = root2->down;
            root1->up = root2;
            root2->down = root1;
            t2->right = root1;
            root1->left = t2;
            root1->right = NULL;
            root2->rank = 3;
            root2->degree++;
        }
    } else if (root1->rank >= 3) {
        if (root2->rank < root1->rank)
        {
            if (root2->rank == 1) {
                assert(root2->degree == 0);
                assert(root2->down == NULL);
                /* Detach root2. */
                t2 = root2;
                if (t2->left != NULL && rootUF->down == t2) rootUF->down = t2->left;
                if (t2->left != NULL) t2->left->right = t2->right;
                if (t2->right != NULL) t2->right->left = t2->left;
                t1 = root1->down;
                uf_node_t *t11 = t1->down;
                root2->up = t1;
                t1->down = root2;
                root2->left = t11;
                root2->right = NULL;
                if (t11 != NULL) {
                    t11->right = root2;
                } else {
                    assert(t1->rank == 1);
                    t1->rank = 2;
                }
                t1->degree++;
            } else {
                /* Detach root2. */
                t2 = root2;
                if (t2->left != NULL && rootUF->down == t2) rootUF->down = t2->left;
                if (t2->left != NULL) t2->left->right = t2->right;
                if (t2->right != NULL) t2->right->left = t2->left;
                t1 = root1->down;
                uf_node_t *t11 = t1->down;
                root2->up = t1;
                t2 = root2->down;
                t2->right = root2;
                root2->left = t2;
                root2->right = NULL;
                root2->down = NULL;
                while (t2->left != NULL) {
                    t2->up = t1;
                    t2 = t2->left;
                }
                t2->up = t1;
                t1->down = root2;
                t2->left = t11;
                if (t11 != NULL) {
                    t11->right = t2;
                } else {
                    assert(t1->rank == 1);
                    t1->rank = 2;
                }
                t1->degree += root2->degree + 1;
                root2->rank = 1;
                root2->degree = 0;
            }
        }
        else  /* root2->height == root1->height */
        {
            /* Detach root2. */
            t2 = root2;
            if (t2->left != NULL && rootUF->down == t2) rootUF->down = t2->left;
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
            
            
            root1->degree += root2->degree;
            
            /* now lists joined together below root 1 */
            if (root1->degree < root1->rank) {
                root2->up = root1->down;    /* point to node on root1 list */
                t1 = root1->down->down;
                root2->left = t1;
                root2->right = NULL;
                if (t1 != NULL) {
                    t1->right = root2;
                } else {
                    root1->down->rank = 2;
                }
                root2->up->down = root2;
                root2->down = NULL;
                root2->rank = 1;
                root2->degree = 0;
                root1->down->degree++;
            }
            else                  /* root2 becomes new root, root1 goes below */
            {
                /* Detach root1. */
                t1 = root1;
                if (t1->left != NULL && rootUF->down == t1) rootUF->down = t1->left;
                if (t1->left != NULL) t1->left->right = t1->right;
                if (t1->right != NULL) t1->right->left = t1->left;
                root1->up = root2;
                root2->down = root1;
                root2->rank = root1->rank + 1;
                root2->degree = 1;
            }
            rootUF->degree--;
        }
    }
}

static tree_node_t *
tree_create (void)
{
    tree_node_t *t = get_node_tree();
    if (t == NULL) return NULL;
    t->left = NULL;
    return t;
}

#if 0
static void
tree_destroy (tree_node_t * tree)
{
    tree_node_t *current_node, *tmp;
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
tree_destroyWithUFAndItem (tree_node_t * tree)
{
    tree_node_t *current_node, *tmp;
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
                return_node_treeWithUFAndItem(current_node->left);
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
        return_node_treeWithUFAndItem(current_node);
    }
}

static void
left_rotation (tree_node_t * n)
{
    tree_node_t *tmp_node;
    key_t tmp_key;
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
right_rotation (tree_node_t * n)
{
    tree_node_t *tmp_node;
    key_t tmp_key;
    tmp_node = n->right;
    tmp_key = n->key;
    n->right = n->left;
    n->key = n->left->key;
    n->left = n->right->left;
    n->right->left = n->right->right;
    n->right->right = tmp_node;
    n->right->key = tmp_key;
}

static object_t *
find (tree_node_t * tree, key_t query_key)
{
    tree_node_t *tmp_node;
    if (tree->left == NULL)
        return (NULL);
    else
    {
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            if (query_key < tmp_node->key)
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
        }
        if (tmp_node->key == query_key)
            return ((object_t *) tmp_node->left);
        else
            return (NULL);
    }
}


static int
insert (tree_node_t * tree, key_t new_key, object_t * new_object)
{
    tree_node_t *tmp_node;
    int finished;
    if (tree->left == NULL) {
        tree->left = (tree_node_t *) new_object;
        tree->key = new_key;
        tree->height = 0;
        tree->right = NULL;
    } else {
        tree_node_t *path_stack[100];
        int path_st_p = 0;
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            path_stack[path_st_p++] = tmp_node;
            if (new_key < tmp_node->key)
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
        }
        /* found the candidate leaf. Test whether key distinct */
        if (tmp_node->key == new_key)
            return (-1);
        /* key is distinct, now perform the insert */
        {
            tree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node_tree();
            old_leaf->left = tmp_node->left;
            old_leaf->key = tmp_node->key;
            old_leaf->right = NULL;
            old_leaf->height = 0;
            new_leaf = get_node_tree();
            new_leaf->left = (tree_node_t *) new_object;
            new_leaf->key = new_key;
            new_leaf->right = NULL;
            new_leaf->height = 0;
            if (tmp_node->key < new_key) {
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
                    right_rotation (tmp_node);
                    tmp_node->right->height = tmp_node->right->left->height + 1;
                    tmp_node->height = tmp_node->right->height + 1;
                } else {
                    left_rotation (tmp_node->left);
                    right_rotation (tmp_node);
                    tmp_height = tmp_node->left->left->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            } else if (tmp_node->left->height - tmp_node->right->height == -2) {
                if (tmp_node->right->right->height - tmp_node->left->height == 1) {
                    left_rotation (tmp_node);
                    tmp_node->left->height = tmp_node->left->right->height + 1;
                    tmp_node->height = tmp_node->left->height + 1;
                } else {
                    right_rotation (tmp_node->right);
                    left_rotation (tmp_node);
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
    return (0);
}



static object_t *
delete (tree_node_t * tree, key_t delete_key)
{
    tree_node_t *tmp_node, *upper_node, *other_node;
    object_t *deleted_object;
    int finished;
    if (tree->left == NULL)
        return (NULL);
    else if (tree->right == NULL)
    {
        if (tree->key == delete_key)
        {
            deleted_object = (object_t *) tree->left;
            tree->left = NULL;
            return (deleted_object);
        }
        else
            return (NULL);
    }
    else
    {
        tree_node_t *path_stack[100];
        int path_st_p = 0;
        tmp_node = tree;
        while (tmp_node->right != NULL)
        {
            path_stack[path_st_p++] = tmp_node;
            upper_node = tmp_node;
            if (delete_key < tmp_node->key)
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
        if (tmp_node->key != delete_key)
            deleted_object = NULL;
        else
        {
            upper_node->key = other_node->key;
            upper_node->left = other_node->left;
            upper_node->right = other_node->right;
            upper_node->height = other_node->height;
            deleted_object = (object_t *) tmp_node->left;
            return_node_tree(tmp_node);
            return_node_tree(other_node);
            
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
                    right_rotation (tmp_node);
                    tmp_node->right->height = tmp_node->right->left->height + 1;
                    tmp_node->height = tmp_node->right->height + 1;
                }
                else
                {
                    left_rotation (tmp_node->left);
                    right_rotation (tmp_node);
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
                    left_rotation (tmp_node);
                    tmp_node->left->height = tmp_node->left->right->height + 1;
                    tmp_node->height = tmp_node->left->height + 1;
                }
                else
                {
                    right_rotation (tmp_node->right);
                    left_rotation (tmp_node);
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
 */
alder_uf_t * alder_uf_create()
{
    alder_uf_t *uf;
    uf = malloc(sizeof(*uf));
    if (uf == NULL) {
        return NULL;
    }
    uf->tree = tree_create();
    item_node_t *new_item;
    new_item = get_node_item();
    uf->uf = uf_create(new_item);
   if (uf->tree == NULL || uf->uf == NULL) {
       if (uf->tree != NULL) return_node_tree(uf->tree);
       if (uf->uf != NULL) return_node_uf(uf->uf);
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
    tree_destroyWithUFAndItem(uf->tree);
    return_node_item(uf->uf->item);
    return_node_uf(uf->uf);
    
    /* Check if the memory is clean. */
    destroy_node_item();
    destroy_node_tree();
    destroy_node_uf();
    
    /* Free the memory for the union-find variable. */
    free(uf);
}

/* This function creates a new set with a single element.
 * returns ALDER_SUCCESS or ALDER_ERROR.
 */
int alder_uf_makeset(alder_uf_t *uf, item_t e)
{
    int s;
    item_node_t *new_item;
    uf_node_t *new_uf_node;
    
    new_item = get_node_item();
    if (new_item == NULL) return ALDER_ERROR;
    new_item->item = e;
    new_uf_node = uf_insert(new_item);
    if (new_uf_node == NULL) {
        return_node_item(new_item);
        return ALDER_ERROR;
    }
    
    uf_node_t *rootUF = uf->uf;
    uf_node_t *t;
    t = rootUF->down;
    if (t != NULL) {
        assert(t->right==NULL);
        t->right = new_uf_node;
        new_uf_node->left = t;
    }
    new_uf_node->up = rootUF;
    rootUF->down = new_uf_node;
    rootUF->degree++;
    
    s = insert (uf->tree, e, new_uf_node);
    
    return s;
}

int alder_uf_union(alder_uf_t *uf, item_t e1, item_t e2)
{
    uf_node_t *n1, *n2;
    n1 = find (uf->tree, e1);
    n2 = find (uf->tree, e2);
    join (n1, n2);
    return 0;
}

int alder_uf_find(alder_uf_t *uf, item_t e)
{
    uf_node_t *n;
    n = find (uf->tree, e);
    return n->item->item;
}

int alder_uf_same(alder_uf_t *uf, item_t e1, item_t e2)
{
    uf_node_t *n1, *n2;
    n1 = find (uf->tree, e1);
    n2 = find (uf->tree, e2);
    return same_class(n1, n2);
}

int alder_uf_delete(alder_uf_t *uf, item_t e)
{
    uf_node_t *n;
    n = find (uf->tree, e);
    delete(uf->tree, e);
    uf_delete(n);
    return 0;
}

/* This function constructs a linked list of items for a set.
 * returns a uf node, which can be used to list all of the elements.
 */
uf_node_t * alder_uf_enumerate(alder_uf_t *uf, item_t e)
{
    uf_node_t *rootUF;
    uf_node_t *root;
    uf_node_t *n;
    uf_node_t *c;
    n = find (uf->tree, e);
    for (root = n; root->up->rank > 0; root = root->up);
    rootUF = root->up;
    
    alder_uf_node_stack_t *stack = uf_stack_create(100);
    uf_stack_push(root, stack);
    c = rootUF;
    while (!uf_stack_empty(stack)) {
        uf_node_t *t = uf_stack_pop(stack);
        uf_node_t *t2 = t->down;
        while (t2 != NULL) {
            uf_stack_push(t2, stack);
            t2 = t2->left;
        }
        c->list = t;
        c = t;
    }
    c->list = NULL;
    uf_stack_destroy(stack);
    return root;
}

void alder_uf_printSet(alder_uf_t *uf, FILE *fp)
{
    uf_node_t *t = uf->uf->list;
    while (t != NULL) {
        item_print(fp, t->item);
        fputc(' ', fp);
        t = t->list;
    }
    fputc('\n', fp);
}



