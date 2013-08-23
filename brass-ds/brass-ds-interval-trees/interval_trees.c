//
//  interval_trees.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "interval_trees.h"

extern tree_node_t *global_tree;

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
    tmp->key = -1;
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
    tmp->left_list = NULL;
    tmp->right_list = NULL;
    return( tmp );
}
void blockTreeReturnNode(tree_node_t *node)
{
    refcountblockTreeSize--;
    
//    printf("free tree node: %llu [%llu]\n", node->key, refcountblockTreeSize);
//    if (global_tree == node)
//        printf("global tree was deallocated at %llu\n", refcountblockTreeSize);

//    assert(node->left_list == NULL && node->right_list == NULL);
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

#pragma mark Interval Trees

list_node_t *find_intervals(tree_node_t *tree, key_t query_key)
{
    tree_node_t *current_tree_node;
    list_node_t *current_list, *result_list,
    *new_result;
    if( tree->left == NULL )
        return(NULL);
    else
    {
        current_tree_node = tree;
        result_list = NULL;
        while( current_tree_node->right != NULL )
        {
            if( query_key < current_tree_node->key )
            {
                current_list = current_tree_node->left_list;
                while( current_list != NULL && current_list->key <= query_key )
                {
                    new_result = blockListGetNode();
                    new_result->left = result_list;
                    new_result->object = current_list->object;
                    result_list = new_result;
                    current_list = current_list->left;
                }
                current_tree_node = current_tree_node->left;
            }
            else
            {
                current_list = current_tree_node->right_list;
                while( current_list != NULL && current_list->key >= query_key )
                {
                    new_result = blockListGetNode();
                    new_result->left = result_list;
                    new_result->object = current_list->object;
                    result_list = new_result;
                    current_list = current_list->left;
                }
                current_tree_node = current_tree_node->right;
            }
        }
        return( result_list );
    }
}


// sort newIntervals by .b and add them to the left side.
// sort newIntervals by .e and add them to the right side.
//
// To build it from a given list of n intervals, we first build the search tree for
// the interval endpoints in O(nlogn) time. Next we construct a list of the
// intervals sorted in decreasing order of their left interval endpoints, in O
// (n log n), and find for each interval the node where it should be stored, and
// insert it there in front of the left list, in O(logn) per interval. Finally, we
// construct a list of the intervals sorted in increasing order of their right
// interval endpoints, in O(nlogn), and find for each interval the node where it
// should be stored, and insert it there in front of the right list, in O (log n)
// per interval. By this initial sorting and inserting in that order, all node
// lists are in the correct order.
int compareIntervalByBegin(const void *a,const void *b) {
    object_t *x = (object_t *) a;
    object_t *y = (object_t *) b;
    if (x->b > y->b)
        return -1;
    else if (x->b < y->b)
        return 1;
    return 0;
}

int compareIntervalByEnd(const void *a,const void *b) {
    object_t *x = (object_t *) a;
    object_t *y = (object_t *) b;
    if (x->e < y->e)
        return -1;
    else if (x->e > y->e)
        return 1;
    return 0;
}

int insertInterval(tree_node_t *tree, object_t *newIntervals, size_t sizeInterval)
{
    tree_node_t *current_tree_node;
    
    for (size_t isRight = 0; isRight < 2; isRight++) {
        if (isRight == 0)
            qsort(newIntervals, sizeInterval, sizeof(object_t), compareIntervalByBegin);
        else
            qsort(newIntervals, sizeInterval, sizeof(object_t), compareIntervalByEnd);
        
        for (size_t i = 0; i < sizeInterval; i++) {
            object_t *anInterval = newIntervals + i;
            if( tree->left == NULL )
            {
                // No tree is created yet.
                return -1;
            }
            else
            {
                current_tree_node = tree;
                while (anInterval->e < current_tree_node->key
                       || current_tree_node->key < anInterval->b) {
                    if (anInterval->e < current_tree_node->key)
                        current_tree_node = current_tree_node->left;
                    else
                        current_tree_node = current_tree_node->right;
                }
                // We must reach to a node.
                assert(current_tree_node != NULL);
                if (isRight == 0)
                {
                    // Insert a new list node at the front of the left list.
                    list_node_t *t = current_tree_node->left_list;
                    list_node_t *newListNode = blockListGetNode();
                    newListNode->key = anInterval->b;
                    newListNode->object = *anInterval;
                    current_tree_node->left_list = newListNode;
                    newListNode->left = t;
                }
                else
                {
                    // Insert a new list node at the front of the left list.
                    list_node_t *t = current_tree_node->right_list;
                    list_node_t *newListNode = blockListGetNode();
                    newListNode->key = anInterval->e;
                    newListNode->object = *anInterval;
                    current_tree_node->right_list = newListNode;
                    newListNode->left = t;
                }
            }
        }        
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
        /* found the candidate leaf. Test whether
         key distinct */
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
                remove_list( current_node->left->left_list );
                remove_list( current_node->left->right_list );
                blockTreeReturnNode( current_node->left );
                tmp = current_node->right;
                remove_list( current_node->left_list );
                remove_list( current_node->right_list );
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

