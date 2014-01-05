/**
 * This file, alder_node_memory_source.c, is part of alder-uc.
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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "alder_cmacro.h"

/* This function tests whether the memory is clean.
 * returns
 * true if the memory is clean, and false otherwise.
 */
static bool
FUNCTION(check_node)(struct MEMORY_TYPE *block)
{
    TYPE *t = NULL;
    if (block->head != NULL) {
        for (size_t i = 0; i < block->nhead; i++) {
            t = block->head[i];
            for (size_t j = 0; j < block->size; j++) {
                if (t->use == true) {
                    return false;
                }
            }
        }
    }
    return true;
}

static TYPE *
FUNCTION(get_node)(struct MEMORY_TYPE *block)
{
    TYPE *t = NULL;
    if (block->free != NULL) {
        t = block->free;
        block->free = block->free->next;
    } else {
        if (block->current  == NULL || block->size == 0) {
            block->nhead++;
            TYPE* *th = realloc(block->head, block->nhead * sizeof(*th));
            if (th == NULL) {
                return NULL;
            }
            block->head = th;
            block->head[block->nhead-1] = malloc(BLOCKSIZE*sizeof(*block->head[0]));
            memset(block->head[block->nhead-1], 0, BLOCKSIZE*sizeof(*block->head[0]));
            if (block->head[block->nhead-1] == NULL) {
                return NULL;
            }
            block->current = block->head[block->nhead-1];
            block->size = BLOCKSIZE;
            
            assert(FUNCTION(check_node)(block) == true);
        }
        t = block->current++;
        block->size--;
    }
    /* Initialize the allocated one. */
    t->use = true;
    return t;
}

static void
FUNCTION(return_node)(struct MEMORY_TYPE *block, TYPE *node)
{
    if (node != NULL) {
        node->use = false;
        node->next = block->free;
        block->free = node;
    }
}

static void
FUNCTION(destroy_node)(struct MEMORY_TYPE *block)
{
    assert(FUNCTION(check_node)(block) == true);
    if (block->head != NULL) {
        for (size_t i = 0; i < block->nhead; i++) {
            XFREE(block->head[i]);
        }
        XFREE(block->head);
    }
    XFREE(block);
}

static struct MEMORY_TYPE *
FUNCTION(create_node)()
{
    struct MEMORY_TYPE *block = malloc(sizeof(*block));
    ALDER_RETURN_NULL_IF_NULL(block);
    memset(block,0,sizeof(*block));
    block->head    = NULL;
    block->current = NULL;
    block->free    = NULL;
    block->size    = 0;
    block->nhead   = 0;
    return block;
}

