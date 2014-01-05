/**
 * This file, alder_uf_client.c, is part of alder-uf.
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "alder_uf_void.h"
#include "alder_uf_client.h"

/* Use alder_uf_void.
 */

static int
tree_compare_key(const void *rb_a, const void *rb_b)
{
    const alder_item_node_t *a = rb_a;
    const alder_item_node_t *b = rb_b;
    
    if (a->item.a < b->item.a) {
        return -1;
    } else if (a->item.a > b->item.a) {
        return +1;
    } else {
        if (a->item.b < b->item.b) {
            return -1;
        } else if (a->item.b > b->item.b) {
            return +1;
        } else {
            return 0;
        }
    }
}

static void
tree_print (FILE *fp, const void *item)
{
    const alder_item_node_t *a = item;
    fprintf(fp, "%lld-%lld", a->item.a, a->item.b);
}


#pragma mark memory item

static alder_item_node_t *
get_node_item(struct alder_item_memory *block_item)
{
    alder_item_node_t *t = NULL;
    if (block_item->free != NULL) {
        t = block_item->free;
        block_item->free = block_item->free->next;
    } else {
        if (block_item->current  == NULL || block_item->size == 0) {
            block_item->nhead++;
            alder_item_node_t* *th = realloc(block_item->head, block_item->nhead * sizeof(*th));
            if (th == NULL) {
                return NULL;
            }
            block_item->head = th;
            block_item->head[block_item->nhead-1] = malloc(BLOCKSIZE*sizeof(*block_item->head[0]));
            assert(sizeof(alder_item_node_t)==sizeof(*block_item->head[0]));
            if (block_item->head[block_item->nhead-1] == NULL) {
                return NULL;
            }
            block_item->current = block_item->head[block_item->nhead-1];
            block_item->size = BLOCKSIZE;
        }
        t = block_item->current++;
        block_item->size--;
    }
    /* Initialize the allocated one. */
    return t;
}

static void
return_node_item(struct alder_item_memory *block_item, alder_item_node_t *node)
{
    if (node != NULL) {
        node->next = block_item->free;
        block_item->free = node;
    }
}

static void destroy_node_item(struct alder_item_memory *block_item)
{
    if (block_item->head != NULL) {
        for (size_t i = 0; i < block_item->nhead; i++) {
            free(block_item->head[i]);
        }
        free(block_item->head);
        block_item->head = NULL;
        block_item->current = NULL;
        block_item->free = NULL;
        block_item->size = 0;
        block_item->nhead = 0;
    }
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


#pragma mark item

//static void
//alder_tuple_fprint(FILE *fp, item_t i)
//{
//    fprintf(fp, "[%lld:%lld]", i.a, i.b);
//}
//
//static void
//item_print(FILE *fp, alder_item_node_t *i)
//{
//    alder_tuple_fprint(fp, i->item);
//}


#pragma mark uf tuple

alder_uf_tuple_t * alder_uf_tuple_create()
{
    alder_uf_tuple_t *ut = malloc(sizeof(*ut));
    if (ut == NULL) {
        return NULL;
    }
    ut->block_size = BLOCKSIZE;
    
    ut->block_item.current = NULL;
    ut->block_item.free    = NULL;
    ut->block_item.head    = NULL;
    ut->block_item.nhead   = 0;
    ut->block_item.size    = 0;
    ut->uf = alder_uf_create(tree_compare_key, tree_print);
    if (ut->uf == NULL) {
        free(ut);
        return NULL;
    }
    return ut;
}

void alder_uf_tuple_destroy(alder_uf_tuple_t *ut)
{
    if (ut != NULL) {
        alder_uf_destroy(ut->uf);
        destroy_node_item(&ut->block_item);
        free(ut);
    }
}

/* creates a new item, and stores it in the tree.
 */
int alder_uf_tuple_makeset(alder_uf_tuple_t *ut, item_t e)
{
    alder_item_node_t *n = get_node_item(&ut->block_item);
    if (n == NULL) {
        return ALDER_ERROR_MEMORY;
    }
    n->item = e;
    int s = alder_uf_makeset(ut->uf, n);
    return s;
}

/* takes the union of two nodes in the union-find structure.
 * NOTE: item nodes created for the two input items must be deleted.
 */
int alder_uf_tuple_union(alder_uf_tuple_t *ut, item_t e1, item_t e2)
{
    alder_item_node_t *n1 = get_node_item(&ut->block_item);
    alder_item_node_t *n2 = get_node_item(&ut->block_item);
    if (n1 == NULL || n2 == NULL) {
        return_node_item(&ut->block_item, n1);
        return_node_item(&ut->block_item, n2);
        return ALDER_ERROR_MEMORY;
    }
    n1->item = e1;
    n2->item = e2;
    int s = alder_uf_union(ut->uf, n1, n2);
    return_node_item(&ut->block_item, n1);
    return_node_item(&ut->block_item, n2);
    return s;
}

int alder_uf_tuple_find(alder_uf_tuple_t *ut, item_t *e)
{
    alder_item_node_t *n = get_node_item(&ut->block_item);
    if (n == NULL) {
        return_node_item(&ut->block_item, n);
        return ALDER_ERROR_MEMORY;
    }
    n->item = *e;
    alder_item_node_t *r = NULL;
    int s = alder_uf_find(ut->uf, n, (void**)&r);
    if (s == ALDER_SUCCESS) {
        *e = r->item;
    }
    return_node_item(&ut->block_item, n);
    return s;
}

int alder_uf_tuple_same(alder_uf_tuple_t *ut, item_t e1, item_t e2, int *is_same)
{
    alder_item_node_t *n1 = get_node_item(&ut->block_item);
    alder_item_node_t *n2 = get_node_item(&ut->block_item);
    if (n1 == NULL || n2 == NULL) {
        return_node_item(&ut->block_item, n1);
        return_node_item(&ut->block_item, n2);
        return ALDER_ERROR_MEMORY;
    }
    n1->item = e1;
    n2->item = e2;
    int s = alder_uf_same(ut->uf, n1, n2, is_same);
    return_node_item(&ut->block_item, n1);
    return_node_item(&ut->block_item, n2);
    return s;
}

int alder_uf_tuple_count(alder_uf_tuple_t *ut)
{
    return alder_uf_count(ut->uf);
}

item_t alder_uf_tuple_set_k(alder_uf_tuple_t *ut, int k)
{
    alder_item_node_t * v = alder_uf_set_k(ut->uf, k);
    return v->item;
}

int alder_uf_tuple_delete(alder_uf_tuple_t *ut, item_t e)
{
    alder_item_node_t *n = get_node_item(&ut->block_item);
    if (n == NULL) {
        return_node_item(&ut->block_item, n);
        return ALDER_ERROR_MEMORY;
    }
    n->item = e;
    alder_item_node_t *r = NULL;
    int s = alder_uf_item(ut->uf, n, (void**)&r);
    s = alder_uf_delete(ut->uf, n);
    return_node_item(&ut->block_item, n);
    return_node_item(&ut->block_item, r);
    return s;
}

int alder_uf_tuple_enumerate(alder_uf_tuple_t *ut, item_t e)
{
    alder_item_node_t *n = get_node_item(&ut->block_item);
    if (n == NULL) {
        return_node_item(&ut->block_item, n);
        return ALDER_ERROR_MEMORY;
    }
    n->item = e;
    
    int s = alder_uf_enumerate(ut->uf, n);
    
    return_node_item(&ut->block_item, n);
    return s;
}

void alder_uf_tuple_printSet(alder_uf_tuple_t *ut, FILE *fp)
{
    alder_uf_printSet(ut->uf, fp);
}

/* This function prints the content of the union-find structure.
 */
void alder_uf_tuple_print(alder_uf_tuple_t *ut, FILE *fp, int i)
{
    alder_uf_node_t *rootUF;
    
    rootUF = ut->uf->uf;
    
    alder_uf_node_stack_t *stack = uf_stack_create(100);
    uf_stack_push(rootUF, stack);
    fprintf(fp, "digraph G%d {\n", i);
    while (!uf_stack_empty(stack)) {
        alder_uf_node_t *t = uf_stack_pop(stack);
        alder_item_node_t *item  = t->item;
        if (item != NULL) {
            fprintf(fp, "a%lldb%lld [label=\"%lld-%lld (%d:%d)",
                    item->item.a,
                    item->item.b,
                    item->item.a,
                    item->item.b,
                    t->rank, t->degree);
        } else {
            fprintf(fp, "axbx [label=\"x-x (%d:%d)",
                    t->rank, t->degree);
        }
        if (t->left != NULL) {
            alder_item_node_t *item  = t->left->item;
            fprintf(fp, " %lld-%lld", item->item.a, item->item.b);
        } else {
            fprintf(fp, " x");
        }
        if (t->right != NULL) {
            alder_item_node_t *item  = t->right->item;
            fprintf(fp, " %lld-%lld", item->item.a, item->item.b);
        } else {
            fprintf(fp, " x");
        }
        if (t->down != NULL) {
            alder_item_node_t *item  = t->down->item;
            fprintf(fp, " %lld-%lld", item->item.a, item->item.b);
        } else {
            fprintf(fp, " x");
        }
        fprintf(fp, "\"];\n");
        
        if (t->up != NULL) {
            alder_item_node_t *upitem = t->up->item;
            if (upitem != NULL) {
                fprintf(fp, "a%lldb%lld -> a%lldb%lld;\n",
                        upitem->item.a,
                        upitem->item.b,
                        item->item.a,
                        item->item.b);
            } else {
                fprintf(fp, "axbx -> a%lldb%lld;\n",
                        item->item.a,
                        item->item.b);
            }
        }
        if (t->down != NULL) {
            alder_item_node_t *downitem = t->down->item;
            fprintf(fp, "a%lldb%lld [color=red];\n",
                    downitem->item.a,
                    downitem->item.b);
        }
        alder_uf_node_t *t2 = t->down;
        while (t2 != NULL) {
            uf_stack_push(t2, stack);
            t2 = t2->left;
        }
    }
    fprintf(fp, "}\n");
    uf_stack_destroy(stack);
}

#pragma mark test

/* A manual test.
 */
int alder_uf_client_test1()
{
    item_t v, v1, v2;
    int i = 0;
    
    alder_uf_tuple_t *ut = alder_uf_tuple_create();
    
    v.a = 1; v.b = 2;
    alder_uf_tuple_makeset(ut, v);
    v.a = 1; v.b = 3;
    alder_uf_tuple_makeset(ut, v);
    alder_uf_tuple_print(ut, stdout, i++);
    
    alder_uf_tuple_destroy(ut);
    
    
#if 0
    
    v.a = 1; v.b = 3;
    alder_uf_tuple_makeset(ut, v);
    v.a = 4; v.b = 5;
    alder_uf_tuple_makeset(ut, v);
    v.a = 4; v.b = 5;
    alder_uf_tuple_delete(ut, v);
    v.a = 4; v.b = 5;
    alder_uf_tuple_makeset(ut, v);
    v1.a = 1; v1.b = 2;
    v2.a = 4; v2.b = 5;
    alder_uf_tuple_union(ut, v1, v2);
    
    
    
       v.a = 1; v.b = 2;
    alder_uf_makeset(uf, v);
    v.a = 1; v.b = 3;
    alder_uf_makeset(uf, v);
    v.a = 4; v.b = 5;
    alder_uf_makeset(uf, v);
    v.a = 4; v.b = 5;
    alder_uf_delete(uf, v);
    v.a = 4; v.b = 5;
    alder_uf_makeset(uf, v);
    v1.a = 1; v1.b = 2;
    v2.a = 4; v2.b = 5;
    alder_uf_union(uf, v1, v2);
    alder_uf_print(uf, stdout, i++);
    
    
    
    alder_uf_t *uf;
    /* Test: creats and destroys a union-find type variable. */
    //    uf = alder_uf_create();
    //    alder_uf_destroy(uf);
    
    /* Test: makes a new set with an element. */
    int i = 0;
    uf = alder_uf_create();
    alder_uf_makeset(uf, 1);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 2);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 3);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 4);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 5);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 6);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 7);
    alder_uf_print(uf, stdout, i++);
    
    alder_uf_makeset(uf, 11);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 12);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 13);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 14);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 15);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 16);
    alder_uf_print(uf, stdout, i++);
    alder_uf_makeset(uf, 17);
    alder_uf_print(uf, stdout, i++);
    alder_uf_union(uf, 11, 12);
    alder_uf_print(uf, stdout, i++);
    alder_uf_union(uf, 11, 13);
    alder_uf_print(uf, stdout, i++);
    alder_uf_union(uf, 11, 14);
    alder_uf_print(uf, stdout, i++);
    alder_uf_union(uf, 11, 15);
    alder_uf_print(uf, stdout, i++);
    alder_uf_union(uf, 11, 16);
    alder_uf_print(uf, stdout, i++);
    alder_uf_union(uf, 11, 17);
    alder_uf_print(uf, stdout, i++);
    
    alder_uf_union(uf, 1, 2);
    alder_uf_print(uf, stdout, i++);
    alder_uf_union(uf, 3, 4);
    alder_uf_print(uf, stdout, i++);
    
    //    alder_uf_enumerate(uf, 1);
    //    alder_uf_printSet(uf, stdout);
    //    alder_uf_enumerate(uf, 3);
    //    alder_uf_printSet(uf, stdout);
    
    alder_uf_union(uf, 2, 4);
    alder_uf_print(uf, stdout, i++);
    //    alder_uf_enumerate(uf, 2);
    //    alder_uf_printSet(uf, stdout);
    
    alder_uf_union(uf, 5, 6);
    alder_uf_print(uf, stdout, i++);
    alder_uf_union(uf, 5, 7);
    alder_uf_print(uf, stdout, i++);
    //    alder_uf_enumerate(uf, 5);
    //    alder_uf_printSet(uf, stdout);
    
    alder_uf_union(uf, 2, 5);
    alder_uf_print(uf, stdout, i++);
    //    alder_uf_enumerate(uf, 2);
    //    alder_uf_printSet(uf, stdout);
    
    alder_uf_union(uf, 2, 11);
    alder_uf_print(uf, stdout, i++);
    //    alder_uf_enumerate(uf, 2);
    //    alder_uf_printSet(uf, stdout);
    alder_uf_delete(uf, 6);
    alder_uf_print(uf, stdout, i++);
    alder_uf_delete(uf, 3);
    alder_uf_print(uf, stdout, i++);
    alder_uf_delete(uf, 11);
    alder_uf_print(uf, stdout, i++);
    alder_uf_delete(uf, 13);
    alder_uf_print(uf, stdout, i++);
    alder_uf_delete(uf, 5);
    alder_uf_print(uf, stdout, i++);
    alder_uf_delete(uf, 1);
    alder_uf_delete(uf, 2);
    alder_uf_delete(uf, 4);
    alder_uf_delete(uf, 7);
    alder_uf_delete(uf, 12);
    alder_uf_delete(uf, 14);
    alder_uf_delete(uf, 15);
    alder_uf_print(uf, stdout, i++);
    alder_uf_delete(uf, 16);
    alder_uf_print(uf, stdout, i++);
    alder_uf_delete(uf, 17);
    alder_uf_print(uf, stdout, i++);
    
    int s;
    item_t eFound = 0;
    s = alder_uf_find(uf, &eFound);
    assert(s == ALDER_ERROR_NOTEXIST);
    
    alder_uf_destroy(uf);
#endif
    return 0;
}
