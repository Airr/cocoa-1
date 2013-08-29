//
//  union_array.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/21/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "union_array.h"


interval_t alder_union_interval_init()
{
    interval_t I = malloc((UNIONINTERVALSIZE*2 + 2)*sizeof(int64_t));
    if (I == NULL) {
        return NULL;
    }
    I[0] = UNIONINTERVALSIZE*2 + 2;
    I[1] = 0;
    return I;
}

void alder_union_interval_reset(interval_t I)
{
    I[1] = 0;
}

int comparePoint(const void *a, const void *b)
{
    int64_t *x = (int64_t*)a;
    int64_t *y = (int64_t*)b;
    if (*x < *y)
        return -1;
    else if (*x > *y)
        return 1;
    return 0;
}

// key: a key value
// v: already sorted array of size nel.
// nel: array size
// return: r = 0 if key < min(v)
//         r = nel if key >= max(v)
//         otherwise, r (0 < r < nel) such that v[r-1] <= key < v[r].
size_t bsearch_int64_t(int64_t key, int64_t *v, size_t nel)
{
    size_t lowerbound = 0;
    size_t upperbound = nel - 1;
    size_t testpos;
    if (nel == 0 || v[upperbound] <= key) {
        upperbound = nel;
        lowerbound = upperbound;
    }
    while (lowerbound < upperbound)
    {
        testpos = lowerbound + ((upperbound-lowerbound) / 2);
        
        if (v[testpos] > key)
        {
            //  new best-so-far
            upperbound = testpos;
        }
        else
        {
            lowerbound = testpos + 1;
        }
    }
    assert(lowerbound == upperbound);
//    printf("----------\n");
//    printf("key: %lld\n", key);
//    printf("upperbound: %zd\n", upperbound);
    return upperbound;
}

int alder_union_interval_add(interval_t *refI, int64_t b, int64_t e)
{
    interval_t I = *refI;
    const int64_t capacity = I[0]; // length of the array
    const int64_t size = I[1] + 2; // number of endpoints + two for the header
    const size_t n = (size_t)((double)size * UNIONINTERVALINCSIZE);
    if (capacity < n) {
        assert(2*n > capacity);
        // Resize a->block
        interval_t t = realloc(I, 2*n*sizeof(int64_t));
        if (t == NULL) {
            return 1;
        }
        *refI = t;
        I = *refI;
//        I = t;
        I[0] = (int64_t)(2*n);
    }
    const size_t sizeI = I[1];
    assert(sizeI + 2 + 2 <= I[0]);

    // Add two endpoints to the union interval set.
    const size_t indexB = bsearch_int64_t(b, I+2, sizeI);
    const int64_t db = indexB%2 == 1 ? ((int64_t)indexB-1)/2 : ((int64_t)indexB-2)/2;
    const int64_t *sdb = sizeI > 0 ? bsearch(&b, I+2, sizeI, sizeof(int64_t), comparePoint) : NULL;
    const bool isWithinIntervalB = indexB%2==0 && sdb==NULL ? false : true;
    
    const size_t indexE = bsearch_int64_t(e, I+2, sizeI);
    const int64_t de = indexE%2 == 1 ? ((int64_t)indexE-1)/2 : ((int64_t)indexE-2)/2;
    const int64_t *sde = sizeI > 0 ? bsearch(&e, I+2, sizeI, sizeof(int64_t), comparePoint) : NULL;
    const bool isWithinIntervalE = indexE%2==0 && sde==NULL ? false : true;
    
    const int64_t k = de - db;
    assert(db <= de);
    
    if (isWithinIntervalB == true && isWithinIntervalE == true) {
        // bB---b---bE    eB---e---eE
        int64_t *bB = I + 2*db + 2;
        int64_t *bE = I + 2*db + 3;
        int64_t *eB = I + 2*de + 2;
        int64_t *eE = I + 2*de + 3;
        assert(*bB <= b && b <= *bE);
        assert(*eB <= e && e <= *eE);
        if (k == 0) {
            // No code.
        } else {
            size_t tailSize = sizeI - (2*de + 1);
            memmove(bE, eE, tailSize*sizeof(int64_t));
        }
        I[1] -= k;
    } else if (isWithinIntervalB == true && isWithinIntervalE == false) {
        // bB---b---bE    eE---e---eB
        int64_t *bB = I + 2*db + 2;
        int64_t *bE = I + 2*db + 3;
        int64_t *eE = I + 2*de + 3;
//        int64_t *eB = 2*de + 4 < sizeI + 2 ? I + 2*de+ 4: NULL;
        assert(*bB <= b && b <= *bE);
        assert(*eE < e);
//        if (eB == NULL) {
//            assert(*eE < e);
//        } else {
//            assert(*eE < e && e < *eB);
//        }
        if (k == 0) {
            // No code.
        } else {
            size_t tailSize = sizeI - (2*de + 1);
            assert(tailSize > 0);
            memmove(bE, eE, tailSize*sizeof(int64_t));
        }
        *bE = e;
        I[1] -= k;
    } else if (isWithinIntervalB == false && isWithinIntervalE == true) {
        // bE---b---bB    eB---e---eE
//        int64_t *bE = 2*db + 3 < 2 ? NULL : I + 2*db + 3;
        int64_t *bB = I + 2*db + 4;
        int64_t *eB = I + 2*de + 2;
        int64_t *eE = I + 2*de + 3;
        assert(b < *bB);
        assert(*eB <= e && e <= *eE);
        assert(k > 0);
        if (k == 1) {
            // No code.
        } else if (k > 1) {
            size_t tailSize = sizeI - (2*de);
            assert(tailSize > 1);
            memmove(bB, eB, tailSize*sizeof(int64_t));
        }
        *bB = b;
        I[1] -= (k-1);
    } else if (isWithinIntervalB == false && isWithinIntervalE == false) {
        // bE---b---bB    eE---e---eB
//        int64_t *bE = 2*db + 3 < 2 ? NULL : I + 2*db + 3;
        int64_t *bB = I + 2*db + 4;
        int64_t *eE = 2*de + 3 < 2 ? NULL : I + 2*de + 3;
//        int64_t *eB = I + 2*de + 4;
        if (k == 0) {
            size_t tailSize = sizeI - (2*de + 2);
            // tailSize can be zero, and memmove has no effect safely.
            memmove(bB + 2, bB, tailSize*sizeof(int64_t));
            I[1] += 2;
        } else if (k == 1) {
            // No code.
        } else if (k > 1) {
            size_t tailSize = sizeI - (2*de + 1);
            assert(tailSize > 0);
            memmove(bB+1, eE, tailSize*sizeof(int64_t));
            I[1] -= (k-1);
        }
        *bB = b;
        *(bB + 1) = e;
    }
    
    return 0;
}

void alder_union_interval_free(interval_t I)
{
    if (I != NULL) {
        free(I);
    }
}

int alder_union_interval_print(interval_t I)
{
    int64_t capacity = I[0];
    int64_t sizeI = I[1]; // number of endpoints + two for the header
    printf("Capacity: %lld\n", capacity);
    printf("Size: %lld\n", sizeI);
    for (size_t i = 0; i < sizeI/2; i++) {
        printf("[%lld - %lld]\n", I[2+i*2], I[2+i*2+1]);
    }
    return 0;
}

int alder_union_interval_find_interval_point(interval_t I, int64_t b,
                                             int64_t *indexI,
                                             bool *isWithinInterval)
{
//    const int64_t capacity = I[0]; // length of the array
//    const int64_t size = I[1] + 2; // number of endpoints + two for the header
    const size_t sizeI = I[1];
    const size_t indexB = bsearch_int64_t(b, I+2, sizeI);
    const int64_t db = indexB%2 == 1 ? ((int64_t)indexB-1)/2 : ((int64_t)indexB-2)/2;
    const int64_t *sdb = sizeI > 0 ? bsearch(&b, I+2, sizeI, sizeof(int64_t), comparePoint) : NULL;
    const bool isWithinIntervalB = indexB%2==0 && sdb==NULL ? false : true;
    *indexI = db;
    *isWithinInterval = isWithinIntervalB;
    return 0;
}

int alder_union_interval_find_endpoints_point(interval_t I, int64_t p,
                                              int64_t *b, int64_t *e,
                                              bool *isWithinInterval)
{
//    const int64_t capacity = I[0]; // length of the array
//    const int64_t size = I[1] + 2; // number of endpoints + two for the header
    const size_t sizeI = I[1];
    const size_t indexB = bsearch_int64_t(p, I+2, sizeI);
    const int64_t db = indexB%2 == 1 ? ((int64_t)indexB-1)/2 : ((int64_t)indexB-2)/2;
    const int64_t *sdb = sizeI > 0 ? bsearch(&b, I+2, sizeI, sizeof(int64_t), comparePoint) : NULL;
    const bool isWithinIntervalB = indexB%2==0 && sdb==NULL ? false : true;
    if (isWithinIntervalB == true) {
        *b = I[2 + 2*db];
        *e = I[2 + 2*db + 1];
    } else {
        if (db == -1) {
            *e = 0;
            *b = I[2 + 2*db + 2] - 1;
        } else if ((sizeI-2)/2 == db) {
            *e = I[2 + 2*db + 1] + 1;
            *b = 0;
        } else {
            *e = I[2 + 2*db + 1] + 1;
            *b = I[2 + 2*db + 2] - 1;
        }
    }
    *isWithinInterval = isWithinIntervalB;
    return 0;
}
