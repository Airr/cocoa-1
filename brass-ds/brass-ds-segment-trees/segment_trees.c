//
//  segment_trees.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/17/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "segment_trees.h"

#define BLOCKSIZE 2

#pragma mark Dynamic allocation of list nodes

static list_node_t **blockListHead = NULL;
static list_node_t *blockListCurrent = NULL;
static list_node_t *blockListFree = NULL;
static int blockListSizeLeft;
static size_t blockListSize = 0;
static int64_t refcountblockListSize = 0;

void checkListRC()
{
    assert(refcountblockListSize == 0);
}

list_node_t *blockListGetNode()
{
    refcountblockListSize++;
    list_node_t *tmp = NULL;
    if( blockListFree != NULL )
    {
        tmp = blockListFree;
        blockListFree = blockListFree->left;
    }
    else
    {
        if( blockListCurrent == NULL || blockListSizeLeft == 0)
        {
            blockListSize++;
            
            list_node_t **tmpblockListHead = realloc(blockListHead, blockListSize * sizeof(list_node_t *));
            if (tmpblockListHead == NULL)
            {
                return tmp;
            }
            blockListHead = tmpblockListHead;
            blockListHead[blockListSize - 1] = (list_node_t *) malloc(BLOCKSIZE * sizeof(list_node_t) );
            if (blockListHead[blockListSize - 1] == NULL)
            {
                return tmp;
            }
            blockListCurrent = blockListHead[blockListSize - 1];
            
            blockListSizeLeft = BLOCKSIZE;
        }
        tmp = blockListCurrent++;
        blockListSizeLeft -= 1;
    }
    tmp->key_a = -1;
    tmp->key_b = -1;
    tmp->left = NULL;
    return( tmp );
}

void blockListReturnNode(list_node_t *node)
{
    refcountblockListSize--;
    node->left = blockListFree;
    blockListFree = node;
}

void blockListFreeNode()
{
    if (blockListHead != NULL)
    {
        for (size_t i = 0; i < blockListSize; i++) {
            free(blockListHead[i]);
            blockListHead[i] = NULL;
        }
        free(blockListHead);
    }
    blockListHead = NULL;
    blockListCurrent = NULL;
    blockListSize = 0;
    blockListFree = NULL;
}

#pragma mark List

void remove_list(list_node_t *node)
{
    while (node != NULL)
    {
        list_node_t *t;
        t = node->left;
        blockListReturnNode(node);
        node = t;
    }
}

#pragma mark Dynamic allocation of tree nodes

static tree_node_t **blockTreeHead = NULL;
static tree_node_t *blockTreeCurrent = NULL;
static tree_node_t *blockTreeFree = NULL;
static int blockTreeSizeLeft;
static size_t blockTreeSize = 0;
static int64_t refcountblockTreeSize = 0;

void checkTreeRC()
{
    assert(refcountblockTreeSize == 0);
}

tree_node_t *blockTreeGetNode()
{
    refcountblockTreeSize++;
    tree_node_t *tmp = NULL;
    if( blockTreeFree != NULL )
    {
        tmp = blockTreeFree;
        blockTreeFree = blockTreeFree->left;
    }
    else
    {
        if( blockTreeCurrent == NULL || blockTreeSizeLeft == 0)
        {
            blockTreeSize++;
            
            tree_node_t **tmpblockTreeHead = realloc(blockTreeHead, blockTreeSize * sizeof(tree_node_t *));
            if (tmpblockTreeHead == NULL)
            {
                return tmp;
            }
            blockTreeHead = tmpblockTreeHead;
            blockTreeHead[blockTreeSize - 1] = (tree_node_t *) malloc(BLOCKSIZE * sizeof(tree_node_t) );
            if (blockTreeHead[blockTreeSize - 1] == NULL)
            {
                return tmp;
            }
            blockTreeCurrent = blockTreeHead[blockTreeSize - 1];
            
            blockTreeSizeLeft = BLOCKSIZE;
        }
        tmp = blockTreeCurrent++;
        blockTreeSizeLeft -= 1;
    }
    tmp->key = -1;
    tmp->left = NULL;
    tmp->right = NULL;
    tmp->interval_list = NULL;
    return( tmp );
}
void blockTreeReturnNode(tree_node_t *node)
{
    refcountblockTreeSize--;
    
    //    printf("free tree node: %llu [%llu]\n", node->key, refcountblockTreeSize);
    //    if (global_tree == node)
    //        printf("global tree was deallocated at %llu\n", refcountblockTreeSize);
    assert(node->interval_list == NULL);
    node->left = blockTreeFree;
    blockTreeFree = node;
}

void blockTreeFreeNode()
{
    if (blockTreeHead != NULL)
    {
        for (size_t i = 0; i < blockTreeSize; i++) {
            free(blockTreeHead[i]);
            blockTreeHead[i] = NULL;
        }
        free(blockTreeHead);
    }
    blockTreeHead = NULL;
    blockTreeCurrent = NULL;
    blockTreeSize = 0;
    blockTreeFree = NULL;
}

#pragma mark Segment Trees

list_node_t *find_intervals(tree_node_t *tree, key_t query_key)
{
    tree_node_t *current_tree_node;
    list_node_t *current_list, *result_list,
    *new_result;
    if( tree->left == NULL )   /* tree empty */
        return(NULL);
    else   /* tree nonempty, follow search path */
    {
        current_tree_node = tree;
        result_list = NULL;
        while( current_tree_node->right != NULL )
        {
            if( query_key < current_tree_node->key )
                current_tree_node = current_tree_node->left;
            else
                current_tree_node = current_tree_node->right;
            current_list = current_tree_node->interval_list;
            while( current_list != NULL )
            {
                /* copy entry from node list to result list */
                new_result = blockListGetNode();
                new_result->left = result_list;
                new_result->key_a = current_list->key_a;
                new_result->key_b = current_list->key_b;
                new_result->object = current_list->object;
                result_list = new_result;
                current_list = current_list->left;
            }
        }
        return( result_list );
    }
}

void attach_intv_node(tree_node_t *tree_node, key_t a, key_t b, object_t *object)
{
    list_node_t *new_node;
    new_node = blockListGetNode();
    new_node->left = tree_node->interval_list;
    new_node->key_a = a;
    new_node->key_b = b;
    new_node->object = object;
    tree_node->interval_list = new_node;
}


void insert_interval(tree_node_t *tree, key_t a, key_t b, object_t *object)
{
    tree_node_t *current_node, *right_path, *left_path;
    if( tree->left == NULL )
        exit(-1); /* tree incorrect */
    else
    {
        current_node = tree;
        right_path = left_path = NULL;
        while( current_node->right != NULL ) /* not at leaf */
        {
            if( b < current_node->key ) /* go left: a < b < key */
                current_node = current_node->left;
            else if( current_node->key < a) /* go right: key < b < a */
                current_node = current_node->right;
            else if( a < current_node->key && current_node->key < b )
            /* split: a < key < b */
            {
                right_path = current_node->right; /* both right */
                left_path  = current_node->left; /* and left */
                break;
            }
            else if( a == current_node->key ) /* a = key < b */
            {
                right_path = current_node->right; /* no left */
                break;
            }
            else /*   current_node->key == b, so a < key = b */
            {
                left_path  = current_node->left; /* no right */
                break;
            }
        }
        if( left_path != NULL )
        {
            /* now follow the path of the left endpoint a*/
            while( left_path->right != NULL )
            {
                if( a < left_path->key )
                {
                    /* right node must be selected */
                    attach_intv_node(left_path-> right, a,b,object);
                    left_path = left_path->left;
                }
                else if ( a == left_path->key )
                {
                    attach_intv_node(left_path->right, a,b,object);
                    break; /* no further descent necessary */
                }
                else /* go right, no node selected */
                    left_path = left_path->right;
            }
            /* left leaf of a needs to be selected if reached */
            if( left_path->right == NULL && left_path->key == a )
                attach_intv_node(left_path, a,b,object);
        } /* end left path */
        if( right_path != NULL )
        {
            /* and now follow the path of the right endpoint b */
            while( right_path->right != NULL )
            {
                if( right_path->key < b )
                { /* left node must be selected */
                    attach_intv_node(right_path->left, a,b, object);
                    right_path = right_path->right;
                }
                else if ( right_path->key == b)
                {
                    attach_intv_node(right_path->left, a,b, object);
                    break; /* no further descent necessary */
                }
                else /* go left, no node selected */
                    right_path = right_path->left;
            }
            /* on the right side, the leaf of b is never attached */
        }   /* end right path */
    }
}

int insertInterval(tree_node_t *tree, object_t *newIntervals, size_t sizeInterval)
{
    for (size_t i = 0; i < sizeInterval; i++) {
        insert_interval(tree, newIntervals[i].b, newIntervals[i].e, NULL);
    }
    return 1;
}
#pragma mark Search Trees

tree_node_t *create_tree(void)
{
    tree_node_t *tmp_node;
    tmp_node = blockTreeGetNode();
    tmp_node->left = NULL;
    return( tmp_node );
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

int insert(tree_node_t *tree, key_t new_key, tree_node_t *new_object)
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
        /* found the candidate leaf. Test whether key distinct */
        if( tmp_node->key == new_key )
            return( -1 );
        /* key is distinct, now perform the insert */
        {
            tree_node_t *old_leaf, *new_leaf;
            old_leaf = blockTreeGetNode();
            old_leaf->left = tmp_node->left;
            old_leaf->key = tmp_node->key;
            old_leaf->right  = NULL;
            new_leaf = blockTreeGetNode();
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
            blockTreeReturnNode( tmp_node );
            blockTreeReturnNode( other_node );
            return( deleted_object );
        }
    }
}

void remove_tree(tree_node_t *tree)
{
    tree_node_t *current_node, *tmp;
    if( tree->left == NULL )
        blockTreeReturnNode( tree );
    else
    {
        current_node = tree;
        while(current_node->right != NULL )
        {
            if( current_node->left->right == NULL )
            {
                blockTreeReturnNode( current_node->left->left);
                remove_list( current_node->left->interval_list);
                current_node->left->interval_list = NULL;
                blockTreeReturnNode( current_node->left );
                tmp = current_node->right;
                remove_list( current_node->interval_list);
                current_node->interval_list = NULL;
                blockTreeReturnNode( current_node );
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
        blockTreeReturnNode( current_node->left);
        blockTreeReturnNode( current_node );
    }
}

