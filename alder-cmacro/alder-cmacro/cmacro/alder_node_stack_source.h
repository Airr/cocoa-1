/**
 * This file, alder_node_stack_source.h, is part of alder-stack.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-stack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-stack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-stack.  If not, see <http://www.gnu.org/licenses/>.
*/

STACK_TYPE *
FUNCTION(alder_stack_create)(int size)
{
    STACK_TYPE *s;
    s = malloc(sizeof(*s));
    memset(s,0,sizeof(*s));
    s->base = malloc( size * sizeof(*s->base) );
    memset(s->base,0,size * sizeof(*s->base));
    s->size = size;
    s->top = s->base;
    s->prev = NULL;
    return s;
}

int
FUNCTION(alder_stack_empty)(STACK_TYPE *s)
{
    return s->base == s->top && s->prev == NULL;
}

void
FUNCTION(alder_stack_push)(STACK_TYPE *s, BASE *x)
{
    if ( s->top < s->base + s->size )
    {
        *(s->top) = x;
        s->top++;
    }
    else
    {
        STACK_TYPE *new;
        new = malloc(sizeof(*new));
        memset(new,0,sizeof(*new));
        new->base = s->base;
        new->top = s->top;
        new->size = s->size;
        new->prev = s->prev;
        s->prev = new;
        s->base = malloc(s->size * sizeof(*s->base));
        memset(s->base,0,s->size * sizeof(*s->base));
        s->top = s->base+1;
        *(s->base) = x;
    }
}

BASE*
FUNCTION(alder_stack_pop)(STACK_TYPE *s)
{
    if( s->top == s->base )
    {
        STACK_TYPE *old;
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

BASE*
FUNCTION(alder_stack_top)(STACK_TYPE *s)
{
    if (s->top == s->base)
        return *(s->prev->top - 1);
    else
        return *(s->top - 1);
}

void
FUNCTION(alder_stack_destroy)(STACK_TYPE *s)
{
    STACK_TYPE *tmp;
    do
    {
        tmp = s->prev;
        free(s->base);
        free(s);
        s = tmp;
    }
    while (s != NULL);
}