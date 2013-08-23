//
//  search_trees.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "search_trees.h"

#pragma mark Dynamic allocation of nodes

#define BLOCKSIZE 10000

static tree_node_t **headblock = NULL;
static tree_node_t *currentblock = NULL;
static tree_node_t *freeblock = NULL;
int size_left;
size_t sizeblock = 0;

tree_node_t *get_node()
{
    tree_node_t *tmp = NULL;
    if( freeblock != NULL )
    {
        tmp = freeblock;
        freeblock = freeblock->left;
    }
    else
    {
        if( currentblock == NULL || size_left == 0)
        {
            sizeblock++;
            
            tree_node_t **tmpHeadblock = realloc(headblock, sizeblock * sizeof(tree_node_t *));
            if (tmpHeadblock == NULL)
            {
                return tmp;
            }
            headblock = tmpHeadblock;
            headblock[sizeblock - 1] = (tree_node_t *) malloc(BLOCKSIZE * sizeof(tree_node_t) );
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
void return_node(tree_node_t *node)
{
    node->left = freeblock;
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

#pragma mark Search Trees

tree_node_t *create_tree(void)
{
    tree_node_t *tmp_node;
    tmp_node = get_node();
    tmp_node->left = NULL;
    return( tmp_node );
}

void left_rotation(tree_node_t *n)
{
    tree_node_t *tmp_node;
    key_t        tmp_key;
    tmp_node = n->left;
    tmp_key  = n->key;
    n->left  = n->right;
    n->key   = n->right->key;
    n->right = n->left->right;
    n->left->right = n->left->left;
    n->left->left  = tmp_node;
    n->left->key   = tmp_key;
}

void right_rotation(tree_node_t *n)
{
    tree_node_t *tmp_node;
    key_t        tmp_key;
    tmp_node = n->right;
    tmp_key  = n->key;
    n->right = n->left;
    n->key   = n->left->key;
    n->left  = n->right->left;
    n->right->left = n->right->right;
    n->right->right  = tmp_node;
    n->right->key = tmp_key;
}

object_t *find(tree_node_t *tree, key_t query_key)
{
    tree_node_t *tmp_node;
    if( tree->left == NULL )
        return(NULL);
    else
    {
        tmp_node = tree;
        while( tmp_node->right != NULL )
        {
            if( query_key < tmp_node->key )
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
        }
        if( tmp_node->key == query_key )
            return( (object_t *) tmp_node->left );
        else
            return( NULL );
    }
}

object_t *findWithRecursion(tree_node_t *tree, key_t query_key)
{
    if( tree->left == NULL || (tree->right == NULL && tree->key != query_key ) )
        return(NULL);
    else if (tree->right == NULL && tree->key == query_key )
        return( (object_t *) tree->left );
    else
    {
        if( query_key < tree->key )
            return( find(tree->left, query_key) );
        else
            return( find(tree->right, query_key) );
    }
}

int insert(tree_node_t *tree, key_t new_key, object_t *new_object)
{
    tree_node_t *tmp_node;
    if( tree->left == NULL )
    {
        tree->left = (tree_node_t *) new_object;
        tree->key  = new_key;
        tree->right  = NULL;
    }
    else
    {
        tmp_node = tree;
        while( tmp_node->right != NULL )
        {
            if( new_key < tmp_node->key )
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
        }
        /* found the candidate leaf. Test whether
         key distinct */
        if( tmp_node->key == new_key )
            return( -1 );
        /* key is distinct, now perform the insert */
        {
            tree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node();
            old_leaf->left = tmp_node->left;
            old_leaf->key = tmp_node->key;
            old_leaf->right  = NULL;
            new_leaf = get_node();
            new_leaf->left = (tree_node_t *) new_object;
            new_leaf->key = new_key;
            new_leaf->right  = NULL;
            if( tmp_node->key < new_key )
            {
                tmp_node->left  = old_leaf;
                tmp_node->right = new_leaf;
                tmp_node->key = new_key;
            }
            else
            {
                tmp_node->left  = new_leaf;
                tmp_node->right = old_leaf;
            }
        }
    }
    return( 0 );
}

object_t *delete(tree_node_t *tree, key_t delete_key)
{
    tree_node_t *tmp_node, *upper_node, *other_node;
    object_t *deleted_object;
    if( tree->left == NULL )
        return( NULL );
    else if( tree->right == NULL )
    {
        if(  tree->key == delete_key )
        {
            deleted_object = (object_t *) tree->left;
            tree->left = NULL;
            return( deleted_object );
        }
        else
            return( NULL );
    }
    else
    {
        tmp_node = tree;
        while( tmp_node->right != NULL )
        {
            upper_node = tmp_node;
            if( delete_key < tmp_node->key )
            {
                tmp_node   = upper_node->left;
                other_node = upper_node->right;
            }
            else
            {
                tmp_node   = upper_node->right;
                other_node = upper_node->left;
            }
        }
        if( tmp_node->key != delete_key )
            return( NULL );
        else
        {
            upper_node->key   = other_node->key;
            upper_node->left  = other_node->left;
            upper_node->right = other_node->right;
            deleted_object = (object_t *) tmp_node->left;
            return_node( tmp_node );
            return_node( other_node );
            return( deleted_object );
        }
    }
}

void remove_tree(tree_node_t *tree)
{
    tree_node_t *current_node, *tmp;
    if( tree->left == NULL )
        return_node( tree );
    else
    {
        current_node = tree;
        while(current_node->right != NULL )
        {
            if( current_node->left->right == NULL )
            {
                return_node( current_node->left );
                tmp = current_node->right;
                return_node( current_node );
                current_node = tmp;
            }
            else
            {
                tmp = current_node->left;
                current_node->left = tmp->right;
                tmp->right = current_node;
                current_node = tmp;
            }
        }
        return_node( current_node );
    }
}

