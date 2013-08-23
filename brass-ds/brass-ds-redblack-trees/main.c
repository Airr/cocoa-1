//
//  main.c
//  brass-ds-redblack-trees
//
//  Created by Sang Chul Choi on 8/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "redblack_trees.h"

int main(int argc, const char * argv[])
{
//    srand((unsigned int)time(NULL));
    srand(1);
    
    for (size_t k = 0; k < 1000000; k++) {
        tree_node_t *t = create_tree();
        for (key_t i = 0; i < 100; i++) {
            key_t j = (key_t)rand();
            insert(t, j, t);
        }
// brass-ds-redblack-trees | sed 's/0x//g' | dot -Tpdf -o 1.pdf; open 1.pdf
//        print_tree(t);
        remove_tree(t);
    }
    free_tree();
    return 0;
}

