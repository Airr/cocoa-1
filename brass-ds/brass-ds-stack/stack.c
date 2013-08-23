//
//  stack.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

#pragma mark Dynamic allocation of nodes

#define BLOCKSIZE 2

static node_t **headblock = NULL;
static node_t *currentblock = NULL;
static int size_left;
static size_t sizeblock = 0;
static node_t *freeblock = NULL;

static node_t *get_node()
{
    node_t *tmp = NULL;
    if( freeblock != NULL )
    {
        tmp = freeblock;
        freeblock = freeblock->next;
    }
    else
    {
        if( currentblock == NULL || size_left == 0)
        {
            sizeblock++;
            
            node_t **tmpHeadblock = realloc(headblock, sizeblock * sizeof(node_t *));
            if (tmpHeadblock == NULL)
            {
                return tmp;
            }
            headblock = tmpHeadblock;
            headblock[sizeblock - 1] = (node_t *) malloc(BLOCKSIZE * sizeof(node_t) );
            if (headblock[sizeblock - 1] == NULL)
            {
                return tmp;
            }
            currentblock = headblock[sizeblock - 1];
            
            size_left = BLOCKSIZE;
        }
        tmp = currentblock++;
        size_left -= 1;
    }
    return( tmp );
}

static void return_node(node_t *node)
{
    node->next = freeblock;
    freeblock = node;
}

static void free_node()
{
    if (headblock != NULL)
    {
        for (size_t i = 0; i < sizeblock; i++) {
            free(headblock[i]);
            headblock[i] = NULL;
        }
        free(headblock);
        headblock = NULL;
    }
    currentblock = NULL;
//    size_left;
    sizeblock = 0;
    freeblock = NULL;
}



#pragma mark Stack

node_t *create_stack(void)
{
    node_t *st;
    st = get_node();
    st->next = NULL;
    return( st );
}

int stack_empty(node_t *st)
{
    return( st->next == NULL );
}

void push( item_t x, node_t *st)
{
    node_t *tmp;
    tmp = get_node();
    tmp->item = x;
    tmp->next = st->next;
    st->next = tmp;
}

item_t pop(node_t *st)
{
    node_t *tmp;
    item_t tmp_item;
    tmp = st->next;
    st->next = tmp->next;
    tmp_item = tmp->item;
    return_node( tmp );
    return( tmp_item );
}

item_t top_element(node_t *st)
{
    return( st->next->item );
}

void remove_stack(node_t *st)
{
    node_t *tmp;
    do
    {
        tmp = st->next;
        return_node(st);
        st = tmp;
    }
    while ( tmp != NULL );
}

void free_stack()
{
    free_node();
}
