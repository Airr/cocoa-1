//
//  stack.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "stack.h"

stack_t *create_stack(int size)
{
    stack_t *st;
    st = (stack_t *) malloc( sizeof(stack_t) );
    st->base = (item_t *) malloc( size * sizeof(item_t) );
    st->size = size;
    st->top = st->base;
    st->previous = NULL;
    return( st );
}

int stack_empty(stack_t *st)
{
    return( st->base == st->top && st->previous == NULL);
}

void push( item_t x, stack_t *st)
{
    if ( st->top < st->base + st->size )
    {
        *(st->top) = x;
        st->top += 1;
    }
    else
    {
        stack_t *new;
        new = (stack_t *) malloc( sizeof(stack_t) );
        new->base = st->base;
        new->top = st->top;
        new->size = st->size;
        new->previous = st->previous;
        st->previous = new;
        st->base = (item_t *) malloc( st->size * sizeof(item_t) );
        st->top = st->base+1;
        *(st->base) = x;
    }
}

item_t pop(stack_t *st)
{
    if( st->top == st->base )
    {
        stack_t *old;
        old = st->previous;
        st->previous = old->previous;
        free( st->base );
        st->base = old->base;
        st->top = old->top;
        st->size = old->size;
        free( old );
    }
    st->top -= 1;
    return( *(st->top) );
}

item_t top_element(stack_t *st)
{
    if( st->top == st->base )
        return( *(st->previous->top -1) );
    else
        return( *(st->top -1) );
}

void remove_stack(stack_t *st)
{
    stack_t *tmp;
    do
    {
        tmp = st->previous;
        free( st->base );
        free( st );
        st = tmp;
    }
    while( st != NULL );
}
