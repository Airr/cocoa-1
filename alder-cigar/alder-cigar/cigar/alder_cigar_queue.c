/**
 * This file, alder_cigar_queue.c, is part of alder-cigar.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-cigar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-cigar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-cigar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_cigar_queue.h"

//
//  alder_cigar_dequeue.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/29/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alder_cigar_queue.h"

#pragma mark node

#define BLOCKSIZE 10000

static alder_cigar_queue_t **headblock = NULL;
static alder_cigar_queue_t *currentblock = NULL;
static alder_cigar_queue_t *freeblock = NULL;
static int size_left;
static size_t sizeblock = 0;

alder_cigar_queue_t *get_node()
{
    alder_cigar_queue_t *tmp = NULL;
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
            
            alder_cigar_queue_t **tmpHeadblock = realloc(headblock, sizeblock * sizeof(alder_cigar_queue_t *));
            if (tmpHeadblock == NULL)
            {
                return tmp;
            }
            headblock = tmpHeadblock;
            headblock[sizeblock - 1] = (alder_cigar_queue_t *) malloc(BLOCKSIZE * sizeof(alder_cigar_queue_t) );
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
void return_node(alder_cigar_queue_t *node)
{
    node->next = freeblock;
    freeblock = node;
}

void free_node()
{
    for (size_t i = 0; i < sizeblock; i++) {
        free(headblock[i]);
        headblock[i] = NULL;
    }
    free(headblock);
    headblock = NULL;
    currentblock = NULL;
    sizeblock = 0;
    freeblock = NULL;
}


#pragma mark queue

alder_cigar_queue_t * alder_cigar_queue_create ()
{
    alder_cigar_queue_t *entrypoint, *placeholder;
    //    entrypoint = (alder_cigar_queue_t *) malloc (sizeof (alder_cigar_queue_t));
    //    placeholder = (alder_cigar_queue_t *) malloc (sizeof (alder_cigar_queue_t));
    
    entrypoint = get_node();
    entrypoint->item.n = 0;
    placeholder = get_node();
    placeholder->item.n = 0;
    
    entrypoint->next = placeholder;
    placeholder->next = placeholder;
    return (entrypoint);
}

int alder_cigar_queue_empty (alder_cigar_queue_t * qu)
{
    return (qu->next == qu->next->next);
}

void alder_cigar_enqueue (alder_cigar_item_t x, alder_cigar_queue_t * qu)
{
    alder_cigar_queue_t *tmp, *new;
    
    tmp = qu->next;
    
    if (!alder_cigar_queue_empty(qu) && tmp->item.t == x.t) {
        tmp->item.n++;
    } else {
        new = get_node ();
        x.n = 1;
        new->item = x;
        //        tmp = qu->next;
        qu->next = new;
        new->next = tmp->next;
        tmp->next = new;
    }
}

alder_cigar_item_t alder_cigar_dequeue (alder_cigar_queue_t * qu)
{
    alder_cigar_queue_t *tmp;
    alder_cigar_item_t tmp_item;
    tmp = qu->next->next->next;
    qu->next->next->next = tmp->next;
    if (tmp == qu->next)
        qu->next = tmp->next;
    tmp_item = tmp->item;
    return_node (tmp);
    return (tmp_item);
}

alder_cigar_item_t alder_cigar_front_element (alder_cigar_queue_t * qu)
{
    return (qu->next->next->next->item);
}

void alder_cigar_remove_queue (alder_cigar_queue_t * qu)
{
    alder_cigar_queue_t *tmp;
    tmp = qu->next->next;
    while (tmp != qu->next)
    {
        qu->next->next = tmp->next;
        return_node (tmp);
        tmp = qu->next->next;
    }
    return_node (qu->next);
    return_node (qu);
}

size_t alder_cigar_size (alder_cigar_queue_t * qu)
{
    size_t n = 0;
    alder_cigar_queue_t *tmp;
    tmp = qu->next->next;
    while (tmp != qu->next)
    {
        tmp = tmp->next;
        n += tmp->item.n;
    }
    return n;
}

size_t alder_cigar_strsize (alder_cigar_queue_t * qu)
{
    size_t n = 0;
    alder_cigar_queue_t *tmp;
    tmp = qu->next->next;
    while (tmp != qu->next)
    {
        tmp = tmp->next;
        int number = tmp->item.n;
        size_t digits = 0; while (number != 0) { number /= 10; digits++; }
        n += (digits + 1);
    }
    return n;
}


