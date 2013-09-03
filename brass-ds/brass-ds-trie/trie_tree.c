//
//  trie_tree.c
//  brass-ds
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie_tree.h"

#pragma mark Dynamic allocation of nodes

#define BLOCKSIZE 10000

static trie_node_t **headblock = NULL;
static trie_node_t *currentblock = NULL;
static trie_node_t *freeblock = NULL;
int size_next;
size_t sizeblock = 0;

trie_node_t *get_node()
{
    trie_node_t *tmp = NULL;
    if( freeblock != NULL )
    {
        tmp = freeblock;
        freeblock = freeblock->next;
    }
    else
    {
        if( currentblock == NULL || size_next == 0)
        {
            sizeblock++;
            
            trie_node_t **tmpHeadblock = realloc(headblock, sizeblock * sizeof(trie_node_t *));
            if (tmpHeadblock == NULL)
            {
                return tmp;
            }
            headblock = tmpHeadblock;
            headblock[sizeblock - 1] = (trie_node_t *) malloc(BLOCKSIZE * sizeof(trie_node_t) );
            if (headblock[sizeblock - 1] == NULL)
            {
                return tmp;
            }
            currentblock = headblock[sizeblock - 1];
            
            size_next = BLOCKSIZE;
        }
        tmp = currentblock++;
        size_next -= 1;
    }
    return( tmp );
}
void return_node(trie_node_t *node)
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

trie_node_t *
create_trie (void)
{
    trie_node_t *tmp_node;
    int i;
    tmp_node = get_node ();
    for (i = 0; i < 256; i++)
        tmp_node->next[i] = NULL;
    return (tmp_node);
}

object_t *
find (trie_node_t * trie, char *query_string)
{
    trie_node_t *tmp_node;
    char *query_next;
    tmp_node = trie;
    query_next = query_string;
    while (*query_next != '\0')
    {
        if (tmp_node->next[(int) (*query_next)] == NULL)
            return (NULL);          /* query string
                                     not found */
        else
        {
            tmp_node = tmp_node->next[(int) (*query_next)];
            query_next += 1;      /* move to next
                                   character of query */
        }
    }
    return ((object_t *) tmp_node->next[(int) '\0']);
}

int
insert (trie_node_t * trie, char *new_string, object_t * new_object)
{
    trie_node_t *tmp_node, *new_node;
    char *query_next;
    tmp_node = trie;
    query_next = new_string;
    while (*query_next != ‚Äô\0 ‚ Äô)
    {
        if (tmp_node->next[(int) (*query_next)] == NULL)
        {
            new_node = get_node ();
            /* create missing node */
            for (i = 0; i < 256; i++)
                new_node->next[i] = NULL;
            tmp_node->next[(int) (*query_next)] = new_node;
        }
        /* move to next character */
        tmp_node = tmp_node->next[(int) (*query_next)];
        query_next += 1;          /* move to
                                   next character */
    }
    if (tmp_node->next[(int) ‚Äô\0 ‚ Äô] != NULL)
        return (-1);                /* string already exists,
                                     has object */
    else
        tmp_node->next[(int) ‚Äô\0 ‚ Äô] = (trie_node_t *) new_object;
    return (0);
}

object_t *
delete (trie_node_t * trie, char *delete_string)
{
    trie_node_t *tmp_node;
    object_t *tmp_object;
    char *next_char;
    int finished = 0;
    
    create_stack ();
    tmp_node = trie;
    next_char = delete_string;
    while (*next_char != ‚Äô\0 ‚ Äô)
    {
        if (tmp_node->next[(int) (*next_char)] == NULL)
            return (NULL);          /* delete_string
                                     does not exist */
        else
        {
            tmp_node = tmp_node->next[(int) (*next_char)];
            next_char += 1;       /* move to next
                                   character */
            push (tmp_node);
        }}
    tmp_object = (object_t *) tmp_node->next[(int) ‚Äô\0 ‚ Äô];
    /* remove all nodes that became unnecessary */
    /* the root is not on the stack, so it is
     never deleted */
    while (!stack_empty () && !finished)
    {
        tmp_node = pop ();
        tmp_node->next[(int) (*next_char)] = NULL;
        for (i = 0; i < 256; i++)
            finished ||= (tmp_node->next[i] != NULL);
        /* if  tmp_node is all NULL,
         it should be deleted */
        if (!finished)
        {
            return_node (tmp_node);
            next_char -= 1;
        }
    }
    return (tmp_object);
}

void
remove_trie (trie_node_t * trie)
{
    trie_node_t *tmp_node;
    create_stack ();
    push (trie);
    while (!stack_empty ())
    {
        inti;
        tmp_node = pop ();
        for (i = 0; i < 256; i++)
        {
            if (tmp_node->next[i] != NULL && i != (int) ‚Äô\0 ‚ Äô)
                push (tmp_node->next[i]);
        }
        return_node (tmp_node);
    }
}
