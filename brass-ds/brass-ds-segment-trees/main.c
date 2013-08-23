//
//  main.c
//  brass-ds-segment-trees
//
//  Created by Sang Chul Choi on 8/17/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "segment_trees.h"

#define OBJECTSIZE 11

int main(int argc, const char * argv[])
{
    int status;
    object_t a[OBJECTSIZE] = {
        {.b = 1, .e = 9},
        {.b = 2, .e = 4},
        {.b = 3, .e = 7},
        {.b = 3, .e = 11},
        {.b = 4, .e = 12},
        {.b = 5, .e = 6},
        {.b = 7, .e = 16},
        {.b = 8, .e = 9},
        {.b = 10, .e = 15},
        {.b = 13, .e = 15},
        {.b = 14, .e = 16}
    };
    
    for (size_t k = 0; k < 10000000; k++)
    {
        size_t numberOfFailedStatus = 0;
        
        tree_node_t *t = create_tree();
        for (size_t i = 0; i < OBJECTSIZE; i++) {
            tree_node_t *leaf = blockTreeGetNode();
            leaf->key = 100 + i;
            status = insert(t, a[i].b, leaf);
            if (status != 0)
            {
                blockTreeReturnNode(leaf);
                numberOfFailedStatus++;
            }
            leaf = blockTreeGetNode();
            leaf->key = 200 + i;
            status = insert(t, a[i].e, leaf);
            if (status != 0)
            {
                blockTreeReturnNode(leaf);
                numberOfFailedStatus++;
            }
        }
//        printf("Failed status: %zu\n", numberOfFailedStatus);
        // sort a by .b
        // sort a by .e
        
        insertInterval(t, a, OBJECTSIZE);
        
        for (key_t k = 1; k <= 16; k++) {
            list_node_t *l = find_intervals(t, k);
//            printf("Query: %llu\n", k);
            for (list_node_t *il = l; il != NULL; il = il->left)
            {
//                printf("  List: [%llu, %llu]\n", il->key_a, il->key_b);
            }
            remove_list(l);
        }
        
        remove_tree(t);
//        blockTreeReturnNode(t);
        checkListRC();
        checkTreeRC();
    }
   
    blockListFreeNode();
    blockTreeFreeNode();
    return 0;
}

