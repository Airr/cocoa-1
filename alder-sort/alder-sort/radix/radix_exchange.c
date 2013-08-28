//
//  radix_exchange.c
//  alder-sort
//
//  Created by Sang Chul Choi on 8/23/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAXN 1000000

static unsigned int array[ MAXN ];
static unsigned int n;

static void radix_exchange(int start, int end, unsigned int mask)
{
    /* Empty Cases */
    if (start>=end+1) return;
    if (!mask) return;
    
    int left, right;
    for(left=start, right=end;;)
    {
        /* Shift the pointers (partition the array) */
        while ( left<=right && !( array[left]  & mask) ) left++ ;
        while ( left<=right &&  ( array[right] & mask) ) right--;
        
        /* The two pointers meet => partitioned => recursive call */
        if (left>=right)
        {
            radix_exchange(start, left-1, mask>>1);
            radix_exchange(left, end, mask>>1);
            return;
        }
        else /* Exchange step */
        {
            array[left]  ^= array[right];;
            array[right] ^= array[left];
            array[left]  ^= array[right];
        }
    }
}

static void create_random()
{
    srand((unsigned)time(NULL));
    for (int i=0;i<n;i++)
        array[i]=rand()<<16 | rand();
}

static bool sorted()
{
    for (int i=0;i<n-1;i++)
        if (array[i]>array[i+1])
            return false;
    return true;
}

static void printit()
{
    for (int i=0;i<n;i++)
        printf("%u\n",array[i]);
}


int radix_exchange_main()
{
    n=20;
    create_random();
    printit();
    
    radix_exchange(0,n-1,1<<31);
    
    printf("Sorted? %s\n", sorted()?"Yes":"No");
    printit();
    
    return 0;
}