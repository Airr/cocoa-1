//
//  main.c
//  openmp
//
//  Created by Sang Chul Choi on 8/8/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
int main(int argc, const char * argv[])
{
    // insert code here...
    const long N = 9268547758;
    long a[100000];
    long li;
    long int delta;
    
    struct timeval begin, end;
    gettimeofday(&begin, NULL);;
//    #pragma omp parallel for
    for (li = 0; li < N; li++)
    {
        a[li%100000] = 2*li;
    }
    gettimeofday(&end, NULL);
    delta = ((end.tv_sec  - begin.tv_sec) * 1000000u +
             end.tv_usec - begin.tv_usec);

    
    printf("%ld %d %ld\n", delta, INT_MAX, LONG_MAX);
    return 0;
}

