//
//  main.c
//  brass-ds-height-balanced-tree
//
//  Created by Sang Chul Choi on 8/18/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "balanced_tree.h"
#define TREE
#include "../brass-ds-stack/stack.h"

int main(int argc, const char * argv[])
{
    srand(1);
    
    for (size_t k = 0; k < 100000; k++) {
        tree_node_t *t = create_tree();
        for (key_t i = 0; i < 1000; i++) {
            key_t j = (key_t)rand();
            //        object_t *n = get_node();
            //        object_t *n = NULL;
            //        n->key = i * 2;
            insert(t, j, t);
        }
        //    object_t *foundObject = find(t, 5);
        //    if (foundObject != NULL)
        //    {
        //        //            printf("A value %llu found at 5\n", foundObject->key);
        //    }
        remove_tree(t);
    }
    
    free_stack();
    free_tree();
    return 0;
}

