//
//  redblack_trees.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

// Given a set S = {s1, s2, ...} of possibly overlapping intervals on a line
// we consider the problem of maintaining the union of the intervals in S.
// The union of intervals is defined to be the set of intervals that are created
// by merging intervals in the set S. Note that the union is not the term in
// the sense of the set theory.
//
// S    : a set of intervals
// T    : a red-black tree
// si   : i-th interval in the set S.
// l(si): the leaf sotring the left endpoint of si
// r(si): the leaf storing the right endpoint of si
// v    : an internal node of T
// T(v) : the subtree of T rooted at v
// S(v) : the set of intervals (or a subset of S) such that l(si) or r(si) lies in T(v)
//        We say that a leaf lies in a tree as we can reach from the leaf to the root of the tree.
// U(v) : the union of intervals in S(v)
//
// left(v) : the left child of node v
// right(v): the right child of node v
// p(v)    : the parent of of node v
// min(v)  : pointer to the leftmost leaf of T(v), we can reach min(v) from v
//           by repeatedly using left() function
// max(v)  : pointer to the rightmost leaf of T(v), we can reach max(v) from v
//           by using right() function
// start(v): points to the left endpoint of the rightmost interval in U(v).
//           Note that U(v) contains merged intervals not ones in S.
// end(v)  : points to the right endpoint of the leftmost interval in U(v).
// left_cross(v) : consider intervals in S(v) whose right endpoint is to the right of max(v).
//                 Choose the interval whose right endpoint is the largest or
//                 far to the right. The left_cross(v) points to the left
//                 endpoint of the interval.
// right_cross(v): consider intervals in S(v) whose left endpoint is to the left of min(v).
//                 Choose the interval whose left endpoint is the smallest or
//                 far to the left. The right_cross(v) points to the right
//                 endpoint of the interval.
//
// Consider a leaf w that is a left endpoint of an interval: w=l(si).
// min(w) = max(w) = start(w) = left_cross(w) = w
// end(w) = right_cross(w) = NULL
// R(w) = r(si).
// transfer(w): Consider all of the ancestors of w. If they satisfy the following
//              condition - start(left(v))=w and left_cross(left(v)) is not NULL,
//              then transfer(w)=v.
// List(w): a list that starts at transfer(w).
// next_down(v): If v is not a leaf and start(left(v)) is not NULL, and
//               left_cross(left(v)) is not NULL, then next_down(v) points to
//               the nearest or highest descendant u of v such that
//               start(left(u))=start(left(v)), and left_cross(left(u)) is not NULL.
//               Otherwise, next_down(v)=NULL.
//
// Consider a leaf w that is a right endpoint of an interval: w=r(si).
// L(w) = l(si).
// succ(w) points to the lext leaf in a sorted left-to-right order.
// 
//
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"
#include "union_tree.h"

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
    tmp = {
        .key = -1,
        .left = NULL,
        .right = NULL,
        .min = NULL,
        .max = NULL,
        .start = NULL,
        .end = NULL,
        .left_cross = NULL,
        .right_cross = NULL,
        .transfer = NULL,
        .next_down = NULL,
        .succ = NULL,
        .L = NULL,
        .R = NULL,
        .color = red
    };
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
        /* else: current_node is upper node of red-red conflict*/
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

#pragma mark Union Tree

tree_node_t *create_union_tree(interval_t *I, size_t sizeI)
{
    tree_node_t *tree = create_tree();
    for (size_t i = 0; i < sizeI; i++) {
        stack_node_t *sB = create_stack();
        stack_node_t *sE = create_stack();
        insert_search_tree(tree, I[i].b, tree, sB);
        insert_search_tree(tree, I[i].e, tree, sE);
        
        update_union_tree(tree, sB, sE);
//        update_union_tree(tree, sE);
        
        remove_stack(sB);
        remove_stack(sE);
        
        // 
    }
    return tree;
}

int update_union_tree(tree_node_t *tree, stack_node_t *stackB, stack_node_t *stackE)
{
    stack_node_t *stackPointB = stackB;
    stack_node_t *stackPointE = stackE;
    stack_node_t *stackPoint;
    tree_node_t *current;
    tree_node_t *intervalB;
    tree_node_t *intervalE;
    tree_node_t *x;
    tree_node_t *y;
    tree_node_t *z;
    tree_node_t *xo;
    tree_node_t *yo;
    tree_node_t *zo;
    tree_node_t *w;
    tree_node_t *xs; // for swap
    int finished = 0;
    
    if( tree->left == NULL )
    {
        // Tree is not built yet.
        assert(0);
    }
    else
    {
        // FIXME: check this for
        // two cases.
        // one is two nodes are the two endpoints in an interval.
        // the other is that they are part of different intervals.
        for (size_t isBeginOfInterval = 0; isBeginOfInterval < 2; isBeginOfInterval++) {
            if (isBeginOfInterval == 0) {
                x = top_element(stackPointB);
                xo = top_element(stackPointE);
            } else {
                x = top_element(stackPointE);
                xo = top_element(stackPointB);
                if (x == xo) {
                    break;
                }
            }
            assert(x->left != NULL);
            assert(x->right != NULL);
            assert(xo->left != NULL);
            assert(xo->right != NULL);
            y = x->left;
            z = x->right;
            yo = xo->left;
            zo = xo->right;
            if (x == xo) {
                intervalE = z;
            }
            else
            {
                if (yo->min == NULL) {
                    assert(zo->min != NULL);
                    intervalE = yo;
                }
                else {
                    assert(zo->min == NULL);
                    intervalE = zo;
                }
            }
            assert(y->left != NULL && y->right == NULL);
            assert(z->left != NULL && z->right == NULL);
            if (x == xo) {
                y->L = NULL;
                y->R = z;
                y->transfer = x; // ????
                y->next_down = NULL;
                y->succ = NULL;
                z->L = y;
                z->R = NULL;
                z->transfer = NULL;
                z->next_down = NULL;
                z->succ = NULL; // ???
            } else {
                if (y->min == NULL) {
                    y->L = NULL;
                    y->R = intervalE;
                    y->transfer = x; // ????
                    y->next_down = NULL;
                    y->succ = NULL;
                    //                z->L = y;
                    //                z->R = NULL;
                    //                z->transfer = NULL;
                    //                z->next_down = NULL;
                    //                z->succ = NULL; // ???
                } else {
                    z->L = NULL;
                    z->R = intervalE;
                    z->transfer = NULL;
                    z->next_down = NULL;
                    z->succ = NULL; // ???
                }
            }
            y->min = y;
            y->max = y;
            y->start = y;
            y->end = NULL;
            y->left_cross = y
            y->right_cross = NULL;
            
            z->min = z;
            z->max = z;
            z->start = NULL;
            z->end = z;
            z->left_cross = NULL;
            z->right_cross = z;
        }
        
        ////////////////////////////////////////////////////////////////////////
        // Now, go up to the root starting at x.
        // Since the leftmost leaf in T(y) is also the leftmost leaf in T(x),
        // we set min(x) = min(y).
        //
        // FIXME: stackPointE?
        // FIXME: symmetry?
        for (size_t isBeginOfInterval = 0; isBeginOfInterval < 2; isBeginOfInterval++) {
            if (isBeginOfInterval == 0) {
                stackPoint = stackPointB;
            } else {
                stackPoint = stackPointE;
            }
            while (!last_empty(stackPoint))
            {
                x->min = y->min;
                x->max = z->max;
                if (y->left_cross == NULL || y->left_cross->R->key <= z->max->key) {
                    x->left_cross = z->left_cross;
                } else {
                    if (z->left_cross == NULL || y->left_cross->R->key > z->left_cross->key) {
                        x->left_cross = y->left_cross;
                    } else {
                        x->left_cross = z->left_cross;
                    }
                }
                // JUST GUESS!!!???
                // y->min   <---> z->max
                // y->left_cross ---> z->right_cross
                // z->left_cross ---> y->right_cross
                // Switch operant in the comparison.
                if (z->right_cross == NULL || y->min->key <= z->right_cross->L->key) {
                    x->right_cross = y->right_cross;
                } else {
                    if (y->right_cross == NULL || y->right_cross->key > z->right_cross->L->key) {
                        x->right_cross = z->right_cross;
                    } else {
                        x->right_cross = y->right_cross;
                    }
                }
                
                if (z->start == NULL) {
                    if (z->right_cross->L->key < y->min->key) {
                        x->start = NULL;
                    } else {
                        x->start = y->start;
                    }
                } else {
                    if (y->left_cross != NULL && y->left_cross->R->key >= z->max->key) {
                        x->start = y->start;
                    } else {
                        x->start = z->start;
                    }
                }
                // JUST GUESS!!!???
                // z->start ---> y->end
                // y->min   <---> z->max
                // y->left_cross ---> z->right_cross
                // Switch operant in the comparison.
                if (y->end == NULL) {
                    if (z->max->key < y->left_cross->R->key) {
                        x->end = NULL;
                    } else {
                        x->end = z->end;
                    }
                } else {
                    if (z->right_cross != NULL && y->min->key >= z->right_cross->L->key) {
                        x->end = z->end;
                    } else {
                        x->end = y->end;
                    }
                }
                
                w = x->left->start;
                if (x->left->left_cross != NULL) {
                    x->next_down = w->transfer;
                    w->transfer = x;
                }
                
                x->succ = NULL;
                
                x = next_element(&stackPoint);
                if (stackPoint != NULL)
                {
                    assert(x->left != NULL);
                    assert(x->right != NULL);
                    assert(xo->left != NULL);
                    assert(xo->right != NULL);
                    y = x->left;
                    z = x->right;
                    yo = xo->left;
                    zo = xo->right;
                }
            }
        }
        
        // Red-black tree balancing
        for (size_t isBeginOfInterval = 0; isBeginOfInterval < 2; isBeginOfInterval++) {
            if (isBeginOfInterval == 0) {
                stackPoint = stackPointB;
            } else {
                stackPoint = stackPointE;
            }
            current = pop(stackPoint);
            
            if( current->color == black || current == tree )
                finished = 1;
            
            while( !stack_empty(stackPoint) && !finished )
            {
                tree_node_t *upper_node, *other_node;
                upper_node = pop(stackPoint);
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
                            // If x is in List(start(y)),
                            // remove it from the list.
                            // Then, perform the rotation.
                            // Copy the pointers from x to yp.
                            // Reconfigure
                            // start(xp), end(xp),
                            // left_cross(xp), right_cross(xp), min(xp)
                            // by using left(xp), and right(xp).
                            // Finally, insert xp into List(start(left(xp)))
                            // if start(left(xp)) is not NULL, and
                            // left_cross(left(xp)) is not NULL.
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
                            // If y is in List(start(left(y))),
                            // remove it from the list
                            // by traversing Llist(start(left(y))).
                            // Then, perform the rotation.
                            // Copy pointers from x to yp.
                            // Reconfigure
                            // start(xp), end(xp),
                            // left_cross(xp), right_cross(xp), max(xp)
                            // by using left(xp), and right(xp).
                            x = upper_node;
                            y = x->right;
                            left_rotation( upper_node );
                            yp = x;
                            xp = yp->left;
                            assert(x == yp);
                            assert(y == xp);
                            if (xp->start != NULL && xp->left_cross != NULL) {
                                // Add yp in List(start(xp)).
                            }
                            
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
        }
    }
}

#pragma mark Search Trees

int insert_search_tree(tree_node_t *tree, key_t new_key, object_t *new_object, stack_node_t *stack)
{
    tree_node_t *current;
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
        current = tree;
        while( current->right != NULL )
        {
            push( current, stack );
            
            // Go down P from the root of T.
            // Whenever a node v is encountered such that
            // transfer(start(left(v))) = v,
            // set transfer(start(left(v))) to next_down(v).
            if (current->left->start->transfer == current)
            {
                current->left->start->transfer = current->next_down;
            }
            if( new_key < current->key )
                current = current->left;
            else
                current = current->right;
        }
        /* found the candidate leaf.
         Test whether key distinct */
        if( current->key == new_key )
            return( -1 );
        /* key is distinct,
         now perform the insert */
        {
            tree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node();
            old_leaf->left = current->left;
            old_leaf->key = current->key;
            old_leaf->right  = NULL;
            old_leaf->color = red;
            new_leaf = get_node();
            new_leaf->left = (tree_node_t *)
            new_object;
            new_leaf->key = new_key;
            new_leaf->right  = NULL;
            new_leaf->color = red;
            if( node->key < new_key )
            {
                current->left  = old_leaf;
                current->right = new_leaf;
                current->key = new_key;
            }
            else
            {
                current->left  = new_leaf;
                current->right = old_leaf;
            }
//            push(new_leaf, stack);
        }
        
        if( current->color == black || current == tree )
            finished = 1;
    }
    return( 0 );
}

tree_node_t *make_list_leaves(tree_node_t *tree)
{
    tree_node_t *list;
    tree_node_t *node;
    stack_node_t *s;
    uint64_t n = 0;
    uint64_t i = 0;
    if( tree->left == NULL )
    {
        return( NULL );
    }
    else
    {
        s = create_stack();
        push( tree, s );
        while( !stack_empty(s) )
        {
            node = pop(s);
            if( node->right == NULL )
            {
//                node->right = list;
                n++;
            }
            else
            {
                push( node->left, s );
                push( node->right, s );
            }
        }
    }
    list = malloc(n * sizeof(tree_node_t*));
    stack_node_t *s = create_stack();
    push( tree, s );
    while( !stack_empty(s) )
    {
        node = pop(s);
        if( node->right == NULL )
        {
            list[i++] = node;
        }
        else
        {
            push( node->left, s );
            push( node->right, s );
        }
    }
    return list;
}
