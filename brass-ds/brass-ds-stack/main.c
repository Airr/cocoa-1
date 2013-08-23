//
//  main.c
//  brass-ds-stack
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "stack.h"

int main(int argc, const char * argv[])
{
    for (size_t k = 0; k < 10000; k++)
    {
        node_t *s = create_stack();
        if (stack_empty(s))
            printf("Stack is empty.\n");
        
        for (item_t i = 0; i < 1000; i++)
            push(i, s);
        
        for (item_t i = 0; i < 1000; i++)
        {
            item_t v = pop(s);
            printf("%d ", v);
        }
        printf("\n");
        free_node();
    }
    return 0;
}

