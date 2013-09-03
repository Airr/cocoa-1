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

#define BLOCKSIZE 10000

static stack_node_t *st = NULL;
static stack_node_t **headblock = NULL;
static stack_node_t *currentblock = NULL;
static stack_node_t *freeblock = NULL;
static size_t sizeblock = 0;
static int size_left;

static stack_node_t *get_node()
{
    stack_node_t *tmp = NULL;
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
            
            stack_node_t **tmpHeadblock = realloc(headblock, sizeblock * sizeof(stack_node_t *));
            if (tmpHeadblock == NULL)
            {
                return tmp;
            }
            headblock = tmpHeadblock;
            headblock[sizeblock - 1] = (stack_node_t *) malloc(BLOCKSIZE * sizeof(stack_node_t) );
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

static void return_node(stack_node_t *node)
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
    sizeblock = 0;
    freeblock = NULL;
    st = NULL;
    //    size_left;
}

void create_stack(void)
{
    if (st == NULL)
    {
        st = get_node();
        st->next = NULL;
    }
    else
    {
        fprintf(stderr, "fatal: stack is already created.");
    }
}

int stack_empty(void)
{
    return( st->next == NULL );
}

void push( item_t x)
{
    stack_node_t *tmp;
    tmp = get_node();
    tmp->item = x;
    tmp->next = st->next;
    st->next = tmp;
}

item_t pop(void)
{
    stack_node_t *tmp;
    item_t tmp_item;
    tmp = st->next;
    st->next = tmp->next;
    tmp_item = tmp->item;
    return_node( tmp );
    return( tmp_item );
}

item_t top_element(void)
{
    return( st->next->item );
}

void remove_stack(void)
{
    stack_node_t *tmp;
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
