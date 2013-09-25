/**
 * This file, alder_avl_dictionary_interval.c, is part of alder-avl.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-avl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-avl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-avl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rb.h"
#include "alder_avl_dictionary_interval.h"

/* Insertion order. */
enum insert_order
{
    INS_RANDOM,            /* Random order. */
    INS_ASCENDING,         /* Ascending order. */
    INS_DESCENDING,        /* Descending order. */
    INS_BALANCED,          /* Balanced tree order. */
    INS_ZIGZAG,            /* Zig-zag order. */
    INS_ASCENDING_SHIFTED, /* Ascending from middle, then beginning. */
    INS_CUSTOM,            /* Custom order. */
    INS_CNT                /* Number of insertion orders. */
};

/* Deletion order. */
enum delete_order
{
    DEL_RANDOM,      /* Random order. */
    DEL_REVERSE,     /* Reverse of insertion order. */
    DEL_SAME,        /* Same as insertion order. */
    DEL_CUSTOM,      /* Custom order. */
    DEL_CNT          /* Number of deletion orders. */
};

/* Memory tracking policy. */
enum mt_policy
{
    MT_TRACK,        /* Track allocation for leak detection. */
    MT_NO_TRACK,     /* No leak detection. */
    MT_FAIL_COUNT,   /* Fail allocations after a while. */
    MT_FAIL_PERCENT, /* Fail allocations randomly. */
    MT_SUBALLOC      /* Suballocate from larger blocks. */
};

/* Prints |message| on |stderr|, which is formatted as for |printf()|,
 and terminates the program unsuccessfully. */
static void
fail (const char *message, ...)
{
    va_list args;
    
    fprintf (stderr, "alder-avl: ");
    
    va_start (args, message);
    vfprintf (stderr, message, args);
    va_end (args);
    
    putchar ('\n');
    
    exit (EXIT_FAILURE);
}

/* Allocates and returns a pointer to |size| bytes of memory.
 Aborts if allocation fails. */
static void *
xmalloc (size_t size)
{
    void *block = malloc (size);
    if (block == NULL && size != 0)
        fail ("out of memory");
    return block;
}

/* Memory tracking allocator. */

/* A memory block. */
struct block
{
    struct block *next;                 /* Next in linked list. */
    
    int idx;                            /* Allocation order index number. */
    size_t size;                        /* Size in bytes. */
    size_t used;                        /* MT_SUBALLOC: amount used so far. */
    void *content;                      /* Allocated region. */
};

/* Indexes into |arg[]| within |struct mt_allocator|. */
enum mt_arg_index
{
    MT_COUNT = 0,      /* |MT_FAIL_COUNT|: Remaining successful allocations. */
    MT_PERCENT = 0,    /* |MT_FAIL_PERCENT|: Failure percentage. */
    MT_BLOCK_SIZE = 0, /* |MT_SUBALLOC|: Size of block to suballocate. */
    MT_ALIGN = 1       /* |MT_SUBALLOC|: Alignment of suballocated blocks. */
};

/* Memory tracking allocator. */
struct mt_allocator
{
    struct libavl_allocator allocator;  /* Allocator.  Must be first member. */
    
    /* Settings. */
    enum mt_policy policy;              /* Allocation policy. */
    int arg[2];                         /* Policy arguments. */
    int verbosity;                      /* Message verbosity level. */
    
    /* Current state. */
    struct block *head, *tail;          /* Head and tail of block list. */
    int alloc_idx;                      /* Number of allocations so far. */
    int block_cnt;                      /* Number of still-allocated blocks. */
};

static void *mt_allocate (struct libavl_allocator *, size_t);
static void mt_free (struct libavl_allocator *, void *);

/* Initializes the memory manager for use
 with allocation policy |policy| and policy arguments |arg[]|,
 at verbosity level |verbosity|, where 0 is a ``normal'' value. */
struct mt_allocator *
mt_create (enum mt_policy policy, int arg[2], int verbosity)
{
    struct mt_allocator *mt = xmalloc (sizeof *mt);
    
    mt->allocator.libavl_malloc = mt_allocate;
    mt->allocator.libavl_free = mt_free;
    
    mt->policy = policy;
    mt->arg[0] = arg[0];
    mt->arg[1] = arg[1];
    mt->verbosity = verbosity;
    
    mt->head = mt->tail = NULL;
    mt->alloc_idx = 0;
    mt->block_cnt = 0;
    
    return mt;
}

/* Frees and destroys memory tracker |mt|,
 reporting any memory leaks. */
void
mt_destroy (struct mt_allocator *mt)
{
    assert (mt != NULL);
    
    if (mt->block_cnt == 0)
    {
        if (mt->policy != MT_NO_TRACK && mt->verbosity >= 1)
            printf ("  No memory leaks.\n");
    }
    else
    {
        struct block *iter, *next;
        
        if (mt->policy != MT_SUBALLOC)
            printf ("  Memory leaks detected:\n");
        for (iter = mt->head; iter != NULL; iter = next)
        {
            if (mt->policy != MT_SUBALLOC)
                printf ("    block #%d: %lu bytes\n",
                        iter->idx, (unsigned long) iter->size);
            
            next = iter->next;
            free (iter->content);
            free (iter);
        }
    }
    
    free (mt);
}

/* Returns the |struct libavl_allocator| associated with |mt|. */
void *
mt_allocator (struct mt_allocator *mt)
{
    return &mt->allocator;
}

/* Creates a new |struct block| containing |size| bytes of content
 and returns a pointer to content. */
static void *
new_block (struct mt_allocator *mt, size_t size)
{
    struct block *new;
    
    /* Allocate and initialize new |struct block|. */
    new = xmalloc (sizeof *new);
    new->next = NULL;
    new->idx = mt->alloc_idx++;
    new->size = size;
    new->used = 0;
    new->content = xmalloc (size);
    
    /* Add block to linked list. */
    if (mt->head == NULL)
        mt->head = new;
    else
        mt->tail->next = new;
    mt->tail = new;
    
    /* Alert user. */
    if (mt->verbosity >= 3)
        printf ("    block #%d: allocated %lu bytes\n",
                new->idx, (unsigned long) size);
    
    /* Finish up and return. */
    mt->block_cnt++;
    return new->content;
}

/* Prints a message about a rejected allocation if appropriate. */
static void
reject_request (struct mt_allocator *mt, size_t size)
{
    if (mt->verbosity >= 2)
        printf ("    block #%d: rejected request for %lu bytes\n",
                mt->alloc_idx++, (unsigned long) size);
}

/* Allocates and returns a block of |size| bytes. */
static void *
mt_allocate (struct libavl_allocator *allocator, size_t size)
{
    struct mt_allocator *mt = (struct mt_allocator *) allocator;
    
    /* Special case. */
    if (size == 0)
        return NULL;
    
    switch (mt->policy)
    {
        case MT_TRACK:
            return new_block (mt, size);
            
        case MT_NO_TRACK:
            return xmalloc (size);
            
        case MT_FAIL_COUNT:
            if (mt->arg[MT_COUNT] == 0)
            {
                reject_request (mt, size);
                return NULL;
            }
            mt->arg[MT_COUNT]--;
            return new_block (mt, size);
            
        case MT_FAIL_PERCENT:
            if (rand () / (RAND_MAX / 100 + 1) < mt->arg[MT_PERCENT])
            {
                reject_request (mt, size);
                return NULL;
            }
            else
                return new_block (mt, size);
            
        case MT_SUBALLOC:
            if (mt->tail == NULL
                || mt->tail->used + size > (size_t) mt->arg[MT_BLOCK_SIZE])
                new_block (mt, mt->arg[MT_BLOCK_SIZE]);
            if (mt->tail->used + size <= (size_t) mt->arg[MT_BLOCK_SIZE])
            {
                void *p = (char *) mt->tail->content + mt->tail->used;
                size = ((size + mt->arg[MT_ALIGN] - 1)
                        / mt->arg[MT_ALIGN] * mt->arg[MT_ALIGN]);
                mt->tail->used += size;
                if (mt->verbosity >= 3)
                    printf ("    block #%d: suballocated %lu bytes\n",
                            mt->tail->idx, (unsigned long) size);
                return p;
            }
            else
                fail ("blocksize %lu too small for %lu-byte allocation",
                      (unsigned long) mt->tail->size, (unsigned long) size);
            
        default:
            assert (0);
    }
}

/* Releases |block| previously returned by |mt_allocate()|. */
static void
mt_free (struct libavl_allocator *allocator, void *block)
{
    struct mt_allocator *mt = (struct mt_allocator *) allocator;
    struct block *iter, *prev;
    
    /* Special cases. */
    if (block == NULL || mt->policy == MT_NO_TRACK)
    {
        free (block);
        return;
    }
    if (mt->policy == MT_SUBALLOC)
        return;
    
    /* Search for |block| within the list of allocated blocks. */
    for (prev = NULL, iter = mt->head; iter; prev = iter, iter = iter->next)
    {
        if (iter->content == block)
        {
            /* Block found.  Remove it from the list. */
            struct block *next = iter->next;
            
            if (prev == NULL)
                mt->head = next;
            else
                prev->next = next;
            if (next == NULL)
                mt->tail = prev;
            
            /* Alert user. */
            if (mt->verbosity >= 4)
                printf ("    block #%d: freed %lu bytes\n",
                        iter->idx, (unsigned long) iter->size);
            
            /* Free block. */
            free (iter->content);
            free (iter);
            
            /* Finish up and return. */
            mt->block_cnt--;
            return;
        }
    }
    
    /* Block not in list. */
    printf ("    attempt to free unknown block %p (already freed?)\n", block);
}

#pragma mark print

static void
print_tree_structure (const struct rb_node *node, int level)
{
    /* You can set the maximum level as high as you like.
     Most of the time, you'll want to debug code using small trees,
     so that a large |level| indicates a ``loop'', which is a bug. */
    if (level > 16)
    {
        printf ("[...]");
        return;
    }
    
    if (node == NULL)
        return;
    
    alder_avl_dictionary_interval_t *p = node->rb_data;
    printf ("[%lld,%lld]", p->a, p->b);
    if (node->rb_link[0] != NULL || node->rb_link[1] != NULL)
    {
        putchar ('(');
        
        print_tree_structure (node->rb_link[0], level + 1);
        if (node->rb_link[1] != NULL)
        {
            putchar (',');
            print_tree_structure (node->rb_link[1], level + 1);
        }
        
        putchar (')');
    }
}

/* Prints the entire structure of |tree| with the given |title|. */
void
print_whole_tree (const struct rb_table *tree, const char *title)
{
    printf ("%s: ", title);
    print_tree_structure (tree->rb_root, 0);
    putchar ('\n');
}

#pragma mark traversal

static int
test_bst_t_next (struct rb_table *tree)
{
    struct rb_traverser trav;
    
    rb_t_first (&trav, tree);
    alder_avl_dictionary_interval_t *p = NULL;
    p = rb_t_cur(&trav);
    while (p != NULL) {
        printf ("[%lld,%lld]", p->a, p->b);
        putchar (' ');
        p = rb_t_next (&trav);
    }
    putchar ('\n');
    
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// My own functions
///////////////////////////////////////////////////////////////////////////////
#pragma mark alder

int
compare_interval (const void *pa, const void *pb, void *param)
{
    const alder_avl_dictionary_interval_t *a = pa;
    const alder_avl_dictionary_interval_t *b = pb;
    
    if (a->a < b->a) {
        return -1;
    } else if (a->a > b->a) {
        return +1;
    } else {
        if (a->b < b->b) {
            return -1;
        } else if (a->b > b->b) {
            return +1;
        } else {
            return 0;
        }
    }
}


struct rb_table*
alder_avl_dictionary_interval_create()
{
    struct rb_table *tree = NULL;
    //    int okay = 1;
    //    int i;
    
    /* Test creating a RB and inserting into it. */
    //    tree = rb_create (compare_interval, NULL, allocator);
    
    
    return tree;
}

int alder_avl_dictionary_interval_destroy(struct rb_table *p)
{
    
    return 0;
}

int alder_avl_dictionary_interval_insert(int64_t a, int64_t b)
{
    
    return 0;
}

int alder_avl_dictionary_interval()
{
    // in test.c
    int alloc_arg[2];
    alloc_arg[0] = 0;
    alloc_arg[1] = 0;
    struct mt_allocator *mta;
    mta = mt_create(MT_NO_TRACK, alloc_arg, 1);
    struct libavl_allocator *allocator = mt_allocator (mta);
    
    // Interval data
    alder_avl_dictionary_interval_t dataInterval[4];
    dataInterval[0].a = 2;
    dataInterval[0].b = 3;
    dataInterval[1].a = 9;
    dataInterval[1].b = 17;
    dataInterval[2].a = 9;
    dataInterval[2].b = 11;
    dataInterval[3].a = 9;
    dataInterval[3].b = 11;
    
    // *-test.c such as bst-test.c
    struct rb_table *tree;
    tree = rb_create (compare_interval, NULL, allocator);
    
    void **p = rb_probe(tree, &dataInterval[0]);
    p = rb_probe(tree, &dataInterval[1]);
    p = rb_probe(tree, &dataInterval[2]);
    p = rb_probe(tree, &dataInterval[3]);
    if (*p != &dataInterval[3]) {
        fprintf(stdout, "Already in the dictoinary\n");
    }
    
    // what if I found one?
    
    print_whole_tree(tree, "dictionary of intervals");
    test_bst_t_next (tree);
    
    rb_destroy(tree, NULL);
    
    // back to test.c
    mt_destroy(mta);
    return 0;
}










