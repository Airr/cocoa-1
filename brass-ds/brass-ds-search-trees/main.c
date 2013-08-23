//
//  main.c
//  brass-ds-search-trees
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "search_trees.h"

int main(int argc, const char * argv[])
{
    for (size_t k = 0; k < 10000; k++)
    {
        tree_node_t *t = create_tree();
        for (key_t i = 0; i < 1000; i++) {
            object_t *n = get_node();
            n->key = i * 2;
            insert(t, i, n);
        }
        object_t *foundObject = find(t, 5);
        if (foundObject != NULL)
        {
//            printf("A value %llu found at 5\n", foundObject->key);
        }
        remove_tree(t);
        free_node();
    }
    return 0;
}

