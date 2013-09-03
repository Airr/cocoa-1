//
//  trie_tree_list.c
//  brass-ds
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../brass-ds-redblack-trees/stack.h"
#include "trie_tree_list.h"

#pragma mark Tree Memory Management

#define BLOCKSIZE 10000

static trie_node_t **headblock = NULL;
static trie_node_t *currentblock = NULL;
static trie_node_t *freeblock = NULL;
int size_next;
static size_t sizeblock = 0;

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

void free_trie()
{
    free_node();
    free_stack();
}

#pragma mark Trie List Trees


trie_node_t *
create_trie (void)
{
    trie_node_t *tmp_node;
    tmp_node = get_node ();
    tmp_node->next = tmp_node->list = NULL;
    tmp_node->this_char = '\0';
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
        while (tmp_node->this_char != *query_next)
        {
            if (tmp_node->list == NULL)
                return (NULL);
            /* query string not found */
            else
                tmp_node = tmp_node->list;
        }
        tmp_node = tmp_node->next;
        query_next += 1;
    }
    /* reached end of query string */
    while (tmp_node->this_char != '\0')
    {
        if (tmp_node->list == NULL)
            return (NULL);
        /* query string not found */
        else
            tmp_node = tmp_node->list;
    }
    return ((object_t *) tmp_node->next);
}

int
insert (trie_node_t * trie, char *new_string, object_t * new_object)
{
    trie_node_t *tmp_node;
    char *query_next;
    int finished = 0;
    tmp_node = trie;
    query_next = new_string;
    /* first go as far as possible in existing trie */
    while (!finished) {
        /* follow list till matching character is found */
        while (tmp_node->this_char != *query_next && tmp_node->list != NULL) {
            tmp_node = tmp_node->list;
        }
        if (tmp_node->this_char == *query_next)
        {
            /* matching character found, might be last */
            if (*query_next != '\0')
            {
                /* not last. follow */
                tmp_node = tmp_node->next;
                query_next += 1;
            } else {
                /* insertion not possible, string already exists */
                return (-1);
            }
        } else {
            finished = 1;
        }
    }
    /* left existing trie, create new branch */
    tmp_node->list = get_node ();
    tmp_node = tmp_node->list;
    tmp_node->list = NULL;
    tmp_node->this_char = *query_next;
    
    while (*query_next != '\0')
    {
        query_next += 1;
        tmp_node->next = get_node ();
        tmp_node = tmp_node->next;
        tmp_node->list = NULL;
        tmp_node->this_char = *query_next;
    }
    tmp_node->next = (trie_node_t *) new_object;
    return (0);
}

object_t *
delete (trie_node_t * trie, char *delete_string)
{
    trie_node_t *tmp, *tmp_prev, *first_del, *last_undel;
    object_t *del_object;
    char *del_next;
    if (trie->list == NULL || *delete_string == '\0') {
        /* delete failed: trie empty */
        return (NULL);
    } else {
        /* trie not empty, can start */
        int finished = 0;
        int branch = 1;
        last_undel = tmp_prev = trie;
        first_del = tmp = trie->list;
        del_next = delete_string;
        while (!finished) {
            while (tmp->this_char != *del_next) {
                /* follow list to find matching character */
                if (tmp->list == NULL) {
                    /* none found */
                    return (NULL);
                } else {
                    /* deletion failed */
                    /* branching trie node */
                    tmp_prev = tmp;
                    tmp = tmp->list;
                    branch = 1;
                }
            }                   /* tmp has matching next character */
            if (branch || (tmp->list != NULL)) {
                /* update position where to start deleting */
                last_undel = tmp_prev;
                first_del = tmp;
                branch = 0;
            }
            if (*del_next == '\0') {
                 /* found deletion string */
                finished = 1;
            } else {
                del_next += 1;
                tmp_prev = tmp;
                tmp = tmp->next;
            }
        }
        /* reached the end. now unlink and delete path */
        del_object = (object_t *) tmp->next;
        tmp->next = NULL;         /* unlink del_object */
        if (first_del == last_undel->next)
            last_undel->next = first_del->list;
        else                      /* first_del == last_undel->list */
            last_undel->list = first_del->list;
        /* final path of nonbranching
         nodes unlinked */
        tmp = first_del;
        while (tmp != NULL) {
            /* follow path, return nodes */
            first_del = tmp->next;
            return_node (tmp);
            tmp = first_del;
        }
        return (del_object);
    }
}

void
remove_trie (trie_node_t * trie)
{
    trie_node_t *tmp_node;
    create_stack ();
    push (trie);
    while (!stack_empty ())
    {
        tmp_node = pop ();
        if (tmp_node->this_char != '\0')
            push (tmp_node->next);
        if (tmp_node->list != NULL)
            push (tmp_node->list);
        return_node (tmp_node);
    }
    free_trie();
}
