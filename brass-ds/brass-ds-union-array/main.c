//
//  main.c
//  brass-ds-union-array
//
//  Created by Sang Chul Choi on 8/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "union_array.h"

int test_bsearch_int64_t();
int test_union_array_t();
int test_union_array_find_t();

int main(int argc, const char * argv[])
{
//    test_bsearch_int64_t();
    
//    for (size_t i = 0; i < 100; i++) {
//        test_union_array_t();
    test_union_array_find_t();
//    }
    return 0;
}

int test_union_array_find_t()
{
    
    int64_t intervals[] = {
//        1,2, 3,4, 1,3
//        1,2, 4,5, 1,3
//        1,2, 4,5, 3,4
//        1,3, 7,9, 2,8
//        11,22
//        11,22, 3,5
//        11,22, 33,55, 77,99
//        11,22, 77,88, 33,55
        3,5, 9,12, 15,20
        
    };
    int64_t *I = alder_union_interval_init();
    
    for (size_t i = 0; i < sizeof(intervals)/sizeof(intervals[0])/2; i++) {
        alder_union_interval_add(&I, intervals[2*i], intervals[2*i+1]);
    }
    alder_union_interval_print(I);
    int64_t b, e;
    bool isWithin;
    alder_union_interval_find_endpoints_point(I, 4, &b, &e, &isWithin);
    printf("Point 4: [%lld - %lld]\n", b, e);
    alder_union_interval_find_endpoints_point(I, 8, &b, &e, &isWithin);
    printf("Point 8: [%lld - %lld]\n", e, b);
    alder_union_interval_find_endpoints_point(I, 1, &b, &e, &isWithin);
    printf("Point 1: [%lld - %lld]\n", e, b);
    alder_union_interval_find_endpoints_point(I, 22, &b, &e, &isWithin);
    printf("Point 22: [%lld - %lld]\n", e, b);
    
    
    alder_union_interval_free(I);
    
    return 0;
}

#define INTERVALSIZE 10
int test_union_array_t()
{
//    size_t max_size = (size_t)-1;
    
    int64_t intervals[] = {
//        1,2, 3,4, 1,3
//        1,2, 4,5, 1,3
//        1,2, 4,5, 3,4
//        1,3, 7,9, 2,8
//        11,22
//        11,22, 3,5
//        11,22, 33,55, 77,99
//        11,22, 77,88, 33,55
        
    };
//    int64_t *intervals = malloc(INTERVALSIZE*sizeof(int64_t));
//    if (intervals == NULL) {
//        fprintf(stderr, "memory error\n");
//        return 0;
//    }
//    for (size_t i = 0; i < sizeof(intervals)/sizeof(intervals[0]); i++) {
//        intervals[i] = i + 1;
//    }
    int64_t *I = alder_union_interval_init();
    
    for (size_t i = 0; i < sizeof(intervals)/sizeof(intervals[0])/2; i++) {
        alder_union_interval_add(&I, intervals[2*i], intervals[2*i+1]);
    }
    alder_union_interval_print(I);
    alder_union_interval_free(I);
    
//    free(intervals);
    return 0;
}

int test_bsearch_int64_t()
{
    
    int64_t I[] = { 1, 5, 9, 24, 39 };
    // insert code here...
    for (int64_t i = 0; i < 41; i++) {
        size_t k = bsearch_int64_t(i, I, 5);
        if (k == 0) {
            printf("key: %zd < %lld\n", i, I[k]);
        } else if (k == 5) {
            printf("key: %zd >= %lld\n", i, I[4]);
        } else {
            printf("key: %lld <= %zd < %lld\n", I[k-1], i, I[k]);
        }
    }
    size_t k = bsearch_int64_t(232, I, 0);
    printf("key: 232 can be insearted at %zd\n", k);
    printf("Hello, World!\n");
    
    
    return 0;
}
