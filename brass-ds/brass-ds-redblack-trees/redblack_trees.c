//
//  redblack_trees.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"
#include "redblack_trees.h"

#pragma mark Tree Memory Management

#define BLOCKSIZE 10000

static tree_node_t **headblock = NULL;
static tree_node_t *currentblock = NULL;
static tree_node_t *freeblock = NULL;
int size_left;
static size_t sizeblock = 0;

static tree_node_t *get_node()
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
static void return_node(tree_node_t *node)
{
    node->left = freeblock;
    freeblock = node;
}

static void free_node()
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

void free_tree()
{
    free_node();
    free_stack();
}

#pragma mark Red-Black Trees

int insert(tree_node_t *tree, key_t new_key, object_t *new_object)
{
    tree_node_t *current_node;
    int finished = 0;
    if( tree->left == NULL )
    {
        tree->left = (tree_node_t *) new_object;
        tree->key  = new_key;
        tree->color = black;
        /* root is always black */
        tree->right  = NULL;
    }
    else
    {
        create_stack();
        current_node = tree;
        while( current_node->right != NULL )
        {
            push( current_node );
            if( new_key < current_node->key )
                current_node = current_node->left;
            else
                current_node = current_node->right;
        }
        /* found the candidate leaf.
         Test whether key distinct */
        if( current_node->key == new_key )
            return( -1 );
        /* key is distinct,
         now perform the insert */
        {
            tree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node();
            old_leaf->left = current_node->left;
            old_leaf->key = current_node->key;
            old_leaf->right  = NULL;
            old_leaf->color = red;
            new_leaf = get_node();
            new_leaf->left = (tree_node_t *)
            new_object;
            new_leaf->key = new_key;
            new_leaf->right  = NULL;
            new_leaf->color = red;
            if( current_node->key < new_key )
            {
                current_node->left  = old_leaf;
                current_node->right = new_leaf;
                current_node->key = new_key;
            }
            else
            {
                current_node->left  = new_leaf;
                current_node->right = old_leaf;
            }
        }
        /* rebalance */
        if( current_node->color == black || current_node == tree )
            finished = 1;
        /* else: current_node is upper node of
         red-red conflict*/
        while( !stack_empty() && !finished )
        {
            tree_node_t *upper_node, *other_node;
            upper_node = pop();
            if(upper_node->left->color == upper_node->right->color)
            { /* both red, and upper_node black */
                upper_node->left->color = black;
                upper_node->right->color = black;
                upper_node->color = red;
            }
            else /* current_node red,other_node black */
            {
                if( current_node == upper_node->left)
                {
                    other_node = upper_node->right;
                    /* other_node->color == black */
                    if( current_node->right->color == black )
                    {
                        right_rotation( upper_node );
                        upper_node->right->color = red;
                        upper_node->color = black;
                        finished = 1;
                    }
                    else /* current_node->right->color == red */
                    {
                        left_rotation( current_node );
                        right_rotation( upper_node );
                        upper_node->right->color = black;
                        upper_node->left->color = black;
                        upper_node->color = red;
                    }
                }
                else /* current_node == upper_node->right */
                {
                    other_node = upper_node->left;
                    /* other_node->color == black */
                    if( current_node->left->color == black )
                    {
                        left_rotation( upper_node );
                        upper_node->left->color = red;
                        upper_node->color = black;
                        finished = 1;
                    }
                    else /* current_node->left->color == red */
                    {
                        right_rotation( current_node );
                        left_rotation( upper_node );
                        upper_node->right->color = black;
                        upper_node->left->color = black;
                        upper_node->color = red;
                    }
                } /* end current_node left/right of upper */
                current_node = upper_node;
            }  /*end other_node red/black */
            if( !finished && !stack_empty() )
            /* upper is red, conflict possibly propagates upward */
            {
                current_node = pop();
                if( current_node->color == black )
                    finished = 1;
                /* no conflict above */
                /* else: current is upper node of red-red conflict*/
            }
        }   /* end while loop moving back to root */
        tree->color = black;  /* root is always black */
        remove_stack();
    }
    return( 0 );
}

object_t *delete(tree_node_t *tree, key_t delete_key)
{
    tree_node_t *current, *upper, *other;
    object_t *deleted_object = NULL;
    int finished = 0;
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
        create_stack();
        current = tree;
        while( current ->right != NULL )
        {
            push( current );
            upper = current;
            if( delete_key < current->key )
            {
                current = upper->left;
                other = upper->right;
            }
            else
            {
                current = upper->right;
                other = upper->left;
            }
        }
        if( current->key != delete_key )
        {
//            return( NULL );
            finished = 1;
        }
        else
        {
            if (other->color == red && current->color == red)
            {
                finished = 1;
            }
            pop(); // the leave node is not used.
            upper->key   = other->key;
            upper->left  = other->left;
            upper->right = other->right;
            
            deleted_object = (object_t *) current->left;
            return_node( current);
            return_node( other);
            
            current = pop();
            assert(current == upper);
//            return( deleted_object );
        }
        while( !stack_empty() && !finished )
        {
            upper = pop();
            // 1. If current is red, we color it black.
            if (current->color == red)
            {
                current->color = black;
                finished = 1;
            }
            // 2. If current is the root, then (1) holds anyway.
            else if (current == tree)
            {
                finished = 1;
                assert(current->color == black);
            }
            else if (current == upper->left)
            {
                // 3.1 If upper is black, other is black, and other->left is black,
                // perform a left rotation around upper and color upper->left red
                // and upper black. Then the violation of (1) occurs in upper.
                if (upper->color == black
                    && other->color == black
                    && other->left->color == black) {
                    left_rotation(upper);
                    upper->left->color = red;
                    upper->color = black;
                }
                // 3.2 If upper is black, other is black, and other->left is red,
                // perform a right rotation around other,
                // followed by a left rotation around upper, and
                // color upper->left, upper->right and upper black.
                // Then (1) is restored.
                else if (upper->color == black
                         && other->color == black
                         && other->left->color == red)
                {
                    left_rotation(upper);
                    upper->left->color = black;
                    upper->right->color = black;
                    upper->color = black;
                }
                // 3.3 If upper is black, other is red, and other->left->left is black,
                // perform a left rotation around upper,
                // followed by a left rotation around upper->left, and
                // color upper->left->left red, upper->left and upper black.
                // Then (1) is restored.
                else if (upper->color == black
                         && other->color == red
                         && other->left->left->color == black)
                {
                    left_rotation(upper);
                    left_rotation(upper->left);
                    upper->left->left->color = red;
                    upper->left->color = black;
                    upper->color = black;
                }
                // 3.4 If upper is black, other is red, and other->left->left is red,
                // perform a left rotation around upper,
                // followed by a right rotation around upper->left->right, and
                // a left rotation around upper->left, and
                // color upper->left->left and upper->left->right black,
                // upper->left red, and
                // upper black. Then (1) is restored.
                else if (upper->color == black
                         && other->color == red
                         && other->left->left->color == red)
                {
                    left_rotation(upper);
                    right_rotation(upper->left->right);
                    left_rotation(upper->left);
                    upper->left->left->color = black;
                    upper->left->right->color = black;
                    upper->left->color = red;
                    upper->color = black;
                }
                // 3.5 If upper is red, other is black, and other->left is black,
                // perform a left rotation around upper and
                // color upper->left red and upper black.
                // Then (1) is restored.
                else if (upper->color == red
                         && other->color == black
                         && other->left->color == black)
                {
                    left_rotation(upper);
                    upper->left->color = red;
                    upper->color = black;
                }
                // 3.6 If upper is red, other is black, and other->left is red,
                // perform a right rotation around other,
                // followed by a left rotation around upper, and
                // color upper->left and upper->right black and
                // upper red. Then (1) is restored.
                else if (upper->color == red
                         && other->color == black
                         && other->left->color == red)
                {
                    left_rotation(upper);
                    upper->left->color = black;
                    upper->right->color = black;
                    upper->color = red;
                }
                else
                {
                    assert(0);
                }
            }
            else if (current == upper->right)
            {
                // 4.1 If upper is black, other is black, and other->right is black,
                // perform a right rotation around upper and
                // color upper->right red and
                // upper black.
                // Then the violation of (1) occurs in upper.
                if (upper->color == black
                    && other->color == black
                    && other->right->color == black) {
                    right_rotation(upper);
                    upper->right->color = red;
                    upper->color = black;
                }
                // 4.2 If upper is black, other is black, and other->right is red,
                // perform a left rotation around other,
                // followed by a right rotation around upper, and
                // color upper->left, upper->right and upper black.
                // Then (1) is restored.
                else if (upper->color == black
                         && other->color == black
                         && other->right->color == red)
                {
                    left_rotation(other);
                    right_rotation(upper);
                    upper->left->color = black;
                    upper->right->color = black;
                    upper->color = black;
                }
                // 4.3 If upper is black, other is red, and other->right->right is black,
                // perform a right rotation around upper,
                // followed by a right rotation around upper->right, and
                // color upper->right->right red,
                // upper->right and upper black.
                // Then (1) is restored.
                else if (upper->color == black
                         && other->color == red
                         && other->right->right->color == black)
                {
                    right_rotation(upper);
                    right_rotation(upper->right);
                    upper->right->right->color = red;
                    upper->right->color = black;
                    upper->color = black;
                }
                // 4.4 If upper is black, other is red, and other->right->right is red,
                // perform a right rotation around upper,
                // followed by a left rotation around upper->right->left, and
                // a right rotation around upper->right, and
                // color upper->right->right and upper->right->left black,
                // upper->right red, and
                // upper black.
                // Then (1) is restored.
                else if (upper->color == black
                         && other->color == red
                         && other->right->right->color == red)
                {
                    right_rotation(upper);
                    left_rotation(upper->right->left);
                    right_rotation(upper->right);
                    upper->right->right->color = black;
                    upper->right->left->color = black;
                    upper->right->color = red;
                    upper->color = black;
                }
                // 4.5 If upper is red, other is black, and other->right is black,
                // perform a right rotation around upper and
                // color upper->right red and
                // upper black.
                // Then (1) is restored.
                else if (upper->color == red
                         && other->color == black
                         && other->right->color == black)
                {
                    right_rotation(upper);
                    upper->right->color = red;
                    upper->color = black;
                }
                // 4.6 If upper is red, other is black, and other->right is red,
                // perform a left rotation around other,
                // followed by a right rotation around upper, and
                // color upper->left and upper->right black and
                // upper red.
                // Then (1) is restored.
                else if (upper->color == red
                         && other->color == black
                         && other->right->color == red)
                {
                    left_rotation(other);
                    right_rotation(upper);
                    upper->left->color = black;
                    upper->right->color = black;
                    upper->color = red;
                }
                else
                {
                    assert(0);
                }
            }
            current= upper;
        }
//        if( !finished && !stack_empty() )
//        {
//            current = pop();
//            if( current->color == black )
//                finished = 1;
//        }
        remove_stack();
    }
    return deleted_object;
}

#pragma Search Trees

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

void print_tree(tree_node_t *tree)
{
    tree_node_t *node;
    printf("digraph g {\n");
    if( tree->left == NULL )
    {
        return;
    }
    else
    {
        create_stack();
        push( tree );
        while( !stack_empty() )
        {
            node = pop();
            if( node->right != NULL )
            {
                printf("\"%p\" -> \"%p\";\n", node, node->left);
                printf("\"%p\" -> \"%p\";\n", node, node->right);
                push( node->left );
                push( node->right );
            }
        }
        remove_stack();
    }
    printf("}\n");
}
