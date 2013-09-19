//
//  main.c
//  brass-ds-stack-array
//
//  Created by Sang Chul Choi on 9/15/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//
#include <stdio.h>

#define MAXSIZE 100
typedef int item_t;
int i=0;
item_t stack[MAXSIZE];

int stack_empty(void)
{
    return( i == 0 );
}

int push( item_t x)
{
    if(i<MAXSIZE)
    {
        stack[i++] = x ;  return( 0 );
    }
    else
        return( -1 );
}

item_t pop(void)
{
    if (i > 0) {
        return( stack[--i] );
    } else {
        fprintf(stderr, "Error! The stack is empty!");
        return -1;
    }
}

item_t top_element(void)
{
    return( stack[i-1] );
}


int main(int argc, const char * argv[])
{
    char nextop;
    printf ("Made Fixed Array-Based Stack with size 100\n");
    while ((nextop = getchar ()) != 'q')
    {
        if (nextop == 'i')
        {
            int insitem;
            scanf (" %d", &insitem);
            push (insitem);
            printf (" pushed %d. The current top item is %d\n", insitem,
                    top_element ());
        }
        if (nextop == 'd')
        {
            int de_item;
            getchar ();
            de_item = pop ();
            printf ("  popped item %d", de_item);
            if (stack_empty ())
                printf (" the stack is now empty\n");
            else
                printf (" the top element is now %d\n", top_element ());
            
        }
        if (nextop == '?')
        {
            getchar ();
            if (stack_empty ())
                printf ("the stack is empty\n");
            else
                printf ("the top element is %d\n", top_element ());
        }
        
    }
    return (0);
}

